#include <HFCLib.h>
#include "Locate32.h"

/////////////////////////////////////////////
// CSchedule

CSchedule::CSchedule()
:	m_bFlags(flagEnabled|flagRunnedAtStartup),m_nType(typeDaily),m_pDatabases(NULL)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	m_tStartTime=st;
	m_tLastStartDate=st;
	m_tLastStartTime=st;
	m_tDaily.wEvery=1;
}

CSchedule::CSchedule(BYTE*& pData,BYTE nVersion)
{
	if (nVersion==1)
	{
		sMemCopy(this,pData,SCHEDULE_V1_LEN);
		m_pDatabases=NULL;
		pData+=SCHEDULE_V1_LEN;
	}
	else if (nVersion==2)
	{
		sMemCopy(this,pData,sizeof(CSchedule));
		pData+=sizeof(CSchedule);
		if (m_pDatabases==NULL)
		{
			pData++;
			return;
		}
        DWORD dwLength=1;
        BYTE* pOrig=pData;
        while (*pData!='\0')
		{
			int iStrLen=istrlen(LPSTR(pData))+1;
			dwLength+=iStrLen;
			pData+=iStrLen;
		}
		pData++;

		m_pDatabases=new char[dwLength];
		CopyMemory(m_pDatabases,pOrig,dwLength);
	}
		
	
	if (m_tLastStartDate.wYear<1900) // There is something wrong
		GetCurrentDateAndTime(&m_tLastStartDate,&m_tLastStartTime);
	if (m_nType==typeAtStartup)
		m_bFlags&=~flagRunnedAtStartup;
}

DWORD CSchedule::GetDataLen() const
{
	if (m_pDatabases==NULL)
		return sizeof(CSchedule)+1;
    	
	DWORD dwLength=sizeof(CSchedule)+1;
	LPSTR pPtr=m_pDatabases;
	while (*pPtr!='\0')
	{
        int iStrLen=istrlen(pPtr)+1;
		dwLength+=iStrLen;
		pPtr+=iStrLen;
	}
	return dwLength;
}

DWORD CSchedule::GetData(BYTE* pData) const
{
	CopyMemory(pData,this,sizeof(CSchedule));
	pData+=sizeof(CSchedule);
	if (m_pDatabases==NULL)
	{
		*pData='\0';
		return sizeof(CSchedule)+1;
	}
	
	DWORD dwLength=sizeof(CSchedule)+1;
	LPSTR pPtr=m_pDatabases;
	while (*pPtr!='\0')
	{
		int iStrLen=istrlen(pPtr)+1;
		CopyMemory(pData,pPtr,iStrLen);
        dwLength+=iStrLen;
		pPtr+=iStrLen;
		pData+=iStrLen;
	}
    *pData='\0';
	return dwLength;
}


