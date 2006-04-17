////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#ifdef WIN32

LPITEMIDLIST GetFileIDList(LPCSTR lpszFileName)
{
	LPITEMIDLIST ret=NULL;
	HRESULT hres;
	IShellLink *psl;
	
	hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		hres=psl->SetPath(lpszFileName);
		if (SUCCEEDED(hres))
			psl->GetIDList(&ret);
		psl->Release();
	}
	return ret;
}

LPITEMIDLIST GetFolderIDList(LPCSTR lpszFileName)
{
	CHAR szFolder[_MAX_PATH];
	LPITEMIDLIST ret=NULL;
	HRESULT hres;
	IShellLink *psl;
	
	int temp=LastCharIndex(lpszFileName,'\\')+1;
	iMemCopy(szFolder,lpszFileName,temp);
	szFolder[temp]='\0';
	
	hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		hres=psl->SetPath(szFolder);
		if (SUCCEEDED(hres))
			psl->GetIDList(&ret);
		psl->Release();
	}
	return ret;
}

DWORD GetIDListSize(LPITEMIDLIST lpil)
{
	DWORD nSize=0;
	while(*((WORD*)((LPCSTR)lpil+nSize))!=0)
		nSize+=*((WORD*)((LPCSTR)lpil+nSize));
	return nSize+2;
}


HRESULT CreateShortcut(LPCSTR pszShortcutFile,LPCSTR pszLink,LPCSTR pszDesc,LPCSTR pszParams)
{
	HRESULT hres;
	IShellLink* psl;
	hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;
		hres=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
		if (SUCCEEDED(hres))
		{
			hres=psl->SetPath(pszLink);
			if (SUCCEEDED(hres))
			{
				int nIndex=LastCharIndex(pszLink,'\\');
				if (nIndex>=0)
				{
					char szWDir[MAX_PATH];
					iMemCopy(szWDir,pszLink,nIndex);
					szWDir[nIndex]='\0';
					psl->SetWorkingDirectory(szWDir);
				}

				if (pszDesc!=NULL)
					psl->SetDescription(pszDesc);
				if (pszParams!=NULL)
					psl->SetArguments(pszParams);
				
				WCHAR wsz[MAX_PATH];
				MultiByteToWideChar(CP_ACP,0,pszShortcutFile,-1,wsz,MAX_PATH);
				hres=ppf->Save(wsz,TRUE);    
			}
			ppf->Release(); 
		}
		psl->Release();
	}
	return hres;
}

HRESULT ResolveShortcut(HWND hWnd,LPCSTR pszShortcutFile,LPSTR pszPath)
{
	HRESULT hres;  
	IShellLink* psl;
	WIN32_FIND_DATA wfd;
	pszPath[0]='\0';

	hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;
		hres=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
		if (SUCCEEDED(hres))
		{
			WCHAR wsz[MAX_PATH];
			MultiByteToWideChar(CP_ACP,0,pszShortcutFile,-1,wsz,MAX_PATH);
			hres=ppf->Load(wsz,STGM_READ);
			if (SUCCEEDED(hres))
			{
				hres=psl->Resolve(hWnd,SLR_ANY_MATCH);
				if (pszPath!=NULL && SUCCEEDED(hres))
					hres=psl->GetPath(pszPath,MAX_PATH,(WIN32_FIND_DATA*)&wfd,0);
			}
			ppf->Release();
		}
		psl->Release();  
	}
	return hres;
}

HRESULT GetShortcutTarget(LPCSTR pszShortcutFile,LPSTR pszTarget,SIZE_T nBufSize)
{
	HRESULT hres;  
	IShellLink* psl;
	WIN32_FIND_DATA wfd;

	hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;
		hres=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
		if (SUCCEEDED(hres))
		{
			WCHAR wsz[MAX_PATH];
			MultiByteToWideChar(CP_ACP,0,pszShortcutFile,-1,wsz,MAX_PATH);
			hres=ppf->Load(wsz,STGM_READ);
			if (SUCCEEDED(hres))
				hres=psl->GetPath(pszTarget,nBufSize,(WIN32_FIND_DATA*)&wfd,0);
			ppf->Release();
		}
		psl->Release();  
	}
	return hres;
}

