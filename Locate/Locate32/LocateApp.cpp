#include <HFCLib.h>
#include "Locate32.h"

#include "wfext.h"
#include "shlwapi.h"

CLocateApp::CLocateApp()
:	CWinApp("LOCATE32"),m_nDelImage(0),m_nStartup(0),
	m_ppUpdaters(NULL),m_pLastDatabase(NULL),m_nFileSizeFormat(fsfOverKBasKB),
	m_dwProgramFlags(pfDefault)
{
	DebugMessage("CLocateApp::CLocateApp()");
	m_pStartData=new CStartData;

	m_hUpdatersPointerInUse=CreateMutex(NULL,FALSE,NULL);
}

CLocateApp::~CLocateApp()
{
	DebugMessage("CLocateApp::~CLocateApp()");
	if (m_pStartData!=NULL)
	{
		delete m_pStartData;
		m_pStartData=NULL;
	}

	if (m_hUpdatersPointerInUse!=NULL)
	{
		CloseHandle(m_hUpdatersPointerInUse);
		m_hUpdatersPointerInUse=NULL;
	}
}

BOOL CLocateApp::InitInstance()
{
	CWinApp::InitInstance();

	DebugNumMessage("CLocateApp::InitInstance(), thread is 0x%X",GetCurrentThreadId());

	// Initializing HFC library
	if (!InitializeHFC())
	{
		MessageBox(NULL,"InitializeHFC() returned FALSE","ERROR",MB_ICONSTOP);
		m_nStartup|=CStartData::startupExitedBeforeInitialization;
		return FALSE;
	}
	
	// Initializing locater library
	InitLocaterLibrary();

	m_pGetLongPathName=(DWORD(WINAPI *)(LPCSTR,LPSTR,DWORD))GetProcAddress(GetModuleHandle("kernel32.dll"),"GetLongPathNameA");
	if (m_pGetLongPathName==NULL)
		m_pGetLongPathName=CLocateApp::GetLongPathName;

	// Handling command line arguments
	DebugFormatMessage("CommandLine: %s",m_lpCmdLine);
	ParseParameters(m_lpCmdLine,m_pStartData);
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
		if (ChechOtherInstances())
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
	return 0;
}

