////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2004 Janne Huttunen
////////////////////////////////////////////////////////////////////


#ifndef HFCDIALOGS_H
#define HFCDIALOGS_H

#if defined(DEF_RESOURCES) && defined(DEF_WINDOWS)

class CDialog : public CWnd
{
protected:
	LPCTSTR m_lpszTemplateName;
public:
	CDialog(LPCTSTR lpTemplate);
	CDialog(int iTemplate);
	virtual ~CDialog();
	
	BOOL Create(HWND hWndParent=NULL,TypeOfResourceHandle bType=LanguageSpecificResource);

	int DoModal(HWND hWndParent=NULL,TypeOfResourceHandle bType=LanguageSpecificResource);

public:
	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);

public:
	BOOL EndDialog(int nResult) const;
};

class CPropertyPage : public CDialog
{
public:
	CPropertyPage();
	CPropertyPage(HWND hWnd);
	CPropertyPage(UINT nIDTemplate,UINT nIDCaption=0);
	CPropertyPage(LPCTSTR lpszTemplateName,UINT nIDCaption=0);
	
	void Construct(UINT nIDTemplate,UINT nIDCaption=0,
		TypeOfResourceHandle bType=LanguageSpecificResource);
	void Construct(LPCTSTR lpszTemplateName,UINT nIDCaption=0,
		TypeOfResourceHandle bType=LanguageSpecificResource);
	
	PROPSHEETPAGE m_psp;
	operator HPROPSHEETPAGE() const;

	void CancelToClose();
	void SetModified(BOOL bChanged=TRUE);
	LRESULT QuerySiblings(WPARAM wParam,LPARAM lParam);
	
	void EndDialog(int nID);

public:
	virtual BOOL OnApply();
	virtual void OnReset();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual BOOL OnQueryCancel();

	virtual BOOL OnWizardBack();
	virtual BOOL OnWizardNext();
	virtual BOOL OnWizardFinish();

protected:
	CString m_strCaption;
	BOOL m_bFirstSetActive;

	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
		
	friend class CPropertySheet;
};

class CPropertySheet : public CWnd
{
public:
	CPropertySheet();
	CPropertySheet(HWND hWnd);
	CPropertySheet(UINT nIDCaption,HWND hParentWnd=NULL,UINT iSelectPage=0);
	CPropertySheet(LPCTSTR pszCaption,HWND hParentWnd=NULL,UINT iSelectPage=0);
	virtual ~CPropertySheet();

	void Construct(UINT nIDCaption,HWND hParentWnd=NULL,UINT iSelectPage=0);
	void Construct(LPCTSTR pszCaption,HWND hParentWnd=NULL,UINT iSelectPage=0);

	BOOL Create(HWND hParentWnd=NULL,DWORD dwStyle=(DWORD)-1,DWORD dwExStyle=0);
	void EnableStackedTabs(BOOL bStacked);

public:
	PROPSHEETHEADER m_psh;

	int GetPageCount() const;
	HWND GetActivePage() const;
	int GetActiveIndex() const;
	CPropertyPage* GetPage(int nPage) const;
	int GetPageIndex(CPropertyPage* pPage);

	BOOL SetActivePage(int nPage);
	BOOL SetActivePage(CPropertyPage* pPage);
	void SetTitle(LPCTSTR lpszText,UINT nStyle=0);
	HWND GetTabControl() const;

	void SetWizardMode();
	void SetFinishText(LPCTSTR lpszText);
	void SetWizardButtons(DWORD dwFlags);

public:
	virtual int DoModal();
	void AddPage(CPropertyPage* pPage);
	void RemovePage(CPropertyPage* pPage);
	void RemovePage(int nPage);
	void EndDialog(int nEndID);
	BOOL PressButton(int nButton);

	virtual void BuildPropPageArray();
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnInitDialog(HWND hwndFocus);
	
protected:
	CPtrArray m_pages;
	CString m_strCaption;
	HWND m_hParentWnd;
	BOOL m_bStacked;
	
	friend class CPropertyPage;
};

class CCommonDialog : public CDialog
{
public:
	CCommonDialog();

public:
	virtual void OnOK();
	virtual void OnCancel();
};

class CInputDialog : public CDialog  
{
public:
	CInputDialog(LPCTSTR lpTemplate,BYTE bFlags=0);
	CInputDialog(int iTemplate,BYTE bFlags=0);

