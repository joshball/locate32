#if !defined(LOCATEDLG_INL)
#define LOCATEDLG_INL

#if _MSC_VER >= 1000
#pragma once
#endif

inline CLocateDlg::CLocateDlg()
:	CDialog(IDD_MAIN),m_dwFlags(fgDefault),m_dwExtraFlags(efDefault),m_nSorting(BYTE(-1)),
	m_nMaxYMinimized(0),m_nMaxYMaximized(0),m_nLargeY(354),
	m_ClickWait(FALSE),m_hSendToListFont(NULL),m_hActivePopupMenu(NULL),
	m_pListCtrl(NULL),m_pTabCtrl(NULL),m_pStatusCtrl(NULL),m_pListTooltips(NULL),
	m_pLocater(NULL),m_pBackgroundUpdater(NULL),m_pActiveContextMenu(NULL),
	m_pLocateAnimBitmaps(NULL),m_pUpdateAnimBitmaps(NULL),
	m_pFileNotificationsThread(NULL),m_dwMaxFoundFiles(0),
	m_pImageHandler(NULL),m_iTooltipItem(-1),m_iTooltipSubItem(-1),m_bTooltipActive(FALSE),
	m_hLastFocus(NULL),m_WaitEvery30(10),m_WaitEvery60(20)
{
	DebugNumMessage("CLocateDlg::CLocateDlg() this is %X",DWORD(this));

	ZeroMemory(m_aResultListActions,TypeCount*ListActionCount*sizeof(void*));
}

inline CLocateDlg::CNameDlg::CNameDlg()
:	CDialog(IDD_NAME),m_nMaxBrowse(DEFAULT_NUMBEROFDIRECTORIES),
	m_pBrowse(NULL),m_pMultiDirs(NULL)
{
}

inline WORD CLocateDlg::CNameDlg::ComputeChecksumFromDir(LPCSTR szDir)
{				
	WORD wCheksum=0;
	for (int j=0;szDir[j]!='\0';j++)
		wCheksum+=WORD(szDir[j]*(j+1));
	return wCheksum;
}

inline CLocateDlg::CNameDlg::DirSelection::DirSelection(BYTE bSelected_)
:	nType(CLocateDlg::CNameDlg::NotSelected),pTitleOrDirectory(NULL),bSelected(bSelected_)
{
}

inline CLocateDlg::CNameDlg::DirSelection::~DirSelection()
{
	FreeData();
}

inline void CLocateDlg::CNameDlg::DirSelection::FreeData()
{
	if (pTitleOrDirectory!=NULL)
	{
		delete[] pTitleOrDirectory;
		pTitleOrDirectory=NULL;
	}
}

inline void CLocateDlg::CNameDlg::AddDirectoryToList(CArray<LPSTR>& aDirectories,LPCSTR szDirectory)
{
	for (int i=0;i<aDirectories.GetSize();i++)
	{
		if (strcmp(aDirectories[i],szDirectory)==0)
			return;
	}
	aDirectories.Add(alloccopy(szDirectory));
}

inline void CLocateDlg::CNameDlg::AddDirectoryToList(CArray<LPSTR>& aDirectories,LPCSTR szDirectory,DWORD dwLength)
{
	for (int i=0;i<aDirectories.GetSize();i++)
	{
		if (strncmp(aDirectories[i],szDirectory,dwLength)==0)
		{
			if (aDirectories[i][dwLength]=='\0')
				return;
		}
	}
	aDirectories.Add(alloccopy(szDirectory,dwLength));
}

inline void CLocateDlg::CNameDlg::AddDirectoryToList(CArray<LPSTR>& aDirectories,FREEDATA szDirectory)
{
	for (int i=0;i<aDirectories.GetSize();i++)
	{
		if (strcmp(aDirectories[i],LPCSTR(szDirectory))==0)
		{
			delete[] (LPSTR)szDirectory;
			return;
		}
	}
	aDirectories.Add(LPSTR(szDirectory));
}



inline CLocateDlg::CSizeDateDlg::CSizeDateDlg()
:	CDialog(IDD_SIZEDATE)
{
}

inline BOOL CLocateDlg::CSizeDateDlg::IsChanged()
{
	return (IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE)||
			IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE) ||
			IsDlgButtonChecked(IDC_CHECKMINDATE) ||
			IsDlgButtonChecked(IDC_CHECKMAXDATE));
}

inline CLocateDlg::CAdvancedDlg::CAdvancedDlg()
:	CDialog(IDD_ADVANCED),m_hTypeUpdaterThread(NULL),m_hDefaultTypeIcon(NULL),m_dwFlags(0)
{
}

