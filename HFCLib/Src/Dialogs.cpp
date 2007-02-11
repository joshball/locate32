////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2007 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#if defined(HFC_USEDEBUGNEW)
	#define new DEBUG_NEW
#endif

#if defined(DEF_RESOURCES) && defined(DEF_WINDOWS)

///////////////////////////
// Class CDialog
///////////////////////////

CDialog::CDialog(LPCSTR lpTemplate)
	: CTargetWnd(NULL)
{
	if (IS_INTRESOURCE(lpTemplate))
		m_lpszTemplateName=MAKEINTRESOURCE(lpTemplate);
	else if (IsUnicodeSystem())
		m_lpszTemplateNameW=alloccopyAtoW(lpTemplate);
	else
		m_lpszTemplateName=alloccopy(lpTemplate);
}

CDialog::CDialog(LPCWSTR lpTemplate)
	: CTargetWnd(NULL)
{
	if (IS_INTRESOURCE(lpTemplate))
		m_lpszTemplateName=MAKEINTRESOURCE(lpTemplate);
	else if (IsUnicodeSystem())
		m_lpszTemplateNameW=alloccopy(lpTemplate);
	else
		m_lpszTemplateName=alloccopyWtoA(lpTemplate);
}

CDialog::~CDialog()
{
	if (m_lpszTemplateName!=NULL && !IS_INTRESOURCE(m_lpszTemplateName))
	{
		if (IsUnicodeSystem())
			delete[] m_lpszTemplateNameW;
		else
			delete[] m_lpszTemplateName;
	}
}

BOOL CDialog::OnInitDialog(HWND hwndFocus)
{
	AddDebugMenuItems(*this);
	
	return FALSE;
}

LRESULT CDialog::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
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

CPropertyPage::~CPropertyPage()
{
	if (IsUnicodeSystem())
	{
		if (m_pspw.pszTitle!=NULL)
			delete[] m_pspw.pszTitle;
	}
	else
	{
		if (m_psp.pszTitle!=NULL)
			delete[] m_psp.pszTitle;
	}
}

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

void CPropertyPage::Construct(UINT nIDCaption,TypeOfResourceHandle bType)
{
	if (IsUnicodeSystem())
	{
		ZeroMemory(&m_pspw,sizeof(PROPSHEETPAGEW));
		m_pspw.dwSize=sizeof(PROPSHEETPAGEW);
		m_pspw.dwFlags=PSP_DEFAULT ; //PSP_USECALLBACK;
		if (m_lpszTemplateNameW!=NULL)
		{
			m_pspw.hInstance=GetResourceHandle(bType);
			m_pspw.pszTemplate=m_lpszTemplateNameW;
		}
		m_pspw.pfnDlgProc=(DLGPROC)CAppData::PropPageWndProc;
		m_pspw.lParam=(LPARAM)this;
		if (nIDCaption)
		{
			m_pspw.pszTitle=allocstringW(nIDCaption);
			m_pspw.dwFlags|=PSP_USETITLE;
		}
		m_pspw.pfnCallback=NULL; //PropertySheetPageProc;
		m_bFirstSetActive=TRUE;
	}
	else
	{
		ZeroMemory(&m_psp,sizeof(PROPSHEETPAGE));
		m_psp.dwSize=sizeof(PROPSHEETPAGE);
		m_psp.dwFlags=PSP_DEFAULT ; //PSP_USECALLBACK;
		if (m_lpszTemplateName!=NULL)
		{
			m_psp.hInstance=GetResourceHandle(bType);
			m_psp.pszTemplate=m_lpszTemplateName;
		}
		m_psp.pfnDlgProc=(DLGPROC)CAppData::PropPageWndProc;
		m_psp.lParam=(LPARAM)this;
		if (nIDCaption)
		{
			m_psp.pszTitle=allocstring(nIDCaption);
			m_psp.dwFlags|=PSP_USETITLE;
		}
		m_psp.pfnCallback=NULL; //PropertySheetPageProc;
		m_bFirstSetActive=TRUE;
	}
}

void CPropertyPage::Construct(LPCSTR szTitle,TypeOfResourceHandle bType)
{
	if (IsUnicodeSystem())
	{
		ZeroMemory(&m_pspw,sizeof(PROPSHEETPAGEW));
		m_pspw.dwSize=sizeof(PROPSHEETPAGEW);
		m_pspw.dwFlags=PSP_DEFAULT ; //PSP_USECALLBACK;
		if (m_lpszTemplateNameW!=NULL)
		{
			m_pspw.hInstance=GetResourceHandle(bType);
			m_pspw.pszTemplate=m_lpszTemplateNameW;
		}
		m_pspw.pfnDlgProc=(DLGPROC)CAppData::PropPageWndProc;
		m_pspw.lParam=(LPARAM)this;
		m_pspw.pszTitle=alloccopyAtoW(szTitle);
		m_pspw.dwFlags|=PSP_USETITLE;
		m_pspw.pfnCallback=NULL; //PropertySheetPageProc;
		m_bFirstSetActive=TRUE;
	}
	else
	{
		ZeroMemory(&m_psp,sizeof(PROPSHEETPAGE));
		m_psp.dwSize=sizeof(PROPSHEETPAGE);
		m_psp.dwFlags=PSP_DEFAULT ; //PSP_USECALLBACK;
		if (m_lpszTemplateName!=NULL)
		{
			m_psp.hInstance=GetResourceHandle(bType);
			m_psp.pszTemplate=m_lpszTemplateName;
		}
		m_psp.pfnDlgProc=(DLGPROC)CAppData::PropPageWndProc;
		m_psp.lParam=(LPARAM)this;
		m_psp.pszTitle=alloccopy(szTitle);
		m_psp.dwFlags|=PSP_USETITLE;
		m_psp.pfnCallback=NULL; //PropertySheetPageProc;
		m_bFirstSetActive=TRUE;
	}
}

