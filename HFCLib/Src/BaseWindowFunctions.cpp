////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2004 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#if defined(WIN32)

COLORREF GetSystemColor(LPCSTR szKey)
{
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,"Control Panel\\Colors",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RGBTRIPLE rgb;
		char szBuffer[15];
		int i;
		RegKey.QueryValue(szKey,szBuffer,15);
		for (i=0;szBuffer[i]!='\0';i--)
		{
			if (szBuffer[i]==' ')
				szBuffer[i]=0;
		}
		rgb.rgbtRed=atoi(szBuffer);
		i=fstrlen(szBuffer)+1;
		rgb.rgbtGreen=atoi(szBuffer+i);
		i+=fstrlen(szBuffer+i)+1;
		rgb.rgbtBlue=atoi(szBuffer+i);
		return RGB(rgb.rgbtRed,rgb.rgbtGreen,rgb.rgbtBlue);	
	}
	return (COLORREF)-1;
}

#endif


////////////////////////////
// Class CWinApp
////////////////////////////

#ifdef DEF_APP

CWinApp::CWinApp(LPCTSTR lpszAppName)
:	m_pszAppName(lpszAppName)
{
	m_pszExeName=NULL;
	GetAppData()->pAppClass=this;
	GetAppData()->hCommonResourceHandle=NULL;
	GetAppData()->hLanguageSpecificResourceHandle=NULL;
	GetAppData()->m_pCommonDialog=NULL;
	m_hThread=GetCurrentThread();
	m_nThreadID=GetCurrentThreadId();
}

BOOL CWinApp::InitApplication()
{
	return FALSE;
}

CWinApp::~CWinApp()
{
	m_hThread=NULL;
	m_nThreadID=0;
	if (m_pszExeName!=NULL)
		delete[] m_pszExeName;
	else
		DebugMessage("CWinApp::~CWinApp(): m_pszExeName was NULL");
	EndDebugLogging();
}


#endif
#ifdef DEF_RESOURCES

HINSTANCE SetResourceHandle(HINSTANCE hHandle,TypeOfResourceHandle bType)
{
	HINSTANCE hOldHandle=GetAppData()->hCommonResourceHandle;
	if (bType==SetBoth || bType==CommonResource)
		GetAppData()->hCommonResourceHandle=hHandle;
	if (bType==SetBoth || bType==LanguageSpecificResource)
		GetAppData()->hLanguageSpecificResourceHandle=hHandle;
	return hOldHandle;
}

#endif