void CSchedule::GetString(CStringA& str) const
{
	CStringA time;
	SYSTEMTIME st;
	GetLocalTime(&st);
	st.wHour=m_tStartTime.bHour;
	st.wMinute=m_tStartTime.bMinute;
	st.wSecond=m_tStartTime.bSecond;
	
	str.Empty();
	GetTimeFormatA(LOCALE_USER_DEFAULT,0,&st,NULL,time.GetBuffer(1000),1000);

	time.FreeExtra();
	switch (m_nType)
	{
	case typeMinutely:
		if (m_tMinutely.wEvery==1)
			str.LoadString(IDS_MINUTELYEVERYMINUTE);
		else
			str.FormatEx(IDS_MINUTELYEVERYXMINUTE,(int)m_tMinutely.wEvery);
		break;
	case typeHourly:
		if (m_tHourly.wEvery==1)
			str.FormatEx(IDS_HOURLYLYEVERYHOUR,(int)m_tHourly.wMinute);
		else
			str.FormatEx(IDS_HOURLYLYEVERYXHOUR,(int)m_tHourly.wEvery,(int)m_tHourly.wMinute);
		break;
	case typeDaily:
		if (m_tDaily.wEvery==1)
			str.FormatEx(IDS_DAILYEVERYDAY,(LPCSTR)time);
		else
			str.FormatEx(IDS_DAILYEVERYXDAYS,(LPCSTR)time,(int)m_tDaily.wEvery);
		break;
	case typeWeekly:
		{
			CStringA days;
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Monday)
			{
				char szDate[10];
				LoadStringA(GetResourceHandle(LanguageSpecificResource),IDS_MON,szDate,10);
				days << szDate << ' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Tuesday)
			{
				char szDate[10];
				LoadStringA(GetResourceHandle(LanguageSpecificResource),IDS_TUE,szDate,10);
				days << szDate << ' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Wednesday)
			{
				char szDate[10];
				LoadStringA(GetResourceHandle(LanguageSpecificResource),IDS_WED,szDate,10);
				days << szDate << ' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Thursday)
			{
				char szDate[10];
				LoadStringA(GetResourceHandle(LanguageSpecificResource),IDS_THU,szDate,10);
				days << szDate << ' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Friday)
			{
				char szDate[10];
				LoadStringA(GetResourceHandle(LanguageSpecificResource),IDS_FRI,szDate,10);
				days << szDate << ' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Saturday)
			{
				char szDate[10];
				LoadStringA(GetResourceHandle(LanguageSpecificResource),IDS_SAT,szDate,10);
				days << szDate << ' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Sunday)
			{
				char szDate[10];
				LoadStringA(GetResourceHandle(LanguageSpecificResource),IDS_SUN,szDate,10);
				days << szDate << ' ';
			}
			
			if (m_tWeekly.wEvery==1)
				str.FormatEx(IDS_WEEKLYEVERYWEEK,(LPCSTR)time,(LPCSTR)days);
			else
				str.FormatEx(IDS_WEEKLYEVERYXWEEKS,(LPCSTR)time,(LPCSTR)days,(int)m_tWeekly.wEvery);	
			break;
		}
	case typeMonthly:
		if (m_tMonthly.nType==CSchedule::SMONTHLYTYPE::Type::Day)
			str.Format(IDS_MONTHLYEVERYDAY,(LPCSTR)time,(int)m_tMonthly.bDay);
		else
		{
			char day[10];
			CStringA type;
			type.LoadString(IDS_FIRST+m_tMonthly.nWeek);
			type.MakeLower();
			st.wYear=1999;
			st.wMonth=8;
			st.wDay=2+m_tMonthly.bDay;
			LoadStringA(GetResourceHandle(LanguageSpecificResource),IDS_MONDAY+m_tMonthly.bDay,day,10);
			str.FormatEx(IDS_MONTHLYEVERYTYPE,(LPCSTR)time,(LPCSTR)type,day);
		}
		break;
	case typeOnce:
		{
			CStringA date;
			st.wYear=m_dStartDate.wYear;
			st.wMonth=m_dStartDate.bMonth;
			st.wDay=m_dStartDate.bDay;
			GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,NULL,date.GetBuffer(100),100);
			str.FormatEx(IDS_ATTIMEON,(LPCSTR)time,(LPCSTR)date);
			break;
		}
	case typeAtStartup:
		str.LoadString(IDS_ATSTARTUPSTR);
		break;
	}
	if (m_bFlags&flagRunned &&
		!(m_tLastStartTime.bHour==0 && m_tLastStartTime.bMinute==0 && m_tLastStartTime.bSecond==0 &&
		m_tLastStartDate.wYear<1995 && m_tLastStartDate.bMonth==0 && m_tLastStartDate.bDay==0))
	{
		SYSTEMTIME st;
		st.wYear=m_tLastStartDate.wYear;
		st.wMonth=m_tLastStartDate.bMonth;
		st.wDay=m_tLastStartDate.bDay;
		st.wHour=m_tLastStartTime.bHour;
		st.wMinute=m_tLastStartTime.bMinute;
		st.wSecond=m_tLastStartTime.bSecond;
		st.wMilliseconds=0;
		CString tmp;
		char szDate[100];
		char szTime[100];
		GetTimeFormat(LOCALE_USER_DEFAULT,0,&st,NULL,szTime,100);
		GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,NULL,szDate,100);
		tmp.Format(IDS_LASTRUN,szDate,szTime);
		str << tmp;
	}
}


void CSchedule::GetString(CStringW& str) const
{
	CStringW time;
	SYSTEMTIME st;
	GetLocalTime(&st);
	st.wHour=m_tStartTime.bHour;
	st.wMinute=m_tStartTime.bMinute;
	st.wSecond=m_tStartTime.bSecond;
	
	str.Empty();
	GetTimeFormatW(LOCALE_USER_DEFAULT,0,&st,NULL,time.GetBuffer(1000),1000);

	time.FreeExtra();
	switch (m_nType)
	{
	case typeMinutely:
		if (m_tMinutely.wEvery==1)
			str.LoadString(IDS_MINUTELYEVERYMINUTE);
		else
			str.FormatEx(IDS_MINUTELYEVERYXMINUTE,(long)m_tMinutely.wEvery);
		break;
	case typeHourly:
		if (m_tHourly.wEvery==1)
			str.FormatEx(IDS_HOURLYLYEVERYHOUR,(long)m_tHourly.wMinute);
		else
			str.FormatEx(IDS_HOURLYLYEVERYXHOUR,(long)m_tHourly.wEvery,(long)m_tHourly.wMinute);
		break;
	case typeDaily:
		if (m_tDaily.wEvery==1)
			str.FormatEx(IDS_DAILYEVERYDAY,(LPCWSTR)time);
		else
			str.FormatEx(IDS_DAILYEVERYXDAYS,(LPCWSTR)time,(long)m_tDaily.wEvery);
		break;
	case typeWeekly:
		{
			CStringW days;
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Monday)
			{
				WCHAR szDate[10];
				/*st.wYear=1999;
				st.wMonth=8;
				st.wDay=2;
				if (!GetDateFormatW(LOCALE_USER_DEFAULT,0,&st,L"ddd",szDate,10))
				{*/
					LoadStringW(GetResourceHandle(LanguageSpecificResource),IDS_MON,szDate,10);
					//szDate[3]='\0';
					//CharLowerW(szDate);
				//}
				days << szDate << L' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Tuesday)
			{
				WCHAR szDate[10];
				/*st.wYear=1999;
				st.wMonth=8;
				st.wDay=3;
				if (!GetDateFormatW(LOCALE_USER_DEFAULT,0,&st,L"ddd",szDate,10))
				{*/
					LoadStringW(GetResourceHandle(LanguageSpecificResource),IDS_TUE,szDate,10);
					//szDate[3]='\0';
					//CharLowerW(szDate);
				//}
				days << szDate << L' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Wednesday)
			{
				WCHAR szDate[10];
				/*st.wYear=1999;
				st.wMonth=8;
				st.wDay=4;
				if (!GetDateFormatW(LOCALE_USER_DEFAULT,0,&st,L"ddd",szDate,10))
				{*/
					LoadStringW(GetResourceHandle(LanguageSpecificResource),IDS_WED,szDate,10);
					//szDate[3]='\0';
					//CharLowerW(szDate);
				//}
				days << szDate << L' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Thursday)
			{
				WCHAR szDate[10];
				/*st.wYear=1999;
				st.wMonth=8;
				st.wDay=5;
				if (!GetDateFormatW(LOCALE_USER_DEFAULT,0,&st,L"ddd",szDate,10))
				{*/
					LoadStringW(GetResourceHandle(LanguageSpecificResource),IDS_THU,szDate,10);
					//szDate[3]='\0';
					//CharLowerW(szDate);
				//}
				days << szDate << L' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Friday)
			{
				WCHAR szDate[10];
				/*st.wYear=1999;
				st.wMonth=8;
				st.wDay=6;
				if (!GetDateFormatW(LOCALE_USER_DEFAULT,0,&st,L"ddd",szDate,10))
				{*/
					LoadStringW(GetResourceHandle(LanguageSpecificResource),IDS_FRI,szDate,10);
					//szDate[3]='\0';
					//CharLowerW(szDate);
				//}
				days << szDate << L' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Saturday)
			{
				WCHAR szDate[10];
				/*st.wYear=1999;
				st.wMonth=8;
				st.wDay=7;
				if (!GetDateFormatW(LOCALE_USER_DEFAULT,0,&st,L"ddd",szDate,10))
				{*/
					LoadStringW(GetResourceHandle(LanguageSpecificResource),IDS_SAT,szDate,10);
					//szDate[3]='\0';
					//CharLowerW(szDate);
				//}
				days << szDate << L' ';
			}
			if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Sunday)
			{
				WCHAR szDate[10];
				/*st.wYear=1999;
				st.wMonth=8;
				st.wDay=1;
				if (!GetDateFormatW(LOCALE_USER_DEFAULT,0,&st,L"ddd",szDate,10))
				{*/
					LoadStringW(GetResourceHandle(LanguageSpecificResource),IDS_SUN,szDate,10);
					//szDate[3]='\0';
					//CharLowerW(szDate);
				//}
				days << szDate << L' ';
			}
			
			if (m_tWeekly.wEvery==1)
				str.FormatEx(IDS_WEEKLYEVERYWEEK,(LPCWSTR)time,(LPCWSTR)days);
			else
				str.FormatEx(IDS_WEEKLYEVERYXWEEKS,(LPCWSTR)time,(LPCWSTR)days,(int)m_tWeekly.wEvery);	
			break;
		}
	case typeMonthly:
		if (m_tMonthly.nType==CSchedule::SMONTHLYTYPE::Type::Day)
			str.FormatEx(IDS_MONTHLYEVERYDAY,(LPCWSTR)time,(int)m_tMonthly.bDay);
		else
		{
			WCHAR day[10];
			CStringW type;
			type.LoadString(IDS_FIRST+m_tMonthly.nWeek);
			type.MakeLower();
			st.wYear=1999;
			st.wMonth=8;
			st.wDay=2+m_tMonthly.bDay;
			LoadStringW(GetResourceHandle(LanguageSpecificResource),IDS_MONDAY+m_tMonthly.bDay,day,10);
			str.FormatEx(IDS_MONTHLYEVERYTYPE,(LPCWSTR)time,(LPCWSTR)type,day);
		}
		break;
	case typeOnce:
		{
			CStringW date;
			st.wYear=m_dStartDate.wYear;
			st.wMonth=m_dStartDate.bMonth;
			st.wDay=m_dStartDate.bDay;
			GetDateFormatW(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,NULL,date.GetBuffer(100),100);
			str.FormatEx(IDS_ATTIMEON,(LPCWSTR)time,(LPCWSTR)date);
			break;
		}
	case typeAtStartup:
		str.LoadString(IDS_ATSTARTUPSTR);
		break;
	}
	if (m_bFlags&flagRunned &&
		!(m_tLastStartTime.bHour==0 && m_tLastStartTime.bMinute==0 && m_tLastStartTime.bSecond==0 &&
		m_tLastStartDate.wYear<1995 && m_tLastStartDate.bMonth==0 && m_tLastStartDate.bDay==0))
	{
		SYSTEMTIME st;
		st.wYear=m_tLastStartDate.wYear;
		st.wMonth=m_tLastStartDate.bMonth;
		st.wDay=m_tLastStartDate.bDay;
		st.wHour=m_tLastStartTime.bHour;
		st.wMinute=m_tLastStartTime.bMinute;
		st.wSecond=m_tLastStartTime.bSecond;
		st.wMilliseconds=0;
		CStringW tmp;
		WCHAR szDate[100];
		WCHAR szTime[100];
		GetTimeFormatW(LOCALE_USER_DEFAULT,0,&st,NULL,szTime,100);
		GetDateFormatW(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,NULL,szDate,100);
		tmp.FormatEx(IDS_LASTRUN,szDate,szTime);
		str << tmp;
	}
}

BOOL CSchedule::GetCurrentDateAndTime(SDATE* pDate,STIME* pTime,UINT* pnWeekDay)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	if (pDate!=NULL)
		*pDate=st;
	if (pTime!=NULL)
		*pTime=st;
	if (pnWeekDay!=NULL)
		*pnWeekDay=st.wDayOfWeek;
	return TRUE;
}

DWORD CSchedule::WhenShouldRun(STIME& tTime,SDATE& tDate,UINT nDayOfWeek) const
{
	if (!(m_bFlags&flagEnabled))
		return (DWORD)-1;
	ASSERT(m_tLastStartDate.wYear>1900);
			
	switch (m_nType)
	{
	case typeMinutely:
		{
			// Computing difference of time in minutes
			
			// Minutes in day=24*60
			int dMinutes=(CTimeX::GetIndex(tDate)-CTimeX::GetIndex(m_tLastStartDate))*(24*60)+
				(int(tTime.bHour)-int(m_tLastStartTime.bHour))*60+
                int(tTime.bMinute)-int(m_tLastStartTime.bMinute);
			
			if (dMinutes>=m_tMinutely.wEvery)
				return 0;
			return (DWORD)-1;
		}
	case typeHourly:
		{
			int dDays=CTimeX::GetIndex(tDate)-CTimeX::GetIndex(m_tLastStartDate);
			if (dDays>1)
			{
				// Runned at least 2 days ago
				if (m_bFlags&flagAtThisTime)
					return tTime.bMinute==m_tHourly.wMinute?0:(DWORD)-1;
				return 0;
			}
			else if (dDays==1)
			{
				// Next day
				if (int(tTime.bHour)>int(m_tLastStartTime.bHour)-24+m_tHourly.wEvery)
				{
					if (m_bFlags&flagAtThisTime)
						return tTime.bMinute==m_tHourly.wMinute?0:(DWORD)-1;
					return 0;
				}
				else if (int(tTime.bHour)==int(m_tLastStartTime.bHour)-24+m_tHourly.wEvery)
				{
					if (m_bFlags&flagAtThisTime)
						return tTime.bMinute==m_tHourly.wMinute?0:(DWORD)-1;
					return tTime.bMinute>m_tHourly.wMinute?0:(DWORD)-1;
				}
				return (DWORD)-1;
			}
			if (tTime.bHour>m_tLastStartTime.bHour+m_tHourly.wEvery)
			{
				if (m_bFlags&flagAtThisTime)
					return tTime.bMinute==m_tHourly.wMinute?0:(DWORD)-1;
				return 0;
			}
			else if (tTime.bHour==m_tLastStartTime.bHour+m_tHourly.wEvery)
			{
				if (m_bFlags&flagAtThisTime)
					return tTime.bMinute==m_tHourly.wMinute?0:(DWORD)-1;
				return tTime.bMinute>=m_tHourly.wMinute?0:(DWORD)-1;
			}
			return (DWORD)-1;
		}
	case typeDaily: // Checked 140103, OK
		{
			if (m_bFlags&flagRunned) // This has been runned before, 
									 // thus there should be at least one day between new run
			{
				int dx=CTimeX::GetIndex(tDate)-CTimeX::GetIndex(m_tLastStartDate);
				if (m_bFlags&flagAtThisTime)
				{
					if (dx>=(int)m_tDaily.wEvery)
					{
						int diff=tTime-m_tStartTime;
						if (diff>=0 && diff<60)
							return 0;
					}
				}
				else
				{
					if ((dx==(int)m_tDaily.wEvery && tTime>=m_tStartTime) ||
						dx>(int)m_tDaily.wEvery)
						return 0;
				}
			}
			else
			{
				ASSERT(m_tLastStartDate<=tDate);
				if (m_bFlags&flagAtThisTime)
				{
					int diff=tTime-m_tStartTime;
					if (diff>0 && diff<60)
						return 0;
				}
				else
				{
					
					if ((m_tLastStartDate<tDate &&  tTime>=m_tStartTime) ||
						(m_tLastStartTime<m_tStartTime && tTime>=m_tStartTime))
						return 0;
				}
			}
			return (DWORD)-1;
		}
	case typeWeekly: // Checked 140103, Should be OK
		{
			if (m_tWeekly.bDays==0)
				return (DWORD)-1;
			if (tDate==m_tLastStartDate && // This has been runned at this day
				(m_bFlags&flagRunned && m_tLastStartTime>m_tStartTime))
				return (DWORD)-1;
			BOOL bIsMondayFirst='0';
			GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IFIRSTDAYOFWEEK,(LPSTR)&bIsMondayFirst,2);
			bIsMondayFirst=bIsMondayFirst=='0'?TRUE:FALSE;


			if (m_bFlags&flagRunned)
			{
				int nWeekDiff=CTimeX::GetWeekIndex(tDate,bIsMondayFirst)-CTimeX::GetWeekIndex(m_tLastStartDate,bIsMondayFirst);
				ASSERT(nWeekDiff>=0);
				if (nWeekDiff>0 && nWeekDiff<m_tWeekly.wEvery)
					return (DWORD)-1;
			}
			
			switch (nDayOfWeek)
			{
			case 0:
				if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Sunday)
					break;
				return (DWORD)-1;
			case 1:
				if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Monday)
					break;
				return (DWORD)-1;
			case 2:
				if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Tuesday)
					break;
				return (DWORD)-1;
			case 3:
				if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Wednesday)
					break;
				return (DWORD)-1;
			case 4:
				if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Thursday)
					break;
				return (DWORD)-1;
			case 5:
				if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Friday)
					break;
				return (DWORD)-1;
			case 6:
				if (m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Saturday)
					break;
				return (DWORD)-1;
			}
			int diff=tTime-m_tStartTime;
			if (diff>0)
			{
				if (!(m_bFlags&flagAtThisTime) || diff<60)
					return 0;
			}
			return (DWORD)-1;
		}
	case typeMonthly: // Checked 140103, Should be OK
		{
			ASSERT(tDate>=m_tLastStartDate);

			if (m_bFlags&flagRunned && // Runned in this month?
				tDate.wYear==m_tLastStartDate.wYear && 
				tDate.bMonth==m_tLastStartDate.bMonth)
				return (DWORD)-1;

			if (m_tMonthly.nType==CSchedule::SMONTHLYTYPE::Type::Day)
			{
				if (m_tMonthly.bDay==tDate.bDay)
				{
					int diff=tTime-m_tStartTime;
					if (diff>0)
					{
						if (!(m_bFlags&flagAtThisTime) || diff<60)
							return 0;
					}
					return (DWORD)-1;
				}
			}
			else
			{
				BOOL bIsMondayFirst='0';
				GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IFIRSTDAYOFWEEK,(LPSTR)&bIsMondayFirst,2);
				bIsMondayFirst=bIsMondayFirst=='0'?TRUE:FALSE;
				BYTE bDay=m_tMonthly.bDay==6?0:m_tMonthly.bDay+1;
				if (nDayOfWeek!=bDay)
					return (DWORD)-1;

				int nCurrentWeek=CTimeX::GetWeekIndex(tDate,bIsMondayFirst);
				int nWeekIndex;
				if (m_tMonthly.nWeek==SMONTHLYTYPE::LastWeek)
				{
					BYTE nLastDay=CTime::GetDaysInMonth(tDate.bMonth,tDate.wYear);
					int nLastDayIndex=CTime::GetIndex(nLastDay,tDate.bMonth,tDate.wYear);
					nWeekIndex=CTimeX::GetWeekIndex(nLastDayIndex,bIsMondayFirst);
					if (nLastDayIndex%7==0 && !bIsMondayFirst) // Last week contains only synday
					{
						if (bDay!=0)
							nWeekIndex--;
					}
					else
					{
						if (nLastDayIndex%7<bDay) // Last week does not have required day
							nWeekIndex--;
					}
				}
				else
				{
					int nFirstDayIndex=CTime::GetIndex(1,tDate.bMonth,tDate.wYear);
					nWeekIndex=CTimeX::GetWeekIndex(nFirstDayIndex,bIsMondayFirst)+m_tMonthly.nWeek;
					
					if (nFirstDayIndex%7==0 && bIsMondayFirst) // First week contains only sunday
					{
						if (bDay!=0)
							nWeekIndex++;
					}
					else
					{
						if (nFirstDayIndex%7>bDay) // First week does not have required day
							nWeekIndex++;	
					}
				}
				if (nWeekIndex!=nCurrentWeek)
					return (DWORD)-1;
			}
			int diff=tTime-m_tStartTime;
			if (diff>0)
			{
				if (!(m_bFlags&flagAtThisTime) || diff<60)
					return 0;
			}
			break;
		}
	case typeOnce: // Checked 140103, OK
		if (m_bFlags&flagRunned)
			return (DWORD)-1;
		if (tDate==m_dStartDate)
		{
			int diff=tTime-m_tStartTime;
			if (diff>0)
			{
				if (!(m_bFlags&flagAtThisTime) || diff<60)
					return 0;
			}
			return (DWORD)-1;
		}
		else if (tDate>=m_dStartDate && !(m_bFlags&flagAtThisTime))
			return 0;
		else
			return (DWORD)-1;
		break;		
	case typeAtStartup: // Checked 140103, OK
		if (!(m_bFlags&flagRunnedAtStartup))
			return 0;
		return (DWORD)-1;
	}
	return (DWORD)-1;
}
	