void CPropertyPage::Construct(LPCWSTR szTitle,TypeOfResourceHandle bType)
{
	if (IsUnicodeSystem())
	{
		ZeroMemory(&m_pspw,sizeof(PROPSHEETPAGEW));
		m_pspw.dwSize=sizeof(PROPSHEETPAGEW);
		m_pspw.dwFlags=PSP_DEFAULT ; //PSP_USECALLBACK;
		if (m_lpszTemplateNameW!=NULL)
		{
			m_pspw.hInstance=GetResourceHandle(bType);
			m_pspw.pszTemplate=m_lpszTemplateNameW;
		}
		m_pspw.pfnDlgProc=(DLGPROC)CAppData::PropPageWndProc;
		m_pspw.lParam=(LPARAM)this;
		m_pspw.pszTitle=alloccopy(szTitle);
		m_pspw.dwFlags|=PSP_USETITLE;
		m_pspw.pfnCallback=NULL; //PropertySheetPageProc;
		m_bFirstSetActive=TRUE;
	}
	else
	{
		ZeroMemory(&m_psp,sizeof(PROPSHEETPAGE));
		m_psp.dwSize=sizeof(PROPSHEETPAGE);
		m_psp.dwFlags=PSP_DEFAULT ; //PSP_USECALLBACK;
		if (m_lpszTemplateName!=NULL)
		{
			m_psp.hInstance=GetResourceHandle(bType);
			m_psp.pszTemplate=m_lpszTemplateName;
		}
		m_psp.pfnDlgProc=(DLGPROC)CAppData::PropPageWndProc;
		m_psp.lParam=(LPARAM)this;
		m_psp.pszTitle=alloccopyWtoA(szTitle);
		m_psp.dwFlags|=PSP_USETITLE;
		m_psp.pfnCallback=NULL; //PropertySheetPageProc;
		m_bFirstSetActive=TRUE;
	}
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
	::SetWindowLongPtr(m_hWnd,DWLP_MSGRESULT,(ULONG_PTR)ret);
	return TRUE;
}

///////////////////////////////////////////
// CPropertySheet
///////////////////////////////////////////

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

CPropertySheet::~CPropertySheet()
{
	if (m_psh.phpage!=NULL)
		delete[] m_psh.phpage;
	if (IsUnicodeSystem())
	{
		if (m_pshw.pszCaption)
			delete[] m_pshw.pszCaption;
		
		if (m_psh.dwFlags&=PSH_USEICONID && !IS_INTRESOURCE(m_psh.pszIcon))
			delete[] m_pshw.pszIcon;
	}
	else
	{
		if (m_pshw.pszCaption)
			delete[] m_psh.pszCaption;
		
		if (m_psh.dwFlags&=PSH_USEICONID && !IS_INTRESOURCE(m_psh.pszIcon))
			delete[] m_psh.pszIcon;
	}
}

void CPropertySheet::Construct(UINT nIDCaption,HWND hParentWnd,UINT iSelectPage)
{
	Construct(ID2W(nIDCaption),hParentWnd,iSelectPage);
}

void CPropertySheet::Construct(LPCSTR pszCaption,HWND hParentWnd,UINT iSelectPage)
{
	m_hParentWnd=hParentWnd;

	if (IsUnicodeSystem())
	{
		ZeroMemory(&m_pshw,sizeof(PROPSHEETHEADERW));
		m_pshw.dwSize=PROPSHEETHEADERW_V1_SIZE; //sizeof(PROPSHEETHEADER);
		
		if (pszCaption!=NULL)
			m_pshw.pszCaption=alloccopyAtoW(pszCaption);
		else
			DebugMessage("CPropertySheet::Construct(): pszCaption==NULL");
		m_pshw.dwFlags=PSH_DEFAULT;
		m_pshw.nStartPage=iSelectPage;
		m_pshw.pfnCallback=NULL;
		m_pshw.hInstance=GetCommonResourceHandle();
	}
	else
	{
		ZeroMemory(&m_psh,sizeof(PROPSHEETHEADER));
		m_psh.dwSize=PROPSHEETHEADER_V1_SIZE; //sizeof(PROPSHEETHEADER);
		
		if (pszCaption!=NULL)
			m_psh.pszCaption=alloccopy(pszCaption);
		else
			DebugMessage("CPropertySheet::Construct(): pszCaption==NULL");
		m_psh.dwFlags=PSH_DEFAULT;
		m_psh.nStartPage=iSelectPage;
		m_psh.pfnCallback=NULL;
		m_psh.hInstance=GetCommonResourceHandle();
	}
}

void CPropertySheet::Construct(LPCWSTR pszCaption,HWND hParentWnd,UINT iSelectPage)
{
	m_hParentWnd=hParentWnd;
	if (IsUnicodeSystem())
	{
		ZeroMemory(&m_pshw,sizeof(PROPSHEETHEADERW));
		m_pshw.dwSize=PROPSHEETHEADERW_V1_SIZE; //sizeof(PROPSHEETHEADER);
		
		if (pszCaption!=NULL)
			m_pshw.pszCaption=alloccopy(pszCaption);
		else
			DebugMessage("CPropertySheet::Construct(): pszCaption==NULL");
		m_pshw.dwFlags=PSH_DEFAULT;
		m_pshw.nStartPage=iSelectPage;
		m_pshw.pfnCallback=NULL;
		m_pshw.hInstance=GetCommonResourceHandle();
	}
	else
	{
		ZeroMemory(&m_psh,sizeof(PROPSHEETHEADERW));
		m_psh.dwSize=PROPSHEETHEADER_V1_SIZE; //sizeof(PROPSHEETHEADER);
		
		if (pszCaption!=NULL)
			m_psh.pszCaption=alloccopyWtoA(pszCaption);
		else
			DebugMessage("CPropertySheet::Construct(): pszCaption==NULL");
		m_psh.dwFlags=PSH_DEFAULT;
		m_psh.nStartPage=iSelectPage;
		m_psh.pfnCallback=NULL;
		m_psh.hInstance=GetCommonResourceHandle();
	}
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
		return (BOOL)::SendMessage(m_hWnd,PSM_SETCURSEL,nPage,NULL);
	m_psh.nStartPage=nPage;
	return FALSE;
}

