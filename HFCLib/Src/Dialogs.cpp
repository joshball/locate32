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

///////////////////////////
// Class COptionsPropertyPage
///////////////////////////
#define IDC_EDITCONTROLFORSELECTEDITEM  1300
#define IDC_SPINCONTROLFORSELECTEDITEM  1301
#define IDC_COMBOCONTROLFORSELECTEDITEM 1302
#define sMemCopyW	MemCopyW

BOOL COptionsPropertyPage::Initialize(COptionsPropertyPage::Item** pItems)
{
	if (m_pTree==NULL)
	{
		m_pTree=new CTreeCtrl(GetDlgItem(IDC_SETTINGS));
		m_Images.Create(IDB_OPTIONSPROPERTYPAGEBITMAPS,16,0,RGB(255,255,255),IMAGE_BITMAP,LR_SHARED|LR_CREATEDIBSECTION);
		m_pTree->SetImageList(m_Images,TVSIL_STATE);
	}

	if (pItems==NULL)
		return FALSE;
	
	// Counting items
	for (int iItems=0;pItems[iItems]!=NULL;iItems++);
	
	m_pItems=new Item*[iItems+1];
	m_pItems[iItems]=NULL;
	CopyMemory(m_pItems,pItems,sizeof(Item*)*(iItems+1));
	return InsertItemsToTree(NULL,m_pItems,NULL);
	
}



