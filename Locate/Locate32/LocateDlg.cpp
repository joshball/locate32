#include <HFCLib.h>
#include "Locate32.h"

CBufferAllocator<BYTE*,2000,BUFFERALLOC_EXTRALEN> FileTypeAllocator;	

LPSTR g_szBuffer=NULL; 


BOOL CLocateDlgThread::InitInstance()
{
	DebugNumMessage("CLocateDlgThread::InitInstance(), thread is 0x%X",GetCurrentThreadId());

	CWinThread::InitInstance();
	CoInitialize(NULL);
	
	m_pMainWnd=m_pLocate=new CLocateDlg;
	LoadAccelTable(IDR_MAINACCEL);
	m_pLocate->Create(NULL);
	m_pLocate->SetMenu(LoadMenu(IDR_MAINMENU));
	//m_pLocate->SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	
	return TRUE;
}

int CLocateDlgThread::ExitInstance()
{
	delete m_pLocate;
	m_pLocate=NULL;
	GetLocateAppWnd()->m_pLocateDlgThread=NULL;
	CoUninitialize();
	return CWinThread::ExitInstance();
}

BOOL CLocateDlgThread::OnThreadMessage(MSG* pMsg)
{
	if (m_pLocate->m_pListTooltips==NULL)
		return FALSE;

	if (pMsg->hwnd==*m_pLocate->m_pListTooltips)
	{
		switch (pMsg->message)
		{
		case WM_MOUSEMOVE:
			// If this is given to work, tooltip will be shown and 
			// hidden sequentally when mouse is over tooltip
			if (m_pLocate->m_bTooltipActive)
				return TRUE;
            return FALSE;
		case WM_TIMER:
			// If 3rd timer is given to work, tooltip will be shown and 
			// hidden sequentally when mouse is over tooltip
			if (m_pLocate->m_bTooltipActive && pMsg->wParam==3)
				return TRUE;
			break;
		}
		return FALSE;
	}
	if (pMsg->hwnd==*m_pLocate->m_pListCtrl)
	{
		if (pMsg->message!=WM_MOUSEMOVE || 
			pMsg->wParam&(MK_LBUTTON|MK_MBUTTON|MK_RBUTTON|MK_XBUTTON1|MK_XBUTTON2))
			return FALSE;

			
		// Resolving item below cursor, if any
		LVHITTESTINFO ht;
		ht.pt.x=LOWORD(pMsg->lParam);
		ht.pt.y=HIWORD(pMsg->lParam);
		int iRet=m_pLocate->m_pListCtrl->SubItemHitTest(&ht);

		if (iRet!=-1)
		{
			if (m_pLocate->m_iTooltipItem!=ht.iItem || 
				m_pLocate->m_iTooltipSubItem!=ht.iSubItem)
			{
				// First, let control handle messages
				m_pLocate->m_pListTooltips->RelayEvent(pMsg);

				// Deleting previous tools
				m_pLocate->DeleteTooltipTools();

				if (CLocateDlg::DetailType(m_pLocate->m_pListCtrl->GetColumnIDFromSubItem(ht.iSubItem))==CLocateDlg::DetailType::Title)
				{
					TOOLINFO tii;
					tii.cbSize = TTTOOLINFOA_V2_SIZE;
					
					tii.uFlags = TTF_IDISHWND;
					tii.hwnd   = *m_pLocate;
					tii.uId    = (UINT)pMsg->hwnd;
					tii.hinst  = NULL;
					tii.lpszText  = LPSTR_TEXTCALLBACK;
				
					tii.lParam = m_pLocate->m_pListCtrl->GetItemData(ht.iItem);
				
					m_pLocate->m_pListTooltips->AddTool(&tii);
					m_pLocate->m_pListTooltips->SetMaxTipWidth(400);

					m_pLocate->m_iTooltipItem=ht.iItem;
					m_pLocate->m_iTooltipSubItem=ht.iSubItem;
				}
				else
				{
					CRect rc,rc2;
					m_pLocate->m_pListCtrl->GetSubItemRect(ht.iItem,ht.iSubItem,LVIR_LABEL,&rc);
					m_pLocate->m_pListCtrl->GetSubItemRect(ht.iItem,ht.iSubItem,LVIR_ICON,&rc2);


					
					int nBufferLength=100;
					char* pText;
					for (;;)
					{	
						pText=new char[nBufferLength+2];
						if (m_pLocate->m_pListCtrl->GetItemText(ht.iItem,ht.iSubItem,pText,nBufferLength)>=nBufferLength-1)
						{
							nBufferLength*=2;
							delete[] pText;
						}
						else
							break;
					}
                   
					int nWidth=m_pLocate->m_pListCtrl->GetStringWidth(pText)+12;
					delete[] pText;
					
					// InFolder need also space for icon
					if (CLocateDlg::DetailType(m_pLocate->m_pListCtrl->GetColumnIDFromSubItem(ht.iSubItem))==CLocateDlg::DetailType::InFolder)
						nWidth+=rc2.Width()+5;

					if (nWidth>rc.Width())
					{
						TOOLINFO tii;
						tii.cbSize = TTTOOLINFOA_V2_SIZE;
		
						tii.uFlags = TTF_IDISHWND;
						tii.hwnd   = *m_pLocate;
						tii.uId    = (UINT)pMsg->hwnd;
						tii.hinst  = NULL;
						tii.lpszText  = LPSTR_TEXTCALLBACK;
						tii.lParam = m_pLocate->m_pListCtrl->GetItemData(ht.iItem);
						m_pLocate->GetClientRect(&tii.rect);
						m_pLocate->m_pListTooltips->AddTool(&tii);
				
                        m_pLocate->m_iTooltipItem=ht.iItem;
						m_pLocate->m_iTooltipSubItem=ht.iSubItem;

						
					}
					else
					{
						m_pLocate->m_iTooltipItem=-1;
						m_pLocate->m_iTooltipSubItem=-1;
					}


				}
			}
			else
			{
				if (!m_pLocate->m_bTooltipActive) 
					m_pLocate->m_pListTooltips->RelayEvent(pMsg);
				return FALSE;
			}
		}
		else
		{
			if (m_pLocate->m_iTooltipItem!=-1)
				m_pLocate->DeleteTooltipTools();
			m_pLocate->m_pListTooltips->RelayEvent(pMsg);			
		}
	}
	return FALSE;
}

CLocateDlg::CLocateDlg()
:	CDialog(IDD_MAIN),m_dwFlags(fgDefault),m_dwExtraFlags(efDefault),m_nSorting(BYTE(-1)),
	m_nMaxYMinimized(0),m_nMaxYMaximized(0),m_nLargeY(354),
	m_ClickWait(FALSE),m_hSendToListFont(NULL),m_hActivePopupMenu(NULL),
	m_pListCtrl(NULL),m_pTabCtrl(NULL),m_pStatusCtrl(NULL),m_pListTooltips(NULL),
	m_pLocater(NULL),m_pBackgroundUpdater(NULL),m_pActiveContextMenu(NULL),
	m_pLocateAnimBitmaps(NULL),m_pUpdateAnimBitmaps(NULL),
	m_pFileNotificationsThread(NULL),m_dwMaxFoundFiles(0),
	m_pImageHandler(NULL),m_iTooltipItem(-1),m_iTooltipSubItem(-1),m_bTooltipActive(FALSE),
	m_hLastFocus(NULL)
{
}

