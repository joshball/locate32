////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#if defined(DEF_RESOURCES) && defined(DEF_WINDOWS)

///////////////////////////
// Class CDialog
///////////////////////////


BOOL CDialog::OnInitDialog(HWND hwndFocus)
{
	AddDebugMenuItems(*this);
	
	return FALSE;
}

BOOL CDialog::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
#ifdef _DEBUG
	void DebugCommandsProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	DebugCommandsProc(*this,msg,wParam,lParam);
#endif
	return FALSE;
}

///////////////////////////////////////////
// CPropertyPage
///////////////////////////////////////////

/*
UINT CALLBACK PropertySheetPageProc(HWND hWnd,UINT uMsg,LPPROPSHEETPAGE  ppsp)
{
	switch (uMsg)
	{
	case PSPCB_CREATE:
		return 1;
	case PSPCB_RELEASE:
		break;
	}
	return 0;
}
*/

void CPropertyPage::Construct(LPCTSTR lpszTemplateName,UINT nIDCaption,TypeOfResourceHandle bType)
{
	iMemSet(&m_psp,0,sizeof(PROPSHEETPAGE));
	m_psp.dwSize=sizeof(PROPSHEETPAGE);
	m_psp.dwFlags=PSP_DEFAULT ; //PSP_USECALLBACK;
	if (lpszTemplateName!=NULL)
		m_psp.hInstance=GetResourceHandle(bType);
	m_psp.pszTemplate=lpszTemplateName;
	m_psp.pfnDlgProc=(DLGPROC)CAppData::PropPageWndProc;
	m_psp.lParam=(LPARAM)this;
	if (nIDCaption)
	{
		m_strCaption.LoadString(nIDCaption);
		m_psp.pszTitle=m_strCaption;
		m_psp.dwFlags|=PSP_USETITLE;
	}
	m_psp.pfnCallback=NULL; //PropertySheetPageProc;
	m_lpszTemplateName=m_psp.pszTemplate;
	m_bFirstSetActive=TRUE;
}

void CPropertyPage::SetModified(BOOL bChanged)
{
	if (m_hWnd==NULL)
		return;
	HWND hParentWnd=GetParent();
	if (bChanged)
		::SendMessage(hParentWnd,PSM_CHANGED,(WPARAM)m_hWnd,0);
	else
		::SendMessage(hParentWnd,PSM_UNCHANGED,(WPARAM)m_hWnd,0);
}

BOOL CPropertyPage::OnApply()
{
	OnOK();
	return TRUE;
}

void CPropertyPage::OnReset()
{
	OnCancel();
}

void CPropertyPage::OnOK()
{
}

void CPropertyPage::OnCancel()
{
}

BOOL CPropertyPage::OnSetActive()
{
	if (m_bFirstSetActive)
		m_bFirstSetActive=FALSE;
	return TRUE;
}

BOOL CPropertyPage::OnKillActive()
{
	return TRUE;
}

BOOL CPropertyPage::OnQueryCancel()
{
	return TRUE;
}

BOOL CPropertyPage::OnWizardBack()
{
	return TRUE;
}

BOOL CPropertyPage::OnWizardNext()
{
	return TRUE;
}

BOOL CPropertyPage::OnWizardFinish()
{
	return TRUE;
}

BOOL CPropertyPage::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	BOOL ret;
	if (CDialog::OnNotify(idCtrl,pnmh))
		return TRUE;
	if (pnmh->hwndFrom!=m_hWnd && pnmh->hwndFrom!=::GetParent(m_hWnd))
		return FALSE;
	switch(pnmh->code)
	{
	case PSN_APPLY:
		if (!OnApply())
			ret=PSNRET_INVALID_NOCHANGEPAGE;
		else
			ret=PSNRET_NOERROR;
		break;
	case PSN_RESET:
		OnReset();
		return TRUE;
	case PSN_SETACTIVE:
		if (OnSetActive())
			ret=0;
		else
			ret=-1;
		break;
	case PSN_KILLACTIVE:
		ret=!OnKillActive();
		break;
	case PSN_QUERYCANCEL:
		ret=!OnQueryCancel();
		break;
	case PSN_WIZBACK:
		if (OnWizardBack())
			ret=0;
		else
			ret=-1;
		break;
	case PSN_WIZNEXT:
		if (OnWizardNext())
			ret=0;
		else
			ret=-1;
		break;
	case PSN_WIZFINISH:
		ret=!OnWizardFinish();
		break;
	default:
		return FALSE;
	}
	::SetWindowLong(m_hWnd,DWL_MSGRESULT,ret);
	return TRUE;
}

///////////////////////////////////////////
// CPropertySheet
///////////////////////////////////////////
/*
int CALLBACK PropertySheetProc(HWND hWndDlg,UINT uMsg,LPARAM lParam)
{
	switch (uMsg)
	{
	case PSCB_INITIALIZED:
		DebugMessage("PropertySheetProc(): PSCB_INITIALIZED");
		break;
	}
	return 0;
}
*/

CPropertySheet::~CPropertySheet()
{
	if (m_psh.phpage!=NULL)
		delete[] m_psh.phpage;
}

void CPropertySheet::Construct(UINT nIDCaption,HWND hParentWnd,UINT iSelectPage)
{
	if (nIDCaption)
		m_strCaption.LoadString(nIDCaption);
	Construct((LPCTSTR)NULL,hParentWnd,iSelectPage);
}

void CPropertySheet::Construct(LPCTSTR pszCaption,HWND hParentWnd,UINT iSelectPage)
{
	if (pszCaption!=NULL)
		m_strCaption=pszCaption;
	else
		DebugMessage("CPropertySheet::Construct(): pszCaption==NULL");
	iMemSet(&m_psh,0,sizeof(PROPSHEETHEADER));
	m_psh.dwSize=PROPSHEETHEADER_V1_SIZE; //sizeof(PROPSHEETHEADER);
	m_psh.dwFlags=PSH_DEFAULT;
	m_psh.pszCaption=m_strCaption;
	m_psh.nStartPage=iSelectPage;
	m_psh.pfnCallback=NULL;
	m_bStacked=TRUE;
	m_hParentWnd=hParentWnd;
}

