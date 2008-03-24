/* Locate32 - Copyright (c) 1997-2008 Janne Huttunen */

#include <HFCLib.h>
#include "Locate32.h"


////////////////////////////////////////////////////////////
// CLocateDlg - Menu - General
////////////////////////////////////////////////////////////

void CLocateDlg::SetMenus()
{
	HMENU hOldMenu=GetMenu();
	HMENU hMenu=::LoadMenu(IDR_MAINMENU);
    	
	ClearMenuVariables();
	FreeSendToMenuItems(GetSubMenu(GetMenu(),0));
	

	// Load popup menus
	if ((HMENU)m_Menu!=NULL)
		m_Menu.DestroyMenu();
	m_Menu.LoadMenu(IDR_POPUPMENU);
	ASSERT(::GetSubMenu(m_Menu.GetSubMenu(SUBMENU_EXTRACONTEXTMENUITEMS),SUBMENU_SPECIALMENU)!=NULL);


	HMENU hSubMenu=GetSubMenu(hMenu,2);
	if (GetLocateApp()->m_wComCtrlVersion<0x0600) // Disabling features
	{
		::EnableMenuItem(hSubMenu,IDM_ALIGNTOGRID,MF_GRAYED|MF_BYCOMMAND);
		::EnableMenuItem(hSubMenu,IDM_LINEUPICONS,MF_GRAYED|MF_BYCOMMAND);
	}

	if (hSubMenu!=NULL && !(m_dwThumbnailFlags&tfVistaFeaturesAvailable))
	{
		::SetMenuItemBitmaps(hSubMenu,IDM_EXTRALARGEICONS,MF_BYCOMMAND,NULL,m_CircleBitmap);
		::SetMenuItemBitmaps(hSubMenu,IDM_LARGEICONS,MF_BYCOMMAND,NULL,m_CircleBitmap);
		::SetMenuItemBitmaps(hSubMenu,IDM_MEDIUMICONS,MF_BYCOMMAND,NULL,m_CircleBitmap);
		::SetMenuItemBitmaps(hSubMenu,IDM_SMALLICONS,MF_BYCOMMAND,NULL,m_CircleBitmap);
		::SetMenuItemBitmaps(hSubMenu,IDM_LIST,MF_BYCOMMAND,NULL,m_CircleBitmap);
		::SetMenuItemBitmaps(hSubMenu,IDM_DETAILS,MF_BYCOMMAND,NULL,m_CircleBitmap);
	}

	CShortcut::ModifyMenus(m_aShortcuts,hMenu,m_Menu);

	SetMenu(hMenu);
	if (hOldMenu!=NULL)
		DestroyMenu(hOldMenu);

	SetMenuCheckMarkForListType();
}

void CLocateDlg::OnInitMainMenu(HMENU hPopupMenu,UINT nIndex)
{
	switch (nIndex)
	{
	case 1:
		if (m_pListCtrl->GetSelectedCount())
		{
			EnableMenuItem(hPopupMenu,IDM_CUT,MF_BYCOMMAND|MF_ENABLED);
			EnableMenuItem(hPopupMenu,IDM_COPY,MF_BYCOMMAND|MF_ENABLED);
		}
		else
		{
			EnableMenuItem(hPopupMenu,IDM_CUT,MF_BYCOMMAND|MF_GRAYED);
			EnableMenuItem(hPopupMenu,IDM_COPY,MF_BYCOMMAND|MF_GRAYED);
		}
		break;
	case 2:
		if (m_dwFlags&fgLargeMode)
			EnableMenuItem(hPopupMenu,IDM_REFRESH,MF_BYCOMMAND|MF_ENABLED);
		else
			EnableMenuItem(hPopupMenu,IDM_REFRESH,MF_BYCOMMAND|MF_GRAYED);
		break;

	}
}