BOOL CLocateDlg::OnInitDialog(HWND hwndFocus)
{
	ShowDlgItem(IDC_FILELIST,swHide);
	ShowDlgItem(IDC_STATUS,swHide);
	
	// Loading "imghnd.dll"
	m_pImageHandler=new ImageHandlerDll;
	if (!m_pImageHandler->IsLoaded())
	{
		delete m_pImageHandler;
		m_pImageHandler=NULL;
	}


	CDialog::OnInitDialog(hwndFocus);
	
	CMenu menu(GetSubMenu(GetMenu(),2));

	
	m_Menu.LoadMenu(IDR_POPUPMENU);
	
	if (((CLocateApp*)GetApp())->m_wComCtrlVersion<0x0600) // Disabling features
	{
		menu.EnableMenuItem(IDM_ALIGNTOGRID,MF_GRAYED|MF_BYCOMMAND);
		m_Menu.EnableMenuItem(IDM_ALIGNTOGRID,MF_GRAYED|MF_BYCOMMAND);
	}

	m_hNextClipboardViewer=SetClipboardViewer(*this);

	m_CircleBitmap.LoadBitmap(IDB_CIRCLE);
	if (menu.GetSafeHmenu()!=NULL)
	{
		menu.SetMenuItemBitmaps(0,MF_BYPOSITION,NULL,&m_CircleBitmap);
		menu.SetMenuItemBitmaps(1,MF_BYPOSITION,NULL,&m_CircleBitmap);
		menu.SetMenuItemBitmaps(2,MF_BYPOSITION,NULL,&m_CircleBitmap);
		menu.SetMenuItemBitmaps(3,MF_BYPOSITION,NULL,&m_CircleBitmap);
	}
	m_pTabCtrl=new CTabCtrl(GetDlgItem(IDC_TAB));
	m_pListCtrl=new CListCtrlEx(GetDlgItem(IDC_FILELIST));
	m_pStatusCtrl=new CStatusBarCtrl(GetDlgItem(IDC_STATUS));
	
	// Setting tab control labels	
	if (IsFullUnicodeSupport())
	{
		WCHAR Buffer[80];
		TC_ITEMW ti;
	
		LoadString(IDS_NAME,Buffer,80);
		ti.pszText=Buffer;
		ti.mask=TCIF_TEXT;
		m_pTabCtrl->InsertItem(0,&ti);
		LoadString(IDS_SIZEDATE,Buffer,80);
		ti.pszText=Buffer;
		ti.mask=TCIF_TEXT;
		m_pTabCtrl->InsertItem(1,&ti);
		LoadString(IDS_ADVANCED,Buffer,80);
		ti.pszText=Buffer;
		ti.mask=TCIF_TEXT;
		m_pTabCtrl->InsertItem(2,&ti);
	}
	else
	{
		CHAR Buffer[80];
		TC_ITEM ti;

		LoadString(IDS_NAME,Buffer,80);
		ti.pszText=Buffer;
		ti.mask=TCIF_TEXT;
		m_pTabCtrl->InsertItem(0,&ti);
		LoadString(IDS_SIZEDATE,Buffer,80);
		ti.pszText=Buffer;
		ti.mask=TCIF_TEXT;
		m_pTabCtrl->InsertItem(1,&ti);
		LoadString(IDS_ADVANCED,Buffer,80);
		ti.pszText=Buffer;
		ti.mask=TCIF_TEXT;
		m_pTabCtrl->InsertItem(2,&ti);
	}

	m_pListCtrl->InsertColumn(Title,IDS_LISTNAME,TRUE,LVCFMT_LEFT,200,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(InFolder,IDS_LISTINFOLDER,TRUE,LVCFMT_LEFT,300,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(FullPath,IDS_LISTFULLPATH,FALSE,LVCFMT_LEFT,300,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(ShortFileName,IDS_LISTSHORTFILENAME,FALSE,LVCFMT_LEFT,200,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(ShortFilePath,IDS_LISTSHORTFILEPATH,FALSE,LVCFMT_LEFT,300,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(FileSize,IDS_LISTSIZE,TRUE,LVCFMT_LEFT,70,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(FileType,IDS_LISTTYPE,TRUE,LVCFMT_LEFT,130,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(DateModified,IDS_LISTMODIFIED,TRUE,LVCFMT_LEFT,100,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(DateCreated,IDS_LISTCREATED,FALSE,LVCFMT_LEFT,100,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(DateAccessed,IDS_LISTACCESSED,FALSE,LVCFMT_LEFT,100,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(Attributes,IDS_LISTATTRIBUTES,FALSE,LVCFMT_LEFT,70,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(ImageDimensions,IDS_LISTIMAGEINFO,FALSE,LVCFMT_LEFT,150,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(Owner,IDS_LISTOWNER,FALSE,LVCFMT_LEFT,130,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(Database,IDS_LISTDATABASE,FALSE,LVCFMT_LEFT,70,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(DatabaseDescription,IDS_LISTDATABASEDESC,FALSE,LVCFMT_LEFT,150,LanguageSpecificResource);
	m_pListCtrl->InsertColumn(DatabaseArchive,IDS_LISTDATABASEFILE,FALSE,LVCFMT_LEFT,150,LanguageSpecificResource);
	
	m_pListCtrl->SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP,LVS_EX_HEADERDRAGDROP);
	m_pListCtrl->LoadColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General","ListWidths");

	
	// Creating windows
	m_NameDlg.Create(*this);
	m_NameDlg.ShowWindow(swShow);
	m_SizeDateDlg.Create(*this);
	m_SizeDateDlg.ShowWindow(swHide);
	m_AdvancedDlg.Create(*this);
	m_AdvancedDlg.ShowWindow(swHide);
	
	
	// Retrieving the maximum height
	CRect rcTemp,rcTemp2;
	m_NameDlg.GetClientRect(&rcTemp);
	m_nTabbedDialogHeight=rcTemp.Height();
    m_SizeDateDlg.GetClientRect(&rcTemp);
	if (m_nTabbedDialogHeight<rcTemp.Height())
		m_nTabbedDialogHeight=rcTemp.Height();
	m_AdvancedDlg.GetClientRect(&rcTemp);
	if (m_nTabbedDialogHeight<rcTemp.Height())
		m_nTabbedDialogHeight=rcTemp.Height();
	if (m_nTabbedDialogHeight<30)
		m_nTabbedDialogHeight=143;
	
	// Computing the height of dialog when dialog is minimized
	::GetWindowRect(GetDlgItem(IDC_TAB),&rcTemp);
	m_nTabHeaderHeight=rcTemp.Height();
	m_nMaxYMinimized=m_nTabbedDialogHeight+m_nTabHeaderHeight+GetSystemMetrics(SM_CYCAPTION)
		+GetSystemMetrics(SM_CYMENU)+2*GetSystemMetrics(SM_CYDLGFRAME)+30;	
	
	// Get width and spacing of buttons
	::GetWindowRect(GetDlgItem(IDC_OK),&rcTemp);
	m_nButtonWidth=rcTemp.Width();
	::GetWindowRect(GetDlgItem(IDC_STOP),&rcTemp2);
	m_nButtonSpacing=BYTE(rcTemp2.top-rcTemp.top);
	
	::GetWindowRect(GetDlgItem(IDC_PRESETS),&rcTemp);
	m_nPresetsButtonWidth=rcTemp.Width();
	m_nPresetsButtonHeight=rcTemp.Height();
    	
	

	// Loading registry
	LoadRegistry();
	// Refreshing dialog box
	if ((GetLocateDlg()->GetFlags()&CLocateDlg::fgNameRootFlag)!=CLocateDlg::fgNameDontAddRoots)
        m_NameDlg.InitDriveBox();
		
	// Set acceleration tables for subdialogs
	GetCurrentWinThread()->SetAccelTableForChilds(m_NameDlg,IDR_NAMEDLGACCEL,TRUE,*this);
	GetCurrentWinThread()->SetAccelTableForChilds(m_SizeDateDlg,IDR_SIZEDATEDLGACCEL,TRUE,*this);
	GetCurrentWinThread()->SetAccelTableForChilds(m_AdvancedDlg,IDR_ADVANCEDDLGACCEL,TRUE,*this);
	
	SetDialogMode(FALSE);
		
	

	
	
	// Setting list control imagelists and style
	SetSystemImagelists(m_pListCtrl,&m_AdvancedDlg.m_hDefaultTypeIcon);
	SetListSelStyle();

    // Setting icons
	HICON hIcon=(HICON)LoadImage(IDI_APPLICATIONICON,IMAGE_ICON,32,32,LR_SHARED);
	SetIcon(hIcon,TRUE);
	SetClassLong(gclHIcon,(LONG)hIcon);
	hIcon=(HICON)LoadImage(IDI_APPLICATIONICON,IMAGE_ICON,16,16,LR_SHARED);
	SetIcon(hIcon,FALSE);
	SetClassLong((CWnd::ClassLongIndex)GCLP_HICONSM,(LONG)hIcon);


	// Setting tooltips
	if (!(GetFlags()&fgLVDontShowTooltips))
	{
		m_pListTooltips=new CToolTipCtrl;
		m_pListTooltips->Create(*this);
		m_pListTooltips->SetFont(m_pListCtrl->GetFont());
	}

	// Enabling multidirectory support if needed
	m_NameDlg.EnableMultiDirectorySupport(GetFlags()&fgNameMultibleDirectories?TRUE:FALSE);
	
	// Loading texts which are used at last time
	if (m_dwFlags&fgDialogRememberFields)
		LoadDialogTexts();
	
	// Taking command line parameters to use
	if (GetLocateApp()->GetStartData()!=NULL)
	{
		SetStartData(GetLocateApp()->GetStartData());
		GetLocateApp()->ClearStartData();
	}
	// If updating is started via command line arguments or otherwise, start animation
	if (GetLocateAppWnd()->m_pUpdateAnimIcons!=NULL)
		StartUpdateAnimation();

#ifdef _DEBUG
	CMenu smenu(GetSystemMenu());
	MENUITEMINFO mii;
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_ID|MIIM_TYPE;
	mii.wID=0x76;
	mii.dwTypeData="Show shedule from data";
	mii.dwItemData=0;
	mii.fType=MFT_STRING;
	smenu.InsertMenu(0x76,FALSE,&mii);
	mii.wID=0x77;
	mii.dwTypeData="Show background op state";
	mii.dwItemData=0;
	mii.fType=MFT_STRING;
	smenu.InsertMenu(0x77,FALSE,&mii);
	
#endif

	m_NameDlg.SetFocus();
	m_NameDlg.SetFocus(IDC_NAME);
	
	return FALSE;
}


BOOL CLocateDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch(wID)
	{
	case IDC_RETURN:
		if (GetFocus()==*m_pListCtrl)
		{
			OnMenuCommands(0);
			break;
		}
	case IDC_OK:
		if (::IsWindowEnabled(GetDlgItem(IDC_OK)))
			OnOk();
		break;
	case IDM_FINDUSINGDBSACCEL:
	case IDM_FINDUSINGDBS:
		if (::IsWindowEnabled(GetDlgItem(IDC_OK)))
			OnOk(TRUE);
		break;		
	case IDC_STOP:
		if (::IsWindowEnabled(GetDlgItem(IDC_STOP)))
			OnStop();
		break;
	case IDC_NEWSEARCH:
		if (::IsWindowEnabled(GetDlgItem(IDC_NEWSEARCH)))
			OnNewSearch();
		break;
	case IDM_ABOUT:
		{
			CAboutDlg About;
			About.DoModal(*this);
		}
		break;
	case IDM_LARGEICONS:
		SetListStyle(0);
		break;
	case IDM_SMALLICONS:
		SetListStyle(1);
		break;
	case IDM_LIST:
		SetListStyle(2);
		break;
	case IDM_DETAILS:
		SetListStyle(3);
		break;
	case IDM_ARRANGENAME:
		SortItems(DetailType::Title);
		break;
	case IDM_ARRANGEFOLDER:
		SortItems(DetailType::InFolder);
		break;
	case IDM_ARRANGETYPE:
		SortItems(DetailType::FileType);
		break;
	case IDM_ARRANGESIZE:
		SortItems(DetailType::FileSize);
		break;
	case IDM_ARRANGEDATE:
		SortItems(DetailType::DateModified);
		break;
	case IDM_AUTOARRANGE:
		OnAutoArrange();
		break;
	case IDM_ALIGNTOGRID:
		OnAlignToGrid();
		break;
	case IDM_REFRESH:
		OnRefresh();
		break;
	case IDM_SETTINGS:
		OnSettings();
		break;
	case IDM_SELECTDETAILS:
		OnSelectDetails();
		break;
	case IDM_GLOBALUPDATEDB:
	case IDM_GLOBALUPDATEDBACCEL:
		GetLocateAppWnd()->OnUpdate(FALSE);
		break;
	case IDM_STOPUPDATING:
		// Stopping updating quite nicely
		GetLocateAppWnd()->OnUpdate(TRUE);
        break;	
	case IDM_UPDATEDATABASES:
	case IDM_UPDATEDATABASESACCEL:
		GetLocateAppWnd()->OnUpdate(FALSE,LPSTR(-1));
		break;
	case IDC_PRESETS:
		OnPresets();
		break;
	case IDM_DATABASEINFO:
		{
			CDatabaseInfos info(*this);
			info.DoModal();
			break;
		}
	case IDM_CLOSE:
		DestroyWindow();
		break;
	case IDM_CUT:
		if (GetFocus()==GetDlgItem(IDC_FILELIST))
			OnCopy(TRUE);
		else
			::SendMessage(GetFocus(),WM_CUT,0,0);
		break;
	case IDM_COPY:
		if (GetFocus()==GetDlgItem(IDC_FILELIST))
			OnCopy(FALSE);
		else
			::SendMessage(GetFocus(),WM_COPY,0,0);
		break;
	case IDM_OPENCONTAININGFOLDER:
		OnOpenContainingFolder();
		break;
	case IDM_CREATESHORTCUT:
		OnCreateShortcut();
		break;
	case IDC_DELETEKEY:
	case IDM_DELETE:
		OnDelete();
		break;
	case IDM_REMOVEFROMTHISLIST:
		OnRemoveFromThisList();
		break;
	case IDC_SELECTALLKEY:
		if (GetFocus()!=GetDlgItem(IDC_FILELIST))
			break;
	case IDM_SELECTALL:
		OnSelectAll();
		break;
	case IDM_INVERTSELECTION:
		OnInvertSelection();
		break;
	case IDM_LINEUPICONS:
		m_pListCtrl->Arrange(LVA_SNAPTOGRID);
		break;
	case IDM_COPYPATHTOCB:
		OnCopyPathToClipboard(FALSE);
		break;
	case IDM_COPYSHORTPATHTOCB:
		OnCopyPathToClipboard(TRUE);
		break;
	case IDM_CHANGECASE:
		OnChangeFileNameCase();
		break;
	case IDM_FORCEUPDATE:
		OnUpdateLocatedItem();
		break;
	case IDM_CHANGEFILENAME:
	case IDM_CHANGEFILENAMEACCEL:
		OnChangeFileName();
		break;
	case IDC_NAME:
	case IDC_TYPE:
	case IDC_LOOKIN:
	case IDC_BROWSE:
		// This is to ensure that these conrols get focus e.g. when alt+n is pressed
		return m_NameDlg.SendMessage(WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),(LPARAM)hControl);
	case IDC_CHECKMINIMUMSIZE:
	case IDC_CHECKMAXIMUMSIZE:
	case IDC_CHECKMINDATE:
	case IDC_CHECKMAXDATE:
		// This is to ensure that these conrols get focus e.g. when alt+n is pressed
		return m_SizeDateDlg.SendMessage(WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),(LPARAM)hControl);
	case IDC_CHECK:
	case IDC_MATCHWHOLENAME:
	case IDC_FILETYPE:
	case IDC_CONTAINDATACHECK:
	case IDC_DATAMATCHCASE:
	case IDC_REPLACESPACES:
		// This is to ensure that these conrols get focus e.g. when alt+n is pressed
		return m_AdvancedDlg.SendMessage(WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),(LPARAM)hControl);
	case IDC_PTAB:
		::SetFocus(GetNextDlgTabItem(GetFocus(),FALSE));
		break;
	case IDC_PUNTAB:
		::SetFocus(GetNextDlgTabItem(GetFocus(),TRUE));
		break;
	case IDM_SAVERESULT:
		OnSaveResults();
		break;
	case IDM_PROPERTIES:
		OnProperties();
		break;
	case IDM_DEFOPEN:
		OnMenuCommands(0);
		break;
	case IDM_ACTIVATENAMETAB:
		m_pTabCtrl->SetCurSel(0);
		SetVisibleWindowInTab();
		break;
	case IDM_ACTIVATESIZETAB:
		m_pTabCtrl->SetCurSel(1);
		SetVisibleWindowInTab();
		break;
	case IDM_ACTIVATEADVANCEDTAB:
		m_pTabCtrl->SetCurSel(2);
		SetVisibleWindowInTab();
		break;
	case IDM_LOOKINNEWSELECTION:
	case IDM_LOOKINREMOVESELECTION:
	case IDM_LOOKINNEXTSELECTION:
	case IDM_LOOKINPREVSELECTION:
		return m_NameDlg.OnCommand(wID,wNotifyCode,hControl);
	default:
		// IDM_ should be in descending order
		if (wID>=IDM_DEFCOLSELITEM && wID<IDM_DEFCOLSELITEM+1000)
			m_pListCtrl->ColumnSelectionMenuProc(wID,IDM_DEFCOLSELITEM);
		else if (wID>=IDM_DEFSENDTOITEM && wID<IDM_DEFSENDTOITEM+1000)
			OnSendToCommand(wID);
		else if (wID>=IDM_DEFCONTEXTITEM && wID<IDM_DEFCONTEXTITEM+1000)
			OnMenuCommands(wID);
		else if (wID>=IDM_DEFUPDATEDBITEM && wID<IDM_DEFUPDATEDBITEM+1000)
			GetLocateApp()->OnDatabaseMenuItem(wID);
		break;
	}
	return CDialog::OnCommand(wID,wNotifyCode,hControl);
}

BOOL CLocateDlg::SetListStyle(int id,BOOL bInit)
{
	CMenu menu(GetMenu());
	DWORD dwStyle=m_pListCtrl->GetStyle();

	if ((dwStyle & LVS_TYPEMASK)==LVS_REPORT && id!=3)
	{
		m_pListCtrl->SaveColumnsState(HKCU,
			CString(IDS_REGPLACE,CommonResource)+"\\General","ListWidths");
	}

	switch(id)
	{
	case 0:
		if ((dwStyle & LVS_TYPEMASK)!=LVS_ICON || bInit)
		{
			menu.CheckMenuItem(IDM_LARGEICONS,MF_BYCOMMAND|MF_CHECKED);
			menu.CheckMenuItem(IDM_SMALLICONS,MF_BYCOMMAND|MF_UNCHECKED);
			menu.CheckMenuItem(IDM_LIST,MF_BYCOMMAND|MF_UNCHECKED);
			menu.CheckMenuItem(IDM_DETAILS,MF_BYCOMMAND|MF_UNCHECKED);
			menu.EnableMenuItem(IDM_LINEUPICONS,MF_BYCOMMAND|MF_ENABLED);
			menu.EnableMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_ENABLED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LARGEICONS,MF_BYCOMMAND|MF_CHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_SMALLICONS,MF_BYCOMMAND|MF_UNCHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LIST,MF_BYCOMMAND|MF_UNCHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_DETAILS,MF_BYCOMMAND|MF_UNCHECKED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LINEUPICONS,MF_BYCOMMAND|MF_ENABLED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_ENABLED);
			
			if (((CLocateApp*)GetApp())->m_wComCtrlVersion>=0x0600)
			{
				menu.EnableMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_ENABLED);
				EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_ENABLED);
			}
			
			m_pListCtrl->SetStyle((dwStyle & ~LVS_TYPEMASK)|LVS_ICON);
			m_pListCtrl->Arrange(LVA_DEFAULT);
		}
		break;
	case 1:
		if ((dwStyle & LVS_TYPEMASK)!=LVS_SMALLICON || bInit)
		{
			menu.CheckMenuItem(IDM_LARGEICONS,MF_BYCOMMAND|MF_UNCHECKED);
			menu.CheckMenuItem(IDM_SMALLICONS,MF_BYCOMMAND|MF_CHECKED);
			menu.CheckMenuItem(IDM_LIST,MF_BYCOMMAND|MF_UNCHECKED);
			menu.CheckMenuItem(IDM_DETAILS,MF_BYCOMMAND|MF_UNCHECKED);
			menu.EnableMenuItem(IDM_LINEUPICONS,MF_BYCOMMAND|MF_ENABLED);
			menu.EnableMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_ENABLED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LARGEICONS,MF_BYCOMMAND|MF_UNCHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_SMALLICONS,MF_BYCOMMAND|MF_CHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LIST,MF_BYCOMMAND|MF_UNCHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_DETAILS,MF_BYCOMMAND|MF_UNCHECKED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LINEUPICONS,MF_BYCOMMAND|MF_ENABLED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_ENABLED);

			if (((CLocateApp*)GetApp())->m_wComCtrlVersion>=0x0600)
			{
				menu.EnableMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_ENABLED);
				EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_ENABLED);
			}

			m_pListCtrl->SetStyle((dwStyle & ~LVS_TYPEMASK)|LVS_SMALLICON);
			m_pListCtrl->Arrange(LVA_DEFAULT);
		}
		break;
	case 2:
		if ((dwStyle  &LVS_TYPEMASK)!=LVS_LIST || bInit)
		{
			menu.CheckMenuItem(IDM_LARGEICONS,MF_BYCOMMAND|MF_UNCHECKED);
			menu.CheckMenuItem(IDM_SMALLICONS,MF_BYCOMMAND|MF_UNCHECKED);
			menu.CheckMenuItem(IDM_LIST,MF_BYCOMMAND|MF_CHECKED);
			menu.CheckMenuItem(IDM_DETAILS,MF_BYCOMMAND|MF_UNCHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LARGEICONS,MF_BYCOMMAND|MF_UNCHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_SMALLICONS,MF_BYCOMMAND|MF_UNCHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LIST,MF_BYCOMMAND|MF_CHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_DETAILS,MF_BYCOMMAND|MF_UNCHECKED);
			
			menu.EnableMenuItem(IDM_LINEUPICONS,MF_BYCOMMAND|MF_GRAYED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LINEUPICONS,MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_GRAYED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_GRAYED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_GRAYED);
			
			m_pListCtrl->SetStyle((dwStyle & ~LVS_TYPEMASK)|LVS_LIST);
			m_pListCtrl->Arrange(LVA_DEFAULT);
		}
		break;
	case 3:
		if ((dwStyle & LVS_TYPEMASK)!=LVS_REPORT || bInit)
		{
			menu.CheckMenuItem(IDM_LARGEICONS,MF_BYCOMMAND|MF_UNCHECKED);
			menu.CheckMenuItem(IDM_SMALLICONS,MF_BYCOMMAND|MF_UNCHECKED);
			menu.CheckMenuItem(IDM_LIST,MF_BYCOMMAND|MF_UNCHECKED);
			menu.CheckMenuItem(IDM_DETAILS,MF_BYCOMMAND|MF_CHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LARGEICONS,MF_BYCOMMAND|MF_UNCHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_SMALLICONS,MF_BYCOMMAND|MF_UNCHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LIST,MF_BYCOMMAND|MF_UNCHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_DETAILS,MF_BYCOMMAND|MF_CHECKED);

			menu.EnableMenuItem(IDM_LINEUPICONS,MF_BYCOMMAND|MF_GRAYED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LINEUPICONS,MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_GRAYED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_GRAYED);
			menu.EnableMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_GRAYED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_GRAYED);
			m_pListCtrl->SetStyle((dwStyle & ~LVS_TYPEMASK)|LVS_REPORT);
		}
		break;
	}
	return TRUE;
}

BOOL CLocateDlg::UpdateSettings()
{
	CRegKey RegKey;
	CString Path;
	DWORD temp=3;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\General";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		//Dialog flags
		temp=m_dwFlags;
		RegKey.QueryValue("Program Status",temp);
        m_dwFlags&=~fgSave;
		m_dwFlags|=temp&fgSave;

		//Extra flags
		temp=m_dwExtraFlags;
		RegKey.QueryValue("Program StatusExtra",temp);
        m_dwExtraFlags&=~efSave;
		m_dwExtraFlags|=temp&efSave;
	}
	m_pListCtrl->RedrawItems(0,m_pListCtrl->GetItemCount());
	SetListSelStyle();

	
    // Update tooltip setting	
	if (m_dwFlags&fgLVDontShowTooltips)
	{
		if (m_pListTooltips!=NULL)
		{
			//m_pListCtrl->SetToolTips(NULL);
			m_pListTooltips->DestroyToolTipCtrl();
			delete m_pListTooltips;
			m_pListTooltips=NULL;
		}
	}
	else if (m_pListTooltips==NULL)
	{
		m_pListTooltips=new CToolTipCtrl;
		m_pListTooltips->Create(*this);
		m_pListTooltips->SetFont(m_pListCtrl->GetFont());
		
		m_iTooltipItem=-1;
		m_iTooltipSubItem=-1;
		m_bTooltipActive=FALSE;
	}

	if (m_NameDlg.EnableMultiDirectorySupport(GetFlags()&fgNameMultibleDirectories?TRUE:FALSE))
	{
		// This trick causes OnSize call
		WINDOWPLACEMENT wp;
		wp.length=sizeof(wp);
		m_NameDlg.GetWindowPlacement(&wp);
		wp.showCmd=m_pTabCtrl->GetCurSel()==0?SW_SHOWNORMAL:SW_HIDE;
		wp.rcNormalPosition.bottom++;
		m_NameDlg.SetWindowPlacement(&wp);
		wp.rcNormalPosition.bottom--;
		m_NameDlg.SetWindowPlacement(&wp);

	}
	
	m_NameDlg.InitDriveBox();

	SetDialogMode(GetFlags()&fgLargeMode);

	if ((GetExtraFlags()&efItemUpdatingMask)==efEnableItemUpdating)
	{
		if (m_pFileNotificationsThread!=NULL)
			m_pFileNotificationsThread->Stop();

        StartBackgroundOperations(); 
	}
	else
		StopBackgroundOperations();
	return TRUE;
}

void CLocateDlg::OnSelectAll()
{
	int nItem=m_pListCtrl->GetNextItem(-1,LVNI_ALL);
	while (nItem!=-1)
	{
		m_pListCtrl->SetItemState(nItem,LVIS_SELECTED,LVIS_SELECTED);
		nItem=m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
	}
	m_pListCtrl->SetFocus();
}

void CLocateDlg::OnInvertSelection()
{
	int nItem=m_pListCtrl->GetNextItem(-1,LVNI_ALL);
	while (nItem!=-1)
	{
		if (m_pListCtrl->GetItemState(nItem,LVIS_SELECTED))
			m_pListCtrl->SetItemState(nItem,0,LVIS_SELECTED);
		else
			m_pListCtrl->SetItemState(nItem,LVIS_SELECTED,LVIS_SELECTED);
		nItem=m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
	}
	m_pListCtrl->SetFocus();
}

void CLocateDlg::StartBackgroundOperations()
{
	//DebugMessage("CLocateDlg::StartBackgroundOperations():  BEGIN");
	if (m_pBackgroundUpdater==NULL)
		m_pBackgroundUpdater=new CBackgroundUpdater(m_pListCtrl);

	if ((GetExtraFlags()&efItemUpdatingMask)==efDisableItemUpdating)
		return;

	if (GetExtraFlags()&efEnableFSTracking)
	{
		if (m_pFileNotificationsThread==NULL)
		{
			m_pFileNotificationsThread=new CCheckFileNotificationsThread;
			//DebugFormatMessage("CLocateDlg::StartBackgroundOperations(): changentfr %X created",m_pFileNotificationsThread);
		}
		
		if (m_pListCtrl->GetItemCount()>0)
		{
			m_pFileNotificationsThread->Start();
			//m_pBackgroundUpdater->Start();
		}
	}
	else if (m_pFileNotificationsThread!=NULL)
		m_pFileNotificationsThread->CouldStop();

	//DebugMessage("CLocateDlg::StartBackgroundOperations():  END");
	
}

void CLocateDlg::StopBackgroundOperations()
{
	//DebugMessage("CLocateDlg::StopBackgroundOperations():  BEGIN");
	if (m_pLocater!=NULL)
		return;

	if (m_pFileNotificationsThread!=NULL)
		m_pFileNotificationsThread->CouldStop();
	if (m_pBackgroundUpdater!=NULL)
		m_pBackgroundUpdater->CouldStop();
	//DebugMessage("CLocateDlg::StopBackgroundOperations():  END");
}

void CLocateDlg::ChangeBackgroundOperationsPriority(BOOL bLower)
{
	DebugFormatMessage("CLocateDlg::ChangeBackgroundOperationsPriority(bLower=%d) B=%X F=%X:  BEGIN",bLower,m_pBackgroundUpdater,m_pFileNotificationsThread);
	
	int nPriority=bLower?THREAD_PRIORITY_LOWEST:THREAD_PRIORITY_BELOW_NORMAL;

	if (m_pBackgroundUpdater!=NULL)
		SetThreadPriority(m_pBackgroundUpdater->m_hThread,nPriority);
	
	if (m_pFileNotificationsThread!=NULL)
		SetThreadPriority(m_pFileNotificationsThread->m_hThread,nPriority);


	DebugMessage("CLocateDlg::ChangeBackgroundOperationsPriority():  END");
}

void CLocateDlg::SetDialogMode(BOOL bLarge)
{
	if (bLarge || GetFlags()&fgDialogLargeModeOnly)
	{
		if (!(m_dwFlags&fgLargeMode))
		{
			m_dwFlags|=fgLargeMode;
			m_pListCtrl->SetStyle(m_pListCtrl->GetStyle()|WS_TABSTOP);
				
			WINDOWPLACEMENT wp;
			wp.length=sizeof(WINDOWPLACEMENT);
			CRect rect;
			GetWindowRect(&rect);
			GetWindowPlacement(&wp);
			
			if (wp.showCmd==SW_MAXIMIZE)
			{
				wp.rcNormalPosition.bottom=wp.rcNormalPosition.top+m_nLargeY;
				SetWindowPlacement(&wp);
				SetWindowPos(HWND_BOTTOM,0,0,rect.Width(),m_nMaxYMaximized,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
			}
			else
				SetWindowPos(HWND_BOTTOM,0,0,rect.Width(),m_nLargeY,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		
			ShowDlgItem(IDC_FILELIST,swShow);
			ShowDlgItem(IDC_STATUS,swShow);

			// Set arrows of listbox header
			if (m_nSorting!=BYTE(-1))
				SetSortArrowToHeader(DetailType(m_nSorting&127),FALSE,m_nSorting&128);
		}
	}
	else
	{
		if (m_dwFlags&fgLargeMode)
		{
			SetSortArrowToHeader(DetailType(m_nSorting&127),TRUE,FALSE);
			Sleep(10);
			m_nSorting=BYTE(-1);

			CRect rect;
			m_dwFlags&=~fgLargeMode;
			m_pListCtrl->SetStyle(m_pListCtrl->GetStyle()&~WS_TABSTOP);
			GetWindowRect(&rect);
			m_nLargeY=rect.Height();
			SetWindowPos(HWND_BOTTOM,0,0,rect.Width(),m_nMaxYMinimized,
				SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);

			// Now, terminate available background updater
			m_pListCtrl->DeleteAllItems();

			ISDLGTHREADOK
			if (g_szBuffer!=NULL)
			{
				delete[] g_szBuffer;
				g_szBuffer=NULL;
			}
			m_pListCtrl->ShowWindow(CWndCtrl::swHide);
		}
	}
}

void CLocateDlg::DeleteTooltipTools()
{
	m_pListTooltips->Pop();

	m_iTooltipItem=-1;
	m_iTooltipSubItem=-1;
	
	TOOLINFO ti;
	ti.cbSize=TTTOOLINFOA_V2_SIZE;
	for (int i=m_pListTooltips->GetToolCount()-1;m_pListTooltips->EnumTools(i,&ti) && i>=0;i--)
		m_pListTooltips->DelTool(&ti);
	
	/*
	if (m_pListTooltips->GetToolCount()>0)
	{
		for (int i=0;i<m_pListTooltips->GetToolCount();i++)
		{
			BOOL bRet=m_pListTooltips->EnumTools(i,&ti);
			DebugFormatMessage("bRet=%X, ti.hwnd=%X ti.uID=%X ti.uFlags=%X",bRet,ti.hwnd,ti.uId,ti.uFlags);
		}
	}
	*/
}

void CLocateDlg::OnOk(BOOL bSelectDatabases)
{
	DlgDebugMessage("CLocateDlg::OnOk BEGIN");
	
	
	CWaitCursor wait;
	CString Name,Title;
	CArrayFAP<LPSTR> aExtensions,aDirectories;
	DWORD nRet;

	if (m_pBackgroundUpdater!=NULL)
		m_pBackgroundUpdater->Stop();
	if (m_pFileNotificationsThread!=NULL)
		m_pFileNotificationsThread->Stop();
	if (m_pLocater!=NULL)
		m_pLocater->StopLocating();
	
	// Deleting previous items
	m_pListCtrl->DeleteAllItems();

	if (m_pListTooltips!=NULL)
		DeleteTooltipTools();

	if (GetKeyState(VK_CONTROL)&0x8000)
		bSelectDatabases=TRUE;


	// If dialog is not large mode, change it
	SetDialogMode(TRUE);
	// Clearing possible exclamation icons
	m_pStatusCtrl->SetText(0,2,0);
	m_pStatusCtrl->SetText(0,3,0);
	
	
	// Resolving Name and Type
	if (!m_NameDlg.OnOk(Name,aExtensions,aDirectories))
	{
		DlgDebugMessage("CLocateDlg::OnOk END_1");
		return;
	}
	


	// Loading databases
	CArray<PDATABASE>* pDatabases;
	if (bSelectDatabases)
	{
		pDatabases=new CArray<PDATABASE>;
		
		CSelectDatabasesDlg dbd(GetLocateApp()->GetDatabases(),*pDatabases,
			GetLocateApp()->GetStartupFlags()&CLocateApp::CStartData::startupDatabasesOverridden?CSelectDatabasesDlg::flagDisablePresets:0,
			CString(IDS_REGPLACE,CommonResource)+"\\Dialogs\\SelectDatabases/Locate");
		if (!dbd.DoModal(*this))
		{
			delete pDatabases;
			return;
		}

	}
	else
	{
		pDatabases=GetLocateApp()->GetDatabasesPtr();
		if (pDatabases->GetSize()==0)
		{
			ShowErrorMessage(IDS_ERRORCNODATABASESSELECTED,IDS_ERROR);
			return;
		}
	}

	m_pLocater=new CLocater(*pDatabases);

	// Calling routines for subdialogs
	m_SizeDateDlg.OnOk(m_pLocater);
	nRet=m_AdvancedDlg.OnOk(m_pLocater);
	
	// Checking name, inserting asterisks etc..
	if (!Name.IsEmpty()) 
	{
		if (Name[0]==':') // Is regexp
		{
			Name.DelChar(0);
			nRet|=CAdvancedDlg::flagNameIsRegularExpression;
			if (Name[0]==':')
			{
				Name.DelChar(0);
				nRet|=CAdvancedDlg::flagNameIsRegularExpressionInPath;
			}
			else if (Name[0]==' ')
				Name.DelChar(0);
		}
		else
		{
			DWORD i=0;
			if (nRet&CAdvancedDlg::flagReplaceSpaces)
			{
				// Replacing spaces with asterisks
				for (i=0;i<Name.GetLength();i++)
				{
					if (Name[i]==' ')
						Name[i]='*';
				}
			}
			
			if (!(nRet&CAdvancedDlg::flagMatchCase))
			{
				// Insert asterisks to begin and end of name
				if (Name[0]!='*')
					Name.InsChar(0,'*');
				if (Name.LastChar()!='*')
					Name << '*';
			}
			
			// Removing extra asterisks
			for (i=0;i<Name.GetLength();i++)
			{
				if (Name[i]=='*')
				{
					while (Name[i+1]=='*')
						Name.DelChar(i+1);
				}
			}
		}
	}
	
	// Extension no extensions, checking if name contains extension
	if (aExtensions.GetSize()==0 && 
		(m_pLocater->GetAdvancedFlags()&(LOCATE_FILENAMES|LOCATE_FOLDERNAMES))!=LOCATE_FOLDERNAMES)
		m_pLocater->AddAdvancedFlags(LOCATE_EXTENSIONWITHNAME);

	// Set funtion pointers
	m_pLocater->SetFunctions(LocateProc,LocateFoundProc,DWORD(this));
	
	// This is not needed anymore
	if (bSelectDatabases)
		delete pDatabases; 


	// Making title
	Title.LoadString(IDS_TITLE);
	if (Name.IsEmpty())
	{
		if (aExtensions.GetSize()==1)
		{
			Title.AddString(IDS_FILESNAMED);
			Title << "*." << aExtensions[0];
		}
		else
			Title.AddString(IDS_ALLFILES);
	}
	else if (nRet&CAdvancedDlg::flagNameIsRegularExpression)
	{
		if (nRet&CAdvancedDlg::flagNameIsRegularExpressionInPath)
			Title.AddString(IDS_REGULAREXPRESSIONFULLPATH);
		else
			Title.AddString(IDS_REGULAREXPRESSION);
		Title << Name;
	}
	else
	{
		Title.AddString(IDS_FILESNAMED);
		Title << Name;
		if (aExtensions.GetSize()==1)
			Title << "." << aExtensions[0];
	}
	SetText(Title);


	// LocateFoundProc uses UpdateList
	StartBackgroundOperations();

	// Starting location
	if (!(nRet&CAdvancedDlg::flagNameIsRegularExpression))
	{
		m_pLocater->LocateFiles(TRUE,Name,
			(LPCSTR*)aExtensions.GetData(),aExtensions.GetSize(),
			(LPCSTR*)aDirectories.GetData(),aDirectories.GetSize());
	}
	else
	{
		m_pLocater->LocateFiles(TRUE,Name,(nRet&CAdvancedDlg::flagNameIsRegularExpressionInPath)?TRUE:FALSE,
			(LPCSTR*)aDirectories.GetData(),aDirectories.GetSize());
	}

	DlgDebugMessage("CLocateDlg::OnOk END");
	
}

BOOL CLocateDlg::LocateProc(DWORD dwParam,CallingReason crReason,UpdateError ueCode,DWORD dwFoundFiles,const CLocater* pLocater)
{
	DbcDebugFormatMessage2("CLocateDlg::LocateProc BEGIN, reason=%d, code=%d",crReason,ueCode);
	
	switch (crReason)
	{
	case Initializing:
	{
		if (ueCode!=ueStillWorking && ueCode!=ueSuccess) // Initializing failed
		{
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(CString(IDS_LOCATINGFAILED),1,0);
			return FALSE;
		}

		// Disabling items
		((CLocateDlg*)dwParam)->DisableItems();
		
		((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(szEmpty,0,0);
		((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(CString(IDS_LOCATING),1,0);
		((CLocateDlg*)dwParam)->StartLocateAnimation();

		// Selecting path column
		((CLocateDlg*)dwParam)->m_pListCtrl->SendMessage(LVM_FIRST+140/* LVM_SETSELECTEDCOLUMN */,1,0);
		return TRUE;
	}
	case FinishedLocating:
	{
		((CLocateDlg*)dwParam)->StopLocateAnimation();
		((CLocateDlg*)dwParam)->EnableItems();
		

		if (ueCode==ueStopped)
		{
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(CString(IDS_LOCATINGCANCELLED),1,0);
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(LPCSTR(::LoadIcon(NULL,IDI_WARNING)),2,SBT_OWNERDRAW);
		}
		else if (ueCode==ueLimitReached)
		{
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(CString(IDS_LOCATINGLIMITREACHED),1,0);
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(LPCSTR(::LoadIcon(NULL,IDI_INFORMATION)),2,SBT_OWNERDRAW);
		}
		else if (ueCode!=ueStillWorking && ueCode!=ueSuccess) // Locating failed
		{
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(CString(IDS_LOCATINGFAILED),1,0);
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(LPCSTR(::LoadIcon(NULL,IDI_ERROR)),2,SBT_OWNERDRAW);
		}
		else
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(CString(IDS_LOCATINGSUCCESS),1,0);
		

		::InvalidateRect(*((CLocateDlg*)dwParam)->m_pStatusCtrl,NULL,TRUE);

		CString text;
		text.Format(IDS_FILESFOUND,dwFoundFiles);
		((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(text,0,0);

		((CLocateDlg*)dwParam)->CheckClipboard();
		return TRUE;
	}
	case SearchingStarted:
	{
		CString text;
		text.Format(IDS_SEARCHINGFROMFILE,pLocater->GetFileName());
		((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(text,0,0);
		break;
	}
	case SearchingEnded:
		((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(szEmpty,0,0);
		break;
	case ClassShouldDelete:
		delete pLocater;
		((CLocateDlg*)dwParam)->m_pLocater=NULL;
		
		if (((CLocateDlg*)dwParam)->m_pBackgroundUpdater!=NULL)
			((CLocateDlg*)dwParam)->StartBackgroundOperations();
		
		((CLocateDlg*)dwParam)->m_pBackgroundUpdater->StopWaiting();
		
		// To update items, looks like this is only way
		((CLocateDlg*)dwParam)->SetTimer(ID_REDRAWITEMS,50);
		return TRUE;
	case ErrorOccured:
		switch (ueCode)
		{
		case ueUnknown:
			((CLocateDlg*)dwParam)->ShowErrorMessage(IDS_ERRORUNKNOWN,IDS_ERROR);
			return FALSE;
		case ueOpen:
		case ueCreate:
			{
				CString str;
				str.Format(IDS_ERRORCANNOTOPENDB,pLocater->GetCurrentDatabaseFile());
				((CLocateDlg*)dwParam)->MessageBox(str,CString(IDS_ERROR),MB_ICONERROR|MB_OK);
				return FALSE;
			}
		case ueRead:
			{
				CString str;
				str.Format(IDS_ERRORCANNOTREADDB,pLocater->GetCurrentDatabaseFile());
				((CLocateDlg*)dwParam)->MessageBox(str,CString(IDS_ERROR),MB_ICONERROR|MB_OK);
				return FALSE;
			}
		case ueAlloc:
			((CLocateDlg*)dwParam)->ShowErrorMessage(IDS_ERRORCANNOTALLOCATE,IDS_ERROR);
			break;
		case ueInvalidDatabase:
			{
				CString str;
				str.Format(IDS_ERRORINVALIDDB,pLocater->GetCurrentDatabaseFile());
				((CLocateDlg*)dwParam)->MessageBox(str,CString(IDS_ERROR),MB_ICONERROR|MB_OK);
				return FALSE;
			}
		}
		break;
	}
	
	DbcDebugMessage("CLocateDlg::LocateProc END");
	return TRUE;
}

BOOL CALLBACK CLocateDlg::LocateFoundProc(DWORD dwParam,BOOL bFolder,const CLocater* pLocater)
{
	DbcDebugMessage("CLocateDlg::LocateFoundProc BEGIN");
	
	// Hide system and hidden files if it's wanted
	if (((CLocateDlg*)dwParam)->GetFlags()&fgLVDontShowHiddenFiles)
	{
		if (bFolder)
		{
			if (pLocater->GetFolderAttributes()&(LITEMATTRIB_HIDDEN|LITEMATTRIB_SYSTEM))
				return TRUE;
		}
		else if (pLocater->GetFileAttributes()&(LITEMATTRIB_HIDDEN|LITEMATTRIB_SYSTEM))
			return TRUE;
	}

	// Check wheter item is already added
	if (((CLocateDlg*)dwParam)->GetFlags()&fgLVNoDoubleItems)
	{
		// Setting path, name and extension
		char szPath[MAX_PATH+2],szPath2[MAX_PATH+2];

		if (bFolder)
		{
			DWORD nPathLen=pLocater->GetCurrentPathLen();

			ASSERT(pLocater->GetCurrentPathLen()+pLocater->GetFolderNameLen()<MAX_PATH-1);

			sMemCopy(szPath,pLocater->GetCurrentPath(),nPathLen);
			szPath[nPathLen++]='\\';
			sMemCopy(szPath+nPathLen,pLocater->GetFolderName(),pLocater->GetFolderNameLen()+1);
		}
		else
		{
			DWORD nPathLen=pLocater->GetCurrentPathLen();

			ASSERT(pLocater->GetCurrentPathLen()+pLocater->GetFileNameLen()<MAX_PATH-1);

			sMemCopy(szPath,pLocater->GetCurrentPath(),nPathLen);
			szPath[nPathLen++]='\\';
			sMemCopy(szPath+nPathLen,pLocater->GetFileName(),pLocater->GetFileNameLen()+1);
		}
		CharLower(szPath);

		
		CListCtrl* pList=((CLocateDlg*)dwParam)->m_pListCtrl;
		int nItem=pList->GetNextItem(-1,LVNI_ALL);
		while (nItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)pList->GetItemData(nItem);
			if (pItem!=NULL)
			{
				ASSERT(pItem->GetPathLen()<MAX_PATH);

				sMemCopy(szPath2,pItem->GetPath(),pItem->GetPathLen()+1);
				CharLower(szPath2);
			
				
				if (strcmp(szPath,szPath2)==0)
					return TRUE; // Alreafy found
				
			}
			nItem=pList->GetNextItem(nItem,LVNI_ALL);
		}
		

	}

	LV_ITEM li;
	li.mask=LVIF_TEXT|LVIF_STATE|LVIF_IMAGE|LVIF_PARAM;
	li.iSubItem=0;
	li.iImage=I_IMAGECALLBACK;
	li.lParam=(LPARAM)new CLocatedItem(bFolder,pLocater);
	if (pLocater->IsCurrentDatabaseNamesOEM())
		((CLocatedItem*)li.lParam)->OemtoAnsi();
	if (li.lParam==NULL)
		return FALSE;
	li.pszText=LPSTR_TEXTCALLBACK;

	// To prevent drawing error
	if (pLocater->GetFoundFiles()%60==59)
		Sleep(20);
	else if (pLocater->GetFoundFiles()%30==29)
		Sleep(10);

	li.state=0;
	li.stateMask=0;

	if (((CLocateDlg*)dwParam)->m_nSorting==BYTE(-1))
		li.iItem=((CLocateDlg*)dwParam)->m_pListCtrl->GetItemCount();
	else
	{
		li.iItem=SortNewItem(((CLocateDlg*)dwParam)->m_pListCtrl,
			(CLocatedItem*)li.lParam,((CLocateDlg*)dwParam)->m_nSorting);
	}

	((CLocateDlg*)dwParam)->m_pListCtrl->InsertItem(&li);

	DbcDebugMessage("CLocateDlg::LocateFoundProc END");
	return TRUE;
}


int CLocateDlg::SortNewItem(CListCtrl* pList,CLocatedItem* pNewItem,BYTE bSorting)
{
	int dwMaxItems=pList->GetItemCount();
	if (dwMaxItems==0)
		return 0;

	int a=0; // start index
	int b=dwMaxItems-1; // end idex
	int c=(a+b)/2;
	for (;;)
	{
		ASSERT(c>=0 && c<int(dwMaxItems));
		CLocatedItem* pItem=(CLocatedItem*)pList->GetItemData(c);
		if (pItem==NULL || DWORD(pItem)==DWORD(-1))
			::MessageBox(NULL,"CLocateDlg::SortNewItem:Something is wrong! Contact jmhuttun@venda.uku.fi",NULL,MB_OK);
		int nRet=ListViewCompareProc(LPARAM(pItem),LPARAM(pNewItem),bSorting);
		if (nRet<0) // New item should be later
		{
			a=c+1;
			c=(a+b)/2;

			if (a>b)
			{
				ASSERT(a>=0 && a<=int(dwMaxItems));
				return a;
			}
		}
		else if (nRet>0) // New item should be previous
		{
			b=c;
			c=(a+b)/2;
			if (a==b)
			{
				ASSERT(a>=0 && a<=int(dwMaxItems));
				return a;
			}
		}
		else
		{
			do
			{
				if (++c>=int(dwMaxItems))
					break;
				pItem=(CLocatedItem*)pList->GetItemData(c);
			}
			while (ListViewCompareProc(LPARAM(pItem),LPARAM(pNewItem),bSorting)==0);
			
			ASSERT(c>=0 && c<=int(dwMaxItems));
			return c;
		}
	}
}
	
void CLocateDlg::OnStop()
{
	CWaitCursor wait;
	StartBackgroundOperations();
	if (m_pBackgroundUpdater!=NULL)
		m_pBackgroundUpdater->StopWaiting();

	if (m_pLocater!=NULL)
		m_pLocater->StopLocating();
}

void CLocateDlg::OnNewSearch()
{
	CWaitCursor wait;
	// Backgorund operation should end
	StopBackgroundOperations();
		
	// StopBackgroundOperations uses only CouldStop
	if (m_pBackgroundUpdater!=NULL)
		m_pBackgroundUpdater->Stop();
	
	if (m_pLocater!=NULL)
		m_pLocater->StopLocating();

	m_pListCtrl->DeleteAllItems();
	if (m_pListTooltips!=NULL)
		DeleteTooltipTools();
	SetDialogMode(FALSE);

		
	m_NameDlg.OnClear();
	m_SizeDateDlg.OnClear();
	m_AdvancedDlg.OnClear();
	if (m_pTabCtrl->GetCurSel()!=0)
	{
		m_AdvancedDlg.ShowWindow(swHide);
		m_SizeDateDlg.ShowWindow(swHide);
		m_NameDlg.ShowWindow(swShow);
		m_pTabCtrl->SetCurSel(0);
	}
	CString title,temp;
	title.LoadString(IDS_TITLE);
	temp.LoadString(IDS_ALLFILES);
	title << temp;
	SetText(title);

	m_NameDlg.SetFocus(IDC_NAME);
}

BOOL CLocateDlg::OnClose()
{
	DebugMessage("CLocateDlg::OnClose");
	CDialog::OnClose();
	DestroyWindow();
	return 1;
}

void CLocateDlg::OnDestroy()
{
	DebugMessage("CLocateDlg::OnDestroy() BEGIN");
	
	CDialog::OnDestroy();
	
	if (m_pLocater!=NULL)
	{
		m_pLocater->StopLocating();

		DebugNumMessage("CLocateDlg::OnDestroy(): m_pLocater=%X",(DWORD)m_pLocater);
	}

	StopUpdateAnimation();
	
	if (m_pFileNotificationsThread!=NULL)
		m_pFileNotificationsThread->Stop();
	if (m_pBackgroundUpdater!=NULL)
		m_pBackgroundUpdater->Stop();

	ChangeClipboardChain(*this,m_hNextClipboardViewer);
	ClearMenuVariables();
	
	SaveDialogTexts();
	m_NameDlg.DestroyWindow();
	m_SizeDateDlg.DestroyWindow();
	m_AdvancedDlg.DestroyWindow();
	
	if (m_pTabCtrl!=NULL)
	{
		delete m_pTabCtrl;
		m_pTabCtrl=NULL;
	}
	if (m_pStatusCtrl!=NULL)
	{
		delete m_pStatusCtrl;
		m_pStatusCtrl=NULL;
	}

	// Freeing target paths in dwItemData
	FreeSendToMenuItems(GetSubMenu(GetMenu(),0));
	m_Menu.DestroyMenu();

	if (m_pListCtrl!=NULL)
	{
		if ((m_pListCtrl->GetStyle() & LVS_TYPEMASK)==LVS_REPORT)
			m_pListCtrl->SaveColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General","ListWidths");
		delete m_pListCtrl;
		m_pListCtrl=NULL;
	}
	if (m_pListTooltips!=NULL)
	{
		m_pListTooltips->DestroyToolTipCtrl();
		delete m_pListTooltips;
		m_pListTooltips=NULL;
	}

	SaveRegistry();
	ISDLGTHREADOK
	if (g_szBuffer!=NULL)
	{
		delete[] g_szBuffer;
		g_szBuffer=NULL;
	}
	if (m_hSendToListFont!=NULL)
	{
		DeleteObject(m_hSendToListFont);
		m_hSendToListFont=NULL;
	}

	if (m_pImageHandler!=NULL)
	{
		delete m_pImageHandler;
		m_pImageHandler=NULL;
	}

	if (!(m_dwFlags&fgDialogLeaveLocateBackground))
		GetLocateAppWnd()->PostMessage(WM_COMMAND,MAKEWPARAM(IDC_COMEWITHME,0));
	PostQuitMessage(0);

	DebugMessage("CLocateDlg::OnDestroy() END");
}

void CLocateDlg::ResetFileNotificators()
{
	DebugMessage("CLocateDlg::ResetFileNotificators() BEGIN");
		
	if (m_pFileNotificationsThread!=NULL)
		m_pFileNotificationsThread->Stop();
	if (GetFlags()&fgLargeMode)
	{
		m_pFileNotificationsThread=new CCheckFileNotificationsThread;
		m_pFileNotificationsThread->Start();
	}
	else
		m_pFileNotificationsThread=NULL;

	DebugMessage("CLocateDlg::ResetFileNotificators() END");

}

void CLocateDlg::OnTimer(DWORD wTimerID)
{
	// Somehow this hangs program (?)
	//CDialog::OnTimer(wTimerID);
	
	switch (wTimerID)
	{
	case ID_REDRAWITEMS:
		KillTimer(ID_REDRAWITEMS);
		m_pListCtrl->InvalidateRect(NULL,FALSE);
		m_pListCtrl->UpdateWindow();
		break;
	case ID_CLICKWAIT:
		KillTimer(ID_CLICKWAIT);
		m_ClickWait=FALSE;
		break;
	case ID_LOCATEANIM:
		m_nCurLocateAnimBitmap++;
		if (m_nCurLocateAnimBitmap>5)
			m_nCurLocateAnimBitmap=0;
		if (m_pStatusCtrl!=NULL && m_pLocateAnimBitmaps!=NULL)
			m_pStatusCtrl->SetText((LPCSTR)m_pLocateAnimBitmaps[m_nCurLocateAnimBitmap],2,SBT_OWNERDRAW);
		break;
	case ID_UPDATEANIM:
		m_nCurUpdateAnimBitmap++;
		if (m_nCurUpdateAnimBitmap>12)
			m_nCurUpdateAnimBitmap=0;
		if (m_pStatusCtrl!=NULL && m_pUpdateAnimBitmaps!=NULL)
			m_pStatusCtrl->SetText((LPCSTR)m_pUpdateAnimBitmaps[m_nCurUpdateAnimBitmap],3,SBT_OWNERDRAW);
		break;
	}
}

inline void CLocateDlg::SetControlPositions(UINT nType,int cx, int cy)
{
	CRect rc;
	// Frame line
	::SetWindowPos(GetDlgItem(IDC_FRAME),HWND_BOTTOM,0,0,cx,2,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	
	// Buttons
	::SetWindowPos(GetDlgItem(IDC_OK),HWND_BOTTOM,cx-m_nButtonWidth-7,28,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
	::SetWindowPos(GetDlgItem(IDC_STOP),HWND_BOTTOM,cx-m_nButtonWidth-7,28+m_nButtonSpacing,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
	::SetWindowPos(GetDlgItem(IDC_NEWSEARCH),HWND_BOTTOM,cx-m_nButtonWidth-7,28+m_nButtonSpacing*2,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
	::SetWindowPos(GetDlgItem(IDC_PRESETS),HWND_BOTTOM,cx-m_nPresetsButtonWidth-8,35+m_nTabbedDialogHeight-int(m_nPresetsButtonHeight),0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
	
	
	
	CRect rect(0,0,10,m_nTabbedDialogHeight);
	m_pTabCtrl->AdjustRect(TRUE,&rect);
	m_pTabCtrl->SetWindowPos(HWND_BOTTOM,0,0,cx-m_nButtonWidth-30,m_nTabbedDialogHeight+m_nTabHeaderHeight,SWP_NOACTIVATE|SWP_NOMOVE);
	
	GetWindowRect(&rect);
	m_pTabCtrl->GetWindowRect(&rc);
	


	m_NameDlg.SetWindowPos(HWND_TOP,0,0,cx-m_nButtonWidth-37,m_nTabbedDialogHeight,SWP_NOACTIVATE|SWP_NOMOVE);
	m_SizeDateDlg.SetWindowPos(HWND_TOP,0,0,cx-m_nButtonWidth-37,m_nTabbedDialogHeight,SWP_NOACTIVATE|SWP_NOMOVE);
	m_AdvancedDlg.SetWindowPos(HWND_TOP,0,0,cx-m_nButtonWidth-37,m_nTabbedDialogHeight,SWP_NOACTIVATE|SWP_NOMOVE);
	
	
	if (m_dwFlags&fgLargeMode)
	{
		int parts[4];
		if (nType&SIZE_MAXIMIZED)
		{
			parts[0]=(cx-44)/2;
			parts[1]=cx-44;
			parts[2]=cx-22;
			parts[3]=cx;
		}
		else
		{
			if (((CLocateApp*)GetApp())->m_wComCtrlVersion<0x0600)
			{
				parts[0]=(cx-62)/2;
				parts[1]=cx-62;
				parts[2]=cx-40;
				parts[3]=cx-18;
			}
			else // Using theme
			{
				parts[0]=(cx-72)/2;
				parts[1]=cx-72;
				parts[2]=cx-47;
				parts[3]=-1;
			}
		}
		m_pStatusCtrl->SetWindowPos(HWND_BOTTOM,0,cy-20,cx,20,SWP_NOACTIVATE|SWP_NOZORDER);
		RECT rect,rect2;
		m_pStatusCtrl->GetWindowRect(&rect);
		m_pListCtrl->GetWindowRect(&rect2);
		m_pListCtrl->SetWindowPos(HWND_BOTTOM,0,0,cx-5,rect.top-rect2.top,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		m_pListCtrl->UpdateWindow();
		m_pStatusCtrl->SetParts(4,parts);
	}
}
	
void CLocateDlg::OnSize(UINT nType, int cx, int cy)
{
	//DebugFormatMessage("CLocateDlg::OnSize(%d,%d,%d) BEGIN",nType,cx,cy);
	
	CDialog::OnSize(nType,cx,cy);
	switch (nType)
	{
	case SIZE_MINIMIZED:
		// Freeing not used memory
		ISDLGTHREADOK
		//StopBackgroundOperations();
		ChangeBackgroundOperationsPriority(TRUE);

		if (g_szBuffer!=NULL)
		{
			delete[] g_szBuffer;
			g_szBuffer=NULL;
		}
		if (m_hSendToListFont!=NULL)
		{
			DeleteObject(m_hSendToListFont);
			m_hSendToListFont=NULL;
		}

		// Minimizing to system tray
		if (m_dwFlags&fgDialogMinimizeToST)
			ShowWindow(swHide);
		break;
	case SIZE_RESTORED:
		if (m_dwFlags&fgLargeMode)
		{
			RECT rect;
			GetWindowRect(&rect);
			m_nLargeY=rect.bottom-rect.top;
		}
	case SIZE_MAXIMIZED:
		if (m_dwFlags&fgLargeMode)
		{
			if (m_pListCtrl->GetItemCount()>0)
			{
				ChangeBackgroundOperationsPriority(FALSE);
				//StartBackgroundOperations();
				m_pBackgroundUpdater->StopWaiting();
			}
		}
		SetControlPositions(nType,cx,cy);
		break;
	default:
		// Cannot happen?
		ASSERT(0);
		break;
	}

	//DebugMessage("CLocateDlg::OnSize END");
}

// Window showing background statistics
#ifdef _DEBUG
LRESULT CALLBACK CLocateDlg::DebugWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		::CreateWindow("EDIT","",WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_MULTILINE,0,0,100,100,
			hwnd,(HMENU)100,GetInstanceHandle(),NULL);
		::SetTimer(hwnd,1,100,NULL);
		return 0;
	case WM_SIZE:
		if (wParam==SIZE_RESTORED || wParam==0)
		{
			HWND hEdit=::GetDlgItem(hwnd,100);
			::SetWindowPos(hEdit,NULL,0,0,LOWORD(lParam),HIWORD(lParam),SWP_NOZORDER);

		}
		break;
	case WM_CLOSE:
		::KillTimer(hwnd,1);
		::DestroyWindow(hwnd);
		break;
	case WM_TIMER:
	{
		CString str;
		CLocateDlg* pDlg=GetLocateDlg();
		if (pDlg->m_pBackgroundUpdater!=NULL)
		{
			str << "Background updated running, isWaiting=" << (int) pDlg->m_pBackgroundUpdater->IsWaiting();
			str << " items=" << (DWORD) pDlg->m_pBackgroundUpdater->GetUpdateListSize();
			str.SetBase(16);
			str << " hThread=0x" << (DWORD) pDlg->m_pBackgroundUpdater->m_hThread;
		}
		else
			str << "Background updater is not running";
		
		if (pDlg->m_pFileNotificationsThread!=NULL)
		{
			str << "\r\nFileNotfications is running";
			if (pDlg->m_pFileNotificationsThread->m_pReadDirectoryChangesW==NULL)
				str << " using old method";
			str << " hThread=0x" << (DWORD) pDlg->m_pFileNotificationsThread->m_hThread;
		}
		else
			str << "\r\nFileNotifications is not running";

		::SetDlgItemText(hwnd,100,LPCSTR(str));
		break;
	}
	};
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}


#endif

BOOL CLocateDlg::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
	case WM_REFRESHNOTIFIERHANDLERS:
		ResetFileNotificators();
		break;
	case WM_UPDATENEEDEDDETAILTS:
		if (m_pBackgroundUpdater!=NULL)
		{
			m_pBackgroundUpdater->AddToUpdateList((CLocatedItem*)lParam,int(wParam),CLocateDlg::Needed);
			m_pBackgroundUpdater->StopWaiting();
		}
		break;
	case WM_GETMINMAXINFO:
		if (m_nMaxYMinimized==0)
			break;
		if (!(m_dwFlags&fgLargeMode))
		{
			if (m_nMaxYMaximized==0)
				m_nMaxYMaximized=WORD(((LPMINMAXINFO)lParam)->ptMaxTrackSize.y);
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.y=m_nMaxYMinimized;

			//((LPMINMAXINFO)lParam)->ptMinTrackSize.y=((LPMINMAXINFO)lParam)->ptMaxSize.y;
			//((LPMINMAXINFO)lParam)->ptMaxTrackSize.y=((LPMINMAXINFO)lParam)->ptMaxSize.y;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y=m_nMaxYMinimized;
		}
		else
		{
			RECT rc;
			m_pStatusCtrl->GetClientRect(&rc);
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y=m_nMaxYMinimized+rc.bottom-rc.top;
		}		
		return FALSE;
	case WM_EXITMENULOOP:
		if (m_hSendToListFont!=NULL)
		{
			DeleteObject(m_hSendToListFont);
			m_hSendToListFont=NULL;
		}
		break;
	case WM_SETTINGCHANGE:
		if (wParam==0x2a && lParam==NULL) // Possibly shell icon cache is updeted
		{
			int nItem=m_pListCtrl->GetNextItem(-1,LVNI_ALL);
			while (nItem!=-1)
			{
				CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);

				pItem->RemoveFlags(LITEM_ICONOK|LITEM_PARENTICONOK);
				
				nItem=m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
			}
			m_pListCtrl->RedrawItems(0,m_pListCtrl->GetItemCount()-1);
			m_pListCtrl->UpdateWindow();
		}
		SetListSelStyle();
		if (m_pBackgroundUpdater!=NULL)
			m_pBackgroundUpdater->StopWaiting();
		break;
	case WM_DRAWCLIPBOARD:
		return CDialog::WindowProc(msg,wParam,lParam);
	case WM_INITMENUPOPUP:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (m_pActiveContextMenu!=NULL)
		{
			IContextMenu2* pContextMenu2;
			HRESULT hRes=m_pActiveContextMenu->QueryInterface(IID_IContextMenu2,(void**)&pContextMenu2);
			if (SUCCEEDED(hRes))
			{
				hRes=pContextMenu2->HandleMenuMsg(msg,wParam,lParam);
				pContextMenu2->Release();
				if (hRes==NOERROR)
					return TRUE;
			}
		}
		break;
	case WM_MENUCHAR:
		if (m_pActiveContextMenu!=NULL)
		{
			IContextMenu3* pContextMenu3;
			HRESULT hRes=m_pActiveContextMenu->QueryInterface(IID_IContextMenu3,(void**)&pContextMenu3);
			if (SUCCEEDED(hRes))
			{
				HRESULT hRet;
				hRes=pContextMenu3->HandleMenuMsg2(msg,wParam,lParam,&hRet);
				pContextMenu3->Release();
				if (hRes==NOERROR)
					return hRet;
			}
		}
		break;
	case WM_GETICON:
	case WM_SETICON:
		DefWindowProc(*this,msg,wParam,lParam);
		break;
/*	case WM_LBUTTONDOWN: 
	case WM_LBUTTONUP: 
	case WM_MBUTTONDOWN: 
	case WM_MBUTTONUP: 
	case WM_MOUSEMOVE: 
	case WM_RBUTTONDOWN: 
	case WM_RBUTTONUP:
		if (m_pListTooltips!=NULL)
		{
			//m_pListTooltips->RelayEvent((MSG*)GetLocateAppWnd()->m_pLocateDlgThread->GetCurrentMessage());
			DebugMessage("CLocateDlg::WindowProc: message relayed to tooltip control");
		}
		break;*/
#ifdef _DEBUG
	case WM_SYSCOMMAND:
		if (wParam==0x76)
		{
			CInputDialog ib(IDD_INPUTBOX);
			ib.SetText("Insert HEX string");
			ib.SetTitle("Show schedule");
			ib.SetOKButtonText("OK");
			ib.SetCancelButtonText("Cancel");
			if (ib.DoModal(*this))
			{
				CString text;
				ib.GetInputText(text);
				DWORD dwLength;
				BYTE* pData=dataparser2(text,&dwLength);
				BYTE* pPtr=pData+6;
       			if (pData[1]==1)
				{
					if (dwLength>=6 && pData[0]==sizeof(CSchedule) && 
						dwLength==6+sizeof(CSchedule)*(*(DWORD*)(pData+2)))
					{
						for (DWORD n=0;n<*(DWORD*)(pData+2);n++)
						{
							CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg sud;
							sud.m_pSchedule=new CSchedule(pPtr,1);
							sud.DoModal(*this);
							delete sud.m_pSchedule;
						}
					}		
				}
				else if (pData[1]==2)
				{
					if (dwLength>=6 && pData[0]==sizeof(CSchedule))
					{
						BYTE* pPtr=pData+6;
						for (DWORD n=0;n<*(DWORD*)(pData+2);n++)
						{
							if (pPtr+sizeof(CSchedule)>=pData+dwLength)
								break;

							CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg sud;
							sud.m_pSchedule=new CSchedule(pPtr,2);
							sud.DoModal(*this);
							delete sud.m_pSchedule;
						}
					}
				}
				delete[] pData;
			}
		}
		else if (wParam==0x77)
		{
			WNDCLASSEX wce;
			ZeroMemory(&wce,sizeof(WNDCLASSEX));
			wce.cbSize=sizeof(wce);
			wce.style=CS_HREDRAW|CS_VREDRAW;
			wce.lpfnWndProc=DebugWindowProc;
			wce.hInstance=GetInstanceHandle();
			wce.hbrBackground=(HBRUSH)COLOR_WINDOW;
			wce.lpszClassName="DEBUGWINDOWCLASS";
			RegisterClassEx(&wce);
			
			SetLastError(0);
			if (CreateWindow("DEBUGWINDOWCLASS","DebugWindow",WS_OVERLAPPEDWINDOW|WS_VISIBLE,0,0,500,100,
				NULL,NULL,GetInstanceHandle(),NULL)==NULL)
				ReportSystemError();
            
		}
		break;
#endif
	}
	return CDialog::WindowProc(msg,wParam,lParam);
}

void CLocateDlg::OnMeasureItem(int nIDCtl,LPMEASUREITEMSTRUCT lpmis)
{
	CDialog::OnMeasureItem(nIDCtl,lpmis);
	
	if (nIDCtl==0)
	{
		// This is IContextMenu item
		if (m_pActiveContextMenu!=NULL && lpmis->itemID<IDM_DEFSENDTOITEM)
			return;
		
		char szTitle[_MAX_PATH];
		CDC dc(this);
		CSize sz;
		HGDIOBJ hOld=dc.SelectObject(m_hSendToListFont);
		GetFileTitle((LPCSTR)lpmis->itemData,szTitle,_MAX_PATH);
		GetTextExtentPoint32(dc,szTitle,strlen(szTitle),&sz);
		lpmis->itemWidth=40+sz.cx;
		if (sz.cy>16)
			lpmis->itemHeight=sz.cy+4;
		else
			lpmis->itemHeight=20;
		dc.SelectObject(hOld);
	}
}
	
void CLocateDlg::OnContextMenu(HWND hWnd,CPoint& pos)
{
	ClearMenuVariables();

	CRect rect;
	BOOL bFound=FALSE;
	int iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	CPoint pt(pos);
	
	//Checking whether mouse is in the list control's area
	m_pListCtrl->GetWindowRect(&rect);
	if (!rect.IsPtInRect(pt))
	{
		CDialog::OnContextMenu(hWnd,pos);
		return;
	}

	// Trying to find file item 
	m_pListCtrl->ScreenToClient(&pt);
	while (iItem!=-1)
	{
		m_pListCtrl->GetItemRect(iItem,&rect,LVIR_BOUNDS);
		if (rect.left<=pt.x && rect.right>=pt.y && rect.top<=pt.y && rect.bottom>=pt.y)
		{
			m_hActivePopupMenu=CreateFileContextMenu(NULL);
			TrackPopupMenu(m_hActivePopupMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,*this,NULL);	
			bFound=TRUE;
			break;
		}
		iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
	}
	if (!bFound)
	{
		HWND hHeader=m_pListCtrl->GetHeader();
		CRect rc;
		::GetWindowRect(hHeader,&rc);

		if (rc.IsPtInRect(pos))
		{
			HMENU hColMenu=m_pListCtrl->CreateColumnSelectionMenu(IDM_DEFCOLSELITEM);
			CString text(IDS_SELECTDETAILS);
			
			MENUITEMINFO mii;
			mii.cbSize=sizeof(MENUITEMINFO);
			// Inserting separator
			mii.fMask=MIIM_TYPE;
			mii.fType=MFT_SEPARATOR;
			InsertMenuItem(hColMenu,WORD(-1),FALSE,&mii);
			mii.fMask=MIIM_ID|MIIM_TYPE;
			mii.fType=MFT_STRING;
			mii.dwTypeData=text.GetBuffer();
			mii.wID=IDM_SELECTDETAILS;
			InsertMenuItem(hColMenu,WORD(-1),FALSE,&mii);
			
			TrackPopupMenu(hColMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,*this,NULL);	
			DestroyMenu(hColMenu);
		}
		else
			TrackPopupMenu(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,*this,NULL);	
	}
	CDialog::OnContextMenu(hWnd,pos);
}	

void CLocateDlg::OnDrawClipboard()
{
	::SendMessage(m_hNextClipboardViewer,WM_DRAWCLIPBOARD,0,0);
	CheckClipboard();
	CDialog::OnDrawClipboard();
}

void CLocateDlg::OnChangeCbChain(HWND hWndRemove,HWND hWndAfter)
{
	CDialog::OnChangeCbChain(hWndRemove,hWndAfter);
	if (m_hNextClipboardViewer==hWndRemove)
		m_hNextClipboardViewer=hWndAfter;
	if (m_hNextClipboardViewer!=NULL)
		::SendMessage(m_hNextClipboardViewer,WM_CHANGECBCHAIN,(WPARAM)hWndRemove,(LPARAM)hWndAfter);
}
	


void CLocateDlg::SaveRegistry()
{
	CRegKey RegKey;
	CString Path;
	DWORD temp;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\General";

			

	if(RegKey.OpenKey(HKCU,Path,CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		CRect rect;
		CMenu menu(GetSubMenu(GetMenu(),2));
		RegKey.SetValue("Program Status",m_dwFlags&fgSave);
		RegKey.SetValue("Program StatusExtra",m_dwExtraFlags&efSave);
		
		SavePosition(RegKey,NULL,"Position");

		GetWindowRect(&rect);
		WINDOWPLACEMENT wp;
		wp.length=sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(&wp);
		if (m_dwFlags&fgLargeMode && wp.showCmd!=SW_MAXIMIZE)
			m_nLargeY=rect.bottom-rect.top;
		RegKey.SetValue("LargeCY",(LPCTSTR)&m_nLargeY,4,REG_DWORD);
		for (temp=0;menu.GetMenuState(temp,MF_BYPOSITION)!=MF_CHECKED && temp<3;temp++);
		RegKey.SetValue("ListView",(LPCTSTR)&temp,4,REG_DWORD);
		if (menu.GetMenuState(IDM_AUTOARRANGE,MF_BYCOMMAND)==MF_CHECKED)
			temp=1;
		else
			temp=0;
		RegKey.SetValue("AutoArrange",(LPCTSTR)&temp,4,REG_DWORD);
		if (menu.GetMenuState(IDM_ALIGNTOGRID,MF_BYCOMMAND)==MF_CHECKED)
			temp=1;
		else
			temp=0;
		RegKey.SetValue("AlignToGrid",(LPCTSTR)&temp,4,REG_DWORD);
	}
}

void CLocateDlg::LoadRegistry()
{
	CMenu menu(GetMenu());
	CRegKey RegKey;
	DWORD temp;
	DWORD x=100,y=100,cx=438;

	CString Path(IDS_REGPLACE,CommonResource);
	Path<<"\\Locate";

	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("MaximumFoundFiles",(LPTSTR)&m_dwMaxFoundFiles,sizeof(DWORD));
		RegKey.CloseKey();
	}
	
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\General";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		// Loading dwFlags
		temp=m_dwFlags;
		RegKey.QueryValue("Program Status",temp);
		m_dwFlags&=~fgSave;
		m_dwFlags|=temp&fgSave;
		
		temp=m_dwExtraFlags;
		RegKey.QueryValue("Program StatusExtra",temp);
		m_dwExtraFlags&=~efSave;
		m_dwExtraFlags|=temp&efSave;
		

		DWORD dwOldFlags=m_dwFlags;
		m_dwFlags|=fgLargeMode;
		LoadPosition(RegKey,NULL,"Position",fgAllowMaximized|fgOnlyNormalPosition);
		m_dwFlags=dwOldFlags;
		
		if (!LoadPosition(RegKey,NULL,"Position",fgAllowMaximized|fgOnlySpecifiedPosition))
		{
			// Could not load previous position, we will cause OnSize to be runned
			WINDOWPLACEMENT wp;
			wp.length=sizeof(wp);
			GetWindowPlacement(&wp);
			wp.rcNormalPosition.right+=10;
			wp.showCmd=SW_SHOW;
			SetWindowPlacement(&wp);
		} 
		
		RegKey.QueryValue("LargeCY",m_nLargeY);
		
		
		temp=0;
		RegKey.QueryValue("AutoArrange",temp);
		if (temp)
		{
			menu.CheckMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_CHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_CHECKED);
			m_pListCtrl->SetStyle(m_pListCtrl->GetStyle()|LVS_AUTOARRANGE);
		}
		temp=0;
		RegKey.QueryValue("AlignToGrid",temp);
		if (temp)
		{
			menu.CheckMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_CHECKED);
			CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_CHECKED);
			m_pListCtrl->SetExtendedListViewStyle(0x00080000 /*LVS_EX_SNAPTOGRID*/,0x00080000);
		}

		temp=3;
		RegKey.QueryValue("ListView",temp);
		SetListStyle(LOBYTE(temp)&3,TRUE);
	}
	else
	{	
		SetListStyle(3,TRUE);

		// Could not load previous position, we will cause OnSize to be runned
		WINDOWPLACEMENT wp;
		wp.length=sizeof(wp);
		GetWindowPlacement(&wp);
		wp.rcNormalPosition.right+=10;
		wp.rcNormalPosition.bottom=wp.rcNormalPosition.top+m_nMaxYMinimized;
		wp.showCmd=SW_SHOW;
		SetWindowPlacement(&wp);
	}
}

void CLocateDlg::LoadDialogTexts()
{
	CRegKey RegKey;
	CString Path;

	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Dialogs";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		m_NameDlg.LoadControlStates(RegKey);
		m_NameDlg.EnableItems(TRUE);

		m_SizeDateDlg.LoadControlStates(RegKey);
		m_SizeDateDlg.EnableItems(TRUE);

		m_AdvancedDlg.LoadControlStates(RegKey);
		m_AdvancedDlg.EnableItems(TRUE);
	}
	
}

void CLocateDlg::SaveDialogTexts()
{
	CRegKey RegKey;
	CString Path;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Dialogs";
	if(RegKey.OpenKey(HKCU,Path,CRegKey::createNew|CRegKey::samAll|CRegKey::samEnumerateSubkeys)==ERROR_SUCCESS)
	{
		m_NameDlg.SaveControlStates(RegKey);
		m_SizeDateDlg.SaveControlStates(RegKey);
		m_AdvancedDlg.SaveControlStates(RegKey);
		
	}
}

void CLocateDlg::SetVisibleWindowInTab()
{
	switch (m_pTabCtrl->GetCurSel())
	{
	case 0:
		if (!m_NameDlg.IsWindowVisible())
		{
			m_SizeDateDlg.ShowWindow(swHide);
			m_AdvancedDlg.ShowWindow(swHide);				
			m_NameDlg.ShowWindow(swShow);
			
			
			m_NameDlg.SetFocus();
		}
		break;
	case 1:
		if (!m_SizeDateDlg.IsWindowVisible())
		{
			m_NameDlg.ShowWindow(swHide);
			m_AdvancedDlg.ShowWindow(swHide);				
			m_SizeDateDlg.ShowWindow(swShow);
			
			m_SizeDateDlg.SetFocus();
		}
		break;
	case 2:
		if (!m_AdvancedDlg.IsWindowVisible())
		{
			m_NameDlg.ShowWindow(swHide);
			m_SizeDateDlg.ShowWindow(swHide);
			m_AdvancedDlg.ShowWindow(swShow);				

			m_AdvancedDlg.SetFocus();
		}
		break;
	}
}
	

BOOL CLocateDlg::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	switch (idCtrl)
	{
	case IDC_FILELIST:
		ListNotifyHandler((LV_DISPINFO*)pnmh,(NMLISTVIEW*)pnmh);
		break;
	case IDC_TAB:
		if (pnmh->code==TCN_SELCHANGE)
			SetVisibleWindowInTab();
		break;
	default:
		if (m_pListTooltips!=NULL)
		{
			if (pnmh->hwndFrom!=*m_pListTooltips)
				break;
			if (m_iTooltipItem==-1 || m_iTooltipSubItem==-1)
				break;

			switch (pnmh->code)
			{
			case TTN_GETDISPINFO:
				if (DetailType(m_pListCtrl->GetColumnIDFromSubItem(m_iTooltipSubItem))==DetailType::Title)
				{
					((NMTTDISPINFO*)pnmh)->hinst=NULL;
						
					CLocatedItem* pItem=(CLocatedItem*)((NMTTDISPINFO*)pnmh)->lParam;
					
					if (pItem!=NULL)
						((NMTTDISPINFO*)pnmh)->lpszText=pItem->GetToolTipText();
					else
						((NMTTDISPINFO*)pnmh)->lpszText=(LPSTR)szEmpty;
				}
				else
				{
					CLocatedItem* pItem=(CLocatedItem*)((NMTTDISPINFO*)pnmh)->lParam;
					((NMTTDISPINFO*)pnmh)->lpszText=pItem->GetDetailText(
						DetailType(m_pListCtrl->GetColumnIDFromSubItem(m_iTooltipSubItem)));
				}
				break;
			case TTN_SHOW:
				m_bTooltipActive=TRUE;
				DebugMessage("TTN_SHOW");
				
				if (DetailType(m_pListCtrl->GetColumnIDFromSubItem(m_iTooltipSubItem))!=DetailType::Title)
				{
					CRect rc;
					m_pListCtrl->GetSubItemRect(m_iTooltipItem,m_iTooltipSubItem,LVIR_LABEL,&rc);
					m_pListCtrl->ClientToScreen(&rc);
                                     
					if (DetailType(m_pListCtrl->GetColumnIDFromSubItem(m_iTooltipSubItem))==DetailType::InFolder)
					{
						CRect rc2;
						m_pListCtrl->GetSubItemRect(m_iTooltipItem,m_iTooltipSubItem,LVIR_ICON,&rc2);
						rc.left+=rc2.Width()+2;
					}

					rc.left+=3;
					
					m_pListTooltips->SetWindowPos(HWND_TOPMOST,rc.left,rc.top,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
					SetWindowLong(dwlMsgResult,1);

					return 1;
				}
				return 0;				
			case TTN_POP:
				m_bTooltipActive=FALSE;
				DebugMessage("TTN_POP");
				break;
			case NM_CUSTOMDRAW:
				return CDRF_DODEFAULT;
			}
		}
		break;
	}
	return CDialog::OnNotify(idCtrl,pnmh);
}

void CLocateDlg::SetSortArrowToHeader(DetailType nType,BOOL bRemove,BOOL bDownArrow)
{
	if (((CLocateApp*)GetApp())->m_wComCtrlVersion<0x0600)
		return;

	if (int(nType)>int(DetailType::LastType))
		return;

	int nColumn=m_pListCtrl->GetVisibleColumn(m_pListCtrl->GetColumnFromID(nType));

	HWND hHeader=m_pListCtrl->GetHeader();
	HDITEM hi;
	hi.mask=HDI_FORMAT;
	::SendMessage(hHeader,HDM_GETITEM,nColumn,(LPARAM)&hi);
	hi.fmt&=~(0x0400|0x0200);
	if (!bRemove)
	{
		if (!bDownArrow)
			hi.fmt|=0x400;
		else
			hi.fmt|=0x200;
	}
	::SendMessage(hHeader,HDM_SETITEM,nColumn,(LPARAM)&hi);
}

	
BOOL CLocateDlg::ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm)
{
	switch(pLvdi->hdr.code)
	{
	case NM_CLICK:
		if (!(m_dwFlags&fgLVStyleSingleClick))
			break;
	case NM_DBLCLK:
		{
			if (m_ClickWait)
				break;
			CWaitCursor wait;
			m_ClickWait=TRUE;
			OnMenuCommands(0);
			SetTimer(ID_CLICKWAIT,700,NULL);
			break;
		}
	case LVN_COLUMNCLICK:
		SortItems(DetailType(m_pListCtrl->GetColumnIDFromSubItem(pNm->iSubItem)));
		break;
	case LVN_GETDISPINFO:
		{
			CLocatedItem* pItem=(CLocatedItem*)pLvdi->item.lParam;
			if (pItem==NULL)
				break;
			
			DetailType nDetail=DetailType(m_pListCtrl->GetColumnIDFromSubItem(pLvdi->item.iSubItem));

			switch (nDetail)
			{
			// Title and parent are special since they have icons
			case DetailType::Title:
				if (pItem->ShouldUpdateTitle() || pItem->ShouldUpdateIcon())
				{
					if (m_pBackgroundUpdater==NULL)
						m_pBackgroundUpdater=new CBackgroundUpdater(m_pListCtrl);
					
					//DebugFormatMessage("Calling %X->AddToUpdateList(%X,%X,Title)",m_pBackgroundUpdater,pItem,pLvdi->item.iItem);
					m_pBackgroundUpdater->AddToUpdateList(pItem,pLvdi->item.iItem,Title);
					if (m_pLocater==NULL) // Locating in process
						m_pBackgroundUpdater->StopWaiting();
				}

				if (pItem->GetTitle()!=NULL)
					pLvdi->item.pszText=pItem->GetTitle();
				else
					pLvdi->item.pszText=pItem->GetName();
				
				pLvdi->item.iImage=pItem->GetIcon();

				if (pItem->GetAttributes()&(LITEMATTRIB_CUTTED|LITEMATTRIB_HIDDEN))
				{
					pLvdi->item.mask|=LVIF_STATE;
					pLvdi->item.state=LVIS_CUT;
					pLvdi->item.stateMask=LVIS_CUT;
				}
				break;
			case DetailType::InFolder:
				if (pItem->ShouldUpdateParentIcon())
				{
					if (m_pBackgroundUpdater==NULL)
						m_pBackgroundUpdater=new CBackgroundUpdater(m_pListCtrl);

					//DebugFormatMessage("Calling %X->AddToUpdateList(%X,%X,InFolder)",m_pBackgroundUpdater,pItem,pLvdi->item.iItem);
					m_pBackgroundUpdater->AddToUpdateList(pItem,pLvdi->item.iItem,InFolder);
					if (m_pLocater==NULL) // Locating is not in process
						m_pBackgroundUpdater->StopWaiting();
				}

				pLvdi->item.mask=LVIF_TEXT|LVIF_IMAGE;
				pLvdi->item.pszText=pItem->GetParent();
				pLvdi->item.iImage=pItem->GetParentIcon();
				break;
			default:
				ASSERT (nDetail<=DetailType::LastType);
				
				if (pItem->ShouldUpdateByDetail(nDetail))
				{
					if (m_pBackgroundUpdater==NULL)
						m_pBackgroundUpdater=new CBackgroundUpdater(m_pListCtrl);
					//DebugFormatMessage("Calling %X->AddToUpdateList(%X,%X,%d)",m_pBackgroundUpdater,pItem,pLvdi->item.iItem,DWORD(nDetail));
					
					m_pBackgroundUpdater->AddToUpdateList(pItem,pLvdi->item.iItem,nDetail);
					if (m_pLocater==NULL) // Locating is not in process
						m_pBackgroundUpdater->StopWaiting();
				}
				pLvdi->item.pszText=pItem->GetDetailText(nDetail);
			}
			break;
		}
	case LVN_DELETEITEM:
		if (pNm->lParam!=NULL)
			delete (CLocatedItem*)pNm->lParam;
		break;
	case LVN_BEGINDRAG:
	case LVN_BEGINRDRAG:
		if (m_pListCtrl->GetNextItem(-1,LVNI_SELECTED)!=-1)
			BeginDragFiles(m_pListCtrl);
		break;
	case LVN_GETINFOTIP:
		break;
	}
	return FALSE;
}

void CLocateDlg::SortItems(DetailType nColumn,BYTE bDescending)
{
	DebugFormatMessage("CLocateDlg::SortItems(%X,%d) BEGIN",int(nColumn),bDescending);


	
	CWaitCursor wait;
	if ((m_nSorting&127)!=nColumn && (m_nSorting&127)<100)
		SetSortArrowToHeader(DetailType(m_nSorting&127),TRUE,FALSE); // Removing old arrow
	if (bDescending==BYTE(-1))
		bDescending=(m_nSorting&128)==0 && (m_nSorting&127)==nColumn;

	SetSortArrowToHeader(nColumn,FALSE,bDescending);

	if (!bDescending)
	{ 
		DebugFormatMessage("Going to sort(1), nColumn is %X",LPARAM(nColumn));
		BOOL bRet=m_pListCtrl->SortItems(ListViewCompareProc,(LPARAM)(nColumn));
		DebugFormatMessage("bRet=%X",bRet);
		m_nSorting=nColumn&127;
	}
	else
	{
		DebugFormatMessage("Going to sort(2), nColumn is %X",LPARAM(nColumn));
		BOOL bRet=m_pListCtrl->SortItems(ListViewCompareProc,(LPARAM)(nColumn|128));
		DebugFormatMessage("bRet=%X",bRet);
		m_nSorting=nColumn|128;
	}
	m_pListCtrl->SendMessage(LVM_FIRST+140/* LVM_SETSELECTEDCOLUMN */,nColumn,0);

	DebugMessage("CLocateDlg::SortItems END");
}

int CALLBACK CLocateDlg::ListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CLocatedItem* pItem1=(CLocatedItem*)lParam1;
	CLocatedItem* pItem2=(CLocatedItem*)lParam2;


	DetailType nDetail=DetailType(lParamSort&127);
	switch (nDetail)
	{
	case DetailType::Title:
		if (pItem1->ShouldUpdateTitle())
			pItem1->UpdateTitle();
		if (pItem2->ShouldUpdateTitle())
			pItem2->UpdateTitle();
		if (lParamSort&128)
			return lstrcmpi(pItem2->GetTitle(),pItem1->GetTitle());
		return lstrcmpi(pItem1->GetTitle(),pItem2->GetTitle());
	case DetailType::InFolder:
		if (lParamSort&128)
			return lstrcmpi(pItem2->GetParent(),pItem1->GetParent());
		return lstrcmpi(pItem1->GetParent(),pItem2->GetParent());
	case DetailType::FullPath:
		if (lParamSort&128)
			return lstrcmpi(pItem2->GetPath(),pItem1->GetPath());
		return lstrcmpi(pItem1->GetPath(),pItem2->GetPath());
	case DetailType::FileSize:
		if (pItem1->ShouldUpdateFileSize())
			pItem1->UpdateFileSizeAndTime();
		if (pItem2->ShouldUpdateFileSize())
			pItem2->UpdateFileSizeAndTime();

		if (pItem2->IsFolder())
		{
			if (pItem1->IsFolder())
				return 0;
			return lParamSort&128?-1:0;
		}
		else if (pItem1->IsFolder())
			return lParamSort&128?1:-1;
		
		if (pItem2->GetFileSizeHi()!=pItem1->GetFileSizeHi())
		{		
			if (pItem2->GetFileSizeHi()>pItem1->GetFileSizeHi())
				return lParamSort&128?1:-1;
			else
				return lParamSort&128?-1:1;

		}
		if (pItem2->GetFileSizeLo()==pItem1->GetFileSizeLo())
			return 0;
		else if (pItem2->GetFileSizeLo()>pItem1->GetFileSizeLo())
			return lParamSort&128?1:-1;
		else
			return lParamSort&128?-1:1;
	case DetailType::FileType:
		if (pItem1->ShouldUpdateType())
			pItem1->UpdateType();
		if (pItem2->ShouldUpdateType())
			pItem2->UpdateType();
		if (lParamSort&128)
			return lstrcmpi(pItem2->GetType(),pItem1->GetType());
		return lstrcmpi(pItem1->GetType(),pItem2->GetType());
	case DetailType::DateModified:
		if (pItem1->ShouldUpdateTimeAndDate())
			pItem1->UpdateFileSizeAndTime();
		if (pItem2->ShouldUpdateTimeAndDate())
			pItem2->UpdateFileSizeAndTime();
		
		if (pItem1->GetModifiedDate()==pItem2->GetModifiedDate())
		{
			// Same day
			if (pItem2->GetModifiedTime()==pItem1->GetModifiedTime())
				return 0;
			else if (pItem2->GetModifiedTime()>pItem1->GetModifiedTime())
				return lParamSort&128?1:-1;
			else
				return lParamSort&128?-1:1;
		}
		if (pItem2->GetModifiedDate()>pItem1->GetModifiedDate())
			return lParamSort&128?1:-1;
		else
			return lParamSort&128?-1:1;
	case DetailType::DateCreated:
		if (pItem1->ShouldUpdateTimeAndDate())
			pItem1->UpdateFileSizeAndTime();
		if (pItem2->ShouldUpdateTimeAndDate())
			pItem2->UpdateFileSizeAndTime();
		
		if (pItem1->GetCreatedDate()==pItem2->GetCreatedDate())
		{
			// Same day
			if (pItem2->GetModifiedTime()==pItem1->GetCreatedTime())
				return 0;
			else if (pItem2->GetCreatedTime()>pItem1->GetCreatedTime())
				return lParamSort&128?1:-1;
			else
				return lParamSort&128?-1:1;
		}
		if (pItem2->GetCreatedDate()>pItem1->GetCreatedDate())
			return lParamSort&128?1:-1;
		else
			return lParamSort&128?-1:1;
	case DetailType::DateAccessed:
		if (pItem1->ShouldUpdateTimeAndDate())
			pItem1->UpdateFileSizeAndTime();
		if (pItem2->ShouldUpdateTimeAndDate())
			pItem2->UpdateFileSizeAndTime();
		
		if (pItem1->GetAccessedDate()==pItem2->GetAccessedDate())
		{
			// Same day
			if (pItem2->GetAccessedTime()==pItem1->GetAccessedTime())
				return 0;
			else if (pItem2->GetAccessedTime()>pItem1->GetAccessedTime())
				return lParamSort&128?1:-1;
			else
				return lParamSort&128?-1:1;
		}
		if (pItem2->GetAccessedDate()>pItem1->GetAccessedDate())
			return lParamSort&128?1:-1;
		else
			return lParamSort&128?-1:1;
	case DetailType::Attributes:
		if (pItem1->ShouldUpdateAttributes())
			pItem1->UpdateAttributes();
		if (pItem2->ShouldUpdateAttributes())
			pItem2->UpdateAttributes();

		if (pItem2->IsDeleted())
		{
			if (pItem1->IsDeleted())
				return 0;
			return lParamSort&128?-1:1;
		}
		else if (pItem2->IsDeleted())
			return lParamSort&128?1:-1;
		if (lParamSort&128)
			return int(pItem2->GetAttributes())-int(pItem1->GetAttributes());
		return int(pItem1->GetAttributes())-int(pItem2->GetAttributes());
	case DetailType::ImageDimensions:
		if (pItem1->ShouldUpdateExtra(ImageDimensions))
			pItem1->UpdateDimensions();
		if (pItem2->ShouldUpdateExtra(ImageDimensions))
			pItem2->UpdateDimensions();
		
		if (pItem2->GetImageDimensionsProduct()==pItem1->GetImageDimensionsProduct())
			return 0;
		else if (pItem2->GetImageDimensionsProduct()>pItem1->GetImageDimensionsProduct())
			return lParamSort&128?1:-1;
		else
			return lParamSort&128?-1:1;
	case DetailType::Owner:
	case DetailType::ShortFileName:	
	case DetailType::ShortFilePath:
		{
			if (pItem1->ShouldUpdateExtra(nDetail))
				pItem1->UpdateByDetail(nDetail);
			if (pItem2->ShouldUpdateExtra(nDetail))
				pItem2->UpdateByDetail(nDetail);
	
			LPCSTR pText1=pItem1->GetExtraText(nDetail);
			LPCSTR pText2=pItem2->GetExtraText(nDetail);

			if (pText2==NULL)
			{
				if (pText1==NULL)
					return 0;
				return lParamSort&128?-1:0;
			}
			else if (pText1==NULL)
				return lParamSort&128?1:-1;
			
			if (lParamSort&128)
				return lstrcmpi(pText2,pText1);
			return lstrcmpi(pText1,pText2);
		}
	case DetailType::Database:
		if (lParamSort&128)
		{
			return lstrcmpi(GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetName(),
				GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetName());
		}
		return lstrcmpi(GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetName(),
			GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetName());
	case DetailType::DatabaseDescription:
		if (lParamSort&128)
		{
			return lstrcmpi(GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetDescription(),
				GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetDescription());
		}
		return lstrcmpi(GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetDescription(),
			GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetDescription());
	case DetailType::DatabaseArchive:
		if (lParamSort&128)
		{
			return lstrcmpi(GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetArchiveName(),
				GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetArchiveName());
		}
		return lstrcmpi(GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetArchiveName(),
			GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetArchiveName());
	}
	return 0;
}


void CLocateDlg::SetSystemImagelists(CListCtrl* pList,HICON* phIcon)
{
	SHFILEINFO fi;
	HIMAGELIST hList=(HIMAGELIST)SHGetFileInfo(szEmpty,0,&fi,sizeof(SHFILEINFO),/*SHGFI_ICON|*/SHGFI_SYSICONINDEX);
	pList->SetImageList(hList,LVSIL_NORMAL);
	hList=(HIMAGELIST)SHGetFileInfo(szEmpty,0,&fi,sizeof(SHFILEINFO),/*SHGFI_ICON|*/SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
	pList->SetImageList(hList,LVSIL_SMALL);
	if (phIcon!=NULL)
		*phIcon=ImageList_ExtractIcon(NULL,hList,DEF_IMAGE);
}

void CLocateDlg::EnableItems()
{
	CButton OK(GetDlgItem(IDC_OK));
	CButton Stop(GetDlgItem(IDC_STOP));
	OK.EnableWindow(TRUE);
	OK.SetButtonStyle(BS_DEFPUSHBUTTON);
	Stop.SetButtonStyle(BS_PUSHBUTTON);
	Stop.EnableWindow(FALSE);
	EnableDlgItem(IDC_PRESETS,TRUE);
	m_NameDlg.EnableItems(TRUE);
	m_SizeDateDlg.EnableItems(TRUE);
	m_AdvancedDlg.EnableItems(TRUE);

	if (m_pListCtrl->GetItemCount())
	{
		SetFocus();
		m_pListCtrl->SetItemState(0,LVIS_SELECTED,LVIS_SELECTED);
	}
	else if (m_pTabCtrl->GetCurSel()==0)
	{
		m_NameDlg.SetFocus();
		::SetFocus(m_NameDlg.GetDlgItem(IDC_NAME));
	}

}

void CLocateDlg::DisableItems()
{
	CButton OK(GetDlgItem(IDC_OK));
	CButton Stop(GetDlgItem(IDC_STOP));
	OK.SetButtonStyle(BS_PUSHBUTTON);
	OK.EnableWindow(FALSE);
	Stop.EnableWindow(TRUE);
	Stop.SetButtonStyle(BS_DEFPUSHBUTTON);
	EnableDlgItem(IDC_PRESETS,FALSE);
	m_NameDlg.EnableItems(FALSE);
	m_SizeDateDlg.EnableItems(FALSE);
	m_AdvancedDlg.EnableItems(FALSE);
}

void CLocateDlg::OnMenuCommands(WORD wID)
{
	CWaitCursor wait;
	if (m_pListCtrl->GetSelectedCount()==0)
		return;

	CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(m_pListCtrl->GetNextItem(-1,LVNI_SELECTED));
	
	if (!pItem->IsFolder() && !CFile::IsFile(pItem->GetPath()))
		return;
	if (pItem->IsFolder() && !CFile::IsDirectory(pItem->GetPath()))
		return;

	if (wID>=IDM_DEFCONTEXTITEM && m_pActiveContextMenu!=NULL)
	{
		char szName[100];
		m_pActiveContextMenu->GetCommandString(wID-IDM_DEFCONTEXTITEM,GCS_VERBA,NULL,
			szName,100);
		DebugFormatMessage("Running context verb: %s",szName);
		
		// Overriding these command, works better
		if (strcmp(szName,"copy")==0)
		{
			OnCopy(FALSE);
			ClearMenuVariables();
			return;
		}
		else if (strcmp(szName,"cut")==0)
		{
			OnCopy(TRUE);
			ClearMenuVariables();
			return;
		}
		else if (strcmp(szName,"link")==0)
		{
			OnCreateShortcut();
			ClearMenuVariables();
			return;
		}
		else if (strcmp(szName,"delete")==0)
		{
			OnDelete();
			ClearMenuVariables();
			return;
		}
		
		CMINVOKECOMMANDINFO cii;
		cii.cbSize=sizeof(CMINVOKECOMMANDINFO);
		cii.fMask=0;
		cii.hwnd=*this;
		cii.lpVerb=(LPCSTR)MAKELONG(wID-IDM_DEFCONTEXTITEM,0);
		cii.lpParameters=NULL;
		cii.lpDirectory=pItem->GetParent();
		cii.nShow=SW_SHOWDEFAULT;
		m_pActiveContextMenu->InvokeCommand(&cii);
		ClearMenuVariables();
	}
	else if (wID==0)
	{
		int iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
		while (iItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
			if (pItem->IsFolder())
				OpenFolder(pItem->GetPath());
			else if (int(ShellExecute(*this,NULL,pItem->GetPath(),NULL,NULL,SW_SHOW))<=32)
			{
				CArrayFP<CString*> aFile;
				aFile.Add(new CString(pItem->GetPath()));
				IContextMenu* pContextMenu=GetContextMenuForFiles(pItem->GetParent(),aFile);
				if (pContextMenu!=NULL)
				{
					CMINVOKECOMMANDINFO cii;
					cii.cbSize=sizeof(CMINVOKECOMMANDINFO);
					cii.fMask=0;
					cii.hwnd=*this;
					cii.lpVerb=0;
					cii.lpParameters=NULL;
					cii.lpDirectory=NULL;
					cii.nShow=SW_SHOWDEFAULT;
					HMENU hMenu=CreatePopupMenu();
					pContextMenu->QueryContextMenu(hMenu,0,IDM_DEFCONTEXTITEM,IDM_DEFSENDTOITEM,CMF_DEFAULTONLY|CMF_VERBSONLY);
					pContextMenu->InvokeCommand(&cii);
					pContextMenu->Release();
					DestroyMenu(hMenu);
				}
			}
			iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
		}
	}
	else
		DebugFormatMessage("CLocateDlg::OnMenuCommands(): wID is invalid=%d",wID);

}

void CLocateDlg::OnProperties()
{
	CWaitCursor wait;
	if (m_pActiveContextMenu!=NULL)
	{
		CMINVOKECOMMANDINFO cii;
		cii.cbSize=sizeof(CMINVOKECOMMANDINFO);
		cii.fMask=0;
		cii.hwnd=*this;
		cii.lpVerb="properties";
		cii.lpParameters=NULL;
		cii.lpDirectory=NULL;
		cii.nShow=SW_SHOWDEFAULT;
		m_pActiveContextMenu->InvokeCommand(&cii);
	}
}

void CLocateDlg::OnAutoArrange()
{
	CMenu menu(GetMenu());
	if (menu.GetMenuState(IDM_AUTOARRANGE,MF_BYCOMMAND)==MF_UNCHECKED)
	{
		menu.CheckMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_CHECKED);
		CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_CHECKED);
		m_pListCtrl->SetStyle(m_pListCtrl->GetStyle()|LVS_AUTOARRANGE);
	}
	else
	{
		menu.CheckMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_UNCHECKED);
		CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_UNCHECKED);
		m_pListCtrl->SetStyle(m_pListCtrl->GetStyle()&~LVS_AUTOARRANGE);
	}
}

void CLocateDlg::OnAlignToGrid()
{
	CMenu menu(GetMenu());
	if (menu.GetMenuState(IDM_ALIGNTOGRID,MF_BYCOMMAND)==MF_UNCHECKED)
	{
		menu.CheckMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_CHECKED);
		CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_CHECKED);
		m_pListCtrl->SetExtendedListViewStyle(0x00080000 /*LVS_EX_SNAPTOGRID*/,0x00080000);
	}
	else
	{
		menu.CheckMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_UNCHECKED);
		CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_UNCHECKED);
		m_pListCtrl->SetExtendedListViewStyle(0,0x00080000 /*LVS_EX_SNAPTOGRID*/);
	}
}