BOOL COptionsPropertyPage::InsertItemsToTree(HTREEITEM hParent,COptionsPropertyPage::Item** pItems,COptionsPropertyPage::Item* pParent)
{
	INITIALIZEPARAMS bp;
	bp.pPage=this;

	

	union {
		TVINSERTSTRUCTA tisa;
		TVINSERTSTRUCTW tisw;
	};
	
	if (!IsFullUnicodeSupport())
	{
		// Windows 9x
		tisa.hInsertAfter=TVI_LAST;
		tisa.hParent=hParent;
		tisa.itemex.stateMask=TVIS_STATEIMAGEMASK|TVIS_EXPANDED;
		tisa.itemex.mask=TVIF_STATE|TVIF_CHILDREN|TVIF_TEXT|TVIF_PARAM;
	}
	else
	{
		// Windows NT/2000/XP
		tisw.hInsertAfter=TVI_LAST;
		tisw.hParent=hParent;
		tisw.itemex.stateMask=TVIS_STATEIMAGEMASK|TVIS_EXPANDED;
		tisw.itemex.mask=TVIF_STATE|TVIF_CHILDREN|TVIF_TEXT|TVIF_PARAM;
	}

	HTREEITEM hSelectedRadioItem=NULL;
	
	int nItemHeight=18;
    for (int i=0;pItems[i]!=NULL;i++)
	{
		if (pItems[i]->pProc!=NULL)
		{
			bp.crReason=BASICPARAMS::Get;
			pItems[i]->SetValuesForBasicParams(&bp);
			if (pItems[i]->pProc(&bp))
				pItems[i]->GetValuesFromBasicParams(&bp);
		}
        			
		if (!IsFullUnicodeSupport())
		{
			DWORD iStrLen;
			dstrlen(pItems[i]->pString,iStrLen);
			tisa.itemex.pszText=new char [iStrLen+2];
			MemCopyWtoA(tisa.itemex.pszText,pItems[i]->pString,iStrLen+1);
			tisa.itemex.cChildren=pItems[i]->pChilds==0?0:1;
			tisa.itemex.lParam=LPARAM(pItems[i]);
			tisa.itemex.state=TVIS_EXPANDED|INDEXTOSTATEIMAGEMASK(pItems[i]->GetStateImage());
			tisa.hInsertAfter=m_pTree->InsertItem(&tisa);
			delete[] tisa.itemex.pszText;
		}
		else
		{
			tisw.itemex.pszText=pItems[i]->pString;
			tisw.itemex.cChildren=pItems[i]->pChilds==0?0:1;
			tisw.itemex.lParam=LPARAM(pItems[i]);
			tisw.itemex.state=TVIS_EXPANDED|INDEXTOSTATEIMAGEMASK(pItems[i]->GetStateImage());
			tisw.hInsertAfter=m_pTree->InsertItem(&tisw);
		}


		if (pItems[i]->pChilds!=NULL)
			InsertItemsToTree(!IsFullUnicodeSupport()?tisa.hInsertAfter:tisw.hInsertAfter,pItems[i]->pChilds,pItems[i]);

		// Type specified actions
		switch (pItems[i]->nType)
		{
		case Item::RadioBox:
			if (pItems[i]->bChecked)
				hSelectedRadioItem=!IsFullUnicodeSupport()?tisa.hInsertAfter:tisw.hInsertAfter;
			// Continuing
		case Item::CheckBox:
			EnableChilds(!IsFullUnicodeSupport()?tisa.hInsertAfter:tisw.hInsertAfter,pItems[i]->bChecked);
			break;
		case Item::Edit:
            pItems[i]->hControl=CreateWindow("EDIT","",
				ES_AUTOHSCROLL|WS_TABSTOP|WS_CHILDWINDOW|WS_BORDER,
				10,10,100,13,*this,(HMENU)IDC_EDITCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
			::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);
			
			// Initializing
			if (pItems[i]->pProc!=NULL)
			{
				bp.crReason=BASICPARAMS::Initialize;
				bp.hControl=pItems[i]->hControl;
				pItems[i]->pProc(&bp);
			}
			if (pItems[i]->pData!=NULL)
				::SendMessage(pItems[i]->hControl,WM_SETTEXT,0,LPARAM(pItems[i]->pData));
			break;
		case Item::Numeric:
			{
				pItems[i]->hControl=CreateWindow("EDIT","",
					ES_AUTOHSCROLL|WS_TABSTOP|WS_CHILDWINDOW|WS_BORDER|ES_NUMBER,
					10,10,50,20,*this,(HMENU)IDC_EDITCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
				::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);
	
				// Initializing
				if (pItems[i]->pProc!=NULL)
				{
					bp.crReason=BASICPARAMS::Initialize;
					bp.hControl=pItems[i]->hControl;
					pItems[i]->pProc(&bp);
				}
				char szText[100];
				itoa(pItems[i]->lValue,szText,10);
				::SendMessage(pItems[i]->hControl,WM_SETTEXT,0,LPARAM(szText));
			}
			break;
		case Item::List:
			pItems[i]->hControl=CreateWindow("COMBOBOX","",
				CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP|WS_CHILDWINDOW|WS_BORDER,
				10,10,100,100,*this,(HMENU)IDC_COMBOCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
			::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

			// Initializing
			if (pItems[i]->pProc!=NULL)
			{
				bp.crReason=BASICPARAMS::Initialize;
				bp.hControl=pItems[i]->hControl;
				pItems[i]->pProc(&bp);
			}
			::SendMessage(pItems[i]->hControl,CB_SETCURSEL,pItems[i]->lValue,NULL);
		

			break;
		case Item::Combo:
			pItems[i]->hControl=CreateWindow("COMBOBOX","",
				CBS_DROPDOWN|WS_VSCROLL|WS_TABSTOP|WS_CHILDWINDOW|WS_BORDER,
				10,10,100,100,*this,(HMENU)IDC_COMBOCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
			::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

			// Initializing
			if (pItems[i]->pProc!=NULL)
			{
				bp.crReason=BASICPARAMS::Initialize;
				bp.hControl=pItems[i]->hControl;
				pItems[i]->pProc(&bp);
			}
			
			if (pItems[i]->pData!=NULL)
			{
				// Checking whether value is found in combo
				int nFind=::SendMessage(pItems[i]->hControl,CB_FINDSTRINGEXACT,0,LPARAM(pItems[i]->pData));
				::SendMessage(pItems[i]->hControl,CB_SETCURSEL,nFind,0);
				if (nFind==CB_ERR)
					::SendMessage(pItems[i]->hControl,WM_SETTEXT,0,LPARAM(pItems[i]->pData));
			}
			break;
		}
		
		// Setting text
		LPWSTR pCurText=pItems[i]->GetText();
		if (pCurText!=pItems[i]->pString)
		{
			if (!IsFullUnicodeSupport())
			{
				DWORD iStrLen;
				dstrlen(pCurText,iStrLen);
				tisa.itemex.pszText=new char[iStrLen+2];
				MemCopyWtoA(tisa.itemex.pszText,pCurText,iStrLen+1);
				m_pTree->SetItemText(tisa.hInsertAfter,tisa.itemex.pszText);
				delete[] tisa.itemex.pszText;
			}
			else
				m_pTree->SetItemText(tisw.hInsertAfter,pCurText);
			
		}


		
		if (pItems[i]->hControl!=NULL)
		{
			CRect rc;
			::GetWindowRect(pItems[i]->hControl,&rc);
			if (rc.Height()-2>nItemHeight)
				nItemHeight=rc.Height()-2;
		}
	}

	// Ensuring that one radio is at least selected
	if (hSelectedRadioItem==NULL)
	{
		HTREEITEM hItem;
		if (hParent==NULL)
			hItem=m_pTree->GetNextItem(NULL,TVGN_ROOT);
		else
			hItem=m_pTree->GetNextItem(hParent,TVGN_CHILD);

		while (hItem!=NULL)
		{
			Item* pItem=(Item*)m_pTree->GetItemData(hItem);
			if (pItem!=NULL)
			{
				if (pItem->nType==Item::RadioBox)
				{
					hSelectedRadioItem=hItem;
					SetCheckState(hItem,pItem,Checked);					
					break;
				}
			}            			
			hItem=m_pTree->GetNextItem(hItem,TVGN_NEXT);
		}
	}
	else
		UncheckOtherRadioButtons(hSelectedRadioItem,hParent);

	if (nItemHeight>m_pTree->GetItemHeight())
		m_pTree->SetItemHeight(nItemHeight);
	return TRUE;
}

BOOL COptionsPropertyPage::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CPropertyPage::OnCommand(wID,wNotifyCode,hControl);

	switch (wID)
	{
	case IDC_EDITCONTROLFORSELECTEDITEM:
		switch (wNotifyCode)
		{
		case EN_CHANGE:
			{
				HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
				if (hItem==NULL)
					break;
				Item* pItem=(Item*)m_pTree->GetItemData(hItem);
				if (pItem==NULL)
					break;
				if (pItem->hControl!=hControl)
					break;

				if (pItem->nType==Item::Numeric)
					SetNumericValue(pItem);
				else if (pItem->nType==Item::Edit)
					SetTextValue(pItem);
				break;
			}
		case EN_SETFOCUS:
			::SendMessage(hControl,EM_SETSEL,0,MAKELPARAM(0,-1));
			break;
		}
		break;
	case IDC_SPINCONTROLFORSELECTEDITEM:
		break;
	case IDC_COMBOCONTROLFORSELECTEDITEM:
		switch (wNotifyCode)
		{
		case CBN_SELCHANGE:
			{
				HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
				if (hItem==NULL)
					break;
				Item* pItem=(Item*)m_pTree->GetItemData(hItem);
				if (pItem==NULL)
					break;
				if (pItem->hControl!=hControl)
					break;
				if (pItem->nType==Item::Combo)
					SetTextValue(pItem);
				else if (pItem->nType==Item::List)
					SetListValue(pItem);
				break;
			}
		case CBN_EDITCHANGE:
			{
				HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
				if (hItem==NULL)
					break;
				Item* pItem=(Item*)m_pTree->GetItemData(hItem);
				if (pItem==NULL)
					break;
				if (pItem->hControl!=hControl)
					break;

				if (pItem->nType==Item::Combo)
					SetTextValue(pItem);
				break;
			}
		case CBN_SETFOCUS:
			::SendMessage(hControl,CB_SETEDITSEL,0,MAKELPARAM(0,-1));
			break;
		default:
			CAppData::stdfunc();
			break;
		}
		break;
	}
	return FALSE;
}

BOOL COptionsPropertyPage::OnApply()
{
	CPropertyPage::OnApply();
	if (m_pItems!=NULL)
		CallApply(m_pItems);
	return TRUE;
}

void COptionsPropertyPage::CallApply(Item** pItems)
{
	COMBOAPPLYPARAMS bp;
	bp.pPage=this;
	bp.crReason=BASICPARAMS::Apply;

	for (int i=0;pItems[i]!=NULL;i++)
	{
		if (pItems[i]->bEnabled)
		{
			if (pItems[i]->pProc!=NULL)
			{
				pItems[i]->SetValuesForBasicParams(&bp);
				if (pItems[i]->nType==Item::Combo)
					bp.nCurSel=::SendMessage(pItems[i]->hControl,CB_GETCURSEL,0,0);
				pItems[i]->pProc(&bp);
			}
			if (pItems[i]->pChilds!=NULL)
				CallApply(pItems[i]->pChilds);
		}
	}
}

void COptionsPropertyPage::OnDestroy()
{
	CPropertyPage::OnDestroy();

	if (m_pTree!=NULL)
	{
		delete m_pTree;
		m_pTree=NULL;
	}

	if (m_pItems!=NULL)
	{
		for (int i=0;m_pItems[i]!=NULL;i++)
			delete m_pItems[i];
		delete[] m_pItems;
	}
}
	
void COptionsPropertyPage::OnActivate(WORD fActive,BOOL fMinimized,HWND hwnd)
{
	CPropertyPage::OnActivate(fActive,fMinimized,hwnd);

	if (fActive!=WA_INACTIVE)
		PostMessage(WM_REDRAWSELITEMCONTROL);
}

/*void COptionsPropertyPage::OnTimer(DWORD wTimerID)
{
	switch (wTimerID)
	{
	case 0:
		KillTimer(0);
		PostMessage(WM_REDRAWSELITEMCONTROL);
		break;
	}
	CPropertyPage::OnTimer(wTimerID);
}*/

	
BOOL COptionsPropertyPage::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
	case WM_REDRAWSELITEMCONTROL:
		{
			HTREEITEM hActiveItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
			if (hActiveItem==NULL)
				return TRUE;

			Item* pItem=(Item*)m_pTree->GetItemData(hActiveItem);
			if (pItem==NULL)
				return TRUE;

			if (pItem->hControl!=NULL)
			{
				// Checking that should position change
				RECT rcItem,rcOrig,rcTree,rcOther;
				m_pTree->GetClientRect(&rcTree);
				m_pTree->GetItemRect(hActiveItem,&rcItem,TRUE);
				BOOL bNotVisible=rcItem.top<0 || rcItem.bottom>rcTree.bottom;
				m_pTree->ClientToScreen(&rcItem);
				
				::GetWindowRect(pItem->hControl,&rcOrig);
				if (pItem->hControl2!=NULL)
				{
					::GetWindowRect(pItem->hControl2,&rcOther);
					rcOther.left-=rcOrig.left;
					rcOther.top-=rcOrig.top;
				}


				if (rcOrig.left!=rcItem.right+1 || rcOrig.top!=rcItem.top-1)
				{
					// Moving control to correct place
					ScreenToClient(&rcItem);
					::SetWindowPos(pItem->hControl,HWND_TOP,rcItem.right+1,rcItem.top-1,0,0,
						(bNotVisible?SWP_HIDEWINDOW:SWP_SHOWWINDOW)|SWP_NOSIZE|SWP_NOACTIVATE);
					

					if (pItem->hControl2!=NULL)
					{
						::SetWindowPos(pItem->hControl2,HWND_TOP,rcItem.right+1+rcOther.left,rcItem.top-1+rcOther.top,0,0,
							(bNotVisible?SWP_HIDEWINDOW:SWP_SHOWWINDOW)|SWP_NOSIZE|SWP_NOACTIVATE);
						::InvalidateRect(pItem->hControl2,NULL,FALSE);
					}

				}

				::InvalidateRect(pItem->hControl,NULL,FALSE);
				if (pItem->hControl2!=NULL)
					::InvalidateRect(pItem->hControl2,NULL,FALSE);
			}
			break;
		}
	case WM_FOCUSSELITEMCONTROL:
		{
			HTREEITEM hActiveItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
			if (hActiveItem==NULL)
				return TRUE;

			Item* pItem=(Item*)m_pTree->GetItemData(hActiveItem);
			if (pItem==NULL)
				return TRUE;
			
			if (pItem->hControl!=NULL)
			{
				::InvalidateRect(pItem->hControl,NULL,FALSE);
				::SetFocus(pItem->hControl);
			}

			break;
		}
	}
	return CPropertyPage::WindowProc(msg,wParam,lParam);
}