BOOL CPropertySheet::SetActivePage(CPropertyPage* pPage)
{
	if (m_hWnd!=NULL)
		return (BOOL)::SendMessage(m_hWnd,PSM_SETCURSEL,0,(LPARAM)pPage->m_hWnd);
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

void CPropertySheet::SetTitle(LPCSTR lpszText,UINT nStyle)
{
	if (IsUnicodeSystem())
	{
		if (m_pshw.pszCaption!=NULL)
			delete[] m_pshw.pszCaption;
		m_pshw.pszCaption=lpszText!=NULL?alloccopyAtoW(lpszText):NULL;
		m_pshw.dwFlags&=~PSH_PROPTITLE;
		m_pshw.dwFlags|=nStyle;
		
		if (m_hWnd!=NULL)
			::SendMessageW(m_hWnd,PSM_SETTITLEW,nStyle,(LPARAM)(LPCWSTR)A2W(lpszText));
	}
	else
	{
		if (m_psh.pszCaption!=NULL)
			delete[] m_psh.pszCaption;
		m_psh.pszCaption=lpszText!=NULL?alloccopy(lpszText):NULL;
		m_psh.dwFlags&=~PSH_PROPTITLE;
		m_psh.dwFlags|=nStyle;
		
		if (m_hWnd!=NULL)
			::SendMessageA(m_hWnd,PSM_SETTITLEA,nStyle,(LPARAM)(LPCSTR)(lpszText));
	}
}

void CPropertySheet::SetIcon(HICON hIcon)
{
	if (m_psh.dwFlags&PSH_USEICONID && !IS_INTRESOURCE(m_psh.pszIcon))
	{
		if (IsUnicodeSystem())
			delete[] m_pshw.pszIcon;
		else
			delete[] m_psh.pszIcon;
		
	}

	m_psh.dwFlags&=~PSH_USEICONID;
	m_psh.dwFlags|=PSH_USEHICON;
	m_psh.hIcon=hIcon;
}

void CPropertySheet::SetIcon(LPCSTR szIcon)
{
	if (m_psh.dwFlags&PSH_USEICONID && !IS_INTRESOURCE(m_psh.pszIcon))
	{
		if (IsUnicodeSystem())
			delete[] m_pshw.pszIcon;
		else
			delete[] m_psh.pszIcon;
	}
	else
		m_psh.dwFlags&=~PSH_USEHICON;
	
	m_psh.dwFlags|=PSH_USEICONID;
	if (IsUnicodeSystem())
		m_pshw.pszIcon=alloccopyAtoW(szIcon);
	else
		m_psh.pszIcon=alloccopy(szIcon);
}

void CPropertySheet::SetIcon(LPCWSTR szIcon)
{
	if (m_psh.dwFlags&PSH_USEICONID && !IS_INTRESOURCE(m_psh.pszIcon))
	{
		if (IsUnicodeSystem())
			delete[] m_pshw.pszIcon;
		else
			delete[] m_psh.pszIcon;
	}
	else
		m_psh.dwFlags&=~PSH_USEHICON;
	
	m_psh.dwFlags|=PSH_USEICONID;
	if (IsUnicodeSystem())
		m_pshw.pszIcon=alloccopy(szIcon);
	else
		m_psh.pszIcon=alloccopyWtoA(szIcon);
}

void CPropertySheet::SetIcon(int nIcon)
{
	if (m_psh.dwFlags&PSH_USEICONID && !IS_INTRESOURCE(m_psh.pszIcon))
	{
		if (IsUnicodeSystem())
			delete[] m_pshw.pszIcon;
		else
			delete[] m_psh.pszIcon;
	}
	else
		m_psh.dwFlags&=~PSH_USEHICON;
	
	m_psh.dwFlags|=PSH_USEICONID;
	m_psh.pszIcon=MAKEINTRESOURCE(nIcon);
}

void CPropertySheet::SetTitle(LPCWSTR lpszText,UINT nStyle)
{
	if (IsUnicodeSystem())
	{
		if (m_pshw.pszCaption!=NULL)
			delete[] m_pshw.pszCaption;
		m_pshw.pszCaption=lpszText!=NULL?alloccopy(lpszText):NULL;
		m_pshw.dwFlags&=~PSH_PROPTITLE;
		m_pshw.dwFlags|=nStyle;
		
		if (m_hWnd!=NULL)
			::SendMessageW(m_hWnd,PSM_SETTITLEW,nStyle,(LPARAM)(LPCWSTR)(lpszText));
	}
	else
	{
		if (m_psh.pszCaption!=NULL)
			delete[] m_psh.pszCaption;
		m_psh.pszCaption=lpszText!=NULL?alloccopyWtoA(lpszText):NULL;
		m_psh.dwFlags&=~PSH_PROPTITLE;
		m_psh.dwFlags|=nStyle;
		
		if (m_hWnd!=NULL)
			::SendMessageA(m_hWnd,PSM_SETTITLEA,nStyle,(LPARAM)(LPCSTR)W2A(lpszText));
	}
}

INT_PTR CPropertySheet::DoModal()
{
	BuildPropPageArray();
	m_psh.hwndParent=m_hParentWnd;
	if (IsUnicodeSystem())
	{
		if (m_pshw.dwFlags&PSH_MODELESS)
		{
			m_hWnd=(HWND)::PropertySheetW(&m_pshw);
			return m_hWnd!=NULL;
		}
		return ::PropertySheetW(&m_pshw);
	}
	else
	{
		if (m_psh.dwFlags&PSH_MODELESS)
		{
			m_hWnd=(HWND)::PropertySheet(&m_psh);
			return m_hWnd!=NULL;
		}
		return ::PropertySheet(&m_psh);
	}

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
	
	if (IsUnicodeSystem())
	{
		m_psh.phpage=new HPROPSHEETPAGE[max(2,m_pages.GetSize())];
		for (int i=0;i<m_pages.GetSize();i++)
		{
			CPropertyPage* pPage=(CPropertyPage*)m_pages.GetAt(i);
			m_pshw.phpage[i]=CreatePropertySheetPageW(&pPage->m_pspw);
		}
		m_pshw.nPages=m_pages.GetSize();
	}
	else
	{
		m_psh.phpage=new HPROPSHEETPAGE[max(2,m_pages.GetSize())];
		for (int i=0;i<m_pages.GetSize();i++)
		{
			m_psh.phpage[i]=CreatePropertySheetPageA(
				&((CPropertyPage*)m_pages.GetAt(i))->m_psp);
		}
		m_psh.nPages=m_pages.GetSize();
	}
}



///////////////////////////
// Class CInputDialog
///////////////////////////

INT_PTR CInputDialog::DoModal(HWND hWndParent)
{
	INT_PTR ret=CDialog::DoModal(hWndParent);
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
	MemCopy(szText,m_Input,i);
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

#ifdef DEF_WCHAR
void CInputDialog::SetTitle(LPCWSTR szTitle)
{
	m_Title=szTitle;
	if (m_hWnd!=NULL)
		SetWindowText(m_Title);
}

BOOL CInputDialog::SetText(LPCWSTR szText)
{
	m_Text=szText;
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_TEXT,m_Text);
	return TRUE;
}

void CInputDialog::SetInputText(LPCWSTR szText)
{
	m_Input=szText;
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_EDIT,m_Input);
}

