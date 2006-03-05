#if !defined(LOCATEAPP_H)
#define LOCATEAPP_H

#if _MSC_VER >= 1000
#pragma once
#endif

class CLocateAppWnd;
class CAboutDlg;
class CSettingsProperties;
class CUpdateThread;
class CLocateDlgThread;

DOUBLE GetCpuTime();

class CLocateAppWnd : public CFrameWnd
{
public:
	
	struct RootInfo {
		LPSTR pName;
		LPSTR pRoot;
		DWORD dwNumberOfDatabases;
		DWORD dwCurrentDatabase;
		WORD wProgressState;
		UpdateError ueError;
	};

	class CUpdateStatusWnd : public CFrameWnd 
	{
	public:
		CUpdateStatusWnd();
		virtual ~CUpdateStatusWnd();
	
		virtual int OnCreate(LPCREATESTRUCT lpcs);
		virtual void OnNcDestroy();
		virtual void OnTimer(DWORD wTimerID);
		virtual void OnPaint();
		virtual BOOL WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);
	
		

	public:
		void Update();
		void Update(WORD wThreads,WORD wRunning,RootInfo* pRootInfos);
		void IdleClose();
		virtual BOOL DestroyWindow();
		void CheckForegroundWindow();

		void SetFonts();
		void SetPosition();
		void FormatErrorForStatusTooltip(UpdateError ueError,CDatabaseUpdater* pUpdater);
		void FormatStatusTextLine(CString& str,const CLocateAppWnd::RootInfo& pRootInfo,int nThreadID=-1,int nThreads=1);
		static void EnlargeSizeForText(CDC& dc,CString& str,CSize& szSize);
		static void EnlargeSizeForText(CDC& dc,LPCSTR szText,int nLength,CSize& szSize);
		static void FillFontStructs(LOGFONT* pTextFont,LOGFONT* pTitleFont);

			
		CString sStatusText;
		CArrayFAP<LPSTR> m_aErrors;
		
		CFont m_TitleFont,m_Font;
		CSize m_WindowSize;

		COLORREF m_cTextColor;
		COLORREF m_cTitleColor;
		COLORREF m_cErrorColor;
		COLORREF m_cBackColor;
		
		HANDLE m_hUpdateMutex;
	};

public:
	CLocateAppWnd();
	virtual ~CLocateAppWnd();

public:

	virtual int OnCreate(LPCREATESTRUCT lpcs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual void OnDestroy();
	virtual void OnTimer(DWORD wTimerID); 
	virtual void OnInitMenuPopup(HMENU hPopupMenu,UINT nIndex,BOOL bSysMenu);
    virtual BOOL WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);
	
	DWORD OnAnotherInstance(ATOM aCommandLine);
	DWORD OnSystemTrayMessage(UINT uID,UINT msg);
	BOOL SetUpdateStatusInformation(HICON hIcon=NULL,UINT uTip=0);
	void GetRootInfos(WORD& wThreads,WORD& wRunning,RootInfo*& pRootInfos);
	static void FreeRootInfos(WORD wThreads,RootInfo* pRootInfos);

	void AddTaskbarIcon();
	void DeleteTaskbarIcon();
	
	DWORD SetSchedules(CList<CSchedule*>* pSchedules=NULL);
	BOOL SaveSchedules();
	void CheckSchedules();
	void RunStartupSchedules();

	BOOL StartUpdateStatusNotification();
	BOOL StopUpdateStatusNotification();
	
	BOOL TurnOnShortcuts();
	BOOL TurnOffShortcuts();



	BYTE OnAbout();
	BYTE OnSettings();
	BYTE OnLocate();
	
	// If pDatabases is:
	//  NULL: Global update
	//  0xFFFFFFFF: Select databases
	//  otherwise Use databases in pDatabase array which is 
	//  double zero terminated seqeuence of strings
	BYTE OnUpdate(BOOL bStopIfProcessing,LPSTR pDatabases=NULL); 
	BYTE OnUpdate(BOOL bStopIfProcessing,LPSTR pDatabases,int nThreadPriority); 

	static DWORD WINAPI KillUpdaterProc(LPVOID lpParameter);
    
