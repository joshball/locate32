////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#if defined(DEF_WINDOWS) && defined(DEF_RESOURCES)

///////////////////////////
// Class CWnd
///////////////////////////

/* virtual */BOOL CWnd::CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName,
		LPCTSTR lpszWindowName, DWORD dwStyle,
		int x, int y, int nWidth, int nHeight,
		HWND hWndParent, HMENU nIDorHMenu, LPVOID lpParam)
{
	return (m_hWnd=CreateWindowEx(dwExStyle,lpszClassName,
		lpszWindowName,dwStyle,x,y,nWidth,nHeight,
		hWndParent,nIDorHMenu,GetInstanceHandle(),lpParam))!=NULL;
}

BOOL CWnd::ModifyStyle(DWORD dwRemove,DWORD dwAdd,UINT nFlags)
{
	if (!::SetWindowLong(m_hWnd,GWL_STYLE,(::GetWindowLong(m_hWnd,GWL_STYLE)&~dwRemove)|dwAdd))
		return FALSE;
	if (nFlags)
		::SetWindowPos(m_hWnd,HWND_TOP,0,0,0,0,nFlags|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	return TRUE;
}
	
BOOL CWnd::ModifyStyleEx(DWORD dwRemove,DWORD dwAdd,UINT nFlags)
{
	if (!::SetWindowLong(m_hWnd,GWL_EXSTYLE,(::GetWindowLong(m_hWnd,GWL_EXSTYLE)&~dwRemove)|dwAdd))
		return FALSE;
	if (nFlags)
		::SetWindowPos(m_hWnd,HWND_TOP,0,0,0,0,nFlags|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	return TRUE;
}

void CWnd::MapWindowPoints(HWND hwndTo, LPRECT lpRect) const
{
	POINT pt={lpRect->left,lpRect->top};
	::MapWindowPoints(m_hWnd,hwndTo,&pt,1);
	lpRect->left=pt.x;
	lpRect->top=pt.y;
	pt.x=lpRect->right;
	pt.y=lpRect->bottom;
	::MapWindowPoints(m_hWnd,hwndTo,&pt,1);
	lpRect->right=pt.x;
	lpRect->bottom=pt.y;
}
	
int CWnd::GetCheckedRadioButton(int nIDFirstButton,int nIDLastButton)
{
	for (int i=nIDFirstButton;i<=nIDLastButton;i++)
	{
		if (::IsDlgButtonChecked(m_hWnd,i))
			return i;
	}
	return 0;
}
	
UINT CWnd::GetDlgItemText(int nIDDlgItem,CStringA& str) const
{
	HWND hCtrl=::GetDlgItem(m_hWnd,nIDDlgItem);
	UINT len=(UINT)::SendMessage(hCtrl,WM_GETTEXTLENGTH,0,0);
	::SendMessage(hCtrl,WM_GETTEXT,len+1,(LPARAM)str.GetBuffer(len));
	return len;
}

UINT CWnd::GetText(LPTSTR lpszText,UINT cchTextMax)
{
	return ::SendMessage(m_hWnd,WM_GETTEXT,(WPARAM)cchTextMax,(LPARAM)lpszText);
}

UINT CWnd::GetText(CStringA& str)
{
	UINT len=::SendMessage(m_hWnd,WM_GETTEXTLENGTH,0,0);
	len=::SendMessage(m_hWnd,WM_GETTEXT,(WPARAM)len+1,(LPARAM)str.GetBuffer(len));
	return len;
}

BOOL CWnd::SetText(LPCSTR lpsz)
{
	return ::SendMessage(m_hWnd,WM_SETTEXT,0,(LPARAM)lpsz);
}

BOOL CWnd::SetText(LPCWSTR lpsz)
{
	return ::SendMessage(m_hWnd,WM_SETTEXT,0,(LPARAM)lpsz);
}

void CWnd::CenterWindow()
{
	SIZE sRes = {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
	RECT rect;
	::GetWindowRect(m_hWnd,&rect);
	::SetWindowPos(m_hWnd,HWND_TOP,(sRes.cx-(rect.right-rect.left))/2,(sRes.cy-(rect.bottom-rect.top))/2,1,1,SWP_NOSIZE|SWP_NOZORDER);
}

#ifdef DEF_RESOURCES
int CWnd::ShowErrorMessage(UINT nIDMsgStr,UINT nIDTitleStr,UINT uType) const
{
	char title[100];
	char msg[1000];
	LoadString(nIDMsgStr,msg,1000);
	if (nIDTitleStr)
		LoadString(nIDTitleStr,title,100);
	else
		StringCbCopy(title,100,szError);
	return ::MessageBox(m_hWnd,msg,title,uType);
}
#endif


//Message Handlers

void CWnd::OnActivate(WORD fActive,BOOL fMinimized,HWND hwnd)
{
	WindowProc(WM_ACTIVATE,MAKEWPARAM(fActive,fMinimized),(LPARAM)hwnd);
}

void CWnd::OnActivateApp(BOOL fActive,DWORD dwThreadID)
{
	WindowProc(WM_ACTIVATEAPP,(WPARAM)fActive,(LPARAM)dwThreadID);
}

void CWnd::OnCancelMode()
{
	WindowProc(WM_CANCELMODE,0,0);
}

void CWnd::OnCaptureChanged(HWND hwndNewCapture)
{
	WindowProc(WM_CAPTURECHANGED,0,(LPARAM)hwndNewCapture);
}

void CWnd::OnChangeCbChain(HWND hWndRemove,HWND hWndAfter)
{
	WindowProc(WM_CHANGECBCHAIN,(WPARAM)hWndRemove,(LPARAM)hWndAfter);
}

void CWnd::OnChar(TCHAR chCharCode,DWORD lKeyData)
{
	WindowProc(WM_CHAR,(WPARAM)chCharCode,(LPARAM)lKeyData);
}

BOOL CWnd::OnClose()
{
	return WindowProc(WM_CLOSE,0,0);
}

BOOL CWnd::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	return WindowProc(WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),(LPARAM)hControl);
}

void CWnd::OnContextMenu(HWND hWnd,CPoint& pos)
{
	WindowProc(WM_CONTEXTMENU,(WPARAM)hWnd,MAKELPARAM(pos.x,pos.y));
}

int CWnd::OnCreate(LPCREATESTRUCT lpcs)
{
	return WindowProc(WM_CREATE,0,(LPARAM)lpcs);
}
	
	
void CWnd::OnDestroy()
{
	WindowProc(WM_DESTROY,0,0);
}

void CWnd::OnDestroyClipboard()
{
	WindowProc(WM_DESTROYCLIPBOARD,0,0);
}

void CWnd::OnDrawClipboard()
{
	WindowProc(WM_DRAWCLIPBOARD,0,0);
}

void CWnd::OnDrawItem(UINT idCtl,LPDRAWITEMSTRUCT lpdis)
{
	WindowProc(WM_DRAWITEM,idCtl,(LPARAM)lpdis);
}	

void CWnd::OnDropFiles(HDROP hDrop)
{
	WindowProc(WM_DROPFILES,(WPARAM)hDrop,0);
}

void CWnd::OnHelp(LPHELPINFO lphi)
{
	WindowProc(WM_HELP,0,(LPARAM)lphi);
}

void CWnd::OnHScroll(UINT nSBCode,UINT nPos,HWND hScrollBar)
{
	WindowProc(WM_HSCROLL,MAKEWPARAM(nSBCode,nPos),(LPARAM)hScrollBar);
}

void CWnd::OnInitMenu(HMENU hMenu)
{
	WindowProc(WM_INITMENU,(WPARAM)hMenu,0);
}

void CWnd::OnInitMenuPopup(HMENU hPopupMenu,UINT nIndex,BOOL bSysMenu)
{
	WindowProc(WM_INITMENUPOPUP,(WPARAM)hPopupMenu,MAKELPARAM(nIndex,bSysMenu));
}
	
void CWnd::OnKeyDown(int nVirtKey,LONG lKeyData)
{
	WindowProc(WM_KEYDOWN,(WPARAM)nVirtKey,(LPARAM)lKeyData);
}

void CWnd::OnKeyUp(int nVirtKey,LONG lKeyData)
{
	WindowProc(WM_KEYUP,(WPARAM)nVirtKey,(LPARAM)lKeyData);
}

void CWnd::OnKillFocus(HWND hwndGetFocus)
{
	WindowProc(WM_KILLFOCUS,(WPARAM)hwndGetFocus,0);
}

void CWnd::OnMeasureItem(int nIDCtl,LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	WindowProc(WM_MEASUREITEM,nIDCtl,(LPARAM)lpMeasureItemStruct);
}

LRESULT CWnd::OnMenuChar(UINT nChar,UINT nFlags,HMENU hMenu)
{
	return WindowProc(WM_MENUCHAR,(WPARAM)MAKELONG(nChar,nFlags),(LPARAM)hMenu);
}

void CWnd::OnMenuSelect(UINT uItem,UINT fuFlags,HMENU hmenu)
{
	WindowProc(WM_MENUSELECT,MAKEWPARAM(uItem,fuFlags),(LPARAM)hmenu);
}
	
int CWnd::OnMouseActivate(HWND hDesktopWnd,UINT nHitTest,UINT message)
{
	return WindowProc(WM_MOUSEACTIVATE,(WPARAM)hDesktopWnd,MAKELPARAM(nHitTest,message));
}

void CWnd::OnMouseMove(UINT fwKeys,WORD xPos,WORD yPos)
{
	WindowProc(WM_MOUSEMOVE,(WPARAM)fwKeys,(LPARAM)MAKELONG(xPos,yPos));
}

void CWnd::OnMove(int x, int y)
{
	WindowProc(WM_MOVE,0,(LPARAM)MAKELONG(x,y));
}

BOOL CWnd::OnNcActivate(BOOL bActive)
{
	return WindowProc(WM_NCACTIVATE,(WPARAM)bActive,0);
}
	
BOOL CWnd::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	return WindowProc(WM_NCCREATE,0,(LPARAM)lpCreateStruct);
}