BOOL CPropertySheet::Create(HWND hParentWnd,DWORD dwStyle,DWORD dwExStyle)
{
	BuildPropPageArray();
	m_psh.dwFlags|=PSH_MODELESS;
	m_hWnd=(HWND)PropertySheet(&m_psh);
	if (m_hWnd==NULL)
	{
		DebugMessage("CPropertyPage::Create(): NULL returned by PropertySheet()");
		return FALSE;
	}
	DebugMessage("CProperyPage::Create(): PropertySheet()!=NULL NOERROR");
	return TRUE;
}

int CPropertySheet::GetActiveIndex() const
{
	if (m_hWnd==NULL)
		return -1;
	HWND hActivePage=(HWND)::SendMessage(m_hWnd,PSM_GETCURRENTPAGEHWND,0,0);
	for (int i=m_pages.GetSize()-1;i>=0;i--)
	{
		if (((CPropertyPage*)m_pages.GetAt(i))->m_hWnd==hActivePage)
			return i;
	}
	return -1;
}

int CPropertySheet::GetPageIndex(CPropertyPage* pPage)
{
	for (int i=m_pages.GetSize()-1;i>=0;i--)
	{
		if ((CPropertyPage*)m_pages.GetAt(i)==pPage)
			return i;
	}
	return -1;
}

BOOL CPropertySheet::SetActivePage(int nPage)
{
	if (m_hWnd!=NULL)
		return ::SendMessage(m_hWnd,PSM_SETCURSEL,nPage,NULL);
	m_psh.nStartPage=nPage;
	return FALSE;
}

BOOL CPropertySheet::SetActivePage(CPropertyPage* pPage)
{
	if (m_hWnd!=NULL)
		return ::SendMessage(m_hWnd,PSM_SETCURSEL,0,(LPARAM)pPage->m_hWnd);
	else
	{
		for (int i=m_pages.GetSize()-1;i>=0;i--)
		{
			if ((CPropertyPage*)m_pages.GetAt(i)==pPage)
			{
				m_psh.nStartPage=i;
				break;
			}
		}
	}
	return FALSE;
}

void CPropertySheet::SetTitle(LPCTSTR lpszText,UINT nStyle)
{
	if (m_hWnd==NULL)
	{
		m_strCaption=lpszText;
		m_psh.pszCaption=m_strCaption;
		m_psh.dwFlags&=~PSH_PROPTITLE;
		m_psh.dwFlags|=nStyle;
	}
	else
		::SendMessage(m_hWnd,PSM_SETTITLE,nStyle,(LPARAM)lpszText);
}

int CPropertySheet::DoModal()
{
	BuildPropPageArray();
	m_psh.hwndParent=m_hParentWnd;
	return ::PropertySheet(&m_psh);
}

void CPropertySheet::EndDialog(int nEndID)
{
	if (m_hWnd==NULL)
	{
		DebugMessage("CProperySheet::EndDialog(): m_hWnd==NULL");
		return;
	}
	if (m_psh.dwFlags&PSH_MODELESS)
		::DestroyWindow(m_hWnd);
	else
		::PostMessage(m_hWnd,PSM_PRESSBUTTON,IDCANCEL,0);
}

void CPropertySheet::BuildPropPageArray()
{
	if (m_psh.ppsp!=NULL)
	{
		DebugMessage("CPropertySheet::BuildPropPageArray(): m_psg.ppsp!=NULL");
	}
	m_psh.phpage=new HPROPSHEETPAGE[max(2,m_pages.GetSize())];
	for (int i=0;i<m_pages.GetSize();i++)
	{
		CPropertyPage* pPage=(CPropertyPage*)m_pages.GetAt(i);
		m_psh.phpage[i]=CreatePropertySheetPage(&pPage->m_psp);
	}
	m_psh.nPages=m_pages.GetSize();
}

BOOL CPropertySheet::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	return FALSE;
}

BOOL CPropertySheet::OnInitDialog(HWND hwndFocus)
{
	if (!m_bStacked)
	{
		HWND hWndTab=(HWND)SendMessage(PSM_GETTABCONTROL);
		if (hWndTab!=NULL)
			::SetWindowLong(hWndTab,GWL_STYLE,(::GetWindowLong(hWndTab,GWL_STYLE)|TCS_MULTILINE)&~TCS_SINGLELINE);
	}
	return 0;
}

///////////////////////////
// Class CInputDialog
///////////////////////////

int CInputDialog::DoModal(HWND hWndParent)
{
	int ret=CDialog::DoModal(hWndParent);
	if (ret==IDC_OK)
		return 1;
	m_hWnd=NULL;
	return 0;
}

void CInputDialog::SetTitle(LPCSTR szTitle)
{
	m_Title=szTitle;
	if (m_hWnd!=NULL)
		SetWindowText(m_Title);
}

void CInputDialog::SetTitle(int iTitle)
{
	m_Title.LoadString(iTitle);
	if (m_hWnd!=NULL)
		SetWindowText(m_Title);
}

BOOL CInputDialog::SetText(LPCSTR szText)
{
	m_Text=szText;
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_TEXT,m_Text);
	return TRUE;
}

BOOL CInputDialog::SetText(int iText)
{
	m_Text.LoadString(iText);
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_TEXT,m_Text);
	return TRUE;
}

void CInputDialog::SetInputText(LPCSTR szText)
{
	m_Input=szText;
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_EDIT,m_Input);
}

int CInputDialog::GetInputText(LPSTR szText,int nTextLen) const
{
	int i=min(nTextLen-1,(int)m_Input.GetLength());
	iMemCopy(szText,m_Input,i);
	szText[i]='\0';
	return i;
}

void CInputDialog::SetOKButtonText(LPCSTR szText)
{
	m_OKButton=szText;
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_OK,m_OKButton);
}

void CInputDialog::SetOKButtonText(int nText)
{
	m_OKButton.LoadString(nText);
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_OK,m_OKButton);
}

void CInputDialog::SetCancelButtonText(LPCSTR szText)
{
	if (m_bFlags&ID_NOCANCELBUTTON)
		return;
	m_CancelButton=szText;
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_CANCEL,m_CancelButton);
}

