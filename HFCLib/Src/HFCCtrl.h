////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////


#ifndef HFCCTRL_H
#define HFCCTRL_H

#ifdef DEF_WINDOWS

// Classes for windows basic controls
class CWndCtrl : public CCmdTarget
{
public:
	enum ShowState { 
		swHide=SW_HIDE,
		swShowNormal=SW_SHOWNORMAL,
		swNormal=SW_NORMAL,
		swShowMinimized=SW_SHOWMINIMIZED,
		swShowMaximized=SW_SHOWMAXIMIZED,
		swMaximize=SW_MAXIMIZE,
		swShowNoActivate=SW_SHOWNOACTIVATE,
		swShow=SW_SHOW,
		swMinimize=SW_MINIMIZE,
		swShowMinNiActive=SW_SHOWMINNOACTIVE,
		swShowNA=SW_SHOWNA,
		swRestore=SW_RESTORE,
		swShowDefault=SW_SHOWDEFAULT,
		swForceMinimize=SW_FORCEMINIMIZE,
		swMax=SW_MAX
	};
	
	enum WindowLongIndex {
		gwlWndProc=GWL_WNDPROC,
		gwlHInstance=GWL_HINSTANCE,
		gwlHWndParent=GWL_HWNDPARENT,
		gwlStyle=GWL_STYLE,
		gwlExStyle=GWL_EXSTYLE,
		gwlUserData=GWL_USERDATA,
		gwlID=GWL_ID,
		dwlMsgResult=DWL_MSGRESULT,
		dwlDlgProc=DWL_DLGPROC,
		dwlUser=DWL_USER
	};
	
public:
	CWndCtrl(HWND hWnd=NULL) { m_hWnd=hWnd; }

	HWND GetHandle() const { return m_hWnd; }
	void SetHandle(HWND hWnd) { m_hWnd=hWnd; }
	void AssignToDlgItem(HWND hDialog,int nID) { m_hWnd=::GetDlgItem(hDialog,nID); }
	operator HWND() const { return m_hWnd; }
	
	BOOL operator==(const CWnd& wnd) const { return (m_hWnd==wnd.m_hWnd); }
	BOOL operator!=(const CWnd& wnd) const { return (m_hWnd!=wnd.m_hWnd); }
	BOOL operator==(const CWndCtrl& wnd) const { return (m_hWnd==wnd.m_hWnd); }
	BOOL operator!=(const CWndCtrl& wnd) const { return (m_hWnd!=wnd.m_hWnd); }
	
	DWORD GetStyle() const { return ::GetWindowLong(m_hWnd,GWL_STYLE); }
	DWORD SetStyle(DWORD dwStyle) { return ::SetWindowLong(m_hWnd,GWL_STYLE,dwStyle); }
	BOOL ModifyStyle(DWORD dwRemove,DWORD dwAdd,UINT nFlags=0);
	DWORD GetExStyle() const { return ::GetWindowLong(m_hWnd,GWL_EXSTYLE); }
	DWORD SetExStyle(DWORD dwExStyle) { return ::SetWindowLong(m_hWnd,GWL_EXSTYLE,dwExStyle); }
	BOOL ModifyStyleEx(DWORD dwRemove,DWORD dwAdd,UINT nFlags=0);

	LONG GetWindowLong(WindowLongIndex nIndex) const { return ::GetWindowLong(m_hWnd,nIndex); }	
	LONG SetWindowLong(WindowLongIndex nIndex,LONG lNewLong) { return ::SetWindowLong(m_hWnd,nIndex,lNewLong); }
	
	BOOL Create(LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT* rect,
		HWND hParentWnd, UINT nID);

	BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		int x, int y, int nWidth, int nHeight,
		HWND hWndParent, UINT nID, LPVOID lpParam = NULL);

	BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		const RECT* rect,
		HWND hParentWnd, UINT nID,
		LPVOID lpParam = NULL);

	BOOL DestroyWindow() {return ::DestroyWindow(m_hWnd);}
	
	int GetDlgCtrlID() const {return ::GetDlgCtrlID(m_hWnd);}
	int SetDlgCtrlID(int nID) {return ::SetWindowLong(m_hWnd,GWL_ID,(LONG)nID);}
	HWND GetParent() const { return ::GetParent(m_hWnd); }
	HWND SetParent(HWND hwndNewParent) {return ::SetParent(m_hWnd,hwndNewParent);}
	
	BOOL PostMessage(UINT uMsg,WPARAM wParam=0,LPARAM lParam=0) const { return ::PostMessage(m_hWnd,uMsg,wParam,lParam); }
	LRESULT SendMessage(UINT uMsg,WPARAM wParam=0,LPARAM lParam=0) const { return ::SendMessage(m_hWnd,uMsg,wParam,lParam); }
	
	BOOL SetWindowText(LPCSTR lpsz) {return ::SetWindowText(m_hWnd,lpsz); }
	int GetWindowText(LPSTR lpString,int nMaxCount) const { return ::GetWindowText(m_hWnd,lpString,nMaxCount); }
	int GetWindowText(CString& str) const;
	int GetWindowTextLength() const { return ::GetWindowTextLength(m_hWnd); }
	void SetFont(HFONT hFont,BOOL bRedraw=TRUE) { ::SendMessage(m_hWnd,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(bRedraw,0)); }
	HFONT GetFont() const { return (HFONT)::SendMessage(m_hWnd,WM_GETFONT,0,0); }
	
	void MoveWindow(int x,int y,int nWidth,int nHeight,BOOL bRepaint=TRUE) { ::MoveWindow(m_hWnd,x,y,nWidth,nHeight,bRepaint); }
	void MoveWindow(LPCRECT lpRect,BOOL bRepaint=TRUE) {::MoveWindow(m_hWnd,lpRect->left,lpRect->top,lpRect->right-lpRect->left,lpRect->bottom-lpRect->top,bRepaint);}
	int SetWindowRgn(HRGN hRgn, BOOL bRedraw) { return ::SetWindowRgn(m_hWnd,hRgn,bRedraw); }
	int GetWindowRgn(HRGN hRgn) const {return ::GetWindowRgn(m_hWnd,hRgn);}
	BOOL SetWindowPos(HWND hWndInsertAfter,int x,int y,int cx,int cy,UINT nFlags) {return ::SetWindowPos(m_hWnd,hWndInsertAfter,x,y,cx,cy,nFlags);}
	void GetWindowRect(LPRECT lpRect) const { ::GetWindowRect(m_hWnd,lpRect); }
	void GetClientRect(LPRECT lpRect) const { ::GetClientRect(m_hWnd,lpRect); }

	void ClientToScreen(LPPOINT lpPoint) const {::ClientToScreen(m_hWnd,lpPoint); }
	void ClientToScreen(LPRECT lpRect) const;
	void ScreenToClient(LPPOINT lpPoint) const { ::ScreenToClient(m_hWnd,lpPoint); }
	void ScreenToClient(LPRECT lpRect) const;
	void MapWindowPoints(HWND hwndTo, LPPOINT lpPoint, UINT nCount) const { ::MapWindowPoints(m_hWnd,hwndTo,lpPoint,nCount); }
	void MapWindowPoints(HWND hwndTo, LPRECT lpRect) const;

	HDC GetDC() const { return ::GetDC(m_hWnd); }
	CDC* GetCDC() { return new CDC(this); }
	HDC GetWindowDC() const { return ::GetWindowDC(m_hWnd); }
	int ReleaseDC(HDC hDC) { return ::ReleaseDC(m_hWnd,hDC); }
	void ReleaseCDC(CDC* pDC) { delete pDC; }

	BOOL UpdateWindow() const { return ::UpdateWindow(m_hWnd); }
	
	BOOL ShowWindow(ShowState nCmdShow) const { return ::ShowWindow(m_hWnd,nCmdShow); }
	BOOL IsWindowVisible() const { return ::IsWindowVisible(m_hWnd); }
	BOOL RedrawWindow(LPCRECT lpRectUpdate=NULL,HRGN hrgnUpdate=NULL,UINT flags=RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE) { return ::RedrawWindow(m_hWnd,lpRectUpdate,hrgnUpdate,flags); }
	
	BOOL GetUpdateRect(LPRECT lpRect,BOOL bErase=FALSE) {return ::GetUpdateRect(m_hWnd,lpRect,bErase); }
	int GetUpdateRgn(HRGN hRgn,BOOL bErase=FALSE) { return ::GetUpdateRgn(m_hWnd,hRgn,bErase); }
	BOOL Invalidate(BOOL bErase=TRUE) {return ::InvalidateRect(m_hWnd,NULL,bErase); }
	BOOL InvalidateRect(CONST RECT* lpRect,BOOL bErase) { return ::InvalidateRect(m_hWnd,lpRect,bErase); }
	BOOL InvalidateRgn(HRGN hRgn,BOOL bErase=TRUE) {return  ::InvalidateRgn(m_hWnd,hRgn,bErase); }

	BOOL IsWindowEnabled() const { return ::IsWindowEnabled(m_hWnd); }
	BOOL EnableWindow(BOOL bEnable=TRUE) { return ::EnableWindow(m_hWnd,bEnable); }
	HWND SetFocus() const { return ::SetFocus(m_hWnd); }
	
	UINT GetTextLength() const { return (UINT)::SendMessage(m_hWnd,WM_GETTEXTLENGTH,0,0); } 
	UINT GetText(LPSTR lpszText,UINT cchTextMax) const { return (UINT)::SendMessage(m_hWnd,WM_GETTEXT,(WPARAM)cchTextMax,(LPARAM)lpszText); } 
	UINT GetText(CStringA& str) const;
	BOOL SetText(LPCSTR lpsz) { return (BOOL)::SendMessage(m_hWnd,WM_SETTEXT,0,(LPARAM)lpsz); }

