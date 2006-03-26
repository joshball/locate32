////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////


#ifndef HFCAPP_H
#define HFCAPP_H


	

// Every application global variables and functions should be here
// This class is only for library use
class CAppData {
public:
	CAppData();
	~CAppData();

public:
	static void stdfunc(); // Do nothing

private:
	// Default CALLBACKS
	static DWORD CALLBACK ThreadProc(LPVOID);
	static BOOL CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
	static BOOL CALLBACK MdiWndProc(HWND,UINT,WPARAM,LPARAM);
	static BOOL CALLBACK CommonDialogProc(HWND,UINT,WPARAM,LPARAM);
	static int CALLBACK FolderDialogProc(HWND,UINT,LPARAM,LPARAM);
	static BOOL CALLBACK PagePaintProc(HWND,UINT,WPARAM,LPARAM);
	static BOOL CALLBACK PropPageWndProc(HWND,UINT,WPARAM,LPARAM);

public:
	static void MatrixErrorProc(void*,DWORD,DWORD);

private:
#ifdef DEF_APP
	CList<CWinThread*>* m_pThreads;
	CWinApp* pAppClass;
	CCommonDialog* m_pCommonDialog;
#endif
#ifdef DEF_RESOURCES
	HINSTANCE hAppInstance;
	HINSTANCE hCommonResourceHandle;
	HINSTANCE hLanguageSpecificResourceHandle;	
#endif

	// Functions allowed to use CAppData
#ifdef DEF_APP
	friend int PASCAL WinMain(HINSTANCE,HINSTANCE,LPTSTR,int);
	friend CWinThread* GetCurrentWinThread();
	friend CWinThread* GetWinThread(DWORD);
	friend CWinApp* GetApp();
	friend CWnd* GetMainWnd();
	friend HINSTANCE GetInstanceHandle();
#endif
#ifdef DEF_RESOURCES
	friend HINSTANCE GetResourceHandle(TypeOfResourceHandle bType);
	friend HINSTANCE SetResourceHandle(HINSTANCE,TypeOfResourceHandle bType);
	friend HINSTANCE GetCommonResourceHandle();
	friend HINSTANCE GetLanguageSpecificResourceHandle();
#endif
#ifdef DEF_WINDOWS
	friend class CFileDialog;
	friend class CFontDialog;
	friend void CPropertyPage::Construct(LPCTSTR,UINT,TypeOfResourceHandle);
	friend CColorDialog::CColorDialog(COLORREF,DWORD);
	friend CPageSetupDialog::CPageSetupDialog(DWORD);
	friend CPrintDialog::CPrintDialog(BOOL,DWORD);
	friend BOOL CPageSetupDialog::DoModal(HWND);
	friend CFindReplaceDialog::CFindReplaceDialog();
	friend CFolderDialog::CFolderDialog(LPCSTR,UINT,LPCITEMIDLIST);
	friend CFolderDialog::CFolderDialog(UINT,UINT,LPCITEMIDLIST);
	friend BOOL RegisterWndClass(LPCTSTR,UINT,HCURSOR,HBRUSH,HICON,HICON);
	friend BOOL RegisterMDIChildClass(LPCTSTR,UINT,HCURSOR,HBRUSH,HICON,HICON);
	friend int CDialog::DoModal(HWND,TypeOfResourceHandle);
	friend BOOL CDialog::Create(HWND,TypeOfResourceHandle);
#endif 
#ifdef _DEBUG_LOGGING
	friend void StartDebugLogging();
#endif

	// Classes allowed to use CAppData
	friend CWinThread;
	friend CWinApp;
};

inline CAppData* GetAppData() 
{
	extern CAppData m_AppData;
	return &m_AppData;
}

#ifdef DEF_APP
inline CWinApp * GetApp()
{
	return GetAppData()->pAppClass;
}

inline HINSTANCE GetInstanceHandle()
{
	return GetAppData()->hAppInstance;
}

#endif

#ifdef DEF_RESOURCES
inline HINSTANCE GetResourceHandle(TypeOfResourceHandle bType)
{
	if (bType==LanguageSpecificResource)
		return GetAppData()->hLanguageSpecificResourceHandle;
	return GetAppData()->hCommonResourceHandle;
}

inline HINSTANCE GetCommonResourceHandle()
{
	return GetAppData()->hCommonResourceHandle;
}

inline HINSTANCE GetLanguageSpecificResourceHandle()
{
	return GetAppData()->hLanguageSpecificResourceHandle;
}
#endif

#ifdef DEF_APP


class CWinThread : public CCmdTarget
{
public:
	CWinThread();
	
	virtual ~CWinThread();