	int DoModal(HWND hWndParent=NULL);
	void SetTitle(LPCSTR szTitle);
	void SetTitle(int iTitle);
	BOOL SetText(LPCSTR szText);
	BOOL SetText(int iText);
	void SetInputText(LPCSTR szText);
	void SetInputText(int iText);
	int GetInputText(CString& text) const;
	int GetInputText(LPSTR szText,int nTextLen) const;
	
	void SetOKButtonText(LPCSTR szText);
	void SetOKButtonText(int nText);
	void SetCancelButtonText(LPCSTR szText);
	void SetCancelButtonText(int nText);
	
	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();
protected:
	BYTE m_bFlags;
	CString m_Title;
	CString m_Text;
	CString m_Input;
	CString m_OKButton;
	CString m_CancelButton;
};

class CFileDialog : public CCommonDialog
{
public:
	OPENFILENAME* m_pofn;

	CFileDialog(BOOL bOpenFileDialog,LPCTSTR lpszDefExt=NULL,
		LPCTSTR lpszFileName=NULL,
		DWORD dwFlags=OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter=NULL);
	virtual ~CFileDialog();

	BOOL EnableFeatures(DWORD nFlags=efCheck);
	BOOL DoModal(HWND hParentWnd=NULL);

	CString GetPathName() const;
	CString GetFileName() const;
	CString GetFileExt() const;
	CString GetFileTitle() const;
	BOOL GetReadOnlyPref() const;

	POSITION GetStartPosition() const;
	CString GetNextPathName(POSITION& pos) const;

	void SetTemplate(UINT nID,TypeOfResourceHandle bType=LanguageSpecificResource);
	void SetTemplate(LPCTSTR lpID,TypeOfResourceHandle bType=LanguageSpecificResource);

	CString GetFolderPath() const;
	void SetControlText(int nID,LPCSTR lpsz);
	void HideControl(int nID);
	void SetDefExt(LPCSTR lpsz);

protected:
	virtual UINT OnShareViolation(LPCTSTR lpszPathName);
	virtual BOOL OnFileNameOK();
	virtual void OnInitDone();
	virtual void OnFileNameChange();
	virtual void OnFolderChange();
	virtual void OnTypeChange();

protected:
	BOOL m_bOpenFileDialog;
	LPTSTR m_strFilter;
	TCHAR m_szFileTitle[64];
	LPTSTR m_szFileName;

	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
};

class CFontDialog : public CCommonDialog
{
public:
	CHOOSEFONT m_cf;

	CFontDialog(LPLOGFONT lplfInitial=NULL,DWORD dwFlags=CF_EFFECTS|CF_SCREENFONTS,
		HDC hdcPrinter=NULL);
	CFontDialog(const CHARFORMAT& charformat,
		DWORD dwFlags=CF_SCREENFONTS,
		HDC hdcPrinter=NULL);
	
	BOOL DoModal(HWND hParentWnd=NULL);

	void GetCurrentFont(LPLOGFONT lplf);

	CString GetFaceName() const; 
	CString GetStyleName() const;
	int GetSize() const;         
	COLORREF GetColor() const; 
	int GetWeight() const;
	BOOL IsStrikeOut() const;
	BOOL IsUnderline() const;
	BOOL IsBold() const;    
	BOOL IsItalic() const; 
	void GetCharFormat(CHARFORMAT& cf) const;

	LOGFONT m_lf;
	DWORD FillInLogFont(const CHARFORMAT& cf);
	TCHAR m_szStyleName[64]; 
};

class CColorDialog : public CCommonDialog
{
public:
	CHOOSECOLOR m_cc;

	CColorDialog(COLORREF clrInit=0,DWORD dwFlags=0);
	virtual ~CColorDialog();
	BOOL DoModal(HWND hParentWnd=NULL);

	void SetCurrentColor(COLORREF clr);
	COLORREF GetColor() const;
};

class CPageSetupDialog : public CCommonDialog
{
public:
	PAGESETUPDLG m_psd;

	CPageSetupDialog(DWORD dwFlags=PSD_MARGINS|PSD_INWININIINTLMEASURE);
	~CPageSetupDialog();
	LPDEVMODE GetDevMode() const;
	CString GetDriverName() const;
	CString GetDeviceName() const;
	CString GetPortName() const;
	HDC CreatePrinterDC();
	CSize GetPaperSize() const;
	void GetMargins(LPRECT lpRectMargins,LPRECT lpRectMinMargins) const;

