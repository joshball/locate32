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

class CCpuUsage;


// For SetUpdateStatusInformation
#define	DEFAPPICON		(HICON)INVALID_HANDLE_VALUE  

class CLocateAppWnd : public CFrameWnd
{
public:
	
	struct RootInfo {
		LPWSTR pName;
		LPWSTR pRoot;
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
		virtual void OnDestroy();
		virtual void OnNcDestroy();
		virtual void OnTimer(DWORD wTimerID);
		virtual void OnPaint();
		virtual void OnMouseMove(UINT fwKeys,WORD xPos,WORD yPos);
	
		virtual LRESULT WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);
	
		

	public:
		void Update();
		void Update(WORD wThreads,WORD wRunning,RootInfo* pRootInfos);
		void IdleClose();
		virtual BOOL DestroyWindow();
		void CheckForegroundWindow();

		void SetFonts();
		void SetPosition();
		void FormatErrorForStatusTooltip(UpdateError ueError,CDatabaseUpdater* pUpdater);
		void FormatStatusTextLine(CStringW& str,const CLocateAppWnd::RootInfo& pRootInfo,int nThreadID=-1,int nThreads=1);
		static void EnlargeSizeForText(CDC& dc,CStringW& str,CSize& szSize);
		static void EnlargeSizeForText(CDC& dc,LPCWSTR szText,int nLength,CSize& szSize);
		static void FillFontStructs(LOGFONT* pTextFont,LOGFONT* pTitleFont);

			
		CStringW m_sStatusText;
		CArrayFAP<LPWSTR> m_aErrors;
		
		CFont m_TitleFont,m_Font;
		CSize m_WindowSize;

		COLORREF m_cTextColor;
		COLORREF m_cTitleColor;
		COLORREF m_cErrorColor;
		COLORREF m_cBackColor;
		
		CRITICAL_SECTION m_cUpdate;


		// Mouse move
		struct MouseMove {
			SHORT nStartPointX; // Point in client in which cursor is pressed
			SHORT nStartPointY;
		};
		MouseMove* m_pMouseMove;

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
    virtual LRESULT WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);
	
	DWORD OnActivateAnotherInstance(ATOM aCommandLine);
	DWORD OnSystemTrayMessage(UINT uID,UINT msg);
	BOOL SetUpdateStatusInformation(HICON hIcon=NULL,UINT uTip=0,LPCWSTR szText=NULL);
	BOOL GetRootInfos(WORD& wThreads,WORD& wRunning,RootInfo*& pRootInfos);
	static void FreeRootInfos(WORD wThreads,RootInfo* pRootInfos);

	void AddTaskbarIcon();
	void DeleteTaskbarIcon();
	void LoadAppIcon();
	
	DWORD SetSchedules(CList<CSchedule*>* pSchedules=NULL);
	BOOL SaveSchedules();
	void CheckSchedules();
	BOOL RunStartupSchedules(); // return value = Should run again?

	BOOL StartUpdateStatusNotification();
	BOOL StopUpdateStatusNotification();
	void NotifyFinishingUpdating();

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
	BYTE OnUpdate(BOOL bStopIfProcessing,LPWSTR pDatabases=NULL); 
	BYTE OnUpdate(BOOL bStopIfProcessing,LPWSTR pDatabases,int nThreadPriority); 

	static DWORD WINAPI KillUpdaterProc(LPVOID lpParameter);
    
public:
	