#ifdef DEF_WCHAR
LPITEMIDLIST GetFileIDList(LPCWSTR lpszFileName)
{
	LPITEMIDLIST ret=NULL;
	HRESULT hres;
		
	if (IsUnicodeSystem())
	{
		IShellLinkW *psl;
		hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLinkW,(LPVOID*)&psl);
		if (SUCCEEDED(hres))
		{
			hres=psl->SetPath(lpszFileName);
			if (SUCCEEDED(hres))
				psl->GetIDList(&ret);
			psl->Release();
		}
	}
	else
	{
		IShellLink *psl;
		hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&psl);
		if (SUCCEEDED(hres))
		{
			hres=psl->SetPath(W2A(lpszFileName));
			if (SUCCEEDED(hres))
				psl->GetIDList(&ret);
			psl->Release();
		}
	}
	return ret;
}

LPITEMIDLIST GetFolderIDList(LPCWSTR lpszFileName)
{
	WCHAR szFolder[MAX_PATH];
	int temp=LastCharIndex(lpszFileName,'\\')+1;
	MemCopyW(szFolder,lpszFileName,temp);
	szFolder[temp]='\0';
	return GetFileIDList(szFolder);
}

HRESULT CreateShortcut(LPCWSTR pszShortcutFile,LPCWSTR pszLink,LPCWSTR pszDesc,LPCWSTR pszParams)
{
	HRESULT hres;
	
	if (IsUnicodeSystem())
	{
		IShellLinkW* psl;
		hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLinkW,(void**)&psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf;
			hres=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
			if (SUCCEEDED(hres))
			{
				hres=psl->SetPath(pszLink);
				if (SUCCEEDED(hres))
				{
					
					int nIndex=LastCharIndex(pszLink,'\\');
					if (nIndex>=0)
					{
						WCHAR szWDir[MAX_PATH];
						MemCopyW(szWDir,pszLink,nIndex);
						szWDir[nIndex]='\0';
						psl->SetWorkingDirectory(szWDir);
					}
										
					if (pszDesc!=NULL)
						psl->SetDescription(pszDesc);
					if (pszParams!=NULL)
						psl->SetArguments(pszParams);
					
					hres=ppf->Save(pszShortcutFile,TRUE);    
				}
				ppf->Release(); 
			}
			psl->Release();
		}
	}
	else
	{
		IShellLink* psl;
		hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf;
			hres=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
			if (SUCCEEDED(hres))
			{
				hres=psl->SetPath(W2A(pszLink));
				if (SUCCEEDED(hres))
				{
					int nIndex=LastCharIndex(pszLink,'\\');
					if (nIndex>=0)
					{
						char szWDir[MAX_PATH];
						WideCharToMultiByte(CP_ACP,0,pszLink,nIndex,szWDir,MAX_PATH,0,0);
						szWDir[nIndex]='\0';
						psl->SetWorkingDirectory(szWDir);
					}

					if (pszDesc!=NULL)
						psl->SetDescription(W2A(pszDesc));
					if (pszParams!=NULL)
						psl->SetArguments(W2A(pszParams));

					hres=ppf->Save(pszShortcutFile,TRUE);    
				}
				ppf->Release(); 
			}
			psl->Release();
		}
	}
	return hres;
}