INT_PTR CALLBACK CLocateApp::DummyDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return FALSE;
}

	
BOOL CLocateApp::ParseParameters(LPCTSTR lpCmdLine,CStartData* pStartData)
{
	int idx=0,temp;
	while (lpCmdLine[idx]==' ') idx++;
	if (lpCmdLine[idx]=='/' || lpCmdLine[idx]=='-')
	{
		switch(lpCmdLine[++idx])
		{
		case 'P': // put 'path' to 'Look in' field
			idx++;
			if (lpCmdLine[idx]==':')
				idx++;
			while(lpCmdLine[idx]==' ') idx++;
			if (lpCmdLine[idx]!='\"')
			{
				temp=FirstCharIndex(lpCmdLine+idx,' ');
				ChangeAndAlloc(pStartData->m_pStartPath,lpCmdLine+idx,temp);
				if (temp<0)
					return TRUE;
				idx+=temp;
			}
			else
			{
				idx++;
				temp=FirstCharIndex(lpCmdLine+idx,'\"');
				ChangeAndAlloc(pStartData->m_pStartPath,lpCmdLine+idx,temp);
				if (temp<0)
					return TRUE;
				idx+=temp+1;
			}
			break;				
		case 'p': // Check also if path is correct
			idx++;
			if (lpCmdLine[idx]==':')
				idx++;
			while(lpCmdLine[idx]==' ') idx++;
			if (lpCmdLine[idx]!='\"')
			{
				temp=FirstCharIndex(lpCmdLine+idx,' ');
				if (temp!=-1)
					*(char*)(lpCmdLine+idx+temp)='\0'; // Setting line end for 
				int nLength=0;

				char szPath[MAX_PATH+10];
				nLength=GetLocateApp()->m_pGetLongPathName(lpCmdLine+idx,szPath,MAX_PATH+10);
				ChangeAndAlloc(pStartData->m_pStartPath,szPath,nLength);
								
				if (temp<0)
					return TRUE;
				*(char*)(lpCmdLine+idx+temp)=' '; // Setting line end for 
				idx+=temp;
			}
			else
			{
				idx++;
				temp=FirstCharIndex(lpCmdLine+idx,'\"');
				if (temp!=-1)
					*(char*)(lpCmdLine+idx+temp)='\0'; // Setting line end for 
				int nLength;

				char szPath[MAX_PATH+10];
				nLength=GetLocateApp()->m_pGetLongPathName(lpCmdLine+idx,szPath,200);
				ChangeAndAlloc(pStartData->m_pStartPath,szPath,nLength);
				if (temp<0)
					return TRUE;
				*(char*)(lpCmdLine+idx+temp)='\"'; // Setting line end for 
				idx+=temp+1;
			}
			break;				
		case 'c':
		case 'C':
			{
				OpenClipboard(NULL);
				HANDLE hData=GetClipboardData(CF_TEXT);
				if (hData!=NULL)
				{
					LPCSTR szLine=(LPCSTR)GlobalLock(hData);
					if (szLine!=NULL)
						ChangeAndAlloc(pStartData->m_pStartString,szLine);
					GlobalUnlock(hData);
				}
				CloseClipboard();
				idx++;
				break;
			}
		case 'T': // put 'type' to 'Extensions' field
		case 't':
			idx++;
			if (lpCmdLine[idx]==':')
				idx++;
			while(lpCmdLine[idx]==' ') idx++;
			if (lpCmdLine[idx]!='\"')
			{
				temp=FirstCharIndex(lpCmdLine+idx,' ');
				ChangeAndAlloc(pStartData->m_pTypeString,lpCmdLine+idx,temp);
				if (temp<0)
					return TRUE;
				idx+=temp;
			}
			else
			{
				idx++;
				temp=FirstCharIndex(lpCmdLine+idx,'\"');
				ChangeAndAlloc(pStartData->m_pTypeString,lpCmdLine+idx,temp);
				if (temp<0)
					return TRUE;
				idx+=temp+1;
			}
			break;				
		case 'D': // activates database named 'name'
			idx++;
			if (lpCmdLine[idx]==':')
					idx++;
			while(lpCmdLine[idx]==' ') idx++;
			if (lpCmdLine[idx]!='\"')
			{
				temp=FirstCharIndex(lpCmdLine+idx,' ');

				if (CDatabase::FindByName(pStartData->m_aDatabases,lpCmdLine+idx,temp)==NULL)
				{
					CDatabase* pDatabase=CDatabase::FromName(HKCU,
						"Software\\Update\\Databases",lpCmdLine+idx,temp);
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
				temp=FirstCharIndex(lpCmdLine+idx,'\"');
				
				if (CDatabase::FindByName(pStartData->m_aDatabases,lpCmdLine+idx,temp)==NULL)
				{
					CDatabase* pDatabase=CDatabase::FromName(HKCU,
						"Software\\Update\\Databases",lpCmdLine+idx,temp);
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
		case 'd': // activates database file 'dbfile'
			idx++;
			if (lpCmdLine[idx]==':')
				idx++;
			while(lpCmdLine[idx]==' ') idx++;
			if (lpCmdLine[idx]!='\"')
			{
				temp=FirstCharIndex(lpCmdLine+idx,' ');
				if (pStartData->m_aDatabases.GetSize()==1 && strcmp(pStartData->m_aDatabases[0]->GetName(),"DEFAULTX")==0)
				{
					pStartData->m_aDatabases[0]->SetNamePtr(alloccopy("PARAMX"));
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
				temp=FirstCharIndex(lpCmdLine+idx,'\"');
				if (pStartData->m_aDatabases.GetSize()==1 && strcmp(pStartData->m_aDatabases[0]->GetName(),"DEFAULTX")==0)
				{
					pStartData->m_aDatabases[0]->SetNamePtr(alloccopy("PARAMX"));
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
		case 'r': // start locating when Locate32 is opened
			idx++;
			pStartData->m_nStatus|=CStartData::statusRunAtStartUp;
			break;
		case 'i':
		case 'I':
			idx++;
			pStartData->m_nStartup|=CStartData::startupNewInstance;
			break;
		case 'S': // start locate32 to background, (adds icon to tastbar)
			pStartData->m_nStartup|=CStartData::startupLeaveBackground|CStartData::startupDoNotOpenDialog;
			idx++;
			break;
		case 's': // leave locate32 background when dialog is closed
			pStartData->m_nStartup|=CStartData::startupLeaveBackground;
			idx++;
			break;
		case 'u': // start update process at start
			idx++;
			pStartData->m_nStartup|=CStartData::startupUpdate;
			break;
		case 'U': // start update process and exit
			idx++;
			pStartData->m_nStartup|=CStartData::startupUpdate|CStartData::startupDoNotOpenDialog;
			break;
		case 'R':
			switch(lpCmdLine[++idx])
			{
			case 'h':
			case 'H':
				pStartData->m_nPriority=CStartData::priorityHigh;
				break;	
			case 'a':
			case 'A':
			case '+':
				pStartData->m_nPriority=CStartData::priorityAbove;
				break;
			case 'n':
			case 'N':
			case '0':
				pStartData->m_nPriority=CStartData::priorityNormal;
				break;
			case 'b':
			case 'B':
			case '-':
				pStartData->m_nPriority=CStartData::priorityBelow;
				break;
			case 'i':
			case 'I':
				pStartData->m_nPriority=CStartData::priorityIdle;
				break;
			case 'r':
			case 'R':
				pStartData->m_nPriority=CStartData::priorityRealTime;
				break;
			}
			idx++;
			break;
		case 'L':
			idx++;
			if (lpCmdLine[idx]=='1')
			{
				if (pStartData->m_aDatabases.GetSize()==0)
				{
					pStartData->m_aDatabases.Add(CDatabase::FromDefaults(TRUE,GetApp()->GetExeName(),LastCharIndex(GetApp()->GetExeName(),'\\')+1));
					pStartData->m_aDatabases[0]->SetNamePtr(alloccopy("DEFAULTX"));
					pStartData->m_aDatabases[0]->SetThreadId(0);
					pStartData->m_nStartup|=CStartData::startupDatabasesOverridden;
					pStartData->m_aDatabases.GetLast()->AddLocalRoots();
				}
				else if (strncmp(pStartData->m_aDatabases.GetLast()->GetName(),"PARAMX",6)==0 || 
					strncmp(pStartData->m_aDatabases.GetLast()->GetName(),"DEFAULTX",8)==0)
					pStartData->m_aDatabases.GetLast()->AddLocalRoots();
				
				while (lpCmdLine[idx]!=' ') idx++;
			}
			else 
			{
				while (lpCmdLine[idx]==' ') idx++;
				
				CString Directory;
				if (lpCmdLine[idx]!='\"')
				{
					Directory.Copy(lpCmdLine+idx,FirstCharIndex(lpCmdLine+idx,' '));
					idx+=Directory.GetLength();
				}
				else
				{
					idx++;
					int nIndex=FirstCharIndex(lpCmdLine+idx,'\"');
					if (nIndex==-1)
						return TRUE;
					Directory.Copy(lpCmdLine+idx,nIndex);
					idx+=nIndex+1;
				}
				while (Directory.LastChar()=='\\')
					Directory.DelLastChar();
					
				if (Directory.GetLength()>1)
				{
					LPCSTR pDir=NULL;
					if (Directory[1]==':' && Directory.GetLength()==2)
						pDir=alloccopy(Directory);
					else if (CFile::IsDirectory(Directory))
						pDir=alloccopy(Directory);

					if (pDir!=NULL)
					{
						if (pStartData->m_aDatabases.GetSize()==0)
						{
							pStartData->m_aDatabases.Add(CDatabase::FromDefaults(TRUE,GetApp()->GetExeName(),LastCharIndex(GetApp()->GetExeName(),'\\')+1));
							pStartData->m_aDatabases[0]->SetNamePtr(alloccopy("DEFAULTX"));
							pStartData->m_aDatabases[0]->SetThreadId(0);
							pStartData->m_nStartup|=CStartData::startupDatabasesOverridden;
							pStartData->m_aDatabases.GetLast()->AddRoot(pDir);
						}
						else if (strncmp(pStartData->m_aDatabases.GetLast()->GetName(),"PARAMX",6)==0 || 
							strncmp(pStartData->m_aDatabases.GetLast()->GetName(),"DEFAULTX",8)==0)
							pStartData->m_aDatabases.GetLast()->AddRoot(pDir);
						else
							delete[] pDir;
					}
				}
			}
			break;
		case 'l':
			switch(lpCmdLine[++idx])
			{
			case 'n': // set number of maximum found files
				{
					idx++;
					if (lpCmdLine[idx]==':')
						idx++;
					while (lpCmdLine[idx]==' ') idx++;
					temp=FirstCharIndex(lpCmdLine+idx,' ');
					CString str(lpCmdLine+idx,temp);
					
					int val=atoi(str);
					if (val!=0)
						pStartData->m_dwMaxFoundFiles=val;
					else if (str.Compare("0")==0)
						pStartData->m_dwMaxFoundFiles=0;

					if (temp<0)
						return TRUE;
					idx+=temp+1;
						break;
				}
			case 'f': // Set check field to 'File Names Only'
				idx++;
				pStartData->m_nStatus|=CStartData::statusFindFileNames;
				if (lpCmdLine[idx]=='d')
				{
					pStartData->m_nStatus|=CStartData::statusFindFolderNames;
					idx++;
				}
				break;
			case 'd': // Set check field to 'Folder Names Only'
				idx++;
				pStartData->m_nStatus|=CStartData::statusFindFolderNames;
				if (lpCmdLine[idx]=='f')
				{
					pStartData->m_nStatus|=CStartData::statusFindFileNames;
					idx++;
				}
				break;
			case 'c': // put 'text' to 'file containing text' field
				idx++;
				if (lpCmdLine[idx]=='n' && lpCmdLine[idx+1]=='m')
				{
					idx+=2;
					pStartData->m_nStatus|=CStartData::statusFindIsNotMatchCase;
					break;
				}
				
				if (lpCmdLine[idx]==':')
					idx++;
				
				while (lpCmdLine[idx]==' ') idx++;
				if (lpCmdLine[idx]!='\"')
				{
					temp=FirstCharIndex(lpCmdLine+idx,' ');
					ChangeAndAlloc(pStartData->m_pFindText,lpCmdLine+idx,temp);
					if (temp<0)
						return TRUE;
					idx+=temp;
				}
				else
				{
					idx++;
					temp=FirstCharIndex(lpCmdLine+idx,'\"');
					ChangeAndAlloc(pStartData->m_pFindText,lpCmdLine+idx,temp);
					if (temp<0)
						return TRUE;
					idx+=temp+1;
				}
				break;
			case 'w': // check 'Match whole name only' field
				idx++;
				if (lpCmdLine[idx]=='n')
				{
					idx++;
					pStartData->m_nStatus|=CStartData::statusNoMatchWholeName;
				}
				else
					pStartData->m_nStatus|=CStartData::statusMatchWholeName;
				break;
			case 'r': // check 'Replace asterisks' field
				idx++;
				if (lpCmdLine[idx]=='n')
				{
					idx++;
					pStartData->m_nStatus|=CStartData::statusNoReplaceSpacesWithAsterisks;
				}
				else
					pStartData->m_nStatus|=CStartData::statusReplaceSpacesWithAsterisks;
				break;
			case 'W': // check 'Use whole path' field
				idx++;
				if (lpCmdLine[idx]=='n')
				{
					idx++;
					pStartData->m_nStatus|=CStartData::statusNoUseWholePath;
				}
				else
					pStartData->m_nStatus|=CStartData::statusUseWholePath;
				break;
			case 'm': // set minumum file size
				{
					idx++;
					if (lpCmdLine[idx]==':')
						idx++;
					while (lpCmdLine[idx]==' ') idx++;
					temp=FirstCharIndex(lpCmdLine+idx,' ');
					CString str(lpCmdLine+idx,temp);
					while ((str.LastChar()<'0' || str.LastChar()>'9') && !str.IsEmpty())
					{
						pStartData->m_cMinSizeType=str.LastChar();
						str.DelLastChar();
					}
					
					int val=atoi(str);
					if (val!=0)
						pStartData->m_dwMinFileSize=val;
					else if (str.Compare("0")==0)
						pStartData->m_dwMinFileSize=0;


					if (temp<0)
						return TRUE;
					idx+=temp+1;
					break;
				}
			case 'M': // set maximum file size
				{
					idx++;
					if (lpCmdLine[idx]==':')
						idx++;
					while (lpCmdLine[idx]==' ') idx++;
					temp=FirstCharIndex(lpCmdLine+idx,' ');
					CString str(lpCmdLine+idx,temp);
					while ((str.LastChar()<'0' || str.LastChar()>'9') && !str.IsEmpty())
					{
						pStartData->m_cMaxSizeType=str.LastChar();
						str.DelLastChar();
					}
					
					int val=atoi(str);
					if (val!=0)
						pStartData->m_dwMaxFileSize=val;
					else if (str.Compare("0")==0)
						pStartData->m_dwMaxFileSize=0;

					if (temp<0)
						return TRUE;
					idx+=temp+1;
					break;
				}
			case 'D': // dates
				{
					idx++;
					while (lpCmdLine[idx]==' ')
						idx++;
                    int nLength=LastCharIndex(lpCmdLine+idx,' ');
					if (nLength<0)
					{
						nLength=strlen(lpCmdLine+idx);
						if (nLength<7)
                            return TRUE;
					}
					if (nLength<7)
					{
						idx+=nLength;
						break;
					}

					char szBuf[]="XX";
					szBuf[0]=lpCmdLine[idx+1];
					szBuf[1]=lpCmdLine[idx+2];
					WORD bYear=atoi(szBuf);
					if (bYear<60)
						bYear+=2000;
					else
						bYear+=1900;
					szBuf[0]=lpCmdLine[idx+3];
					szBuf[1]=lpCmdLine[idx+4];
					BYTE bMonth=atoi(szBuf);
					if (bMonth<1 || bMonth>12)
						bMonth=1;
					szBuf[0]=lpCmdLine[idx+5];
					szBuf[1]=lpCmdLine[idx+6];
					BYTE bDay=atoi(szBuf);
					if (bDay<1 || bDay>CTime::GetDaysInMonth(bMonth,bYear))
						bDay=1;					
					
					if (isupper(lpCmdLine[idx])) // max date
					{
						pStartData->m_cMaxDateType=lpCmdLine[idx];
						pStartData->m_dwMaxDate=(bYear<<16)|(bMonth<<8)|(bDay);
					}
					else
					{
						pStartData->m_cMinDateType=lpCmdLine[idx];
						pStartData->m_dwMinDate=(bYear<<16)|(bMonth<<8)|(bDay);
					}
					idx+=nLength;
					break;
				}
			case 's':
			case 'S':
				idx++;
				if (lpCmdLine[idx]>='0' && lpCmdLine[idx]<=9)
					pStartData->m_nSorting=lpCmdLine[idx]-'0';
				else
				{
					switch (lpCmdLine[idx])
					{
					case 'n':
					case 'N':
						pStartData->m_nSorting=0;
						break;
					case 'f':
					case 'F':
						pStartData->m_nSorting=1;
						break;
					case 's':
					case 'S':
						pStartData->m_nSorting=2;
						break;
					case 't':
					case 'T':
						pStartData->m_nSorting=3;
						break;
					case 'd':
					case 'D':
						pStartData->m_nSorting=4;
						break;
					}
				}
				idx++;
				if (lpCmdLine[idx]=='d' || lpCmdLine[idx]=='D')
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
	if (lpCmdLine[idx]!='\0')
		ChangeAndAlloc(pStartData->m_pStartString,lpCmdLine+idx);
	return TRUE;
}

BYTE CLocateApp::CheckDatabases()
{
	// First, check that there is database 
	if (m_aDatabases.GetSize()==0)
		CDatabase::LoadFromRegistry(HKCU,"Software\\Update\\Databases",m_aDatabases);

	// If there is still no any available database, try to load old style db
	if (m_aDatabases.GetSize()==0)
	{
		CDatabase* pDatabase=CDatabase::FromOldStyleDatabase(HKCU,"Software\\Update\\Database");
		if (pDatabase==NULL)
		{
			pDatabase=CDatabase::FromDefaults(TRUE,GetApp()->GetExeName(),
				LastCharIndex(GetApp()->GetExeName(),'\\')+1); // Nothing else can be done?
		}
		else
		{
			if (CDatabase::SaveToRegistry(HKCU,"Software\\Update\\Databases",&pDatabase,1))
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
	CString Path(GetExeName());
	Path << ",1";
	
	if (Key.OpenKey(HKCR,".ltmp",CRegKey::createNew|CRegKey::samCreateSubkey|CRegKey::samWrite)!=NOERROR)
		return FALSE;
	Key.SetValue(szEmpty,"LTMPFile",8,REG_SZ);
	if (Key.OpenKey(HKCR,"LTMPFile",CRegKey::createNew|CRegKey::samCreateSubkey|CRegKey::samWrite)!=NOERROR)
		return FALSE;
	Key.SetValue(szEmpty,"Deleted / Moved File (REMOVE THIS TYPE)",39,REG_SZ);
	if (Key.OpenKey(HKCR,"LTMPFile\\DefaultIcon",CRegKey::createNew|CRegKey::samCreateSubkey|CRegKey::samWrite)!=NOERROR)
		return FALSE;
	Key.SetValue(szEmpty,Path);
	Key.CloseKey();
	GetTempPath(_MAX_PATH,Path.GetBuffer(_MAX_PATH));
	Path.FreeExtra();
	if (Path.LastChar()!='\\')
		Path << '\\';
	Path << "temp.ltmp";
	HANDLE hFile=CreateFile(Path,GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
		OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE,NULL);
	if (hFile==NULL)
		return FALSE;
	SHFILEINFO fi;
	fi.iIcon=1;
	SHGetFileInfo(Path,0,&fi,sizeof(SHFILEINFO),SHGFI_ICON|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
	m_nDelImage=fi.iIcon;
	CloseHandle(hFile);
	RegDeleteKey(HKCR,".ltmp");
	RegDeleteKey(HKCR,"LTMPFile\\DefaultIcon");
	RegDeleteKey(HKCR,"LTMPFile");

	Path << '2';
	hFile=CreateFile(Path,GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
		OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE,NULL);
	if (hFile==NULL)
		return FALSE;
	fi.iIcon=1;
	SHGetFileInfo(Path,0,&fi,sizeof(SHFILEINFO),SHGFI_ICON|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
	m_nDefImage=fi.iIcon;
	CloseHandle(hFile);
	
	GetSystemDirectory(Path.GetBuffer(_MAX_PATH+3),_MAX_PATH);
	fi.iIcon=1;
	SHGetFileInfo(Path,0,&fi,sizeof(SHFILEINFO),SHGFI_ICON|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
	m_nDirImage=fi.iIcon;
	SHGetFileInfo(Path.Left(3),0,&fi,sizeof(SHFILEINFO),SHGFI_ICON|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
	m_nDriveImage=fi.iIcon;
	return TRUE;
}


BOOL CLocateApp::ChechOtherInstances()
{
	HWND hWnd=FindWindow("LOCATEAPPST","Locate ST");
	if (hWnd!=NULL)
	{
		ATOM aCommandLine=GlobalAddAtom(GetApp()->GetCmdLine());
		::SendMessage(hWnd,WM_ANOTHERINSTANCE,0,(LPARAM)aCommandLine);
		if (aCommandLine!=NULL)
			DeleteAtom(aCommandLine);
		return TRUE;
	}	
	return FALSE;
}


	
LPSTR CLocateApp::FormatDateAndTimeString(WORD wDate,WORD wTime)
{
	DWORD dwLength=2;
	
	enum {
		fDateIsDefault = 0x1,
		fTimeIsDefault = 0x2
	};
	BYTE fFlags=0;

	// wDate/wTime is 0xFFFFFFFF, omit date/time
	if (wDate!=WORD(-1))
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

			GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,
				NULL,m_strDateFormat.GetBuffer(1000),1000);
			m_strDateFormat.FreeExtra();

			dwLength+=m_strDateFormat.GetLength();
		}
		else
            dwLength+=m_strDateFormat.GetLength()*2;
	}
	if (wTime!=WORD(-1))
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
			GetTimeFormat(LOCALE_USER_DEFAULT,TIME_NOSECONDS,&st,
				NULL,m_strTimeFormat.GetBuffer(1000),1000);
			m_strTimeFormat.FreeExtra();

			dwLength+=m_strTimeFormat.GetLength();
		}
		else
			dwLength+=m_strTimeFormat.GetLength()*2;
	
		
	}

	LPSTR szRet=new char[dwLength];
	LPSTR pPtr=szRet;

	//Formatting date
    if (wDate!=WORD(-1))
	{
		for (DWORD i=0;i<m_strDateFormat.GetLength();i++)
		{
			switch (m_strDateFormat[i])
			{
			case 'd':
				if (m_strDateFormat[i+1]=='d') // "dd" , "ddd" and "dddd" will not be handled
				{
					pPtr[0]=DOSDATETODAY(wDate)/10+'0';
					pPtr[1]=DOSDATETODAY(wDate)%10+'0';
					pPtr+=2;
					i++;
				}
				else // "d"
				{
					if (DOSDATETODAY(wDate)>9)
					{
						pPtr[0]=DOSDATETODAY(wDate)/10+'0';
						pPtr[1]=DOSDATETODAY(wDate)%10+'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSDATETODAY(wDate)+'0';
						pPtr++;
					}
				}
				break;
			case 'M':
				if (m_strDateFormat[i+1]=='M') // "MM", "MMM" & "MMMM" will not be handled
				{
					pPtr[0]=DOSDATETOMONTH(wDate)/10+'0';
					pPtr[1]=DOSDATETOMONTH(wDate)%10+'0';
					pPtr+=2;
					i++;
				}
				else // "M"
				{
					if (DOSDATETOMONTH(wDate)>9)
					{
						pPtr[0]=DOSDATETOMONTH(wDate)/10+'0';
						pPtr[1]=DOSDATETOMONTH(wDate)%10+'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSDATETOMONTH(wDate)+'0';
						pPtr++;
					}
				}
				break;
			case 'y':
				if (m_strDateFormat[i+1]=='y')
				{
					if (m_strDateFormat[i+2]=='y') // "yyy" & "yyyy"
					{
						pPtr[0]=DOSDATETOYEAR(wDate)/1000+'0';
						pPtr[1]=(DOSDATETOYEAR(wDate)/100)%10+'0';
						pPtr[2]=(DOSDATETOYEAR(wDate)/10)%10+'0';
						pPtr[3]=DOSDATETOYEAR(wDate)%10+'0';
						if (m_strDateFormat[i+3]=='y')
							i+=3;
						else
							i+=2;
						pPtr+=4;
					}
					else // "yy"
					{
						pPtr[0]=(DOSDATETOYEAR(wDate)/10)%10+'0';
						pPtr[1]=DOSDATETOYEAR(wDate)%10+'0';
						pPtr+=2;
						i++;
					}			
				}
				else // "y"
				{
					if (DOSDATETOYEAR(wDate)/1000>9)
					{
						pPtr[0]=(DOSDATETOYEAR(wDate)/10)%10+'0';
						pPtr[1]=DOSDATETOYEAR(wDate)%10+'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSDATETOYEAR(wDate)%10+'0';
						pPtr++;
					}
				}
				break;
			case '\'':
				continue;
			default:
				*pPtr=m_strDateFormat[i];
				pPtr++;
				break;
			}
		}
	}
	
	// Formatting time
	if (wTime!=WORD(-1))
	{
		*pPtr=' ';
		pPtr++;
		
		for (DWORD i=0;i<m_strTimeFormat.GetLength();i++)
		{
			switch (m_strTimeFormat[i])
			{
			case 'h':
				if (m_strTimeFormat[i+1]=='h')
				{
					pPtr[0]=DOSTIMETO12HOUR(wTime)/10+'0';
					pPtr[1]=DOSTIMETO12HOUR(wTime)%10+'0';
					pPtr+=2;
					i++;
				}
				else
				{
					if (DOSTIMETO12HOUR(wTime)>9)
					{
						pPtr[0]=DOSTIMETO12HOUR(wTime)/10+'0';
						pPtr[1]=DOSTIMETO12HOUR(wTime)%10+'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSTIMETO12HOUR(wTime)%10+'0';
						pPtr++;
					}
				}
				break;
			case 'H':
				if (m_strTimeFormat[i+1]=='H')
				{
					pPtr[0]=DOSTIMETO24HOUR(wTime)/10+'0';
					pPtr[1]=DOSTIMETO24HOUR(wTime)%10+'0';
					pPtr+=2;
					i++;
				}
				else
				{
					if (DOSTIMETO24HOUR(wTime)>9)
					{
						pPtr[0]=DOSTIMETO24HOUR(wTime)/10+'0';
						pPtr[1]=DOSTIMETO24HOUR(wTime)%10+'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSTIMETO24HOUR(wTime)%10+'0';
						pPtr++;
					}
				}
				break;
			case 'm':
				if (m_strTimeFormat[i+1]=='m')
				{
					pPtr[0]=DOSTIMETOMINUTE(wTime)/10+'0';
					pPtr[1]=DOSTIMETOMINUTE(wTime)%10+'0';
					pPtr+=2;
					i++;
				}
				else
				{
					if (DOSTIMETOMINUTE(wTime)>9)
					{
						pPtr[0]=DOSTIMETOMINUTE(wTime)/10+'0';
						pPtr[1]=DOSTIMETOMINUTE(wTime)%10+'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSTIMETOMINUTE(wTime)%10+'0';
						pPtr++;
					}
				}
				break;
			case 's':
				if (m_strTimeFormat[i+1]=='s')
				{
					pPtr[0]=DOSTIMETOSECOND(wTime)/10+'0';
					pPtr[1]=DOSTIMETOSECOND(wTime)%10+'0';
					pPtr+=2;
					i++;
				}
				else
				{
					if (DOSTIMETOSECOND(wTime)>9)
					{
						pPtr[0]=DOSTIMETOSECOND(wTime)/10+'0';
						pPtr[1]=DOSTIMETOSECOND(wTime)%10+'0';
						pPtr+=2;
					}
					else
					{
						*pPtr=DOSTIMETOSECOND(wTime)%10+'0';
						pPtr++;
					}
				}
				break;
			case 't':
				{
					char szAMPM[10];
					LoadString(DOSTIMETO24HOUR(wTime)>11?IDS_PM:IDS_AM,szAMPM,10);
					
					if (m_strTimeFormat[i+1]=='t')
					{
						for (char* ptr=szAMPM;*ptr!='\0';ptr++,pPtr++)
							*pPtr=*ptr;
						i++;
					}
					else
						*pPtr=szAMPM[0];
				}
				break;
			case '\'':
				continue;
			default:
				*pPtr=m_strTimeFormat[i];
				pPtr++;
				break;
			}
		}
	}

	*pPtr='\0';

	if (fFlags&fDateIsDefault)
		m_strDateFormat.Empty();
	if (fFlags&fTimeIsDefault)
		m_strTimeFormat.Empty();
	return szRet;
}

LPSTR CLocateApp::FormatFileSizeString(DWORD dwFileSizeLo,DWORD bFileSizeHi) const
{
 	char* szRet=new char[40];
	char szUnit[10];
	BOOL bDigits=0;
		

	switch (m_nFileSizeFormat)
	{
	case fsfOverKBasKB:
		if (bFileSizeHi>0)
		{
			LoadString(IDS_KB,szUnit,10);
			
			_int64 uiFileSize=((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo;
			_ui64toa(uiFileSize/1024,szRet,10);
		}
		else if (dwFileSizeLo<1024)
		{
			LoadString(IDS_BYTES,szUnit,10);
			
			ultoa(dwFileSizeLo,szRet,10);
		}
		else
		{
			LoadString(IDS_KB,szUnit,10);
			
			ultoa((dwFileSizeLo/1024)+(dwFileSizeLo%1024==0?0:1),szRet,10);
		}
		break;
	case fsfBestUnit:
		if (bFileSizeHi>0)
		{
			DWORD num=DWORD(((((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo))/(1024*1024));

			if (num>=10*1024)
				ultoa(num/1024,szRet,10);
			else
			{
				bDigits=1;
				sprintf(szRet,"%1.1f",double(num)/1024);
			}

			LoadString(IDS_GB,szUnit,10);
		}
		else if (dwFileSizeLo>1024*1024*1024)
		{
			DWORD num=dwFileSizeLo/(1024*1024);

			if (num>=10*1024)
				ultoa(num/1024,szRet,10);
			else
			{
				bDigits=1;
				sprintf(szRet,"%1.1f",double(num)/1024);
			}

			LoadString(IDS_GB,szUnit,10);
		}
		else if (dwFileSizeLo>1048576) // As MB
		{
			DWORD num=dwFileSizeLo/1024;
			
			if (num>=10*1024)
				ultoa(num/1024,szRet,10);
			else
			{
				bDigits=1;
				sprintf(szRet,"%1.1f",double(num)/1024);
			}

			LoadString(IDS_MB,szUnit,10);
		}
		else if (dwFileSizeLo>1024) // As KB
		{
			if (dwFileSizeLo>=10*1024)
				ultoa(dwFileSizeLo/1024,szRet,10);
			else
			{
				bDigits=1;
				sprintf(szRet,"%1.1f",double(dwFileSizeLo)/1024);
			}
			
			LoadString(IDS_KB,szUnit,10);
		}
		else // As B
		{
			ultoa(dwFileSizeLo,szRet,10);
		
			LoadString(IDS_BYTES,szUnit,10);
		}		
		break;
	case fsfBytes:
		LoadString(IDS_BYTES,szUnit,10);
	case fsfBytesNoUnit:
		if (bFileSizeHi>0)
		{
			_int64 uiFileSize=((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo;
			_ui64toa(uiFileSize,szRet,10);
		}
		else
			ultoa(dwFileSizeLo,szRet,10);
		
		break;
	case fsfKiloBytes:
		LoadString(IDS_KB,szUnit,10);
	case fsfKiloBytesNoUnit:
		if (bFileSizeHi>0)
		{
			_int64 uiFileSize=((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo;
			_ui64toa(uiFileSize/1024,szRet,10);
		}
		else if (dwFileSizeLo>10*1024)
			ultoa(dwFileSizeLo/1024,szRet,10);
		else if (dwFileSizeLo<1024)
		{
			bDigits=3;
			sprintf(szRet,"%1.3f",((double)dwFileSizeLo)/1024);
		}
		else
		{
			bDigits=1;
			sprintf(szRet,"%1.1f",((double)dwFileSizeLo)/1024);
		}
		break;
	case fsfMegaBytesMegaBytes:
		LoadString(IDS_MB,szUnit,10);
	case fsfMegaBytesMegaBytesNoUnit:
		if (bFileSizeHi>0)
		{
			_int64 uiFileSize=((_int64)bFileSizeHi)<<32|(_int64)dwFileSizeLo;
			_ui64toa(uiFileSize/1048576,szRet,10);
		}
		else if (dwFileSizeLo>10*1048576)
			ultoa(dwFileSizeLo/1048576,szRet,10);
		else if (dwFileSizeLo<1048576)
		{
			bDigits=3;
			sprintf(szRet,"%1.3f",((double)dwFileSizeLo)/1048576);
		}
		else
		{
			bDigits=1;
			sprintf(szRet,"%1.1f",((double)dwFileSizeLo)/1048576);
		}		
		break;
	}

	if (m_dwProgramFlags&pfFormatUseLocaleFormat)
	{
		CRegKey RegKey;
		if (RegKey.OpenKey(HKCU,"Control Panel\\International",CRegKey::defRead)==ERROR_SUCCESS)
		{
			char* szRet2=new char[50];
			char szTemp[10]=".",szTemp2[10]=" ";

			NUMBERFMT fmt;
			
			// Defaults;
			fmt.NumDigits=bDigits; 
			fmt.LeadingZero=1;
			fmt.Grouping=3; 
			fmt.lpDecimalSep=szTemp; 
			fmt.lpThousandSep=szTemp2; 
			fmt.NegativeOrder=1; 
			
			if (RegKey.QueryValue("iLZero",szTemp,10)>1)
				fmt.LeadingZero=atoi(szTemp);
			if (RegKey.QueryValue("sGrouping",szTemp,10)>1)
				fmt.Grouping=atoi(szTemp);
			RegKey.QueryValue("sDecimal",szTemp,10);
			RegKey.QueryValue("sThousand",szTemp2,10);

			
			if (GetNumberFormat(LOCALE_USER_DEFAULT,0,szRet,&fmt,szRet2,50)>0)
			{
				delete[] szRet;
				szRet=szRet2;
			}
			else
				delete[] szRet2;
		}
	}

	if (!(m_nFileSizeFormat==fsfBytesNoUnit || 
		m_nFileSizeFormat==fsfKiloBytesNoUnit|| 
		m_nFileSizeFormat==fsfMegaBytesMegaBytesNoUnit))
		strcat(szRet,szUnit);

	return szRet;
}


BOOL CLocateApp::StopUpdating(BOOL bForce)
{
    if (!IsUpdating())
		return TRUE; // Already stopped

	BOOL bRet=TRUE;
	GetUpdatersPointer();
	for (int i=0;m_ppUpdaters!=NULL && m_ppUpdaters[i]!=NULL;i++)
	{
		if (!IS_UPDATER_EXITED(m_ppUpdaters[i]))
		{
			ReleaseUpdatersPointer();
			if (!m_ppUpdaters[i]->StopUpdating(bForce))
				bRet=FALSE;
			GetUpdatersPointer();
		}
	}
	ReleaseUpdatersPointer();
	
	GetLocateAppWnd()->StopUpdateStatusNotification();
	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg!=NULL)
		pLocateDlg->StopUpdateAnimation();
	
	// It is good to check this handle again
	pLocateDlg=GetLocateDlg();
	if (pLocateDlg!=NULL)
	{
		pLocateDlg->m_NameDlg.InitDriveBox();
		//pLocateDlg->ResetFileNotificators();
		pLocateDlg->PostMessage(WM_REFRESHNOTIFIERHANDLERS);
	}
	GetLocateAppWnd()->SetUpdateStatusInformation((HICON)LoadImage(IDI_APPLICATIONICON,IMAGE_ICON,16,16,0),IDS_NOTIFYLOCATE);
	
	// I think pointers are removed elsewhere
	ASSERT(m_ppUpdaters==NULL);



	return bRet;
}

void CLocateApp::SetDatabases(const CArray<CDatabase*>& rDatabases)
{
	m_aDatabases.RemoveAll();

	for (int i=0;i<rDatabases.GetSize();i++)
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

BOOL CALLBACK CLocateApp::UpdateProc(DWORD dwParam,CallingReason crReason,UpdateError ueCode,CDatabaseUpdater* pUpdater)
{
	DbcDebugFormatMessage2("CLocateApp::UpdateProc BEGIN, reason=%d, code=%d",crReason,ueCode);
	
	switch (crReason)
	{
	case Initializing:
		return TRUE;
	case RootChanged:
	{
		CLocateDlg* pLocateDlg=GetLocateDlg();
		if (pLocateDlg!=NULL)
		{
			CString str;

			if (pUpdater->GetCurrentRoot()!=NULL)
			{
				str.Format(IDS_UPDATINGDATABASE2,
					pUpdater->GetCurrentDatabaseName(),
					pUpdater->GetCurrentRoot()->m_Path);
			}
			else
			{
				str.Format(IDS_UPDATINGDATABASE2,
					pUpdater->GetCurrentDatabaseName(),
					(LPCSTR)CString(IDS_UPDATINGWRITINGDATABASE));
			}
			
			pLocateDlg->m_pStatusCtrl->SetText(str,1,0);
		}
	
		
		if (dwParam!=NULL)
			((CLocateAppWnd*)dwParam)->SetUpdateStatusInformation(NULL,IDS_NOTIFYUPDATING);
		return TRUE;
	}
	case FinishedUpdating:
	{
		CLocateDlg* pLocateDlg=GetLocateDlg();
		if (pLocateDlg!=NULL)
		{
			if (ueCode!=ueStopped) // This is done at the end of CLocateApp::StopUpdating
			{
				pLocateDlg->m_NameDlg.InitDriveBox();
				
				//pLocateDlg->ResetFileNotificators();
				pLocateDlg->PostMessage(WM_REFRESHNOTIFIERHANDLERS);
			}
		
			if (ueCode==ueStopped)
			{
				CString str;
				str.Format(IDS_UPDATINGDATABASE2,
					pUpdater->GetCurrentDatabaseName(),
					(LPCSTR)CString(IDS_UPDATINGCANCELLED2));

				pLocateDlg->m_pStatusCtrl->SetText(str,1,0);
				return FALSE;
			}
			else if (ueCode!=ueSuccess)
			{
				CString str;
				str.Format(IDS_UPDATINGDATABASE2,
					pUpdater->GetCurrentDatabaseName(),
					(LPCSTR)CString(IDS_UPDATINGFAILED));

				pLocateDlg->m_pStatusCtrl->SetText(str,1,0);
				return FALSE;
			}
			CString str;
			str.Format(IDS_UPDATINGDATABASE2,
				pUpdater->GetCurrentDatabaseName(),
				(LPCSTR)CString(IDS_UPDATINGDONE));

			pLocateDlg->m_pStatusCtrl->SetText(str,1,0);
		}

		return TRUE;
	}
	case ClassShouldDelete:
		{
			if (!GetLocateApp()->IsUpdating())
				return TRUE; // One thread mode

						
			CLocateAppWnd* pAppWnd=(CLocateAppWnd*)dwParam;
			DWORD dwRunning=0;
			
			CDatabaseUpdater*** pppUpdaters=GetLocateApp()->GetUpdatersPointerPtr();
			
			if (pppUpdaters==NULL)
				return FALSE;

			
			for (int i=0;(*pppUpdaters)[i]!=NULL;i++)
			{
				if ((*pppUpdaters)[i]==pUpdater)
					(*pppUpdaters)[i]=UPDATER_EXITED(ueCode);
				else if (!IS_UPDATER_EXITED((*pppUpdaters)[i]))
					dwRunning++;
			}
			delete pUpdater;
			
			if (dwRunning==0)
			{
				// No updaters running anymore...
				
				// ...so stopping animations
				if (pAppWnd!=NULL)
				{
					if (pAppWnd->m_pUpdateStatusWnd!=NULL)
						pAppWnd->m_pUpdateStatusWnd->Update();
					pAppWnd->StopUpdateStatusNotification();
				}

				CLocateDlg* pLocateDlg=GetLocateDlg();
				if (pLocateDlg!=NULL)
				{
					pLocateDlg->StopUpdateAnimation();
					
					CString str;
					
					// ... and constucting notification message:
					// checking wheter all are stopped, or cancelled 
					for (i=0;(*pppUpdaters)[i]!=NULL;i++)
					{
						if (GET_UPDATER_CODE((*pppUpdaters)[i])!=ueStopped)
							break;
					}
					if ((*pppUpdaters)[i]==NULL)
					{
						// All updaters are interrupted by user
						str.LoadString(IDS_UPDATINGCANCELLED);
						pLocateDlg->m_pStatusCtrl->SetText(str,1,0);
						pLocateDlg->m_pStatusCtrl->SetText(LPCSTR(::LoadIcon(NULL,IDI_EXCLAMATION)),3,SBT_OWNERDRAW);

					}
					else
					{
						// All succeeded or some updaters failed/interrupted
                        CString str2;
						str.LoadString(IDS_UPDATINGENDED);
						int added=0;
							
						for (i=0;(*pppUpdaters)[i]!=NULL;i++)
						{
							switch (GET_UPDATER_CODE((*pppUpdaters)[i]))
							{
							case ueSuccess:
								break;
							case ueStopped:
								if (added>0)
									str2 << ", ";
								str2 << CString(IDS_UPDATINGTHREAD);
								str2 << ' ' << (int)(i+1) << ": ";
								str2 << CString(IDS_UPDATINGCANCELLED2);
								added++;
								break;
							case ueFolderUnavailable:
								if (added>0)
									str2 << ", ";
								str2 << CString(IDS_UPDATINGTHREAD);
								str2 << ' ' << (int)(i+1) << ": ";
								str2 << CString(IDS_UPDATINGUNAVAILABLEROOT);
								added++;
								break;
							default:
								if (added>0)
									str2 << ", ";
								str2 << CString(IDS_UPDATINGTHREAD);
								str2 << ' ' << (int)(i+1) << ": ";
								str2 << CString(IDS_UPDATINGFAILED);
								added++;
								break;
							}
						}

						if (str2.IsEmpty())
							str.LoadString(IDS_UPDATINGSUCCESS);
						else
						{
							pLocateDlg->m_pStatusCtrl->SetText(LPCSTR(::LoadIcon(NULL,IDI_EXCLAMATION)),3,SBT_OWNERDRAW);
							str << ' ' << str2;
						}
						pLocateDlg->m_pStatusCtrl->SetText(str,1,0);
					}
				}
		
				// Freeing memory
				delete[] *pppUpdaters;
				*pppUpdaters=NULL;

				GetLocateApp()->ReleaseUpdatersPointer();

				if (GetLocateApp()->m_nStartup&CStartData::startupExitAfterUpdating && pAppWnd!=NULL)
					pAppWnd->PostMessage(WM_COMMAND,IDM_EXIT,NULL);

			}
			else 
			{
				// Updaters still running, updating shell notify icons
				GetLocateApp()->ReleaseUpdatersPointer();
				
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
				char* pError;

				if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,
					GetLastError(),LANG_USER_DEFAULT,(LPSTR)&pError,0,NULL))
				{
					CString str,state;
					if (pUpdater->GetCurrentRoot()==NULL)
						state.Format(IDS_ERRORUNKNOWNWRITEDB,pUpdater->GetCurrentDatabaseFile());
					else
						state.Format(IDS_ERRORUNKNOWNSCANROOT,pUpdater->GetCurrentRootPath());
					
					
					str.Format(IDS_ERRORUNKNOWNOS,pError);
					while (str.LastChar()=='\n' || str.LastChar()=='\r')
						str.DelLastChar();
					str << state;
					
					::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,
						str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
					LocalFree(pError);

					
				}
				else
					::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,
						CString(IDS_ERRORUNKNOWN),CString(IDS_ERROR),MB_OK|MB_ICONERROR);
				return FALSE;
			}
			break;
		case ueCreate:
		case ueOpen:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORCANNOTOPENDB,pUpdater->GetCurrentDatabaseFile());
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
				return FALSE;
			}
			break;
		case ueRead:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORCANNOTREADDB,pUpdater->GetCurrentDatabaseFile());
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
				return FALSE;
			}
			break;
		case ueWrite:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORCANNOTWRITEDB,pUpdater->GetCurrentDatabaseFile());
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
                return FALSE;
			}
			break;
		case ueAlloc:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
			{
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,
					CString(IDS_ERRORCANNOTALLOCATE),CString(IDS_ERROR),MB_OK|MB_ICONERROR);
			}
			break;
		case ueInvalidDatabase:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORINVALIDDB,pUpdater->GetCurrentDatabaseName());
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
				return FALSE;
			}
			break;
		case ueFolderUnavailable:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowNonCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORROOTNOTAVAILABLE,pUpdater->GetCurrentRootPath()!=NULL?pUpdater->GetCurrentRootPath():"");
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
				return FALSE;
			}
			break;
		case ueCannotIncrement:
			if (CLocateApp::GetProgramFlags()&CLocateApp::pfShowNonCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORCANNOTWRITEINCREMENTALLY,pUpdater->GetCurrentDatabaseName());
				
				return ::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_ICONERROR|MB_YESNO)==IDYES;
			}
			return TRUE;
		}
		break;
	}
	
	DebugMessage("CLocateApp::UpdateProc END");
	return TRUE;
}

BOOL CLocateApp::SetLanguageSpecifigHandles()
{
	CRegKey RegKey;
	CString LangFile;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource),
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("Language",LangFile);
		RegKey.CloseKey();
	}
	if (LangFile.IsEmpty())
		LangFile="lan_en.dll";

	CString Path(GetApp()->GetExeName(),LastCharIndex(GetApp()->GetExeName(),'\\')+1);
	
	

	HINSTANCE hLib=LoadLibrary(Path+LangFile);
	if (hLib==NULL)
	{
		hLib=LoadLibrary(Path+"lan_en.dll");

		if (hLib==NULL)
		{
			MessageBox(NULL,CString(IDS_ERRORCANNOTLOADLANGUAGEFILE,CommonResource),
				CString(IDS_ERROR),MB_ICONERROR|MB_OK);
			return FALSE;
		}

		MessageBox(NULL,CString(IDS_ERRORCANNOTLOADLANGUAGEFILE2,CommonResource),
			CString(IDS_ERROR),MB_ICONERROR|MB_OK);
	}

	SetResourceHandle(hLib,LanguageSpecificResource);
	return TRUE;
}