	BOOL DoModal(HWND hParentWnd=NULL);

	virtual UINT OnDrawPage(HDC hDC,UINT nMessage,LPRECT lpRect);
};


class CPrintDialog : public CCommonDialog
{
public:
	PRINTDLG m_pd;

	CPrintDialog(BOOL bPrintSetupOnly,
		DWORD dwFlags=PD_ALLPAGES|PD_USEDEVMODECOPIES|PD_NOPAGENUMS|PD_HIDEPRINTTOFILE|PD_NOSELECTION);
	~CPrintDialog();
	BOOL DoModal(HWND hParentWnd=NULL);

	BOOL GetDefaults();

	int GetCopies() const; 
	BOOL PrintCollate() const;
	BOOL PrintSelection() const;
	BOOL PrintAll() const; 
	BOOL PrintRange() const;
	int GetFromPage() const;
	int GetToPage() const;  
	LPDEVMODE GetDevMode() const;
	CString GetDriverName() const;
	CString GetDeviceName() const;
	CString GetPortName() const; 
	HDC GetPrinterDC() const;
	HDC CreatePrinterDC();
};

class CFindReplaceDialog : public CCommonDialog
{
public:
	FINDREPLACE m_fr;

	CFindReplaceDialog();
	BOOL Create(BOOL bFindDialogOnly,LPCTSTR lpszFindWhat,
			LPCTSTR lpszReplaceWith=NULL,DWORD dwFlags=FR_DOWN,HWND hParentWnd=NULL);

	CString GetReplaceString() const;
	CString GetFindString() const;
	BOOL SearchDown() const;
	BOOL FindNext() const;
	BOOL MatchCase() const;
	BOOL MatchWholeWord() const;
	BOOL ReplaceCurrent() const;
	BOOL ReplaceAll() const;
	
protected:
	TCHAR m_szFindWhat[128];
	TCHAR m_szReplaceWith[128];
};

class CFolderDialog
{
public:
	CFolderDialog(LPCSTR lpszTitle,UINT ulFlags,LPCITEMIDLIST pidlRoot=NULL);
	CFolderDialog(UINT nTitleID,UINT ulFlags,LPCITEMIDLIST pidlRoot=NULL);
	virtual ~CFolderDialog();

	void DontFreeLPIL() { m_lpil=NULL; } // GetFolder() must call before this
	BROWSEINFO m_bi;
	HWND m_hWnd;
	LPITEMIDLIST m_lpil;

public:
	BOOL DoModal(HWND hOwner=NULL);
	LPITEMIDLIST GetFolder() const;
	BOOL GetFolder(CString& Folder) const;
	BOOL GetFolder(LPSTR szFolder) const;
	BOOL GetDisplayName(CString& strDisplayName) const;
	BOOL GetDisplayName(LPSTR szDisplayName,DWORD nSize);
	int GetImage() const { return m_bi.iImage; } 

	BOOL EnableOK(BOOL bEnable=TRUE);
	BOOL SetSelection(LPITEMIDLIST lpil);
	BOOL SetSelection(LPCSTR lpFolder);
	BOOL SetStatusText(LPCSTR lpStatus);

	virtual BOOL OnInitialized();
	virtual BOOL OnSelChanged(LPITEMIDLIST lpil);
	virtual BOOL OnValidateFailed(LPITEMIDLIST lpil);

protected:
	CString m_strTitle;
	CString m_strDisplayName;
	CString m_strDefaultFolder;
	LPITEMIDLIST m_lpDefaultIL;

};

/////////////////////////////////////////////////
// Inline function
/////////////////////////////////////////////////

/////////////////////////////////////////////////
// CDialog

inline CDialog::CDialog(LPCTSTR lpTemplate)
	: CWnd(NULL),m_lpszTemplateName(lpTemplate)
{
}

inline CDialog::CDialog(int iTemplate)
	: CWnd(NULL),m_lpszTemplateName(MAKEINTRESOURCE(iTemplate))
{
}

inline CDialog::~CDialog()
{
}

/////////////////////////////////////////////////
// CPropertyPage

inline CPropertyPage::CPropertyPage()
:	CDialog(0),m_bFirstSetActive(FALSE)
{
}

inline CPropertyPage::CPropertyPage(HWND hWnd)
:	CDialog(0),m_bFirstSetActive(FALSE)
{
}

