////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2007 Janne Huttunen
////////////////////////////////////////////////////////////////////


#ifndef HFCDIALOGS_H
#define HFCDIALOGS_H

#if defined(DEF_RESOURCES) && defined(DEF_WINDOWS)

class CDialog : public CTargetWnd
{
protected:
	union {
		LPCSTR m_lpszTemplateName;
		LPCWSTR m_lpszTemplateNameW;
	};

public:
	CDialog(LPCSTR lpTemplate);
	CDialog(LPCWSTR lpTemplate);
	CDialog(int iTemplate);
	virtual ~CDialog();
	
	BOOL Create(HWND hWndParent=NULL,TypeOfResourceHandle bType=LanguageSpecificResource);

	INT_PTR DoModal(HWND hWndParent=NULL,TypeOfResourceHandle bType=LanguageSpecificResource);

public:
	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual LRESULT WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);

public:
	BOOL EndDialog(int nResult) const;
};

class CPropertyPage : public CDialog
{
public:
	CPropertyPage();
	CPropertyPage(HWND hWnd);
	CPropertyPage(UINT nIDTemplate,UINT nIDCaption=0,TypeOfResourceHandle bType=LanguageSpecificResource);
	CPropertyPage(UINT nIDTemplate,LPCSTR lpszCaption,TypeOfResourceHandle bType=LanguageSpecificResource);
	CPropertyPage(UINT nIDTemplate,LPCWSTR lpszCaption,TypeOfResourceHandle bType=LanguageSpecificResource);
	CPropertyPage(LPCSTR lpszTemplateName,UINT nIDCaption=0,TypeOfResourceHandle bType=LanguageSpecificResource);
	CPropertyPage(LPCSTR lpszTemplateName,LPCSTR lpszCaption,TypeOfResourceHandle bType=LanguageSpecificResource);
	CPropertyPage(LPCWSTR lpszTemplateName,UINT nIDCaption=0,TypeOfResourceHandle bType=LanguageSpecificResource);
	CPropertyPage(LPCWSTR lpszTemplateName,LPCWSTR lpszCaption,TypeOfResourceHandle bType=LanguageSpecificResource);
	virtual ~CPropertyPage();
	
	
	void Construct(UINT nIDCaption=0,TypeOfResourceHandle bType=LanguageSpecificResource);
	void Construct(LPCSTR szCaption,TypeOfResourceHandle bType=LanguageSpecificResource);
	void Construct(LPCWSTR szCaption,TypeOfResourceHandle bType=LanguageSpecificResource);
	
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
	union {
		PROPSHEETPAGE m_psp;
		PROPSHEETPAGEW m_pspw;
	};

	
protected:
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
	CPropertySheet(LPCSTR pszCaption,HWND hParentWnd=NULL,UINT iSelectPage=0);
	CPropertySheet(LPCWSTR pszCaption,HWND hParentWnd=NULL,UINT iSelectPage=0);
	virtual ~CPropertySheet();

	void Construct(UINT nIDCaption,HWND hParentWnd=NULL,UINT iSelectPage=0);
	void Construct(LPCSTR pszCaption,HWND hParentWnd=NULL,UINT iSelectPage=0);
	void Construct(LPCWSTR pszCaption,HWND hParentWnd=NULL,UINT iSelectPage=0);

	BOOL Create(HWND hParentWnd=NULL,DWORD dwStyle=(DWORD)-1,DWORD dwExStyle=0);

public:
	int GetPageCount() const;
	HWND GetActivePage() const;
	int GetActiveIndex() const;
	CPropertyPage* GetPage(int nPage) const;
	int GetPageIndex(CPropertyPage* pPage);

	BOOL SetActivePage(int nPage);
	BOOL SetActivePage(CPropertyPage* pPage);
	void SetTitle(LPCSTR lpszText,UINT nStyle=0);
	void SetTitle(LPCWSTR lpszText,UINT nStyle=0);
	HWND GetTabControl() const;

