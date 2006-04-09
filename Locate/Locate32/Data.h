#ifndef DATA_H
#define DATA_H

#define SCHEDULE_V1_LEN		0x20
#define SCHEDULE_V2_LEN		0x24
#define SCHEDULE_V3_LEN		0x28

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
		static int GetDayOfWeek(const SDATE& tDate) { return CTime::GetDayOfWeek(tDate.bDay,tDate.bMonth,tDate.wYear); }

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
	WCHAR* m_pDatabases;
	int m_nThreadPriority;

	DWORD GetDataLen() const;
	DWORD GetData(BYTE* pData) const;

#ifdef _DEBUG
public:
	inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
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
			int iStrLen=istrlenw(m_pDatabases)+1;
			dwLength+=iStrLen;
			m_pDatabases+=iStrLen;
		}
		m_pDatabases=new WCHAR[dwLength];
		CopyMemory(m_pDatabases,pSchedule->m_pDatabases,dwLength*2);
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