BOOL CSchedule::STIME::operator>=(const STIME& t) const
{
	if (bHour>t.bHour)
		return TRUE;
	if (bHour<t.bHour)
		return FALSE;
	if (bMinute>t.bMinute)
		return TRUE;
	if (bMinute<t.bMinute)
		return FALSE;
	if (bSecond>=t.bSecond)
		return TRUE;
	return FALSE;
}

BOOL CSchedule::STIME::operator>(const STIME& t) const
{
	if (bHour>t.bHour)
		return TRUE;
	if (bHour<t.bHour)
		return FALSE;
	if (bMinute>t.bMinute)
		return TRUE;
	if (bMinute<t.bMinute)
		return FALSE;
	if (bSecond>t.bSecond)
		return TRUE;
	return FALSE;
}

BOOL CSchedule::SDATE::operator>=(const SDATE& t) const
{
	if (wYear>t.wYear)
		return TRUE;
	if (wYear<t.wYear)
		return FALSE;
	if (bMonth>t.bMonth)
		return TRUE;
	if (bMonth<t.bMonth)
		return FALSE;
	if (bDay>=t.bDay)
		return TRUE;
	return FALSE;
}

BOOL CSchedule::SDATE::operator>(const SDATE& t) const
{
	if (wYear>t.wYear)
		return TRUE;
	if (wYear<t.wYear)
		return FALSE;
	if (bMonth>t.bMonth)
		return TRUE;
	if (bMonth<t.bMonth)
		return FALSE;
	if (bDay>t.bDay)
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////
// CShortcut

CShortcut::CShortcut()
:	m_dwFlags(sfDefault),m_nDelay(0),
	m_pClass(NULL),m_pTitle(NULL),
	m_bModifiers(0),m_bVirtualKey(0)
{
	// At least one action is needed
	m_apActions.Add(new CKeyboardAction);

	if ((m_dwFlags&sfKeyTypeMask)==sfLocal)
		m_wWherePressed=wpDefault;
}

CShortcut::CShortcut(CShortcut& rCopyFrom)
{
	CopyMemory(this,&rCopyFrom,sizeof(CShortcut));

	if ((m_dwFlags&sfKeyTypeMask)!=sfLocal)
	{
		if (m_pClass!=NULL && m_pClass!=LPSTR(-1))
			m_pClass=alloccopy(m_pClass);
		if (m_pTitle!=NULL)
			m_pTitle=alloccopy(m_pTitle);
	}

	for (int i=0;i<m_apActions.GetSize();i++)
		m_apActions[i]=new CKeyboardAction(*m_apActions[i]);
}
	

CShortcut::~CShortcut()
{
	if ((m_dwFlags&sfKeyTypeMask)!=sfLocal)
	{
		if (m_pClass!=NULL && m_pClass!=LPSTR(-1))
			delete[] m_pClass;
		if (m_pTitle!=NULL)
			delete[] m_pTitle;
	}
	m_apActions.RemoveAll();
}

BYTE CShortcut::HotkeyModifiersToModifiers(BYTE bHotkeyModifier)
{
	BYTE bModifiers=0;
	if (bHotkeyModifier&HOTKEYF_ALT)
		bModifiers|=ModifierAlt;
	if (bHotkeyModifier&HOTKEYF_CONTROL)
		bModifiers|=ModifierControl;
	if (bHotkeyModifier&HOTKEYF_SHIFT)
		bModifiers|=ModifierShift;
	return bModifiers;
}

BYTE CShortcut::ModifiersToHotkeyModifiers(BYTE bModifier)
{
	BYTE bRet=0;
	if (bModifier&ModifierAlt)
		bRet|=HOTKEYF_ALT;
	if (bModifier&ModifierControl)
		bRet|=HOTKEYF_CONTROL;
	if (bModifier&ModifierShift)
		bRet|=HOTKEYF_SHIFT;
	return bRet;
}

BOOL CShortcut::LoadShortcuts(CArrayFP<CShortcut*>& aShortcuts,BYTE bLoadFlags)
{
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::defRead)!=ERROR_SUCCESS)
		return FALSE;

	// TODO: take bLoadFlags into account
	
	DWORD dwDataLength=RegKey.QueryValueLength("Shortcuts");
	if (dwDataLength<4)
		return FALSE;

	BYTE* pData=new BYTE[dwDataLength];
	RegKey.QueryValue("Shortcuts",(LPSTR)pData,dwDataLength,NULL);
	RegKey.CloseKey();

	// TODO: this part to another function, for GetDefaultShortuts


	BYTE* pPtr=pData;
	while (dwDataLength>=4 && *((DWORD*)pPtr)!=NULL)
	{
		DWORD dwLength;
		CShortcut* pShortcut=CShortcut::FromData(pPtr,dwDataLength,dwLength);

		if (pShortcut==NULL)
		{
			delete[] pData;
			return FALSE;
		}

		aShortcuts.Add(pShortcut);

		pPtr+=dwLength;
		dwDataLength-=dwLength;
	}
	return TRUE;
}