void CInputDialog::SetCancelButtonText(int nText)
{
	if (m_bFlags&ID_NOCANCELBUTTON)
		return;
	m_CancelButton.LoadString(nText);
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_CANCEL,m_CancelButton);
}

	
BOOL CInputDialog::OnInitDialog(HWND hwndFocus)
{
	SetDlgItemText(IDC_TEXT,m_Text);
	SetDlgItemText(IDC_EDIT,m_Input);
	SetWindowText(m_Title);
	SetDlgItemText(IDC_OK,m_OKButton);
	if (m_bFlags&ID_NOCANCELBUTTON)
		ShowDlgItem(IDC_CANCEL,swHide);
	else
		SetDlgItemText(IDC_CANCEL,m_CancelButton);
	SetFocus(IDC_EDIT);
	return CDialog::OnInitDialog(hwndFocus);
}

BOOL CInputDialog::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch (wID)
	{
	case IDC_OK:
		GetDlgItemText(IDC_EDIT,m_Input);
		if (m_bFlags&ID_DONTALLOWEMPTY && m_Input.IsEmpty())
			break;
		EndDialog(IDC_OK);
		break;
	case IDC_CANCEL:
		GetDlgItemText(IDC_EDIT,m_Input);
		EndDialog(IDC_CANCEL);
		break;
	case IDC_EDIT:
		if (wNotifyCode==EN_SETFOCUS)
			::SendMessage(hControl,EM_SETSEL,0,-1);
		break;
	}
	return CDialog::OnCommand(wID,wNotifyCode,hControl);
}

BOOL CInputDialog::OnClose()
{
	GetDlgItemText(IDC_EDIT,m_Input);
	EndDialog(IDC_CANCEL);
	return CDialog::OnClose();
}

///////////////////////////
// Class CCommonDialog
///////////////////////////


BOOL CALLBACK CAppData::CommonDialogProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	CCommonDialog* Wnd;
	if (uMsg==WM_INITDIALOG)
	{
		switch (((DWORD*)lParam)[0])
		{
#if !(_WIN32_WINNT >= 0x0500)
		case sizeof (OPENFILENAME):
		case sizeof (OPENFILENAME)+2*sizeof(DWORD)+sizeof(void*): // Win2000 Extensions
			Wnd=(CCommonDialog*)((OPENFILENAME*)lParam)->lCustData;
			break;
#else		
		case sizeof (OPENFILENAME):
			Wnd=(CCommonDialog*)((OPENFILENAME*)lParam)->lCustData;
			break;
#endif		
		case sizeof (CHOOSEFONT):
			Wnd=(CCommonDialog*)((CHOOSEFONT*)lParam)->lCustData;
			break;
		case sizeof (CHOOSECOLOR):
			Wnd=(CCommonDialog*)((CHOOSECOLOR*)lParam)->lCustData;
			break;
		case sizeof (PAGESETUPDLG):
			Wnd=(CCommonDialog*)((PAGESETUPDLG*)lParam)->lCustData;
			break;
		case sizeof (PRINTDLG):
			Wnd=(CCommonDialog*)((PRINTDLG*)lParam)->lCustData;
			break;
		case sizeof (FINDREPLACE):
			Wnd=(CCommonDialog*)((FINDREPLACE*)lParam)->lCustData;
			break;
		}
		Wnd->SetHandle(hWnd);
		SetWindowLong(hWnd,GWL_USERDATA,(LONG)Wnd);
		return Wnd->OnInitDialog((HWND)wParam);
	}
	Wnd=(CCommonDialog*)GetWindowLong(hWnd,GWL_USERDATA);
	if (Wnd!=NULL && uMsg==WM_COMMAND)
	{
		switch(LOWORD(wParam))
		{
		case IDOK:
			Wnd->OnOK();
			return FALSE;
		case IDCANCEL:
			Wnd->OnCancel();
			return FALSE;
		}
	}
	return WndProc(hWnd,uMsg,wParam,lParam);
}

CCommonDialog::CCommonDialog()
:	CDialog(0)
{
}

void CCommonDialog::OnOK()
{
}

void CCommonDialog::OnCancel()
{
}

///////////////////////////
// Class CFileDialog
///////////////////////////

CFileDialog::CFileDialog(BOOL bOpenFileDialog,LPCTSTR lpszDefExt,
						 LPCTSTR lpszFileName,DWORD dwFlags,LPCTSTR lpszFilter)
:	CCommonDialog(),m_bOpenFileDialog(bOpenFileDialog)
{
#if (_WIN32_WINNT >= 0x0500)
	m_pofn=(OPENFILENAME*)new char[sizeof(OPENFILENAME)];
	if (m_pofn==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return;
	}		
	iMemSet(m_pofn,0,sizeof(OPENFILENAME));
	m_pofn->lStructSize=sizeof(OPENFILENAME);
	
#else
	m_pofn=(OPENFILENAME*)new char[sizeof(OPENFILENAME)+16];
	if (m_pofn==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return;
	}		
	iMemSet(m_pofn,0,sizeof(OPENFILENAME)+16);
	if (GetSystemFeaturesFlag()&(efWin2000|efWinXP))
		m_pofn->lStructSize=sizeof(OPENFILENAME);
	else
		m_pofn->lStructSize=sizeof(OPENFILENAME_SIZE_VERSION_400);
	
#endif
	m_pofn->Flags=dwFlags|OFN_ENABLEHOOK|OFN_EXPLORER;
	m_pofn->lpfnHook=(LPOFNHOOKPROC)CAppData::CommonDialogProc;
	m_pofn->lCustData=(DWORD)this;
	m_pofn->lpstrFileTitle=m_szFileTitle;
	m_szFileTitle[0]='\0';
	m_pofn->nMaxFileTitle=64;
	m_pofn->hInstance=GetLanguageSpecificResourceHandle();
	m_pofn->lpstrCustomFilter=NULL;
	m_pofn->nMaxCustFilter=0;
	
	m_szFileName=new TCHAR[_MAX_PATH];
	if (m_szFileName==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return;
	}
	m_pofn->nMaxFile=_MAX_PATH;
	m_pofn->lpstrFile=m_szFileName;
	if (lpszFileName!=NULL)
		strcpy(m_szFileName,lpszFileName);
	else
	{
		m_szFileName[0]='\0';
		m_szFileName[1]='\0';
	}

	if (lpszFilter!=NULL)
	{
		DWORD iFilterLen;
		dstrlen(lpszFilter,iFilterLen);

		m_strFilter=new TCHAR[iFilterLen+2];
		if (m_strFilter==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			return;
		}
		iMemCopy(m_strFilter,lpszFilter,iFilterLen+1);
	}
	else
		m_strFilter=NULL;
	
	m_pofn->nFilterIndex=0;
	m_pofn->lpstrInitialDir=NULL;
	m_pofn->lpstrTitle=NULL;
	m_pofn->lpTemplateName=NULL;
	if (lpszDefExt!=NULL)
	{
		DWORD iExtLen;
		dstrlen(lpszDefExt,iExtLen);
		m_pofn->lpstrDefExt=new char [iExtLen+2];
		if (m_pofn->lpstrDefExt==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			return;
		}
		if (m_pofn->lpstrDefExt!=NULL)
			iMemCopy((LPSTR)m_pofn->lpstrDefExt,lpszDefExt,iExtLen+1);
	}
	else
		m_pofn->lpstrDefExt=NULL;
}