HRESULT ResolveShortcut(HWND hWnd,LPCWSTR pszShortcutFile,LPWSTR pszPath)
{
	HRESULT hres;  
	pszPath[0]='\0';

	if (IsUnicodeSystem())
	{
		IShellLinkW* psl;
		WIN32_FIND_DATAW wfd;
		
		hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLinkW,(void**)&psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf;
			hres=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
			if (SUCCEEDED(hres))
			{
				hres=ppf->Load(pszShortcutFile,STGM_READ);
				if (SUCCEEDED(hres))
				{
					hres=psl->Resolve(hWnd,SLR_ANY_MATCH);
					if (pszPath!=NULL && SUCCEEDED(hres))
						hres=psl->GetPath(pszPath,MAX_PATH,(WIN32_FIND_DATAW*)&wfd,0);
				}
				ppf->Release();
			}
			psl->Release();  
		}
	}
	else
	{
		IShellLink* psl;
		WIN32_FIND_DATA wfd;
		
		hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf;
			hres=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
			if (SUCCEEDED(hres))
			{
				hres=ppf->Load(pszShortcutFile,STGM_READ);
				if (SUCCEEDED(hres))
				{
					hres=psl->Resolve(hWnd,SLR_ANY_MATCH);
					if (pszPath!=NULL && SUCCEEDED(hres))
					{
						char szPathA[MAX_PATH];
						hres=psl->GetPath(szPathA,MAX_PATH,(WIN32_FIND_DATA*)&wfd,0);
						if (SUCCEEDED(hres))
							MultiByteToWideChar(CP_ACP,0,szPathA,-1,pszPath,MAX_PATH);
					}
				}
				ppf->Release();
			}
			psl->Release();  
		}
	}
	
	return hres;
}

HRESULT GetShortcutTarget(LPCWSTR pszShortcutFile,LPWSTR pszTarget,SIZE_T nBufSize)
{
	HRESULT hres;
	if (IsUnicodeSystem())
	{
		IShellLinkW* psl;
		WIN32_FIND_DATAW wfd;

		hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLinkW,(void**)&psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf;
			hres=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
			if (SUCCEEDED(hres))
			{
				hres=ppf->Load(pszShortcutFile,STGM_READ);
				if (SUCCEEDED(hres))
					hres=psl->GetPath(pszTarget,nBufSize,(WIN32_FIND_DATAW*)&wfd,0);
				ppf->Release();
			}
			psl->Release();  
		}
	}
	else
	{
		IShellLinkA* psl;
		WIN32_FIND_DATA wfd;

		hres=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLinkA,(void**)&psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf;
			hres=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
			if (SUCCEEDED(hres))
			{
				char* pTargetTmp=new char[nBufSize+2];
				hres=ppf->Load(pszShortcutFile,STGM_READ);
				if (SUCCEEDED(hres))
					hres=psl->GetPath(pTargetTmp,nBufSize,(WIN32_FIND_DATA*)&wfd,0);
				MultiByteToWideChar(CP_ACP,0,pTargetTmp,-1,pszTarget,nBufSize);
				delete[] pTargetTmp;
				ppf->Release();
			}
			psl->Release();  
		}
	}
	return hres;
}
#endif


BOOL RunRegistryCommand(HKEY hKey,LPCTSTR szFile)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	CRegKey CommandKey;
	CString ExecuteStr;
	CString CommandLine;
	DWORD i;

	if (CommandKey.OpenKey(hKey,"command",CRegKey::openExist|CRegKey::samAll)!=ERROR_SUCCESS)
		return FALSE;
	if (CommandKey.QueryValue(szEmpty,ExecuteStr)<2)
		return FALSE;
	i=ExecuteStr.FindFirst('%');
	while (i!=-1)
	{
		if (ExecuteStr[i+1]=='1')
		{
			CommandLine.Copy(ExecuteStr,i);
			CommandLine<<szFile;
			CommandLine<<((LPCSTR)ExecuteStr+i+2);
			break;
		}
		i=ExecuteStr.FindNext('%',i);
	}
	if (i==-1)
		CommandLine=ExecuteStr;
	if (!ExpandEnvironmentStrings(CommandLine,ExecuteStr.GetBuffer(1000),1000))
		ExecuteStr.Swap(CommandLine);
	si.cb=sizeof(STARTUPINFO);
	si.lpReserved=NULL;
	si.cbReserved2=0;
	si.lpReserved2=NULL;
	si.lpDesktop=NULL;
	si.lpTitle=NULL;
	si.dwFlags=STARTF_USESHOWWINDOW;
	si.wShowWindow=SW_SHOWDEFAULT;
	if (!CreateProcess(NULL,ExecuteStr.GetBuffer(),NULL,
		NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS,
		NULL,NULL,&si,&pi))
	{
		CommandKey.CloseKey();
		return FALSE;
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	CommandKey.CloseKey();
	return TRUE;
}

