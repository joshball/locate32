////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

////////////////////////////////////////////
// CTime
////////////////////////////////////////////


CTime::CTime(int nYear,int nMonth,int nDay,int nHour,int nMin,int nSec,int nMSec,int nDST)
{
	struct tm lt;
	lt.tm_sec=nSec;
	lt.tm_min=nMin;
	lt.tm_hour=nHour;
	lt.tm_mday=nDay;
	lt.tm_mon=nMonth-1;
	lt.tm_year=nYear-1900;
	lt.tm_isdst=nDST;
	msec=nMSec;
	m_time=mktime(&lt);
}

CTime::CTime(WORD wDosDate, WORD wDosTime, int nDST)
{
	struct tm lt;
	lt.tm_sec=(wDosTime&~0xFFE0)<<1;
	lt.tm_min=(wDosTime&~0xF800)>>5;
	lt.tm_hour=wDosTime>>11;
	lt.tm_mday=wDosDate&~0xFFE0;
	lt.tm_mon=((wDosDate&~0xFE00)>>5)-1;
	lt.tm_year=(wDosDate>>9)+80;
	lt.tm_isdst=nDST;
	msec=0;
	m_time=mktime(&lt);
}

CTime::CTime(const CTime& timeSrc)
{
	m_time=timeSrc.m_time;
	msec=timeSrc.msec;
}

#ifdef WIN32
CTime::CTime(const SYSTEMTIME& sysTime, int nDST)
{
	struct tm lt;
	if (sysTime.wYear<1900)
	{
		m_time=0;
		msec=0;
	}
	else
	{
		lt.tm_sec=sysTime.wSecond;
		lt.tm_min=sysTime.wMinute;
		lt.tm_hour=sysTime.wHour;
		lt.tm_mday=sysTime.wDay;
		lt.tm_mon=sysTime.wMonth-1;
		lt.tm_year=sysTime.wYear-1900;
		lt.tm_isdst=nDST;
		msec=sysTime.wMilliseconds;
		m_time=mktime(&lt);
	}
}

CTime::CTime(const FILETIME& fileTime, int nDST)
{
	struct tm lt;
	FILETIME localTime;
	if (!FileTimeToLocalFileTime(&fileTime,&localTime))
	{
		m_time=0;
		msec=0;
		
	}
	else
	{
		SYSTEMTIME sysTime;
		if (!FileTimeToSystemTime(&fileTime,&sysTime))
		{
			m_time=0;
			msec=0;
		}
		else
		{
			lt.tm_sec=sysTime.wSecond;
			lt.tm_min=sysTime.wMinute;
			lt.tm_hour=sysTime.wHour;
			lt.tm_mday=sysTime.wDay;
			lt.tm_mon=sysTime.wMonth-1;
			lt.tm_year=sysTime.wYear-1900;
			lt.tm_isdst=nDST;
			msec=sysTime.wMilliseconds;
			m_time=mktime(&lt);
		}
	}
}

CTime::operator SYSTEMTIME() const
{
	SYSTEMTIME st;
	struct tm Time;
	localtime_s(&Time,&m_time);
	st.wYear=Time.tm_year+1900;
	st.wMonth=Time.tm_mon+1;
	st.wDayOfWeek=Time.tm_wday;
	st.wDay=Time.tm_mday;
	st.wHour=Time.tm_hour;
	st.wMinute=Time.tm_min;
	st.wSecond=Time.tm_sec;
	st.wMilliseconds=msec;
	return st;
}

CTime::operator FILETIME() const
{
	FILETIME ft;
	FILETIME localTime;
	SYSTEMTIME st;
	struct tm Time;
	localtime_s(&Time,&m_time);
	st.wYear=Time.tm_year+1900;
	st.wMonth=Time.tm_mon+1;
	st.wDayOfWeek=Time.tm_wday;
	st.wDay=Time.tm_mday;
	st.wHour=Time.tm_hour;
	st.wMinute=Time.tm_min;
	st.wSecond=Time.tm_sec;
	st.wMilliseconds=msec;
	SystemTimeToFileTime(&st,&localTime);
	LocalFileTimeToFileTime(&localTime,&ft);
	return ft;
}
#endif

const CTime& CTime::operator=(const CTime& timeSrc)
{
	m_time=timeSrc.m_time;
	msec=timeSrc.msec;
	return *this;
}