#ifdef DEF_WCHAR
	BOOL SetWindowText(LPCWSTR lpsz);
	int GetWindowText(LPWSTR lpString,int nMaxCount) const;
	int GetWindowText(CStringW& str) const;
	
	//widechar support
	UINT GetText(CStringW& str) const;
	UINT GetText(LPWSTR lpszText,UINT cchTextMax) const;
	BOOL SetText(LPCWSTR lpsz);
#endif

	friend class CWnd;

protected:
	HWND m_hWnd;
};

class CButton : public CWndCtrl
{
public:
	CButton();
	CButton(HWND hWnd);
	BOOL Create(LPCTSTR lpszCaption,DWORD dwStyle,const RECT* rect,HWND hParentWnd,UINT nID);

public:
	UINT GetState() const;
	void SetState(BOOL bHighlight);
	int GetCheck() const;
	void SetCheck(int nCheck);
	UINT GetButtonStyle() const;
	void SetButtonStyle(UINT nStyle,BOOL bRedraw=TRUE);

	HICON SetIcon(HICON hIcon);
	HICON GetIcon() const;
	HBITMAP SetBitmap(HBITMAP hBitmap);
	HBITMAP GetBitmap() const;
	HCURSOR SetCursor(HCURSOR hCursor);
	HCURSOR GetCursor();
};

class CEdit : public CWndCtrl
{
public:
	CEdit();
	CEdit(HWND hWnd);
	BOOL Create(DWORD dwStyle,const RECT* rect,HWND hParentWnd,UINT nID);

public:
	void LimitText(int iLimit);
	void SetLimitText(int iLimit);
	void SetSel(int iStart,int iEnd);

	BOOL Undo();
	void Clear();
	void Copy();
	void Cut();
	void Paste();
};

class CRichEditCtrl : public CWndCtrl
{
public:
	CRichEditCtrl();
	CRichEditCtrl(HWND hWnd);

	BOOL Create(DWORD dwStyle, const RECT* rect, HWND hParentWnd, UINT nID);
	BOOL CreateV2(DWORD dwStyle, const RECT* rect, HWND hParentWnd, UINT nID);
	
	BOOL CanUndo() const;
	BOOL CanRedo() const;
	UNDONAMEID GetUndoName() const;
	UNDONAMEID GetRedoName() const;

	BOOL Undo();
	BOOL Redo();
	void Clear();
	void Copy();
	void Cut();
	void Paste();

	int GetLineCount() const;
	BOOL GetModify() const;
	
	void SetModify(BOOL bModified=TRUE);
	void GetRect(LPRECT lpRect) const;
	void SetRect(LPCRECT lpRect);
	
	CPoint GetCharPos(long lChar) const;
	void SetOptions(WORD wOp, DWORD dwFlags);