int CInputDialog::GetInputText(LPWSTR szText,int nTextLen) const
{
	int i=min(nTextLen-1,(int)m_Input.GetLength());
	MemCopy(szText,m_Input,i);
	szText[i]='\0';
	return i;
}

void CInputDialog::SetOKButtonText(LPCWSTR szText)
{
	m_OKButton=szText;
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_OK,m_OKButton);
}

void CInputDialog::SetCancelButtonText(LPCWSTR szText)
{
	if (m_bFlags&ID_NOCANCELBUTTON)
		return;
	m_CancelButton=szText;
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_CANCEL,m_CancelButton);
}

#endif

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


LRESULT CALLBACK CAppData::CommonDialogProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	CCommonDialog* Wnd;
	if (uMsg==WM_INITDIALOG && lParam!=NULL)
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
		case OPENFILENAME_SIZE_VERSION_400:
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
		default: 
			ASSERT(0);
			return FALSE;
		}
		
		if (Wnd==NULL)
			return FALSE;
		
		Wnd->SetHandle(hWnd);
		SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)Wnd);
		return Wnd->OnInitDialog((HWND)wParam);
	}
	
	Wnd=(CCommonDialog*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
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


void CFileDialog::Init(LPCSTR lpszDefExt,LPCSTR lpszFileName,DWORD dwFlags,LPCSTR lpszFilter)
{
	DebugMessage("CFileDialog::Init BEGIN");

	if (IsUnicodeSystem())
	{

#if (_WIN32_WINNT < 0x0500)
		m_pwofn=(OPENFILENAMEW*)new char[sizeof(OPENFILENAMEW)+16];
		if (m_pwofn==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			return;
		}		
		iMemSet(m_pwofn,0,sizeof(OPENFILENAMEW)+16);
		m_pwofn->lStructSize=sizeof(OPENFILENAME);
#else
		m_pwofn=(OPENFILENAMEW*)new char[sizeof(OPENFILENAMEW)];
		if (m_pwofn==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			return;
		}		
		ZeroMemory(m_pwofn,sizeof(OPENFILENAMEW));
		if (GetSystemFeaturesFlag()&(efWin2000|efWinXP))
		{
			DebugMessage("CFileDialog::CFileDialog: m_pwofn->lStructSize=sizeof(OPENFILENAMEW)");
			m_pwofn->lStructSize=sizeof(OPENFILENAMEW);
		}
		else
		{
			DebugMessage("CFileDialog::CFileDialog: m_pwofn->lStructSize=sizeof(OPENFILENAME_SIZE_VERSION_400W)");
			m_pwofn->lStructSize=OPENFILENAME_SIZE_VERSION_400W;
		}
#endif


		m_pwofn->Flags=dwFlags|OFN_ENABLEHOOK|OFN_EXPLORER;
		m_pwofn->lpfnHook=(LPOFNHOOKPROC)CAppData::CommonDialogProc;
		m_pwofn->lCustData=(LPARAM)this;
		m_pwofn->lpstrFileTitle=new WCHAR[65];
		m_pwofn->lpstrFileTitle[0]='\0';
		m_pwofn->nMaxFileTitle=64;
		m_pwofn->hInstance=GetLanguageSpecificResourceHandle();
		m_pwofn->lpstrCustomFilter=NULL;
		m_pwofn->nMaxCustFilter=0;
		
		m_pwFileName=new WCHAR[MAX_PATH];
		if (m_pwFileName==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			DebugMessage("CFileDialog::CFileDialog END ERR1");
			return;
		}
		m_pwofn->nMaxFile=MAX_PATH;
		m_pwofn->lpstrFile=m_pwFileName;
		if (lpszFileName!=NULL)
		{
			int nRet=(int)istrlen(lpszFileName);
			if (nRet>MAX_PATH-2)
				nRet=MAX_PATH-2;
            
			MultiByteToWideChar(CP_ACP,0,lpszFileName,nRet,m_pwFileName,nRet);
			m_pwFileName[nRet]='\0';
			m_pwFileName[nRet+1]='\0';

		}
		else
		{
			m_pwFileName[0]='\0';
			m_pwFileName[1]='\0';
		}

		if (lpszFilter!=NULL)
			m_pwFilter=alloccopyAtoW(lpszFilter);
		else
			m_pwFilter=NULL;
		
		m_pwofn->nFilterIndex=0;
		m_pwofn->lpstrInitialDir=NULL;
		m_pwofn->lpstrTitle=NULL;
		m_pwofn->lpTemplateName=NULL;
		if (lpszDefExt!=NULL)
			m_pwofn->lpstrDefExt=alloccopyAtoW(lpszDefExt);
		else
			m_pwofn->lpstrDefExt=NULL;
	}
	else
	{
#if (_WIN32_WINNT < 0x0500)
		m_pofn=(OPENFILENAME*)new char[sizeof(OPENFILENAME)+16];
		if (m_pofn==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			return;
		}		
		iMemSet(m_pofn,0,sizeof(OPENFILENAME)+16);
		m_pofn->lStructSize=sizeof(OPENFILENAME);
#else
		m_pofn=(OPENFILENAME*)new char[sizeof(OPENFILENAME)];
		if (m_pofn==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			return;
		}		
		ZeroMemory(m_pofn,sizeof(OPENFILENAME));
		if (GetSystemFeaturesFlag()&(efWin2000|efWinXP))
		{
			DebugMessage("CFileDialog::CFileDialog: m_pofn->lStructSize=sizeof(OPENFILENAME)");
			m_pofn->lStructSize=sizeof(OPENFILENAME);
		}
		else
		{
			DebugMessage("CFileDialog::CFileDialog: m_pofn->lStructSize=sizeof(OPENFILENAME_SIZE_VERSION_400)");
			m_pofn->lStructSize=OPENFILENAME_SIZE_VERSION_400;
		}
#endif


		m_pofn->Flags=dwFlags|OFN_ENABLEHOOK|OFN_EXPLORER;
		m_pofn->lpfnHook=(LPOFNHOOKPROC)CAppData::CommonDialogProc;
		m_pofn->lCustData=(LPARAM)this;
		m_pofn->lpstrFileTitle=new char[65];
		m_pofn->lpstrFileTitle[0]='\0';
		m_pofn->nMaxFileTitle=64;
		m_pofn->hInstance=GetLanguageSpecificResourceHandle();
		m_pofn->lpstrCustomFilter=NULL;
		m_pofn->nMaxCustFilter=0;
		
		m_pFileName=new CHAR[MAX_PATH];
		if (m_pFileName==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			DebugMessage("CFileDialog::CFileDialog END ERR1");
			return;
		}
		m_pofn->nMaxFile=MAX_PATH;
		if (lpszFileName!=NULL)
		{
			int nLen=(int)istrlen(lpszFileName);
			if (nLen>=MAX_PATH-1)
				nLen=MAX_PATH-2;
			MemCopy(m_pFileName,lpszFileName,nLen+1);
			m_pFileName[nLen+1]='\0';
		}
		else
		{
			m_pFileName[0]='\0';
			m_pFileName[1]='\0';
		}
		m_pofn->lpstrFile=m_pFileName;
		

		if (lpszFilter!=NULL)
			m_pFilter=alloccopy(lpszFilter);
		else
			m_pFilter=NULL;
		
		m_pofn->nFilterIndex=0;
		m_pofn->lpstrInitialDir=NULL;
		m_pofn->lpstrTitle=NULL;
		m_pofn->lpTemplateName=NULL;
		
		if (lpszDefExt!=NULL)
			m_pofn->lpstrDefExt=alloccopy(lpszDefExt);
		else
			m_pofn->lpstrDefExt=NULL;
	}
	DebugMessage("CFileDialog::CFileDialog END");
}


void CFileDialog::Init(LPCWSTR lpszDefExt,LPCWSTR lpszFileName,DWORD dwFlags,LPCWSTR lpszFilter)
{
	DebugMessage("CFileDialog::Init BEGIN");
	
	if (IsUnicodeSystem())
	{

#if (_WIN32_WINNT < 0x0500)
		m_pwofn=(OPENFILENAMEW*)new char[sizeof(OPENFILENAMEW)+16];
		if (m_pwofn==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			return;
		}		
		iMemSet(m_pwofn,0,sizeof(OPENFILENAMEW)+16);
		m_pwofn->lStructSize=sizeof(OPENFILENAME);
#else
		m_pwofn=(OPENFILENAMEW*)new char[sizeof(OPENFILENAMEW)];
		if (m_pwofn==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			return;
		}		
		ZeroMemory(m_pwofn,sizeof(OPENFILENAMEW));
		if (GetSystemFeaturesFlag()&(efWin2000|efWinXP))
		{
			DebugMessage("CFileDialog::CFileDialog: m_pwofn->lStructSize=sizeof(OPENFILENAMEW)");
			m_pwofn->lStructSize=sizeof(OPENFILENAMEW);
		}
		else
		{
			DebugMessage("CFileDialog::CFileDialog: m_pwofn->lStructSize=sizeof(OPENFILENAME_SIZE_VERSION_400W)");
			m_pwofn->lStructSize=OPENFILENAME_SIZE_VERSION_400W;
		}
#endif


		m_pwofn->Flags=dwFlags|OFN_ENABLEHOOK|OFN_EXPLORER;
		m_pwofn->lpfnHook=(LPOFNHOOKPROC)CAppData::CommonDialogProc;
		m_pwofn->lCustData=(LPARAM)this;
		m_pwofn->lpstrFileTitle=new WCHAR[65];
		m_pwofn->lpstrFileTitle[0]='\0';
		m_pwofn->nMaxFileTitle=64;
		m_pwofn->hInstance=GetLanguageSpecificResourceHandle();
		m_pwofn->lpstrCustomFilter=NULL;
		m_pwofn->nMaxCustFilter=0;
		
		m_pwFileName=new WCHAR[MAX_PATH];
		if (m_pwFileName==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			DebugMessage("CFileDialog::CFileDialog END ERR1");
			return;
		}
		m_pwofn->nMaxFile=MAX_PATH;
		if (lpszFileName!=NULL)
		{
			int nRet=(int)istrlenw(lpszFileName);
			if (nRet>MAX_PATH-2)
				nRet=MAX_PATH-2;
				
			MemCopyW(m_pwFileName,lpszFileName,nRet);
			m_pwFileName[nRet]='\0';
			m_pwFileName[nRet+1]='\0';
		}
		else
		{
			m_pwFileName[0]='\0';
			m_pwFileName[1]='\0';
		}
		m_pwofn->lpstrFile=m_pwFileName;
		

		if (lpszFilter!=NULL)
			m_pwFilter=alloccopy(lpszFilter);
		else
			m_pwFilter=NULL;
		
		m_pwofn->nFilterIndex=0;
		m_pwofn->lpstrInitialDir=NULL;
		m_pwofn->lpstrTitle=NULL;
		m_pwofn->lpTemplateName=NULL;
		if (lpszDefExt!=NULL)
			m_pwofn->lpstrDefExt=alloccopy(lpszDefExt);
		else
			m_pwofn->lpstrDefExt=NULL;
	}
	else
	{
#if (_WIN32_WINNT < 0x0500)
		m_pofn=(OPENFILENAME*)new char[sizeof(OPENFILENAME)+16];
		if (m_pofn==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			return;
		}		
		iMemSet(m_pofn,0,sizeof(OPENFILENAME)+16);
		m_pofn->lStructSize=sizeof(OPENFILENAME);
#else
		m_pofn=(OPENFILENAME*)new char[sizeof(OPENFILENAME)];
		if (m_pofn==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			return;
		}		
		ZeroMemory(m_pofn,sizeof(OPENFILENAME));
		if (GetSystemFeaturesFlag()&(efWin2000|efWinXP))
		{
			DebugMessage("CFileDialog::CFileDialog: m_pofn->lStructSize=sizeof(OPENFILENAME)");
			m_pofn->lStructSize=sizeof(OPENFILENAME);
		}
		else
		{
			DebugMessage("CFileDialog::CFileDialog: m_pofn->lStructSize=sizeof(OPENFILENAME_SIZE_VERSION_400)");
			m_pofn->lStructSize=OPENFILENAME_SIZE_VERSION_400;
		}
#endif


		m_pofn->Flags=dwFlags|OFN_ENABLEHOOK|OFN_EXPLORER;
		m_pofn->lpfnHook=(LPOFNHOOKPROC)CAppData::CommonDialogProc;
		m_pofn->lCustData=(LPARAM)this;
		m_pofn->lpstrFileTitle=new char[65];
		m_pofn->lpstrFileTitle[0]='\0';
		m_pofn->nMaxFileTitle=64;
		m_pofn->hInstance=GetLanguageSpecificResourceHandle();
		m_pofn->lpstrCustomFilter=NULL;
		m_pofn->nMaxCustFilter=0;
		
		m_pFileName=new CHAR[MAX_PATH];
		if (m_pFileName==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			DebugMessage("CFileDialog::CFileDialog END ERR1");
			return;
		}
		m_pofn->nMaxFile=MAX_PATH;
		m_pofn->lpstrFile=m_pFileName;
		if (lpszFileName!=NULL)
		{
			int nLen=(int)istrlenw(lpszFileName);
			if (nLen>=MAX_PATH-1)
				nLen=MAX_PATH-2;
			WideCharToMultiByte(CP_ACP,0,lpszFileName,nLen+1,m_pFileName,nLen+1,NULL,NULL);
			m_pFileName[nLen+1]='\0';
		}
		else
		{
			m_pFileName[0]='\0';
			m_pFileName[1]='\0';
		}

		if (lpszFilter!=NULL)
			m_pFilter=alloccopyWtoA(lpszFilter);
		else
			m_pFilter=NULL;
		
		m_pofn->nFilterIndex=0;
		m_pofn->lpstrInitialDir=NULL;
		m_pofn->lpstrTitle=NULL;
		m_pofn->lpTemplateName=NULL;
		
		if (lpszDefExt!=NULL)
			m_pofn->lpstrDefExt=alloccopyWtoA(lpszDefExt);
		else
			m_pofn->lpstrDefExt=NULL;
	}
	DebugMessage("CFileDialog::CFileDialog END");
}

CFileDialog::~CFileDialog()
{
	if (IsUnicodeSystem())
	{
		if (m_pwofn!=NULL)
		{
			if (m_pwofn->lpstrTitle!=NULL)
				delete[] (LPWSTR)m_pwofn->lpstrTitle;
			if (m_pwofn->lpstrDefExt!=NULL)
				delete[] (LPWSTR)m_pwofn->lpstrDefExt;
			if (m_pwofn->lpstrFileTitle!=NULL)
				delete[] m_pwofn->lpstrFileTitle;
			delete[] (CHAR*)m_pwofn;
			m_pwofn=NULL;
		}
		if (m_pwFilter!=NULL)
			delete[] m_pwFilter;
		if (m_pwFileName!=NULL)
			delete[] m_pwFileName;
		
	}
	else
	{
		if (m_pofn!=NULL)
		{
			if (m_pofn->lpstrTitle!=NULL)
				delete[] (LPSTR)m_pofn->lpstrTitle;
			if (m_pofn->lpstrDefExt!=NULL)
				delete[] (LPSTR)m_pofn->lpstrDefExt;
			if (m_pofn->lpstrFileTitle!=NULL)
				delete[] m_pofn->lpstrFileTitle;
			delete[] (char*)m_pofn;
			m_pofn=NULL;
		}
		if (m_pFilter!=NULL)
			delete[] m_pFilter;
		if (m_pFileName!=NULL)
			delete[] m_pFileName;
	}
	
}

BOOL CFileDialog::EnableFeatures(DWORD nFlags)
{
	if (nFlags==efCheck)
		nFlags=GetSystemFeaturesFlag();

	if (IsUnicodeSystem())
	{
#if (_WIN32_WINNT >= 0x0500)
		if (nFlags&(efWin2000|efWinME) && m_pofn!=NULL)
		{
			m_pwofn->lStructSize=sizeof(OPENFILENAMEW);
		}
#else
		if (nFlags&(efWin2000|efWinME) && m_pofn!=NULL)
		{
			m_pwofn->lStructSize=sizeof(OPENFILENAMEW)+2*sizeof(DWORD)+sizeof(void*);
		}
#endif
	}
	else
	{
#if (_WIN32_WINNT >= 0x0500)
		if (nFlags&(efWin2000|efWinME) && m_pofn!=NULL)
		{
			m_pofn->lStructSize=sizeof(OPENFILENAME);
		}
#else
		if (nFlags&(efWin2000|efWinME) && m_pofn!=NULL)
		{
			m_pofn->lStructSize=sizeof(OPENFILENAME)+2*sizeof(DWORD)+sizeof(void*);
		}
#endif
	}
	return TRUE;
}

BOOL CFileDialog::DoModal(HWND hParentWnd)
{
	BOOL bError;
	int i;
		
	if (hParentWnd!=NULL)
		::EnableWindow(hParentWnd,FALSE);
	
	if (IsUnicodeSystem())
	{
		m_pwofn->hwndOwner=hParentWnd;
		m_pwofn->lpstrFilter=m_pwFilter;
		if (m_pwFilter!=NULL)
		{
			for (i=0;m_pwFilter[i]!='\0';i++)
			{
				if (m_pwFilter[i]=='|')
					m_pwFilter[i]='\0';
			}
		}
		
		
		if (m_bOpenFileDialog)
			bError=::GetOpenFileNameW(m_pwofn);
		else
			bError=::GetSaveFileNameW(m_pwofn);
	}
	else
	{
		m_pofn->hwndOwner=hParentWnd;
		m_pofn->lpstrFilter=m_pFilter;
		if (m_pFilter!=NULL)
		{
			for (i=0;m_pFilter[i]!='\0';i++)
			{
				if (m_pFilter[i]=='|')
					m_pFilter[i]='\0';
			}
		}
		
		
		if (m_bOpenFileDialog)
			bError=::GetOpenFileName(m_pofn);
		else
			bError=::GetSaveFileName(m_pofn);
	}


	m_hWnd=NULL;
	if (hParentWnd!=NULL)
	{
		::EnableWindow(hParentWnd,TRUE);
		::SetFocus(hParentWnd);
	}
	return bError;	
}


BOOL CFileDialog::GetFilePath(CString& name) const
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			WCHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFILEPATH,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name=m_pwFileName;
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			CHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFILEPATH,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name=m_pFileName;
	}
	return TRUE;
}