CFileDialog::~CFileDialog()
{
	if (m_pofn!=NULL)
	{
		if (m_pofn->lpstrDefExt!=NULL)
			delete[] (LPSTR)m_pofn->lpstrDefExt;
		delete[] (char*)m_pofn;
		m_pofn=NULL;
	}
	if (m_strFilter!=NULL)
		delete[] m_strFilter;
	if (m_szFileName!=NULL)
		delete[] m_szFileName;
}

BOOL CFileDialog::EnableFeatures(DWORD nFlags)
{
#if !(_WIN32_WINNT >= 0x0500)
	if (nFlags==efCheck)
		nFlags=GetSystemFeaturesFlag();
	if (nFlags&(efWin2000|efWinME) && m_pofn!=NULL)
		m_pofn->lStructSize=sizeof(OPENFILENAME)+2*sizeof(DWORD)+sizeof(void*);
#endif
	if (nFlags==efCheck)
		nFlags=GetSystemFeaturesFlag();
	if (nFlags&(efWin2000|efWinME) && m_pofn!=NULL)
		m_pofn->lStructSize=sizeof(OPENFILENAME);
	return TRUE;
}

BOOL CFileDialog::DoModal(HWND hParentWnd)
{
	BOOL OnError;
	int i;
	m_pofn->hwndOwner=hParentWnd;
	m_pofn->lpstrFilter=m_strFilter;
	if (m_strFilter!=NULL)
	{
		for (i=0;m_strFilter[i]!='\0';i++)
		{
			if (m_strFilter[i]=='|')
				m_strFilter[i]='\0';
		}
	}
	if (hParentWnd!=NULL)
		::EnableWindow(hParentWnd,FALSE);
	
	if (m_bOpenFileDialog)
		OnError=::GetOpenFileName(m_pofn);
	else
		OnError=::GetSaveFileName(m_pofn);
	m_hWnd=NULL;
	if (hParentWnd!=NULL)
	{
		::EnableWindow(hParentWnd,TRUE);
		::SetFocus(hParentWnd);
	}
	return OnError;	
}

CString CFileDialog::GetPathName() const
{
	if (m_hWnd!=NULL)
	{
		CString path;
		if (::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,(WPARAM)_MAX_PATH,(LPARAM)path.GetBuffer(_MAX_PATH))<0)
			path.Empty();
		if (!path.IsEmpty())
		{
			if (::SendMessage(::GetParent(m_hWnd),CDM_GETFILEPATH,(WPARAM)_MAX_PATH,(LPARAM)path.GetBuffer())<0)
				path.Empty();
			else
				path.FreeExtra();
		}		
		return path;
	}
	return m_pofn->lpstrFile;
}

CString CFileDialog::GetFileName() const
{
	if (m_hWnd!=NULL)
	{
		CString name;
		if (::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,_MAX_PATH,(LPARAM)name.GetBuffer(_MAX_PATH))<0)
			name.Empty();
		else
			name.FreeExtra();
		return name;
	}
	return (m_szFileName+m_pofn->nFileOffset);
}

CString CFileDialog::GetFileExt() const
{
	if (m_hWnd!=NULL)
	{
		CString ext;
		if (::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,_MAX_PATH,(LPARAM)ext.GetBuffer(_MAX_PATH))<0)
			ext.Empty();
		else
		{
			ext.FreeExtra();
			int pos=ext.FindLast('.');
			if (pos>=0)
				return ((LPCSTR)ext+pos);	
		}
		return ext;
	}
	return (m_szFileName+m_pofn->nFileExtension);
}

BOOL CFileDialog::GetReadOnlyPref() const
{
	if (m_pofn->Flags&OFN_READONLY)
		return TRUE;
	return FALSE;
}

POSITION CFileDialog::GetStartPosition() const
{
	if (m_szFileName==NULL)
		return NULL;
	if (m_szFileName[0]=='\0')
		return NULL;
	return (POSITION)m_szFileName;
}

CString CFileDialog::GetNextPathName(POSITION& pos) const
{
	CString Path((LPTSTR)pos);
	if (Path.IsEmpty())
		pos=NULL;
	else
		pos+=Path.GetLength()+1;
	return Path;
}

void CFileDialog::SetTemplate(LPCTSTR lpID,TypeOfResourceHandle bType)
{
	m_pofn->lpTemplateName=lpID;
	m_pofn->hInstance=GetResourceHandle(bType);
	if (lpID==NULL)
		m_pofn->Flags&=~OFN_ENABLETEMPLATE;
	else
		m_pofn->Flags|=OFN_ENABLETEMPLATE;
}

CString CFileDialog::GetFolderPath() const
{
	CString path;
	if (m_hWnd!=NULL)
	{
		if (::SendMessage(::GetParent(m_hWnd),CDM_GETFOLDERPATH,_MAX_PATH,(LPARAM)path.GetBuffer(_MAX_PATH))>=0)
			path.FreeExtra();	
		else
			path.Empty();
		return path;
	}
	path.Copy(m_szFileName,m_pofn->nFileOffset);
	return path;
}

