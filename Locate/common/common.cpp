//
// Common routines for locate
//
// Copyright 2006-2007 Janne Huttunen


#include <HFCLib.h>
#include "common.h"

#include <parsers.h>

extern "C" {
	#include "JpegLib/jpeglib.h"
}
#ifdef _DEBUG
#pragma comment(lib, "libjpegd.lib")
#else
#pragma comment(lib, "libjpeg.lib")
#endif





typedef HRESULT (__stdcall * PFNSHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPWSTR);  // "SHGetFolderPathW"


LPWSTR GetDefaultFileLocation(LPCWSTR szFileName,BOOL bMustExists,DWORD* lpdwSize)
{
	int nFileNameLen=istrlen(szFileName);
	
	// Check first that is there 
	PFNSHGETFOLDERPATH pGetFolderPath=(PFNSHGETFOLDERPATH)GetProcAddress(GetModuleHandle("shell32.dll"),"SHGetFolderPathW");
	if (pGetFolderPath!=NULL)
	{
		WCHAR szAppDataPath[MAX_PATH];
		if (SUCCEEDED(pGetFolderPath(NULL,CSIDL_APPDATA,NULL,
			SHGFP_TYPE_CURRENT,szAppDataPath)))
		{
			int nPathLen=istrlen(szAppDataPath);

			// Insert \\ if already not there
			if (szAppDataPath[nPathLen-1]!=L'\\')
				szAppDataPath[nPathLen++]=L'\\';
			
			// Buffer for default path
			LPWSTR pStr=new WCHAR[nPathLen+9+nFileNameLen+1];
			MemCopyW(pStr,szAppDataPath,nPathLen);
			MemCopyW(pStr+nPathLen,L"Locate32",9);
			nPathLen+=8;

			// Now pStr contains X:\UsersFolder\AppPath\Locate32

			// Check whether directory exists
			if (FileSystem::IsDirectory(pStr))
			{
				// Directory does exist
				// Copy file name part at tail 
				pStr[nPathLen++]='\\';
				MemCopyW(pStr+nPathLen,szFileName,nFileNameLen+1);
				if (lpdwSize!=NULL)
					*lpdwSize=nPathLen+nFileNameLen;
				
				if (!bMustExists)
					return pStr;	

				// Checking file
				if (FileSystem::IsFile(pStr))
					return pStr;
			}
			else
			{
				// Create directory if does not already exists 
				// and bMustExists is not set (this function is called
				// for default databases with bMustExists=FALSE)
				if (!bMustExists)
				{
					FileSystem::CreateDirectory(pStr);
					// Copy file name part at tail 
					pStr[nPathLen++]='\\';
					MemCopyW(pStr+nPathLen,szFileName,nFileNameLen+1);
					if (lpdwSize!=NULL)
						*lpdwSize=nPathLen+nFileNameLen;
					
					return pStr;
				}	
			
			}
				

			delete[] pStr;
		}
	}

	// Check also Locate32's directory, maybe there is that file
	int iLen;
	LPWSTR pStr;
	WCHAR szExeName[MAX_PATH];
	FileSystem::GetModuleFileName(NULL,szExeName,MAX_PATH);
	iLen=LastCharIndex(szExeName,L'\\')+1;
	pStr=new WCHAR[iLen+nFileNameLen+1];
	MemCopyW(pStr,szExeName,iLen);
	
	MemCopyW(pStr+iLen,szFileName,nFileNameLen+1);
	if (lpdwSize!=NULL)
		*lpdwSize=iLen+nFileNameLen;

	if (!bMustExists)
		return pStr;	

	if (FileSystem::IsFile(pStr))
		return pStr;

	// Return NULL
	delete[] pStr;
	return NULL;
}