	int GetLine(int nIndex,LPTSTR lpszBuffer) const;
	BOOL CanPaste(UINT nFormat=0) const;
	void GetSel(long& nStartChar,long& nEndChar) const;
	void GetSel(CHARRANGE &cr) const;
	void LimitText(long nChars=0);
	long LineFromChar(long nIndex) const;
	void SetSel(long nStartChar,long nEndChar);
	void SetSel(CHARRANGE &cr);
	DWORD GetDefaultCharFormat(CHARFORMAT &cf) const;
	DWORD GetDefaultCharFormat(CHARFORMAT2 &cf) const;
	DWORD GetSelectionCharFormat(CHARFORMAT &cf) const;
	DWORD GetSelectionCharFormat(CHARFORMAT2 &cf) const;
	long GetEventMask() const;
	long GetLimitText() const;
	DWORD GetParaFormat(PARAFORMAT &pf) const;
	DWORD GetParaFormat(PARAFORMAT2 &pf) const;
	long GetSelText(LPSTR lpBuf) const;
	CString GetSelText() const;
	WORD GetSelectionType() const;
	COLORREF SetBackgroundColor(BOOL bSysColor,COLORREF cr);
	BOOL SetCharFormat(DWORD dwFlags,CHARFORMAT &cf);
	BOOL SetCharFormat(DWORD dwFlags,CHARFORMAT2 &cf);
	BOOL SetDefaultCharFormat(CHARFORMAT &cf);
	BOOL SetDefaultCharFormat(CHARFORMAT2 &cf);
	BOOL SetSelectionCharFormat(CHARFORMAT &cf);
	BOOL SetSelectionCharFormat(CHARFORMAT2 &cf);
	BOOL SetWordCharFormat(CHARFORMAT &cf);
	BOOL SetWordCharFormat(CHARFORMAT2 &cf);
	DWORD SetEventMask(DWORD dwEventMask);
	BOOL SetParaFormat(PARAFORMAT &pf);
	BOOL SetParaFormat(PARAFORMAT2 &pf);
	BOOL SetTargetDevice(HDC hDC,long lLineWidth);
	
	long GetTextLength() const;
	BOOL SetReadOnly(BOOL bReadOnly = TRUE);
	int GetFirstVisibleLine() const;
	BOOL SetTextEx(LPCSTR szText,DWORD dwFlags=ST_DEFAULT,UINT codepage=CP_ACP);
	BOOL SetTextEx(LPCWSTR szText,DWORD dwFlags=ST_DEFAULT,UINT codepage=1200);
	BOOL SetZoom(DWORD nNumerator,DWORD nDenominator);

	void EmptyUndoBuffer();

	int LineIndex(int nLine=-1) const;
	int LineLength(int nLine=-1) const;
	void LineScroll(int nLines,int nChars=0);
	void ReplaceSel(LPCTSTR lpszNewText,BOOL bCanUndo=FALSE);
	
	BOOL DisplayBand(LPRECT pDisplayRect);
	long FindText(DWORD dwFlags,FINDTEXTEX* pFindText) const;
	long FormatRange(FORMATRANGE* pfr,BOOL bDisplay=TRUE);
	void HideSelection(BOOL bHide,BOOL bPerm=0);
	void PasteSpecial(UINT nClipFormat);
	void RequestResize();
	long StreamIn(int nFormat,EDITSTREAM &es);
	long StreamOut(int nFormat,EDITSTREAM &es);

	DWORD SetOptions(DWORD dwOptions,DWORD dwOperation=ECOOP_SET);
	DWORD GetOptions() const;

	BOOL GetOleInterface(LPVOID* ppOleInterface) const; // Release must call
	IRichEditOle* GetOleInterface() const; // Release must call
	BOOL SetOleCallback(IRichEditOleCallback *pOleCallback); // Release must call

	BOOL SetAlignmentForSelection(WORD wAlignment);
	BOOL SetLineSpacingRuleForSelection(BYTE bLineSpacingRule,LONG dyLineSpacing=0);
	BOOL SetEffectForSelection(DWORD dwEffects,DWORD dwMask);
	BOOL SetUnderlineTypeForSelection(BYTE bUnderlineType);
		
};