BOOL CShortcut::SaveShortcuts(const CArrayFP<CShortcut*>& aShortcuts)
{
	DWORD dwLength=sizeof(DWORD);
	int i;

	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::defWrite)!=ERROR_SUCCESS)
		return FALSE;

	for (i=0;i<aShortcuts.GetSize();i++)
		dwLength+=aShortcuts[i]->GetDataLength();

    BYTE* pData=new BYTE[dwLength];
	DWORD dwUsed=0;

	for (i=0;i<aShortcuts.GetSize();i++)
		dwUsed+=aShortcuts[i]->GetData(pData+dwUsed);
	
	*((DWORD*)(pData+dwUsed))=NULL;

	ASSERT(dwUsed+sizeof(DWORD)==dwLength);

	BOOL bRet=RegKey.SetValue("Shortcuts",(LPSTR)pData,dwLength,REG_BINARY)==dwLength;
    delete[] pData;

	return bRet;
}

DWORD CShortcut::GetData(BYTE* _pData) const
{
	BYTE* pData=_pData;
	DWORD dwUsed;

	*((WORD*)pData)=0xFFED; // Start mark
	pData+=sizeof(WORD);

	CopyMemory(pData,this,sizeof(CShortcut));
	pData+=sizeof(CShortcut);


	if ((m_dwFlags&sfKeyTypeMask)!=sfLocal)
	{
		if (m_pClass!=NULL && m_pClass!=LPSTR(-1))
		{
			dwUsed=istrlen(m_pClass)+1;
			CopyMemory(pData,m_pClass,dwUsed);
			pData+=dwUsed;
		}

		if (m_pTitle!=NULL)
		{
            dwUsed=istrlen(m_pTitle)+1;
			CopyMemory(pData,m_pTitle,dwUsed);
			pData+=dwUsed;
		}
	}

	for (int i=0;i<m_apActions.GetSize();i++)
	{
		dwUsed=m_apActions[i]->GetData(pData);
		pData+=dwUsed;
	}

	return DWORD(pData-_pData);
}