public:
	
public:
	CMenu m_Menu;
	CAboutDlg* m_pAbout;
	CSettingsProperties* m_pSettings;
	

	CLocateDlgThread* m_pLocateDlgThread;
	CUpdateStatusWnd* m_pUpdateStatusWnd;
	
	HICON* m_pUpdateAnimIcons;
	int m_nCurUpdateAnimBitmap;
	
	CListFP <CSchedule*> m_Schedules;

	// Keyboard shortcuts
	CArrayFP<CShortcut*> m_aShortcuts;
	HHOOK m_hHook;
	
		
	friend inline CLocateDlg* GetLocateWnd();

#ifdef _DEBUG
public:
	inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif

	UINT nHFCInstallationMessage;
	UINT nTaskbarCreated;


};


class CLocateApp : public CWinApp  
{
public:
	CLocateApp();
	virtual ~CLocateApp();
	
	virtual int ExitInstance();
	virtual BOOL InitInstance();

public:
	class CStartData
	{
	public:
		enum StatusFlags {
            statusRunAtStartUp=0x1,
			statusFindFileNames=0x8,
			statusFindFolderNames=0x10,
			statusFindFileAndFolderNames=statusFindFileNames|statusFindFolderNames,
			statusFindIsNotMatchCase=0x40,
			statusMatchWholeName=0x80,
			statusNoMatchWholeName=0x100,
			statusReplaceSpacesWithAsterisks=0x200,
			statusNoReplaceSpacesWithAsterisks=0x400,
			statusUseWholePath=0x800,
			statusNoUseWholePath=0x1000
		};

		enum StartupFlags {
			startupLeaveBackground=0x1,
			startupUpdate=0x2,
			startupDoNotOpenDialog=0x4,
			startupExitAfterUpdating=0x8,
			startupNewInstance=0x10,
			startupDatabasesOverridden=0x20,
			startupExitedBeforeInitialization=0x40

		};

		enum PriorityFlags {
			priorityDontChange=0,
			priorityHigh=HIGH_PRIORITY_CLASS,
			priorityAbove=ABOVE_NORMAL_PRIORITY_CLASS,
			priorityNormal=NORMAL_PRIORITY_CLASS,
			priorityBelow=BELOW_NORMAL_PRIORITY_CLASS,
			priorityIdle=IDLE_PRIORITY_CLASS,
			priorityRealTime=REALTIME_PRIORITY_CLASS
		};


		CStartData();
		~CStartData();

		LPSTR m_pStartPath;
		LPSTR m_pStartString;
		LPSTR m_pTypeString;
		LPSTR m_pFindText;
		LPSTR m_pLoadPreset;
		DWORD m_nStatus;
		DWORD m_nPriority;
		BYTE m_nStartup;
		DWORD m_dwMaxFoundFiles;
		DWORD m_dwMaxFileSize;
		DWORD m_dwMinFileSize;
		char m_cMaxSizeType;
		char m_cMinSizeType;
		DWORD m_dwMaxDate;
		DWORD m_dwMinDate;
		char m_cMaxDateType;
		char m_cMinDateType;
		BYTE m_nSorting;

		CArrayFP<CDatabase*> m_aDatabases;

#ifdef _DEBUG
	public:
		inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
		inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
		inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif

	};

	enum ProgramFlags {
		// Errors 
		pfShowCriticalErrors = 0x01, // Option
        pfShowNonCriticalErrors = 0x02, // Option
        pfErrorMask = 0x03,
		pfErrorDefault = pfShowCriticalErrors|pfShowNonCriticalErrors,
		pfErrorSave = pfShowCriticalErrors|pfShowNonCriticalErrors,
		