DWORD GetDisplayNameFromIDList(LPITEMIDLIST lpiil,LPSTR szName,SIZE_T dwBufferLen)
{
	// May be computer?
	IShellFolder *psf;
	
	if (!SUCCEEDED(SHGetDesktopFolder(&psf)))
		return 0;

	SHDESCRIPTIONID di;
	if (!SUCCEEDED(SHGetDataFromIDList(psf,lpiil,SHGDFIL_DESCRIPTIONID,&di,sizeof(SHDESCRIPTIONID))))
	{
		psf->Release();
		return 0;
	}

	if (di.clsid!=CLSID_NetworkPlaces)
	{
		psf->Release();
		return 0;
	}

	STRRET str;
	if (!SUCCEEDED(psf->GetDisplayNameOf(lpiil,SHGDN_NORMAL | SHGDN_FORPARSING,&str)))
	{
		psf->Release();
		return 0;
	}
	psf->Release();


	switch (str.uType)
	{
	case STRRET_CSTR:
		StringCbCopy(szName,dwBufferLen,str.cStr);
		return istrlen(szName);
	case STRRET_WSTR:
		return WideCharToMultiByte(CP_ACP,0,str.pOleStr,wcslen(str.pOleStr)+1,szName,dwBufferLen,NULL,NULL);
	}
	return 0;
}

#ifdef DEF_WCHAR
DWORD GetDisplayNameFromIDList(LPITEMIDLIST lpiil,LPWSTR szName,SIZE_T dwBufferLen)
{
	// May be computer?
	IShellFolder *psf;
	
	if (!SUCCEEDED(SHGetDesktopFolder(&psf)))
		return 0;

	SHDESCRIPTIONID di;
	if (!SUCCEEDED(SHGetDataFromIDList(psf,lpiil,SHGDFIL_DESCRIPTIONID,&di,sizeof(SHDESCRIPTIONID))))
	{
		psf->Release();
		return 0;
	}

	if (di.clsid!=CLSID_NetworkPlaces)
	{
		psf->Release();
		return 0;
	}

	STRRET str;
	if (!SUCCEEDED(psf->GetDisplayNameOf(lpiil,SHGDN_NORMAL | SHGDN_FORPARSING,&str)))
	{
		psf->Release();
		return 0;
	}
	psf->Release();


	switch (str.uType)
	{
	case STRRET_CSTR:
		return MultiByteToWideChar(CP_ACP,0,str.cStr,-1,szName,dwBufferLen);
	case STRRET_WSTR:
		StringCbCopyW(szName,dwBufferLen,str.pOleStr);
		return wcslen(szName);
	}
	return 0;
}
#endif


