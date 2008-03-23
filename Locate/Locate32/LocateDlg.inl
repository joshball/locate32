/* Locate32 - Copyright (c) 1997-2008 Janne Huttunen */

#if !defined(LOCATEDLG_INL)
#define LOCATEDLG_INL

#if _MSC_VER >= 1000
#pragma once
#endif


//////////////////////////////////////////////////////////////////////////////
// CLocateDlgThread
//////////////////////////////////////////////////////////////////////////////

inline CLocateDlgThread::CLocateDlgThread()
:	m_pLocate(NULL)
{
	m_bAutoDelete=TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// CLocateDlg::VolumeInformation
//////////////////////////////////////////////////////////////////////////////



inline CLocateDlg::VolumeInformation::VolumeInformation(WORD wDB_,WORD wRootID_,BYTE bType_,DWORD dwVolumeSerial,LPCWSTR szVolumeLabel,LPCWSTR szFileSystem)
:	wDB(wDB_),wRootID(wRootID_),bType(bType_)
{
	if (dwVolumeSerial!=0 && dwVolumeSerial!=DWORD(-1))
	{
		szVolumeSerial=new WCHAR[12];
		StringCbPrintfW(szVolumeSerial,12*sizeof(WCHAR),L"%0X-%0X",HIWORD(dwVolumeSerial),LOWORD(dwVolumeSerial));
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


//////////////////////////////////////////////////////////////////////////////
// CLocateDlg
//////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Constructor

inline CLocateDlg::CLocateDlg()
:	CDialog(IDD_MAIN),m_dwFlags(fgDefault),m_dwExtraFlags(efDefault),
	m_nSorting(BYTE(-1)),m_dwInstantFlags(isDefault),
	m_dwInstantLimit(DEFAULT_INSTANTSEARCHLIMIT),
	m_dwInstantDelay(DEFAULT_INSTANTSEARCHDELAY),
	m_dwInstantChars(DEFAULT_INSTANTSEARCHCHARS),
	m_nMaxYMinimized(0),m_nMaxYMaximized(0),m_nLargeY(354),
	m_ClickWait(FALSE),m_hSendToListFont(NULL),m_hActivePopupMenu(NULL),
	m_pListCtrl(NULL),m_pTabCtrl(NULL),m_pStatusCtrl(NULL),m_pListTooltips(NULL),
	m_pLocater(NULL),m_pBackgroundUpdater(NULL),m_pActiveContextMenu(NULL),
	m_pLocateAnimBitmaps(NULL),m_pUpdateAnimBitmaps(NULL),
	m_pFileNotificationsThread(NULL),m_dwMaxFoundFiles(0),m_pDropTarget(NULL),
	m_pImageHandler(NULL),m_iTooltipItem(-1),m_iTooltipSubItem(-1),m_bTooltipActive(FALSE),
	m_hLastFocus(NULL),m_WaitEvery30(0),m_WaitEvery60(0),m_hDialogFont(NULL),
	m_hLargeDialogIcon(NULL),m_hSmallDialogIcon(NULL)
{
	ZeroMemory(m_aResultListActions,TypeCount*ListActionCount*sizeof(void*));

	
	if (FAILED(SHGetDesktopFolder(&m_pDesktopFolder)))
		m_pDesktopFolder=NULL;

	InitializeCriticalSection(&m_csUpdateAnimBitmaps);
	InitializeCriticalSection(&m_csLocateAnimBitmaps);
}

////////////////////////////////////////////////////////////
// Dialog, tabs and items

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


////////////////////////////////////////////////////////////
// Menu related functions

inline BOOL CLocateDlg::IsSendToMenu(HMENU hMenu)
{
	/*UINT nID=GetMenuItemID(hMenu,0);
	return nID==IDM_DEFSENDTOITEM && nID<IDM_DEFSENDTOITEM+1000;*/

	return GetMenuItemID(hMenu,0)==IDM_DEFSENDTOITEM;
}
	

////////////////////////////////////////////////////////////
// Results list
	
inline void CLocateDlg::RemoveResultsFromList()
{
	ClearMenuVariables();
	m_pListCtrl->DeleteAllItems();
	m_aVolumeInformation.RemoveAll();

	m_pStatusCtrl->SetText("",STATUSBAR_MISC,0);
	m_pStatusCtrl->SetText("",STATUSBAR_OPERATIONSTATUS,0);
	m_pStatusCtrl->SetText("",STATUSBAR_LOCATEICON,0);
	m_pStatusCtrl->SetText("",STATUSBAR_UPDATEICON,0);
}
			

inline LPCWSTR CLocateDlg::GetDBVolumeLabel(WORD wDB,WORD wRootID) 
{
	CArrayFP<VolumeInformation*>& aVolumeInformation=GetTrayIconWnd()->m_pLocateDlgThread->m_pLocate->m_aVolumeInformation;

	for (int i=0;i<aVolumeInformation.GetSize();i++)
	{
		if (aVolumeInformation[i]->wDB==wDB && aVolumeInformation[i]->wRootID==wRootID)
			return aVolumeInformation[i]->szVolumeLabel!=NULL?aVolumeInformation[i]->szVolumeLabel:szwEmpty;
	}
	return szwEmpty;
}

inline LPCWSTR CLocateDlg::GetDBVolumeSerial(WORD wDB,WORD wRootID) 
{
	CArrayFP<VolumeInformation*>& aVolumeInformation=GetTrayIconWnd()->m_pLocateDlgThread->m_pLocate->m_aVolumeInformation;

	for (int i=0;i<aVolumeInformation.GetSize();i++)
	{
		if (aVolumeInformation[i]->wDB==wDB && aVolumeInformation[i]->wRootID==wRootID)
			return aVolumeInformation[i]->szVolumeSerial!=NULL?aVolumeInformation[i]->szVolumeSerial:szwEmpty;
	}
	return szwEmpty;
}

inline LPCWSTR CLocateDlg::GetDBVolumeFileSystem(WORD wDB,WORD wRootID) 
{
	CArrayFP<VolumeInformation*>& aVolumeInformation=GetTrayIconWnd()->m_pLocateDlgThread->m_pLocate->m_aVolumeInformation;

	for (int i=0;i<aVolumeInformation.GetSize();i++)
	{
		if (aVolumeInformation[i]->wDB==wDB && aVolumeInformation[i]->wRootID==wRootID)
			return aVolumeInformation[i]->szFileSystem!=NULL?aVolumeInformation[i]->szFileSystem:szwEmpty;
	}
	return szwEmpty;
}



////////////////////////////////////////////////////////////
// Keyboard shortcuts and results list mouse actions


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

inline void CLocateDlg::SetDefaultActions(CSubAction*** pActions) const
{
	pActions[Name][
		GetFlags()&fgLVStylePointToSelect?LeftMouseButtonClick:LeftMouseButtonDblClick]=new CSubAction(CSubAction::Execute);
	pActions[Name][RightMouseButtonClick]=new CSubAction(CSubAction::OpenContextMenu);
}


////////////////////////////////////////////////////////////
// Misc helpers

inline HANDLE CLocateDlg::GetLocaterThread(BOOL bDuplicate)
{
	if (m_pLocater==NULL)
		return NULL;
	return m_pLocater->GetThreadHandle(bDuplicate);
}




//////////////////////////////////////////////////////////////////////////////
// CLocateDlg::CRemovePresetDlg
//////////////////////////////////////////////////////////////////////////////


inline CLocateDlg::CRemovePresetDlg::CRemovePresetDlg()
: CDialog(IDD_PRESETREMOVE)
{
}



//////////////////////////////////////////////////////////////////////////////
// CLocateDlg::ImageHandlerDll
//////////////////////////////////////////////////////////////////////////////

inline CLocateDlg::ImageHandlerDll::ImageHandlerDll()
:	pGetImageDimensionsW(NULL),uToken(0)
{
	DWORD bLoadDll=TRUE;
	{
		CRegKey2 RegKey;
		if (RegKey.OpenKey(HKCU,"",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
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
		pGetImageDimensionsW=(IH_GETIMAGEDIMENSIONSW)GetProcAddress(hModule,"GetImageDimensionsW");
		if (pGetImageDimensionsW==NULL)
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



//////////////////////////////////////////////////////////////////////////////
// CLocateDlg::ContextMenuStuff
//////////////////////////////////////////////////////////////////////////////


inline CLocateDlg::ContextMenuStuff::ContextMenuStuff()
:	pContextMenu3(NULL),pContextMenu2(NULL),pContextMenu(NULL),
	pParentFolder(NULL),pParentIDL(NULL),ppSimpleIDLs(NULL)
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
	

	CoTaskMemFree(pParentIDL);

	if (ppSimpleIDLs!=NULL)
	{
		for (int i=0;i<nIDLCount;i++)
			CoTaskMemFree((void*)ppSimpleIDLs[i]);
		delete[] ppSimpleIDLs;
	}

	//delete[] apcidl;
}


#endif