class CListBox : public CWndCtrl
{
public:
	CListBox();
	CListBox(HWND hWnd);

	BOOL Create(DWORD dwStyle,const RECT* rect,HWND hParentWnd,UINT nID);
public:

	int GetCount() const;
	int GetHorizontalExtent() const;
	void SetHorizontalExtent(int cxExtent);
	int GetTopIndex() const;
	int SetTopIndex(int nIndex);
	LCID GetLocale() const;
	LCID SetLocale(LCID nNewLocale);
	int InitStorage(int nItems,UINT nBytes);
	UINT ItemFromPoint(CPoint pt,BOOL& bOutside) const;

	int GetCurSel() const;
	int SetCurSel(int nSelect);

	int GetSel(int nIndex) const;
	int SetSel(int nIndex,BOOL bSelect=TRUE);
	int GetSelCount() const;
	int GetSelItems(int nMaxItems,LPINT rgIndex) const;
	void SetAnchorIndex(int nIndex);
	int GetAnchorIndex() const;

	DWORD GetItemData(int nIndex) const;
	int SetItemData(int nIndex,DWORD dwItemData);
	void* GetItemDataPtr(int nIndex) const;
	int SetItemDataPtr(int nIndex,void* pData);
	int GetItemRect(int nIndex,LPRECT lpRect) const;
	int GetText(int nIndex,LPSTR lpszBuffer) const;
	int GetText(int nIndex,CString& rString) const;
	int GetTextLen(int nIndex) const;

	void SetColumnWidth(int cxWidth);
	BOOL SetTabStops(int nTabStops,LPINT rgTabStops);
	void SetTabStops();
	BOOL SetTabStops(const int& cxEachStop);

	int SetItemHeight(int nIndex,UINT cyItemHeight);
	int GetItemHeight(int nIndex) const;
	int FindStringExact(int nIndexStart,LPCSTR lpszFind) const;
	int GetCaretIndex() const;
	int SetCaretIndex(int nIndex,BOOL bScroll=TRUE);

	int AddString(LPCTSTR lpszItem);
	int DeleteString(UINT nIndex);
	int InsertString(int nIndex,LPCSTR lpszItem);
	void ResetContent();
	int Dir(UINT attr,LPCSTR lpszWildCard);

	int FindString(int nStartAfter,LPCSTR lpszItem) const;
	int SelectString(int nStartAfter,LPCSTR lpszItem);
	int SelItemRange(BOOL bSelect,int nFirstItem,int nLastItem);

#ifdef DEF_WCHAR
	int Dir(UINT attr,LPCWSTR lpszWildCard);
	int GetText(int nIndex,LPWSTR lpszBuffer) const;
	int GetText(int nIndex,CStringW& rString) const;
	int AddString(LPCWSTR lpszItem);
	int InsertString(int nIndex,LPCWSTR lpszItem);
	int FindString(int nStartAfter,LPCWSTR lpszItem) const;
	int FindStringExact(int nIndexStart,LPCWSTR lpszFind) const;
	int SelectString(int nStartAfter,LPCWSTR lpszItem);
#endif
};

class CComboBox : public CWndCtrl
{
public:
	CComboBox();
	CComboBox(HWND hWnd);

	BOOL Create(DWORD dwStyle,const RECT* rect,HWND hParentWnd,UINT nID);

	int GetCount() const;
	int GetCurSel() const;
	int SetCurSel(int nSelect);
	LCID GetLocale() const;
	LCID SetLocale(LCID nNewLocale);
	int GetTopIndex() const;
	int SetTopIndex(int nIndex);
	int InitStorage(int nItems, UINT nBytes);
	void SetHorizontalExtent(UINT nExtent);
	UINT GetHorizontalExtent() const;
	int SetDroppedWidth(UINT nWidth);
	int GetDroppedWidth() const;

	DWORD GetEditSel() const;
	BOOL LimitText(int nMaxChars);
	BOOL SetEditSel(int nStartChar, int nEndChar);

	DWORD GetItemData(int nIndex) const;
	int SetItemData(int nIndex, DWORD dwItemData);
	void* GetItemDataPtr(int nIndex) const;
	int SetItemDataPtr(int nIndex, void* pData);
	int GetLBText(int nIndex, LPSTR lpszText) const;
	int GetLBText(int nIndex, CStringA& rString) const;
	int GetLBTextLen(int nIndex) const;