#ifdef WIN32
const CTime& CTime::operator=(SYSTEMTIME& timeSrc)
{
	struct tm lt;
	lt.tm_sec=timeSrc.wSecond;
	lt.tm_min=timeSrc.wMinute;
	lt.tm_hour=timeSrc.wHour;
	lt.tm_mday=timeSrc.wDay;
	lt.tm_mon=timeSrc.wMonth-1;
	lt.tm_year=timeSrc.wYear-1900;
	lt.tm_isdst=0;
	msec=timeSrc.wMilliseconds;
	m_time=mktime(&lt);
	return *this;
}

const CTime& CTime::operator=(FILETIME& timeSrc)
{
	struct tm lt;
	SYSTEMTIME sysTime;
	FILETIME localTime;
	FileTimeToLocalFileTime(&timeSrc,&localTime);
	FileTimeToSystemTime(&localTime,&sysTime);
	lt.tm_sec=sysTime.wSecond;
	lt.tm_min=sysTime.wMinute;
	lt.tm_hour=sysTime.wHour;
	lt.tm_mday=sysTime.wDay;
	lt.tm_mon=sysTime.wMonth-1;
	lt.tm_year=sysTime.wYear-1900;
	lt.tm_isdst=-1;
	msec=sysTime.wMilliseconds;
	m_time=mktime(&lt);
	return *this;
}
#endif
struct tm* CTime::GetGmtTm(struct tm* ptm) const
{
	if (ptm!=NULL)
	{
		if (gmtime_s(ptm,&m_time)!=0)
			return NULL;
		return ptm;
	}
	return NULL;
}

struct tm* CTime::GetLocalTm(struct tm* ptm) const
{
	if (ptm!=NULL)
	{
		if (localtime_s(ptm,&m_time)!=0)
			return NULL;
		return ptm;
	}
	return NULL;
}

int CTime::GetYear() const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return Time.tm_year+1900;
}

int CTime::GetMonth() const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return Time.tm_mon+1;
}

int CTime::GetDay() const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return Time.tm_mday;
}

int CTime::GetHour() const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return Time.tm_hour;
}

int CTime::GetMinute() const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return Time.tm_min;
}

int CTime::GetSecond() const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return Time.tm_sec;
}

int CTime::GetDayOfWeek() const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return Time.tm_wday;
}

WORD CTime::DayOfYear() const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return DayOfYear(Time.tm_mday,Time.tm_mon,Time.tm_year+1900);
}

DWORD CTime::GetIndex() const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return ((Time.tm_year+1900-1)*365+LeapYears(Time.tm_year+1900-1)+DayOfYear());
}

BYTE CTime::GetWeek(BYTE nFirstIsMonday) const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return GetWeek(Time.tm_mday,Time.tm_mon,Time.tm_year+1900,nFirstIsMonday);
}

BYTE CTime::GetWeek(WORD nDay,WORD nMonth,WORD nYear,BYTE nFirstIsMonday)
{
	DWORD nFirstDate=GetIndex(1,1,nYear);
	DWORD nIndex=GetIndex(nDay,nMonth,nYear);
	BYTE nWeek=(BYTE)((nIndex-(nFirstDate-nFirstDate%7))/7)-((nIndex%7==0 && nFirstIsMonday!=0)?1:0);
	if((nFirstDate%7)<4)
		nWeek++;
	if (nWeek)
		return nWeek;
	nWeek=GetWeek(31,12,nYear-1,nFirstIsMonday);
	if (nWeek>53)
		return 1;
	return nWeek;
}
	
BYTE CTime::GetDaysInMonth() const
{
	struct tm Time;
	if (localtime_s(&Time,&m_time)!=0)
		return 0;
	return GetDaysInMonth(Time.tm_mon,Time.tm_year+1900);
}
	
WORD CTime::DayOfYear(WORD nDay,WORD nMonth,WORD nYear)
{
	switch(nMonth)
	{
	case 12:
		nDay+=30;
	case 11:
		nDay+=31;
	case 10:
		nDay+=30;
	case 9:
		nDay+=31;
	case 8:
		nDay+=31;
	case 7:
		nDay+=30;
	case 6:
		nDay+=31;
	case 5:
		nDay+=30;
	case 4:
		nDay+=31;
	case 3:
		nDay+=(IsLeapYear(nYear)?29:28);
	case 2:
		nDay+=31;
	case 1:
		break;
	default:
		return 0;
	};  
	return nDay;

}