void CWnd::OnNcDestroy()
{
	WindowProc(WM_NCDESTROY,0,0);
	
	// For WaitForWindow()
	CString name;
	name.SetBase(32);
	name << "WFWE" << (DWORD)this;
	HANDLE hEvent=OpenEvent(EVENT_MODIFY_STATE,FALSE,name);
	SetEvent(hEvent);
	CloseHandle(hEvent);

	// No more messages to this class
	::SetWindowLong(m_hWnd,GWL_USERDATA,0);
}
	
BOOL CWnd::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	return WindowProc(WM_NOTIFY,idCtrl,(LPARAM)pnmh);
}

void CWnd::OnPaint()
{
	WindowProc(WM_PAINT,0,0);
}

void CWnd::OnPaintClipboard(HWND hClipAppWnd,HGLOBAL hPaintStruct)
{
	WindowProc(WM_PAINTCLIPBOARD,(WPARAM)hClipAppWnd,(LPARAM)hPaintStruct);
}

void CWnd::OnSize(UINT nType, int cx, int cy)
{
	WindowProc(WM_SIZE,(WPARAM)nType,(LPARAM)MAKELONG(cx,cy));
}

BOOL CWnd::OnSizing(UINT nSide,LPRECT lpRect)
{
	return WindowProc(WM_SIZING,(WPARAM)nSide,(LPARAM)lpRect);
}