public:
	CMenu m_Menu;
	CAboutDlg* m_pAbout;
	CSettingsProperties* m_pSettings;
	

	CLocateDlgThread* m_pLocateDlgThread;
	CUpdateStatusWnd* m_pUpdateStatusWnd;
	
	HICON m_hAppIcon;
	HICON* m_pUpdateAnimIcons;
	int m_nCurUpdateAnimBitmap;
	CRITICAL_SECTION m_csAnimBitmaps;

	CListFP <CSchedule*> m_Schedules;
	CCpuUsage* m_pCpuUsage;

	// Keyboard shortcuts
	CArrayFP<CShortcut*> m_aShortcuts;
	HHOOK m_hHook;


	
	friend inline CLocateDlg* GetLocateWnd();

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

		LPWSTR m_pStartPath;
		LPWSTR m_pStartString;
		LPWSTR m_pTypeString;
		LPWSTR m_pFindText;
		LPWSTR m_pLoadPreset;
		LPWSTR m_pSettingBranch;

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
		SHORT m_nActivateInstance; // 0 not set, -1 first instance, X instance no

		CArrayFP<CDatabase*> m_aDatabases;



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
		pfUpdateTooltipPositionLastPosition = 0x400,
		pfUpdateTooltipPositionUpLeft = pfUpdateTooltipPositionUp|pfUpdateTooltipPositionLeft,
		pfUpdateTooltipPositionUpRight = pfUpdateTooltipPositionUp|pfUpdateTooltipPositionRight,
		pfUpdateTooltipPositionDownLeft = pfUpdateTooltipPositionDown|pfUpdateTooltipPositionLeft,
		pfUpdateTooltipPositionDownRight = pfUpdateTooltipPositionDown|pfUpdateTooltipPositionRight,
		pfUpdateTooltipPositionMask = 0x0780,
        pfTooltipMask = 0x7F0,

		pfTooltipDefault = pfEnableUpdateTooltip|pfUpdateTooltipAlwaysTopmost|pfUpdateTooltipPositionDefault,
		pfTooltipSave = pfEnableUpdateTooltip|pfUpdateTooltipTopmostMask|pfUpdateTooltipPositionMask,

		// Misc
		pfTrayIconClickActivate = 0x1000,
		pfUseDefaultIconForDirectories =  0x2000,
		pfMiscDefault = 0,
		pfMiscSave = 0x3000,

		// Filesize/time/date format
		pfFormatUseLocaleFormat = 0x4, // Option
		pfFormatMask = 0x4,
		pfFormatDefault = pfFormatUseLocaleFormat,
		pfFormatSave = pfFormatUseLocaleFormat,

		pfDefault = pfErrorDefault|pfTooltipDefault|pfFormatDefault|pfMiscDefault,
		pfSave = pfErrorSave|pfTooltipSave|pfFormatSave|pfMiscSave
	};

	enum FileSizeFormats {
		fsfOverKBasKB = 0,
		fsfBestUnit = 1,
		fsfBytes = 2,
		fsfBytesNoUnit = 3,
		fsfKiloBytes = 4,
		fsfKiloBytesNoUnit = 5,
		fsfMegaBytesMegaBytes = 6,
		fsfMegaBytesMegaBytesNoUnit = 7,
		fsfOverMBasMB = 8
	};

	

protected:
	BYTE CheckDatabases();
	BYTE SetDeleteAndDefaultImage();
	
	
public:
	static BOOL ParseParameters(LPCWSTR lpCmdLine,CStartData* pStartData);
	static BOOL CALLBACK EnumLocateSTWindows(HWND hwnd,LPARAM lParam);

	BOOL ActivateOtherInstances(LPCWSTR pCmdLine);
	
	void SaveRegistry() const;
	void LoadRegistry();
	BOOL UpdateSettings();

	LPWSTR FormatDateAndTimeString(WORD wDate,WORD wTime=WORD(-1));
	LPWSTR FormatFileSizeString(DWORD dwFileSizeLo,DWORD bFileSizeHi) const;
	BOOL SetLanguageSpecifigHandles();


	BOOL StopUpdating(BOOL bForce=TRUE);
    BOOL IsUpdating() const;
	
	
	BOOL GlobalUpdate(CArray<PDATABASE>* paDatabases=NULL,int nThreadPriority=THREAD_PRIORITY_NORMAL);

	

	// Database menu functions
	static BOOL IsDatabaseMenu(HMENU hMenu);
	void OnInitDatabaseMenu(CMenu& PopupMenu);
	void OnDatabaseMenuItem(WORD wID);
	static int GetDatabaseMenuIndex(HMENU hPopupMenu);

	static DWORD GetProgramFlags();
    	
private:
	static BOOL CALLBACK UpdateProc(DWORD_PTR dwParam,CallingReason crReason,UpdateError ueCode,CDatabaseUpdater* pUpdater);
	

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
	CStringW m_strDateFormat;
	CStringW m_strTimeFormat;
	FileSizeFormats m_nFileSizeFormat;


	DWORD(WINAPI* m_pGetLongPathName)(LPCWSTR,LPWSTR,DWORD);

	static DWORD WINAPI GetLongPathName(LPCWSTR lpszShortPath,LPWSTR lpszLongPath,DWORD cchBuffer);
	static DWORD WINAPI GetLongPathNameNoUni(LPCWSTR lpszShortPath,LPWSTR lpszLongPath,DWORD cchBuffer);

	UINT m_nInstance;

	// Registered messages
	static UINT m_nHFCInstallationMessage;
	static UINT m_nTaskbarCreated;
	static UINT m_nLocateAppMessage;


	// Locale number information
	struct LocaleNumberFormat {
		LocaleNumberFormat();		
		~LocaleNumberFormat()
		{
			delete[] pDecimal;
			delete[] pThousand;
		}
			
		UINT uLeadingZero;
		UINT uGrouping;
		WCHAR* pDecimal;
		WCHAR* pThousand;
	} *m_pLocaleNumberFormat;
	