CTime CTime::GetDayFromDayOfYear(WORD nDay,WORD nYear)
{
	struct tm lt;
	lt.tm_isdst=0;
	lt.tm_sec=0;
	lt.tm_min=0;
	lt.tm_hour=0;
	
	if (nDay<32)
	{
		lt.tm_mon=0;
		lt.tm_mday=nDay;
		return CTime(mktime(&lt),0);
	}
	if(IsLeapYear(nYear))
	{
		if (nDay<61)
		{
			lt.tm_mon=1;
			lt.tm_mday=nDay-31;
			return CTime(mktime(&lt),0);
		}
		nDay--;
	}
	else
	{
		if (nDay<60)
		{
			lt.tm_mon=1;
			lt.tm_mday=nDay-31;
			return CTime(mktime(&lt),0);
		}
	}
	
	if (nDay<91)
	{	
		lt.tm_mon=2;
		lt.tm_mday=nDay-59;
	}
	else if (nDay<121)
	{
		lt.tm_mon=3;
		lt.tm_mday=nDay-90;
	}
	else if (nDay<152)
	{
		lt.tm_mon=4;
		lt.tm_mday=nDay-120;
	}
	else if (nDay<182)
	{
		lt.tm_mon=5;
		lt.tm_mday=nDay-151;
	}
	else if (nDay<213)
	{
		lt.tm_mon=6;
		lt.tm_mday=nDay-181;
	}
	else if (nDay<244)
	{
		lt.tm_mon=7;
		lt.tm_mday=nDay-212;
	}
	else if (nDay<274)
	{
		lt.tm_mon=8;
		lt.tm_mday=nDay-243;
	}
	else if (nDay<305)
	{
		lt.tm_mon=9;
		lt.tm_mday=nDay-273;
	}
	else if (nDay<335)
	{
		lt.tm_mon=10;
		lt.tm_mday=nDay-304;
	}
	else if (nDay<366)
	{
		lt.tm_mon=11;
		lt.tm_mday=nDay-334;
	}
	return CTime(mktime(&lt),0);
}

CTime CTime::GetDayFromIndex(DWORD nIndex)
{
	WORD nYear=(WORD)(nIndex/365.25)+1;
	return GetDayFromDayOfYear((WORD)(nIndex-((nYear-1)*365+LeapYears(nYear-1))),nYear);
}

BYTE CTime::GetDaysInMonth(BYTE nMonth,WORD nYear)
{
	switch (nMonth)
	{
	case 1:
		return 31;
	case 2:
		if (IsLeapYear(nYear))
			return 29;
		else
			return 28;
	case 3:
		return 31;
	case 4:
		return 30;
	case 5:
		return 31;
	case 6:
		return 30;
	case 7:
		return 31;
	case 8:
		return 31;
	case 9:
		return 30;
	case 10:
		return 31;
	case 11:
		return 30;
	case 12:
		return 31;
	}
	return 0;
}
	
BOOL CTime::operator==(CTime Time) const
{
	if (m_time==Time.m_time)
	{
		if (msec==Time.msec)
			return 2;
		else
			return 1;
	}
	else
		return 0;
}

BOOL CTime::operator!=(CTime Time) const
{
	if (m_time==Time.m_time)
	{
		if (msec==Time.msec)
			return 0;
		else
			return 1;
	}
	else
		return 2;
}

BOOL CTime::operator<(CTime Time) const
{
	if (m_time==Time.m_time)
	{
		if (msec<Time.msec)
			return TRUE;
		return FALSE;
	}
	if (m_time<Time.m_time)
		return TRUE;
	return FALSE;
}

BOOL CTime::operator>(CTime Time) const
{
	if (m_time==Time.m_time)
	{
		if (msec>Time.msec)
			return TRUE;
		return FALSE;
	}
	if (m_time>Time.m_time)
		return TRUE;
	return FALSE;
}

BOOL CTime::operator<=(CTime Time) const
{
	if (m_time==Time.m_time)
	{
		if (msec<=Time.msec)
			return TRUE;
		return FALSE;
	}
	if (m_time<Time.m_time)
		return TRUE;
	return FALSE;
}

