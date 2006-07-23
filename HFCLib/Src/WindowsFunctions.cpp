////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#ifdef DEF_WINDOWS

LRESULT CALLBACK CAppData::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CWnd* Wnd;
#ifdef DEF_RESOURCES
	if (msg==WM_INITDIALOG)
	{
		Wnd=(CWnd*)lParam;
		Wnd->SetHandle(hWnd);
		SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)Wnd);
		return ((CDialog*)Wnd)->OnInitDialog((HWND)wParam);
	} else 
#endif
	if (msg==WM_NCCREATE)
	{
		Wnd=(CWnd*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		Wnd->SetHandle(hWnd);
		SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)Wnd);
		return Wnd->OnNcCreate((LPCREATESTRUCT)lParam);
	}

	Wnd=(CWnd*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if (Wnd!=NULL)
	{
		switch(msg)
		{
		case WM_ACTIVATE:
			Wnd->OnActivate(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);
			return FALSE;
		case WM_ACTIVATEAPP:
			Wnd->OnActivateApp((BOOL) wParam,(DWORD) lParam); 
			return FALSE;
		case WM_CANCELMODE:
			Wnd->OnCancelMode();
			return FALSE;
		case WM_CAPTURECHANGED:
			Wnd->OnCaptureChanged((HWND) lParam);
			return FALSE;
		case WM_CHANGECBCHAIN:
			Wnd->OnChangeCbChain((HWND)wParam,(HWND)lParam);
			return FALSE;
		case WM_CHAR:
			Wnd->OnChar((TCHAR) wParam,(DWORD)lParam);
			return FALSE;
		case WM_CLOSE:
			return Wnd->OnClose();
		case WM_CREATE:
			return Wnd->OnCreate((LPCREATESTRUCT)lParam);
		case WM_COMMAND:
			return Wnd->OnCommand(LOWORD(wParam),HIWORD(wParam),(HWND)lParam);
		case WM_CONTEXTMENU:
			Wnd->OnContextMenu((HWND)wParam,CPoint(LOWORD(lParam),HIWORD(lParam)));
			return 0;
		case WM_DESTROY:
			Wnd->OnDestroy();
			return FALSE;
		case WM_DESTROYCLIPBOARD:
			Wnd->OnDestroyClipboard();
			return FALSE;
		case WM_DRAWCLIPBOARD:
			Wnd->OnDrawClipboard();
			return FALSE;
		case WM_DRAWITEM:
			Wnd->OnDrawItem((UINT) wParam,(LPDRAWITEMSTRUCT) lParam);
			return FALSE;
		case WM_DROPFILES:
			Wnd->OnDropFiles((HDROP) wParam);
			return FALSE;
		case WM_GETICON:
			return DefWindowProc(hWnd,msg,wParam,lParam);
		case WM_INITMENU:
			Wnd->OnInitMenu((HMENU)wParam);
			return FALSE;
		case WM_INITMENUPOPUP:
			Wnd->OnInitMenuPopup((HMENU)wParam,LOWORD(lParam),HIWORD(lParam));
			return FALSE;
		case WM_HELP:
			Wnd->OnHelp((LPHELPINFO)lParam);
			return TRUE;
		case WM_HSCROLL:
			Wnd->OnHScroll((UINT)LOWORD(wParam),(UINT)HIWORD(wParam),(HWND)lParam);
			return FALSE;
		case WM_KEYDOWN:
			Wnd->OnKeyDown((int) wParam,(LONG)lParam);
			return FALSE;
		case WM_KEYUP:
			Wnd->OnKeyUp((int) wParam,(LONG)lParam);
			return FALSE;
		case WM_KILLFOCUS:
			Wnd->OnKillFocus((HWND) wParam);
			return FALSE;
#ifdef DEF_RESOURCES
		case WM_MDIACTIVATE:
			((CMDIChildWnd*)Wnd)->OnMDIActivate((HWND)lParam,(HWND)wParam);
			return 0;
#endif
		case WM_MEASUREITEM:
			Wnd->OnMeasureItem((int)wParam,(MEASUREITEMSTRUCT*)lParam);
			return 0;
		case WM_MENUCHAR:
			return Wnd->OnMenuChar((char)LOWORD(wParam),(UINT)HIWORD(wParam),(HMENU)lParam);
		case WM_MENUSELECT:
			Wnd->OnMenuSelect((UINT) LOWORD(wParam),(UINT) HIWORD(wParam),(HMENU) lParam);
			return TRUE;
		case WM_MOUSEACTIVATE:
			return Wnd->OnMouseActivate((HWND)wParam,(UINT)LOWORD(lParam),(UINT)HIWORD(lParam));
		case WM_MOUSEMOVE:
			Wnd->OnMouseMove((UINT)wParam,LOWORD(lParam),HIWORD(lParam));
			return FALSE;
		case WM_MOVE:
			Wnd->OnMove((int)(short)LOWORD(lParam),(int)(short)HIWORD(lParam));
			return FALSE;
		case WM_NCACTIVATE:
			return Wnd->OnNcActivate((BOOL)wParam);
		case WM_NCDESTROY:
			Wnd->OnNcDestroy();
			return 0;
		case WM_NOTIFY:
			return Wnd->OnNotify((int)wParam,(LPNMHDR)lParam);
		case WM_PAINT:
			Wnd->OnPaint();
			break;
		case WM_PAINTCLIPBOARD:
			Wnd->OnPaintClipboard((HWND)wParam,(HGLOBAL)lParam);
			return FALSE;
		case WM_SETICON:
			return Wnd->WindowProc(msg,wParam,lParam);
		case WM_SIZE:
			Wnd->OnSize((UINT)wParam,LOWORD(lParam),HIWORD(lParam));
			return FALSE;
		case WM_SIZING:
			return Wnd->OnSizing((UINT)wParam,(LPRECT)lParam);
		case WM_TIMER:
			Wnd->OnTimer((DWORD)wParam);
			return FALSE;
		case WM_SYSCOMMAND:
			Wnd->OnSysCommand((UINT)wParam,lParam);
			return FALSE;
		case WM_VSCROLL:
			Wnd->OnVScroll((UINT)LOWORD(wParam),(UINT)HIWORD(wParam),(HWND) lParam);
			return FALSE;
		case WM_WINDOWPOSCHANGED:
			Wnd->OnWindowPosChanged((LPWINDOWPOS)lParam);
			return FALSE;
		case WM_WINDOWPOSCHANGING:
			Wnd->OnWindowPosChanging((LPWINDOWPOS)lParam);
			return FALSE;
		default:
			return Wnd->WindowProc(msg,wParam,lParam);
		}
		return FALSE;
	}
	return FALSE;
}

