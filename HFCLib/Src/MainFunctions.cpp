////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2004 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

// Library variables

#define  HFCVERSIONMS		MAKEVERSION(5,32)
#define  HFCVERSIONLS		MAKEVERSION(4,11070)

#ifdef _DEBUG
#define  HFCVERSIONSTR		"HFC Library (DEBUG) v5.32.4.11070";
#else
#define  HFCVERSIONSTR		"HFC Library v5.32.4.11070";
#endif

LPCSTR szEmpty="";
LPCSTR szError="Error";
#ifdef DEF_WCHAR
LPCWSTR szwEmpty=L"";
LPCWSTR	szwError=L"Error";
#endif

DWORD GetHFCLibVerMS(void)
{
	return HFCVERSIONMS;
}

DWORD GetHFCLibVerLS(void)
{
	return HFCVERSIONLS;
}

ULONGLONG GetHFCLibVer(void)
{
	return ((((ULONGLONG)HFCVERSIONMS)<<32)|HFCVERSIONLS);
}

LPCSTR GetHFCLibStr(void)
{
	return HFCVERSIONSTR;
}

DWORD GetHFCLibFlags(void)
{
	return 0
#ifdef WIN32
	|HFCFLAG_WIN32
#endif
#ifdef DLL
	|HFCFLAG_DLL
#endif
#ifdef _DEBUG
	|HFCFLAG_DEBUG
#endif
#ifdef CONSOLE
	|HFCFLAG_CONSOLE
#endif
#ifdef DEF_WCHAR
	|HFCFLAG_WCHAR
#endif
#ifdef DEF_RESOURCES
	|HFCFLAG_RESOURCES
#endif
#ifdef DEF_WINDOWS
	|HFCFLAG_WINDOWS
#endif
#ifdef DEF_COM
	|HFCFLAG_COM
#endif
	;
}


#ifdef _DEBUG
void MsgToText(DWORD msg,LPSTR text)
{
	switch (msg)
	{
#ifdef WIN32
	case WM_ACTIVATE:
		strcpy(text,"WM_ACTIVATE");
		break;
	case WM_ACTIVATEAPP:
		strcpy(text,"WM_ACTIVATEAPP");
		break;
	case WM_CANCELMODE:
		strcpy(text,"WM_CANCELMODE");
		break;
	case WM_CAPTURECHANGED:
		strcpy(text,"WM_CAPTURECHANGED");
		break;
	case WM_CHAR:
		strcpy(text,"WM_CHAR");
		break;
	case WM_CLOSE:
		strcpy(text,"WM_CLOSE");
		break;
	case WM_CREATE:
		strcpy(text,"WM_CREATE");
		break;
	case WM_COMMAND:
		strcpy(text,"WM_COMMAND");
		break;
	case WM_DESTROY:
		strcpy(text,"WM_DESTROY");
		break;
	case WM_DRAWITEM:
		strcpy(text,"WM_DRAWITEM");
		break;
	case WM_DROPFILES:
		strcpy(text,"WM_DROPFILES");
		break;
	case WM_GETICON:
		strcpy(text,"WM_GETICON");
		break;
	case WM_HSCROLL:
		strcpy(text,"WM_HSCROLL");
		break;
	case WM_KEYDOWN:
		strcpy(text,"WM_KEYDOWN");
		break;
	case WM_KEYUP:
		strcpy(text,"WM_KEYUP");
		break;
	case WM_KILLFOCUS:
		strcpy(text,"WM_KILLFOCUS");
		break;
	case WM_MENUSELECT:
		strcpy(text,"WM_MENUSELECT");
		break;
	case WM_MOUSEMOVE:
		strcpy(text,"WM_MOUSEMOVE");
		break;
	case WM_MOVE:
		strcpy(text,"WM_MOVE");
		break;
	case WM_NOTIFY:
		strcpy(text,"WM_NOTIFY");
		break;
	case WM_PAINT:
		strcpy(text,"WM_PAINT");
		break;
	case WM_SETICON:
		strcpy(text,"WM_SETICON");
		break;
	case WM_SIZE:
		strcpy(text,"WM_SIZE");
		break;
	case WM_SIZING:
		strcpy(text,"WM_SIZING");
		break;
	case WM_TIMER:
		strcpy(text,"WM_TIMER");
		break;
	case WM_VSCROLL:
		strcpy(text,"WM_VSCROLL");
		break;
#endif
	default:
		sprintf(text,"%lX",msg);
		break;
	}
}

#endif

#ifdef WIN32

DWORD GetFileVersion(LPCSTR szFile,DWORD* dwFileVersionLo)
{
	DWORD dwHandle;
	DWORD nLen=GetFileVersionInfoSize((char*)szFile,&dwHandle);
	if (!nLen)
		return 0;

	BYTE* pVersion=new BYTE[nLen+2];
	if (GetFileVersionInfo((char*)szFile,0,nLen,pVersion))
	{
		VS_FIXEDFILEINFO* pffi;
		UINT nflen;
		VerQueryValue(pVersion,"\\",(void**)&pffi,&nflen);
		if (dwFileVersionLo!=NULL)
			*dwFileVersionLo=pffi->dwFileVersionLS;	
		return pffi->dwFileVersionMS;
	}
	return 0;
}
	
#endif