	void SetWizardMode();
	void SetFinishText(LPCSTR lpszText);
	void SetFinishText(LPCWSTR lpszText);
	void SetWizardButtons(DWORD dwFlags);

	void SetIcon(LPCSTR szIcon);
	void SetIcon(LPCWSTR szIcon);
	void SetIcon(int nIcon);
	void SetIcon(HICON hIcon);

	DWORD GetFlags() const { return m_psh.dwFlags; }
	void AddFlags(DWORD dwAdd) { m_psh.dwFlags|=dwAdd; }
	void RemoveFlags(DWORD dwRemove) { m_psh.dwFlags&=~dwRemove; }
	
public:
	virtual INT_PTR DoModal();
	void AddPage(CPropertyPage* pPage);
	void RemovePage(CPropertyPage* pPage);
	void RemovePage(int nPage);
	void EndDialog(int nEndID);
	BOOL PressButton(int nButton);

	virtual void BuildPropPageArray();
	
protected:
	union {
		PROPSHEETHEADER m_psh;
		PROPSHEETHEADERW m_pshw;
	};
	
protected:

	CPtrArray m_pages;
	HWND m_hParentWnd;
	
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
	CInputDialog(LPCSTR lpTemplate,BYTE bFlags=0);
	CInputDialog(int iTemplate,BYTE bFlags=0);

	INT_PTR DoModal(HWND hWndParent=NULL);
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

#ifdef DEF_WCHAR
	void SetTitle(LPCWSTR szTitle);
	BOOL SetText(LPCWSTR szText);
	void SetInputText(LPCWSTR szText);
	int GetInputText(CStringW& text) const;
	int GetInputText(LPWSTR szText,int nTextLen) const;
	
	void SetOKButtonText(LPCWSTR szText);
	void SetCancelButtonText(LPCWSTR szText);
#endif

protected:
	BYTE m_bFlags;
	CStringW m_Title;
	CStringW m_Text;
	CStringW m_Input;
	CStringW m_OKButton;
	CStringW m_CancelButton;
};

class CFileDialog : public CCommonDialog
{
public:
#ifdef DEF_WCHAR
	union {
		OPENFILENAME* m_pofn;
		OPENFILENAMEW* m_pwofn;
	};
#else
	OPENFILENAME* m_pofn;
#endif

	CFileDialog(BOOL bOpenFileDialog,LPCSTR lpszDefExt=NULL,LPCSTR lpszFileName=NULL,
		DWORD dwFlags=OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,	LPCSTR lpszFilter=NULL);
	CFileDialog(BOOL bOpenFileDialog,LPCSTR lpszDefExt,LPCSTR lpszFileName,DWORD dwFlags,	UINT uFilterId);
	virtual ~CFileDialog();

	BOOL EnableFeatures(DWORD nFlags=efCheck);
	void SetTitle(LPCSTR szTitle);
	BOOL DoModal(HWND hParentWnd=NULL);

	BOOL GetFilePath(CString& sPath) const;
	BOOL GetFileName(CString& sFileName) const;
	void GetFileTitle(CString& sFileTitle) const;
	
	BOOL GetFilePath(LPSTR pFilePath,DWORD nMaxLen) const;
	BOOL GetFileName(LPSTR pFileName,DWORD nMaxLen) const;
	void GetFileTitle(LPSTR pFileTitle,DWORD nMaxLen) const;
	
	void SetFileTitle(LPCSTR szTitle);

	
	int GetFilterIndex() const;
	BOOL GetReadOnlyPref() const;

	void SetTemplate(UINT nID,TypeOfResourceHandle bType=LanguageSpecificResource);
	void SetTemplate(LPCTSTR lpID,TypeOfResourceHandle bType=LanguageSpecificResource);