void CFileDialog::SetDefExt(LPCSTR lpsz)
{
	if (m_hWnd!=NULL)
	{
		::SendMessage(::GetParent(m_hWnd),CDM_SETDEFEXT,0,(LPARAM)lpsz);
		return;
	}
	if (m_pofn->lpstrDefExt!=NULL)
		delete[] (LPSTR)m_pofn->lpstrDefExt;
	m_pofn->lpstrDefExt=new char [strlen(lpsz)+2];
	if (m_pofn->lpstrDefExt==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return;
	}
	if (lpsz!=NULL)
	{
		if (m_pofn->lpstrDefExt!=NULL)
			strcpy((LPSTR)m_pofn->lpstrDefExt,lpsz);
	}
	else
		m_pofn->lpstrDefExt=NULL;
}

UINT CFileDialog::OnShareViolation(LPCTSTR lpszPathName)
{
	return OFN_SHAREWARN;
}

BOOL CFileDialog::OnFileNameOK()
{
	return 0;
}

void CFileDialog::OnInitDone()
{
}

void CFileDialog::OnFileNameChange()
{
}

void CFileDialog::OnFolderChange()
{
}

void CFileDialog::OnTypeChange()
{
}

BOOL CFileDialog::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	if (CCommonDialog::OnNotify(idCtrl,pnmh))
		return TRUE;
	switch(pnmh->code)
	{
	case CDN_SHAREVIOLATION:
		return OnShareViolation(((LPOFNOTIFY)pnmh)->pszFile);
	case CDN_FILEOK:
		return OnFileNameOK();
	case CDN_SELCHANGE:
		OnFileNameChange();
		break;
	case CDN_INITDONE:
		OnInitDone();
		break;
	case CDN_TYPECHANGE:
		OnTypeChange();
		break;
	case CDN_FOLDERCHANGE:
		OnFolderChange();
		break;
	}
	return 0;
}

///////////////////////////
// Class CFontDialog
///////////////////////////

CFontDialog::CFontDialog(LPLOGFONT lplfInitial,DWORD dwFlags,HDC hdcPrinter)
:	CCommonDialog()
{
	m_cf.lStructSize=sizeof(CHOOSEFONT);
	m_cf.Flags=dwFlags|CF_ENABLEHOOK;
	if (lplfInitial!=NULL)
	{
		m_lf=*lplfInitial;
		m_cf.Flags|=CF_INITTOLOGFONTSTRUCT;
	}
	m_cf.lpLogFont=&m_lf;
	m_cf.rgbColors=NULL;
	m_cf.lCustData=(DWORD)this;
	m_cf.lpfnHook=(LPOFNHOOKPROC)CAppData::CommonDialogProc;
	m_cf.lpTemplateName=NULL;
	m_cf.hInstance=GetLanguageSpecificResourceHandle();
	m_cf.lpszStyle=m_szStyleName;
	if (hdcPrinter!=NULL)
	{
		m_cf.hDC=hdcPrinter;
		m_cf.Flags|=CF_PRINTERFONTS;
	}
	m_szStyleName[0]='\0';
}

CFontDialog::CFontDialog(const CHARFORMAT& charformat,DWORD dwFlags,HDC hdcPrinter)
:	CCommonDialog()
{
	m_cf.lStructSize=sizeof(CHOOSEFONT);
	m_cf.Flags=dwFlags|CF_ENABLEHOOK|CF_INITTOLOGFONTSTRUCT;
	FillInLogFont(charformat);
	m_cf.lpLogFont=&m_lf;
	m_cf.rgbColors=NULL;
	m_cf.lCustData=(DWORD)this;
	m_cf.lpfnHook=(LPOFNHOOKPROC)CAppData::CommonDialogProc;
	m_cf.lpTemplateName=NULL;
	m_cf.hInstance=GetLanguageSpecificResourceHandle();
	m_cf.lpszStyle=m_szStyleName;
	m_szStyleName[0]='\0';
	if (hdcPrinter!=NULL)
	{
		m_cf.hDC=hdcPrinter;
		m_cf.Flags|=CF_PRINTERFONTS;
	}
}

BOOL CFontDialog::DoModal(HWND hParentWnd)
{
	BOOL ret;
	m_cf.hwndOwner=hParentWnd;
	ret=ChooseFont(&m_cf);
	m_hWnd=NULL;
	return ret;
}

void CFontDialog::GetCurrentFont(LPLOGFONT lplf)
{
	if (lplf!=NULL)
	{
		if (m_hWnd!=NULL)
			::SendMessage(m_hWnd,WM_CHOOSEFONT_GETLOGFONT,0,(LPARAM)lplf);
		else
			*lplf=m_lf;
	}
}

void CFontDialog::GetCharFormat(CHARFORMAT& cf) const
{
	cf.cbSize=sizeof(CHARFORMAT);
	cf.dwMask=CFM_BOLD|CFM_COLOR|CFM_FACE|CFM_ITALIC|CFM_SIZE|CFM_STRIKEOUT|CFM_UNDERLINE;
	cf.dwEffects=0;
	if (m_lf.lfItalic)
		cf.dwEffects|=CFE_ITALIC;
	if (m_lf.lfWeight>=600)
		cf.dwEffects|=CFE_BOLD;
	if (m_lf.lfStrikeOut)
		cf.dwEffects|=CFE_STRIKEOUT;
	if (m_lf.lfUnderline)
		cf.dwEffects|=CFE_UNDERLINE;
	cf.yHeight=m_cf.iPointSize;
	cf.crTextColor=m_cf.rgbColors;
	cf.bCharSet=m_lf.lfCharSet;
	cf.bPitchAndFamily=m_lf.lfPitchAndFamily;
	strcpy(cf.szFaceName,m_lf.lfFaceName);
}