		// Tooltip
		pfEnableUpdateTooltip = 0x10, // Option
		pfUpdateTooltipNeverTopmost = 0,
		pfUpdateTooltipAlwaysTopmost = 0x20, // Option
		pfUpdateTooltipNoTopmostWhenForegroundWndMaximized = 0x40, // Option
		pfUpdateTooltipNoTopmostWhdnForegroundWndInFullScreen = 0x60, // Option
		pfUpdateTooltipTopmostMask = 0x60,
		pfUpdateTooltipPositionDefault = 0x00,
		pfUpdateTooltipPositionUp = 0x080,
		pfUpdateTooltipPositionDown = 0x180,
		pfUpdateTooltipPositionLeft = 0x080,
		pfUpdateTooltipPositionRight = 0x280,
		pfUpdateTooltipPositionUpLeft = pfUpdateTooltipPositionUp|pfUpdateTooltipPositionLeft,
		pfUpdateTooltipPositionUpRight = pfUpdateTooltipPositionUp|pfUpdateTooltipPositionRight,
		pfUpdateTooltipPositionDownLeft = pfUpdateTooltipPositionDown|pfUpdateTooltipPositionLeft,
		pfUpdateTooltipPositionDownRight = pfUpdateTooltipPositionDown|pfUpdateTooltipPositionRight,
		pfUpdateTooltipPositionMask = 0x0380,
        pfTooltipMask = 0x3F0,

		pfTooltipDefault = pfEnableUpdateTooltip|pfUpdateTooltipAlwaysTopmost|pfUpdateTooltipPositionDefault,
		pfTooltipSave = pfEnableUpdateTooltip|pfUpdateTooltipTopmostMask|pfUpdateTooltipPositionMask,

		// Filesize/time/date format
		pfFormatUseLocaleFormat = 0x4, // Option
		pfFormatMask = 0x4,
		pfFormatDefault = pfFormatUseLocaleFormat,
		pfFormatSave = pfFormatUseLocaleFormat,

		pfDefault = pfErrorDefault|pfTooltipDefault|pfFormatDefault,
		pfSave = pfErrorSave|pfTooltipSave|pfFormatSave
	};

	enum FileSizeFormats {
		fsfOverKBasKB = 0,
		fsfBestUnit = 1,
		fsfBytes = 2,
		fsfBytesNoUnit = 3,
		fsfKiloBytes = 4,
		fsfKiloBytesNoUnit = 5,
		fsfMegaBytesMegaBytes = 6,
		fsfMegaBytesMegaBytesNoUnit = 7
	};


protected:
	BYTE CheckDatabases();
	BYTE SetDeleteAndDefaultImage();
	
public:
	static BOOL ParseParameters(LPCTSTR lpCmdLine,CStartData* pStartData);

	static BOOL ChechOtherInstances();
	
	void SaveRegistry() const;
	void LoadRegistry();
	BOOL UpdateSettings();

	LPSTR FormatDateAndTimeString(WORD wDate,WORD wTime=WORD(-1));
	LPSTR FormatFileSizeString(DWORD dwFileSizeLo,DWORD bFileSizeHi) const;
	BOOL SetLanguageSpecifigHandles();


	BOOL StopUpdating(BOOL bForce=TRUE);
    BOOL IsUpdating() const;
	
	// ReleaseUpdatersPointer should always be called after pointer is free to use for other
	CDatabaseUpdater** GetUpdatersPointer();
	CDatabaseUpdater*** GetUpdatersPointerPtr();
	void ReleaseUpdatersPointer();
	
	BOOL GlobalUpdate(CArray<PDATABASE>* paDatabases=NULL,int nThreadPriority=THREAD_PRIORITY_NORMAL);



	// Database menu functions
	static BOOL IsDatabaseMenu(HMENU hMenu);
	void OnInitDatabaseMenu(HMENU hPopupMenu);
	void OnDatabaseMenuItem(WORD wID);
	static int GetDatabaseMenuIndex(HMENU hPopupMenu);

	static DWORD GetProgramFlags();
    	
private:
	static BOOL CALLBACK UpdateProc(DWORD dwParam,CallingReason crReason,UpdateError ueCode,CDatabaseUpdater* pUpdater);
	

protected:
	DWORD m_dwProgramFlags;

public:
	WORD m_wComCtrlVersion;
	WORD m_wShellDllVersion;
	
	int m_nDelImage;
	int m_nDefImage;
	int m_nDirImage;
	int m_nDriveImage;

	// Date & Time format string
	CString m_strDateFormat;
	CString m_strTimeFormat;
	FileSizeFormats m_nFileSizeFormat;