BOOL CLocateApp::GlobalUpdate(CArray<PDATABASE>* paDatabasesArg)
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

	GetUpdatersPointer();
	
	m_ppUpdaters=new CDatabaseUpdater*[wThreads+1];
	
	for (WORD wThread=0;wThread<wThreads;wThread++)
	{
		m_ppUpdaters[wThread]=new CDatabaseUpdater(*paDatabases,paDatabases->GetSize(),
			UpdateProc,wThread,(DWORD)m_pMainWnd);
	}
	m_ppUpdaters[wThreads]=NULL;
    
	// Starting
	for (wThread=0;wThread<wThreads;wThread++)
		m_ppUpdaters[wThread]->Update(TRUE);
	
	
	ReleaseUpdatersPointer();
	
	if (paDatabasesArg==NULL)
	{
		for (int i=0;i<paDatabases->GetSize();i++)
			delete paDatabases->GetAt(i);
		delete paDatabases;
	}
	return TRUE;
}

void CLocateApp::OnInitDatabaseMenu(HMENU hPopupMenu)
{
	// Removing existing items
	for(int i=GetMenuItemCount(hPopupMenu)-1;i>=0;i--)
		DeleteMenu(hPopupMenu,i,MF_BYPOSITION);

	CString title;
	MENUITEMINFO mi;
	mi.cbSize=sizeof(MENUITEMINFO);
	mi.fMask=MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_TYPE;
	mi.wID=IDM_DEFUPDATEDBITEM;
	mi.fType=MFT_STRING;
	mi.fState=MFS_ENABLED;
	
	if (m_aDatabases.GetSize()==0)
	{
		// Inserting default items
		title.LoadString(IDS_EMPTY);
		mi.dwTypeData=(LPSTR)(LPCSTR)title;
		mi.dwItemData=0;
		mi.fState=MFS_GRAYED;
		InsertMenuItem(hPopupMenu,mi.wID,FALSE,&mi);
		return;
	}

	// Starting to insert database items 
	for (int i=0;i<m_aDatabases.GetSize();i++)
	{
		title.Format("&%d: %s",i+1,m_aDatabases[i]->GetName());
		mi.dwTypeData=(LPSTR)(LPCSTR)title;
		mi.dwItemData=m_aDatabases[i]->GetID();
		mi.wID=IDM_DEFUPDATEDBITEM+i;
		InsertMenuItem(hPopupMenu,mi.wID,FALSE,&mi);
	}	
}

