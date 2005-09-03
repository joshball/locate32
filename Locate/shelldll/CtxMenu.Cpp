//
// Locate Shell Extension
// Copyright 2005 Janne Huttunen
//
// class id:  73ACE856-AD8F-4346-9A07-775E45221BC8
//
//           CShellExt::AddRef and CShellExt::Release code.


#ifndef STRICT
#define STRICT
#endif

#define INC_OLE2        // WIN32, get ole2 from windows.h

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include "../lan_resources.h"
#include "LocateShellExtension.h"
#include "resource.h"

#define _CONSOLE
#include <hfclib.h>

extern HINSTANCE g_hmodThisDll;

INT IsDirectory(LPCTSTR szDirectoryName)
{
#ifdef WIN32
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	int ret=TRUE;
	if (szDirectoryName[0]=='\0')
		return 0;
	if (szDirectoryName[1]=='\0')
		return 0;
	if (szDirectoryName[2]=='\0')
		return 0;
	if (szDirectoryName[1]==':' && szDirectoryName[2]=='\\' && szDirectoryName[3]=='\0')
	{
		switch (GetDriveType(szDirectoryName))
		{
		case DRIVE_UNKNOWN:
		case DRIVE_NO_ROOT_DIR:
			return 0;
		case DRIVE_FIXED:
			return 1;
		default:
			return 2;
		}
	}
	
	// Taking last '\\' 
	LPSTR szPath;
	DWORD dwPathLen;
	dstrlen(szDirectoryName,dwPathLen);
	if (szDirectoryName[dwPathLen-1]=='\\' && dwPathLen>3)
	{
		szPath=new char[dwPathLen+5];
		CopyMemory(szPath,szDirectoryName,--dwPathLen);
		szPath[dwPathLen]='\0';
	}
	else
		szPath=LPSTR(szDirectoryName);

	hFind=FindFirstFile(szPath,&fd);
	if (hFind!=INVALID_HANDLE_VALUE)
	{
		while (!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && ret)
			ret=FindNextFile(hFind,&fd);
	
		if (szPath!=szDirectoryName)
			delete[] szPath;

		FindClose(hFind);	
		if (ret)
		{
			if (szDirectoryName[0]=='\\')
			{
				if (szDirectoryName[1]=='\\')
					return 2;
				switch (GetDriveType(NULL))
				{
				case DRIVE_UNKNOWN:
				case DRIVE_NO_ROOT_DIR:
					return 0;
				case DRIVE_FIXED:
					return 1;
				default:
					return 2;
				}
 
			}
			if (szDirectoryName[1]==':' && szDirectoryName[2]=='\\')
			{
				char szTemp[4]="X:\\";
				szTemp[0]=szDirectoryName[0];

				switch (GetDriveType(szTemp))
				{
				case DRIVE_UNKNOWN:
					return 0;
				case DRIVE_FIXED:
					return 1;
				default:
					return 2;
				}
			}
		}
		return 0;
	}
	else if (szDirectoryName[0]=='\\' && szDirectoryName[1]=='\\')
	{
		// UNC share name
		if (szPath==szDirectoryName)
		{
			szPath=new char[dwPathLen+5];
			CopyMemory(szPath,szDirectoryName,dwPathLen);
		}		
		CopyMemory(szPath+dwPathLen,"\\*.*",5);

		hFind=FindFirstFile(szPath,&fd);
		delete[] szPath;
		if (hFind==INVALID_HANDLE_VALUE)
			return 0;
		// Is this really needed, e.g. \\pc\c$ does not have '.' in directory list
		//ret=1;
		//while ((fd.cFileName[0]!='.' || fd.cFileName[1]!='\0') && ret==1)
		//	ret=FindNextFile(hFind,&fd);
		FindClose(hFind);
		//return ret?2:0;
		return 2;
	}
	
	
	if (szPath!=szDirectoryName)
		delete[] szPath;
	return 0;
#else
	struct ffblk fd;
	return !findfirst(szDirectoryName,&fd,FA_DIREC);
#endif
}

