#include <HFCLib.h>
#include "Locate32.h"

#include "wfext.h"
#include <pdh.h>

#include "3rdparty/cpuusage.h"



#pragma comment(lib, "pdh.lib")

UINT CLocateApp::m_nHFCInstallationMessage=0;
UINT CLocateApp::m_nTaskbarCreated=0;
UINT CLocateApp::m_nLocateAppMessage=0;


CLocateApp::CLocateApp()
:	CWinApp("LOCATE32"),m_nDelImage(0),m_nStartup(0),
	m_ppUpdaters(NULL),m_pLastDatabase(NULL),m_nFileSizeFormat(fsfOverKBasKB),
	m_dwProgramFlags(pfDefault),m_nInstance(0),m_szCommonRegFile(NULL),
	m_szCommonRegKey(NULL),m_pLocaleNumberFormat(NULL)
{
	DebugMessage("CLocateApp::CLocateApp()");
	m_pStartData=new CStartData;

	InitializeCriticalSection(&m_cUpdatersPointersInUse);
}

CLocateApp::~CLocateApp()
{
	DebugMessage("CLocateApp::~CLocateApp()");
	if (m_pStartData!=NULL)
	{
		delete m_pStartData;
		m_pStartData=NULL;
	}

	if (m_pLocaleNumberFormat!=NULL)
	{
		delete m_pLocaleNumberFormat;
		m_pLocaleNumberFormat=NULL;
	}
	
	EnterCriticalSection(&m_cUpdatersPointersInUse);
	ASSERT(m_ppUpdaters==NULL);
	LeaveCriticalSection(&m_cUpdatersPointersInUse);

	DeleteCriticalSection(&m_cUpdatersPointersInUse);
}

BOOL CALLBACK CLocateApp::EnumLocateSTWindows(HWND hwnd,LPARAM lParam)
{
	char szClass[101];
	GetClassName(hwnd,szClass,100);

	if (strcmp(szClass,"LOCATEAPPST")==0)
		++*((DWORD*)lParam);
	return TRUE;	
}

BOOL CLocateApp::InitInstance()
{
	CWinApp::InitInstance();
	LPCWSTR pCommandLine;
	
	//DebugSetFlags(DebugLogHandleOperations);

	//extern BOOL bIsFullUnicodeSupport;
	//bIsFullUnicodeSupport=FALSE;

	DebugFormatMessage("CLocateApp::InitInstance(), thread is 0x%X",GetCurrentThreadId());

	// Initializing HFC library
	if (!InitializeHFC())
	{
		MessageBox(NULL,"InitializeHFC() returned FALSE","ERROR",MB_ICONSTOP);
		m_nStartup|=CStartData::startupExitedBeforeInitialization;
		return FALSE;
	}
	
	// Initializing locater library
	InitLocaterLibrary();


	if (!IsUnicodeSystem())
		m_pGetLongPathName=CLocateApp::GetLongPathNameNoUni;
	else 
	{
		m_pGetLongPathName=(DWORD(WINAPI *)(LPCWSTR,LPWSTR,DWORD))GetProcAddress(GetModuleHandle("kernel32.dll"),"GetLongPathNameW");
		if (m_pGetLongPathName==NULL)
			m_pGetLongPathName=CLocateApp::GetLongPathName;
	}
	
	// Set messages
	m_nHFCInstallationMessage=RegisterWindowMessage("HFCINSTALLMESSAGE");
	m_nTaskbarCreated=RegisterWindowMessage("TaskbarCreated");
	m_nLocateAppMessage=RegisterWindowMessage("Locate32Communication");

	// Enumerate instances
	EnumWindows(EnumLocateSTWindows,(LPARAM)&m_nInstance);

	// Handling command line arguments
	pCommandLine=GetCommandLineW();
	while (*pCommandLine==L' ') pCommandLine++;
	if (*pCommandLine==L'\"')
	{
		pCommandLine++;
		while (*pCommandLine!=L'\"' && *pCommandLine!=L'\0') 
			pCommandLine++;
		if (*pCommandLine==L'\"')
			pCommandLine++;
	}
	else
	{
		while (*pCommandLine!=L' ' && *pCommandLine!=L'\0') 
			pCommandLine++;
	}			
	while (*pCommandLine==L' ') pCommandLine++;
	
	DebugFormatMessage("CommandLine: %S",pCommandLine);
	ParseParameters(pCommandLine,m_pStartData);

	InitCommonRegKey();


	m_nStartup=m_pStartData->m_nStartup;
	
	// If databases are specified by command line, use it
	if (m_pStartData->m_nStartup&CStartData::startupDatabasesOverridden)
	{
		m_aDatabases.Swap(m_pStartData->m_aDatabases);
		if (m_aDatabases.GetSize()>0)
			m_pLastDatabase=m_aDatabases[0];
	}
	CheckDatabases();

	// Setting prioriy if needed
	if (m_pStartData->m_nPriority!=CStartData::priorityDontChange)
		SetPriorityClass(GetCurrentProcess(),m_pStartData->m_nPriority);
	
	if (m_pStartData->m_nStartup&CStartData::startupUpdate && 
		m_pStartData->m_nStartup&CStartData::startupDoNotOpenDialog &&
		(m_pStartData->m_nStartup&CStartData::startupLeaveBackground)==0)
	{
		// Starting updating and exiting
		m_nStartup|=CStartData::startupExitedBeforeInitialization;

		if (GlobalUpdate())
		{
			while (IsUpdating())
				Sleep(100);
		}
		return FALSE;
	}
	
	
	if ((m_pStartData->m_nStartup&CStartData::startupNewInstance)==0)
	{
		// Chechkin whether locate32 is already running
		if (ActivateOtherInstances(pCommandLine))
		{
			m_nStartup|=CStartData::startupExitedBeforeInitialization;
			return FALSE;
		}
	}


	if (!SetLanguageSpecifigHandles())
	{
		m_nStartup|=CStartData::startupExitedBeforeInitialization;
		return FALSE;
	}
	
	// Ppreventing error messages when e.g. CDROM is not available
	SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS); 
    
	// Initialize common controls
	INITCOMMONCONTROLSEX icex;
	icex.dwSize=sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC=ICC_DATE_CLASSES|ICC_USEREX_CLASSES|ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icex);
	
	// Get versions of comctr32.dll and shell32.dll
	DWORD dwTemp=GetFileVersion("comctl32.dll");
	m_wComCtrlVersion=WORD(dwTemp>>8|LOBYTE(dwTemp));
	dwTemp=GetFileVersion("shell32.dll");
	m_wShellDllVersion=WORD(dwTemp>>8|LOBYTE(dwTemp));
	
	// Initializing COM 
	CoInitialize(NULL);
	
	
	// Load date and time format strings from registry	
	LoadRegistry();

	// Retrieving default icons
	SetDeleteAndDefaultImage();

	// Registering window class for notify icon handler window
	RegisterWndClass("LOCATEAPPST",0,NULL,NULL,NULL,NULL);

	m_pMainWnd=&m_AppWnd;
	RECT rc={-10,-10,-9,-9};
	m_AppWnd.Create("LOCATEAPPST","Locate ST",WS_OVERLAPPED,&rc,0,(UINT)0,0);
	
	return TRUE;
}

int CLocateApp::ExitInstance()
{
	DebugMessage("CLocateApp::ExitInstance()");

	CWinApp::ExitInstance();

	if (!(m_nStartup&CStartData::startupExitedBeforeInitialization))
	{
		// Unitializing COM
		CoUninitialize();

		// Savind date and time format strings
		SaveRegistry();
		
	}


	if (GetLanguageSpecificResourceHandle()!=GetInstanceHandle())
	{
		FreeLibrary(GetLanguageSpecificResourceHandle());
		SetResourceHandle(GetInstanceHandle(),SetBoth);
	}

	FinalizeCommonRegKey();
	return 0;
}

INT_PTR CALLBACK CLocateApp::DummyDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return FALSE;
}

DWORD CLocateApp::GetLongPathName(LPCWSTR lpszShortPath,LPWSTR lpszLongPath,DWORD cchBuffer)
{
	LPWSTR pTemp;
	return GetFullPathNameW(lpszShortPath,cchBuffer,lpszLongPath,&pTemp);
}

DWORD CLocateApp::GetLongPathNameNoUni(LPCWSTR lpszShortPath,LPWSTR lpszLongPath,DWORD cchBuffer)
{
	LPSTR pTmp;
	CHAR sPathA[MAX_PATH+10];
	DWORD dwRet=::GetFullPathNameA(W2A(lpszShortPath),MAX_PATH+10,sPathA,&pTmp);
	
	if (dwRet==0)
		return 0;
	MultiByteToWideChar(CP_ACP,0,sPathA,dwRet+1,lpszLongPath,cchBuffer);
	return dwRet;
}	

BOOL CLocateApp::ParseParameters(LPCWSTR lpCmdLine,CStartData* pStartData)
{
	int idx=0,temp;
	while (lpCmdLine[idx]==L' ') idx++;
	
	if ((lpCmdLine[idx]==L'/' || lpCmdLine[idx]==L'-'))
	{
		switch(lpCmdLine[++idx])
		{
		case L'-':
			if (lpCmdLine[++idx]!=L'\0')
			{
				if (lpCmdLine[idx]==L' ')
					idx++;
				ChangeAndAlloc(pStartData->m_pStartString,lpCmdLine+idx);
			}
			return TRUE;
		case L'X': // settings branch
			idx++;
			if (lpCmdLine[idx]==L':')
				idx++;
			while(lpCmdLine[idx]==L' ') idx++;
			temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');
			ChangeAndAlloc(pStartData->m_pSettingBranch,lpCmdLine+idx,temp);
			if (temp<0)
				return TRUE;
			idx+=temp;
			break;				
		case L'P': // put 'path' to 'Look in' field
			idx++;
			if (lpCmdLine[idx]==L':')
				idx++;
			while(lpCmdLine[idx]==L' ') idx++;
			if (lpCmdLine[idx]!=L'\"')
			{
				temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');
				ChangeAndAlloc(pStartData->m_pStartPath,lpCmdLine+idx,temp);
				if (temp<0)
					return TRUE;
				idx+=temp;
			}
			else
			{
				idx++;
				temp=(int)FirstCharIndex(lpCmdLine+idx,L'\"');
				ChangeAndAlloc(pStartData->m_pStartPath,lpCmdLine+idx,temp);
				if (temp<0)
					return TRUE;
				idx+=temp+1;
			}
			break;				
		case L'p': // Check also if path is correct
			idx++;
			if (lpCmdLine[idx]==L':')
				idx++;
			while(lpCmdLine[idx]==L' ') idx++;
			if (lpCmdLine[idx]!=L'\"')
			{
				temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');
				if (temp!=-1)
					*(char*)(lpCmdLine+idx+temp)=L'\0'; // Setting line end for 
				int nLength=0;

				WCHAR szPath[MAX_PATH+10];
				WCHAR* tmp;
				nLength=FileSystem::GetFullPathName(lpCmdLine+idx,MAX_PATH,szPath,&tmp);
				ChangeAndAlloc(pStartData->m_pStartPath,szPath,nLength);
								
				if (temp<0)
					return TRUE;
				*(char*)(lpCmdLine+idx+temp)=' '; // Setting line end for 
				idx+=temp;
			}
			else
			{
				idx++;
				temp=(int)FirstCharIndex(lpCmdLine+idx,L'\"');
				if (temp!=-1)
					*(char*)(lpCmdLine+idx+temp)=L'\0'; // Setting line end for 
				int nLength;

				WCHAR szPath[MAX_PATH+10];
				nLength=GetLocateApp()->m_pGetLongPathName(lpCmdLine+idx,szPath,200);
				ChangeAndAlloc(pStartData->m_pStartPath,szPath,nLength);
				if (temp<0)
					return TRUE;
				*(char*)(lpCmdLine+idx+temp)=L'\"'; // Setting line end for 
				idx+=temp+1;
			}
			break;				
		case L'c':
		case L'C':
			{
				OpenClipboard(NULL);
				
				
				HANDLE hData=GetClipboardData(CF_UNICODETEXT);
				if (hData!=NULL)
				{
					LPCWSTR szLine=(LPCWSTR)GlobalLock(hData);
					if (szLine!=NULL)
						ChangeAndAlloc(pStartData->m_pStartString,szLine);
					GlobalUnlock(hData);
				}
				else
				{
					hData=GetClipboardData(CF_TEXT);
					if (hData!=NULL)
					{
						LPCSTR szLine=(LPCSTR)GlobalLock(hData);
						if (szLine!=NULL)
							ChangeAndAlloc(pStartData->m_pStartString,A2W(szLine));
						GlobalUnlock(hData);
					}
				}
				CloseClipboard();
				idx++;
				break;
			}
		case L'T': // put 'type' to 'Extensions' field
		case L't':
			idx++;
			if (lpCmdLine[idx]==L':')
				idx++;
			while(lpCmdLine[idx]==L' ') idx++;
			if (lpCmdLine[idx]!=L'\"')
			{
				temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');
				ChangeAndAlloc(pStartData->m_pTypeString,lpCmdLine+idx,temp);
				if (temp<0)
					return TRUE;
				idx+=temp;
			}
			else
			{
				idx++;
				temp=(int)FirstCharIndex(lpCmdLine+idx,L'\"');
				ChangeAndAlloc(pStartData->m_pTypeString,lpCmdLine+idx,temp);
				if (temp<0)
					return TRUE;
				idx+=temp+1;
			}
			break;				
		case L'D': // activates database named 'name'
			idx++;
			if (lpCmdLine[idx]==L':')
					idx++;
			while(lpCmdLine[idx]==L' ') idx++;
			if (lpCmdLine[idx]!=L'\"')
			{
				temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');

				if (CDatabase::FindByName(pStartData->m_aDatabases,lpCmdLine+idx,temp)==NULL)
				{
					CDatabase* pDatabase=CDatabase::FromName(HKCU,
						GetRegKey("Databases"),lpCmdLine+idx,temp);
					if (pDatabase!=NULL)
					{
						pDatabase->SetThreadId(0);
						pStartData->m_aDatabases.Add(pDatabase);
                        pStartData->m_nStartup|=CStartData::startupDatabasesOverridden;
					}
					if (temp<0)
						return TRUE;
				}
				idx+=temp;
			}
			else
			{
				idx++;
				temp=(int)FirstCharIndex(lpCmdLine+idx,L'\"');
				
				if (CDatabase::FindByName(pStartData->m_aDatabases,lpCmdLine+idx,temp)==NULL)
				{
					CDatabase* pDatabase=CDatabase::FromName(HKCU,
						GetRegKey("Databases"),lpCmdLine+idx,temp);
					if (pDatabase!=NULL)
					{
						pDatabase->SetThreadId(0);
						pStartData->m_aDatabases.Add(pDatabase);
						pStartData->m_nStartup|=CStartData::startupDatabasesOverridden;
					}
				}
				if (temp<0)
					return TRUE;
				idx+=temp+1;
			}
			break;				
		case L'd': // activates database file 'dbfile'
			idx++;
			if (lpCmdLine[idx]==L':')
				idx++;
			while(lpCmdLine[idx]==L' ') idx++;
			if (lpCmdLine[idx]!=L'\"')
			{
				temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');
				if (pStartData->m_aDatabases.GetSize()==1 && wcscmp(pStartData->m_aDatabases[0]->GetName(),L"DEFAULTX")==0)
				{
					pStartData->m_aDatabases[0]->SetNamePtr(alloccopy(L"PARAMX"));
					if (temp!=-1)
						pStartData->m_aDatabases[0]->SetArchiveNamePtr(alloccopy(lpCmdLine+idx,temp));
					else
						pStartData->m_aDatabases[0]->SetArchiveNamePtr(alloccopy(lpCmdLine+idx));
				}
				else if (CDatabase::FindByFile(pStartData->m_aDatabases,lpCmdLine+idx,temp)==NULL)
				{
					CDatabase* pDatabase=CDatabase::FromFile(lpCmdLine+idx,temp);
					if (pDatabase!=NULL)
					{
						pStartData->m_aDatabases.Add(pDatabase);
						pStartData->m_nStartup|=CStartData::startupDatabasesOverridden;
					}
				}
				if (temp<0)
					return TRUE;
				idx+=temp;
			}
			else
			{
				idx++;
				temp=(int)FirstCharIndex(lpCmdLine+idx,L'\"');
				if (pStartData->m_aDatabases.GetSize()==1 && wcscmp(pStartData->m_aDatabases[0]->GetName(),L"DEFAULTX")==0)
				{
					pStartData->m_aDatabases[0]->SetNamePtr(alloccopy(L"PARAMX"));
					if (temp!=-1)
						pStartData->m_aDatabases[0]->SetArchiveNamePtr(alloccopy(lpCmdLine+idx,temp));
					else
						pStartData->m_aDatabases[0]->SetArchiveNamePtr(alloccopy(lpCmdLine+idx));
				}
				else if (CDatabase::FindByFile(pStartData->m_aDatabases,lpCmdLine+idx,temp)==NULL)
				{
					CDatabase* pDatabase=CDatabase::FromFile(lpCmdLine+idx,temp);
					if (pDatabase!=NULL)
					{
						pStartData->m_aDatabases.Add(pDatabase);
						pStartData->m_nStartup|=CStartData::startupDatabasesOverridden;
					}
				}
				if (temp<0)
					return TRUE;
				idx+=temp+1;
			}
			break;				
		case L'r': // start locating when Locate32 is opened
			idx++;
			pStartData->m_nStatus|=CStartData::statusRunAtStartUp;
			break;
		case L'i':
		case L'I':
			idx++;
			pStartData->m_nStartup|=CStartData::startupNewInstance;
			break;
		case L'a': // Activate instance
		case L'A': 
			idx++;
			if (lpCmdLine[idx]==L':')
				idx++;
			if (lpCmdLine[idx]==L' ')
				pStartData->m_nActivateInstance=-1;
			else if (lpCmdLine[idx]==L'\0')
			{
				pStartData->m_nActivateInstance=-1;
				return TRUE;
			}
			else
			{
				pStartData->m_nActivateInstance=_wtoi(lpCmdLine+idx);
				while(lpCmdLine[idx]!=' ' && lpCmdLine[idx]!='\0') idx++;
				if (lpCmdLine[idx]==L'\0')
					return TRUE;
			}
			break;		
		case L'S': // start locate32 to background, (adds icon to tastbar)
			pStartData->m_nStartup|=CStartData::startupLeaveBackground|CStartData::startupDoNotOpenDialog;
			idx++;
			break;
		case L's': // leave locate32 background when dialog is closed
			pStartData->m_nStartup|=CStartData::startupLeaveBackground;
			idx++;
			break;
		case L'u': // start update process at start
			idx++;
			pStartData->m_nStartup|=CStartData::startupUpdate;
			break;
		case L'U': // start update process and exit
			idx++;
			pStartData->m_nStartup|=CStartData::startupUpdate|CStartData::startupDoNotOpenDialog;
			break;
		case L'R':
			switch(lpCmdLine[++idx])
			{
			case L'h':
			case L'H':
				pStartData->m_nPriority=CStartData::priorityHigh;
				break;	
			case L'a':
			case L'A':
			case L'+':
				pStartData->m_nPriority=CStartData::priorityAbove;
				break;
			case L'n':
			case L'N':
			case L'0':
				pStartData->m_nPriority=CStartData::priorityNormal;
				break;
			case L'b':
			case L'B':
			case L'-':
				pStartData->m_nPriority=CStartData::priorityBelow;
				break;
			case L'i':
			case L'I':
				pStartData->m_nPriority=CStartData::priorityIdle;
				break;
			case L'r':
			case L'R':
				pStartData->m_nPriority=CStartData::priorityRealTime;
				break;
			}
			idx++;
			break;
		case L'L':
			idx++;
			if (lpCmdLine[idx]==L'1')
			{
				if (pStartData->m_aDatabases.GetSize()==0)
				{
					pStartData->m_aDatabases.Add(CDatabase::FromDefaults(TRUE));
					pStartData->m_aDatabases[0]->SetNamePtr(alloccopy(L"DEFAULTX"));
					pStartData->m_aDatabases[0]->SetThreadId(0);
					pStartData->m_nStartup|=CStartData::startupDatabasesOverridden;
					pStartData->m_aDatabases.GetLast()->AddLocalRoots();
				}
				else if (wcsncmp(pStartData->m_aDatabases.GetLast()->GetName(),L"PARAMX",6)==0 || 
					wcsncmp(pStartData->m_aDatabases.GetLast()->GetName(),L"DEFAULTX",8)==0)
					pStartData->m_aDatabases.GetLast()->AddLocalRoots();
				
				while (lpCmdLine[idx]!=L' ') idx++;
			}
			else 
			{
				while (lpCmdLine[idx]==L' ') idx++;
				
				CStringW Directory;
				if (lpCmdLine[idx]!=L'\"')
				{
					Directory.Copy(lpCmdLine+idx,FirstCharIndex(lpCmdLine+idx,L' '));
					idx+=(int)Directory.GetLength();
				}
				else
				{
					idx++;
					int nIndex=(int)FirstCharIndex(lpCmdLine+idx,L'\"');
					if (nIndex==-1)
						return TRUE;
					Directory.Copy(lpCmdLine+idx,nIndex);
					idx+=nIndex+1;
				}
				while (Directory.LastChar()==L'\\')
					Directory.DelLastChar();
					
				if (Directory.GetLength()>1)
				{
					LPCWSTR pDir=NULL;
					if (Directory[1]==L':' && Directory.GetLength()==2)
						pDir=alloccopy(Directory);
					else if (FileSystem::IsDirectory(Directory))
						pDir=alloccopy(Directory);

					if (pDir!=NULL)
					{
						if (pStartData->m_aDatabases.GetSize()==0)
						{
							pStartData->m_aDatabases.Add(CDatabase::FromDefaults(TRUE));
							pStartData->m_aDatabases[0]->SetNamePtr(alloccopy(L"DEFAULTX"));
							pStartData->m_aDatabases[0]->SetThreadId(0);
							pStartData->m_nStartup|=CStartData::startupDatabasesOverridden;
							pStartData->m_aDatabases.GetLast()->AddRoot(pDir);
						}
						else if (wcsncmp(pStartData->m_aDatabases.GetLast()->GetName(),L"PARAMX",6)==0 || 
							wcsncmp(pStartData->m_aDatabases.GetLast()->GetName(),L"DEFAULTX",8)==0)
							pStartData->m_aDatabases.GetLast()->AddRoot(pDir);
						else
							delete[] pDir;
					}
				}
			}
			break;
		case L'l':
			switch(lpCmdLine[++idx])
			{
			case L'P':
				idx++;
				if (lpCmdLine[idx]==L':')
					idx++;
				
				while (lpCmdLine[idx]==L' ') idx++;
				if (lpCmdLine[idx]!=L'\"')
				{
					temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');
					ChangeAndAlloc(pStartData->m_pLoadPreset,lpCmdLine+idx,temp);
					if (temp<0)
						return TRUE;
					idx+=temp;
				}
				else
				{
					idx++;
					temp=(int)FirstCharIndex(lpCmdLine+idx,L'\"');
					ChangeAndAlloc(pStartData->m_pLoadPreset,lpCmdLine+idx,temp);
					if (temp<0)
						return TRUE;
					idx+=temp+1;
				}
				break;				
			case L'n': // set number of maximum found files
				{
					idx++;
					if (lpCmdLine[idx]==L':')
						idx++;
					while (lpCmdLine[idx]==L' ') idx++;
					temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');
					CStringW str(lpCmdLine+idx,temp);
					
					int val=_wtoi(str);
					if (val!=0)
						pStartData->m_dwMaxFoundFiles=val;
					else if (str.Compare(L"0")==0)
						pStartData->m_dwMaxFoundFiles=0;

					if (temp<0)
						return TRUE;
					idx+=temp+1;
						break;
				}
			case L'f': // Set check field to 'File Names Only'
				idx++;
				pStartData->m_nStatus|=CStartData::statusFindFileNames;
				if (lpCmdLine[idx]==L'd')
				{
					pStartData->m_nStatus|=CStartData::statusFindFolderNames;
					idx++;
				}
				break;
			case L'd': // Set check field to 'Folder Names Only'
				idx++;
				pStartData->m_nStatus|=CStartData::statusFindFolderNames;
				if (lpCmdLine[idx]==L'f')
				{
					pStartData->m_nStatus|=CStartData::statusFindFileNames;
					idx++;
				}
				break;
			case L'c': // put 'text' to 'file containing text' field
				idx++;
				if (lpCmdLine[idx]==L'n' && lpCmdLine[idx+1]==L'm')
				{
					idx+=2;
					pStartData->m_nStatus|=CStartData::statusFindIsNotMatchCase;
					break;
				}
				
				if (lpCmdLine[idx]==L':')
					idx++;
				
				while (lpCmdLine[idx]==L' ') idx++;
				if (lpCmdLine[idx]!=L'\"')
				{
					temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');
					ChangeAndAlloc(pStartData->m_pFindText,lpCmdLine+idx,temp);
					if (temp<0)
						return TRUE;
					idx+=temp;
				}
				else
				{
					idx++;
					temp=(int)FirstCharIndex(lpCmdLine+idx,L'\"');
					ChangeAndAlloc(pStartData->m_pFindText,lpCmdLine+idx,temp);
					if (temp<0)
						return TRUE;
					idx+=temp+1;
				}
				break;
			case L'w': // check 'Match whole name only' field
				idx++;
				if (lpCmdLine[idx]==L'n')
				{
					idx++;
					pStartData->m_nStatus|=CStartData::statusNoMatchWholeName;
				}
				else
					pStartData->m_nStatus|=CStartData::statusMatchWholeName;
				break;
			case L'r': // check 'Replace asterisks' field
				idx++;
				if (lpCmdLine[idx]==L'n')
				{
					idx++;
					pStartData->m_nStatus|=CStartData::statusNoReplaceSpacesWithAsterisks;
				}
				else
					pStartData->m_nStatus|=CStartData::statusReplaceSpacesWithAsterisks;
				break;
			case L'W': // check 'Use whole path' field
				idx++;
				if (lpCmdLine[idx]==L'n')
				{
					idx++;
					pStartData->m_nStatus|=CStartData::statusNoUseWholePath;
				}
				else
					pStartData->m_nStatus|=CStartData::statusUseWholePath;
				break;
			case L'm': // set minumum file size
				{
					idx++;
					if (lpCmdLine[idx]==':')
						idx++;
					while (lpCmdLine[idx]==' ') idx++;
					temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');
					CStringW str(lpCmdLine+idx,temp);
					while ((str.LastChar()<L'0' || str.LastChar()>L'9') && !str.IsEmpty())
					{
						pStartData->m_cMinSizeType=W2Ac(str.LastChar());
						str.DelLastChar();
					}
					
					int val=_wtoi(str);
					if (val!=0)
						pStartData->m_dwMinFileSize=val;
					else if (str.Compare(L"0")==0)
						pStartData->m_dwMinFileSize=0;


					if (temp<0)
						return TRUE;
					idx+=temp+1;
					break;
				}
			case L'M': // set maximum file size
				{
					idx++;
					if (lpCmdLine[idx]==L':')
						idx++;
					while (lpCmdLine[idx]==L' ') idx++;
					temp=(int)FirstCharIndex(lpCmdLine+idx,L' ');
					CStringW str(lpCmdLine+idx,temp);
					while ((str.LastChar()<L'0' || str.LastChar()>L'9') && !str.IsEmpty())
					{
						pStartData->m_cMaxSizeType=W2Ac(str.LastChar());
						str.DelLastChar();
					}
					
					int val=_wtoi(str);
					if (val!=0)
						pStartData->m_dwMaxFileSize=val;
					else if (str.Compare(L"0")==0)
						pStartData->m_dwMaxFileSize=0;

					if (temp<0)
						return TRUE;
					idx+=temp+1;
					break;
				}
			case L'D': // dates
				{
					idx++;
					while (lpCmdLine[idx]==L' ')
						idx++;
                    int nLength=(int)LastCharIndex(lpCmdLine+idx,L' ');
					if (nLength<0)
					{
						nLength=(int)wcslen(lpCmdLine+idx);
						if (nLength<7)
                            return TRUE;
					}

					if (nLength<2)
					{
						idx+=nLength;
						break;
					}

					BOOL bMaxDate=IsCharUpper(lpCmdLine[idx]);
					if (bMaxDate) // max date
						pStartData->m_cMaxDateType=W2Ac(lpCmdLine[idx++]);
					else
						pStartData->m_cMinDateType=W2Ac(lpCmdLine[idx++]);
					nLength--;


					DWORD dwDate;
					if (nLength<6)
					{
						dwDate=_wtoi(lpCmdLine+idx);
						dwDate|=1<<31;
					}
					else
					{
						WCHAR szBuf[]=L"XX";
						szBuf[0]=lpCmdLine[idx];
						szBuf[1]=lpCmdLine[idx+1];
						WORD bYear=_wtoi(szBuf);
						if (bYear<60)
							bYear+=2000;
						else
							bYear+=1900;
						szBuf[0]=lpCmdLine[idx+2];
						szBuf[1]=lpCmdLine[idx+3];
						BYTE bMonth=_wtoi(szBuf);
						if (bMonth<1 || bMonth>12)
							bMonth=1;
						szBuf[0]=lpCmdLine[idx+4];
						szBuf[1]=lpCmdLine[idx+5];
						BYTE bDay=_wtoi(szBuf);
						if (bDay<1 || bDay>CTime::GetDaysInMonth(bMonth,bYear))
							bDay=1;			

						dwDate=(bYear<<16)|(bMonth<<8)|(bDay);
					}
					idx+=nLength;
					
					if (bMaxDate)
						pStartData->m_dwMaxDate=dwDate;
					else
						pStartData->m_dwMinDate=dwDate;

					break;
				}
			case L's':
			case L'S':
				idx++;
				if (lpCmdLine[idx]>=L'0' && lpCmdLine[idx]<=9)
					pStartData->m_nSorting=lpCmdLine[idx]-L'0';
				else
				{
					switch (lpCmdLine[idx])
					{
					case L'n':
					case L'N':
						pStartData->m_nSorting=0;
						break;
					case L'f':
					case L'F':
						pStartData->m_nSorting=1;
						break;
					case L's':
					case L'S':
						pStartData->m_nSorting=2;
						break;
					case L't':
					case L'T':
						pStartData->m_nSorting=3;
						break;
					case L'd':
					case L'D':
						pStartData->m_nSorting=4;
						break;
					}
				}
				idx++;
				if (lpCmdLine[idx]==L'd' || lpCmdLine[idx]==L'D')
					pStartData->m_nSorting|=128;
				idx++;
				break;
			}
			break;
		default:
			idx++;
			break;
		}
		return ParseParameters(lpCmdLine+idx,pStartData);
	
	}
	if (lpCmdLine[idx]!=L'\0')
		ChangeAndAlloc(pStartData->m_pStartString,lpCmdLine+idx);
	return TRUE;
}