void CLocateApp::OnDatabaseMenuItem(WORD wID)
{
	int iDB=wID-IDM_DEFUPDATEDBITEM;

	ASSERT(iDB>=0 && iDB<m_aDatabases.GetSize());

	DWORD dwLength=istrlen(m_aDatabases[iDB]->GetName());
	LPSTR pDatabaseName=new char[dwLength+2];
	sMemCopy(pDatabaseName,m_aDatabases[iDB]->GetName(),dwLength);
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

int CLocateAppWnd::OnCreate(LPCREATESTRUCT lpcs)
{
	// Loading menu
	m_Menu.LoadMenu(IDR_SYSTEMTRAYMENU);


	// Set schedules
	SetSchedules();
	SetMenuDefaultItem(m_Menu.GetSubMenu(0),IDM_OPENLOCATE,FALSE);
	
	// Setting icons
	HICON hIcon=(HICON)LoadImage(IDI_APPLICATIONICON,IMAGE_ICON,32,32,LR_SHARED);
	SetIcon(hIcon,TRUE);
	SetClassLong(gclHIcon,(LONG)hIcon);

	nHFCInstallationMessage=RegisterWindowMessage("HFCINSTALLMESSAGE");
	nTaskbarCreated=RegisterWindowMessage("TaskbarCreated");

	return CFrameWnd::OnCreate(lpcs);
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
		bCanHook?CShortcut::loadGlobalHotkey:0)|CShortcut::loadGlobalHook))
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
	CRegKey RegKey;
	if(RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",
		CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		RegKey.SetValue("General Flags",m_dwProgramFlags&pfSave);

		RegKey.SetValue("DateFormat",m_strDateFormat);
		RegKey.SetValue("TimeFormat",m_strTimeFormat);
		RegKey.SetValue("SizeFormat",(DWORD)m_nFileSizeFormat);
	}


}