HMODULE GetLanguageResource()
{
	HKEY hKey;
	LPSTR lanFile=NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\Update\\Locate32",0,KEY_READ,&hKey)==ERROR_SUCCESS)
	{
		DWORD lLength;
		if (RegQueryValueEx(hKey,"Language",0,NULL,NULL,&lLength)==ERROR_SUCCESS)
		{
			lanFile=new char[lLength+1];
			if (RegQueryValueEx(hKey,"Language",0,NULL,(LPBYTE)lanFile,&lLength)!=ERROR_SUCCESS)
			{
				delete[] lanFile;
				lanFile=NULL;
			}
		}
		RegCloseKey(hKey);
	}


	char szPath[MAX_PATH];
	int nLen=GetModuleFileName(g_hmodThisDll,szPath,MAX_PATH);
	for (nLen--;nLen>=0 && szPath[nLen]!='\\';nLen--);
	nLen++;
    
	if (lanFile==NULL)
		strcpy(szPath+nLen,"lan_en.dll");
	else
	{
        strcpy(szPath+nLen,lanFile);
		delete[] lanFile;
	}

	return LoadLibrary(szPath);
}

//
//  FUNCTION: CShellExt::QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
//
//  PURPOSE: Called by the shell just before the context menu is displayed.
//           This is where you add your specific menu items.
//
//  PARAMETERS:
//    hMenu      - Handle to the context menu
//    indexMenu  - Index of where to begin inserting menu items
//    idCmdFirst - Lowest value for new menu ID's
//    idCmtLast  - Highest value for new menu ID's
//    uFlags     - Specifies the context of the menu event
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CLocateShellExtension::QueryContextMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags)
{
#ifdef _LOGGING
	char szTemp[100];
    wsprintf(szTemp, "CShellExt::QueryContextMenu(), uFlags==>%d", uFlags);
    ODS(szTemp);
#endif    
	
	if (uFlags & CMF_DEFAULTONLY)
	{
		ODS("CMF_DEFAULTONLY...");
			return NOERROR;
	}
    
	HMODULE hLanDll=GetLanguageResource();
	if (hLanDll==NULL)
		return NOERROR;

	DWORD dwSettings=ReadSettingsFromRegistry();

	if (dwSettings&fSubMenu)
	{
		HMENU hSubMenu=CreatePopupMenu();
		UINT uHighestId=0;
		if (InsertMenuItems(hSubMenu,0,idCmdFirst,uHighestId))
		{
			MENUITEMINFO mii;
			char szTitle[200];
			LoadString(hLanDll,IDS_EXTLOCATEMENU,szTitle,200);

			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_SUBMENU|MIIM_TYPE;
			mii.fType=MFT_STRING;
			mii.dwTypeData=szTitle;
			mii.hSubMenu=hSubMenu;
			InsertMenuItem(hMenu,indexMenu,TRUE,&mii);

			FreeLibrary(hLanDll);
			return ResultFromShort(uHighestId+1-idCmdFirst);
		}
		
		DestroyMenu(hSubMenu);
		FreeLibrary(hLanDll);
		return NOERROR;	
	}
	else
	{
		UINT uHighestId=0;
		if (InsertMenuItems(hMenu,indexMenu,idCmdFirst,uHighestId))
		{
			FreeLibrary(hLanDll);
			return ResultFromShort(uHighestId+1-idCmdFirst);
		}
		FreeLibrary(hLanDll);
		return NOERROR;
	}
}

//
//  FUNCTION: CShellExt::ReadSettingsFromRegistry()
//
//  PURPOSE: Reads settings from registry
//
//  RETURN VALUE:
//	  flags
//
//

DWORD CLocateShellExtension::ReadSettingsFromRegistry()
{

	char szRegpath[200];
	LoadString(g_hmodThisDll,IDS_REGPLACE,szRegpath,200);

	HKEY hKey;
	
	if (RegOpenKeyEx(HKEY_CURRENT_USER,szRegpath,0,KEY_READ,&hKey)!=ERROR_SUCCESS)
		return fDidnSuccess|fDefaults;

    DWORD dwData=fDefaults,dwLength=sizeof(DWORD);

	LONG lRet=RegQueryValueEx(hKey,"Flags",NULL,NULL,(LPBYTE)&dwData,&dwLength);
	
	RegCloseKey(hKey);

	if (lRet!=ERROR_SUCCESS || dwLength!=sizeof(DWORD))
		return fDidnSuccess|fDefaults;
	return dwData;
}
    
//
//  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)
//
//  PURPOSE: Called by the shell after the user has selected on of the
//           menu items that was added in QueryContextMenu().
//
//  PARAMETERS:
//    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CLocateShellExtension::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    ODS("CShellExt::InvokeCommand()	");

    HRESULT hr = E_INVALIDARG;

    //If HIWORD(lpcmi->lpVerb) then we have been called programmatically
    //and lpVerb is a command that should be invoked.  Otherwise, the shell
    //has called us, and LOWORD(lpcmi->lpVerb) is the menu ID the user has
    //selected.  Actually, it's (menu ID - idCmdFirst) from QueryContextMenu().
    if (!HIWORD(lpcmi->lpVerb))
    {
        UINT idCmd = LOWORD(lpcmi->lpVerb);

        switch (idCmd)
        {
            case 0:
                hr = DoGAKMenu1(lpcmi->hwnd,
                                lpcmi->lpDirectory,
                                lpcmi->lpVerb,
                                lpcmi->lpParameters,
                                lpcmi->nShow);
                break;

           
        }
    }
    return hr;
}

