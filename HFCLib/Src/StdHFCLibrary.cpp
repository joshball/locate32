////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2004 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#ifdef DEF_WCHAR
BOOL bIsFullUnicodeSupport=(GetVersion()&0x80000000)?FALSE:TRUE;
#endif

void CAppData::stdfunc()
{
	strncpy(LPSTR(szEmpty),"",0);
}



////////////////////////////
// WinMain
////////////////////////////

#ifdef DEF_APP

int PASCAL WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
	 LPTSTR lpszCmdLine,int nCmdShow)
{
	int nResult;
	if (GetAppData()->pAppClass!=NULL)
	{
		if (GetAppData()->m_pThreads!=NULL)
		{
			DebugMessage("WinMain(): m_pThreads already allocated?");
		}
		GetAppData()->m_pThreads=new CList<CWinThread*>;
		if (GetAppData()->m_pThreads==NULL)
		{
			MessageBox(NULL,"Cannot Allocate Memory",szError,MB_ICONSTOP|MB_OK);
			return FALSE;
		}
		GetAppData()->m_pThreads->AddTail(GetAppData()->pAppClass);
		GetAppData()->hAppInstance=hInstance;
		SetResourceHandle(hInstance,SetBoth);
		GetAppData()->pAppClass->m_lpCmdLine=lpszCmdLine;
		GetAppData()->pAppClass->m_nCmdShow=nCmdShow;
		{
			TCHAR *pExeName;
			DWORD len;
			pExeName=new TCHAR[_MAX_PATH];
			if (pExeName==NULL)
			{
				DebugMessage("WinMain(): Cannot allocate pExeName");
			}
			len=GetModuleFileName(hInstance,pExeName,_MAX_PATH);
			GetAppData()->pAppClass->m_pszExeName=new TCHAR[len+2];
			if (GetAppData()->pAppClass->m_pszExeName==NULL)
			{
				DebugMessage("WinMain(): Cannot allocate pApp->m_pExeName");
			}
			strcpy(GetAppData()->pAppClass->m_pszExeName,pExeName);
			delete[] pExeName;
		}
		StartDebugLogging();
		DebugMessage(GetAppData()->pAppClass->m_pszExeName);
		GetAppData()->pAppClass->InitApplication();
		if (GetAppData()->pAppClass->InitInstance())
			GetAppData()->pAppClass->ModalLoop();	
		nResult=GetAppData()->pAppClass->ExitInstance();
#ifdef _DEBUG
		void CheckAllocators();
		CheckAllocators();
#endif
		GetAppData()->m_pThreads->RemoveAll();
		delete GetAppData()->m_pThreads;
		GetAppData()->m_pThreads=NULL;
		return nResult;
	}
	else
	{
		MessageBox(NULL,"You have to create CWinApp based class.","Program Error",MB_ICONSTOP|MB_OK);
		DebugMessage("No any CWinApp class");
		return 1;
	}
}

#endif

#ifdef DLL
int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	return 0;
}
#endif