LRESULT CALLBACK CAppData::MdiWndProc(HWND hWnd, UINT msg,
					  WPARAM wParam, LPARAM lParam)
{
	if (msg==WM_NCCREATE)
	{
		CWnd* Wnd=(CWnd*)((MDICREATESTRUCT*)((LPCREATESTRUCT)lParam)->lpCreateParams)->lParam;
		Wnd->SetHandle(hWnd);
		SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)Wnd);
		return Wnd->OnNcCreate((LPCREATESTRUCT)lParam);
	}
	return WndProc(hWnd,msg,wParam,lParam);
}

#ifdef DEF_RESOURCES
LRESULT CALLBACK CAppData::PropPageWndProc(HWND hWnd,UINT msg,
							  WPARAM wParam, LPARAM lParam)
{
	if (msg==WM_INITDIALOG)
	{
		CWnd* Wnd=(CWnd*)(((PROPSHEETPAGE*)lParam)->lParam);
		Wnd->SetHandle(hWnd);
		SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)Wnd);
		return ((CPropertyPage*)Wnd)->OnInitDialog((HWND)wParam);
	}
	return WndProc(hWnd,msg,wParam,lParam);
}
#endif

DWORD CALLBACK CAppData::ThreadProc(LPVOID lpThreadParameter)
{
	DWORD ret;
	CWinThread* pThread=(CWinThread*)lpThreadParameter;
	if (pThread->InitInstance())
		pThread->ModalLoop();
	ret=pThread->ExitInstance();
	return ret;
}

BOOL RegisterWndClass(LPCTSTR lpszClassName,UINT nClassStyle,HCURSOR hCursor,HBRUSH hbrBackground,HICON hIcon,HICON hSmallIcon)
{
	WNDCLASSEX wc;

    wc.cbSize=sizeof(WNDCLASSEX); 
    wc.style=nClassStyle;
    wc.lpfnWndProc=(WNDPROC)CAppData::WndProc; 
    wc.cbClsExtra=0;
    wc.cbWndExtra=0; 
    wc.hInstance=GetInstanceHandle();
    wc.hIcon=hIcon;
    wc.hCursor=hCursor; 
    wc.hbrBackground=hbrBackground;
    wc.lpszMenuName=0;
    wc.lpszClassName=lpszClassName; 
    if (hSmallIcon!=NULL)
		wc.hIconSm=hSmallIcon;
	else
		wc.hIconSm=hIcon;
	if (!RegisterClassEx(&wc))
		return FALSE;
	return TRUE;		
}