//
//  FUNCTION: CShellExt::InsertMenuItems(HMENU hMenu,int nStartID)
//
//  PURPOSE: Creates menu items
//
//  RETURN VALUE: how many items are inserted

int CLocateShellExtension::InsertMenuItems(HMENU hMenu,UINT nStartIndex,UINT idCmdFirst,UINT& nHighestUsedID)
{
	ODS("CShellExt::InsertMenuItems()");
	
	if (m_pDataObj==NULL)
		return 0;

	CArrayFAP<LPSTR> aNetworkResources,aDirectories,aFiles;
    
	FORMATETC FormatEtc;
	STGMEDIUM med;
	FormatEtc.cfFormat=RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	FormatEtc.ptd=NULL;
	FormatEtc.dwAspect=DVASPECT_CONTENT;
	FormatEtc.lindex=-1;
    FormatEtc.tymed=TYMED_HGLOBAL;
	
	if (m_pDataObj->GetData(&FormatEtc,&med)==S_OK)
	{
		// May be computer?
		IShellFolder *psf;
	
		
		if (!SUCCEEDED(SHGetDesktopFolder(&psf)))
		{
			DebugMessage("No IShellFolder interface");
			return 0;
		}
		
		// Retrieving data from item id list
		BYTE* pData=(BYTE*)GlobalLock(med.hGlobal);
		
		UINT nFiles=((int*)pData)[0];
		UINT i,nLines=1,nSubIDListLen,nIDListLen;
		char szPath[_MAX_PATH];

		nSubIDListLen=GetIDListSize((LPITEMIDLIST)(pData+((int*)pData)[1]))-2;
		
		for (i=0;i<nFiles;i++)
		{
			nIDListLen=GetIDListSize((LPITEMIDLIST)(pData+((int*)pData)[2+i]));

			// Constructing IDList from parent item and item
            LPITEMIDLIST pItemIDList=(LPITEMIDLIST)new BYTE[nSubIDListLen+nIDListLen+2];
			CopyMemory((BYTE*)pItemIDList,(LPCSTR)(pData+((int*)pData)[1]),nSubIDListLen);
			CopyMemory((BYTE*)(pItemIDList)+nSubIDListLen,(LPCSTR)(pData+((int*)pData)[2+i]),nIDListLen);
			
			if (SHGetPathFromIDList(pItemIDList,szPath))
			{
				if (IsDirectory(szPath))
					aDirectories.Add(alloccopy(szPath));
				else
					aFiles.Add(alloccopy(szPath));
			}
			else
			{
				DebugHexDump("idlist",(BYTE*)pItemIDList,nSubIDListLen+nIDListLen);
				
				
				if (GetDisplayNameFromIDList(pItemIDList,szPath,MAX_PATH))
				{
					if (szPath[0]!='\\' && szPath[1]!='\\')
					{
						DebugFormatMessage("Cannot add item %s",szPath);
					}
					else
						aNetworkResources.Add(alloccopy(szPath));
				}
			}
			
			delete[] (BYTE*)pItemIDList;
		}
		
		GlobalUnlock(med.hGlobal);
		if (med.pUnkForRelease==NULL)
            GlobalFree(med.hGlobal);
		else 
			med.pUnkForRelease->Release();
	
		psf->Release();
	}
	else 
	{
		FormatEtc.cfFormat=CF_HDROP;
		if (m_pDataObj->GetData(&FormatEtc,&med)==S_OK)
		{
			LPDROPFILES df=(LPDROPFILES)GlobalLock(med.hGlobal);
			if (df->fWide)
			{
				LPWSTR lpPtr=(LPWSTR)((BYTE*)df+df->pFiles);
				DWORD nLen;
				while (nLen=wcslen(lpPtr))
				{
					LPSTR lpFile=alloccopyWtoA(lpPtr,nLen);
					
					if (IsDirectory(lpFile))
						aDirectories.Add(lpFile);
					else
						aFiles.Add(lpFile);
					
					lpPtr+=nLen+1;
				}
			}
			else
			{
				LPSTR lpPtr=(LPSTR)df+df->pFiles;
				DWORD nLen;
				while (nLen=strlen(lpPtr))
				{
					LPSTR lpFile=alloccopy(lpPtr,nLen);					

					if (IsDirectory(lpFile))
						aDirectories.Add(lpFile);
					else
						aFiles.Add(lpFile);

					lpPtr+=nLen+1;
				}
			}	
			
			GlobalUnlock(med.hGlobal);
			
			if (med.pUnkForRelease==NULL)
                GlobalFree(med.hGlobal);
			else 
				med.pUnkForRelease->Release();
		}
	}


	

	
	MENUITEMINFO mii;
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_ID|MIIM_TYPE;
	mii.fType=MFT_STRING;
	mii.dwTypeData="Files:";
	mii.wID=idCmdFirst;
    InsertMenuItem(hMenu,nStartIndex++,TRUE,&mii);

	for (int i=0;i<aFiles.GetSize();i++)
	{
		mii.wID++;
		mii.dwTypeData=aFiles[i];
		InsertMenuItem(hMenu,nStartIndex++,TRUE,&mii);
	}

	mii.wID++;
	mii.dwTypeData="Directories:";
	InsertMenuItem(hMenu,nStartIndex++,TRUE,&mii);
	
	for (int i=0;i<aDirectories.GetSize();i++)
	{
		mii.wID++;
		mii.dwTypeData=aDirectories[i];
		InsertMenuItem(hMenu,nStartIndex++,TRUE,&mii);
	}


	mii.wID++;
	mii.dwTypeData="Other:";
	InsertMenuItem(hMenu,nStartIndex++,TRUE,&mii);
	
	for (int i=0;i<aNetworkResources.GetSize();i++)
	{
		mii.wID++;
		mii.dwTypeData=aNetworkResources[i];
		InsertMenuItem(hMenu,nStartIndex++,TRUE,&mii);
	}

	mii.wID++;
	mii.dwTypeData="Net resources:";
	InsertMenuItem(hMenu,nStartIndex++,TRUE,&mii);
	
	FormatEtc.cfFormat=RegisterClipboardFormat(CFSTR_NETRESOURCES);
	if (m_pDataObj->GetData(&FormatEtc,&med)==S_OK)
	{
		NRESARRAY* array=(NRESARRAY*)GlobalLock(med.hGlobal);
		DebugHexDump("got netresource",(BYTE*)array,GlobalSize(med.hGlobal));

		for (DWORD i=0;i<array->cItems;i++)
		{
			mii.dwTypeData=LPSTR(array)+UINT(array->nr[i].lpRemoteName);
			mii.wID++;
			InsertMenuItemW(hMenu,nStartIndex++,TRUE,(LPMENUITEMINFOW)&mii);
		}

		GlobalUnlock(med.hGlobal);
			
		if (med.pUnkForRelease==NULL)
            GlobalFree(med.hGlobal);
		else 
			med.pUnkForRelease->Release();
	}

	nHighestUsedID=mii.wID;
	
	return mii.wID-idCmdFirst+1;
}