void CLocateDlg::OnRefresh()
{
	m_pListCtrl->RedrawItems(m_pListCtrl->GetTopIndex(),m_pListCtrl->GetTopIndex()+m_pListCtrl->GetCountPerPage());
	m_pListCtrl->UpdateWindow();
}	
		
void CLocateDlg::OnDelete()
{
	if (GetFocus()==GetDlgItem(IDC_FILELIST))
	{
		CWaitCursor wait;
		CArray<CLocatedItem*> aItems;
		int nBufferLength=1; // For buffer length
		
		// Resolving files
		int iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
		while (iItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
			LPCSTR szPath=pItem->GetPath();
			if (CFile::IsFile(szPath))
			{
				aItems.Add(pItem);
				nBufferLength+=pItem->GetPathLen()+1;
			}
			else if (CFile::IsDirectory(szPath))
			{
				aItems.Add(pItem);
				nBufferLength+=pItem->GetPathLen()+1;
			}
			iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
		}
		
		if (aItems.GetSize()==0) // No files
			return;

		// Filling OPSTRUCT fields
		SHFILEOPSTRUCT fo;
		fo.hwnd=*this;
		fo.wFunc=FO_DELETE;
		fo.pTo=NULL;
		if (HIBYTE(GetKeyState(VK_SHIFT)))
			fo.fFlags=0;
		else
			fo.fFlags=FOF_ALLOWUNDO;

		// Creating file buffer: file1\0file2\0...filen\0\0
		BYTE* pFiles=new BYTE[nBufferLength];
		fo.pFrom=LPCSTR(pFiles);
		for (int i=0;i<aItems.GetSize();i++)
		{
			sMemCopy(pFiles,aItems.GetAt(i)->GetPath(),aItems.GetAt(i)->GetPathLen()+1);
			pFiles+=aItems.GetAt(i)->GetPathLen()+1;
		}
		*pFiles='\0';
		
		StopBackgroundOperations();
		
		
		// Delete files
		int iRet=SHFileOperation(&fo);

		
		delete[] (BYTE*)fo.pFrom;

		
		
		if (iRet==0)
		{
			// Error maybe occur we don't clear this
			if (m_pListTooltips!=NULL)
				DeleteTooltipTools();
			
			// Removing deleted items from list
			int iItem;
			int iSeekStart=-1;
			while ((iItem=m_pListCtrl->GetNextItem(iSeekStart,LVNI_SELECTED))!=-1)
			{
				CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
				LPCSTR szPath=pItem->GetPath();
			
				if (CFile::IsFile(szPath))
				{
					iSeekStart=iItem;
					continue;
				}
				else if (CFile::IsDirectory(szPath))
				{
					iSeekStart=iItem;
					continue;
				}

				
				m_pListCtrl->SetItemData(iItem,NULL);
				delete pItem;
				// File or directory do not exist, deleting it
				m_pListCtrl->DeleteItem(iItem);
			}
		}

		StartBackgroundOperations();

		// Todo: change this code to check items which are in deleted folders and
		// remove them		
		int nItem=m_pListCtrl->GetNextItem(-1,LVNI_ALL);
		while(nItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
			pItem->CheckIfDeleted();
			ASSERT(m_pBackgroundUpdater!=NULL);
			//DebugFormatMessage("Calling %X->AddToUpdateList(%X,%X,CLocateDlg::Needed)",m_pBackgroundUpdater,pItem,nItem);
					
			m_pBackgroundUpdater->AddToUpdateList(pItem,nItem,CLocateDlg::Needed);			
			nItem=m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
		}
		m_pListCtrl->RedrawItems(0,m_pListCtrl->GetItemCount());
		m_pListCtrl->UpdateWindow();
		m_pBackgroundUpdater->StopWaiting();
		
	}
}