	DWORD(WINAPI* m_pGetLongPathName)(LPCSTR,LPSTR,DWORD);

	static DWORD WINAPI GetLongPathName(LPCSTR lpszShortPath,LPSTR lpszLongPath,DWORD cchBuffer);


protected:
	CStartData* m_pStartData;
	CLocateAppWnd m_AppWnd;

	BYTE m_nStartup;
	
	HANDLE m_hUpdatersPointerInUse;
	CDatabaseUpdater** m_ppUpdaters;
	

	
	CArrayFP<CDatabase*> m_aDatabases;
	mutable CDatabase* m_pLastDatabase;

	static void ChangeAndAlloc(LPSTR& pVar,LPCSTR szText);
	static void ChangeAndAlloc(LPSTR& pVar,LPCSTR szText,DWORD dwLength);


public:
	
	const CArray<PDATABASE>& GetDatabases() const { return m_aDatabases; }
	CArray<PDATABASE>* GetDatabasesPtr() { return &m_aDatabases; }
	void SetDatabases(const CArray<CDatabase*>& rDatabases);
	WORD GetDatabasesNumberOfThreads() const;

	BYTE GetStartupFlags() const { return m_nStartup; }
	BOOL IsStartupFlagSet(CStartData::StartupFlags flag) { return m_nStartup&flag?1:0; }
	void SetStartupFlag(CStartData::StartupFlags flag) { m_nStartup|=BYTE(flag); }
	void ClearStartupFlag(CStartData::StartupFlags flag) { m_nStartup&=~BYTE(flag); }

	const CStartData* GetStartData() const { return m_pStartData; }
	void ClearStartData();
	void SetStartData(CStartData* pStarData);
	
	const CDatabase* GetDatabase(WORD wID) const;

	static INT_PTR CALLBACK DummyDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

	friend CLocateAppWnd* GetLocateAppWnd();
	friend CLocateDlg* GetLocateDlg();
};

inline CLocateApp::CStartData::CStartData()
:	m_nStatus(0),m_nStartup(0),m_dwMaxFoundFiles(DWORD(-1)),
	m_dwMaxFileSize(DWORD(-1)),m_dwMinFileSize(DWORD(-1)),
    m_dwMaxDate(DWORD(-1)),m_dwMinDate(DWORD(-1)),
    m_cMaxSizeType(0),m_cMinSizeType(0),
    m_cMaxDateType(0),m_cMinDateType(0),
    m_nSorting(BYTE(-1)),m_nPriority(priorityDontChange),
	m_pStartPath(NULL),m_pStartString(NULL),
	m_pTypeString(NULL),m_pFindText(NULL),m_pLoadPreset(NULL)
{ 
}

inline CLocateApp::CStartData::~CStartData()
{
	if (m_pStartPath!=NULL)
		delete[] m_pStartPath;
	if (m_pStartString!=NULL)
		delete[] m_pStartString;
	if (m_pTypeString!=NULL)
		delete[] m_pTypeString;
	if (m_pFindText!=NULL)
		delete[] m_pFindText;
	if (m_pLoadPreset!=NULL)
		delete[] m_pLoadPreset;

}

inline void CLocateApp::SetStartData(CStartData* pStarData)
{
	if (m_pStartData!=NULL)
		delete m_pStartData;
	m_pStartData=pStarData;
}

inline const CDatabase* CLocateApp::GetDatabase(WORD wID) const
{
	if (m_pLastDatabase->GetID()==wID)
		return m_pLastDatabase;

	for (int i=0;i<m_aDatabases.GetSize();i++)
	{
		m_pLastDatabase=m_aDatabases[i];
		if (m_pLastDatabase->GetID()==wID)
			return m_pLastDatabase;
	}
	return NULL;
}

	
inline BOOL CLocateApp::IsDatabaseMenu(HMENU hMenu)
{
	UINT nID=GetMenuItemID(hMenu,0);
	return nID>=IDM_DEFUPDATEDBITEM && nID<IDM_DEFUPDATEDBITEM+1000;
}


inline CLocateApp* GetLocateApp()
{
	extern CLocateApp theApp;
	return &theApp;
}