void CWnd::OnSysCommand(UINT nID,LPARAM lParam)
{
	WindowProc(WM_SYSCOMMAND,(WPARAM)nID,lParam);
}
	
void CWnd::OnTimer(DWORD wTimerID)
{
	WindowProc(WM_TIMER,wTimerID,NULL);
}

void CWnd::OnVScroll(UINT nSBCode,UINT nPos,HWND hScrollBar)
{
	WindowProc(WM_VSCROLL,MAKEWPARAM(nSBCode,nPos),(LPARAM)hScrollBar);
}

void CWnd::OnWindowPosChanged(LPWINDOWPOS lpWndPos)
{
	WindowProc(WM_WINDOWPOSCHANGED,0,(LPARAM)lpWndPos);
}

void CWnd::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{
	WindowProc(WM_WINDOWPOSCHANGING,0,(LPARAM)lpWndPos);
}

BOOL CWnd::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
#ifdef _DEBUG
	void DebugCommandsProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	DebugCommandsProc(*this,msg,wParam,lParam);
#endif
	return DefWindowProc(m_hWnd,msg,wParam,lParam);
}

#ifdef DEF_WCHAR
int CWnd::GetWindowText(CStringW& str) const
{
	int len=::GetWindowTextLengthW(m_hWnd);
	len=::GetWindowTextW(m_hWnd,str.GetBuffer(len),len+1);
	return len;
}