void CLocateDlg::OnRemoveFromThisList()
{
	if (GetFocus()==GetDlgItem(IDC_FILELIST))
	{
		int iItem;
		while ((iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED))!=-1)
			m_pListCtrl->DeleteItem(iItem);
	}
}


BOOL CLocateDlg::CheckClipboard()
{
	if (m_pListCtrl==NULL)
		return TRUE;
	CArrayFP<CString*> aFiles;
	BYTE bIsCuttedFiles=FALSE;
	UINT nPreferredDropEffectFormat=RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	
	
	OpenClipboard();
	
	if (IsClipboardFormatAvailable(nPreferredDropEffectFormat))
	{
		if (IsClipboardFormatAvailable(CF_HDROP))
		{
			HANDLE hData=GetClipboardData(CF_HDROP);
			if (hData!=NULL)
			{
				CString temp;
				bIsCuttedFiles=TRUE;
				int last=DragQueryFile((HDROP)hData,0xFFFFFFFF,NULL,0);
				for (int i=0;i<last;i++)
				{
					DragQueryFile((HDROP)hData,i,temp.GetBuffer(_MAX_PATH),_MAX_PATH);
					temp.FreeExtra();
					aFiles.Add(new CString(temp));
				}
			}
		}
	}
	CloseClipboard();
	
	int nReDraws=0;
	
	if (bIsCuttedFiles)
	{
		int nItem=m_pListCtrl->GetNextItem(-1,LVNI_ALL);
		while (nItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
			int i;
			for (i=0;i<aFiles.GetSize();i++)
			{
				if (aFiles[i]->CompareNoCase(pItem->GetPath())==0)
				{
					pItem->AddAttribute(LITEMATTRIB_CUTTED);
					m_pListCtrl->RedrawItems(nItem,nItem);
					nReDraws++;
					break;
				}
			}
			if (i==aFiles.GetSize() && pItem->IsCutted())
			{
				pItem->RemoveAttribute(LITEMATTRIB_CUTTED);
				m_pListCtrl->RedrawItems(nItem,nItem);
				nReDraws++;
			}					
			nItem=m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
		}
	}
	else
	{
		int nItem=m_pListCtrl->GetNextItem(-1,LVNI_ALL);
		while (nItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
			if (pItem->IsCutted())
			{
				pItem->RemoveAttribute(LITEMATTRIB_CUTTED);
				m_pListCtrl->RedrawItems(nItem,nItem);
				nReDraws++;
			}
			nItem=m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
		}
	}
	if (nReDraws)
		m_pListCtrl->UpdateWindow();
	
	aFiles.RemoveAll();
	return TRUE;
}
	
void CLocateDlg::OnCopy(BOOL bCut)
{
	if (m_pListCtrl->GetSelectedCount()==0)
		return;

	CWaitCursor wait;
	CArray<CLocatedItem*> aItems;
	CFileObject fo;
	fo.SetFiles(m_pListCtrl);

	// Opening clipboard
	OpenClipboard();
	EmptyClipboard();

	// Setting formats
	SetClipboardData(RegisterClipboardFormat(CFSTR_SHELLIDLIST),fo.GetItemIDList());
	
	HANDLE hData=(LPSTR)GlobalAlloc(GMEM_FIXED,4);
	if (bCut)
		((int*)hData)[0]=DROPEFFECT_MOVE;
	else
		((int*)hData)[0]=DROPEFFECT_COPY|DROPEFFECT_LINK;
	SetClipboardData(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT),hData);
	SetClipboardData(RegisterClipboardFormat(CFSTR_FILENAME),fo.GetFileNameA());
	SetClipboardData(RegisterClipboardFormat(CFSTR_FILENAMEW),fo.GetFileNameW());
	SetClipboardData(CF_HDROP,fo.GetHDrop());
	
	// Closing clipboard
	CloseClipboard();
}
		
void CLocateDlg::OnOpenContainingFolder()
{
	CWaitCursor wait;
	

	// Retrieving folders
	CArray<LPSTR> aFolders;
	int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED|LVNI_ALL);
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
		
		BOOL bFound=FALSE;
		for (int i=0;i<aFolders.GetSize();i++)
		{
			if (strcmp(aFolders[i],pItem->GetParent())==0)
			{
				bFound=TRUE;
				break;
			}
		}
		if (!bFound)
				aFolders.Add(pItem->GetParent());

        
		nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED|LVNI_ALL);
	}

	for (int i=0;i<aFolders.GetSize();i++)
		OpenFolder(aFolders[i]);
}

void CLocateDlg::OpenFolder(LPCSTR szFolder)
{
	CString sProgram;
	
	// Loading some general settings
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::defRead)==ERROR_SUCCESS)
	{
		DWORD dwTemp;
		RegKey.QueryValue("Use other program to open folders",dwTemp);
		if (dwTemp)	
			RegKey.QueryValue("Open folders with",sProgram);
	}
	
	if (sProgram.IsEmpty())
	{
		SHELLEXECUTEINFO sxi;
		sxi.cbSize=sizeof(SHELLEXECUTEINFO);
		sxi.fMask=SEE_MASK_NOCLOSEPROCESS;
		sxi.hwnd=*this;
		sxi.lpVerb="open";
		sxi.lpFile=szFolder;
		sxi.lpParameters=szEmpty;
		sxi.lpDirectory=szEmpty;
		sxi.nShow=SW_SHOWNORMAL;
		ShellExecuteEx(&sxi);	
	}
	else
	{
		CString sTemp;
		int nIndex=sProgram.FindFirst('%');
		int nAdded=0;
        while (nIndex!=-1)
		{
			sTemp.Append(sProgram,nIndex);
			if (sProgram[nIndex+1]=='d')
			{
				sTemp<< szFolder;
				nIndex+=2;
			}
			else
			{
				sTemp<<'%';
				nIndex++;
			}
			nAdded=nIndex;
			nIndex=sProgram.FindNext('%',nIndex);
		}
		sTemp<<(LPCSTR(sProgram)+nAdded);
		WinExec(sTemp,SW_SHOWDEFAULT);
	}
}

typedef HRESULT (__stdcall * PFNSHGETFOLDERPATHA)(HWND, int, HANDLE, DWORD, LPSTR);  // "SHGetFolderPathA"

void CLocateDlg::OnCreateShortcut()
{
	if (m_pListCtrl->GetSelectedCount()==0)
		return;
	
	CWaitCursor wait;
	
	// Resolving desktop path
	CString sDesktopPathTmp;
	CStringW sDesktopPath;

	PFNSHGETFOLDERPATHA pGetFolderPath=(PFNSHGETFOLDERPATHA)GetProcAddress(GetModuleHandle("shell32.dll"),"SHGetFolderPathA");
	if (pGetFolderPath!=NULL)
		pGetFolderPath(*this,CSIDL_DESKTOPDIRECTORY,NULL,SHGFP_TYPE_CURRENT,sDesktopPathTmp.GetBuffer(MAX_PATH));
	else
	{
		CRegKey RegKey;
		if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
			RegKey.QueryValue("Desktop",sDesktopPathTmp);
		if (sDesktopPathTmp.IsEmpty())
			return;
	}
	sDesktopPathTmp.FreeExtra();
	sDesktopPath=sDesktopPathTmp;
	if (sDesktopPath[sDesktopPath.GetLength()-1]!=L'\\')
		sDesktopPath<<L'\\';
	
	// Creating instance to shell link handler
	IShellLink* psl;
    HRESULT hRes=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl);
	if (SUCCEEDED(hRes))
	{
		// Creating instance to PersistFile interface
		IPersistFile* ppf;
		hRes=psl->QueryInterface(IID_IPersistFile,(void**)&ppf);
		
		if (SUCCEEDED(hRes))
		{
			int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
			BOOL bMsgShowed=FALSE;
		
			while (nItem!=-1)
			{
				CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
				if (CFile::IsFile(pItem->GetPath()))
				{
					if (!bMsgShowed)
					{
						if (ShowErrorMessage(IDS_SHORTCUTTODESKTOP,IDS_SHORTCUT,MB_YESNO|MB_ICONQUESTION)==IDNO)
						{
							psl->Release();
							ppf->Release();
							return;
						}
						bMsgShowed=TRUE;
					}
				}
			
				// Setting link path
				hRes=psl->SetPath(pItem->GetPath());
				if (!SUCCEEDED(hRes))
				{
					ppf->Release();
					psl->Release();
					return;
				}
				
				if (pItem->ShouldUpdateTitle())
					pItem->UpdateTitle();
				hRes=psl->SetDescription(CString(IDS_SHORTCUTTO)+pItem->GetTitle());
				if (!SUCCEEDED(hRes))
				{
					ppf->Release();
					psl->Release();
					return;
				}
				
				hRes=ppf->Save(sDesktopPath+pItem->GetTitle()+L".lnk",TRUE);    
				if (!SUCCEEDED(hRes))
				{
					ppf->Release();
					psl->Release();
					return;
				}
				
				nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
			}
			ppf->Release();
		}
		psl->Release();
	}
}

void CLocateDlg::OnInitMenuPopup(HMENU hPopupMenu,UINT nIndex,BOOL bSysMenu)
{
	if (bSysMenu)
		return;

	HMENU hMainMenu=GetMenu();
	HMENU hFileMenu=GetSubMenu(hMainMenu,0);
	HMENU hEditMenu=GetSubMenu(hMainMenu,1);
	// m_hActivePopupMenu points to last activate menu 
	
	if (nIndex>0 && GetSubMenu(GetMenu(),nIndex)==hPopupMenu)
	{
		// Initializing other main menu's submenus than File menu
		OnInitMainMenu(hPopupMenu,nIndex);
		return;
	}

	CDialog::OnInitMenuPopup(hPopupMenu,nIndex,bSysMenu);
	
	if (hPopupMenu==GetSubMenu(GetMenu(),0))
	{
		// File menu in main menu bar

		// Inserting default menu items
		CreateFileContextMenu(hPopupMenu);

		// Enable items
		OnInitFileMenu(hPopupMenu);
	}
	else if (hPopupMenu==m_hActivePopupMenu)
	{
		// Context menu for file item(s)		
		// Enable items
		OnInitFileMenu(hPopupMenu);
	}
	else if (IsSendToMenu(hPopupMenu))
	{
		// SentTo menu, deleting previous menu items and inserting 
		// new ones corresponding selected items
		OnInitSendToMenu(hPopupMenu);
	}
	else if (CLocateApp::IsDatabaseMenu(hPopupMenu))
	{
		// Database menu, deleting previous menu items and inserting 
		// new ones database items
		GetLocateApp()->OnInitDatabaseMenu(hPopupMenu);
	}
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
		EnableMenuItem(hPopupMenu,IDM_PROPERTIES,m_pActiveContextMenu!=NULL?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hPopupMenu,IDM_OPENCONTAININGFOLDER,MF_BYCOMMAND|MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hPopupMenu,IDM_CREATESHORTCUT,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hPopupMenu,IDM_DELETE,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hPopupMenu,IDM_PROPERTIES,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hPopupMenu,IDM_OPENCONTAININGFOLDER,MF_BYCOMMAND|MF_GRAYED);
	}
}

void CLocateDlg::OnInitSendToMenu(HMENU hPopupMenu)
{
	// Removing default items
	for(int i=GetMenuItemCount(hPopupMenu)-1;i>=0;i--)
		DeleteMenu(hPopupMenu,i,MF_BYPOSITION);

	CString SendToPath;
	
	// Resolving Send To -directory location
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("SendTo",SendToPath);
		RegKey.CloseKey();
	}

	if (m_hSendToListFont!=NULL)
		DeleteObject(m_hSendToListFont);
	
	// Initializing fonts
	if (RegKey.OpenKey(HKCU,"Control Panel\\Desktop\\WindowMetrics",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		LOGFONTW font;
		RegKey.QueryValue("MenuFont",(LPSTR)&font,sizeof(LOGFONTW));
		m_hSendToListFont=CreateFontIndirectW(&font);
	}
	else
		m_hSendToListFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);

	AddSendToMenuItems(hPopupMenu,SendToPath,IDM_DEFSENDTOITEM);
}

UINT CLocateDlg::AddSendToMenuItems(HMENU hMenu,CString& sSendToPath,UINT wStartID)
{
	CString Path(sSendToPath);
	CFileFind Find;
	MENUITEMINFO mi;
	BOOL bErr;
	
	mi.cbSize=sizeof(MENUITEMINFO);
	Path << "\\*.*";
	mi.fMask=MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_TYPE|MIIM_SUBMENU;
	mi.fType=MFT_OWNERDRAW;
	mi.fState=MFS_ENABLED;
	mi.wID=wStartID;
	mi.dwTypeData=(LPSTR)hMenu;
	bErr=Find.FindFile(Path);
	while (bErr)
	{
		Path=Find.GetFileName();
		if (Path[0]!='.' && !Find.IsSystem() && !Find.IsHidden())
		{
			Path=sSendToPath;
			Path << '\\' << Find.GetFileName();
			mi.dwItemData=(DWORD)new CHAR[Path.GetLength()+2];
			strcpy((LPSTR)mi.dwItemData,Path);
			if (Find.IsDirectory())
			{
				mi.hSubMenu=CreateMenu();
				mi.wID+=AddSendToMenuItems(mi.hSubMenu,Path,mi.wID);
			}
			else
				mi.hSubMenu=NULL;
			InsertMenuItem(hMenu,mi.wID,FALSE,&mi);
			mi.wID++;
		}
		bErr=Find.FindNextFile();
	}
	Find.Close();
	if (mi.wID==wStartID)
	{
		// Inserting default menu items
		Path.LoadString(IDS_EMPTY);
		mi.dwTypeData=(LPSTR)(LPCSTR)Path;
		mi.dwItemData=0;
		mi.fState=MFS_GRAYED;
		mi.fType=MFT_STRING;
		InsertMenuItem(hMenu,mi.wID,FALSE,&mi);
		mi.wID++;
	}
	return mi.wID-wStartID;
}

BOOL CLocateDlg::InsertMenuItemsFromTemplate(HMENU hMenu,HMENU hTemplate,UINT uStartPosition,int nDefaultItem)
{
	MENUITEMINFO mii;
	char szName[1000];
	mii.cbSize=sizeof(MENUITEMINFO);
	int nMenuLength=GetMenuItemCount(hTemplate);
	for (int i=0;i<nMenuLength;i++)
	{
		mii.fMask=MIIM_ID|MIIM_TYPE|MIIM_STATE|MIIM_SUBMENU;
		mii.dwTypeData=szName;
		mii.cch=1000;
		
		// Checking whether popupmenu is popup menu or item
		if (!GetMenuItemInfo(hTemplate,i,TRUE,&mii))
			return FALSE;

		if (mii.wID==nDefaultItem)
			mii.fState|=MFS_DEFAULT;

		if (mii.hSubMenu!=NULL)
		{
			// It is popup menu
            HMENU hNewMenu=CreatePopupMenu();
			if (!InsertMenuItemsFromTemplate(hNewMenu,mii.hSubMenu,0))
				return FALSE;
			mii.hSubMenu=hNewMenu;		
		}
		
		if (!InsertMenuItem(hMenu,i+uStartPosition,TRUE,&mii))
			return FALSE;
	}
	return TRUE;
}

HMENU CLocateDlg::CreateFileContextMenu(HMENU hFileMenu)
{
	ClearMenuVariables();

	if (hFileMenu!=NULL)
	{
		// Freeing memyry in SentToMenuItems
		FreeSendToMenuItems(hFileMenu);
		
		// Removing all items
		for (int i=GetMenuItemCount(hFileMenu)-1;i>=0;i--)
			DeleteMenu(hFileMenu,i,MF_BYPOSITION);
		
		// Copying menu from template menu in resource
		if (m_pListCtrl->GetSelectedCount()==0)
		{
			InsertMenuItemsFromTemplate(hFileMenu,m_Menu.GetSubMenu(SUBMENU_FILEMENUNOITEMS),0);
			return hFileMenu;
		}
		InsertMenuItemsFromTemplate(hFileMenu,m_Menu.GetSubMenu(SUBMENU_FILEMENU),0);
	}
	
	// Creating context menu for file items
	CArrayFP<CString*> aFiles;
	CString sParent; // For checking that are files in same folder
	
	// Checking first item
	int iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
	if (!pItem->IsItemShortcut())
	{
		sParent=pItem->GetParent();
		aFiles.Add(new CString(pItem->GetPath()));
	}
	else
	{
		// If item is shortcut, check parent
	    CString* pStr=new CString;
		GetShortcutTarget(pItem->GetPath(),pStr->GetBuffer(MAX_PATH));
		pStr->FreeExtra();
		sParent.Copy(*pStr,pStr->FindLast('\\')+1);
		aFiles.Add(pStr);
	}
	
	// Checking other items
	iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
	while (iItem!=-1)
	{
		pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
		if (pItem->IsItemShortcut())
		{
			// If item is shortcut, check parent
			CString* pStr=new CString;
			GetShortcutTarget(pItem->GetPath(),pStr->GetBuffer(_MAX_PATH));
			int nIndex=LastCharIndex(*pStr,'\\');
			if (strncmp(*pStr,pItem->GetPath(),nIndex)!=0)
			{
				delete pStr;
				break;
			}
			pStr->FreeExtra(nIndex);
			aFiles.Add(pStr);
		}
		else
		{
			// If parent is same, break
			if (sParent.Compare(pItem->GetParent())!=0)
				break;
			aFiles.Add(new CString(pItem->GetPath()));
		}
		iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
	}

	// This creates pointers to IContextMenu interfaces
	// This is possible, if files are in same folder, 
	if (iItem==-1)
		m_pActiveContextMenu=GetContextMenuForFiles(sParent,aFiles);

	if (m_pActiveContextMenu!=NULL)
	{
		// IContextMenu interface has created succesfully,
		// so they can insert their own menu items

		HRESULT hRes;
		if (hFileMenu==NULL)
		{
			hFileMenu=CreatePopupMenu();
			InsertMenuItemsFromTemplate(hFileMenu,m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUFORQUERIED),0);

			if (HIBYTE(GetKeyState(VK_SHIFT)))
			{
				hRes=m_pActiveContextMenu->QueryContextMenu(hFileMenu,0,
					IDM_DEFCONTEXTITEM,IDM_DEFSENDTOITEM,CMF_VERBSONLY|CMF_EXTENDEDVERBS);

			}
			else
			{
				hRes=m_pActiveContextMenu->QueryContextMenu(hFileMenu,0,
					IDM_DEFCONTEXTITEM,IDM_DEFSENDTOITEM,CMF_VERBSONLY);

			}
		}
		else
		{
			hRes=m_pActiveContextMenu->QueryContextMenu(hFileMenu,0,
				IDM_DEFCONTEXTITEM,IDM_DEFSENDTOITEM,CMF_VERBSONLY);
		}
		if (!SUCCEEDED(hRes))
		{
			// Didn't succee, so freeing pointer
			m_pActiveContextMenu->Release();
			m_pActiveContextMenu=NULL;
		}
		else
		{
			// Inserting other items to the begin of menu 
			// ("Open containing folder", "Remove from this list", ...)
			MENUITEMINFO mii;
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_ID|MIIM_TYPE;
			mii.fType=MFT_STRING;
			mii.wID=IDM_OPENCONTAININGFOLDER;
			CString str(IDS_OPENCONTAININGFOLDER);
			mii.dwTypeData=str.GetBuffer();
			InsertMenuItem(hFileMenu,0,TRUE,&mii);
			mii.wID=IDM_REMOVEFROMTHISLIST;
			str.LoadString(IDS_REMOVEFROMTHISLIST);
			mii.dwTypeData=str.GetBuffer();
			InsertMenuItem(hFileMenu,1,TRUE,&mii);
			mii.fMask=MIIM_TYPE;
			mii.fType=MFT_SEPARATOR;
			InsertMenuItem(hFileMenu,2,TRUE,&mii);
			return hFileMenu;
		}
	}

	if (hFileMenu==NULL)
		hFileMenu=CreatePopupMenu();
	InsertMenuItemsFromTemplate(hFileMenu,m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUPLAIN),0,IDM_DEFOPEN);
	
	return hFileMenu;
	
}
	
IContextMenu* CLocateDlg::GetContextMenuForFiles(LPCSTR szParent,CArrayFP<CString*>& aFiles)
{
	IContextMenu* pContextMenu;

	// Creating shell link
	IShellLink* pShellLink;
	HRESULT hRes=CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID*)&pShellLink);
	if (!SUCCEEDED(hRes))
		return NULL;
	
	// And resolving parent ID list
	LPITEMIDLIST pParentIDList;
	hRes=pShellLink->SetPath(szParent);
	if (!SUCCEEDED(hRes))
	{
		pShellLink->Release();
		return NULL;
	}
	hRes=pShellLink->GetIDList(&pParentIDList);
	if (!SUCCEEDED(hRes))
	{
		pShellLink->Release();
		return NULL;
	}
	
	// Creating Itemidlist struct
	LPITEMIDLIST* apidl=new LPITEMIDLIST[aFiles.GetSize()];
	BYTE** apcidl=new BYTE*[aFiles.GetSize()];
	for (int i=0;i<aFiles.GetSize();i++)
	{
		hRes=pShellLink->SetPath(*aFiles.GetAt(i));
		if (!SUCCEEDED(hRes))
		{
			pShellLink->Release();
			return NULL;
		}
		pShellLink->GetIDList(apidl+i);
		apcidl[i]=(BYTE*)apidl[i];
		while (*(SHORT*)(apcidl[i]+*((SHORT*)apcidl[i]))!=0)
			apcidl[i]+=*((SHORT*)apcidl[i]);
		
	}
	pShellLink->Release();
	
	
	// Getting desktop IShellFolder interface
	IShellFolder *pDesktopFolder,*pParentFolder;
	hRes=SHGetDesktopFolder(&pDesktopFolder);
	if (!SUCCEEDED(hRes))
		return NULL;
	
	// Getting parent folder's IShellFolder interface
	hRes=pDesktopFolder->BindToObject(pParentIDList,NULL,IID_IShellFolder,(void**)&pParentFolder);
	if (!SUCCEEDED(hRes))
	{
		pDesktopFolder->Release();
		return NULL;
	}	

	// Getting IContextMenu
	hRes=pParentFolder->GetUIObjectOf(*this,aFiles.GetSize(),(LPCITEMIDLIST*)apcidl,
		IID_IContextMenu,NULL,(void**)&pContextMenu);
	if (!SUCCEEDED(hRes))
		pContextMenu=NULL;
	
	// Releasing memory
	IMalloc* pMalloc;
	if (SHGetMalloc(&pMalloc)==NOERROR)
	{
		pMalloc->Free(pParentIDList);
		for (int i=0;i<aFiles.GetSize();i++)
			pMalloc->Free((void*)apidl[i]);
		pMalloc->Release();
	}
	delete[] apidl;
	delete[] apcidl;
	
	pParentFolder->Release();
	pDesktopFolder->Release();
	return pContextMenu;
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
	
void CLocateDlg::OnDrawItem(UINT idCtl,LPDRAWITEMSTRUCT lpdis)
{
	CDialog::OnDrawItem(idCtl,lpdis);
	
	switch (idCtl)
	{
	case 0:
		{
			// This is IContextMenu item
			if (m_pActiveContextMenu!=NULL && lpdis->itemID<IDM_DEFSENDTOITEM)
				return;

			CDC dc(lpdis->hDC);
			COLORREF backcolor,forecolor;
			
			if (lpdis->itemState&ODS_SELECTED)
			{
				forecolor=GetSysColor(COLOR_HIGHLIGHTTEXT);
				backcolor=GetSysColor(COLOR_HIGHLIGHT);
			}
			else
			{
				forecolor=GetSysColor(COLOR_MENUTEXT);
				backcolor=GetSysColor(COLOR_MENU);
			}
			if (lpdis->itemAction&ODA_SELECT || lpdis->itemAction&ODA_DRAWENTIRE)
			{
				HPEN oldpen;
				HBRUSH oldbrush;
				CBrush brush;
				CPen pen;
				brush.CreateSolidBrush(backcolor);
				pen.CreatePen(PS_SOLID,1,backcolor);
				oldbrush=(HBRUSH)dc.SelectObject(brush);
				oldpen=(HPEN)dc.SelectObject(pen);
				dc.Rectangle(lpdis->rcItem.left,lpdis->rcItem.top+1,lpdis->rcItem.right,lpdis->rcItem.bottom-1);
				dc.SelectObject(oldbrush);
				dc.SelectObject(oldpen);
			}
			SHFILEINFO fi;
			fi.hIcon=NULL;
			HGDIOBJ hOld=dc.SelectObject(m_hSendToListFont);
			SHGetFileInfo((LPCSTR)(lpdis->itemData),0,&fi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME|SHGFI_ICON|SHGFI_SMALLICON);
			dc.SetTextColor(forecolor);
			dc.SetBkColor(backcolor);
			if (fi.hIcon!=NULL)
			{
				dc.DrawState(CPoint(lpdis->rcItem.left+5,lpdis->rcItem.top+(lpdis->rcItem.bottom-lpdis->rcItem.top)/2-8),CSize(16,16),(DRAWSTATEPROC)NULL,(LPARAM)fi.hIcon,DST_ICON);
				DestroyIcon(fi.hIcon);
			}
			CRect rect(lpdis->rcItem.left+27,lpdis->rcItem.top,lpdis->rcItem.right,lpdis->rcItem.bottom);
			dc.DrawText(fi.szDisplayName,-1,&rect,DT_SINGLELINE|DT_VCENTER|DT_LEFT);
			dc.SelectObject(hOld);

			
		}
		break;
	case IDC_STATUS:
		{
			switch(lpdis->itemID)
			{
			case 2:
			case 3:
				if (lpdis->itemData!=NULL)
				{
					DrawIconEx(lpdis->hDC,lpdis->rcItem.left+1,lpdis->rcItem.top,
						(HICON)lpdis->itemData,16,16,NULL,NULL,DI_NORMAL);
				}
				break;
			}		
			break;
		}
	}
}

void CLocateDlg::OnSendToCommand(WORD wID)
{
	CWaitCursor wait;
	CString SendToPath;
	CString m_nSelectedFiles;
	CRegKey BaseKey;
	MENUITEMINFO mii;
	CLSID clsid;

	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_DATA;
	if (m_hActivePopupMenu!=NULL)
		GetMenuItemInfo(m_hActivePopupMenu,wID,FALSE,&mii);
	else
		GetMenuItemInfo(GetSubMenu(GetMenu(),0),wID,FALSE,&mii);

	SendToPath=(LPCSTR)mii.dwItemData;
	if (GetFileClassID(SendToPath,clsid,"DropHandler"))
		SendFiles(SendToPath,m_pListCtrl,clsid);
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
			delete[] (LPSTR)mii.dwItemData;
		wID++;
	}
}

BOOL CLocateDlg::GetFileClassID(CString& file,CLSID& clsid,LPCSTR szType)
{
	CRegKey BaseKey;
	CString Key;
	int i=file.FindLast('.');
	if (i>=0)
		Key=(LPCSTR)file+i;
	if (BaseKey.OpenKey(HKCR,Key,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return FALSE;
	BaseKey.QueryValue(szEmpty,Key);
	Key << "\\ShellEx\\" << szType;
	if (BaseKey.OpenKey(HKCR,Key,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return FALSE;
	BaseKey.QueryValue(szEmpty,Key);
	if (Key.IsEmpty())
		return FALSE;
	WORD olestr[50];
	MultiByteToWideChar(CP_ACP,0,Key,Key.GetLength()+1,olestr,50);
	return CLSIDFromString(olestr,&clsid)==NOERROR;
}

BOOL CLocateDlg::SendFiles(CString& dst,CListCtrl* pList,CLSID& clsid)
{
	OleInitialize(NULL);

	HRESULT hres;
	IUnknown* punk;
	
	// Creating instance
	hres=CoCreateInstance(clsid,NULL,CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER|CLSCTX_LOCAL_SERVER,IID_IUnknown,(void**)&punk);
	if (SUCCEEDED(hres))
	{
		// Creating file objects and ItemIDList
		CFileObject *pfoSrc=new CFileObject,*pfoDst=new CFileObject;
		pfoSrc->AutoDelete();
		pfoDst->AutoDelete();
		pfoSrc->AddRef();
		pfoDst->AddRef();
		pfoDst->SetFile(dst);
		pfoSrc->SetFiles(pList);
		
		// Initializing IShellExtInit if interface exists
		IShellExtInit* psxi=NULL;
		hres=punk->QueryInterface(IID_IShellExtInit,(void**)&psxi);
		if (SUCCEEDED(hres))
		{
			HGLOBAL lpilDst=pfoDst->GetItemIDList();
			hres=psxi->Initialize(NULL,pfoDst,NULL);
			GlobalFree(lpilDst);
		}
			
		// Creating DropTarget object
		IDropTarget* pdt;
		hres=punk->QueryInterface(IID_IDropTarget,(void**)&pdt);
		if (SUCCEEDED(hres))
		{
			DWORD de=DROPEFFECT_COPY;
			POINTL pt={0,0};
			
			// Neseccary?
			pdt->DragEnter(pfoSrc,MK_LBUTTON,pt,&de);
			
			// Drop file
			hres=pdt->Drop(pfoSrc,MK_LBUTTON,pt,&de);
			
			// Releasing DropTarget
			pdt->Release();
		}

		// Releasing IShellExtInit if exists
		if (psxi!=NULL)
			psxi->Release();
		
		// Releasing FileObjects
		pfoSrc->Release();
		pfoDst->Release();

		// Releasing IUnknown
		punk->Release();
	}
	
	OleUninitialize();
	return SUCCEEDED(hres);
}

BYTE CLocateDlg::BeginDragFiles(CListCtrl* pList)
{
	DebugMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList) BEGIN");
	
	OleInitialize(NULL);

	CFileTarget* pTarget=new CFileTarget;
	pTarget->AutoDelete();
	pTarget->AddRef();
	RegisterDragDrop(*m_pListCtrl,pTarget);
	CFileObject* pfo=new CFileObject;
	pfo->AutoDelete();
	pfo->AddRef();
	CFileSource fs;
	DWORD nEffect;
	pfo->SetFiles(pList);
	
	DebugMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList): DoDragDrop is about to be called");
	HRESULT hRes=DoDragDrop(pfo,&fs,DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK|DROPEFFECT_SCROLL,&nEffect);
	DebugFormatMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList): DoDragDrop returned %X",hRes);
	
	// Updating selected items
	int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
		DebugFormatMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList): updating item %X",pItem);
	
		// Just disabling flags, let background thread do the rest
		pItem->RemoveFlags(LITEM_COULDCHANGE);
		DebugMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList): removed flags");
	
		if (m_pBackgroundUpdater!=NULL)
		{
			m_pBackgroundUpdater->AddToUpdateList(pItem,nItem,Needed);

			DebugMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList): calling m_pBackgroundUpdater->StopWaiting()");
			m_pBackgroundUpdater->StopWaiting();
		}
		nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
	}
	pfo->Release();
	

	DebugMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList): calling RevokeDragDrop(*m_pListCtrl)");
	RevokeDragDrop(*m_pListCtrl);
	pTarget->Release();

	OleUninitialize();
	DebugMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList) END");
	return TRUE;
}