BOOL GetNethoodTarget(LPCWSTR szFolder,LPWSTR szTarget,SIZE_T nBufferLen)
{
	CStringW file(szFolder);
	if (file.LastChar()!=L'\\')
		file << L'\\';
	file << L"desktop.ini";

	WCHAR cls[300];
	if (!GetPrivateProfileStringW(L".ShellClassInfo",L"CLSID2",szwEmpty,cls,300,file))
		return FALSE;

	if (wcscmp(cls,L"{0AFACED1-E828-11D1-9187-B532F1E9575D}")!=0)
		return FALSE; // Folder shortcut

	
	IShellLink* psl;
	if (!SUCCEEDED(CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl)))
		return FALSE;

	IPersistFile* ppf;
	if (!SUCCEEDED(psl->QueryInterface(IID_IPersistFile,(void**)&ppf)))
	{
		psl->Release();
		return FALSE;
	}

	IShellFolder* psf;
	if (!SUCCEEDED(SHGetDesktopFolder(&psf)))
	{
		ppf->Release();
		psl->Release();
		return FALSE;
	}

	file=szFolder;
	if (file.LastChar()!=L'\\')
		file << L'\\';
	file << L"target.lnk";


	BOOL bRet=FALSE;

	if (SUCCEEDED(ppf->Load(file,0)))
	{
		LPITEMIDLIST il;
		if (SUCCEEDED(psl->GetIDList(&il)))
		{
			STRRET str;
			if (SHGetPathFromIDListW(il,szTarget))
				bRet=2;
			else
			{
				SHDESCRIPTIONID di;
				if (SUCCEEDED(SHGetDataFromIDList(psf,il,SHGDFIL_DESCRIPTIONID,&di,sizeof(SHDESCRIPTIONID))))
				{
                    if (di.clsid==CLSID_NetworkPlaces)
					{		
						if (SUCCEEDED(psf->GetDisplayNameOf(il,SHGDN_NORMAL | SHGDN_FORPARSING,&str)))
						{
							switch (str.uType)
							{
							case STRRET_CSTR:
								if (str.cStr[0]=='\\' && str.cStr[1]=='\\')
									bRet=MultiByteToWideChar(CP_ACP,0,str.cStr,-1,szTarget,nBufferLen)>0?1:0;
								break;
							case STRRET_WSTR:
								if (str.pOleStr[0]==L'\\' && str.pOleStr[1]==L'\\')
								{
									UINT nlen=istrlenw(str.pOleStr);
									if (nBufferLen<nlen)
										nlen=nBufferLen-1;
									CopyMemory(szTarget,str.pOleStr,nlen);
									szTarget[nlen]='\0';
									bRet=1;
								}			
								break;
							}
						}
					}
				}
			}
			
		}
	}
	
	psf->Release();
	ppf->Release();
	psl->Release();
	return bRet;
}

DWORD_PTR GetFileInfo(LPCWSTR pszPath,DWORD dwFileAttributes,SHFILEINFOW *psfi,UINT uFlags)
{
	if (IsUnicodeSystem())
		return SHGetFileInfoW(pszPath,dwFileAttributes,psfi,sizeof(SHFILEINFOW),uFlags);

	SHFILEINFO fi;
	DWORD_PTR ret=SHGetFileInfoA(W2A(pszPath),dwFileAttributes,&fi,sizeof(SHFILEINFO),uFlags);
	if (ret==0)
		return 0;

	if (uFlags&SHGFI_DISPLAYNAME)
		MultiByteToWideChar(CP_ACP,0,fi.szDisplayName,-1,psfi->szDisplayName,MAX_PATH);
	if (uFlags&SHGFI_TYPENAME)
		MultiByteToWideChar(CP_ACP,0,fi.szTypeName,-1,psfi->szTypeName,80);
	
	psfi->hIcon=fi.hIcon;
	psfi->iIcon=fi.iIcon;
	psfi->dwAttributes=fi.dwAttributes;
	return ret;	
}

DWORD_PTR GetFileInfo(LPITEMIDLIST piil,DWORD dwFileAttributes,SHFILEINFOW *psfi,UINT uFlags)
{
	if (IsUnicodeSystem())
		return SHGetFileInfoW((LPCWSTR)piil,dwFileAttributes,psfi,sizeof(SHFILEINFOW),uFlags|SHGFI_PIDL);

	SHFILEINFO fi;
	DWORD_PTR ret=SHGetFileInfoA((LPCSTR)piil,dwFileAttributes,&fi,sizeof(SHFILEINFO),uFlags|SHGFI_PIDL);
	if (ret==0)
		return 0;

	if (uFlags&SHGFI_DISPLAYNAME)
		MultiByteToWideChar(CP_ACP,0,fi.szDisplayName,-1,psfi->szDisplayName,MAX_PATH);
	if (uFlags&SHGFI_TYPENAME)
		MultiByteToWideChar(CP_ACP,0,fi.szTypeName,-1,psfi->szTypeName,80);
	
	psfi->hIcon=fi.hIcon;
	psfi->iIcon=fi.iIcon;
	psfi->dwAttributes=fi.dwAttributes;
	return ret;	
}

#endif