void CLocateDlg::OnInitFileMenu(HMENU hPopupMenu)
{
	EnableMenuItem(hPopupMenu,IDM_GLOBALUPDATEDB,!GetLocateApp()->IsUpdating()?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hPopupMenu,IDM_UPDATEDATABASES,!GetLocateApp()->IsUpdating()?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED);
	EnableMenuItem(hPopupMenu,IDM_STOPUPDATING,GetLocateApp()->IsUpdating()?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED);

	int iDatabaseMenu=CLocateApp::GetDatabaseMenuIndex(hPopupMenu);
	if (iDatabaseMenu!=-1)
		EnableMenuItem(hPopupMenu,iDatabaseMenu,!GetLocateApp()->IsUpdating()?MF_BYPOSITION|MF_ENABLED:MF_BYPOSITION|MF_GRAYED);

	
	if (m_pListCtrl->GetSelectedCount())
	{
		EnableMenuItem(hPopupMenu,IDM_CREATESHORTCUT,MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(hPopupMenu,IDM_DELETE,MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(hPopupMenu,IDM_RENAME,MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(hPopupMenu,IDM_PROPERTIES,MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(hPopupMenu,IDM_OPENCONTAININGFOLDER,MF_BYCOMMAND|MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hPopupMenu,IDM_CREATESHORTCUT,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hPopupMenu,IDM_RENAME,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hPopupMenu,IDM_DELETE,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hPopupMenu,IDM_PROPERTIES,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hPopupMenu,IDM_OPENCONTAININGFOLDER,MF_BYCOMMAND|MF_GRAYED);
	}
}

void CLocateDlg::OnInitSendToMenu(HMENU hPopupMenu)
{
	// Removing default items
	for(int i=GetMenuItemCount(hPopupMenu)-1;i>=0;i--)
	{
		DebugCloseGdiObject(hPopupMenu);
		DeleteMenu(hPopupMenu,i,MF_BYPOSITION);
	}

	if (m_hSendToListFont!=NULL)
	{
		DebugCloseGdiObject(m_hSendToListFont);
		DeleteObject(m_hSendToListFont);
	}
	
	// Initializing fonts
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,"Control Panel\\Desktop\\WindowMetrics",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		LOGFONTW font;
		RegKey.QueryValue("MenuFont",(LPSTR)&font,sizeof(LOGFONTW));
		m_hSendToListFont=CreateFontIndirectW(&font);
	}
	else
		m_hSendToListFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);

	AddSendToMenuItems(CMenu(hPopupMenu),NULL,IDM_DEFSENDTOITEM);
}



////////////////////////////////////////////////////////////
// CLocateDlg - Menu - Context menu/file menu helpers
////////////////////////////////////////////////////////////


HMENU CLocateDlg::CreateFileContextMenu(HMENU hFileMenu,CLocatedItem** pItems,int nItems,BOOL bSimple)
{
	ClearMenuVariables();
	
	if (hFileMenu!=NULL)
	{
		CMenu FileMenu(hFileMenu);

		// Freeing memyry in SentToMenuItems
		FreeSendToMenuItems(FileMenu);
		
		// Removing all items
		for (int i=FileMenu.GetMenuItemCount()-1;i>=0;i--)
			FileMenu.DeleteMenu(i,MF_BYPOSITION);
		
		// Copying menu from template menu in resource
		if (nItems==0)
		{
			InsertMenuItemsFromTemplate(FileMenu,m_Menu.GetSubMenu(SUBMENU_FILEMENUNOITEMS),0);
			return hFileMenu;
		}
		InsertMenuItemsFromTemplate(FileMenu,m_Menu.GetSubMenu(SUBMENU_FILEMENU),0);
	}
	
	if (!bSimple)
	{
		// Creating context menu for file items
		m_pActiveContextMenu=GetContextMenuForItems(nItems,pItems);

		if (m_pActiveContextMenu!=NULL)
		{
			// IContextMenu interface has created succesfully,
			// so they can insert their own menu items

			HRESULT hRes;
			if (hFileMenu==NULL)
			{
				hFileMenu=CreatePopupMenu();

				if (HIBYTE(GetKeyState(VK_SHIFT)))
				{
					hRes=m_pActiveContextMenu->pContextMenu->QueryContextMenu(hFileMenu,0,
						IDM_DEFCONTEXTITEM,IDM_DEFSENDTOITEM,CMF_EXPLORE|CMF_EXTENDEDVERBS|CMF_CANRENAME);

				}
				else
				{
					hRes=m_pActiveContextMenu->pContextMenu->QueryContextMenu(hFileMenu,0,
						IDM_DEFCONTEXTITEM,IDM_DEFSENDTOITEM,CMF_EXPLORE|CMF_CANRENAME);
				}
			}
			else
			{
				hRes=m_pActiveContextMenu->pContextMenu->QueryContextMenu(hFileMenu,0,
					IDM_DEFCONTEXTITEM,IDM_DEFSENDTOITEM,CMF_EXPLORE|CMF_VERBSONLY|CMF_CANRENAME);
			}
			if (SUCCEEDED(hRes))
			{
				
				

				/* Insert "Rename" 
				UINT uID;
				if (IsUnicodeSystem())
				{
					for (int iPos=0;(uID=FileMenu.GetMenuItemID(iPos))!=UINT(-1);iPos++)
					{
						if (uID>=IDM_DEFCONTEXTITEM && uID<IDM_DEFCONTEXTITEM+1000)
						{
							WCHAR szName[200];
							HRESULT hRes=m_pActiveContextMenu->pContextMenu->GetCommandString(
								uID-IDM_DEFCONTEXTITEM,GCS_VERBW,NULL,(LPSTR)szName,200);
							
							if (hRes!=NOERROR)
							{
								if (wcscmp(szName,L"delete")==0)
								{
									CAppData::stdfunc();
								}
							}
						}							
					}
				}*/

				// Insert special menu, ...
				InsertMenuItemsFromTemplate(CMenu(hFileMenu),m_Menu.GetSubMenu(SUBMENU_EXTRACONTEXTMENUITEMS),0);
				return hFileMenu;
			}

			// Didn't succee, so freeing pointer
			delete m_pActiveContextMenu;
			m_pActiveContextMenu=NULL;
		}
	}

	if (hFileMenu==NULL)
	{
		CMenu FileMenu;
		FileMenu.CreatePopupMenu();
		InsertMenuItemsFromTemplate(FileMenu,m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUPLAIN),0,IDM_DEFOPEN);
		InsertMenuItemsFromTemplate(FileMenu,m_Menu.GetSubMenu(SUBMENU_EXTRACONTEXTMENUITEMS),0);
		return FileMenu;
	}
	else
		InsertMenuItemsFromTemplate(CMenu(hFileMenu),m_Menu.GetSubMenu(SUBMENU_OPENITEMFORFILEMENU),0);
		
			

	return hFileMenu;
	
}

CLocateDlg::ContextMenuStuff* CLocateDlg::GetContextMenuForItems(int nItems,CLocatedItem** ppItems)
{
	if (m_pDesktopFolder==NULL)
		return NULL;

	LPITEMIDLIST* ppFullIDLs=new LPITEMIDLIST[nItems];
	LPITEMIDLIST pParentIDL;

	int iItem=0;
	for (int i=0;i<nItems;i++)
	{
		if (SUCCEEDED(m_pDesktopFolder->ParseDisplayName(*this,NULL,ppItems[i]->GetPath(),NULL,&ppFullIDLs[iItem],NULL)))
			iItem++;
	}
	nItems=iItem;

	if (iItem==0)
		return NULL;

	LPITEMIDLIST* ppSimpleIDLs=new LPITEMIDLIST[nItems];
	ContextMenuStuff* pContextMenuStuff=NULL;
	int nParentIDLLevel=-1;

	if (GetSimpleIDLsandParentfromIDLs(nItems,ppFullIDLs,&pParentIDL,ppSimpleIDLs,&nParentIDLLevel))
	{
		pContextMenuStuff=GetContextMenuForFiles(nItems,pParentIDL,ppSimpleIDLs,nParentIDLLevel);
		if (pContextMenuStuff==NULL)
		{
			for (int i=0;i<nItems;i++)
				CoTaskMemFree(ppSimpleIDLs[i]);
			CoTaskMemFree(pParentIDL);
			delete[] ppSimpleIDLs;
		}
	}
	else
		delete[] ppSimpleIDLs;
				
	for (int i=0;i<nItems;i++)
		CoTaskMemFree(ppFullIDLs[i]);
	delete[] ppFullIDLs;
	
	return pContextMenuStuff;
}
	
	
CLocateDlg::ContextMenuStuff* CLocateDlg::GetContextMenuForFiles(int nItems,LPITEMIDLIST pParentIDL,LPITEMIDLIST* ppSimpleIDLs,int nParentIDLlevel)
{
	if (m_pDesktopFolder==NULL)
		return NULL;

	if (nParentIDLlevel<=1)
		return NULL; // Not enough?
	
	ASSERT(nItems!=0);


	ContextMenuStuff* pcs=new ContextMenuStuff;
	pcs->nIDLCount=nItems;
	pcs->nIDLParentLevel=nParentIDLlevel;
	
	// Querying IShellFolder interface for parent
	HRESULT hRes=m_pDesktopFolder->BindToObject(pParentIDL,NULL,IID_IShellFolder,(void**)&pcs->pParentFolder);
	if (!SUCCEEDED(hRes))
	{
		delete pcs;
		return NULL;
	}


	pcs->pParentFolder->GetUIObjectOf(*this,nItems,(LPCITEMIDLIST*)ppSimpleIDLs,
		IID_IContextMenu,NULL,(void**)&pcs->pContextMenu);
	if (!SUCCEEDED(hRes))
	{
		delete pcs;
		return NULL;
	}

	pcs->pParentIDL=pParentIDL;
	pcs->ppSimpleIDLs=ppSimpleIDLs;

	hRes=pcs->pContextMenu->QueryInterface(IID_IContextMenu2,(void**)&pcs->pContextMenu2);
	if (!SUCCEEDED(hRes))
		pcs->pContextMenu2=NULL;

	hRes=pcs->pContextMenu->QueryInterface(IID_IContextMenu3,(void**)&pcs->pContextMenu3);
	if (!SUCCEEDED(hRes))
		pcs->pContextMenu3=NULL;

	return pcs;
} 



void CLocateDlg::OnContextMenuCommands(WORD wID)
{
	CWaitCursor wait;
	if (m_pListCtrl->GetSelectedCount()==0)
		return;

	ASSERT(wID>=IDM_DEFCONTEXTITEM && m_pActiveContextMenu!=NULL);

	
	/*
	CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(m_pListCtrl->GetNextItem(-1,LVNI_SELECTED));
	
	
	if (!pItem->IsFolder() && !FileSystem::IsFile(pItem->GetPath()))
		return;
	if (pItem->IsFolder() && !FileSystem::IsDirectory(pItem->GetPath()))
		return;
	*/

	WCHAR szName[221];  
	
	if (IsUnicodeSystem())
	{
		if (m_pActiveContextMenu->pContextMenu->GetCommandString(wID-IDM_DEFCONTEXTITEM,
			GCS_VERBW,NULL,(LPSTR)szName,200)!=NOERROR)
			szName[0]=L'\0';
	}
	else
	{
		char szNameA[401];   // Some stupid context menu handlers tries to put help text as UNICODE anyway
		if (m_pActiveContextMenu->pContextMenu->GetCommandString(wID-IDM_DEFCONTEXTITEM,
			GCS_VERBA,NULL,szNameA,200)!=NOERROR)
			szName[0]=L'\0';
		else
			MultiByteToWideChar(CP_ACP,0,szNameA,-1,szName,401);
	}

	// Overriding these command, works better
	if (wcscmp(szName,L"copy")==0)
	{
		OnCopy(FALSE);
		ClearMenuVariables();
		return;
	}
	else if (wcscmp(szName,L"cut")==0)
	{
		OnCopy(TRUE);
		ClearMenuVariables();
		return;
	}
	else if (wcscmp(szName,L"link")==0)
	{
		OnCreateShortcut();
		ClearMenuVariables();
		return;
	}
	else if (wcscmp(szName,L"delete")==0)
	{
		OnDelete();
		ClearMenuVariables();
		return;
	}
	else if (wcscmp(szName,L"rename")==0)
	{
		OnRenameFile();
		ClearMenuVariables();
		return;
	}
	else if (wcscmp(szName,L"properties")==0 && m_pActiveContextMenu->nIDLParentLevel<=1)
	{
		ClearMenuVariables();
		
		int nItems;
		CLocatedItem** pItems=GetSeletedItems(nItems);
		CPropertiesSheet* fileprops=new CPropertiesSheet(nItems,pItems);
		delete[] pItems;
		fileprops->Open();
		return;
	}
	
	CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(m_pListCtrl->GetNextItem(-1,LVNI_SELECTED));
	
	CMINVOKECOMMANDINFOEX cii;
	ZeroMemory(&cii,sizeof(CMINVOKECOMMANDINFOEX));
	cii.cbSize=sizeof(CMINVOKECOMMANDINFOEX);
	cii.fMask=CMIC_MASK_UNICODE;
	cii.hwnd=*this;
	cii.lpVerb=(LPCSTR)MAKELONG(wID-IDM_DEFCONTEXTITEM,0);
	cii.lpVerbW=(LPCWSTR)MAKELONG(wID-IDM_DEFCONTEXTITEM,0);
	cii.lpDirectoryW=pItem->GetParent();
	cii.lpDirectory=alloccopyWtoA(cii.lpDirectoryW);
	cii.nShow=SW_SHOWDEFAULT;
	m_pActiveContextMenu->pContextMenu->InvokeCommand((CMINVOKECOMMANDINFO*)&cii);
	delete[] (LPSTR)cii.lpDirectory;
	
	//ClearMenuVariables();

}


void CLocateDlg::ClearMenuVariables()
{
	if (m_pActiveContextMenu!=NULL)
	{
		//m_pActiveContextMenu->Release();
		delete m_pActiveContextMenu;
		m_pActiveContextMenu=NULL;
	}
	if (m_hActivePopupMenu!=NULL)
	{
		FreeSendToMenuItems(m_hActivePopupMenu);
		DestroyMenu(m_hActivePopupMenu);
		m_hActivePopupMenu=NULL;
	}
}


////////////////////////////////////////////////////////////
// CLocateDlg - Menu - Send To menu helpers
////////////////////////////////////////////////////////////

UINT CLocateDlg::AddSendToMenuItems(CMenu& Menu,LPITEMIDLIST pIDListToPath,UINT wStartID)
{
	MENUITEMINFOW mi;
	mi.cbSize=sizeof(MENUITEMINFOW);
	mi.fMask=MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_TYPE|MIIM_SUBMENU;
	mi.fType=MFT_OWNERDRAW;
	mi.fState=MFS_ENABLED;
	mi.wID=wStartID;
	
			
	BOOL bUseFileFind=TRUE;
	if (m_pDesktopFolder!=NULL)
	{
		HRESULT hRes;
		IShellFolder* psf=NULL;
		BOOL bFreeIDList=FALSE;

		DWORD dwAddLogicalDrives=0;

		if (pIDListToPath==NULL)
		{
			hRes=SHGetSpecialFolderLocation(*this,CSIDL_SENDTO,&pIDListToPath);
			if (SUCCEEDED(hRes)) 
			{
				dwAddLogicalDrives=GetLogicalDrives();
				bFreeIDList=TRUE;
			}
			else
				pIDListToPath=NULL;
		}


		if (pIDListToPath!=NULL)
		{
			hRes=m_pDesktopFolder->BindToObject(pIDListToPath,NULL,IID_IShellFolder,(LPVOID *)&psf);
			if (FAILED(hRes)) 
				psf=NULL;
		}


		
		if (psf!=NULL)
		{
			IEnumIDList* peidl;
			DWORD dwParentListSize=GetIDListSize(pIDListToPath)-sizeof(WORD);

			hRes=psf->EnumObjects(NULL,SHCONTF_FOLDERS|SHCONTF_NONFOLDERS,&peidl);
			if (SUCCEEDED(hRes))
			{
				LPITEMIDLIST pidl,pidlFull;
				
				while (peidl->Next(1,&pidl,NULL)==S_OK)
				{
					DWORD dwListSize=GetIDListSize(pidl);
					// Form full ID list
					pidlFull=(LPITEMIDLIST)CoTaskMemAlloc(dwParentListSize+dwListSize);
					CopyMemory(pidlFull,pIDListToPath,dwParentListSize);
					CopyMemory(((BYTE*)pidlFull)+dwParentListSize,pidl,dwListSize);
										
					mi.hSubMenu=NULL;
					BOOL bDontAdd=FALSE;
					
					SFGAOF aof=SFGAO_DROPTARGET|SFGAO_FOLDER|SFGAO_STREAM;
					hRes=psf->GetAttributesOf(1,(LPCITEMIDLIST*)&pidl,&aof);
					if (SUCCEEDED(hRes))
					{
						if (aof&SFGAO_DROPTARGET)
						{
							if (aof&SFGAO_FOLDER && !(aof&SFGAO_STREAM))
							{
								CMenu Menu;
								Menu.CreateMenu();
								mi.wID+=AddSendToMenuItems(Menu,pidlFull,mi.wID);
								mi.hSubMenu=Menu;
							}
						}
						else 
							bDontAdd=TRUE;
					}
					
					if (!bDontAdd)
					{
						if (dwAddLogicalDrives)
						{
							if (IsUnicodeSystem())
							{
								IShellLinkW* psl;
								hRes=psf->GetUIObjectOf(*this,1,(LPCITEMIDLIST*)&pidl,IID_IShellLinkW,NULL,(void**)&psl);
								if (FAILED(hRes))
								{
									// Alternative way
									STRRET str;
									hRes=psf->GetDisplayNameOf(pidl,SHGDN_FORPARSING,&str);
									if (SUCCEEDED(hRes))
									{
										LPWSTR pStr=StrRetToPtrW(str,pidl);
										if (pStr!=NULL)
										{
											hRes=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLinkW,(void**)&psl);
											if (SUCCEEDED(hRes))
											{
												IPersistFile* ppf;
												hRes=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
												if (SUCCEEDED(hRes))
												{
													hRes=ppf->Load(pStr,STGM_READ);
													ppf->Release();
												}

												if (FAILED(hRes))
													psl->Release();
											}
											delete[] pStr;
										}
										else
											hRes=E_FAIL;
									}

									
								}
								
								if (SUCCEEDED(hRes))
								{
									WCHAR szPath[MAX_PATH];
									hRes=psl->GetPath(szPath,MAX_PATH,0,0);
									if (hRes==NO_ERROR)
									{
										if (szPath[1]==L':' && 
											(szPath[2]==L'\0' || (szPath[2]==L'\\' && szPath[3]==L'\0')))
										{
											CharUpperBuffW(szPath,1);
											dwAddLogicalDrives&=~(1<<BYTE(szPath[0]-L'A'));
										}
									}
									psl->Release();
								}
							}
							else
							{
								IShellLinkA* psl;
								hRes=psf->GetUIObjectOf(*this,1,(LPCITEMIDLIST*)&pidl,IID_IShellLinkA,NULL,(void**)&psl);
								if (FAILED(hRes))
								{
									// Alternative way
									STRRET str;
									hRes=psf->GetDisplayNameOf(pidl,SHGDN_FORPARSING,&str);
									if (SUCCEEDED(hRes))
									{
										LPWSTR pStr=StrRetToPtrW(str,pidl);
										if (pStr!=NULL)
										{
											hRes=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl);
											if (SUCCEEDED(hRes))
											{
												IPersistFile* ppf;
												hRes=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
												if (SUCCEEDED(hRes))
												{
													hRes=ppf->Load(pStr,STGM_READ);
													ppf->Release();
												}

												if (FAILED(hRes))
													psl->Release();

											}
											delete[] pStr;
										}
										else
											hRes=E_FAIL;
									}
								}

								
								if (SUCCEEDED(hRes))
								{
									char szPath[MAX_PATH];
									hRes=psl->GetPath(szPath,MAX_PATH,0,0);
									if (hRes==NO_ERROR)
									{
										if (szPath[1]==':' && 
											(szPath[2]=='\0' || (szPath[2]=='\\' && szPath[3]=='\0')))
										{
											CharUpperBuffA(szPath,1);
											dwAddLogicalDrives&=~(1<<BYTE(szPath[0]-'A'));
										}
									}
									psl->Release();
								}
							}

							
						}
						mi.dwItemData=(DWORD)pidlFull;
						Menu.InsertMenu(mi.wID,FALSE,&mi);
						mi.wID++;
					}
					else
						CoTaskMemFree(pidlFull);

					CoTaskMemFree(pidl);
					
					
				}
				peidl->Release();

				if (dwAddLogicalDrives)
				{
					// Add removable drives
					char drive[]="X:\\";
					for (int i=0;i<='Z'-'A';i++)
					{
						if (dwAddLogicalDrives&(1<<i))
						{
							drive[0]='A'+i;
							DWORD dwDriveType=FileSystem::GetDriveType(drive);
							if (dwDriveType==DRIVE_REMOVABLE)
							{
								mi.dwItemData=(DWORD)GetIDList(drive);
								if (mi.dwItemData!=NULL)
								{
									Menu.InsertMenu(mi.wID,FALSE,&mi);
									mi.wID++;
								}
							}
						}
					}

					// Determine IMAPI burning device
					CArrayFAP<LPWSTR> Burners;
					if (GetIMAPIBurningDevices(Burners))
					{
						for (int i=0;i<Burners.GetSize();i++)
						{
							mi.dwItemData=(DWORD)GetIDList(Burners[i]);
							if (mi.dwItemData!=NULL)
							{
								Menu.InsertMenu(mi.wID,FALSE,&mi);
								mi.wID++;
							}
						}
					}
				}
				bUseFileFind=FALSE;
			}
		}

		if (bFreeIDList)
			CoTaskMemFree(pIDListToPath);
	}
	
	if (bUseFileFind)
	{
		CFileFind Find;
		BOOL bErr;

		WCHAR szSendToPath[MAX_PATH];
		if (pIDListToPath==NULL)
		{
			// Resolving Send To -directory location
			CRegKey RegKey;
			if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
			{
				RegKey.QueryValue(L"SendTo",szSendToPath,MAX_PATH);
				RegKey.CloseKey();
			}
		}
		else
		{
			if (IsUnicodeSystem())
				SHGetPathFromIDListW(pIDListToPath,szSendToPath);
			else
			{
				char szSendToPathA[MAX_PATH];
				SHGetPathFromIDListW(pIDListToPath,szSendToPath);
				MultiByteToWideChar(CP_ACP,0,szSendToPathA,-1,szSendToPath,MAX_PATH);
			}
		}

		wcscat_s(szSendToPath,MAX_PATH,L"\\*.*");
		
		bErr=Find.FindFile(szSendToPath);
		while (bErr)
		{
			WCHAR szPath[MAX_PATH];
			Find.GetFileName(szPath,MAX_PATH);

			
			if (szPath[0]!=L'.' && !Find.IsSystem() && !Find.IsHidden())
			{
				Find.GetFilePath(szPath,MAX_PATH);
				mi.dwItemData=(DWORD)GetIDList(szPath);
				
				if (Find.IsDirectory())
				{
					CMenu Menu;
					Menu.CreateMenu();
					mi.wID+=AddSendToMenuItems(Menu,(LPITEMIDLIST)mi.dwItemData,mi.wID);
					mi.hSubMenu=Menu;
				}
				else
					mi.hSubMenu=NULL;
				Menu.InsertMenu(mi.wID,FALSE,&mi);
				mi.wID++;
			}
			bErr=Find.FindNextFile();
		}
		Find.Close();
	}
	
	if (mi.wID==wStartID)
	{
		// Inserting default menu items
		ID2W EmptyTitle(IDS_EMPTY);
		mi.dwTypeData=(LPWSTR)(LPCWSTR)EmptyTitle;
		mi.dwItemData=0;
		mi.fState=MFS_GRAYED;
		mi.fType=MFT_STRING;
		Menu.InsertMenu(mi.wID,FALSE,&mi);
		mi.wID++;
	}
	return mi.wID-wStartID;
}

