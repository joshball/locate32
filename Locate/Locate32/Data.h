#ifndef DATA_H
#define DATA_H

#define SCHEDULE_V1_LEN		0x20

class CSchedule
{
public:
	enum ScheduleType {
		typeDaily=0,
		typeWeekly=1,
		typeMonthly=2,
		typeOnce=3,
		typeAtStartup=4,
		typeHourly=5,
		typeMinutely=6
	};
	enum SchefuleFlags {
		flagEnabled = 0x1,
		flagDeleteAfterRun = 0x2,
		flagRunned = 0x4,
		flagRunnedAtStartup = 0x8,
		flagAtThisTime = 0x10
	};
	struct STIME
	{
		BYTE bHour;
		BYTE bMinute;
		BYTE bSecond;

		STIME& operator=(SYSTEMTIME& st);
		BOOL operator==(const STIME& t) const;
		BOOL operator!=(const STIME& t) const;
		BOOL operator>=(const STIME& t) const;
		BOOL operator<=(const STIME& t) const { return t>=*this; };
		BOOL operator>(const STIME& t) const;
		BOOL operator<(const STIME& t) const { return t>*this; };
		int operator-(const STIME& t) const;
	};

	struct SDATE
	{
		WORD wYear;
		BYTE bMonth; // 1=January
		BYTE bDay; 
		SDATE& operator=(SYSTEMTIME& st);
		BOOL operator==(const SDATE& t) const;
		BOOL operator!=(const SDATE& t) const;
		BOOL operator>=(const SDATE& t) const;
		BOOL operator<=(const SDATE& t) const { return t>=*this; };
		BOOL operator>(const SDATE& t) const;
		BOOL operator<(const SDATE& t) const { return t>*this; };
	};

	struct SMINUTELYTYPE
	{
		WORD wEvery;
	};

	struct SHOURLYTYPE
	{
		WORD wEvery;
		WORD wMinute;
	};
	
	struct SDAILYTYPE
	{
		WORD wEvery;
	};

	struct SWEEKLYTYPE
	{
		enum Days {
			Monday = 0x1,
			Tuesday = 0x2,
			Wednesday = 0x4,
			Thursday = 0x8,
			Friday = 0x10,
			Saturday = 0x20,
			Sunday = 0x40
		};
		WORD wEvery;
		BYTE bDays;
	};

	struct SMONTHLYTYPE
	{
		enum Type {
			Day=1,
			WeekDay=2
		};
		enum Week {
			FirstWeek=0,
			SecondWeek=1,
			ThirdWeek=2,
			FourthWeek=3,
			LastWeek=4
		};

		Type nType;
		Week nWeek;
		BYTE bDay; // 0=monday, 1= tuesday, ...

	};

	class CTimeX : public CTime // Extension for CTime class
	{
	public:
		static DWORD GetIndex(const SDATE& tDate) { return CTime::GetIndex(tDate.bDay,tDate.bMonth,tDate.wYear); }
		static GetDayOfWeek(const SDATE& tDate) { return CTime::GetDayOfWeek(tDate.bDay,tDate.bMonth,tDate.wYear); }

		static int GetWeekIndex(int nDayIndex,BOOL bMondayIsFisrt);
		static int GetWeekIndex(BYTE bDay,BYTE bMonth,WORD wYear,BOOL bMondayIsFirst);
		static int GetWeekIndex(const SDATE& tDate,BOOL bMondayIsFirst) { return GetWeekIndex(tDate.bDay,tDate.bMonth,tDate.wYear,bMondayIsFirst); }
	};

public:
	CSchedule();
	CSchedule(CSchedule* pSchedule);
	CSchedule(BYTE*& pData,BYTE nVersion);
	~CSchedule();

	void GetString(CStringA& rString) const;
	void GetString(CStringW& rString) const;
	
	DWORD WhenShouldRun(STIME& tTime,SDATE& tDate,UINT nDayOfWeek) const;
	static BOOL GetCurrentDateAndTime(SDATE* pDate=NULL,STIME* pTime=NULL,UINT* pnWeekDay=NULL);

public:
	BYTE m_bFlags;
	ScheduleType m_nType;
	STIME m_tStartTime;
	STIME m_tLastStartTime; // If flagRunned is not set, these are 
	SDATE m_tLastStartDate; // time and date when CSchedule is created
	union
	{
		SMINUTELYTYPE m_tMinutely;
		SHOURLYTYPE m_tHourly;
		SDAILYTYPE m_tDaily;
		SWEEKLYTYPE m_tWeekly;
		SMONTHLYTYPE m_tMonthly;
		SDATE m_dStartDate;
	};
	char* m_pDatabases;

	DWORD GetDataLen() const;
	DWORD GetData(BYTE* pData) const;

#ifdef _DEBUG
public:
	inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
};

// Keyboard shortcut
class CShortcut {
public:
	// Flags
	enum Flags {
		sfLocateDialog = 0x00,
		sfGlobalHotkey = 0x01,
		sfGlobalHook = 0x03,
		sfKeyTypeMask = 0x03,

		sfUseMemonic = 0x04,
		sfExecuteWhenUp = 0x08,

		sfRemoveKeyUpMessage = 0x10,
		sfRemoveKeyDownMessage = 0x20
	};
	DWORD m_dwFlags;
	
	// Shortcut
	WORD m_wVirtualKey;
	WORD m_wModifiers;
	enum Modifiers {
		ModifierAlt= MOD_ALT,
		ModifierControl = MOD_CONTROL,
		ModifierWin = MOD_WIN,
		ModifierShift = MOD_SHIFT
	};
	LPSTR m_pClass; // NULL=none, -1==locate dialog
	LPSTR m_pTitle; // NULL=none
	UINT m_nDelay; // 0=none, -1=post, otherwise it is delay in ms
	
	
	// Actions
	class CKeyboardAction {
	public:
		enum Action {
			ActivateControl = 0,
			ActivateTab = 1
		} nAction;
	