inline void CLocateDlg::ClearMenuVariables()
{
	KillTimer(ID_CLEARMENUVARS);
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

inline void CLocateDlg::OnActivateTab(int nIndex)
{
	if (nIndex<0 && nIndex>=LOCATEDIALOG_TABS)
		return;
	m_pTabCtrl->SetCurSel(nIndex);
	SetVisibleWindowInTab();
}

inline void CLocateDlg::OnActivateNextTab(BOOL bPrev)
{
	int nIndex=m_pTabCtrl->GetCurSel()+(bPrev?-1:1);


	if (nIndex<0)
		nIndex=LOCATEDIALOG_TABS-1;
	else if (nIndex>=LOCATEDIALOG_TABS)
		nIndex=0;
	
	m_pTabCtrl->SetCurSel(nIndex);
	SetVisibleWindowInTab();
}

inline BOOL CLocateDlg::IsSendToMenu(HMENU hMenu)
{
	/*UINT nID=GetMenuItemID(hMenu,0);
	return nID==IDM_DEFSENDTOITEM && nID<IDM_DEFSENDTOITEM+1000;*/

	return GetMenuItemID(hMenu,0)==IDM_DEFSENDTOITEM;
}
	
inline BOOL CLocateDlg::CSizeDateDlg::LookOnlyFiles() const
{
	return IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE) ||
		IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE);
}

inline CLocateDlgThread::CLocateDlgThread()
:	m_pLocate(NULL)
{
	m_bAutoDelete=TRUE;
}

inline CLocateDlg::ImageHandlerDll::ImageHandlerDll()
:	pGetImageDimensionsA(NULL),uToken(0)
{
	DWORD bLoadDll=TRUE;
	{
		CRegKey RegKey;
		if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource),CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
			RegKey.QueryValue("Load imagehandler",bLoadDll);
	}
	if (!bLoadDll)
	{
		hModule=NULL;
		return;
	}

	hModule=LoadLibrary("imghnd.dll");
	if (hModule!=NULL)
	{
		IH_INITLIBRARY pInitLibrary=(IH_INITLIBRARY)GetProcAddress(hModule,"InitLibrary");
		if (pInitLibrary==NULL)
		{
			FreeLibrary(hModule);
			hModule=NULL;
			return;
		}
		if (!pInitLibrary(&uToken))
		{
			FreeLibrary(hModule);
			hModule=NULL;
			return;
		}
		pGetImageDimensionsA=(IH_GETIMAGEDIMENSIONSA)GetProcAddress(hModule,"GetImageDimensionsA");
		if (pGetImageDimensionsA==NULL)
		{
			FreeLibrary(hModule);
			hModule=NULL;
			return;
		}
	}
}

inline CLocateDlg::ImageHandlerDll::~ImageHandlerDll()
{
	if (hModule!=NULL)
	{
		IH_UNINITLIBRARY pUnIninitLibrary=(IH_UNINITLIBRARY)GetProcAddress(hModule,"UninitLibrary");
		if (pUnIninitLibrary!=NULL)
			pUnIninitLibrary(uToken);
		FreeLibrary(hModule);
	}
}
		
inline BOOL CLocateDlg::ImageHandlerDll::IsLoaded()
{
	return hModule!=NULL;
}

inline CLocateDlg::CAdvancedDlg::FileType::FileType()
:	szExtensions(NULL),szTitle(NULL),szType(NULL),hIcon(NULL),szIconPath(NULL)
{
}
			
inline CLocateDlg::CAdvancedDlg::FileType::FileType(FREEDATA frType,FREEDATA frTitle)
:	szExtensions(NULL),szTitle(LPSTR(frTitle)),szType(LPSTR(frType)),szIconPath(NULL),hIcon(NULL)
{
}

inline DWORD CLocateApp::GetLongPathName(LPCSTR lpszShortPath,LPSTR lpszLongPath,DWORD cchBuffer)
{
	LPSTR pTemp;
	return GetFullPathName(lpszShortPath,cchBuffer,lpszLongPath,&pTemp);
}

inline void CLocateDlg::RemoveResultsFromList()
{
	m_pListCtrl->DeleteAllItems();
	m_aVolumeInformation.RemoveAll();
}
			
inline CLocateDlg::VolumeInformation::VolumeInformation(WORD wDB_,WORD wRootID_,BYTE bType_,DWORD dwVolumeSerial,LPCSTR szVolumeLabel,LPCSTR szFileSystem)
:	wDB(wDB_),wRootID(wRootID_),bType(bType_)
{
	if (dwVolumeSerial!=0 && dwVolumeSerial!=DWORD(-1))
	{
		szVolumeSerial=new char[12];
		wsprintf(szVolumeSerial,"%0X-%0X",HIWORD(dwVolumeSerial),LOWORD(dwVolumeSerial));
	}
	else
		szVolumeSerial=NULL;

	if (szVolumeLabel!=NULL)
		this->szVolumeLabel=alloccopy(szVolumeLabel);
	else
		this->szVolumeLabel=NULL;
	if (szFileSystem!=NULL)
		this->szFileSystem=alloccopy(szFileSystem);
	else
		this->szFileSystem=NULL;
}