	BOOL GetFolderPath(CString& sFolderPath) const;
	void SetControlText(int nID,LPCSTR lpsz);
	void HideControl(int nID);
	void SetDefExt(LPCSTR lpsz);

#ifdef DEF_WCHAR
	CFileDialog(BOOL bOpenFileDialog,LPCWSTR lpszDefExt,LPCWSTR lpszFileName=NULL,
		DWORD dwFlags=OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,	LPCWSTR lpszFilter=NULL);
	CFileDialog(BOOL bOpenFileDialog,LPCWSTR lpszDefExt,LPCWSTR lpszFileName,DWORD dwFlags,UINT uFilterId);	

	void SetTitle(LPCWSTR szTitle);

	
	BOOL GetFilePath(CStringW& sPath) const;
	BOOL GetFileName(CStringW& sFileName) const;
	void GetFileTitle(CStringW& sFileTitle) const;
	BOOL GetFolderPath(CStringW& sFolderPath) const;
	void SetFileTitle(LPCWSTR szTitle);
	
	BOOL GetFilePath(LPWSTR pFilePath,DWORD nMaxLen) const;
	BOOL GetFileName(LPWSTR pFileName,DWORD nMaxLen) const;
	void GetFileTitle(LPWSTR pFileTitle,DWORD nMaxLen) const;

	
#endif

protected:
	virtual UINT OnShareViolation(LPCTSTR lpszPathName);
	virtual BOOL OnFileNameOK();
	virtual void OnInitDone();
	virtual void OnFileNameChange();
	virtual void OnFolderChange();
	virtual void OnTypeChange();