BOOL COptionsPropertyPage::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	if (idCtrl==m_nTreeID)
	{
		CPropertyPage::OnNotify(idCtrl,pnmh);
		BOOL bRet=TreeNotifyHandler((NMTVDISPINFO*)pnmh,(NMTREEVIEW*)pnmh);
		if (bRet)
			SetWindowLong(dwlMsgResult,bRet);
		return bRet;
	}			
	return CPropertyPage::OnNotify(idCtrl,pnmh);
}

BOOL COptionsPropertyPage::TreeNotifyHandler(NMTVDISPINFO *pTvdi,NMTREEVIEW *pNm)
{
	switch (pNm->hdr.code)
	{
	case TVN_KEYDOWN:
		if (((NMTVKEYDOWN*)pNm)->wVKey==VK_SPACE)
		{
			HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
			if (hItem==NULL)
				break;
			Item* pItem=(Item*)m_pTree->GetItemData(hItem);
			if (pItem==NULL)
				break;
			if (pItem->bEnabled && (pItem->nType==Item::RadioBox || pItem->nType==Item::CheckBox))
				SetCheckState(hItem,pItem,Toggle);
			
		}
		break;
	case NM_CLICK:
	case NM_DBLCLK:
		{
			TVHITTESTINFO ht;
			GetCursorPos(&ht.pt);
			m_pTree->ScreenToClient(&ht.pt);
			HTREEITEM hItem=m_pTree->HitTest(&ht);
			if (hItem==NULL)
				break;
			Item* pItem=(Item*)m_pTree->GetItemData(hItem);
			if (pItem==NULL)
				break;
			if (pItem->nType==Item::RadioBox || pItem->nType==Item::CheckBox)
			{
				if (pItem->bEnabled && (ht.flags&TVHT_ONITEMSTATEICON || pNm->hdr.code==NM_DBLCLK))
					SetCheckState(hItem,pItem,Toggle);
			}
			break;
		}
	case TVN_SELCHANGING:
		// Checking if selection cannot be vhanged
		if (pNm->itemNew.lParam!=NULL)
		{
			if (!((Item*)pNm->itemNew.lParam)->bEnabled)
				return TRUE;
		}

		// Hiding control for previous item
		if (pNm->itemOld.lParam!=NULL)
		{
			Item* pItem=(Item*)pNm->itemOld.lParam;
			if (pItem->hControl!=NULL)
			{
				// Hiding window and ensuring that that part of tree is redrawn
				RECT rc;
				::GetWindowRect(pItem->hControl,&rc);
				::ShowWindow(pItem->hControl,SW_HIDE);
				m_pTree->ScreenToClient(&rc);
				::InvalidateRect(*m_pTree,&rc,FALSE);
				
				// Setting text
				WCHAR* pText=pItem->GetText(FALSE);
				if (!IsFullUnicodeSupport())
				{
					DWORD iStrLen;
					dstrlen(pText,iStrLen);
					char* paText=new char [iStrLen+2];
                    MemCopyWtoA(paText,pText,iStrLen+1);
					m_pTree->SetItemText(pNm->itemOld.hItem,paText);
					delete[] paText;
				
				}
				else
					m_pTree->SetItemText(pNm->itemOld.hItem,pText);
				pItem->FreeText(pText);
				
				// Deleting another control
				if (pItem->hControl2!=NULL)
				{
					::DestroyWindow(pItem->hControl2);
					pItem->hControl2=NULL;
				}
			}
		}
		// Showing control for previous item
		if (pNm->itemNew.lParam!=NULL)
		{
			Item* pItem=(Item*)pNm->itemNew.lParam;
			if (pItem->hControl!=NULL)
			{
				// Changing text
				// Setting text
				WCHAR* pText=pItem->GetText(TRUE);
				if (!IsFullUnicodeSupport())
				{
					DWORD iStrLen;
					dstrlen(pText,iStrLen);
					char* paText=new char [iStrLen+2];
                    MemCopyWtoA(paText,pText,iStrLen+1);
					m_pTree->SetItemText(pNm->itemNew.hItem,paText);
					delete[] paText;
				
				}
				else
					m_pTree->SetItemText(pNm->itemNew.hItem,pText);
				
				
				// Show control
				::ShowWindow(((Item*)pNm->itemNew.lParam)->hControl,SW_SHOW);
                
				// Moving it
				RECT rc;
				m_pTree->GetItemRect(pNm->itemNew.hItem,&rc,TRUE);
				m_pTree->ClientToScreen(&rc);
				ScreenToClient(&rc);
				
				int nWidth=60; // 60 is for numeric
				if (pItem->nType!=Item::Numeric)
				{
					RECT rcTree;
					m_pTree->GetClientRect(&rcTree);
					nWidth=rcTree.right-rc.right;
				}
				::SetWindowPos(pItem->hControl,HWND_TOP,0,0,nWidth,20,SWP_SHOWWINDOW|SWP_NOMOVE);


				
				if (pItem->nType==Item::Numeric)
				{
					// Creating Up/Down control
					pItem->hControl2=CreateWindow("msctls_updown32","",
						UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_ARROWKEYS|WS_TABSTOP|WS_CHILDWINDOW|WS_VISIBLE,
						rc.right+20,rc.top-1,10,10,*this,(HMENU)IDC_SPINCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
					::SendMessage(pItem->hControl2,UDM_SETBUDDY,WPARAM(pItem->hControl),NULL);
					
					if (pItem->pProc!=NULL)
					{
						SPINPOXPARAMS spb;
						spb.iLow=0;
						spb.iHigh=MAXLONG;
						pItem->SetValuesForBasicParams(&spb);
						spb.crReason=SPINPOXPARAMS::SetSpinRange;
						spb.pPage=this;
						pItem->pProc(&spb);
						::SendMessage(pItem->hControl2,UDM_SETRANGE32,spb.iLow,spb.iHigh);
					}
					else
						::SendMessage(pItem->hControl2,UDM_SETRANGE32,0,MAXLONG);

					::SetWindowPos(pItem->hControl2,HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
				}

				PostMessage(WM_FOCUSSELITEMCONTROL);
			}
		}
		break;
	case TVN_SELCHANGED:
		/*if (pNm->itemNew.lParam!=NULL)
		{
			if (((Item*)pNm->itemNew.lParam)->hControl!=NULL)
				::SetFocus(((Item*)pNm->itemNew.lParam)->hControl);
		}*/
		break;
	case TVN_ITEMEXPANDING:
		if (pNm->action==TVE_COLLAPSE || pNm->action==TVE_TOGGLE)
			return TRUE;
		return FALSE;
	case NM_CUSTOMDRAW:
		{
			NMTVCUSTOMDRAW* pCustomDraw=(NMTVCUSTOMDRAW*)pNm;
			if (pCustomDraw->nmcd.dwDrawStage==CDDS_PREPAINT)
				return CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
			else if (pCustomDraw->nmcd.dwDrawStage==CDDS_POSTPAINT)
			{
				PostMessage(WM_REDRAWSELITEMCONTROL);
				return CDRF_NOTIFYITEMDRAW;
			}
			else if (pCustomDraw->nmcd.dwDrawStage&CDDS_ITEMPREPAINT)
			{
				Item* pItem=(Item*)pCustomDraw->nmcd.lItemlParam;
				if (!pItem->bEnabled)
					pCustomDraw->clrText=GetSysColor(COLOR_GRAYTEXT);
				return CDRF_DODEFAULT;
			}
			break;
		}
	}
	return FALSE;
}


BOOL COptionsPropertyPage::SetCheckState(HTREEITEM hItem,COptionsPropertyPage::Item* pItem,
										 COptionsPropertyPage::NewState nNewState)
{
	if (nNewState==Toggle && pItem->nType==Item::RadioBox)
		nNewState=Checked;

	if (pItem->pProc!=NULL)
	{
		CHANGINGVALPARAMS cp;
		cp.crReason=BASICPARAMS::ChangingValue;
		cp.pPage=this;
		pItem->SetValuesForBasicParams(&cp);
		cp.nNewState=nNewState;
		if (!pItem->pProc(&cp))
			return FALSE;
	}

	if (pItem->nType==Item::CheckBox || pItem->nType==Item::RadioBox)
	{
		if (nNewState==Toggle)
		    pItem->bChecked=!pItem->bChecked;
		else if (nNewState==Checked)
		{
			if (pItem->bChecked)
				return FALSE;
			pItem->bChecked=TRUE;
		}
		else
		{
			if (!pItem->bChecked)
				return FALSE;
			pItem->bChecked=FALSE;
		}
		m_pTree->SetItemState(hItem,INDEXTOSTATEIMAGEMASK(pItem->GetStateImage()),TVIS_STATEIMAGEMASK);

		if (pItem->nType==Item::RadioBox && pItem->bChecked)
			UncheckOtherRadioButtons(hItem,m_pTree->GetParentItem(hItem));
		
		EnableChilds(hItem,pItem->bChecked);
		
		
		if (pItem->pProc!=NULL)
		{
			BASICPARAMS bp;
			bp.crReason=BASICPARAMS::Set;
			bp.pPage=this;
			pItem->SetValuesForBasicParams(&bp);
			pItem->pProc(&bp);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL COptionsPropertyPage::SetNumericValue(Item* pItem)
{
	int iTextLen=::SendMessage(pItem->hControl,WM_GETTEXTLENGTH,0,0)+1;
	char* szText=new char[iTextLen];
	::SendMessage(pItem->hControl,WM_GETTEXT,iTextLen,LPARAM(szText));

	CHANGINGVALPARAMS cp;
	cp.lNewValue=atol(szText);
	delete[] szText;

	// Asking wheter value can be changed
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::ChangingValue;
		pItem->SetValuesForBasicParams(&cp);
		cp.pPage=this;
		if (!pItem->pProc(&cp))
			return FALSE;
	}
	pItem->lValue=cp.lNewValue;
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::Set;
		cp.lValue=pItem->lValue;
		pItem->pProc(&cp);
	}
	return TRUE;
}

BOOL COptionsPropertyPage::SetTextValue(Item* pItem)
{
	CHANGINGVALPARAMS cp;
	int iTextLen,iCurSel;
	switch (pItem->nType)
	{
	case Item::Combo:
	case Item::List:
		iCurSel=::SendMessage(pItem->hControl,CB_GETCURSEL,0,0);
		if (iCurSel!=CB_ERR)
		{
			iTextLen=::SendMessage(pItem->hControl,CB_GETLBTEXTLEN,iCurSel,0)+2;			
			cp.pNewData=new char[iTextLen];
			::SendMessage(pItem->hControl,CB_GETLBTEXT,iCurSel,LPARAM(cp.pNewData));
		}
		else
		{
			iTextLen=::SendMessage(pItem->hControl,WM_GETTEXTLENGTH,iCurSel,0)+2;			
			cp.pNewData=new char[iTextLen];
			::SendMessage(pItem->hControl,WM_GETTEXT,iTextLen,LPARAM(cp.pNewData));
		}

		break;
	default:
		iTextLen=::SendMessage(pItem->hControl,WM_GETTEXTLENGTH,0,0)+1;
		cp.pNewData=new char[iTextLen];
		::SendMessage(pItem->hControl,WM_GETTEXT,iTextLen,LPARAM(cp.pNewData));
		break;
	}
	// Asking wheter value can be changed
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::ChangingValue;
		pItem->SetValuesForBasicParams(&cp);
		cp.pPage=this;
		if (!pItem->pProc(&cp))
		{
			delete[] cp.pNewData;
			return FALSE;
		}
	}
	if (pItem->pData!=NULL)
		delete[] pItem->pData;
	pItem->pData=cp.pNewData;
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::Set;
		cp.pData=cp.pData;
		pItem->pProc(&cp);
	}
	return TRUE;
}

BOOL COptionsPropertyPage::SetListValue(Item* pItem)
{
	CHANGINGVALPARAMS cp;
	cp.lNewValue=::SendMessage(pItem->hControl,CB_GETCURSEL,0,0);
	
	// Asking wheter value can be changed
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::ChangingValue;
		pItem->SetValuesForBasicParams(&cp);
		cp.pPage=this;
		if (!pItem->pProc(&cp))
			return FALSE;
	}
	pItem->lValue=cp.lNewValue;
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::Set;
		cp.lValue=pItem->lValue;
		pItem->pProc(&cp);
	}
	return TRUE;
}

	
void COptionsPropertyPage::EnableChilds(HTREEITEM hItem,BOOL bEnable)
{
	HTREEITEM hChildItem=m_pTree->GetNextItem(hItem,TVGN_CHILD);
    while (hChildItem!=NULL)
	{
		Item* pItem=(Item*)m_pTree->GetItemData(hChildItem);
		if (pItem!=NULL)
			pItem->bEnabled=bEnable;
		
		m_pTree->SetItemState(hChildItem,bEnable?0:TVIS_CUT,TVIS_CUT);
		
		hChildItem=m_pTree->GetNextItem(hChildItem,TVGN_NEXT);
	}
}