inline CPropertyPage::CPropertyPage(UINT nIDTemplate,UINT nIDCaption)
:	CDialog(nIDTemplate)
{
	Construct(MAKEINTRESOURCE(nIDTemplate),nIDCaption);
}

inline CPropertyPage::CPropertyPage(LPCTSTR lpszTemplateName,UINT nIDCaption)
:	CDialog(lpszTemplateName)
{
	Construct(lpszTemplateName,nIDCaption);
}

inline CPropertyPage::operator HPROPSHEETPAGE() const
{
	return (HPROPSHEETPAGE) m_hWnd;
}

inline void CPropertyPage::Construct(UINT nIDTemplate,UINT nIDCaption,TypeOfResourceHandle bType)
{
	Construct(MAKEINTRESOURCE(nIDTemplate),nIDCaption,bType);
}

inline void CPropertyPage::CancelToClose()
{
	::SendMessage(GetParent(),PSM_CANCELTOCLOSE,0,0);
}

inline LRESULT CPropertyPage::QuerySiblings(WPARAM wParam,LPARAM lParam)
{
	return ::SendMessage(GetParent(),PSM_QUERYSIBLINGS,wParam,lParam);
}

inline void CPropertyPage::EndDialog(int nID)
{
	::EndDialog(GetParent(),nID);
}

///////////////////////////////////////////
// CPropertySheet

inline CPropertySheet::CPropertySheet()
:	CWnd(NULL)
{
}

inline CPropertySheet::CPropertySheet(HWND hWnd)
:	CWnd(hWnd)
{
}

inline CPropertySheet::CPropertySheet(UINT nIDCaption,HWND hParentWnd,UINT iSelectPage)
{
	if (nIDCaption)
		m_strCaption.LoadString(nIDCaption);
	Construct((LPCTSTR)NULL,hParentWnd,iSelectPage);
}

inline CPropertySheet::CPropertySheet(LPCTSTR pszCaption,HWND hParentWnd,UINT iSelectPage)
{
	Construct(pszCaption,hParentWnd,iSelectPage);
}

inline void CPropertySheet::EnableStackedTabs(BOOL bStacked)
{
	m_bStacked = bStacked;
}

inline int CPropertySheet::GetPageCount() const
{
	return m_pages.GetSize();
}

inline HWND CPropertySheet::GetActivePage() const
{
	return (HWND)::SendMessage(m_hWnd,PSM_GETCURRENTPAGEHWND,0,0);
}

inline CPropertyPage* CPropertySheet::GetPage(int nPage) const
{
	return (CPropertyPage*)m_pages.GetAt(nPage);
}

inline HWND CPropertySheet::GetTabControl() const
{
	return (HWND)::SendMessage(m_hWnd,PSM_GETTABCONTROL,0,0);
}

inline void CPropertySheet::SetWizardMode()
{
	m_psh.dwFlags|=PSH_WIZARD;
}

inline void CPropertySheet::RemovePage(CPropertyPage* pPage)
{
	RemovePage(GetPageIndex(pPage));
}

inline void CPropertySheet::AddPage(CPropertyPage* pPage)
{
	m_pages.Add(pPage);
	if (m_hWnd!=NULL)
		::SendMessage(m_hWnd,PSM_ADDPAGE,0,(LPARAM)pPage->m_hWnd);
}

inline void CPropertySheet::RemovePage(int nPage)
{
	if (m_hWnd!=NULL)
		::SendMessage(m_hWnd,PSM_REMOVEPAGE,nPage,NULL);
	m_pages.RemoveAt(nPage);
}

inline BOOL CPropertySheet::PressButton(int nButton)
{
	return ::PostMessage(m_hWnd,PSM_PRESSBUTTON,nButton,0);
}

inline void CPropertySheet::SetFinishText(LPCTSTR lpszText)
{
	::SendMessage(m_hWnd,PSM_SETFINISHTEXT,0,(LPARAM)lpszText);
}

inline void CPropertySheet::SetWizardButtons(DWORD dwFlags)
{
	::SendMessage(m_hWnd,PSM_SETWIZBUTTONS,0,(LPARAM)dwFlags);
}

///////////////////////////
// Class CFileDialog

inline CString CFileDialog::GetFileTitle() const
{
	return m_szFileTitle;
}

inline void CFileDialog::SetTemplate(UINT nID,TypeOfResourceHandle bType)
{
	SetTemplate(MAKEINTRESOURCE(nID),bType);
}