DWORD CShortcut::GetDataLength() const
{
	DWORD dwLen=sizeof(CShortcut)+sizeof(WORD);
	if ((m_dwFlags&sfKeyTypeMask)!=sfLocal)
	{
		if (m_pClass!=NULL && m_pClass!=LPSTR(-1))
			dwLen+=istrlen(m_pClass)+1;

		if (m_pTitle!=NULL)
			dwLen+=istrlen(m_pTitle)+1;
	}

	for (int i=0;i<m_apActions.GetSize();i++)
		dwLen+=m_apActions[i]->GetDataLength();

	return dwLen;
}

CShortcut* CShortcut::FromData(const BYTE* pData,DWORD dwDataLen,DWORD& dwUsed)
{
	DWORD dwLen;

	if (dwDataLen<sizeof(CShortcut)+sizeof(WORD))
		return NULL;

    if (*((WORD*)pData)!=0xFFED)
		return NULL;

	CShortcut* pShortcut=new CShortcut((void*)NULL);
	CopyMemory(pShortcut,pData+sizeof(WORD),sizeof(CShortcut));	
	dwUsed=sizeof(WORD)+sizeof(CShortcut);
	pData+=sizeof(WORD)+sizeof(CShortcut);
	dwDataLen-=dwUsed;

	if ((pShortcut->m_dwFlags&sfKeyTypeMask)!=sfLocal) // Load class and title 
	{
		if (pShortcut->m_pClass!=NULL && pShortcut->m_pClass!=LPSTR(-1))
		{
			for (dwLen=0;dwLen<dwDataLen && pData[dwLen]!='\0';dwLen++);

			if (dwLen==dwDataLen)
			{
				delete pShortcut;
				return NULL;
			}

			dwLen++;
			pShortcut->m_pClass=new char[dwLen];
			CopyMemory(pShortcut->m_pClass,pData,dwLen);
			dwUsed+=dwLen;
			pData+=dwLen;
			dwDataLen-=dwLen;
		}
		
		if (pShortcut->m_pTitle!=NULL)
		{
			for (dwLen=0;dwLen<dwDataLen && pData[dwLen]!='\0';dwLen++);

			if (dwLen==dwDataLen)
			{
				delete pShortcut;
				return NULL;
			}

			dwLen++;
			pShortcut->m_pTitle=new char[dwLen];
			CopyMemory(pShortcut->m_pTitle,pData,dwLen);
			dwUsed+=dwLen;
			pData+=dwLen;
			dwDataLen-=dwLen;
		}
	}

	DWORD dwActions=pShortcut->m_apActions.GetSize();
	pShortcut->m_apActions.GiveBuffer(); // There is no allocated data
	
    for (DWORD i=0;i<dwActions;i++)
	{
		CKeyboardAction* pAction=CKeyboardAction::FromData(pData,dwDataLen,dwLen);

		if (pAction==NULL)
		{
			delete pShortcut;
			return NULL;
		}

		pShortcut->m_apActions.Add(pAction);
		pData+=dwLen;
		dwUsed+=dwLen;
		dwDataLen-=dwLen;

	}
	return pShortcut;
}