BOOL CTime::operator>=(CTime Time) const
{
	if (m_time==Time.m_time)
	{
		if (msec>=Time.msec)
			return TRUE;
		return FALSE;
	}
	if (m_time>Time.m_time)
		return TRUE;
	return FALSE;
}

CStringA CTime::Format(LPCSTR pFormat) const
{
	CStringA str;
	struct tm Time;
	if (localtime_s(&Time,&m_time))
		return szEmpty;

	strftime(str.GetBuffer(1000),1000,pFormat,&Time);
	str.FreeExtra();
	return str;
}

CStringA CTime::FormatGmt(LPCSTR pFormat) const
{
	CStringA str;
	struct tm Time;
	if (gmtime_s(&Time,&m_time))
		return szEmpty;

	strftime(str.GetBuffer(1000),1000,pFormat,&Time);
	str.FreeExtra();
	return str;
}

#ifdef DEF_RESOURCES
CStringA CTime::Format(UINT nFormatID) const
{
	CStringA str,str2;
	struct tm Time;
	if (localtime_s(&Time,&m_time))
		return szEmpty;

	str2.LoadString(nFormatID);
	strftime(str.GetBuffer(1000),1000,str2,&Time);
	str.FreeExtra();
	return str;
}

CStringA CTime::FormatGmt(UINT nFormatID) const
{
	CStringA str,str2;
	struct tm Time;
	if (gmtime_s(&Time,&m_time))
		return szEmpty;

	str2.LoadString(nFormatID);
	strftime(str.GetBuffer(1000),1000,str2,&Time);
	str.FreeExtra();
	return str;
}
#endif

#ifdef DEF_WCHAR
CStringW CTime::Format(LPCWSTR pFormat) const
{
	CStringW str;
	struct tm Time;
	if (localtime_s(&Time,&m_time))
		return szEmpty;

	wcsftime(str.GetBuffer(1000),1000,pFormat,&Time);
	str.FreeExtra();
	return str;
}

CStringW CTime::FormatGmt(LPCWSTR pFormat) const
{
	CStringW str;
	struct tm Time;
	if (localtime_s(&Time,&m_time))
		return szEmpty;

	wcsftime(str.GetBuffer(1000),1000,pFormat,&Time);
	str.FreeExtra();
	return str;
}


#ifdef DEF_RESOURCES
CStringW CTime::FormatW(UINT nFormatID) const
{
	CStringW str,str2;
	struct tm Time;
	if (localtime_s(&Time,&m_time))
		return szEmpty;

	str2.LoadString(nFormatID);
	wcsftime(str.GetBuffer(1000),1000,str2,&Time);
	str.FreeExtra();
	return str;
}

CStringW CTime::FormatGmtW(UINT nFormatID) const
{
	CStringW str,str2;
	struct tm Time;
	if (gmtime_s(&Time,&m_time))
		return szEmpty;

	str2.LoadString(nFormatID);
	wcsftime(str.GetBuffer(1000),1000,str2,&Time);
	str.FreeExtra();
	return str;
}

#endif
#endif

BYTE CTime::Wait(DWORD nTime)
{
#ifdef WIN32
	DWORD TempTime=::GetCurrentTime();
	DWORD Time=TempTime;
	while (TempTime<Time+nTime)
	{
		if (TempTime<Time)
			return FALSE;
		TempTime=::GetCurrentTime();
	}
#else
	clock_t TempTime=clock();
	clock_t Time=TempTime;
	clock_t TimeInc=DWORD((double)nTime/4*CLOCKS_PER_SEC);
	while (TempTime<Time+TimeInc)
	{
		if (TempTime<Time)
			return FALSE;
		TempTime=clock();
	}
#endif
	return TRUE;
}


#ifdef WIN32
////////////////////////////
// Class CRect
////////////////////////////