BOOL CFileDialog::GetFilePath(LPSTR pFilePath,DWORD nMaxLen) const
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			WCHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFILEPATH,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				WideCharToMultiByte(CP_ACP,0,path,nRet,pFilePath,nMaxLen,NULL,NULL);
		}
		else
			WideCharToMultiByte(CP_ACP,0,m_pwFileName,-1,pFilePath,nMaxLen,NULL,NULL);
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFILEPATH,nMaxLen,(LPARAM)pFilePath);
			if (nRet<0)
				return FALSE;
		}
		else
			strcpy_s(pFilePath,nMaxLen,m_pFileName);
	}
	return TRUE;
}


BOOL CFileDialog::GetFileName(CString& name) const
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			WCHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name=m_pwFileName+m_pwofn->nFileOffset;
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			CHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name=m_pwFileName+m_pofn->nFileOffset;
	}
	return TRUE;
}

BOOL CFileDialog::GetFileName(LPSTR pFileName,DWORD nMaxLen) const
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			WCHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				WideCharToMultiByte(CP_ACP,0,path,nRet,pFileName,nMaxLen,NULL,NULL);
		}
		else
			WideCharToMultiByte(CP_ACP,0,m_pwFileName+m_pwofn->nFileOffset,-1,pFileName,nMaxLen,NULL,NULL);
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,nMaxLen,(LPARAM)pFileName);
			if (nRet<0)
				return FALSE;
		}
		else
			strcpy_s(pFileName+m_pofn->nFileOffset,nMaxLen,m_pFileName);
	}
	return TRUE;
}