inline void CFileDialog::SetControlText(int nID,LPCSTR lpsz)
{
	::SendMessage(::GetParent(m_hWnd),CDM_SETCONTROLTEXT,nID,(LPARAM)lpsz);
}

inline void CFileDialog::HideControl(int nID)
{
	::SendMessage(::GetParent(m_hWnd),CDM_HIDECONTROL,nID,0);
}

///////////////////////////
// Class CFontDialog

inline CString CFontDialog::GetFaceName() const
{
	return m_lf.lfFaceName;
}

inline CString CFontDialog::GetStyleName() const
{
	return m_szStyleName;
}

inline int CFontDialog::GetSize() const
{
	return m_cf.iPointSize;
}

inline COLORREF CFontDialog::GetColor() const
{
	return (COLORREF)m_cf.rgbColors;
}

inline int CFontDialog::GetWeight() const
{
	return m_lf.lfWeight;
}

inline BOOL CFontDialog::IsStrikeOut() const
{
	return m_lf.lfStrikeOut;
}

inline BOOL CFontDialog::IsUnderline() const
{
	return m_lf.lfUnderline;
}

inline BOOL CFontDialog::IsBold() const
{
    return (m_lf.lfWeight>=600);
}

inline BOOL CFontDialog::IsItalic() const
{
	return m_lf.lfItalic;
}

///////////////////////////
// Class CColorDialog

inline void CColorDialog::SetCurrentColor(COLORREF clr)
{
	m_cc.rgbResult=clr;
}

inline COLORREF CColorDialog::GetColor() const
{
	return m_cc.rgbResult;
}

///////////////////////////
// Class CPrintDialog

inline BOOL CPrintDialog::PrintCollate() const
{
	return (m_pd.Flags&PD_COLLATE);
}

inline BOOL CPrintDialog::PrintSelection() const
{
	return (m_pd.Flags&PD_SELECTION);
}

inline BOOL CPrintDialog::PrintAll() const
{
	return (m_pd.Flags&PD_ALLPAGES);
}

inline BOOL CPrintDialog::PrintRange() const
{
	return (m_pd.Flags&PD_PAGENUMS);
}

inline int CPrintDialog::GetFromPage() const
{
	return m_pd.nFromPage;
}

inline int CPrintDialog::GetToPage() const
{
	return m_pd.nToPage;
}

inline HDC CPrintDialog::GetPrinterDC() const
{
	return m_pd.hDC;
}

///////////////////////////
// Class CFindReplaceDialog

inline CString CFindReplaceDialog::GetReplaceString() const
{
	return m_szReplaceWith;
}

inline CString CFindReplaceDialog::GetFindString() const
{
	return m_szFindWhat;
}

inline BOOL CFindReplaceDialog::SearchDown() const
{
	return (m_fr.Flags&FR_DOWN);
}

inline BOOL CFindReplaceDialog::FindNext() const
{
	return (m_fr.Flags&FR_FINDNEXT);
}

inline BOOL CFindReplaceDialog::MatchCase() const
{
	return (m_fr.Flags&FR_MATCHCASE);
}

inline BOOL CFindReplaceDialog::MatchWholeWord() const
{
	return (m_fr.Flags&FR_WHOLEWORD);
}

inline BOOL CFindReplaceDialog::ReplaceCurrent() const
{
	return (m_fr.Flags&FR_REPLACE);
}

inline BOOL CFindReplaceDialog::ReplaceAll() const
{
	return (m_fr.Flags&FR_REPLACE);
}

///////////////////////////
// Class CInputDialog


inline CInputDialog::CInputDialog(LPCTSTR lpTemplate,BYTE bFlags)
:	CDialog(lpTemplate),m_bFlags(bFlags),
	m_OKButton("OK"),m_CancelButton("CANCEL")
{
}

inline CInputDialog::CInputDialog(int iTemplate,BYTE bFlags)
:	CDialog(iTemplate),m_bFlags(bFlags),
	m_OKButton("OK"),m_CancelButton("CANCEL")
{
}

inline void CInputDialog::SetInputText(int iText)
{
	m_Input.LoadString(iText);
	if (m_hWnd!=NULL)
		SetDlgItemText(IDC_EDIT,m_Input);
}

inline int CInputDialog::GetInputText(CString& text) const
{
	text=m_Input;
	return text.GetLength();
}



#endif
#endif