void CLocateApp::LoadRegistry()
{
	// When modifications are done, check whether 
	// function is applicable for UpdateSettings

	CRegKey RegKey;
	m_strDateFormat.Empty();
	m_strTimeFormat.Empty();

	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		// Loading dwFlags
		DWORD temp=m_dwProgramFlags;
		RegKey.QueryValue("General Flags",temp);
		m_dwProgramFlags&=~pfSave;
		m_dwProgramFlags|=temp&pfSave;



		RegKey.QueryValue("DateFormat",m_strDateFormat);
		RegKey.QueryValue("TimeFormat",m_strTimeFormat);
		RegKey.QueryValue("SizeFormat",*((DWORD*)&m_nFileSizeFormat));

	}
}

BOOL CLocateApp::UpdateSettings()
{
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		// Loading dwFlags
		DWORD temp=m_dwProgramFlags;
		RegKey.QueryValue("General Flags",temp);
		m_dwProgramFlags&=~pfSave;
		m_dwProgramFlags|=temp&pfSave;
	}

	return TRUE;
}

void CLocateAppWnd::GetRootInfos(WORD& wThreads,WORD& wRunning,RootInfo*& pRootInfos)
{
	// Counting threads		
	CDatabaseUpdater** ppUpdaters=GetLocateApp()->GetUpdatersPointer();
	wThreads=0;
	if (ppUpdaters!=NULL)
	{
		for (;ppUpdaters[wThreads]!=NULL;wThreads++);
	}
	GetLocateApp()->ReleaseUpdatersPointer();
	
	pRootInfos=new RootInfo[max(wThreads,2)];
	
	// Retrieving information
	wRunning=0;
	ppUpdaters=GetLocateApp()->GetUpdatersPointer();
	for (WORD i=0;i<wThreads;i++)
	{
		if (IS_UPDATER_EXITED(ppUpdaters[i]))
		{
			pRootInfos[i].pName=NULL;
			pRootInfos[i].pRoot=NULL;
			pRootInfos[i].ueError=GET_UPDATER_CODE(ppUpdaters[i]);
		}
		else 
		{
			wRunning++;

			pRootInfos[i].ueError=ueStillWorking;

			if (ppUpdaters[i]->GetCurrentDatabaseName()==NULL)
			{
				// Not started yet
				pRootInfos[i].pName=allocempty();
				pRootInfos[i].pRoot=NULL;

				if (ppUpdaters[i]->GetStatus()==CDatabaseUpdater::statusFinishing)
					pRootInfos[i].ueError=ueSuccess;

				if (m_pUpdateStatusWnd!=NULL)
				{
					pRootInfos[i].dwNumberOfDatabases=ppUpdaters[i]->GetNumberOfDatabases();
					pRootInfos[i].dwCurrentDatabase=0;
					pRootInfos[i].wProgressState=0;
				}
			}
			else
			{
				pRootInfos[i].pName=ppUpdaters[i]->GetCurrentDatabaseNameStr();
				if (ppUpdaters[i]->GetCurrentRoot()==NULL)
					pRootInfos[i].pRoot=NULL; // Is writing database
				else
					pRootInfos[i].pRoot=ppUpdaters[i]->GetCurrentRootPathStr();
				
				if (m_pUpdateStatusWnd!=NULL)
				{
					pRootInfos[i].dwNumberOfDatabases=ppUpdaters[i]->GetNumberOfDatabases();
					pRootInfos[i].dwCurrentDatabase=ppUpdaters[i]->GetCurrentDatabase();
					pRootInfos[i].wProgressState=ppUpdaters[i]->GetProgressStatus();
				}

			}
			
		}
	}
	GetLocateApp()->ReleaseUpdatersPointer();
}