inline CLocateAppWnd* GetLocateAppWnd()
{
	extern CLocateApp theApp;
	return (CLocateAppWnd*)&theApp.m_AppWnd;
}

inline CLocateAppWnd::CLocateAppWnd()
:	m_pAbout(NULL),m_pSettings(NULL),
	m_pLocateDlgThread(NULL),
	m_pUpdateAnimIcons(NULL),m_hHook(NULL)
{
	DebugMessage("CLocateAppWnd::CLocateAppWnd()");
}

inline CLocateAppWnd::~CLocateAppWnd()
{
	DebugMessage("CLocateAppWnd::~CLocateAppWnd()");
	//m_Schedules.RemoveAll();

	

}

inline BOOL CLocateApp::IsUpdating() const
{
	return m_ppUpdaters!=NULL;
}

inline void CLocateApp::ReleaseUpdatersPointer()
{
	ReleaseMutex(m_hUpdatersPointerInUse);
	DebugMessage("CLocateApp::ReleaseUpdatersPointer(): mutex released");
}

inline CDatabaseUpdater** CLocateApp::GetUpdatersPointer()
{
	DebugMessage("CLocateApp::GetUpdatersPointer(): waiting mutex");
	if (WaitForMutex(m_hUpdatersPointerInUse,LOCATEAPPUPDATERSMUTEXTIMEOUT))
		return NULL;
	DebugMessage("CLocateApp::GetUpdatersPointer(): continuing");
	return m_ppUpdaters;
}

inline CDatabaseUpdater*** CLocateApp::GetUpdatersPointerPtr()
{
	DebugMessage("CLocateApp::GetUpdatersPointerPtr(): waiting mutex");
	if (WaitForMutex(m_hUpdatersPointerInUse,LOCATEAPPUPDATERSMUTEXTIMEOUT))
		return NULL;
	DebugMessage("CLocateApp::GetUpdatersPointerPtr(): continuing");
	
	return &m_ppUpdaters;
}
	

inline void CLocateApp::ChangeAndAlloc(LPSTR& pVar,LPCSTR szText)
{
	SIZE_T dwLength=istrlen(szText);
	if (pVar!=NULL)
		delete[] pVar;
	pVar=new char [dwLength+1];
	CopyMemory(pVar,szText,dwLength);
	pVar[dwLength]='\0';
}

inline void CLocateApp::ChangeAndAlloc(LPSTR& pVar,LPCSTR szText,DWORD dwLength)
{
	if (dwLength==DWORD(-1))
		dwLength=istrlen(szText);
	
	if (pVar!=NULL)
		delete[] pVar;
	pVar=new char [dwLength+1];
	CopyMemory(pVar,szText,dwLength);
	pVar[dwLength]='\0';
}

inline DWORD CLocateApp::GetProgramFlags()
{
	extern CLocateApp theApp;
	return theApp.m_dwProgramFlags;
}



inline void CLocateAppWnd::CUpdateStatusWnd::EnlargeSizeForText(CDC& dc,CString& str,CSize& szSize)
{
	EnlargeSizeForText(dc,str,str.GetLength(),szSize);
}

inline void CLocateAppWnd::CUpdateStatusWnd::EnlargeSizeForText(CDC& dc,LPCSTR szText,int nLength,CSize& szSize)
{
	CRect rc(0,0,0,0);
	dc.DrawText(szText,nLength,&rc,DT_SINGLELINE|DT_CALCRECT);
	if (szSize.cx<rc.Width())
		szSize.cx=rc.Width();
	if (szSize.cy<rc.Height())
		szSize.cy=rc.Height();
}

inline BYTE CLocateAppWnd::OnUpdate(BOOL bStopIfProcessing,LPSTR pDatabases)
{
	DWORD nThreadPriority=THREAD_PRIORITY_NORMAL;

	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::defRead)==ERROR_SUCCESS)
		RegKey.QueryValue("Update Process Priority",nThreadPriority);
	
	return OnUpdate(bStopIfProcessing,pDatabases,(int)nThreadPriority);
}

extern FASTALLOCATORTYPE Allocation;


#endif
