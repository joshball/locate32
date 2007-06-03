#include <HFCLib.h>
#include "Locate32.h"
#include <uxtheme.h>
#include <tmschema.h>




CComboBoxAutoComplete::CComboBoxAutoComplete()
:	CComboBox(),m_pACData(NULL)
{
}	

CComboBoxAutoComplete::CComboBoxAutoComplete(HWND hWnd)
:	CComboBox(hWnd),m_pACData(NULL)
{
}

CComboBoxAutoComplete::~CComboBoxAutoComplete()
{
	if (m_pACData!=NULL)
		delete m_pACData;
}

void CComboBoxAutoComplete::EnableAutoComplete(BOOL bEnable)
{
	// Do not work with ownerdraw items
	ASSERT(!(GetStyle()&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE)))

	if (bEnable)
	{
		if (m_pACData!=NULL)
			return;

		// Initializing structure
		m_pACData=new ACDATA;
		m_pACData->bFlags=0;

		// Probing current items
		int nCount=CComboBox::GetCount();
		for (int i=0;i<nCount;i++)
		{
			int nLength=(int)CComboBox::GetLBTextLen(i);
			WCHAR* pText=new WCHAR[max(nLength+1,2)];
			CComboBox::GetLBText(i,pText);
			m_pACData->aItems.Add(pText);		
		}
	}
	else if (m_pACData!=NULL)
	{
		if (m_pACData->bFlags&ACDATA::afAutoCompleting)
		{
			CComboBox::ResetContent();
			for (int i=0;i<m_pACData->aItems.GetSize();i++)
				CComboBox::AddString(m_pACData->aItems[i]);
		}

		delete m_pACData;
		m_pACData=NULL;
	}
}

BOOL CComboBoxAutoComplete::IsAutoCompleteEnabled() const
{
	return m_pACData!=NULL;
}

int CComboBoxAutoComplete::GetCount() const
{
	if (m_pACData==NULL)
		return CComboBox::GetCount();

	return m_pACData->aItems.GetSize();
}

int CComboBoxAutoComplete::GetCurSel() const
{
	if (m_pACData!=NULL)
	{
		if (m_pACData->bFlags&ACDATA::afAutoCompleting)
		{
			int nCurSel=CComboBox::GetCurSel();
			if (nCurSel==CB_ERR || nCurSel>=m_pACData->aItemsInList.GetSize())
				return CB_ERR;
			return m_pACData->aItemsInList[nCurSel];
		}
	}	
	return CComboBox::GetCurSel();
}

int CComboBoxAutoComplete::SetCurSel(int nSelect)
{
	if (m_pACData!=NULL)
	{
		if (m_pACData->bFlags&ACDATA::afAutoCompleting)
		{
			int nIndex=m_pACData->aItemsInList.Find(nSelect);
			if (nIndex==-1)
				return CB_ERR;
			return CComboBox::SetCurSel(nIndex);
		}	
	}
		
	return CComboBox::SetCurSel(nSelect);
}

int CComboBoxAutoComplete::GetTopIndex() const
{
	if (m_pACData!=NULL)
	{
		if (m_pACData->bFlags&ACDATA::afAutoCompleting)
		{
			int nTopIndex=CComboBox::GetTopIndex();
			if (nTopIndex==CB_ERR || nTopIndex>=m_pACData->aItemsInList.GetSize())
				return CB_ERR;
			return m_pACData->aItemsInList[nTopIndex];
		}
	}			
	return CComboBox::GetTopIndex();
}

int CComboBoxAutoComplete::SetTopIndex(int nSelect)
{
	if (m_pACData!=NULL)
	{
		if (m_pACData->bFlags&ACDATA::afAutoCompleting)
		{
			int nIndex=m_pACData->aItemsInList.Find(nSelect);
			if (nIndex==-1)
				return CB_ERR;
			return CComboBox::SetTopIndex(nIndex);
		}	
	}
		
	return CComboBox::SetTopIndex(nSelect);
}


int CComboBoxAutoComplete::GetLBText(int nIndex, LPSTR lpszText) const
{
	if (m_pACData==NULL)
		return CComboBox::GetLBText(nIndex, lpszText);

	if (nIndex>=m_pACData->aItems.GetSize())
		return CB_ERR;
	
	DWORD nLength=istrlenw(m_pACData->aItems[nIndex]);
	MemCopyWtoA(lpszText,m_pACData->aItems[nIndex],nLength+1);
	return nLength;
}

int CComboBoxAutoComplete::GetLBText(int nIndex, CStringA& rString) const
{
	if (m_pACData==NULL)
		return CComboBox::GetLBText(nIndex,rString);

	if (nIndex>=m_pACData->aItems.GetSize())
		return CB_ERR;
	
	rString.Copy(m_pACData->aItems[nIndex]);
	return rString.GetLength();
}

#ifdef DEF_WCHAR
int CComboBoxAutoComplete::GetLBText(int nIndex, LPWSTR lpszText) const
{
	if (m_pACData==NULL)
		return CComboBox::GetLBText(nIndex, lpszText);

	if (nIndex>=m_pACData->aItems.GetSize())
		return CB_ERR;
	
	DWORD nLength=istrlenw(m_pACData->aItems[nIndex]);
	MemCopy(lpszText,m_pACData->aItems[nIndex],nLength+1);
	return nLength;
}

int CComboBoxAutoComplete::GetLBText(int nIndex, CStringW& rString) const
{
	if (m_pACData==NULL)
		return CComboBox::GetLBText(nIndex,rString);

	if (nIndex>=m_pACData->aItems.GetSize())
		return CB_ERR;
	
	rString.Copy(m_pACData->aItems[nIndex]);
	return rString.GetLength();
}
#endif

int CComboBoxAutoComplete::GetLBTextLen(int nIndex) const
{
	if (m_pACData==NULL)
		return CComboBox::GetLBTextLen(nIndex);

	if (nIndex>=m_pACData->aItems.GetSize())
		return CB_ERR;
	
	return istrlenw(m_pACData->aItems[nIndex]);
}



int CComboBoxAutoComplete::FindStringExact(int nIndex, LPCSTR lpszFind) const
{
	if (m_pACData==NULL)
		return CComboBox::FindStringExact(nIndex, lpszFind);

	for (;nIndex<m_pACData->aItems.GetSize();nIndex++)
	{
		if (wcscmp(m_pACData->aItems[nIndex],A2W(lpszFind))==0)
			return nIndex;
	}
	return -1;
}

#ifdef DEF_WCHAR
int CComboBoxAutoComplete::FindStringExact(int nIndex, LPCWSTR lpszFind) const
{
	if (m_pACData==NULL)
		return CComboBox::FindStringExact(nIndex, lpszFind);

	for (;nIndex<m_pACData->aItems.GetSize();nIndex++)
	{
		if (wcscmp(m_pACData->aItems[nIndex],lpszFind)==0)
			return nIndex;
	}
	return -1;
}
#endif

BOOL CComboBoxAutoComplete::GetDroppedState() const
{
	if (m_pACData!=NULL)
	{
		if (m_pACData->bFlags&ACDATA::afAutoCompleting)
			return FALSE;
	}
	return CComboBox::GetDroppedState();
}


void CComboBoxAutoComplete::ShowDropDown(BOOL bShowIt)
{
	if (m_pACData!=NULL)
	{
		if (m_pACData->bFlags&ACDATA::afAutoCompleting)
		{
			// TODO: You know
		
		}
	}
	return CComboBox::ShowDropDown(bShowIt);
}


int CComboBoxAutoComplete::AddString(LPCSTR lpszString)
{
	if (m_pACData==NULL)
		return CComboBox::AddString(lpszString);

	return 0;
}

int CComboBoxAutoComplete::DeleteString(UINT nIndex)
{
	if (m_pACData==NULL)
		return CComboBox::DeleteString(nIndex);

	return 0;
}

int CComboBoxAutoComplete::InsertString(int nIndex, LPCSTR lpszString)
{
	if (m_pACData==NULL)
		return CComboBox::InsertString(nIndex, lpszString);

	return 0;
}

void CComboBoxAutoComplete::ResetContent()
{
	if (m_pACData==NULL)
		return CComboBox::ResetContent();

}


int CComboBoxAutoComplete::FindString(int nStartAfter,LPCSTR lpszString) const
{
	if (m_pACData==NULL)
		return CComboBox::FindString(nStartAfter,lpszString);

	return 0;
}

int CComboBoxAutoComplete::SelectString(int nStartAfter,LPCSTR lpszString)
{
	if (m_pACData==NULL)
		return CComboBox::SelectString(nStartAfter,lpszString);

	return 0;
}


BOOL CComboBoxAutoComplete::HandleOnCommand(WORD wNotifyCode)
{
	if (m_pACData==NULL)
		return FALSE;

	return FALSE;
}


#ifdef DEF_WCHAR


int CComboBoxAutoComplete::AddString(LPCWSTR lpszString)
{
	if (m_pACData==NULL)
		return CComboBox::AddString(lpszString);

	return 0;
}

int CComboBoxAutoComplete::InsertString(int nIndex, LPCWSTR lpszString)
{
	if (m_pACData==NULL)
		return CComboBox::InsertString(nIndex, lpszString);

	return 0;
}

int CComboBoxAutoComplete::FindString(int nStartAfter,LPCWSTR lpszString) const
{
	if (m_pACData==NULL)
		return CComboBox::FindString(nStartAfter,lpszString);

	return 0;
}

int CComboBoxAutoComplete::SelectString(int nStartAfter,LPCWSTR lpszString)
{
	if (m_pACData==NULL)
		return CComboBox::SelectString(nStartAfter,lpszString);

	return 0;
}

#endif



//CBufferAllocator<BYTE*,2000,BUFFERALLOC_EXTRALEN> FileTypeAllocator;	
//CBufferAllocatorThreadSafe<BYTE*,2000,BUFFERALLOC_EXTRALEN> FileTypeAllocator;	

LPSTR g_szBuffer=NULL; 
LPWSTR g_szwBuffer=NULL; 

BOOL CLocateDlgThread::InitInstance()
{
	DebugFormatMessage("CLocateDlgThread::InitInstance(), thread is 0x%X",GetCurrentThreadId());

	CWinThread::InitInstance();
	CoInitialize(NULL);
	
	RegisterDataTimeExCltr();

	m_pMainWnd=m_pLocate=new CLocateDlg;
	m_pLocate->Create(NULL);
	
	// Settings transparency
	BOOL(WINAPI * pSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD)=(BOOL(WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(GetModuleHandle("user32.dll"),"SetLayeredWindowAttributes");
	if (pSetLayeredWindowAttributes!=NULL)
	{
		CRegKey2 RegKey;
		if (RegKey.OpenKey(HKCU,"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		{
			DWORD dwTransparency=0;
			if (RegKey.QueryValue("Transparency",dwTransparency))
			{
				if (dwTransparency>0)
				{
					m_pLocate->SetWindowLong(CWnd::gwlExStyle,WS_EX_CONTROLPARENT|WS_EX_LAYERED);
					pSetLayeredWindowAttributes(*m_pLocate,0,BYTE(255-min(dwTransparency,255)),LWA_ALPHA);
				}
			}				
				
		}
	}


	
	RegisterDialog(*m_pLocate);
	return TRUE;
}

int CLocateDlgThread::ExitInstance()
{
	delete m_pLocate;
	m_pLocate=NULL;
	InterlockedExchangePointer((PVOID*)&GetLocateAppWnd()->m_pLocateDlgThread,NULL);
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

				if (CLocateDlg::DetailType(m_pLocate->m_pListCtrl->GetColumnIDFromSubItem(ht.iSubItem))==CLocateDlg::Name)
				{
					TOOLINFOW tii;
					tii.cbSize = TTTOOLINFOW_V2_SIZE;
					
					tii.uFlags = TTF_IDISHWND;
					tii.hwnd   = *m_pLocate;
					tii.uId    = (UINT)pMsg->hwnd;
					tii.hinst  = NULL;
					tii.lpszText  = LPSTR_TEXTCALLBACKW;
					tii.lParam = 0;
					
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


					
					CStringW sText;
					m_pLocate->m_pListCtrl->GetItemText(sText,ht.iItem,ht.iSubItem);
					int nWidth=m_pLocate->m_pListCtrl->GetStringWidth(sText)+12;
					
					// InFolder need also space for icon
					if (CLocateDlg::DetailType(m_pLocate->m_pListCtrl->GetColumnIDFromSubItem(ht.iSubItem))==CLocateDlg::InFolder)
						nWidth+=rc2.Width()+5;

					if (nWidth>rc.Width())
					{
						TOOLINFOW tii;
						tii.cbSize = TTTOOLINFOW_V2_SIZE;
		
						tii.uFlags = TTF_IDISHWND;
						tii.hwnd   = *m_pLocate;
						tii.uId    = (UINT)pMsg->hwnd;
						tii.hinst  = NULL;
						tii.lpszText  = LPSTR_TEXTCALLBACKW;
						tii.lParam = 0;

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

CLocateDlg::~CLocateDlg()
{
	DebugFormatMessage("CLocateDlg::~CLocateDlg() this is %X",DWORD(this));

	ASSERT(m_hActivePopupMenu==NULL);
	ASSERT(m_pActiveContextMenu==NULL);

	if (m_pDesktopFolder!=NULL)
		m_pDesktopFolder->Release();

	DeleteCriticalSection(&m_csAnimBitmaps);
}

CLocateDlg::ViewDetails* CLocateDlg::GetDefaultDetails()
{
	ViewDetails aDetails[]={
		{/*Name,*/IDS_LISTNAME,TRUE,LVCFMT_LEFT,200},
		{/*InFolder,*/IDS_LISTINFOLDER,TRUE,LVCFMT_LEFT,300},
		{/*FullPath,*/IDS_LISTFULLPATH,FALSE,LVCFMT_LEFT,300},
		{/*ShortFileName,*/IDS_LISTSHORTFILENAME,FALSE,LVCFMT_LEFT,200},
		{/*ShortFilePath,*/IDS_LISTSHORTFILEPATH,FALSE,LVCFMT_LEFT,300},
		{/*FileSize,*/IDS_LISTSIZE,TRUE,LVCFMT_RIGHT,70},
		{/*FileType,*/IDS_LISTTYPE,TRUE,LVCFMT_LEFT,130},
		{/*DateModified,*/IDS_LISTMODIFIED,TRUE,LVCFMT_LEFT,100},
		{/*DateCreated,*/IDS_LISTCREATED,FALSE,LVCFMT_LEFT,100},
		{/*DateAccessed,*/IDS_LISTACCESSED,FALSE,LVCFMT_LEFT,100},
		{/*Attributes,*/IDS_LISTATTRIBUTES,FALSE,LVCFMT_LEFT,70},
		{/*ImageDimensions,*/IDS_LISTIMAGEINFO,FALSE,LVCFMT_LEFT,150},
		{/*Owner,*/IDS_LISTOWNER,FALSE,LVCFMT_LEFT,130},
		{/*Database,*/IDS_LISTDATABASE,FALSE,LVCFMT_LEFT,70},
		{/*DatabaseDescription,*/IDS_LISTDATABASEDESC,FALSE,LVCFMT_LEFT,150},
		{/*DatabaseArchive,*/IDS_LISTDATABASEFILE,FALSE,LVCFMT_LEFT,150},
		{/*VolumeLabel,*/IDS_LISTVOLUMELABEL,FALSE,LVCFMT_LEFT,100},
		{/*VolumeSerial,*/IDS_LISTVOLUMESERIAL,FALSE,LVCFMT_LEFT,90},
		{/*VOlumeFileSystem,*/IDS_LISTVOLUMEFILESYSTEM,FALSE,LVCFMT_LEFT,90},
		{/*MD5sum,*/IDS_LISTMD5SUM,FALSE,LVCFMT_LEFT,100},
		{/*Author,*/IDS_LISTAUTHOR,FALSE,LVCFMT_LEFT,100},
		{/*Title,*/IDS_LISTTITLE,FALSE,LVCFMT_LEFT,100},
		{/*Subject,*/IDS_LISTSUBJECT,FALSE,LVCFMT_LEFT,100},
		{/*Category,*/IDS_LISTCATEGORY,FALSE,LVCFMT_LEFT,100},
		{/*Pages,*/IDS_LISTPAGES,FALSE,LVCFMT_LEFT,100},
		{/*Comments,*/IDS_LISTCOMMENTS,FALSE,LVCFMT_LEFT,100},
		{/*Description,*/IDS_LISTDESCRIPTION,FALSE,LVCFMT_LEFT,100},
		{/*FileVersion,*/IDS_LISTFILEVERSION,FALSE,LVCFMT_LEFT,100},
		{/*ProductName,*/IDS_LISTPRODUCTNAME,FALSE,LVCFMT_LEFT,100},
		{/*ProductVersion,*/IDS_LISTPRODUCTVERSION,FALSE,LVCFMT_LEFT,100}
	};

	ViewDetails* pRet=new ViewDetails[sizeof(aDetails)/sizeof(ViewDetails)];
	CopyMemory(pRet,aDetails,sizeof(aDetails));
	return pRet;
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
	
	m_hNextClipboardViewer=SetClipboardViewer(*this);

	m_CircleBitmap.LoadBitmap(IDB_CIRCLE);
	
	m_pTabCtrl=new CTabCtrl(GetDlgItem(IDC_TAB));
	m_pListCtrl=new CListCtrlEx(GetDlgItem(IDC_FILELIST));
	m_pStatusCtrl=new CStatusBarCtrl(GetDlgItem(IDC_STATUS));

	if (IsUnicodeSystem())
	{
		m_pTabCtrl->SetUnicodeFormat(TRUE);
		m_pListCtrl->SetUnicodeFormat(TRUE);
		m_pStatusCtrl->SetUnicodeFormat(TRUE);
	}
	

	
	// Setting tab control labels	
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

	ViewDetails* pDetails=GetDefaultDetails();
	for (int i=0;i<TypeCount;i++)
	{
		m_pListCtrl->InsertColumn(DetailType(i),pDetails[i].nString,
			pDetails[i].bShow,pDetails[i].nAlign,pDetails[i].nWidth,LanguageSpecificResource);
	}
	delete[] pDetails;

	m_pListCtrl->SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP,LVS_EX_HEADERDRAGDROP);
	m_pListCtrl->LoadColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\General","ListWidths");

	// Initializing drop target
	OleInitialize(NULL);
	m_pDropTarget=new CFileTarget;
	m_pDropTarget->AutoDelete();
	m_pDropTarget->AddRef();
	RegisterDragDrop(*m_pListCtrl,m_pDropTarget);



	// Creating windows
	m_NameDlg.Create(*this);
	m_NameDlg.ShowWindow(swShow);
	m_SizeDateDlg.Create(*this);
	DWORD dwError=GetLastError();
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
	m_nPresetsButtonOffset=char(rcTemp.left-rcTemp2.left);
    	
	

	// Loading registry
	LoadRegistry();
	// Refreshing dialog box
	m_NameDlg.InitDriveBox(TRUE);


	// Check default shortcut integrity
#ifdef _DEBUG
	CArrayFP<CShortcut*> aDefaultShortcuts;
	ASSERT(CShortcut::GetDefaultShortcuts(aDefaultShortcuts,CShortcut::loadAll));
#endif

	// Set dialog more
	SetDialogMode(FALSE);

	// Set result list font
	SetResultListFont();
		
	// Setting topmost mode if needed
	if (GetFlags()&fgDialogTopMost)
		SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	
	// Setting list control imagelists and style
	SetSystemImagelists(m_pListCtrl,&m_AdvancedDlg.m_hDefaultTypeIcon);
	SetListSelStyle();
	if (GetFlags()&fgLVAllowInPlaceRenaming)
		m_pListCtrl->ModifyStyle(0,LVS_EDITLABELS);


	


    // Setting icons
	HICON hIcon=(HICON)LoadImage(IDI_APPLICATIONICON,IMAGE_ICON,32,32,LR_SHARED);
	SetIcon(hIcon,TRUE);
	SetClassLong(gclHIcon,(LONG_PTR)hIcon);
	hIcon=(HICON)LoadImage(IDI_APPLICATIONICON,IMAGE_ICON,16,16,LR_SHARED);
	SetIcon(hIcon,FALSE);
	SetClassLong((CWnd::ClassLongIndex)GCLP_HICONSM,(LONG_PTR)hIcon);


	// Setting tooltips
	InitTooltips();
	
	// Enabling multidirectory support if needed
	m_NameDlg.EnableMultiDirectorySupport(GetFlags()&fgNameMultibleDirectories?TRUE:FALSE);
	
	// Loading texts which are used at last time
	if (GetFlags()&fgDialogRememberFields)
		LoadDialogTexts();

	// Sorting
	SetSorting();
	
	// Load shortcuts and actions
	SetShortcuts();	
	LoadResultlistActions();


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
	mii.dwTypeData="Show debug related information";
	mii.dwItemData=0;
	mii.fType=MFT_STRING;
	smenu.InsertMenu(0x77,FALSE,&mii);
	
#endif

	m_NameDlg.SetFocus();
	m_NameDlg.m_Name.SetFocus();
	
	// Make title
	CStringW title;
	title.LoadString(IDS_TITLE);
	title.AddString(IDS_ALLFILES);
	
	if (GetLocateApp()->m_nInstance>0)
		title << L" (" << DWORD(GetLocateApp()->m_nInstance+1) << L')';
	SetText(title);
	
	return FALSE;
}

void CLocateDlg::SetResultListFont()
{
	if (m_hDialogFont==NULL)
		m_hDialogFont=m_pListCtrl->GetFont();

	
	HFONT hOldFont=m_pListCtrl->GetFont();
	HFONT hNewFont=m_hDialogFont;

	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::defRead)==ERROR_SUCCESS)
	{
		LOGFONT lFont;
		if (RegKey.QueryValue("ResultListFont",(LPSTR)&lFont,sizeof(LOGFONT))==sizeof(LOGFONT))
			hNewFont=CreateFontIndirect(&lFont);
		
	}
	
	m_pListCtrl->SetFont(hNewFont);

	if (m_pListTooltips!=NULL)
	{
		if (m_pListTooltips->GetFont()==hOldFont)
			m_pListTooltips->SetFont(hNewFont);
	}

	if (hOldFont!=m_hDialogFont)
		DeleteObject(hOldFont);
}


BOOL CLocateDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch(wID)
	{
	case IDC_FILELIST:
		if (hControl==NULL && wNotifyCode==1 && GetFlags()&fgLargeMode) // Accelerator
			SetFocus(IDC_FILELIST);
		break;
	case IDC_OK:
		if (::IsWindowEnabled(GetDlgItem(IDC_OK)))
			OnOk(hControl==NULL && wNotifyCode==1,FALSE);
		break;
	case IDM_FINDUSINGDBS:
		if (::IsWindowEnabled(GetDlgItem(IDC_OK)))
			OnOk(hControl==NULL && wNotifyCode==1,TRUE);
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
	case IDM_HELPTOPICS:
		HtmlHelp(HH_DISPLAY_TOPIC,0);
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
		SortItems(Name);
		break;
	case IDM_ARRANGEFOLDER:
		SortItems(InFolder);
		break;
	case IDM_ARRANGETYPE:
		SortItems(FileType);
		break;
	case IDM_ARRANGESIZE:
		SortItems(FileSize);
		break;
	case IDM_ARRANGEDATE:
		SortItems(DateModified);
		break;
	case IDM_ARRANGEEXTENSION:
		SortItems(Extension);
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
	case IDM_SETTOOL:
		OnSettingsTool();
		break;
	case IDM_SELECTDETAILS:
		OnSelectDetails();
		break;
	case IDM_GLOBALUPDATEDB:
	case IDM_UPDATEDATABASES:
	case IDM_STOPUPDATING:
		//CLocateAppWnd handles these
		GetLocateAppWnd()->PostMessage(WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),LPARAM(hControl));
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
	case IDM_EXIT:
		GetLocateAppWnd()->PostMessage(WM_COMMAND,IDM_EXIT,NULL);
		break;
	case IDM_CUT:
	case IDM_EDIT_CUT:
		OnCopy(TRUE);
		break;
	case IDM_COPY:
	case IDM_EDIT_COPY:
		OnCopy(FALSE);
		break;
	case IDM_OPENCONTAININGFOLDER:
		OnOpenFolder(TRUE);
		break;
	case IDM_CREATESHORTCUT:
		OnCreateShortcut();
		break;
	case IDM_DELETE:
		OnDelete();
		break;
	case IDM_RENAME:
		OnRenameFile();
		break;
	case IDM_REMOVEFROMTHISLIST:
		OnRemoveFromThisList();
		break;
	case IDC_SELECTALLKEY:
		if (GetFocus()!=GetDlgItem(IDC_FILELIST))
			break;
	case IDM_EDIT_SELECTALL:
		OnSelectAll();
		break;
	case IDM_EDIT_INVERTSELECTION:
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
	case IDM_COMPUTEMD5SUM:
		OnComputeMD5Sums(FALSE);
		break;
	case IDM_MD5SUMSFORSAMESIZEFILES:
		OnComputeMD5Sums(TRUE);
		break;
	case IDM_SHOWFILEINFORMATION:
		OnShowFileInformation();
		break;
	case IDM_REMOVEDELETEDFILES:
		OnRemoveDeletedFiles();
		break;
	case IDM_CHANGEFILENAME:
		OnChangeFileName();
		break;
	case IDC_NAME:
	case IDC_TYPE:
	case IDC_LOOKIN:
	case IDC_BROWSE:
	case IDC_NOSUBDIRECTORIES:
		// This is to ensure that these conrols get focus e.g. when alt+n is pressed
		return (BOOL)m_NameDlg.SendMessage(WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),(LPARAM)hControl);
	case IDC_CHECKMINIMUMSIZE:
	case IDC_MINIMUMSIZE:
	case IDC_MINSIZETYPE:
	case IDC_CHECKMAXIMUMSIZE:
	case IDC_MAXIMUMSIZE:
	case IDC_MAXSIZETYPE:
	case IDC_CHECKMINDATE:
	case IDC_MINDATE:
	case IDC_MINDATEMODE:
	case IDC_MINTYPE:
	case IDC_CHECKMAXDATE:
	case IDC_MAXDATE:
	case IDC_MAXDATEMODE:
	case IDC_MAXTYPE:
		// This is to ensure that these conrols get focus e.g. when alt+n is pressed
		return (BOOL)m_SizeDateDlg.SendMessage(WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),(LPARAM)hControl);
	case IDC_CHECK:
	case IDC_MATCHWHOLENAME:
	case IDC_FILETYPE:
	case IDC_CONTAINDATACHECK:
	case IDC_CONTAINDATA:
	case IDC_DATAMATCHCASE:
	case IDC_REPLACESPACES:
	case IDC_USEWHOLEPATH:
		// This is to ensure that these conrols get focus e.g. when alt+n is pressed
		return (BOOL)m_AdvancedDlg.SendMessage(WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),(LPARAM)hControl);
	case IDC_NEXTCONTROL:
		DefDlgProc(*this,WM_NEXTDLGCTL,FALSE,0);
		//::SetFocus(GetNextDlgTabItem(GetFocus(),FALSE));
		break;
	case IDC_PREVCONTROL:
		DefDlgProc(*this,WM_NEXTDLGCTL,TRUE,0);
		//::SetFocus(GetNextDlgTabItem(GetFocus(),TRUE));
		break;
	case IDM_SAVERESULT:
		OnSaveResults();
		break;
	case IDM_PROPERTIES:
		OnProperties();
		break;
	case IDM_DEFOPEN:
		OnExecuteFile(NULL);
		break;
	case IDM_LOOKINNEWSELECTION:
	case IDM_LOOKINREMOVESELECTION:
	case IDM_LOOKINNEXTSELECTION:
	case IDM_LOOKINPREVSELECTION:
		return m_NameDlg.OnCommand(wID,wNotifyCode,hControl);
	default:
		// IDM_ should be in descending order
		if (wID>=IDM_DEFSHORTCUTITEM && wID<IDM_DEFSHORTCUTITEM+m_aActiveShortcuts.GetSize())
		{
			CShortcut** pShortcutList=m_aActiveShortcuts[wID-IDM_DEFSHORTCUTITEM];

			while (*pShortcutList!=NULL)
			{
				// Checking Win key state
				if ((GetKeyState(VK_LWIN)|GetKeyState(VK_RWIN)) & 0x8000)
				{
					if (pShortcutList[0]->m_bModifiers&CShortcut::ModifierWin)
					{
						if (pShortcutList[0]->IsWhenAndWhereSatisfied(*this))
							pShortcutList[0]->ExecuteAction();
					}
				}
				else if (!(pShortcutList[0]->m_bModifiers&CShortcut::ModifierWin))
				{
					if (pShortcutList[0]->IsWhenAndWhereSatisfied(*this))
						pShortcutList[0]->ExecuteAction();
				}
				else
				{
					// Shortcut wants Win but it is not pressed, acceltable has stolen
					// enevent, send that event to control
					HWND hControl=GetFocus();
					if (hControl!=NULL)
					{
						DWORD lParam=1;
						if ((GetKeyState(VK_LMENU)|GetKeyState(VK_RMENU)) & 0x8000)
							lParam|=1<<29;
						::SendMessage(hControl,WM_CHAR,pShortcutList[0]->m_bVirtualKey,lParam);
					}

				}
	
				pShortcutList++;
			}
		}
		else if (wID>=IDM_DEFCOLSELITEM && wID<IDM_DEFCOLSELITEM+1000)
			m_pListCtrl->ColumnSelectionMenuProc(wID,IDM_DEFCOLSELITEM);
		else if (wID>=IDM_DEFSENDTOITEM && wID<IDM_DEFSENDTOITEM+1000)
			OnSendToCommand(wID);
		else if (m_pActiveContextMenu!=NULL && wID>=IDM_DEFCONTEXTITEM && wID<IDM_DEFCONTEXTITEM+1000)
			OnContextMenuCommands(wID);
		else if (wID>=IDM_DEFUPDATEDBITEM && wID<IDM_DEFUPDATEDBITEM+1000)
			GetLocateAppWnd()->SendMessage(WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),LPARAM(hControl));
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
			CRegKey2::GetCommonKey()+"\\General","ListWidths");
	}

	switch(id)
	{
	case 0:
		if ((dwStyle & LVS_TYPEMASK)!=LVS_ICON || bInit)
		{
			m_pListCtrl->SetStyle((dwStyle & ~LVS_TYPEMASK)|LVS_ICON);
			m_pListCtrl->Arrange(LVA_DEFAULT);
		}
		break;
	case 1:
		if ((dwStyle & LVS_TYPEMASK)!=LVS_SMALLICON || bInit)
		{
			m_pListCtrl->SetStyle((dwStyle & ~LVS_TYPEMASK)|LVS_SMALLICON);
			m_pListCtrl->Arrange(LVA_DEFAULT);
		}
		break;
	case 2:
		if ((dwStyle  &LVS_TYPEMASK)!=LVS_LIST || bInit)
		{
			m_pListCtrl->SetStyle((dwStyle & ~LVS_TYPEMASK)|LVS_LIST);
			m_pListCtrl->Arrange(LVA_DEFAULT);
		}
		break;
	case 3:
		if ((dwStyle & LVS_TYPEMASK)!=LVS_REPORT || bInit)
			m_pListCtrl->SetStyle((dwStyle & ~LVS_TYPEMASK)|LVS_REPORT);
		break;
	}

	SetMenuCheckMarkForListStyle();
	return TRUE;
}


void CLocateDlg::OnHelp(LPHELPINFO lphi)
{
	//CTargetWnd::OnHelp(lphi);

	// Menu item
	if (HtmlHelp(HH_HELP_CONTEXT,lphi->iCtrlId)==NULL)
	{
		// No topic found, show topics window
		HtmlHelp(HH_DISPLAY_TOPIC,0);
	}
	
}
	
void CLocateDlg::SetMenuCheckMarkForListStyle()
{
	ASSERT(m_pListCtrl!=NULL);

	CMenu menu(GetMenu());

	DWORD dwListStyle=m_pListCtrl->GetStyle()&LVS_TYPEMASK;


	// Check marks for menu
	menu.CheckMenuItem(IDM_LARGEICONS,MF_BYCOMMAND|(dwListStyle==LVS_ICON?MF_CHECKED:MF_UNCHECKED));
	menu.CheckMenuItem(IDM_SMALLICONS,MF_BYCOMMAND|(dwListStyle==LVS_SMALLICON?MF_CHECKED:MF_UNCHECKED));
	menu.CheckMenuItem(IDM_LIST,MF_BYCOMMAND|(dwListStyle==LVS_LIST?MF_CHECKED:MF_UNCHECKED));
	menu.CheckMenuItem(IDM_DETAILS,MF_BYCOMMAND|(dwListStyle==LVS_REPORT?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LARGEICONS,MF_BYCOMMAND|(dwListStyle==LVS_ICON?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_SMALLICONS,MF_BYCOMMAND|(dwListStyle==LVS_SMALLICON?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LIST,MF_BYCOMMAND|(dwListStyle==LVS_LIST?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_DETAILS,MF_BYCOMMAND|(dwListStyle==LVS_REPORT?MF_CHECKED:MF_UNCHECKED));
			
	
	// Enable/disable "Line up icons" and "Auto arrange"
	if (dwListStyle==LVS_LIST || dwListStyle==LVS_REPORT)
	{
		menu.EnableMenuItem(IDM_LINEUPICONS,MF_BYCOMMAND|MF_GRAYED);
		menu.EnableMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LINEUPICONS,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_GRAYED);
	
		menu.EnableMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_GRAYED);
	}
	else
	{
		menu.EnableMenuItem(IDM_LINEUPICONS,MF_BYCOMMAND|MF_ENABLED);
		menu.EnableMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_ENABLED);

		EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_LINEUPICONS,MF_BYCOMMAND|MF_ENABLED);
		EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_ENABLED);

		if (((CLocateApp*)GetApp())->m_wComCtrlVersion>=0x0600)
		{
			menu.EnableMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_ENABLED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_ENABLED);
		}
		else
		{
			menu.EnableMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_GRAYED);
			EnableMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_GRAYED);
		}
	}
			
	
	if (m_pListCtrl->GetStyle()&LVS_AUTOARRANGE)
	{
		menu.CheckMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_CHECKED);
		CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_CHECKED);
	}

	if (m_pListCtrl->GetExtendedListViewStyle()&LVS_EX_SNAPTOGRID)
	{
		menu.CheckMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_CHECKED);
		CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_CHECKED);
	}
}

BOOL CLocateDlg::UpdateSettings()
{
	CRegKey2 RegKey;
	DWORD temp=3;
	
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
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

		if ((m_dwExtraFlags&efItemUpdatingMask)==efDisableItemUpdating)
			m_dwExtraFlags|=efLVNoUpdateWhileSorting;


	}
	m_pListCtrl->RedrawItems(0,m_pListCtrl->GetItemCount());
	SetListSelStyle();


	// Update tooltip setting	
	InitTooltips();


	// Set result list font
	SetResultListFont();



    
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
	ResetFileNotificators();

	SetDialogMode(GetFlags()&fgLargeMode);

	if (GetFlags()&fgLVAllowInPlaceRenaming)
		m_pListCtrl->ModifyStyle(0,LVS_EDITLABELS);
	else
		m_pListCtrl->ModifyStyle(LVS_EDITLABELS,0);



	BOOL(WINAPI * pSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD)=(BOOL(WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(GetModuleHandle("user32.dll"),"SetLayeredWindowAttributes");
	if (pSetLayeredWindowAttributes!=NULL)
	{
		// Transparency
		if (RegKey.OpenKey(HKCU,"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		{
			if (!RegKey.QueryValue("Transparency",temp))
				temp=0;

			if (temp>255)
				temp=255;

			DWORD dwExtraStyle=GetExStyle();
			if (temp==0)
			{
				// Disabling layer
				if (dwExtraStyle&WS_EX_LAYERED)
					SetExStyle(dwExtraStyle&~WS_EX_LAYERED);
			}
			else
			{
				if (!(dwExtraStyle&WS_EX_LAYERED))
					SetExStyle(dwExtraStyle|WS_EX_LAYERED);
				pSetLayeredWindowAttributes(*this,0,BYTE(255-temp),LWA_ALPHA);
			}
		}
	}



	// Setting topmost status
	if (GetFlags()&fgDialogTopMost)
	{
		if (!(GetExStyle()&WS_EX_TOPMOST))
			SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	}
	else if (GetExStyle()&WS_EX_TOPMOST)
		SetWindowPos(HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);




	// Background operations
	if ((GetExtraFlags()&efItemUpdatingMask)==efEnableItemUpdating)
	{
		// This would mess things
		if (m_pFileNotificationsThread!=NULL)
		{
			MSG msg;
			while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
			{
				do {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				} while (PeekMessage(&msg,NULL,0,0,PM_REMOVE));
				Sleep(50);
			}
			
			m_pFileNotificationsThread->Stop();
		}
		StartBackgroundOperations(); 
	}
	else
		StopBackgroundOperations();


	
	return TRUE;
}

void CLocateDlg::OnSelectAll()
{
	if (m_pListCtrl==NULL)
		return;
	if (m_pListCtrl->GetItemCount()==0)
		return;

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
	if (m_pListCtrl==NULL)
		return;
	if (m_pListCtrl->GetItemCount()==0)
		return;

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
	{
		InterlockedExchangePointer((PVOID*)&m_pBackgroundUpdater,new CBackgroundUpdater(m_pListCtrl));
		m_pBackgroundUpdater->CreateEventsAndMutex();
	}

	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;


	if (GetExtraFlags()&efEnableFSTracking)
	{
		if (m_pFileNotificationsThread==NULL)
			InterlockedExchangePointer((PVOID*)&m_pFileNotificationsThread,new CCheckFileNotificationsThread);
		
		m_pFileNotificationsThread->Start();
	}
	else if (m_pFileNotificationsThread!=NULL)
		m_pFileNotificationsThread->CouldStop();

	//DebugMessage("CLocateDlg::StartBackgroundOperations():  END");
	
}

void CLocateDlg::StopBackgroundOperations()
{
	//DebugMessage("CLocateDlg::StopBackgroundOperations():  BEGIN");
	if (IsLocating())
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
		}
	}
	else
	{
		if (m_dwFlags&fgLargeMode)
		{
			CRect rect;
			m_dwFlags&=~fgLargeMode;
			m_pListCtrl->SetStyle(m_pListCtrl->GetStyle()&~WS_TABSTOP);
			GetWindowRect(&rect);
			m_nLargeY=rect.Height();
			SetWindowPos(HWND_BOTTOM,0,0,rect.Width(),m_nMaxYMinimized,
				SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);

			// Now, terminate available background updater
			RemoveResultsFromList();
			
			ISDLGTHREADOK
			if (g_szBuffer!=NULL)
			{
				delete[] g_szBuffer;
				g_szBuffer=NULL;
			}
			if (g_szwBuffer!=NULL)
			{
				delete[] g_szwBuffer;
				g_szwBuffer=NULL;
			}
			m_pListCtrl->ShowWindow(swHide);
		}
	}
}

void CLocateDlg::InitTooltips()
{
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
	else 
	{
		if (m_pListTooltips==NULL)
		{
			m_pListTooltips=new CToolTipCtrl;
			m_pListTooltips->Create(*this);
			m_pListTooltips->SetFont(m_pListCtrl->GetFont());
			if (IsUnicodeSystem())
				m_pListCtrl->SetUnicodeFormat(TRUE);
			
			m_iTooltipItem=-1;
			m_iTooltipSubItem=-1;
			m_bTooltipActive=FALSE;
		}

		// Set delay times
		CRegKey2 RegKey;
		if (RegKey.OpenKey(HKCU,"\\General",CRegKey::defRead)==ERROR_SUCCESS)
		{
			DWORD dwDelay;
			if (RegKey.QueryValue("TooltipDelayAutopop",dwDelay))
				m_pListTooltips->SetDelayTime(dwDelay,CToolTipCtrl::autoPop);
			else
				m_pListTooltips->SetDelayTime(UINT(-1),CToolTipCtrl::autoPop);

			if (RegKey.QueryValue("TooltipDelayInitial",dwDelay))
			{
				m_pListTooltips->SetDelayTime(dwDelay,CToolTipCtrl::initial);
				m_pListTooltips->SetDelayTime(dwDelay/5,CToolTipCtrl::reshow);
			}
			else
			{
				m_pListTooltips->SetDelayTime(UINT(-1),CToolTipCtrl::initial);
				m_pListTooltips->SetDelayTime(UINT(-1),CToolTipCtrl::reshow);
			}

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

void CLocateDlg::OnOk(BOOL bShortcut,BOOL bSelectDatabases)
{
	DlgDebugMessage("CLocateDlg::OnOk BEGIN");
	
	CWaitCursor wait;
	CStringW Name,Title;
	CArrayFAP<LPWSTR> aExtensions,aDirectories,aNames;
	DWORD nRet;
	BOOL bPlusOrMinusFound=FALSE;

	if (m_pBackgroundUpdater!=NULL)
		m_pBackgroundUpdater->Stop();
	if (m_pFileNotificationsThread!=NULL)
		m_pFileNotificationsThread->Stop();
	if (IsLocating())
		m_pLocater->StopLocating();
	
	// Deleting previous items
	RemoveResultsFromList();
	
	if (m_pListTooltips!=NULL)
		DeleteTooltipTools();

	if (!bShortcut && GetKeyState(VK_CONTROL)&0x8000)
		bSelectDatabases=TRUE;


	// If dialog is not large mode, change it
	SetDialogMode(TRUE);
	// Clearing possible exclamation icons
	m_pStatusCtrl->SetText("",STATUSBAR_LOCATEICON,0);
	m_pStatusCtrl->SetText("",STATUSBAR_UPDATEICON,0);
	
	
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
			CRegKey2::GetCommonKey()+"\\Dialogs\\SelectDatabases/Locate");
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

	// Create locater object
	m_pLocater=new CLocater(*pDatabases);



	// Calling routines for subdialogs
	m_SizeDateDlg.OnOk(m_pLocater);
	nRet=m_AdvancedDlg.OnOk(m_pLocater);

	

	
	// Checking name, inserting asterisks etc..
	if (!Name.IsEmpty()) 
	{
		if (Name[0]==':') // Is regular expression
		{
			Name.DelChar(0);
			nRet|=CAdvancedDlg::flagNameIsRegularExpression;
			if (Name[0]==':')
			{
				Name.DelChar(0);
				nRet|=CAdvancedDlg::flagUseWholePath;
			}
			else if (Name[0]==' ')
				Name.DelChar(0);
		}
		else
		{
			int i=0;
			UINT nIndex;

			if (nRet&CAdvancedDlg::flagReplaceSpaces)
			{
				// Replacing spaces with asterisks
				for (i=0;i<Name.GetLength();i++)
				{
					if (Name[i]==' ')
						Name[i]='*';
				}
			}

			// Removing extra asterisks
			for (i=0;i<Name.GetLength();i++)
			{
				if (Name[i]==L'*')
				{
					while (Name[i+1]==L'*')
						Name.DelChar(i+1);
				}
			}

			
			LPCWSTR pStr=Name;
			
			// Parse Name string
			for(;;)
			{
				
				// First, if logical operations can be used, check is + or - present
				enum {
					NotSpecified,
					MustExist, // + found
					MustNotExist // - found
				} nType=NotSpecified;
				
				if (GetExtraFlags()&efEnableLogicalOperations)
				{
					if (pStr[0]==L'+')
					{
						nType=MustExist;
						bPlusOrMinusFound=TRUE;
						pStr++;
					}
					else if (pStr[0]==L'-')
					{
						nType=MustNotExist;
						bPlusOrMinusFound=TRUE;
						pStr++;
					}
				}


				// Check whether parenthes are used
				BOOL bParenthes=FALSE;
				for (nIndex=0;pStr[nIndex]==L' ';nIndex++);
				if (pStr[nIndex]==L'\"')
				{
					// Parenthes on use
					bParenthes=TRUE;
					pStr+=nIndex+1;
					// Calculate length
					for (nIndex=0;pStr[nIndex]!=L'\"' && pStr[nIndex]!=L'\0';nIndex++);
				}
				else
				{
					// Calculate length
					for (nIndex=0;pStr[nIndex]!=L',' && pStr[nIndex]!=L';' && pStr[nIndex]!=L'\0';nIndex++);
				}
		
				if (nIndex>0)
				{
					if (nRet&CAdvancedDlg::flagMatchCase)
					{
						if (nType==NotSpecified)
							aNames.Add(alloccopy(pStr,nIndex));
						else
						{
							// Insert '+' or '-'
							WCHAR* pNewString=new WCHAR[nIndex+2];
							pNewString[0]=nType==MustExist?L'+':L'-';
							MemCopyW(pNewString+1,pStr,nIndex);
							pNewString[nIndex+1]='\0';
							aNames.Add(pNewString);
						}
					}
					else
					{
						// Inserting '*'
						
						
						// 4 is enough for '*' at the begin and end of string,
						// for '\0' and for '+' or '-'
						WCHAR* pNewString=new WCHAR[nIndex+4]; 

						if (nType!=NotSpecified)
						{
							pNewString[0]=nType==MustExist?L'+':L'-';
							// Move pointer forward
							pNewString++;
						}


						if (pStr[0]!=L'*')
						{
							// Insert asterisk to begin
							pNewString[0]=L'*';
							MemCopyW(pNewString+1,pStr,nIndex);
							
							// Insert asterisk to the end of string if thre is no 
							// that already and extension is not specified
							if (pStr[nIndex-1]!=L'*' && FirstCharIndex(pStr,L'.')==-1)
							{	
								pNewString[nIndex+1]=L'*';
								pNewString[nIndex+2]=L'\0';
							}
							else
								pNewString[nIndex+1]=L'\0';
						}
						else
						{
							sMemCopyW(pNewString,pStr,nIndex);
							
							// Insert asterisk to the end of string if thre is no 
							// that already and extension is not specified
							if (pStr[nIndex-1]!=L'*' && FirstCharIndex(pStr,L'.')==-1)
							{	
								pNewString[nIndex]=L'*';
								pNewString[nIndex+1]=L'\0';
							}
							else
								pNewString[nIndex]=L'\0';
						}

						
						if (nType!=NotSpecified)
						{
							// Move pointer back
							pNewString--;
						}

						aNames.Add(pNewString);


					}
	
			
				}				

				if (bParenthes)
				{
					if (pStr[nIndex]!=L'\"')
						break;
					pStr+=nIndex+1;

					while (*pStr==L' ')
						pStr++;
					if (*pStr!=L',')
						break;
					pStr++;
				}
				else
				{
					if (pStr[nIndex]=='\0')
						break;
					pStr+=nIndex+1;	
				}
			}
		}
	}
	
	// Extension no extensions, checking if name contains extension
	// No extension needed if "use whole path" is set
	if (nRet&CAdvancedDlg::flagUseWholePath)
	{
		aExtensions.RemoveAll();
		m_pLocater->AddAdvancedFlags(LOCATE_EXTENSIONWITHNAME|LOCATE_CHECKWHOLEPATH);
	}
    else if (aExtensions.GetSize()==0 && 
		(m_pLocater->GetAdvancedFlags()&(LOCATE_FILENAMES|LOCATE_FOLDERNAMES))!=LOCATE_FOLDERNAMES)
		m_pLocater->AddAdvancedFlags(LOCATE_EXTENSIONWITHNAME);

	// Subdirectories
	if (m_NameDlg.IsDlgButtonChecked(IDC_NOSUBDIRECTORIES))
		m_pLocater->AddAdvancedFlags(LOCATE_NOSUBDIRECTORIES);

	

	// Set funtion pointers
	m_pLocater->SetFunctions(LocateProc,LocateFoundProc,LocateFoundProcW,DWORD(this));
	
	// This is not needed anymore
	if (bSelectDatabases)
		delete pDatabases; 


	// Making title
	Title.LoadString(IDS_TITLE);
	if (nRet&CAdvancedDlg::flagNameIsRegularExpression)
	{
		if (nRet&CAdvancedDlg::flagUseWholePath)
			Title.AddString(IDS_REGULAREXPRESSIONFULLPATH);
		else
			Title.AddString(IDS_REGULAREXPRESSION);
		Title << Name;
	}
	else if (aNames.GetSize()==0)
	{
		if (aExtensions.GetSize()==1)
		{
			Title.AddString(IDS_FILESNAMED);
			Title << L"*." << aExtensions[0];
		}
		else
			Title.AddString(IDS_ALLFILES);
	}
	else
	{
		Title.AddString(IDS_FILESNAMED);
		Title << Name;
		if (aExtensions.GetSize()==1 && aNames.GetSize()==1)
			Title << L"." << aExtensions[0];
	}

	if (GetLocateApp()->m_nInstance>0)
		Title << L" (" << DWORD(GetLocateApp()->m_nInstance+1) << L')';
	SetText(Title);


	// Enable LogicalOperations
	if (bPlusOrMinusFound)
	{
		// '+' or '-' found
		m_pLocater->AddAdvancedFlags(LOCATE_LOGICALOPERATIONS);

		// Insert '+' for string which does not have '+' or '-'
		for (int i=0;i<aNames.GetSize();i++)
		{
			if (aNames[i][0]!=L'+' && aNames[i][0]!=L'-')
			{
				int nLen=istrlen(aNames[i])+1;
				LPWSTR pNew=new WCHAR[nLen+1];
				pNew[0]=L'+';
				MemCopyW(pNew+1,aNames[i],nLen);
				delete[] aNames[i];
				aNames[i]=pNew;
			}
		}
	}
				

	// LocateFoundProc uses UpdateList
	StartBackgroundOperations();

	// Save last focus
	m_hLastFocus=GetFocus();


	// Starting location
	if (!(nRet&CAdvancedDlg::flagNameIsRegularExpression))
	{
		m_pLocater->LocateFiles(TRUE,(LPCWSTR*)aNames.GetData(),aNames.GetSize(),
			(LPCWSTR*)aExtensions.GetData(),aExtensions.GetSize(),
			(LPCWSTR*)aDirectories.GetData(),aDirectories.GetSize());
	}
	else
	{
		// Regular expression
		m_pLocater->LocateFiles(TRUE,Name,nRet&CAdvancedDlg::flagRexExpIsCaleSensitive,
			(LPCWSTR*)aDirectories.GetData(),aDirectories.GetSize());
	}
	
	DlgDebugMessage("CLocateDlg::OnOk END");
	
}

BOOL CLocateDlg::LocateProc(DWORD_PTR dwParam,CallingReason crReason,UpdateError ueCode,DWORD_PTR dwInfo,const CLocater* pLocater)
{
	DbcDebugFormatMessage2("CLocateDlg::LocateProc BEGIN, reason=%d, code=%d",crReason,ueCode);
	
	switch (crReason)
	{
	case Initializing:
	{
		if (ueCode!=ueStillWorking && ueCode!=ueSuccess) // Initializing failed
		{
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(ID2W(IDS_LOCATINGFAILED),STATUSBAR_OPERATIONSTATUS,0);
			return FALSE;
		}

		// Disabling items and give focus to to whole dialog
		((CLocateDlg*)dwParam)->SendMessage(WM_ENABLEITEMS,FALSE);

		
		((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(szEmpty,STATUSBAR_MISC,0);
		((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(ID2W(IDS_LOCATING),STATUSBAR_OPERATIONSTATUS,0);
		((CLocateDlg*)dwParam)->StartLocateAnimation();

		
		// Selecting path column
		int nColumn=-1;
		if (((CLocateDlg*)dwParam)->m_nSorting==BYTE(-1))
			nColumn=((CLocateDlg*)dwParam)->m_pListCtrl->GetVisibleColumn(((CLocateDlg*)dwParam)->m_pListCtrl->GetColumnFromID(InFolder));
		else if ((((CLocateDlg*)dwParam)->m_nSorting&127)!=Extension)
			nColumn=((CLocateDlg*)dwParam)->m_pListCtrl->GetVisibleColumn(((CLocateDlg*)dwParam)->m_pListCtrl->GetColumnFromID(((CLocateDlg*)dwParam)->m_nSorting&127));
		
		if (nColumn!=-1)
			((CLocateDlg*)dwParam)->m_pListCtrl->SendMessage(LVM_FIRST+140/* LVM_SETSELECTEDCOLUMN */,nColumn,0);


		// Clearing volume information
		((CLocateDlg*)dwParam)->m_aVolumeInformation.RemoveAll();
		return TRUE;
	}
	case FinishedLocating:
	{
		((CLocateDlg*)dwParam)->StopLocateAnimation();
		((CLocateDlg*)dwParam)->SendMessage(WM_ENABLEITEMS,TRUE);


		CString NumberOfFiles;
		if (pLocater->GetNumberOfFoundFiles()>0)
		{
			if (pLocater->GetNumberOfFoundDirectories()>0)
				NumberOfFiles.Format(IDS_ITEMSFOUND,pLocater->GetNumberOfFoundFiles(),pLocater->GetNumberOfFoundDirectories());
			else
				NumberOfFiles.Format(IDS_FILESFOUND,pLocater->GetNumberOfFoundFiles());
		}
		else if (pLocater->GetNumberOfFoundDirectories()>0)
			NumberOfFiles.Format(IDS_DIRECTORIESFOUND,pLocater->GetNumberOfFoundDirectories());
		else
			NumberOfFiles.LoadString(IDS_NORESULTS);


		CStringW str;
			
		if (ueCode==ueStopped)
		{
			str.LoadString(IDS_LOCATINGCANCELLED);
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(LPCSTR(::LoadIcon(NULL,IDI_WARNING)),STATUSBAR_LOCATEICON,SBT_OWNERDRAW);
		}
		else if (ueCode==ueLimitReached)
		{
			str.LoadString(IDS_LOCATINGLIMITREACHED);
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(LPCSTR(::LoadIcon(NULL,IDI_INFORMATION)),STATUSBAR_LOCATEICON,SBT_OWNERDRAW);
		}
		else if (ueCode!=ueStillWorking && ueCode!=ueSuccess) // Locating failed
		{
			CStringW str2;
			switch (ueCode)
			{
			case ueUnknown:
				str2.LoadString(IDS_LASTERRORUNKNOWN);
				break;
			case ueOpen:
				str2.LoadString(IDS_LASTERRORCANNOTOPEN);
				break;
			case ueRead:
				str2.LoadString(IDS_LASTERRORCANNOTREAD);
				break;
			case ueAlloc:
				str2.LoadString(IDS_LASTERRORCANNOTALLOCATE);
				break;
			case ueInvalidDatabase:
				str2.LoadString(IDS_LASTERRORINVALIDDB);
				break;
			}
			if (str2.IsEmpty())
			{
				((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(LPCSTR(::LoadIcon(NULL,IDI_ERROR)),STATUSBAR_LOCATEICON,SBT_OWNERDRAW);
				str.LoadString(IDS_LOCATINGFAILED);
			}
			else
			{
				int nIndex=str2.Find(L'%');
				if (nIndex!=-1)
				{
					str2.DelChar(nIndex);
					str2.DelChar(nIndex);
					if (nIndex>0 && str2[nIndex-1]==' ')
						str2.DelChar(nIndex-1);
				}
				str.Format(IDS_LOCATINGFAILED2,(LPCWSTR)str2);
			}
		}
		else
			str.LoadString(IDS_LOCATINGSUCCESS);

		str << L", " << NumberOfFiles;			
		((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(str,STATUSBAR_OPERATIONSTATUS,0);		
		((CLocateDlg*)dwParam)->m_pStatusCtrl->InvalidateRect(NULL,TRUE);
		((CLocateDlg*)dwParam)->CheckClipboard();
		return TRUE;
	}
	case SearchingStarted:
	{
		CStringW text;
		text.Format(IDS_SEARCHINGFROMFILE,pLocater->GetFileName());
		((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(text,STATUSBAR_SEARCHFROMFILE,0);
		break;
	}
	case SearchingEnded:
		((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(STRNULL,STATUSBAR_SEARCHFROMFILE,0);
		break;
	case ClassShouldDelete:
		InterlockedExchangePointer((PVOID*)&((CLocateDlg*)dwParam)->m_pLocater,NULL);
		delete pLocater;
		
		((CLocateDlg*)dwParam)->StartBackgroundOperations();
		((CLocateDlg*)dwParam)->m_pBackgroundUpdater->StopWaiting();
		
		// To update items, looks like this is only way
		((CLocateDlg*)dwParam)->SetTimer(ID_REDRAWITEMS,50);
		return TRUE;
	case ErrorOccured:
		switch (ueCode)
		{
		case ueUnknown:
			{
				WCHAR* pError=CLocateApp::FormatLastOsError();
				if (pError!=NULL)
				{
					CStringW str;
					str.Format(IDS_ERRORUNKNOWNOS,pError);
					while (str.LastChar()=='\n' || str.LastChar()=='\r')
					str.DelLastChar();
						
					if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
						((CLocateDlg*)dwParam)->MessageBox(str,ID2W(IDS_ERROR),MB_OK|MB_ICONERROR);
					((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(str,STATUSBAR_LOCATEERRORS,0);
					LocalFree(pError);
				}
				else
				{
					ID2W str(IDS_ERRORUNKNOWN);
					((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(str,STATUSBAR_LOCATEERRORS,0);
					if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
						((CLocateDlg*)dwParam)->MessageBox(str,ID2W(IDS_ERROR),MB_OK|MB_ICONERROR);
				}
			}
			return FALSE;
		case ueOpen:
		case ueCreate:
			{
				CStringW str;
				str.Format(IDS_ERRORCANNOTOPENDB,pLocater->GetCurrentDatabaseFile());
				((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(str,STATUSBAR_LOCATEERRORS,0);
				if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
					((CLocateDlg*)dwParam)->MessageBox(str,ID2W(IDS_ERROR),MB_ICONERROR|MB_OK);
			}
			return FALSE;
		case ueRead:
			{
				CStringW str;
				str.Format(IDS_ERRORCANNOTREADDB,pLocater->GetCurrentDatabaseFile());
				((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(str,STATUSBAR_LOCATEERRORS,0);
				if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
					((CLocateDlg*)dwParam)->MessageBox(str,ID2W(IDS_ERROR),MB_ICONERROR|MB_OK);
				
				return FALSE;
			}
		case ueAlloc:
			((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(ID2W(IDS_ERRORCANNOTALLOCATE),STATUSBAR_LOCATEERRORS,0);
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
				((CLocateDlg*)dwParam)->ShowErrorMessage(IDS_ERRORCANNOTALLOCATE,IDS_ERROR);
			return FALSE;
		case ueInvalidDatabase:
			{
				CStringW str;
				str.Format(IDS_ERRORINVALIDDB,pLocater->GetCurrentDatabaseFile());
				((CLocateDlg*)dwParam)->m_pStatusCtrl->SetText(str,STATUSBAR_LOCATEERRORS,0);
						
				if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
					((CLocateDlg*)dwParam)->MessageBox(str,ID2W(IDS_ERROR),MB_ICONERROR|MB_OK);
				
				return FALSE;
			}
		}
		break;
	case RootInformationAvail:
		((CLocateDlg*)dwParam)->m_aVolumeInformation.Add(new VolumeInformation(
			pLocater->GetCurrentDatabaseID(),pLocater->GetCurrentDatabaseRootID(),
			pLocater->GetCurrentDatabaseRootType(),pLocater->GetCurrentDatabaseVolumeSerial(),
			pLocater->GetCurrentDatabaseVolumeLabel(),pLocater->GetCurrentDatabaseFileSystem()));
		break;
	}
	
	DbcDebugMessage("CLocateDlg::LocateProc END");
	return TRUE;
}

BOOL CALLBACK CLocateDlg::LocateFoundProc(DWORD_PTR dwParam,BOOL bFolder,const CLocater* pLocater)
{
	DbcDebugMessage("CLocateDlg::LocateFoundProc BEGIN");
	
	// Hide system and hidden files if it's wanted
	if (((CLocateDlg*)dwParam)->GetFlags()&fgLVDontShowHiddenFiles)
	{
		if (bFolder)
		{
			if (pLocater->GetFolderAttributes()&(LITEMATTRIB_HIDDEN|LITEMATTRIB_SYSTEM))
			{
				pLocater->IgnoreThisResult(TRUE);
				return TRUE;
			}
		}
		else if (pLocater->GetFileAttributes()&(LITEMATTRIB_HIDDEN|LITEMATTRIB_SYSTEM))
		{
			pLocater->IgnoreThisResult(FALSE);
			return TRUE;
		}
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
		MakeLower(szPath);

		
		if (((CLocateDlg*)dwParam)->m_pListCtrl->GetItemCount()>0)
		{
			int nItem=((CLocateDlg*)dwParam)->m_pListCtrl->GetNextItem(-1,LVNI_ALL);
			while (nItem!=-1)
			{
				CLocatedItem* pItem=(CLocatedItem*)((CLocateDlg*)dwParam)->m_pListCtrl->GetItemData(nItem);
				if (pItem!=NULL)
				{
					ASSERT(pItem->GetPathLen()<MAX_PATH);

					MemCopyWtoA(szPath2,pItem->GetPath(),pItem->GetPathLen()+1);
					MakeLower(szPath2);
				
					
					if (strcmp(szPath,szPath2)==0)
					{
						pLocater->IgnoreThisResult(bFolder);
						return TRUE; // Alreafy found
					}
					
				}
				nItem=((CLocateDlg*)dwParam)->m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
			}
		}		

	}

	LV_ITEM li;
	li.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	li.iSubItem=0;
	li.iImage=I_IMAGECALLBACK;
	li.lParam=(LPARAM)new CLocatedItem;
	if (li.lParam==NULL)
		return FALSE;
	if (bFolder)
		((CLocatedItem*)li.lParam)->SetFolder(pLocater);
	else
		((CLocatedItem*)li.lParam)->SetFile(pLocater);

	li.pszText=LPSTR_TEXTCALLBACK;

	// Check wheter file/folder is deleted
	if (((CLocateDlg*)dwParam)->GetExtraFlags()&efLVDontShowDeletedFiles)
	{
		if (bFolder)
		{
			if (!FileSystem::IsDirectory(((CLocatedItem*)li.lParam)->GetPath()))
			{
				delete (CLocatedItem*)li.lParam;
				pLocater->IgnoreThisResult(TRUE);
				return TRUE;
			}
		}
		else
		{
			if (!FileSystem::IsFile(((CLocatedItem*)li.lParam)->GetPath()))
			{
				delete (CLocatedItem*)li.lParam;
				pLocater->IgnoreThisResult(FALSE);
				return TRUE;
			}
		}
	}


	// To prevent drawing error
	DWORD dwResults=pLocater->GetNumberOfResults();
	
	if (dwResults%60==59)
		Sleep(((CLocateDlg*)dwParam)->m_WaitEvery60);
	else if (dwResults%30==29)
		Sleep(((CLocateDlg*)dwParam)->m_WaitEvery30);

	
	if (((CLocateDlg*)dwParam)->m_nSorting==BYTE(-1))
	{
		//li.iItem=((CLocateDlg*)dwParam)->m_pListCtrl->GetItemCount();
		if (((CLocateDlg*)dwParam)->GetFlags()&fgLVFoldersFirst && bFolder)
			li.iItem=pLocater->GetNumberOfFoundDirectories();
		else
			li.iItem=dwResults;
	}
	else
	{
		li.iItem=SortNewItem(((CLocateDlg*)dwParam)->m_pListCtrl,
			(CLocatedItem*)li.lParam,((CLocateDlg*)dwParam)->m_nSorting);
	}

	
	((CLocateDlg*)dwParam)->m_pListCtrl->InsertItem(&li);

	
	DbcDebugMessage("CLocateDlg::LocateFoundProc END");
	return TRUE;
}

BOOL CALLBACK CLocateDlg::LocateFoundProcW(DWORD_PTR dwParam,BOOL bFolder,const CLocater* pLocater)
{
	DbcDebugMessage("CLocateDlg::LocateFoundProcW BEGIN");
	
	// Hide system and hidden files if it's wanted
	if (((CLocateDlg*)dwParam)->GetFlags()&fgLVDontShowHiddenFiles)
	{
		if (bFolder)
		{
			if (pLocater->GetFolderAttributes()&(LITEMATTRIB_HIDDEN|LITEMATTRIB_SYSTEM))
			{
				pLocater->IgnoreThisResult(TRUE);
				return TRUE;
			}
		}
		else if (pLocater->GetFileAttributes()&(LITEMATTRIB_HIDDEN|LITEMATTRIB_SYSTEM))
		{
			pLocater->IgnoreThisResult(FALSE);
			return TRUE;
		}
	}

	// Check wheter item is already added
	if (((CLocateDlg*)dwParam)->GetFlags()&fgLVNoDoubleItems)
	{
		// Setting path, name and extension
		WCHAR szPath[MAX_PATH+2],szPath2[MAX_PATH+2];

		if (bFolder)
		{
			DWORD nPathLen=pLocater->GetCurrentPathLen();

			ASSERT(pLocater->GetCurrentPathLen()+pLocater->GetFolderNameLen()<MAX_PATH-1);

			MemCopyW(szPath,pLocater->GetCurrentPathW(),nPathLen);
			szPath[nPathLen++]=L'\\';
			MemCopyW(szPath+nPathLen,pLocater->GetFolderNameW(),pLocater->GetFolderNameLen()+1);
		}
		else
		{
			DWORD nPathLen=pLocater->GetCurrentPathLen();

			ASSERT(pLocater->GetCurrentPathLen()+pLocater->GetFileNameLen()<MAX_PATH-1);

			MemCopyW(szPath,pLocater->GetCurrentPathW(),nPathLen);
			szPath[nPathLen++]='\\';
			MemCopyW(szPath+nPathLen,pLocater->GetFileNameW(),pLocater->GetFileNameLen()+1);
		}
		MakeLower(szPath);
		
		if (((CLocateDlg*)dwParam)->m_pListCtrl->GetItemCount())
		{
			int nItem=((CLocateDlg*)dwParam)->m_pListCtrl->GetNextItem(-1,LVNI_ALL);
			while (nItem!=-1)
			{
				CLocatedItem* pItem=(CLocatedItem*)((CLocateDlg*)dwParam)->m_pListCtrl->GetItemData(nItem);
				if (pItem!=NULL)
				{
					ASSERT(pItem->GetPathLen()<MAX_PATH);

					MemCopyW(szPath2,pItem->GetPath(),pItem->GetPathLen()+1);
					MakeLower(szPath2);
				
					
					if (wcscmp(szPath,szPath2)==0)
					{
						pLocater->IgnoreThisResult(bFolder);
						return TRUE; // Alreafy found
					}
					
				}
				nItem=((CLocateDlg*)dwParam)->m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
			}
		}		

	}

	LV_ITEMW li;
	li.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	li.iSubItem=0;
	li.iImage=I_IMAGECALLBACK;
	li.lParam=(LPARAM)new CLocatedItem;
	if (li.lParam==NULL)
		return FALSE;
	if (bFolder)
		((CLocatedItem*)li.lParam)->SetFolderW(pLocater);
	else
		((CLocatedItem*)li.lParam)->SetFileW(pLocater);

	li.pszText=LPSTR_TEXTCALLBACKW;

		// Check wheter file/folder is deleted
	if (((CLocateDlg*)dwParam)->GetExtraFlags()&efLVDontShowDeletedFiles)
	{
		if (bFolder)
		{
			if (!FileSystem::IsDirectory(((CLocatedItem*)li.lParam)->GetPath()))
			{
				delete (CLocatedItem*)li.lParam;
				pLocater->IgnoreThisResult(TRUE);
				return TRUE;
			}
		}
		else
		{
			if (!FileSystem::IsFile(((CLocatedItem*)li.lParam)->GetPath()))
			{
				delete (CLocatedItem*)li.lParam;
				pLocater->IgnoreThisResult(FALSE);
				return TRUE;
			}
		}
	}

	// To prevent drawing error
	DWORD dwResults=pLocater->GetNumberOfResults();
	
	if (dwResults%60==59)
		Sleep(((CLocateDlg*)dwParam)->m_WaitEvery60);
	else if (dwResults%30==29)
		Sleep(((CLocateDlg*)dwParam)->m_WaitEvery30);

	
	if (((CLocateDlg*)dwParam)->m_nSorting==BYTE(-1))
	{
		//li.iItem=((CLocateDlg*)dwParam)->m_pListCtrl->GetItemCount();
		if (((CLocateDlg*)dwParam)->GetFlags()&fgLVFoldersFirst && bFolder)
			li.iItem=pLocater->GetNumberOfFoundDirectories();
		else
			li.iItem=dwResults;
	}
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
		{
			::MessageBox(NULL,"CLocateDlg::SortNewItem:Something is wrong! Contact jmhuttun@venda.uku.fi",NULL,MB_OK);
			return 0;
		}

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
	
void CLocateDlg::OnSettingsTool()
{
	// Save settings
	SaveResultlistActions();
	SaveDialogTexts();
	SaveRegistry();
	if ((m_pListCtrl->GetStyle() & LVS_TYPEMASK)==LVS_REPORT)
			m_pListCtrl->SaveColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\General","ListWidths");
	GetLocateAppWnd()->SaveSchedules();
	GetLocateApp()->SaveRegistry();


	CStringW sExeName(GetApp()->GetExeNameW());
	ShellExecuteW(*this,NULL,sExeName.Left(sExeName.FindLast('\\')+1)+L"settool.exe",
		NULL,NULL,SW_SHOW);

}

void CLocateDlg::OnStop()
{
	CWaitCursor wait;
	StartBackgroundOperations();
	if (m_pBackgroundUpdater!=NULL)
		m_pBackgroundUpdater->StopWaiting();

	if (IsLocating())
		m_pLocater->StopLocating();
}

void CLocateDlg::OnNewSearch()
{
	CWaitCursor wait;
	// Backgorund operation should end
	StopBackgroundOperations();
		
	// StopBackgroundOperations uses only CouldStop
	if (m_pFileNotificationsThread!=NULL)
		m_pFileNotificationsThread->Stop();
	if (m_pBackgroundUpdater!=NULL)
		m_pBackgroundUpdater->Stop();
	
	if (IsLocating())
		m_pLocater->StopLocating();

	RemoveResultsFromList();
	if (m_pListTooltips!=NULL)
		DeleteTooltipTools();
	SetDialogMode(FALSE);

	// Sorting
	SetSorting();
	
    
	// Clear dialogs
	m_NameDlg.OnClear(FALSE);
	m_SizeDateDlg.OnClear(FALSE);
	m_AdvancedDlg.OnClear(FALSE);
	if (m_pTabCtrl->GetCurSel()!=0)
	{
		m_AdvancedDlg.ShowWindow(swHide);
		m_SizeDateDlg.ShowWindow(swHide);
		m_NameDlg.ShowWindow(swShow);
		m_pTabCtrl->SetCurSel(0);
	}
	
	
	// Make title
	CStringW title;
	title.LoadString(IDS_TITLE);
	title.AddString(IDS_ALLFILES);
	
	if (GetLocateApp()->m_nInstance>0)
		title << L" (" << DWORD(GetLocateApp()->m_nInstance+1) << L')';
	SetText(title);

	m_NameDlg.m_Name.SetFocus();
}

BOOL CLocateDlg::OnClose()
{
	if (GetFlags()&fgDialogCloseMinimizesDialog)
	{
		ShowWindow(swMinimize);
		return 1;
	}

	CDialog::OnClose();
	DestroyWindow();
	return 1;
}

void CLocateDlg::OnDestroy()
{
	DebugMessage("CLocateDlg::OnDestroy() BEGIN");
	
	CDialog::OnDestroy();
	
	// Ensure that locating process is terminated
	if (IsLocating())
	{
		m_pLocater->StopLocating();
		DebugFormatMessage("CLocateDlg::OnDestroy(): m_pLocater=%X",(DWORD)m_pLocater);
	}

	// Ensuring that update animations are not on
	StopUpdateAnimation();
	
	// Stop background operations
	if (m_pFileNotificationsThread!=NULL)
		m_pFileNotificationsThread->Stop();
	if (m_pBackgroundUpdater!=NULL)
		m_pBackgroundUpdater->Stop();

	ASSERT(m_pFileNotificationsThread==NULL);
	ASSERT(m_pBackgroundUpdater==NULL);

	ChangeClipboardChain(*this,m_hNextClipboardViewer);
	

	// Clearing accelerators
	ClearShortcuts();
	SaveResultlistActions();
	ClearResultlistActions();


	SaveDialogTexts();
	m_NameDlg.DestroyWindow();
	m_SizeDateDlg.DestroyWindow();
	m_AdvancedDlg.DestroyWindow();
	
	
	SaveRegistry();
	ISDLGTHREADOK

	// Freeing target paths in dwItemData
	ClearMenuVariables();
	HMENU hOldMenu=GetMenu();
	FreeSendToMenuItems(GetSubMenu(hOldMenu,0));
	::DestroyMenu(hOldMenu);
	m_Menu.DestroyMenu(); // Destroy submenu

	// Releasing drop target
	RevokeDragDrop(*m_pListCtrl);
	m_pDropTarget->Release(); //This deletes class
	OleUninitialize();


	// Delete window classes
	// Tab control and statusbar
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

	// List control and related stuff
	if (m_pListCtrl!=NULL)
	{
		if ((m_pListCtrl->GetStyle() & LVS_TYPEMASK)==LVS_REPORT)
			m_pListCtrl->SaveColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\General","ListWidths");
		delete m_pListCtrl;
		m_pListCtrl=NULL;
	}
	if (m_pListTooltips!=NULL)
	{
		m_pListTooltips->DestroyToolTipCtrl();
		delete m_pListTooltips;
		m_pListTooltips=NULL;
	}
	if (m_pImageHandler!=NULL)
	{
		delete m_pImageHandler;
		m_pImageHandler=NULL;
	}



	// Buffers
	if (g_szBuffer!=NULL)
	{
		delete[] g_szBuffer;
		g_szBuffer=NULL;
	}
	if (g_szwBuffer!=NULL)
	{
		delete[] g_szwBuffer;
		g_szwBuffer=NULL;
	}

	// SendTo list related stuff
	if (m_hSendToListFont!=NULL)
	{
		DeleteObject(m_hSendToListFont);
		m_hSendToListFont=NULL;
	}

	

	if (!(m_dwFlags&fgDialogLeaveLocateBackground))
		GetLocateAppWnd()->PostMessage(WM_COMMAND,MAKEWPARAM(IDC_COMEWITHME,0));
	PostQuitMessage(0);

	DebugMessage("CLocateDlg::OnDestroy() END");



}

void CLocateDlg::ResetFileNotificators()
{
	//DebugMessage("CLocateDlg::ResetFileNotificators() BEGIN");
		
	if (m_pFileNotificationsThread!=NULL)
		m_pFileNotificationsThread->Stop();
	if (GetFlags()&fgLargeMode)
	{
		m_pFileNotificationsThread=new CCheckFileNotificationsThread;
		m_pFileNotificationsThread->Start();
	}
	else
		m_pFileNotificationsThread=NULL;

	//DebugMessage("CLocateDlg::ResetFileNotificators() END");

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
	case ID_UPDATESELECTED:
		{
			KillTimer(ID_UPDATESELECTED);
				
			int nSelecetedCount=m_pListCtrl->GetSelectedCount();
			int nFolders=0;
			if (nSelecetedCount>0)
			{
				ULONGLONG nTotalSize=0;
				int nIntex=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
				while (nIntex!=-1)
				{
					CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nIntex);
					if (pItem!=NULL)
					{
						if (pItem->IsFolder())
							nFolders++;
						else if (pItem->GetFileSizeLo()!=DWORD(-1))
							nTotalSize+=pItem->GetFileSize();
					}
					nIntex=m_pListCtrl->GetNextItem(nIntex,LVNI_SELECTED);
				}

				

				CStringW str;
				
				if (nFolders==0)
					str.Format(IDS_SELECTEDCOUNT,nSelecetedCount);
				else if (nFolders==nSelecetedCount)
					str.Format(IDS_SELECTEDCOUNT2,nSelecetedCount);
				else
					str.Format(IDS_SELECTEDCOUNT3,nSelecetedCount-nFolders,nFolders);

				
				if (nTotalSize>0)
				{
					CStringW size;
					LPWSTR pFileSize=GetLocateApp()->FormatFileSizeString((DWORD)nTotalSize,(DWORD)(nTotalSize>>32));
					size.Format(IDS_TOTALSIZE,pFileSize);
					str << L", " << size;
					delete[] pFileSize;
				}

				
				m_pStatusCtrl->SetText(str,STATUSBAR_MISC,0);
				
			}
			else
				m_pStatusCtrl->SetText("",STATUSBAR_MISC,0);
			break;
		}
	case ID_CLICKWAIT:
		KillTimer(ID_CLICKWAIT);
		m_ClickWait=FALSE;
		break;
	case ID_LOCATEANIM:
		m_nCurLocateAnimBitmap++;
		if (m_nCurLocateAnimBitmap>5)
			m_nCurLocateAnimBitmap=0;
		EnterCriticalSection(&m_csAnimBitmaps);
		if (m_pStatusCtrl!=NULL && m_pLocateAnimBitmaps!=NULL)
			m_pStatusCtrl->SetText((LPCSTR)m_pLocateAnimBitmaps[m_nCurLocateAnimBitmap],STATUSBAR_LOCATEICON,SBT_OWNERDRAW);
		LeaveCriticalSection(&m_csAnimBitmaps);
		break;
	case ID_UPDATEANIM:
		m_nCurUpdateAnimBitmap++;
		if (m_nCurUpdateAnimBitmap>12)
			m_nCurUpdateAnimBitmap=0;
		EnterCriticalSection(&m_csAnimBitmaps);
		if (m_pStatusCtrl!=NULL && m_pUpdateAnimBitmaps!=NULL)
			m_pStatusCtrl->SetText((LPCSTR)m_pUpdateAnimBitmaps[m_nCurUpdateAnimBitmap],STATUSBAR_UPDATEICON,SBT_OWNERDRAW);
		LeaveCriticalSection(&m_csAnimBitmaps);
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
	::SetWindowPos(GetDlgItem(IDC_PRESETS),HWND_BOTTOM,cx+m_nPresetsButtonOffset-m_nPresetsButtonWidth-8,35+m_nTabbedDialogHeight-int(m_nPresetsButtonHeight),0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
	
	
	
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
			parts[3]=cx; // Update icon
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
				parts[3]=cx-25;
			}
		}
		m_pStatusCtrl->SetWindowPos(HWND_BOTTOM,0,cy-20,cx,20,SWP_NOACTIVATE|SWP_NOZORDER);
		RECT rect,rect2;
		m_pStatusCtrl->GetWindowRect(&rect);
		m_pListCtrl->GetWindowRect(&rect2);
		m_pListCtrl->SetWindowPos(HWND_BOTTOM,0,0,cx-5,rect.top-rect2.top,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		m_pListCtrl->UpdateWindow();
		m_pStatusCtrl->SetParts(5,parts);
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

		// Free buffers
		if (g_szBuffer!=NULL)
		{
			delete[] g_szBuffer;
			g_szBuffer=NULL;
		}
		if (g_szwBuffer!=NULL)
		{
			delete[] g_szwBuffer;
			g_szwBuffer=NULL;
		}

		if (m_hSendToListFont!=NULL)
		{
			DeleteObject(m_hSendToListFont);
			m_hSendToListFont=NULL;
		}
		ClearMenuVariables();

		// Minimizing to system tray
		if (m_dwFlags&fgDialogMinimizeToST)
			ShowWindow(swHide);

		{
			CStringW Named;
			m_NameDlg.GetDlgItemText(IDC_NAME,Named);
			if (Named.GetLength()>0)
				GetLocateAppWnd()->SetUpdateStatusInformation(NULL,IDS_NOTIFYLOCATEWITHNAMED,Named);
			else
				GetLocateAppWnd()->SetUpdateStatusInformation(NULL,IDS_NOTIFYLOCATE,NULL);

		}
		break;
	case SIZE_RESTORED:
		GetLocateAppWnd()->SetUpdateStatusInformation(NULL,IDS_NOTIFYLOCATE,NULL);
		if (m_dwFlags&fgLargeMode)
		{
			RECT rect;
			GetWindowRect(&rect);
			m_nLargeY=rect.bottom-rect.top;

			
			if (m_pListCtrl->GetItemCount()>0)
			{
				ChangeBackgroundOperationsPriority(FALSE);
				//StartBackgroundOperations();
				if (m_pBackgroundUpdater!=NULL)
					m_pBackgroundUpdater->StopWaiting();
			}
		}

		SetControlPositions(nType,cx,cy);
		break;
	case SIZE_MAXIMIZED:
		if (m_dwFlags&fgLargeMode)
		{
			if (m_pListCtrl->GetItemCount()>0)
			{
				ChangeBackgroundOperationsPriority(FALSE);
				//StartBackgroundOperations();
				if (m_pBackgroundUpdater!=NULL)
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
		::SetTimer(hwnd,1,500,NULL);
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
		//int nCPU=int(100*GetCpuTime());
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

		if (GetLocateAppWnd()->m_pCpuUsage!=NULL)
		{
			str.SetBase(10);
			str << "\r\nCpu time: " << GetLocateAppWnd()->m_pCpuUsage->GetCpuUsage();
		}

		::SetDlgItemText(hwnd,100,LPCSTR(str));
		break;
	}
	};
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}


#endif

LRESULT CLocateDlg::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
	case WM_UPDATINGSTOPPED:
		StopUpdateAnimation();
	
		m_NameDlg.InitDriveBox();
		ResetFileNotificators();
		break;
	case WM_SETOPERATIONSTATUSBAR:
		if (m_pStatusCtrl!=NULL)
		{
			if (lParam!=NULL)
				m_pStatusCtrl->SetText((LPCWSTR)lParam,STATUSBAR_OPERATIONSTATUS,0);
			if (wParam!=0)
				m_pStatusCtrl->SetText(LPCSTR(::LoadIcon(NULL,(LPCSTR)wParam)),STATUSBAR_UPDATEICON,SBT_OWNERDRAW);
		}
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
		if (wParam==0x2a && lParam==NULL && m_pListCtrl!=NULL) // Possibly shell icon cache is updeted
		{
			if (m_pListCtrl->GetItemCount()>0)
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
		}
		SetListSelStyle();
		if (m_pBackgroundUpdater!=NULL)
			m_pBackgroundUpdater->StopWaiting();
		break;
	case WM_DRAWCLIPBOARD:
		return CDialog::WindowProc(msg,wParam,lParam);
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (wParam)
			break;
	case WM_INITMENUPOPUP:
		if (m_pActiveContextMenu!=NULL)
		{
			if (m_pActiveContextMenu->pContextMenu3!=NULL)
			{
				HRESULT hRes;
				LRESULT hRet;
				hRes=m_pActiveContextMenu->pContextMenu3->HandleMenuMsg2(msg,wParam,lParam,&hRet);
				if (hRes==NOERROR)
					return hRet;
			}
			if (m_pActiveContextMenu->pContextMenu2!=NULL)
				m_pActiveContextMenu->pContextMenu2->HandleMenuMsg(msg,wParam,lParam);

			return (msg==WM_INITMENUPOPUP ? 0 : TRUE); // handled
		}
		break;
	/*case WM_MENUCHAR:
		if (m_pActiveContextMenu!=NULL)
		{
			if (m_pActiveContextMenu->pContextMenu3!=NULL)
			{
				HRESULT hRet,hRes;
				hRes=m_pActiveContextMenu->pContextMenu3->HandleMenuMsg2(msg,wParam,lParam,&hRet);
				if (hRes==NOERROR)
					return hRet;
			}
			if (m_pActiveContextMenu->pContextMenu2!=NULL)
				m_pActiveContextMenu->pContextMenu2->HandleMenuMsg(msg,wParam,lParam);
		}
		break;*/


	case WM_GETICON:
	case WM_SETICON:
		DefWindowProc(*this,msg,wParam,lParam);
		break;
	case WM_RESETSHORTCUTS:
		SetShortcuts();
		break;
	case WM_RESULTLISTACTION:
		OnExecuteResultAction((CAction::ActionResultList)wParam,(void*)lParam);
        break;
	case WM_GETSELECTEDITEMPATH:
		if (lParam!=NULL && wParam!=NULL)
		{
			INT nAllocSize=(*((INT*)wParam))&~(1<<31);
			if (nAllocSize<=0)
				return 0;

			// Copy the path of currently selected item to lparam

			int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
			if (nItem==-1)
				return 0;

			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
			if (pItem==NULL)
				return 0;

			if ((UINT)nAllocSize>=pItem->GetPathLen()+1)
			{
				CopyMemory((LPSTR)lParam,pItem->GetPath(),pItem->GetPathLen()+1);
				*((UINT*)wParam)=pItem->GetPathLen();
			}
			else
			{
				nAllocSize--;
				CopyMemory((LPSTR)lParam,pItem->GetPath(),nAllocSize);
				((LPSTR)lParam)[nAllocSize]='\0';
				*((UINT*)wParam)=nAllocSize;
			}


		}
		return 0;
	case WM_ENABLEITEMS:
		EnableItems((BOOL)wParam);
		return 0;
	case WM_SETITEMFOCUS:
		::SetFocus((HWND)wParam);
		return 0;
	case WM_CLOSEDIALOG:
		DestroyWindow();
		break;
	case WM_SETFONT:
		m_hDialogFont=HFONT(wParam);
		break;
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
				CStringW text;
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
				free(pData);
			}
		}
		else if (wParam==0x77)
		{
			WNDCLASSEX wce;
			ZeroMemory(&wce,sizeof(WNDCLASSEX));
			wce.cbSize=sizeof(wce);
			wce.style=CS_HREDRAW|CS_VREDRAW;
			wce.hInstance=GetInstanceHandle();
			wce.hbrBackground=(HBRUSH)COLOR_WINDOW;
			wce.lpszClassName="DEBUGWINDOWCLASS";
			wce.lpfnWndProc=DebugWindowProc;
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

CLocatedItem** CLocateDlg::GetSeletedItems(int& nItems,int nIncludeIfNoneSeleted)
{
	nItems=m_pListCtrl->GetSelectedCount();

	if (nItems==0)
	{
		CLocatedItem** pRet=new CLocatedItem*[nItems=1];
		if (nIncludeIfNoneSeleted!=-1)
            pRet[0]=(CLocatedItem*)m_pListCtrl->GetItemData(nIncludeIfNoneSeleted);
		else
		{
			nItems=0;
			pRet[0]=NULL;
		}
		return pRet;
	}

	CLocatedItem** pRet=new CLocatedItem*[nItems];

	int iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	for (int i=0;i<nItems;i++)
	{
		pRet[i]=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
		iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
	}   

	return pRet;
}


void CLocateDlg::ExecuteCommand(LPCWSTR szCommand,int nItem)
{
	if (szCommand==NULL)
		return;
	
	int nIndexToPercent=nIndexToPercent=(int)FirstCharIndex(szCommand,L'%');
	if (nIndexToPercent==-1 || (szCommand[nIndexToPercent+1]!=L'd' && szCommand[nIndexToPercent+1]!=L'p'))
	{
		// Just execute command
		PROCESS_INFORMATION pi;
		STARTUPINFO si; // Ansi and Unicode versions are same
		si.cb=sizeof(STARTUPINFO);
		si.lpReserved=NULL;
		si.cbReserved2=0;
		si.lpReserved2=NULL;
		si.lpDesktop=NULL;
		si.lpTitle=NULL;
		si.dwFlags=STARTF_USESHOWWINDOW;
		si.wShowWindow=SW_SHOWDEFAULT;
		
		if (IsUnicodeSystem())
		{
			if (!CreateProcessW(NULL,LPWSTR(szCommand),NULL,
				NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS,
				NULL,NULL,(STARTUPINFOW*)&si,&pi))
				return;
		}
		else
		{
			if (!CreateProcess(NULL,(LPSTR)(LPCSTR)W2A(szCommand),NULL,
				NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS,
				NULL,NULL,&si,&pi))
				return;
		}

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return;		
	}

	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg==NULL)
		return;

	int nItems;
	CLocatedItem** pItems=pLocateDlg->GetSeletedItems(nItems,nItem);
	
	for (int i=0;i<nItems;i++)
	{
		if (pItems[i]!=NULL)
		{
			int nIndex=nIndexToPercent;
			LPWSTR pCommand=(LPWSTR)szCommand;
		
			// Replace %d with item path
			do
			{
				int nLen;
				LPCWSTR pPath;
                if (pCommand[nIndex+1]==L'd')
				{
					nLen=(int)pItems[i]->GetPathLen();
					pPath=pItems[i]->GetPath();
				}
				else 
				{
					pPath=pItems[i]->GetParent();
					nLen=(int)istrlenw(pPath);					
				}



				UINT nCommandLen=(UINT)istrlenw(pCommand);
				LPWSTR pNewCommand=new WCHAR[nCommandLen-2+nLen+1];
				MemCopyW(pNewCommand,pCommand,nIndex);
				MemCopyW(pNewCommand+nIndex,pPath,nLen);
				MemCopyW(pNewCommand+nIndex+nLen,pCommand+nIndex+2,nCommandLen-nIndex-2+1);


				if (pCommand!=szCommand)
					delete[] pCommand;

				pCommand=pNewCommand;
			}
			while ((nIndex=(int)FirstCharIndex(pCommand,L'%'))!=-1 && (pCommand[nIndex+1]==L'd' || pCommand[nIndex+1]==L'p'));
	
			// Execute command
			PROCESS_INFORMATION pi;
			STARTUPINFO si;// Ansi and Unicode versions are same
			si.cb=sizeof(STARTUPINFO);
			si.lpReserved=NULL;
			si.cbReserved2=0;
			si.lpReserved2=NULL;
			si.lpDesktop=NULL;
			si.lpTitle=NULL;
			si.dwFlags=STARTF_USESHOWWINDOW;
			si.wShowWindow=SW_SHOWDEFAULT;
			
			if (IsUnicodeSystem())
			{
				if (CreateProcessW(NULL,pCommand,NULL,
					NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS,
					NULL,NULL,(STARTUPINFOW*)&si,&pi))
				{
					CloseHandle(pi.hThread);
					CloseHandle(pi.hProcess);
				}
			}
			else
			{
				if (CreateProcess(NULL,(LPSTR)(LPCSTR)W2A(pCommand),NULL,
					NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS,
					NULL,NULL,&si,&pi))
				{
					CloseHandle(pi.hThread);
					CloseHandle(pi.hProcess);
				}
			}

		

			if (pCommand!=szCommand)
				delete[] pCommand;
		}
	}

	delete[] pItems;
}

void CLocateDlg::OnExecuteResultAction(CAction::ActionResultList m_nResultAction,void* pExtraInfo,int nItem,DetailType nDetail)
{
	switch (m_nResultAction)
	{
	case CAction::Execute:
		OnExecuteFile((LPCWSTR)pExtraInfo,nItem);
		break;
	case CAction::Copy:
		OnCopy(FALSE,nItem);
		break;
	case CAction::Cut:
		OnCopy(TRUE,nItem);
		break;
	case CAction::MoveToRecybleBin:
		OnDelete(Recycle,nItem);
		break;
	case CAction::DeleteFile:
		OnDelete(Delete,nItem);
		break;
	case CAction::OpenContextMenu:
	case CAction::OpenContextMenuSimple:
		{
			ClearMenuVariables();

			int nSelectedItems;
			CLocatedItem** pSelectedItems=GetSeletedItems(nSelectedItems,nItem);
			m_hActivePopupMenu=CreateFileContextMenu(NULL,pSelectedItems,nSelectedItems,
				m_nResultAction==CAction::OpenContextMenuSimple);
			delete[] pSelectedItems;
			if (m_hActivePopupMenu==NULL)
				break;

			if (pExtraInfo!=NULL)
			{
	            TrackPopupMenu(m_hActivePopupMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,
					((POINT*)pExtraInfo)->x,((POINT*)pExtraInfo)->y,0,*this,NULL);	
			}
			else
			{
				POINT pos;
				GetCursorPos(&pos);
				TrackPopupMenu(m_hActivePopupMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,
					pos.x,pos.y,0,*this,NULL);	
			}
			break;
		}
	case CAction::OpenFolder:
		OnOpenFolder(FALSE,nItem);
		break;
	case CAction::OpenContainingFolder:
		OnOpenFolder(TRUE,nItem);
		break;
	case CAction::Properties:
		OnProperties(nItem);
		break;
	case CAction::ShowSpecialMenu:
		if (m_pListCtrl->GetSelectedCount()==0)
			m_pListCtrl->SetItemState(nItem,LVIS_SELECTED,LVIS_SELECTED);

		if (pExtraInfo!=NULL)
		{
			TrackPopupMenu(::GetSubMenu(m_Menu.GetSubMenu(SUBMENU_EXTRACONTEXTMENUITEMS),SUBMENU_SPECIALMENU),TPM_LEFTALIGN|TPM_RIGHTBUTTON,
				((POINT*)pExtraInfo)->x,((POINT*)pExtraInfo)->y,0,*this,NULL);	
		}
		else
		{
			POINT pos;
			GetCursorPos(&pos);
			TrackPopupMenu(::GetSubMenu(m_Menu.GetSubMenu(SUBMENU_EXTRACONTEXTMENUITEMS),SUBMENU_SPECIALMENU),TPM_LEFTALIGN|TPM_RIGHTBUTTON,
				pos.x,pos.y,0,*this,NULL);	
		}
		break;
	case CAction::ExecuteCommand:
		ExecuteCommand((LPCWSTR)pExtraInfo,nItem);
		break;
	case CAction::SelectFile:
		{
			int nItem,nSelectedItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
			
			switch ((CAction::SelectFileType)(DWORD)pExtraInfo)
			{
			case CAction::NextFile:
				if ((m_pListCtrl->GetStyle()&LVS_TYPEMASK)!=LVS_REPORT)
				{
					nItem=m_pListCtrl->GetNextItem(nSelectedItem,LVNI_TORIGHT);
					if (nItem==nSelectedItem)
						nItem=m_pListCtrl->GetNextItem(nSelectedItem,LVNI_BELOW);
				}
				else
					nItem=m_pListCtrl->GetNextItem(nSelectedItem,LVNI_BELOW);
				break;
			case CAction::PrevFile:
				if ((m_pListCtrl->GetStyle()&LVS_TYPEMASK)!=LVS_REPORT)
				{
					nItem=m_pListCtrl->GetNextItem(nSelectedItem,LVNI_TOLEFT);
					if (nItem==nSelectedItem)
						nItem=m_pListCtrl->GetNextItem(nSelectedItem,LVNI_ABOVE);
				}
				else
					nItem=m_pListCtrl->GetNextItem(nSelectedItem,LVNI_ABOVE);
				break;
			case CAction::NextNonDeletedFile:
				nItem=nSelectedItem;
				for (;;)
				{
					// Next item
					if ((m_pListCtrl->GetStyle()&LVS_TYPEMASK)!=LVS_REPORT)
					{
						nItem=m_pListCtrl->GetNextItem(nItem,LVNI_TORIGHT);
						if (nItem==nSelectedItem)
							nItem=m_pListCtrl->GetNextItem(nItem,LVNI_BELOW);
					}
					else
						nItem=m_pListCtrl->GetNextItem(nItem,LVNI_BELOW);

					if (nItem==-1|| nSelectedItem==nItem)
						return;

					CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
					if (pItem!=NULL)
					{
						if (!pItem->IsDeleted())
							break;
                   	}                    								
				}
				break;
			case CAction::PrevNonDeletedFile:
				nItem=nSelectedItem;
				for (;;)
				{
					// Next item
					if ((m_pListCtrl->GetStyle()&LVS_TYPEMASK)!=LVS_REPORT)
					{
						nItem=m_pListCtrl->GetNextItem(nItem,LVNI_TOLEFT);
						if (nItem==nSelectedItem)
							nItem=m_pListCtrl->GetNextItem(nItem,LVNI_ABOVE);
					}
					else
						nItem=m_pListCtrl->GetNextItem(nItem,LVNI_ABOVE);

					if (nItem==-1|| nSelectedItem==nItem)
						return;

					CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
					if (pItem!=NULL)
					{
						if (!pItem->IsDeleted())
							break;
                   	}                    								
				}
				break;
			}

			if (nItem==-1 || nSelectedItem==nItem)
				return;
			
			if (nSelectedItem!=-1)
				m_pListCtrl->SetItemState(nSelectedItem,0,LVIS_SELECTED|LVIS_FOCUSED);
			m_pListCtrl->SetItemState(nItem,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
			m_pListCtrl->EnsureVisible(nItem,FALSE);
			
			break;


		} 
	case CAction::RenameFile:
		OnRenameFile(nItem);
		break;
	case CAction::SelectNthFile:
	case CAction::ExecuteNthFile:
		{
			// Unselect other items
			int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
			while (nItem!=-1)
			{
				if (nItem!=(int)pExtraInfo)					
					m_pListCtrl->SetItemState(nItem,0,LVIS_SELECTED|LVIS_FOCUSED);
				nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
			}

			// Selcet item
			m_pListCtrl->SetItemState((int)pExtraInfo,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
			m_pListCtrl->EnsureVisible((int)pExtraInfo,FALSE);
			
			
			// Execute item
			if (m_nResultAction==CAction::ExecuteNthFile)
				OnExecuteFile(NULL,(int)pExtraInfo);
			break;
		}
	}
}

void CLocateDlg::OnMeasureItem(int nIDCtl,LPMEASUREITEMSTRUCT lpmis)
{
	CDialog::OnMeasureItem(nIDCtl,lpmis);
	
	if (nIDCtl==0)
	{
		// This is IContextMenu item
		if (m_pActiveContextMenu!=NULL && lpmis->itemID<IDM_DEFSENDTOITEM)
			return;

		// SendTo menu item
		
		CDC dc(this);
		HGDIOBJ hOld=dc.SelectObject(m_hSendToListFont);
		
		SHFILEINFOW fi;
		fi.hIcon=NULL;
		GetFileInfo((LPITEMIDLIST)lpmis->itemData,0,&fi,SHGFI_DISPLAYNAME|SHGFI_ICON|SHGFI_SMALLICON);
			
		CSize sz=dc.GetTextExtent(fi.szDisplayName,(int)wcslen(fi.szDisplayName));
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
	CRect rect;
		
	if (DWORD(pos.x)==0xffff && DWORD(pos.y)==0xffff)
	{
		// Key
		GetCursorPos(&pos);

		int nSelectedItems;
		CLocatedItem** pSelectedItems=GetSeletedItems(nSelectedItems);
		if (nSelectedItems>0)
		{
			ClearMenuVariables();

			m_hActivePopupMenu=CreateFileContextMenu(NULL,pSelectedItems,nSelectedItems);
			if (m_hActivePopupMenu!=NULL)
			{
				TrackPopupMenu(m_hActivePopupMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,
					pos.x,pos.y,0,*this,NULL);	
			}
		}
		else
		{	
			TrackPopupMenu(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),TPM_LEFTALIGN|TPM_RIGHTBUTTON,
				pos.x,pos.y,0,*this,NULL);	
		}

		delete pSelectedItems;

		return;			
	}


	m_pListCtrl->GetWindowRect(&rect);
	//Checking whether mouse is in the list control's area
	if (!rect.IsPtInRect(pos))
	{
		CDialog::OnContextMenu(hWnd,pos);
		return;
	}

	

	HWND hHeader=m_pListCtrl->GetHeader();
	::GetWindowRect(hHeader,&rect);
	if (rect.IsPtInRect(pos))
	{
		// Show context menu for header
		
		CMenu ColMenu(m_pListCtrl->CreateColumnSelectionMenu(IDM_DEFCOLSELITEM));
		CStringW text(IDS_SELECTDETAILS);
		
		MENUITEMINFOW mii;
		mii.cbSize=sizeof(MENUITEMINFOW);
		// Inserting separator
		mii.fMask=MIIM_TYPE;
		mii.fType=MFT_SEPARATOR;
		ColMenu.InsertMenu(WORD(-1),FALSE,&mii);
		mii.fMask=MIIM_ID|MIIM_TYPE;
		mii.fType=MFT_STRING;
		mii.dwTypeData=text.GetBuffer();
		mii.wID=IDM_SELECTDETAILS;
		ColMenu.InsertMenu(WORD(-1),FALSE,&mii);
		
		TrackPopupMenu(ColMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,*this,NULL);	
		ColMenu.DestroyMenu();
	}
	else
	{
		LVHITTESTINFO ht;
		ht.pt=pos;
		m_pListCtrl->ScreenToClient(&ht.pt);
		
		if (m_pListCtrl->SubItemHitTest(&ht)==-1)
		{
			// Not any file item
			TrackPopupMenu(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y,0,*this,NULL);	
		}
		CDialog::OnContextMenu(hWnd,pos);
	}
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
	CRegKey2 RegKey;
	DWORD temp;

	if(RegKey.OpenKey(HKCU,"\\General",CRegKey::defWrite)==ERROR_SUCCESS)
	{
		CRect rect;
		//CMenu menu(GetSubMenu(GetMenu(),2));
		CMenu menu(GetMenu());
		RegKey.SetValue("Program Status",m_dwFlags&fgSave);
		RegKey.SetValue("Program StatusExtra",m_dwExtraFlags&efSave);
		
		SavePosition(RegKey,NULL,"Position");
		
		//GetWindowRect(&rect);
		WINDOWPLACEMENT wp;
		wp.length=sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(&wp);
		
		if (m_dwFlags&fgLargeMode)
			m_nLargeY=wp.rcNormalPosition.bottom-wp.rcNormalPosition.top;;

		RegKey.SetValue("LargeCY",(LPCTSTR)&m_nLargeY,4,REG_DWORD);
		
		

		//for (temp=0;menu.GetMenuState(temp,MF_BYPOSITION)!=MF_CHECKED && temp<3;temp++);
		switch (m_pListCtrl->GetStyle()&LVS_TYPEMASK)
		{
		case LVS_ICON:
			temp=0;
			break;
		case LVS_SMALLICON:
			temp=1;
			break;
		case LVS_LIST:
			temp=2;
			break;
		default:
			temp=3;
			break;
		}

		RegKey.SetValue("ListView",temp);
		RegKey.SetValue("AutoArrange",(m_pListCtrl->GetStyle()&LVS_AUTOARRANGE)?1L:0L);
		RegKey.SetValue("AlignToGrid",(m_pListCtrl->GetExtendedListViewStyle()&LVS_EX_SNAPTOGRID)?1L:0L);
	
	}

}

void CLocateDlg::LoadRegistry()
{
	CMenu menu(GetMenu());
	CRegKey2 RegKey;
	DWORD temp;
	DWORD x=100,y=100,cx=438;

	if (RegKey.OpenKey(HKCU,"\\Locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("MaximumFoundFiles",(LPTSTR)&m_dwMaxFoundFiles,sizeof(DWORD));
		RegKey.CloseKey();
	}

	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
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
		if ((m_dwExtraFlags&efItemUpdatingMask)==efDisableItemUpdating)
			m_dwExtraFlags|=efLVNoUpdateWhileSorting;

		

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
			m_pListCtrl->SetStyle(m_pListCtrl->GetStyle()|LVS_AUTOARRANGE);
		temp=0;
		RegKey.QueryValue("AlignToGrid",temp);
		if (temp)
			m_pListCtrl->SetExtendedListViewStyle(LVS_EX_SNAPTOGRID,LVS_EX_SNAPTOGRID);
		
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

	
	if (RegKey.OpenKey(HKCU,"\\Locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD dwTemp;
		if (RegKey.QueryValue("WaitEvery30",dwTemp))
			m_WaitEvery30=WORD(dwTemp);
		if (RegKey.QueryValue("WaitEvery60",dwTemp))
			m_WaitEvery60=WORD(dwTemp);
	}
}

void CLocateDlg::LoadDialogTexts()
{
	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\Dialogs",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		m_NameDlg.LoadControlStates(RegKey);
		m_NameDlg.EnableItems(TRUE);

		m_SizeDateDlg.LoadControlStates(RegKey,FALSE);
		m_SizeDateDlg.EnableItems(TRUE);

		m_AdvancedDlg.LoadControlStates(RegKey);
		m_AdvancedDlg.EnableItems(TRUE);
	}
	
}

void CLocateDlg::SaveDialogTexts()
{
	CRegKey2 RegKey;

	if(RegKey.OpenKey(HKCU,"\\Dialogs",CRegKey::defWrite)==ERROR_SUCCESS)
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
	//DebugFormatMessage("%X->CLocateDlg::OnNotify(%d,%X)",DWORD(this),idCtrl,DWORD(pnmh));

	switch (idCtrl)
	{
	case IDC_FILELIST:
		return ListNotifyHandler((NMLISTVIEW*)pnmh);
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
			case TTN_GETDISPINFOA:
				//DebugNumMessage("CLocateDlg::OnNotify; TTN_GETDISPINFO, ((NMTTDISPINFO*)pnmh)->lParam=%X",((NMTTDISPINFO*)pnmh)->lParam);
				
				if (m_iTooltipItem==-1)
					break;

				((NMTTDISPINFO*)pnmh)->hinst=NULL;
				if (DetailType(m_pListCtrl->GetColumnIDFromSubItem(m_iTooltipSubItem))==Name)
				{
						
					CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(m_iTooltipItem);
					if (pItem!=NULL)
					{
						if (g_szBuffer!=NULL)
							delete[] g_szBuffer;
						g_szBuffer=alloccopyWtoA((LPCWSTR)pItem->GetToolTipText());
						((NMTTDISPINFO*)pnmh)->lpszText=g_szBuffer;
					}
					else
						((NMTTDISPINFO*)pnmh)->lpszText=(LPSTR)szEmpty;
				}
				else
				{
					CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(m_iTooltipItem);
					if (pItem!=NULL)
					{
						if (g_szBuffer!=NULL)
							delete[] g_szBuffer;
						
						g_szBuffer=alloccopyWtoA((LPCWSTR)pItem->GetDetailText(
							DetailType(m_pListCtrl->GetColumnIDFromSubItem(m_iTooltipSubItem))));
						((NMTTDISPINFO*)pnmh)->lpszText=g_szBuffer;
					}
					else
						((NMTTDISPINFO*)pnmh)->lpszText=(LPSTR)szEmpty;

				}
				break;
			case TTN_GETDISPINFOW:
				//DebugNumMessage("CLocateDlg::OnNotify; TTN_GETDISPINFO, ((NMTTDISPINFO*)pnmh)->lParam=%X",((NMTTDISPINFO*)pnmh)->lParam);
				
				if (m_iTooltipItem==-1)
					break;

				((NMTTDISPINFOW*)pnmh)->hinst=NULL;
				if (DetailType(m_pListCtrl->GetColumnIDFromSubItem(m_iTooltipSubItem))==Name)
				{
						
					CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(m_iTooltipItem);
					
					if (pItem!=NULL)
						((NMTTDISPINFOW*)pnmh)->lpszText=pItem->GetToolTipText();
					else
						((NMTTDISPINFOW*)pnmh)->lpszText=(LPWSTR)szEmpty;
				}
				else
				{
					CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(m_iTooltipItem);
					if (pItem!=NULL)
					{
						((NMTTDISPINFOW*)pnmh)->lpszText=pItem->GetDetailText(
							DetailType(m_pListCtrl->GetColumnIDFromSubItem(m_iTooltipSubItem)));
					}
					else
						((NMTTDISPINFOW*)pnmh)->lpszText=(LPWSTR)szwEmpty;

				}
				break;
			case TTN_SHOW:
				DebugMessage("CLocateDlg::OnNotify; TTN_SHOW");

				m_bTooltipActive=TRUE;

				if (DetailType(m_pListCtrl->GetColumnIDFromSubItem(m_iTooltipSubItem))!=Name)
				{
					CRect rc;
					m_pListCtrl->GetSubItemRect(m_iTooltipItem,m_iTooltipSubItem,LVIR_LABEL,&rc);
					m_pListCtrl->ClientToScreen(&rc);
                                     
					if (DetailType(m_pListCtrl->GetColumnIDFromSubItem(m_iTooltipSubItem))==InFolder)
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
				DebugMessage("CLocateDlg::OnNotify; TTN_POP");
				m_bTooltipActive=FALSE;
				break;
			case NM_CUSTOMDRAW:
				return CDRF_DODEFAULT;
			}
		}
		break;
	}

	//DebugMessage("CLocateDlg::OnNotify exit");
	return CDialog::OnNotify(idCtrl,pnmh);
}

void CLocateDlg::SetSortArrowToHeader(DetailType nType,BOOL bRemove,BOOL bDownArrow)
{
	if (((CLocateApp*)GetApp())->m_wComCtrlVersion<0x0600)
		return;

	if (int(nType)>int(LastType))
		return;

	int nColumn=m_pListCtrl->GetVisibleColumn(m_pListCtrl->GetColumnFromID(nType));
	if (nColumn==-1)
		return;

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

	
BOOL CLocateDlg::ListNotifyHandler(NMLISTVIEW *pNm)
{
	switch(pNm->hdr.code)
	{
	case NMX_CLICK:
		{
			DetailType nDetail=DetailType(m_pListCtrl->GetColumnIDFromSubItem(((NMHDR_MOUSE*)pNm)->iSubItem));
			if (nDetail>LastType || nDetail<0)
				break;

			if (m_aResultListActions[nDetail][LeftMouseButtonClick]!=NULL)
			{
				CWaitCursor wait;
				OnExecuteResultAction(m_aResultListActions[nDetail][LeftMouseButtonClick]->m_nResultList,
					m_aResultListActions[nDetail][LeftMouseButtonClick]->m_pExtraInfo,((NMHDR_MOUSE*)pNm)->iItem,nDetail);
			}
			break;
		}
	case NMX_DBLCLICK:
		{
            if (m_ClickWait)
				break;

			DetailType nDetail=DetailType(m_pListCtrl->GetColumnIDFromSubItem(((NMHDR_MOUSE*)pNm)->iSubItem));
			if (nDetail>LastType || nDetail<0)
				break;

			if (m_aResultListActions[nDetail][LeftMouseButtonDblClick]!=NULL)
			{
				CWaitCursor wait;
	            m_ClickWait=TRUE;
				OnExecuteResultAction(m_aResultListActions[nDetail][LeftMouseButtonDblClick]->m_nResultList,
					m_aResultListActions[nDetail][LeftMouseButtonDblClick]->m_pExtraInfo,((NMHDR_MOUSE*)pNm)->iItem,nDetail);

				SetTimer(ID_CLICKWAIT,500,NULL);
			}
			break;
		}
	case NMX_RCLICK:
		{
			DetailType nDetail=DetailType(m_pListCtrl->GetColumnIDFromSubItem(((NMHDR_MOUSE*)pNm)->iSubItem));
			if (nDetail>LastType || nDetail<0)
				break;

			if (m_aResultListActions[nDetail][RightMouseButtonClick]!=NULL)
			{
				CWaitCursor wait;
				OnExecuteResultAction(m_aResultListActions[nDetail][RightMouseButtonClick]->m_nResultList,
					m_aResultListActions[nDetail][RightMouseButtonClick]->m_pExtraInfo,((NMHDR_MOUSE*)pNm)->iItem,nDetail);
			}
			break;
		}
	case NMX_RDBLCLICK:
		{
            if (m_ClickWait)
				break;

			DetailType nDetail=DetailType(m_pListCtrl->GetColumnIDFromSubItem(((NMHDR_MOUSE*)pNm)->iSubItem));
			if (nDetail>LastType || nDetail<0)
				break;

			if (m_aResultListActions[nDetail][RightMouseButtonDblClick]!=NULL)
			{
				CWaitCursor wait;
	            m_ClickWait=TRUE;
				OnExecuteResultAction(m_aResultListActions[nDetail][RightMouseButtonDblClick]->m_nResultList,
					m_aResultListActions[nDetail][RightMouseButtonDblClick]->m_pExtraInfo,((NMHDR_MOUSE*)pNm)->iItem,nDetail);

				SetTimer(ID_CLICKWAIT,500,NULL);
			}
			break;
		}
	case NMX_MCLICK:
		{
			DetailType nDetail=DetailType(m_pListCtrl->GetColumnIDFromSubItem(((NMHDR_MOUSE*)pNm)->iSubItem));
			if (nDetail>LastType || nDetail<0)
				break;

			if (m_aResultListActions[nDetail][MiddleMouseButtonClick]!=NULL)
			{
				CWaitCursor wait;
				OnExecuteResultAction(m_aResultListActions[nDetail][MiddleMouseButtonClick]->m_nResultList,
					m_aResultListActions[nDetail][MiddleMouseButtonClick]->m_pExtraInfo,((NMHDR_MOUSE*)pNm)->iItem,nDetail);
			}
			break;
		}
	case NMX_MDBLCLICK:
		{
            if (m_ClickWait)
				break;

			DetailType nDetail=DetailType(m_pListCtrl->GetColumnIDFromSubItem(((NMHDR_MOUSE*)pNm)->iSubItem));
			if (nDetail>LastType || nDetail<0)
				break;

			if (m_aResultListActions[nDetail][MiddleMouseButtonDblClick]!=NULL)
			{
				CWaitCursor wait;
	            m_ClickWait=TRUE;
				OnExecuteResultAction(m_aResultListActions[nDetail][MiddleMouseButtonDblClick]->m_nResultList,
					m_aResultListActions[nDetail][MiddleMouseButtonDblClick]->m_pExtraInfo,((NMHDR_MOUSE*)pNm)->iItem,nDetail);

				SetTimer(ID_CLICKWAIT,500,NULL);
			}
			break;
		}
	case LVN_COLUMNCLICK:
		SortItems(DetailType(m_pListCtrl->GetColumnIDFromSubItem(pNm->iSubItem)),-1,TRUE);
		break;
	case LVN_GETDISPINFOA:
		{
			LV_DISPINFOA *pLvdi=(LV_DISPINFOA*)pNm;

			CLocatedItem* pItem=(CLocatedItem*)pLvdi->item.lParam;
			if (pItem==NULL)
				break;

			
			DetailType nDetail=DetailType(m_pListCtrl->GetColumnIDFromSubItem(pLvdi->item.iSubItem));

			

			if (GetFlags()&fgLVNoDelayedUpdate) 
			{
				ItemDebugFormatMessage1("LVN_GETDISPINFOA %d BEGIN1",nDetail);
			
				// Update detail instantaneously
				if (g_szBuffer!=NULL)
					delete[] g_szBuffer;

				switch (nDetail)
				{
				// Title and parent are special since they have icons
				case Name:
					if (pItem->ShouldUpdateFileTitle())
						pItem->UpdateFileTitle();

					if (pItem->ShouldUpdateIcon())
                        pItem->UpdateIcon();

					pLvdi->item.mask=LVIF_TEXT|LVIF_IMAGE;
					if (pItem->GetFileTitle()!=NULL)
						pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(pItem->GetFileTitle());
					else
						pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(pItem->GetName());
					
					pLvdi->item.iImage=pItem->GetIcon();

					if (pItem->GetAttributes()&(LITEMATTRIB_CUTTED|LITEMATTRIB_HIDDEN))
					{
						pLvdi->item.mask|=LVIF_STATE;
						pLvdi->item.state=LVIS_CUT;
						pLvdi->item.stateMask=LVIS_CUT;
					}
					break;
				case InFolder:
					if (pItem->ShouldUpdateParentIcon())
						pItem->UpdateParentIcon();

					pLvdi->item.mask=LVIF_TEXT|LVIF_IMAGE;
					pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(pItem->GetParent());
					pLvdi->item.iImage=pItem->GetParentIcon();
					break;
				default:
					ASSERT (nDetail<=LastType);
			
					if (GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating &&
						pItem->ShouldUpdateByDetail(nDetail))
						pItem->UpdateByDetail(nDetail);
					pLvdi->item.mask=LVIF_TEXT;
					pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(pItem->GetDetailText(nDetail));
					break;
				}
				
				ItemDebugMessage("LVN_GETDISPINFOA END1");
			}
			else
			{
				ItemDebugFormatMessage1("LVN_GETDISPINFOA %d BEGIN2",nDetail);
			
				// Update detail instantaneously
				if (g_szBuffer!=NULL)
					delete[] g_szBuffer;

				// Delayed updating
				switch (nDetail)
				{
				// Title and parent are special since they have icons
				case Name:
					if (pItem->ShouldUpdateFileTitle() || pItem->ShouldUpdateIcon())
					{
						if (m_pBackgroundUpdater==NULL)
						{
							InterlockedExchangePointer((PVOID*)&m_pBackgroundUpdater,new CBackgroundUpdater(m_pListCtrl));
							m_pBackgroundUpdater->CreateEventsAndMutex();
						}
						
						m_pBackgroundUpdater->AddToUpdateList(pItem,pLvdi->item.iItem,Name);
						
						if (!IsLocating()) // Locating in process
							m_pBackgroundUpdater->StopWaiting();
					}
					
					pLvdi->item.mask=LVIF_TEXT|LVIF_IMAGE;
					if (pItem->GetFileTitle()!=NULL)
						pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(pItem->GetFileTitle());
					else
						pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(pItem->GetName());
					pLvdi->item.iImage=pItem->GetIcon();

					if (pItem->GetAttributes()&(LITEMATTRIB_CUTTED|LITEMATTRIB_HIDDEN))
					{
						pLvdi->item.mask|=LVIF_STATE;
						pLvdi->item.state=LVIS_CUT;
						pLvdi->item.stateMask=LVIS_CUT;
					}
					break;
				case InFolder:
					if (pItem->ShouldUpdateParentIcon())
					{
						if (m_pBackgroundUpdater==NULL)
						{
							InterlockedExchangePointer((PVOID*)&m_pBackgroundUpdater,new CBackgroundUpdater(m_pListCtrl));
							m_pBackgroundUpdater->CreateEventsAndMutex();
						}

						//DebugFormatMessage("Calling %X->AddToUpdateList(%X,%X,InFolder)",m_pBackgroundUpdater,pItem,pLvdi->item.iItem);
						m_pBackgroundUpdater->AddToUpdateList(pItem,pLvdi->item.iItem,InFolder);
						if (!IsLocating()) // Locating is not in process
							m_pBackgroundUpdater->StopWaiting();
					}

					pLvdi->item.mask=LVIF_TEXT|LVIF_IMAGE;
					pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(pItem->GetParent());
					pLvdi->item.iImage=pItem->GetParentIcon();
					break;
				default:
					ASSERT (nDetail<=LastType);
			
					if (GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating &&
						pItem->ShouldUpdateByDetail(nDetail))
					{
						if (m_pBackgroundUpdater==NULL)
						{
							InterlockedExchangePointer((PVOID*)&m_pBackgroundUpdater,new CBackgroundUpdater(m_pListCtrl));
							m_pBackgroundUpdater->CreateEventsAndMutex();
						}

						//DebugFormatMessage("Calling %X->AddToUpdateList(%X,%X,%d)",m_pBackgroundUpdater,pItem,pLvdi->item.iItem,DWORD(nDetail));
						
						m_pBackgroundUpdater->AddToUpdateList(pItem,pLvdi->item.iItem,nDetail);
						if (!IsLocating()) // Locating is not in process
							m_pBackgroundUpdater->StopWaiting();
					}
					pLvdi->item.mask=LVIF_TEXT;
					pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(pItem->GetDetailText(nDetail));
					break;
				}
				ItemDebugMessage("LVN_GETDISPINFOA END2");
			}
			break;
		}
	case LVN_GETDISPINFOW:
		{
			LV_DISPINFOW *pLvdi=(LV_DISPINFOW*)pNm;

			CLocatedItem* pItem=(CLocatedItem*)pLvdi->item.lParam;
			if (pItem==NULL)
				break;

			
			DetailType nDetail=DetailType(m_pListCtrl->GetColumnIDFromSubItem(pLvdi->item.iSubItem));

			if (GetFlags()&fgLVNoDelayedUpdate) 
			{
				ItemDebugFormatMessage1("LVN_GETDISPINFOW %d BEGIN1",nDetail);
				// Update detail instantaneously

				switch (nDetail)
				{
				// Title and parent are special since they have icons
				case Name:
					if (pItem->ShouldUpdateFileTitle())
						pItem->UpdateFileTitle();
					if (pItem->ShouldUpdateIcon())
                        pItem->UpdateIcon();

					pLvdi->item.mask=LVIF_TEXT|LVIF_IMAGE;
					if (pItem->GetFileTitle()!=NULL)
						pLvdi->item.pszText=pItem->GetFileTitle();
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
				case InFolder:
					if (pItem->ShouldUpdateParentIcon())
						pItem->UpdateParentIcon();

					pLvdi->item.mask=LVIF_TEXT|LVIF_IMAGE;
					pLvdi->item.pszText=pItem->GetParent();
					pLvdi->item.iImage=pItem->GetParentIcon();
					break;
				default:
					ASSERT (nDetail<=LastType);
			
					if (GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating &&
						pItem->ShouldUpdateByDetail(nDetail))
						pItem->UpdateByDetail(nDetail);
					pLvdi->item.mask=LVIF_TEXT;
					pLvdi->item.pszText=pItem->GetDetailText(nDetail);
					break;
				}
				ItemDebugMessage("LVN_GETDISPINFOW END1");
			}
			else
			{
				ItemDebugFormatMessage1("LVN_GETDISPINFOW %d BEGIN2",nDetail);
			
				// Delayed updating
				switch (nDetail)
				{
				// Title and parent are special since they have icons
				case Name:
					if (pItem->ShouldUpdateFileTitle() || pItem->ShouldUpdateIcon())
					{
						if (m_pBackgroundUpdater==NULL)
						{
							InterlockedExchangePointer((PVOID*)&m_pBackgroundUpdater,new CBackgroundUpdater(m_pListCtrl));
							m_pBackgroundUpdater->CreateEventsAndMutex();
						}
						
						
						m_pBackgroundUpdater->AddToUpdateList(pItem,pLvdi->item.iItem,Name);
						
						if (!IsLocating()) // Locating in process
							m_pBackgroundUpdater->StopWaiting();
					}
					
					pLvdi->item.mask=LVIF_TEXT|LVIF_IMAGE;
					if (pItem->GetFileTitle()!=NULL)
						pLvdi->item.pszText=pItem->GetFileTitle();
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
				case InFolder:
					if (pItem->ShouldUpdateParentIcon())
					{
						if (m_pBackgroundUpdater==NULL)
						{
							InterlockedExchangePointer((PVOID*)&m_pBackgroundUpdater,new CBackgroundUpdater(m_pListCtrl));
							m_pBackgroundUpdater->CreateEventsAndMutex();
						}


						//DebugFormatMessage("Calling %X->AddToUpdateList(%X,%X,InFolder)",m_pBackgroundUpdater,pItem,pLvdi->item.iItem);
						m_pBackgroundUpdater->AddToUpdateList(pItem,pLvdi->item.iItem,InFolder);
						if (!IsLocating()) // Locating is not in process
							m_pBackgroundUpdater->StopWaiting();
					}

					pLvdi->item.mask=LVIF_TEXT|LVIF_IMAGE;
					pLvdi->item.pszText=pItem->GetParent();
					pLvdi->item.iImage=pItem->GetParentIcon();
					break;
				default:
					ASSERT (nDetail<=LastType);
			
					if (GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating &&
						pItem->ShouldUpdateByDetail(nDetail))
					{
						if (m_pBackgroundUpdater==NULL)
						{
							InterlockedExchangePointer((PVOID*)&m_pBackgroundUpdater,new CBackgroundUpdater(m_pListCtrl));
							m_pBackgroundUpdater->CreateEventsAndMutex();
						}

						//DebugFormatMessage("Calling %X->AddToUpdateList(%X,%X,%d)",m_pBackgroundUpdater,pItem,pLvdi->item.iItem,DWORD(nDetail));
						
						m_pBackgroundUpdater->AddToUpdateList(pItem,pLvdi->item.iItem,nDetail);
						if (!IsLocating()) // Locating is not in process
							m_pBackgroundUpdater->StopWaiting();
					}
					pLvdi->item.mask=LVIF_TEXT;
					pLvdi->item.pszText=pItem->GetDetailText(nDetail);
					break;
				}
				ItemDebugMessage("LVN_GETDISPINFOW END2");
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
	case LVN_BEGINLABELEDITA:
	case LVN_BEGINLABELEDITW:
		{
			NMLVDISPINFOA* pDistInfo=(NMLVDISPINFOA*)pNm;
			if (m_pListCtrl->GetColumnIDFromSubItem(pDistInfo->item.iSubItem)!=CLocateDlg::Name)
			{
				// Not allowed for other fields than Name
				SetWindowLong(dwlMsgResult,TRUE);
				return TRUE;
			}

			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(pDistInfo->item.iItem);
			if (pItem==NULL)
			{
				// Not allowed for other fields than Name
				SetWindowLong(dwlMsgResult,TRUE);
				return TRUE;
			}

			ASSERT(!pItem->ShouldUpdateFileTitle());
			ASSERT(!pItem->ShouldUpdateFilename());

			if (pItem->GetFileTitle()!=pItem->GetName())
			{
				int nTitleLen=istrlen(pItem->GetFileTitle());
				if (_wcsnicmp(pItem->GetName(),pItem->GetFileTitle(),nTitleLen)!=0)
				{
					// We can't determine which part should be changed in filename
					SetWindowLong(dwlMsgResult,TRUE);
					return TRUE;
				}
			}

			SetWindowLong(dwlMsgResult,FALSE);
			return FALSE;
		}
		break;
	case LVN_ENDLABELEDITA:
		{
			NMLVDISPINFOA* pDistInfo=(NMLVDISPINFOA*)pNm;
			if (pDistInfo->item.pszText==NULL)
				return TRUE;

			if (m_pListCtrl->GetColumnIDFromSubItem(pDistInfo->item.iSubItem)!=CLocateDlg::Name)
			{
				// Not allowed for other fields than Name
				SetWindowLong(dwlMsgResult,FALSE);
				return FALSE;
			}

			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(pDistInfo->item.iItem);
			if (pItem==NULL)
			{
				// Not allowed for other fields than Name
				SetWindowLong(dwlMsgResult,FALSE);
				return FALSE;
			}

			if (pItem->GetFileTitle()!=pItem->GetName())
			{
				int nOldTitleLen=istrlen(pItem->GetFileTitle());
				if (_wcsnicmp(pItem->GetName(),pItem->GetFileTitle(),nOldTitleLen)!=0)
				{
					// We can't determine which part should be changed in filename
					SetWindowLong(dwlMsgResult,FALSE);
					return FALSE;
				}
				int nNewTitleLen=istrlen(pDistInfo->item.pszText);

				ASSERT(pItem->GetNameLen()>=(UINT)nOldTitleLen);

				WCHAR* pNewName=new WCHAR[nNewTitleLen+pItem->GetNameLen()-nOldTitleLen+1];
				MemCopyAtoW(pNewName,pDistInfo->item.pszText,nNewTitleLen);
				MemCopyW(pNewName+nNewTitleLen,pItem->GetName()+nOldTitleLen,pItem->GetNameLen()-nOldTitleLen+1);
				pItem->ChangeName(this,pNewName,nNewTitleLen+pItem->GetNameLen()-nOldTitleLen+1);
				delete[] pNewName;
			}
			else
				pItem->ChangeName(this,A2W(pDistInfo->item.pszText));


			SetWindowLong(dwlMsgResult,TRUE);
			return TRUE;
		}
	case LVN_ENDLABELEDITW:
		{
			NMLVDISPINFOW* pDistInfo=(NMLVDISPINFOW*)pNm;
			if (pDistInfo->item.pszText==NULL)
				return TRUE;

			if (m_pListCtrl->GetColumnIDFromSubItem(pDistInfo->item.iSubItem)!=CLocateDlg::Name)
			{
				// Not allowed for other fields than Name
				SetWindowLong(dwlMsgResult,FALSE);
				return FALSE;
			}

			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(pDistInfo->item.iItem);
			if (pItem==NULL)
			{
				// Not allowed for other fields than Name
				SetWindowLong(dwlMsgResult,FALSE);
				return FALSE;
			}

			if (pItem->GetFileTitle()!=pItem->GetName())
			{
				int nOldTitleLen=istrlen(pItem->GetFileTitle());
				if (_wcsnicmp(pItem->GetName(),pItem->GetFileTitle(),nOldTitleLen)!=0)
				{
					// We can't determine which part should be changed in filename
					SetWindowLong(dwlMsgResult,FALSE);
					return FALSE;
				}
				int nNewTitleLen=istrlen(pDistInfo->item.pszText);

				ASSERT(pItem->GetNameLen()>=(UINT)nOldTitleLen);

				WCHAR* pNewName=new WCHAR[nNewTitleLen+pItem->GetNameLen()-nOldTitleLen+1];
				MemCopyW(pNewName,pDistInfo->item.pszText,nNewTitleLen);
				MemCopyW(pNewName+nNewTitleLen,pItem->GetName()+nOldTitleLen,pItem->GetNameLen()-nOldTitleLen+1);
				pItem->ChangeName(this,pNewName,nNewTitleLen+pItem->GetNameLen()-nOldTitleLen+1);
				delete[] pNewName;
			}
			else
				pItem->ChangeName(this,pDistInfo->item.pszText);


			SetWindowLong(dwlMsgResult,TRUE);
			return TRUE;
		}
	case LVN_GETINFOTIP:
		break;
	case LVN_ITEMCHANGED:
		if ((!(pNm->uOldState&LVIS_SELECTED) && pNm->uNewState&LVIS_SELECTED)||
			(!(pNm->uNewState&LVIS_SELECTED) && pNm->uOldState&LVIS_SELECTED))
		{
			SetTimer(ID_UPDATESELECTED,100,NULL);
		}
		break;
	}
	return FALSE;
}

void CLocateDlg::SetSorting(BYTE bSorting)
{
	if (bSorting==BYTE(-2))
	{
		CRegKey2 RegKey;
		bSorting=BYTE(-1);
		
		if (RegKey.OpenKey(HKCU,"\\General",CRegKey::defRead)==ERROR_SUCCESS)
		{
			DWORD nTemp=BYTE(-1);
			if (RegKey.QueryValue("Default Sorting",nTemp))
				bSorting=(BYTE)nTemp;
		}
	}


	if (bSorting==m_nSorting)
		return;

	if ((m_nSorting&127)!=(bSorting&127))
	{
		// Not same column, remove arrow
		SetSortArrowToHeader(DetailType(m_nSorting&127),TRUE,FALSE); 
	}

	m_nSorting=bSorting;
	
	SetSortArrowToHeader(DetailType(bSorting&127),FALSE,(bSorting&128)?TRUE:FALSE);
}

void CLocateDlg::SortItems(DetailType nDetail,BYTE bDescending,BOOL bNoneIsPossible)
{
	DebugFormatMessage("CLocateDlg::SortItems(%X,%d) BEGIN",int(nDetail),bDescending);

	// no sorting: m_nSorting=0xFF
	// descent=m_nSorting&128
	// detail=m_nSorting&127

	
	CWaitCursor wait;

	if ((m_nSorting&127)!=nDetail)
	{
		// Not same column, remove arrow
		SetSortArrowToHeader(DetailType(m_nSorting&127),TRUE,FALSE); 
	}

	// Toggle?        
	if (bDescending==BYTE(-1))
	{
		if ((m_nSorting&127)!=nDetail) // Different columnt, always ascending
			bDescending=FALSE;
		else if (bNoneIsPossible && m_nSorting&128)
		{
			// Disable sorting

			// Remove arrow
			SetSortArrowToHeader(DetailType(m_nSorting&127),TRUE,FALSE); 

			m_nSorting=BYTE(-1);	
			return;
		}
		else
			bDescending=(m_nSorting&128)==0;
	}

	if (nDetail!=Extension)
		SetSortArrowToHeader(nDetail,FALSE,bDescending);

	if (!bDescending)
	{ 
		// Ascending
		DebugFormatMessage("Going to sort(1), nColumn is %X",LPARAM(nDetail));
		BOOL bRet=m_pListCtrl->SortItems(ListViewCompareProc,(DWORD)(nDetail));
		DebugFormatMessage("bRet=%X",bRet);
		m_nSorting=nDetail&127;
	}
	else
	{
		// Descending
		DebugFormatMessage("Going to sort(2), nColumn is %X",LPARAM(nDetail));
		BOOL bRet=m_pListCtrl->SortItems(ListViewCompareProc,(DWORD)(nDetail|128));
		DebugFormatMessage("bRet=%X",bRet);
		m_nSorting=nDetail|128;
	}

	if (nDetail!=Extension)
	{
		int nColumn=m_pListCtrl->GetVisibleColumn(m_pListCtrl->GetColumnFromID(nDetail));
		if (nColumn!=-1)
			m_pListCtrl->SendMessage(LVM_FIRST+140/* LVM_SETSELECTEDCOLUMN */,nColumn,0);
	}

	DebugMessage("CLocateDlg::SortItems END");
}

int CALLBACK CLocateDlg::ListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CLocatedItem* pItem1=(CLocatedItem*)lParam1;
	CLocatedItem* pItem2=(CLocatedItem*)lParam2;

	// If "Show folders first" is chosen, separate files and directories
	if (GetLocateDlg()->GetFlags()&fgLVFoldersFirst)
	{
		if (pItem1->IsFolder() && !pItem2->IsFolder())
			return -1;
		else if (!pItem1->IsFolder() && pItem2->IsFolder())
			return 1;
	}

	// Search criteria is Extension
	if ((lParamSort&127)==Extension)
	{
		// Sort by extension
		if (lParamSort&128)
			return _wcsicmp(pItem2->GetExtension(),pItem1->GetExtension());
		return _wcsicmp(pItem1->GetExtension(),pItem2->GetExtension());
	}

	DetailType nDetail=DetailType(lParamSort&127);
	switch (nDetail)
	{
	case Name:
		if (pItem1->ShouldUpdateFileTitle())
			pItem1->UpdateFileTitle();
		if (pItem2->ShouldUpdateFileTitle())
			pItem2->UpdateFileTitle();
		
		if (lParamSort&128)
			return _wcsicmp(pItem2->GetFileTitle(),pItem1->GetFileTitle());
		return _wcsicmp(pItem1->GetFileTitle(),pItem2->GetFileTitle());
	case InFolder:
		if (lParamSort&128)
			return _wcsicmp(pItem2->GetParent(),pItem1->GetParent());
		return _wcsicmp(pItem1->GetParent(),pItem2->GetParent());
	case FullPath:
		if (lParamSort&128)
			return _wcsicmp(pItem2->GetPath(),pItem1->GetPath());
		return _wcsicmp(pItem1->GetPath(),pItem2->GetPath());
	case FileSize:
		if (!(GetLocateDlg()->GetExtraFlags()&efLVNoUpdateWhileSorting))
		{
			if (pItem1->ShouldUpdateFileSize())
				pItem1->UpdateFileSize();
			if (pItem2->ShouldUpdateFileSize())
				pItem2->UpdateFileSize();
		}

		if (pItem2->IsFolder())
		{
			if (pItem1->IsFolder())
				return 0;
			return lParamSort&128?-1:1;
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
	case FileType:
		if (pItem1->ShouldUpdateType())
			pItem1->UpdateType();
		if (pItem2->ShouldUpdateType())
			pItem2->UpdateType();
		if (lParamSort&128)
			return _wcsicmp(pItem2->GetType(),pItem1->GetType());
		return _wcsicmp(pItem1->GetType(),pItem2->GetType());
	case DateModified:
		if (!(GetLocateDlg()->GetExtraFlags()&efLVNoUpdateWhileSorting))
		{
			if (pItem1->ShouldUpdateTimeAndDate())
				pItem1->UpdateFileTime();
			if (pItem2->ShouldUpdateTimeAndDate())
				pItem2->UpdateFileTime();
		}		
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
	case DateCreated:
		if (!(GetLocateDlg()->GetExtraFlags()&efLVNoUpdateWhileSorting))
		{
			if (pItem1->ShouldUpdateTimeAndDate())
				pItem1->UpdateFileTime();
			if (pItem2->ShouldUpdateTimeAndDate())
				pItem2->UpdateFileTime();
		}
		
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
	case DateAccessed:
		if (!(GetLocateDlg()->GetExtraFlags()&efLVNoUpdateWhileSorting))
		{
			if (pItem1->ShouldUpdateTimeAndDate())
				pItem1->UpdateFileTime();
			if (pItem2->ShouldUpdateTimeAndDate())
				pItem2->UpdateFileTime();
		}

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
	case Attributes:
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
	case ImageDimensions:
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
	case Pages:
		if (pItem1->ShouldUpdateExtra(Pages))
			pItem1->UpdateSummaryProperties();
		if (pItem2->ShouldUpdateExtra(Pages))
			pItem2->UpdateSummaryProperties();
		
		if (pItem2->GetPages()==pItem1->GetPages())
			return 0;
		else if (pItem2->GetPages()>pItem1->GetPages())
			return lParamSort&128?1:-1;
		else
			return lParamSort&128?-1:1;
	case Owner:
	case ShortFileName:	
	case ShortFilePath:
	case MD5sum:
	case Author:
	case Title:
	case Subject:
	case Category:
	case Comments:
	case Description:
	case FileVersion:
	case ProductName:
	case ProductVersion:
		{
			if (pItem1->ShouldUpdateExtra(nDetail))
				pItem1->UpdateByDetail(nDetail);
			if (pItem2->ShouldUpdateExtra(nDetail))
				pItem2->UpdateByDetail(nDetail);
	
			LPCWSTR pText1=pItem1->GetExtraText(nDetail);
			LPCWSTR pText2=pItem2->GetExtraText(nDetail);

			if (pText2==NULL)
			{
				if (pText1==NULL)
					return 0;
				return lParamSort&128?-1:0;
			}
			else if (pText1==NULL)
				return lParamSort&128?1:-1;
			
			if (lParamSort&128)
				return _wcsicmp(pText2,pText1);
			return _wcsicmp(pText1,pText2);
		}
	case Database:
		if (lParamSort&128)
		{
			return _wcsicmp(GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetName(),
				GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetName());
		}
		return _wcsicmp(GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetName(),
			GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetName());
	case DatabaseDescription:
		if (lParamSort&128)
		{
			return _wcsicmp(GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetDescription(),
				GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetDescription());
		}
		return _wcsicmp(GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetDescription(),
			GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetDescription());
	case DatabaseArchive:
		if (lParamSort&128)
		{
			return _wcsicmp(GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetArchiveName(),
				GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetArchiveName());
		}
		return _wcsicmp(GetLocateApp()->GetDatabase(pItem1->GetDatabaseID())->GetArchiveName(),
			GetLocateApp()->GetDatabase(pItem2->GetDatabaseID())->GetArchiveName());
	case VolumeLabel:
		if (lParamSort&128)
		{
			return _wcsicmp(CLocateDlg::GetDBVolumeLabel(pItem2->GetDatabaseID(),pItem2->GetRootID()),
				CLocateDlg::GetDBVolumeLabel(pItem1->GetDatabaseID(),pItem1->GetRootID()));
		}
		return _wcsicmp(CLocateDlg::GetDBVolumeLabel(pItem1->GetDatabaseID(),pItem1->GetRootID()),
			CLocateDlg::GetDBVolumeLabel(pItem2->GetDatabaseID(),pItem2->GetRootID()));
	case VolumeSerial:
		if (lParamSort&128)
		{
			return _wcsicmp(CLocateDlg::GetDBVolumeSerial(pItem2->GetDatabaseID(),pItem2->GetRootID()),
				CLocateDlg::GetDBVolumeSerial(pItem1->GetDatabaseID(),pItem1->GetRootID()));
		}
		return _wcsicmp(CLocateDlg::GetDBVolumeSerial(pItem1->GetDatabaseID(),pItem1->GetRootID()),
			CLocateDlg::GetDBVolumeSerial(pItem2->GetDatabaseID(),pItem2->GetRootID()));
	case VolumeFileSystem:
		if (lParamSort&128)
		{
			return _wcsicmp(CLocateDlg::GetDBVolumeFileSystem(pItem2->GetDatabaseID(),pItem2->GetRootID()),
				CLocateDlg::GetDBVolumeFileSystem(pItem1->GetDatabaseID(),pItem1->GetRootID()));
		}
		return _wcsicmp(CLocateDlg::GetDBVolumeFileSystem(pItem1->GetDatabaseID(),pItem1->GetRootID()),
			CLocateDlg::GetDBVolumeFileSystem(pItem2->GetDatabaseID(),pItem2->GetRootID()));
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

void CLocateDlg::EnableItems(BOOL bEnable)
{
	CButton OK(GetDlgItem(IDC_OK));
	CButton Stop(GetDlgItem(IDC_STOP));
	OK.EnableWindow(bEnable);
	OK.SetButtonStyle(bEnable?BS_DEFPUSHBUTTON:BS_PUSHBUTTON);
	Stop.SetButtonStyle(bEnable?BS_PUSHBUTTON:BS_DEFPUSHBUTTON);
	Stop.EnableWindow(!bEnable);
	EnableDlgItem(IDC_PRESETS,bEnable);
	m_NameDlg.EnableItems(bEnable);
	m_SizeDateDlg.EnableItems(bEnable);
	m_AdvancedDlg.EnableItems(bEnable);

	if (bEnable)
	{
		HWND hFocus=GetFocus();
			
		if ((GetFlags()&fgLVActivateFirstResult) && m_pListCtrl->GetItemCount()>0 &&
			m_pListCtrl->GetSelectedCount()==0)
		{
			if (hFocus==NULL)
			{
				// Not any controls activated
				int nItem=m_pListCtrl->GetNextItem(-1,LVNI_ALL);
				if (nItem!=-1)
					m_pListCtrl->SetItemState(nItem,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);

				PostMessage(WM_SETITEMFOCUS,(WPARAM)(HWND)*m_pListCtrl);
			}
			else if (hFocus==*m_pListCtrl && m_pListCtrl->GetSelectedCount()==0)
			{
				// Result list where activated but not any items selected
				int nItem=m_pListCtrl->GetNextItem(-1,LVNI_ALL);
				if (nItem!=-1)
					m_pListCtrl->SetItemState(nItem,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
			}
		}
		else
		{
			if (hFocus==NULL)
			{
				// Give focus to the selected tab
				switch (m_pTabCtrl->GetCurSel())
				{
				case 0:
					m_NameDlg.SetFocus();
					break;
				case 1:
					m_SizeDateDlg.SetFocus();
					break;
				case 2:
					m_AdvancedDlg.SetFocus();
					break;
				}
				
				
				if (m_hLastFocus!=NULL)
				{
					PostMessage(WM_SETITEMFOCUS,(WPARAM)::GetParent(m_hLastFocus));
					PostMessage(WM_SETITEMFOCUS,(WPARAM)m_hLastFocus);
					m_hLastFocus=NULL;
				}
			}
		}
	}
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
	
	CMINVOKECOMMANDINFOEX cii;
	ZeroMemory(&cii,sizeof(CMINVOKECOMMANDINFOEX));
	cii.cbSize=sizeof(CMINVOKECOMMANDINFOEX);
	cii.fMask=CMIC_MASK_UNICODE;
	cii.hwnd=*this;
	cii.lpVerb=(LPCSTR)MAKELONG(wID-IDM_DEFCONTEXTITEM,0);
	cii.lpVerbW=(LPCWSTR)MAKELONG(wID-IDM_DEFCONTEXTITEM,0);
	cii.lpDirectoryW=NULL;
	cii.lpDirectory=alloccopyWtoA(cii.lpDirectoryW);
	cii.nShow=SW_SHOWDEFAULT;
	m_pActiveContextMenu->pContextMenu->InvokeCommand((CMINVOKECOMMANDINFO*)&cii);
	delete[] (LPSTR)cii.lpDirectory;
	
	//ClearMenuVariables();

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
	
void CLocateDlg::OnExecuteFile(LPCWSTR szVerb,int nItem)
{
	CWaitCursor wait;

	int nSelectedItems;
	CLocatedItem** pItems=GetSeletedItems(nSelectedItems,nItem);
	
	for (int i=0;i<nSelectedItems;i++)
	{
		if (pItems[i]==NULL)
			continue;

		if (!pItems[i]->IsFolder() && !FileSystem::IsFile(pItems[i]->GetPath()))
			continue;
		if (pItems[i]->IsFolder() && !FileSystem::IsDirectory(pItems[i]->GetPath()))
			continue;
		
		if (pItems[i]->IsFolder())
			OpenFolder(pItems[i]->GetPath());
		else 
		{
			int nRet;
			if (IsUnicodeSystem())
				nRet=(int)ShellExecuteW(*this,szVerb,pItems[i]->GetPath(),NULL,NULL,SW_SHOW);
			else
				nRet=(int)ShellExecuteA(*this,szVerb==NULL?NULL:(LPCSTR)W2A(szVerb),W2A(pItems[i]->GetPath()),NULL,NULL,SW_SHOW);

			if (nRet<=32)
			{
				ContextMenuStuff* pContextMenuStuff=GetContextMenuForItems(1,&pItems[i]);
				if (pContextMenuStuff!=NULL)
				{
					CMINVOKECOMMANDINFOEX cii;
					ZeroMemory(&cii,sizeof(CMINVOKECOMMANDINFOEX));
					cii.cbSize=sizeof(CMINVOKECOMMANDINFOEX);
					cii.fMask=CMIC_MASK_UNICODE;
					cii.hwnd=*this;
					cii.lpVerbW=szVerb;
					cii.lpVerb=szVerb!=NULL?alloccopyWtoA(szVerb):NULL;
					cii.nShow=SW_SHOWDEFAULT;
					HMENU hMenu=CreatePopupMenu();
					pContextMenuStuff->pContextMenu->QueryContextMenu(hMenu,0,IDM_DEFCONTEXTITEM,IDM_DEFSENDTOITEM,CMF_DEFAULTONLY|CMF_VERBSONLY);
					pContextMenuStuff->pContextMenu->InvokeCommand((CMINVOKECOMMANDINFO*)&cii);
					if (szVerb!=NULL)
						delete[] (LPSTR)cii.lpVerb;
					
					delete pContextMenuStuff;
					DestroyMenu(hMenu);
				}
			}
		}
		
	}

	delete[] pItems;
}

BOOL CLocateDlg::GetSimpleIDLsandParentForSelectedItems(int& nItems,LPITEMIDLIST& rpParentIDL,LPITEMIDLIST*& rpSimpleIDLs)
{
	int nSelected=m_pListCtrl->GetSelectedCount();

	if (nSelected==0)
		return FALSE;

	if (m_pDesktopFolder==NULL)
		return FALSE;

	LPITEMIDLIST* pFullIDLs=new LPITEMIDLIST[nSelected];
	
	int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	nItems=0;
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
		
		if (pItem!=NULL)
		{
			if (SUCCEEDED(m_pDesktopFolder->ParseDisplayName(*this,NULL,pItem->GetPath(),NULL,&pFullIDLs[nItems],NULL)))
				nItems++;
			
		}

		nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
	}

	ASSERT(nItems<nSelected);

	BOOL bRet=GetSimpleIDLsandParentfromIDLs(nItems,pFullIDLs,&rpParentIDL,rpSimpleIDLs);

	for (int i=0;i<nItems;i++)
		CoTaskMemFree(pFullIDLs[i]);
	delete[] pFullIDLs;

	if (!bRet)
		delete[] rpSimpleIDLs;	
	
	return bRet;
}
	
BOOL CLocateDlg::GetSimpleIDLsandParentfromIDLs(int nItems,LPITEMIDLIST* pFullIDLs,LPITEMIDLIST* rpParentIDL,LPITEMIDLIST* rpSimpleIDLs,int* pParentIDLLevel)
{
	if (m_pDesktopFolder==NULL)
		return FALSE;


	int nEqualLevels=0;
	
	if (nItems==1)
	{
		// Handle 1 file separately

		LPITEMIDLIST pidlLast, pidlNext;
		
		// Split pidl into parent part and the last part
		for (pidlLast = pFullIDLs[0]; 
			pidlNext = (LPITEMIDLIST)((BYTE*)pidlLast + pidlLast->mkid.cb),pidlNext->mkid.cb;
			pidlLast = pidlNext) 
			nEqualLevels++;

		if (pFullIDLs[0]==pidlLast)
			return FALSE;

		// Copy parent IDL
		DWORD nLength=DWORD(LPBYTE(pidlLast)-LPBYTE(pFullIDLs[0]));
		*rpParentIDL=(LPITEMIDLIST)CoTaskMemAlloc(nLength+2);
		CopyMemory(*rpParentIDL,pFullIDLs[0],nLength);
		*((WORD*)(LPBYTE(*rpParentIDL)+nLength))=0;

	

		// Copy simple IDL
		rpSimpleIDLs[0]=(LPITEMIDLIST)CoTaskMemAlloc(pidlLast->mkid.cb+2);
		CopyMemory(rpSimpleIDLs[0],pidlLast,pidlLast->mkid.cb+2);

		if (pParentIDLLevel!=NULL)
			*pParentIDLLevel=nEqualLevels;
		return TRUE;
	}

	if (pFullIDLs[0]->mkid.cb==0)
	{
		// Empty IDL
		return FALSE;
	}


	
	// Pointers
	LPITEMIDLIST* pPtr=new LPITEMIDLIST[nItems];
	for (int i=0;i<nItems;i++)
		pPtr[i]=pFullIDLs[i];
	

	// Check highest parent level
	BOOL bAllEqual=TRUE;
	DWORD nParentLength=0;
	
	while (((LPITEMIDLIST)((BYTE*)pPtr[0] + pPtr[0]->mkid.cb))->mkid.cb) // Net the last
	{
		// Side of first ID of first item lsit
		int i;
		WORD cb0,cb,cbpre=pPtr[0]->mkid.cb;

		pPtr[0]=(LPITEMIDLIST)((BYTE*)pPtr[0] + pPtr[0]->mkid.cb);
		cb0=pPtr[0]->mkid.cb;
		pPtr[0]->mkid.cb=0;

		for (i=1;i<nItems;i++)
		{
			pPtr[i]=(LPITEMIDLIST)((BYTE*)pPtr[i] + pPtr[i]->mkid.cb);
			cb=pPtr[i]->mkid.cb;
			pPtr[i]->mkid.cb=0;

			
			HRESULT hRes=m_pDesktopFolder->CompareIDs(0,pFullIDLs[0],pFullIDLs[i]);
			pPtr[i]->mkid.cb=cb;

			if (FAILED(hRes) || HRESULT_CODE(hRes)!=0)
			{
				bAllEqual=FALSE;
				break;
			}

		}

		pPtr[0]->mkid.cb=cb0;

		if (!bAllEqual)
			break;

		nEqualLevels++;

		nParentLength+=cbpre;
		
	}

	if (nEqualLevels==0)
	{
		delete[] pPtr;
		return FALSE;
	}

	if (pParentIDLLevel!=NULL)
		*pParentIDLLevel=nEqualLevels;
	
	
	// Copy parent IDL
	*rpParentIDL=(LPITEMIDLIST)CoTaskMemAlloc(nParentLength+2);
	CopyMemory(*rpParentIDL,pFullIDLs[0],nParentLength);
	*((WORD*)(LPBYTE(*rpParentIDL)+nParentLength))=0;

	// Copy simple IDLs
	for (int i=0;i<nItems;i++)
	{
		DWORD nLength=GetIDListSize((LPITEMIDLIST)(LPBYTE(pFullIDLs[i])+nParentLength));

		rpSimpleIDLs[i]=(LPITEMIDLIST)CoTaskMemAlloc(nLength);
		CopyMemory(rpSimpleIDLs[i],LPBYTE(pFullIDLs[i])+nParentLength,nLength);
	}
	return TRUE;
}

void CLocateDlg::OnProperties(int nItem)
{
	if (m_pListCtrl->GetSelectedCount()==0)
		return;

	
	
	int nItems;
	CLocatedItem** pItems=GetSeletedItems(nItems,nItem);
	ContextMenuStuff* pContextMenu=GetContextMenuForItems(nItems,pItems);

	if (pContextMenu!=NULL)
	{
		if (pContextMenu->nIDLParentLevel>1)
		{
			CMINVOKECOMMANDINFO cii;
			cii.cbSize=sizeof(CMINVOKECOMMANDINFO);
			cii.fMask=0;
			cii.hwnd=*this;
			cii.lpVerb="properties";
			cii.lpParameters=NULL;
			cii.lpDirectory=NULL;
			cii.nShow=SW_SHOWDEFAULT;
			HMENU hMenu=CreatePopupMenu();
			pContextMenu->pContextMenu->QueryContextMenu(hMenu,0,IDM_DEFCONTEXTITEM,IDM_DEFSENDTOITEM,CMF_DEFAULTONLY|CMF_VERBSONLY);
			pContextMenu->pContextMenu->InvokeCommand(&cii);
			DestroyMenu(hMenu);
			delete[] pItems;
			delete pContextMenu;		
			return;
		}
		delete pContextMenu;		
	}

	CPropertiesSheet* fileprops=new CPropertiesSheet(nItems,pItems);
	delete[] pItems;

	fileprops->Open();
}

void CLocateDlg::OnAutoArrange()
{
	CMenu menu(GetMenu());
	if (m_pListCtrl->GetStyle()&LVS_AUTOARRANGE)
	{
		menu.CheckMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_UNCHECKED);
		CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_UNCHECKED);
		m_pListCtrl->SetStyle(m_pListCtrl->GetStyle()&~LVS_AUTOARRANGE);
	}
	else
	{
		menu.CheckMenuItem(IDM_AUTOARRANGE,MF_BYCOMMAND|MF_CHECKED);
		CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_AUTOARRANGE,MF_BYCOMMAND|MF_CHECKED);
		m_pListCtrl->SetStyle(m_pListCtrl->GetStyle()|LVS_AUTOARRANGE);
	}
}

void CLocateDlg::OnAlignToGrid()
{
	CMenu menu(GetMenu());

	if (m_pListCtrl->GetExtendedListViewStyle()&LVS_EX_SNAPTOGRID)
	{
		menu.CheckMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_UNCHECKED);
		CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_UNCHECKED);
		m_pListCtrl->SetExtendedListViewStyle(LVS_EX_SNAPTOGRID,0);
	}
	else
	{
		menu.CheckMenuItem(IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_CHECKED);
		CheckMenuItem(m_Menu.GetSubMenu(SUBMENU_CONTEXTMENUNOITEMS),IDM_ALIGNTOGRID,MF_BYCOMMAND|MF_CHECKED);
		m_pListCtrl->SetExtendedListViewStyle(LVS_EX_SNAPTOGRID,LVS_EX_SNAPTOGRID);
		m_pListCtrl->Arrange(LVA_SNAPTOGRID);
		
	}
}

void CLocateDlg::OnRefresh()
{
	m_pListCtrl->RedrawItems(m_pListCtrl->GetTopIndex(),m_pListCtrl->GetTopIndex()+m_pListCtrl->GetCountPerPage());
	m_pListCtrl->UpdateWindow();
}	
		
void CLocateDlg::OnDelete(CLocateDlg::DeleteFlag DeleteFlag,int nItem)
{
	if (DeleteFlag==BasedOnShift)
	{
		if (HIBYTE(GetKeyState(VK_SHIFT)))
			DeleteFlag=Delete;
		else
			DeleteFlag=Recycle;
	}

	CWaitCursor wait;
	CArray<CLocatedItem*> aItems;
	int nBufferLength=1; // For buffer length
	
	// Resolving files
	int iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	if (iItem==-1)
	{
		if (nItem==-1)
			return;

		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
		LPCWSTR szPath=pItem->GetPath();
		if (pItem->IsFolder())
		{
			if (FileSystem::IsDirectory(szPath))
			{
                aItems.Add(pItem);
				nBufferLength+=pItem->GetPathLen()+1;
			}
		}
		else
		{
			if (FileSystem::IsFile(szPath))
			{
                aItems.Add(pItem);
				nBufferLength+=pItem->GetPathLen()+1;
			}
		}
	
	}
	else
	{
		while (iItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
			LPCWSTR szPath=pItem->GetPath();
			if (pItem->IsFolder())
			{
				if (FileSystem::IsDirectory(szPath))
				{
					aItems.Add(pItem);
					nBufferLength+=pItem->GetPathLen()+1;
				}
			}
			else
			{
				if (FileSystem::IsFile(szPath))
				{
					aItems.Add(pItem);
					nBufferLength+=pItem->GetPathLen()+1;
				}
			}
			iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
		}
	}


	if (aItems.GetSize()==0) // No files
		return;


	// Filling OPSTRUCT fields
	SHFILEOPSTRUCTW fo;
	fo.hwnd=*this;
	fo.wFunc=FO_DELETE;
	fo.pTo=NULL;

	switch (DeleteFlag)
	{
	case Delete:
		fo.fFlags=0;
		break;
	case Recycle:
		fo.fFlags=FOF_ALLOWUNDO;
		break;
	default:
		if (HIBYTE(GetKeyState(VK_SHIFT)))
			fo.fFlags=0;
		else
			fo.fFlags=FOF_ALLOWUNDO;
		break;
	}
	// Creating file buffer: file1\0file2\0...filen\0\0
	WCHAR* pFiles=new WCHAR[nBufferLength];
	fo.pFrom=pFiles;
	for (int i=0;i<aItems.GetSize();i++)
	{
		MemCopyW(pFiles,aItems.GetAt(i)->GetPath(),aItems.GetAt(i)->GetPathLen()+1);
		pFiles+=aItems.GetAt(i)->GetPathLen()+1;
	}
	*pFiles='\0';
	
	StopBackgroundOperations();
	
	
	// Delete files
	int iRet=FileOperation(&fo);

	
	delete[] fo.pFrom;

	
	
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
			LPCWSTR szPath=pItem->GetPath();
		
			if (FileSystem::IsFile(szPath))
			{
				iSeekStart=iItem;
				continue;
			}
			else if (FileSystem::IsDirectory(szPath))
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
	if (m_pListCtrl->GetItemCount()>0)
	{
		iItem=m_pListCtrl->GetNextItem(-1,LVNI_ALL);
		while(iItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
			pItem->CheckIfDeleted();
			ASSERT(m_pBackgroundUpdater!=NULL);
			//DebugFormatMessage("Calling %X->AddToUpdateList(%X,%X,CLocateDlg::Needed)",m_pBackgroundUpdater,pItem,nItem);
					
			m_pBackgroundUpdater->AddToUpdateList(pItem,iItem,CLocateDlg::Needed);			
			iItem=m_pListCtrl->GetNextItem(iItem,LVNI_ALL);
		}

		m_pListCtrl->RedrawItems(0,m_pListCtrl->GetItemCount());
	}
	
	m_pListCtrl->UpdateWindow();
	m_pBackgroundUpdater->StopWaiting();

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

void CLocateDlg::OnRemoveDeletedFiles()
{
	CIntArray aDeleted;

	if (m_pListCtrl==NULL)
		return;
	if (m_pListCtrl->GetItemCount()==0)
		return;

	int iPrevItem=-1;
	int iItem=m_pListCtrl->GetNextItem(-1,LVNI_ALL);
	while (iItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
		if (pItem!=NULL)
		{
			if (pItem->IsDeleted())
			{
				m_pListCtrl->DeleteItem(iItem);
				iItem=m_pListCtrl->GetNextItem(iPrevItem,LVNI_ALL);
				continue;
			}
		}
		iItem=m_pListCtrl->GetNextItem(iItem,LVNI_ALL);
	}
}


BOOL CLocateDlg::CheckClipboard()
{
	//DebugMessage("CLocateDlg::CheckClipboard() BEGIN");
	
	if (m_pListCtrl==NULL)
		return TRUE;
	if (m_pListCtrl->GetItemCount()==0)
		return TRUE; // No need to do anything
	
	
	CArrayFAP<LPWSTR> aFiles;
	BYTE bIsCuttedFiles=FALSE;
	UINT nPreferredDropEffectFormat=RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	
	OpenClipboard();
	
	while (IsClipboardFormatAvailable(nPreferredDropEffectFormat))
	{
		HANDLE hData=GetClipboardData(nPreferredDropEffectFormat);
		if (hData==NULL)
			break;

		BYTE* pData=(BYTE*)GlobalLock(hData);
		if (pData==NULL)
			break;
        
		BYTE bDropEffect=*pData;
        GlobalUnlock(hData);        		

		if (!(bDropEffect&DROPEFFECT_MOVE))
			break;

		if (IsClipboardFormatAvailable(CF_HDROP))
		{
			HANDLE hData=GetClipboardData(CF_HDROP);
			if (hData!=NULL)
			{
				if (IsUnicodeSystem())
				{
					bIsCuttedFiles=TRUE;
					int last=DragQueryFileW((HDROP)hData,0xFFFFFFFF,NULL,0);
					for (int i=0;i<last;i++)
					{
						UINT nLen=DragQueryFileW((HDROP)hData,i,NULL,0);
						if (nLen>0)
						{
							LPWSTR szPath=new WCHAR[nLen+2];
							if (DragQueryFileW((HDROP)hData,i,szPath,nLen+2)>0)
								aFiles.Add(szPath);
							else
								delete[] szPath;
						}
					}
				}
				else
				{
					bIsCuttedFiles=TRUE;
					int last=DragQueryFile((HDROP)hData,0xFFFFFFFF,NULL,0);
					for (int i=0;i<last;i++)
					{
						char szPath[MAX_PATH];
						if (DragQueryFile((HDROP)hData,i,szPath,MAX_PATH)>0)
							aFiles.Add(alloccopyAtoW(szPath));
					}
				}
			}
		}
		break;
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
				if (_wcsicmp(aFiles[i],pItem->GetPath())==0)
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

void CLocateDlg::OnRenameFile(int nItem)
{
	if (m_pListCtrl->GetSelectedCount()==0 && nItem==-1)
		return;
	

	if (GetFlags()&fgLVAllowInPlaceRenaming)
	{
		if (m_pListCtrl->GetSelectedCount()>0)
			m_pListCtrl->EditLabel(m_pListCtrl->GetNextItem(-1,LVNI_SELECTED));
		else if (nItem!=-1)
			m_pListCtrl->EditLabel(nItem);
	}
	else
		OnChangeFileName();
}
	


void CLocateDlg::OnCopy(BOOL bCut,int nItem)
{
	if (m_pListCtrl->GetSelectedCount()==0 && nItem==-1)
		return;

	CWaitCursor wait;
	CArray<CLocatedItem*> aItems;
	CFileObject fo;

	if (m_pListCtrl->GetSelectedCount()>0)
		fo.SetFiles(m_pListCtrl,TRUE);
    else
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
		if (!FileSystem::IsFile(pItem->GetPath()) &&
			!FileSystem::IsDirectory(pItem->GetPath()))
			return;

		fo.SetFile(pItem->GetPath());
	}


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
	

struct FolderInfo{
	CStringW sFolder;
	CArrayFAP<LPCWSTR> aItems;

	FolderInfo(LPCWSTR szFolder) { sFolder=szFolder;}
	
};

void CLocateDlg::OnOpenFolder(BOOL bContaining,int nItem)
{
	CWaitCursor wait;
	
	int nSelectedItems;
	CLocatedItem** pItems=GetSeletedItems(nSelectedItems,nItem);

	if (nSelectedItems==0)
	{
		delete[] pItems;
		return;
	}


	if (bContaining)
	{
		// Loading some general settings
		CRegKey2 RegKey;
		if (RegKey.OpenKey(HKCU,"\\General",CRegKey::defRead)==ERROR_SUCCESS)
		{	
			DWORD dwTemp=0;
			RegKey.QueryValue("Use other program to open folders",dwTemp);
			RegKey.CloseKey();

			if (!dwTemp)	
			{
				// No program specified, using explorer
				HRESULT(STDAPICALLTYPE * pSHOpenFolderAndSelectItems)(LPCITEMIDLIST,UINT,LPCITEMIDLIST*,DWORD)=
					(HRESULT(STDAPICALLTYPE *)(LPCITEMIDLIST,UINT,LPCITEMIDLIST*,DWORD))GetProcAddress(GetModuleHandle("shell32.dll"),"SHOpenFolderAndSelectItems");
				
				if (pSHOpenFolderAndSelectItems!=NULL && m_pDesktopFolder!=NULL)
				{
					CArrayFP<FolderInfo*> aFolders;
					int i;

					// Sorting folders
					for (i=0;i<nSelectedItems;i++)
					{
						int j;
						for (j=0;j<aFolders.GetSize();j++)
						{
							if (aFolders[j]->sFolder.Compare(pItems[i]->GetParent())==0)
							{
								aFolders[j]->aItems.Add(alloccopy(pItems[i]->GetName()));
								break;
							}
						}

						if (j==aFolders.GetSize())
						{
							aFolders.Add(new FolderInfo(pItems[i]->GetParent()));
							aFolders.GetLast()->aItems.Add(alloccopy(pItems[i]->GetName()));
						}
					}

					// Initializing pDesktopFolder
					IShellFolder* pParentFolder;
					LPITEMIDLIST pParentIDList;
					HRESULT hRes;

					
								
					// Open folders
					for (i=0;i<aFolders.GetSize();i++)
					{
						CStringW sFolder(aFolders[i]->sFolder);


						if (sFolder[1]==':' && sFolder[2]=='\0')
							sFolder << L'\\';
	
						// Getting ID list of parent
						hRes=m_pDesktopFolder->ParseDisplayName(*this,NULL,(LPOLESTR)(LPCWSTR)sFolder,NULL,&pParentIDList,NULL);
						if (!SUCCEEDED(hRes))
							continue;
						
						// Querying IShellFolder interface for parent
						hRes=m_pDesktopFolder->BindToObject(pParentIDList,NULL,IID_IShellFolder,(void**)&pParentFolder);
						if (!SUCCEEDED(hRes))
						{
							CoTaskMemFree(pParentIDList);
							continue;
						}
						

                        // Querying id lists for files
						LPCITEMIDLIST* pItemPids=new LPCITEMIDLIST[aFolders[i]->aItems.GetSize()];
                        for (int j=0;j<aFolders[i]->aItems.GetSize();j++)
						{
							hRes=pParentFolder->ParseDisplayName(*this,NULL,(LPOLESTR)aFolders[i]->aItems[j],NULL,(LPITEMIDLIST*)&pItemPids[j],NULL);
							if (!SUCCEEDED(hRes))
								pItemPids[j]=NULL;
						}


						// Opening folder and selecting items
						hRes=pSHOpenFolderAndSelectItems(pParentIDList,aFolders[i]->aItems.GetSize(),pItemPids,0);

						// Free 
						for (int j=0;j<aFolders[i]->aItems.GetSize();j++)
							CoTaskMemFree((void*)pItemPids[j]);

						CoTaskMemFree(pParentIDList);

						pParentFolder->Release();
					}
				}
				else if (IsUnicodeSystem())
				{
					CStringW sArg;
					SHELLEXECUTEINFOW sxi;
					sxi.cbSize=sizeof(SHELLEXECUTEINFOW);
					sxi.fMask=SEE_MASK_NOCLOSEPROCESS;
					sxi.hwnd=*this;
					sxi.lpVerb=L"open";
					sxi.lpFile=L"explorer.exe";
					sxi.lpDirectory=szwEmpty;
					sxi.nShow=SW_SHOWNORMAL;
						
					for (int i=0;i<nSelectedItems;i++)
					{
						if (pItems[i]->IsDeleted())
							OpenFolder(pItems[i]->GetParent());
						else
						{
                            sArg.Format(L"/e,/select,\"%s\"",pItems[i]->GetPath());
							sxi.lpParameters=sArg;
							ShellExecuteExW(&sxi);
						}
					}
					
				}
				else
				{
					CString sArg;
					SHELLEXECUTEINFO sxi;
					sxi.cbSize=sizeof(SHELLEXECUTEINFO);
					sxi.fMask=SEE_MASK_NOCLOSEPROCESS;
					sxi.hwnd=*this;
					sxi.lpVerb="open";
					sxi.lpFile="explorer.exe";
					sxi.lpDirectory=szEmpty;
					sxi.nShow=SW_SHOWNORMAL;
						
					for (int i=0;i<nSelectedItems;i++)
					{
						if (pItems[i]->IsDeleted())
							OpenFolder(pItems[i]->GetParent());
						else
						{
                            sArg.Format("/e,/select,\"%S\"",pItems[i]->GetPath());
							sxi.lpParameters=sArg;
							ShellExecuteEx(&sxi);
						}
					}
					
				}

				delete[] pItems;
				return;
			}
			
		}

        // Retrieving folders
		CArray<LPCWSTR> aFolders;
		
		for (int i=0;i<nSelectedItems;i++)
		{
			
			if (pItems[i]!=NULL)
			{
				BOOL bFound=FALSE;
				for (int j=0;i<aFolders.GetSize();i++)
				{
					if (wcscmp(aFolders[j],pItems[i]->GetParent())==0)
					{
						bFound=TRUE;
						break;
					}
				}
				if (!bFound)
					aFolders.Add(pItems[i]->GetParent());
			}
		    
		}

		for (int i=0;i<aFolders.GetSize();i++)
			OpenFolder(aFolders[i]);
	}
	else
	{
		for (int i=0;i<nSelectedItems;i++)
		{
			if (pItems[i]!=NULL)
				OpenFolder(pItems[i]->GetPath());
		}
	}

	delete[] pItems;
}

void CLocateDlg::OpenFolder(LPCWSTR szFolder)
{
	CString sProgram;
	
	// Loading some general settings
	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::defRead)==ERROR_SUCCESS)
	{
		DWORD dwTemp=0;
		RegKey.QueryValue("Use other program to open folders",dwTemp);
		if (dwTemp)	
			RegKey.QueryValue("Open folders with",sProgram);
	}
	
	if (sProgram.IsEmpty())
	{
		SHELLEXECUTEINFOW sxi;
		sxi.cbSize=sizeof(SHELLEXECUTEINFO);
		sxi.fMask=SEE_MASK_NOCLOSEPROCESS;
		sxi.hwnd=*this;
		sxi.nShow=SW_SHOWNORMAL;
		sxi.lpParameters=szwEmpty;
		sxi.lpDirectory=szwEmpty;
			
		if (IsUnicodeSystem())		
		{
			sxi.lpVerb=L"open";
			sxi.lpFile=szFolder;
			ShellExecuteExW(&sxi);	
		}
		else
		{
			sxi.lpVerb=(LPWSTR)"open";
			sxi.lpFile=(LPWSTR)alloccopyWtoA(szFolder);
			ShellExecuteEx((SHELLEXECUTEINFOA*)&sxi);	
			delete[] (LPSTR)sxi.lpFile;
		}
		
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

		PROCESS_INFORMATION pi;
		STARTUPINFO si;
		si.cb=sizeof(STARTUPINFO);
		si.lpReserved=NULL;
		si.cbReserved2=0;
		si.lpReserved2=NULL;
		si.lpDesktop=NULL;
		si.lpTitle=NULL;
		si.dwFlags=STARTF_USESHOWWINDOW;
		si.wShowWindow=SW_SHOWDEFAULT;
		
		if (CreateProcess(NULL,sTemp.GetBuffer(),NULL,
			NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS,
			NULL,NULL,&si,&pi))
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}

	}
}

typedef HRESULT (__stdcall * PFNSHGETFOLDERPATHA)(HWND, int, HANDLE, DWORD, LPSTR);  // "SHGetFolderPathA"
typedef HRESULT (__stdcall * PFNSHGETFOLDERPATHW)(HWND, int, HANDLE, DWORD, LPWSTR);  // "SHGetFolderPathW"

void CLocateDlg::OnCreateShortcut()
{
	if (m_pListCtrl->GetSelectedCount()==0)
		return;
	
	CWaitCursor wait;
	CStringW sTargetFolder;
	union {
		IShellLinkW* pslw;
		IShellLink* psl;
	};
	IPersistFile* ppf;
	
	// Resolving desktop path
	if (IsUnicodeSystem())
	{
		CStringW sDesktopPathTmp;
		PFNSHGETFOLDERPATHW pGetFolderPath=(PFNSHGETFOLDERPATHW)GetProcAddress(GetModuleHandle("shell32.dll"),"SHGetFolderPathW");
		if (pGetFolderPath!=NULL)
			pGetFolderPath(*this,CSIDL_DESKTOPDIRECTORY,NULL,SHGFP_TYPE_CURRENT,sDesktopPathTmp.GetBuffer(MAX_PATH));
		else
		{
			CRegKey RegKey;
			if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
				RegKey.QueryValue(L"Desktop",sDesktopPathTmp);
			
		}
	
		if 	(!sDesktopPathTmp.IsEmpty())
		{
			sDesktopPathTmp.FreeExtra();
			sTargetFolder=sDesktopPathTmp;
		}
	}
	else
	{
		CString sDesktopPathTmp;
		PFNSHGETFOLDERPATHA pGetFolderPath=(PFNSHGETFOLDERPATHA)GetProcAddress(GetModuleHandle("shell32.dll"),"SHGetFolderPathA");
		if (pGetFolderPath!=NULL)
			pGetFolderPath(*this,CSIDL_DESKTOPDIRECTORY,NULL,SHGFP_TYPE_CURRENT,sDesktopPathTmp.GetBuffer(MAX_PATH));
		else
		{
			CRegKey RegKey;
			if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
				RegKey.QueryValue("Desktop",sDesktopPathTmp);
			
		}		

		if 	(!sDesktopPathTmp.IsEmpty())
		{
			sDesktopPathTmp.FreeExtra();
			sTargetFolder=sDesktopPathTmp;
		}
	}

	if (!sTargetFolder.IsEmpty())
	{
		switch(ShowErrorMessage(IDS_SHORTCUTTODESKTOP,IDS_SHORTCUT,MB_YESNOCANCEL|MB_ICONQUESTION))
		{
		case IDCANCEL:
			return;
		case IDNO:
			sTargetFolder.Empty();
			break;
		}
	}

	if (sTargetFolder.IsEmpty())
	{
		CFolderDialog fd(IDS_GETFOLDER,BIF_RETURNONLYFSDIRS|BIF_USENEWUI|BIF_NONEWFOLDERBUTTON);
		if (!fd.DoModal(*this))
			return;

		if (!fd.GetFolder(sTargetFolder))
			return;
	}

	if (sTargetFolder.LastChar()!=L'\\')
		sTargetFolder<<L'\\';



	if (IsUnicodeSystem())
	{
		// Creating instance to shell link handler
		if (!SUCCEEDED(CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLinkW,(void**)&pslw)))
			return; 

		// Creating instance to PersistFile interface
		if (!SUCCEEDED(pslw->QueryInterface(IID_IPersistFile,(void**)&ppf)))
		{
			pslw->Release();
			return;
		}
	}
	else
	{
		// Creating instance to shell link handler
		if (!SUCCEEDED(CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl)))
			return;

		// Creating instance to PersistFile interface
		if (!SUCCEEDED(psl->QueryInterface(IID_IPersistFile,(void**)&ppf)))
		{
			psl->Release();
			return;
		}

	}
	


	int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	BOOL bMsgShowed=FALSE;
	
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
		
	
		if (pItem->ShouldUpdateFileTitle())
			pItem->UpdateFileTitle();
			
		if (IsUnicodeSystem())
		{
			// Setting link path
			if (!SUCCEEDED(pslw->SetPath(pItem->GetPath())))
			{
				ppf->Release();
				pslw->Release();
				return;
			}
		
			if (!SUCCEEDED(pslw->SetDescription(CStringW(IDS_SHORTCUTTO)+pItem->GetFileTitle())))
			{
				ppf->Release();
				pslw->Release();
				return;
			}
		}
		else
		{
			// Setting link path
			if (!SUCCEEDED(psl->SetPath(W2A(pItem->GetPath()))))
			{
				ppf->Release();
				psl->Release();
				return;
			}
		
			if (!SUCCEEDED(psl->SetDescription(CStringA(IDS_SHORTCUTTO)+W2A(pItem->GetFileTitle()))))
			{
				ppf->Release();
				psl->Release();
				return;
			}
		}

		
		if (!SUCCEEDED(ppf->Save(sTargetFolder+pItem->GetFileTitle()+L".lnk",TRUE)))
		{
			ppf->Release();
			if (IsUnicodeSystem())
				pslw->Release();
			else
				psl->Release();

			return;
		}
		
		nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
	}

	ppf->Release();
	if (IsUnicodeSystem())
		pslw->Release();
	else
		psl->Release();
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
		int nSelectedItems=m_pListCtrl->GetSelectedCount();
		if (nSelectedItems>0)
		{
			CLocatedItem** pItems=new CLocatedItem* [nSelectedItems];
			int iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
			for (int i=0;i<nSelectedItems;i++)
			{
				pItems[i]=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
				iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
			}
			
			CreateFileContextMenu(hPopupMenu,pItems,nSelectedItems);
			delete[] pItems;
		}
		else
			CreateFileContextMenu(hPopupMenu,NULL,0);
			

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
		GetLocateApp()->OnInitDatabaseMenu(CMenu(hPopupMenu));
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
		DeleteMenu(hPopupMenu,i,MF_BYPOSITION);

	if (m_hSendToListFont!=NULL)
		DeleteObject(m_hSendToListFont);
	
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

			// SendTo menu item
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

			
			SHFILEINFOW fi;
			fi.hIcon=NULL;
			HGDIOBJ hOld=dc.SelectObject(m_hSendToListFont);
			GetFileInfo((LPITEMIDLIST)(lpdis->itemData),0,&fi,SHGFI_DISPLAYNAME|SHGFI_ICON|SHGFI_SMALLICON);
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
	/*case IDC_TAB:
		if (lpdis->itemAction==ODA_DRAWENTIRE)
		{
	
			HMODULE hUxTheme=GetModuleHandle("uxtheme.dll");
			if (hUxTheme!=NULL)
			{
				HRESULT(STDAPICALLTYPE *pDrawThemeParentBackground)(HWND hwnd,HDC hdc, RECT *prc);
				HRESULT(STDAPICALLTYPE *pDrawThemeEdge)(HANDLE,HDC,int,int,const RECT*,UINT,UINT, RECT *);
				HRESULT(STDAPICALLTYPE *pDrawThemeBackground)(HANDLE,HDC,int,int,const RECT*,const RECT*);
				HANDLE(STDAPICALLTYPE * pOpenThemeData)(HWND,LPCWSTR);
				HRESULT(STDAPICALLTYPE * pCloseThemeData)(HANDLE);

				pDrawThemeParentBackground=(HRESULT(STDAPICALLTYPE *)(HWND hwnd,HDC hdc, RECT *prc))GetProcAddress(hUxTheme,"DrawThemeParentBackground");
				pDrawThemeEdge=(HRESULT(STDAPICALLTYPE *)(HANDLE,HDC,int,int,const RECT*,UINT,UINT, RECT *))GetProcAddress(hUxTheme,"DrawThemeEdge");
				pDrawThemeBackground=(HRESULT(STDAPICALLTYPE *)(HANDLE,HDC,int,int,const RECT*,const RECT*))GetProcAddress(hUxTheme,"DrawThemeBackground");
				pOpenThemeData=(HANDLE(STDAPICALLTYPE*)(HWND,LPCWSTR))GetProcAddress(hUxTheme,"OpenThemeData");
				pCloseThemeData=(HRESULT(STDAPICALLTYPE *)(HANDLE))GetProcAddress(GetModuleHandle("uxtheme.dll"),"CloseThemeData");
		
			
				if (pDrawThemeParentBackground!=NULL)
					pDrawThemeParentBackground(lpdis->hwndItem,lpdis->hDC,&lpdis->rcItem);

				if (pOpenThemeData!=NULL)
				{
					HANDLE hTheme=pOpenThemeData(lpdis->hwndItem,L"tab");
					if (hTheme!=NULL)
					{
						if (pDrawThemeBackground!=NULL)
							pDrawThemeBackground(hTheme,lpdis->hDC,TABP_TOPTABITEMBOTHEDGE,(lpdis->itemState)&ODS_HOTLIGHT?TTIBES_HOT:TIBES_NORMAL,&lpdis->rcItem,NULL);
						
						//if (pDrawThemeEdge!=NULL)
						//	pDrawThemeEdge(hTheme,lpdis->hDC,TABP_TABITEMBOTHEDGE,TIBES_NORMAL,&lpdis->rcItem,EDGE_SUNKEN,BF_RECT,NULL);

						if (pCloseThemeData!=NULL)
							pCloseThemeData(hTheme);
					}
				}

			}
			
		}*/
	}
}

IDropTarget* CLocateDlg::GetDropTarget(LPITEMIDLIST pidl) const
{
	LPITEMIDLIST pidlLast, pidlNext;
    USHORT cb;
    IShellFolder* psf;
    IDropTarget* pdt;
		
	
    // Split pidl into parent part and the last part
	pidlLast = pidl;
    while(pidlNext = (LPITEMIDLIST)((BYTE*)pidlLast + pidlLast->mkid.cb),pidlNext->mkid.cb)
		pidlLast = pidlNext;

    //  Temporarily split the pidl at the point we found.
    cb = pidlLast->mkid.cb;
    pidlLast->mkid.cb = 0;

    //  Bind to the folder part.  
    if (pidl == pidlLast) 
	{
        psf = m_pDesktopFolder;
        m_pDesktopFolder->AddRef();
    }
	else
	{
		if (FAILED(m_pDesktopFolder->BindToObject(pidl,NULL,IID_IShellFolder,(LPVOID *)&psf)))
		{
			// Restore the pidl before exiting
			pidlLast->mkid.cb = cb;
			return NULL;
		}
	}
    
    // Restore the pidl we temporarily edited.
    pidlLast->mkid.cb = cb;
    HRESULT hRes = psf->GetUIObjectOf(*this,1,(LPCITEMIDLIST*)&pidlLast,IID_IDropTarget,NULL,(LPVOID*)&pdt);
    psf->Release();
	
	if (FAILED(hRes))
		return NULL;
	return pdt;
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

void CLocateDlg::BeginDragFiles(CListCtrl* pList)
{
	DebugMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList) BEGIN");
	{
		// Initializing COM objects
		CFileObject* pfo=new CFileObject;
		CFileSource* pfs=new CFileSource;
		
		pfo->AutoDelete();
		pfo->AddRef();
		pfo->SetFiles(pList,TRUE);

		pfs->AutoDelete();
		pfs->AddRef();
		
		// Inserting selected items to array
		int i=0,nSelectedItems=pList->GetSelectedCount();
		int* pSelectedItems=new int[max(nSelectedItems,2)];
		int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
		while (nItem!=-1 && i<nSelectedItems)
		{
			pSelectedItems[i++]=nItem;
			nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
		}
		ASSERT(nSelectedItems==i); // If didn't get all selected items
        
		

		DWORD nEffect;
		DebugMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList): DoDragDrop is about to be called");
		HRESULT hRes=DoDragDrop(pfo,pfs,DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK|DROPEFFECT_SCROLL,&nEffect);
		DebugFormatMessage("CLocateDlg::BeginDragFiles(CListCtrl* pList): DoDragDrop returned %X",hRes);

		pfo->Release();
		pfs->Release();

		for (int i=0;i<nSelectedItems;i++)
		{
			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(pSelectedItems[i]);
			DebugFormatMessage("CLocateDlg::WM_UPDATEITEMS: updating item %X",pItem);
	        
			// Just disabling flags, let background thread do the rest
			pItem->RemoveFlags(LITEM_COULDCHANGE);
			DebugMessage("CLocateDlg::WM_UPDATEITEMS: removed flags");

			if (m_pBackgroundUpdater!=NULL)
			{
				m_pBackgroundUpdater->AddToUpdateList(pItem,pSelectedItems[i],Needed);
	
				DebugMessage("CLocateDlg::WM_UPDATEITEMS: calling m_pBackgroundUpdater->StopWaiting()");
				m_pBackgroundUpdater->StopWaiting();
			}
		}
		
		delete[] pSelectedItems;
	}
}


BOOL CLocateDlg::ResolveSystemLVStatus()
{
	m_dwFlags=(m_dwFlags&~fgLVStyleFlag)|fgLVStyleSystemDefine;

	SHELLFLAGSTATE sfs;
	SHGetSettings(&sfs,SSF_DOUBLECLICKINWEBVIEW);

	if (sfs.fDoubleClickInWebView==0)
	{
		CRegKey RegKey;
		if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		{
			char temp[4];
			m_dwFlags|=fgLVStylePointToSelect;
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
	return TRUE;
}

BOOL CLocateDlg::SetListSelStyle()
{
	if (m_pListCtrl==NULL)
		return FALSE;
	if (m_dwFlags&fgLVStyleSystemDefine)
		ResolveSystemLVStatus();
	if (m_dwFlags&fgLVStylePointToSelect)
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
	
	m_pListCtrl->SetExtendedListViewStyle(LVS_EX_FULLROWSELECT,m_dwFlags&fgLVSelectFullRow?LVS_EX_FULLROWSELECT:0);
	return TRUE;
}

BOOL CLocateDlg::StartLocateAnimation()
{
	EnterCriticalSection(&m_csAnimBitmaps);
	if (m_pLocateAnimBitmaps!=NULL)
	{
		LeaveCriticalSection(&m_csAnimBitmaps);
		return TRUE;
	}
	
		
	m_pLocateAnimBitmaps=new HICON[6];
	m_pLocateAnimBitmaps[0]=(HICON)LoadImage(IDI_LANIM1,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
	m_pLocateAnimBitmaps[1]=(HICON)LoadImage(IDI_LANIM2,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
	m_pLocateAnimBitmaps[2]=(HICON)LoadImage(IDI_LANIM3,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
	m_pLocateAnimBitmaps[3]=(HICON)LoadImage(IDI_LANIM4,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
	m_pLocateAnimBitmaps[4]=(HICON)LoadImage(IDI_LANIM5,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
	m_pLocateAnimBitmaps[5]=(HICON)LoadImage(IDI_LANIM6,IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
	
	SetTimer(ID_LOCATEANIM,200);
	m_nCurLocateAnimBitmap=0;
	m_pStatusCtrl->SetText((LPCSTR)m_pLocateAnimBitmaps[0],STATUSBAR_LOCATEICON,SBT_OWNERDRAW);
	LeaveCriticalSection(&m_csAnimBitmaps);
	return TRUE;
}

BOOL CLocateDlg::StopLocateAnimation()
{
	EnterCriticalSection(&m_csAnimBitmaps);
	if (m_pLocateAnimBitmaps!=NULL)
	{
		KillTimer(ID_LOCATEANIM);
		for (int i=0;i<6;i++)
			DestroyIcon(m_pLocateAnimBitmaps[i]);
		delete[] m_pLocateAnimBitmaps;
		m_pLocateAnimBitmaps=NULL;
		LeaveCriticalSection(&m_csAnimBitmaps);
		
		if (m_pStatusCtrl!=NULL)
			m_pStatusCtrl->SetText(STRNULL,STATUSBAR_LOCATEICON,SBT_OWNERDRAW);
		return TRUE;
	}
	LeaveCriticalSection(&m_csAnimBitmaps);
	return TRUE;
}
	
BOOL CLocateDlg::StartUpdateAnimation()
{
	if (m_pStatusCtrl==NULL)
		return FALSE;

	EnterCriticalSection(&m_csAnimBitmaps);
	if (m_pUpdateAnimBitmaps!=NULL)
	{
		LeaveCriticalSection(&m_csAnimBitmaps);
		return TRUE;
	}
	
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
	
	SetTimer(ID_UPDATEANIM,100);
	m_nCurUpdateAnimBitmap=0;
	m_pStatusCtrl->SetText((LPCSTR)m_pUpdateAnimBitmaps[0],STATUSBAR_UPDATEICON,SBT_OWNERDRAW);
	
	LeaveCriticalSection(&m_csAnimBitmaps);
	return TRUE;
}

BOOL CLocateDlg::StopUpdateAnimation()
{
	EnterCriticalSection(&m_csAnimBitmaps);
	if (m_pUpdateAnimBitmaps!=NULL)
	{
		KillTimer(ID_UPDATEANIM);
		for (int i=0;i<13;i++)
			DestroyIcon(m_pUpdateAnimBitmaps[i]);
		delete[] m_pUpdateAnimBitmaps;
		m_pUpdateAnimBitmaps=NULL;
		LeaveCriticalSection(&m_csAnimBitmaps);
		if (m_pStatusCtrl!=NULL)
			m_pStatusCtrl->SetText(STRNULL,STATUSBAR_UPDATEICON,SBT_OWNERDRAW);
	
		return TRUE;
	}
	LeaveCriticalSection(&m_csAnimBitmaps);
	return TRUE;
}

void CLocateDlg::OnSaveResults()
{
	CSaveResultsDlg SaveResultsDlg;
	
	// Loading previous state
	CRegKey2 RegKey;
	
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
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
	if(RegKey.OpenKey(HKCU,"\\General",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
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

		CString File;
		SaveResultsDlg.GetFilePath(File);
		if (SaveResultsDlg.GetFilterIndex()==2)
			Results.SaveToHtmlFile(File);
		else
			Results.SaveToFile(File);
	}
	catch (CFileException ex)
	{
		char szError[2000];
		ex.GetErrorMessage(szError,2000);
		MessageBox(szError,ID2A(IDS_ERROR),MB_ICONERROR|MB_OK);
	}
	catch (CException ex)
	{
		char szError[2000];
		ex.GetErrorMessage(szError,2000);
		MessageBox(szError,ID2A(IDS_ERROR),MB_ICONERROR|MB_OK);
	}
	catch (...)
	{
		char szError[2000];
		CException ex(CException::unknown,GetLastError());
		ex.GetErrorMessage(szError,2000);
		MessageBox(szError,ID2A(IDS_ERROR),MB_ICONERROR|MB_OK);
	}
}

void CLocateDlg::OnCopyPathToClipboard(BOOL bShortPath)
{
	CStringW Text;

	int nItems=0;
	int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);

		if (nItems>0)
			Text << L"\r\n";
		if (!bShortPath)
			Text<<pItem->GetPath();
		else
		{
			WCHAR szPath[MAX_PATH];
			if (FileSystem::GetShortPathName(pItem->GetPath(),szPath,MAX_PATH))
				Text<<szPath;
			else
				Text<<pItem->GetPath();
		}
        nItems++;
		nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
	}
  

	if (IsUnicodeSystem())
	{
		HGLOBAL hMem=GlobalAlloc(GHND,(Text.GetLength()+1)*2);
		if (hMem==NULL)
		{
			ShowErrorMessage(IDS_ERRORCANNOTALLOCATE);
			return;
		}
		
		BYTE* pData=(BYTE*)GlobalLock(hMem);
		MemCopyW((LPWSTR)pData,LPCWSTR(Text),Text.GetLength()+1);
		GlobalUnlock(hMem);

		if (OpenClipboard())
		{
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT,hMem);
			CloseClipboard();
		}
	}
	else
	{
		HGLOBAL hMem=GlobalAlloc(GHND,Text.GetLength()+1);
		if (hMem==NULL)
		{
			ShowErrorMessage(IDS_ERRORCANNOTALLOCATE);
			return;
		}

		BYTE* pData=(BYTE*)GlobalLock(hMem);
		WideCharToMultiByte(CP_ACP,0,LPCWSTR(Text),(int)Text.GetLength()+1,(LPSTR)pData,Text.GetLength()+1,NULL,NULL);
		GlobalUnlock(hMem);

		if (OpenClipboard())
		{
			EmptyClipboard();
			SetClipboardData(CF_TEXT,hMem);
			CloseClipboard();
		}

	}
}

void CLocateDlg::OnChangeFileName()
{
	CChangeFilenameDlg fnd;
	
	CRegKey2 RegKey;
	BOOL bNoExtension=FALSE;
	if (RegKey.OpenKey(HKCU,"\\Misc",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD dwTemp=0;;
		RegKey.QueryValue("NoExtensionInRename",dwTemp);
		if (dwTemp)
			bNoExtension=TRUE;
	}

	int iItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	while (iItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(iItem);
		if (pItem!=NULL)
		{
			fnd.m_sParent=pItem->GetParent();
			fnd.m_sFileName.Copy(pItem->GetName(),pItem->GetNameLen());
			
			if (!bNoExtension || pItem->IsFolder())
				fnd.m_dwFlags&=~CChangeFilenameDlg::fNoExtension;
			else
				fnd.m_dwFlags|=CChangeFilenameDlg::fNoExtension;

			if (fnd.DoModal(*this))
			{
				if (fnd.m_sFileName.Compare(pItem->GetName())!=0)
					pItem->ChangeName(this,fnd.m_sFileName,fnd.m_sFileName.GetLength());
				for (int iColumn=0;iColumn<m_pListCtrl->GetColumnCount();iColumn++)
					m_pListCtrl->SetItemText(iItem,iColumn,LPSTR_TEXTCALLBACKW);

				m_pListCtrl->Update(iItem);
				m_pListCtrl->RedrawItems(iItem,iItem);
			}
		}
		iItem=m_pListCtrl->GetNextItem(iItem,LVNI_SELECTED);
	}
	m_pListCtrl->UpdateWindow();
}

void CLocateDlg::OnChangeFileNameCase()
{
	if (m_pListCtrl->GetNextItem(-1,LVNI_SELECTED)==-1)
		return;

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
				CStringW sOldName(pItem->GetName());
				LPWSTR szName=pItem->GetName();

				if (!cd.bForExtension)
				{
					iLength=LastCharIndex(szName,L'.')+1;
					if (iLength==0)
						iLength=pItem->GetNameLen();
				}
				else
					iLength=pItem->GetNameLen();

				switch (cd.nSelectedCase)
				{
				case CChangeCaseDlg::Sentence:
					MakeLower(szName+1,iLength-1);
					MakeUpper(szName,1);
					break;
				case CChangeCaseDlg::Uppercase:
					MakeUpper(szName,iLength);
					break;
				case CChangeCaseDlg::Lowercase:
					MakeLower(szName,iLength);
					break;
				case CChangeCaseDlg::Title:
					for (int i=0;i<iLength;)
					{
						MakeUpper(szName+(i++),1);
						
						int nIndex=FirstCharIndex(szName+i,L' ');
						if (nIndex==-1)
						{
							MakeLower(szName+i,iLength-i);
							break;
						}
                        						
						MakeLower(szName+i,nIndex);
                        i+=nIndex+1;                        
					}
					break;
				case CChangeCaseDlg::Toggle:
					for (int i=0;i<iLength;i++)
					{
						// Todo: check these in Win9X
						if (IsCharUpper(szName[i]))
							MakeLower(szName+i,1);
						else if (IsCharLower(szName[i]))
							MakeUpper(szName+i,1);
					}
					break;
				}

				FileSystem::MoveFile(pItem->GetPath(),pItem->GetPath());
				pItem->RemoveFlags(LITEM_FILETITLEOK|LITEM_FILENAMEOK);
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

void CLocateDlg::OnComputeMD5Sums(BOOL bForSameSizeFilesOnly)
{
	CWaitCursor e;
	if (bForSameSizeFilesOnly)
	{
		UINT uSelected=m_pListCtrl->GetSelectedCount();
		if (uSelected<2)
			return; 
        
		CLocatedItem** pItems=new CLocatedItem*[uSelected];
		int* pItemsID=new int[uSelected];

		int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
		for (UINT i=0;i<uSelected;i++)
		{
			ASSERT(nItem!=-1);
			pItems[i]=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
			pItemsID[i]=nItem;
			nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
		}

		for (UINT i=0;i<uSelected;i++)
		{
			if (pItems[i]==NULL)
				continue;

			BOOL bFound=FALSE;

			for (UINT j=i+1;j<uSelected;j++)
			{
				if (pItems[j]==NULL)
					continue;

				if (pItems[i]->GetFileSizeHi()==pItems[j]->GetFileSizeHi() &&
					pItems[i]->GetFileSizeLo()==pItems[j]->GetFileSizeLo())
				{
					bFound=TRUE;
					pItems[j]->ComputeMD5sum(TRUE);
					m_pListCtrl->RedrawItems(pItemsID[j],pItemsID[j]);
					pItems[j]=NULL;
				}
			}

			if (bFound)
			{
				pItems[i]->ComputeMD5sum(TRUE);
				m_pListCtrl->RedrawItems(pItemsID[i],pItemsID[i]);
				pItems[i]=NULL;
			}
		}

		delete[] pItems;
		delete[] pItemsID;
	}
	else
	{
		int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
		while (nItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
			if (pItem!=NULL)
			{
				pItem->ComputeMD5sum(TRUE);
				m_pListCtrl->RedrawItems(nItem,nItem);
			}
			nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
		}
	
        
	}
}

void CLocateDlg::OnShowFileInformation()
{
	DWORD dwFiles=0,dwDirectories=0;
	LONGLONG llTotalSize=0;
	int nItem=m_pListCtrl->GetNextItem(-1,LVNI_SELECTED);
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)m_pListCtrl->GetItemData(nItem);
		if (pItem!=NULL)
		{
			if (pItem->IsFolder())
				dwDirectories++;
			else
			{
				dwFiles++;
				llTotalSize+=pItem->GetFileSize();
			}
		}

		nItem=m_pListCtrl->GetNextItem(nItem,LVNI_SELECTED);
	}

	if (dwFiles>0 || dwDirectories>0)
	{
		CString str;
		char number[200],*numberfmt=NULL;

		_ui64toa_s(llTotalSize,number,200,10);
		CRegKey RegKey;
		if (RegKey.OpenKey(HKCU,"Control Panel\\International",CRegKey::defRead)==ERROR_SUCCESS)
		{
			numberfmt=new char[200];
			char szTemp[10]=".",szTemp2[10]=" ";

			NUMBERFMT fmt;
			
			// Defaults;
			fmt.NumDigits=0; 
			fmt.LeadingZero=1;
			fmt.Grouping=3; 
			fmt.lpDecimalSep=szTemp; 
			fmt.lpThousandSep=szTemp2; 
			fmt.NegativeOrder=1; 
			
			if (RegKey.QueryValue("iLZero",szTemp,10)>1)
				fmt.LeadingZero=atoi(szTemp);
			if (RegKey.QueryValue("sGrouping",szTemp,10)>1)
				fmt.Grouping=atoi(szTemp);
			RegKey.QueryValue("sDecimal",szTemp,10);
			RegKey.QueryValue("sThousand",szTemp2,10);

			
			if (GetNumberFormat(LOCALE_USER_DEFAULT,0,number,&fmt,numberfmt,200)==0)
			{
				numberfmt=NULL;
				delete[] numberfmt;
			}
		}

		if (numberfmt!=NULL)
		{
			str.FormatEx(IDS_FILEINFORMATIONFMT,dwFiles,dwDirectories,numberfmt);
			delete[] numberfmt;
		}
		else
			str.FormatEx(IDS_FILEINFORMATIONFMT,dwFiles,dwDirectories,number);
		MessageBox(str,ID2A(IDS_FILEINFORMATION),MB_OK|MB_ICONINFORMATION);
	}
}

void CLocateDlg::OnSelectDetails()
{
	CSelectColumnsDlg dlg;
	

	int nColumns=m_pListCtrl->GetColumnCount();
	dlg.m_aSelectedCols.SetSize(m_pListCtrl->GetVisibleColumnCount());
	dlg.m_aWidths.SetSize(nColumns);
	dlg.m_aIDs.SetSize(nColumns);
	dlg.m_aAligns.SetSize(nColumns);
	dlg.m_aActions.SetSize(nColumns);
	
	m_pListCtrl->GetColumnOrderArray(m_pListCtrl->GetVisibleColumnCount(),
		dlg.m_aSelectedCols.GetData());

	

	LVCOLUMN lc;int iCol;
	lc.mask=LVCF_FMT|LVCF_WIDTH;
	for (iCol=0;iCol<nColumns;iCol++)
	{
		// Set column id
		dlg.m_aIDs[iCol]=m_pListCtrl->GetColumnID(iCol);
		m_pListCtrl->GetColumn(iCol,&lc);

		// Set column width
		dlg.m_aWidths[iCol]=lc.cx;

        // Set align
		dlg.m_aAligns[iCol]=(CSelectColumnsDlg::ColumnItem::Align)(lc.fmt&LVCFMT_JUSTIFYMASK);

		dlg.m_aActions[iCol]=new CSubAction*[ListActionCount];
		ZeroMemory(dlg.m_aActions[iCol],ListActionCount*sizeof(CSubAction*));
			
		if (dlg.m_aIDs[iCol]>LastType)
			continue;
		
		for (int nAct=0;nAct<ListActionCount;nAct++)
		{
			if (m_aResultListActions[dlg.m_aIDs[iCol]][nAct]!=NULL)
			{
				dlg.m_aActions[iCol][nAct]=new CSubAction(
					CAction::ResultListItems,*m_aResultListActions[dlg.m_aIDs[iCol]][nAct]);
			}
		}
        
	}

	if (!dlg.DoModal(*this))
		return; // Cancel
	
	m_pListCtrl->SetColumnOrderArray(dlg.m_aSelectedCols.GetSize(),
		dlg.m_aSelectedCols.GetData());

	ClearResultlistActions();

	for (iCol=0;iCol<nColumns;iCol++)
	{
		lc.mask=LVCF_FMT;
		m_pListCtrl->GetColumn(iCol,&lc);
			
		lc.mask=LVCF_FMT|LVCF_WIDTH;
		// Set column width
		lc.cx=dlg.m_aWidths[iCol];
        // Set align
		lc.fmt&=~LVCFMT_JUSTIFYMASK;
		lc.fmt|=dlg.m_aAligns[iCol];

		
		m_pListCtrl->SetColumn(iCol,&lc);

		if (dlg.m_aActions[iCol]==NULL)
			continue;

		for (int nAct=0;nAct<ListActionCount;nAct++)
		{
			
			if (dlg.m_aActions[iCol][nAct]!=NULL)
				m_aResultListActions[dlg.m_aIDs[iCol]][nAct]=dlg.m_aActions[iCol][nAct];
		}
		delete[] dlg.m_aActions[iCol];
	}

}

void CLocateDlg::SetStartData(const CLocateApp::CStartData* pStartData)
{
	// Loading preset first
	if (pStartData->m_pLoadPreset!=NULL)
		LoadPreset(pStartData->m_pLoadPreset);

    // Set fields	
	m_NameDlg.SetStartData(pStartData);
	m_SizeDateDlg.SetStartData(pStartData);
	m_AdvancedDlg.SetStartData(pStartData);

	// Set sorting
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
			SetSorting(pStartData->m_nSorting);
	}

	if (pStartData->m_dwMaxFoundFiles!=DWORD(-1))
		m_dwMaxFoundFiles=pStartData->m_dwMaxFoundFiles;
	
	if (pStartData->m_nStatus&CLocateApp::CStartData::statusRunAtStartUp)
		OnOk(FALSE,FALSE);
}



BOOL CLocateDlg::CNameDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);

	m_Name.AssignToDlgItem(*this,IDC_NAME);
	m_Type.AssignToDlgItem(*this,IDC_TYPE);
	m_LookIn.AssignToDlgItem(*this,IDC_LOOKIN);
		
	LoadRegistry();
	//InitDriveBox(TRUE);

	RECT rc1,rc2;
	GetWindowRect(&rc1);
	m_Name.GetWindowRect(&rc2);
	m_nFieldLeft=WORD(rc2.left-rc1.left);
	
	::GetWindowRect(GetDlgItem(IDC_BROWSE),&rc2);
	m_nButtonWidth=WORD(rc2.right-rc2.left);
	m_nBrowseTop=WORD(rc2.top-rc1.top);

	::GetWindowRect(GetDlgItem(IDC_NOSUBDIRECTORIES),&rc2);
	m_nCheckWidth=WORD(rc2.right-rc2.left);
	

	::GetWindowRect(GetDlgItem(IDC_MOREDIRECTORIES),&rc2);	
	m_nMoreDirsTop=WORD(rc2.top-rc1.top);
	m_nMoreDirsWidth=BYTE(rc2.right-rc2.left);



	return FALSE;
}

void CLocateDlg::CNameDlg::ChangeNumberOfItemsInLists(int iNumberOfNames,int iNumberOfTypes,int iNumberOfDirectories)
{
	if (iNumberOfNames<=0)
	{
		m_Name.ResetContent();
		m_nMaxNamesInList=0;
	}
	else if (iNumberOfNames!=m_nMaxNamesInList)
	{
		while (m_Name.GetCount()>iNumberOfNames)
			m_Name.DeleteString(iNumberOfNames);
		m_nMaxNamesInList=iNumberOfNames;
	}

	if (iNumberOfTypes<=0)
	{
		m_Type.ResetContent();
		m_Type.AddString(ID2A(IDS_NOEXTENSION));
		m_nMaxTypesInList=0;
	}
	else if (iNumberOfTypes!=m_nMaxTypesInList)
	{
		while (m_Type.GetCount()>iNumberOfTypes+1)
			m_Type.DeleteString(iNumberOfTypes+1);
		m_nMaxTypesInList=iNumberOfTypes;
	}

	EnterCriticalSection(&m_cBrowse);
	if (iNumberOfDirectories<=0)
	{
		if (m_pBrowse!=NULL)
		{
			for (int i=0;i<m_LookIn.GetCount();i++)
			{
				LPARAM lParam=m_LookIn.GetItemData(i);
				if (LOWORD(lParam)==Custom)
				{
					m_LookIn.DeleteItem(i);
					i--;
				}
			}
			
			for (int i=0;i<int(m_nMaxBrowse);i++)
				m_pBrowse[i].Empty();
			delete[] m_pBrowse;
			m_pBrowse=NULL;
		}
		m_nMaxBrowse=0;
	}
	else if (iNumberOfDirectories!=m_nMaxBrowse)
	{
		CStringW* pBrowseNew=new CStringW[iNumberOfDirectories];
		int i;
		for (i=0;i<iNumberOfDirectories && i<int(m_nMaxBrowse);i++)
			pBrowseNew[i].Swap(m_pBrowse[i]);
		
		if (iNumberOfDirectories<int(m_nMaxBrowse))
		{
			for (;i<int(m_nMaxBrowse);i++)
				m_pBrowse[i].Empty();

			// Removing items from combobox
			for (i=0;i<m_LookIn.GetCount();i++)
			{
				LPARAM lParam=m_LookIn.GetItemData(i);
				if (HIWORD(lParam)>=iNumberOfDirectories && LOWORD(lParam)==Custom)
				{
					m_LookIn.DeleteItem(i);
					i--;
				}
			}
		}
		delete[] m_pBrowse;
		m_pBrowse=pBrowseNew;
		m_nMaxBrowse=iNumberOfDirectories;
	}
	LeaveCriticalSection(&m_cBrowse);

}

BOOL CLocateDlg::CNameDlg::InitDriveBox(BYTE nFirstTime)
{
	//DebugFormatMessage("CNameDlg::InitDriveBox BEGIN, items in list %d",m_LookIn.GetCount());

	// Handle to locate dialog
	CLocateDlg* pLocateDlg=GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate;

	COMBOBOXEXITEMW ci;
	LPITEMIDLIST idl;
	SHFILEINFOW fi;
	CRegKey RegKey;

	// Buffers
	CStringW temp;
	WCHAR szBuf[]=L"X:\\";
	WCHAR Buffer[100];
	
	
	LPARAM lParam;
	int nSelection=-1;
	
	
	if (!nFirstTime)
	{
		ci.mask=CBEIF_LPARAM;
		ci.iItem=m_LookIn.GetCurSel();
		if (ci.iItem!=CB_ERR)
		{
			m_LookIn.GetItem(&ci);
			lParam=ci.lParam;
		}
		else
			lParam=LPARAM(-1);
		m_LookIn.ResetContent();
	}
	else
		lParam=MAKELPARAM(Everywhere,0);

	

	m_LookIn.SetImageList((HIMAGELIST)GetFileInfo(szwEmpty,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON));

	// Everywhere, icon from Network Neighborhood
	SHGetSpecialFolderLocation(*this,CSIDL_NETWORK,&idl);
	GetFileInfo(idl,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
	ci.iImage=fi.iIcon;
	GetFileInfo(idl,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
	ci.iSelectedImage=fi.iIcon;
	LoadString(IDS_EVERYWERE,Buffer,100);
	ci.pszText=Buffer;
	ci.iItem=0;
	ci.iIndent=0;
	ci.lParam=MAKELPARAM(Everywhere,Original);
	if (lParam==ci.lParam || nSelection==-1)
		nSelection=(int)ci.iItem;
	ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
	m_LookIn.InsertItem(&ci);
	CoTaskMemFree(idl);
	

	// Document folders
	SHGetSpecialFolderLocation(*this,CSIDL_PERSONAL,&idl);
	GetFileInfo(idl,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
	ci.iImage=fi.iIcon;
	GetFileInfo(idl,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
	ci.iSelectedImage=fi.iIcon;
	LoadString(IDS_DOCUMENTFOLDERS,Buffer,80);
	ci.pszText=Buffer;
	ci.iItem++;
	ci.iIndent=0;
	ci.lParam=MAKELPARAM(Special,Documents);
	if (lParam==ci.lParam)
		nSelection=(int)ci.iItem;
	ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
	m_LookIn.InsertItem(&ci);
	CoTaskMemFree(idl);
	
	// Desktop
	SHGetSpecialFolderLocation(*this,CSIDL_DESKTOP,&idl);
	GetFileInfo(idl,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
	ci.iImage=fi.iIcon;
	GetFileInfo(idl,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
	ci.iSelectedImage=fi.iIcon;
	GetFileInfo(idl,0,&fi,SHGFI_DISPLAYNAME);
	ci.pszText=fi.szDisplayName;
	ci.iItem++;
	ci.iIndent=1;
	ci.lParam=MAKELPARAM(Special,Desktop);
	if (lParam==ci.lParam)
		nSelection=(int)ci.iItem;
	ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
	m_LookIn.InsertItem(&ci);
	CoTaskMemFree(idl);
	
	// My Documents
	temp.Empty();
	if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue(L"Personal",temp);
		RegKey.CloseKey();
	}	
	if	(FileSystem::IsDirectory(temp))
	{
		GetFileInfo(temp,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
		ci.iImage=fi.iIcon;
		GetFileInfo(temp,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
		ci.iSelectedImage=fi.iIcon;
		GetFileInfo(temp,0,&fi,SHGFI_DISPLAYNAME);
		ci.pszText=fi.szDisplayName;
		ci.iItem++;
		ci.iIndent=1;
		ci.lParam=MAKELPARAM(Special,Personal);
		if (lParam==ci.lParam)
			nSelection=(int)ci.iItem;
		ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
		m_LookIn.InsertItem(&ci);
	}
	
	// My Computer
	SHGetSpecialFolderLocation(*this,CSIDL_DRIVES,&idl);
	GetFileInfo(idl,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
	ci.iImage=fi.iIcon;
	GetFileInfo(idl,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
	ci.iSelectedImage=fi.iIcon;
	GetFileInfo(idl,0,&fi,SHGFI_DISPLAYNAME);
	ci.pszText=fi.szDisplayName;
	ci.iItem++;
	ci.iIndent=0;
	ci.lParam=MAKELPARAM(Special,MyComputer);
	if (lParam==ci.lParam)
		nSelection=(int)ci.iItem;
	ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
	m_LookIn.InsertItem(&ci);
	CoTaskMemFree(idl);
	
	// Drives
	CArrayFAP<LPWSTR> aRoots;
	CDatabaseInfo::GetRootsFromDatabases(aRoots,GetLocateApp()->GetDatabases());
	
	for (int j=0;j<aRoots.GetSize();j++)
	{
		BOOL bAdd=TRUE;
		if (aRoots.GetAt(j)[0]=='\\' && aRoots.GetAt(j)[1]=='\\')
			continue;

		szBuf[0]=aRoots.GetAt(j)[0];
		MakeUpper(szBuf);
		
		if (FileSystem::GetDriveType(szBuf)<2)
			continue;
		
		// Checking whether drive exists
		for (int k=0;k<j;k++)
		{
			WCHAR cDrive=aRoots.GetAt(k)[0];
			MakeUpper(&cDrive,1);

			if (cDrive==szBuf[0])
			{
				bAdd=FALSE;
				break;
			}
		}
		if (!bAdd)
			continue;

		if (GetLocateDlg()->GetExtraFlags()&efNameDontResolveIconAndDisplayNameForDrives)
		{
			// Icon from system drive
			WCHAR szWindowsDir[MAX_PATH]=L"C:\\";
			FileSystem::GetWindowsDirectory(szWindowsDir,MAX_PATH);
			if (szWindowsDir[1]==L':' && szWindowsDir[2]==L'\\')
				szWindowsDir[3]='\0';
			GetFileInfo(szWindowsDir,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
			ci.iSelectedImage=fi.iIcon;
			GetFileInfo(szWindowsDir,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
			ci.iImage=fi.iIcon;
			
			fi.szDisplayName[0]='\0';
			FileSystem::GetVolumeInformation(szBuf,fi.szDisplayName,MAX_PATH-7,NULL,
				NULL,NULL,NULL,0);
			int nLen=istrlen(fi.szDisplayName);
			fi.szDisplayName[nLen++]=L' ';
			fi.szDisplayName[nLen++]=L'(';
			fi.szDisplayName[nLen++]=szBuf[0];
			fi.szDisplayName[nLen++]=L':';
			fi.szDisplayName[nLen++]=L')';
			fi.szDisplayName[nLen++]=L'\0';

		}
		else
		{
			GetFileInfo(szBuf,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
			ci.iSelectedImage=fi.iIcon;
			GetFileInfo(szBuf,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_DISPLAYNAME);
			ci.iImage=fi.iIcon;
		}

		ci.pszText=fi.szDisplayName;
		ci.iItem++;
		ci.iIndent=1;
		ci.lParam=MAKELPARAM(Drive,szBuf[0]);
		if (lParam==ci.lParam)
			nSelection=(int)ci.iItem;
		ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
		m_LookIn.InsertItem(&ci);

	}

	
	if ((pLocateDlg->GetFlags()&CLocateDlg::fgNameRootFlag)!=CLocateDlg::fgNameDontAddRoots)
	{
		if ((pLocateDlg->GetFlags()&CLocateDlg::fgNameRootFlag)==CLocateDlg::fgNameAddEnabledRoots)
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
			for (ci.iItem=m_LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
			{
				m_LookIn.GetItem(&ci);
				if (static_cast<TypeOfItem>(LOWORD(ci.lParam))==Drive)
					aCurrentlyAddedDrives.Add((char)HIWORD(ci.lParam));
			}
			// Make drives upper
			MakeUpper(aCurrentlyAddedDrives.GetData(),aCurrentlyAddedDrives.GetSize());
		}
		
		for (int i=0;i<aRoots.GetSize();)
		{
			LPCWSTR pRoot=aRoots[i];
			if (pRoot[1]==L':' && pRoot[2]==L'\0')
			{
				WCHAR cDrive=pRoot[0];
				MakeUpper(&cDrive,1);
				
				// Checking whether drive is already added
				int j;
				for (j=0;j<aCurrentlyAddedDrives.GetSize() && cDrive!=aCurrentlyAddedDrives[j];j++);
				
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
			EnterCriticalSection(&m_cBrowse);
			
			SHGetSpecialFolderLocation(*this,CSIDL_NETWORK,&idl);
			GetFileInfo(idl,0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
			ci.iImage=fi.iIcon;
			GetFileInfo(m_pBrowse!=NULL?m_pBrowse[0]:L"C:\\",0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
			ci.iSelectedImage=fi.iIcon;
			LoadString(IDS_ROOTS,Buffer,100);
			ci.pszText=Buffer;
			ci.iItem++;
			ci.iIndent=0;
			ci.lParam=MAKELPARAM(Everywhere,RootTitle);
			if (lParam==ci.lParam)
				nSelection=(int)ci.iItem;
			ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
			m_LookIn.InsertItem(&ci);
			CoTaskMemFree(idl);

			LeaveCriticalSection(&m_cBrowse);

			for (int i=0;i<aRoots.GetSize();i++)
			{
				if (GetLocateApp()->GetProgramFlags()&CLocateApp::pfUseDefaultIconForDirectories)
				{
					ci.iImage=DIR_IMAGE;
					ci.iSelectedImage=DIR_IMAGE;
				}
				else
				{
					GetFileInfo(aRoots[i],0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
					ci.iImage=fi.iIcon;
					GetFileInfo(aRoots[i],0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
					ci.iSelectedImage=fi.iIcon;
				}
				ci.pszText=aRoots[i];
				ci.iItem++;
				ci.iIndent=1;
				
				// Computing cheksum from directory
				// which is used to save control state
				ci.lParam=MAKELPARAM(Root,ComputeChecksumFromDir(aRoots[i]));
					
				if (lParam==ci.lParam)
					nSelection=(int)ci.iItem;
				ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
				m_LookIn.InsertItem(&ci);

				//DebugFormatMessage("DriveList: root %S added, i=%d",aRoots.GetAt(i),i);

			}

			
		}
	}

	EnterCriticalSection(&m_cBrowse);
	if (m_pBrowse!=NULL) // NULL is possible is locate is just closing
	{
		// Remembered directories
		for (int j=0;j<int(m_nMaxBrowse);j++)
		{
			if (m_pBrowse[j].IsEmpty())
				break;

			if (GetLocateApp()->GetProgramFlags()&CLocateApp::pfUseDefaultIconForDirectories)
			{
				ci.iImage=DIR_IMAGE;
				ci.iSelectedImage=DIR_IMAGE;
			}
			else
			{
				GetFileInfo(m_pBrowse[j],0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
				ci.iImage=fi.iIcon;
				GetFileInfo(m_pBrowse[j],0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
				ci.iSelectedImage=fi.iIcon;
			}
			ci.pszText=m_pBrowse[j].GetBuffer();
			ci.iItem++;
			ci.iIndent=0;
			ci.lParam=MAKELPARAM(Custom,j);
			if (lParam==ci.lParam)
				nSelection=(int)ci.iItem;
			ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
			m_LookIn.InsertItem(&ci);

			//DebugFormatMessage("DriveList: directory %s added",m_pBrowse[j].GetBuffer());
		}
	}
	LeaveCriticalSection(&m_cBrowse);

	if (lParam!=LPARAM(-1))
	{
		m_LookIn.SetCurSel(nSelection);
		m_LookIn.SetItemText(-1,m_LookIn.GetItemTextW(nSelection));
	}

	//DebugMessage("CNameDlg::InitDriveBox END");
	return TRUE;
}
	
BOOL CLocateDlg::CNameDlg::SelectByLParam(LPARAM lParam)
{
	COMBOBOXEXITEM ci;
	ci.mask=CBEIF_LPARAM;
	
	for (ci.iItem=m_LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
	{
		m_LookIn.GetItem(&ci);
		if (ci.lParam==lParam)
		{
			m_LookIn.SetCurSel((int)ci.iItem);
			m_LookIn.SetItemText(-1,m_LookIn.GetItemTextW((int)ci.iItem));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CLocateDlg::CNameDlg::SelectByRootName(LPCWSTR szDirectory)
{
	COMBOBOXEXITEMW ci;
	ci.mask=CBEIF_LPARAM;
	
	for (ci.iItem=m_LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
	{
		m_LookIn.GetItem(&ci);
		if (static_cast<TypeOfItem>(LOWORD(ci.lParam))==Root)
		{
			WCHAR szPath[MAX_PATH+10];
			ci.pszText=szPath;
			ci.cchTextMax=MAX_PATH+10;
			ci.mask=CBEIF_TEXT;
			m_LookIn.GetItem(&ci);

			if (strcasecmp(ci.pszText,szDirectory)==0)
			{
				m_LookIn.SetCurSel((int)ci.iItem);
				m_LookIn.SetItemText(-1,szPath);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CLocateDlg::CNameDlg::SelectByCustomName(LPCWSTR szDirectory)
{
	COMBOBOXEXITEM ci;
	ci.mask=CBEIF_LPARAM;
	
	for (ci.iItem=m_LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
	{
		m_LookIn.GetItem(&ci);
		if (static_cast<TypeOfItem>(LOWORD(ci.lParam))==Custom)
		{
			EnterCriticalSection(&m_cBrowse);
			if (m_pBrowse!=NULL && HIWORD(ci.lParam)<m_nMaxBrowse)
			{
				if (m_pBrowse[HIWORD(ci.lParam)].CompareNoCase(szDirectory)==0)
				{
					m_LookIn.SetCurSel((int)ci.iItem);
					m_LookIn.SetItemText(-1,m_pBrowse[HIWORD(ci.lParam)]);
					LeaveCriticalSection(&m_cBrowse);
					return TRUE;
				}
			}
			LeaveCriticalSection(&m_cBrowse);
		}
	}

	m_LookIn.SetCurSel(-1);
	m_LookIn.SetItemText(-1,szDirectory);
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
			break;
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
				m_Name.SetFocus();
			else
				HilightTab(IsChanged());
			break;
		case CBN_SETFOCUS:
			m_Name.SetEditSel(0,-1);
			break;
		case CBN_EDITCHANGE:
			HilightTab(m_Name.GetTextLength()>0);
			break;
		}
		break;
	case IDC_TYPE:
		switch (wNotifyCode)
		{
		case 1: /* ACCEL or CBN_SELCHANGE */
			if (hControl==NULL)
				m_Type.SetFocus();
			else
			{
				GetLocateDlg()->m_AdvancedDlg.ChangeEnableStateForCheck();
				HilightTab(IsChanged());
			}
			break;
		case CBN_SETFOCUS:
			m_Type.SetEditSel(0,-1);
			break;
		case CBN_EDITCHANGE:
			GetLocateDlg()->m_AdvancedDlg.ChangeEnableStateForCheck();
			HilightTab(IsChanged());
			break;
		}
		break;
	case IDC_LOOKIN:
		switch (wNotifyCode)
		{
		case 1: /* ACCEL or CBN_SELCHANGE */
			if (hControl==NULL)
				m_LookIn.SetFocus();
			else
				HilightTab(IsChanged());
			break;
		case CBN_EDITCHANGE:
			HilightTab(IsChanged());
			break;
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
	case IDC_NOSUBDIRECTORIES:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
		{
			CheckDlgButton(wID,!IsDlgButtonChecked(wID));
			SetFocus(wID);
		}
		HilightTab(IsChanged());
		break;
	}
	return FALSE;
}

BOOL CLocateDlg::CNameDlg::IsChanged()
{
	int nSel=m_Name.GetCurSel();
	if (nSel==CB_ERR)
	{
		if (m_Name.GetTextLength()>0)
			return TRUE;
	}
	else
	{
		if (m_Name.GetLBTextLen(nSel)>0)
			return TRUE;
	}

	if (m_Type.IsWindowEnabled())
	{
		nSel=m_Type.GetCurSel();
		if (nSel==CB_ERR)
		{
			if (m_Type.GetTextLength()>0)
				return TRUE;
		}
		else
		{
			if (m_Type.GetLBTextLen(nSel)>0)
				return TRUE;
		}
	}

	if (IsDlgButtonChecked(IDC_NOSUBDIRECTORIES))
		return TRUE;

	if (m_pMultiDirs!=NULL)
	{
		if (m_pMultiDirs[1]!=NULL)
			return TRUE;
	}
	
	int nCurSel=m_LookIn.GetCurSel();
	if (nCurSel==CB_ERR)
		return TRUE; // Type directory

	return m_LookIn.GetItemData(nCurSel)!=MAKELPARAM(Everywhere,Original);
}
		
void CLocateDlg::HilightTab(int nTab,int nID,BOOL bHilight)
{
	TC_ITEMW ti;
	WCHAR szText[80];
	ti.mask=TCIF_TEXT;
	ti.cchTextMax=80;
	ti.pszText=szText;
		
	if (!m_pTabCtrl->GetItem(nTab,&ti))
		return;
	
	int nLen=istrlenw(szText);
	if (nLen==0)
		nLen=LoadString(nID,szText,80);

	if (bHilight && szText[nLen-1]!=L'*')
	{
		szText[nLen]=L'*';
		szText[++nLen]=L'\0';
		m_pTabCtrl->SetItem(nTab,&ti);
	}
	else if (!bHilight && szText[nLen-1]==L'*')
	{
		szText[nLen-1]=L'\0';
		m_pTabCtrl->SetItem(nTab,&ti);
	}
}


CLocateDlg::CNameDlg::~CNameDlg()
{
	DeleteCriticalSection(&m_cBrowse);
}

void CLocateDlg::CNameDlg::OnDestroy()
{
	SaveRegistry();	
	CDialog::OnDestroy();

	EnterCriticalSection(&m_cBrowse);
	if (m_pBrowse!=NULL)
	{
		for (DWORD i=0;i<m_nMaxBrowse;i++)
			m_pBrowse[i].Empty();
		delete[] m_pBrowse;
		m_pBrowse=NULL;
	}
	LeaveCriticalSection(&m_cBrowse);
	
	if (m_pMultiDirs!=NULL)
	{
		for (DWORD i=0;m_pMultiDirs[i]!=NULL;i++)
			delete m_pMultiDirs[i];
		delete[] m_pMultiDirs;
		m_pMultiDirs=NULL;
	}

}


BOOL CLocateDlg::CNameDlg::OnOk(CStringW& sName,CArray<LPWSTR>& aExtensions,CArrayFAP<LPWSTR>& aDirectories)
{
	DlgDebugMessage("CLocateDlg::CNameDlg::OnOk BEGIN");
	
	
	// Setting recent combobox for name
	CStringW Buffer;
	m_Name.GetText(sName);
	for (int i=m_Name.GetCount()-1;i>=0;i--)
	{
		m_Name.GetLBText(i,Buffer);
		if (sName.CompareNoCase(Buffer)==0)
			m_Name.DeleteString(i);
	}	
	m_Name.InsertString(0,sName);
	m_Name.SetText(sName);

	if (m_Name.GetCount()>int(m_nMaxNamesInList))
		m_Name.DeleteString(m_nMaxNamesInList);

	
	// Setting recent combobox for type
	if (!GetLocateDlg()->m_AdvancedDlg.IsDlgButtonChecked(IDC_USEWHOLEPATH))
	{
		CStringW sType;
		m_Type.GetText(sType);
		

		if (m_Type.GetCurSel()==0) // Empty extension
			aExtensions.Add(allocemptyW());
		else
		{
			for (int i=m_Type.GetCount()-1;i>=1;i--)
			{
				m_Type.GetLBText(i,Buffer);
				if (sType.CompareNoCase(Buffer)==0)
					m_Type.DeleteString(i);
			}	

			m_Type.InsertString(1,sType); // 0 == (none)
			m_Type.SetText(sType);
			if (m_Type.GetCount()>int(m_nMaxTypesInList)+1)
				m_Type.DeleteString(m_nMaxTypesInList+1);

			// Parsing extensions
			LPCWSTR pType=sType;
			for (;pType[0]==L' ';pType++);
			while (*pType!=L'\0')
			{
				DWORD nLength;
				for (nLength=0;pType[nLength]!=L'\0' && pType[nLength]!=L' ';nLength++);		
				aExtensions.Add(alloccopy(pType,nLength));
				pType+=nLength;
				for (;pType[0]==L' ';pType++);
			}
		}
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


	// Saves searches
	SaveRegistry();
	
	DlgDebugMessage("CLocateDlg::CNameDlg::OnOk END");
	return TRUE;
}

DWORD CLocateDlg::CNameDlg::GetCurrentlySelectedComboItem() const
{
	DWORD nCurSel=m_LookIn.GetCurSel();
	
	if (nCurSel!=DWORD(CB_ERR))
	{
		WCHAR szTmp1[MAX_PATH],szTmp2[MAX_PATH];
		m_LookIn.GetItemText(nCurSel,szTmp1,MAX_PATH);
		m_LookIn.GetItemText(-1,szTmp2,MAX_PATH);
		
		if (wcscmp(szTmp1,szTmp2)!=0)
			nCurSel=DWORD(CB_ERR);
	}	

	return nCurSel;
}

BOOL CLocateDlg::CNameDlg::GetDirectoriesForActiveSelection(CArray<LPWSTR>& aDirectories,TypeOfItem* pType,BOOL bNoWarningIfNotExists)
{
	//DebugMessage("CNameDlg::GetDirectoriesForActiveSelection BEGIN");
	
	DWORD nCurSel=GetCurrentlySelectedComboItem();
	
	if (nCurSel==DWORD(CB_ERR))
	{
		//DebugMessage("CNameDlg::GetDirectoriesForActiveSelection A");
	

		// Getting directory from combo
		DWORD dwBufferSize=MAX_PATH;
		WCHAR* pFolder;
		
		DWORD dwLength;
		for(;;)
		{
			pFolder=new WCHAR[dwBufferSize];
			
			if (!m_LookIn.GetItemText(-1,pFolder,dwBufferSize))
			{
				delete[] pFolder;
				return FALSE;
			}

			dwLength=istrlenw(pFolder);
			
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

		//DebugMessage("CNameDlg::GetDirectoriesForActiveSelection END 1");
		return bRet;
	}
	else
	{
		//DebugMessage("CNameDlg::GetDirectoriesForActiveSelection B");
			
		COMBOBOXEXITEMW ci;
		ci.mask=CBEIF_LPARAM;
		ci.iItem=nCurSel;
		m_LookIn.GetItem(&ci);

		if (pType!=NULL)
			*pType=static_cast<TypeOfItem>(LOWORD(ci.lParam));

		if (static_cast<TypeOfItem>(LOWORD(ci.lParam))==Root)
		{
			WCHAR szPath[MAX_PATH+10];
			ci.pszText=szPath;
			ci.cchTextMax=MAX_PATH+10;
			ci.mask=CBEIF_TEXT;
			m_LookIn.GetItem(&ci);
			AddDirectoryToList(aDirectories,szPath);
			return TRUE;
		}
		else
		{
			//DebugMessage("CNameDlg::GetDirectoriesForActiveSelection END 2");
			return GetDirectoriesFromLParam(aDirectories,ci.lParam);
		}
	}

	//DebugMessage("CNameDlg::GetDirectoriesForActiveSelection END 3");
}

BOOL CLocateDlg::CNameDlg::GetDirectoriesFromCustomText(CArray<LPWSTR>& aDirectories,LPCWSTR szCustomText,DWORD dwLength,BOOL bCurrentSelection,BOOL bNoWarning)
{
	// Removing spaces and \\ from beginning and end
	LPCWSTR pPtr=szCustomText;
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

BOOL CLocateDlg::CNameDlg::GetDirectoriesFromLParam(CArray<LPWSTR>& aDirectories,LPARAM lParam)
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
						WCHAR* szDir=new WCHAR[nLength];
						RegKey.QueryValue(L"Desktop",szDir,nLength);
						while (szDir[nLength-2]==L'\\')
						{
							szDir[nLength-2]='\0';
							nLength--;
						}
						AddDirectoryToList(aDirectories,szDir);
						
						nLength=RegKey.QueryValueLength(L"Personal");
						szDir=new WCHAR[nLength];
						RegKey.QueryValue(L"Personal",szDir,nLength);
						while (szDir[nLength-2]==L'\\')
						{
							szDir[nLength-2]='\0';
							nLength--;
						}
						AddDirectoryToList(aDirectories,szDir);
						
					}
					break;
				}
			case Desktop:
				{
					CRegKey RegKey;
					if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
					{
						DWORD nLength=RegKey.QueryValueLength("Desktop");
						WCHAR* szDir=new WCHAR[nLength+1];
						RegKey.QueryValue(L"Desktop",szDir,nLength);
						while (szDir[nLength-2]==L'\\')
						{
							szDir[nLength-2]=L'\0';
							nLength--;
						}
						AddDirectoryToList(aDirectories,szDir);
					}
					break;
				}
			case Personal:
				{
					CRegKey RegKey;
					if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
					{
						DWORD nLength=RegKey.QueryValueLength("Personal");
						WCHAR* szDir=new WCHAR[nLength+1];
						RegKey.QueryValue(L"Personal",szDir,nLength);
						while (szDir[nLength-2]==L'\\')
						{
							szDir[nLength-2]=L'\0';
							nLength--;
						}
						AddDirectoryToList(aDirectories,szDir,nLength);
					}
					break;
				}
			case MyComputer:
				{
					DWORD nLength=GetLogicalDriveStrings(0,NULL)+1;
					if (nLength>=2)
					{
						WCHAR* szDrives=new WCHAR[nLength+1];
						FileSystem::GetLogicalDriveStrings(nLength,szDrives);
						for (LPWSTR szDrive=szDrives;szDrive[0]!=L'\0';szDrive+=4)
						{
							if (FileSystem::GetDriveType(szDrive)!=DRIVE_REMOTE)
								AddDirectoryToList(aDirectories,szDrive,2);
						}
						delete[] szDrives;
					}
					break;
				}
			}
			break;
		}
	case Drive:
		{
			WCHAR* szDir=new WCHAR[3];
			szDir[0]=static_cast<WCHAR>(HIWORD(lParam));
			szDir[1]=':';
			szDir[2]=L'\0';
			AddDirectoryToList(aDirectories,szDir);
			break;
		}
	case Custom:
		EnterCriticalSection(&m_cBrowse);	
		if (m_pBrowse!=NULL && HIWORD(lParam)<m_nMaxBrowse)
			AddDirectoryToList(aDirectories,(LPCWSTR)m_pBrowse[HIWORD(lParam)],m_pBrowse[HIWORD(lParam)].GetLength());
		LeaveCriticalSection(&m_cBrowse);
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



void CLocateDlg::CNameDlg::AddDirectoryToList(CArray<LPWSTR>& aDirectories,LPCWSTR szDirectory)
{
	CStringW sDirectory;
	int nLength;


	// Multiple directories
	LPCWSTR pPtr=szDirectory;
	while (*pPtr==';' || *pPtr==' ')
		pPtr++;

	while (*pPtr!='\0')
	{
		// Check how long first string is
		if (pPtr[0]=='\"')
		{
			nLength=FirstCharIndex(pPtr+1,L'\"');
			sDirectory.Copy(pPtr+1,nLength);

			if (nLength!=-1)
			{
				for (nLength++;pPtr[nLength]!=';' && pPtr[nLength]!='\0';nLength++);
			}
			else
				nLength=istrlenw(pPtr);
		}
		else
		{
			nLength=FirstCharIndex(pPtr,L';');
			if (nLength==-1)
				nLength=istrlenw(pPtr);
			sDirectory.Copy(pPtr,nLength);

			while (sDirectory.LastChar()==' ')
				sDirectory.DelLastChar();
		}


		BOOL bFound=FALSE;
		for (int i=0;i<aDirectories.GetSize();i++)
		{
			if (sDirectory.Compare(aDirectories[i])==0)
			{
				// Directory already in the list
				bFound=TRUE;
			}
		}

		if (!bFound)
			aDirectories.Add(sDirectory.GiveBuffer());
		
		pPtr+=nLength;

		while (*pPtr==';' || *pPtr==' ')
			pPtr++;
	}
}

void CLocateDlg::OnPresets()
{
	CMenu Menu;
	int nPresets=0;

	Menu.CreatePopupMenu();
	

	CLocateDlg::InsertMenuItemsFromTemplate(Menu,m_Menu.GetSubMenu(SUBMENU_PRESETSELECTION),0);
	
	
	CRegKey2 RegKey;

	if (RegKey.OpenKey(HKCU,"\\Dialogs\\SearchPresets",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		CRegKey PresetKey;
		WCHAR szBuffer[30];
		
		// Creating preset menu list
		MENUITEMINFOW mii;
		mii.cbSize=sizeof(MENUITEMINFOW);
		mii.wID=IDM_DEFMENUITEM;
		mii.fType=MFT_STRING;
		mii.fMask=MIIM_TYPE|MIIM_ID;
	

		for (int j=0;j<1000;j++)
		{
			StringCbPrintfW(szBuffer,30*sizeof(WCHAR),L"Preset %03d",j);

			if (PresetKey.OpenKey(RegKey,szBuffer,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
				break;

			CStringW sCurrentName;
			if (!PresetKey.QueryValue(WSTRNULL,sCurrentName))
				continue;

			
			// Preset found
			mii.dwTypeData=sCurrentName.GetBuffer();
			mii.wID=IDM_DEFMENUITEM+j;
			
			Menu.InsertMenu(nPresets,TRUE,&mii);

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
		Menu.InsertMenu(nPresets,TRUE,&mii);
	}
	else	
		Menu.EnableMenuItem(IDM_PRESETREMOVE,MF_BYCOMMAND|MF_GRAYED);

	RECT rcButton;
	::GetWindowRect(GetDlgItem(IDC_PRESETS),&rcButton);
		
	SetForegroundWindow();
	int wID=TrackPopupMenu(Menu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,rcButton.left,rcButton.bottom,0,*this,NULL);	
	Menu.DestroyMenu();

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

	HWND hFocus=GetFocus();
	if (hFocus==NULL || hFocus==GetDlgItem(IDC_PRESETS))
	{
		switch (m_pTabCtrl->GetCurFocus())
		{
		case 0:
			::SetFocus(m_NameDlg.GetNextDlgTabItem(NULL,FALSE));
			break;
		case 1:
			::SetFocus(m_SizeDateDlg.GetNextDlgTabItem(NULL,FALSE));
			break;
		case 2:
			::SetFocus(m_AdvancedDlg.GetNextDlgTabItem(NULL,FALSE));
			break;
		}
	}
}

DWORD CLocateDlg::CheckExistenceOfPreset(LPCWSTR szName,DWORD* pdwPresets) // Returns index to preset or FFFFFFFF
{
	// First, find free indentifier
	CRegKey2 RegKey;
	
	if (RegKey.OpenKey(HKCU,"\\Dialogs\\SearchPresets",CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
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
		StringCbPrintf(szBuffer,30,"Preset %03d",nPreset);

		if (RegKey2.OpenKey(RegKey,szBuffer,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
			break;

		CStringW sCurrentName;
		if (RegKey2.QueryValue(L"",sCurrentName)>0)
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

	CRegKey2 MainKey;
	CRegKey PresetKey;

	if (MainKey.OpenKey(HKCU,"\\Dialogs\\SearchPresets",
		CRegKey::createNew|CRegKey::samAll)!=ERROR_SUCCESS)
		return;

	char szKeyName[30];
	StringCbPrintf(szKeyName,30,"Preset %03d",dwID);

	// Deleting key if it exists
	MainKey.DeleteKey(szKeyName);
	
	if (PresetKey.OpenKey(MainKey,szKeyName,CRegKey::createNew|CRegKey::samAll)!=ERROR_SUCCESS)
		return;
	
	PresetKey.SetValue(L"",PresetDialog.m_sReturnedPreset);
	m_NameDlg.SaveControlStates(PresetKey);
	m_SizeDateDlg.SaveControlStates(PresetKey);
	m_AdvancedDlg.SaveControlStates(PresetKey);

}

void CLocateDlg::OnPresetsSelection(int nPreset)
{
	CRegKey2 RegKey;
	CRegKey PresetKey;
	
	if (RegKey.OpenKey(HKCU,"\\Dialogs\\SearchPresets",CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return;

	char szBuffer[30];
	StringCbPrintf(szBuffer,30,"Preset %03d",nPreset);

	if (PresetKey.OpenKey(RegKey,szBuffer,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return;

	m_NameDlg.LoadControlStates(PresetKey);
	m_NameDlg.EnableItems(TRUE);

	m_SizeDateDlg.LoadControlStates(PresetKey,TRUE);
	m_SizeDateDlg.EnableItems(TRUE);

	m_AdvancedDlg.LoadControlStates(PresetKey);
	m_AdvancedDlg.EnableItems(TRUE);

}

void CLocateDlg::LoadPreset(LPCWSTR szPreset)
{
	CRegKey2 RegKey;
	CRegKey PresetKey;


	if (RegKey.OpenKey(HKCU,"\\Dialogs\\SearchPresets",CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return;
	
	// Finding registry entry for the preset
	for (int nPreset=0;;nPreset++) 
	{
		CHAR szBuffer[30];
		StringCbPrintf(szBuffer,30,"Preset %03d",nPreset);

		if (PresetKey.OpenKey(RegKey,szBuffer,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
			return;

		CStringW Name;
		if (PresetKey.QueryValue(L"",Name))
		{
			if (Name.Compare(szPreset)==0)
				break;
		}
	} 

	m_NameDlg.LoadControlStates(PresetKey);
	m_NameDlg.EnableItems(TRUE);

	m_SizeDateDlg.LoadControlStates(PresetKey,TRUE);
	m_SizeDateDlg.EnableItems(TRUE);

	m_AdvancedDlg.LoadControlStates(PresetKey);
	m_AdvancedDlg.EnableItems(TRUE);

}

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
		::EnableMenuItem(hSubMenu,IDM_ALIGNTOGRID,MF_GRAYED|MF_BYCOMMAND);
	}

	if (hSubMenu!=NULL)
	{
		::SetMenuItemBitmaps(hSubMenu,0,MF_BYPOSITION,NULL,m_CircleBitmap);
		::SetMenuItemBitmaps(hSubMenu,1,MF_BYPOSITION,NULL,m_CircleBitmap);
		::SetMenuItemBitmaps(hSubMenu,2,MF_BYPOSITION,NULL,m_CircleBitmap);
		::SetMenuItemBitmaps(hSubMenu,3,MF_BYPOSITION,NULL,m_CircleBitmap);
	}

	CShortcut::ModifyMenus(m_aShortcuts,hMenu,m_Menu);

	SetMenu(hMenu);
	if (hOldMenu!=NULL)
		DestroyMenu(hOldMenu);

	SetMenuCheckMarkForListStyle();
}
	

void CLocateDlg::SetShortcuts()
{
	CLocateDlgThread* pLocateDlgThread=GetLocateAppWnd()->m_pLocateDlgThread;
	// This function should not be called from other thread than dialog's thread
	// (to be sure that m_Accels is not accessed during this call
	ASSERT(GetCurrentThread()!=*pLocateDlgThread);
	

	// Clear accel tables
	ClearShortcuts();

	// Loading shortcuts
	if (!CShortcut::LoadShortcuts(m_aShortcuts,CShortcut::loadLocal))
	{
        if (!CShortcut::GetDefaultShortcuts(m_aShortcuts,CShortcut::loadLocal))
		{
			SetMenus();
			return;
		}
	}

	HWND hDialogs[]={*this,m_NameDlg,m_SizeDateDlg,m_AdvancedDlg,NULL};
	CShortcut::ResolveMnemonics(m_aShortcuts,hDialogs);

	// Cointing items
	UINT nInResultListAccels=0,nNotInResultListAccels=0;
	for (int i=0;i<m_aShortcuts.GetSize();i++)
	{
		ASSERT((m_aShortcuts[i]->m_dwFlags&CShortcut::sfKeyTypeMask)==CShortcut::sfLocal);

		if (m_aShortcuts[i]->m_wWhenPressed&CShortcut::wpFocusInResultList)
			nInResultListAccels++;
		if (m_aShortcuts[i]->m_wWhenPressed&CShortcut::wpFocusNotInResultList)
			nNotInResultListAccels++;
	}

	// Creating result list accels
	if (nInResultListAccels>0)
	{
		ASSERT(m_pListCtrl!=NULL);

		ACCEL* pAccels=new ACCEL[nInResultListAccels];
		

		int nAccels=0;
		for (int i=0;i<m_aShortcuts.GetSize();i++)
		{
			if (!(m_aShortcuts[i]->m_wWhenPressed&CShortcut::wpFocusInResultList))
				continue;
			
			pAccels[nAccels].fVirt=FNOINVERT|FVIRTKEY;
			if (m_aShortcuts[i]->m_bModifiers&CShortcut::ModifierAlt)
				pAccels[nAccels].fVirt|=FALT;
			if (m_aShortcuts[i]->m_bModifiers&CShortcut::ModifierControl)
				pAccels[nAccels].fVirt|=FCONTROL;
			if (m_aShortcuts[i]->m_bModifiers&CShortcut::ModifierShift)
				pAccels[nAccels].fVirt|=FSHIFT;

			// Check whether accel already listed
			for (int k=0;k<nAccels;k++)
			{
				if (pAccels[k].key==m_aShortcuts[i]->m_bVirtualKey &&
					pAccels[k].fVirt==pAccels[nAccels].fVirt)
				{
					UINT nAlreadyInList;
					for (nAlreadyInList=0;m_aActiveShortcuts[k][nAlreadyInList]!=NULL;nAlreadyInList++);

                    CShortcut** pList=new CShortcut*[nAlreadyInList+2];
					CopyMemory(pList,m_aActiveShortcuts[k],sizeof(CShortcut*)*nAlreadyInList);
					pList[nAlreadyInList++]=m_aShortcuts[i];
					pList[nAlreadyInList]=NULL;

					delete[] m_aActiveShortcuts[k];
					m_aActiveShortcuts[k]=pList;

					continue;
				}
			}


			// Registering new active shortcut
			CShortcut** pList=new CShortcut*[2];
			pList[0]=m_aShortcuts[i];
			pList[1]=NULL;

			pAccels[nAccels].key=m_aShortcuts[i]->m_bVirtualKey;
			pAccels[nAccels].cmd=IDM_DEFSHORTCUTITEM+m_aActiveShortcuts.GetSize();
			m_aActiveShortcuts.Add(pList);
			

			nAccels++;
		}
		HACCEL hAccel=CreateAcceleratorTable(pAccels,nInResultListAccels);
		pLocateDlgThread->SetAccelTableForWindow(*m_pListCtrl,hAccel,FALSE,*this);
		delete[] pAccels;
	}

	// Creating accels for other controls
	if (nNotInResultListAccels>0)
	{
		ASSERT((HWND)m_NameDlg!=NULL);

		ACCEL* pAccels=new ACCEL[nNotInResultListAccels];
		int nFirstActive=m_aActiveShortcuts.GetSize();

		int nAccels=0;
		for (int i=0;i<m_aShortcuts.GetSize();i++)
		{
			if (!(m_aShortcuts[i]->m_wWhenPressed&CShortcut::wpFocusNotInResultList))
				continue;

			
			pAccels[nAccels].fVirt=FNOINVERT|FVIRTKEY;
			if (m_aShortcuts[i]->m_bModifiers&CShortcut::ModifierAlt)
				pAccels[nAccels].fVirt|=FALT;
			if (m_aShortcuts[i]->m_bModifiers&CShortcut::ModifierControl)
				pAccels[nAccels].fVirt|=FCONTROL;
			if (m_aShortcuts[i]->m_bModifiers&CShortcut::ModifierShift)
				pAccels[nAccels].fVirt|=FSHIFT;
			pAccels[nAccels].cmd=IDM_DEFSHORTCUTITEM+i;
			
			// Check whether accel already listed
			for (int k=0;k<nAccels;k++)
			{
				if (pAccels[k].key==m_aShortcuts[i]->m_bVirtualKey &&
					pAccels[k].fVirt==pAccels[nAccels].fVirt)
				{
					UINT nAlreadyInList;
					for (nAlreadyInList=0;m_aActiveShortcuts[nFirstActive+k][nAlreadyInList]!=NULL;nAlreadyInList++);

                    CShortcut** pList=new CShortcut*[nAlreadyInList+2];
					CopyMemory(pList,m_aActiveShortcuts[nFirstActive+k],sizeof(CShortcut*)*nAlreadyInList);
					pList[nAlreadyInList++]=m_aShortcuts[i];
					pList[nAlreadyInList]=NULL;

					delete[] m_aActiveShortcuts[nFirstActive+k];
					m_aActiveShortcuts[nFirstActive+k]=pList;

					continue;
				}
			}


			// Registering new active shortcut
			CShortcut** pList=new CShortcut*[2];
			pList[0]=m_aShortcuts[i];
			pList[1]=NULL;

			pAccels[nAccels].key=m_aShortcuts[i]->m_bVirtualKey;
			pAccels[nAccels].cmd=IDM_DEFSHORTCUTITEM+m_aActiveShortcuts.GetSize();
			m_aActiveShortcuts.Add(pList);
			

			nAccels++;
		}
		HACCEL hAccel=CreateAcceleratorTable(pAccels,nNotInResultListAccels);
		
		pLocateDlgThread->SetAccelTableForWindow(*this,hAccel,FALSE,*this);
		pLocateDlgThread->SetAccelTableForChilds(*this,hAccel,TRUE,*this);
		pLocateDlgThread->SetAccelTableForWindow(m_NameDlg,hAccel,FALSE,*this);
		pLocateDlgThread->SetAccelTableForChilds(m_NameDlg,hAccel,FALSE,*this);
		pLocateDlgThread->SetAccelTableForWindow(m_SizeDateDlg,hAccel,FALSE,*this);
		pLocateDlgThread->SetAccelTableForChilds(m_SizeDateDlg,hAccel,FALSE,*this);
		pLocateDlgThread->SetAccelTableForWindow(m_AdvancedDlg,hAccel,FALSE,*this);
		pLocateDlgThread->SetAccelTableForChilds(m_AdvancedDlg,hAccel,FALSE,*this);
		
		delete[] pAccels;
	}


	SetMenus();
}

void CLocateDlg::ClearShortcuts()
{
	CLocateDlgThread* pLocateDlgThread=GetLocateAppWnd()->m_pLocateDlgThread;
	// This function should not be called from other thread than dialog's thread
	// (to be sure that m_Accels is not accessed during this call
	ASSERT(GetCurrentThread()!=*pLocateDlgThread);
	
	// Name dlg accel
	HACCEL hAccel=pLocateDlgThread->GetAccelTableForWindow(m_NameDlg);
	if (hAccel!=NULL)
		DestroyAcceleratorTable(hAccel);
	
	// Size and date dlg accel
	hAccel=pLocateDlgThread->GetAccelTableForWindow(m_SizeDateDlg);
	if (hAccel!=NULL)
		DestroyAcceleratorTable(hAccel);
	
	// Advanced dialog accel
	hAccel=pLocateDlgThread->GetAccelTableForWindow(m_AdvancedDlg);
	if (hAccel!=NULL)
		DestroyAcceleratorTable(hAccel);
	
	// Result list accel
	if (m_pListCtrl!=NULL)
	{
		hAccel=pLocateDlgThread->GetAccelTableForWindow(*m_pListCtrl);
		if (hAccel!=NULL)
			DestroyAcceleratorTable(hAccel);
	}

	pLocateDlgThread->ClearAccelTables();
	m_aShortcuts.RemoveAll();
	m_aActiveShortcuts.RemoveAll();

}
	

void CLocateDlg::SetDefaultActions(CSubAction*** pActions) const
{
	pActions[Name][
		GetFlags()&fgLVStylePointToSelect?LeftMouseButtonClick:LeftMouseButtonDblClick]=new CSubAction(CSubAction::Execute);
	pActions[Name][RightMouseButtonClick]=new CSubAction(CSubAction::OpenContextMenu);
}


/* CLocateDlg::LoadResultlistActions and CLocateDlg::SaveResultlistActions:
Format in registry: all non "zero" (do something) actions are stored 
one after another in the following format

1. column index (CLocateDlg::DetailType) as DWORD value 
2. action (CLocateDlg::ResultListAction) as DWORD value 
3. data given by CSubAction::GetData and CSubAction::FromData

*/

void CLocateDlg::LoadResultlistActions()
{
	// Going to set default actions
	ClearResultlistActions();

	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::defRead)==ERROR_SUCCESS)
	{
		BOOL bOk;
		DWORD dwLength=RegKey.QueryValueLength("ResultListActions",bOk);
		if (bOk)
		{
			//DebugFormatMessage("CLocateDlg::LoadResultlistActions(): ResultListActions length: %d",dwLength);

			if (dwLength==0)
				return;


			BYTE *pData=new BYTE[dwLength];
			if (RegKey.QueryValue("ResultListActions",LPSTR(pData),dwLength)==dwLength)
			{
				// Check all actions
                BYTE* pPtr=pData;
				while (dwLength>0)
				{
					// Stop if action is not suitable
					if (*((WORD*)pPtr)>LastType || *((WORD*)pPtr+1)>=ListActionCount)
						break;

					DWORD dwUsed;
					CSubAction* pSubAction=CSubAction::FromData(CAction::ResultListItems,pPtr+2*sizeof(WORD),dwLength-2*sizeof(WORD),dwUsed);
					if (pSubAction==NULL)
						break;

					m_aResultListActions[*((WORD*)pPtr)][*((WORD*)pPtr+1)]=pSubAction;

					pPtr+=2*sizeof(WORD)+dwUsed;
					dwLength-=2*sizeof(WORD)+dwUsed;
                    
				}

				delete[] pData;

				// If dwLength==0, all actions was OK
				if (dwLength==0)
					return;

				
				// Going to set default actions
				ClearResultlistActions();

			}
			else
				delete[] pData;

			
		}
	}

	// Set default actions, 2D array is converted to 
	// 1D array for SetDefaultActions
	CSubAction*** ppActions=new CSubAction**[TypeCount];
	for (int i=0;i<TypeCount;i++)
		ppActions[i]=m_aResultListActions[i];
	SetDefaultActions(ppActions);
	delete[] ppActions;
}

void CLocateDlg::SaveResultlistActions()
{
	// Counting length of data
	DWORD dwLength=0;
	for (int iCol=0;iCol<TypeCount;iCol++)
	{
		for (int iAct=0;iAct<ListActionCount;iAct++)
		{
			if (m_aResultListActions[iCol][iAct]!=NULL)
				dwLength+=2*sizeof(WORD)+m_aResultListActions[iCol][iAct]->GetDataLength(CAction::ResultListItems);
		}
	}

	// Constuct data
	BYTE* pData=new BYTE[dwLength];
	BYTE* pPtr=pData;
	for (int iCol=0;iCol<TypeCount;iCol++)
	{
		for (int iAct=0;iAct<ListActionCount;iAct++)
		{
			if (m_aResultListActions[iCol][iAct]!=NULL)
			{
				*((WORD*)pPtr)=iCol;
				*((WORD*)pPtr+1)=iAct;
                pPtr+=2*sizeof(WORD);

				DWORD dwUsed=m_aResultListActions[iCol][iAct]->GetData(CAction::ResultListItems,pPtr,TRUE);
                
				pPtr+=dwUsed;
			}
		}
	}

	ASSERT(DWORD(pPtr-pData)==dwLength);

	//DebugFormatMessage("CLocateDlg::SaveResultlistActions(): ResultListActions length: %d",dwLength);

	// Save to registry
	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::defWrite)==ERROR_SUCCESS)
	{
		
		RegKey.SetValue("ResultListActions",LPCSTR(pData),dwLength,REG_BINARY);
		RegKey.CloseKey();
	}	

	delete[] pData;

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
	if (dwID!=DWORD(-1) && SendDlgItemMessage(IDC_EDIT,CB_GETCURSEL)==CB_ERR)
	{
		CStringW msg;
		msg.Format(IDS_OVERWRITEPRESET,LPCWSTR(m_sReturnedPreset));

		if (MessageBox(msg,ID2W(IDS_PRESETSAVETITLE),MB_YESNO|MB_ICONQUESTION)==IDNO)
		{
			SetFocus(IDC_EDIT);
			return;
		}
	}
		
	EndDialog(1);
}

BOOL CLocateDlg::CSavePresetDlg::OnInitDialog(HWND hwndFocus)
{
	::CSavePresetDlg::OnInitDialog(hwndFocus);
	
	CString Path;
	CRegKey2 RegKey;
	CRegKey PresetKey;
	CComboBox Combo(GetDlgItem(IDC_EDIT));
	char szBuffer[30];

	if (RegKey.OpenKey(HKCU,"\\Dialogs\\SearchPresets",CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return FALSE;
	
	
	
	for (int nPreset=0;nPreset<1000;nPreset++)
	{
		StringCbPrintf(szBuffer,30,"Preset %03d",nPreset);

		if (PresetKey.OpenKey(RegKey,szBuffer,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
			break;

		CStringW sCurrentName;
		PresetKey.QueryValue(L"",sCurrentName);
		Combo.AddString(sCurrentName);

		PresetKey.CloseKey();
	}	
	return FALSE;
}

void CLocateDlg::CNameDlg::OnMoreDirectories()
{
	if (m_pMultiDirs==NULL)
		return;

	CMenu Menu;
	CStringW str;
	MENUITEMINFOW mii;
	
	Menu.CreatePopupMenu();

	CLocateDlg::InsertMenuItemsFromTemplate(Menu,GetLocateDlg()->m_Menu.GetSubMenu(SUBMENU_MULTIDIRSELECTION),0);
	
	mii.cbSize=sizeof(MENUITEMINFOW);
	// Inserting separator
	mii.fMask=MIIM_TYPE|MIIM_ID;
	mii.wID=IDM_DEFMENUITEM;
	mii.fType=MFT_STRING;
	int i;
	for (i=0;m_pMultiDirs[i]!=NULL;i++)
	{
		if (m_pMultiDirs[i]->bSelected)
		{
			EnterCriticalSection(&m_cBrowse);
			m_pMultiDirs[i]->SetValuesFromControl(m_LookIn,m_pBrowse,m_nMaxBrowse);
			LeaveCriticalSection(&m_cBrowse);
			mii.fMask=MIIM_TYPE|MIIM_ID|MIIM_STATE;
			mii.fState=MFS_ENABLED|MFS_CHECKED;
		}
		else
			mii.fMask=MIIM_TYPE|MIIM_ID;

		int nLen=10+istrlenw(m_pMultiDirs[i]->pTitleOrDirectory);
		WCHAR* pString=new WCHAR[nLen];
		StringCbPrintfW(pString,nLen*sizeof(WCHAR),L"%d: %s",i+1,m_pMultiDirs[i]->pTitleOrDirectory);
		mii.dwTypeData=pString;
	
		if (Menu.InsertMenu(i,TRUE,&mii))
			mii.wID++;

		delete[] pString;
	}

	if (i==1) // Only one item, disabling remove
		Menu.EnableMenuItem(IDM_LOOKINREMOVESELECTION,MF_BYCOMMAND|MF_GRAYED);

	RECT rcButton;
	::GetWindowRect(GetDlgItem(IDC_MOREDIRECTORIES),&rcButton);
		
	SetForegroundWindow();
	int wID=TrackPopupMenu(Menu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,rcButton.left,rcButton.bottom,0,*this,NULL);	
	Menu.DestroyMenu();

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
	EnterCriticalSection(&m_cBrowse);
	m_pMultiDirs[nCurrentSelection]->SetValuesFromControl(m_LookIn,m_pBrowse,m_nMaxBrowse);
	LeaveCriticalSection(&m_cBrowse);
	m_pMultiDirs[nCurrentSelection]->bSelected=FALSE;
			
	// Set new selection
	if (m_pMultiDirs[nNewSelection]->nType==Root)
		SelectByRootName(m_pMultiDirs[nNewSelection]->pTitleOrDirectory);
	else if (m_pMultiDirs[nNewSelection]->nType==Custom)
		SelectByCustomName(m_pMultiDirs[nNewSelection]->pTitleOrDirectory);
	else
	{
		EnterCriticalSection(&m_cBrowse);
		SelectByLParam(m_pMultiDirs[nNewSelection]->GetLParam(m_pBrowse,m_nMaxBrowse));
		LeaveCriticalSection(&m_cBrowse);
	}
	m_pMultiDirs[nNewSelection]->bSelected=TRUE;

	char szName[10];
	StringCbPrintf(szName,10,"#%d",nNewSelection+1);
	SetDlgItemText(IDC_MOREDIRECTORIES,szName);
}

void CLocateDlg::CNameDlg::OnLookInNextSelection(BOOL bNext)
{
	if (m_pMultiDirs==NULL)
		return;

	// Getting current selection
	int nCurrentSelection=-1,nSelection,nDirs;
	for (nDirs=0;m_pMultiDirs[nDirs]!=NULL;nDirs++)
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

	int nCurrentSelection=-1,nDirs;
	for (nDirs=0;m_pMultiDirs[nDirs]!=NULL;nDirs++)
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

	int nSelected=0,nDirs;
	for (nDirs=0;m_pMultiDirs[nDirs]!=NULL;nDirs++)
	{
		if (m_pMultiDirs[nDirs]->bSelected)
		{
			nSelected=nDirs;
			m_pMultiDirs[nDirs]->bSelected=FALSE;
		}			
	}

	EnterCriticalSection(&m_cBrowse);
	m_pMultiDirs[nSelected]->SetValuesFromControl(m_LookIn,m_pBrowse,m_nMaxBrowse);
	LeaveCriticalSection(&m_cBrowse);
			
    DirSelection** pMultiDirsNew=new DirSelection*[nDirs+2];
	CopyMemory(pMultiDirsNew,m_pMultiDirs,sizeof(DirSelection*)*nDirs);
    delete[] m_pMultiDirs;
	m_pMultiDirs=pMultiDirsNew;

	m_pMultiDirs[nDirs++]=new DirSelection(TRUE);
    m_pMultiDirs[nDirs]=NULL;

	SelectByLParam(MAKELPARAM(Everywhere,Original));

	char szName[10];
	StringCbPrintf(szName,10,"#%d",nDirs);
	SetDlgItemText(IDC_MOREDIRECTORIES,szName);

	HilightTab(TRUE);
}

void CLocateDlg::CNameDlg::OnLookInRemoveSelection()
{
	if (m_pMultiDirs==NULL)
		return;
	
	int nCurrentSelection=-1,nDirs;
	for (nDirs=0;m_pMultiDirs[nDirs]!=NULL;nDirs++)
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
	{
		EnterCriticalSection(&m_cBrowse);
		SelectByLParam(m_pMultiDirs[nCurrentSelection]->GetLParam(m_pBrowse,m_nMaxBrowse));
		LeaveCriticalSection(&m_cBrowse);
	}
	m_pMultiDirs[nCurrentSelection]->bSelected=TRUE;

	char szName[10];
	StringCbPrintf(szName,10,"#%d",nCurrentSelection+1);
	SetDlgItemText(IDC_MOREDIRECTORIES,szName);

	HilightTab(IsChanged());
}
		

void CLocateDlg::CNameDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType,cx,cy);
	CRect rc;
	m_Name.GetWindowRect(&rc);
	m_Name.SetWindowPos(HWND_BOTTOM,0,0,cx-m_nFieldLeft-8,rc.Height(),SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	m_Type.SetWindowPos(HWND_BOTTOM,0,0,cx-m_nFieldLeft-8,rc.Height(),SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	SetDlgItemPos(IDC_NOSUBDIRECTORIES,HWND_BOTTOM,cx-m_nButtonWidth-m_nCheckWidth-12,m_nBrowseTop+3,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOCOPYBITS);
	SetDlgItemPos(IDC_BROWSE,HWND_BOTTOM,cx-m_nButtonWidth-8,m_nBrowseTop,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOCOPYBITS);
	
	if (GetLocateDlg()->GetFlags()&CLocateDlg::fgNameMultibleDirectories)
	{
		m_LookIn.SetWindowPos(HWND_BOTTOM,0,0,cx-m_nFieldLeft-m_nMoreDirsWidth-11,rc.Height(),SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOCOPYBITS);
		SetDlgItemPos(IDC_MOREDIRECTORIES,HWND_BOTTOM,cx-m_nMoreDirsWidth-8,m_nMoreDirsTop,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOCOPYBITS);
	}
	else
		m_LookIn.SetWindowPos(HWND_BOTTOM,0,0,cx-m_nFieldLeft-8,rc.Height(),SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
	
}

void CLocateDlg::CNameDlg::OnClear(BOOL bInitial)
{
	m_Name.SetText(szEmpty);
	m_Type.SetText(szEmpty);
	m_Type.EnableWindow(TRUE);
	CheckDlgButton(IDC_NOSUBDIRECTORIES,FALSE);

	if (m_pMultiDirs!=NULL)
	{
		for (int i=0;m_pMultiDirs[i]!=NULL;i++)
			delete m_pMultiDirs[i];
		delete[] m_pMultiDirs;
		
		// Initializing struct
		m_pMultiDirs=new DirSelection*[2];
        m_pMultiDirs[0]=new DirSelection(TRUE);
		m_pMultiDirs[1]=NULL;

		SetDlgItemText(IDC_MOREDIRECTORIES,"#1");
	}
	
	for (int i=0;m_LookIn.GetCount();i++)
	{
		if (m_LookIn.GetItemData(i)==MAKELPARAM(Everywhere,Original))
		{
			m_LookIn.SetCurSel(i);
			m_LookIn.SetItemText(-1,m_LookIn.GetItemText(i));
			break;
		}
	}

	HilightTab(FALSE);
}

void CLocateDlg::CNameDlg::SaveRegistry() const
{
	CRegKey2 RegKey;
	if (GetLocateDlg()->GetExtraFlags()&efNameDontSaveNameTypeAndDirectories)
	{
		// Do not save list items, just delete key
		RegKey.DeleteKey("\\Recent Strings");
		return;
	}

	if(RegKey.OpenKey(HKCU,"\\Recent Strings",CRegKey::defWrite)==ERROR_SUCCESS)
	{
		CStringW buffer,bfr;
		int i=0;

		// Remove existing items
		while(RegKey.EnumValue(0,buffer))
			RegKey.DeleteValue(buffer);
		
	
		RegKey.SetValue("NumberOfNames",m_nMaxNamesInList);
		for (i=m_Name.GetCount()-1;i>=0;i--)
		{
			bfr="Name";
			bfr<<int(i);
			m_Name.GetLBText(i,buffer);
			RegKey.SetValue(bfr,buffer);
		}
		
	
		RegKey.SetValue("NumberOfTypes",m_nMaxTypesInList);
		for (i=m_Type.GetCount()-1;i>0;i--) // 0 is (none)
		{
			bfr="Type";
			bfr<<int(i-1);
			m_Type.GetLBText(i,buffer);
			RegKey.SetValue(bfr,buffer);
		}
		
		EnterCriticalSection(&m_cBrowse);
		if (m_pBrowse!=NULL)
		{
			RegKey.SetValue("NumberOfDirectories",m_nMaxBrowse);
			for (i=0;i<int(m_nMaxBrowse);i++)
			{
				bfr="Directory";
				bfr<<(int)i;
				RegKey.SetValue(bfr,m_pBrowse[i]);
			}
		}
		LeaveCriticalSection(&m_cBrowse);
			
	}


}

void CLocateDlg::CNameDlg::LoadRegistry()
{
	CRegKey2 RegKey;
	m_Name.ResetContent();
	m_Type.ResetContent();
	m_Type.AddString(ID2W(IDS_NOEXTENSION));
	

	if (RegKey.OpenKey(HKCU,"\\Recent Strings",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		WORD i;
		CStringW name,buffer;
		
		RegKey.QueryValue("NumberOfNames",m_nMaxNamesInList);
		if (m_nMaxNamesInList>255)
			m_nMaxNamesInList=DEFAULT_NUMBEROFNAMES;

		RegKey.QueryValue("NumberOfTypes",m_nMaxTypesInList);
		if (m_nMaxTypesInList>255)
			m_nMaxTypesInList=DEFAULT_NUMBEROFTYPES;

		RegKey.QueryValue("NumberOfDirectories",m_nMaxBrowse);
		if (m_nMaxBrowse>255)
			m_nMaxBrowse=DEFAULT_NUMBEROFDIRECTORIES;

		


		for (i=0;i<m_nMaxNamesInList;i++)
		{
			name=L"Name";
			name<<(int)i;
			if (RegKey.QueryValue(name,buffer))
				m_Name.AddString(buffer);
		}
		for (i=0;i<m_nMaxTypesInList;i++)
		{
			name=L"Type";
			name<<(int)i;
			if (RegKey.QueryValue(name,buffer))
				m_Type.AddString(buffer);
		}

		EnterCriticalSection(&m_cBrowse);
		if (m_nMaxBrowse>0)
			m_pBrowse=new CStringW[m_nMaxBrowse];

		for (DWORD i=0;i<m_nMaxBrowse;i++)
		{
			name=L"Directory";
			name<<(int)i;
			if (!RegKey.QueryValue(name,m_pBrowse[i]))
				m_pBrowse[i].Empty();
		}
		LeaveCriticalSection(&m_cBrowse);

	}
	else
	{
		EnterCriticalSection(&m_cBrowse);
		if (m_pBrowse==NULL && m_nMaxBrowse>0)
			m_pBrowse=new CStringW[m_nMaxBrowse];
		LeaveCriticalSection(&m_cBrowse);
	}
}

BOOL CLocateDlg::CNameDlg::CheckAndAddDirectory(LPCWSTR pFolder,DWORD dwLength,BOOL bAlsoSet,BOOL bNoWarning)
{
	CStringW FolderLower(pFolder,dwLength);
	FolderLower.MakeLower();

	DebugFormatMessage(L"Directory to add: %s",(LPCWSTR)FolderLower);

	COMBOBOXEXITEMW ci;
	SHFILEINFOW fi;
			
	CArrayFAP<LPWSTR> aRoots;
	CDatabaseInfo::GetRootsFromDatabases(aRoots,GetLocateApp()->GetDatabases());
	
	// Checking whether Folder is subdirectory of any aRoots
	BOOL bFound=FALSE;
	for (int i=0;i<aRoots.GetSize();i++)
	{
		CStringW Path(aRoots.GetAt(i));
		Path.MakeLower();
		
		if (wcsncmp(Path,FolderLower,min((DWORD)Path.GetLength(),dwLength))==0)
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

	// Checking whether path is drive which already exists
	if (dwLength==2 && pFolder[1]==L':')
	{
		for (int i=0;i<m_LookIn.GetCount();i++)
		{
			LPARAM lParam=m_LookIn.GetItemData(i);
			if (static_cast<TypeOfItem>(LOWORD(lParam))!=Drive)
			{
				WCHAR cDrive=(WCHAR)HIWORD(lParam);
				MakeLower(&cDrive,1);
				if (cDrive==FolderLower[0] && bAlsoSet)
				{
					m_LookIn.SetCurSel(i);
					m_LookIn.SetItemText(-1,m_LookIn.GetItemTextW(i));
					return TRUE;
				}
			}
		}
	}

	if (GetLocateDlg()->GetExtraFlags()&CLocateDlg::efNameDontSaveNetworkDrivesAndDirectories)
	{
		if (pFolder[0]=='\\' && pFolder[1]=='\\')
		{
			// UNC path
			return TRUE;
		}

		if (pFolder[1]==':')
		{
			WCHAR pDrive[]=L"X:\\";
			pDrive[0]=pFolder[0];
			if (FileSystem::GetDriveType(pDrive)==DRIVE_REMOTE)
			{
				// Mapped drive
				return TRUE;
			}
		}
	}

	EnterCriticalSection(&m_cBrowse);

	if (m_pBrowse!=NULL)
	{
		// Check whether folder already exists in other directory list
		for (DWORD i=0;i<m_nMaxBrowse;i++)
		{
			CStringW str(m_pBrowse[i]);
			str.MakeLower();
			if (str.Compare(FolderLower)==0)
			{
				// Deleting previous one
				for (DWORD j=i+1;j<m_nMaxBrowse;j++)
					m_pBrowse[j-1].Swap(m_pBrowse[j]);
				m_pBrowse[m_nMaxBrowse-1].Empty();
			}
		}

		// moving previous folders one down
		for (int i=m_nMaxBrowse-1;i>0;i--)
			m_pBrowse[i].Swap(m_pBrowse[i-1]);
		
		m_pBrowse[0].Copy(pFolder,dwLength);

		
			
		// If selection is on, following code may mess it
		int nCurSel=m_LookIn.GetCurSel();
		m_LookIn.SetCurSel(-1);


		// Removing old items from list
		ci.mask=CBEIF_LPARAM;
		for (ci.iItem=m_LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
		{
			m_LookIn.GetItem(&ci);
			if (static_cast<TypeOfItem>(LOWORD(ci.lParam))!=Custom)
				break;
			m_LookIn.DeleteItem((int)ci.iItem);
		}
		
		// Adding new items
		int nSel=(int)++ci.iItem;
		ci.iIndent=0;
		ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
		for (DWORD i=0;i<m_nMaxBrowse;i++)
		{
			if (m_pBrowse[i].IsEmpty())
				break;

			if (GetLocateApp()->GetProgramFlags()&CLocateApp::pfUseDefaultIconForDirectories)
			{
				ci.iImage=DIR_IMAGE;
				ci.iSelectedImage=DIR_IMAGE;
			}
			else
			{
				GetFileInfo(m_pBrowse[i],0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
				ci.iImage=fi.iIcon;
				GetFileInfo(m_pBrowse[i],0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
				ci.iSelectedImage=fi.iIcon;
			}

		
			ci.pszText=m_pBrowse[i].GetBuffer();
			ci.lParam=MAKELPARAM(Custom,i);
			m_LookIn.InsertItem(&ci);
			ci.iItem++;
		}

		if (bAlsoSet)
		{
			m_LookIn.SetCurSel(nSel);
			m_LookIn.SetItemText(-1,m_LookIn.GetItemTextW(nSel));
		}
		else
			m_LookIn.SetCurSel(nCurSel);
	
	}			
	
	LeaveCriticalSection(&m_cBrowse);
		
	return TRUE;		
}

void CLocateDlg::CNameDlg::OnBrowse()
{
	CWaitCursor wait;
	DebugMessage("CLocateDlg::CNameDlg::OnBrowse() BEGIN");
	CFolderDialog fd(IDS_GETFOLDER,BIF_RETURNONLYFSDIRS|BIF_USENEWUI|BIF_NONEWFOLDERBUTTON);
	if (fd.DoModal(*this))
	{
	

		CStringW Folder;
		if (fd.GetFolder(Folder))
			CheckAndAddDirectory(Folder,Folder.GetLength(),TRUE,FALSE);
		else
		{
			WCHAR szName[500];
			if (GetComputerNameFromIDList(fd.m_lpil,szName,500))
			{
				if (szName[0]==L'\\' && szName[1]==L'\\')
					CheckAndAddDirectory(szName,istrlenw(szName),TRUE,FALSE);
			}
	
		}
		m_LookIn.SetFocus();

		HilightTab(TRUE);
		DebugMessage("CLocateDlg::CNameDlg::OnBrowse() END1");
		return;
	}
	
	DebugMessage("CLocateDlg::CNameDlg::OnBrowse() END2");
}

void CLocateDlg::CNameDlg::EnableItems(BOOL bEnable)
{
	m_Name.EnableWindow(bEnable);
	m_Type.EnableWindow(bEnable && 
		GetLocateDlg()->m_AdvancedDlg.SendDlgItemMessage(IDC_FILETYPE,CB_GETCURSEL)==0 &&
		!GetLocateDlg()->m_AdvancedDlg.IsDlgButtonChecked(IDC_USEWHOLEPATH));
	m_LookIn.EnableWindow(bEnable);
	EnableDlgItem(IDC_MOREDIRECTORIES,bEnable);
	EnableDlgItem(IDC_NOSUBDIRECTORIES,bEnable);
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
		m_Name.SetText(pStartData->m_pStartString);
	if (pStartData->m_pTypeString!=NULL)
	{
		if (pStartData->m_pTypeString[0]=='\0')
			m_Type.SetCurSel(0);
		else
			m_Type.SetText(pStartData->m_pTypeString);
	}
	if (pStartData->m_pStartPath!=NULL)
		SetPath(pStartData->m_pStartPath);
}

BOOL CLocateDlg::CNameDlg::SetPath(LPCWSTR szPath)
{
	WCHAR temp[MAX_PATH];
	LPWSTR tmp;
	int ret=_wtoi(szPath);


	
	if ((ret>0 && ret<=4) || szPath[0]==L'0')
	{
		if (ret==0)
			SelectByLParam(MAKELPARAM(Everywhere,Original));
		else
			SelectByLParam(MAKELPARAM(Special,ret));
		return FALSE;	
	}

	ret=FileSystem::GetFullPathName(szPath,MAX_PATH,temp,&tmp);
	if (ret>1)
	{
		if (ret<4)
		{
			MakeUpper(temp);
			if (temp[1]==':')
			{
				COMBOBOXEXITEM ci;
				ci.mask=CBEIF_LPARAM;
				for (ci.iItem=m_LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
				{
					m_LookIn.GetItem(&ci);
					if (static_cast<TypeOfItem>(LOWORD(ci.lParam))!=Drive)
						continue;
					if ((char)HIWORD(ci.lParam)==temp[0])
					{
						m_LookIn.SetCurSel((int)ci.iItem);
						m_LookIn.SetItemText(-1,m_LookIn.GetItemTextW((int)ci.iItem));
						return TRUE;
					}
				}
					
			}
		}
		

		EnterCriticalSection(&m_cBrowse);

		if (m_pBrowse!=NULL && FileSystem::IsDirectory(szPath)) 
		{
			COMBOBOXEXITEMW ci;
			SHFILEINFOW fi;
			CStringW FolderLower(szPath);
			FolderLower.MakeLower();

			// Check whether folder already exists in other directory list
			for (DWORD i=0;i<m_nMaxBrowse;i++)
			{
				CStringW str(m_pBrowse[i]);
				str.MakeLower();
				if (str.Compare(FolderLower)==0)
				{
					// Deleting previous one
					for (DWORD j=i+1;j<m_nMaxBrowse;j++)
						m_pBrowse[j-1].Swap(m_pBrowse[j]);
					m_pBrowse[m_nMaxBrowse-1].Empty();
				}
			}

			// moving previous folders one down
			for (int i=m_nMaxBrowse-1;i>0;i--)
				m_pBrowse[i].Swap(m_pBrowse[i-1]);
			
			m_pBrowse[0].Copy(szPath);

			
				
			// If selection is on, following code may mess it
			m_LookIn.SetCurSel(-1);


			// Removing old items from list
			ci.mask=CBEIF_LPARAM;
			for (ci.iItem=m_LookIn.GetCount()-1;ci.iItem>=0;ci.iItem--)
			{
				m_LookIn.GetItem(&ci);
				if (static_cast<TypeOfItem>(LOWORD(ci.lParam))!=Custom)
					break;
				m_LookIn.DeleteItem((int)ci.iItem);
			}
			
			// Adding new items
			int nSel=(int)++ci.iItem;
			ci.iIndent=0;
			ci.mask=CBEIF_TEXT|CBEIF_IMAGE|CBEIF_INDENT|CBEIF_SELECTEDIMAGE|CBEIF_LPARAM;
			for (DWORD i=0;i<m_nMaxBrowse;i++)
			{
				if (m_pBrowse[i].IsEmpty())
					break;

				if (GetLocateApp()->GetProgramFlags()&CLocateApp::pfUseDefaultIconForDirectories)
				{
					ci.iImage=DIR_IMAGE;
					ci.iSelectedImage=DIR_IMAGE;
				}
				else
				{
					GetFileInfo(m_pBrowse[i],0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON);
					ci.iImage=fi.iIcon;
					GetFileInfo(m_pBrowse[i],0,&fi,SHGFI_SYSICONINDEX|SHGFI_SMALLICON|SHGFI_OPENICON);
					ci.iSelectedImage=fi.iIcon;
				}

			
				ci.pszText=m_pBrowse[i].GetBuffer();
				ci.lParam=MAKELPARAM(Custom,i);
				m_LookIn.InsertItem(&ci);
				ci.iItem++;
			}

			m_LookIn.SetCurSel(nSel);
			m_LookIn.SetItemText(-1,m_LookIn.GetItemTextW(nSel));
			
			LeaveCriticalSection(&m_cBrowse);
			return TRUE;
		}

		LeaveCriticalSection(&m_cBrowse);
	}

	for (int i=0;m_LookIn.GetCount();i++)
	{
		if (m_LookIn.GetItemData(i)==MAKELPARAM(Everywhere,Original))
		{
			m_LookIn.SetCurSel(i);
			m_LookIn.SetItemText(-1,m_LookIn.GetItemTextW(i));
			break;
		}
	}
	return TRUE;
}

void CLocateDlg::CNameDlg::DirSelection::SetValuesFromControl(HWND hControl,const CStringW* pBrowse,int nBrowseDirs)
{
	FreeData();

	int nCurSel=(int)::SendMessage((HWND)::SendMessage(hControl,CBEM_GETCOMBOCONTROL,0,0),CB_GETCURSEL,0,0);

	COMBOBOXEXITEMW ci;
	ci.mask=CBEIF_LPARAM|CBEIF_TEXT;
	ci.iItem=nCurSel;
	WCHAR szTitle[MAX_PATH+10]=L"";
	ci.pszText=szTitle;
	ci.cchTextMax=MAX_PATH+10;
	


	if (!::SendMessage(hControl,CBEM_GETITEMW,0,(LPARAM)&ci))
		return;
	
	if (nCurSel==-1)
	{
		// No selection, custom
		DWORD dwLength=istrlenw(szTitle);

		if (dwLength<MAX_PATH+9)
		{
            // Whole string is in szTitle
			nType=CLocateDlg::CNameDlg::Custom;
			pTitleOrDirectory=alloccopy(szTitle,dwLength);
		}
		else
		{
            DWORD dwBufferSize=2*MAX_PATH;
			WCHAR* pFolder;
		
			for(;;)
			{	
				pFolder=new WCHAR[dwBufferSize];
				ci.mask=CBEIF_TEXT;
				ci.pszText=pFolder;

				if (!::SendMessage(hControl,CBEM_GETITEMW,0,(LPARAM)&ci))
				{
					delete[] pFolder;
					return;
				}

				dwLength=istrlenw(pFolder);
			
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
			if (int(HIWORD(ci.lParam))>=0 && HIWORD(ci.lParam)<nBrowseDirs)
				pTitleOrDirectory=alloccopy(pBrowse[HIWORD(ci.lParam)]);
			break;
		case CLocateDlg::CNameDlg::Root:
			break;
		}
	}
}

LPARAM CLocateDlg::CNameDlg::DirSelection::GetLParam(const CStringW* pBrowse,int nBrowseDirs) const
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

BOOL CLocateDlg::CNameDlg::GetDirectoriesForSelection(CArray<LPWSTR>& aDirectories,const DirSelection* pSelection,BOOL bNoWarnings)
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
		return GetDirectoriesFromCustomText(aDirectories,pSelection->pTitleOrDirectory,istrlenw(pSelection->pTitleOrDirectory),pSelection->bSelected,bNoWarnings);
	case Root:
		aDirectories.Add(alloccopy(pSelection->pTitleOrDirectory));
		return TRUE;
	default:
		return FALSE;
	}
}

void CLocateDlg::CNameDlg::LoadControlStates(CRegKey& RegKey)
{
	CStringW str;
	RegKey.QueryValue(L"Name/Name",str);
	m_Name.SetText(str);

	DWORD dwDataLength,dwType;
	dwDataLength=RegKey.QueryValue("Name/Type",NULL,0,&dwType);

	if (dwType==REG_DWORD)
	{
		DWORD dwSel;
		RegKey.QueryValue("Name/Type",dwSel);
		m_Type.SetCurSel(dwSel);
	}
	else if (dwType==REG_SZ)
	{
		WCHAR* pData=new WCHAR[dwDataLength+2];
		if (RegKey.QueryValue(L"Name/Type",pData,dwDataLength+2)>0)
			m_Type.SetText(pData);	
		else
			m_Type.SetText(szEmpty);
		delete[] pData;
	}

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
		int i;
		for (i=0;i<aSelections.GetSize();i++)
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

		for (int i=0;m_pMultiDirs[i]!=NULL;i++)
			delete m_pMultiDirs[i];
		delete[] m_pMultiDirs;
		if (aSelections.GetSize()>0)
		{
			m_pMultiDirs=new DirSelection*[aSelections.GetSize()+1];
			m_pMultiDirs[aSelections.GetSize()]=NULL;

			for (int i=aSelections.GetSize()-1;i>=0;i--)
			{
				StringCbPrintf(szName,200,"Name/LookIn%04d",aSelections[i]);
				
				LONG lLength=RegKey.QueryValueLength(szName);
				CHAR* pData=new CHAR[lLength+2];
				RegKey.QueryValue(szName,pData,lLength);
					
					
				if (*((WORD*)pData)==CNameDlg::NotSelected ||
					(*((WORD*)pData)==CNameDlg::Custom && lLength>4))
				{
					*((LPWSTR)(pData+lLength))=L'\0';
					
					SelectByCustomName((LPWSTR)(pData+4));
	
				}
				else if (lLength==4) 
				{
					DWORD dwLParam;
					RegKey.QueryValue(szName,dwLParam);
	                
					SelectByLParam(MAKELPARAM(HIWORD(dwLParam),LOWORD(dwLParam)));
				}
				
				delete[] pData;
				m_pMultiDirs[i]=new DirSelection(i==0);
				EnterCriticalSection(&m_cBrowse);
				m_pMultiDirs[i]->SetValuesFromControl(m_LookIn,m_pBrowse,m_nMaxBrowse);
				LeaveCriticalSection(&m_cBrowse);
			}
		}
		else
		{
			// It is possible that multidir mode was 
			// not enabled when state is saved, trying it (at least create empty selection)
			m_pMultiDirs=new DirSelection*[2];
			m_pMultiDirs[1]=NULL;
            
			LONG lLength=RegKey.QueryValueLength("Name/LookIn");
			CHAR* pData=new CHAR[lLength+2];
			RegKey.QueryValue(szName,pData,lLength);
			
			
			if (*((WORD*)pData)==CNameDlg::NotSelected ||
				(*((WORD*)pData)==CNameDlg::Custom && lLength>4))
			{
				*((LPWSTR)(pData+lLength))=L'\0';
				SelectByCustomName(LPWSTR(pData+4));
			}
			else if (lLength==4) 
			{
				DWORD dwLParam;
				RegKey.QueryValue(szName,dwLParam);
                
				SelectByLParam(MAKELPARAM(HIWORD(dwLParam),LOWORD(dwLParam)));
			}
				
			delete[] pData;

			m_pMultiDirs[i]=new DirSelection(TRUE);
			EnterCriticalSection(&m_cBrowse);
			m_pMultiDirs[i]->SetValuesFromControl(m_LookIn,m_pBrowse,m_nMaxBrowse);
			LeaveCriticalSection(&m_cBrowse);

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
	        CHAR* pData=new CHAR[lLength+2];
			RegKey.QueryValue("Name/LookIn",pData,lLength);
			if (*((WORD*)pData)==CNameDlg::NotSelected ||
				(*((WORD*)pData)==CNameDlg::Custom && lLength>4))
			{
				// Data is REG_BINARY so '\' is not added by default
				*((LPWSTR)(pData+lLength))=L'\0';
				SelectByCustomName(LPWSTR(pData+4));
			}
			else
			{
				// Choose by lParam

				LPARAM lParam=MAKELPARAM(((WORD*)pData)[1],((WORD*)pData)[0]);
				for (int i=0;i<m_LookIn.GetCount();i++)
				{
					if (lParam==m_LookIn.GetItemData(i))
					{
						m_LookIn.SetCurSel(i);
						m_LookIn.SetItemText(-1,m_LookIn.GetItemTextW(i));
						break;
					}
				}
			}
			delete[] pData;
		}
	}


	DWORD dwTemp;
	if (!RegKey.QueryValue("Name/NoSubDirectories",dwTemp))
		dwTemp=0;
	CheckDlgButton(IDC_NOSUBDIRECTORIES,dwTemp);



	HilightTab(IsChanged());
}

void CLocateDlg::CNameDlg::SaveControlStates(CRegKey& RegKey)
{
	CStringW str;
		
	// Name dialog
	m_Name.GetText(str);
	RegKey.SetValue(L"Name/Name",str);
	
	if (m_Type.GetCurSel()==0) // (none)
		RegKey.SetValue("Name/Type",DWORD(0));
	else
	{
		m_Type.GetText(str);
		RegKey.SetValue(L"Name/Type",str);
	}
	
	
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



	if (m_pMultiDirs==NULL)
	{
		int nCurSel=m_LookIn.GetCurSel();
		LPARAM lParam=MAKELPARAM(NotSelected,0);
		if (nCurSel!=CB_ERR)
            lParam=m_LookIn.GetItemData(nCurSel);
			
		if (LOWORD(lParam)==NotSelected || 
			LOWORD(lParam)==Custom)
		{
			m_LookIn.GetItemText(-1,str.GetBuffer(2000),2000);
			str.FreeExtra();
			char* pData=new char[str.GetLength()*2+4];
			*((DWORD*)pData)=(DWORD)lParam;
			CopyMemory(pData+4,LPCWSTR(str),str.GetLength()*2);
			RegKey.SetValue("Name/LookIn",pData,str.GetLength()*2+4,REG_BINARY);
		}
		else
			RegKey.SetValue("Name/LookIn",(DWORD)MAKELONG(HIWORD(lParam),LOWORD(lParam)));            
	}
	else
	{
		for (int i=0;m_pMultiDirs[i]!=NULL;i++)
		{
			StringCbPrintf(szName,200,"Name/LookIn%04d",i);
					
			if (m_pMultiDirs[i]->bSelected)
			{
				// Current selection
				int nCurSel=m_LookIn.GetCurSel();
				LPARAM lParam=MAKELPARAM(NotSelected,0);
				if (nCurSel!=CB_ERR)
					lParam=m_LookIn.GetItemData(nCurSel);
				
				if (LOWORD(lParam)==NotSelected || 
					LOWORD(lParam)==Custom)
				{
					m_LookIn.GetItemText(-1,str.GetBuffer(2000),2000);
					str.FreeExtra();
					char* pData=new char[str.GetLength()*2+4];
					*((DWORD*)pData)=(DWORD)lParam;
					CopyMemory(pData+4,LPCWSTR(str),str.GetLength()*2);
					RegKey.SetValue(szName,pData,DWORD(str.GetLength()*2+4),REG_BINARY);
					delete[] pData;
				}
				else
				{
					LPARAM lParam=m_LookIn.GetItemData(nCurSel);
					RegKey.SetValue(szName,MAKELONG(HIWORD(lParam),LOWORD(lParam)));            
				}

				RegKey.SetValue("Name/LookInSel",DWORD(i));
			}
			else if (m_pMultiDirs[i]->nType==Custom)
			{	
				DWORD dwLength=istrlenw(m_pMultiDirs[i]->pTitleOrDirectory)*2;
				
				char* pData=new char[dwLength+4];
				*((DWORD*)pData)=DWORD(Custom);
				CopyMemory(pData+4,m_pMultiDirs[i]->pTitleOrDirectory,dwLength);
				RegKey.SetValue(szName,pData,dwLength+4,REG_BINARY);
				delete[] pData;
			}
			else
			{
				EnterCriticalSection(&m_cBrowse);
				LPARAM lParam=m_pMultiDirs[i]->GetLParam(m_pBrowse,m_nMaxBrowse);
				LeaveCriticalSection(&m_cBrowse);
				RegKey.SetValue(szName,(DWORD)MAKELONG(HIWORD(lParam),LOWORD(lParam)));
			}


		}
	}


	// No subdirectories
	RegKey.SetValue("Name/NoSubDirectories",IsDlgButtonChecked(IDC_NOSUBDIRECTORIES));

}


BOOL CLocateDlg::CSizeDateDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	
	// Set spins
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

	CComboBox Min(GetDlgItem(IDC_MINTYPE)),Max(GetDlgItem(IDC_MAXTYPE));
	CStringW str;
	str.LoadString(IDS_MODIFIED);
	Min.AddString(str);
	Max.AddString(str);
	str.LoadString(IDS_CREATED);
	Min.AddString(str);
	Max.AddString(str);
	str.LoadString(IDS_LASTACCESSED);
	Min.AddString(str);
	Max.AddString(str);
	
	Min.AssignToDlgItem(*this,IDC_MINSIZETYPE);
	Max.AssignToDlgItem(*this,IDC_MAXSIZETYPE);
	str.LoadString(IDS_SIZEBYTES);
	Min.AddString(str);
	Max.AddString(str);
	str.LoadString(IDS_SIZEKB);
	Min.AddString(str);
	Max.AddString(str);
	str.LoadString(IDS_SIZEMB);
	Min.AddString(str);
	Max.AddString(str);
	
	OnClear(TRUE);
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

			HilightTab(TRUE);
		}
		else
		{
			EnableDlgItem(IDC_MINIMUMSIZE,FALSE);
			EnableDlgItem(IDC_MINIMUMSIZESPIN,FALSE);
			EnableDlgItem(IDC_MINSIZETYPE,FALSE);

			if (hControl==NULL && wNotifyCode==1) // Accelerator
				SetFocus(IDC_CHECKMINIMUMSIZE);

			HilightTab(IsChanged());
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
			HilightTab(TRUE);
		
		}
		else
		{
			EnableDlgItem(IDC_MAXIMUMSIZE,FALSE);
			EnableDlgItem(IDC_MAXIMUMSIZESPIN,FALSE);
			EnableDlgItem(IDC_MAXSIZETYPE,FALSE);

			if (hControl==NULL && wNotifyCode==1) // Accelerator
				SetFocus(IDC_CHECKMAXIMUMSIZE);

			HilightTab(IsChanged());
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
			
			HilightTab(TRUE);
		}
		else
		{
			EnableDlgItem(IDC_MINDATE,FALSE);
			EnableDlgItem(IDC_MINTYPE,FALSE);
			
			if (hControl==NULL && wNotifyCode==1) // Accelerator
				SetFocus(IDC_CHECKMINDATE);
			HilightTab(IsChanged());
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
			HilightTab(TRUE);
		}
		else
		{
			EnableDlgItem(IDC_MAXDATE,FALSE);
			EnableDlgItem(IDC_MAXTYPE,FALSE);

			if (hControl==NULL && wNotifyCode==1) // Accelerator
				SetFocus(IDC_CHECKMAXDATE);
			HilightTab(IsChanged());
		}
		break;
	case IDC_MINIMUMSIZE:
	case IDC_MAXIMUMSIZE:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
			SetFocus(wID);
		else if (wNotifyCode==EN_SETFOCUS)
			::SendMessage(hControl,EM_SETSEL,0,MAKELPARAM(0,-1));
		break;
	case IDC_MAXDATEMODE:
	case IDC_MINDATEMODE:
		{
			CDateTimeCtrlEx* pCtrl=(CDateTimeCtrlEx*)SendDlgItemMessage(
				wID==IDC_MAXDATEMODE?IDC_MAXDATE:IDC_MINDATE,DTMX_GETCLASS);
			if (pCtrl->IsWindowEnabled())
				pCtrl->ChangeMode(!pCtrl->GetMode());
			break;
		}
	case IDC_MINSIZETYPE:
	case IDC_MAXSIZETYPE:
	case IDC_MINDATE:
	case IDC_MAXDATE:
	case IDC_MINTYPE:
	case IDC_MAXTYPE:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
			SetFocus(wID);
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

	if (IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE) &&
		GetDlgItemTextLength(IDC_MINIMUMSIZE)>0)
	{
		BOOL bError;
		dwMinSize=(DWORD)SendDlgItemMessage(IDC_MINIMUMSIZESPIN,UDM_GETPOS32,0,(LPARAM)&bError);
		if (bError)
		{
			dwMinSize=GetDlgItemInt(IDC_MINIMUMSIZE,&bError,FALSE);
			if (!bError)
				dwMinSize=DWORD(-1);
		}
		
		if (dwMinSize!=DWORD(-1))
		{
			int nCurSel=(int)SendDlgItemMessage(IDC_MINSIZETYPE,CB_GETCURSEL,0,0);
			if (nCurSel)
				dwMinSize*=1024;
			if (nCurSel==2)
				dwMinSize*=1024;
		}
	}
	if (IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE) && 
		GetDlgItemTextLength(IDC_MAXIMUMSIZE)>0)
	{
		BOOL bError;
		dwMaxSize=(DWORD)SendDlgItemMessage(IDC_MAXIMUMSIZESPIN,UDM_GETPOS32,0,(LPARAM)&bError);
		if (bError)
		{
			dwMaxSize=GetDlgItemInt(IDC_MAXIMUMSIZE,&bError,FALSE);
			if (!bError)
				dwMaxSize=DWORD(-1);
		}

		if (dwMaxSize!=DWORD(-1))
		{
			int nCurSel=(int)SendDlgItemMessage(IDC_MAXSIZETYPE,CB_GETCURSEL,0,0);
			if (nCurSel)
				dwMaxSize*=1024;
			if (nCurSel==2)
				dwMaxSize*=1024;
		}
	}
	if (IsDlgButtonChecked(IDC_CHECKMINDATE))
	{
		SYSTEMTIME st;
		SendDlgItemMessage(IDC_MINDATE,DTM_GETSYSTEMTIME,0,(LPARAM)&st);
		wMinDate=SYSTEMTIMETODOSDATE(st);
		int nCurSel=(int)SendDlgItemMessage(IDC_MINTYPE,CB_GETCURSEL,0,0);
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
		int nCurSel=(int)SendDlgItemMessage(IDC_MAXTYPE,CB_GETCURSEL,0,0);
		if (nCurSel==1)
			dwFlags|=LOCATE_MAXCREATIONDATE;
		else if (nCurSel==2)
			dwFlags|=LOCATE_MAXACCESSDATE;
	}

	pLocater->SetSizeAndDate(dwFlags,dwMinSize,dwMaxSize,wMinDate,wMaxDate);
	
	DlgDebugMessage("CLocateDlg::CSizeDateDlg::OnOk END");
	return IsDlgButtonChecked(IDC_MATCHWHOLENAME);
}
	
void CLocateDlg::CSizeDateDlg::OnClear(BOOL bInitial)
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

	CDateTimeCtrlEx* pMinDateCtrl=CDateTimeCtrlEx::GetClass(GetDlgItem(IDC_MINDATE));
	CDateTimeCtrlEx* pMaxDateCtrl=CDateTimeCtrlEx::GetClass(GetDlgItem(IDC_MAXDATE));

	SYSTEMTIME st;
	GetLocalTime(&st);
	if (pMinDateCtrl!=NULL)
	{
		pMinDateCtrl->SetExplicitDate(&st,GDT_VALID|DTXF_NOMODECHANGE);
		pMinDateCtrl->SetRelativeDate(0,DTXF_NOMODECHANGE);
	}
	if (pMaxDateCtrl!=NULL)
	{
		pMaxDateCtrl->SetExplicitDate(&st,GDT_VALID|DTXF_NOMODECHANGE);
		pMaxDateCtrl->SetRelativeDate(0,DTXF_NOMODECHANGE);
	}

	if (bInitial)
	{
		DWORD dwMinType=0,dwMaxType=0,dwMinSizeType=0,dwMaxSizeType=0;
		CRegKey2 RegKey;
		if(RegKey.OpenKey(HKCU,"\\Dialogs",CRegKey::defRead)==ERROR_SUCCESS)
		{
			RegKey.QueryValue("SizeDate/MinimumSizeType",dwMinSizeType);
			RegKey.QueryValue("SizeDate/MaximumSizeType",dwMaxSizeType);
			
			char szTemp[sizeof(SYSTEMTIME)+4];
			DWORD dwLength=RegKey.QueryValue("SizeDate/MaximumDate",szTemp,sizeof(SYSTEMTIME)+4);
			if (dwLength>=sizeof(DWORD))
			{
				dwMaxType=LOWORD(*((DWORD*)szTemp));
				if (pMaxDateCtrl!=NULL)
					pMaxDateCtrl->ChangeMode(HIWORD(*((DWORD*)szTemp)));

			}

			dwLength=RegKey.QueryValue("SizeDate/MinimumDate",szTemp,sizeof(SYSTEMTIME)+4);
			if (dwLength>=sizeof(DWORD))
			{
				dwMinType=LOWORD(*((DWORD*)szTemp));
				if (pMinDateCtrl!=NULL)
					pMinDateCtrl->ChangeMode(HIWORD(*((DWORD*)szTemp)));
			}
		}
		
		SendDlgItemMessage(IDC_MINTYPE,CB_SETCURSEL,dwMinType,0);
		SendDlgItemMessage(IDC_MAXTYPE,CB_SETCURSEL,dwMaxType,0);
		SendDlgItemMessage(IDC_MINSIZETYPE,CB_SETCURSEL,dwMinSizeType,0);
		SendDlgItemMessage(IDC_MAXSIZETYPE,CB_SETCURSEL,dwMaxSizeType,0);
		
	}


	HilightTab(FALSE);
}

void CLocateDlg::CSizeDateDlg::EnableItems(BOOL bEnable_)
{
	EnableDlgItem(IDC_CHECKMINIMUMSIZE,bEnable_);
	BOOL bEnable=bEnable_ && IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE);
	EnableDlgItem(IDC_MINIMUMSIZE,bEnable);
	EnableDlgItem(IDC_MINIMUMSIZESPIN,bEnable);
	EnableDlgItem(IDC_MINSIZETYPE,bEnable);
	
	EnableDlgItem(IDC_CHECKMAXIMUMSIZE,bEnable_);
	bEnable=bEnable_ && IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE);
	EnableDlgItem(IDC_MAXIMUMSIZE,bEnable);
	EnableDlgItem(IDC_MAXIMUMSIZESPIN,bEnable);
	EnableDlgItem(IDC_MAXSIZETYPE,bEnable);
	
	EnableDlgItem(IDC_CHECKMINDATE,bEnable_);
	bEnable=bEnable_ && IsDlgButtonChecked(IDC_CHECKMINDATE);
	EnableDlgItem(IDC_MINDATE,bEnable);
	EnableDlgItem(IDC_MINTYPE,bEnable);
	
	EnableDlgItem(IDC_CHECKMAXDATE,bEnable_);
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
		if (pStartData->m_dwMinDate&(1<<31))
		{
			st.wDay=WORD(pStartData->m_dwMinDate);
			st.wMonth=0xFFFF;
			st.wYear=0xFFFF;
		}
		else
		{			
			st.wDay=BYTE(pStartData->m_dwMinDate);
			st.wMonth=BYTE(pStartData->m_dwMinDate>>8);
			st.wYear=WORD(pStartData->m_dwMinDate>>16);
		}
		SendDlgItemMessage(IDC_MINDATE,DTM_SETSYSTEMTIME,GDT_VALID,LPARAM(&st));

		switch (pStartData->m_cMinDateType)
		{
		case 'C':
		case 'c':
			SendDlgItemMessage(IDC_MINTYPE,CB_SETCURSEL,1);
			break;
		case 'A':
		case 'a':
			SendDlgItemMessage(IDC_MINTYPE,CB_SETCURSEL,2);
			break;
		default:
			SendDlgItemMessage(IDC_MINTYPE,CB_SETCURSEL,0);
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
		
		if (pStartData->m_dwMaxDate&(1<<31))
		{
			st.wDay=WORD(pStartData->m_dwMaxDate);
			st.wMonth=0xFFFF;
			st.wYear=0xFFFF;
		}
		else
		{			
			st.wDay=BYTE(pStartData->m_dwMaxDate);
			st.wMonth=BYTE(pStartData->m_dwMaxDate>>8);
			st.wYear=WORD(pStartData->m_dwMaxDate>>16);
		}
		SendDlgItemMessage(IDC_MAXDATE,DTM_SETSYSTEMTIME,GDT_VALID,LPARAM(&st));

		switch (pStartData->m_cMaxDateType)
		{
		case 'C':
		case 'c':
			SendDlgItemMessage(IDC_MAXTYPE,CB_SETCURSEL,1);
			break;
		case 'a':
		case 'A':
			SendDlgItemMessage(IDC_MAXTYPE,CB_SETCURSEL,2);
			break;
		default:
			SendDlgItemMessage(IDC_MAXTYPE,CB_SETCURSEL,0);
			break;
		}
	}

	EnableItems(TRUE);
}

void CLocateDlg::CSizeDateDlg::LoadControlStates(CRegKey& RegKey,BOOL bPreset)
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
    DWORD dwType;
	DWORD dwLen=RegKey.QueryValue("SizeDate/MinimumDate",szData,sizeof(SYSTEMTIME)+4,&dwType);
	if (dwLen>=sizeof(SYSTEMTIME)+4 && dwType==REG_BINARY)
	{
		CheckDlgButton(IDC_CHECKMINDATE,TRUE);
		SendDlgItemMessage(IDC_MINDATE,DTM_SETSYSTEMTIME,0,(LPARAM)(szData+4));
		SendDlgItemMessage(IDC_MINTYPE,CB_SETCURSEL,LOWORD(*((LONG*)szData)),0);
	}
	else 
	{
		if (!bPreset)
		{
			if (dwType==REG_DWORD && dwLen>=sizeof(DWORD))
			{
				SendDlgItemMessage(IDC_MINTYPE,CB_SETCURSEL,LOWORD(*((DWORD*)szData)),0);
				SendDlgItemMessage(IDC_MINDATE,DTMX_CHANGEMODE,HIWORD(*((DWORD*)szData)),0);
			}
			else
				SendDlgItemMessage(IDC_MINDATE,DTMX_CHANGEMODE,0,0);
		}
		CheckDlgButton(IDC_CHECKMINDATE,FALSE);
	}
		
	dwLen=RegKey.QueryValue("SizeDate/MaximumDate",szData,sizeof(SYSTEMTIME)+4,&dwType);
	if (dwLen>=sizeof(SYSTEMTIME)+4 && dwType==REG_BINARY)
	{
		CheckDlgButton(IDC_CHECKMAXDATE,TRUE);
		SendDlgItemMessage(IDC_MAXDATE,DTM_SETSYSTEMTIME,0,(LPARAM)(szData+4));
		SendDlgItemMessage(IDC_MAXTYPE,CB_SETCURSEL,LOWORD(*((LONG*)szData)),0);
	}
	else 
	{
		if (!bPreset)
		{
			if (dwType==REG_DWORD && dwLen>=sizeof(DWORD))
			{
				SendDlgItemMessage(IDC_MAXTYPE,CB_SETCURSEL,LOWORD(*((DWORD*)szData)),0);
				SendDlgItemMessage(IDC_MAXDATE,DTMX_CHANGEMODE,HIWORD(*((DWORD*)szData)),0);
			}
			else
				SendDlgItemMessage(IDC_MAXDATE,DTMX_CHANGEMODE,0,0);
		}
		CheckDlgButton(IDC_CHECKMAXDATE,FALSE);
	}
		
	HilightTab(IsChanged());
}

void CLocateDlg::CSizeDateDlg::SaveControlStates(CRegKey& RegKey)
{
	if (IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE))
	{
		BOOL bError;
		DWORD dwMinSize=(DWORD)SendDlgItemMessage(IDC_MINIMUMSIZESPIN,UDM_GETPOS32,0,(LPARAM)&bError);
		if (bError)
		{
			dwMinSize=GetDlgItemInt(IDC_MINIMUMSIZE,&bError,FALSE);
			if (!bError)
				dwMinSize=0;
		}
		RegKey.SetValue("SizeDate/MinimumSize",dwMinSize);
	}
	else
		RegKey.DeleteValue("SizeDate/MinimumSize");
	RegKey.SetValue("SizeDate/MinimumSizeType",DWORD(SendDlgItemMessage(IDC_MINSIZETYPE,CB_GETCURSEL,0,0)));

	if (IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE))
	{
		BOOL bError;
		DWORD dwMaxSize=(DWORD)SendDlgItemMessage(IDC_MAXIMUMSIZESPIN,UDM_GETPOS32,0,(LPARAM)&bError);
		if (bError)
		{
			dwMaxSize=GetDlgItemInt(IDC_MAXIMUMSIZE,&bError,FALSE);
			if (!bError)
				dwMaxSize=0;
		}
		RegKey.SetValue("SizeDate/MaximumSize",dwMaxSize);
	}
	else
		RegKey.DeleteValue("SizeDate/MaximumSize");
	RegKey.SetValue("SizeDate/MaximumSizeType",DWORD(SendDlgItemMessage(IDC_MAXSIZETYPE,CB_GETCURSEL,0,0)));
	
	if (IsDlgButtonChecked(IDC_CHECKMINDATE))
	{
		char szTemp[sizeof(SYSTEMTIME)+4];
		SendDlgItemMessage(IDC_MINDATE,DTM_GETSYSTEMTIME,DTXF_FORSAVE,(LPARAM)(szTemp+4));
		*((LONG*)szTemp)=MAKELONG(SendDlgItemMessage(IDC_MINTYPE,CB_GETCURSEL),
			SendDlgItemMessage(IDC_MINDATE,DTMX_GETMODE));
		RegKey.SetValue("SizeDate/MinimumDate",szTemp,sizeof(SYSTEMTIME)+4,REG_BINARY);
	}
	else
	{
		RegKey.SetValue("SizeDate/MinimumDate",
			(DWORD)MAKELONG(SendDlgItemMessage(IDC_MINTYPE,CB_GETCURSEL),
			SendDlgItemMessage(IDC_MINDATE,DTMX_GETMODE)));
	}

	if (IsDlgButtonChecked(IDC_CHECKMAXDATE))
	{
		char szTemp[sizeof(SYSTEMTIME)+4];
		SendDlgItemMessage(IDC_MAXDATE,DTM_GETSYSTEMTIME,DTXF_FORSAVE,(LPARAM)(szTemp+4));
		*((LONG*)szTemp)=MAKELONG(SendDlgItemMessage(IDC_MAXTYPE,CB_GETCURSEL),
			SendDlgItemMessage(IDC_MAXDATE,DTMX_GETMODE));
		RegKey.SetValue("SizeDate/MaximumDate",szTemp,sizeof(SYSTEMTIME)+4,REG_BINARY);
	}
	else
	{
		RegKey.SetValue("SizeDate/MaximumDate",
			(DWORD)MAKELONG(SendDlgItemMessage(IDC_MAXTYPE,CB_GETCURSEL),
			SendDlgItemMessage(IDC_MAXDATE,DTMX_GETMODE)));
	}
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
	CComboBox Check(GetDlgItem(IDC_CHECK));
	Check.AddString(ID2W(IDS_FILENAMESONLY));
	Check.AddString(ID2W(IDS_FILEANDFOLDERNAMES));
	Check.AddString(ID2W(IDS_FOLDERNAMESONLY));
	

	// Adding (by extension)
	SendDlgItemMessage(IDC_FILETYPE,CB_ADDSTRING,0,LPARAM(szwEmpty));
	OnClear(TRUE);
	
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

	DebugMessage("CAdvancedDlg::AddBuildInFileTypes() BEGIN");

	if (m_dwFlags&fgBuildInTypesAdded)
		return;

	CStringW strTypes;
	CRegKey2 RegKey;
	DWORD bRet=FALSE;

	// Loading buildin types from registry if needed	
	if (RegKey.OpenKey(HKCU,"\\Locate",
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue(L"Load Buildin Types",bRet);
		if (bRet)
			RegKey.QueryValue(L"Buildin Types",strTypes);
	}

	// Saving buildin types to registry
	if (strTypes.IsEmpty())
	{
		strTypes.LoadString(IDS_BUILDINTYPES);

        if (RegKey.OpenKey(HKCU,"\\Locate",
			CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			if (!bRet)		
				RegKey.SetValue(L"Load Buildin Types",DWORD(0));
			RegKey.SetValue(L"Buildin Types",strTypes);
		}			
	}
	RegKey.CloseKey();

	// This is not very best way to do this
	strTypes.ReplaceChars(L'|',L'\0');
	LPCWSTR pPtr=strTypes;
	CImageList il;
	il.Create(IDB_DEFAULTTYPEICONS,16,0,RGB(255,0,255));
	while (*pPtr!=L'\0')
		SendDlgItemMessage(IDC_FILETYPE,CB_ADDSTRING,0,LPARAM(new FileType(pPtr,il)));
	il.DeleteImageList();

	m_dwFlags|=fgBuildInTypesAdded;

	DebugMessage("CAdvancedDlg::AddBuildInFileTypes() END");
}

LRESULT CLocateDlg::CAdvancedDlg::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	if (msg==WM_COMPAREITEM && wParam==IDC_FILETYPE)
	{
		FileType* ft1=((FileType*)((COMPAREITEMSTRUCT*)lParam)->itemData1);
		FileType* ft2=((FileType*)((COMPAREITEMSTRUCT*)lParam)->itemData2);

		if (ft1==NULL || ft1==(FileType*)szwEmpty)
			return -1;
		if (ft2==NULL || ft2==(FileType*)szwEmpty)
			return 1;
		if (ft1->szType==NULL)
		{
			if (ft2->szType!=NULL)
				return -1;
		}
		else if (ft2->szType==NULL) // ft2 is buildin
			return 1;

		return _wcsicmp(
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
			LPCSTR pStr=(LPCSTR)LockResource(hGlobal);

			// Counting length
			int len;
			for (len=0;pStr[len]!='\0';len++)
			{
				if (pStr[len]=='E' && pStr[len+1]=='O' && pStr[len+2]=='F')
					break;
			}


			MessageBox(CString(pStr,len),ID2A(IDS_HELPINFO),MB_OK|MB_ICONINFORMATION);
			
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
			HilightTab(TRUE);		
		}
		else
		{
			EnableDlgItem(IDC_DATAMATCHCASE,FALSE);
			EnableDlgItem(IDC_CONTAINDATA,FALSE);
			EnableDlgItem(IDC_HELPTOOLBAR,FALSE);

			if (hControl==NULL && wNotifyCode==1) // Accelerator
				SetFocus(IDC_CONTAINDATACHECK);

			HilightTab(IsChanged());
		}
		ChangeEnableStateForCheck();
		break;
	case IDC_USEWHOLEPATH:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
		{
			CheckDlgButton(wID,!IsDlgButtonChecked(wID));
			SetFocus(wID);
		}
		if (IsDlgButtonChecked(IDC_USEWHOLEPATH))
		{
			SendDlgItemMessage(IDC_FILETYPE,CB_SETCURSEL,0);
			EnableDlgItem(IDC_FILETYPE,FALSE);
			
			CLocateDlg* pLocateDlg=GetLocateDlg();
			pLocateDlg->m_NameDlg.m_Type.EnableWindow(FALSE);
			pLocateDlg->m_NameDlg.m_Type.SetText(szEmpty);
			
			ChangeEnableStateForCheck();
		}
		else
		{
			EnableDlgItem(IDC_FILETYPE,TRUE);
			GetLocateDlg()->m_NameDlg.m_Type.EnableWindow(TRUE);
		}
		HilightTab(IsChanged());
		break;
	case IDC_FILETYPE:
		DebugFormatMessage("IDC_FILETYPE, wNotifuCode=%d, hControl=%X this=%X",wNotifyCode,hControl,this);
		switch (wNotifyCode)
		{
		case 1: // Accel or CBN_SELCHANGE
			if (hControl==NULL)
			{
				UpdateTypeList();
				SetFocus(IDC_FILETYPE);
			}
			else
			{
				CComboBox FileTypeCombo(GetDlgItem(IDC_FILETYPE));
				FileType* ft=NULL;
				int nSel=FileTypeCombo.GetCurSel();
				if (nSel!=CB_ERR)
				{
					ft=(FileType*)FileTypeCombo.GetItemData(nSel);
					if (ft==NULL || ft==(FileType*)szwEmpty)
					{
						ft=NULL;
						if (!GetLocateDlg()->m_NameDlg.m_Type.IsWindowEnabled())
							GetLocateDlg()->m_NameDlg.m_Type.SetText("");
					}
					else
					{
						WCHAR* pEx=new WCHAR[max(ft->dwExtensionLength,2)];
						MemCopyW(pEx,ft->szExtensions,ft->dwExtensionLength);
						for (int i=0;pEx[i]!=L'\0' || pEx[i+1]!=L'\0';i++)
						{
							if (pEx[i]==L'\0')
								pEx[i]=L' ';
						}
						GetLocateDlg()->m_NameDlg.m_Type.SetText(pEx);
						delete[] pEx;
					}
					
				}
				GetLocateDlg()->m_NameDlg.m_Type.EnableWindow(ft==NULL);
				ChangeEnableStateForCheck();
			
			}
			HilightTab(IsChanged());
			break;
		case CBN_DROPDOWN:
			UpdateTypeList();
			break;
		case CBN_CLOSEUP:
			if (m_hTypeUpdaterThread==NULL)
			{
				int nSelected=(int)SendDlgItemMessage(IDC_FILETYPE,CB_GETCURSEL);

				// Destroying unnecessary icons
				for (int i=(int)SendDlgItemMessage(IDC_FILETYPE,CB_GETCOUNT)-1;i>0;i--)
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
		switch (wNotifyCode)
		{
		case CBN_SELCHANGE:
			if (hControl==NULL && wNotifyCode==1 && IsDlgItemEnabled(IDC_CHECK)) // Accelerator
				SetFocus(IDC_CHECK);
			else
				HilightTab(IsChanged());
			break;
		}
		break;
	case IDC_REPLACESPACES:
	case IDC_MATCHWHOLENAME:
		if (hControl==NULL && wNotifyCode==1) // Accelerator
		{
			CheckDlgButton(wID,!IsDlgButtonChecked(wID));
			SetFocus(wID);
		}
		HilightTab(IsChanged());
		break;
	case IDC_DATAMATCHCASE:
		if (hControl==NULL && wNotifyCode==1 && IsDlgButtonChecked(IDC_CONTAINDATACHECK)) // Accelerator
		{
			CheckDlgButton(IDC_DATAMATCHCASE,!IsDlgButtonChecked(IDC_DATAMATCHCASE));
			SetFocus(IDC_DATAMATCHCASE);
		}
		HilightTab(IsChanged());
		break;

	}
	
	return CDialog::OnCommand(wID,wNotifyCode,hControl);
}

BOOL CLocateDlg::CAdvancedDlg::IsChanged()
{
	DWORD dwDefaultCheck=0,dwDefaultMatchWholeName=0,dwDefaultReplaceSpaces=0,dwDefaultUseWholePath=0;

	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("Default CheckIn",dwDefaultCheck);
		RegKey.QueryValue("Default MatchWholeName",dwDefaultMatchWholeName);
		RegKey.QueryValue("Default ReplaceSpaces",dwDefaultReplaceSpaces);
		RegKey.QueryValue("Default UseWholePath",dwDefaultUseWholePath);
		RegKey.CloseKey();
	}

	if (IsDlgItemEnabled(IDC_CHECK))
	{
		if (SendDlgItemMessage(IDC_CHECK,CB_GETCURSEL)!=dwDefaultCheck)
			return TRUE;
	}

	if (IsDlgButtonChecked(IDC_MATCHWHOLENAME)!=dwDefaultMatchWholeName)
		return TRUE;

	if (IsDlgButtonChecked(IDC_REPLACESPACES)!=dwDefaultReplaceSpaces)
		return TRUE;

	if (IsDlgButtonChecked(IDC_USEWHOLEPATH)!=dwDefaultUseWholePath)
		return TRUE;

	if (IsDlgButtonChecked(IDC_CONTAINDATACHECK))
		return TRUE;

	if (IsDlgItemEnabled(IDC_FILETYPE))
	{
		if (SendDlgItemMessage(IDC_FILETYPE,CB_GETCURSEL)>0)
			return TRUE;
	}
	
	
	return FALSE;
}





void CLocateDlg::CAdvancedDlg::ReArrangeAllocatedData()
{
	//TODO: Buffered allocator does not work
	/*

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
	*/
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
	
	if (IsDlgItemEnabled(IDC_CHECK))
	{
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
	}
	else
		dwFlags=LOCATE_FILENAMES; // Assumption

	if (IsDlgButtonChecked(IDC_CONTAINDATACHECK))
	{
		if (IsDlgButtonChecked(IDC_DATAMATCHCASE))
			dwFlags|=LOCATE_CONTAINTEXTISMATCHCASE;
		CStringW str;
		GetDlgItemText(IDC_CONTAINDATA,str);
		
		
		DWORD dwDataLength;
		BYTE* pData=NULL;
		if (_wcsnicmp(str,L"regexp:",7)==0)
		{
			dwDataLength=str.GetLength()-7+1;
            if (dwDataLength>1)
			{
				pData=(BYTE*)malloc(dwDataLength+1);
				WideCharToMultiByte(CP_ACP,0,LPCWSTR(str)+7,dwDataLength,(LPSTR)pData,dwDataLength+1,NULL,NULL);
				dwFlags|=LOCATE_REGULAREXPRESSIONSEARCH;
			}
		}
		else
			pData=dataparser(str,&dwDataLength);
			

		pLocater->SetAdvanced(dwFlags,pData,dwDataLength,GetLocateDlg()->m_dwMaxFoundFiles==0?-1:GetLocateDlg()->m_dwMaxFoundFiles);
		if (pData!=NULL)
			free(pData);
	}
	else
		pLocater->SetAdvanced(dwFlags,NULL,0,GetLocateDlg()->m_dwMaxFoundFiles==0?-1:GetLocateDlg()->m_dwMaxFoundFiles);

	DlgDebugMessage("CLocateDlg::CAdvancedDlg::OnOk END");

	return (IsDlgButtonChecked(IDC_MATCHWHOLENAME)?flagMatchCase:0)|
		(IsDlgButtonChecked(IDC_USEWHOLEPATH)?flagUseWholePath:0)|
		(IsDlgButtonChecked(IDC_REPLACESPACES)?flagReplaceSpaces:0);

}

	
void CLocateDlg::CAdvancedDlg::OnClear(BOOL bInitial)
{
	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
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
		nTemp=0;
		RegKey.QueryValue("Default UseWholePath",nTemp);
		CheckDlgButton(IDC_USEWHOLEPATH,nTemp);
	}
	else
	{
		SendDlgItemMessage(IDC_CHECK,CB_SETCURSEL,1);
		CheckDlgButton(IDC_MATCHWHOLENAME,0);
		CheckDlgButton(IDC_DATAMATCHCASE,1);
		CheckDlgButton(IDC_REPLACESPACES,0);
		CheckDlgButton(IDC_USEWHOLEPATH,0);
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
	HilightTab(FALSE);
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
	if (pStartData->m_nStatus&CLocateApp::CStartData::statusUseWholePath)
		CheckDlgButton(IDC_USEWHOLEPATH,1);
	else if (pStartData->m_nStatus&CLocateApp::CStartData::statusNoUseWholePath)
		CheckDlgButton(IDC_USEWHOLEPATH,0);
	ChangeEnableStateForCheck();
}
void CLocateDlg::CAdvancedDlg::EnableItems(BOOL bEnable)
{
	EnableDlgItem(IDC_MATCHWHOLENAME,bEnable);
	EnableDlgItem(IDC_REPLACESPACES,bEnable);
	EnableDlgItem(IDC_USEWHOLEPATH,bEnable);


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
	
	int nCurSel=GetLocateDlg()->m_NameDlg.m_Type.GetCurSel();
	if (nCurSel==0)
		bEnable=FALSE; // (none)
	else if (nCurSel==CB_ERR)
	{
		if (GetLocateDlg()->m_NameDlg.m_Type.GetTextLength()>0)
			bEnable=FALSE;
	}
	else
	{
		if (GetLocateDlg()->m_NameDlg.m_Type.GetTextLength()>0)
			bEnable=FALSE;
	}
	
	if (bEnable)
		EnableDlgItem(IDC_CHECK,TRUE);
	else
	{
		//SendDlgItemMessage(IDC_CHECK,CB_SETCURSEL,0);
		EnableDlgItem(IDC_CHECK,FALSE);
	}
}

void CLocateDlg::CAdvancedDlg::OnDrawItem(UINT idCtl,LPDRAWITEMSTRUCT lpdis)
{
	CDialog::OnDrawItem(idCtl,lpdis);

	if (idCtl==IDC_FILETYPE && lpdis->itemID!=CB_ERR)
	{
		CRect rc(lpdis->rcItem);
		CDC dc(lpdis->hDC);
		HBRUSH hHighLight=GetSysColorBrush(COLOR_HIGHLIGHT);
		CStringW Text;
		
		// Drawing background
		if (lpdis->itemState&ODS_DISABLED)
			dc.FillRect(&lpdis->rcItem,GetSysColorBrush(COLOR_3DFACE));
		else
			dc.FillRect(&lpdis->rcItem,GetSysColorBrush(COLOR_WINDOW));
		
		if (lpdis->itemID==0)
		{
			ASSERT(lpdis->itemData==LPARAM(szwEmpty));
			Text.LoadString(IDS_BYEXTENSION);
		}
		else
		{
			Text=((FileType*)lpdis->itemData)->szTitle;
			if (((FileType*)lpdis->itemData)->hIcon==NULL)
				((FileType*)lpdis->itemData)->ExtractIconFromPath();

			dc.DrawIcon(rc.left,rc.top,((FileType*)lpdis->itemData)->hIcon,
				16,16,0,lpdis->itemAction&ODA_FOCUS?hHighLight:NULL,DI_NORMAL);
		}
		
		dc.SetBkMode(TRANSPARENT);
		rc.left+=18;
				
		if (lpdis->itemState&ODS_DISABLED)
			dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
		else if (lpdis->itemAction&ODA_FOCUS ||
			lpdis->itemState&ODS_SELECTED)
		{
			// Filling text shade
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			CSize sz=dc.GetTextExtent(Text);
			dc.FillRect(&CRect(rc.left,rc.top+1,rc.left+sz.cx+1,rc.bottom-1),hHighLight);

			if (lpdis->itemState&ODS_FOCUS && !(lpdis->itemState&ODS_NOFOCUSRECT))
				dc.DrawFocusRect(&CRect(rc.left,rc.top+1,rc.left+sz.cx+1,rc.bottom-1));
		}
		else 
			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			
		dc.DrawText(Text,Text.GetLength(),&rc,DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	}
}

void CLocateDlg::CAdvancedDlg::OnMeasureItem(int nIDCtl,LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	CDialog::OnMeasureItem(nIDCtl,lpMeasureItemStruct);
		
	if (nIDCtl==IDC_FILETYPE)
	{
		CDC dc(&CWnd(GetDlgItem(IDC_FILETYPE)));
		
		CSize sz;
		if (lpMeasureItemStruct->itemID==0)
			sz=dc.GetTextExtent(CStringW(IDS_BYEXTENSION));
		else
			sz=dc.GetTextExtent(((FileType*)lpMeasureItemStruct->itemData)->szTitle,istrlenw(((FileType*)lpMeasureItemStruct->itemData)->szTitle));
		lpMeasureItemStruct->itemHeight=max(sz.cy+1,16);
		lpMeasureItemStruct->itemWidth=sz.cx+18;
	}
}

void CLocateDlg::CAdvancedDlg::OnDestroy()
{
	CDialog::OnDestroy();
	if (m_hTypeUpdaterThread!=NULL)
	{
		TerminateThread(m_hTypeUpdaterThread,0,TRUE);
		CloseHandle(m_hTypeUpdaterThread);
		DebugCloseThread(m_hTypeUpdaterThread);
		m_hTypeUpdaterThread=NULL;
	}

	for (int i=(int)SendDlgItemMessage(IDC_FILETYPE,CB_GETCOUNT)-1;i>0;i--)
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
	CStringW str;

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

	if (!RegKey.QueryValue("Advanced/UseWholePath",dwTemp))
		dwTemp=0;
    CheckDlgButton(IDC_USEWHOLEPATH,dwTemp);

	if (!RegKey.QueryValue("Advanced/TextIsMatchCase",dwTemp))
		dwTemp=1;	
	CheckDlgButton(IDC_DATAMATCHCASE,dwTemp);
	
	if (RegKey.QueryValue(L"Advanced/ContainData",str))
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
	BYTE* pData=new BYTE[dwLength];
	RegKey.QueryValue("Advanced/TypeOfFile",(LPSTR)pData,dwLength,&dwType);
	if (dwType==REG_DWORD && dwLength==sizeof(DWORD))
	{
		if (*((int*)pData)>0)
		{
			AddBuildInFileTypes();
			SendDlgItemMessage(IDC_FILETYPE,CB_SETCURSEL,*((DWORD*)pData));
			GetLocateDlg()->m_NameDlg.m_Type.EnableWindow(FALSE);
		}
	}
	else if (dwType==REG_BINARY)
	{
		AddBuildInFileTypes();
		int nItem=AddTypeToList(pData);
		if (nItem>=0)
		{
			SendDlgItemMessage(IDC_FILETYPE,CB_SETCURSEL,nItem);
			GetLocateDlg()->m_NameDlg.m_Type.EnableWindow(FALSE);
		}
	}
	delete[] pData;

	HilightTab(IsChanged());
}

void CLocateDlg::CAdvancedDlg::SaveControlStates(CRegKey& RegKey)
{
	// Advanced dialog
	RegKey.SetValue("Advanced/Check",(DWORD)SendDlgItemMessage(IDC_CHECK,CB_GETCURSEL));
	RegKey.SetValue("Advanced/MatchWholeName",IsDlgButtonChecked(IDC_MATCHWHOLENAME));
	RegKey.SetValue("Advanced/ReplaceSpaces",IsDlgButtonChecked(IDC_REPLACESPACES));
	RegKey.SetValue("Advanced/UseWholePath",IsDlgButtonChecked(IDC_USEWHOLEPATH));
	if (IsDlgButtonChecked(IDC_CONTAINDATACHECK))
	{
		CStringW str;
		GetDlgItemText(IDC_CONTAINDATA,str);
		RegKey.SetValue(L"Advanced/ContainData",str);
	}
	else
		RegKey.DeleteValue("Advanced/ContainData");
	RegKey.SetValue("Advanced/TextIsMatchCase",IsDlgButtonChecked(IDC_DATAMATCHCASE));

	// Type box
	int nCurSel=(int)SendDlgItemMessage(IDC_FILETYPE,CB_GETCURSEL);
	FileType* pType=(FileType*)SendDlgItemMessage(IDC_FILETYPE,CB_GETITEMDATA,nCurSel);
	if (pType==NULL || pType==(CAdvancedDlg::FileType*)szwEmpty || pType->szType==NULL)
		RegKey.SetValue("Advanced/TypeOfFile",DWORD(nCurSel));
	else
	{
		DWORD dwLength=istrlenw(pType->szType);
		WCHAR* szData=new WCHAR[++dwLength+pType->dwExtensionLength];
		MemCopyW(szData,pType->szType,dwLength);
		MemCopyW(szData+dwLength,pType->szExtensions,pType->dwExtensionLength);
        RegKey.SetValue("Advanced/TypeOfFile",(LPCSTR)szData,(dwLength+pType->dwExtensionLength)*2,REG_BINARY);
		delete[] szData;
	}
}

void CLocateDlg::CAdvancedDlg::UpdateTypeList()
{
	if (m_hTypeUpdaterThread!=NULL) // Already running
		return;

	AddBuildInFileTypes();

	if (!(m_dwFlags&fgOtherTypeAdded) && GetLocateDlg()->m_dwFlags&CLocateDlg::fgLoadRegistryTypes)
	{
		DWORD dwID;
		m_hTypeUpdaterThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)UpdaterProc,this,0,&dwID);
		DebugOpenThread(m_hTypeUpdaterThread);

		if (m_hTypeUpdaterThread==NULL)
			DebugFormatMessage("m_hTypeUpdaterThread==NULL, last error=%d",GetLastError());
	}
}

int CLocateDlg::CAdvancedDlg::AddTypeToList(LPCWSTR szKey,CArray<FileType*>& aFileTypes)
{
	DebugFormatMessage(L"CAdvancedDlg::AddTypeToList(szKey=%s) ",szKey);

	CRegKey RegKey;
	if (RegKey.OpenKey(HKCR,szKey,CRegKey::openExist|CRegKey::samQueryValue|CRegKey::samExecute)!=ERROR_SUCCESS)
		return CB_ERR;

	CStringW sType,sTitle;
	if (!RegKey.QueryValue(L"",sType))
		return CB_ERR;
	sType.MakeLower();

	for (int i=0;i<aFileTypes.GetSize();i++)
	{
		FileType* pType=aFileTypes.GetAt(i);
		if (sType.Compare(pType->szType)==0)
		{
			pType->AddExtension(szKey+1,istrlenw(szKey));
			DebugMessage("AddTypeToList: 1err");
			return -2;
		}
	}

	if (RegKey.OpenKey(HKCR,sType,CRegKey::openExist|CRegKey::samQueryValue|CRegKey::samExecute)!=ERROR_SUCCESS)
	{
		DebugMessage("AddTypeToList: 3err");
		return CB_ERR;
	}

	
	if (!RegKey.QueryValue(L"",sTitle))
		return CB_ERR;
	if (sTitle.IsEmpty())
		return CB_ERR;

	FileType* pType=new FileType(alloccopy(sType,sType.GetLength()),alloccopy(sTitle,sTitle.GetLength()));
	pType->AddExtension(szKey+1,istrlenw(szKey));
	aFileTypes.Add(pType);
	pType->SetIcon(RegKey);

	
	return (int)SendDlgItemMessage(IDC_FILETYPE,CB_ADDSTRING,0,LPARAM(pType));
}

int CLocateDlg::CAdvancedDlg::AddTypeToList(BYTE* pTypeAndExtensions)
{
	CRegKey RegKey;
	
	DWORD dwLength=istrlenw((LPCWSTR)pTypeAndExtensions);
	if (dwLength==0)
		return CB_ERR;
	WCHAR* szType=new WCHAR[++dwLength];
	MemCopyW(szType,(LPCWSTR)pTypeAndExtensions,dwLength);
	pTypeAndExtensions+=dwLength*2;
	
	if (RegKey.OpenKey(HKCR,szType,CRegKey::openExist|CRegKey::samQueryValue|CRegKey::samExecute)!=ERROR_SUCCESS)
	{
		delete[] szType;
		return CB_ERR;
	}
	
	CStringW sTitle;
	if (!RegKey.QueryValue(L"",sTitle))
	{
		delete szType;
		return CB_ERR;
	}
	
	FileType* pFileType=new FileType(szType,sTitle.GiveBuffer());
	while (*pTypeAndExtensions!=L'\0')
	{
        dwLength=istrlenw((LPCWSTR)pTypeAndExtensions);
		pFileType->AddExtension((LPCWSTR)pTypeAndExtensions,dwLength++);
		pTypeAndExtensions+=dwLength*2;
	}
	
	pFileType->SetIcon(RegKey);

	return (int)SendDlgItemMessage(IDC_FILETYPE,CB_ADDSTRING,0,LPARAM(pFileType));
}

DWORD WINAPI CLocateDlg::CAdvancedDlg::UpdaterProc(CLocateDlg::CAdvancedDlg* pAdvancedDlg)
{
	DebugMessage("CLocateDlg::CAdvancedDlg::UpdaterProc BEGIN");

	CArray<FileType*> aFileTypes;

	for (int i=(int)pAdvancedDlg->SendDlgItemMessage(IDC_FILETYPE,CB_GETCOUNT)-1;i>=0;i--)
	{
		FileType* pParam=(FileType*)pAdvancedDlg->SendDlgItemMessage(IDC_FILETYPE,CB_GETITEMDATA,i);
		if (pParam!=NULL && pParam!=(FileType*)szwEmpty && pParam->szType!=NULL)
			aFileTypes.Add(pParam);
	}

	if (IsUnicodeSystem())
	{
		WCHAR szKey[1000];
		DWORD dwIndex=0,dwKeyLength=1000;
		while (RegEnumKeyExW(HKCR,dwIndex,szKey,&dwKeyLength,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
		{
			if (szKey[0]==L'.') // Is Extension
				pAdvancedDlg->AddTypeToList(szKey,aFileTypes);

			dwIndex++;
			dwKeyLength=1000;
		}
	}
	else
	{
		CHAR szKey[1000];
		DWORD dwIndex=0,dwKeyLength=1000;
		DebugMessage("Enumerating the root of HKCR");
		while (RegEnumKeyEx(HKCR,dwIndex,szKey,&dwKeyLength,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
		{
			DebugFormatMessage("find %s",szKey);
			if (szKey[0]=='.') // Is Extension
				pAdvancedDlg->AddTypeToList(A2W(szKey),aFileTypes);

			dwIndex++;
			dwKeyLength=1000;
		}
		DebugMessage("Enumerating the root of END");
		
	}
	pAdvancedDlg->m_dwFlags|=fgOtherTypeAdded;

	CloseHandle(pAdvancedDlg->m_hTypeUpdaterThread);
	DebugCloseThread(pAdvancedDlg->m_hTypeUpdaterThread);
	pAdvancedDlg->m_hTypeUpdaterThread=NULL;
	
	if (!pAdvancedDlg->SendDlgItemMessage(IDC_FILETYPE,CB_GETDROPPEDSTATE))
		pAdvancedDlg->ReArrangeAllocatedData(); // Drop down is closed, arranging data
	
	DebugMessage("CLocateDlg::CAdvancedDlg::UpdaterProc END");
	return 0;
}

CLocateDlg::CAdvancedDlg::FileType::FileType(LPCWSTR& szBuildIn,HIMAGELIST hImageList)
:	szType(NULL),szIconPath(NULL)
{
	
	
	// First string is title
	DWORD dwLength=istrlenw(szBuildIn);
	szTitle=new WCHAR[++dwLength];
	MemCopyW(szTitle,szBuildIn,dwLength);
	szBuildIn+=dwLength;
	
	// Next string is extension
	dwLength=istrlenw(szBuildIn);
	dwExtensionLength=(++dwLength)+1;
	szExtensions=new WCHAR[dwExtensionLength];
	MemCopyW(szExtensions,szBuildIn,dwLength);
	szExtensions[dwLength]=L'\0';
	szBuildIn+=dwLength;
	replacech(szExtensions,L' ',L'\0');
	
	// Third is icon index, if available
	if (*szBuildIn!=L'\0')
	{
		hIcon=ImageList_ExtractIcon(NULL,hImageList,_wtoi(szBuildIn));
		for (;*szBuildIn!=L'\0';szBuildIn++);
	}
	else
		hIcon=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon;
	
	szBuildIn++;
}

CLocateDlg::CAdvancedDlg::FileType::~FileType()
{
	if (szType!=NULL)
		delete[] szType;
	if (szTitle!=NULL)
		delete[] szTitle;
	if (szExtensions!=NULL)
		delete[] szExtensions;
	if (hIcon!=NULL && hIcon!=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon)
		DestroyIcon(hIcon);
	if (szIconPath!=NULL)
		delete[] szIconPath;
}

void CLocateDlg::CAdvancedDlg::FileType::AddExtension(LPCWSTR szExtension,DWORD dwNewExtensionLength)
{
	if (szExtensions==NULL)
	{
		dwExtensionLength=dwNewExtensionLength+1;
		szExtensions=new WCHAR[dwExtensionLength];
		MemCopyW(szExtensions,szExtension,dwNewExtensionLength);
		szExtensions[dwNewExtensionLength]=L'\0';
	}
	else
	{
		WCHAR* szNewExtensions=new WCHAR[dwExtensionLength+dwNewExtensionLength];
		MemCopyW(szNewExtensions,szExtensions,dwExtensionLength);
		MemCopyW(szNewExtensions+dwExtensionLength-1,szExtension,dwNewExtensionLength);
		dwExtensionLength+=dwNewExtensionLength;
		szNewExtensions[dwExtensionLength-1]=L'\0';
		delete[] szExtensions;
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
	CStringW sIconPath;
	if (!RegKey.QueryValue(L"",sIconPath))
		return;

	szIconPath=alloccopy(sIconPath,sIconPath.GetLength());

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

	WCHAR* szIconIndex;
	for (szIconIndex=szIconPath;*szIconIndex!=L',' && *szIconIndex!=L'\0';szIconIndex++);
	if (*szIconIndex!=L'\0')
	{
		iIndex=_wtoi(szIconIndex+1);
		*szIconIndex=L'\0';
	}

	if (IsUnicodeSystem())
	{
		WCHAR szExpanded[MAX_PATH];
		if (ExpandEnvironmentStringsW(szIconPath,szExpanded,MAX_PATH)==0)
		{
			// Error
			hIcon=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon;
			return;
		}
		
		ExtractIconExW(szExpanded,iIndex,NULL,&hIcon,1);
		if (hIcon==NULL)
			hIcon=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon;
	}
	else
	{
		char szExpanded[MAX_PATH];
		if (ExpandEnvironmentStrings(W2A(szIconPath),szExpanded,MAX_PATH)==0)
		{
			// Error
			hIcon=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon;
			return;
		}
		
		ExtractIconEx(szExpanded,iIndex,NULL,&hIcon,1);
		if (hIcon==NULL)
			hIcon=GetLocateDlg()->m_AdvancedDlg.m_hDefaultTypeIcon;
	}
}