UINT CWnd::GetDlgItemText(int nIDDlgItem,CStringW& str)
{
	UINT len=::SendMessage(m_hWnd,WM_GETTEXTLENGTH,0,0);
	len=::GetDlgItemTextW(m_hWnd,nIDDlgItem,str.GetBuffer(len),len+1);
	str.FreeExtra();
	return len;
}

UINT CWnd::GetText(CStringW& str)
{
	UINT len=::SendMessage(m_hWnd,WM_GETTEXTLENGTH,0,0);
	LPWSTR text=new WCHAR[len];
	if (text==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return FALSE;
	}
	len=::SendMessage(m_hWnd,WM_GETTEXT,(WPARAM)len,(LPARAM)text);
	str=text;
	delete[] text;
	return len;
}
#endif

BOOL CWnd::LoadPosition(HKEY hRootKey,LPCSTR lpKey,LPCSTR lpSubKey,DWORD fFlags)
{
	CRegKey RegKey;
	if (lpKey==NULL)
		RegKey.m_hKey=hRootKey;
	else if (RegKey.OpenKey(hRootKey,lpKey,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return FALSE;
	
	WINDOWPLACEMENT wp;
    DWORD nRet=RegKey.QueryValue(lpSubKey,LPSTR(&wp),sizeof(WINDOWPLACEMENT));
	if (nRet==sizeof(WINDOWPLACEMENT) && wp.length==sizeof(WINDOWPLACEMENT))
	{
		switch (wp.showCmd)
		{
		case SW_MAXIMIZE:
			if (fFlags&fgOnlySpecifiedPosition)
			{
				if (fFlags&fgAllowMaximized)
					ShowWindow(swMaximize);
				if (lpKey==NULL)
					RegKey.m_hKey=NULL;
                return TRUE;
			}
			SetWindowPos(HWND_TOP,wp.rcNormalPosition.left,wp.rcNormalPosition.top,
				wp.rcNormalPosition.right-wp.rcNormalPosition.left,wp.rcNormalPosition.bottom-wp.rcNormalPosition.top,
				SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOREDRAW);
			if (fFlags&fgOnlyNormalPosition)
			{
				if (lpKey==NULL)
					RegKey.m_hKey=NULL;
                return TRUE;
			}
			if (!(fFlags&fgAllowMaximized))
				wp.showCmd=SW_SHOWNORMAL;
			break;
		case SW_MINIMIZE:
			if (fFlags&fgOnlySpecifiedPosition)
			{
				if (fFlags&fgAllowMinimized)
					ShowWindow(swMinimize);
				if (lpKey==NULL)
					RegKey.m_hKey=NULL;
                return TRUE;
			}
			SetWindowPos(HWND_TOP,wp.rcNormalPosition.left,wp.rcNormalPosition.top,
				wp.rcNormalPosition.right-wp.rcNormalPosition.left,wp.rcNormalPosition.bottom-wp.rcNormalPosition.top,
				SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOREDRAW);
			if (fFlags&fgOnlyNormalPosition)
			{
				if (lpKey==NULL)
					RegKey.m_hKey=NULL;
                return TRUE;
			}
			if (!(fFlags&fgAllowMinimized))
				wp.showCmd=SW_SHOWNORMAL;
			break;
		case SW_HIDE:
			if (fFlags&fgOnlySpecifiedPosition)
			{
				if (fFlags&fgAllowHide)
					ShowWindow(swHide);
				if (lpKey==NULL)
					RegKey.m_hKey=NULL;
                return TRUE;
			}
			SetWindowPos(HWND_TOP,wp.rcNormalPosition.left,wp.rcNormalPosition.top,
				wp.rcNormalPosition.right-wp.rcNormalPosition.left,wp.rcNormalPosition.bottom-wp.rcNormalPosition.top,
				SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOREDRAW);
			if (fFlags&fgOnlyNormalPosition)
			{
				if (lpKey==NULL)
					RegKey.m_hKey=NULL;
                return TRUE;
			}
			if (!(fFlags&fgAllowHide))
				wp.showCmd=SW_SHOWNORMAL;
			break;
		default:
			wp.showCmd=SW_SHOWNORMAL;
			break;
		}
	}	
	else
	{
		if (lpKey==NULL)
			RegKey.m_hKey=NULL;
		return FALSE;
	}
	SetWindowPlacement(&wp);
	if (lpKey==NULL)
		RegKey.m_hKey=NULL;
	return TRUE;
}

BOOL CWnd::SavePosition(HKEY hRootKey,LPCSTR lpKey,LPCSTR lpSubKey) const
{
	CRegKey RegKey;
	if (lpKey==NULL)
		RegKey.m_hKey=hRootKey;
	else if (RegKey.OpenKey(hRootKey,lpKey,CRegKey::createNew|CRegKey::samAll)!=ERROR_SUCCESS)
		return FALSE;
	
	WINDOWPLACEMENT wp;
	wp.length=sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wp);
	
	BOOL bRet=RegKey.SetValue(lpSubKey,LPCSTR(&wp),sizeof(WINDOWPLACEMENT),REG_BINARY)==ERROR_SUCCESS;

	if (lpKey==NULL)
		RegKey.m_hKey=NULL;
	return bRet;
}

///////////////////////////
// Class CFrameDlg
///////////////////////////

BOOL CFrameWnd::Create(LPCTSTR lpszClassName,LPCTSTR lpszWindowName,DWORD dwStyle,
				const RECT* rect,HWND hParentWnd,LPCTSTR lpszMenuName,DWORD dwExStyle)
{
	int x,y,nWidth,nHeight;
	if (rect==NULL)
	{
		if (!(dwStyle&WS_OVERLAPPED))
		{
			x=CW_USEDEFAULT;
			y=CW_USEDEFAULT;
			nWidth=CW_USEDEFAULT;
			nHeight=CW_USEDEFAULT;
		}
		else
		{
			x=0;
			y=0;
			nWidth=0;
			nHeight=0;
		}
	}
	else
	{
		x=rect->left;
		y=rect->top;
		nWidth=rect->right-rect->left;
		nHeight=rect->bottom-rect->top;
	}
	if(lpszMenuName!=NULL)
		m_hMenu=LoadMenu(lpszMenuName);
	
	m_hWnd=CreateWindowEx(dwExStyle,lpszClassName,lpszWindowName,dwStyle,
		x,y,nWidth,nHeight,hParentWnd,m_hMenu,GetInstanceHandle(),(void*)this);
	if (m_hWnd==NULL)
		return FALSE;
	if (dwExStyle&WS_VISIBLE)
		::ShowWindow(m_hWnd,SW_SHOWDEFAULT);
	::UpdateWindow(m_hWnd);
	return TRUE;
}

int CFrameWnd::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CWnd::OnCreate(lpcs)==-1)
		return -1;
	AddDebugMenuItems(*this);
	if (OnCreateClient(lpcs))
		return 0;
	return -1;
}

