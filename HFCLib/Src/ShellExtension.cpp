////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
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
	TCHAR szFolder[_MAX_PATH];
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
			WCHAR wsz[MAX_PATH];
			hres=psl->SetPath(pszLink);
			if (SUCCEEDED(hres))
			{
				char szWDir[MAX_PATH];
				int nIndex;
				if (pszDesc!=NULL)
					psl->SetDescription(pszDesc);
				if (pszParams!=NULL)
					psl->SetArguments(pszParams);
				nIndex=LastCharIndex(pszLink,'\\');
				if (nIndex>=0)
				{
					iMemCopy(szWDir,pszLink,nIndex);
					szWDir[nIndex]='\0';
					psl->SetWorkingDirectory(szWDir);
				}
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

HRESULT GetShortcutTarget(LPCSTR pszShortcutFile,LPSTR pszTarget)
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
				hres=psl->GetPath(pszTarget,MAX_PATH,(WIN32_FIND_DATA*)&wfd,0);
			ppf->Release();
		}
		psl->Release();  
	}
	return hres;
}


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

DWORD GetDisplayNameFromIDList(LPITEMIDLIST lpiil,char* szName,SIZE_T dwBufferLen)
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
		{
			DWORD dwLength;
			dstrlen(str.cStr,dwLength);
			if (dwLength>dwBufferLen-1)
				dwLength=dwBufferLen-1;
			CopyMemory(szName,str.cStr,dwLength);
			szName[dwLength];
			return dwLength+1;
		}
	case STRRET_WSTR:
		return WideCharToMultiByte(CP_ACP,0,str.pOleStr,wcslen(str.pOleStr)+1,szName,299,NULL,NULL);
	}
	return 0;
}



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

#endif