		enum ActionActivateControls { // First is control to be activated, second is for memonics
			// Dialog itself
			FindNow = MAKELONG(IDC_OK,IDC_OK),
			Stop = MAKELONG(IDC_STOP,IDC_STOP),
			NewSearch = MAKELONG(IDC_NEWSEARCH,IDC_NEWSEARCH),
			ResultList = MAKELONG(IDC_FILELIST,0),
			Presets = MAKELONG(IDC_PRESETS,IDC_PRESETS),

			// Name and Location
			Name = MAKELONG(IDC_NAME,IDC_NAMESTATIC),
			Type = MAKELONG(IDC_TYPE,IDC_TYPESTATIC),
            LookIn= MAKELONG(IDC_LOOKIN,IDC_LOOKINSTATIC),
			MoreDirectories = MAKELONG(IDC_MOREDIRECTORIES,0),
			Browse = MAKELONG(IDC_BROWSE,IDC_BROWSE),

			// Size and Data
			MinimumSize = MAKELONG(IDC_MINIMUMSIZE,IDC_CHECKMINIMUMSIZE),
			MaximumSize = MAKELONG(IDC_MAXIMUMSIZE,IDC_CHECKMINIMUMSIZE),
			MinimumDate = MAKELONG(IDC_MINDATE,IDC_CHECKMINIMUMSIZE),
			MaximumDate = MAKELONG(IDC_MAXDATE,IDC_CHECKMINIMUMSIZE),
			
			// Advanced
			CheckFilesOrFolders = MAKELONG(IDC_CHECK,IDC_CHECKSTATIC),
			MatchWholeName = MAKELONG(IDC_MATCHWHOLENAME,IDC_MATCHWHOLENAME),
			ReplaceSpaces = MAKELONG(IDC_REPLACESPACES,IDC_REPLACESPACES),
			TypeOfFile = MAKELONG(IDC_FILETYPE,IDC_FILETYPESTATIC),
			ContainingText = MAKELONG(IDC_CONTAINDATACHECK,IDC_CONTAINDATACHECK),
			TextMatchCase = MAKELONG(IDC_DATAMATCHCASE,IDC_DATAMATCHCASE),
			TextHelp = MAKELONG(IDC_HELPTOOLBAR,0)
		};

		enum ActionActivateTabs {
			NameAndLocation = 0,
			SizeAndData = 1,
			Advanced = 2
		};

		union { // Action specifig type
			ActionActivateControls m_nActivateControl;
			ActionActivateTabs m_nActivateTab;
	    };

		CKeyboardAction* m_pNextAction;
	};

	CKeyboardAction* m_pFirstAction;
};

////////////////////////////////////////////////////////////
// Inliners
////////////////////////////////////////////////////////////

inline CSchedule::CSchedule(CSchedule* pSchedule)
{
	sMemCopy(this,pSchedule,sizeof(CSchedule));
	if (m_pDatabases!=NULL)
	{
		DWORD dwLength=1;
		while (*m_pDatabases!='\0')
		{
			int iStrLen=istrlen(m_pDatabases)+1;
			dwLength+=iStrLen;
			m_pDatabases+=iStrLen;
		}
		m_pDatabases=new char[dwLength];
		CopyMemory(m_pDatabases,pSchedule->m_pDatabases,dwLength);
	}
}
	
inline CSchedule::~CSchedule()
{
	if (m_pDatabases!=NULL)
	{
		delete[] m_pDatabases;
		m_pDatabases=NULL;
	}
}

inline CSchedule::STIME& CSchedule::STIME::operator=(SYSTEMTIME& st)
{
	bHour=(BYTE)st.wHour;
	bMinute=(BYTE)st.wMinute;
	bSecond=(BYTE)st.wSecond;
	return *this;
}

inline BOOL CSchedule::STIME::operator==(const STIME& t) const
{
	return (bHour==t.bHour && bMinute==t.bMinute && bSecond==t.bSecond);
}

inline BOOL CSchedule::STIME::operator!=(const STIME& t) const
{
	return !(bHour==t.bHour && bMinute==t.bMinute && bSecond==t.bSecond);
}

inline int CSchedule::STIME::operator-(const STIME& t) const
{
	return (int(bHour)-int(t.bHour))*3600+(int(bMinute)-int(t.bMinute))*60+(int(bSecond)-int(t.bSecond));
}

inline CSchedule::SDATE& CSchedule::SDATE::operator=(SYSTEMTIME& st)
{
	wYear=st.wYear;
	bMonth=(BYTE)st.wMonth;
	bDay=(BYTE)st.wDay;
	return *this;
}

inline BOOL CSchedule::SDATE::operator==(const SDATE& t) const
{
	return (wYear==t.wYear && bMonth==t.bMonth && bDay==t.bDay);
}

inline BOOL CSchedule::SDATE::operator!=(const SDATE& t) const
{
	return !(wYear==t.wYear && bMonth==t.bMonth && bDay==t.bDay);
}

inline int CSchedule::CTimeX::GetWeekIndex(BYTE bDay,BYTE bMonth,WORD wYear,BOOL bMondayIsFirst)
{
	return GetWeekIndex(CTime::GetIndex(bDay,bMonth,wYear),bMondayIsFirst);
}
	
inline int CSchedule::CTimeX::GetWeekIndex(int nDayIndex,BOOL bMondayIsFirst)
{
	if (bMondayIsFirst)
		nDayIndex--;		
	return int(nDayIndex/7);
}
		



#endif