BOOL CFrameWnd::OnCreateClient(LPCREATESTRUCT lpcs)
{
	return TRUE;
}
	
BOOL CFrameWnd::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	return CWnd::WindowProc(msg,wParam,lParam);
}

///////////////////////////
// Class CMDIFrameDlg
///////////////////////////

HWND CMDIFrameWnd::MDIGetActive(BOOL* pbMaximized) const
{
	if (m_hWndMDIClient==NULL)
	{
		if (pbMaximized!=NULL)
			*pbMaximized=FALSE;
		return NULL;
	}

	HWND hWnd=(HWND)::SendMessage(m_hWndMDIClient,WM_MDIGETACTIVE,0,(LPARAM)pbMaximized);
	
	if ((::GetWindowLong(hWnd,GWL_STYLE)&WS_VISIBLE)==0)
	{
		hWnd=NULL;
		if (pbMaximized!=NULL)
			*pbMaximized=FALSE;
	}
	return hWnd;
}

BOOL CMDIFrameWnd::CreateClient(LPCREATESTRUCT lpCreateStruct,HMENU hWindowMenu)
{
	DWORD dwStyle=WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|MDIS_ALLCHILDSTYLES;
	DWORD dwExStyle=WS_EX_CLIENTEDGE;
	
	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu=hWindowMenu;
	ccs.idFirstChild=IDM_FIRST_MDICHILD;

	if (lpCreateStruct->style&(WS_HSCROLL|WS_VSCROLL))
	{
		dwStyle|=(lpCreateStruct->style&(WS_HSCROLL|WS_VSCROLL));
		ModifyStyle(WS_HSCROLL|WS_VSCROLL,0,SWP_NOREDRAW|SWP_FRAMECHANGED);
	}

	if ((m_hWndMDIClient=::CreateWindowEx(dwExStyle,"MDICLIENT",NULL,dwStyle,0,0,0,0,m_hWnd,(HMENU)IDW_PANE_FIRST,
		GetInstanceHandle(),(LPVOID)&ccs))==NULL)
	{
		return FALSE;
	}
	
	::BringWindowToTop(m_hWndMDIClient);
	return TRUE;
}