	void Init(LPCSTR lpszDefExt,LPCSTR lpszFileName,DWORD dwFlags,LPCSTR lpszFilter);
#ifdef DEF_WCHAR
	void Init(LPCWSTR lpszDefExt,LPCWSTR lpszFileName,DWORD dwFlags,LPCWSTR lpszFilter);
#endif

protected:
	BOOL m_bOpenFileDialog;
	union {
		LPSTR m_pFilter;
		LPWSTR m_pwFilter;
	};
	union {
        LPSTR m_pFileName;
		LPWSTR m_pwFileName;
	};

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
	BOOL GetFolder(CStringW& Folder) const;
	BOOL GetFolder(LPSTR szFolder) const;
	BOOL GetFolder(LPWSTR szFolder) const;
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

// class COptionsPropertyPage : public CPropertyPage





/////////////////////////////////////////////////
// Inline function
/////////////////////////////////////////////////

/////////////////////////////////////////////////
// CDialog


inline CDialog::CDialog(int iTemplate)
	: CTargetWnd(NULL),m_lpszTemplateName(MAKEINTRESOURCE(iTemplate))
{
}


/////////////////////////////////////////////////
// CPropertyPage

inline CPropertyPage::CPropertyPage()
:	CDialog(0),m_bFirstSetActive(FALSE)
{
	m_psp.pszTitle=NULL;
}

inline CPropertyPage::CPropertyPage(HWND hWnd)
:	CDialog(0),m_bFirstSetActive(FALSE)
{
	m_psp.pszTitle=NULL;
}

inline CPropertyPage::CPropertyPage(UINT nIDTemplate,UINT nIDCaption,TypeOfResourceHandle bType)
:	CDialog(nIDTemplate)
{
	Construct(nIDCaption,bType);
}

inline CPropertyPage::CPropertyPage(UINT nIDTemplate,LPCSTR lpszCaption,TypeOfResourceHandle bType)
:	CDialog(nIDTemplate)
{
	Construct(lpszCaption,bType);
}


inline CPropertyPage::CPropertyPage(UINT nIDTemplate,LPCWSTR lpszCaption,TypeOfResourceHandle bType)
:	CDialog(nIDTemplate)
{
	Construct(lpszCaption,bType);
}

inline CPropertyPage::CPropertyPage(LPCSTR lpszTemplateName,UINT nIDCaption,TypeOfResourceHandle bType)
:	CDialog(lpszTemplateName)
{
	Construct(nIDCaption,bType);
}

inline CPropertyPage::CPropertyPage(LPCSTR lpszTemplateName,LPCSTR lpszCaption,TypeOfResourceHandle bType)
:	CDialog(lpszTemplateName)
{
	Construct(lpszCaption,bType);
}

inline CPropertyPage::CPropertyPage(LPCWSTR lpszTemplateName,UINT nIDCaption,TypeOfResourceHandle bType)
:	CDialog(lpszTemplateName)
{
	Construct(nIDCaption,bType);
}

inline CPropertyPage::CPropertyPage(LPCWSTR lpszTemplateName,LPCWSTR lpszCaption,TypeOfResourceHandle bType)
:	CDialog(lpszTemplateName)
{
	Construct(lpszCaption,bType);
}

inline CPropertyPage::operator HPROPSHEETPAGE() const
{
	return (HPROPSHEETPAGE) m_hWnd;
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
{
	ZeroMemory(&m_pshw,sizeof(PROPSHEETHEADERW));
	m_pshw.dwSize=PROPSHEETHEADERW_V1_SIZE; //sizeof(PROPSHEETHEADER);
}

inline CPropertySheet::CPropertySheet(HWND hWnd)
:	CWnd(hWnd)
{
	ZeroMemory(&m_pshw,sizeof(PROPSHEETHEADERW));
	m_pshw.dwSize=PROPSHEETHEADERW_V1_SIZE; //sizeof(PROPSHEETHEADER);
}

inline CPropertySheet::CPropertySheet(UINT nIDCaption,HWND hParentWnd,UINT iSelectPage)
{
	Construct(nIDCaption,hParentWnd,iSelectPage);
}

inline CPropertySheet::CPropertySheet(LPCSTR pszCaption,HWND hParentWnd,UINT iSelectPage)
{
	Construct(pszCaption,hParentWnd,iSelectPage);
}

inline CPropertySheet::CPropertySheet(LPCWSTR pszCaption,HWND hParentWnd,UINT iSelectPage)
{
	Construct(pszCaption,hParentWnd,iSelectPage);
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

inline CFileDialog::CFileDialog(BOOL bOpenFileDialog,LPCSTR lpszDefExt,
						 LPCSTR lpszFileName,DWORD dwFlags,LPCSTR lpszFilter)
:	CCommonDialog(),m_bOpenFileDialog(bOpenFileDialog)
{
	Init(lpszDefExt,lpszFileName,dwFlags,lpszFilter);
}

inline CFileDialog::CFileDialog(BOOL bOpenFileDialog,LPCWSTR lpszDefExt,
						 LPCWSTR lpszFileName,DWORD dwFlags,LPCWSTR lpszFilter)
:	CCommonDialog(),m_bOpenFileDialog(bOpenFileDialog)
{
	Init(lpszDefExt,lpszFileName,dwFlags,lpszFilter);
}

inline CFileDialog::CFileDialog(BOOL bOpenFileDialog,LPCSTR lpszDefExt,
						 LPCSTR lpszFileName,DWORD dwFlags,UINT nFilderID)
:	CCommonDialog(),m_bOpenFileDialog(bOpenFileDialog)
{
	Init(lpszDefExt,lpszFileName,dwFlags,CString(nFilderID));
}

inline CFileDialog::CFileDialog(BOOL bOpenFileDialog,LPCWSTR lpszDefExt,
						 LPCWSTR lpszFileName,DWORD dwFlags,UINT nFilderID)
:	CCommonDialog(),m_bOpenFileDialog(bOpenFileDialog)
{
	Init(lpszDefExt,lpszFileName,dwFlags,CStringW(nFilderID));
}

inline void CFileDialog::GetFileTitle(CString& sFileTitle) const
{
	if (IsUnicodeSystem())	
		sFileTitle=m_pwofn->lpstrFileTitle;
	else
		sFileTitle=m_pofn->lpstrFileTitle;
}

inline void CFileDialog::GetFileTitle(LPSTR pFileTitle,DWORD nMaxLen) const
{
	if (IsUnicodeSystem())	
		WideCharToMultiByte(CP_ACP,0,m_pwofn->lpstrFileTitle,-1,pFileTitle,nMaxLen,NULL,NULL);
	else
		StringCbCopy(pFileTitle,nMaxLen-1,m_pofn->lpstrFileTitle);
}


inline void CFileDialog::GetFileTitle(CStringW& sFileTitle) const
{
	if (IsUnicodeSystem())	
		sFileTitle=m_pwofn->lpstrFileTitle;
	else
		sFileTitle=m_pofn->lpstrFileTitle;
}

inline void CFileDialog::GetFileTitle(LPWSTR pFileTitle,DWORD nMaxLen) const
{
	if (IsUnicodeSystem())	
		StringCbCopyW(pFileTitle,nMaxLen*2,m_pwofn->lpstrFileTitle);
	else
		MultiByteToWideChar(CP_ACP,0,m_pofn->lpstrFileTitle,-1,pFileTitle,nMaxLen);
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

inline int CFileDialog::GetFilterIndex() const
{
	return m_pofn->nFilterIndex;
}

inline void CFileDialog::SetFileTitle(LPCSTR pFileTitle)
{
	if (IsUnicodeSystem())	
		MultiByteToWideChar(CP_ACP,0,pFileTitle,-1,m_pwofn->lpstrFileTitle,64);
	else
		StringCbCopy(m_pofn->lpstrFileTitle,64,pFileTitle);
}

inline void CFileDialog::SetTitle(LPCSTR pTitle)
{
	if (IsUnicodeSystem())	
	{
		if (m_pwofn->lpstrTitle!=NULL)
			delete[] m_pwofn->lpstrTitle;
		m_pwofn->lpstrTitle=alloccopyAtoW(pTitle);
	}
	else
	{
		if (m_pwofn->lpstrTitle!=NULL)
			delete[] m_pwofn->lpstrTitle;
		m_pofn->lpstrTitle=alloccopy(pTitle);
	}
}
#ifdef DEF_WCHAR
inline void CFileDialog::SetFileTitle(LPCWSTR pFileTitle)
{
	if (IsUnicodeSystem())	
		StringCbCopyW(m_pwofn->lpstrFileTitle,64*2,pFileTitle);
	else
		WideCharToMultiByte(CP_ACP,0,pFileTitle,-1,m_pofn->lpstrFileTitle,64*2,NULL,NULL);
	
}


inline void CFileDialog::SetTitle(LPCWSTR pTitle)
{
	if (IsUnicodeSystem())	
	{
		if (m_pwofn->lpstrTitle!=NULL)
			delete[] m_pwofn->lpstrTitle;
		m_pwofn->lpstrTitle=alloccopy(pTitle);
	}
	else
	{
		if (m_pwofn->lpstrTitle!=NULL)
			delete[] m_pwofn->lpstrTitle;
		m_pofn->lpstrTitle=alloccopyWtoA(pTitle);
	}
}
#endif

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


inline CInputDialog::CInputDialog(LPCSTR lpTemplate,BYTE bFlags)
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
	return (int)text.GetLength();
}

#ifdef DEF_WCHAR
inline int CInputDialog::GetInputText(CStringW& text) const
{
	text=m_Input;
	return (int)text.GetLength();
}

#endif



///////////////////////////
// Class COptionsPropertyPage






///////////////////////////
// Class CFolderDialog


inline LPITEMIDLIST CFolderDialog::GetFolder() const
{
	return m_lpil;
}

inline BOOL CFolderDialog::GetFolder(CString& Folder) const
{
	if (!SHGetPathFromIDList(m_lpil,Folder.GetBuffer(_MAX_PATH)))
		return FALSE;
	Folder.FreeExtra();
	return TRUE;
}

inline BOOL CFolderDialog::GetFolder(LPSTR szFolder) const
{
	return SHGetPathFromIDList(m_lpil,szFolder);
}



#endif
#endif