// First, couple of registry extensions
LONG CRegKey::CopyKey(HKEY hSource,HKEY hDestination)
{
	LONG lRet=ERROR_SUCCESS;

	// First copy subkeys
	char szName[1000];
	DWORD cb,i;
	for (i=0;;i++)
	{
		char szClass[1000];
		DWORD cb2=1000;
		cb=1000;

		if (RegEnumKeyEx(hSource,i,szName,&cb,NULL,szClass,&cb2,NULL)!=ERROR_SUCCESS)
			break;

		HKEY hSubSource,hSubDestination;

		LONG lErr=RegOpenKeyEx(hSource,szName,0,KEY_READ,&hSubSource);
		if (lErr!=ERROR_SUCCESS)
		{
			lRet=lErr;
			continue;
		}

		lErr=RegCreateKeyEx(hDestination,szName,0,szClass,0,KEY_ALL_ACCESS,NULL,&hSubDestination,NULL);
		if (lErr!=ERROR_SUCCESS)
		{
			RegCloseKey(hSubSource);
			lRet=lErr;
			continue;
		}

		if (CopyKey(hSubSource,hSubDestination)<=0)
			lRet=lErr;
			

		RegCloseKey(hSubSource);
		RegCloseKey(hSubDestination);
	}

	// Now copying values
	for (i=0;;i++)
	{
		cb=1000;
		DWORD dwType,dwDataSize;
		if (RegEnumValue(hSource,i,szName,&cb,NULL,&dwType,NULL,&dwDataSize)!=ERROR_SUCCESS)
			break;

        BYTE* pData=new BYTE[dwDataSize+1];

		LONG lErr;
		if ((lErr=RegQueryValueEx(hSource,szName,NULL,NULL,pData,&dwDataSize))!=ERROR_SUCCESS)
			lRet=lErr;
		else if ((lErr=RegSetValueEx(hDestination,szName,NULL,dwType,pData,dwDataSize))!=ERROR_SUCCESS)
			lRet=lErr;

		delete[] pData;
	}

     return lRet;   
}


LONG CRegKey::RenameSubKey(HKEY hKey,LPCSTR szOldName,LPCSTR szNewName)
{
	HKEY hSource,hDestination;
	
	LONG lErr=RegOpenKeyEx(hKey,szOldName,0,KEY_READ,&hSource);
	if (lErr!=ERROR_SUCCESS)
		return lErr;
	

	lErr=RegCreateKeyEx(hKey,szNewName,0,NULL,0,KEY_ALL_ACCESS,NULL,&hDestination,NULL);
	if (lErr!=ERROR_SUCCESS)
	{
		RegCloseKey(hSource);
		return lErr;
	}

	LONG lRet=CopyKey(hSource,hDestination);
	RegCloseKey(hSource);
	RegCloseKey(hDestination);

    if (lRet==ERROR_SUCCESS)
	{
		RegDeleteKey(hKey,szOldName);
		return ERROR_SUCCESS;
	}
	return lRet;
}

LONG CRegKey::RenameSubKey(HKEY hKey,LPCWSTR szOldName,LPCWSTR szNewName)
{
	HKEY hSource,hDestination;
	
	LONG lErr=RegOpenKeyExW(hKey,szOldName,0,KEY_READ,&hSource);
	if (lErr!=ERROR_SUCCESS)
		return lErr;
	

	lErr=RegCreateKeyExW(hKey,szNewName,0,NULL,0,KEY_ALL_ACCESS,NULL,&hDestination,NULL);
	if (lErr!=ERROR_SUCCESS)
	{
		RegCloseKey(hSource);
		return lErr;
	}

	LONG lRet=CopyKey(hSource,hDestination);
	RegCloseKey(hSource);
	RegCloseKey(hDestination);

    if (lRet==ERROR_SUCCESS)
	{
		RegDeleteKeyW(hKey,szOldName);
		return ERROR_SUCCESS;
	}
	return lRet;
}

LONG CRegKey::OpenKey(HKEY hKey,LPCSTR lpszSubKey,DWORD fStatus,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	REGSAM  samDesired=0;
	DWORD  fdwOptions;
	LONG ret;
	if (m_hKey!=NULL)
		RegCloseKey(m_hKey);
	if (fStatus==samAll)
		samDesired=KEY_ALL_ACCESS;
	else
	{
		if (fStatus&samCreateLink)
			samDesired|=KEY_CREATE_LINK;
		if (fStatus&samCreateSubkey)
			samDesired|=KEY_CREATE_SUB_KEY;
		if (fStatus&samEnumerateSubkeys)
			samDesired|=KEY_ENUMERATE_SUB_KEYS;
		if (fStatus&samExecute)
			samDesired|=KEY_EXECUTE;
		if (fStatus&samNotify)
			samDesired|=KEY_NOTIFY;
		if (fStatus&samQueryValue)
			samDesired|=KEY_QUERY_VALUE;
		if (fStatus&samSetValue)
			samDesired|=KEY_SET_VALUE;
	}
	if (fStatus&optionVolatile)
		fdwOptions=REG_OPTION_VOLATILE;
	else
		fdwOptions=REG_OPTION_NON_VOLATILE;
	if (fStatus&openExist)
		ret=RegOpenKeyEx(hKey,lpszSubKey,0,samDesired,&m_hKey);
	else
	{
		DWORD type;
		ret=RegCreateKeyEx(hKey,lpszSubKey,0,NULL,fdwOptions,samDesired,lpSecurityAttributes,&m_hKey,&type); 
	}
	if (ret!=ERROR_SUCCESS)
		m_hKey=NULL;
	return ret;
}