//
//  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)
//
//  PURPOSE: Called by the shell after the user has selected on of the
//           menu items that was added in QueryContextMenu().
//
//  PARAMETERS:
//    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CLocateShellExtension::GetCommandString(UINT_PTR idCmd,
                                         UINT uFlags,
                                         UINT FAR *reserved,
                                         LPSTR pszName,
                                         UINT cchMax)
{
    ODS("CShellExt::GetCommandString()");

	HMODULE hLanDll=GetLanguageResource();
	if (hLanDll==NULL)
		return NOERROR;

	if (uFlags&GCS_UNICODE)
	{
        switch (idCmd)
		{
	        case 0:
				LoadStringW(hLanDll,IDS_EXTLOCATEMENU,(LPWSTR)pszName,cchMax);
				break;
	
		}
	}
	else
	{
        switch (idCmd)
		{
	        case 0:
				LoadString(hLanDll,IDS_EXTLOCATEMENU,pszName,cchMax);
				break;
	
		}
	}

	FreeLibrary(hLanDll);
    return NOERROR;
}

STDMETHODIMP CLocateShellExtension::DoGAKMenu1(HWND hParent,
                                   LPCSTR pszWorkingDir,
                                   LPCSTR pszCmd,
                                   LPCSTR pszParam,
                                   int iShowCmd)
{
    ODS("CShellExt::DoGAKMenu1()");

    MessageBox(hParent, "Menu item 1!", "Shell Extension Sample", MB_OK);

    return NOERROR;
}