	int SetItemHeight(int nIndex, UINT cyItemHeight);
	int GetItemHeight(int nIndex) const;
	int FindStringExact(int nIndexStart, LPCSTR lpszFind) const;
	int SetExtendedUI(BOOL bExtended = TRUE);
	BOOL GetExtendedUI() const;
	void GetDroppedControlRect(LPRECT lprect) const;
	BOOL GetDroppedState() const;

	void ShowDropDown(BOOL bShowIt = TRUE);

	int AddString(LPCSTR lpszString);
	int DeleteString(UINT nIndex);
	int InsertString(int nIndex, LPCSTR lpszString);
	void ResetContent();
	int Dir(UINT attr, LPCSTR lpszWildCard);

	int FindString(int nStartAfter,LPCSTR lpszString) const;
	int SelectString(int nStartAfter,LPCSTR lpszString);

	void Clear();
	void Copy();
	void Cut();
	void Paste();

#ifdef DEF_WCHAR
	int GetLBText(int nIndex, LPWSTR lpszText) const;
	int GetLBText(int nIndex, CStringW& rString) const;
	int FindStringExact(int nIndexStart, LPCWSTR lpszFind) const;
	int AddString(LPCWSTR lpszString);
	int InsertString(int nIndex, LPCWSTR lpszString);
	int Dir(UINT attr, LPCWSTR lpszWildCard);
	int FindString(int nStartAfter,LPCWSTR lpszString) const;
	int SelectString(int nStartAfter,LPCWSTR lpszString);
#endif
};

class CScrollBar : public CWndCtrl
{
public:
	CScrollBar();
	CScrollBar(HWND hWnd,int nBar=SB_CTL);
	
	BOOL Create(DWORD dwStyle,const RECT* rect,HWND hParentWnd,UINT nID);

	int GetScrollPos() const;
	int SetScrollPos(int nPos,BOOL bRedraw=TRUE);
	int SetScrollPage(UINT nPage,BOOL bRedraw=TRUE);
	void GetScrollRange(LPINT lpMinPos,LPINT lpMaxPos) const;
	void SetScrollRange(int nMinPos,int nMaxPos,BOOL bRedraw=TRUE);
	void ShowScrollBar(BOOL bShow=TRUE);

	BOOL EnableScrollBar(UINT nArrowFlags=ESB_ENABLE_BOTH);

	BOOL SetScrollInfo(LPSCROLLINFO lpScrollInfo,BOOL bRedraw=TRUE);
	BOOL SetScrollInfo(int nMin=0,int nMax=0,UINT nPage=1,int nPos=0,UINT fMask=SIF_ALL,BOOL bRedraw=TRUE);
	BOOL GetScrollInfo(LPSCROLLINFO lpScrollInfo,UINT nMask=SIF_ALL);
	int GetScrollLimit();
protected:
	int m_nBar;
};

class C3DStaticCtrl : public CWndCtrl
{
public:
	C3DStaticCtrl();
	C3DStaticCtrl(HWND hWnd);
	BOOL Create(LPCTSTR lpszCaption,DWORD dwStyle,const RECT* rect,HWND hParentWnd,UINT nID);

public:
};

class C3DButtonCtrl : public CWndCtrl
{
public:
	C3DButtonCtrl();
	C3DButtonCtrl(HWND hWnd);
	BOOL Create(LPCTSTR lpszCaption,DWORD dwStyle,const RECT* rect,HWND hParentWnd,UINT nID);

public:
	BOOL SetBitmap(HBITMAP hBitmap);
	BOOL SetHotBitmap(HBITMAP hBitmap);
	BOOL SetDisabledBitmap(HBITMAP hBitmap);
	HBITMAP GetBitmap() const;
	HBITMAP GetHotBitmap() const;
	HBITMAP GetDisabledBitmap() const;
	BOOL SetColor(DWORD nFlags,COLORREF cr);
	COLORREF GetColor(DWORD nFlags) const;

	BOOL IsPressed() const;
	UINT GetButtonStyle() const;
	void SetButtonStyle(UINT nStyle,BOOL bRedraw=TRUE);
};


#include "Controls.inl"

#endif
#endif