DWORD CFontDialog::FillInLogFont(const CHARFORMAT& cf)
{
	m_cf.rgbColors=(cf.dwMask&CFM_COLOR?cf.crTextColor:0);
	m_cf.Flags|=CF_EFFECTS;
	
	if (cf.dwMask&CFM_SIZE)
	{
		HDC hDC=CreateDC("DISPLAY",NULL,NULL,NULL);
		m_lf.lfHeight=-MulDiv(cf.yHeight,::GetDeviceCaps(hDC,LOGPIXELSY),72);
		DeleteDC(hDC);
	}
	else
		m_lf.lfHeight=0;
	m_lf.lfWidth=0;
	m_lf.lfEscapement=0;
	m_lf.lfOrientation=0;
	m_lf.lfWeight=((cf.dwMask&CFM_BOLD && cf.dwEffects&CFE_BOLD)?700:500);
	m_lf.lfItalic=(cf.dwMask&CFM_ITALIC && cf.dwEffects&CFE_ITALIC);
	m_lf.lfUnderline=(cf.dwMask&CFM_UNDERLINE && cf.dwEffects&CFE_UNDERLINE);
	m_lf.lfStrikeOut=(cf.dwMask&CFM_STRIKEOUT && cf.dwEffects&CFE_STRIKEOUT);
	m_lf.lfCharSet=(cf.dwMask&CFM_CHARSET?cf.bCharSet:DEFAULT_CHARSET);
	m_lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
	m_lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;
	m_lf.lfQuality=DEFAULT_QUALITY;
	if (cf.dwMask&CFM_FACE)
	{
		m_lf.lfPitchAndFamily=cf.bPitchAndFamily;
		strcpy(m_lf.lfFaceName,cf.szFaceName);
	}
	else
	{
		m_lf.lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;
		m_lf.lfFaceName[0]='\0';
	}
	return TRUE;
}

///////////////////////////
// Class CColorDialog
///////////////////////////

CColorDialog::CColorDialog(COLORREF clrInit,DWORD dwFlags)
:	CCommonDialog()
{
	m_cc.lStructSize=sizeof(CHOOSECOLOR);
	m_cc.Flags=dwFlags|CC_ENABLEHOOK;
	m_cc.hInstance=(HWND)GetLanguageSpecificResourceHandle();
	if (m_cc.rgbResult=clrInit)
		m_cc.Flags|=CC_RGBINIT;
	m_cc.lpCustColors=new COLORREF[16];
	if (m_cc.lpCustColors==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return;
	}
	iMemSet(m_cc.lpCustColors,255,sizeof(COLORREF)*16);
	m_cc.lCustData=(DWORD)this;
	m_cc.lpfnHook=(LPOFNHOOKPROC)CAppData::CommonDialogProc;
	m_cc.lpTemplateName=NULL;
}

CColorDialog::~CColorDialog()
{
	if (m_cc.lpCustColors!=NULL)
		delete[] m_cc.lpCustColors;
}

BOOL CColorDialog::DoModal(HWND hParentWnd)
{
	BOOL ret;
	m_cc.hwndOwner=hParentWnd;
	ret=ChooseColor(&m_cc);
	m_hWnd=NULL;
	return ret;
}

///////////////////////////
// Class CPageSetupDialog
///////////////////////////

BOOL CALLBACK CAppData::PagePaintProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return ((CPageSetupDialog*)GetAppData()->m_pCommonDialog)->OnDrawPage((HDC)wParam,uMsg,(LPRECT)lParam);
}

CPageSetupDialog::CPageSetupDialog(DWORD dwFlags)
:	CCommonDialog()
{
	m_psd.lStructSize=sizeof(PAGESETUPDLG);
	m_psd.hDevMode=NULL;
	m_psd.hDevNames=NULL;
	m_psd.Flags=dwFlags|PSD_ENABLEPAGESETUPHOOK|PSD_ENABLEPAGEPAINTHOOK;
	fMemSet(&m_psd.rtMinMargin,0,sizeof(RECT));
	fMemSet(&m_psd.rtMargin,0,sizeof(RECT));
	m_psd.hInstance=GetLanguageSpecificResourceHandle();
	m_psd.lCustData=(DWORD)this;
	m_psd.lpfnPageSetupHook=(LPPAGESETUPHOOK)CAppData::CommonDialogProc;
	m_psd.lpfnPagePaintHook=(LPPAGEPAINTHOOK)CAppData::PagePaintProc;
	GetAppData()->m_pCommonDialog=this;
	m_psd.lpPageSetupTemplateName=NULL;
	m_psd.hPageSetupTemplate=NULL;
}

CPageSetupDialog::~CPageSetupDialog()
{
	if (m_psd.hDevMode!=NULL)
		GlobalFree(m_psd.hDevMode);
	if (m_psd.hDevNames!=NULL)
		GlobalFree(m_psd.hDevNames);
}

LPDEVMODE CPageSetupDialog::GetDevMode() const
{
	if (m_psd.hDevMode==NULL)
		return NULL;
	return (LPDEVMODE)GlobalLock(m_psd.hDevMode);
}

CString CPageSetupDialog::GetDriverName() const
{
	if (m_psd.hDevNames==NULL)
		return szEmpty;
	LPDEVNAMES lpdn=(LPDEVNAMES)GlobalLock(m_psd.hDevNames);
	CString str((LPCSTR)lpdn+lpdn->wDriverOffset);
	GlobalUnlock(m_psd.hDevNames);
	return str;
}

CString CPageSetupDialog::GetDeviceName() const
{
	if (m_psd.hDevNames==NULL)
		return szEmpty;
	LPDEVNAMES lpdn=(LPDEVNAMES)GlobalLock(m_psd.hDevNames);
	CString str((LPCSTR)lpdn+lpdn->wDeviceOffset);
	GlobalUnlock(m_psd.hDevNames);
	return str;
}

CString CPageSetupDialog::GetPortName() const
{
	if (m_psd.hDevNames==NULL)
		return szEmpty;
	LPDEVNAMES lpdn=(LPDEVNAMES)GlobalLock(m_psd.hDevNames);
	CString str((LPCSTR)lpdn+lpdn->wOutputOffset);
	GlobalUnlock(m_psd.hDevNames);
	return str;
}

HDC CPageSetupDialog::CreatePrinterDC()
{
	return CreateDC((LPCSTR)m_psd.hDevNames+((DEVNAMES*)m_psd.hDevNames)->wDriverOffset,
		(LPCSTR)m_psd.hDevNames+((DEVNAMES*)m_psd.hDevNames)->wDeviceOffset,
		NULL,(CONST DEVMODE*)&m_psd.hDevMode);
}

CSize CPageSetupDialog::GetPaperSize() const
{
	return (m_psd.rtMargin.right-m_psd.rtMargin.left,m_psd.rtMargin.bottom-m_psd.rtMargin.top);
}