void COptionsPropertyPage::UncheckOtherRadioButtons(HTREEITEM hItem,HTREEITEM hParent)
{
	if (hParent==NULL)
		return;
	
	HTREEITEM hChilds;
	if (hParent==NULL)
		hChilds=m_pTree->GetNextItem(NULL,TVGN_ROOT);
	else
		hChilds=m_pTree->GetNextItem(hParent,TVGN_CHILD);



	while (hChilds!=NULL)
	{
		Item* pItem=(Item*)m_pTree->GetItemData(hChilds);
		if (pItem!=NULL)
		{
			if (pItem->nType==Item::RadioBox && hChilds!=hItem)
				SetCheckState(hChilds,(Item*)m_pTree->GetItemData(hChilds),Unchecked);
		}
		hChilds=m_pTree->GetNextItem(hChilds,TVGN_NEXT);
	}
}

WCHAR* COptionsPropertyPage::Item::GetText(BOOL bActive) const
{
	switch (nType)
	{
	case Numeric:
		if (hControl!=NULL && !bActive)
		{
			WCHAR szText[100];
			_itow(lValue,szText,10);
			int iLength=istrlenw(szText)+1;
			int iLabelLen=istrlenw(pString);
			
			WCHAR* pText=new WCHAR[iLabelLen+iLength+2];
			sMemCopyW(pText,pString,iLabelLen);
			pText[iLabelLen++]=' ';
			sMemCopyW(pText+iLabelLen,szText,iLength);
			return pText;
		}
		return pString;
	case List:
    case Combo:
		if (hControl!=NULL && !bActive)
		{
			int nCurSel=::SendMessage(hControl,CB_GETCURSEL,0,0);
			int iLength=(nCurSel!=-1)?
				::SendMessage(hControl,CB_GETLBTEXTLEN,nCurSel,0)+1:
				::SendMessage(hControl,WM_GETTEXTLENGTH,0,0)+1;
			int iLabelLen=istrlenw(pString);
			
			WCHAR* pText=new WCHAR[iLabelLen+iLength+2];
			sMemCopyW(pText,pString,iLabelLen);
			pText[iLabelLen++]=' ';
				
			if (nCurSel!=-1)
			{
				char* pTemp=new char[iLength+2];
				::SendMessage(hControl,CB_GETLBTEXT,nCurSel,LPARAM(pTemp));
				MemCopyAtoW(pText+iLabelLen,pTemp,iLength+1);
				delete[] pTemp;
			}
			else
			{
				if (!IsFullUnicodeSupport())
				{
					// 9x
					char* pTemp=new char[iLength+2];
					::GetWindowText(hControl,pTemp,iLength);
					MemCopyAtoW(pText+iLabelLen,pTemp,iLength+1);
					delete[] pTemp;
				}
				else
					::GetWindowTextW(hControl,pText+iLabelLen,iLength);
			}

			return pText;
		}
		return pString;
	case Edit:
		if (hControl!=NULL && !bActive)
		{
			int iLength=::SendMessage(hControl,WM_GETTEXTLENGTH,0,0)+1;
			int iLabelLen=istrlenw(pString);
			
			WCHAR* pText=new WCHAR[iLabelLen+iLength+1];
			sMemCopyW(pText,pString,iLabelLen);
			pText[iLabelLen++]=' ';
				
			if (!IsFullUnicodeSupport())
			{
				// 9x
				char* pTemp=new char[iLength+2];
				::GetWindowText(hControl,pTemp,iLength);
				MemCopyAtoW(pText+iLabelLen,pTemp,iLength+1);
				delete[] pTemp;
			}
			else
				::GetWindowTextW(hControl,pText+iLabelLen,iLength);
				
			return pText;
		}
		return pString;
	case RadioBox:
	case CheckBox:
	case Root:
	default:
		return pString;
	}
}

