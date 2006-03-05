////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#ifdef DEF_WINDOWS

///////////////////////////
// Class CWndCtrl
///////////////////////////

BOOL CWndCtrl::ModifyStyle(DWORD dwRemove,DWORD dwAdd,UINT nFlags)
{
	if (!::SetWindowLong(m_hWnd,GWL_STYLE,(::GetWindowLong(m_hWnd,GWL_STYLE)&~dwRemove)|dwAdd))
		return FALSE;
	if (nFlags)
		::SetWindowPos(m_hWnd,HWND_TOP,0,0,0,0,nFlags|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	return TRUE;
}

BOOL CWndCtrl::ModifyStyleEx(DWORD dwRemove,DWORD dwAdd,UINT nFlags)
{
	if (!::SetWindowLong(m_hWnd,GWL_EXSTYLE,(::GetWindowLong(m_hWnd,GWL_EXSTYLE)&~dwRemove)|dwAdd))
		return FALSE;
	if (nFlags)
		::SetWindowPos(m_hWnd,HWND_TOP,0,0,0,0,nFlags|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	return TRUE;
}

int CWndCtrl::GetWindowText(CString& str) const
{
	int len=::GetWindowTextLength(m_hWnd);
	len=::GetWindowText(m_hWnd,str.GetBuffer(len),len+1);
	return len;
}

void CWndCtrl::ClientToScreen(LPRECT lpRect) const
{
	POINT pt={lpRect->left,lpRect->top};
	::ClientToScreen(m_hWnd,&pt);
	lpRect->left=pt.x;
	lpRect->top=pt.y;
	pt.x=lpRect->right;
	pt.y=lpRect->bottom;
	::ClientToScreen(m_hWnd,&pt);
	lpRect->right=pt.x;
	lpRect->bottom=pt.y;
}

void CWndCtrl::ScreenToClient(LPRECT lpRect) const
{
	POINT pt={lpRect->left,lpRect->top};
	::ScreenToClient(m_hWnd,&pt);
	lpRect->left=pt.x;
	lpRect->top=pt.y;
	pt.x=lpRect->right;
	pt.y=lpRect->bottom;
	::ScreenToClient(m_hWnd,&pt);
	lpRect->right=pt.x;
	lpRect->bottom=pt.y;
}

void CWndCtrl::MapWindowPoints(HWND hwndTo, LPRECT lpRect) const
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

UINT CWndCtrl::GetText(CStringA& str)
{
	UINT len=::SendMessage(m_hWnd,WM_GETTEXTLENGTH,0,0);
	len=::SendMessage(m_hWnd,WM_GETTEXT,(WPARAM)len+1,(LPARAM)str.GetBuffer(len));
	return len;
}

#ifdef DEF_WCHAR
UINT CWndCtrl::GetText(CStringW& str)
{
	UINT len=::SendMessage(m_hWnd,WM_GETTEXTLENGTH,0,0)+2;
	LPWSTR text=new WCHAR[len];
	if (text==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return FALSE;
	}
	len=::SendMessage(m_hWnd,WM_GETTEXT,(WPARAM)len,(LPARAM)text);
	str.Copy(text,len);
	delete[] text;
	return len;
}
#endif


///////////////////////////
// Class CComboBox
///////////////////////////

void CComboBox::GetLBText(int nIndex, CStringA& rString) const
{
	LPSTR pointer=new CHAR [::SendMessage(m_hWnd,CB_GETLBTEXTLEN,(WPARAM)nIndex,0)+2];
	if (pointer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return;
	}
	::SendMessage(m_hWnd,CB_GETLBTEXT,(WPARAM)nIndex,(LPARAM)pointer);
	rString=pointer;
	delete[] pointer;
}

#ifdef DEF_WCHAR
void CComboBox::GetLBText(int nIndex, CStringW& rString) const
{
	LPWSTR pointer=new WCHAR [::SendMessage(m_hWnd,CB_GETLBTEXTLEN,(WPARAM)nIndex,0)+2];
	if (pointer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return;
	}
	::SendMessage(m_hWnd,CB_GETLBTEXT,(WPARAM)nIndex,(LPARAM)pointer);
	rString=pointer;
	delete[] pointer;
}
#endif

///////////////////////////
// Class ScrollBar
///////////////////////////

int CScrollBar::GetScrollPos() const
{
	SCROLLINFO lpsi;
	lpsi.cbSize=sizeof(SCROLLINFO);
	lpsi.fMask=SIF_POS;
	lpsi.nPos=0;
	::GetScrollInfo(m_hWnd,m_nBar,&lpsi);
	return lpsi.nPos;
}

int CScrollBar::SetScrollPos(int nPos,BOOL bRedraw)
{
	SCROLLINFO lpsi;
	lpsi.cbSize=sizeof(SCROLLINFO);
	lpsi.fMask=SIF_POS;
	lpsi.nPos=nPos;
	return ::SetScrollInfo(m_hWnd,m_nBar,&lpsi,bRedraw);
}

int CScrollBar::SetScrollPage(UINT nPage,BOOL bRedraw)
{
	SCROLLINFO lpsi;
	lpsi.cbSize=sizeof(SCROLLINFO);
	lpsi.fMask=SIF_PAGE;
	lpsi.nPage=nPage;
	return ::SetScrollInfo(m_hWnd,m_nBar,&lpsi,bRedraw);
}

void CScrollBar::GetScrollRange(LPINT lpMinPos,LPINT lpMaxPos) const
{
	SCROLLINFO lpsi;
	lpsi.cbSize=sizeof(SCROLLINFO);
	lpsi.fMask=SIF_RANGE;
	lpsi.nMin=0;
	lpsi.nMax=0;
	::GetScrollInfo(m_hWnd,m_nBar,&lpsi);
	*lpMinPos=lpsi.nMin;
	*lpMaxPos=lpsi.nMax;
}

void CScrollBar::SetScrollRange(int nMinPos,int nMaxPos,BOOL bRedraw)
{
	SCROLLINFO lpsi;
	lpsi.cbSize=sizeof(SCROLLINFO);
	lpsi.fMask=SIF_RANGE;
	lpsi.nMin=nMinPos;
	lpsi.nMax=nMaxPos;
	::SetScrollInfo(m_hWnd,m_nBar,&lpsi,bRedraw);
}

BOOL CScrollBar::SetScrollInfo(int nMin,int nMax,UINT nPage,int nPos,UINT fMask,BOOL bRedraw)
{
	SCROLLINFO lpsi;
	lpsi.cbSize=sizeof(SCROLLINFO);
	lpsi.fMask=fMask;
	lpsi.nMin=nMin;
	lpsi.nMax=nMax;
	lpsi.nPage=nPage;
	lpsi.nPos=nPos;
	return ::SetScrollInfo(m_hWnd,m_nBar,&lpsi,bRedraw);
}

BOOL CScrollBar::GetScrollInfo(LPSCROLLINFO lpScrollInfo,UINT nMask)
{
	fMemSet(lpScrollInfo,0,sizeof(SCROLLINFO));
	lpScrollInfo->cbSize=sizeof(SCROLLINFO);
	lpScrollInfo->fMask=nMask;
	return ::GetScrollInfo(m_hWnd,m_nBar,lpScrollInfo);
}

int CScrollBar::GetScrollLimit()
{
	SCROLLINFO lpsi;
	lpsi.cbSize=sizeof(SCROLLINFO);
	lpsi.fMask=SIF_RANGE;
	lpsi.nMax=0;
	::GetScrollInfo(m_hWnd,m_nBar,&lpsi);
	return lpsi.nMax;
}

#endif