void CPageSetupDialog::GetMargins(LPRECT lpRectMargins,LPRECT lpRectMinMargins) const
{
	if (lpRectMargins!=NULL)
		*lpRectMargins=m_psd.rtMargin;
	if (lpRectMinMargins!=NULL)
		*lpRectMinMargins=m_psd.rtMinMargin;
}

BOOL CPageSetupDialog::DoModal(HWND hParentWnd)
{
	BOOL ret;
	m_psd.hwndOwner=hParentWnd;
	ret=PageSetupDlg(&m_psd);
	GetAppData()->m_pCommonDialog=NULL;
	m_hWnd=NULL;
	return ret;
}

UINT CPageSetupDialog::OnDrawPage(HDC hDC,UINT nMessage,LPRECT lpRect)
{
	return FALSE;
}

///////////////////////////
// Class CPrintDialog
///////////////////////////

CPrintDialog::CPrintDialog(BOOL bPrintSetupOnly,DWORD dwFlags)
{
	m_pd.lStructSize=sizeof(PRINTDLG);
	m_pd.hDC=NULL;
	m_pd.Flags=dwFlags|PD_ENABLEPRINTHOOK;
	if (bPrintSetupOnly)
		m_pd.Flags|=PD_PRINTSETUP;
	else
		m_pd.Flags|=PD_RETURNDC;
	m_pd.hInstance=GetLanguageSpecificResourceHandle();
	m_pd.lCustData=(DWORD)this;
	m_pd.lpfnPrintHook=(LPPRINTHOOKPROC)CAppData::CommonDialogProc;
	m_pd.lpfnSetupHook=NULL;
	m_pd.lpPrintTemplateName=NULL;
	m_pd.lpSetupTemplateName=NULL;
	m_pd.nFromPage=1;
	m_pd.nToPage=1;
	m_pd.nMinPage=1;
	m_pd.nMaxPage=1;
	m_pd.nCopies=1;
}

CPrintDialog::~CPrintDialog()
{
	if (m_pd.hDevMode!=NULL)
		GlobalFree(m_pd.hDevMode);
	if (m_pd.hDevNames!=NULL)
		GlobalFree(m_pd.hDevNames);
}

BOOL CPrintDialog::DoModal(HWND hParentWnd)
{
	BOOL ret;
	m_pd.hwndOwner=hParentWnd;
	ret=PrintDlg(&m_pd);
	m_hWnd=NULL;
	return ret;
}

BOOL CPrintDialog::GetDefaults()
{
	BOOL ret;
	m_pd.Flags|=PD_RETURNDEFAULT;
	ret=::PrintDlg(&m_pd);
	m_pd.Flags&=~PD_RETURNDEFAULT;
	return ret;
}

int CPrintDialog::GetCopies() const
{
	if (m_pd.Flags&PD_USEDEVMODECOPIES && m_pd.hDevMode!=NULL)
	{
		LPDEVMODE lpdm=(LPDEVMODE)GlobalLock(m_pd.hDevMode);
		int ret=lpdm->dmCopies;
		GlobalUnlock(m_pd.hDevMode);
		return ret;
	}
	return m_pd.nCopies;
}

LPDEVMODE CPrintDialog::GetDevMode() const
{
	LPDEVMODE lpdm=(LPDEVMODE)GlobalAlloc(GMEM_FIXED,sizeof(DEVMODE));
	LPDEVMODE lpmdm=(LPDEVMODE)GlobalLock(m_pd.hDevMode);
	iMemCopy(lpdm,lpmdm,sizeof(DEVMODE));
	GlobalUnlock(m_pd.hDevMode);
	return lpdm;
}

CString CPrintDialog::GetDriverName() const
{
	if (m_pd.hDevNames==NULL)
		return szEmpty;
	LPDEVNAMES lpdn=(LPDEVNAMES)GlobalLock(m_pd.hDevNames);
	CString str((LPCSTR)lpdn+lpdn->wDriverOffset);
	GlobalUnlock(m_pd.hDevNames);
	return str;
}

CString CPrintDialog::GetDeviceName() const
{
	if (m_pd.hDevNames==NULL)
		return szEmpty;
	LPDEVNAMES lpdn=(LPDEVNAMES)GlobalLock(m_pd.hDevNames);
	CString str((LPCSTR)lpdn+lpdn->wDeviceOffset);
	GlobalUnlock(m_pd.hDevNames);
	return str;
}

CString CPrintDialog::GetPortName() const
{
	if (m_pd.hDevNames==NULL)
		return szEmpty;
	LPDEVNAMES lpdn=(LPDEVNAMES)GlobalLock(m_pd.hDevNames);
	CString str((LPCSTR)lpdn+lpdn->wOutputOffset);
	GlobalUnlock(m_pd.hDevNames);
	return str;
}

HDC CPrintDialog::CreatePrinterDC()
{
	HDC ret;
	LPDEVNAMES lpdn=(LPDEVNAMES)GlobalLock(m_pd.hDevNames);
	LPDEVMODE lpdm=(LPDEVMODE)GlobalLock(m_pd.hDevMode);
	ret=CreateDC((LPCSTR)lpdn+lpdn->wDriverOffset,(LPCSTR)lpdn+lpdn->wDeviceOffset,
		NULL,lpdm);
	GlobalUnlock(m_pd.hDevNames);
	GlobalUnlock(m_pd.hDevMode);
	return ret;
}

///////////////////////////
// Class CFindReplaceDialog
///////////////////////////

CFindReplaceDialog::CFindReplaceDialog()
{	
	m_fr.lStructSize=sizeof(FINDREPLACE);
	m_fr.hInstance=GetLanguageSpecificResourceHandle();
	m_fr.lpstrFindWhat=m_szFindWhat;
	m_fr.lpstrReplaceWith=m_szReplaceWith;
	m_fr.wFindWhatLen=128;
	m_fr.wReplaceWithLen=128;
	m_fr.lCustData=(DWORD)this;
	m_fr.lpfnHook=(LPFRHOOKPROC)CAppData::CommonDialogProc;
	m_fr.lpTemplateName=NULL;
}