	BOOL CreateThread(DWORD dwCreateFlags = 0, UINT nStackSize = 0,
		LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

	HANDLE m_hThread;
	operator HANDLE() const;
	
	HACCEL GetAccelTable() const;
	void SetAccelTable(HACCEL hAccel);
	void ClearAccelTables();
	void LoadAccelTable(LPCSTR lpTable,TypeOfResourceHandle bType=LanguageSpecificResource);
	void LoadAccelTable(int iTableId,TypeOfResourceHandle bType=LanguageSpecificResource);

	void RegisterDialog(HWND hDialog);
	void UnRegisterDialog(HWND hDialog);
	
	int GetThreadPriority() const;
	BOOL SetThreadPriority(int nPriority);

	BOOL TerminateThread(DWORD dwExitCode);
	BOOL GetExitCodeThread(LPDWORD lpExitCode) const;
	BOOL IsRunning() const;

	DWORD SuspendThread();
	DWORD ResumeThread();
	BOOL PostThreadMessage(UINT message, WPARAM wParam, LPARAM lParam);

    virtual BOOL InitInstance(); // if return value is TRUE, thread will go to modal loop
	virtual int ExitInstance();

	virtual CWnd* GetMainWnd();

	const MSG* GetCurrentMessage() const;
	virtual BOOL OnThreadMessage(MSG* pMsg);
	


public:
	virtual BOOL OnIdle(LONG lCount);

	virtual DWORD ModalLoop();

public:
	//Accelator table support
	HACCEL GetAccelTableForWindow(HWND hWnd);
	BOOL SetAccelTableForWindow(HWND hWnd,HACCEL hTable,BOOL bDontChangeIfExist=FALSE,HWND hWndTo=(HWND)-1);
	BOOL SetAccelTableForWindow(HWND hWnd,LPCSTR lpTable,BOOL bDontChangeIfExist=FALSE,HWND hWndTo=(HWND)-1,TypeOfResourceHandle bType=LanguageSpecificResource);
	BOOL SetAccelTableForWindow(HWND hWnd,int iTableId,BOOL bDontChangeIfExist=FALSE,HWND hWndTo=(HWND)-1,TypeOfResourceHandle bType=LanguageSpecificResource);
	BOOL SetAccelTableForChilds(HWND hParent,HACCEL hTable,BOOL bDontChangeIfExist=FALSE,HWND hWndTo=(HWND)-1,DWORD dwStyleFlags=WS_CHILD); // if hWndTo=NULL message will be sent to child
	BOOL SetAccelTableForChilds(HWND hParent,LPCSTR lpTable,BOOL bDontChangeIfExist=FALSE,HWND hWndTo=(HWND)-1,DWORD dwStyleFlags=WS_CHILD,TypeOfResourceHandle bType=LanguageSpecificResource); // if hWndTo=NULL message will be sent to child
	BOOL SetAccelTableForChilds(HWND hParent,int iTableId,BOOL bDontChangeIfExist=FALSE,HWND hWndTo=(HWND)-1,DWORD dwStyleFlags=WS_CHILD,TypeOfResourceHandle bType=LanguageSpecificResource); // if hWndTo=NULL message will be sent to child
	
protected:
	class CAccelNode
	{
	public:
		CAccelNode() { }
		CAccelNode(HWND hwnd,HACCEL haccel,HWND hwndto=(HWND)-1) { hWnd=hwnd;hAccel=haccel; if (hwndto==(HWND)-1) hWndTo=hwnd; else hWndTo=hwndto; }
		HWND hWnd;
		HACCEL hAccel;
		HWND hWndTo;
	};
	CArray<CAccelNode> m_Accels;
	CArray<HWND> m_Dialogs;
	MSG m_currentMessage;

public:
	CWnd* m_pMainWnd;
	CWnd* m_pMouseMessagesTo;
	DWORD m_nThreadID;
	BOOL m_bAutoDelete;
};
	
class CWinApp : public CWinThread
{
public:
	LPTSTR m_lpCmdLine;
	int m_nCmdShow;
	LPCTSTR m_pszAppName;
	LPTSTR m_pszExeName;

public:
	CWinApp(LPCTSTR lpszAppName);
	
	virtual ~CWinApp();
	

	LPCSTR GetCmdLine() const { return m_lpCmdLine; }
	LPCSTR GetAppName() const { return m_pszAppName; }
	LPCSTR GetExeName() const { return m_pszExeName; }
	void SetMainWnd(CWnd* pWnd);
	int GetCmdShow() { return m_nCmdShow; }
		
	virtual BOOL InitApplication();

private:
	CAppData m_AppData;
};

#endif

////////////////////////////////////////
// Inline functions

////////////////////////////////////////
// CAppData

inline CAppData::CAppData()
#ifdef DEF_APP
:	m_pThreads(NULL)
#endif
{
}

inline CAppData::~CAppData()
{
}

#ifdef DEF_APP

////////////////////////////////////////
// CWinThread

inline BOOL CWinThread::CreateThread(DWORD dwCreateFlags,UINT nStackSize,LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
	return (m_hThread=::CreateThread(lpSecurityAttrs,nStackSize,(LPTHREAD_START_ROUTINE)CAppData::ThreadProc,this,dwCreateFlags,&m_nThreadID))!=NULL;
}

inline CWnd* CWinThread::GetMainWnd()
{
	return m_pMainWnd;
}

inline CWinThread::operator HANDLE() const
{
	return m_hThread;
}

inline int CWinThread::GetThreadPriority() const
{
	return  ::GetThreadPriority(m_hThread);
}
   
inline BOOL CWinThread::SetThreadPriority(int nPriority)
{
	return ::SetThreadPriority(m_hThread,nPriority);
}

inline DWORD CWinThread::SuspendThread()
{
	return ::SuspendThread(m_hThread);
}

inline DWORD CWinThread::ResumeThread()
{
	return ::ResumeThread(m_hThread);
}

inline const MSG* CWinThread::GetCurrentMessage() const
{
	return &m_currentMessage;
}

inline BOOL CWinThread::PostThreadMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	return ::PostThreadMessage(m_nThreadID,message,wParam,lParam);
}