LONG CRegKey::CloseKey()
{
	LONG ret;
	if (m_hKey!=NULL)
	{
		ret=RegCloseKey(m_hKey);
		if (ret==ERROR_SUCCESS)
			m_hKey=NULL;
		return ret;
	}
	return ERROR_SUCCESS;
}

LONG CRegKey::OpenKey(HKEY hKey,LPCWSTR lpszSubKey,DWORD fStatus,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	REGSAM  samDesired=0;
	DWORD  fdwOptions;
	LONG ret;
	if (m_hKey!=NULL)
		RegCloseKey(m_hKey);
	if (fStatus==samAll)
		samDesired=KEY_ALL_ACCESS;
	else
	{
		if (fStatus&samCreateLink)
			samDesired|=KEY_CREATE_LINK;
		if (fStatus&samCreateSubkey)
			samDesired|=KEY_CREATE_SUB_KEY;
		if (fStatus&samEnumerateSubkeys)
			samDesired|=KEY_ENUMERATE_SUB_KEYS;
		if (fStatus&samExecute)
			samDesired|=KEY_EXECUTE;
		if (fStatus&samNotify)
			samDesired|=KEY_NOTIFY;
		if (fStatus&samQueryValue)
			samDesired|=KEY_QUERY_VALUE;
		if (fStatus&samSetValue)
			samDesired|=KEY_SET_VALUE;
	}
	if (fStatus&optionVolatile)
		fdwOptions=REG_OPTION_VOLATILE;
	else
		fdwOptions=REG_OPTION_NON_VOLATILE;
	if (fStatus&openExist)
		ret=RegOpenKeyExW(hKey,lpszSubKey,0,samDesired,&m_hKey);
	else
	{
		DWORD type;
		ret=RegCreateKeyExW(hKey,lpszSubKey,0,NULL,fdwOptions,samDesired,lpSecurityAttributes,&m_hKey,&type); 
	}
	if (ret!=ERROR_SUCCESS)
		m_hKey=NULL;
	return ret;
}



BOOL CRegKey::QueryValue(LPCSTR lpszValueName,CString& strData) const
{
	DWORD dwLength=0;
	DWORD dwType=REG_SZ;
	if (::RegQueryValueEx(m_hKey,(LPSTR)lpszValueName,NULL,&dwType,NULL,&dwLength)!=ERROR_SUCCESS)
	{
		strData.Empty();
		return FALSE;
	}

	if (dwType!=REG_SZ && dwType!=REG_EXPAND_SZ)
	{
		strData.Empty();
		return FALSE;
	}

	if (dwLength<=1)
	{
		strData.Empty();
		return TRUE;
	}

	if (::RegQueryValueEx(m_hKey,(LPSTR)lpszValueName,NULL,NULL,(LPBYTE)strData.GetBuffer(dwLength-1),&dwLength)!=ERROR_SUCCESS)
	{
		strData.Empty();
		return FALSE;
	}
	return TRUE;
}

BOOL CRegKey::QueryValue(LPCWSTR lpszValueName,CStringW& strData) const
{
	DWORD dwLength=0;
	DWORD dwType=REG_SZ;
	if (::RegQueryValueExW(m_hKey,(LPWSTR)lpszValueName,NULL,&dwType,NULL,&dwLength)!=ERROR_SUCCESS)
	{
		strData.Empty();
		return FALSE;
	}

	if (dwType!=REG_SZ && dwType!=REG_EXPAND_SZ)
	{
		strData.Empty();
		return FALSE;
	}

	if (dwLength<=1)
	{
		strData.Empty();
		return TRUE;
	}


	if (::RegQueryValueEx(m_hKey,(LPSTR)lpszValueName,NULL,NULL,(LPBYTE)strData.GetBuffer(dwLength/2-1),&dwLength)!=ERROR_SUCCESS)
	{
		strData.Empty();
		return FALSE;
	}
	return TRUE;
}