#ifdef DEF_WCHAR
BOOL CFileDialog::GetFilePath(CStringW& name) const
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			WCHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFILEPATH,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name=m_pwFileName;
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			CHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFILEPATH,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name=m_pFileName;
	}
	return TRUE;
}

BOOL CFileDialog::GetFilePath(LPWSTR pFilePath,DWORD nMaxLen) const
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFILEPATH,nMaxLen,(LPARAM)pFilePath);
			if (nRet<0)
				return FALSE;
		}
		else
			wcscpy_s(pFilePath,nMaxLen,m_pwFileName);
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			CHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFILEPATH,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				MultiByteToWideChar(CP_ACP,0,path,nRet,pFilePath,nMaxLen);
		}
		else
			MultiByteToWideChar(CP_ACP,0,m_pFileName,-1,pFilePath,nMaxLen);
	}
	return TRUE;
}

BOOL CFileDialog::GetFileName(CStringW& name) const
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			WCHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name=m_pwFileName+m_pofn->nFileOffset;
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			CHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name=m_pFileName+m_pofn->nFileOffset;
	}
	return TRUE;
}

BOOL CFileDialog::GetFileName(LPWSTR pFileName,DWORD nMaxLen) const
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,nMaxLen,(LPARAM)pFileName);
			if (nRet<0)
				return FALSE;
		}
		else
			wcscpy_s(pFileName+m_pwofn->nFileOffset,nMaxLen,m_pwFileName);
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			CHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETSPEC,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			MultiByteToWideChar(CP_ACP,0,path,nRet,pFileName,nMaxLen);
		}
		else
			MultiByteToWideChar(CP_ACP,0,m_pFileName+m_pofn->nFileOffset,-1,pFileName,nMaxLen);
	}
	return TRUE;
}