inline void CWinThread::ClearAccelTables()
{
	m_Accels.RemoveAll();
}	

inline void CWinThread::LoadAccelTable(LPCSTR lpTable,TypeOfResourceHandle bType)
{
	SetAccelTable(LoadAccelerators(GetResourceHandle(bType),lpTable));
}
	
inline void CWinThread::LoadAccelTable(int iTableId,TypeOfResourceHandle bType)
{
	SetAccelTable(LoadAccelerators(GetResourceHandle(bType),MAKEINTRESOURCE(iTableId)));
}

inline BOOL CWinThread::SetAccelTableForWindow(HWND hWnd,LPCSTR lpTable,BOOL bDontChangeIfExist,HWND hWndTo,TypeOfResourceHandle bType)
{
	return SetAccelTableForWindow(hWnd,LoadAccelerators(GetResourceHandle(bType),lpTable),bDontChangeIfExist,hWndTo);
}

inline BOOL CWinThread::SetAccelTableForWindow(HWND hWnd,int iTableId,BOOL bDontChangeIfExist,HWND hWndTo,TypeOfResourceHandle bType)
{
	return SetAccelTableForWindow(hWnd,LoadAccelerators(GetResourceHandle(bType),MAKEINTRESOURCE(iTableId)),bDontChangeIfExist,hWndTo);
}

inline BOOL CWinThread::SetAccelTableForChilds(HWND hParent,LPCSTR lpTable,BOOL bDontChangeIfExist,HWND hWndTo,DWORD dwStyleFlags,TypeOfResourceHandle bType)
{
	return SetAccelTableForChilds(hParent,LoadAccelerators(GetResourceHandle(bType),lpTable),bDontChangeIfExist,hWndTo,dwStyleFlags);
}

inline BOOL CWinThread::SetAccelTableForChilds(HWND hParent,int iTableId,BOOL bDontChangeIfExist,HWND hWndTo,DWORD dwStyleFlags,TypeOfResourceHandle bType)
{
	return SetAccelTableForChilds(hParent,LoadAccelerators(GetResourceHandle(bType),MAKEINTRESOURCE(iTableId)),bDontChangeIfExist,hWndTo,dwStyleFlags);
}

inline void CWinThread::RegisterDialog(HWND hDialog)
{
	for (int i=0;i<m_Dialogs.GetSize();i++)
	{
		if (m_Dialogs[i]==hDialog)
			return;
	}
	m_Dialogs.Add(hDialog);
}

inline void CWinThread::UnRegisterDialog(HWND hDialog)
{
	for (int i=0;i<m_Dialogs.GetSize();i++)
	{
		if (m_Dialogs[i]==hDialog)
		{
			m_Dialogs.RemoveAt(i);
			return;
		}
	}
}

inline void CWinApp::SetMainWnd(CWnd* pWnd)
{
	m_pMainWnd=pWnd;
}

inline CWnd* GetMainWnd()
{
	return GetAppData()->pAppClass->GetMainWnd();
}


inline int CDialog::DoModal(HWND hWndParent,TypeOfResourceHandle bType)
{
	return DialogBoxParam(GetResourceHandle(bType),m_lpszTemplateName,hWndParent,(DLGPROC)CAppData::WndProc,(LPARAM)this);
}

inline BOOL CDialog::Create(HWND hWndParent,TypeOfResourceHandle bType)
{
	if (IsFullUnicodeSupport())
	{
		if (IS_INTRESOURCE(m_lpszTemplateName))
			return (m_hWnd=::CreateDialogParamW(GetResourceHandle(bType),(LPCWSTR)m_lpszTemplateName,hWndParent,(DLGPROC)CAppData::WndProc,(LPARAM)this))!=NULL;
		CStringW tmp(m_lpszTemplateName);
		return (m_hWnd=::CreateDialogParamW(GetResourceHandle(bType),tmp,hWndParent,(DLGPROC)CAppData::WndProc,(LPARAM)this))!=NULL;
	}
	return (m_hWnd=::CreateDialogParamA(GetResourceHandle(bType),m_lpszTemplateName,hWndParent,(DLGPROC)CAppData::WndProc,(LPARAM)this))!=NULL;
}

inline BOOL CDialog::EndDialog(int nResult) const
{
	return ::EndDialog(m_hWnd,nResult);
}

	
#endif
#endif