BYTE CLocateApp::CheckDatabases()
{
	// First, check that there is database 
	if (m_aDatabases.GetSize()==0)
		CDatabase::LoadFromRegistry(HKCU,CLocateApp::GetRegKey("Databases"),m_aDatabases);

	// If there is still no any available database, try to load old style db
	if (m_aDatabases.GetSize()==0)
	{
		CDatabase* pDatabase=CDatabase::FromOldStyleDatabase(HKCU,"Software\\Update\\Database");
		if (pDatabase==NULL)
		{
			pDatabase=CDatabase::FromDefaults(TRUE); // Nothing else can be done?
		}
		else
		{
			if (CDatabase::SaveToRegistry(HKCU,CLocateApp::GetRegKey("Databases"),&pDatabase,1))
				CRegKey::DeleteKey(HKCU,"Software\\Update\\Database");
		}
		m_aDatabases.Add(pDatabase);
	}

	m_pLastDatabase=m_aDatabases[0];

	CDatabase::CheckValidNames(m_aDatabases);
	CDatabase::CheckDoubleNames(m_aDatabases);
	CDatabase::CheckIDs(m_aDatabases);
	return TRUE;
}

BYTE CLocateApp::SetDeleteAndDefaultImage()
{
	CRegKey Key;
	SHFILEINFOW fi;
	CStringW Path(GetExeNameW());
	Path << L",1";
	
	if (Key.OpenKey(HKCR,".ltmp",CRegKey::createNew|CRegKey::samCreateSubkey|CRegKey::samWrite)==NOERROR)
		Key.SetValue(szEmpty,"LTMPFile",8,REG_SZ);
	if (Key.OpenKey(HKCR,"LTMPFile",CRegKey::createNew|CRegKey::samCreateSubkey|CRegKey::samWrite)==NOERROR)
		Key.SetValue(szEmpty,"Deleted / Moved File (REMOVE THIS TYPE)",39,REG_SZ);
	if (Key.OpenKey(HKCR,"LTMPFile\\DefaultIcon",CRegKey::createNew|CRegKey::samCreateSubkey|CRegKey::samWrite)==NOERROR)
		Key.SetValue(szwEmpty,Path);
	Key.CloseKey();

	FileSystem::GetTempPath(_MAX_PATH,Path.GetBuffer(_MAX_PATH));
	Path.FreeExtra();
	if (Path.LastChar()!='\\')
		Path << '\\';
	Path << L"temp.ltmp";

	try {
		// Create file
		CFile File(Path,CFile::defWrite,TRUE);
		File.Close();

		fi.iIcon=1;
		GetFileInfo(Path,0,&fi,/*SHGFI_ICON|SHGFI_SMALLICON|*/ SHGFI_SYSICONINDEX);
		m_nDelImage=fi.iIcon;

		FileSystem::Remove(Path);
	}
	catch(...)
	{
	}
	RegDeleteKey(HKCR,".ltmp");
	RegDeleteKey(HKCR,"LTMPFile\\DefaultIcon");
	RegDeleteKey(HKCR,"LTMPFile");

	Path << L'2';
	try {
		// Create file
		CFile File(Path,CFile::defWrite,TRUE);
		File.Close();

		fi.iIcon=1;
		GetFileInfo(Path,0,&fi,/*SHGFI_ICON|SHGFI_SMALLICON|*/ SHGFI_SYSICONINDEX);
		m_nDefImage=fi.iIcon;

		FileSystem::Remove(Path);
	}
	catch(...)
	{
	}

	if (FileSystem::GetSystemDirectory(Path.GetBuffer(_MAX_PATH+3),_MAX_PATH)>0)
	{
		fi.iIcon=1;
		GetFileInfo(Path,0,&fi,/*SHGFI_ICON|SHGFI_SMALLICON|*/SHGFI_SYSICONINDEX);
		m_nDirImage=fi.iIcon;

		WCHAR szDrives[100];
		if (FileSystem::GetLogicalDriveStrings(100,szDrives)>0)
		{
			LPCWSTR pPtr=szDrives;
			while (*pPtr!='\0')
			{
				if (FileSystem::GetDriveType(pPtr)==DRIVE_FIXED &&
					(*pPtr!=Path[0] || Path[1]!=':'))
					break;

				pPtr+=istrlenw(pPtr)+1;
			}

			GetFileInfo(*pPtr!='\0'?pPtr:szDrives,0,&fi,SHGFI_ICON|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
			m_nDriveImage=fi.iIcon;
		}
		
	}
	return TRUE;
}


BOOL CLocateApp::ActivateOtherInstances(LPCWSTR pCmdLine)
{
	//HWND hWnd=FindWindow("LOCATEAPPST","Locate ST");
	//if (hWnd!=NULL)

	if (m_nInstance!=0 || m_pStartData->m_nActivateInstance!=0)
	{
		ATOM aCommandLine;
		if (IsUnicodeSystem())
			aCommandLine=GlobalAddAtomW(pCmdLine);
		else
			aCommandLine=GlobalAddAtom(W2A(pCmdLine));

		if (m_pStartData->m_nActivateInstance>0)
		{
			::SendNotifyMessage(HWND_BROADCAST,m_nLocateAppMessage,
				MAKEWPARAM(LOCATEMSG_ACTIVATEINSTANCE,m_pStartData->m_nActivateInstance-1),
				(LPARAM)aCommandLine);
		}
		else
			::SendNotifyMessage(HWND_BROADCAST,m_nLocateAppMessage,LOCATEMSG_ACTIVATEINSTANCE,(LPARAM)aCommandLine);

		if (aCommandLine!=NULL)
			DeleteAtom(aCommandLine);
		return TRUE;
	}	
	return FALSE;
}


	
LPWSTR CLocateApp::FormatDateAndTimeString(WORD wDate,WORD wTime)
{
	DWORD dwLength=2;
	
	enum {
		fDateIsDefault = 0x1,
		fTimeIsDefault = 0x2
	};
	BYTE fFlags=0;

	// wDate/wTime is 0xFFFFFFFF, omit date/time
	if (wDate!=WORD(-1) && wDate!=0)
	{
		if (m_strDateFormat.IsEmpty())
		{
			// Using default format
			fFlags|=fDateIsDefault;

			// Using GetDateFormat as default
			SYSTEMTIME st;
			st.wDay=DOSDATETODAY(wDate);
			st.wMonth=DOSDATETOMONTH(wDate);
			st.wYear=DOSDATETOYEAR(wDate);

			if (IsUnicodeSystem())
			{
				GetDateFormatW(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,
					NULL,m_strDateFormat.GetBuffer(1000),1000);
				m_strDateFormat.FreeExtra();
			}
			else
			{
				char szFormat[1000];
				GetDateFormatA(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,
					NULL,szFormat,1000);
				m_strDateFormat=szFormat;
			}

			

			dwLength+=(DWORD)m_strDateFormat.GetLength();
		}
		else
            dwLength+=(DWORD)m_strDateFormat.GetLength()*2;
	}
	
	
	if (wTime!=WORD(-1) && wTime!=0)
	{
		if (m_strTimeFormat.IsEmpty())
		{
			// Using default format
			fFlags|=fTimeIsDefault;

			// Using GetTimeFormat as default
			SYSTEMTIME st;
			st.wHour=DOSTIMETO24HOUR(wTime);
			st.wMinute=DOSTIMETOMINUTE(wTime);
			st.wSecond=DOSTIMETOSECOND(wTime);
			st.wMilliseconds=0;
			
			if (IsUnicodeSystem())
			{
				GetTimeFormatW(LOCALE_USER_DEFAULT,TIME_NOSECONDS,&st,
					NULL,m_strTimeFormat.GetBuffer(1000),1000);
				m_strTimeFormat.FreeExtra();
			}
			else
			{
				char szTimeFormat[1000];
				GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOSECONDS,&st,
					NULL,szTimeFormat,1000);
				m_strTimeFormat=szTimeFormat;
			}

			dwLength+=(DWORD)m_strTimeFormat.GetLength();
		}
		else
			dwLength+=(DWORD)m_strTimeFormat.GetLength()*2;
	
		
	}

	LPWSTR szRet=new WCHAR[dwLength];
	LPWSTR pPtr=szRet;

	//Formatting date

	// TODO: GetTimeFormat and GetDateFormat ?

    if (wDate!=WORD(-1) && wDate!=0)
	{
		for (int i=0;i<m_strDateFormat.GetLength();i++)
		{
			switch (m_strDateFormat[i])
			{
			case L'd':
				if (m_strDateFormat[i+1]=='d') // "dd" , "ddd" and "dddd" will not be handled
				{
					pPtr[0]=DOSDATETODAY(wDate)/10+L'0';
					pPtr[1]=DOSDATETODAY(wDate)%10+L'0';
					pPtr+=2;
					i++;
				}
				else // "d"
				{
					if (DOSDATETODAY(wDate)>9)
					{
						pPtr[0]=DOSDATETODAY(wDate)/10+L'0';
						pPtr[1]=DOSDATETODAY(wDate)%10+L'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSDATETODAY(wDate)+L'0';
						pPtr++;
					}
				}
				break;
			case L'M':
				if (m_strDateFormat[i+1]==L'M') // "MM", "MMM" & "MMMM" will not be handled
				{
					pPtr[0]=DOSDATETOMONTH(wDate)/10+L'0';
					pPtr[1]=DOSDATETOMONTH(wDate)%10+L'0';
					pPtr+=2;
					i++;
				}
				else // "M"
				{
					if (DOSDATETOMONTH(wDate)>9)
					{
						pPtr[0]=DOSDATETOMONTH(wDate)/10+L'0';
						pPtr[1]=DOSDATETOMONTH(wDate)%10+L'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSDATETOMONTH(wDate)+L'0';
						pPtr++;
					}
				}
				break;
			case L'y':
				if (m_strDateFormat[i+1]==L'y')
				{
					if (m_strDateFormat[i+2]==L'y') // "yyy" & "yyyy"
					{
						pPtr[0]=DOSDATETOYEAR(wDate)/1000+L'0';
						pPtr[1]=(DOSDATETOYEAR(wDate)/100)%10+L'0';
						pPtr[2]=(DOSDATETOYEAR(wDate)/10)%10+L'0';
						pPtr[3]=DOSDATETOYEAR(wDate)%10+L'0';
						if (m_strDateFormat[i+3]==L'y')
							i+=3;
						else
							i+=2;
						pPtr+=4;
					}
					else // "yy"
					{
						pPtr[0]=(DOSDATETOYEAR(wDate)/10)%10+L'0';
						pPtr[1]=DOSDATETOYEAR(wDate)%10+L'0';
						pPtr+=2;
						i++;
					}			
				}
				else // "y"
				{
					if (DOSDATETOYEAR(wDate)/1000>9)
					{
						pPtr[0]=(DOSDATETOYEAR(wDate)/10)%10+L'0';
						pPtr[1]=DOSDATETOYEAR(wDate)%10+L'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSDATETOYEAR(wDate)%10+L'0';
						pPtr++;
					}
				}
				break;
			case L'\'':
				continue;
			default:
				*pPtr=m_strDateFormat[i];
				pPtr++;
				break;
			}
		}
	}
	
	// Formatting time
	if (wTime!=WORD(-1) && wTime!=0)
	{
		*pPtr=' ';
		pPtr++;
		
		for (int i=0;i<m_strTimeFormat.GetLength();i++)
		{
			switch (m_strTimeFormat[i])
			{
			case L'h':
				if (m_strTimeFormat[i+1]==L'h')
				{
					pPtr[0]=DOSTIMETO12HOUR(wTime)/10+L'0';
					pPtr[1]=DOSTIMETO12HOUR(wTime)%10+L'0';
					pPtr+=2;
					i++;
				}
				else
				{
					if (DOSTIMETO12HOUR(wTime)>9)
					{
						pPtr[0]=DOSTIMETO12HOUR(wTime)/10+L'0';
						pPtr[1]=DOSTIMETO12HOUR(wTime)%10+L'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSTIMETO12HOUR(wTime)%10+L'0';
						pPtr++;
					}
				}
				break;
			case L'H':
				if (m_strTimeFormat[i+1]==L'H')
				{
					pPtr[0]=DOSTIMETO24HOUR(wTime)/10+L'0';
					pPtr[1]=DOSTIMETO24HOUR(wTime)%10+L'0';
					pPtr+=2;
					i++;
				}
				else
				{
					if (DOSTIMETO24HOUR(wTime)>9)
					{
						pPtr[0]=DOSTIMETO24HOUR(wTime)/10+L'0';
						pPtr[1]=DOSTIMETO24HOUR(wTime)%10+L'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSTIMETO24HOUR(wTime)%10+L'0';
						pPtr++;
					}
				}
				break;
			case L'm':
				if (m_strTimeFormat[i+1]==L'm')
				{
					pPtr[0]=DOSTIMETOMINUTE(wTime)/10+L'0';
					pPtr[1]=DOSTIMETOMINUTE(wTime)%10+L'0';
					pPtr+=2;
					i++;
				}
				else
				{
					if (DOSTIMETOMINUTE(wTime)>9)
					{
						pPtr[0]=DOSTIMETOMINUTE(wTime)/10+L'0';
						pPtr[1]=DOSTIMETOMINUTE(wTime)%10+L'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSTIMETOMINUTE(wTime)%10+L'0';
						pPtr++;
					}
				}
				break;
			case L's':
				if (m_strTimeFormat[i+1]==L's')
				{
					pPtr[0]=DOSTIMETOSECOND(wTime)/10+L'0';
					pPtr[1]=DOSTIMETOSECOND(wTime)%10+L'0';
					pPtr+=2;
					i++;
				}
				else
				{
					if (DOSTIMETOSECOND(wTime)>9)
					{
						pPtr[0]=DOSTIMETOSECOND(wTime)/10+L'0';
						pPtr[1]=DOSTIMETOSECOND(wTime)%10+L'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSTIMETOSECOND(wTime)%10+L'0';
						pPtr++;
					}
				}
				break;
			case 't':
				{
					WCHAR szAMPM[10];
					LoadString(DOSTIMETO24HOUR(wTime)>11?IDS_PM:IDS_AM,szAMPM,10);
					
					if (m_strTimeFormat[i+1]==L't')
					{
						for (WCHAR* ptr=szAMPM;*ptr!=L'\0';ptr++,pPtr++)
							*pPtr=*ptr;
						i++;
					}
					else
						*pPtr=szAMPM[0];
				}
				break;
			case L'\'':
				continue;
			default:
				*pPtr=m_strTimeFormat[i];
				pPtr++;
				break;
			}
		}
	}

	*pPtr=L'\0';

	if (fFlags&fDateIsDefault)
		m_strDateFormat.Empty();
	if (fFlags&fTimeIsDefault)
		m_strTimeFormat.Empty();
	
	return szRet;
}