BOOL CFindReplaceDialog::Create(BOOL bFindDialogOnly,LPCTSTR lpszFindWhat,LPCTSTR lpszReplaceWith,DWORD dwFlags,HWND hParentWnd)
{
	m_fr.hwndOwner=hParentWnd;
	m_fr.Flags=dwFlags|FR_ENABLEHOOK;
	strcpy(m_szFindWhat,lpszFindWhat);
	strcpy(m_szReplaceWith,lpszReplaceWith);
	if (bFindDialogOnly)
		m_hWnd=FindText(&m_fr);
	else
		m_hWnd=ReplaceText(&m_fr);
	if (m_hWnd==NULL)
		return FALSE;
	return TRUE;
}


///////////////////////////
// Class CFolderDialog
///////////////////////////

int CALLBACK CAppData::FolderDialogProc(HWND hWnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
{
	CFolderDialog* pDlg=(CFolderDialog*)lpData;
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		pDlg->m_hWnd=hWnd;
		return (pDlg->OnInitialized()?0:1);
	case BFFM_SELCHANGED:
		return (pDlg->OnSelChanged((LPITEMIDLIST)lParam)?0:1);
	case BFFM_VALIDATEFAILED :
		return (pDlg->OnValidateFailed((LPITEMIDLIST)lParam)?0:1);
	}
	return FALSE;
}

CFolderDialog::CFolderDialog(LPCSTR lpszTitle,UINT ulFlags,LPCITEMIDLIST pidlRoot)
:	m_hWnd(NULL),m_lpil(NULL),m_lpDefaultIL(NULL)
{
	CoInitialize(NULL);
	m_strTitle=lpszTitle;
	m_bi.pszDisplayName=m_strDisplayName.GetBuffer(_MAX_PATH);
	m_bi.ulFlags=ulFlags;
	m_bi.pidlRoot=pidlRoot;
	m_bi.lpfn=(BFFCALLBACK)CAppData::FolderDialogProc;
	m_bi.iImage=0;
}

CFolderDialog::CFolderDialog(UINT nTitleID,UINT ulFlags,LPCITEMIDLIST pidlRoot)
:	m_hWnd(NULL),m_lpil(NULL),m_lpDefaultIL(NULL)
{
	CoInitialize(NULL);
	m_strTitle.LoadString(nTitleID);
	m_bi.pszDisplayName=m_strDisplayName.GetBuffer(_MAX_PATH);
	m_bi.ulFlags=ulFlags;
	m_bi.pidlRoot=pidlRoot;
	m_bi.lpfn=(BFFCALLBACK)CAppData::FolderDialogProc;
	m_bi.iImage=0;
}
	
CFolderDialog::~CFolderDialog()
{
	if (m_lpil!=NULL)
	{
		LPMALLOC pMalloc;
		SHGetMalloc(&pMalloc);
		pMalloc->Free(m_lpil);
		m_lpil=NULL;
	}
	CoUninitialize();
}

BOOL CFolderDialog::DoModal(HWND hOwner)
{
	m_bi.lpszTitle=m_strTitle;
	m_bi.lParam=(LPARAM)this;
	m_bi.hwndOwner=hOwner;
	m_lpil=SHBrowseForFolder(&m_bi);
	if (m_lpil==NULL)
		return FALSE;
	return TRUE;
}

LPITEMIDLIST CFolderDialog::GetFolder() const
{
	return m_lpil;
}

BOOL CFolderDialog::GetFolder(CString& Folder) const
{
	if (!SHGetPathFromIDList(m_lpil,Folder.GetBuffer(_MAX_PATH)))
		return FALSE;
	Folder.FreeExtra();
	return TRUE;
}

BOOL CFolderDialog::GetFolder(LPSTR szFolder) const
{
	return SHGetPathFromIDList(m_lpil,szFolder);
}

BOOL CFolderDialog::GetDisplayName(CString& strDisplayName) const
{
	strDisplayName=m_strDisplayName;
	strDisplayName.FreeExtra();
	return TRUE;
}

BOOL CFolderDialog::GetDisplayName(LPSTR szDisplayName,DWORD nSize)
{
	if (!nSize)
		return FALSE;
	m_strDisplayName.FreeExtra();
	if (nSize<=m_strDisplayName.GetLength())
	{
		iMemCopy(szDisplayName,(LPCSTR)m_strDisplayName,nSize-1);
		szDisplayName[nSize-1]='\0';
	}
	else
		iMemCopy(szDisplayName,(LPCSTR)m_strDisplayName,m_strDisplayName.GetLength()+1);
	return TRUE;
}

BOOL CFolderDialog::EnableOK(BOOL bEnable)
{
	if (m_hWnd==NULL)
		return FALSE;
	SendMessage(m_hWnd,BFFM_ENABLEOK,(WPARAM)bEnable,0);
	return TRUE;
}

BOOL CFolderDialog::SetSelection(LPITEMIDLIST lpil)
{
	if (m_hWnd==NULL)
	{
		m_lpDefaultIL=lpil;
		m_strDefaultFolder.Empty();
		return TRUE;
	}
	SendMessage(m_hWnd,BFFM_SETSELECTION,FALSE,(LPARAM)lpil);
	return TRUE;
}

BOOL CFolderDialog::SetSelection(LPCSTR lpFolder)
{
	if (m_hWnd==NULL)
	{
		m_strDefaultFolder=lpFolder;
		m_lpDefaultIL=NULL;
		return TRUE;
	}
	SendMessage(m_hWnd,BFFM_SETSELECTION,TRUE,(LPARAM)lpFolder);
	return TRUE;
}

BOOL CFolderDialog::SetStatusText(LPCSTR lpStatus)
{
	if (m_hWnd==NULL)
		return FALSE;
	SendMessage(m_hWnd,BFFM_SETSTATUSTEXT,0,(LPARAM)lpStatus);
	return TRUE;
}

BOOL CFolderDialog::OnInitialized()
{
	if (!m_strDefaultFolder.IsEmpty())
		SetSelection(m_strDefaultFolder);
	else if (m_lpDefaultIL!=NULL)
		SetSelection(m_lpDefaultIL);
	return TRUE;
}

BOOL CFolderDialog::OnSelChanged(LPITEMIDLIST lpil)
{
	return TRUE;
}

BOOL CFolderDialog::OnValidateFailed(LPITEMIDLIST lpil)
{
	return TRUE;
}


#endif