void CLocateAppWnd::FreeRootInfos(WORD wThreads,RootInfo* pRootInfos)
{
	// Freeing memory
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

BOOL CLocateAppWnd::SetUpdateStatusInformation(HICON hIcon,UINT uTip)
{
	DebugFormatMessage("CLocateAppWnd::SetUpdateStatusInformation: BEGIN, hIcon=%X, uTip=%d",DWORD(hIcon),uTip);

	NOTIFYICONDATA nid;
	ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
	
	if (GetLocateApp()->m_wShellDllVersion>=0x0500)
		nid.cbSize=sizeof(NOTIFYICONDATA);
	else
		nid.cbSize=sizeof(NOTIFYICONDATA_V1_SIZE);

	nid.hWnd=*this;
	nid.uID=1000;
	nid.uFlags=0;
		
	if (hIcon!=NULL)
	{
		// Updating icon
		nid.hIcon=hIcon;
		nid.uFlags|=NIF_ICON;
	}
	
	if (uTip==IDS_NOTIFYUPDATING)
	{
		nid.uFlags|=NIF_TIP;
		
		WORD wThreads,wRunning;
		RootInfo* pRootInfos;
		GetRootInfos(wThreads,wRunning,pRootInfos);
		
		if (m_pUpdateStatusWnd!=NULL)
			m_pUpdateStatusWnd->Update(wThreads,wRunning,pRootInfos);
			
		if (wThreads>1)
		{
			if (((CLocateApp*)GetApp())->m_wShellDllVersion>=0x0500 &&	wThreads<10)
			{
				// Loading string
				char szThread[20];
				int iThreadLen=LoadString(IDS_NOTIFYTHREAD,szThread,20);
				char szCaption[30];
				int iCaptionLen=LoadString(IDS_NOTIFYUPDATINGDBS2,szCaption,30);
                char szDone[20];
				int iDoneLen=LoadString(IDS_NOTIFYDONE,szDone,20);
				char szWriting[25];
				int iWritingLen=LoadString(IDS_NOTIFYWRITINGDATABASE,szWriting,25);
				char szInitializing[25];
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
						iRequired+=istrlen(pRootInfos[i].pName);
						if (pRootInfos[i].pRoot==NULL)
							iRequiredForRoots+=iWritingLen+2;
						else
                        	iRequiredForRoots+=istrlen(pRootInfos[i].pRoot)+2;
					}
				}
				
				if (iRequired>=MAXTIPTEXTLENGTH)
				{
					char szTemp[54];
					LoadString(IDS_NOTIFYUPDATINGDBS,szTemp,54);
					wsprintf(nid.szTip,szTemp,(int)wRunning,(int)wThreads);
				}
				else
				{
					LPSTR pPtr=nid.szTip;
					CopyMemory(pPtr,szCaption,iCaptionLen);
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
							CopyMemory(pPtr,szDone,iDoneLen);
							pPtr+=iDoneLen;
						}
						else if (pRootInfos[i].pName[0]=='\0')
						{
							CopyMemory(pPtr,szInitializing,iInitializingLen);
							pPtr+=iInitializingLen;
						}
						else
						{
							int iLen=istrlen(pRootInfos[i].pName);
							CopyMemory(pPtr,pRootInfos[i].pName,iLen);
							pPtr+=iLen;
							if (iRequired+iRequiredForRoots<MAXTIPTEXTLENGTH)
							{
								*(pPtr++)=',';
								*(pPtr++)=' ';
								if (pRootInfos[i].pRoot==NULL)
								{
									CopyMemory(pPtr,szWriting,iWritingLen);
									pPtr+=iWritingLen;
								}
								else
								{
									int iLen=istrlen(pRootInfos[i].pRoot);
									CopyMemory(pPtr,pRootInfos[i].pRoot,iLen);
									pPtr+=iLen;
								}
							}
						}

					}
					
					// Checking that space is correctly calculated
					ASSERT(iRequired+iRequiredForRoots<MAXTIPTEXTLENGTH?
						DWORD(pPtr)-DWORD(nid.szTip)==iRequired+iRequiredForRoots:
						DWORD(pPtr)-DWORD(nid.szTip)==iRequired);

					*pPtr='\0';
				}
			}
			else
			{
				char szTemp[54];
                LoadString(IDS_NOTIFYUPDATINGDBS,szTemp,54);
				wsprintf(nid.szTip,szTemp,(int)wRunning,(int)wThreads);
			}
		}
		else
		{
			// Only one thread
			if (pRootInfos[0].pRoot==NULL) // Is writing database
				wsprintf(nid.szTip,(LPCSTR)CString(IDS_UPDATINGWRITINGDATABASE));
			else
			{
				char szBuf[50];
				LoadString(IDS_NOTIFYUPDATING,szBuf,50);
			
				LPSTR pRoot=pRootInfos[0].pRoot;

				// Cutting to 35 characters
				for (int i=0;i<35 && pRoot[i]!='\0';i++); 
				if (i==35)
				{
					pRoot[32]='.';
					pRoot[33]='.';
					pRoot[34]='.';
					pRoot[35]='\0';
				}
				wsprintf(nid.szTip,szBuf,pRoot);
			}
		}


		FreeRootInfos(wThreads,pRootInfos);
			
	}
	else if (uTip!=0)
	{
		LoadString(uTip,nid.szTip,63);
		nid.uFlags|=NIF_TIP;
	}
	
	DebugMessage("CLocateAppWnd::SetUpdateStatusInformation: END");
	
	return Shell_NotifyIcon(NIM_MODIFY,&nid);
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
	
	if (CLocateApp::GetProgramFlags()&CLocateApp::pfEnableUpdateTooltip && 
		m_pUpdateStatusWnd==NULL)
	{
		m_pUpdateStatusWnd=new CUpdateStatusWnd;
		
		// Registering window class for notify icon handler window
		BOOL(WINAPI * pSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD)=(BOOL(WINAPI *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(GetModuleHandle("user32.dll"),"SetLayeredWindowAttributes");
	
		BYTE nTransparency=0;
		DWORD dwExtra=WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE|WS_EX_TOPMOST;

		if (pSetLayeredWindowAttributes!=NULL)
		{
			CRegKey RegKey;
			if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs\\Updatestatus",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
			{
				DWORD dwTemp;
				if (RegKey.QueryValue("Transparency",dwTemp))
					nTransparency=(BYTE)min(dwTemp,255);
			}

			if (nTransparency>0)
				dwExtra|=WS_EX_LAYERED;
		}
		
		m_pUpdateStatusWnd->Create("LOCATEAPPUPDATESTATUS","US",WS_POPUPWINDOW|WS_VISIBLE,
			NULL,NULL,LPCSTR(0),dwExtra);

		if (nTransparency>0)
			pSetLayeredWindowAttributes(*m_pUpdateStatusWnd,0,255-nTransparency,LWA_ALPHA);

		m_pUpdateStatusWnd->SetPosition();
		

	}

	return TRUE;
}

BOOL CLocateAppWnd::StopUpdateStatusNotification()
{
	if (m_pUpdateStatusWnd!=NULL)
		m_pUpdateStatusWnd->IdleClose();
	
	if (m_pUpdateAnimIcons!=NULL)
	{
		KillTimer(ID_UPDATEANIM);
		delete[] m_pUpdateAnimIcons;
		m_pUpdateAnimIcons=NULL;
		GetLocateAppWnd()->SetUpdateStatusInformation((HICON)LoadImage(IDI_APPLICATIONICON,IMAGE_ICON,16,16,0),IDS_NOTIFYLOCATE);
	}
	return TRUE;
}
	
void CLocateAppWnd::OnInitMenuPopup(HMENU hPopupMenu,UINT nIndex,BOOL bSysMenu)
{
	CFrameWnd::OnInitMenuPopup(hPopupMenu,nIndex,bSysMenu);

	if (bSysMenu)
		return;

	if (CLocateApp::IsDatabaseMenu(hPopupMenu))
		GetLocateApp()->OnInitDatabaseMenu(hPopupMenu);
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
		OnUpdate(FALSE,LPSTR(-1));
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
				if (ShowErrorMessage(IDS_QUITNOW,IDS_UPDATINGDATABASE,MB_YESNO|MB_ICONQUESTION)==IDYES)
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
		
		if (!m_pSettings->IsFlagSet(CSettingsProperties::settingsCancelled))
		{
			// Saving settings to registry
			m_pSettings->SaveSettings();
			
			// Set CLocateAppWnd to use new settings
			GetLocateApp()->UpdateSettings();
			SetSchedules(m_pSettings->GetSchedules());
			SaveSchedules();
			
			if (GetLocateDlg()!=NULL)
			{
				// Set LocateDlg to use new seetings
				if (m_pSettings->IsFlagSet(CSettingsProperties::settingsIsUsedDatabaseChanged))
				{
					GetLocateDlg()->m_NameDlg.InitDriveBox();
					GetLocateDlg()->ResetFileNotificators();
				}
				GetLocateDlg()->UpdateSettings();
			}
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
	GetLocateApp()->ClearStartupFlag(CLocateApp::CStartData::startupExitAfterUpdating);
	
	// Refreshing icon
	SetUpdateStatusInformation(m_pUpdateAnimIcons!=NULL?m_pUpdateAnimIcons[m_nCurUpdateAnimBitmap]:NULL);
	
	if (m_pLocateDlgThread==NULL)
	{
		// Hiding LocateST
		if (GetFocus()==NULL)
			ForceForegroundAndFocus();
		
		m_pLocateDlgThread=new CLocateDlgThread;
		m_pLocateDlgThread->CreateThread();

		while (m_pLocateDlgThread->m_pLocate==NULL)
			Sleep(10);
		while (m_pLocateDlgThread->m_pLocate->GetHandle()==NULL)
			Sleep(10);

		ShowWindow(swHide);

		m_pLocateDlgThread->m_pLocate->ForceForegroundAndFocus();
		
	}
	else
	{
		ForceForegroundAndFocus();
		
		CLocateDlg* pLocateDlg=GetLocateDlg();

		// Restore dialog if needed
		WINDOWPLACEMENT wp;
		wp.length=sizeof(WINDOWPLACEMENT);
		pLocateDlg->GetWindowPlacement(&wp);
		if (wp.showCmd!=SW_MAXIMIZE)
            pLocateDlg->ShowWindow(swRestore);

		pLocateDlg->BringWindowToTop();
		pLocateDlg->ForceForegroundAndFocus();
	}

	return TRUE;
}

DWORD WINAPI CLocateAppWnd::KillUpdaterProc(LPVOID lpParameter)
{
	return ((CLocateApp*)GetApp())->StopUpdating(TRUE);
}



BYTE CLocateAppWnd::OnUpdate(BOOL bStopIfProcessing,LPSTR pDatabases)
{
	if (!GetLocateApp()->IsUpdating())
	{
		if (pDatabases==LPSTR(-1))
		{
			CArrayFP<PDATABASE> aDatabases;
			CSelectDatabasesDlg dbd(GetLocateApp()->GetDatabases(),aDatabases,
				(GetLocateApp()->GetStartupFlags()&CLocateApp::CStartData::startupDatabasesOverridden?CSelectDatabasesDlg::flagDisablePresets:0)|
				CSelectDatabasesDlg::flagShowThreads|CSelectDatabasesDlg::flagSetUpdateState,
				CString(IDS_REGPLACE,CommonResource)+"\\Dialogs\\SelectDatabases/Update");
			if (!dbd.DoModal(m_pLocateDlgThread!=NULL?HWND(*GetLocateDlg()):HWND(*this)))
                return FALSE;
			if (GetLocateApp()->IsUpdating())
				return FALSE;
			if (!GetLocateApp()->GlobalUpdate(&aDatabases))
				return FALSE;
		}
		else if (pDatabases==NULL)
		{
			if (!GetLocateApp()->GlobalUpdate(NULL))
				return FALSE;
		}
		else
		{
			CArrayFP<PDATABASE> aDatabases;
			const CArray<PDATABASE>& aGlobalDatabases=GetLocateApp()->GetDatabases();
			for (int i=0;i<aGlobalDatabases.GetSize();i++)
			{
				LPSTR pPtr=pDatabases;
				BOOL bFound=FALSE;
				while (*pPtr!=NULL)
				{
					int iStrLen=istrlen(pPtr)+1;
					if (strncmp(pPtr,aGlobalDatabases[i]->GetName(),iStrLen)==0)
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
					aDatabases.Add(pDatabase);
				}
			}

			if (aDatabases.GetSize()==0)
				return FALSE;
			if (!GetLocateApp()->GlobalUpdate(&aDatabases))
				return FALSE;
		}

		StartUpdateStatusNotification();

		CLocateDlg* pLocateDlg=GetLocateDlg();
		if (pLocateDlg!=NULL)
		{
			pLocateDlg->StartUpdateAnimation();
		
			pLocateDlg->m_pStatusCtrl->SetText(szEmpty,0,0);
			pLocateDlg->m_pStatusCtrl->SetText(CString(IDS_UPDATINGDATABASE),1,0);		
		}
	}
	else if (bStopIfProcessing)
	{
		// Starting thread which stops updating		
		DWORD dwThreadID;
		HANDLE hThread=CreateThread(NULL,0,KillUpdaterProc,(void*)this,0,&dwThreadID);
		if (hThread!=NULL)
			CloseHandle(hThread);
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
			GetLocateDlg()->PostMessage(WM_CLOSE);
			
			if (m_pLocateDlgThread!=NULL)
			{
				WaitForSingleObject(hLocateThread,1000);
			}
		
			if (m_pLocateDlgThread!=NULL)
			{
				DebugNumMessage("Terminating locate dialog thread %X",(DWORD)m_pLocateDlgThread);
				::TerminateThread(hLocateThread,1);
				
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
	
	CFrameWnd::OnDestroy();
	DebugMessage("void CLocateAppWnd::OnDestroy() END");
}

BOOL CLocateAppWnd::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
	case WM_SYSTEMTRAY:
		return OnSystemTrayMessage((UINT)wParam,(UINT)lParam);
	case WM_ANOTHERINSTANCE:
		return OnAnotherInstance((ATOM)lParam);
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
			if (!m_aShortcuts[int(wParam)]->IsForegroundWindowOk(*this))
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
	case WM_RESETSHORTCUTS:
		TurnOnShortcuts();
		break;
	default:
		if (msg==nHFCInstallationMessage)
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
		else if (msg==nTaskbarCreated)
			AddTaskbarIcon();
		break;
	}
	return CFrameWnd::WindowProc(msg,wParam,lParam);
}

void CLocateAppWnd::AddTaskbarIcon()
{
	// Creating taskbar icon
	NOTIFYICONDATA nid;
	nid.cbSize=NOTIFYICONDATA_V1_SIZE;
	nid.hWnd=*this;
	nid.uID=1000;
	nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
	nid.uCallbackMessage=WM_SYSTEMTRAY;
	nid.hIcon=(HICON)LoadImage(IDI_APPLICATIONICON,IMAGE_ICON,16,16,LR_DEFAULTCOLOR|LR_SHARED);
	LoadString(IDS_NOTIFYLOCATE,nid.szTip,63);

	Shell_NotifyIcon(NIM_ADD,&nid);

	if (((CLocateApp*)GetApp())->m_wShellDllVersion>=0x0500)
	{
		nid.cbSize=sizeof(NOTIFYICONDATA);
		nid.uVersion=NOTIFYICON_VERSION;
		Shell_NotifyIcon(NIM_SETVERSION,&nid);
	}
}

void CLocateAppWnd::DeleteTaskbarIcon()
{
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
		m_nCurUpdateAnimBitmap++;
		if (m_nCurUpdateAnimBitmap>12)
			m_nCurUpdateAnimBitmap=0;
		SetUpdateStatusInformation(m_pUpdateAnimIcons[m_nCurUpdateAnimBitmap]);
		break;
	case ID_SYNCSCHEDULES:
		KillTimer(ID_SYNCSCHEDULES);
		SetTimer(ID_CHECKSCHEDULES,1000,NULL);
	case ID_CHECKSCHEDULES:
		CheckSchedules();
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

DWORD CLocateAppWnd::OnAnotherInstance(ATOM aCommandLine)
{
	if (aCommandLine==NULL)
		OnLocate();
	else
	{
		char szCmdLine[257];
		GlobalGetAtomName(aCommandLine,szCmdLine,256);
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
	DebugNumMessage("CLocateAppWnd::SetSchedules(0x%X) START",(DWORD)pSchedules);
	if (pSchedules==NULL)
	{
		CRegKey RegKey;
		CString Path;
		Path.LoadString(IDS_REGPLACE,CommonResource);
		Path<<"\\General";
		if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		{
			DWORD nKeyLen=RegKey.QueryValueLength("Schedules");
			BYTE* pSchedules=new BYTE[nKeyLen];
			RegKey.QueryValue("Schedules",(LPSTR)pSchedules,nKeyLen);
#ifdef _DEBUG
			char* pTmpData=new char[nKeyLen*2+2];
			for (DWORD i=0;i<nKeyLen;i++)
				sprintf(pTmpData+i*2,"%02X",pSchedules[i]);
			DebugFormatMessage("SCHEDULES(length=%d): %s",nKeyLen,pTmpData);
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
						DebugFormatMessage("SCHEDULEV1: type=%d",((CSchedule*)pPtr)->m_nType);
						m_Schedules.AddTail(new CSchedule(pPtr,1));
					}
				}
			}	
			else if (pSchedules[1]==2)
			{
				if (nKeyLen>=6 && pSchedules[0]==sizeof(CSchedule))
				{
					BYTE* pPtr=pSchedules+6;
					for (DWORD n=0;n<*(DWORD*)(pSchedules+2);n++)
					{
						if (pPtr+sizeof(CSchedule)>=pSchedules+nKeyLen)
							break;

						DebugFormatMessage("SCHEDULEV2: type=%d",((CSchedule*)pPtr)->m_nType);
						m_Schedules.AddTail(new CSchedule(pPtr,2));
					}
				}
			}
			delete[] pSchedules;
		}	
	}
	else
	{
		
		m_Schedules.Swap(*pSchedules);
#ifdef _DEBUG
		POSITION pPos=m_Schedules.GetHeadPosition();
		while (pPos!=NULL)
		{
			DebugFormatMessage("SCHEDULE: type=%d",m_Schedules.GetAt(pPos)->m_nType);
			pPos=m_Schedules.GetNextPosition(pPos);
		}
#endif
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	SetTimer(ID_SYNCSCHEDULES,1000-st.wMilliseconds,NULL);
	DebugMessage("CLocateAppWnd::SetSchedules END");
	return m_Schedules.GetCount();
}

BOOL CLocateAppWnd::SaveSchedules()
{
	DebugMessage("CLocateAppWnd::SaveSchedules() START");
	
	CRegKey RegKey;
	CString Path;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\General";
	
	if (RegKey.OpenKey(HKCU,Path,CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		DWORD dwDataLen=6;
		POSITION pPos=m_Schedules.GetHeadPosition();
		while (pPos!=NULL)
		{
			dwDataLen+=m_Schedules.GetAt(pPos)->GetDataLen();
			pPos=m_Schedules.GetNextPosition(pPos);
		}		
		
		BYTE* pSchedules=new BYTE[dwDataLen];
		if (pSchedules==NULL)
		{
			SetHFCError(HFC_CANNOTALLOC);
			DebugMessage("LocateAppWnd::OnDestroy(): Cannot allocate memory.");
		}
		pSchedules[0]=sizeof(CSchedule);
		pSchedules[1]=2; //version
		*(DWORD*)(pSchedules+2)=m_Schedules.GetCount();
		
		
		BYTE* pPtr=pSchedules+6;
		pPos=m_Schedules.GetHeadPosition();
		while (pPos!=NULL)
		{
			DebugFormatMessage("SCHEDULE: type %d",m_Schedules.GetAt(pPos)->m_nType);
			pPtr+=m_Schedules.GetAt(pPos)->GetData(pPtr);
			pPos=m_Schedules.GetNextPosition(pPos);
		}
		RegKey.SetValue("Schedules",(LPCSTR)pSchedules,dwDataLen,REG_BINARY);
		
#ifdef _DEBUG
		DWORD dwTmpDataLen=sizeof(CSchedule)*m_Schedules.GetCount()+6;
		char* pTmpData=new char[dwTmpDataLen*2+2];
		for (DWORD i=0;i<dwTmpDataLen;i++)
			sprintf(pTmpData+i*2,"%02X",pSchedules[i]);
		DebugFormatMessage("SCHEDULES(length=%d): %s",dwTmpDataLen,pTmpData);
		delete[] pTmpData;
#endif
		delete[] pSchedules;
	}
	DebugMessage("CLocateAppWnd::SaveSchedules() END");
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
				bSchedulesChanged=TRUE;
				
				sMemCopy(&pSchedule->m_tLastStartDate,&tCurDate,sizeof(CSchedule::SDATE));
				sMemCopy(&pSchedule->m_tLastStartTime,&tCurTime,sizeof(CSchedule::STIME));
				
				pSchedule->m_bFlags|=CSchedule::flagRunned;
				if (pSchedule->m_nType==CSchedule::typeAtStartup)
					pSchedule->m_bFlags|=CSchedule::flagRunnedAtStartup;
				
				OnUpdate(FALSE,pSchedule->m_pDatabases);
				
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
		pPos=m_Schedules.GetNextPosition(pPos);
	}
	if (bSchedulesChanged)
		SaveSchedules();
}

////////////////////////////
// CLocateAppWnd::CUpdateStatusWnd
CLocateAppWnd::CUpdateStatusWnd::CUpdateStatusWnd()
:	CFrameWnd(),m_WindowSize(0,0)
{
	
	RegisterWndClass("LOCATEAPPUPDATESTATUS",CS_HREDRAW|CS_VREDRAW,LoadCursor(NULL,IDC_ARROW),
		(HBRUSH)(COLOR_INFOBK+1),NULL);

	m_cTextColor=GetSysColor(COLOR_INFOTEXT);
	m_cTitleColor=GetSysColor(COLOR_INFOTEXT);
	m_cErrorColor=GetSysColor(COLOR_INFOTEXT);
	m_cBackColor=GetSysColor(COLOR_INFOBK);

	// Update status tooltip
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs\\Updatestatus",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
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

	m_hUpdateMutex=CreateMutex(NULL,FALSE,NULL);
}

CLocateAppWnd::CUpdateStatusWnd::~CUpdateStatusWnd()
{
	if (m_hUpdateMutex!=NULL)
	{
		if (WaitForMutex(m_hUpdateMutex,BACKGROUNDUPDATERMUTEXTIMEOUT))
		{
			DebugMessage("CLocateAppWnd::CUpdateStatusWnd::~CUpdateStatusWnd() WaitForMutex returns error");
		}
	
		CloseHandle(m_hUpdateMutex);
		m_hUpdateMutex=NULL;
	}
	
	m_aErrors.RemoveAll();

	m_Font.DeleteObject();
	m_TitleFont.DeleteObject();

	
}
	
int CLocateAppWnd::CUpdateStatusWnd::OnCreate(LPCREATESTRUCT lpcs)
{
	SetTimer(ID_UPDATESTATUS,500,NULL);
	return CFrameWnd::OnCreate(lpcs);
}



void CLocateAppWnd::CUpdateStatusWnd::OnNcDestroy()
{
	CWnd::OnNcDestroy();

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

	if (WaitForMutex(m_hUpdateMutex,BACKGROUNDUPDATERMUTEXTIMEOUT))
	{
		DebugMessage("CLocateAppWnd::CUpdateStatusWnd::OnPaint() WaitForMutex returns error");
		return;
	}
	
	RECT rcClient,rc2;
	GetClientRect(&rcClient);
	rcClient.left+=EXTRA_MARGINSX;
	//rcClient.top+=EXTRA_MARGINSY;

	
    // Drawing title
	LPCSTR pPtr=sStatusText;
	int nLength=FirstCharIndex(pPtr,'\n');
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
		nLength=FirstCharIndex(pPtr,'\n');

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
	ReleaseMutex(m_hUpdateMutex);
}


void CLocateAppWnd::CUpdateStatusWnd::FormatErrorForStatusTooltip(UpdateError ueError,CDatabaseUpdater* pUpdater)
{
	if (ueError==ueStopped)
		return;

	if (WaitForMutex(m_hUpdateMutex,BACKGROUNDUPDATERMUTEXTIMEOUT))
	{
		DebugMessage("CLocateAppWnd::CUpdateStatusWnd::~CUpdateStatusWnd() WaitForMutex returns error");
		return;
	}

	// Now, change pointer to null, if someone is accesing pointer, it may have enough time to read
	char error[300];
	int nLabelLength=LoadString(IDS_LASTERROR,error,200);

	LPCSTR szExtra=NULL;

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
		sprintf(error+nLabelLength,"%d",(int)ueError);
		break;
	}

	LPSTR szNewPtr;
	int nLength;
	if (szExtra!=NULL)
	{
		szNewPtr=new char[istrlen(error)+istrlen(szExtra)+1];
		nLength=wsprintf(szNewPtr,error,szExtra);
	}
	else
	    szNewPtr=alloccopy(error,nLength=istrlen(error));

	m_aErrors.Add(szNewPtr);

	// Make status window bigger
	{
		CDC dc(this);
		dc.SetMapMode(MM_TEXT);
		dc.SelectObject(m_Font);
		CSize sz=dc.GetTextExtent(szNewPtr,nLength);
	
		if (m_WindowSize.cx<sz.cx)
			m_WindowSize.cx=sz.cx;
		m_WindowSize.cy+=sz.cy+EXTRA_MARGINSY;
	}
	
	ReleaseMutex(m_hUpdateMutex);

	SetPosition();
}


void CLocateAppWnd::CUpdateStatusWnd::FormatStatusTextLine(CString& str,const CLocateAppWnd::RootInfo& pRootInfo,int nThreadID,int nThreads)
{
	// #X  thread number
	if (nThreadID!=-1)
		str << '#' << (int)nThreadID;

	if (pRootInfo.pName==NULL)
	{
		// Finished
		if (nThreadID!=-1)
			str << ": ";

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
	else if (pRootInfo.pName[0]=='\0')
	{
		// Initializing/finishing
		if (nThreadID!=-1)
			str << ": ";

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
                str << ' ';
			str << (int)(pRootInfo.dwCurrentDatabase+1) << '/' << (int)pRootInfo.dwNumberOfDatabases << ": ";
		}
		else if (nThreadID!=-1)
			str << ": ";
		str.AddString(IDS_UPDATINGUPDATING);
		str << pRootInfo.pName;

		
		if (pRootInfo.pRoot==NULL)
		{
			str << ": ";
			str.AddString(IDS_NOTIFYWRITINGDATABASE);
		}
		else
		{
			if (pRootInfo.wProgressState!=WORD(-1))
			{
				str << ' ' << (int)((int)(pRootInfo.wProgressState)/10) << '%';
			}

			str << ": ";
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
		strcpy(pTextFont->lfFaceName,"Tahoma");
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
		strcpy(pTitleFont->lfFaceName,"Tahoma");
	}
}


void CLocateAppWnd::CUpdateStatusWnd::SetFonts()
{
	if (m_TitleFont.m_hObject!=NULL && m_Font.m_hObject!=NULL)
		return;

	// Update status tooltip
	CRegKey RegKey;
	LOGFONT lTitleFont,lTextFont;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs\\Updatestatus",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
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

BOOL CLocateAppWnd::CUpdateStatusWnd::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
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
	}
	return CWnd::WindowProc(msg,wParam,lParam);
}


void CLocateAppWnd::CUpdateStatusWnd::SetPosition()
{	
	CRect rcDesktopRect,rcTrayRect;
	HWND hShellTrayWnd=FindWindow("Shell_TrayWnd",NULL); // Whole tray window
	HWND hDesktop=FindWindow("Progman","Program Manager");
	if (hDesktop==NULL)
		hDesktop=GetDesktopWindow();
	
	CPoint ptUpperLeft;
	CSize szSize;
	
	// Computing width and height
	if (m_WindowSize.cx==0 && m_WindowSize.cy==0)
	{
		SetFonts();
	
		CDC dc(this);
		dc.SetMapMode(MM_TEXT);

		// Checking how much space title will take
		CString str(IDS_UPDATINGTOOLTIPTITLE);
		dc.SelectObject(m_TitleFont);
		szSize=dc.GetTextExtent(str);	
		
		str.LoadString(IDS_UPDATINGENDED);
		EnlargeSizeForText(dc,str,szSize);
		
        // Changing font
		dc.SelectObject(m_Font);
		
		CDatabaseUpdater** ppUpdaters=GetLocateApp()->GetUpdatersPointer();
		if (ppUpdaters!=NULL)
		{
			for (WORD wThreads=0;ppUpdaters[wThreads]!=NULL;wThreads++);

            CLocateAppWnd::RootInfo ri;
			for (int i=0;ppUpdaters[i]!=NULL;i++)
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
				ri.pName=(LPSTR)szEmpty;
				str.Empty();
				ri.ueError=ueStillWorking;
				FormatStatusTextLine(str,ri,wThreads>1?i+1:-1,wThreads);
				EnlargeSizeForText(dc,str,szThisLine);
				ri.ueError=ueSuccess;
				FormatStatusTextLine(str,ri,wThreads>1?i+1:-1,wThreads);
				EnlargeSizeForText(dc,str,szThisLine);
					
				
				LPCSTR szFile=NULL;
				CDatabase::ArchiveType nArchiveType;
				CDatabaseUpdater::CRootDirectory* pRoot;
				ri.dwNumberOfDatabases=ppUpdaters[i]->GetNumberOfDatabases();
				for (ri.dwCurrentDatabase=0;ppUpdaters[i]->EnumDatabases(ri.dwCurrentDatabase,ri.pName,szFile,nArchiveType,pRoot);ri.dwCurrentDatabase++)
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
		GetLocateApp()->ReleaseUpdatersPointer();
		
		szSize.cx+=2*EXTRA_MARGINSX;
		szSize.cy+=2*EXTRA_MARGINSY;

	}
	else
		szSize=m_WindowSize;

	// Checking where tray window is
	::GetWindowRect(hDesktop,&rcDesktopRect);
	if (hShellTrayWnd==NULL)
	{
		// Default
		ptUpperLeft.x=rcDesktopRect.right-szSize.cx-10;
		ptUpperLeft.y=rcDesktopRect.bottom-szSize.cy-30;
		return;
	}

    ::GetWindowRect(hShellTrayWnd,&rcTrayRect);
	POINT ptDesktopCenter={(rcDesktopRect.left+rcDesktopRect.right)/2,(rcDesktopRect.top+rcDesktopRect.bottom)/2};
	
	if (rcTrayRect.top>ptDesktopCenter.y)
	{
		// Bottom
		ptUpperLeft.x=rcTrayRect.right-szSize.cx-1;
		ptUpperLeft.y=rcTrayRect.top-szSize.cy-1;
	}
	else if (rcTrayRect.bottom<ptDesktopCenter.y)
	{
		// Top
		ptUpperLeft.x=rcTrayRect.right-szSize.cx-1;
		ptUpperLeft.y=rcTrayRect.bottom+1;
	}
	else if (rcTrayRect.right<ptDesktopCenter.x)
	{
		// Left
		ptUpperLeft.y=rcDesktopRect.bottom-szSize.cy-1;
		ptUpperLeft.x=rcTrayRect.right+1;
	}
	else
	{
		// Right
		ptUpperLeft.y=rcDesktopRect.bottom-szSize.cy-1;
		ptUpperLeft.x=rcTrayRect.left-szSize.cx-1;
	}

	SetWindowPos(HWND_TOP,ptUpperLeft.x,ptUpperLeft.y,szSize.cx,szSize.cy,SWP_NOZORDER|SWP_NOACTIVATE);
	m_WindowSize=szSize;
}


void CLocateAppWnd::CUpdateStatusWnd::Update(WORD wThreads,WORD wRunning,RootInfo* pRootInfos)
{
	if (WaitForMutex(m_hUpdateMutex,BACKGROUNDUPDATERMUTEXTIMEOUT))
	{
		DebugMessage(" CLocateAppWnd::CUpdateStatusWnd::Update WaitForMutex returns error");
		return;
	}
	
	
	sStatusText.Empty();

	// Forming title
	if (wRunning>0)
		sStatusText.LoadString(IDS_UPDATINGTOOLTIPTITLE);
	else
		sStatusText.LoadString(IDS_UPDATINGENDED);

	for (WORD i=0;i<wThreads;i++)
	{
		sStatusText << '\n';
		FormatStatusTextLine(sStatusText,pRootInfos[i],wThreads>1?i+1:-1,wThreads);
	}
	
	ReleaseMutex(m_hUpdateMutex);

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
	GetLocateAppWnd()->m_pUpdateStatusWnd=NULL;
	CWnd::DestroyWindow();
	return TRUE;
}

#ifdef _DEBUG
DEBUGALLOCATORTYPE DebugAlloc;
#endif
FASTALLOCATORTYPE Allocation;

CLocateApp theApp;