// lParam is pointer to DWORD value which is will be set
// wParam is used mask
BOOL CALLBACK COptionsPropertyPage::DefaultCheckBoxProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch (pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		pParams->bChecked=(*((DWORD*)pParams->lParam))&pParams->wParam?TRUE:FALSE;
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		if (pParams->bChecked)
			*((DWORD*)pParams->lParam)|=DWORD(pParams->wParam);
		else
			*((DWORD*)pParams->lParam)&=~DWORD(pParams->wParam);
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}
	return TRUE;
}

// lParam is pointer to DWORD value which is will be set
// HIWORD of wParam is mask to be setted, LOWORD is value
BOOL CALLBACK COptionsPropertyPage::DefaultRadioBoxProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch (pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		if (pParams->lParam!=NULL)
			pParams->bChecked=((*((DWORD*)pParams->lParam))&(HIWORD(pParams->wParam)))==LOWORD(pParams->wParam);
		else
			pParams->bChecked=0;
		break;
	case BASICPARAMS::Set:
		/*
		*((DWORD*)pParams->lParam)&=~HIWORD(pParams->wParam);
		if (pParams->bChecked)
			*((DWORD*)pParams->lParam)|=LOWORD(pParams->wParam);
		*/
		break;
	case BASICPARAMS::Apply:
		if (pParams->bChecked && pParams->lParam!=NULL)
		{
			*((DWORD*)pParams->lParam)&=~HIWORD(pParams->wParam);
			*((DWORD*)pParams->lParam)|=LOWORD(pParams->wParam);
		}
		break;		
	case BASICPARAMS::ChangingValue:
		break;
	}
	return TRUE;
}