void CLocateDlg::FreeSendToMenuItems(HMENU hMenu)
{
	UINT wID=IDM_DEFSENDTOITEM;
	MENUITEMINFO mii;
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_DATA;
	while (GetMenuItemInfo(hMenu,wID,FALSE,&mii))
	{
		if (mii.dwItemData!=NULL)
			CoTaskMemFree((LPVOID)mii.dwItemData);
		wID++;
	}
}

int CLocateDlg::GetSendToMenuPos(HMENU hMenu)
{
	int nPos=0;
	UINT nID=GetMenuItemID(hMenu,nPos);
	while (nID!=IDM_CUT && nID!=IDM_CREATESHORTCUT)
	{
		if (nID==DWORD(-1))
		{		
			if (IsSendToMenu(GetSubMenu(hMenu,nPos)))
				return nPos;
		}
		nID=GetMenuItemID(hMenu,++nPos);
	}
	return nPos;
}


void CLocateDlg::OnSendToCommand(WORD wID)
{
	if (m_pDesktopFolder==NULL)
		return;

	CWaitCursor wait;
	MENUITEMINFO mii;
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_DATA;
	if (m_hActivePopupMenu!=NULL)
		GetMenuItemInfo(m_hActivePopupMenu,wID,FALSE,&mii);
	else
		GetMenuItemInfo(GetSubMenu(GetMenu(),0),wID,FALSE,&mii);
	if (mii.dwItemData==NULL)
		return;

	IDropTarget* pdt=GetDropTarget((LPITEMIDLIST)mii.dwItemData);
	if (pdt==NULL)
		return;
	
	CFileObject *pfoSrc=new CFileObject;
	pfoSrc->AutoDelete();
	pfoSrc->AddRef();
	pfoSrc->SetFiles(m_pListCtrl,TRUE);
		
	
	DWORD dwEffect=DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;
	POINTL pt={0,0};
	
	HRESULT hRes=pdt->DragEnter(pfoSrc,MK_LBUTTON,pt,&dwEffect);
	if (SUCCEEDED(hRes) && dwEffect)
	{
		// Drop file
		hRes=pdt->Drop(pfoSrc,MK_LBUTTON,pt,&dwEffect);
	}
	else
	{
		// Drop target didn't like file object
		hRes = pdt->DragLeave();
	}

	// Releasing FileObjects
	pfoSrc->Release();
	
	pdt->Release();
}