inline CLocateDlg::VolumeInformation::~VolumeInformation()
{
	if (szVolumeLabel!=NULL)
		delete[] szVolumeLabel;
	if (szFileSystem!=NULL)
		delete[] szFileSystem;
	if (szVolumeSerial!=NULL)
		delete[] szVolumeSerial;
}

inline LPCSTR CLocateDlg::GetVolumeLabel(WORD wDB,WORD wRootID) 
{
	CArrayFP<VolumeInformation*>& aVolumeInformation=GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->m_aVolumeInformation;

	for (int i=0;i<aVolumeInformation.GetSize();i++)
	{
		if (aVolumeInformation[i]->wDB==wDB && aVolumeInformation[i]->wRootID==wRootID)
			return aVolumeInformation[i]->szVolumeLabel!=NULL?aVolumeInformation[i]->szVolumeLabel:szEmpty;
	}
	return szEmpty;
}

inline LPCSTR CLocateDlg::GetVolumeSerial(WORD wDB,WORD wRootID) 
{
	CArrayFP<VolumeInformation*>& aVolumeInformation=GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->m_aVolumeInformation;

	for (int i=0;i<aVolumeInformation.GetSize();i++)
	{
		if (aVolumeInformation[i]->wDB==wDB && aVolumeInformation[i]->wRootID==wRootID)
			return aVolumeInformation[i]->szVolumeSerial!=NULL?aVolumeInformation[i]->szVolumeSerial:szEmpty;
	}
	return szEmpty;
}

inline LPCSTR CLocateDlg::GetVolumeFileSystem(WORD wDB,WORD wRootID) 
{
	CArrayFP<VolumeInformation*>& aVolumeInformation=GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->m_aVolumeInformation;

	for (int i=0;i<aVolumeInformation.GetSize();i++)
	{
		if (aVolumeInformation[i]->wDB==wDB && aVolumeInformation[i]->wRootID==wRootID)
			return aVolumeInformation[i]->szFileSystem!=NULL?aVolumeInformation[i]->szFileSystem:szEmpty;
	}
	return szEmpty;
}

inline CLocateDlg::ContextMenuStuff::ContextMenuStuff()
:	pContextMenu3(NULL),pContextMenu2(NULL),pContextMenu(NULL),
	pParentFolder(NULL),pParentIDList(NULL),apidl(NULL)
{
}

inline CLocateDlg::ContextMenuStuff::~ContextMenuStuff()
{
	// Releasing memory
	
	if (pContextMenu3!=NULL)
		pContextMenu3->Release();
	if (pContextMenu2!=NULL)
		pContextMenu2->Release();
	if (pContextMenu!=NULL)
		pContextMenu->Release();
	if (pParentFolder!=NULL)
		pParentFolder->Release();
	

	IMalloc* pMalloc;
	if (SHGetMalloc(&pMalloc)==NOERROR)
	{
		if (pParentIDList!=NULL)
			pMalloc->Free(pParentIDList);
		
		if (apidl!=NULL)
		{
			for (int i=0;i<nIDlistCount;i++)
				pMalloc->Free((void*)apidl[i]);
			delete[] apidl;
		}

		pMalloc->Release();
	}
	//delete[] apcidl;

}


inline void CLocateDlg::ClearResultlistActions()
{
	for (int iCol=0;iCol<TypeCount;iCol++)
	{
		for (int iAct=0;iAct<ListActionCount;iAct++)
		{
			if (m_aResultListActions[iCol][iAct]!=NULL)
			{
				delete m_aResultListActions[iCol][iAct];
				m_aResultListActions[iCol][iAct]=NULL;
			}
		}
	}
}

inline void CLocateDlg::CNameDlg::HilightTab(BOOL bHilight)
{
	GetLocateDlg()->HilightTab(0,IDS_NAME,bHilight);
}

inline void CLocateDlg::CSizeDateDlg::HilightTab(BOOL bHilight)
{
	GetLocateDlg()->HilightTab(1,IDS_SIZEDATE,bHilight);
}

inline void CLocateDlg::CAdvancedDlg::HilightTab(BOOL bHilight)
{
	GetLocateDlg()->HilightTab(2,IDS_ADVANCED,bHilight);
}

#endif