LPSTR ReadIniFile(LPSTR* pFile,LPCSTR szSection,BYTE& bFileIsReg)
{
	CAutoPtrA<WCHAR> pIniFile=GetDefaultFileLocation(L"locate.ini",TRUE);
	if (pIniFile==NULL)
		return NULL;

	bFileIsReg=TRUE;

	char* pFileContent=NULL;
	try
	{
		CFile Ini(pIniFile,CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
		DWORD dwSize=Ini.GetLength();
		pFileContent=new char[dwSize+1];
		Ini.Read(pFileContent,dwSize);
		pFileContent[dwSize]='\0';
		Ini.Close();
	}
	catch (...)
	{
		if (pFileContent!=NULL)
			delete[] pFileContent;
		return NULL;
	}
		

	LPCSTR pPtr=NULL;
	LPSTR pKeyName=NULL;

	CString Key,Value;

	if (szSection!=NULL)
		pPtr=FindSectionStart(pFileContent,szSection);
	if (pPtr==NULL)
		pPtr=FindSectionStart(pFileContent,"DEFAULT");

	while (pPtr!=NULL)
	{
		pPtr=FindNextValue(pPtr,Key,Value);
		if (Key.IsEmpty())
			break;

		while (Value.LastChar()==' ')
			Value.DelLastChar();

		if (Key.CompareNoCase("KEY")==0)
			pKeyName=Value.GiveBuffer();
		else if (Key.CompareNoCase("FILE")==0)
		{
			if (pFile!=NULL)
			{
				if (Value.FindFirst('\\')==-1)
				{
					int nDirLength=LastCharIndex((LPCWSTR)pIniFile,L'\\')+1;
					*pFile=new CHAR[nDirLength+Value.GetLength()+1];

					MemCopyWtoA(*pFile,pIniFile,nDirLength);
					MemCopy(*pFile+nDirLength,(LPCSTR)Value,Value.GetLength()+1);

				}
				else
					*pFile=Value.GiveBuffer();
			}
		}
		else if (Key.CompareNoCase("FILETYPE")==0)
		{
			if (Value.CompareNoCase("BIN")==0)
				bFileIsReg=FALSE;
			else if (Value.CompareNoCase("REG")==0)
				bFileIsReg=TRUE;
		}
	}
	
	delete[] pFileContent;
	return pKeyName;		
}

BOOL LoadSettingsFromFile(LPCSTR szKey,LPCSTR szFile,BYTE bFileIsReg)
{
	HKEY hKey;
	LONG lRet=RegOpenKeyEx(HKEY_CURRENT_USER,szKey,
		0,KEY_READ,&hKey);
	
	// Check wheter key exists
	if (lRet!=ERROR_FILE_NOT_FOUND)
	{
		// Key exists, using it
		RegCloseKey(hKey);
		return TRUE;
	}

	if (bFileIsReg)
	{
				
		// Restore key
		char szCommand[2000];
		sprintf_s(szCommand,2000,"regedit /s \"%s\"",szFile);

		PROCESS_INFORMATION pi;
		STARTUPINFO si; // Ansi and Unicode versions are same
		ZeroMemory(&si,sizeof(STARTUPINFO));
		si.cb=sizeof(STARTUPINFO);
		
		if (CreateProcess(NULL,szCommand,NULL,
			NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS,
			NULL,NULL,&si,&pi))
		{
			WaitForSingleObject(pi.hProcess,2000);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);	
		}
		else
			return FALSE;

		return TRUE;
	}

	// Acquiring required privileges	
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
	{
		PTOKEN_PRIVILEGES ns=(PTOKEN_PRIVILEGES)new BYTE[sizeof(DWORD)+sizeof(LUID_AND_ATTRIBUTES)+2];
		if (LookupPrivilegeValue(NULL,SE_BACKUP_NAME,&(ns->Privileges[0].Luid)))
		{
			ns->PrivilegeCount=1;
			ns->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
			if (!AdjustTokenPrivileges(hToken,FALSE,ns,0,NULL,NULL))
			{
				//ShowError(NULL,IDS_ERRORCANNOTENABLEPRIVILEGE,GetLastError());
			}
		}
		if (LookupPrivilegeValue(NULL,SE_RESTORE_NAME,&(ns->Privileges[0].Luid)))
		{
			ns->PrivilegeCount=1;
			ns->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
			if (!AdjustTokenPrivileges(hToken,FALSE,ns,0,NULL,NULL))
			{
				//ShowError(NULL,IDS_ERRORCANNOTENABLEPRIVILEGE,GetLastError());
			}

		}
		delete[] (BYTE*)ns;
		CloseHandle(hToken);
	}

	// First, check that we can restore key
	
	lRet=RegCreateKeyEx(HKEY_CURRENT_USER,szKey,
		0,NULL,REG_OPTION_BACKUP_RESTORE,KEY_ALL_ACCESS,NULL,&hKey,NULL);
	if (lRet!=ERROR_SUCCESS)
	{
		//ShowError(hWnd,IDS_ERRORCANNOTCREATEKEY,lRet);
		return FALSE;
	}
	lRet=RegRestoreKey(hKey,szFile,0);
	RegCloseKey(hKey);		
	
	return lRet==ERROR_SUCCESS;
}