#endif

BOOL CFileDialog::GetReadOnlyPref() const
{
	if (m_pofn->Flags&OFN_READONLY)
		return TRUE;
	return FALSE;
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

BOOL CFileDialog::GetFolderPath(CString& name) const
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			WCHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFOLDERPATH,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name.Copy(m_pwFileName,m_pofn->nFileOffset);
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			CHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFOLDERPATH,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name.Copy(m_pFileName,m_pofn->nFileOffset);
	}
	return TRUE;
}

#ifdef DEF_WCHAR
BOOL CFileDialog::GetFolderPath(CStringW& name) const
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			WCHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFOLDERPATH,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name.Copy(m_pwFileName,m_pofn->nFileOffset);
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			CHAR path[MAX_PATH];
			int nRet=(int)::SendMessage(::GetParent(m_hWnd),CDM_GETFOLDERPATH,MAX_PATH,(LPARAM)path);
			if (nRet<0)
				return FALSE;
			else
				name.Copy(path,nRet-1);
		}
		else
			name.Copy(m_pFileName,m_pofn->nFileOffset);
	}
	return TRUE;
}
#endif

void CFileDialog::SetDefExt(LPCSTR lpsz)
{
	if (IsUnicodeSystem())
	{
		if (m_hWnd!=NULL)
		{
			::SendMessage(::GetParent(m_hWnd),CDM_SETDEFEXT,0,(LPARAM)(LPCWSTR)A2W(lpsz));
			return;
		}

		if (m_pwofn->lpstrDefExt!=NULL)
			delete[] (LPWSTR)m_pwofn->lpstrDefExt;
		
		if (lpsz!=NULL)
		{
			int nLen=(int)istrlen(lpsz)+1;
			m_pwofn->lpstrDefExt=new WCHAR[nLen];
			if (m_pwofn->lpstrDefExt==NULL)
			{
				SetHFCError(HFC_CANNOTALLOC);
				return;
			}
			MultiByteToWideChar(CP_ACP,0,lpsz,(int)nLen,(LPWSTR)m_pwofn->lpstrDefExt,(int)nLen);
		}
		else
			m_pwofn->lpstrDefExt=NULL;
	}
	else
	{
		if (m_hWnd!=NULL)
		{
			::SendMessage(::GetParent(m_hWnd),CDM_SETDEFEXT,0,(LPARAM)lpsz);
			return;
		}

		if (m_pofn->lpstrDefExt!=NULL)
			delete[] (LPSTR)m_pofn->lpstrDefExt;
		
		if (lpsz!=NULL)
		{
			size_t nLen=istrlen(lpsz)+1;
			m_pofn->lpstrDefExt=new char[nLen];
			if (m_pwofn->lpstrDefExt==NULL)
			{
				SetHFCError(HFC_CANNOTALLOC);
				return;
			}
			CopyMemory((LPSTR)m_pofn->lpstrDefExt,lpsz,nLen);
		}
		else
			m_pwofn->lpstrDefExt=NULL;
	}

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
	m_cf.lCustData=(LPARAM)this;
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
	m_cf.lCustData=(LPARAM)this;
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
	StringCbCopy(cf.szFaceName,LF_FACESIZE,m_lf.lfFaceName);
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
		StringCbCopy(m_lf.lfFaceName,LF_FACESIZE,cf.szFaceName);
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
	ZeroMemory(&m_cc,sizeof(CHOOSECOLOR));
	m_cc.lStructSize=sizeof(CHOOSECOLOR);
	m_cc.Flags=dwFlags|CC_ENABLEHOOK;
	m_cc.hInstance=(HWND)GetLanguageSpecificResourceHandle();
	if (m_cc.rgbResult=clrInit)
		m_cc.Flags|=CC_RGBINIT;
	
	m_cc.lCustData=(LPARAM)this;
	m_cc.lpfnHook=(LPOFNHOOKPROC)CAppData::CommonDialogProc;
	//m_cc.lpTemplateName=NULL;

	
}