LPWSTR CLocateApp::FormatFileSizeString(DWORD dwFileSizeLo,DWORD bFileSizeHi) const
{
 	WCHAR* szRet=new WCHAR[40];
	WCHAR szUnit[10];
	BOOL bDigits=0;
		

	switch (m_nFileSizeFormat)
	{
	case fsfOverKBasKB:
		if (bFileSizeHi>0)
		{
			LoadString(IDS_KB,szUnit,10);
			
			_int64 uiFileSize=((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo;
			_ui64tow_s(uiFileSize/1024,szRet,40,10);
		}
		else if (dwFileSizeLo<1024)
		{
			LoadString(IDS_BYTES,szUnit,10);
			
			_ultow_s(dwFileSizeLo,szRet,40,10);
		}
		else
		{
			LoadString(IDS_KB,szUnit,10);
			
			_ultow_s((dwFileSizeLo/1024)+(dwFileSizeLo%1024==0?0:1),szRet,40,10);
		}
		break;
	case fsfOverMBasMB:
		if (bFileSizeHi>0)
		{
			LoadString(IDS_MB,szUnit,10);
			
			_int64 uiFileSize=((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo;
			_ui64tow_s(uiFileSize/(1024*1024),szRet,40,10);
		}
		else if (dwFileSizeLo<1024)
		{
			LoadString(IDS_BYTES,szUnit,10);
			
			_ultow_s(dwFileSizeLo,szRet,40,10);
		}
		else if (dwFileSizeLo<1024*1024)
		{
			LoadString(IDS_KB,szUnit,10);
			
			_ultow_s(dwFileSizeLo/1024,szRet,40,10);
		}
		else
		{
			LoadString(IDS_MB,szUnit,10);
			
			_ultow_s((dwFileSizeLo/(1024*1024))+(dwFileSizeLo%(1024*1024)==0?0:1),szRet,40,10);
		}
		break;
	case fsfBestUnit:
		if (bFileSizeHi>0)
		{
			DWORD num=DWORD(((((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo))/(1024*1024));

			if (num>=10*1024)
				_ultow_s(num/1024,szRet,40,10);
			else
			{
				bDigits=1;
				StringCbPrintfW(szRet,40*sizeof(WCHAR),L"%1.1f",double(num)/1024);
			}

			LoadString(IDS_GB,szUnit,10);
		}
		else if (dwFileSizeLo>1024*1024*1024)
		{
			DWORD num=dwFileSizeLo/(1024*1024);

			if (num>=10*1024)
				_ultow_s(num/1024,szRet,40,10);
			else
			{
				bDigits=1;
				StringCbPrintfW(szRet,40*sizeof(WCHAR),L"%1.1f",double(num)/1024);
			}

			LoadString(IDS_GB,szUnit,10);
		}
		else if (dwFileSizeLo>1048576) // As MB
		{
			DWORD num=dwFileSizeLo/1024;
			
			if (num>=10*1024)
				_ultow_s(num/1024,szRet,40,10);
			else
			{
				bDigits=1;
				StringCbPrintfW(szRet,40*sizeof(WCHAR),L"%1.1f",double(num)/1024);
			}

			LoadString(IDS_MB,szUnit,10);
		}
		else if (dwFileSizeLo>1024) // As KB
		{
			if (dwFileSizeLo>=10*1024)
				_ultow_s(dwFileSizeLo/1024,szRet,40,10);
			else
			{
				bDigits=1;
				StringCbPrintfW(szRet,40*sizeof(WCHAR),L"%1.1f",double(dwFileSizeLo)/1024);
			}
			
			LoadString(IDS_KB,szUnit,10);
		}
		else // As B
		{
			_ultow_s(dwFileSizeLo,szRet,40,10);
		
			LoadString(IDS_BYTES,szUnit,10);
		}		
		break;
	case fsfBytes:
		LoadString(IDS_BYTES,szUnit,10);
	case fsfBytesNoUnit:
		if (bFileSizeHi>0)
		{
			_int64 uiFileSize=((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo;
			_ui64tow_s(uiFileSize,szRet,40,10);
		}
		else
			_ultow_s(dwFileSizeLo,szRet,40,10);
		
		break;
	case fsfKiloBytes:
		LoadString(IDS_KB,szUnit,10);
	case fsfKiloBytesNoUnit:
		if (bFileSizeHi>0)
		{
			_int64 uiFileSize=((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo;
			_ui64tow_s(uiFileSize/1024,szRet,40,10);
		}
		else if (dwFileSizeLo>10*1024)
			_ultow_s(dwFileSizeLo/1024,szRet,40,10);
		else if (dwFileSizeLo<1024)
		{
			bDigits=3;
			StringCbPrintfW(szRet,40*sizeof(WCHAR),L"%1.3f",((double)dwFileSizeLo)/1024);
		}
		else
		{
			bDigits=1;
			StringCbPrintfW(szRet,40*sizeof(WCHAR),L"%1.1f",((double)dwFileSizeLo)/1024);
		}
		break;
	case fsfMegaBytesMegaBytes:
		LoadString(IDS_MB,szUnit,10);
	case fsfMegaBytesMegaBytesNoUnit:
		if (bFileSizeHi>0)
		{
			_int64 uiFileSize=((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo;
			_ui64tow_s(uiFileSize/1048576,szRet,40,10);
		}
		else if (dwFileSizeLo>10*1048576)
			_ultow_s(dwFileSizeLo/1048576,szRet,40,10);
		else if (dwFileSizeLo<1048576)
		{
			bDigits=3;
			StringCbPrintfW(szRet,40*sizeof(WCHAR),L"%1.3f",((double)dwFileSizeLo)/1048576);
		}
		else
		{
			bDigits=1;
			StringCbPrintfW(szRet,40*sizeof(WCHAR),L"%1.1f",((double)dwFileSizeLo)/1048576);
		}		
		break;
	}

	if (m_dwProgramFlags&pfFormatUseLocaleFormat)
	{
		if (GetLocateApp()->m_pLocaleNumberFormat==NULL)
			GetLocateApp()->m_pLocaleNumberFormat=new LocaleNumberFormat;

		union {
			NUMBERFMTW fmtw;
			NUMBERFMT fmt;
		};

		fmt.NumDigits=bDigits; 
		fmt.LeadingZero=GetLocateApp()->m_pLocaleNumberFormat->uLeadingZero;
		fmt.Grouping=GetLocateApp()->m_pLocaleNumberFormat->uGrouping; 
		fmt.NegativeOrder=1; 
			
		if (IsUnicodeSystem())
		{
			WCHAR* szFormattedStr=new WCHAR[60];
			
			fmtw.lpDecimalSep=GetLocateApp()->m_pLocaleNumberFormat->pDecimal; 
			fmtw.lpThousandSep=GetLocateApp()->m_pLocaleNumberFormat->pThousand; 
			
			if (GetNumberFormatW(LOCALE_USER_DEFAULT,0,szRet,&fmtw,szFormattedStr,60)>0)
			{
				delete[] szRet;
				szRet=szFormattedStr;
			}
			else
				delete[] szFormattedStr;
		}
		else
		{
			char szFormattedStr[50];
					
			fmt.lpDecimalSep=alloccopyWtoA(GetLocateApp()->m_pLocaleNumberFormat->pDecimal); 
			fmt.lpThousandSep=alloccopyWtoA(GetLocateApp()->m_pLocaleNumberFormat->pThousand); 
			
			if (GetNumberFormat(LOCALE_USER_DEFAULT,0,W2A(szRet),&fmt,szFormattedStr,50)>0)
				MultiByteToWideChar(CP_ACP,0,szFormattedStr,-1,szRet,40);

			delete[] fmt.lpDecimalSep;
			delete[] fmt.lpThousandSep;
		}
	}

	if (!(m_nFileSizeFormat==fsfBytesNoUnit || 
		m_nFileSizeFormat==fsfKiloBytesNoUnit|| 
		m_nFileSizeFormat==fsfMegaBytesMegaBytesNoUnit))
		StringCbCatW(szRet,40*sizeof(WCHAR),szUnit);

	return szRet;
}


BOOL CLocateApp::StopUpdating(BOOL bForce)
{
    if (!IsUpdating())
		return TRUE; // Already stopped

	BOOL bRet=TRUE;
	EnterCriticalSection(&m_cUpdatersPointersInUse);
	for (int i=0;m_ppUpdaters!=NULL && m_ppUpdaters[i]!=NULL;i++)
	{
		if (!IS_UPDATER_EXITED(m_ppUpdaters[i]))
		{
			LeaveCriticalSection(&m_cUpdatersPointersInUse);
			if (!m_ppUpdaters[i]->StopUpdating(bForce))
				bRet=FALSE;
			EnterCriticalSection(&m_cUpdatersPointersInUse);
		}
	}
	LeaveCriticalSection(&m_cUpdatersPointersInUse);
	
	GetLocateAppWnd()->StopUpdateStatusNotification();
	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg!=NULL)
		pLocateDlg->SendMessage(WM_UPDATINGSTOPPED);
		
	GetLocateAppWnd()->SetUpdateStatusInformation(DEFAPPICON,IDS_NOTIFYLOCATE);
	
	// I think pointers are removed elsewhere
	ASSERT(m_ppUpdaters==NULL);



	return bRet;
}

void CLocateApp::SetDatabases(const CArray<CDatabase*>& rDatabases)
{
	m_aDatabases.RemoveAll();

	int i;
	for (i=0;i<rDatabases.GetSize();i++)
		m_aDatabases.Add(new CDatabase(*rDatabases[i]));

	if (i>0)
		m_pLastDatabase=m_aDatabases[0];
}

WORD CLocateApp::GetDatabasesNumberOfThreads() const
{
	WORD wHighestThread=0;
	for (int i=0;i<m_aDatabases.GetSize();i++)
	{
		if (wHighestThread<m_aDatabases[i]->GetThreadId())
			wHighestThread=m_aDatabases[i]->GetThreadId();
	}
	return wHighestThread+1;
}

void CLocateApp::ClearStartData()
{
	if (m_pStartData!=NULL)
	{
		delete ((CLocateApp*)GetApp())->m_pStartData;
		((CLocateApp*)GetApp())->m_pStartData=NULL;
	}
}

void CLocateAppWnd::NotifyFinishingUpdating()
{
	// No updaters running anymore...
				
	// ...so stopping animations
	if (this!=NULL)
	{
		if (m_pUpdateStatusWnd!=NULL)
			m_pUpdateStatusWnd->Update();
		StopUpdateStatusNotification();
	}

	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg!=NULL)
	{
		pLocateDlg->StopUpdateAnimation();
		
		
		// ... and constucting notification message:
		// checking wheter all are stopped, or cancelled 
		EnterCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
		
		if (GetLocateApp()->m_ppUpdaters!=NULL)
		{
			int iThreads;
			
			// Count threads and check which are interrupted
			BOOL bAllStopped=TRUE;
			for (iThreads=0;GetLocateApp()->m_ppUpdaters[iThreads]!=NULL;iThreads++)
			{
				if (GET_UPDATER_CODE(GetLocateApp()->m_ppUpdaters[iThreads])!=ueStopped)
					bAllStopped=FALSE;
			}

			if (bAllStopped)
			{
				// All updaters are interrupted by user
				pLocateDlg->SendMessage(WM_SETOPERATIONSTATUSBAR,(WPARAM)IDI_EXCLAMATION,
					(LPARAM)(LPCWSTR)ID2W(IDS_UPDATINGCANCELLED));
			}
			else
			{
				// All succeeded or some updaters failed/interrupted
				CStringW str2;
				int added=0;
					
				for (int i=0;i<iThreads;i++)
				{
					switch (GET_UPDATER_CODE(GetLocateApp()->m_ppUpdaters[i]))
					{
					case ueSuccess:
						break;
					case ueStopped:
						if (added>0)
							str2 << L", ";
						if (iThreads>1)
						{
							str2 << ID2W(IDS_UPDATINGTHREAD);
							str2 << ' ' << (int)(i+1) << L": ";
						}

						str2 << ID2W(IDS_UPDATINGCANCELLED2);
						added++;
						break;
					case ueFolderUnavailable:
						if (added>0)
							str2 << L", ";
						if (iThreads>1)
						{
							str2 << ID2W(IDS_UPDATINGTHREAD);
							str2 << ' ' << (int)(i+1) << L": ";
						}
						str2 << ID2W(IDS_UPDATINGUNAVAILABLEROOT);
						added++;
						break;
					default:
						if (added>0)
							str2 << L", ";
						if (iThreads>1)
						{
							str2 << ID2W(IDS_UPDATINGTHREAD);
							str2 << ' ' << (int)(i+1) << L": ";
						}
						str2 << ID2W(IDS_UPDATINGFAILED);
						added++;
						break;
					}
				}

				if (str2.IsEmpty())
				{
					pLocateDlg->SendMessage(WM_SETOPERATIONSTATUSBAR,0,
						(LPARAM)(LPCWSTR)ID2W(IDS_UPDATINGSUCCESS));				
				}
				else
				{
					CStringW str(IDS_UPDATINGENDED);
					str << L' ' << str2;
					pLocateDlg->SendMessage(WM_SETOPERATIONSTATUSBAR,
						(WPARAM)IDI_EXCLAMATION,(LPARAM)(LPCWSTR)str);				
				}
			}
		}
		else
		{
			pLocateDlg->SendMessage(WM_SETOPERATIONSTATUSBAR,0,
				(LPARAM)(LPCWSTR)ID2W(IDS_UPDATINGENDED));				
		}

		LeaveCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
	}

	
}

int CLocateApp::ErrorBox(int nError,UINT uType)
{
	ID2W Text(nError);
	ID2W Title(IDS_ERROR);

	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg!=NULL)
		return pLocateDlg->MessageBox(Text,Title,uType);
	
	extern CLocateApp theApp;;
	if (theApp.m_pMainWnd!=NULL)
	{
		theApp.m_pMainWnd->ForceForegroundAndFocus();
		theApp.m_pMainWnd->SetForegroundWindow();
	}
	
	return CWnd::MessageBox(NULL,Text,Title,uType|MB_TOPMOST);
}


int CLocateApp::ErrorBox(LPCWSTR szError,UINT uType)
{
	ID2W Title(IDS_ERROR);

	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg!=NULL)
		return pLocateDlg->MessageBox(szError,Title,uType);
	
	extern CLocateApp theApp;;
	if (theApp.m_pMainWnd!=NULL)
	{
		theApp.m_pMainWnd->ForceForegroundAndFocus();
		theApp.m_pMainWnd->SetForegroundWindow();	
	}
	
	return CWnd::MessageBox(NULL,szError,Title,uType|MB_TOPMOST);
}
	
BOOL CALLBACK CLocateApp::UpdateProc(DWORD_PTR dwParam,CallingReason crReason,UpdateError ueCode,CDatabaseUpdater* pUpdater)
{
	DbcDebugFormatMessage2("CLocateApp::UpdateProc BEGIN, reason=%d, code=%d",crReason,ueCode);
	
	switch (crReason)
	{
	case Initializing:
	{
		// Start animations
		if (GetLocateAppWnd()->GetHandle()!=NULL)
		{
			GetLocateAppWnd()->StartUpdateStatusNotification();
		
			CLocateDlg* pLocateDlg=GetLocateDlg();
			if (pLocateDlg!=NULL)
			{
				pLocateDlg->StartUpdateAnimation();
			
				//pLocateDlg->m_pStatusCtrl->SetText(szEmpty,0,0);
				pLocateDlg->SendMessage(WM_SETOPERATIONSTATUSBAR,0,(LPARAM)(LPCWSTR)ID2W(IDS_UPDATINGDATABASE));							
			}
		}

		return TRUE;
	}
	case RootChanged:
	{
		CLocateDlg* pLocateDlg=GetLocateDlg();
		if (pLocateDlg!=NULL)
		{
			CStringW str;

			if (pUpdater->GetCurrentRoot()!=NULL)
			{
				str.Format(IDS_UPDATINGDATABASE2,
					pUpdater->GetCurrentDatabaseName(),
					(LPCWSTR)pUpdater->GetCurrentRoot()->m_Path);
			}
			else
			{
				str.Format(IDS_UPDATINGDATABASE2,
					pUpdater->GetCurrentDatabaseName(),
					(LPCWSTR)ID2W(IDS_UPDATINGWRITINGDATABASE));
			}
			
			pLocateDlg->SendMessage(WM_SETOPERATIONSTATUSBAR,0,(LPARAM)(LPCWSTR)str);
		}
	
		
		if (dwParam!=NULL)
			((CLocateAppWnd*)dwParam)->SetUpdateStatusInformation(NULL,IDS_NOTIFYUPDATING);
		return TRUE;
	}
	case FinishedUpdating:
		if (ueCode!=ueStopped) // This is done at the end of CLocateApp::StopUpdating
		{
			CLocateDlg* pLocateDlg=GetLocateDlg();
			if (pLocateDlg!=NULL)
				pLocateDlg->PostMessage(WM_UPDATINGSTOPPED);
			
		}
		break;
	case FinishedDatabase:
	{
		CLocateDlg* pLocateDlg=GetLocateDlg();
		if (pLocateDlg!=NULL)
		{
			if (ueCode==ueStopped)
			{
				CStringW str;
				str.Format(IDS_UPDATINGDATABASE2,
					pUpdater->GetCurrentDatabaseName(),
					(LPCWSTR)ID2W(IDS_UPDATINGCANCELLED2));

				pLocateDlg->SendMessage(WM_SETOPERATIONSTATUSBAR,0,(LPARAM)(LPCWSTR)str);
				return FALSE;
			}
			else if (ueCode!=ueSuccess)
			{
				CStringW str;
				str.Format(IDS_UPDATINGDATABASE2,
					pUpdater->GetCurrentDatabaseName(),
					(LPCWSTR)ID2W(IDS_UPDATINGFAILED));

				pLocateDlg->SendMessage(WM_SETOPERATIONSTATUSBAR,0,(LPARAM)(LPCWSTR)str);
				return FALSE;
			}
			CStringW str;
			str.Format(IDS_UPDATINGDATABASE2,
				pUpdater->GetCurrentDatabaseName(),
				(LPCWSTR)ID2W(IDS_UPDATINGDONE));

			pLocateDlg->SendMessage(WM_SETOPERATIONSTATUSBAR,0,(LPARAM)(LPCWSTR)str);
		}

		return TRUE;
	}
	case ClassShouldDelete:
		{
			if (!GetLocateApp()->IsUpdating())
				return TRUE; // One thread mode

						
			DWORD dwRunning=0;
			
			EnterCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
						
			if (GetLocateApp()->m_ppUpdaters==NULL)
			{
				LeaveCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
				return FALSE;
			}

			
			for (int i=0;GetLocateApp()->m_ppUpdaters[i]!=NULL;i++)
			{
				if (GetLocateApp()->m_ppUpdaters[i]==pUpdater)
					GetLocateApp()->m_ppUpdaters[i]=UPDATER_EXITED(ueCode);
				else if (!IS_UPDATER_EXITED(GetLocateApp()->m_ppUpdaters[i]))
					dwRunning++;
			}
			delete pUpdater;
			
			if (dwRunning==0)
			{
				((CLocateAppWnd*)dwParam)->NotifyFinishingUpdating();
				
				// Freeing memory
				delete[] GetLocateApp()->m_ppUpdaters;
				GetLocateApp()->m_ppUpdaters=NULL;

				LeaveCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
	

				if (dwParam!=NULL && GetLocateApp()->m_nStartup&CStartData::startupExitAfterUpdating)
					((CLocateAppWnd*)dwParam)->PostMessage(WM_COMMAND,IDM_EXIT,NULL);
			}
			else 
			{
				// Updaters still running, updating shell notify icons
				LeaveCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
				
				if (dwParam!=NULL)
				{
					// dwParam can be NULL, if updating process is started via 
					// command line parameters such that dialog is not opened
				
					((CLocateAppWnd*)dwParam)->SetUpdateStatusInformation(NULL,IDS_NOTIFYUPDATING);
				}
			}
			return TRUE;
		}
	case ErrorOccured:
		if (((CLocateAppWnd*)dwParam)->m_pUpdateStatusWnd!=NULL)
			((CLocateAppWnd*)dwParam)->m_pUpdateStatusWnd->FormatErrorForStatusTooltip(ueCode,pUpdater);
			
		switch(ueCode)
		{
		case ueUnknown:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
			{
				WCHAR* pError=FormatLastOsError();

				if (pError!=NULL)
				{
					CStringW str,state;
					if (pUpdater->GetCurrentRoot()==NULL)
						state.Format(IDS_ERRORUNKNOWNWRITEDB,pUpdater->GetCurrentDatabaseFile());
					else
						state.Format(IDS_ERRORUNKNOWNSCANROOT,pUpdater->GetCurrentRootPath());
					
					
					str.Format(IDS_ERRORUNKNOWNOS,pError);
					while (str.LastChar()=='\n' || str.LastChar()=='\r')
						str.DelLastChar();
					str << state;
					
					ErrorBox(str);
					LocalFree(pError);

					
				}
				else
					ErrorBox(IDS_ERRORUNKNOWN);
				
			}
			return FALSE;
		case ueCreate:
		case ueOpen:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
			{
				CStringW str;
				str.Format(IDS_ERRORCANNOTOPENDB,pUpdater->GetCurrentDatabaseFile());
				ErrorBox(str);
			}
			return FALSE;
		case ueRead:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
			{
				CStringW str;
				str.Format(IDS_ERRORCANNOTREADDB,pUpdater->GetCurrentDatabaseFile());
				ErrorBox(str);
			}
			return FALSE;
		case ueWrite:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
			{
				CStringW str;
				str.Format(IDS_ERRORCANNOTWRITEDB,pUpdater->GetCurrentDatabaseFile());
				ErrorBox(str);
			}
			return FALSE;
		case ueAlloc:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
				ErrorBox(IDS_ERRORCANNOTALLOCATE);
			return FALSE;
		case ueInvalidDatabase:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
			{
				CStringW str;
				str.Format(IDS_ERRORINVALIDDB,W2A(pUpdater->GetCurrentDatabaseName()));
				ErrorBox(str);
				return FALSE;
			}
			return FALSE;
		case ueFolderUnavailable:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowNonCriticalErrors)
			{
				CStringW str;
				str.Format(IDS_ERRORROOTNOTAVAILABLE,pUpdater->GetCurrentRootPath()!=NULL?pUpdater->GetCurrentRootPath():L"");
				ErrorBox(str);
			}
			return FALSE;
		case ueCannotIncrement:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowNonCriticalErrors)
			{
				CStringW str;
				str.Format(IDS_ERRORCANNOTWRITEINCREMENTALLY,W2A(pUpdater->GetCurrentDatabaseName()));
				return ErrorBox(str,MB_ICONERROR|MB_YESNO)==IDYES;
			}
			return TRUE;
		case ueWrongCharset:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowNonCriticalErrors)
			{
				CStringW str;
				str.Format(IDS_ERRORDIFFERENTCHARSETINDB,W2A(pUpdater->GetCurrentDatabaseName()));
				return ErrorBox(str,MB_ICONERROR|MB_YESNO)==IDYES;
			}
			return TRUE;
		case ueCannotCreateThread:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
				ErrorBox(IDS_ERRORCANNOTCREATETHREAD);
			return FALSE;
		}
		break;
	}
	
	DbcDebugMessage("CLocateApp::UpdateProc END");
	return TRUE;
}

BOOL CLocateApp::SetLanguageSpecifigHandles()
{
	CRegKey2 RegKey;
	CStringW LangFile;
	if (RegKey.OpenKey(HKCU,"",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue(L"Language",LangFile);
		RegKey.CloseKey();
	}
	if (LangFile.IsEmpty())
		LangFile=L"lan_en.dll";

	CStringW sExeName(GetApp()->GetExeNameW());
	CStringW Path(sExeName,sExeName.FindLast('\\')+1);
	
	

	HINSTANCE hLib=FileSystem::LoadLibrary(Path+LangFile,LOAD_LIBRARY_AS_DATAFILE);
	if (hLib==NULL)
	{
		hLib=FileSystem::LoadLibrary(Path+L"lan_en.dll",LOAD_LIBRARY_AS_DATAFILE);

		if (hLib==NULL)
		{
			MessageBox(NULL,ID2A(IDS_ERRORCANNOTLOADLANGUAGEFILE,CommonResource),
				ID2A(IDS_ERROR),MB_ICONERROR|MB_OK);
			return FALSE;
		}

		MessageBox(NULL,ID2A(IDS_ERRORCANNOTLOADLANGUAGEFILE2,CommonResource),
			ID2A(IDS_ERROR),MB_ICONERROR|MB_OK);
	}

	SetResourceHandle(hLib,LanguageSpecificResource);

	// Set help file
	LangFile.LoadString(IDS_HELPFILE);
	if (FileSystem::IsFile(Path+LangFile))
		GetApp()->SetHelpFile(LangFile);

	return TRUE;
}


BOOL CLocateApp::GlobalUpdate(CArray<PDATABASE>* paDatabasesArg,int nThreadPriority)
{
	if (IsUpdating())
		return FALSE;

	CArray<PDATABASE>* paDatabases;
	if (paDatabasesArg==NULL)
	{
		paDatabases=new CArray<PDATABASE>;
		for (int i=0;i<m_aDatabases.GetSize();i++)
		{
			if (m_aDatabases[i]->IsGloballyUpdated())
				paDatabases->Add(new CDatabase(*m_aDatabases[i]));
		}
	}
	else
		paDatabases=paDatabasesArg;

	WORD wThreads=CDatabase::CheckIDs(*paDatabases);
	if (wThreads==0)
		return FALSE;

	EnterCriticalSection(&m_cUpdatersPointersInUse);	
	m_ppUpdaters=new CDatabaseUpdater*[wThreads+1];
	if (m_ppUpdaters==NULL)
		return FALSE;

	WORD wThread;
	for (wThread=0;wThread<wThreads;wThread++)
	{
		m_ppUpdaters[wThread]=new CDatabaseUpdater(*paDatabases,paDatabases->GetSize(),
			UpdateProc,wThread,(DWORD_PTR)m_pMainWnd);
	}
	m_ppUpdaters[wThreads]=NULL;
    
	// Starting
	DWORD dwRunning=0;
	for (wThread=0;wThread<wThreads;wThread++)
	{
		UpdateError ueCode=m_ppUpdaters[wThread]->Update(TRUE,nThreadPriority);
		if (ueCode==ueSuccess)
			dwRunning++;
		else
		{
			delete m_ppUpdaters[wThread];
			m_ppUpdaters[wThread]=UPDATER_EXITED(ueCode);
		}
	}
			
	if (dwRunning==0)
	{
		((CLocateAppWnd*)m_pMainWnd)->NotifyFinishingUpdating();
				
		// Freeing memory
		delete[] GetLocateApp()->m_ppUpdaters;
		GetLocateApp()->m_ppUpdaters=NULL;

		

		if (GetLocateApp()->m_nStartup&CStartData::startupExitAfterUpdating)
			m_pMainWnd->PostMessage(WM_COMMAND,IDM_EXIT,NULL);
		m_ppUpdaters=NULL;
	}
	
	LeaveCriticalSection(&m_cUpdatersPointersInUse);
	
	if (paDatabasesArg==NULL)
	{
		for (int i=0;i<paDatabases->GetSize();i++)
			delete paDatabases->GetAt(i);
		delete paDatabases;
	}
	return dwRunning>0;
}

void CLocateApp::OnInitDatabaseMenu(CMenu& PopupMenu)
{
	// Removing existing items
	for(int i=PopupMenu.GetMenuItemCount()-1;i>=0;i--)
		PopupMenu.DeleteMenu(i,MF_BYPOSITION);

	CStringW title;
	MENUITEMINFOW mi;
	mi.cbSize=sizeof(MENUITEMINFOW);
	mi.fMask=MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_TYPE;
	mi.wID=IDM_DEFUPDATEDBITEM;
	mi.fType=MFT_STRING;
	mi.fState=MFS_ENABLED;
	
	if (m_aDatabases.GetSize()==0)
	{
		// Inserting default items
		title.LoadString(IDS_EMPTY);
		mi.dwTypeData=(LPWSTR)(LPCWSTR)title;
		mi.dwItemData=0;
		mi.fState=MFS_GRAYED;
		PopupMenu.InsertMenu(mi.wID,FALSE,&mi);
		return;
	}

	// Starting to insert database items 
	for (int i=0;i<m_aDatabases.GetSize();i++)
	{
		title.Format(L"&%d: %s",i+1,m_aDatabases[i]->GetName());
		mi.dwTypeData=(LPWSTR)(LPCWSTR)title;
		mi.dwItemData=m_aDatabases[i]->GetID();
		mi.wID=IDM_DEFUPDATEDBITEM+i;
		PopupMenu.InsertMenu(mi.wID,FALSE,&mi);
	}	
}

void CLocateApp::OnDatabaseMenuItem(WORD wID)
{
	int iDB=wID-IDM_DEFUPDATEDBITEM;

	ASSERT(iDB>=0 && iDB<m_aDatabases.GetSize());

	DWORD dwLength=(DWORD)istrlenw(m_aDatabases[iDB]->GetName());
	LPWSTR pDatabaseName=new WCHAR[dwLength+2];
	MemCopyW(pDatabaseName,m_aDatabases[iDB]->GetName(),dwLength);
	pDatabaseName[dwLength]='\0';
	pDatabaseName[dwLength+1]='\0';

	GetLocateAppWnd()->OnUpdate(FALSE,pDatabaseName);

	delete[] pDatabaseName;
}

int CLocateApp::GetDatabaseMenuIndex(HMENU hPopupMenu)
{
	MENUITEMINFO mii;
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_SUBMENU;
	
	for(int i=GetMenuItemCount(hPopupMenu)-1;i>=0;i--)
	{
		if (!GetMenuItemInfo(hPopupMenu,i,TRUE,&mii))
			continue;

		if (mii.hSubMenu!=NULL)
		{
			if (IsDatabaseMenu(GetSubMenu(hPopupMenu,i)))
				return i;
		}
	}
	return -1;
}



/////////////////////////////////////////////
// CLocateAppWnd

CLocateAppWnd::CLocateAppWnd()
:	m_pAbout(NULL),m_pSettings(NULL),m_hAppIcon(NULL),
	m_pLocateDlgThread(NULL),m_pCpuUsage(NULL),
	m_pUpdateAnimIcons(NULL),m_hHook(NULL)
{
	DebugMessage("CLocateAppWnd::CLocateAppWnd()");

	InitializeCriticalSection(&m_csAnimBitmaps);
}

CLocateAppWnd::~CLocateAppWnd()
{
	DebugMessage("CLocateAppWnd::~CLocateAppWnd()");
	//m_Schedules.RemoveAll();

	DeleteCriticalSection(&m_csAnimBitmaps);
	
	if (m_pCpuUsage!=NULL)
		delete m_pCpuUsage;
}

int CLocateAppWnd::OnCreate(LPCREATESTRUCT lpcs)
{
	// Loading menu
	m_Menu.LoadMenu(IDR_SYSTEMTRAYMENU);


	// Set schedules
	SetSchedules();
	SetTimer(ID_RUNSTARTUPSCHEDULES,500,NULL);

	SetMenuDefaultItem(m_Menu.GetSubMenu(0),IDM_OPENLOCATE,FALSE);
	
	// Setting icons
	HICON hIcon=(HICON)LoadImage(IDI_APPLICATIONICON,IMAGE_ICON,32,32,LR_SHARED);
	SetIcon(hIcon,TRUE);
	SetClassLong(gclHIcon,(LONG)hIcon);

	LoadAppIcon();

	SetTimer(ID_ENSUREVISIBLEICON,2000,NULL);
	
	return CFrameWnd::OnCreate(lpcs);
}

void CLocateAppWnd::LoadAppIcon()
{
	if (m_hAppIcon!=NULL)
		DeleteObject(m_hAppIcon);

	
	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::defRead)==ERROR_SUCCESS)
	{
		CStringW CustomTrayIcon;
		if (RegKey.QueryValue(L"CustomTrayIcon",CustomTrayIcon))
		{
			m_hAppIcon=(HICON)LoadImage(CustomTrayIcon,IMAGE_ICON,16,16,LR_LOADFROMFILE);
			if (m_hAppIcon!=NULL)
				return;
		}
	}

	m_hAppIcon=(HICON)LoadImage(IDI_APPLICATIONICON,IMAGE_ICON,16,16,0);
}

BOOL CLocateAppWnd::OnCreateClient(LPCREATESTRUCT lpcs)
{
	AddTaskbarIcon();

	BOOL bDoOpen=FALSE;

	// Opening dialog if STARTUP_DONOTOPENDIALOG is not set 
	if (GetLocateApp()->GetStartupFlags()&CLocateApp::CStartData::startupDoNotOpenDialog)
		GetLocateApp()->ClearStartData();
	else
		bDoOpen=TRUE;
	
	// Updating database if necessary
	if (GetLocateApp()->IsStartupFlagSet(CLocateApp::CStartData::startupUpdate))
		PostMessage(WM_COMMAND,MAKEWPARAM(IDM_GLOBALUPDATEDB,0),NULL);
	
	TurnOnShortcuts();

	BOOL bRet=CFrameWnd::OnCreateClient(lpcs);
	if (bDoOpen)
		OnLocate();
	return bRet;
}

BOOL CLocateAppWnd::TurnOnShortcuts()
{
	TurnOffShortcuts();

	OSVERSIONINFOEX oi;
	oi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
	BOOL bRet=GetVersionEx((LPOSVERSIONINFO) &oi);
	BOOL bCanHook=!(bRet && oi.dwPlatformId!=VER_PLATFORM_WIN32_NT ||
		!(oi.dwMajorVersion>=5 || (oi.dwMajorVersion==4 && oi.wServicePackMajor>=3) ));

	// Loading new shortcuts
	if (!CShortcut::LoadShortcuts(m_aShortcuts,(
		bCanHook?CShortcut::loadGlobalHook:0)|CShortcut::loadGlobalHotkey))
	{
        if (!CShortcut::GetDefaultShortcuts(m_aShortcuts,
			CShortcut::loadGlobalHotkey|(bCanHook?CShortcut::loadGlobalHook:0)))
			ShowErrorMessage(IDS_ERRORCANNOTLOADDEFAULTSHORTUCS,IDS_ERROR);
	}

	// Resolving mnemonics
	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg!=NULL)
	{
		HWND hDialogs[]={*pLocateDlg,pLocateDlg->m_NameDlg,pLocateDlg->m_SizeDateDlg,
			pLocateDlg->m_AdvancedDlg,NULL};
		CShortcut::ResolveMnemonics(m_aShortcuts,NULL);
	}
	else
	{
		HWND hDialogs[5];
		hDialogs[0]=CreateDialog(GetLanguageSpecificResourceHandle(),
			MAKEINTRESOURCE(IDD_MAIN),NULL,(DLGPROC)CLocateApp::DummyDialogProc);
		hDialogs[1]=CreateDialog(GetLanguageSpecificResourceHandle(),
			MAKEINTRESOURCE(IDD_NAME),hDialogs[0],(DLGPROC)CLocateApp::DummyDialogProc);
		hDialogs[2]=CreateDialog(GetLanguageSpecificResourceHandle(),
			MAKEINTRESOURCE(IDD_SIZEDATE),hDialogs[0],(DLGPROC)CLocateApp::DummyDialogProc);
		hDialogs[3]=CreateDialog(GetLanguageSpecificResourceHandle(),
			MAKEINTRESOURCE(IDD_ADVANCED),hDialogs[0],(DLGPROC)CLocateApp::DummyDialogProc);
		hDialogs[4]=NULL;
		CShortcut::ResolveMnemonics(m_aShortcuts,NULL);
		for (int i=0;hDialogs[i]!=NULL;i++)
            ::DestroyWindow(hDialogs[i]);
	}

	// Count hooks and register hotkeys
	UINT nHooks=0;
	for (int i=0;i<m_aShortcuts.GetSize();i++)
	{
		switch(m_aShortcuts[i]->m_dwFlags&CShortcut::sfKeyTypeMask)
		{
		case CShortcut::sfGlobalHook:
			nHooks++;
			break;
		case CShortcut::sfGlobalHotkey:
			if (!RegisterHotKey(*this,i,m_aShortcuts[i]->m_bModifiers,m_aShortcuts[i]->m_bVirtualKey))
			{
				// TODO: Virhe viesti jos settings dialog auki
			}
			break;
		}
	}

	if (nHooks>0)
	{
		ASSERT(bCanHook);
		m_hHook=SetHook(*this,m_aShortcuts.GetData(),m_aShortcuts.GetSize());
	}
		
	
	return TRUE;
}

BOOL CLocateAppWnd::TurnOffShortcuts()
{
	// Unregister hotkeys
	for (int i=0;i<m_aShortcuts.GetSize();i++)
	{
		if ((m_aShortcuts[i]->m_dwFlags&CShortcut::sfKeyTypeMask)==CShortcut::sfGlobalHotkey)
			UnregisterHotKey(*this,i);
	}

	// Stop hooking
	if (m_hHook!=NULL)
	{
		UnsetHook(m_hHook);
		m_hHook;
	}

	m_aShortcuts.RemoveAll();

	return NULL;
}

void CLocateApp::SaveRegistry() const
{
	CRegKey2 RegKey;
	if(RegKey.OpenKey(HKCU,"\\General",
		CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		RegKey.SetValue("General Flags",m_dwProgramFlags&pfSave);

		RegKey.SetValue(L"DateFormat",m_strDateFormat);
		RegKey.SetValue(L"TimeFormat",m_strTimeFormat);
		RegKey.SetValue("SizeFormat",(DWORD)m_nFileSizeFormat);
	}


}

void CLocateApp::LoadRegistry()
{
	// When modifications are done, check whether 
	// function is applicable for UpdateSettings

	CRegKey2 RegKey;
	m_strDateFormat.Empty();
	m_strTimeFormat.Empty();

	if (RegKey.OpenKey(HKCU,"\\General",
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		// Loading dwFlags
		DWORD temp=m_dwProgramFlags;
		RegKey.QueryValue("General Flags",temp);
		m_dwProgramFlags&=~pfSave;
		m_dwProgramFlags|=temp&pfSave;



		RegKey.QueryValue(L"DateFormat",m_strDateFormat);
		RegKey.QueryValue(L"TimeFormat",m_strTimeFormat);
		RegKey.QueryValue("SizeFormat",*((DWORD*)&m_nFileSizeFormat));

	}
}

BOOL CLocateApp::UpdateSettings()
{
	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\General",
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		// Loading dwFlags
		DWORD dwNewFlags=m_dwProgramFlags;
		RegKey.QueryValue("General Flags",dwNewFlags);
		
		if (dwNewFlags&pfDontShowSystemTrayIcon &&
			!(m_dwProgramFlags&pfDontShowSystemTrayIcon))
			m_AppWnd.DeleteTaskbarIcon(TRUE);
		else if (!(dwNewFlags&pfDontShowSystemTrayIcon) &&
			m_dwProgramFlags&pfDontShowSystemTrayIcon)
		{
			m_AppWnd.AddTaskbarIcon(TRUE);
			m_AppWnd.SetUpdateStatusInformation(NULL,IDS_NOTIFYLOCATE);
		}
				

		m_dwProgramFlags&=~pfSave;
		m_dwProgramFlags|=dwNewFlags&pfSave;
	}

	return TRUE;
}

BOOL CLocateAppWnd::GetRootInfos(WORD& wThreads,WORD& wRunning,RootInfo*& pRootInfos)
{
	// Initializing
	wThreads=0;
	pRootInfos=NULL;
	wRunning=0;
	
	// Counting threads		
	EnterCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
	if (GetLocateApp()->m_ppUpdaters==NULL)
	{
		// Not updating
		LeaveCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
		return FALSE;
	}
	
	for (;GetLocateApp()->m_ppUpdaters[wThreads]!=NULL;wThreads++);

	pRootInfos=new RootInfo[max(wThreads,2)];
	
	
	for (WORD i=0;i<wThreads;i++)
	{
		if (IS_UPDATER_EXITED(GetLocateApp()->m_ppUpdaters[i]))
		{
			pRootInfos[i].pName=NULL;
			pRootInfos[i].pRoot=NULL;
			pRootInfos[i].ueError=GET_UPDATER_CODE(GetLocateApp()->m_ppUpdaters[i]);
		}
		else 
		{
			wRunning++;

			pRootInfos[i].ueError=ueStillWorking;

			if (GetLocateApp()->m_ppUpdaters[i]->GetCurrentDatabaseName()==NULL)
			{
				// Not started yet
				pRootInfos[i].pName=allocemptyW();
				pRootInfos[i].pRoot=NULL;

				if (GetLocateApp()->m_ppUpdaters[i]->GetStatus()==CDatabaseUpdater::statusFinishing)
					pRootInfos[i].ueError=ueSuccess;

				if (m_pUpdateStatusWnd!=NULL)
				{
					pRootInfos[i].dwNumberOfDatabases=GetLocateApp()->m_ppUpdaters[i]->GetNumberOfDatabases();
					pRootInfos[i].dwCurrentDatabase=0;
					pRootInfos[i].wProgressState=0;
				}
			}
			else
			{
				pRootInfos[i].pName=GetLocateApp()->m_ppUpdaters[i]->GetCurrentDatabaseNameStr();
				if (GetLocateApp()->m_ppUpdaters[i]->GetCurrentRoot()==NULL)
					pRootInfos[i].pRoot=NULL; // Is writing database
				else
					pRootInfos[i].pRoot=GetLocateApp()->m_ppUpdaters[i]->GetCurrentRootPathStr();
				
				if (m_pUpdateStatusWnd!=NULL)
				{
					pRootInfos[i].dwNumberOfDatabases=GetLocateApp()->m_ppUpdaters[i]->GetNumberOfDatabases();
					pRootInfos[i].dwCurrentDatabase=GetLocateApp()->m_ppUpdaters[i]->GetCurrentDatabase();
					pRootInfos[i].wProgressState=GetLocateApp()->m_ppUpdaters[i]->GetProgressStatus();
				}

			}
			
		}
	}
	LeaveCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
	return TRUE;
}

void CLocateAppWnd::FreeRootInfos(WORD wThreads,RootInfo* pRootInfos)
{
	// Freeing memory
	if (pRootInfos==NULL)
		return;
	for (int i=0;i<wThreads;i++)
	{
		if (pRootInfos[i].pName!=NULL)
			delete[] pRootInfos[i].pName;
		if (pRootInfos[i].pRoot!=NULL)
			delete[] pRootInfos[i].pRoot;
	}
	delete[] pRootInfos;
}

#define MAXTIPTEXTLENGTH		128
#define MAXINFOTEXTLENGTH		256
#define MAXINFOTITLELENGTH		64

BOOL CLocateAppWnd::SetUpdateStatusInformation(HICON hIcon,UINT uTip,LPCWSTR szText)
{
	if (CLocateApp::GetProgramFlags()&CLocateApp::pfDontShowSystemTrayIcon)
		return TRUE;

	NOTIFYICONDATAW nid;
	ZeroMemory(&nid,sizeof(NOTIFYICONDATAW));
	
	if (GetLocateApp()->m_wShellDllVersion>=0x0500)
		nid.cbSize=sizeof(NOTIFYICONDATAW);
	else
		nid.cbSize=NOTIFYICONDATAW_V1_SIZE;

	nid.hWnd=*this;
	nid.uID=1000;
	nid.uFlags=0;
		
	if (hIcon!=NULL)
	{
		// Updating icon
		if (hIcon==INVALID_HANDLE_VALUE)
			nid.hIcon=m_hAppIcon;
		else
			nid.hIcon=hIcon;
		nid.uFlags|=NIF_ICON;
	}
	
	if (uTip==IDS_NOTIFYUPDATING)
	{
		nid.uFlags|=NIF_TIP;
		
		WORD wThreads,wRunning;
		RootInfo* pRootInfos;
		
		if (!GetRootInfos(wThreads,wRunning,pRootInfos))
		{
			// TODO
			m_pUpdateStatusWnd->Update(0,0,NULL);
			NotifyFinishingUpdating();			
			return FALSE;
		}

		
			
		if (m_pUpdateStatusWnd!=NULL)
			m_pUpdateStatusWnd->Update(wThreads,wRunning,pRootInfos);
			
		if (wThreads>1)
		{
			if (((CLocateApp*)GetApp())->m_wShellDllVersion>=0x0500 &&	wThreads<10)
			{
				// Loading string
				WCHAR szThread[20];
				int iThreadLen=LoadString(IDS_NOTIFYTHREAD,szThread,20);
				WCHAR szCaption[30];
				int iCaptionLen=LoadString(IDS_NOTIFYUPDATINGDBS2,szCaption,30);
                WCHAR szDone[20];
				int iDoneLen=LoadString(IDS_NOTIFYDONE,szDone,20);
				WCHAR szWriting[25];
				int iWritingLen=LoadString(IDS_NOTIFYWRITINGDATABASE,szWriting,25);
				WCHAR szInitializing[25];
				int iInitializingLen=LoadString(IDS_NOTIFYINITIALIZING,szInitializing,25);

				// Computing required length for string
				int iRequired=iCaptionLen;
				int iRequiredForRoots=0;

				// Format: Thread N: name, root
				for (int i=0;i<wThreads;i++)
				{
					iRequired+=iThreadLen+1+4;
					if (pRootInfos[i].pName==NULL)
						iRequired+=iDoneLen;
					else if (pRootInfos[i].pName[0]=='\0')
						iRequired+=iInitializingLen;
					else
					{
						iRequired+=(int)istrlenw(pRootInfos[i].pName);
						if (pRootInfos[i].pRoot==NULL)
							iRequiredForRoots+=iWritingLen+2;
						else
                        	iRequiredForRoots+=(int)istrlenw(pRootInfos[i].pRoot)+2;
					}
				}
				
				if (iRequired>=MAXTIPTEXTLENGTH)
				{
					WCHAR szTemp[54];
					LoadString(IDS_NOTIFYUPDATINGDBS,szTemp,54);
					StringCbPrintfW(nid.szTip,64*sizeof(WCHAR),szTemp,(int)wRunning,(int)wThreads);
				}
				else
				{
					LPWSTR pPtr=nid.szTip;
					StringCbCopyNW(pPtr,128*sizeof(WCHAR),szCaption,iCaptionLen);
					pPtr+=iCaptionLen;

					for (int i=0;i<wThreads;i++)
					{
						*(pPtr++)='\n';

						CopyMemory(pPtr,szThread,iThreadLen);
						pPtr+=iThreadLen;
						*(pPtr++)=' ';
						*(pPtr++)='1'+char(i);
						*(pPtr++)=':';
						*(pPtr++)=' ';
                        
						if (pRootInfos[i].pName==NULL)
						{
							MemCopyW(pPtr,szDone,iDoneLen);
							pPtr+=iDoneLen;
						}
						else if (pRootInfos[i].pName[0]=='\0')
						{
							MemCopyW(pPtr,szInitializing,iInitializingLen);
							pPtr+=iInitializingLen;
						}
						else
						{
							int iLen=(int)istrlenw(pRootInfos[i].pName);
							MemCopyW(pPtr,pRootInfos[i].pName,iLen);
							pPtr+=iLen;
							if (iRequired+iRequiredForRoots<MAXTIPTEXTLENGTH)
							{
								*(pPtr++)=',';
								*(pPtr++)=' ';
								if (pRootInfos[i].pRoot==NULL)
								{
									MemCopyW(pPtr,szWriting,iWritingLen);
									pPtr+=iWritingLen;
								}
								else
								{
									int iLen=(int)istrlenw(pRootInfos[i].pRoot);
									MemCopyW(pPtr,pRootInfos[i].pRoot,iLen);
									pPtr+=iLen;
								}
							}
						}

					}
					
					// Checking that space is correctly calculated
					ASSERT(iRequired+iRequiredForRoots<MAXTIPTEXTLENGTH?
						DWORD(pPtr-nid.szTip)==iRequired+iRequiredForRoots:
						DWORD(pPtr-nid.szTip)==iRequired);

					*pPtr='\0';
				}
			}
			else
			{
				WCHAR szTemp[54];
                LoadString(IDS_NOTIFYUPDATINGDBS,szTemp,54);
				StringCbPrintfW(nid.szTip,64*sizeof(WCHAR),szTemp,(int)wRunning,(int)wThreads);
			}
		}
		else
		{
			// Only one thread
			if (pRootInfos[0].pRoot==NULL) // Is writing database
				StringCbPrintfW(nid.szTip,64*sizeof(WCHAR),(LPCWSTR)ID2W(IDS_UPDATINGWRITINGDATABASE));
			else
			{
				WCHAR szBuf[50];
				LoadString(IDS_NOTIFYUPDATING,szBuf,50);
			
				LPWSTR pRoot=pRootInfos[0].pRoot;

				// Cutting to 35 characters
				int i;
				for (i=0;i<35 && pRoot[i]!='\0';i++); 
				if (i==35)
				{
					pRoot[32]='.';
					pRoot[33]='.';
					pRoot[34]='.';
					pRoot[35]='\0';
				}
				StringCbPrintfW(nid.szTip,64*sizeof(WCHAR),szBuf,pRoot);
			}
		}


		FreeRootInfos(wThreads,pRootInfos);
			
	}
	else if (uTip!=0)
	{
		if (szText!=NULL)
		{
			WCHAR szString[64];
			LoadString(uTip,szString,63);
			StringCbPrintfW(nid.szTip,63*sizeof(WCHAR),szString,szText);
		}
		else
			LoadString(uTip,nid.szTip,63);
		nid.uFlags|=NIF_TIP;
	}
	
	//DebugMessage("CLocateAppWnd::SetUpdateStatusInformation: END");
	
	return Shell_NotifyIconW(NIM_MODIFY,&nid);
}


BOOL CLocateAppWnd::StartUpdateStatusNotification()
{
	// Closing existing update status window if necessary
	if (m_pUpdateStatusWnd!=NULL)
	{
		m_pUpdateStatusWnd->DestroyWindow();
		delete m_pUpdateStatusWnd;
		m_pUpdateStatusWnd=NULL;
	}
	
	EnterCriticalSection(&m_csAnimBitmaps);
	if (m_pUpdateAnimIcons==NULL)
	{
		m_pUpdateAnimIcons=new HICON[13];
		m_pUpdateAnimIcons[0]=(HICON)LoadImage(IDI_UANIM1,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[1]=(HICON)LoadImage(IDI_UANIM2,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[2]=(HICON)LoadImage(IDI_UANIM3,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[3]=(HICON)LoadImage(IDI_UANIM4,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[4]=(HICON)LoadImage(IDI_UANIM5,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[5]=(HICON)LoadImage(IDI_UANIM6,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[6]=(HICON)LoadImage(IDI_UANIM7,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[7]=(HICON)LoadImage(IDI_UANIM8,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[8]=(HICON)LoadImage(IDI_UANIM9,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[9]=(HICON)LoadImage(IDI_UANIM10,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[10]=(HICON)LoadImage(IDI_UANIM11,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[11]=(HICON)LoadImage(IDI_UANIM12,IMAGE_ICON,16,16,0);
		m_pUpdateAnimIcons[12]=(HICON)LoadImage(IDI_UANIM13,IMAGE_ICON,16,16,0);
	
		m_nCurUpdateAnimBitmap=0;
		SetTimer(ID_UPDATEANIM,100);
	}
	LeaveCriticalSection(&m_csAnimBitmaps);
	
	if (CLocateApp::GetProgramFlags()&CLocateApp::pfEnableUpdateTooltip && 
		m_pUpdateStatusWnd==NULL)
	{
		m_pUpdateStatusWnd=new CUpdateStatusWnd;
		
		// Registering window class for notify icon handler window
		BOOL(WINAPI * pSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD)=(BOOL(WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(GetModuleHandle("user32.dll"),"SetLayeredWindowAttributes");
	
		BYTE nTransparency=0;
		DWORD dwExtra=WS_EX_TOOLWINDOW/*|WS_EX_NOACTIVATE*/;
		
		if ((CLocateApp::GetProgramFlags()&CLocateApp::pfUpdateTooltipTopmostMask)==
			CLocateApp::pfUpdateTooltipAlwaysTopmost)
			dwExtra|=WS_EX_TOPMOST;
		else if((CLocateApp::GetProgramFlags()&CLocateApp::pfUpdateTooltipTopmostMask)!=
			CLocateApp::pfUpdateTooltipNeverTopmost)
		{
			// Depends on foreground window
			HWND hForegroundWindow=GetForegroundWindow();
			if (hForegroundWindow==NULL)
				dwExtra|=WS_EX_TOPMOST;
			else
			{
				LONG dwStyle=::GetWindowLong(hForegroundWindow,GWL_STYLE);
				if (dwStyle&WS_MAXIMIZE)
				{
					if ((CLocateApp::GetProgramFlags()&CLocateApp::pfUpdateTooltipTopmostMask)==
						CLocateApp::pfUpdateTooltipNoTopmostWhdnForegroundWndInFullScreen &&
						dwStyle&WS_CAPTION)
						dwExtra|=WS_EX_TOPMOST;
				}
				else
					dwExtra|=WS_EX_TOPMOST;
      		}
		}

		if (pSetLayeredWindowAttributes!=NULL)
		{
			CRegKey2 RegKey;
			if (RegKey.OpenKey(HKCU,"Dialogs\\Updatestatus",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
			{
				DWORD dwTemp;
				if (RegKey.QueryValue("Transparency",dwTemp))
					nTransparency=(BYTE)min(dwTemp,255);
			}

			if (nTransparency>0)
				dwExtra|=WS_EX_LAYERED;
		}
		
		m_pUpdateStatusWnd->Create("LOCATEAPPUPDATESTATUS","US",WS_POPUPWINDOW/*|WS_VISIBLE*/,
			NULL,NULL,LPCSTR(0),dwExtra);

		if (nTransparency>0)
			pSetLayeredWindowAttributes(*m_pUpdateStatusWnd,0,255-nTransparency,LWA_ALPHA);

		m_pUpdateStatusWnd->SetPosition();
		m_pUpdateStatusWnd->ShowWindow(CWnd::swShowNA);
		
		
	}

	return TRUE;
}

BOOL CLocateAppWnd::StopUpdateStatusNotification()
{
	if (m_pUpdateStatusWnd!=NULL)
		m_pUpdateStatusWnd->IdleClose();
	
	EnterCriticalSection(&m_csAnimBitmaps);
	if (m_pUpdateAnimIcons!=NULL)
	{
		KillTimer(ID_UPDATEANIM);
		delete[] m_pUpdateAnimIcons;
		m_pUpdateAnimIcons=NULL;
		GetLocateAppWnd()->SetUpdateStatusInformation(DEFAPPICON,IDS_NOTIFYLOCATE);
	}
	LeaveCriticalSection(&m_csAnimBitmaps);
	return TRUE;
}
	
void CLocateAppWnd::OnInitMenuPopup(HMENU hPopupMenu,UINT nIndex,BOOL bSysMenu)
{
	CFrameWnd::OnInitMenuPopup(hPopupMenu,nIndex,bSysMenu);

	if (bSysMenu)
		return;

	if (CLocateApp::IsDatabaseMenu(hPopupMenu))
		GetLocateApp()->OnInitDatabaseMenu(CMenu(hPopupMenu));
	else if (hPopupMenu==m_Menu.GetSubMenu(0))
	{
		int iDatabaseMenu=CLocateApp::GetDatabaseMenuIndex(hPopupMenu);
		if (iDatabaseMenu!=-1)
			EnableMenuItem(hPopupMenu,iDatabaseMenu,!GetLocateApp()->IsUpdating()?MF_BYPOSITION|MF_ENABLED:MF_BYPOSITION|MF_GRAYED);

		EnableMenuItem(hPopupMenu,IDM_GLOBALUPDATEDB,!GetLocateApp()->IsUpdating()?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hPopupMenu,IDM_UPDATEDATABASES,!GetLocateApp()->IsUpdating()?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hPopupMenu,IDM_STOPUPDATING,GetLocateApp()->IsUpdating()?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED);
	}
}

BOOL CLocateAppWnd::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CFrameWnd::OnCommand(wID,wNotifyCode,hControl);
	switch (wID)
	{
	case IDM_OPENLOCATE:
		OnLocate();
		break;
	case IDM_GLOBALUPDATEDB:
		OnUpdate(FALSE);
		break;
	case IDM_UPDATEDATABASES:
		OnUpdate(FALSE,LPWSTR(-1));
		break;
	case IDM_STOPUPDATING:
		// Stopping updating quite nicely
		ASSERT(GetLocateApp()->IsUpdating());
		OnUpdate(TRUE);
		break;
	case IDM_SETTINGS:
		if (GetLocateDlg()!=NULL)
			GetLocateDlg()->SendMessage(WM_COMMAND,MAKEWPARAM(wID,wNotifyCode),(LPARAM)hControl);
		else
			OnSettings();
		break;
	case IDM_ABOUT:
		OnAbout();
		break;
	case IDC_COMEWITHME:
		// Locate dialog is closed
		if (GetLocateApp()->IsStartupFlagSet(CLocateApp::CStartData::startupLeaveBackground))
		{
			// Showing LocateST
			break;
		}
		if (GetLocateApp()->IsUpdating())
		{
			GetLocateApp()->SetStartupFlag(CLocateApp::CStartData::startupExitAfterUpdating);
			break;
		}
		if (m_pSettings!=NULL)
			break;
	case IDM_EXIT:
		if (GetLocateApp()->IsUpdating())
		{
			if (m_pLocateDlgThread!=NULL)
			{
				if (m_pLocateDlgThread->m_pLocate->ShowErrorMessage(IDS_QUITNOW,IDS_UPDATINGDATABASE,MB_YESNO|MB_ICONQUESTION)==IDYES)
					DestroyWindow();
			}
			else
			{
				switch (ShowErrorMessage(IDS_QUITNOW2,IDS_UPDATINGDATABASE,MB_YESNOCANCEL|MB_ICONQUESTION))
				{
				case IDCANCEL:
					GetLocateApp()->ClearStartupFlag(CLocateApp::CStartData::startupExitAfterUpdating);
					break;
				case IDNO:
					GetLocateApp()->SetStartupFlag(CLocateApp::CStartData::startupExitAfterUpdating);
					break;
				case IDYES:
					DestroyWindow();
					break;
				}
			}
		}
		else
			DestroyWindow();
		break;
	default:
		if (wID>=IDM_DEFUPDATEDBITEM && wID<IDM_DEFUPDATEDBITEM+1000)
			GetLocateApp()->OnDatabaseMenuItem(wID);
		break;
	}
	return FALSE;
}

BYTE CLocateAppWnd::OnAbout()
{
	if (m_pAbout==NULL)
	{
		m_pAbout=new CAboutDlg;

		if (GetLocateDlg()!=NULL)
			m_pAbout->DoModal(*GetLocateDlg());
		else
            m_pAbout->DoModal();
		delete m_pAbout;
		m_pAbout=NULL;
	}
	else
		m_pAbout->SetForegroundWindow();
	return TRUE;
}

BYTE CLocateAppWnd::OnSettings()
{
	GetLocateApp()->ClearStartupFlag(CLocateApp::CStartData::startupExitAfterUpdating);
	
	if (m_pSettings==NULL)
	{
		// Creating new settings dialog
		if (GetLocateDlg()==NULL)
			m_pSettings=new CSettingsProperties(NULL);
		else
			m_pSettings=new CSettingsProperties(*GetLocateDlg());

		// Loading settings
		m_pSettings->LoadSettings();

		// Opening dialog
		m_pSettings->DoModal();
		
		if (!m_pSettings->IsSettingsFlagSet(CSettingsProperties::settingsCancelled))
		{
			// Saving settings to registry
			m_pSettings->SaveSettings();
			
			// Set CLocateAppWnd to use new settings
			GetLocateApp()->UpdateSettings();
			if (GetLocateApp()->m_nInstance==0)
			{
				SetSchedules(m_pSettings->GetSchedules());
				SaveSchedules();
			}

			// Set icon
			LoadAppIcon();
			SetUpdateStatusInformation(DEFAPPICON,0);
			
			// Set LocateDlg to use new seetings
			CLocateDlg* pLocateDlg=GetLocateDlg();
			if (pLocateDlg!=NULL)
				pLocateDlg->UpdateSettings();
			
		}
		
		// Freeing memory
		delete m_pSettings;
		m_pSettings=NULL;

		
	}
	else
	{
		// Settings dialog is already opened, just activating it

		m_pSettings->SetWindowPos(HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
		m_pSettings->SetForegroundWindow();
	}
	return TRUE;
}

BYTE CLocateAppWnd::OnLocate()
{
	DebugMessage("CLocateAppWnd::OnLocate() BEGIN");
	GetLocateApp()->ClearStartupFlag(CLocateApp::CStartData::startupExitAfterUpdating);
	
	// Refreshing icon
	EnterCriticalSection(&m_csAnimBitmaps);
	SetUpdateStatusInformation(m_pUpdateAnimIcons!=NULL?m_pUpdateAnimIcons[m_nCurUpdateAnimBitmap]:NULL);
	LeaveCriticalSection(&m_csAnimBitmaps);
	
	
	if (m_pLocateDlgThread==NULL)
	{
		//DebugMessage("CLocateAppWnd::OnLocate() 1a");

		// Hiding LocateST
		if (GetFocus()==NULL)
			ForceForegroundAndFocus();
		
		//DebugMessage("CLocateAppWnd::OnLocate() 1b");

		m_pLocateDlgThread=new CLocateDlgThread;
		m_pLocateDlgThread->CreateThread();

		//DebugMessage("CLocateAppWnd::OnLocate() 1c");


		while (m_pLocateDlgThread->m_pLocate==NULL)
			Sleep(10);
		while (m_pLocateDlgThread->m_pLocate->GetHandle()==NULL)
			Sleep(10);

		ShowWindow(swHide);

		//DebugFormatMessage("CLocateAppWnd::OnLocate() 1e, pLocate=%X",DWORD(m_pLocateDlgThread->m_pLocate));
		//DebugFormatMessage("CLocateAppWnd::OnLocate() 1e, hWnd=%X",DWORD(m_pLocateDlgThread->m_pLocate->GetHandle()));
		
		m_pLocateDlgThread->m_pLocate->ForceForegroundAndFocus();

		//DebugMessage("CLocateAppWnd::OnLocate() 1f");
		

		
	}
	else
	{
		//DebugMessage("CLocateAppWnd::OnLocate() 2a");

		ForceForegroundAndFocus();
		
		//DebugMessage("CLocateAppWnd::OnLocate() 2b");

		CLocateDlg* pLocateDlg=GetLocateDlg();

		//DebugFormatMessage("CLocateAppWnd::OnLocate() 2c %X",DWORD(pLocateDlg));

		// Restore dialog if needed
		WINDOWPLACEMENT wp;
		wp.length=sizeof(WINDOWPLACEMENT);
		pLocateDlg->GetWindowPlacement(&wp);
		if (wp.showCmd!=SW_MAXIMIZE)
            pLocateDlg->ShowWindow(swRestore);

		//DebugMessage("CLocateAppWnd::OnLocate() 2d");


		pLocateDlg->BringWindowToTop();

		//DebugMessage("CLocateAppWnd::OnLocate() 2e");

		pLocateDlg->ForceForegroundAndFocus();
	}

	DebugMessage("CLocateAppWnd::OnLocate() END");
	return TRUE;
}

DWORD WINAPI CLocateAppWnd::KillUpdaterProc(LPVOID lpParameter)
{
	return ((CLocateApp*)GetApp())->StopUpdating(TRUE);
}



BYTE CLocateAppWnd::OnUpdate(BOOL bStopIfProcessing,LPWSTR pDatabases,int nThreadPriority)
{
	if (!GetLocateApp()->IsUpdating())
	{
		if (pDatabases==LPWSTR(-1))
		{
			CArrayFP<PDATABASE> aDatabases;
			CSelectDatabasesDlg dbd(GetLocateApp()->GetDatabases(),aDatabases,
				(GetLocateApp()->GetStartupFlags()&CLocateApp::CStartData::startupDatabasesOverridden?CSelectDatabasesDlg::flagDisablePresets:0)|
				CSelectDatabasesDlg::flagShowThreads|CSelectDatabasesDlg::flagSetUpdateState|CSelectDatabasesDlg::flagEnablePriority,
				CRegKey2::GetCommonKey()+"\\Dialogs\\SelectDatabases/Update");
			dbd.SetThreadPriority(nThreadPriority);
			if (!dbd.DoModal(m_pLocateDlgThread!=NULL?HWND(*GetLocateDlg()):HWND(*this)))
                return FALSE;
			

			if (GetLocateApp()->IsUpdating())
				return FALSE;
			if (!GetLocateApp()->GlobalUpdate(&aDatabases,dbd.GetThreadPriority()))
				return FALSE;
		}
		else if (pDatabases==NULL)
		{
			if (!GetLocateApp()->GlobalUpdate(NULL,nThreadPriority))
				return FALSE;
		}
		else
		{
			CArrayFP<PDATABASE> aDatabases;
			const CArray<PDATABASE>& aGlobalDatabases=GetLocateApp()->GetDatabases();
			for (int i=0;i<aGlobalDatabases.GetSize();i++)
			{
				LPWSTR pPtr=pDatabases;
				BOOL bFound=FALSE;
				while (*pPtr!=NULL)
				{
					int iStrLen=istrlenw(pPtr)+1;
					if (wcsncmp(pPtr,aGlobalDatabases[i]->GetName(),iStrLen)==0)
					{
						bFound=TRUE;
						break;
					}
					pPtr+=iStrLen;
				}
				if (bFound)
				{
					CDatabase* pDatabase=new CDatabase(*aGlobalDatabases[i]);
					pDatabase->UpdateGlobally(TRUE);
					pDatabase->SetThreadId(0);
					aDatabases.Add(pDatabase);
				}
			}

			if (aDatabases.GetSize()==0)
				return FALSE;
			if (!GetLocateApp()->GlobalUpdate(&aDatabases,nThreadPriority))
				return FALSE;
		}

		
	}
	else if (bStopIfProcessing)
	{
		// Starting thread which stops updating		
		DWORD dwThreadID;
		HANDLE hThread=CreateThread(NULL,0,KillUpdaterProc,(void*)this,0,&dwThreadID);
		DebugOpenThread(hThread);
		if (hThread!=NULL)
		{
			CloseHandle(hThread);
			DebugCloseThread(hThread);
		}
	}
	return TRUE;
}

void CLocateAppWnd::OnDestroy()
{
	DebugMessage("void CLocateAppWnd::OnDestroy() START");
	
	TurnOffShortcuts();
	DeleteTaskbarIcon();

	PostQuitMessage(0);
	m_Menu.DestroyMenu();

	KillTimer(ID_CHECKSCHEDULES);
	KillTimer(ID_ENSUREVISIBLEICON);
	
	// Ensure that update animation and status window are stopped
	StopUpdateStatusNotification();
	if (m_pUpdateStatusWnd!=NULL)
		m_pUpdateStatusWnd->DestroyWindow();
	
	
	



	if (m_pAbout!=NULL)
	{
		m_pAbout->DestroyWindow();
		delete m_pAbout;
		m_pAbout=NULL;
	}
	
	HANDLE hLocateThread;
	
	if (m_pLocateDlgThread!=NULL)
	{
		hLocateThread=m_pLocateDlgThread->m_hThread;

		if (m_pLocateDlgThread->IsRunning())
		{
			GetLocateDlg()->PostMessage(WM_CLOSEDIALOG);
			
			if (m_pLocateDlgThread!=NULL)
			{
				WaitForSingleObject(hLocateThread,1000);
			}
		
			if (m_pLocateDlgThread!=NULL)
			{
				DebugFormatMessage("Terminating locate dialog thread %X",(DWORD)m_pLocateDlgThread);
				TerminateThread(hLocateThread,1,TRUE);
				
				if (m_pLocateDlgThread!=NULL)
					delete m_pLocateDlgThread;
				m_pLocateDlgThread=NULL;
			}
		}
		else
		{
			delete m_pLocateDlgThread;
			m_pLocateDlgThread=NULL;
		}
	}

	((CLocateApp*)GetApp())->StopUpdating();
	
	
	SaveSchedules();

	if (m_hAppIcon!=NULL)
	{
		DeleteObject(m_hAppIcon);
		m_hAppIcon=NULL;
	}

	::SendNotifyMessage(HWND_BROADCAST,CLocateApp::m_nLocateAppMessage,
		LOCATEMSG_INSTANCEEXITED,GetLocateApp()->m_nInstance);
	
	
	CFrameWnd::OnDestroy();
	
	DebugMessage("void CLocateAppWnd::OnDestroy() END");

}

LRESULT CLocateAppWnd::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
	case WM_SYSTEMTRAY:
		return OnSystemTrayMessage((UINT)wParam,(UINT)lParam);
	case WM_GETICON:
	case WM_SETICON:
		DefWindowProc(*this,msg,wParam,lParam);
		break;
	case WM_QUERYENDSESSION:
		DebugFormatMessage("WM_QUERYENDSESSION, wParam:%X lParam:%X",wParam,lParam);
		return TRUE;	
	case WM_ENDSESSION:
		DebugFormatMessage("WM_ENDSESSION, wParam:%X lParam:%X",wParam,lParam);
		OnDestroy();
		return 0;
	case WM_GETLOCATEDLG:
		if (wParam==2)
			return (BOOL)this;
        if (m_pLocateDlgThread==NULL)
			return NULL;
		if (wParam==1)
			return (BOOL)m_pLocateDlgThread->m_pLocate;
		if (m_pLocateDlgThread->m_pLocate==NULL)
			return NULL;
		return (BOOL)(HWND)*m_pLocateDlgThread->m_pLocate;
	case WM_FREEUPDATESTATUSPOINTERS:
		if (m_pUpdateStatusWnd!=NULL)
		{
			delete m_pUpdateStatusWnd;
			m_pUpdateStatusWnd=NULL;
		}
		break;
	case WM_HOTKEY:
		if (int(wParam)>=0 && int(wParam)<m_aShortcuts.GetSize())
		{
			if (!m_aShortcuts[int(wParam)]->IsWhenAndWhereSatisfied(*this))
				break;

			if (m_aShortcuts[int(wParam)]->m_nDelay>0 && 
				m_aShortcuts[int(wParam)]->m_nDelay!=-1)
				SetTimer(ID_SHORTCUTACTIONTIMER+int(wParam),m_aShortcuts[int(wParam)]->m_nDelay);
			else if (m_aShortcuts[int(wParam)]->m_nDelay==-1 && lParam!=LPARAM(-1))
				PostMessage(WM_EXECUTESHORTCUT,wParam,LPARAM(-1));
			else           			
				m_aShortcuts[int(wParam)]->ExecuteAction();
        }
		break;
	case WM_EXECUTESHORTCUT:
		if (int(wParam)>=0 && int(wParam)<m_aShortcuts.GetSize())
		{
			if (m_aShortcuts[int(wParam)]->m_nDelay>0 && 
				m_aShortcuts[int(wParam)]->m_nDelay!=-1)
				SetTimer(ID_SHORTCUTACTIONTIMER+int(wParam),m_aShortcuts[int(wParam)]->m_nDelay);
			else if (m_aShortcuts[int(wParam)]->m_nDelay==-1 && lParam!=LPARAM(-1))
				PostMessage(WM_EXECUTESHORTCUT,wParam,LPARAM(-1));
			else           			
				m_aShortcuts[int(wParam)]->ExecuteAction();
		}
		break;
	case WM_OPENDIALOG:
		OnLocate();
		break;
	case WM_RESETSHORTCUTS:
		TurnOnShortcuts();
		break;
	default:
		if (msg==CLocateApp::m_nHFCInstallationMessage)
		{
			if (lParam!=NULL)
			{	
				char szAppLine[257];
				GlobalGetAtomName((ATOM)lParam,szAppLine,256);
				if (strcasecmp(GetApp()->GetAppName(),szAppLine)==0)
				{
					if (wParam==1 || wParam==2) // Installing (1) or UnInstalling (2)...
						DestroyWindow();
				}
			}
			return (BOOL)(HWND)*this;
		}
		else if (msg==CLocateApp::m_nTaskbarCreated)
			AddTaskbarIcon();
		else if (msg==CLocateApp::m_nLocateAppMessage)
		{
			switch (LOWORD(wParam))
			{
			case LOCATEMSG_ACTIVATEINSTANCE:
				if (GetLocateApp()->m_nInstance==SHORT(HIWORD(wParam)))
					OnActivateAnotherInstance((ATOM)lParam);
				break;
			case LOCATEMSG_INSTANCEEXITED:
				if (GetLocateApp()->m_nInstance>DWORD(lParam))
				{
					GetLocateApp()->m_nInstance--;

					if (GetLocateApp()->m_nInstance==0)
						SetSchedules();
				}
				break;
			case LOCATEMSG_EXITPROCESSS:
				DestroyWindow();
				break;
			}
		}
		break;
	}
	return CFrameWnd::WindowProc(msg,wParam,lParam);
}

void CLocateAppWnd::AddTaskbarIcon(BOOL bForce)
{
	if (!bForce && CLocateApp::GetProgramFlags()&CLocateApp::pfDontShowSystemTrayIcon)
		return;

	// Creating taskbar icon
	NOTIFYICONDATA nid;
	nid.cbSize=NOTIFYICONDATA_V1_SIZE;
	nid.hWnd=*this;
	nid.uID=1000;
	nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
	nid.uCallbackMessage=WM_SYSTEMTRAY;
	nid.hIcon=m_hAppIcon;
	LoadString(IDS_NOTIFYLOCATE,nid.szTip,63);

	Shell_NotifyIcon(NIM_ADD,&nid);

	if (((CLocateApp*)GetApp())->m_wShellDllVersion>=0x0500)
	{
		nid.cbSize=sizeof(NOTIFYICONDATA);
		nid.uVersion=NOTIFYICON_VERSION;
		Shell_NotifyIcon(NIM_SETVERSION,&nid);
	}
}

void CLocateAppWnd::DeleteTaskbarIcon(BOOL bForce)
{
	if (!bForce && CLocateApp::GetProgramFlags()&CLocateApp::pfDontShowSystemTrayIcon)
		return;

	NOTIFYICONDATA nid;
	nid.cbSize=sizeof(NOTIFYICONDATA);
	nid.hWnd=*this;
	nid.uID=1000;
	nid.uFlags=0;
	Shell_NotifyIcon(NIM_DELETE,&nid);
}


DWORD CLocateAppWnd::OnSystemTrayMessage(UINT uID,UINT msg)
{
	if (GetLocateApp()->m_wShellDllVersion>=0x0500)
	{
		switch (msg)
		{
		case WM_CONTEXTMENU:
			{
				/*POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow();
				TrackPopupMenu(m_Menu.GetSubMenu(0),
					TPM_RIGHTALIGN|TPM_BOTTOMALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,0,
					*this,NULL);*/
				break;
			}
		case NIN_KEYSELECT:
			// Keyboard space
			OnLocate();
			break;
		case NIN_SELECT:
			// One click
			break;
		case WM_LBUTTONDBLCLK:
			// Doubleclick
			OnLocate();
			break;
		case WM_LBUTTONDOWN:
			if (GetLocateApp()->GetProgramFlags()&CLocateApp::pfTrayIconClickActivate)
				OnLocate();
			break;
		case WM_RBUTTONUP:
			{
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow();
				TrackPopupMenu(m_Menu.GetSubMenu(0),
					TPM_RIGHTALIGN|TPM_BOTTOMALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,0,
					*this,NULL);
				break;
			}
			break;
		case NIN_BALLOONSHOW:
			break;
		case NIN_BALLOONHIDE:
			break;
		case NIN_BALLOONTIMEOUT:
			break;
		case NIN_BALLOONUSERCLICK:
			break;
		default:
			break;
		}
	}
	else
	{
		switch (msg)
		{
		case WM_LBUTTONDOWN:
			if (GetLocateApp()->GetProgramFlags()&CLocateApp::pfTrayIconClickActivate)
				OnLocate();
			break;
		case WM_LBUTTONDBLCLK:
			OnLocate();
			break;
		case WM_RBUTTONUP:
			{
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow();
				TrackPopupMenu(m_Menu.GetSubMenu(0),
					TPM_RIGHTALIGN|TPM_BOTTOMALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,0,
					*this,NULL);
				break;
			}
		}
	}
	return TRUE;
}

void CLocateAppWnd::OnTimer(DWORD wTimerID)
{
	//CFrameWnd::OnTimer(wTimerID);
	switch (wTimerID)
	{
	case ID_UPDATEANIM:
		// Checking this abnormal state
		if (GetLocateApp()->m_ppUpdaters==NULL)
		{
			// Stop update animation
			NotifyFinishingUpdating();
			break;
		}
		

		EnterCriticalSection(&m_csAnimBitmaps);
		m_nCurUpdateAnimBitmap++;
		if (m_nCurUpdateAnimBitmap>12)
			m_nCurUpdateAnimBitmap=0;
		SetUpdateStatusInformation(m_pUpdateAnimIcons[m_nCurUpdateAnimBitmap]);
		LeaveCriticalSection(&m_csAnimBitmaps);
		break;
	case ID_SYNCSCHEDULES:
		KillTimer(ID_SYNCSCHEDULES);
		SetTimer(ID_CHECKSCHEDULES,1000,NULL);
	case ID_CHECKSCHEDULES:
		CheckSchedules();
		break;
	case ID_ENSUREVISIBLEICON:
		if (!(CLocateApp::GetProgramFlags()&CLocateApp::pfDontShowSystemTrayIcon))
		{
			// Check icon
			NOTIFYICONDATA nid;
			nid.cbSize=NOTIFYICONDATA_V1_SIZE;
			nid.hWnd=*this;
			nid.uID=10000;
			nid.uFlags=0;
			
			if (!Shell_NotifyIcon(NIM_MODIFY,&nid))
				AddTaskbarIcon();
		}
		break;
	case ID_RUNSTARTUPSCHEDULES:
		KillTimer(ID_RUNSTARTUPSCHEDULES);
		if (RunStartupSchedules())
			SetTimer(ID_RUNSTARTUPSCHEDULES,1000);
		break;
	default:
		if (int(wTimerID)>=ID_SHORTCUTACTIONTIMER)
		{
			KillTimer(wTimerID);

			if (int(wTimerID)-ID_SHORTCUTACTIONTIMER<m_aShortcuts.GetSize())
				m_aShortcuts[int(wTimerID)-ID_SHORTCUTACTIONTIMER]->ExecuteAction();

		}
		break;		
	}
}

DWORD CLocateAppWnd::OnActivateAnotherInstance(ATOM aCommandLine)
{
	if (aCommandLine==NULL)
		OnLocate();
	else
	{
		WCHAR szCmdLine[2000];
		if (IsUnicodeSystem())
			GlobalGetAtomNameW(aCommandLine,szCmdLine,2000);
		else
		{
			char szCmdLineA[1000];
			GlobalGetAtomName(aCommandLine,szCmdLineA,2000);
			MultiByteToWideChar(CP_ACP,0,szCmdLineA,-1,szCmdLine,2000);
		}			

		CLocateApp::CStartData* pStartData=new CLocateApp::CStartData;
		CLocateApp::ParseParameters(szCmdLine,pStartData);
		if (pStartData->m_nStartup&CLocateApp::CStartData::startupDoNotOpenDialog &&
			pStartData->m_nStartup&CLocateApp::CStartData::startupUpdate)
			OnUpdate(FALSE);
		else
		{
			BOOL bUpdate=pStartData->m_nStartup&CLocateApp::CStartData::startupUpdate;
			if (m_pLocateDlgThread!=NULL)
			{
				GetLocateDlg()->SetStartData(pStartData);
				GetLocateDlg()->ShowWindow(swRestore);
				GetLocateDlg()->SetActiveWindow();
				m_pLocateDlgThread->m_pLocate->ForceForegroundAndFocus();

				delete pStartData;
			}
			else
			{
				GetLocateApp()->SetStartData(pStartData);
				OnLocate();
			}
			if (bUpdate)
				OnUpdate(FALSE);
		}
	}
	return 0;
}
		
DWORD CLocateAppWnd::SetSchedules(CList<CSchedule*>* pSchedules)
{
	if (GetLocateApp()->m_nInstance!=0)
		return 0;

	
	// Clear existing schedules
	m_Schedules.RemoveAll();

	BOOL bNeedCpuUsage=FALSE;
	
	//DebugFormatMessage("CLocateAppWnd::SetSchedules(0x%X) START",(DWORD)pSchedules);
	if (pSchedules==NULL)
	{
		CRegKey2 RegKey;
		if (RegKey.OpenKey(HKCU,"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		{
			DWORD nKeyLen=RegKey.QueryValueLength("Schedules");
			BYTE* pSchedules=new BYTE[nKeyLen];
			RegKey.QueryValue("Schedules",(LPSTR)pSchedules,nKeyLen);
#ifdef _DEBUG
			char* pTmpData=new char[nKeyLen*2+2];
			for (DWORD i=0;i<nKeyLen;i++)
				StringCbPrintf(pTmpData+i*2,3,"%02X",pSchedules[i]);
			//DebugFormatMessage("SCHEDULES(length=%d): %s",nKeyLen,pTmpData);
			delete[] pTmpData;
#endif
			if (pSchedules[1]==1)
			{
				if (nKeyLen>=6 && pSchedules[0]==SCHEDULE_V1_LEN && 
					nKeyLen==6+SCHEDULE_V1_LEN*(*(DWORD*)(pSchedules+2)))
				{
					BYTE* pPtr=pSchedules+6;
					for (DWORD n=0;n<*(DWORD*)(pSchedules+2);n++)
					{
						//DebugFormatMessage("SCHEDULEV1: type=%d",((CSchedule*)pPtr)->m_nType);
						m_Schedules.AddTail(new CSchedule(pPtr,1));
					}
				}
			}	
			else if (pSchedules[1]==2)
			{
				if (nKeyLen>=6 && pSchedules[0]==SCHEDULE_V2_LEN)
				{
					BYTE* pPtr=pSchedules+6;
					for (DWORD n=0;n<*(DWORD*)(pSchedules+2);n++)
					{
						if (pPtr+SCHEDULE_V2_LEN>=pSchedules+nKeyLen)
							break;

						//DebugFormatMessage("SCHEDULEV2: type=%d",((CSchedule*)pPtr)->m_nType);
						m_Schedules.AddTail(new CSchedule(pPtr,2));
					}
				}
			}
			else if (pSchedules[1]==3 || pSchedules[1]==4)
			{
				if (nKeyLen>=6 && pSchedules[0]==SCHEDULE_V34_LEN)
				{
					BYTE* pPtr=pSchedules+6;
					for (DWORD n=0;n<*(DWORD*)(pSchedules+2);n++)
					{
						if (pPtr+SCHEDULE_V34_LEN>=pSchedules+nKeyLen)
							break;

						//DebugFormatMessage("SCHEDULEV3: type=%d",((CSchedule*)pPtr)->m_nType);
						CSchedule* pSchedule=new CSchedule(pPtr,pSchedules[1]);
						m_Schedules.AddTail(pSchedule);

						if (pSchedule->m_bFlags&CSchedule::flagEnabled &&
							pSchedule->m_wCpuUsageTheshold!=WORD(-1))
							bNeedCpuUsage=TRUE;
					}
				}
			}
			delete[] pSchedules;
		}	
	}
	else
	{
		
		m_Schedules.Swap(*pSchedules);
		POSITION pPos=m_Schedules.GetHeadPosition();
		while (pPos!=NULL)
		{
			CSchedule* pSchedule=m_Schedules.GetAt(pPos);
			//DebugFormatMessage("SCHEDULE: type=%d",pSchedule->m_nType);
			if (pSchedule->m_bFlags&CSchedule::flagEnabled &&
				pSchedule->m_wCpuUsageTheshold!=WORD(-1))
				bNeedCpuUsage=TRUE;
			pPos=m_Schedules.GetNextPosition(pPos);
		}
	}

	if (bNeedCpuUsage)
	{
		if (m_pCpuUsage==NULL)
		{
			m_pCpuUsage=new CCpuUsage;
			m_pCpuUsage->GetCpuUsage();
		}
		else
			bNeedCpuUsage=FALSE;
	}
	else if (m_pCpuUsage!=NULL)
	{
		delete m_pCpuUsage;
		m_pCpuUsage=NULL;
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	SetTimer(ID_SYNCSCHEDULES,(bNeedCpuUsage?2000:1000)-st.wMilliseconds,NULL);
	//DebugMessage("CLocateAppWnd::SetSchedules END");
	return m_Schedules.GetCount();
}

BOOL CLocateAppWnd::SaveSchedules()
{
	if (GetLocateApp()->m_nInstance!=0)
		return 0;

	//DebugMessage("CLocateAppWnd::SaveSchedules() START");
	
	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\General",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		DWORD dwDataLen=6;
		POSITION pPos=m_Schedules.GetHeadPosition();
		while (pPos!=NULL)
		{
			dwDataLen+=m_Schedules.GetAt(pPos)->GetDataLen();
			pPos=m_Schedules.GetNextPosition(pPos);
		}		
		
		//DebugFormatMessage("dwDataLen=%d",dwDataLen);

		BYTE* pSchedules=new BYTE[dwDataLen];
		if (pSchedules==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			//DebugMessage("LocateAppWnd::OnDestroy(): Cannot allocate memory.");
		}
		pSchedules[0]=SCHEDULE_V34_LEN;
		pSchedules[1]=4; //version
		*(DWORD*)(pSchedules+2)=m_Schedules.GetCount();
		
		
		BYTE* pPtr=pSchedules+6;
		pPos=m_Schedules.GetHeadPosition();
		while (pPos!=NULL)
		{
			//DebugFormatMessage("SCHEDULE: type %d",m_Schedules.GetAt(pPos)->m_nType);
			pPtr+=m_Schedules.GetAt(pPos)->GetData(pPtr);
			pPos=m_Schedules.GetNextPosition(pPos);
		}
		//DebugMessage("Saveing schedules into registry");
		RegKey.SetValue("Schedules",(LPCSTR)pSchedules,dwDataLen,REG_BINARY);
		
#ifdef _DEBUG
		DWORD dwTmpDataLen=sizeof(CSchedule)*m_Schedules.GetCount()+6;
		char* pTmpData=new char[dwTmpDataLen*2+2];
		for (DWORD i=0;i<dwTmpDataLen;i++)
			StringCbPrintf(pTmpData+i*2,3,"%02X",pSchedules[i]);
		//DebugFormatMessage("SCHEDULES(length=%d): %s",dwTmpDataLen,pTmpData);
		delete[] pTmpData;
#endif
		delete[] pSchedules;
	}
	
	//DebugMessage("CLocateAppWnd::SaveSchedules() END");
	return TRUE;
}
	
void CLocateAppWnd::CheckSchedules()
{
	if (GetLocateApp()->IsUpdating())
		return;
	
	CSchedule::STIME tCurTime;
	CSchedule::SDATE tCurDate;
	
	DWORD nNext=(DWORD)-1;
	UINT nDayOfWeek;
	CSchedule::GetCurrentDateAndTime(&tCurDate,&tCurTime,&nDayOfWeek);
	BOOL bSchedulesChanged=FALSE;	
	
	POSITION pPos=m_Schedules.GetHeadPosition();
	while (pPos!=NULL)
	{
		CSchedule* pSchedule=m_Schedules.GetAt(pPos);
		if (pSchedule!=NULL)
		{
			DWORD nTemp=pSchedule->WhenShouldRun(tCurTime,tCurDate,nDayOfWeek);
			if (nTemp<500)
			{
				BOOL bRun=TRUE;
				if (pSchedule->m_wCpuUsageTheshold!=WORD(-1))
				{
					ASSERT(m_pCpuUsage!=NULL);

					if (m_pCpuUsage->GetCpuUsage()>pSchedule->m_wCpuUsageTheshold)
						bRun=FALSE;
				}
				if (GetLocateApp()->IsUpdating())
					bRun=FALSE;

				if (bRun)
				{
					bSchedulesChanged=TRUE;
				
					sMemCopy(&pSchedule->m_tLastStartDate,&tCurDate,sizeof(CSchedule::SDATE));
					sMemCopy(&pSchedule->m_tLastStartTime,&tCurTime,sizeof(CSchedule::STIME));
					
					pSchedule->m_bFlags|=CSchedule::flagRunned;
									
					OnUpdate(FALSE,pSchedule->m_pDatabases,pSchedule->m_nThreadPriority);
					
					if (pSchedule->m_bFlags&CSchedule::flagDeleteAfterRun)
					{
						POSITION pTmp=m_Schedules.GetNextPosition(pPos);
						delete pSchedule;
						m_Schedules.RemoveAt(pPos);
						pPos=pTmp;
						continue;
					}
				}
			}
		}
		pPos=m_Schedules.GetNextPosition(pPos);
	}
	if (bSchedulesChanged)
		SaveSchedules();
}

BOOL CLocateAppWnd::RunStartupSchedules()
{
	CSchedule::STIME tCurTime;
	CSchedule::SDATE tCurDate;
	
	DWORD nNext=(DWORD)-1;
	UINT nDayOfWeek;
	CSchedule::GetCurrentDateAndTime(&tCurDate,&tCurTime,&nDayOfWeek);
	BOOL bSchedulesChanged=FALSE;	
	BOOL bShouldBeCalledAgain=FALSE;

	POSITION pPos=m_Schedules.GetHeadPosition();
	while (pPos!=NULL)
	{
		CSchedule* pSchedule=m_Schedules.GetAt(pPos);
		if (pSchedule!=NULL)
		{
			if (pSchedule->m_nType==CSchedule::typeAtStartup && 
				pSchedule->m_bFlags&CSchedule::flagEnabled)
			{
				BOOL bRun=TRUE;
				if (pSchedule->m_wCpuUsageTheshold!=WORD(-1))
				{
					ASSERT(m_pCpuUsage!=NULL);

					if (m_pCpuUsage->GetCpuUsage()>pSchedule->m_wCpuUsageTheshold)
						bRun=FALSE;
				}
				if (GetLocateApp()->IsUpdating())
					bRun=FALSE;

				if (bRun)
				{

					bSchedulesChanged=TRUE;
					
					sMemCopy(&pSchedule->m_tLastStartDate,&tCurDate,sizeof(CSchedule::SDATE));
					sMemCopy(&pSchedule->m_tLastStartTime,&tCurTime,sizeof(CSchedule::STIME));
					
					pSchedule->m_bFlags|=CSchedule::flagRunned;
									
					OnUpdate(FALSE,pSchedule->m_pDatabases,pSchedule->m_nThreadPriority);
					
					if (pSchedule->m_bFlags&CSchedule::flagDeleteAfterRun)
					{
						POSITION pTmp=m_Schedules.GetNextPosition(pPos);
						delete pSchedule;
						m_Schedules.RemoveAt(pPos);
						pPos=pTmp;
						continue;
					}
				}
				else
					bShouldBeCalledAgain=TRUE;
			}
		}
		pPos=m_Schedules.GetNextPosition(pPos);
	}

	if (bSchedulesChanged)
		SaveSchedules();

	return bShouldBeCalledAgain;
}

BOOL CLocateApp::InitCommonRegKey()
{
	m_szCommonRegKey=ReadIniFile(&m_szCommonRegFile,
		m_pStartData!=NULL?W2A(m_pStartData->m_pSettingBranch):NULL,m_bFileIsReg);
	
	if (m_szCommonRegKey!=NULL)
	{
		if (m_szCommonRegFile!=NULL)
			LoadSettingsFromFile(m_szCommonRegKey,m_szCommonRegFile,m_bFileIsReg);
	}
	else
	{
		// Use default
		m_szCommonRegKey=alloccopy("Software\\Update");
	}

	return TRUE;
}

void CLocateApp::FinalizeCommonRegKey()
{
	if (m_szCommonRegKey==NULL)
		return;
	
	if (m_szCommonRegFile!=NULL)
	{
		SaveSettingsToFile(m_szCommonRegKey,m_szCommonRegFile,m_bFileIsReg);

		delete[] m_szCommonRegFile;
	}

	delete[] m_szCommonRegKey;
}

CAutoPtrA<CHAR> CLocateApp::GetRegKey(LPCSTR szSubKey)
{
	extern CLocateApp theApp;

	int nCommonKeyLen=istrlen(theApp.m_szCommonRegKey);
	int nSubKeyLen=istrlen(szSubKey)+1;

	char* pKey=new char[nCommonKeyLen+nSubKeyLen+(szSubKey[0]!='\\')];

	MemCopy(pKey,theApp.m_szCommonRegKey,nCommonKeyLen);
	
	if (szSubKey[0]!='\\')
		pKey[nCommonKeyLen++]='\\';

	MemCopy(pKey+nCommonKeyLen,szSubKey,nSubKeyLen);

	return pKey;
}

LPWSTR CLocateApp::FormatLastOsError()
{
	if (IsUnicodeSystem())
	{
		LPWSTR pError;
		if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,
			GetLastError(),LANG_USER_DEFAULT,(LPWSTR)&pError,0,NULL))
			return pError;
		return NULL;
	}
	else
	{
		LPSTR pError;
		DWORD dwLen=FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,
			GetLastError(),LANG_USER_DEFAULT,(LPSTR)&pError,0,NULL);
		if (dwLen>0)
		{
			LPWSTR pErrorW=(LPWSTR)LocalAlloc(LMEM_FIXED,dwLen+2);
			MultiByteToWideChar(CP_ACP,0,pError,dwLen+1,pErrorW,dwLen+2);
			LocalFree(pError);
			return pErrorW;
		}
		return NULL;
	}
}
////////////////////////////
// CLocateAppWnd::CUpdateStatusWnd
CLocateAppWnd::CUpdateStatusWnd::CUpdateStatusWnd()
:	CFrameWnd(),m_WindowSize(0,0),m_pMouseMove(NULL)
{
	
	RegisterWndClass("LOCATEAPPUPDATESTATUS",CS_HREDRAW|CS_VREDRAW,LoadCursor(NULL,IDC_ARROW),
		(HBRUSH)(COLOR_INFOBK+1),NULL);

	m_cTextColor=GetSysColor(COLOR_INFOTEXT);
	m_cTitleColor=GetSysColor(COLOR_INFOTEXT);
	m_cErrorColor=GetSysColor(COLOR_INFOTEXT);
	m_cBackColor=GetSysColor(COLOR_INFOBK);

	// Update status tooltip
	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\Dialogs\\Updatestatus",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD dwTemp;
		if (RegKey.QueryValue("TextColor",dwTemp))
			m_cTextColor=dwTemp;
		if (RegKey.QueryValue("TitleColor",dwTemp))
			m_cTitleColor=dwTemp;
		if (RegKey.QueryValue("ErrorColor",dwTemp))
			m_cErrorColor=dwTemp;
		if (RegKey.QueryValue("BackColor",dwTemp))
			m_cBackColor=dwTemp;
	}

	InitializeCriticalSection(&m_cUpdate);
}

CLocateAppWnd::CUpdateStatusWnd::~CUpdateStatusWnd()
{
	EnterCriticalSection(&m_cUpdate);
	LeaveCriticalSection(&m_cUpdate);

	DeleteCriticalSection(&m_cUpdate);

	
	m_aErrors.RemoveAll();

	m_Font.DeleteObject();
	m_TitleFont.DeleteObject();

	
}
	
int CLocateAppWnd::CUpdateStatusWnd::OnCreate(LPCREATESTRUCT lpcs)
{
	SetTimer(ID_UPDATESTATUS,500,NULL);
	SetTimer(ID_CHECKFOREGROUNDWND,100,NULL);
	return CFrameWnd::OnCreate(lpcs);
}



void CLocateAppWnd::CUpdateStatusWnd::OnDestroy()
{
	CTargetWnd::OnDestroy();
	if (m_pMouseMove!=NULL)
	{
		delete m_pMouseMove;
		m_pMouseMove=NULL;
		ReleaseCapture();			
	}

	SavePosition(HKCU,CRegKey2::GetCommonKey()+"\\Dialogs\\Updatestatus","WindowPos");

}



void CLocateAppWnd::CUpdateStatusWnd::OnNcDestroy()
{
	CTargetWnd::OnNcDestroy();

	// If this is NULL, class is deleted anyway
	if (GetLocateAppWnd()->m_pUpdateStatusWnd!=NULL)
		GetLocateAppWnd()->PostMessage(WM_FREEUPDATESTATUSPOINTERS);
	else
		delete this;
}


void CLocateAppWnd::CUpdateStatusWnd::OnTimer(DWORD wTimerID)
{
	switch (wTimerID)
	{
	case ID_UPDATESTATUS:
		Update();
		break;
	case ID_IDLEEXIT:
		KillTimer(ID_IDLEEXIT);
		CWnd::DestroyWindow();
		break;
	case ID_CHECKFOREGROUNDWND:
		CheckForegroundWindow();
		break;
	}
}

#define EXTRA_MARGINSX 3
#define EXTRA_MARGINSY 3
#define EXTRA_LINES 2

void CLocateAppWnd::CUpdateStatusWnd::OnPaint()
{
	CPaintDC dc(this);

	dc.SetMapMode(MM_TEXT);
	dc.SetBkMode(TRANSPARENT);
	dc.SelectObject(m_TitleFont);

	EnterCriticalSection(&m_cUpdate);
	
	RECT rcClient,rc2;
	GetClientRect(&rcClient);
	rcClient.left+=EXTRA_MARGINSX;
	//rcClient.top+=EXTRA_MARGINSY;

	
    // Drawing title
	LPCWSTR pPtr=m_sStatusText;
	int nLength=(int)FirstCharIndex(pPtr,L'\n');
	rc2=rcClient;
	dc.SetTextColor(m_cTitleColor);
	dc.DrawText(pPtr,nLength,&rc2,DT_SINGLELINE|DT_CALCRECT);
	dc.DrawText(pPtr,nLength,&rc2,DT_SINGLELINE);
	rcClient.top+=rc2.bottom-rc2.top+EXTRA_LINES;

	dc.SelectObject(m_Font);
	
	// Drawing texts
	dc.SetTextColor(m_cTextColor);
	while (nLength!=-1)
	{
		pPtr+=nLength+1;
		nLength=(int)FirstCharIndex(pPtr,L'\n');

		rc2=rcClient;
		dc.DrawText(pPtr,nLength,&rc2,DT_SINGLELINE|DT_CALCRECT);
		dc.DrawText(pPtr,nLength,&rc2,DT_SINGLELINE);
	
		rcClient.top+=rc2.bottom-rc2.top+EXTRA_LINES;
	}

	// Drawing errors
	dc.SetTextColor(m_cErrorColor);
	for (int i=0;i<m_aErrors.GetSize();i++)
	{
		rc2=rcClient;	
		dc.DrawText(m_aErrors[i],-1,&rc2,DT_SINGLELINE|DT_CALCRECT);
		dc.DrawText(m_aErrors[i],-1,&rc2,DT_SINGLELINE);
	
		rcClient.top+=rc2.bottom-rc2.top+EXTRA_LINES;
	}

	LeaveCriticalSection(&m_cUpdate);
}


void CLocateAppWnd::CUpdateStatusWnd::FormatErrorForStatusTooltip(UpdateError ueError,CDatabaseUpdater* pUpdater)
{
	if (ueError==ueStopped)
		return;

	EnterCriticalSection(&m_cUpdate);

	// Now, change pointer to null, if someone is accesing pointer, it may have enough time to read
	WCHAR error[300];
	int nLabelLength=LoadString(IDS_LASTERROR,error,200);

	LPCWSTR szExtra=NULL;

	switch(ueError)
	{
	case ueUnknown:
		LoadString(IDS_LASTERRORUNKNOWN,error+nLabelLength,300-nLabelLength);
		szExtra=pUpdater->GetCurrentDatabaseName();
		break;
	case ueCreate:
	case ueOpen:
		LoadString(IDS_LASTERRORCANNOTOPEN,error+nLabelLength,300-nLabelLength);
		szExtra=pUpdater->GetCurrentDatabaseName();
		break;
	case ueRead:
		LoadString(IDS_LASTERRORCANNOTREAD,error+nLabelLength,300-nLabelLength);
		szExtra=pUpdater->GetCurrentDatabaseName();
		break;
	case ueWrite:
		LoadString(IDS_LASTERRORCANNOTWRITE,error+nLabelLength,300-nLabelLength);
		szExtra=pUpdater->GetCurrentDatabaseName();
		break;
	case ueAlloc:
		LoadString(IDS_LASTERRORCANNOTALLOCATE,error+nLabelLength,300-nLabelLength);
		break;
	case ueInvalidDatabase:
		LoadString(IDS_LASTERRORINVALIDDB,error+nLabelLength,300-nLabelLength);
		szExtra=pUpdater->GetCurrentDatabaseName();
		break;
	case ueFolderUnavailable:
		LoadString(IDS_LASTERRORROOTUNAVAILABLE,error+nLabelLength,300-nLabelLength);
		szExtra=pUpdater->GetCurrentRootPath();
		break;
	case ueCannotIncrement:
		LoadString(IDS_LASTERRORCANNOTINCREMENTDB,error+nLabelLength,300-nLabelLength);
		szExtra=pUpdater->GetCurrentDatabaseName();
		break;
	default:
		StringCbPrintfW(error+nLabelLength,(300-nLabelLength)*sizeof(WCHAR),L"%d",(int)ueError);
		break;
	}

	LPWSTR szNewPtr;
	int nLength;
	if (szExtra!=NULL)
	{
		int iLen=(int)istrlenw(error)+(int)istrlenw(szExtra)+1;
		szNewPtr=new WCHAR[iLen];
		StringCbPrintfW(szNewPtr,iLen*sizeof(WCHAR),error,szExtra);
		nLength=istrlenw(szNewPtr);
	}
	else
	    szNewPtr=alloccopy(error,nLength=istrlenw(error));

	m_aErrors.Add(szNewPtr);

	// Make status window bigger
	{
		CDC dc(this);
		dc.SetMapMode(MM_TEXT);
		dc.SelectObject(m_Font);
		CSize sz=dc.GetTextExtent(szNewPtr,(int)nLength);
	
		if (m_WindowSize.cx<sz.cx)
			m_WindowSize.cx=sz.cx;
		m_WindowSize.cy+=sz.cy+EXTRA_MARGINSY;
	}
	
	LeaveCriticalSection(&m_cUpdate);

	SetPosition();
}


void CLocateAppWnd::CUpdateStatusWnd::FormatStatusTextLine(CStringW& str,const CLocateAppWnd::RootInfo& pRootInfo,int nThreadID,int nThreads)
{
	// #X  thread number
	if (nThreadID!=-1)
		str << L'#' << (int)nThreadID;

	if (pRootInfo.pName==NULL)
	{
		// Finished
		if (nThreadID!=-1)
			str << L": ";

		switch (pRootInfo.ueError)
		{
		case ueStopped:
			str.AddString(IDS_UPDATINGCANCELLED);
			break;
		case ueFolderUnavailable:
		case ueSuccess:
			str.AddString(IDS_NOTIFYDONE);
			break;
		default:
			str.AddString(IDS_UPDATINGFAILED2);
			break;
		}
	}
	else if (pRootInfo.pName[0]==L'\0')
	{
		// Initializing/finishing
		if (nThreadID!=-1)
			str << L": ";

		if (pRootInfo.ueError==ueSuccess)
			str.AddString(IDS_NOTIFYFINISHING);
		else
			str.AddString(IDS_NOTIFYINITIALIZING);
	}
	else
	{
		if (pRootInfo.dwNumberOfDatabases>1)
		{
			if (nThreadID!=-1)
                str << L' ';
			str << (int)(pRootInfo.dwCurrentDatabase+1) << L'/' << (int)pRootInfo.dwNumberOfDatabases << L": ";
		}
		else if (nThreadID!=-1)
			str << L": ";
		str.AddString(IDS_UPDATINGUPDATING);
		str << pRootInfo.pName;

		
		if (pRootInfo.pRoot==NULL)
		{
			str << L": ";
			str.AddString(IDS_NOTIFYWRITINGDATABASE);
		}
		else
		{
			if (pRootInfo.wProgressState!=WORD(-1))
			{
				str << L' ' << (int)((int)(pRootInfo.wProgressState)/10) << L'%';
			}

			str << L": ";
			str.AddString(IDS_UPDATINGSCANNINGROOT);
			str << pRootInfo.pRoot;
		}
	}
}

void CLocateAppWnd::CUpdateStatusWnd::FillFontStructs(LOGFONT* pTextFont,LOGFONT* pTitleFont)
{
	int nDeviceCaps;
	{
		// Getting device caps
		HDC hScreenDC=::GetDC(NULL);
		nDeviceCaps=::GetDeviceCaps(hScreenDC,LOGPIXELSY);
		::ReleaseDC(NULL,hScreenDC);
	}

	if (pTextFont!=NULL)
	{
		ZeroMemory(pTextFont,sizeof(LOGFONT));
		pTextFont->lfHeight=-MulDiv(8, nDeviceCaps, 72);
		pTextFont->lfWeight=FW_NORMAL;
		//pTextFont->lfCharSet=ANSI_CHARSET;
		//pTextFont->lfOutPrecision=OUT_DEFAULT_PRECIS;
		//pTextFont->lfClipPrecision=CLIP_DEFAULT_PRECIS;
		//pTextFont->lfQuality=DEFAULT_QUALITY;
		//pTextFont->lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;
		StringCbCopy(pTextFont->lfFaceName,LF_FACESIZE,"Tahoma");
	}

	if (pTitleFont!=NULL)
	{
		ZeroMemory(pTitleFont,sizeof(LOGFONT));
		pTitleFont->lfHeight=-MulDiv(10, nDeviceCaps, 72);
		pTitleFont->lfWeight=FW_BOLD;
		//pTitleFont->lfCharSet=ANSI_CHARSET;
		//pTitleFont->lfOutPrecision=OUT_DEFAULT_PRECIS;
		//pTitleFont->lfClipPrecision=CLIP_DEFAULT_PRECIS;
		//pTitleFont->lfQuality=DEFAULT_QUALITY;
		//pTitleFont->lfPitchAndFamily=DEFAULT_PITCH|FF_DONTCARE;
		StringCbCopy(pTitleFont->lfFaceName,LF_FACESIZE,"Tahoma");
	}
}


void CLocateAppWnd::CUpdateStatusWnd::SetFonts()
{
	if (m_TitleFont.m_hObject!=NULL && m_Font.m_hObject!=NULL)
		return;

	// Update status tooltip
	CRegKey2 RegKey;
	LOGFONT lTitleFont,lTextFont;
	if (RegKey.OpenKey(HKCU,"\\Dialogs\\Updatestatus",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		if (RegKey.QueryValue("TextFont",(LPSTR)&lTextFont,sizeof(LOGFONT))<sizeof(LOGFONT))
			CLocateAppWnd::CUpdateStatusWnd::FillFontStructs(&lTextFont,NULL);
	
		if (RegKey.QueryValue("TitleFont",(LPSTR)&lTitleFont,sizeof(LOGFONT))<sizeof(LOGFONT))
			CLocateAppWnd::CUpdateStatusWnd::FillFontStructs(NULL,&lTitleFont);
	}
	else
		CLocateAppWnd::CUpdateStatusWnd::FillFontStructs(&lTextFont,&lTitleFont);

	
	if (m_TitleFont.m_hObject==NULL)
	{
		if (!m_TitleFont.CreateFontIndirect(&lTitleFont))
		{
			lTitleFont.lfFaceName[0]='\0';
			if (!m_TitleFont.CreateFontIndirect(&lTitleFont))
				m_TitleFont.CreateStockObject(DEFAULT_GUI_FONT);
		}
		
	}
	if (m_Font.m_hObject==NULL)
	{
		if (!m_Font.CreateFontIndirect(&lTextFont))
		{
			lTextFont.lfFaceName[0]='\0';
			if (!m_Font.CreateFontIndirect(&lTextFont))
				m_Font.CreateStockObject(DEFAULT_GUI_FONT);
		}
		
	}
}

LRESULT CLocateAppWnd::CUpdateStatusWnd::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		{
			HBRUSH hBrush=CreateSolidBrush(m_cBackColor);
			if (hBrush!=NULL)
			{
				RECT rc;
				GetClientRect(&rc);
                FillRect((HDC)wParam,&rc,hBrush);
				DeleteObject(hBrush);
				return TRUE;
			}
			break;
		}
	case WM_LBUTTONDOWN:
		if (m_pMouseMove==NULL)
		{
			m_pMouseMove=new MouseMove;
			m_pMouseMove->nStartPointX=SHORT(LOWORD(lParam));
			m_pMouseMove->nStartPointY=SHORT(HIWORD(lParam));

			ASSERT(m_pMouseMove->nStartPointX>=0 && m_pMouseMove->nStartPointX<1600);
			ASSERT(m_pMouseMove->nStartPointY>=0 && m_pMouseMove->nStartPointY<1600);


			SetCapture();
		}
		break;
	case WM_LBUTTONUP:
		if (m_pMouseMove!=NULL)
		{
			delete m_pMouseMove;
			m_pMouseMove=NULL;
			ReleaseCapture();			
		}
		break;
	}
	return CTargetWnd::WindowProc(msg,wParam,lParam);
}

void CLocateAppWnd::CUpdateStatusWnd::OnMouseMove(UINT fwKeys,WORD xPos,WORD yPos)
{
	if (m_pMouseMove!=NULL)
	{
		RECT rcWindowRect;
		GetWindowRect(&rcWindowRect);

		LONG nNewCoordX=rcWindowRect.left+(SHORT(xPos)-m_pMouseMove->nStartPointX);
		LONG nNewCoordY=rcWindowRect.top+(SHORT(yPos)-m_pMouseMove->nStartPointY);
		

		SetWindowPos(NULL,nNewCoordX,nNewCoordY,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
	}
}
	
void CLocateAppWnd::CUpdateStatusWnd::SetPosition()
{	
	CRect rcDesktopRect,rcTrayRect;
	CPoint ptUpperLeft;
	CSize szSize;
	BOOL bAdjustDesktopRect=FALSE; // If TRUE, remove tray are from rcDesktopRect

	HWND hShellTrayWnd=FindWindow("Shell_TrayWnd",NULL); // Whole tray window
	HWND hDesktop=FindWindow("Progman","Program Manager");
	
	if (hDesktop==NULL)
		hDesktop=GetDesktopWindow();
	
	
	// Bound monitor arrea
	HMONITOR (WINAPI *pMonitorFromWindow)(HWND,DWORD);
	pMonitorFromWindow=(HMONITOR (WINAPI *)(HWND,DWORD))GetProcAddress(GetModuleHandle("user32.dll"),"MonitorFromWindow");
	if (pMonitorFromWindow!=NULL)
	{
		HMONITOR hCurrentMonitor=MonitorFromWindow(hShellTrayWnd!=NULL?hShellTrayWnd:*this,MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(hCurrentMonitor, &mi);
		
		rcDesktopRect=mi.rcWork;
	}
	else
	{
		// Get screen size
		::GetWindowRect(hDesktop,&rcDesktopRect);
		bAdjustDesktopRect=TRUE;
	}

	// Compute center point
	POINT ptDesktopCenter={(rcDesktopRect.left+rcDesktopRect.right)/2,(rcDesktopRect.top+rcDesktopRect.bottom)/2};
	

	// Computing width and height
	if (m_WindowSize.cx==0 && m_WindowSize.cy==0)
	{
		SetFonts();
	
		CDC dc(this);
		dc.SetMapMode(MM_TEXT);

		// Checking how much space title will take
		CStringW str(IDS_UPDATINGTOOLTIPTITLE);
		dc.SelectObject(m_TitleFont);
		szSize=dc.GetTextExtent(str);	
		
		str.LoadString(IDS_UPDATINGENDED);
		EnlargeSizeForText(dc,str,szSize);
		
        // Changing font
		dc.SelectObject(m_Font);
		
		EnterCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
		if (GetLocateApp()->m_ppUpdaters!=NULL)
		{
			WORD wThreads;
			for (wThreads=0;GetLocateApp()->m_ppUpdaters[wThreads]!=NULL;wThreads++);

            CLocateAppWnd::RootInfo ri;
			for (int i=0;GetLocateApp()->m_ppUpdaters[i]!=NULL;i++)
			{
				szSize.cy+=EXTRA_LINES;

				ri.pName=NULL;
				ri.pRoot=NULL;
				ri.ueError=ueSuccess;
				ri.wProgressState=1000;

				// Cheking how much space "finished" will take
				str.Empty();
				FormatStatusTextLine(str,ri,wThreads>1?i+1:-1,wThreads);
				CSize szThisLine=dc.GetTextExtent(str);
				
				// Cheking how much space "initializing/finished" will take
				ri.pName=(LPWSTR)szwEmpty;
				str.Empty();
				ri.ueError=ueStillWorking;
				FormatStatusTextLine(str,ri,wThreads>1?i+1:-1,wThreads);
				EnlargeSizeForText(dc,str,szThisLine);
				ri.ueError=ueSuccess;
				FormatStatusTextLine(str,ri,wThreads>1?i+1:-1,wThreads);
				EnlargeSizeForText(dc,str,szThisLine);
					
				
				LPWSTR szFile=NULL;
				CDatabase::ArchiveType nArchiveType;
				CDatabaseUpdater::CRootDirectory* pRoot;
				ri.dwNumberOfDatabases=GetLocateApp()->m_ppUpdaters[i]->GetNumberOfDatabases();
				for (ri.dwCurrentDatabase=0;GetLocateApp()->m_ppUpdaters[i]->EnumDatabases(ri.dwCurrentDatabase,ri.pName,szFile,nArchiveType,pRoot);ri.dwCurrentDatabase++)
				{
					// Checking how much space "writing database" will take
					ri.pRoot=NULL;	
					str.Empty();
					FormatStatusTextLine(str,ri,wThreads>1?i+1:-1,wThreads);
					EnlargeSizeForText(dc,str,szThisLine);
				
					while (pRoot!=NULL)
					{
						// Checking how much space "scanning root" will take
						ri.pRoot=pRoot->m_Path.GetBuffer();
						str.Empty();
						FormatStatusTextLine(str,ri,wThreads>1?i+1:-1,wThreads);
						EnlargeSizeForText(dc,str,szThisLine);
						pRoot=pRoot->m_pNext;
					}
					
				}

				szSize.cy+=szThisLine.cy;
				if (szSize.cx<szThisLine.cx)
					szSize.cx=szThisLine.cx;

			}
		}    
		LeaveCriticalSection(&GetLocateApp()->m_cUpdatersPointersInUse);
		
		szSize.cx+=2*EXTRA_MARGINSX;
		szSize.cy+=2*EXTRA_MARGINSY;

	}
	else
		szSize=m_WindowSize;

	// This is postion to where tooltip is placed
	DWORD nPosition=CLocateApp::pfUpdateTooltipPositionDownRight;
	
	// Checking where tray window is
	if (hShellTrayWnd!=NULL)
	{
		::GetWindowRect(hShellTrayWnd,&rcTrayRect);
	
		// Resolvinf position near clock and removing tray are from rcDekstopRect
		if (rcTrayRect.top>ptDesktopCenter.y)
		{
			// Tray is on bottom
			nPosition=CLocateApp::pfUpdateTooltipPositionDownRight;
			
			if (bAdjustDesktopRect)
				rcDesktopRect.bottom=rcTrayRect.top;
		}
		else if (rcTrayRect.bottom<ptDesktopCenter.y)
		{
			// Tray is on top
			nPosition=CLocateApp::pfUpdateTooltipPositionUpRight;
			if (bAdjustDesktopRect)
				rcDesktopRect.top=rcTrayRect.bottom;
		}
		else if (rcTrayRect.right<ptDesktopCenter.x)
		{
			// Tray is on left
			nPosition=CLocateApp::pfUpdateTooltipPositionDownLeft;
			if (bAdjustDesktopRect)
				rcDesktopRect.left=rcTrayRect.right;
		}
		else
		{
			// Tray is on right
			nPosition=CLocateApp::pfUpdateTooltipPositionDownRight;
			if (bAdjustDesktopRect)
				rcDesktopRect.right=rcTrayRect.left;
		}
	}

	switch (GetLocateApp()->GetProgramFlags()&CLocateApp::pfUpdateTooltipPositionMask)
	{
	case CLocateApp::pfUpdateTooltipPositionDefault:
		break;
	case CLocateApp::pfUpdateTooltipPositionLastPosition:
		if (LoadPosition(HKCU,CRegKey2::GetCommonKey()+"\\Dialogs\\Updatestatus","WindowPos",fgOnlyNormalPosition))
			return;
		break;
	default:
		nPosition=GetLocateApp()->GetProgramFlags()&CLocateApp::pfUpdateTooltipPositionMask;
		break;
	}

	if ((nPosition&CLocateApp::pfUpdateTooltipPositionDown)==CLocateApp::pfUpdateTooltipPositionDown)
		ptUpperLeft.y=rcDesktopRect.bottom-szSize.cy-2;
	else
		ptUpperLeft.y=rcDesktopRect.top+2;
	
	if ((nPosition&CLocateApp::pfUpdateTooltipPositionRight)==CLocateApp::pfUpdateTooltipPositionRight)
		ptUpperLeft.x=rcDesktopRect.right-szSize.cx-2;
	else
		ptUpperLeft.x=rcDesktopRect.left+2;

		
	CLocateDlg* pLocateDlg=GetLocateDlg();
	
	if (pLocateDlg!=NULL)
	{
		SetWindowPos(HWND_TOP,ptUpperLeft.x,ptUpperLeft.y,szSize.cx,szSize.cy,
			GetForegroundWindow()==*pLocateDlg?SWP_NOACTIVATE:SWP_NOACTIVATE|SWP_NOZORDER);
	}
	else 
		SetWindowPos(NULL,ptUpperLeft.x,ptUpperLeft.y,szSize.cx,szSize.cy,SWP_NOZORDER|SWP_NOACTIVATE);


	m_WindowSize=szSize;
}

void CLocateAppWnd::CUpdateStatusWnd::Update()
{
	WORD wThreads,wRunning;
	RootInfo* pRootInfos;
	if (GetLocateAppWnd()->GetRootInfos(wThreads,wRunning,pRootInfos))
	{
		Update(wThreads,wRunning,pRootInfos);
		CLocateAppWnd::FreeRootInfos(wThreads,pRootInfos);
	}
	else
	{
		Update(0,0,NULL);
		IdleClose();
	}
}

void CLocateAppWnd::CUpdateStatusWnd::CheckForegroundWindow()
{		
	// Check window status
    if ((CLocateApp::GetProgramFlags()&CLocateApp::pfUpdateTooltipTopmostMask)==
		CLocateApp::pfUpdateTooltipNoTopmostWhenForegroundWndMaximized ||
		(CLocateApp::GetProgramFlags()&CLocateApp::pfUpdateTooltipTopmostMask)==
		CLocateApp::pfUpdateTooltipNoTopmostWhdnForegroundWndInFullScreen)
	{
		BOOL bTopMost=FALSE;
		HWND hForegroundWindow=GetForegroundWindow();

		if (hForegroundWindow==NULL)
			bTopMost=TRUE;
		else
		{
			LONG dwStyle=::GetWindowLong(hForegroundWindow,GWL_STYLE);
			if (dwStyle&WS_MAXIMIZE)
			{
				if ((CLocateApp::GetProgramFlags()&CLocateApp::pfUpdateTooltipTopmostMask)==
					CLocateApp::pfUpdateTooltipNoTopmostWhdnForegroundWndInFullScreen &&
					dwStyle&WS_CAPTION)
					bTopMost=TRUE;
			}
			else
				bTopMost=TRUE;
      	}

		if (GetExStyle()&WS_EX_TOPMOST)
		{
			if (!bTopMost)
			{
                SetWindowPos(HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
				::SetWindowPos(hForegroundWindow,*this,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
			}
		}
		else if (bTopMost)
			SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
		
	}
}

void CLocateAppWnd::CUpdateStatusWnd::Update(WORD wThreads,WORD wRunning,RootInfo* pRootInfos)
{
	EnterCriticalSection(&m_cUpdate);
	
	
	m_sStatusText.Empty();

	// Forming title
	if (wRunning>0)
		m_sStatusText.LoadString(IDS_UPDATINGTOOLTIPTITLE);
	else
		m_sStatusText.LoadString(IDS_UPDATINGENDED);

	if (pRootInfos!=NULL)
	{
		for (WORD i=0;i<wThreads;i++)
		{
			m_sStatusText << L'\n';
			FormatStatusTextLine(m_sStatusText,pRootInfos[i],wThreads>1?i+1:-1,wThreads);
		}
	}
	else
	{
		m_sStatusText << L'\n';
		LPWSTR pError=CLocateApp::FormatLastOsError();
		if (pError!=NULL)
		{
			CStringW str;
			str.Format(IDS_ERRORUNKNOWNOS,pError);
			while (str.LastChar()=='\n' || str.LastChar()=='\r')
			str.DelLastChar();
			m_sStatusText << str;
		}
		else
			m_sStatusText.AddString(IDS_ERRORUNKNOWN);
	}
		
		

	LeaveCriticalSection(&m_cUpdate);

	InvalidateRect(NULL,TRUE);
}


void CLocateAppWnd::CUpdateStatusWnd::IdleClose()
{
	KillTimer(ID_UPDATESTATUS);
	SetTimer(ID_IDLEEXIT,4000,NULL);
}

BOOL CLocateAppWnd::CUpdateStatusWnd::DestroyWindow()
{
	KillTimer(ID_IDLEEXIT);
	KillTimer(ID_CHECKFOREGROUNDWND);
	GetLocateAppWnd()->m_pUpdateStatusWnd=NULL;
	CWnd::DestroyWindow();
	return TRUE;
}


void CLocateAppWnd::OnHelp(LPHELPINFO lphi)
{
	if (GetLocateDlg()!=NULL)
	{
		GetLocateDlg()->OnHelp(lphi);
		return;
	}

	CTargetWnd::OnHelp(lphi);

	if (lphi->iContextType==HELPINFO_MENUITEM)
	{
		// Menu item
		if (HtmlHelp(HH_HELP_CONTEXT,lphi->iCtrlId)==NULL)
		{
			// No topic found, show topics window
			HtmlHelp(HH_DISPLAY_TOPIC,0);
		}
	}
}


CLocateApp::LocaleNumberFormat::LocaleNumberFormat()
:	uLeadingZero(1),uGrouping(3)
{
	
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,"Control Panel\\International",CRegKey::defRead)==ERROR_SUCCESS)
	{
		WCHAR szTemp[10];
		int nLen;

		if (RegKey.QueryValue(L"iLZero",szTemp,10)>1)
			uLeadingZero=_wtoi(szTemp);
		if (RegKey.QueryValue(L"sGrouping",szTemp,10)>1)
			uGrouping=_wtoi(szTemp);

		nLen=RegKey.QueryValueLength(L"sDecimal");
		if (nLen>1)
		{
			pDecimal=new WCHAR[nLen+1];
			RegKey.QueryValue(L"sDecimal",pDecimal,nLen);
		}
		else
		{
			pDecimal=new WCHAR[2];
			pDecimal[0]=L'.';
			pDecimal[1]=L'\0';
		}

		nLen=RegKey.QueryValueLength(L"sThousand");
		if (nLen>1)
		{
			pThousand=new WCHAR[nLen+1];
			RegKey.QueryValue(L"sThousand",pThousand,nLen);
		}
		else
		{
			pThousand=new WCHAR[2];
			pThousand[0]=L' ';
			pThousand[1]=L'\0';
		}
	}
}


CLocateApp theApp;