BOOL CRegKey::EnumKey(DWORD iSubkey,CString& strName,LPSTR lpszClass,LPDWORD lpcchClass,PFILETIME lpftLastWrite) const
{
	DWORD cb=50,cb2=cb;
	LONG ret=::RegEnumKeyEx(m_hKey,iSubkey,strName.GetBuffer(cb),&cb2,0,lpszClass,lpcchClass,lpftLastWrite);
	while (ret==ERROR_MORE_DATA)
	{
		cb+=50;
		cb2=cb;
		ret=::RegEnumKeyEx(m_hKey,iSubkey,strName.GetBuffer(cb),&cb2,0,lpszClass,lpcchClass,lpftLastWrite);
	}
	strName.FreeExtra(cb2);
	return ret==ERROR_SUCCESS;
}

BOOL CRegKey::EnumValue(DWORD iValue,CString& strName,LPDWORD lpdwType,LPBYTE lpbData,LPDWORD lpcbData) const
{
	DWORD cb=2048;
	LONG ret=::RegEnumValue(m_hKey,iValue,strName.GetBuffer(2048),&cb,0,lpdwType,lpbData,lpcbData);
	strName.FreeExtra();
	return ret==ERROR_SUCCESS;
}

BOOL CRegKey::EnumKey(DWORD iSubkey,CStringW& strName,LPWSTR lpszClass,LPDWORD lpcchClass,PFILETIME lpftLastWrite) const
{
	DWORD cb=100,cb2=cb;
	LONG ret=::RegEnumKeyExW(m_hKey,iSubkey,strName.GetBuffer(cb/2),&cb2,0,lpszClass,lpcchClass,lpftLastWrite);
	while (ret==ERROR_MORE_DATA)
	{
		cb+=100;
		cb2=cb;
		ret=::RegEnumKeyExW(m_hKey,iSubkey,strName.GetBuffer(cb/2),&cb2,0,lpszClass,lpcchClass,lpftLastWrite);
	}
	strName.FreeExtra(cb2);
	return ret==ERROR_SUCCESS;
}

BOOL CRegKey::EnumValue(DWORD iValue,CStringW& strName,LPDWORD lpdwType,LPBYTE lpbData,LPDWORD lpcbData) const
{
	DWORD cb=2048;
	LONG ret=::RegEnumValueW(m_hKey,iValue,strName.GetBuffer(2048/2),&cb,0,lpdwType,lpbData,lpcbData);
	strName.FreeExtra();
	return ret==ERROR_SUCCESS;
}

BOOL CRegKey::DeleteKey(HKEY hKey,LPCSTR szKey)
{
	HKEY hSubKey;
	FILETIME ft;
	DWORD cb;
	char szSubKey[200];
	if (RegOpenKeyEx(hKey,szKey,0,
		KEY_ENUMERATE_SUB_KEYS|KEY_SET_VALUE,&hSubKey)!=ERROR_SUCCESS)
		return TRUE;	
	for(;;)
	{
		cb=200;
		if (RegEnumKeyEx(hSubKey,0,szSubKey,&cb,NULL,
			NULL,NULL,&ft)==ERROR_NO_MORE_ITEMS)
			break;
		DeleteKey(hSubKey,szSubKey);
	}
	RegCloseKey(hSubKey);
	RegDeleteKey(hKey,szKey);
	return TRUE;
}


BOOL CRegKey::DeleteKey(HKEY hKey,LPCWSTR szKey)
{
	HKEY hSubKey;
	FILETIME ft;
	DWORD cb;
	WCHAR szSubKey[200];
	if (RegOpenKeyExW(hKey,szKey,0,
		KEY_ENUMERATE_SUB_KEYS|KEY_SET_VALUE,&hSubKey)!=ERROR_SUCCESS)
		return TRUE;	
	for(;;)
	{
		cb=400;
		if (RegEnumKeyExW(hSubKey,0,szSubKey,&cb,NULL,
			NULL,NULL,&ft)==ERROR_NO_MORE_ITEMS)
			break;
		DeleteKey(hSubKey,szSubKey);
	}
	RegCloseKey(hSubKey);
	RegDeleteKeyW(hKey,szKey);
	return TRUE;
}

#endif