HMENU CMDIFrameWnd::GetWindowMenuPopup(HMENU hMenuBar)
{	
	if (hMenuBar==NULL)
		return NULL;

	int iItem=::GetMenuItemCount(hMenuBar);
	while (iItem--)
	{
		HMENU hMenuPop=::GetSubMenu(hMenuBar,iItem);
		if (hMenuPop!= NULL)
		{
			int iItemMax=::GetMenuItemCount(hMenuPop);
			for (int iItemPop=0;iItemPop<iItemMax;iItemPop++)
			{
				UINT nID=GetMenuItemID(hMenuPop,iItemPop);
				if (nID>=IDM_WINDOW_FIRST && nID<=IDM_WINDOW_LAST)
					return hMenuPop;
			}
		}
	}
	return NULL;
}

BOOL CMDIFrameWnd::OnCreateClient(LPCREATESTRUCT lpcs)
{
	CFrameWnd::OnCreateClient(lpcs);
	HMENU hMenu=GetMenu();
	int iMenu=::GetMenuItemCount(hMenu)-2;
	hMenu=::GetSubMenu(hMenu,iMenu);
	return CreateClient(lpcs,hMenu);
}

HWND CMDIFrameWnd::GetActiveFrame()
{
	HWND hActiveChild=MDIGetActive();
	if (hActiveChild==NULL)
		return m_hWnd;
	return hActiveChild;
}

BOOL CMDIFrameWnd::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	if (msg==WM_NCACTIVATE)
	{
		HWND hActiveWnd=MDIGetActive();
		if (hActiveWnd!=NULL)
			::SendMessage(hActiveWnd,WM_NCACTIVATE,wParam,lParam);
	}
	return CFrameWnd::WindowProc(msg,wParam,lParam);
}

BOOL CMDIFrameWnd::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	HWND hActiveChild=MDIGetActive();

	if (hActiveChild!=NULL)
	{
		if (::SendMessage(hActiveChild,WM_COMMAND,(WPARAM)MAKELONG(wID,wNotifyCode),(LPARAM)hControl))
			return TRUE;
	}
	
	if (CFrameWnd::OnCommand(wID,wNotifyCode,hControl))
		return TRUE;

	if (hControl==NULL && (wID&0xf000)==0xf000)
	{
		::DefFrameProc(m_hWnd,m_hWndMDIClient,WM_COMMAND,(WPARAM)MAKELONG(wID,wNotifyCode),(LPARAM)hControl);
		return TRUE;
	}
	return FALSE;
}


void CMDIFrameWnd::OnWindowNew()
{
}

///////////////////////////
// Class CMDIChildWnd
///////////////////////////

BOOL CMDIChildWnd::Create(LPCTSTR lpszClassName,LPCTSTR lpszWindowName,DWORD dwStyle,const RECT* rect,CMDIFrameWnd* pParentWnd)
{
	if (pParentWnd==NULL)
		pParentWnd=(CMDIFrameWnd*)GetApp()->m_pMainWnd;
	m_pParent=pParentWnd;
	MDICREATESTRUCT mcs;
	mcs.szClass=lpszClassName;
	mcs.szTitle=lpszWindowName;
	mcs.hOwner=GetInstanceHandle();
	if (rect!=NULL)
	{
		mcs.x=rect->left;
		mcs.y=rect->top;
		mcs.cx=rect->right-rect->left;
		mcs.cy=rect->bottom-rect->top;
	}
	else
	{
		mcs.x=CW_USEDEFAULT;
		mcs.y=CW_USEDEFAULT;
		mcs.cx=CW_USEDEFAULT;
		mcs.cy=CW_USEDEFAULT;
	}
	mcs.style=dwStyle&~(WS_MAXIMIZE|WS_VISIBLE);
	mcs.lParam=(LPARAM)this;

	HWND m_hWnd=(HWND)::SendMessage(pParentWnd->m_hWndMDIClient,WM_MDICREATE,0,(LPARAM)&mcs);
	
	if (m_hWnd==NULL)
		return FALSE;

	if (dwStyle&WS_VISIBLE)
	{
		::BringWindowToTop(m_hWnd);

		if (dwStyle&WS_MINIMIZE)
			ShowWindow(swShowMinimized);
		else if (dwStyle&WS_MAXIMIZE)
			ShowWindow(swShowMaximized);
		else
			ShowWindow(swShowNormal);

		pParentWnd->MDIActivate(m_hWnd);
		::SendMessage(pParentWnd->m_hWndMDIClient,WM_MDIREFRESHMENU,0,0);
	}
	return TRUE;
}