BOOL SaveSettingsToFile(LPCSTR szKey,LPCSTR szFile,BYTE bFileIsReg)
{
	if (bFileIsReg)
	{
		// Registry script
		char szCommand[2000];
		sprintf_s(szCommand,2000,"regedit /ea \"%s\" HKEY_CURRENT_USER\\%s",szFile,szKey);

		if (FileSystem::IsFile(szFile))
			DeleteFile(szFile);

		PROCESS_INFORMATION pi;
		STARTUPINFO si; // Ansi and Unicode versions are same
		ZeroMemory(&si,sizeof(STARTUPINFO));
		si.cb=sizeof(STARTUPINFO);
		
		
		if (CreateProcess(NULL,szCommand,NULL,
			NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS,
			NULL,NULL,&si,&pi))
		{
			WaitForSingleObject(pi.hProcess,2000);		
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);	
		}
		else
			return FALSE;

		if (FileSystem::IsFile(szFile))
		{
			CRegKey::DeleteKey(HKCU,szKey);
		}

		return TRUE;		
	}

	// Acquiring required privileges	
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
	{
		PTOKEN_PRIVILEGES ns=(PTOKEN_PRIVILEGES)new BYTE[sizeof(DWORD)+sizeof(LUID_AND_ATTRIBUTES)+2];
		if (LookupPrivilegeValue(NULL,SE_BACKUP_NAME,&(ns->Privileges[0].Luid)))
		{
			ns->PrivilegeCount=1;
			ns->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
			if (!AdjustTokenPrivileges(hToken,FALSE,ns,0,NULL,NULL))
			{
				//ShowError(NULL,IDS_ERRORCANNOTENABLEPRIVILEGE,GetLastError());
			}
		}
		if (LookupPrivilegeValue(NULL,SE_RESTORE_NAME,&(ns->Privileges[0].Luid)))
		{
			ns->PrivilegeCount=1;
			ns->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
			if (!AdjustTokenPrivileges(hToken,FALSE,ns,0,NULL,NULL))
			{
				//ShowError(NULL,IDS_ERRORCANNOTENABLEPRIVILEGE,GetLastError());
			}

		}
		delete[] (BYTE*)ns;
	}

	// Data format
	HKEY hRegKey;
	LONG lRet=RegOpenKeyEx(HKEY_CURRENT_USER,szKey,
		0,KEY_READ,&hRegKey);

	if (lRet!=ERROR_SUCCESS)
	{
		//ShowError(hWnd,IDS_ERRORCANNOTOPENKEY,lRet);
		return FALSE;
	}

	DeleteFile(szFile);
	lRet=RegSaveKey(hRegKey,szFile,NULL);
	RegCloseKey(hRegKey);		
	
	if (lRet==ERROR_SUCCESS)
		CRegKey::DeleteKey(HKCU,szKey);
	
	return TRUE;
}