BOOL CColorDialog::DoModal(HWND hParentWnd)
{
	BOOL ret;
	m_cc.hwndOwner=hParentWnd;
	COLORREF aCustomColors[16];
	FillMemory(aCustomColors,sizeof(COLORREF)*16,255);

	if (m_cc.lpCustColors==NULL)
		m_cc.lpCustColors=aCustomColors;

	ret=ChooseColor(&m_cc);
	
	if (m_cc.lpCustColors==aCustomColors)
		m_cc.lpCustColors=NULL;

	m_hWnd=NULL;
	return ret;
}

///////////////////////////
// Class CPageSetupDialog
///////////////////////////

LRESULT CALLBACK CAppData::PagePaintProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
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
	m_psd.lCustData=(LPARAM)this;
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
	m_pd.lCustData=(LPARAM)this;
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
	MemCopy(lpdm,lpmdm,sizeof(DEVMODE));
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
	m_fr.lCustData=(LPARAM)this;
	m_fr.lpfnHook=(LPFRHOOKPROC)CAppData::CommonDialogProc;
	m_fr.lpTemplateName=NULL;
}

BOOL CFindReplaceDialog::Create(BOOL bFindDialogOnly,LPCTSTR lpszFindWhat,LPCTSTR lpszReplaceWith,DWORD dwFlags,HWND hParentWnd)
{
	m_fr.hwndOwner=hParentWnd;
	m_fr.Flags=dwFlags|FR_ENABLEHOOK;
	StringCbCopy(m_szFindWhat,128,lpszFindWhat);
	StringCbCopy(m_szReplaceWith,128,lpszReplaceWith);
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

LRESULT CALLBACK CAppData::FolderDialogProc(HWND hWnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
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
		CoTaskMemFree(m_lpil);
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
	if ((int)nSize<=m_strDisplayName.GetLength())
	{
		MemCopy(szDisplayName,(LPCSTR)m_strDisplayName,nSize-1);
		szDisplayName[nSize-1]='\0';
	}
	else
		MemCopy(szDisplayName,(LPCSTR)m_strDisplayName,m_strDisplayName.GetLength()+1);
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


BOOL CFolderDialog::GetFolder(CStringW& Folder) const
{
	if (IsUnicodeSystem())
	{
		if (!SHGetPathFromIDListW(m_lpil,Folder.GetBuffer(_MAX_PATH)))
			return FALSE;
		Folder.FreeExtra();
		return TRUE;
	}
	else
	{
		char szFolderA[MAX_PATH];
		if (!SHGetPathFromIDListA(m_lpil,szFolderA))
			return FALSE;
		Folder=szFolderA;
		return TRUE;
	}
	
}

BOOL CFolderDialog::GetFolder(LPWSTR szFolder) const
{
	if (IsUnicodeSystem())
		return SHGetPathFromIDListW(m_lpil,szFolder);
	else
	{
		char szFolderA[MAX_PATH];
		if (!SHGetPathFromIDListA(m_lpil,szFolderA))
			return FALSE;
		MultiByteToWideChar(CP_ACP,0,szFolderA,-1,szFolder,MAX_PATH);
		return TRUE;
	}
}

///////////////////////////
// Class COptionsPropertyPage
///////////////////////////






#endif