BOOL CMDIChildWnd::DestroyWindow()
{
	if (m_hWnd==NULL)
		return FALSE;

	HWND hWndFrame=*GetMDIFrame();
	DWORD dwStyle=::SetWindowLong(hWndFrame,GWL_STYLE,::GetWindowLong(hWndFrame,GWL_STYLE));

	MDIDestroy();

	if (::IsWindow(hWndFrame))
		::SetWindowLong(hWndFrame,GWL_STYLE,dwStyle);
	return TRUE;
}

void CMDIChildWnd::ActivateFrame(int nCmdShow)
{
	BOOL bVisibleThen=(GetStyle()&WS_VISIBLE)!=0;
	
	if (nCmdShow == -1)
	{
		BOOL bMaximized;
		m_pParent->MDIGetActive(&bMaximized);

		DWORD dwStyle=GetStyle();
		if (bMaximized || (dwStyle&WS_MAXIMIZE))
			nCmdShow=SW_SHOWMAXIMIZED;
		else if (dwStyle&WS_MINIMIZE)
			nCmdShow=SW_SHOWMINIMIZED;
	}

	CFrameWnd::ActivateFrame(nCmdShow);

	::SendMessage(m_pParent->m_hWndMDIClient,WM_MDIREFRESHMENU,0,0);

	BOOL bVisibleNow=(GetStyle()&WS_VISIBLE)!=0;
	if (bVisibleNow==bVisibleThen)
		return;

	if (!bVisibleNow)
	{
		HWND hWnd=(HWND)::SendMessage(m_pParent->m_hWndMDIClient,WM_MDIGETACTIVE,0,0);
		if (hWnd!=m_hWnd)
			return;
		
		m_pParent->MDINext();

		hWnd=(HWND)::SendMessage(m_pParent->m_hWndMDIClient,WM_MDIGETACTIVE,0,0);
		if (hWnd==m_hWnd)
			OnMDIActivate(NULL,m_hWnd);
	}
	else
		OnMDIActivate(m_hWnd,NULL);
}

BOOL CMDIChildWnd::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	return ::DefMDIChildProc(m_hWnd,msg,wParam,lParam);
}