// Save JPEG 
BOOL SaveBitmapToJpegFile(HBITMAP hBitmap,LPCWSTR szFile,int nQuality)
{
	HBITMAP hNewBitmap;
	BYTE* pBuffer;

	// First read size of bitmap to BITMAP struct
	BITMAP bm;
	GetObject(hBitmap, sizeof(BITMAP), &bm);
	
	
	if (bm.bmBitsPixel==24 && 0)
	{
		// Image already in 24 bit format
	}
	else
	{
		// Create a bitmap in correct 24 bit format
		BITMAPINFO  dibInfo;
		dibInfo.bmiHeader.biBitCount = 24;
		dibInfo.bmiHeader.biClrImportant = 0;
		dibInfo.bmiHeader.biClrUsed = 0;
		dibInfo.bmiHeader.biCompression = 0;
		dibInfo.bmiHeader.biHeight = bm.bmHeight;
		dibInfo.bmiHeader.biPlanes = 1;
		dibInfo.bmiHeader.biSize = 40;
		dibInfo.bmiHeader.biWidth = ( ( bm.bmWidth + 3 ) / 4 ) * 4;
		dibInfo.bmiHeader.biSizeImage = dibInfo.bmiHeader.biWidth*bm.bmHeight*3;
		dibInfo.bmiHeader.biXPelsPerMeter = 3780;
		dibInfo.bmiHeader.biYPelsPerMeter = 3780;
		dibInfo.bmiColors[0].rgbBlue = 0;
		dibInfo.bmiColors[0].rgbGreen = 0;
		dibInfo.bmiColors[0].rgbRed = 0;
		dibInfo.bmiColors[0].rgbReserved = 0;
		HDC hDC = ::GetDC(NULL);
		ASSERT(hDC!=NULL);
		hNewBitmap = CreateDIBSection(hDC,(const BITMAPINFO*)&dibInfo,DIB_RGB_COLORS,(void**)&pBuffer,NULL,0);
		::ReleaseDC(NULL, hDC);

		// Copy the original to the new bitmap
		HDC hMemDc=CreateCompatibleDC(NULL);
		HDC hTargetDC=CreateCompatibleDC(NULL);
		
		HBITMAP hOldBitmap1 = (HBITMAP)::SelectObject(hMemDc,hBitmap);
		HBITMAP hOldBitmap2 = (HBITMAP)::SelectObject(hTargetDC,hNewBitmap);

		::BitBlt(hTargetDC,0,0,bm.bmWidth,bm.bmHeight,hMemDc,0,0,SRCCOPY);

		::SelectObject(hMemDc, hOldBitmap1);
		::SelectObject(hTargetDC, hOldBitmap2);
		
		::DeleteDC(hMemDc);
		::DeleteDC(hTargetDC);
	}
	
	// Save bitmap to jpeg file
	jpeg_compress_struct cinfo;
	struct jpeg_error_mgr error_mgr;
	JSAMPROW row_pointer[1];
	int row_stride;

	FILE* outfile;
	if (_wfopen_s(&outfile,szFile,L"wb")!=0) {
		return FALSE;
	}

	// JPEG lib error routines
	cinfo.err = jpeg_std_error(&error_mgr);
	//error_mgr.error_exit = extended_error_exit;
	//error_mgr.output_message = extended_output_message;
	//error_mgr.reset_error_mgr = extended_reset_error_mgr;

	// Create cinfo structure
	jpeg_create_compress(&cinfo);
	
	// Set destination to outfile
	jpeg_stdio_dest(&cinfo, outfile);

	// Set image destination
	cinfo.image_width = bm.bmWidth; 	
	cinfo.image_height = bm.bmHeight;
	cinfo.input_components = 3;	
	cinfo.in_color_space = JCS_RGB; 	

	// Set defaults and quality
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, nQuality, TRUE);

	// Start compressing
	jpeg_start_compress(&cinfo, TRUE);

	row_stride = bm.bmWidth * 3;

	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = & pBuffer[cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	// Finish compressing and close file
	jpeg_finish_compress(&cinfo);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo);

	




	DeleteObject(hNewBitmap);



	return TRUE;

	/* CSTScreenBufferin Create
	BITMAP bm;
	GetObject(hBitmap, sizeof(BITMAP), &bm);
	CreateBitmap(bm.bmWidth, bm.bmHeight);

	CDC memDc;
	CDC targetDc;
	memDc.CreateCompatibleDC(NULL);
	targetDc.CreateCompatibleDC(NULL);

	HBITMAP hOldBitmap1 = (HBITMAP)::SelectObject(memDc.GetSafeHdc(), hBitmap);
	HBITMAP hOldBitmap2 = (HBITMAP)::SelectObject(targetDc.GetSafeHdc(), m_hBitmap);

	targetDc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &memDc, 0, 0, SRCCOPY);

	::SelectObject(memDc.GetSafeHdc(), hOldBitmap1);
	::SelectObject(targetDc.GetSafeHdc(), hOldBitmap2);
	memDc.DeleteDC();
	targetDc.DeleteDC();
	*/

	/* CorrectedWidth
	return ( ( nWidth + 3 ) / 4 ) * 4;
	*/


	/* Clipboard viewerin kuvan tallennuksesta
	CDC dc(GetMainWnd());
	BITMAPINFO bi;
	UINT nColorData;
	bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biBitCount=0;
	GetDIBits(dc,(HBITMAP)m_pData,0,0,NULL,&bi,DIB_RGB_COLORS);
	
	DWORD nSize=sizeof(BITMAPINFOHEADER);
	switch(bi.bmiHeader.biBitCount)
	{
	case 1:
	case 4:
	case 8:
		nColorData=powi2(BYTE(bi.bmiHeader.biBitCount))*4;
		nSize+=nColorData+bi.bmiHeader.biSizeImage;
		break;
	case 16:
		if (bi.bmiHeader.biCompression==BI_BITFIELDS)
			nColorData=4*3*bi.bmiHeader.biWidth*abs(bi.bmiHeader.biHeight);
		else
			nColorData=0;
		nSize+=nColorData+2*bi.bmiHeader.biWidth*abs(bi.bmiHeader.biHeight);
		break;
	case 24:
		nColorData=0;
		nSize+=3*bi.bmiHeader.biWidth*abs(bi.bmiHeader.biHeight);
		break;
	case 32:
		if (bi.bmiHeader.biCompression==BI_BITFIELDS)
			nColorData=4*3*bi.bmiHeader.biWidth*abs(bi.bmiHeader.biHeight);
		else
			nColorData=0;
		nSize+=nColorData+4*bi.bmiHeader.biWidth*abs(bi.bmiHeader.biHeight);
		break;
	}
	nColorData+=sizeof(BITMAPINFOHEADER);
	rFile.Write(&nSize,4,&fe);
	if (fe.m_cause!=CFileException::none)
		return FALSE;
	BITMAPINFO* lpData=(BITMAPINFO*)new BYTE[nSize+2];
	MemCopy(lpData,&bi,sizeof(BITMAPINFOHEADER));
	GetDIBits(dc,(HBITMAP)m_pData,0,abs(bi.bmiHeader.biHeight),(BYTE*)lpData+nColorData,lpData,DIB_RGB_COLORS);
	rFile.Write(lpData,nSize,&fe);
	delete[] (BYTE*)lpData;
	if (fe.m_cause!=CFileException::none)
		return FALSE;
		*/
}