////////////////////////////////////////////////////////////
// CLocateDlg - Menu - Misc funtions
////////////////////////////////////////////////////////////


BOOL CLocateDlg::InsertMenuItemsFromTemplate(CMenu& Menu,HMENU hTemplate,UINT uStartPosition,int nDefaultItem)
{
	CMenu Template(hTemplate);

	MENUITEMINFOW mii;
	WCHAR szName[1000];
	mii.cbSize=sizeof(MENUITEMINFOW);
	int nMenuLength=Template.GetMenuItemCount();
	for (int i=0;i<nMenuLength;i++)
	{
		mii.fMask=MIIM_ID|MIIM_TYPE|MIIM_STATE|MIIM_SUBMENU;
		mii.dwTypeData=szName;
		mii.cch=1000;
		
		// Checking whether popupmenu is popup menu or item
		if (!Template.GetMenuItemInfo(i,TRUE,&mii))
			return FALSE;

		if (mii.wID==nDefaultItem)
			mii.fState|=MFS_DEFAULT;

		if (mii.hSubMenu!=NULL)
		{
			// It is popup menu
            CMenu NewMenu;
			NewMenu.CreatePopupMenu();
			if (!InsertMenuItemsFromTemplate(NewMenu,CMenu(mii.hSubMenu),0))
			{
				NewMenu.DestroyMenu();
				return FALSE;
			}
			mii.hSubMenu=NewMenu;
			DebugCloseHandle(dhtMenu,mii.hSubMenu,STRNULL);
		}
		
		if (!Menu.InsertMenu(i+uStartPosition,TRUE,&mii))
		{
			if (mii.hSubMenu!=NULL)
				DestroyMenu(mii.hSubMenu);
			return FALSE;
		}
	}
	return TRUE;
}