// lParam is pointer to DWORD value which is will be set
// HIWORD of wParam is mask (shifted 16 bits) to be setted, LOWORD is value (shifted 16 bit)
BOOL CALLBACK COptionsPropertyPage::DefaultRadioBoxShiftProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch (pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		pParams->bChecked=((*((DWORD*)pParams->lParam))&(HIWORD(pParams->wParam)<<16))==LOWORD(pParams->wParam)<<16;
		break;
	case BASICPARAMS::Set:
		/*
		*((DWORD*)pParams->lParam)&=~HIWORD(pParams->wParam);
		if (pParams->bChecked)
			*((DWORD*)pParams->lParam)|=LOWORD(pParams->wParam);
		*/
		break;
	case BASICPARAMS::Apply:
		if (pParams->bChecked)
		{
			*((DWORD*)pParams->lParam)&=~(HIWORD(pParams->wParam)<<16);
			*((DWORD*)pParams->lParam)|=LOWORD(pParams->wParam)<<16;
		}
		break;		
	case BASICPARAMS::ChangingValue:
		break;
	}
	return TRUE;
}

// lParam is pointer to DWORD value which is will be set
// if wParam==0, all values are accepted
// if wParam==-1, positive values are accepted
// otherwise HIWORD is maximum, LOWORD is minimum
BOOL CALLBACK COptionsPropertyPage::DefaultNumericProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch (pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		pParams->lValue=*((DWORD*)pParams->lParam);
		if (pParams->wParam==0) 
			break; // Accept all values
		else if (pParams->wParam==DWORD(-1))
		{
			// -1: Accept only nonnegative values
			if (pParams->lValue<0)
				pParams->lValue=0;
		}
		else if (pParams->lValue>int(HIWORD(pParams->wParam)))
			pParams->lValue=int(HIWORD(pParams->wParam));
		else if (pParams->lValue<int(LOWORD(pParams->wParam)))
			pParams->lValue=int(LOWORD(pParams->wParam));
		break;
	case BASICPARAMS::Set:
		/*
		*((DWORD*)pParams->lParam)&=~HIWORD(pParams->wParam);
		if (pParams->bChecked)
			*((DWORD*)pParams->lParam)|=LOWORD(pParams->wParam);
		*/
		break;
	case BASICPARAMS::Apply:
		*((DWORD*)pParams->lParam)=pParams->lValue;
		break;		
	case BASICPARAMS::SetSpinRange:
		if (pParams->wParam==0)
		{
			((SPINPOXPARAMS*)pParams)->iLow=MINLONG;
			((SPINPOXPARAMS*)pParams)->iHigh=MAXLONG;
		}
		else if (pParams->wParam==DWORD(-1))
		{
			((SPINPOXPARAMS*)pParams)->iLow=0;
			((SPINPOXPARAMS*)pParams)->iHigh=MAXLONG;
		}
		else
		{
			((SPINPOXPARAMS*)pParams)->iLow=LOWORD(pParams->wParam);
			((SPINPOXPARAMS*)pParams)->iHigh=HIWORD(pParams->wParam);
		}
		break;
	case BASICPARAMS::ChangingValue:
		if (pParams->wParam==0) // 
			break;
		else if (pParams->wParam==DWORD(-1))
		{
			if (((CHANGINGVALPARAMS*)pParams)->lNewValue<0)
				return FALSE;
		}
		else if (((CHANGINGVALPARAMS*)pParams)->lNewValue<int(LOWORD(pParams->wParam)) || 
			((CHANGINGVALPARAMS*)pParams)->lNewValue>int(HIWORD(pParams->wParam)))
			return FALSE;
		break;
	}
	return TRUE;
}

// lParam is pointer to string class which will be set
BOOL CALLBACK COptionsPropertyPage::DefaultEditStrProc(BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		pParams->pData=alloccopy(*(CString*)pParams->lParam);
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		if (pParams->pData==NULL)
			((CString*)pParams->lParam)->Empty();
		else
			((CString*)pParams->lParam)->Copy(pParams->pData);
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}		
	return TRUE;
}
#endif