CShortcut::CKeyboardAction* CShortcut::CKeyboardAction::FromData(const BYTE* pData,DWORD dwDataLen,DWORD& dwUsed)
{
	if (dwDataLen<sizeof(CKeyboardAction)+sizeof(WORD))
		return NULL;
	
	if (*((WORD*)pData)!=0xFFEC)
		return NULL;

	
	CKeyboardAction* pAction=new CKeyboardAction((void*)NULL);
	CopyMemory(pAction,pData+sizeof(WORD),sizeof(CKeyboardAction));	
	dwUsed=sizeof(WORD)+sizeof(CKeyboardAction);
	
	return pAction;
}
	
DWORD CShortcut::CKeyboardAction::GetData(BYTE* pData) const
{
	*((WORD*)pData)=0xFFEC;
	CopyMemory(pData+sizeof(WORD),this,sizeof(CKeyboardAction));
	
	return sizeof(CKeyboardAction)+2;
}




BOOL CShortcut::GetDefaultShortcuts(CArrayFP<CShortcut*>& aShortcuts,BYTE bLoadFlag)
{
	aShortcuts.Add(new CShortcut);
	aShortcuts[0]->m_bVirtualKey='b';

	aShortcuts.Add(new CShortcut);
	aShortcuts[0]->m_bVirtualKey='c';

	return TRUE;
}

char CShortcut::GetMnemonicForAction(HWND* hDialogs) const
{

	for (int i=0;i<m_apActions.GetSize();i++)
	{
		CShortcut::CKeyboardAction* pAction=m_apActions[i];

		if (pAction->m_nAction==CShortcut::CKeyboardAction::ActivateControl)
		{
			for (int j=0;hDialogs[j]!=NULL;j++)
			{	
				if (HIWORD(pAction->m_nActivateControl)& (1<<15))
					continue;


				HWND hControl=::GetDlgItem(hDialogs[j],HIWORD(pAction->m_nActivateControl));
				if (hControl!=NULL)
				{
					DWORD dwTextLen=::SendMessage(hControl,WM_GETTEXTLENGTH,0,0);
					char* pText=new char[dwTextLen+2];
					::SendMessage(hControl,WM_GETTEXT,dwTextLen+2,LPARAM(pText));

					for (DWORD k=0;k<dwTextLen-1;k++)
					{
						if (pText[k]=='&')
						{
							char cRet=pText[k+1];
							delete[] pText;
							return cRet;
						}
					}

					delete[] pText;
					break;
				}
			}
		}
	}

	return 0;
}