protected:
	CStartData* m_pStartData;
	CLocateAppWnd m_AppWnd;

	BYTE m_nStartup;
	
	CRITICAL_SECTION m_cUpdatersPointersInUse;
	CDatabaseUpdater** m_ppUpdaters;
	

	
	CArrayFP<CDatabase*> m_aDatabases;
	mutable CDatabase* m_pLastDatabase;

	static void ChangeAndAlloc(LPWSTR& pVar,LPCWSTR szText);
	static void ChangeAndAlloc(LPWSTR& pVar,LPCWSTR szText,DWORD dwLength);





	BOOL InitCommonRegKey();
	void FinalizeCommonRegKey();

	LPSTR m_szCommonRegKey;
	LPSTR m_szCommonRegFile;
	BYTE m_bFileIsReg;
	

public:
	static CAutoPtrA<CHAR> GetRegKey(LPCSTR szSubKey);
	
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
	
	static int ErrorBox(int nError,UINT uType=MB_ICONERROR|MB_OK);
	static int ErrorBox(LPCWSTR szError,UINT uType=MB_ICONERROR|MB_OK);
	static LPWSTR FormatLastOsError();
	
	const CDatabase* GetDatabase(WORD wID) const;

	static INT_PTR CALLBACK DummyDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

	friend CLocateAppWnd;
	friend CLocateAppWnd::CUpdateStatusWnd;
	friend CLocateAppWnd* GetLocateAppWnd();
	friend CLocateDlg* GetLocateDlg();
	friend CRegKey2;

};

inline CLocateApp::CStartData::CStartData()
:	m_nStatus(0),m_nStartup(0),m_dwMaxFoundFiles(DWORD(-1)),
	m_dwMaxFileSize(DWORD(-1)),m_dwMinFileSize(DWORD(-1)),
    m_dwMaxDate(DWORD(-1)),m_dwMinDate(DWORD(-1)),
    m_cMaxSizeType(0),m_cMinSizeType(0),
    m_cMaxDateType(0),m_cMinDateType(0),
    m_nSorting(BYTE(-1)),m_nPriority(priorityDontChange),
	m_pStartPath(NULL),m_pStartString(NULL),
	m_pTypeString(NULL),m_pFindText(NULL),m_pLoadPreset(NULL),
	m_nActivateInstance(0),m_pSettingBranch(NULL)
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
	if (m_pSettingBranch!=NULL)
		delete[] m_pSettingBranch;

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



inline BOOL CLocateApp::IsUpdating() const
{
	return m_ppUpdaters!=NULL;
}



inline void CLocateApp::ChangeAndAlloc(LPWSTR& pVar,LPCWSTR szText)
{
	DWORD dwLength=istrlenw(szText);
	if (pVar!=NULL)
		delete[] pVar;
	pVar=new WCHAR [dwLength+1];
	MemCopyW(pVar,szText,dwLength+1);
}

inline void CLocateApp::ChangeAndAlloc(LPWSTR& pVar,LPCWSTR szText,DWORD dwLength)
{
	if (dwLength==DWORD(-1))
		dwLength=(DWORD)istrlenw(szText);
	
	if (pVar!=NULL)
		delete[] pVar;
	pVar=new WCHAR [dwLength+1];
	MemCopyW(pVar,szText,dwLength);
	pVar[dwLength]='\0';
}

inline DWORD CLocateApp::GetProgramFlags()
{
	extern CLocateApp theApp;
	return theApp.m_dwProgramFlags;
}



inline void CLocateAppWnd::CUpdateStatusWnd::EnlargeSizeForText(CDC& dc,CStringW& str,CSize& szSize)
{
	EnlargeSizeForText(dc,str,(int)str.GetLength(),szSize);
}

inline void CLocateAppWnd::CUpdateStatusWnd::EnlargeSizeForText(CDC& dc,LPCWSTR szText,int nLength,CSize& szSize)
{
	CRect rc(0,0,0,0);
	dc.DrawText(szText,nLength,&rc,DT_SINGLELINE|DT_CALCRECT);
	if (szSize.cx<rc.Width())
		szSize.cx=rc.Width();
	if (szSize.cy<rc.Height())
		szSize.cy=rc.Height();
}

inline BYTE CLocateAppWnd::OnUpdate(BOOL bStopIfProcessing,LPWSTR pDatabases)
{
	DWORD nThreadPriority=THREAD_PRIORITY_NORMAL;

	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::defRead)==ERROR_SUCCESS)
		RegKey.QueryValue("Update Process Priority",nThreadPriority);
	
	return OnUpdate(bStopIfProcessing,pDatabases,(int)nThreadPriority);
}



inline CString CRegKey2::GetCommonKey()
{
	extern CLocateApp theApp;
	return CString(theApp.m_szCommonRegKey)+="\\Locate32";
}

inline CStringW CRegKey2::GetCommonKeyW()
{
	extern CLocateApp theApp;
	return CStringW(theApp.m_szCommonRegKey)+="\\Locate32";
}


#endif
