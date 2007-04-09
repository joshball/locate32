//
// Common routines for locate
//
// Copyright 2006-2007 Janne Huttunen

#include <HFCLib.h>
#include "common.h"

#include <parsers.h>


typedef HRESULT (__stdcall * PFNSHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPWSTR);  // "SHGetFolderPathW"


LPWSTR GetDefaultFileLocation(LPCWSTR szFileName,BOOL bMustExists,DWORD* lpdwSize)
{
	int nFileNameLen=istrlen(szFileName);
	

	PFNSHGETFOLDERPATH pGetFolderPath=(PFNSHGETFOLDERPATH)GetProcAddress(GetModuleHandle("shell32.dll"),"SHGetFolderPathW");
	if (pGetFolderPath!=NULL)
	{
		WCHAR szAppDataPath[MAX_PATH];
		if (SUCCEEDED(pGetFolderPath(NULL,CSIDL_APPDATA,NULL,
			SHGFP_TYPE_CURRENT,szAppDataPath)))
		{
			int nPathLen=istrlen(szAppDataPath);
			if (szAppDataPath[nPathLen-1]!=L'\\')
				szAppDataPath[nPathLen++]=L'\\';
			
			LPWSTR pStr=new WCHAR[nPathLen+9+nFileNameLen+1];
			MemCopyW(pStr,szAppDataPath,nPathLen);

			MemCopyW(pStr+nPathLen,L"Locate32",9);
			if (!FileSystem::IsDirectory(pStr))
				FileSystem::CreateDirectory(pStr);
			
			nPathLen+=8;
			pStr[nPathLen++]='\\';

			
			MemCopyW(pStr+nPathLen,szFileName,nFileNameLen+1);
			
			if (lpdwSize!=NULL)
				*lpdwSize=nPathLen+nFileNameLen;
			
			if (!bMustExists)
				return pStr;	

			if (FileSystem::IsFile(pStr))
				return pStr;

			// Check also programs directory
			delete[] pStr;
		}
	}


	int iLen;
	LPWSTR pStr;
	if (IsUnicodeSystem())
	{
		WCHAR szExeName[MAX_PATH];
		GetModuleFileNameW(NULL,szExeName,MAX_PATH);
		iLen=LastCharIndex(szExeName,L'\\')+1;
		pStr=new WCHAR[iLen+nFileNameLen+1];
		MemCopyW(pStr,szExeName,iLen);
	}	
	else
	{
		char szExeName[MAX_PATH];
		GetModuleFileName(NULL,szExeName,MAX_PATH);
		iLen=LastCharIndex(szExeName,'\\')+1;
		pStr=new WCHAR[iLen+nFileNameLen+1];
		MemCopyAtoW(pStr,szExeName,iLen);
	}

	
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
	LPWSTR pPath=GetDefaultFileLocation(L"locate.ini",TRUE);
	if (pPath==NULL)
		return NULL;

	bFileIsReg=TRUE;

	char* pFileContent=NULL;
	try
	{
		CFile Ini(pPath,CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
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
		delete[] pPath;
		return NULL;
	}
	delete[] pPath;
		

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
				*pFile=Value.GiveBuffer();
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