BOOL RegisterMDIChildClass(LPCTSTR lpszClassName,UINT nClassStyle,HCURSOR hCursor,HBRUSH hbrBackground,HICON hIcon,HICON hSmallIcon)
{
	WNDCLASSEX wc;

    wc.cbSize=sizeof(WNDCLASSEX); 
    wc.style=nClassStyle;
    wc.lpfnWndProc=(WNDPROC)CAppData::MdiWndProc; 
    wc.cbClsExtra=0;
    wc.cbWndExtra=0; 
    wc.hInstance=GetInstanceHandle();
    wc.hIcon=hIcon;
    wc.hCursor=hCursor; 
    wc.hbrBackground=hbrBackground;
    wc.lpszMenuName=0;
    wc.lpszClassName=lpszClassName; 
    if (hSmallIcon!=NULL)
		wc.hIconSm=hSmallIcon;
	else
		wc.hIconSm=hIcon;

	if (!RegisterClassEx(&wc))
		return FALSE;
	return TRUE;		
}

BOOL WaitForWindow(CWnd* pWnd,DWORD dwMilliseconds)
{
	CString name;
	name.SetBase(32);
	name << "WFWE" << (ULONGLONG)pWnd;
	HANDLE hWaitEvent=CreateEvent(NULL,FALSE,FALSE,name);
	if (hWaitEvent==NULL)
		return FALSE;
	int nRet=MsgWaitForMultipleObjectsEx(1,(HANDLE*)&hWaitEvent,dwMilliseconds,
		QS_ALLEVENTS|QS_ALLINPUT,MWMO_WAITALL|MWMO_ALERTABLE);
	CloseHandle(hWaitEvent);
	return nRet==WAIT_OBJECT_0;
}


BOOL ForceForegroundAndFocus(HWND hWindow)
{
	static long dwInProc = 0;
	
    if (dwInProc == 0) {
		InterlockedIncrement(&dwInProc);
		
		HWND hForeground=GetForegroundWindow();
		DWORD dwForegoundThread,dwCurrentThreadId;
		BOOL bDetach=FALSE;

		

		if (hForeground!=NULL)
		{
	
			dwForegoundThread=GetWindowThreadProcessId(hForeground,NULL);
			dwCurrentThreadId=GetWindowThreadProcessId(hWindow,NULL);
				
			if (dwForegoundThread!=dwCurrentThreadId)
				bDetach=AttachThreadInput(dwCurrentThreadId,dwForegoundThread,TRUE);
		}

		SetForegroundWindow(hWindow);
		BringWindowToTop(hWindow);
		SetFocus(hWindow);
		
		if (bDetach)
			AttachThreadInput(dwCurrentThreadId,dwForegoundThread,FALSE);
		
		InterlockedDecrement(&dwInProc);
		return TRUE;
	}
	return FALSE;
}

int ReportSystemError(HWND hWnd,LPCSTR szTitle,DWORD dwError,DWORD dwExtra,LPCSTR szPrefix)
{
	if (dwError==DWORD(-1))
		dwError=GetLastError();

	char* szBuffer;
	DWORD dwLength=FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,NULL,
		dwError,LANG_USER_DEFAULT,(LPSTR)(PVOID*)&szBuffer,0,NULL);
	if (!dwLength)
		return MessageBox(hWnd,"Couldn't format error message",szTitle,MB_OK|MB_ICONERROR);
		
	int nRet;
	if (szPrefix==NULL && dwExtra==0)
		nRet=MessageBox(hWnd,szBuffer,szTitle,MB_OK|MB_ICONERROR);
	else
	{
		if (szPrefix==NULL)
			szPrefix="%s (%d)";

		dwLength+=istrlen(szPrefix)+20;
		char* pMessage=new char[dwLength];
		StringCbPrintf(pMessage,dwLength,szPrefix,szBuffer,dwExtra);
		nRet=MessageBox(hWnd,pMessage,szTitle,MB_OK|MB_ICONERROR);
		delete[] pMessage;
	}
	LocalFree(szBuffer);
	return nRet;
}


#endif