BOOL CLocateDlg::ResolveSystemLVStatus()
{
	m_dwFlags=(m_dwFlags&~fgLVStyleFlag)|fgLVStyleSystemDefine;

	
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD temp[7]={2,0x08};
		RegKey.QueryValue("ShellState",(LPTSTR)temp,7*4);
		if (LOBYTE(temp[1])==0x41 || LOBYTE(temp[1])==0x1)
		{
			m_dwFlags|=fgLVStyleSingleClick;
			RegKey.QueryValue("IconUnderline",(LPTSTR)temp,4);
			if (temp[0]==2)
				m_dwFlags|=fgLVStyleUnderLine;
			else if (temp[0]==3)
			{
				CRegKey Key;
				if (Key.OpenKey(HKCU,"Software\\Microsoft\\Internet Explorer\\Main",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
				{
					CString str;
					Key.QueryValue("Anchor Underline",str);
					if (str.CompareNoCase("yes")==0)
						m_dwFlags|=fgLVStyleAlwaysUnderline;
					else if (str.CompareNoCase("hover")==0)
						m_dwFlags|=fgLVStyleUnderLine;
				}
			}
		}
	}
	else
		return FALSE;
	return TRUE;
}

BOOL CLocateDlg::SetListSelStyle()
{
	if (m_pListCtrl==NULL)
		return FALSE;
	if (m_dwFlags&fgLVStyleSystemDefine)
		ResolveSystemLVStatus();
	if (m_dwFlags&fgLVStyleSingleClick)
	{
		if ((m_dwFlags&fgLVStyleAlwaysUnderline)==fgLVStyleAlwaysUnderline)
			m_pListCtrl->SetExtendedListViewStyle(
				LVS_EX_UNDERLINECOLD|LVS_EX_UNDERLINEHOT|LVS_EX_TWOCLICKACTIVATE|LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT,
				LVS_EX_UNDERLINECOLD|LVS_EX_UNDERLINEHOT|LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT);
		else if ((m_dwFlags&fgLVStyleAlwaysUnderline)==fgLVStyleUnderLine)
			m_pListCtrl->SetExtendedListViewStyle(
				LVS_EX_UNDERLINECOLD|LVS_EX_UNDERLINEHOT|LVS_EX_TWOCLICKACTIVATE|LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT,
				LVS_EX_UNDERLINEHOT|LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT);
		else
			m_pListCtrl->SetExtendedListViewStyle(
				LVS_EX_UNDERLINECOLD|LVS_EX_UNDERLINEHOT|LVS_EX_TWOCLICKACTIVATE|LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT,
				LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT);
	}		
	else
		m_pListCtrl->SetExtendedListViewStyle(
			LVS_EX_UNDERLINECOLD|LVS_EX_UNDERLINEHOT|LVS_EX_TWOCLICKACTIVATE|LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT,0);
	
	return TRUE;
}

BOOL CLocateDlg::StartLocateAnimation()
{
	if (m_pLocateAnimBitmaps==NULL)
	{
		m_pLocateAnimBitmaps=new HICON[6];
		m_pLocateAnimBitmaps[0]=(HICON)LoadImage(IDI_LANIM1,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pLocateAnimBitmaps[1]=(HICON)LoadImage(IDI_LANIM2,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pLocateAnimBitmaps[2]=(HICON)LoadImage(IDI_LANIM3,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pLocateAnimBitmaps[3]=(HICON)LoadImage(IDI_LANIM4,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pLocateAnimBitmaps[4]=(HICON)LoadImage(IDI_LANIM5,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pLocateAnimBitmaps[5]=(HICON)LoadImage(IDI_LANIM6,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
	}
	SetTimer(ID_LOCATEANIM,200);
	m_nCurLocateAnimBitmap=0;
	m_pStatusCtrl->SetText((LPCSTR)m_pLocateAnimBitmaps[0],2,SBT_OWNERDRAW);
	return TRUE;
}

BOOL CLocateDlg::StopLocateAnimation()
{
	if (m_pLocateAnimBitmaps!=NULL)
	{
		KillTimer(ID_LOCATEANIM);
		for (int i=0;i<6;i++)
			DestroyIcon(m_pLocateAnimBitmaps[i]);
		delete[] m_pLocateAnimBitmaps;
		m_pLocateAnimBitmaps=NULL;
		if (m_pStatusCtrl!=NULL)
			m_pStatusCtrl->SetText(NULL,2,SBT_OWNERDRAW);
	}
	return TRUE;
}
	
BOOL CLocateDlg::StartUpdateAnimation()
{
	if (m_pUpdateAnimBitmaps==NULL)
	{
		m_pUpdateAnimBitmaps=new HICON[13];
		m_pUpdateAnimBitmaps[0]=(HICON)LoadImage(IDI_UANIM1,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[1]=(HICON)LoadImage(IDI_UANIM2,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[2]=(HICON)LoadImage(IDI_UANIM3,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[3]=(HICON)LoadImage(IDI_UANIM4,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[4]=(HICON)LoadImage(IDI_UANIM5,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[5]=(HICON)LoadImage(IDI_UANIM6,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[6]=(HICON)LoadImage(IDI_UANIM7,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[7]=(HICON)LoadImage(IDI_UANIM8,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[8]=(HICON)LoadImage(IDI_UANIM9,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[9]=(HICON)LoadImage(IDI_UANIM10,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[10]=(HICON)LoadImage(IDI_UANIM11,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[11]=(HICON)LoadImage(IDI_UANIM12,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		m_pUpdateAnimBitmaps[12]=(HICON)LoadImage(IDI_UANIM13,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
		
	}
	SetTimer(ID_UPDATEANIM,100);
	m_nCurUpdateAnimBitmap=0;
	m_pStatusCtrl->SetText((LPCSTR)m_pUpdateAnimBitmaps[0],3,SBT_OWNERDRAW);
	return TRUE;
}

BOOL CLocateDlg::StopUpdateAnimation()
{
	if (m_pUpdateAnimBitmaps!=NULL)
	{
		KillTimer(ID_UPDATEANIM);
		for (int i=0;i<13;i++)
			DestroyIcon(m_pUpdateAnimBitmaps[i]);
		delete[] m_pUpdateAnimBitmaps;
		m_pUpdateAnimBitmaps=NULL;
		if (m_pStatusCtrl!=NULL)
			m_pStatusCtrl->SetText(NULL,3,SBT_OWNERDRAW);
	}
	return TRUE;
}

void CLocateDlg::OnSaveResults()
{
	CSaveResultsDlg SaveResultsDlg;
	
	// Loading previous state
	CRegKey RegKey;
	CString Path;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\General";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("SaveResultsFlags",(LPTSTR)&SaveResultsDlg.m_nFlags,4);
		SaveResultsDlg.m_nFlags&=RESULT_SAVESTATE;

		DWORD dwLength=RegKey.QueryValueLength("SaveResultsDetails");
			
		if (dwLength>0)
		{
			SaveResultsDlg.m_aDetails.SetSize(dwLength/sizeof(int));
			RegKey.QueryValue("SaveResultsDetails",
				(LPSTR)(LPINT)SaveResultsDlg.m_aDetails,dwLength);
		}

		RegKey.CloseKey();
	}

	
	// Activating selected items feature if possible
	if (m_pListCtrl->GetSelectedCount()>0)
		SaveResultsDlg.m_nFlags|=RESULT_ACTIVATESELECTEDITEMS;

	// Opening dialog
	if (!SaveResultsDlg.DoModal(*this))
		return;

	// Activating wait cursor
	CWaitCursor wait;

	// Saving state
	if(RegKey.OpenKey(HKCU,Path,CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		RegKey.SetValue("SaveResultsFlags",SaveResultsDlg.m_nFlags&RESULT_SAVESTATE);
		RegKey.SetValue("SaveResultsDetails",(LPCTSTR)(const int*)SaveResultsDlg.m_aDetails,
			SaveResultsDlg.m_aDetails.GetSize()*sizeof(int),REG_BINARY);
		RegKey.CloseKey();
	}

	try {
		// Initializing results
		CResults Results(SaveResultsDlg.m_nFlags,SaveResultsDlg.m_strDescription,TRUE);
		Results.Create(m_pListCtrl,SaveResultsDlg.m_aDetails,SaveResultsDlg.m_aDetails.GetSize());
		Results.SaveToFile(SaveResultsDlg.GetFileName());
	}
	catch (CFileException ex)
	{
		char szError[2000];
		ex.GetErrorMessage(szError,2000);
		MessageBox(szError,CString(IDS_ERROR),MB_ICONERROR|MB_OK);
	}
	catch (CException ex)
	{
		char szError[2000];
		ex.GetErrorMessage(szError,2000);
		MessageBox(szError,CString(IDS_ERROR),MB_ICONERROR|MB_OK);
	}
	catch (...)
	{
		char szError[2000];
		CException ex(CException::unknown,GetLastError());
		ex.GetErrorMessage(szError,2000);
		MessageBox(szError,CString(IDS_ERROR),MB_ICONERROR|MB_OK);
	}
}

void CLocateDlg::OnCopyPathToClipboard(BOOL bShortPath)
{
	CString Text;

	int nItems=0;
	int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);

		if (nItems>0)
			Text << "\r\n";
		if (!bShortPath)
			Text<<pItem->GetPath();
		else
		{
			char szPath[MAX_PATH];
			if (GetShortPathName(pItem->GetPath(),szPath,MAX_PATH))
				Text<<szPath;
			else
				Text<<pItem->GetPath();
		}
        nItems++;
		nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
	}
    
	HGLOBAL hMem=GlobalAlloc(GHND,Text.GetLength()+1);
	if (hMem==NULL)
	{
		ShowErrorMessage(IDS_ERRORCANNOTALLOCATE);
		return;
	}

	BYTE* pData=(BYTE*)GlobalLock(hMem);
	CopyMemory(pData,LPCSTR(Text),Text.GetLength()+1);
	GlobalUnlock(hMem);

	if (OpenClipboard())
	{
		EmptyClipboard();
		SetClipboardData(CF_TEXT,hMem);
		CloseClipboard();
	}
}

void CLocateDlg::OnChangeFileName()
{
	CChangeFilenameDlg fnd;
	
	int iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	while (iItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
		if (pItem!=NULL)
		{
			fnd.m_sFileName.Copy(pItem->GetName(),pItem->GetNameLen());
			if (fnd.DoModal(*this))
			{
				pItem->ChangeName(fnd.m_sFileName,fnd.m_sFileName.GetLength());
				m_pListCtrl->RedrawItems(iItem,iItem);
			}
		}
		iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
	}
	m_pListCtrl->UpdateWindow();
}

void CLocateDlg::OnChangeFileNameCase()
{
	CChangeCaseDlg cd;
	if (cd.DoModal(*this))
	{
		int iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
		while (iItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
            if (pItem!=NULL)
			{
				int iLength;
				CString sOldName(pItem->GetName());
				LPSTR szName=pItem->GetName();

				if (!cd.bForExtension)
				{
					iLength=LastCharIndex(szName,'.')+1;
					if (iLength==0)
						iLength=pItem->GetNameLen();
				}
				else
					iLength=pItem->GetNameLen();

				switch (cd.nSelectedCase)
				{
				case CChangeCaseDlg::Sentence:
					CharLowerBuff(szName+1,iLength-1);
					CharUpperBuff(szName,1);
					break;
				case CChangeCaseDlg::Uppercase:
					CharUpperBuff(szName,iLength);
					break;
				case CChangeCaseDlg::Lowercase:
					CharLowerBuff(szName,iLength);
					break;
				case CChangeCaseDlg::Title:
					for (int i=0;i<iLength;)
					{
						CharUpperBuff(szName+(i++),1);
						
						int nIndex=FirstCharIndex(szName+i,' ');
						if (nIndex==-1)
						{
							CharLowerBuff(szName+i,iLength-i);
							break;
						}
                        						
						CharLowerBuff(szName+i,nIndex);
                        i+=nIndex+1;                        
					}
					break;
				case CChangeCaseDlg::Toggle:
					for (int i=0;i<iLength;i++)
					{
						if (IsCharUpper(szName[i]))
							CharLowerBuff(szName+i,1);
						else if (IsCharLower(szName[i]))
							CharUpperBuff(szName+i,1);
					}
					break;
				}

				MoveFile(pItem->GetPath(),pItem->GetPath());
				pItem->RemoveFlags(LITEM_TITLEOK|LITEM_FILENAMEOK);
				m_pListCtrl->RedrawItems(iItem,iItem);
				

			}
			iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
		}
		m_pListCtrl->UpdateWindow();
	}
}

void CLocateDlg::OnUpdateLocatedItem()
{
	int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
		if (pItem!=NULL)
		{
			DWORD dwExtraTemp=m_dwExtraFlags;
			m_dwExtraFlags|=efEnableItemUpdating;

			for (int nDetail=0;nDetail<=LastType;nDetail++)
				pItem->UpdateByDetail((DetailType)nDetail);
			m_dwExtraFlags=dwExtraTemp;      

			m_pListCtrl->RedrawItems(nItem,nItem);
		}
	
        nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
	}
}	

void CLocateDlg::OnSelectDetails()
{
	CSelectColumndDlg dlg;
	dlg.m_aSelectedCols.SetSize(m_pListCtrl->GetVisibleColumnCount());
	dlg.m_aWidths.SetSize(m_pListCtrl->GetColumnCount()),
	dlg.m_aIDs.SetSize(m_pListCtrl->GetColumnCount()),
	
	m_pListCtrl->GetColumnOrderArray(m_pListCtrl->GetVisibleColumnCount(),
		dlg.m_aSelectedCols.GetData());
	m_pListCtrl->GetColumnWidthArray(m_pListCtrl->GetColumnCount(),
		dlg.m_aWidths.GetData());	
	m_pListCtrl->GetColumnIDArray(m_pListCtrl->GetColumnCount(),
		dlg.m_aIDs.GetData());	

	if (!dlg.DoModal(*this))
		return; // Cancel

	m_pListCtrl->SetColumnOrderArray(dlg.m_aSelectedCols.GetSize(),
		dlg.m_aSelectedCols.GetData());
	m_pListCtrl->SetColumnWidthArray(dlg.m_aWidths.GetSize(),
		dlg.m_aWidths.GetData());
}

void CLocateDlg::SetStartData(const CLocateApp::CStartData* pStartData)
{
	m_NameDlg.SetStartData(pStartData);
	m_SizeDateDlg.SetStartData(pStartData);
	m_AdvancedDlg.SetStartData(pStartData);

	if (pStartData->m_nSorting!=BYTE(-1))
	{
		if (m_dwFlags&fgLargeMode && 
			(pStartData->m_nStatus&CLocateApp::CStartData::statusRunAtStartUp)==0)
		{
			if (m_nSorting!=pStartData->m_nSorting)
			{
				SortItems(DetailType(pStartData->m_nSorting&127),pStartData->m_nSorting&128);
				m_nSorting=pStartData->m_nSorting;
			}
		}
		else
			m_nSorting=pStartData->m_nSorting;
	}

	if (pStartData->m_dwMaxFoundFiles!=DWORD(-1))
		m_dwMaxFoundFiles=pStartData->m_dwMaxFoundFiles;
	
	if (pStartData->m_nStatus&CLocateApp::CStartData::statusRunAtStartUp)
		OnOk();
}



BOOL CLocateDlg::CNameDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	
	LoadRegistry();
	InitDriveBox(TRUE);
	
	RECT rc1,rc2;
	GetWindowRect(&rc1);
	::GetWindowRect(GetDlgItem(IDC_NAME),&rc2);
	m_nFieldLeft=WORD(rc2.left-rc1.left);
	
	::GetWindowRect(GetDlgItem(IDC_BROWSE),&rc2);
	m_nButtonWidth=WORD(rc2.right-rc2.left);
	m_nBrowseTop=WORD(rc2.top-rc1.top);

	::GetWindowRect(GetDlgItem(IDC_MOREDIRECTORIES),&rc2);	
	m_nMoreDirsTop=WORD(rc2.top-rc1.top);
	m_nMoreDirsWidth=BYTE(rc2.right-rc2.left);


	return FALSE;
}

void CLocateDlg::CNameDlg::ChangeNumberOfDirectories(int iNumberOfDirectories)
{
	if (iNumberOfDirectories<=0)
	{
		if (m_pBrowse!=NULL)
		{
			CComboBoxEx LookIn(GetDlgItem(IDC_LOOKIN));
			for (int i=0;i<LookIn.GetCount();i++)
			{
				LPARAM lParam=LookIn.GetItemData(i);
				if (LOWORD(lParam)==Custom)
				{
					LookIn.DeleteItem(i);
					i--;
				}
			}
			
			for (i=0;i<m_nMaxBrowse;i++)
				m_pBrowse[i].Empty();
			delete[] m_pBrowse;
			m_pBrowse=NULL;
		}
		m_nMaxBrowse=0;
		return;
	}
	if (iNumberOfDirectories==m_nMaxBrowse)
		return;

	CString* pBrowseNew=new CString[iNumberOfDirectories];
	for (int i=0;i<iNumberOfDirectories && i<m_nMaxBrowse;i++)
		pBrowseNew[i].Swap(m_pBrowse[i]);
	
	if (iNumberOfDirectories<m_nMaxBrowse)
	{
		for (;i<m_nMaxBrowse;i++)
			m_pBrowse[i].Empty();

		// Removing items from combobox
		CComboBoxEx LookIn(GetDlgItem(IDC_LOOKIN));
		for (i=0;i<LookIn.GetCount();i++)
		{
			LPARAM lParam=LookIn.GetItemData(i);
			if (HIWORD(lParam)>=iNumberOfDirectories && LOWORD(lParam)==Custom)
			{
				LookIn.DeleteItem(i);
				i--;
			}
		}
	}
	delete[] m_pBrowse;
	m_pBrowse=pBrowseNew;
	m_nMaxBrowse=iNumberOfDirectories;
}

BOOL CLocateDlg::CNameDlg::InitDriveBox(BYTE nFirstTime)
{

	COMBOBOXEXITEM ci;
	LPITEMIDLIST idl;
	SHFILEINFO fi;
	CRegKey RegKey;
	CComboBoxEx LookIn(GetDlgItem(IDC_LOOKIN));
	CString temp;

	DebugFormatMessage("CNameDlg::InitDriveBox BEGIN, items in list %d",LookIn.GetCount());

	char szBuf[]="X:\\";
	char Buffer[100];
	LPARAM lParam;
	int nSelection=-1;
	if (!nFirstTime)
	{
		ci.mask=CBEIF_LPARAM;
		ci.iItem=LookIn.GetCurSel();
		if (ci.iItem!=CB_ERR)
		{
			LookIn.GetItem(&ci);
			lParam=ci.lParam;
		}
		else
			lParam=LPARAM(-1);
		LookIn.ResetContent();
	}
	else
		lParam=MAKELPARAM(Everywhere,0);

	LPMALLOC pMalloc;
	SHGetMalloc(&pMalloc);


	LookIn.SetImageList((HIMAGELIST)SHGetFileInfo(szEmpty,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON));

	// Everywhere, icon from Network Neighborhood
	SHGetSpecialFolderLocation(*this,CSIDL_NETWORK,&idl);
	SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_PIDL);
	ci.iImage=fi.iIcon;
	SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_PIDL|SHGFI_OPENICON);
	ci.iSelectedImage=fi.iIcon;
	LoadString(IDS_EVERYWERE,Buffer,100);
	ci.pszText=Buffer;
	ci.iItem=0;
	ci.iIndent=0;
	ci.lParam=MAKELPARAM(Everywhere,Original);
	if (lParam==ci.lParam || nSelection==-1)
		nSelection=ci.iItem;
	ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
	LookIn.InsertItem(&ci);
	pMalloc->Free(idl);
	

	// Document folders
	SHGetSpecialFolderLocation(*this,CSIDL_PERSONAL,&idl);
	SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_PIDL);
	ci.iImage=fi.iIcon;
	SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_PIDL|SHGFI_OPENICON);
	ci.iSelectedImage=fi.iIcon;
	LoadString(IDS_DOCUMENTFOLDERS,Buffer,80);
	ci.pszText=Buffer;
	ci.iItem++;
	ci.iIndent=0;
	ci.lParam=MAKELPARAM(Special,Documents);
	if (lParam==ci.lParam)
		nSelection=ci.iItem;
	ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
	LookIn.InsertItem(&ci);
	pMalloc->Free(idl);
	
	// Desktop
	SHGetSpecialFolderLocation(*this,CSIDL_DESKTOP,&idl);
	SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_PIDL);
	ci.iImage=fi.iIcon;
	SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_PIDL|SHGFI_OPENICON);
	ci.iSelectedImage=fi.iIcon;
	SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME|SHGFI_PIDL);
	ci.pszText=fi.szDisplayName;
	ci.iItem++;
	ci.iIndent=1;
	ci.lParam=MAKELPARAM(Special,Desktop);
	if (lParam==ci.lParam)
		nSelection=ci.iItem;
	ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
	LookIn.InsertItem(&ci);
	pMalloc->Free(idl);
	
	// My Documents
	temp.Empty();
	if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("Personal",temp);
		RegKey.CloseKey();
	}	
	if	(CFile::IsDirectory(temp))
	{
		SHGetFileInfo(temp,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
		ci.iImage=fi.iIcon;
		SHGetFileInfo(temp,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
		ci.iSelectedImage=fi.iIcon;
		SHGetFileInfo(temp,0,&fi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);
		ci.pszText=fi.szDisplayName;
		ci.iItem++;
		ci.iIndent=1;
		ci.lParam=MAKELPARAM(Special,Personal);
		if (lParam==ci.lParam)
			nSelection=ci.iItem;
		ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
		LookIn.InsertItem(&ci);
	}
	
	// My Computer
	SHGetSpecialFolderLocation(*this,CSIDL_DRIVES,&idl);
	SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_PIDL);
	ci.iImage=fi.iIcon;
	SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_PIDL|SHGFI_OPENICON);
	ci.iSelectedImage=fi.iIcon;
	SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME|SHGFI_PIDL);
	ci.pszText=fi.szDisplayName;
	ci.iItem++;
	ci.iIndent=0;
	ci.lParam=MAKELPARAM(Special,MyComputer);
	if (lParam==ci.lParam)
		nSelection=ci.iItem;
	ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
	LookIn.InsertItem(&ci);
	pMalloc->Free(idl);
	
	// Drives
	CArrayFAP<LPSTR> aRoots;
	CDatabaseInfo::GetRootsFromDatabases(aRoots,GetLocateApp()->GetDatabases());
	
	for (int j=0;j<aRoots.GetSize();j++)
	{
		BOOL bAdd=TRUE;
		if (aRoots.GetAt(j)[0]=='\\' && aRoots.GetAt(j)[1]=='\\')
			continue;

		szBuf[0]=aRoots.GetAt(j)[0];
		CharUpper(szBuf);
		if (GetDriveType(szBuf)<2)
			continue;
		
		// Checking whether drive exists
		for (int k=0;k<j;k++)
		{
			char cDrive=aRoots.GetAt(k)[0];
			CharUpperBuff(&cDrive,1);

			if (cDrive==szBuf[0])
			{
				bAdd=FALSE;
				break;
			}
		}
		if (!bAdd)
			continue;

		SHGetFileInfo(szBuf,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
		ci.iImage=fi.iIcon;
		SHGetFileInfo(szBuf,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
		ci.iSelectedImage=fi.iIcon;
		SHGetFileInfo(szBuf,0,&fi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME);
		ci.pszText=fi.szDisplayName;
		ci.iItem++;
		ci.iIndent=1;
		ci.lParam=MAKELPARAM(Drive,szBuf[0]);
		if (lParam==ci.lParam)
			nSelection=ci.iItem;
		ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
		LookIn.InsertItem(&ci);

		DebugFormatMessage("DriveList: drive %s added, j=%d",aRoots.GetAt(j),j);
	}

	
	if ((GetLocateDlg()->GetFlags()&CLocateDlg::fgNameRootFlag)!=CLocateDlg::fgNameDontAddRoots)
	{
		if ((GetLocateDlg()->GetFlags()&CLocateDlg::fgNameRootFlag)==CLocateDlg::fgNameAddEnabledRoots)
		{
			// Currently aRoots contains all roots
			aRoots.RemoveAll();
			CDatabaseInfo::GetRootsFromDatabases(aRoots,GetLocateApp()->GetDatabases(),TRUE);	
		}

		// Get drives which are already added
		CArray<char> aCurrentlyAddedDrives;
		{
			COMBOBOXEXITEM ci;
			ci.mask=CBEIF_LPARAM;
			for (ci.iItem=LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
			{
				LookIn.GetItem(&ci);
				if (static_cast<TypeOfItem>(LOWORD(ci.lParam))==Drive)
					aCurrentlyAddedDrives.Add((char)HIWORD(ci.lParam));
			}
			// Make drives upper
			CharUpperBuff(aCurrentlyAddedDrives.GetData(),aCurrentlyAddedDrives.GetSize());
		}
		
		for (int i=0;i<aRoots.GetSize();)
		{
			LPCSTR pRoot=aRoots[i];
			if (pRoot[1]==':' && pRoot[2]=='\0')
			{
				char cDrive=pRoot[0];
				CharUpperBuff(&cDrive,1);
				
				// Checking whether drive is already added
				for (int j=0;j<aCurrentlyAddedDrives.GetSize() && cDrive!=aCurrentlyAddedDrives[j];j++);
				
				if (j<aCurrentlyAddedDrives.GetSize())
				{
					aRoots.RemoveAt(i);
                    continue;
				}
			}

			i++;
		}


		// Checking which one are non-local drives
		if (aRoots.GetSize()>0)
		{
			SHGetSpecialFolderLocation(*this,CSIDL_NETWORK,&idl);
			SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_PIDL);
			ci.iImage=fi.iIcon;
			SHGetFileInfo((LPSTR)idl,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_PIDL|SHGFI_OPENICON);
			ci.iImage=fi.iIcon;
			SHGetFileInfo(m_pBrowse[0],0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
			ci.iSelectedImage=fi.iIcon;
			LoadString(IDS_ROOTS,Buffer,100);
			ci.pszText=Buffer;
			ci.iItem++;
			ci.iIndent=0;
			ci.lParam=MAKELPARAM(Everywhere,RootTitle);
			if (lParam==ci.lParam)
				nSelection=ci.iItem;
			ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
			LookIn.InsertItem(&ci);
			pMalloc->Free(idl);

			for (int i=0;i<aRoots.GetSize();i++)
			{
				SHGetFileInfo(aRoots[i],0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
				ci.iImage=fi.iIcon;
				SHGetFileInfo(aRoots[i],0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
				ci.iSelectedImage=fi.iIcon;
				ci.pszText=aRoots[i];
				ci.iItem++;
				ci.iIndent=1;
				
				// Computing cheksum from directory
				// which is used to save control state
				ci.lParam=MAKELPARAM(Root,ComputeChecksumFromDir(aRoots[i]));
					
				if (lParam==ci.lParam)
					nSelection=ci.iItem;
				ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
				LookIn.InsertItem(&ci);

				DebugFormatMessage("DriveList: root %s added, i=%d",aRoots.GetAt(i),i);

			}

			
		}
	}
	pMalloc->Release();

	
	if (m_pBrowse!=NULL) // NULL is possible is locate is just closing
	{
		// Remembered directories
		for (j=0;j<m_nMaxBrowse;j++)
		{
			if (m_pBrowse[j].IsEmpty())
				break;;
			SHGetFileInfo(m_pBrowse[j],0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
			ci.iImage=fi.iIcon;
			SHGetFileInfo(m_pBrowse[j],0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
			ci.iSelectedImage=fi.iIcon;
			ci.pszText=m_pBrowse[j].GetBuffer();
			ci.iItem++;
			ci.iIndent=0;
			ci.lParam=MAKELPARAM(Custom,j);
			if (lParam==ci.lParam)
				nSelection=ci.iItem;
			ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
			LookIn.InsertItem(&ci);

			DebugFormatMessage("DriveList: directory %s added",m_pBrowse[j].GetBuffer());
		}
	}

	if (lParam!=LPARAM(-1))
	{
		LookIn.SetCurSel(nSelection);
		LookIn.SetItemText(-1,LookIn.GetItemText(nSelection));
	}

	DebugMessage("CNameDlg::InitDriveBox END");
	return TRUE;
}
	
BOOL CLocateDlg::CNameDlg::SelectByLParam(LPARAM lParam)
{
	CComboBoxEx LookIn(GetDlgItem(IDC_LOOKIN));
	
	COMBOBOXEXITEM ci;
	ci.mask=CBEIF_LPARAM;
	
	for (ci.iItem=LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
	{
		LookIn.GetItem(&ci);
		if (ci.lParam==lParam)
		{
			LookIn.SetCurSel(ci.iItem);
			LookIn.SetItemText(-1,LookIn.GetItemText(ci.iItem));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CLocateDlg::CNameDlg::SelectByRootName(LPCSTR szDirectory)
{
	CComboBoxEx LookIn(GetDlgItem(IDC_LOOKIN));
	
	COMBOBOXEXITEM ci;
	ci.mask=CBEIF_LPARAM;
	
	for (ci.iItem=LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
	{
		LookIn.GetItem(&ci);
		if (static_cast<TypeOfItem>(LOWORD(ci.lParam))==Root)
		{
			char szPath[MAX_PATH+10];
			ci.pszText=szPath;
			ci.cchTextMax=MAX_PATH+10;
			ci.mask=CBEIF_TEXT;
			LookIn.GetItem(&ci);

			if (strcasecmp(ci.pszText,szDirectory)==0)
			{
				LookIn.SetCurSel(ci.iItem);
				LookIn.SetItemText(-1,szPath);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CLocateDlg::CNameDlg::SelectByCustomName(LPCSTR szDirectory)
{
	CComboBoxEx LookIn(GetDlgItem(IDC_LOOKIN));
	
	COMBOBOXEXITEM ci;
	ci.mask=CBEIF_LPARAM;
	
	for (ci.iItem=LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
	{
		LookIn.GetItem(&ci);
		if (static_cast<TypeOfItem>(LOWORD(ci.lParam))==Custom)
		{
			ASSERT(HIWORD(ci.lParam)<m_nMaxBrowse);
			if (strcasecmp(m_pBrowse[HIWORD(ci.lParam)],szDirectory)==0)
			{
				LookIn.SetCurSel(ci.iItem);
				LookIn.SetItemText(-1,m_pBrowse[HIWORD(ci.lParam)]);
				return TRUE;
			}
		}
	}

	LookIn.SetCurSel(-1);
	LookIn.SetItemText(-1,szDirectory);
	return TRUE;
}

BOOL CLocateDlg::CNameDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);
	switch (wID)
	{
	case IDC_BROWSE:
		if (!::IsWindowEnabled(GetDlgItem(IDC_BROWSE)))
			break;
		switch (wNotifyCode)
		{
		case 1:
		case BN_CLICKED:
			OnBrowse();
		case EN_KILLFOCUS:
			::SendMessage(hControl,BM_SETSTYLE,BS_PUSHBUTTON,0);
			::SendDlgItemMessage(GetParent(),IDC_OK,BM_SETSTYLE,BS_DEFPUSHBUTTON,0);
			break;
		}
		break;
	case IDC_NAME:
		switch (wNotifyCode)
		{
		case 1: /* ACCEL or CBN_SELCHANGE */
			if (hControl==NULL)
				SetFocus(wID);
			break;
		case CBN_SETFOCUS:
			::SendMessage(hControl,CB_SETEDITSEL,0,MAKELPARAM(0,-1));
			break;
		}
		break;
	case IDC_TYPE:
		switch (wNotifyCode)
		{
		case 1: /* ACCEL or CBN_SELCHANGE */
			if (hControl==NULL)
				SetFocus(wID);
			else
				GetLocateDlg()->m_AdvancedDlg.ChangeEnableStateForCheck();
			break;
		case CBN_EDITCHANGE:
			GetLocateDlg()->m_AdvancedDlg.ChangeEnableStateForCheck();
			break;
		case CBN_SETFOCUS:
			::SendMessage(hControl,CB_SETEDITSEL,0,MAKELPARAM(0,-1));
			break;
		}
		break;
	case IDC_LOOKIN:
		switch (wNotifyCode)
		{
		case 1: /* ACCEL or CBN_SELCHANGE */
			if (hControl==NULL)
				SetFocus(wID);
		}
		break;	
	case IDC_MOREDIRECTORIES:
		OnMoreDirectories();
		break;
	case IDM_LOOKINNEWSELECTION:
		OnLookInNewSelection();
		break;
	case IDM_LOOKINREMOVESELECTION:
		OnLookInRemoveSelection();
		break;
	case IDM_LOOKINNEXTSELECTION:
	case IDM_LOOKINPREVSELECTION:
		OnLookInNextSelection(wID==IDM_LOOKINNEXTSELECTION);
		break;
	}
	return FALSE;
}

void CLocateDlg::CNameDlg::OnDestroy()
{
	SaveRegistry();	
	CDialog::OnDestroy();

	if (m_pBrowse!=NULL)
	{
		for (int i=0;i<m_nMaxBrowse;i++)
			m_pBrowse[i].Empty();
		delete[] m_pBrowse;
		m_pBrowse=NULL;
	}

	if (m_pMultiDirs!=NULL)
	{
		for (int i=0;m_pMultiDirs[i]!=NULL;i++)
			delete m_pMultiDirs[i];
		delete[] m_pMultiDirs;
		m_pMultiDirs=NULL;
	}
}


BOOL CLocateDlg::CNameDlg::OnOk(CString& sName,CArray<LPSTR>& aExtensions,CArrayFAP<LPSTR>& aDirectories)
{
	DlgDebugMessage("CLocateDlg::CNameDlg::OnOk BEGIN");
	
	CString Buffer;
	CComboBox Name(GetDlgItem(IDC_NAME));
	CComboBox Type(GetDlgItem(IDC_TYPE));
	CString sType;
	
	// Setting recent combobox for name
	GetDlgItemText(IDC_NAME,sName);
	for (int i=Name.GetCount()-1;i>=0;i--)
	{
		Name.GetLBText(i,Buffer);
		if (sName.CompareNoCase(Buffer)==0)
			Name.DeleteString(i);
	}	
	Name.InsertString(0,sName);
	Name.SetText(sName);

	if (Name.GetCount()>10)
		Name.DeleteString(10);

	
	// Setting recent combobox for type
	GetDlgItemText(IDC_TYPE,sType);
	if (::IsWindowEnabled(GetDlgItem(IDC_TYPE)))
	{
		for (i=Type.GetCount()-1;i>=0;i--)
		{
			Type.GetLBText(i,Buffer);
			if (sType.CompareNoCase(Buffer)==0)
				Type.DeleteString(i);
		}	
		Type.InsertString(0,sType);
		Type.SetText(sType);
		if (Type.GetCount()>10)
			Type.DeleteString(10);
	}
	
	// Parsing extensions
	LPCSTR pType=sType;
	for (;pType[0]==' ';pType++);
	while (*pType!='\0')
	{
		for (DWORD nLength=0;pType[nLength]!='\0' && pType[nLength]!=' ';nLength++);		
		aExtensions.Add(alloccopy(pType,nLength));
		pType+=nLength;
		for (;pType[0]==' ';pType++);
	}


	// Resolving directories
	if (m_pMultiDirs!=NULL)
	{
		// Check contradictions
		BOOL bEverywhereSelected=FALSE;
		BOOL bOtherSelected=FALSE;

		for (int i=0;m_pMultiDirs[i]!=NULL;i++)
		{
			if (m_pMultiDirs[i]->bSelected)
			{
				TypeOfItem nType;
				if (!GetDirectoriesForActiveSelection(aDirectories,&nType,(GetKeyState(VK_CONTROL)&0x8000)?TRUE:FALSE))
					return FALSE;

				if (nType==Everywhere)
					bEverywhereSelected=TRUE;
				else
					bOtherSelected=TRUE;

			}
			else
			{
				if (!GetDirectoriesForSelection(aDirectories,m_pMultiDirs[i],FALSE))
					return FALSE;

				if (m_pMultiDirs[i]->nType==Everywhere)
					bEverywhereSelected=TRUE;
				else
					bOtherSelected=TRUE;
			}
		}

		if (bEverywhereSelected && bOtherSelected)
		{
			switch(ShowErrorMessage(IDS_LOOKINCONTRADICTION,IDS_LOOKIN,MB_ICONQUESTION|MB_YESNOCANCEL))
			{
			case IDYES:
				break;
			case IDNO: // Everywhere
				aDirectories.RemoveAll();
				break;
			case IDCANCEL:
				return FALSE;
			}
		}
	}
	else if (!GetDirectoriesForActiveSelection(aDirectories,NULL,(GetKeyState(VK_CONTROL)&0x8000)?TRUE:FALSE))
		return FALSE;

	
	// Setting focus
	SetFocus(IDC_NAME);
	
	DlgDebugMessage("CLocateDlg::CNameDlg::OnOk END");
	return TRUE;
}

DWORD CLocateDlg::CNameDlg::GetCurrentlySelectedComboItem(CComboBoxEx& LookIn) const
{
	DWORD nCurSel=LookIn.GetCurSel();
	
	if (nCurSel!=DWORD(CB_ERR))
	{
		if (IsFullUnicodeSupport())
		{
			WCHAR szTmp1[MAX_PATH],szTmp2[MAX_PATH];
			LookIn.GetItemText(nCurSel,szTmp1,MAX_PATH);
			LookIn.GetItemText(-1,szTmp2,MAX_PATH);
			DebugNumMessage("CNameDlg::OnOk UC: wcscmp(szTmp1,szTmp2)=%d",wcscmp(szTmp1,szTmp2));
			if (wcscmp(szTmp1,szTmp2)!=0)
				nCurSel=DWORD(CB_ERR);
		}
		else
		{
			char szTmp1[MAX_PATH],szTmp2[MAX_PATH];
			LookIn.GetItemText(nCurSel,szTmp1,MAX_PATH);
			LookIn.GetItemText(-1,szTmp2,MAX_PATH);
			DebugNumMessage("CNameDlg::OnOk: szTmp1=%s",DWORD(szTmp1));
			DebugNumMessage("CNameDlg::OnOk: szTmp2=%s",DWORD(szTmp2));
			if (strcmp(szTmp1,szTmp2)!=0)
				nCurSel=DWORD(CB_ERR);
		}
	}	

	return nCurSel;
}

BOOL CLocateDlg::CNameDlg::GetDirectoriesForActiveSelection(CArray<LPSTR>& aDirectories,TypeOfItem* pType,BOOL bNoWarningIfNotExists)
{
	CComboBoxEx LookIn(GetDlgItem(IDC_LOOKIN));
	
	DWORD nCurSel=GetCurrentlySelectedComboItem(LookIn);
	
	if (nCurSel==DWORD(CB_ERR))
	{
		// Getting directory from combo
		DWORD dwBufferSize=MAX_PATH;
		char* pFolder;
		
		DWORD dwLength;
		for(;;)
		{
			pFolder=new char[dwBufferSize];
			if (!LookIn.GetItemText(-1,pFolder,dwBufferSize))
			{
				delete[] pFolder;
				return FALSE;
			}

			dwLength=istrlen(pFolder);
			
			if (dwLength==dwBufferSize-1)
			{
				delete[] pFolder;
				dwBufferSize*=2;
			}
			else
                break;
		}

		BOOL bRet=GetDirectoriesFromCustomText(aDirectories,pFolder,dwLength,TRUE,bNoWarningIfNotExists);
        delete[] pFolder;
		
		if (pType!=NULL)
			*pType=Custom;

		return bRet;
	}
	else
	{
		COMBOBOXEXITEM ci;
		ci.mask=CBEIF_LPARAM;
		ci.iItem=nCurSel;
		LookIn.GetItem(&ci);

		if (pType!=NULL)
			*pType=static_cast<TypeOfItem>(LOWORD(ci.lParam));

		if (static_cast<TypeOfItem>(LOWORD(ci.lParam))==Root)
		{
			char szPath[MAX_PATH+10];
			ci.pszText=szPath;
			ci.cchTextMax=MAX_PATH+10;
			ci.mask=CBEIF_TEXT;
			LookIn.GetItem(&ci);
			AddDirectoryToList(aDirectories,szPath);
			return TRUE;
		}
		else
			return GetDirectoriesFromLParam(aDirectories,ci.lParam);
	}
}

BOOL CLocateDlg::CNameDlg::GetDirectoriesFromCustomText(CArray<LPSTR>& aDirectories,LPCSTR szCustomText,DWORD dwLength,BOOL bCurrentSelection,BOOL bNoWarning)
{
	// Removing spaces and \\ from beginning and end
	LPCSTR pPtr=szCustomText;
	while (*pPtr==' ')
	{
		pPtr++;
		dwLength--;
	}
	while (pPtr[dwLength-1]==' ')
		dwLength--;
	if (pPtr[dwLength-1]=='\\')
		dwLength--;
	if (!CheckAndAddDirectory(pPtr,dwLength,bCurrentSelection,bNoWarning))
		return FALSE;
	AddDirectoryToList(aDirectories,pPtr,dwLength);
	return TRUE;
}

BOOL CLocateDlg::CNameDlg::GetDirectoriesFromLParam(CArray<LPSTR>& aDirectories,LPARAM lParam)
{
	switch (static_cast<TypeOfItem>(LOWORD(lParam)))
	{
	case Special:
		{
			switch (static_cast<SpecialType>(HIWORD(lParam)))
			{
			case Documents:
				{
					CRegKey RegKey;
					if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
					{
						DWORD nLength=RegKey.QueryValueLength("Desktop");
						char* szDir=new char[nLength];
						RegKey.QueryValue("Desktop",szDir,nLength);
						while (szDir[nLength-2]=='\\')
						{
							szDir[nLength-2]='\0';
							nLength--;
						}
						AddDirectoryToList(aDirectories,FREEDATA(szDir));
						
						nLength=RegKey.QueryValueLength("Personal");
						szDir=new char[nLength];
						RegKey.QueryValue("Personal",szDir,nLength);
						while (szDir[nLength-2]=='\\')
						{
							szDir[nLength-2]='\0';
							nLength--;
						}
						AddDirectoryToList(aDirectories,FREEDATA(szDir));
						
					}
					break;
				}
			case Desktop:
				{
					CRegKey RegKey;
					if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
					{
						DWORD nLength=RegKey.QueryValueLength("Desktop");
						char* szDir=new char[nLength+1];
						RegKey.QueryValue("Desktop",szDir,nLength);
						while (szDir[nLength-2]=='\\')
						{
							szDir[nLength-2]='\0';
							nLength--;
						}
						AddDirectoryToList(aDirectories,FREEDATA(szDir));
					}
					break;
				}
			case Personal:
				{
					CRegKey RegKey;
					if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
					{
						DWORD nLength=RegKey.QueryValueLength("Personal");
						char* szDir=new char[nLength+1];
						RegKey.QueryValue("Personal",szDir,nLength);
						while (szDir[nLength-2]=='\\')
						{
							szDir[nLength-2]='\0';
							nLength--;
						}
						AddDirectoryToList(aDirectories,FREEDATA(szDir));
					}
					break;
				}
			case MyComputer:
				{
					DWORD nLength=GetLogicalDriveStrings(0,NULL)+1;
					if (nLength>=2)
					{
						char* szDrives=new char[nLength+1];
						GetLogicalDriveStrings(nLength,szDrives);
						for (LPSTR szDrive=szDrives;szDrive[0]!='\0';szDrive+=4)
						{
							if (GetDriveType(szDrive)!=DRIVE_REMOTE)
								AddDirectoryToList(aDirectories,szDrive,2);
						}
					}
					break;
				}
			}
			break;
		}
	case Drive:
		{
			char* szDir=new char[3];
			szDir[0]=static_cast<char>(HIWORD(lParam));
			szDir[1]=':';
			szDir[2]='\0';
			AddDirectoryToList(aDirectories,FREEDATA(szDir));
			break;
		}
	case Custom:
		AddDirectoryToList(aDirectories,(LPCSTR)m_pBrowse[HIWORD(lParam)],m_pBrowse[HIWORD(lParam)].GetLength());
		break;
	case Root:
		break;
	case Everywhere:
		// None
		// TODO: May be good if all roots are added
		break;
	}
	return TRUE;
}

void CLocateDlg::OnPresets()
{
	HMENU hMenu=CreatePopupMenu();
	int nPresets=0;


	CLocateDlg::InsertMenuItemsFromTemplate(hMenu,GetLocateDlg()->m_Menu.GetSubMenu(SUBMENU_PRESETSELECTION),0);
	
	
	CRegKey RegKey;
	CString Path;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Dialogs\\SearchPresets";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		CRegKey PresetKey;
		char szBuffer[30];
		
		// Creating preset menu list
		MENUITEMINFO mii;
		mii.cbSize=sizeof(MENUITEMINFO);
		mii.wID=IDM_DEFMENUITEM;
		mii.fType=MFT_STRING;
		mii.fMask=MIIM_TYPE|MIIM_ID;
	

		for (int j=0;j<1000;j++)
		{
			wsprintf(szBuffer,"Preset %03d",j);

			if (PresetKey.OpenKey(RegKey,szBuffer,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
				break;

			CString sCurrentName;
			if (!PresetKey.QueryValue("",sCurrentName))
				continue;

			
			// Preset found
			mii.dwTypeData=sCurrentName.GetBuffer();
			mii.wID=IDM_DEFMENUITEM+j;
			
			InsertMenuItem(hMenu,nPresets,TRUE,&mii);

            PresetKey.CloseKey();
			nPresets++;
		}		

		

		RegKey.CloseKey();
	}
	
	if (nPresets>0)
	{
		MENUITEMINFO mii;
		mii.cbSize=sizeof(MENUITEMINFO);
		mii.fType=MFT_SEPARATOR;
		mii.dwTypeData=NULL;
		mii.fMask=MIIM_TYPE;
		InsertMenuItem(hMenu,nPresets,TRUE,&mii);
	}
	else	
		EnableMenuItem(hMenu,IDM_PRESETREMOVE,MF_BYCOMMAND|MF_GRAYED);

	RECT rcButton;
	::GetWindowRect(GetDlgItem(IDC_PRESETS),&rcButton);
		
	SetForegroundWindow();
	int wID=TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,rcButton.left,rcButton.bottom,0,*this,NULL);	
	DestroyMenu(hMenu);

	switch (wID)
	{
	case IDM_PRESETSAVE:
		OnPresetsSave();
		break;
	case IDM_PRESETREMOVE:
		{
			// Dialog handles everything
			CRemovePresetDlg RemovePreset;
			RemovePreset.DoModal();
			break;
		}
	default:
		if (wID>=IDM_DEFMENUITEM)
		{
			ASSERT(wID-IDM_DEFMENUITEM<nPresets);
			OnPresetsSelection(wID-IDM_DEFMENUITEM);
		}
		break;
	}
}

DWORD CLocateDlg::CheckExistenceOfPreset(LPCSTR szName,DWORD* pdwPresets) // Returns index to preset or FFFFFFFF
{
	// First, find free indentifier
	CRegKey RegKey;
	CString Path;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Dialogs\\SearchPresets";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
	{
		if (pdwPresets!=NULL)
			*pdwPresets=0;
		return DWORD(-1);
	}
	DWORD nID=DWORD(-1);

	int nPreset;
	CRegKey RegKey2;
	char szBuffer[30];

	for (nPreset=0;nPreset<1000;nPreset++)
	{
		wsprintf(szBuffer,"Preset %03d",nPreset);

		if (RegKey2.OpenKey(RegKey,szBuffer,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
			break;

		CString sCurrentName;
		if (!RegKey2.QueryValue("",sCurrentName))
		{
			if (sCurrentName.CompareNoCase(szName)==0)
			{
				if (pdwPresets==NULL)
					return DWORD(nPreset);
				nID=nPreset;
			}
		}
		RegKey2.CloseKey();
	}		

	if (pdwPresets!=NULL)
		*pdwPresets=nPreset;

	return nID;
}

void CLocateDlg::OnPresetsSave()
{
	CSavePresetDlg PresetDialog;
	if (!PresetDialog.DoModal(*this))
		return; // Cancel pressed



	DWORD dwPresets;
	DWORD dwID=CheckExistenceOfPreset(PresetDialog.m_sReturnedPreset,&dwPresets);

	if (dwID==DWORD(-1))
		dwID=dwPresets;

	CRegKey MainKey,PresetKey;
	if (MainKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs\\SearchPresets",
		CRegKey::createNew|CRegKey::samAll)!=ERROR_SUCCESS)
		return;

	char szKeyName[30];
	wsprintf(szKeyName,"Preset %03d",dwID);

	// Deleting key if it exists
	MainKey.DeleteKey(szKeyName);
	
	if (PresetKey.OpenKey(MainKey,szKeyName,CRegKey::createNew|CRegKey::samAll)!=ERROR_SUCCESS)
		return;
	
	PresetKey.SetValue("",PresetDialog.m_sReturnedPreset);
	m_NameDlg.SaveControlStates(PresetKey);
	m_SizeDateDlg.SaveControlStates(PresetKey);
	m_AdvancedDlg.SaveControlStates(PresetKey);

}

void CLocateDlg::OnPresetsSelection(int nPreset)
{
	CRegKey RegKey,PresetKey;
	CString Path;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Dialogs\\SearchPresets";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return;

	char szBuffer[30];
	wsprintf(szBuffer,"Preset %03d",nPreset);

	if (PresetKey.OpenKey(RegKey,szBuffer,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return;

	m_NameDlg.LoadControlStates(PresetKey);
	m_NameDlg.EnableItems(TRUE);

	m_SizeDateDlg.LoadControlStates(PresetKey);
	m_SizeDateDlg.EnableItems(TRUE);

	m_AdvancedDlg.LoadControlStates(PresetKey);
	m_AdvancedDlg.EnableItems(TRUE);

}

void CLocateDlg::CSavePresetDlg::OnOK()
{
	GetDlgItemText(IDC_EDIT,m_sReturnedPreset);

	if (m_sReturnedPreset.IsEmpty())
	{
		ShowErrorMessage(IDS_PRESETNAMENOTVALID,IDS_PRESETSAVETITLE,MB_OK|MB_ICONEXCLAMATION);
		SetFocus(IDC_EDIT);
		return;
	}

	DWORD dwID=CLocateDlg::CheckExistenceOfPreset(m_sReturnedPreset,NULL);
	if (dwID!=DWORD(-1))
	{
		CString msg;
		msg.Format(IDS_OVERWRITEPRESET,LPCSTR(m_sReturnedPreset));

		if (MessageBox(msg,CString(IDS_PRESETSAVETITLE),MB_YESNO|MB_ICONQUESTION)==IDNO)
		{
			SetFocus(IDC_EDIT);
			return;
		}
	}
		
	EndDialog(1);
}

void CLocateDlg::CNameDlg::OnMoreDirectories()
{
	if (m_pMultiDirs==NULL)
		return;

	HMENU hMenu=CreatePopupMenu();

	CLocateDlg::InsertMenuItemsFromTemplate(hMenu,GetLocateDlg()->m_Menu.GetSubMenu(SUBMENU_MULTIDIRSELECTION),0);
	
	MENUITEMINFO mii;
	mii.cbSize=sizeof(MENUITEMINFO);
	// Inserting separator
	mii.fMask=MIIM_TYPE|MIIM_ID;
	mii.wID=IDM_DEFMENUITEM;
	mii.fType=MFT_STRING;
	CString str;
	for (int i=0;m_pMultiDirs[i]!=NULL;i++)
	{
		if (m_pMultiDirs[i]->bSelected)
		{
			m_pMultiDirs[i]->SetValuesFromControl(GetDlgItem(IDC_LOOKIN),m_pBrowse,m_nMaxBrowse);
			mii.fMask=MIIM_TYPE|MIIM_ID|MIIM_STATE;
			mii.fState=MFS_ENABLED|MFS_CHECKED;
		}
		else
			mii.fMask=MIIM_TYPE|MIIM_ID;

		char* pString=new char[10+istrlen(m_pMultiDirs[i]->pTitleOrDirectory)];
		wsprintf(pString,"%d: %s",i+1,m_pMultiDirs[i]->pTitleOrDirectory);
		mii.dwTypeData=pString;
	
		if (InsertMenuItem(hMenu,i,TRUE,&mii))
			mii.wID++;

		delete[] pString;
	}

	if (i==1) // Only one item, disabling remove
		EnableMenuItem(hMenu,IDM_LOOKINREMOVESELECTION,MF_BYCOMMAND|MF_GRAYED);

	RECT rcButton;
	::GetWindowRect(GetDlgItem(IDC_MOREDIRECTORIES),&rcButton);
		
	SetForegroundWindow();
	int wID=TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,rcButton.left,rcButton.bottom,0,*this,NULL);	
	DestroyMenu(hMenu);

	switch (wID)
	{
	case IDM_LOOKINNEWSELECTION:
		OnLookInNewSelection();
		break;
	case IDM_LOOKINREMOVESELECTION:
		OnLookInRemoveSelection();
		break;
	case IDM_LOOKINNEXTSELECTION:
	case IDM_LOOKINPREVSELECTION:
		OnLookInNextSelection(wID==IDM_LOOKINNEXTSELECTION);
		break;
	default:
		if (wID>=IDM_DEFMENUITEM)
			OnLookInSelection(wID-IDM_DEFMENUITEM);
		break;
	}
}

void CLocateDlg::CNameDlg::LookInChangeSelection(int nCurrentSelection,int nNewSelection)
{
	// Current selection is chosen
    if (nCurrentSelection==nNewSelection)
		return;

	// Saving current selection
	m_pMultiDirs[nCurrentSelection]->SetValuesFromControl(GetDlgItem(IDC_LOOKIN),m_pBrowse,m_nMaxBrowse);
	m_pMultiDirs[nCurrentSelection]->bSelected=FALSE;
			
	// Set new selection
	if (m_pMultiDirs[nNewSelection]->nType==Root)
		SelectByRootName(m_pMultiDirs[nNewSelection]->pTitleOrDirectory);
	else if (m_pMultiDirs[nNewSelection]->nType==Custom)
		SelectByCustomName(m_pMultiDirs[nNewSelection]->pTitleOrDirectory);
	else
		SelectByLParam(m_pMultiDirs[nNewSelection]->GetLParam(m_pBrowse,m_nMaxBrowse));
	m_pMultiDirs[nNewSelection]->bSelected=TRUE;

	char szName[10];
	wsprintf(szName,"#%d",nNewSelection+1);
	SetDlgItemText(IDC_MOREDIRECTORIES,szName);
}

void CLocateDlg::CNameDlg::OnLookInNextSelection(BOOL bNext)
{
	if (m_pMultiDirs==NULL)
		return;

	// Getting current selection
	int nCurrentSelection=-1,nSelection;
	for (int nDirs=0;m_pMultiDirs[nDirs]!=NULL;nDirs++)
	{
		if (m_pMultiDirs[nDirs]->bSelected)
			nCurrentSelection=nDirs;
	}

	ASSERT(nCurrentSelection!=-1);
		
	if (bNext)
	{
		if (nCurrentSelection+1>=nDirs)
			nSelection=0;
		else
			nSelection=nCurrentSelection+1;
	}
	else
	{
		if (nCurrentSelection<=0)
			nSelection=nDirs-1;
		else
			nSelection=nCurrentSelection-1;
	}

	LookInChangeSelection(nCurrentSelection,nSelection);
}
	
void CLocateDlg::CNameDlg::OnLookInSelection(int nSelection)
{
	if (m_pMultiDirs==NULL)
		return;

	int nCurrentSelection=-1;
	for (int nDirs=0;m_pMultiDirs[nDirs]!=NULL;nDirs++)
	{
		if (m_pMultiDirs[nDirs]->bSelected)
			nCurrentSelection=nDirs;
	}

	ASSERT(nCurrentSelection!=-1);
	
    // Item nSelection does not exists
	if (nSelection>=nDirs)
		return;
	LookInChangeSelection(nCurrentSelection,nSelection);
}


void CLocateDlg::CNameDlg::OnLookInNewSelection()
{
	if (m_pMultiDirs==NULL)
		return;

	int nSelected=0;
	for (int nDirs=0;m_pMultiDirs[nDirs]!=NULL;nDirs++)
	{
		if (m_pMultiDirs[nDirs]->bSelected)
		{
			nSelected=nDirs;
			m_pMultiDirs[nDirs]->bSelected=FALSE;
		}			
	}

	m_pMultiDirs[nSelected]->SetValuesFromControl(GetDlgItem(IDC_LOOKIN),m_pBrowse,m_nMaxBrowse);

    DirSelection** pMultiDirsNew=new DirSelection*[nDirs+2];
	CopyMemory(pMultiDirsNew,m_pMultiDirs,sizeof(DirSelection*)*nDirs);
    delete[] m_pMultiDirs;
	m_pMultiDirs=pMultiDirsNew;

	m_pMultiDirs[nDirs++]=new DirSelection(TRUE);
    m_pMultiDirs[nDirs]=NULL;

	SelectByLParam(MAKELPARAM(Everywhere,Original));

	char szName[10];
	wsprintf(szName,"#%d",nDirs);
	SetDlgItemText(IDC_MOREDIRECTORIES,szName);
}

void CLocateDlg::CNameDlg::OnLookInRemoveSelection()
{
	if (m_pMultiDirs==NULL)
		return;
	
	int nCurrentSelection=-1;
	for (int nDirs=0;m_pMultiDirs[nDirs]!=NULL;nDirs++)
	{
		if (m_pMultiDirs[nDirs]->bSelected)
			nCurrentSelection=nDirs;
	}

    ASSERT(nCurrentSelection!=-1);
	if (nDirs<=1)
		return;

	DirSelection** pMultiDirsNew=new DirSelection*[nDirs];
	CopyMemory(pMultiDirsNew,m_pMultiDirs,sizeof(DirSelection*)*nCurrentSelection);
	CopyMemory(pMultiDirsNew+nCurrentSelection,m_pMultiDirs+nCurrentSelection+1,sizeof(DirSelection*)*(nDirs-nCurrentSelection));
    delete[] m_pMultiDirs;
	m_pMultiDirs=pMultiDirsNew;

	if (m_pMultiDirs[nCurrentSelection]==NULL)
		nCurrentSelection--;

	// Set new selection
	if (m_pMultiDirs[nCurrentSelection]->nType==Root)
		SelectByRootName(m_pMultiDirs[nCurrentSelection]->pTitleOrDirectory);
	else if (m_pMultiDirs[nCurrentSelection]->nType==Custom)
		SelectByCustomName(m_pMultiDirs[nCurrentSelection]->pTitleOrDirectory);
	else
		SelectByLParam(m_pMultiDirs[nCurrentSelection]->GetLParam(m_pBrowse,m_nMaxBrowse));
	m_pMultiDirs[nCurrentSelection]->bSelected=TRUE;

	char szName[10];
	wsprintf(szName,"#%d",nCurrentSelection+1);
	SetDlgItemText(IDC_MOREDIRECTORIES,szName);
}
		

void CLocateDlg::CNameDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType,cx,cy);
	CRect rc;
	::GetWindowRect(GetDlgItem(IDC_NAME),&rc);
	SetDlgItemPos(IDC_NAME,HWND_BOTTOM,0,0,cx-m_nFieldLeft-8,rc.Height(),SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	SetDlgItemPos(IDC_TYPE,HWND_BOTTOM,0,0,cx-m_nFieldLeft-8,rc.Height(),SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	SetDlgItemPos(IDC_BROWSE,HWND_BOTTOM,cx-m_nButtonWidth-8,m_nBrowseTop,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);

	if (GetLocateDlg()->GetFlags()&CLocateDlg::fgNameMultibleDirectories)
	{
		SetDlgItemPos(IDC_LOOKIN,HWND_BOTTOM,0,0,cx-m_nFieldLeft-m_nMoreDirsWidth-11,rc.Height(),SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		SetDlgItemPos(IDC_MOREDIRECTORIES,HWND_BOTTOM,cx-m_nMoreDirsWidth-8,m_nMoreDirsTop,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
	}
	else
		SetDlgItemPos(IDC_LOOKIN,HWND_BOTTOM,0,0,cx-m_nFieldLeft-8,rc.Height(),SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	
}

void CLocateDlg::CNameDlg::OnClear()
{
	EnableDlgItem(IDC_TYPE,TRUE);
	SetDlgItemText(IDC_NAME,szEmpty);
	SetDlgItemText(IDC_TYPE,szEmpty);

	if (m_pMultiDirs!=NULL)
	{
		delete[] m_pMultiDirs;
		
		// Initializing struct
		m_pMultiDirs=new DirSelection*[2];
        m_pMultiDirs[0]=new DirSelection(TRUE);
		m_pMultiDirs[1]=NULL;

		SetDlgItemText(IDC_MOREDIRECTORIES,"#1");
	}
	
	CComboBoxEx LookIn(GetDlgItem(IDC_LOOKIN));
	for (int i=0;LookIn.GetCount();i++)
	{
		if (LookIn.GetItemData(i)==MAKELPARAM(Everywhere,Original))
		{
			LookIn.SetCurSel(i);
			LookIn.SetItemText(-1,LookIn.GetItemText(i));
			break;
		}
	}
}

void CLocateDlg::CNameDlg::SaveRegistry() const
{
	CRegKey RegKey;
	CString Path;
	
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Recent Strings";
	if(RegKey.OpenKey(HKCU,Path,CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		CComboBox Name(GetDlgItem(IDC_NAME));
		CComboBox Type(GetDlgItem(IDC_TYPE));
		CString buffer;
		CString bfr;
		int i;
		
		for (i=Name.GetCount()-1;i>=0;i--)
		{
			bfr="Name";
			bfr<<int(i);
			Name.GetLBText(i,buffer);
			RegKey.SetValue(bfr,buffer);
		}
		
		for (i=Name.GetCount()-1;i<10;i++)
		{
			bfr="Name";
			bfr<<(int)i;
			RegKey.DeleteKey(bfr);
		}
	
		for (i=Type.GetCount()-1;i>=0;i--)
		{
			bfr="Type";
			bfr<<int(i);
			Type.GetLBText(i,buffer);
			RegKey.SetValue(bfr,buffer);
		}
		
		for (i=Type.GetCount()-1;i<10;i++)
		{
			bfr="Type";
			bfr<<(int)i;
			RegKey.DeleteKey(bfr);
		}

		RegKey.SetValue("NumberOfDirectories",m_nMaxBrowse);
		for (i=0;i<m_nMaxBrowse;i++)
		{
			bfr="Directory";
			bfr<<(int)i;
			RegKey.SetValue(bfr,m_pBrowse[i]);
		}
	}


}

void CLocateDlg::CNameDlg::LoadRegistry()
{
	CRegKey RegKey;
	CString Path;
	
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Recent Strings";
	
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		CComboBox NameCombo(GetDlgItem(IDC_NAME));
		CComboBox TypeCombo(GetDlgItem(IDC_TYPE));
		WORD i;
		CString name;
		CString buffer;
		
		for (i=0;i<10;i++)
		{
			name="Name";
			name<<(int)i;
			if (RegKey.QueryValue((LPCSTR)name,buffer))
				NameCombo.AddString(buffer);
		}
		for (i=0;i<10;i++)
		{
			name="Type";
			name<<(int)i;
			if (RegKey.QueryValue((LPCSTR)name,buffer))
				TypeCombo.AddString(buffer);
		}

		RegKey.QueryValue("NumberOfDirectories",(LPSTR)&m_nMaxBrowse,sizeof(int));
		if (m_nMaxBrowse<0)
			m_nMaxBrowse=4;

		if (m_nMaxBrowse>0)
			m_pBrowse=new CString[m_nMaxBrowse];

		for (int i=0;i<m_nMaxBrowse;i++)
		{
			name="Directory";
			name<<(int)i;
			if (!RegKey.QueryValue(name,m_pBrowse[i]))
				m_pBrowse[i].Empty();
		}

	}

	if (m_pBrowse==NULL && m_nMaxBrowse>0)
		m_pBrowse=new CString[m_nMaxBrowse];
}

BOOL CLocateDlg::CNameDlg::CheckAndAddDirectory(LPCSTR pFolder,DWORD dwLength,BOOL bAlsoSet,BOOL bNoWarning)
{
	CString FolderLower(pFolder,dwLength);
	FolderLower.MakeLower();

	COMBOBOXEXITEM ci;
	SHFILEINFO fi;
			
	CArrayFAP<LPSTR> aRoots;
	CDatabaseInfo::GetRootsFromDatabases(aRoots,GetLocateApp()->GetDatabases());
	
	// Checking whether Folder is subdirectory of any aRoots
	BOOL bFound=FALSE;
	for (int i=0;i<aRoots.GetSize();i++)
	{
		CString Path(aRoots.GetAt(i));
		Path.MakeLower();
		
		if (strncmp(Path,FolderLower,min(Path.GetLength(),dwLength))==0)
			bFound=TRUE;
	}
	if (!bFound)
	{
		if (!bNoWarning)
		{
			ShowErrorMessage(IDS_ERRORWRONGDRIVE,IDS_ERROR);	
			return FALSE;
		}
		return TRUE;
	}

	if (m_pBrowse!=NULL)		
	{
		// Checking whether path is drive which already exists
		CComboBoxEx LookIn(GetDlgItem(IDC_LOOKIN));
		if (dwLength==2 && pFolder[1]==':')
		{
			for (i=0;i<LookIn.GetCount();i++)
			{
				LPARAM lParam=LookIn.GetItemData(i);
				if (static_cast<TypeOfItem>(LOWORD(lParam))!=Drive)
				{
					char cDrive=(char)HIWORD(lParam);
					CharLowerBuff(&cDrive,1);
					if (cDrive==FolderLower[0])
					{
						LookIn.SetCurSel(i);
						LookIn.SetItemText(-1,LookIn.GetItemText(i));
						return TRUE;
					}
				}
			}
		}

		// Check whether folder already exists in other directory list
		for (int i=0;i<m_nMaxBrowse;i++)
		{
			CString str(m_pBrowse[i]);
			str.MakeLower();
			if (str.Compare(FolderLower)==0)
			{
				// Deleting previous one
				for (int j=i+1;j<m_nMaxBrowse;j++)
					m_pBrowse[j-1].Swap(m_pBrowse[j]);
				m_pBrowse[m_nMaxBrowse-1].Empty();
			}
		}

		// moving previous folders one down
		for (i=m_nMaxBrowse-1;i>0;i--)
			m_pBrowse[i].Swap(m_pBrowse[i-1]);
		
		m_pBrowse[0].Copy(pFolder,dwLength);

		for (ci.iItem=LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
		{
			ci.mask=CBEIF_LPARAM;
			LookIn.GetItem(&ci);
			if (static_cast<TypeOfItem>(LOWORD(ci.lParam))!=Custom)
				break;
			LookIn.DeleteItem(ci.iItem);
		}
		
		int nSel=++ci.iItem;
		for (i=0;i<m_nMaxBrowse;i++)
		{
			if (m_pBrowse[i].IsEmpty())
				break;
			SHGetFileInfo(m_pBrowse[i],0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
			ci.iImage=fi.iIcon;
			SHGetFileInfo(m_pBrowse[i],0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
			ci.iSelectedImage=fi.iIcon;
			ci.pszText=m_pBrowse[i].GetBuffer();
			ci.iIndent=0;
			ci.lParam=MAKELPARAM(Custom,i);
			ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
			LookIn.InsertItem(&ci);
			ci.iItem++;
		}
			
		if (bAlsoSet)
		{
			LookIn.SetCurSel(nSel);
			LookIn.SetItemText(-1,LookIn.GetItemText(nSel));
		}
	}
	return TRUE;		
}

void CLocateDlg::CNameDlg::OnBrowse()
{
	CWaitCursor wait;
	DebugMessage("CLocateDlg::CNameDlg::OnBrowse() BEGIN");
	CFolderDialog fd(IDS_GETFOLDER,BIF_RETURNONLYFSDIRS|BIF_USENEWUI|BIF_NONEWFOLDERBUTTON);
	if (fd.DoModal(*this))
	{
	

		CString Folder;
		if (fd.GetFolder(Folder))
		{
			DebugFormatMessage("CLocateDlg::CNameDlg::OnBrowse(): Folder=\"%s\"",LPCSTR(Folder));
			
			CheckAndAddDirectory(Folder,Folder.GetLength(),TRUE,FALSE);
		}
		else
		{
			char szName[500];
			if (GetDisplayNameFromIDList(fd.m_lpil,szName,500))
			{
				DebugFormatMessage("CLocateDlg::CNameDlg::OnBrowse(): szName=\"%s\"",szName);
				if (szName[0]=='\\' && szName[1]=='\\')
					CheckAndAddDirectory(szName,istrlen(szName),TRUE,FALSE);
			}
			else
				DebugMessage("CLocateDlg::CNameDlg::OnBrowse(): GetDisplayNameFromIDList failed");
	
		}
		SetFocus(IDC_LOOKIN);

		DebugMessage("CLocateDlg::CNameDlg::OnBrowse() END1");
		return;
	}
	
	DebugMessage("CLocateDlg::CNameDlg::OnBrowse() END2");
}

void CLocateDlg::CNameDlg::EnableItems(BOOL bEnable)
{
	EnableDlgItem(IDC_NAME,bEnable);
	EnableDlgItem(IDC_TYPE,bEnable && GetLocateDlg()->m_AdvancedDlg.SendDlgItemMessage(IDC_FILETYPE,CB_GETCURSEL)==0 );
	EnableDlgItem(IDC_LOOKIN,bEnable);
	EnableDlgItem(IDC_MOREDIRECTORIES,bEnable);
	EnableDlgItem(IDC_BROWSE,bEnable);
}

	
BOOL CLocateDlg::CNameDlg::EnableMultiDirectorySupport(BOOL bEnable)
{
	if (bEnable)
	{
		if (m_pMultiDirs!=NULL)
			return FALSE;

		// Showing control
		ShowDlgItem(IDC_MOREDIRECTORIES,swShow);
		
		// Initializing struct
		m_pMultiDirs=new DirSelection*[2];
        m_pMultiDirs[0]=new DirSelection(TRUE);
		m_pMultiDirs[1]=NULL;
		
		SetDlgItemText(IDC_MOREDIRECTORIES,"#1");
		return TRUE;
	}
	else
	{
		// Showing control
		ShowDlgItem(IDC_MOREDIRECTORIES,swHide);

		if (m_pMultiDirs==NULL)
			return FALSE;

		
		// Freeing memory
		if (m_pMultiDirs!=NULL)
		{
			for (int i=0;m_pMultiDirs[i]!=NULL;i++)
				delete m_pMultiDirs[i];
			delete[] m_pMultiDirs;
			m_pMultiDirs=NULL;
		}
		return TRUE;
	}
}

void CLocateDlg::CNameDlg::SetStartData(const CLocateApp::CStartData* pStartData)
{
	if (pStartData->m_pStartString!=NULL)
		SetDlgItemText(IDC_NAME,pStartData->m_pStartString);
	if (pStartData->m_pTypeString!=NULL)
		SetDlgItemText(IDC_TYPE,pStartData->m_pTypeString);
	if (pStartData->m_pStartPath!=NULL)
		SetPath(pStartData->m_pStartPath);
}

BOOL CLocateDlg::CNameDlg::SetPath(LPCTSTR szPath)
{
	CComboBoxEx LookIn(GetDlgItem(IDC_LOOKIN));
	CString temp;
	LPTSTR tmp;
	int ret=atoi(szPath);
	
	if ((ret>0 && ret<4) || szPath[0]=='0')
	{
		LookIn.SetCurSel(ret);
		LookIn.SetItemText(-1,LookIn.GetItemText(ret));
		return TRUE;	
	}

	ret=GetFullPathName(szPath,_MAX_PATH,temp.GetBuffer(_MAX_PATH),&tmp);
	if (ret>1)
	{
		temp.FreeExtra();
		if (ret<4)
		{
			temp.MakeUpper();
			if (temp[1]==':')
			{
				COMBOBOXEXITEM ci;
				ci.mask=CBEIF_LPARAM;
				for (ci.iItem=LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
				{
					LookIn.GetItem(&ci);
					if (static_cast<TypeOfItem>(LOWORD(ci.lParam))!=Drive)
						continue;
					if ((char)HIWORD(ci.lParam)==temp[0])
					{
						LookIn.SetCurSel(ci.iItem);
						LookIn.SetItemText(-1,LookIn.GetItemText(ci.iItem));
						break;
					}
				}
				return TRUE;	
			}
		}
		else
		{
			if (CFile::IsDirectory(szPath)) 
			{
				SHFILEINFO fi;
				COMBOBOXEXITEM ci;
				m_pBrowse[0]=temp;
				SHGetFileInfo((LPCTSTR)temp,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
				ci.iImage=fi.iIcon;
				SHGetFileInfo((LPCTSTR)temp,0,&fi,sizeof(SHFILEINFO),SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
				ci.iSelectedImage=fi.iIcon;
				ci.pszText=(LPSTR)(LPCSTR)temp;
				ci.iItem=LookIn.GetCount();
				ci.iIndent=0;
				ci.lParam=MAKELPARAM(Custom,0);
				ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
				LookIn.InsertItem(&ci);
				LookIn.SetCurSel(ci.iItem);
				LookIn.SetItemText(-1,temp);
				return TRUE;
			}
		}
	}
	LookIn.SetCurSel(3);
	LookIn.SetItemText(-1,LookIn.GetItemText(3));
	return TRUE;
}

void CLocateDlg::CNameDlg::DirSelection::SetValuesFromControl(HWND hControl,const CString* pBrowse,int nBrowseDirs)
{
	FreeData();

	int nCurSel=::SendMessage((HWND)::SendMessage(hControl,CBEM_GETCOMBOCONTROL,0,0),CB_GETCURSEL,0,0);

	COMBOBOXEXITEM ci;
	ci.mask=CBEIF_LPARAM|CBEIF_TEXT;
	ci.iItem=nCurSel;
	char szTitle[MAX_PATH+10]="";
	ci.pszText=szTitle;
	ci.cchTextMax=MAX_PATH+10;
	


	if (!::SendMessage(hControl,CBEM_GETITEM,0,(LPARAM)&ci))
		return;
	
	if (nCurSel==-1)
	{
		// No selection, custom
		DWORD dwLength;
		dstrlen(szTitle,dwLength);

		if (dwLength<MAX_PATH+9)
		{
            // Whole string is in szTitle
			nType=CLocateDlg::CNameDlg::Custom;
			pTitleOrDirectory=alloccopy(szTitle,dwLength);
		}
		else
		{
            DWORD dwBufferSize=2*MAX_PATH;
			char* pFolder;
		
			for(;;)
			{	
				pFolder=new char[dwBufferSize];
				ci.mask=CBEIF_TEXT;
				ci.pszText=pFolder;

				if (!::SendMessage(hControl,CBEM_GETITEM,0,(LPARAM)&ci))
				{
					delete[] pFolder;
					return;
				}

				dwLength=istrlen(pFolder);
			
				if (dwLength==dwBufferSize-1)
				{
					delete[] pFolder;
					dwBufferSize*=2;
				}
				else
					break;
			}

			pTitleOrDirectory=alloccopy(pFolder,dwLength);
			delete[] pFolder;
		}		
	}
	else
	{
		// Selection
		nType=static_cast<CLocateDlg::CNameDlg::TypeOfItem>(LOWORD(ci.lParam));
		
		if (nType!=CLocateDlg::CNameDlg::Custom)
			pTitleOrDirectory=alloccopy(szTitle);

		switch (nType)
		{
		case CLocateDlg::CNameDlg::Everywhere:
			nEverywhereType=static_cast<CLocateDlg::CNameDlg::EveryWhereType>(HIWORD(ci.lParam));
			break;
		case CLocateDlg::CNameDlg::Special:
			nSpecialType=static_cast<CLocateDlg::CNameDlg::SpecialType>(HIWORD(ci.lParam));
			break;
		case CLocateDlg::CNameDlg::Drive:
			cDriveLetter=static_cast<char>(HIWORD(ci.lParam));
			break;
		case CLocateDlg::CNameDlg::Custom:
			ASSERT(int(HIWORD(ci.lParam))>=0 && HIWORD(ci.lParam)<nBrowseDirs);
			pTitleOrDirectory=alloccopy(pBrowse[HIWORD(ci.lParam)]);
			break;
		case CLocateDlg::CNameDlg::Root:
			break;
		}
	}
}

LPARAM CLocateDlg::CNameDlg::DirSelection::GetLParam(const CString* pBrowse,int nBrowseDirs) const
{
	switch (nType)
	{
	case CLocateDlg::CNameDlg::Everywhere:
		return MAKELPARAM(CLocateDlg::CNameDlg::Everywhere,nEverywhereType);
	case CLocateDlg::CNameDlg::Special:
		return MAKELPARAM(CLocateDlg::CNameDlg::Special,nSpecialType);
	case CLocateDlg::CNameDlg::Drive:
		return MAKELPARAM(CLocateDlg::CNameDlg::Drive,cDriveLetter);
	case CLocateDlg::CNameDlg::Custom:
		for (WORD i=0;i<nBrowseDirs;i++)
		{
			if (pBrowse[0].CompareNoCase(pTitleOrDirectory)==0)
				return MAKELPARAM(CLocateDlg::CNameDlg::Custom,i);
		}
        return MAKELPARAM(CLocateDlg::CNameDlg::Custom,0xFFFF);
	case CLocateDlg::CNameDlg::Root:
		return MAKELPARAM(CLocateDlg::CNameDlg::Root,CLocateDlg::CNameDlg::ComputeChecksumFromDir(pTitleOrDirectory));
	default:
		return MAKELPARAM(CLocateDlg::CNameDlg::NotSelected,0xFFFF);
	}
}

BOOL CLocateDlg::CNameDlg::GetDirectoriesForSelection(CArray<LPSTR>& aDirectories,const DirSelection* pSelection,BOOL bNoWarnings)
{
	switch (pSelection->nType)
	{
	case Everywhere:
		return GetDirectoriesFromLParam(aDirectories,MAKELPARAM(Everywhere,pSelection->nEverywhereType));
	case Special:
		return GetDirectoriesFromLParam(aDirectories,MAKELPARAM(Special,pSelection->nSpecialType));
	case Drive:
		return GetDirectoriesFromLParam(aDirectories,MAKELPARAM(Drive,pSelection->cDriveLetter));
	case Custom:
		return GetDirectoriesFromCustomText(aDirectories,pSelection->pTitleOrDirectory,istrlen(pSelection->pTitleOrDirectory),pSelection->bSelected,bNoWarnings);
	case Root:
		return aDirectories.Add(alloccopy(pSelection->pTitleOrDirectory));
	default:
		return FALSE;
	}
}

void CLocateDlg::CNameDlg::LoadControlStates(CRegKey& RegKey)
{
	CString str;
	RegKey.QueryValue("Name/Name",str);
	SetDlgItemText(IDC_NAME,str);

	RegKey.QueryValue("Name/Type",str);
	SetDlgItemText(IDC_TYPE,str);
	
	if (m_pMultiDirs!=NULL)
	{
		// Checking how many directories is in list
		char szName[200];
		CIntArray aSelections;
		
        for (int i=0;RegKey.EnumValue(i,szName,200);i++)
		{
			if (strncmp("Name/LookIn",szName,11)!=0)
				continue;

			int nSel=atoi(szName+11);
			
			// Checking whethe ID is number
			if (nSel==0 && szName[11]!='0')
				continue;

			aSelections.Add(nSel);
		}
	

        // Sorting, bubble sort
		for (int i=0;i<aSelections.GetSize();i++)
		{
			for (int j=0;j<i;j++)
			{
				if (aSelections[j]>aSelections[i])
				{
					int nTemp=aSelections[j];
					aSelections[j]=aSelections[i];
					aSelections[i]=nTemp;
				}
			}
		}

		delete[] m_pMultiDirs;
		if (aSelections.GetSize())
		{
			m_pMultiDirs=new DirSelection*[aSelections.GetSize()+1];
			m_pMultiDirs[aSelections.GetSize()]=NULL;

			for (int i=aSelections.GetSize()-1;i>=0;i--)
			{
				wsprintf(szName,"Name/LookIn%04d",aSelections[i]);
				
				LONG lLength=RegKey.QueryValueLength(szName);
				if (lLength>4)
				{
					char* pData=new char[lLength+1];
					RegKey.QueryValue(szName,pData,lLength);
					pData[lLength]='\0';
					
					SelectByCustomName(pData+4);
					delete[] pData;
	
				}
				else if (lLength==4) 
				{
					DWORD dwLParam;
					RegKey.QueryValue(szName,dwLParam);
	                
					SelectByLParam(MAKELPARAM(HIWORD(dwLParam),LOWORD(dwLParam)));
				}
				
				m_pMultiDirs[i]=new DirSelection(i==0);
				m_pMultiDirs[i]->SetValuesFromControl(GetDlgItem(IDC_LOOKIN),m_pBrowse,m_nMaxBrowse);
			}
		}
		else
		{
			// It is possible that multidir mode was 
			// not enabled when state is saved, trying it (at least create empty selection)
			m_pMultiDirs=new DirSelection*[2];
			m_pMultiDirs[1]=NULL;
            
			LONG lLength=RegKey.QueryValueLength("Name/LookIn");
			if (lLength>4)
			{
				char* pData=new char[lLength+1];
				RegKey.QueryValue(szName,pData,lLength);
				pData[lLength]='\0';
				
				SelectByCustomName(pData+4);
				delete[] pData;

			}
			else if (lLength==4) 
			{
				DWORD dwLParam;
				RegKey.QueryValue(szName,dwLParam);
                
				SelectByLParam(MAKELPARAM(HIWORD(dwLParam),LOWORD(dwLParam)));
			}
				
			m_pMultiDirs[i]=new DirSelection(TRUE);
			m_pMultiDirs[i]->SetValuesFromControl(GetDlgItem(IDC_LOOKIN),m_pBrowse,m_nMaxBrowse);

		}

		DWORD nSelection=0;
		RegKey.QueryValue("Name/LookInSel",nSelection);

		// Marking #1 selected
		SetDlgItemText(IDC_MOREDIRECTORIES,"#1");

		OnLookInSelection(nSelection);
	}
	else
	{
        LONG lLength=RegKey.QueryValueLength("Name/LookIn");
		if (lLength<=0)
		{
			// It is possible that multidirecory mode was enabled
			// when state is saved
			lLength=RegKey.QueryValueLength("Name/LookIn0000");
		}

		if (lLength>=4) // Must be at least 4 bytes long
		{
	        CComboBoxEx LookInEx(GetDlgItem(IDC_LOOKIN));
		
			char* pData=new char[lLength+1];
			RegKey.QueryValue("Name/LookIn",pData,lLength);
			if (*((WORD*)pData)==CNameDlg::TypeOfItem::NotSelected)
			{
				LookInEx.SetCurSel(-1);
				pData[lLength]='\0';
				LookInEx.SetItemText(-1,pData+4);
			}
			else
			{
				LPARAM lParam=MAKELPARAM(((WORD*)pData)[1],((WORD*)pData)[0]);
				for (int i=0;i<LookInEx.GetCount();i++)
				{
					if (lParam==LookInEx.GetItemData(i))
					{
						LookInEx.SetCurSel(i);
						LookInEx.SetItemText(-1,LookInEx.GetItemText(i));
						break;
					}
				}
			}
		}
	}

}

void CLocateDlg::CNameDlg::SaveControlStates(CRegKey& RegKey)
{
	CString str;
		
	// Name dialog
	GetDlgItemText(IDC_NAME,str);
	RegKey.SetValue("Name/Name",str);
	str.Empty();
	GetDlgItemText(IDC_TYPE,str);
	RegKey.SetValue("Name/Type",str);
	
	// Lookin Combo
	
	// Deleting old items
	char szName[200];
	for (int i=0;RegKey.EnumValue(i,szName,200);)
	{
		if (strncmp("Name/LookIn",szName,11)==0)
			RegKey.DeleteValue(szName);
		else
			i++;
	}

	CComboBoxEx LookInEx(GetDlgItem(IDC_LOOKIN));
		
	if (m_pMultiDirs==NULL)
	{
		int nCurSel=LookInEx.GetCurSel();
		if (nCurSel==CB_ERR)
		{
			LookInEx.GetItemText(-1,str.GetBuffer(2000),2000);
			str.FreeExtra();
			char* pData=new char[str.GetLength()+4];
			*((DWORD*)pData)=DWORD(TypeOfItem::NotSelected);
			CopyMemory(pData+4,LPCSTR(str),str.GetLength());
			RegKey.SetValue("Name/LookIn",pData,str.GetLength()+4,REG_BINARY);
		}
		else
		{
			LPARAM lParam=LookInEx.GetItemData(nCurSel);
			RegKey.SetValue("Name/LookIn",MAKELPARAM(HIWORD(lParam),LOWORD(lParam)));            
		}
	}
	else
	{
		for (int i=0;m_pMultiDirs[i]!=NULL;i++)
		{
			wsprintf(szName,"Name/LookIn%04d",i);
					
			if (m_pMultiDirs[i]->bSelected)
			{
				// Current selection
				int nCurSel=LookInEx.GetCurSel();
				if (nCurSel==CB_ERR)
				{
					LookInEx.GetItemText(-1,str.GetBuffer(2000),2000);
					str.FreeExtra();
					char* pData=new char[str.GetLength()+4];
					*((DWORD*)pData)=DWORD(TypeOfItem::NotSelected);
					CopyMemory(pData+4,LPCSTR(str),str.GetLength());
					RegKey.SetValue(szName,pData,str.GetLength()+4,REG_BINARY);
				}
				else
				{
					LPARAM lParam=LookInEx.GetItemData(nCurSel);
					RegKey.SetValue(szName,MAKELPARAM(HIWORD(lParam),LOWORD(lParam)));            
				}

				RegKey.SetValue("Name/LookInSel",DWORD(i));
			}
			else if (m_pMultiDirs[i]->nType==Custom)
			{	
				DWORD dwLength;
				dstrlen(m_pMultiDirs[i]->pTitleOrDirectory,dwLength);
				
				char* pData=new char[dwLength+4];
				*((DWORD*)pData)=DWORD(TypeOfItem::NotSelected);
				CopyMemory(pData+4,m_pMultiDirs[i]->pTitleOrDirectory,dwLength);
				RegKey.SetValue(szName,pData,dwLength+4,REG_BINARY);
			}
			else
			{
				LPARAM lParam=m_pMultiDirs[i]->GetLParam(m_pBrowse,m_nMaxBrowse);
				RegKey.SetValue(szName,MAKELPARAM(HIWORD(lParam),LOWORD(lParam)));
			}


		}
	}
}


BOOL CLocateDlg::CSizeDateDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	SendDlgItemMessage(IDC_MINIMUMSIZESPIN,UDM_SETBUDDY,(WPARAM)GetDlgItem(IDC_MINIMUMSIZE),0);
	SendDlgItemMessage(IDC_MAXIMUMSIZESPIN,UDM_SETBUDDY,(WPARAM)GetDlgItem(IDC_MAXIMUMSIZE),0);
	SendDlgItemMessage(IDC_MINIMUMSIZESPIN,UDM_SETRANGE,0,MAKELPARAM(UD_MAXVAL,0));
	SendDlgItemMessage(IDC_MAXIMUMSIZESPIN,UDM_SETRANGE,0,MAKELPARAM(UD_MAXVAL,0));
	UDACCEL ua[] = {
		{ 1,1 },
		{ 2,5 },
		{ 3,10 },
		{ 4,50 },
		{ 5,250 },
		{ 6,500 },
		{ 7,1000 },
		{ 8,2000 },
		{ 9,5000 },
		{ 10, 10000 }
	};
	SendDlgItemMessage(IDC_MINIMUMSIZESPIN,UDM_SETACCEL,10,(LPARAM)ua);
	SendDlgItemMessage(IDC_MAXIMUMSIZESPIN,UDM_SETACCEL,10,(LPARAM)ua);
	CString str;
	str.LoadString(IDS_MODIFIED);
	SendDlgItemMessage(IDC_MINTYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	SendDlgItemMessage(IDC_MAXTYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	str.LoadString(IDS_CREATED);
	SendDlgItemMessage(IDC_MINTYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	SendDlgItemMessage(IDC_MAXTYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	str.LoadString(IDS_LASTACCESSED);
	SendDlgItemMessage(IDC_MINTYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	SendDlgItemMessage(IDC_MAXTYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	str.LoadString(IDS_SIZEBYTES);
	SendDlgItemMessage(IDC_MINSIZETYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	SendDlgItemMessage(IDC_MAXSIZETYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	str.LoadString(IDS_SIZEKB);
	SendDlgItemMessage(IDC_MINSIZETYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	SendDlgItemMessage(IDC_MAXSIZETYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	str.LoadString(IDS_SIZEMB);
	SendDlgItemMessage(IDC_MINSIZETYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	SendDlgItemMessage(IDC_MAXSIZETYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)str);
	OnClear();
	return FALSE;
}

BOOL CLocateDlg::CSizeDateDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);
	switch (wID)
	{
	case IDC_CHECKMINIMUMSIZE:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
			CheckDlgButton(IDC_CHECKMINIMUMSIZE,!IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE));

		if (IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE))
		{
			EnableDlgItem(IDC_MINIMUMSIZE,TRUE);
			EnableDlgItem(IDC_MINIMUMSIZESPIN,TRUE);
			EnableDlgItem(IDC_MINSIZETYPE,TRUE);
			SetFocus(IDC_MINIMUMSIZE);
		}
		else
		{
			EnableDlgItem(IDC_MINIMUMSIZE,FALSE);
			EnableDlgItem(IDC_MINIMUMSIZESPIN,FALSE);
			EnableDlgItem(IDC_MINSIZETYPE,FALSE);

			if (hControl==NULL && wNotifyCode==1) // Accelerator
				SetFocus(IDC_CHECKMINIMUMSIZE);

		}
		GetLocateDlg()->m_AdvancedDlg.ChangeEnableStateForCheck();
		break;
	case IDC_CHECKMAXIMUMSIZE:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
			CheckDlgButton(IDC_CHECKMAXIMUMSIZE,!IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE));

		if (IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE))
		{
			EnableDlgItem(IDC_MAXIMUMSIZE,TRUE);
			EnableDlgItem(IDC_MAXIMUMSIZESPIN,TRUE);
			EnableDlgItem(IDC_MAXSIZETYPE,TRUE);
			SetFocus(IDC_MAXIMUMSIZE);
		}
		else
		{
			EnableDlgItem(IDC_MAXIMUMSIZE,FALSE);
			EnableDlgItem(IDC_MAXIMUMSIZESPIN,FALSE);
			EnableDlgItem(IDC_MAXSIZETYPE,FALSE);

			if (hControl==NULL && wNotifyCode==1) // Accelerator
				SetFocus(IDC_CHECKMAXIMUMSIZE);
		}
		GetLocateDlg()->m_AdvancedDlg.ChangeEnableStateForCheck();
		break;
	case IDC_CHECKMINDATE:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
			CheckDlgButton(IDC_CHECKMINDATE,!IsDlgButtonChecked(IDC_CHECKMINDATE));

		if (IsDlgButtonChecked(IDC_CHECKMINDATE))
		{
			EnableDlgItem(IDC_MINDATE,TRUE);
			EnableDlgItem(IDC_MINTYPE,TRUE);
			SetFocus(IDC_MINDATE);
		}
		else
		{
			EnableDlgItem(IDC_MINDATE,FALSE);
			EnableDlgItem(IDC_MINTYPE,FALSE);
			
			if (hControl==NULL && wNotifyCode==1) // Accelerator
				SetFocus(IDC_CHECKMINDATE);
		}
		break;
	case IDC_CHECKMAXDATE:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
			CheckDlgButton(IDC_CHECKMAXDATE,!IsDlgButtonChecked(IDC_CHECKMAXDATE));

		if (IsDlgButtonChecked(IDC_CHECKMAXDATE))
		{
			EnableDlgItem(IDC_MAXDATE,TRUE);
			EnableDlgItem(IDC_MAXTYPE,TRUE);
			SetFocus(IDC_MAXDATE);
		}
		else
		{
			EnableDlgItem(IDC_MAXDATE,FALSE);
			EnableDlgItem(IDC_MAXTYPE,FALSE);

			if (hControl==NULL && wNotifyCode==1) // Accelerator
				SetFocus(IDC_CHECKMAXDATE);
		}
		break;
	}
	return FALSE;
}

BOOL CLocateDlg::CSizeDateDlg::OnOk(CLocater* pLocater)
{
	DlgDebugMessage("CLocateDlg::CSizeDateDlg::OnOk BEGIN");
	
	DWORD dwFlags=0;
	DWORD dwMinSize=DWORD(-1),dwMaxSize=DWORD(-1);
	WORD wMinDate=WORD(-1),wMaxDate=WORD(-1);

	if (IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE))
	{
		dwMinSize=GetDlgItemInt(IDC_MINIMUMSIZE);
		int nCurSel=SendDlgItemMessage(IDC_MINSIZETYPE,CB_GETCURSEL,0,0);
		if (nCurSel)
			dwMinSize*=1024;
		if (nCurSel==2)
			dwMinSize*=1024;
	}
	if (IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE))
	{
		dwMaxSize=GetDlgItemInt(IDC_MAXIMUMSIZE);
		int nCurSel=SendDlgItemMessage(IDC_MAXSIZETYPE,CB_GETCURSEL,0,0);
		if (nCurSel)
			dwMaxSize*=1024;
		if (nCurSel==2)
			dwMaxSize*=1024;
	}
	if (IsDlgButtonChecked(IDC_CHECKMINDATE))
	{
		SYSTEMTIME st;
		SendDlgItemMessage(IDC_MINDATE,DTM_GETSYSTEMTIME,0,(LPARAM)&st);
		wMinDate=SYSTEMTIMETODOSDATE(st);
		int nCurSel=SendDlgItemMessage(IDC_MINTYPE,CB_GETCURSEL,0,0);
		if (nCurSel==1)
			dwFlags|=LOCATE_MINCREATIONDATE;
		else if (nCurSel==2)
			dwFlags|=LOCATE_MINACCESSDATE;
	}
	if (IsDlgButtonChecked(IDC_CHECKMAXDATE))
	{
		SYSTEMTIME st;
		SendDlgItemMessage(IDC_MAXDATE,DTM_GETSYSTEMTIME,0,(LPARAM)&st);
		wMaxDate=SYSTEMTIMETODOSDATE(st);
		int nCurSel=SendDlgItemMessage(IDC_MAXTYPE,CB_GETCURSEL,0,0);
		if (nCurSel==1)
			dwFlags|=LOCATE_MAXCREATIONDATE;
		else if (nCurSel==2)
			dwFlags|=LOCATE_MAXACCESSDATE;
	}

	pLocater->SetSizeAndDate(dwFlags,dwMinSize,dwMaxSize,wMinDate,wMaxDate);
	
	DlgDebugMessage("CLocateDlg::CSizeDateDlg::OnOk END");
	return IsDlgButtonChecked(IDC_MATCHWHOLENAME);
}
	
void CLocateDlg::CSizeDateDlg::OnClear()
{
	CheckDlgButton(IDC_CHECKMINIMUMSIZE,0);
	CheckDlgButton(IDC_CHECKMAXIMUMSIZE,0);
	CheckDlgButton(IDC_CHECKMINDATE,0);
	CheckDlgButton(IDC_CHECKMAXDATE,0);
	EnableDlgItem(IDC_MINIMUMSIZE,FALSE);
	EnableDlgItem(IDC_MAXIMUMSIZE,FALSE);
	EnableDlgItem(IDC_MINIMUMSIZESPIN,FALSE);
	EnableDlgItem(IDC_MAXIMUMSIZESPIN,FALSE);
	EnableDlgItem(IDC_MINSIZETYPE,FALSE);
	EnableDlgItem(IDC_MAXSIZETYPE,FALSE);
	EnableDlgItem(IDC_MINDATE,FALSE);
	EnableDlgItem(IDC_MINTYPE,FALSE);
	EnableDlgItem(IDC_MAXDATE,FALSE);
	EnableDlgItem(IDC_MAXTYPE,FALSE);
	SendDlgItemMessage(IDC_MINIMUMSIZESPIN,UDM_SETPOS,0,MAKELPARAM(0,0));
	SendDlgItemMessage(IDC_MAXIMUMSIZESPIN,UDM_SETPOS,0,MAKELPARAM(0,0));
	SendDlgItemMessage(IDC_MINTYPE,CB_SETCURSEL,0,0);
	SendDlgItemMessage(IDC_MAXTYPE,CB_SETCURSEL,0,0);
	SendDlgItemMessage(IDC_MINSIZETYPE,CB_SETCURSEL,0,0);
	SendDlgItemMessage(IDC_MAXSIZETYPE,CB_SETCURSEL,0,0);
	SYSTEMTIME st;
	GetLocalTime(&st);
	SendDlgItemMessage(IDC_MINDATE,DTM_SETSYSTEMTIME,GDT_VALID,(LPARAM)&st);
	SendDlgItemMessage(IDC_MAXDATE,DTM_SETSYSTEMTIME,GDT_VALID,(LPARAM)&st);
}

void CLocateDlg::CSizeDateDlg::EnableItems(BOOL bEnable_)
{
	EnableDlgItem(IDC_CHECKMINIMUMSIZE,TRUE);
	BOOL bEnable=bEnable_ && IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE);
	EnableDlgItem(IDC_MINIMUMSIZE,bEnable);
	EnableDlgItem(IDC_MINIMUMSIZESPIN,bEnable);
	EnableDlgItem(IDC_MINSIZETYPE,bEnable);
	
	EnableDlgItem(IDC_CHECKMAXIMUMSIZE,TRUE);
	bEnable=bEnable_ && IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE);
	EnableDlgItem(IDC_MAXIMUMSIZE,bEnable);
	EnableDlgItem(IDC_MAXIMUMSIZESPIN,bEnable);
	EnableDlgItem(IDC_MAXSIZETYPE,bEnable);
	
	EnableDlgItem(IDC_CHECKMINDATE,TRUE);
	bEnable=bEnable_ && IsDlgButtonChecked(IDC_CHECKMINDATE);
	EnableDlgItem(IDC_MINDATE,bEnable);
	EnableDlgItem(IDC_MINTYPE,bEnable);
	
	EnableDlgItem(IDC_CHECKMAXDATE,TRUE);
	bEnable=bEnable_ && IsDlgButtonChecked(IDC_CHECKMAXDATE);
	EnableDlgItem(IDC_MAXDATE,bEnable);
	EnableDlgItem(IDC_MAXTYPE,bEnable);
}

void CLocateDlg::CSizeDateDlg::SetStartData(const CLocateApp::CStartData* pStartData)
{
	if (pStartData->m_dwMinFileSize!=DWORD(-1))
	{
		CheckDlgButton(IDC_CHECKMINIMUMSIZE,1);
		SetDlgItemInt(IDC_MINIMUMSIZE,pStartData->m_dwMinFileSize);
		switch (pStartData->m_cMinSizeType)
		{
		case 'k':
		case 'K':
			SendDlgItemMessage(IDC_MINSIZETYPE,CB_SETCURSEL,1);
			break;
		case 'm':
		case 'M':
			SendDlgItemMessage(IDC_MINSIZETYPE,CB_SETCURSEL,2);
			break;
		}
	}

	if (pStartData->m_dwMaxFileSize!=DWORD(-1))
	{
		CheckDlgButton(IDC_CHECKMAXIMUMSIZE,1);
		SetDlgItemInt(IDC_MAXIMUMSIZE,pStartData->m_dwMaxFileSize);
		switch (pStartData->m_cMaxSizeType)
		{
		case 'k':
		case 'K':
			SendDlgItemMessage(IDC_MAXSIZETYPE,CB_SETCURSEL,1);
			break;
		case 'm':
		case 'M':
			SendDlgItemMessage(IDC_MAXSIZETYPE,CB_SETCURSEL,2);
			break;
		}
	}

	if (pStartData->m_dwMinDate!=DWORD(-1))
	{
		CheckDlgButton(IDC_CHECKMINDATE,1);
        SYSTEMTIME st;
		st.wHour=1;
		st.wMinute=1;
		st.wSecond=1;
		st.wMilliseconds=1;
		st.wDay=BYTE(pStartData->m_dwMinDate);
		st.wMonth=BYTE(pStartData->m_dwMinDate>>8);
		st.wYear=WORD(pStartData->m_dwMinDate>>16);
		SendDlgItemMessage(IDC_MINDATE,DTM_SETSYSTEMTIME,GDT_VALID,LPARAM(&st));

		switch (pStartData->m_cMinDateType)
		{
		case 'c':
			SendDlgItemMessage(IDC_MINTYPE,CB_SETCURSEL,1);
			break;
		case 'a':
			SendDlgItemMessage(IDC_MINTYPE,CB_SETCURSEL,2);
			break;
		}
	}

	if (pStartData->m_dwMaxDate!=DWORD(-1))
	{
		CheckDlgButton(IDC_CHECKMAXDATE,1);
        SYSTEMTIME st;
		st.wHour=1;
		st.wMinute=1;
		st.wSecond=1;
		st.wMilliseconds=1;
		st.wDay=BYTE(pStartData->m_dwMaxDate);
		st.wMonth=BYTE(pStartData->m_dwMaxDate>>8);
		st.wYear=WORD(pStartData->m_dwMaxDate>>16);
		SendDlgItemMessage(IDC_MAXDATE,DTM_SETSYSTEMTIME,GDT_VALID,LPARAM(&st));

		switch (pStartData->m_cMaxDateType)
		{
		case 'c':
			SendDlgItemMessage(IDC_MAXTYPE,CB_SETCURSEL,1);
			break;
		case 'a':
			SendDlgItemMessage(IDC_MAXTYPE,CB_SETCURSEL,2);
			break;
		}
	}

	EnableItems(TRUE);
}

void CLocateDlg::CSizeDateDlg::LoadControlStates(CRegKey& RegKey)
{
	DWORD dwTemp=0;
	if (RegKey.QueryValue("SizeDate/MinimumSize",dwTemp))
	{
		CheckDlgButton(IDC_CHECKMINIMUMSIZE,TRUE);
		SetDlgItemInt(IDC_MINIMUMSIZE,dwTemp);
		dwTemp=0;
		RegKey.QueryValue("SizeDate/MinimumSizeType",dwTemp);
		SendDlgItemMessage(IDC_MINSIZETYPE,CB_SETCURSEL,dwTemp,0);
	}
	else
		CheckDlgButton(IDC_CHECKMINIMUMSIZE,FALSE);

	dwTemp=0;
	if (RegKey.QueryValue("SizeDate/MaximumSize",dwTemp))
	{
		CheckDlgButton(IDC_CHECKMAXIMUMSIZE,TRUE);
		SetDlgItemInt(IDC_MAXIMUMSIZE,dwTemp);
		dwTemp=0;
		RegKey.QueryValue("SizeDate/MaximumSizeType",dwTemp);
		SendDlgItemMessage(IDC_MAXSIZETYPE,CB_SETCURSEL,dwTemp,0);
	}
	else
		CheckDlgButton(IDC_CHECKMAXIMUMSIZE,FALSE);
		



	char szData[sizeof(SYSTEMTIME)+4];
	if (RegKey.QueryValue("SizeDate/MinimumDate",szData,sizeof(SYSTEMTIME)+4)>=sizeof(SYSTEMTIME)+4)
	{
		CheckDlgButton(IDC_CHECKMINDATE,TRUE);
		SendDlgItemMessage(IDC_MINDATE,DTM_SETSYSTEMTIME,0,(LPARAM)(szData+4));
		SendDlgItemMessage(IDC_MINTYPE,CB_SETCURSEL,*((int*)szData),0);
	}
	else
		CheckDlgButton(IDC_CHECKMINDATE,FALSE);
		

	if (RegKey.QueryValue("SizeDate/MaximumDate",szData,sizeof(SYSTEMTIME)+4)>=sizeof(SYSTEMTIME)+4)
	{
		CheckDlgButton(IDC_CHECKMAXDATE,TRUE);
		SendDlgItemMessage(IDC_MAXDATE,DTM_SETSYSTEMTIME,0,(LPARAM)(szData+4));
		SendDlgItemMessage(IDC_MAXTYPE,CB_SETCURSEL,*((int*)szData),0);
	}
	else
		CheckDlgButton(IDC_CHECKMAXDATE,FALSE);
		
		
}

void CLocateDlg::CSizeDateDlg::SaveControlStates(CRegKey& RegKey)
{
	if (IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE))
	{
		RegKey.SetValue("SizeDate/MinimumSize",DWORD(GetDlgItemInt(IDC_MINIMUMSIZE)));
		RegKey.SetValue("SizeDate/MinimumSizeType",DWORD(SendDlgItemMessage(IDC_MINSIZETYPE,CB_GETCURSEL,0,0)));
	}
	else
	{
		RegKey.DeleteValue("SizeDate/MinimumSize");
		RegKey.DeleteValue("SizeDate/MinimumSizeType");
	}
	if (IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE))
	{
		RegKey.SetValue("SizeDate/MaximumSize",DWORD(GetDlgItemInt(IDC_MAXIMUMSIZE)));
		RegKey.SetValue("SizeDate/MaximumSizeType",DWORD(SendDlgItemMessage(IDC_MAXSIZETYPE,CB_GETCURSEL,0,0)));
	}
	else
	{
		RegKey.DeleteValue("SizeDate/MaximumSize");
		RegKey.DeleteValue("SizeDate/MaximumSizeType");
	}
	if (IsDlgButtonChecked(IDC_CHECKMINDATE))
	{
		char szTemp[sizeof(SYSTEMTIME)+4];
		SendDlgItemMessage(IDC_MINDATE,DTM_GETSYSTEMTIME,0,(LPARAM)(szTemp+4));
		*((int*)szTemp)=SendDlgItemMessage(IDC_MINTYPE,CB_GETCURSEL,0,0);
		RegKey.SetValue("SizeDate/MinimumDate",szTemp,sizeof(SYSTEMTIME)+4,REG_BINARY);
	}
	else
		RegKey.DeleteValue("SizeDate/MinimumDate");
	if (IsDlgButtonChecked(IDC_CHECKMAXDATE))
	{
		char szTemp[sizeof(SYSTEMTIME)+4];
		SendDlgItemMessage(IDC_MAXDATE,DTM_GETSYSTEMTIME,0,(LPARAM)(szTemp+4));
		*((int*)szTemp)=SendDlgItemMessage(IDC_MAXTYPE,CB_GETCURSEL,0,0);
		RegKey.SetValue("SizeDate/MaximumDate",szTemp,sizeof(SYSTEMTIME)+4,REG_BINARY);
	}
	else
		RegKey.DeleteValue("SizeDate/MaximumDate");

}

BOOL CLocateDlg::CAdvancedDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	
	// Initializing Imagelists
	m_ToolbarIL.Create(IDB_TOOLBARBITMAPS,12,1,RGB(255,255,255));
	m_ToolbarILHover.Create(IDB_TOOLBARBITMAPSH,12,1,RGB(255,255,255));
	m_ToolbarILDisabled.Create(IDB_TOOLBARBITMAPSD,12,1,RGB(255,255,255));
	
	// Sets imagelists for toolbar
	SendDlgItemMessage(IDC_HELPTOOLBAR,TB_SETIMAGELIST,0,LPARAM(HIMAGELIST(m_ToolbarIL)));
	SendDlgItemMessage(IDC_HELPTOOLBAR,TB_SETHOTIMAGELIST,0,LPARAM(HIMAGELIST(m_ToolbarILHover)));
	SendDlgItemMessage(IDC_HELPTOOLBAR,TB_SETDISABLEDIMAGELIST,0,LPARAM(HIMAGELIST(m_ToolbarILDisabled)));
	// Inserting Help button to toolbar
	TBBUTTON tb;
	memset(&tb,0,sizeof(TBBUTTON));
	tb.idCommand=IDC_TEXTHELP;
	tb.fsStyle=TBSTYLE_BUTTON;
	tb.fsState=TBSTATE_ENABLED;
	SendDlgItemMessage(IDC_HELPTOOLBAR,TB_INSERTBUTTON,0,LPARAM(&tb));

	SetLastError(0);
	if (IsFullUnicodeSupport())
	{
		DebugMessage("CAdvancedDlg::OnInitDialog: Adding items to check box, Unicode ítems");

		int nRes=SendDlgItemMessageW(IDC_CHECK,CB_ADDSTRING,0,(LPARAM)(LPCWSTR)CStringW(IDS_FILENAMESONLY));
		DebugFormatMessage("1: %X le=%X",nRes,GetLastError());
		nRes=SendDlgItemMessageW(IDC_CHECK,CB_ADDSTRING,0,(LPARAM)(LPCWSTR)CStringW(IDS_FILEANDFOLDERNAMES));
		DebugFormatMessage("2: %X le=%X",nRes,GetLastError());
		nRes=SendDlgItemMessageW(IDC_CHECK,CB_ADDSTRING,0,(LPARAM)(LPCWSTR)CStringW(IDS_FOLDERNAMESONLY));
		DebugFormatMessage("3: %X le=%X",nRes,GetLastError());
		
	}
	else
	{
		DebugMessage("CAdvancedDlg::OnInitDialog: Adding items to check box, Multibyte ítems");

		int nRes=SendDlgItemMessage(IDC_CHECK,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_FILENAMESONLY));
		DebugFormatMessage("1: %X le=%X",nRes,GetLastError());
		nRes=SendDlgItemMessage(IDC_CHECK,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_FILEANDFOLDERNAMES));
		DebugFormatMessage("2: %X le=%X",nRes,GetLastError());
		nRes=SendDlgItemMessage(IDC_CHECK,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_FOLDERNAMESONLY));
		DebugFormatMessage("3: %X le=%X",nRes,GetLastError());
		
	}
	
	// Adding (by extension)
	SendDlgItemMessage(IDC_FILETYPE,CB_ADDSTRING,0,LPARAM(szEmpty));
	OnClear();
	
	RECT rc1,rc2;
	GetWindowRect(&rc1);
	::GetWindowRect(GetDlgItem(IDC_CHECK),&rc2);
	m_nCheckPos=WORD(rc2.left-rc1.left);
	::GetWindowRect(GetDlgItem(IDC_DATAMATCHCASE),&rc2);
	m_nMatchCaseTop=WORD(rc2.top-rc1.top);
	::GetClientRect(GetDlgItem(IDC_DATAMATCHCASE),&rc2);
	m_nMatchCaseWidth=WORD(rc2.right);
	return FALSE;
}

void CLocateDlg::CAdvancedDlg::AddBuildInFileTypes()
{
	if (m_dwFlags&fgBuildInTypesAdded)
		return;

	CString strTypes;
	CRegKey RegKey;
	DWORD bRet=FALSE;

	// Loading buildin types from registry if needed	
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Locate",
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("Load Buildin Types",bRet);
		if (bRet)
			RegKey.QueryValue("Buildin Types",strTypes);
	}

	// Saving buildin types to registry
	if (strTypes.IsEmpty())
	{
		strTypes.LoadString(IDS_BUILDINTYPES);

        if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Locate",
			CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			if (!bRet)		
				RegKey.SetValue("Load Buildin Types",DWORD(0));
			RegKey.SetValue("Buildin Types",strTypes);
		}			
	}
	RegKey.CloseKey();

	// This is not very best way to do this
	strTypes.ReplaceChars('|','\0');
	LPCSTR pPtr=strTypes;
	CImageList il;
	il.Create(IDB_DEFAULTTYPEICONS,16,0,RGB(255,0,255));
	while (*pPtr!='\0')
		SendDlgItemMessage(IDC_FILETYPE,CB_ADDSTRING,0,LPARAM(new FileType(pPtr,il)));
	il.DeleteImageList();

	m_dwFlags|=fgBuildInTypesAdded;
}

BOOL CLocateDlg::CAdvancedDlg::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	if (msg==WM_COMPAREITEM && wParam==IDC_FILETYPE)
	{
		FileType* ft1=((FileType*)((COMPAREITEMSTRUCT*)lParam)->itemData1);
		FileType* ft2=((FileType*)((COMPAREITEMSTRUCT*)lParam)->itemData2);

		if (ft1==NULL || ft1==(FileType*)szEmpty)
			return -1;
		if (ft2==NULL || ft2==(FileType*)szEmpty)
			return 1;
		if (ft1->szType==NULL)
		{
			if (ft2->szType!=NULL)
				return -1;
		}
		else if (ft2->szType==NULL) // ft2 is buildin
			return 1;

		return stricmp(
			((FileType*)((COMPAREITEMSTRUCT*)lParam)->itemData1)->szTitle,
			((FileType*)((COMPAREITEMSTRUCT*)lParam)->itemData2)->szTitle);
	}
	return CDialog::WindowProc(msg,wParam,lParam);
}

BOOL CLocateDlg::CAdvancedDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch (wID)
	{
	case IDC_TEXTHELP:
		{
			HRSRC hRc=FindResource(GetLanguageSpecificResourceHandle(),MAKEINTRESOURCE(IDR_SEARCHFROMFILE),"HELPTEXT");
			HGLOBAL hGlobal=LoadResource(GetLanguageSpecificResourceHandle(),hRc);
			CString title(IDS_HELPINFO);
			MessageBox((LPCSTR)LockResource(hGlobal),title,MB_OK|MB_ICONINFORMATION);
			
			break;
		}
	case IDC_CONTAINDATACHECK:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
			CheckDlgButton(IDC_CONTAINDATACHECK,!IsDlgButtonChecked(IDC_CONTAINDATACHECK));

		if (IsDlgButtonChecked(IDC_CONTAINDATACHECK))
		{
			EnableDlgItem(IDC_DATAMATCHCASE,TRUE);
			EnableDlgItem(IDC_CONTAINDATA,TRUE);
			EnableDlgItem(IDC_HELPTOOLBAR,TRUE);
			SetFocus(IDC_CONTAINDATA);
		}
		else
		{
			EnableDlgItem(IDC_DATAMATCHCASE,FALSE);
			EnableDlgItem(IDC_CONTAINDATA,FALSE);
			EnableDlgItem(IDC_HELPTOOLBAR,FALSE);

			if (hControl==NULL && wNotifyCode==1) // Accelerator
				SetFocus(IDC_CONTAINDATACHECK);
		}
		ChangeEnableStateForCheck();
		break;
	case IDC_FILETYPE:
		switch (wNotifyCode)
		{
		case 1: // Accel or CBN_SELCHANGE
			if (hControl==NULL)
				SetFocus(IDC_FILETYPE);
			else
			{
				ChangeEnableStateForCheck();
				FileType* ft;
				int nSel=SendDlgItemMessage(IDC_FILETYPE,CB_GETCURSEL);
				if (nSel!=CB_ERR)
				{
					ft=(FileType*)SendDlgItemMessage(IDC_FILETYPE,CB_GETITEMDATA,nSel);
					if (ft==NULL || ft==(FileType*)szEmpty)
					{
						GetLocateDlg()->m_NameDlg.EnableDlgItem(IDC_TYPE,TRUE);
						break;
					}

					char* pEx=new char[ft->dwExtensionLength];
					sMemCopy(pEx,ft->szExtensions,ft->dwExtensionLength);
					
					for (int i=0;pEx[i]!='\0' || pEx[i+1]!='\0';i++)
					{
						if (pEx[i]=='\0')
							pEx[i]=' ';
					}
					GetLocateDlg()->m_NameDlg.SetDlgItemText(IDC_TYPE,pEx);
					delete[] pEx;
				}
				GetLocateDlg()->m_NameDlg.EnableDlgItem(IDC_TYPE,ft==NULL);
				break;
			}
		case CBN_DROPDOWN:
			UpdateTypeList();
			break;
		case CBN_CLOSEUP:
			if (m_hTypeUpdaterThread==NULL)
			{
				int nSelected=SendDlgItemMessage(IDC_FILETYPE,CB_GETCURSEL);

				// Destroying unnecessary icons
				for (int i=SendDlgItemMessage(IDC_FILETYPE,CB_GETCOUNT)-1;i>0;i--)
				{
					if (i!=nSelected)
					{
						FileType* pFileType=(FileType*)SendDlgItemMessage(IDC_FILETYPE,CB_GETITEMDATA,i);
						if (pFileType!=NULL)
						{
							if (pFileType->szIconPath!=NULL && 
								pFileType->hIcon!=NULL && 
								pFileType->hIcon!=m_hDefaultTypeIcon)
							{
								DestroyIcon(pFileType->hIcon);
								pFileType->hIcon=NULL;
							}
						}
					}
				}

				// Arranging allocated data
				ReArrangeAllocatedData();			
			}
			break;
		}
		break;
	case IDC_CONTAINDATA:
		switch (wNotifyCode)
		{
		case 1:
			SetFocus(wID);
			break;
		case EN_SETFOCUS:
			::SendMessage(hControl,EM_SETSEL,0,MAKELPARAM(0,-1));
			break;
		}
		break;
	case IDC_CHECK:
		if (hControl==NULL && wNotifyCode==1 && IsDlgItemEnabled(IDC_CHECK)) // Accelerator
			SetFocus(IDC_CHECK);
		break;
	case IDC_REPLACESPACES:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
		{
			CheckDlgButton(IDC_REPLACESPACES,!IsDlgButtonChecked(IDC_REPLACESPACES));
			SetFocus(IDC_REPLACESPACES);
		}
		break;
	case IDC_MATCHWHOLENAME:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
		{
			CheckDlgButton(IDC_MATCHWHOLENAME,!IsDlgButtonChecked(IDC_MATCHWHOLENAME));
			SetFocus(IDC_MATCHWHOLENAME);
		}
		break;
	case IDC_DATAMATCHCASE:
		if (hControl==NULL && wNotifyCode==1 && IsDlgButtonChecked(IDC_CONTAINDATACHECK)) // Accelerator
		{
			CheckDlgButton(IDC_DATAMATCHCASE,!IsDlgButtonChecked(IDC_DATAMATCHCASE));
			SetFocus(IDC_DATAMATCHCASE);
		}
		break;

	}
	return CDialog::OnCommand(wID,wNotifyCode,hControl);
}

void CLocateDlg::CAdvancedDlg::ReArrangeAllocatedData()
{
	int nCount=SendDlgItemMessage(IDC_FILETYPE,CB_GETCOUNT)-1;
	void*** pBlocks=new void**[nCount*2];
	FileType** pft=new FileType*[nCount];
	
	// Firstly moving FileType classes
	for (int i=0;i<nCount;i++)
	{
		pft[i]=(FileType*)SendDlgItemMessage(IDC_FILETYPE,CB_GETITEMDATA,i+1);
		pBlocks[i]=(void**)&(pft[i]);
	}
	FileTypeAllocator.ReArrange(pBlocks,nCount);
	for (i=0;i<nCount;i++)
		SendDlgItemMessage(IDC_FILETYPE,CB_SETITEMDATA,i+1,LPARAM(pft[i]));
	
	// and now, moving rest of data
	for (i=0;i<nCount;i++)
	{
		pBlocks[i*2]=(void**)&(pft[i]->szTitle);
		pBlocks[i*2+1]=(void**)&(pft[i]->szExtensions);
	}
	FileTypeAllocator.ReArrange(pBlocks,nCount*2);
	delete[] pBlocks;
	delete[] pft;
}

void CLocateDlg::CAdvancedDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType,cx,cy);
	
	SetDlgItemPos(IDC_CHECK,HWND_BOTTOM,0,0,cx-m_nCheckPos-19,21,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	SetDlgItemPos(IDC_FILETYPE,HWND_BOTTOM,0,0,cx-34,21,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	SetDlgItemPos(IDC_CONTAINDATA,HWND_BOTTOM,0,0,cx-34,21,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	SetDlgItemPos(IDC_DATAMATCHCASE,HWND_BOTTOM,max(cx-m_nMatchCaseWidth-45,125),m_nMatchCaseTop,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
	SetDlgItemPos(IDC_HELPTOOLBAR,HWND_BOTTOM,max(cx-43,125),m_nMatchCaseTop-3,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
}

DWORD CLocateDlg::CAdvancedDlg::OnOk(CLocater* pLocater)
{
	DlgDebugMessage("CLocateDlg::CAdvancedDlg::OnOk BEGIN");
	
	DWORD dwFlags=0;
	
	switch (SendDlgItemMessage(IDC_CHECK,CB_GETCURSEL))
	{
	case 0:
		dwFlags=LOCATE_FILENAMES;
		break;
	case 1:
		dwFlags=LOCATE_FILENAMES|LOCATE_FOLDERNAMES;
		break;
	case 2:
		dwFlags=LOCATE_FOLDERNAMES;
		break;
	}

	
	if (IsDlgButtonChecked(IDC_CONTAINDATACHECK))
	{
		if (IsDlgButtonChecked(IDC_DATAMATCHCASE))
			dwFlags|=LOCATE_CONTAINTEXTISMATCHCASE;
		CString str;
		GetDlgItemText(IDC_CONTAINDATA,str);
		
		
		DWORD dwDataLength;
		BYTE* pData=NULL;
		if (strnicmp(str,"regexp:",7)==0)
		{
			dwDataLength=istrlen(LPCSTR(str)+7)+1;
            if (dwDataLength>1)
			{
				pData=new BYTE[dwDataLength+1];
				sMemCopy(pData,LPCSTR(str)+7,dwDataLength);
				dwFlags|=LOCATE_REGULAREXPRESSIONSEARCH;
			}
		}
		else
			pData=dataparser(str,&dwDataLength);
			

		pLocater->SetAdvanced(dwFlags,pData,dwDataLength,GetLocateDlg()->m_dwMaxFoundFiles==0?-1:GetLocateDlg()->m_dwMaxFoundFiles);
		if (pData!=NULL)
			delete[] pData;
	}
	else
		pLocater->SetAdvanced(dwFlags,NULL,0,GetLocateDlg()->m_dwMaxFoundFiles==0?-1:GetLocateDlg()->m_dwMaxFoundFiles);

	DlgDebugMessage("CLocateDlg::CAdvancedDlg::OnOk END");

	return (IsDlgButtonChecked(IDC_MATCHWHOLENAME)?flagMatchCase:0)|
		(IsDlgButtonChecked(IDC_REPLACESPACES)?flagReplaceSpaces:0);

}

	
void CLocateDlg::CAdvancedDlg::OnClear()
{
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD nTemp=1;
		RegKey.QueryValue("Default CheckIn",nTemp);
		SendDlgItemMessage(IDC_CHECK,CB_SETCURSEL,nTemp);
        nTemp=0;
		RegKey.QueryValue("Default MatchWholeName",nTemp);
		CheckDlgButton(IDC_MATCHWHOLENAME,nTemp);
		nTemp=1;
		RegKey.QueryValue("Default DataMatchCase",nTemp);
		CheckDlgButton(IDC_DATAMATCHCASE,nTemp);
		nTemp=0;
		RegKey.QueryValue("Default ReplaceSpaces",nTemp);
		CheckDlgButton(IDC_REPLACESPACES,nTemp);
	}
	else
	{
		SendDlgItemMessage(IDC_CHECK,CB_SETCURSEL,1);
		CheckDlgButton(IDC_MATCHWHOLENAME,0);
		CheckDlgButton(IDC_DATAMATCHCASE,1);
		CheckDlgButton(IDC_REPLACESPACES,0);
	}

	SendDlgItemMessage(IDC_FILETYPE,CB_SETCURSEL,0);
	
	CheckDlgButton(IDC_CONTAINDATACHECK,0);
	EnableDlgItem(IDC_DATAMATCHCASE,FALSE);
	EnableDlgItem(IDC_CONTAINDATA,FALSE);
	EnableDlgItem(IDC_HELPTOOLBAR,FALSE);
	SetDlgItemText(IDC_CONTAINDATA,"");	

	if (m_hTypeUpdaterThread==NULL)
		ReArrangeAllocatedData();

	ChangeEnableStateForCheck();
}

void CLocateDlg::CAdvancedDlg::SetStartData(const CLocateApp::CStartData* pStartData)
{
	int sel=-1;
	if ((pStartData->m_nStatus&CLocateApp::CStartData::statusFindFileAndFolderNames)==CLocateApp::CStartData::statusFindFileAndFolderNames)
		sel=1;
	else if (pStartData->m_nStatus&CLocateApp::CStartData::statusFindFileNames)
		sel=0;
	else if (pStartData->m_nStatus&CLocateApp::CStartData::statusFindFolderNames)
		sel=2;
	if (sel!=-1)
		SendDlgItemMessage(IDC_CHECK,CB_SETCURSEL,sel);
    
	if (pStartData->m_pFindText!=NULL)
	{
		EnableDlgItem(IDC_DATAMATCHCASE,TRUE);
		EnableDlgItem(IDC_CONTAINDATA,TRUE);
		CheckDlgButton(IDC_CONTAINDATACHECK,1);
		SetDlgItemText(IDC_CONTAINDATA,pStartData->m_pFindText);
		CheckDlgButton(IDC_DATAMATCHCASE,pStartData->m_nStatus&CLocateApp::CStartData::statusFindIsNotMatchCase?0:1);
	}

	if (pStartData->m_nStatus&CLocateApp::CStartData::statusMatchWholeName)
		CheckDlgButton(IDC_MATCHWHOLENAME,1);
	else if (pStartData->m_nStatus&CLocateApp::CStartData::statusNoMatchWholeName)
		CheckDlgButton(IDC_MATCHWHOLENAME,0);
	if (pStartData->m_nStatus&CLocateApp::CStartData::statusReplaceSpacesWithAsterisks)
		CheckDlgButton(IDC_REPLACESPACES,1);
	else if (pStartData->m_nStatus&CLocateApp::CStartData::statusNoReplaceSpacesWithAsterisks)
		CheckDlgButton(IDC_REPLACESPACES,0);
	ChangeEnableStateForCheck();
}
void CLocateDlg::CAdvancedDlg::EnableItems(BOOL bEnable)
{
	EnableDlgItem(IDC_MATCHWHOLENAME,bEnable);

	EnableDlgItem(IDC_FILETYPE,bEnable);

	EnableDlgItem(IDC_CONTAINDATACHECK,bEnable);
	EnableDlgItem(IDC_DATAMATCHCASE,bEnable && IsDlgButtonChecked(IDC_CONTAINDATACHECK));
	EnableDlgItem(IDC_CONTAINDATA,bEnable && IsDlgButtonChecked(IDC_CONTAINDATACHECK));
	EnableDlgItem(IDC_HELPTOOLBAR,bEnable && IsDlgButtonChecked(IDC_CONTAINDATACHECK));

	
	if (bEnable)
		ChangeEnableStateForCheck();
	else
		EnableDlgItem(IDC_CHECK,FALSE);
}

void CLocateDlg::CAdvancedDlg::ChangeEnableStateForCheck()
{
	BOOL bEnable=TRUE;
	if (GetLocateDlg()->m_SizeDateDlg.LookOnlyFiles())
		bEnable=FALSE;
	else if (IsDlgButtonChecked(IDC_CONTAINDATACHECK))
		bEnable=FALSE;
	else if (SendDlgItemMessage(IDC_FILETYPE,CB_GETCURSEL)!=0)
		bEnable=FALSE;
	
	int nCurSel=GetLocateDlg()->m_NameDlg.SendDlgItemMessage(IDC_TYPE,CB_GETCURSEL);
	if (nCurSel==CB_ERR)
	{
		if (GetLocateDlg()->m_NameDlg.SendDlgItemMessage(IDC_TYPE,WM_GETTEXTLENGTH)>0)
			bEnable=FALSE;
	}
	else
	{
		if (GetLocateDlg()->m_NameDlg.SendDlgItemMessage(IDC_TYPE,CB_GETLBTEXTLEN,nCurSel)>0)
			bEnable=FALSE;
	}
	
	if (bEnable)
		EnableDlgItem(IDC_CHECK,TRUE);
	else
	{
		SendDlgItemMessage(IDC_CHECK,CB_SETCURSEL,0);
		EnableDlgItem(IDC_CHECK,FALSE);
	}
}

void CLocateDlg::CAdvancedDlg::OnDrawItem(UINT idCtl,LPDRAWITEMSTRUCT lpdis)
{
	CDialog::OnDrawItem(idCtl,lpdis);

	if (idCtl==IDC_FILETYPE && lpdis->itemID!=CB_ERR)
	{
		CRect rc(lpdis->rcItem);
		HBRUSH hHighLight=GetSysColorBrush(COLOR_HIGHLIGHT);
		CString Text;
		
		// Drawing background
		if (lpdis->itemState&ODS_DISABLED)
			FillRect(lpdis->hDC,&lpdis->rcItem,GetSysColorBrush(COLOR_3DFACE));
		else
			FillRect(lpdis->hDC,&lpdis->rcItem,GetSysColorBrush(COLOR_WINDOW));
		
		if (lpdis->itemID==0)
		{
			ASSERT(lpdis->itemData==LPARAM(szEmpty));
			Text.LoadString(IDS_BYEXTENSION);
		}
		else
		{
			Text=((FileType*)lpdis->itemData)->szTitle;
			if (((FileType*)lpdis->itemData)->hIcon==NULL)
				((FileType*)lpdis->itemData)->ExtractIconFromPath();

			DrawIconEx(lpdis->hDC,rc.left,rc.top,((FileType*)lpdis->itemData)->hIcon,
				16,16,0,lpdis->itemAction&ODA_FOCUS?hHighLight:NULL,DI_NORMAL);
		}
		
		SetBkMode(lpdis->hDC,TRANSPARENT);
		rc.left+=18;
				
		if (lpdis->itemState&ODS_DISABLED)
			SetTextColor(lpdis->hDC,GetSysColor(COLOR_GRAYTEXT));
		else if (lpdis->itemAction&ODA_FOCUS)
		{
			// Filling text shade
			SIZE sz;
			SetTextColor(lpdis->hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
			GetTextExtentPoint32(lpdis->hDC,Text,Text.GetLength(),&sz);
			FillRect(lpdis->hDC,&CRect(rc.left,rc.top+1,rc.left+sz.cx+1,rc.bottom-1),hHighLight);
		}
		else
			SetTextColor(lpdis->hDC,GetSysColor(COLOR_WINDOWTEXT));
			
		DrawText(lpdis->hDC,Text,Text.GetLength(),&rc,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	}
}

void CLocateDlg::CAdvancedDlg::OnMeasureItem(int nIDCtl,LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CDialog::OnMeasureItem(nIDCtl,lpMeasureItemStruct);
		
	if (nIDCtl==IDC_FILETYPE)
	{
		HWND hWnd=GetDlgItem(IDC_FILETYPE);
		HDC hDC=::GetDC(hWnd);
		
		SIZE sz;
		if (lpMeasureItemStruct->itemID==0)
		{
			CString byex(IDS_BYEXTENSION);
			::GetTextExtentPoint32(hDC,byex,byex.GetLength(),&sz);
		}
		else
			::GetTextExtentPoint32(hDC,((FileType*)lpMeasureItemStruct->itemData)->szTitle,fstrlen(((FileType*)lpMeasureItemStruct->itemData)->szTitle),&sz);

		lpMeasureItemStruct->itemHeight=max(sz.cy+1,16);
		lpMeasureItemStruct->itemWidth=sz.cx+18;

		::ReleaseDC(hWnd,hDC);
	}
}

void CLocateDlg::CAdvancedDlg::OnDestroy()
{
	CDialog::OnDestroy();
	if (m_hTypeUpdaterThread!=NULL)
	{
		TerminateThread(m_hTypeUpdaterThread,0);
		CloseHandle(m_hTypeUpdaterThread);
		m_hTypeUpdaterThread=NULL;
	}

	for (int i=SendDlgItemMessage(IDC_FILETYPE,CB_GETCOUNT)-1;i>0;i--)
	{
		FileType* pFileType=(FileType*)SendDlgItemMessage(IDC_FILETYPE,CB_GETITEMDATA,i);
		if (pFileType!=NULL)
			delete pFileType;
	}

	if (m_hDefaultTypeIcon!=NULL)
	{
		DestroyIcon(m_hDefaultTypeIcon);
		m_hDefaultTypeIcon=NULL;
	}
	SendDlgItemMessage(IDC_FILETYPE,CB_RESETCONTENT);

	m_ToolbarIL.DeleteImageList();

}

void CLocateDlg::CAdvancedDlg::LoadControlStates(CRegKey& RegKey)
{
	CString str;

	DWORD dwTemp;
	// Advanced dialog
	if (!RegKey.QueryValue("Advanced/Check",dwTemp))
		dwTemp=1;
    SendDlgItemMessage(IDC_CHECK,CB_SETCURSEL,dwTemp);

	if (!RegKey.QueryValue("Advanced/MatchWholeName",dwTemp))
		dwTemp=0;
	CheckDlgButton(IDC_MATCHWHOLENAME,dwTemp);
	
	if (!RegKey.QueryValue("Advanced/ReplaceSpaces",dwTemp))
		dwTemp=0;
    CheckDlgButton(IDC_REPLACESPACES,dwTemp);

	if (!RegKey.QueryValue("Advanced/TextIsMatchCase",dwTemp))
		dwTemp=1;	
	CheckDlgButton(IDC_DATAMATCHCASE,dwTemp);
	
	if (RegKey.QueryValue("Advanced/ContainData",str))
	{
		CheckDlgButton(IDC_CONTAINDATACHECK,TRUE);
		SetDlgItemText(IDC_CONTAINDATA,str);
		EnableDlgItem(IDC_DATAMATCHCASE,TRUE);
		EnableDlgItem(IDC_CONTAINDATA,TRUE);
		EnableDlgItem(IDC_HELPTOOLBAR,TRUE);
	}
	else
	{
		CheckDlgButton(IDC_CONTAINDATACHECK,FALSE);
		SetDlgItemText(IDC_CONTAINDATA,"");
		EnableDlgItem(IDC_DATAMATCHCASE,FALSE);
		EnableDlgItem(IDC_CONTAINDATA,FALSE);
		EnableDlgItem(IDC_HELPTOOLBAR,FALSE);
	}


	SendDlgItemMessage(IDC_FILETYPE,CB_SETCURSEL,0);

	DWORD dwType;
	DWORD dwLength=RegKey.QueryValueLength("Advanced/TypeOfFile");
	char* pData=new char[dwLength];
	RegKey.QueryValue("Advanced/TypeOfFile",pData,dwLength,&dwType);
	if (dwType==REG_DWORD && dwLength==sizeof(DWORD))
	{
		if (*((int*)pData)>0)
		{
			AddBuildInFileTypes();
			SendDlgItemMessage(IDC_FILETYPE,CB_SETCURSEL,*((DWORD*)pData));
			GetLocateDlg()->m_NameDlg.EnableDlgItem(IDC_TYPE,FALSE);
		}
	}
	else if (dwType==REG_BINARY)
	{
		AddBuildInFileTypes();
		int nItem=AddTypeToList(pData);
		if (nItem>=0)
		{
			SendDlgItemMessage(IDC_FILETYPE,CB_SETCURSEL,nItem);
			GetLocateDlg()->m_NameDlg.EnableDlgItem(IDC_TYPE,FALSE);
		}
	}
	delete[] pData;
}

void CLocateDlg::CAdvancedDlg::SaveControlStates(CRegKey& RegKey)
{
	// Advanced dialog
	RegKey.SetValue("Advanced/Check",SendDlgItemMessage(IDC_CHECK,CB_GETCURSEL));
	RegKey.SetValue("Advanced/MatchWholeName",IsDlgButtonChecked(IDC_MATCHWHOLENAME));
	RegKey.SetValue("Advanced/ReplaceSpaces",IsDlgButtonChecked(IDC_REPLACESPACES));
	if (IsDlgButtonChecked(IDC_CONTAINDATACHECK))
	{
		CString str;
		GetDlgItemText(IDC_CONTAINDATA,str);
		RegKey.SetValue("Advanced/ContainData",str);
	}
	else
		RegKey.DeleteValue("Advanced/ContainData");
	RegKey.SetValue("Advanced/TextIsMatchCase",IsDlgButtonChecked(IDC_DATAMATCHCASE));

	// Type box
	int nCurSel=SendDlgItemMessage(IDC_FILETYPE,CB_GETCURSEL);
	FileType* pType=(FileType*)SendDlgItemMessage(IDC_FILETYPE,CB_GETITEMDATA,nCurSel);
	if (pType==NULL || pType==(CAdvancedDlg::FileType*)szEmpty || pType->szType==NULL)
		RegKey.SetValue("Advanced/TypeOfFile",DWORD(nCurSel));
	else
	{
		DWORD dwLength;
		dstrlen(pType->szType,dwLength);
		char* szData=new char[++dwLength+pType->dwExtensionLength];
		sMemCopy(szData,pType->szType,dwLength);
		sMemCopy(szData+dwLength,pType->szExtensions,pType->dwExtensionLength);
        RegKey.SetValue("Advanced/TypeOfFile",szData,dwLength+pType->dwExtensionLength,REG_BINARY);
		delete[] szData;
	}
}

void CLocateDlg::CAdvancedDlg::UpdateTypeList()
{
	if (m_hTypeUpdaterThread!=NULL) // Already running
		return;

	AddBuildInFileTypes();

	if (!(m_dwFlags&fgOtherTypeAdded) && GetLocateDlg()->m_dwFlags&CLocateDlg::fgLoadRegistryTypes)
		m_hTypeUpdaterThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)UpdaterProc,this,0,0);
}

int CLocateDlg::CAdvancedDlg::AddTypeToList(LPCSTR szKey,DWORD dwKeyLength,CArray<FileType*>& aFileTypes)
{
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCR,szKey,CRegKey::openExist|CRegKey::samQueryValue|CRegKey::samExecute)!=ERROR_SUCCESS)
		return CB_ERR;

	DWORD dwLength=RegKey.QueryValueLength("");
	if (dwLength<=1)
		return CB_ERR;

	char* szType=(char*)FileTypeAllocator.Allocate(dwLength);
	RegKey.QueryValue("",szType,dwLength);
	CharLower(szType);
	for (int i=0;i<aFileTypes.GetSize();i++)
	{
		FileType* pType=aFileTypes.GetAt(i);
		if (strcmp(pType->szType,szType)==0)
		{
			pType->AddExtension(szKey+1,dwKeyLength);
			FileTypeAllocator.Free(szType);
			return -2;
		}
	}
	
	if (RegKey.OpenKey(HKCR,szType,CRegKey::openExist|CRegKey::samQueryValue|CRegKey::samExecute)!=ERROR_SUCCESS)
	{
		FileTypeAllocator.Free(szType);
		return CB_ERR;
	}
	
	dwLength=RegKey.QueryValueLength("");
	if (dwLength<=1)
	{
		FileTypeAllocator.Free(szType);
		return CB_ERR;
	}
	
	char* szTitle=(char*)FileTypeAllocator.Allocate(dwLength);
	RegKey.QueryValue("",szTitle,dwLength);
	FileType* pType=new FileType(FREEDATA(szType),FREEDATA(szTitle));
	pType->AddExtension(szKey+1,dwKeyLength);
	aFileTypes.Add(pType);
	pType->SetIcon(RegKey);
	return SendDlgItemMessage(IDC_FILETYPE,CB_ADDSTRING,0,LPARAM(pType));
}

int CLocateDlg::CAdvancedDlg::AddTypeToList(LPCSTR pTypeAndExtensions)
{
	CRegKey RegKey;
	
	DWORD dwLength;
	dstrlen(pTypeAndExtensions,dwLength);
	if (dwLength==0)
		return CB_ERR;
	char* szType=(char*)FileTypeAllocator.Allocate(++dwLength);
	CopyMemory(szType,pTypeAndExtensions,dwLength);
	pTypeAndExtensions+=dwLength;
	
	if (RegKey.OpenKey(HKCR,szType,CRegKey::openExist|CRegKey::samQueryValue|CRegKey::samExecute)!=ERROR_SUCCESS)
	{
		FileTypeAllocator.Free(szType);
		return CB_ERR;
	}
	
	dwLength=RegKey.QueryValueLength("");
	if (dwLength<=1)
	{
		FileTypeAllocator.Free(szType);
		return CB_ERR;
	}
    char* szTitle=(char*)FileTypeAllocator.Allocate(dwLength);
	RegKey.QueryValue("",szTitle,dwLength);
	
	FileType* pFileType=new FileType(FREEDATA(szType),FREEDATA(szTitle));
	while (*pTypeAndExtensions!='\0')
	{
        dstrlen(pTypeAndExtensions,dwLength);
		pFileType->AddExtension(pTypeAndExtensions,dwLength++);
		pTypeAndExtensions+=dwLength;
	}
	
	pFileType->SetIcon(RegKey);
	return SendDlgItemMessage(IDC_FILETYPE,CB_ADDSTRING,0,LPARAM(pFileType));
}

DWORD WINAPI CLocateDlg::CAdvancedDlg::UpdaterProc(CLocateDlg::CAdvancedDlg* pAdvancedDlg)
{
	CArray<FileType*> aFileTypes;

	for (int i=pAdvancedDlg->SendDlgItemMessage(IDC_FILETYPE,CB_GETCOUNT)-1;i>=0;i--)
	{
		FileType* pParam=(FileType*)pAdvancedDlg->SendDlgItemMessage(IDC_FILETYPE,CB_GETITEMDATA,i);
		if (pParam!=NULL && pParam!=(FileType*)szEmpty && pParam->szType!=NULL)
			aFileTypes.Add(pParam);
	}

	char szKey[1000];
	DWORD dwIndex=0,dwKeyLength=1000;
	while (RegEnumKeyEx(HKCR,dwIndex,szKey,&dwKeyLength,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
	{
		if (szKey[0]=='.') // Is Extension
			pAdvancedDlg->AddTypeToList(szKey,dwKeyLength,aFileTypes);

		dwIndex++;
		dwKeyLength=1000;

	}
	pAdvancedDlg->m_dwFlags|=fgOtherTypeAdded;

	CloseHandle(pAdvancedDlg->m_hTypeUpdaterThread);
	pAdvancedDlg->m_hTypeUpdaterThread=NULL;
	
	if (!pAdvancedDlg->SendDlgItemMessage(IDC_FILETYPE,CB_GETDROPPEDSTATE))
		pAdvancedDlg->ReArrangeAllocatedData(); // Drop down is closed, arranging data
	return 0;
}

CLocateDlg::CAdvancedDlg::FileType::FileType(LPCSTR& szBuildIn,HIMAGELIST hImageList)
:	szType(NULL),szIconPath(NULL)
{
	DWORD dwLength;
	
	// First string is title
	dstrlen(szBuildIn,dwLength);
	szTitle=(char*)FileTypeAllocator.Allocate(++dwLength);
	sMemCopy(szTitle,szBuildIn,dwLength);
	szBuildIn+=dwLength;
	
	// Next string is extension
	dstrlen(szBuildIn,dwLength);
	dwExtensionLength=(++dwLength)+1;
	szExtensions=(char*)FileTypeAllocator.Allocate(dwExtensionLength);
	sMemCopy(szExtensions,szBuildIn,dwLength);
	szExtensions[dwLength]='\0';
	szBuildIn+=dwLength;
	dreplacech(szExtensions,' ','\0');
	
	// Third is icon index, if available
	if (*szBuildIn!='\0')
	{
		hIcon=ImageList_ExtractIcon(NULL,hImageList,atoi(szBuildIn));
		for (;*szBuildIn!='\0';szBuildIn++);
	}
	else
		hIcon=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon;
	
	szBuildIn++;
}

CLocateDlg::CAdvancedDlg::FileType::~FileType()
{
	if (szType!=NULL)
		FileTypeAllocator.Free(szType);
	if (szTitle!=NULL)
		FileTypeAllocator.Free(szTitle);
	if (szExtensions!=NULL)
		FileTypeAllocator.Free(szExtensions);
	if (hIcon!=NULL && hIcon!=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon)
		DestroyIcon(hIcon);
	if (szIconPath!=NULL)
		delete[] szIconPath;
}

void CLocateDlg::CAdvancedDlg::FileType::AddExtension(LPCSTR szExtension,DWORD dwNewExtensionLength)
{
	if (szExtensions==NULL)
	{
		dwExtensionLength=dwNewExtensionLength+1;
		szExtensions=(char*)FileTypeAllocator.Allocate(dwExtensionLength);
		sMemCopy(szExtensions,szExtension,dwNewExtensionLength);
		szExtensions[dwNewExtensionLength]='\0';
	}
	else
	{
		char* szNewExtensions;
		szNewExtensions=(char*)FileTypeAllocator.Allocate(dwExtensionLength+dwNewExtensionLength);
		sMemCopy(szNewExtensions,szExtensions,dwExtensionLength);
		sMemCopy(szNewExtensions+dwExtensionLength-1,szExtension,dwNewExtensionLength);
		dwExtensionLength+=dwNewExtensionLength;
		szNewExtensions[dwExtensionLength-1]='\0';
		FileTypeAllocator.Free(szExtensions);
		szExtensions=szNewExtensions;
	}
}

void CLocateDlg::CAdvancedDlg::FileType::SetIcon(CRegKey& rKey,BOOL toHandle)
{
	// Cleaning previous values
	if (szIconPath!=NULL)
		delete[] szIconPath;
	if (hIcon!=NULL && hIcon!=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon)
	{
		DestroyIcon(hIcon);
		hIcon=NULL;
	}

	CRegKey RegKey;
	
	if (RegKey.OpenKey(rKey,"DefaultIcon",CRegKey::openExist|CRegKey::samQueryValue|CRegKey::samExecute)!=ERROR_SUCCESS)
		return;
	DWORD dwLength=RegKey.QueryValueLength("");
	if (dwLength<2)
		return;

	szIconPath=new char[dwLength+1];
	if (RegKey.QueryValue("",szIconPath,dwLength+1)<2)
		return;

	if (toHandle)
		ExtractIconFromPath();

}

void CLocateDlg::CAdvancedDlg::FileType::ExtractIconFromPath()
{
	if (hIcon!=NULL)
		return;
	if (szIconPath==NULL) // Maybe general item
	{
		hIcon=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon;
		return;
	}

	int iIndex=0;

	for (char* szIconIndex=szIconPath;*szIconIndex!=',' && *szIconIndex!='\0';szIconIndex++);
	if (*szIconIndex!='\0')
	{
		iIndex=atoi(szIconIndex+1);
		*szIconIndex='\0';
	}

	char szExpanded[MAX_PATH];
	if (ExpandEnvironmentStrings(szIconPath,szExpanded,MAX_PATH)==0)
	{
		// Error
		hIcon=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon;
		return;
	}
	
	ExtractIconEx(szExpanded,iIndex,NULL,&hIcon,1);
	if (hIcon==NULL)
		hIcon=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon;
}