BOOL CMDIChildWnd::UpdateClientEdge(LPRECT lpRect)
{
	CMDIFrameWnd* pFrameWnd=GetMDIFrame();
	HWND hChild=pFrameWnd->MDIGetActive();
	if (hChild==NULL || hChild==m_hWnd)
	{
		DWORD dwStyle=::GetWindowLong(pFrameWnd->m_hWndMDIClient,GWL_EXSTYLE);
		DWORD dwNewStyle=dwStyle;
		if (hChild!=NULL && !(GetExStyle()&WS_EX_CLIENTEDGE) && (GetStyle()&WS_MAXIMIZE))
			dwNewStyle&=~(WS_EX_CLIENTEDGE);
		else
			dwNewStyle|=WS_EX_CLIENTEDGE;

		if (dwStyle!=dwNewStyle)
		{
			::RedrawWindow(pFrameWnd->m_hWndMDIClient,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);
			::SetWindowLong(pFrameWnd->m_hWndMDIClient,GWL_EXSTYLE,dwNewStyle);
			::SetWindowPos(pFrameWnd->m_hWndMDIClient,NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOCOPYBITS);
			if (lpRect!=NULL)
				::GetClientRect(pFrameWnd->m_hWndMDIClient,lpRect);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CMDIChildWnd::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	return CWnd::OnCommand(wID,wNotifyCode,hControl);
}

void CMDIChildWnd::OnWindowNew()
{
}

LRESULT CMDIChildWnd::OnMenuChar(UINT nChar,UINT nFlags,HMENU hMenu)
{
	return CWnd::OnMenuChar(nChar,nFlags,hMenu);
}

BOOL CMDIChildWnd::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	return CWnd::OnNcCreate(lpCreateStruct);
}

int CMDIChildWnd::OnCreate(LPCREATESTRUCT lpcs)
{
	return CFrameWnd::OnCreate(lpcs);	
}

void CMDIChildWnd::OnMDIActivate(HWND hAct,HWND hDeAct)
{
	UpdateClientEdge();
	WindowProc(WM_MDIACTIVATE,WPARAM(hAct),LPARAM(hDeAct));
}

int CMDIChildWnd::OnMouseActivate(HWND hDesktopWnd,UINT nHitTest,UINT message)
{
	int nResult=CFrameWnd::OnMouseActivate(hDesktopWnd,nHitTest,message);
	if (nResult==MA_NOACTIVATE || nResult==MA_NOACTIVATEANDEAT)
		return nResult;

	if (m_pParent->MDIGetActive()!=*this)
		MDIActivate();
	return nResult;
}

void CMDIChildWnd::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{
	if (!(lpWndPos->flags&SWP_NOSIZE))
	{
		RECT rectClient;
		if (UpdateClientEdge(&rectClient)&&(GetStyle()&WS_MAXIMIZE))
		{
			::AdjustWindowRectEx(&rectClient,GetStyle(),FALSE,GetExStyle());
			lpWndPos->x=rectClient.left;
			lpWndPos->y=rectClient.top;
			lpWndPos->cx=rectClient.right-rectClient.left;
			lpWndPos->cy=rectClient.bottom-rectClient.top;
		}
	}
	CWnd::OnWindowPosChanging(lpWndPos);
}

BOOL CMDIChildWnd::OnNcActivate(BOOL bActive)
{
	return CWnd::OnNcActivate(bActive);
}

void CMDIChildWnd::OnDestroy()
{
	UpdateClientEdge();
	CWnd::OnDestroy();
}

///////////////////////////
// Class CMDIChildWndList
///////////////////////////

POSITION CMDIChildWndList::AddWindow(CMDIChildWnd* pWnd)
{
	CNode* tmp=new CNode;
	if (tmp==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return NULL;
	}
	tmp->pWnd=pWnd;
	tmp->pPrev=NULL;
	tmp->pNext=m_pNodeHead;
	if (m_pNodeHead!=NULL)
		m_pNodeHead->pPrev=tmp;
	else
		m_pNodeTail=tmp;
	m_pNodeHead=tmp;
	m_nCount++;
	return (POSITION) tmp;
}

BYTE CMDIChildWndList::RemoveWindow(POSITION pPos)
{
	CNode* node=(CNode*)pPos;
	if (node->pNext!=NULL)
	{
		if (node->pPrev!=NULL)
		{
			node->pPrev->pNext=node->pNext;
			node->pNext->pPrev=node->pPrev;
		}
		else
		{
			m_pNodeHead=node->pNext;	
			node->pNext->pPrev=NULL;
		}
	}
	else
	{
		if (node->pPrev!=NULL)
			node->pPrev->pNext=NULL;
		else
		{
			m_pNodeHead=NULL;
			m_pNodeTail=NULL;
		}
	}
	m_nCount--;
	delete node->pWnd;
	delete node;
	return TRUE;
}

BYTE CMDIChildWndList::RemoveAllWindows()
{
	CNode* tmp;
	while(m_pNodeHead!=NULL)
	{
		tmp=m_pNodeHead;
		delete tmp->pWnd;
		m_pNodeHead=tmp->pNext;
		delete tmp;
	}
	m_nCount=0;
	m_pNodeTail=NULL;
	return TRUE;
}

POSITION CMDIChildWndList::FindWindow(CMDIChildWnd* pWnd) const
{
	CNode* tmp=m_pNodeHead;
	while (tmp!=NULL)
	{
		if (tmp->pWnd==pWnd)
			return (POSITION) tmp;
		tmp=tmp->pNext;
	}
	return NULL;
}

POSITION CMDIChildWndList::FindWindow(HWND hWnd) const
{
	CNode* tmp=m_pNodeHead;
	while (tmp!=NULL)
	{
		if (tmp->pWnd->GetHandle()==hWnd)
			return (POSITION) tmp;
		tmp=tmp->pNext;
	}
	return NULL;
}

#endif