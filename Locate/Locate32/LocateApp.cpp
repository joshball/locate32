#include <HFCLib.h>
#include "Locate32.h"

#include "wfext.h"
#include "shlwapi.h"

CLocateApp::CLocateApp()
:	CWinApp("LOCATE32"),m_nDelImage(0),m_nStartup(0),
	m_ppUpdaters(NULL),m_pLastDatabase(NULL)
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
	LoadDateAndTimeString();
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
		SaveDateAndTimeString();
	}


	if (GetLanguageSpecificResourceHandle()!=GetInstanceHandle())
	{
		FreeLibrary(GetLanguageSpecificResourceHandle());
		SetResourceHandle(GetInstanceHandle(),SetBoth);
	}
	return 0;
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
					CString& sExe=GetApp()->GetExeName();
					pStartData->m_aDatabases.Add(CDatabase::FromDefaults(TRUE,sExe,sExe.FindLast('\\')+1));
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
							CString& sExe=GetApp()->GetExeName();
							pStartData->m_aDatabases.Add(CDatabase::FromDefaults(TRUE,sExe,sExe.FindLast('\\')+1));
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
			CString& sExe=GetApp()->GetExeName();
			pDatabase=CDatabase::FromDefaults(TRUE,sExe,sExe.FindLast('\\')+1); // Nothing else can be done?
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

void CLocateApp::LoadDateAndTimeString()
{
	m_strDateFormat.Empty();
	m_strTimeFormat.Empty();

	CString Path;
	CRegKey RegKey;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\General";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("DateFormat",m_strDateFormat);
		RegKey.QueryValue("TimeFormat",m_strTimeFormat);
	}

}

void CLocateApp::SaveDateAndTimeString()
{
	CString Path;
	CRegKey RegKey;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\General";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		RegKey.SetValue("DateFormat",m_strDateFormat);
		RegKey.SetValue("TimeFormat",m_strTimeFormat);
	}
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
			if (((CLocateAppWnd*)dwParam)->GetProgramFlags()&CLocateAppWnd::pfShowCriticalErrors)
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
			if (((CLocateAppWnd*)dwParam)->GetProgramFlags()&CLocateAppWnd::pfShowCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORCANNOTOPENDB,pUpdater->GetCurrentDatabaseFile());
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
				return FALSE;
			}
			break;
		case ueRead:
			if (((CLocateAppWnd*)dwParam)->GetProgramFlags()&CLocateAppWnd::pfShowCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORCANNOTREADDB,pUpdater->GetCurrentDatabaseFile());
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
				return FALSE;
			}
			break;
		case ueWrite:
			if (((CLocateAppWnd*)dwParam)->GetProgramFlags()&CLocateAppWnd::pfShowCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORCANNOTWRITEDB,pUpdater->GetCurrentDatabaseFile());
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
                return FALSE;
			}
			break;
		case ueAlloc:
			if (((CLocateAppWnd*)dwParam)->GetProgramFlags()&CLocateAppWnd::pfShowCriticalErrors)
			{
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,
					CString(IDS_ERRORCANNOTALLOCATE),CString(IDS_ERROR),MB_OK|MB_ICONERROR);
			}
			break;
		case ueInvalidDatabase:
			if (((CLocateAppWnd*)dwParam)->GetProgramFlags()&CLocateAppWnd::pfShowCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORINVALIDDB,pUpdater->GetCurrentDatabaseName());
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
				return FALSE;
			}
			break;
		case ueFolderUnavailable:
			if (((CLocateAppWnd*)dwParam)->GetProgramFlags()&CLocateAppWnd::pfShowNonCriticalErrors)
			{
				CString str;
				str.Format(IDS_ERRORROOTNOTAVAILABLE,pUpdater->GetCurrentRootPath()!=NULL?pUpdater->GetCurrentRootPath():"");
				::MessageBox(dwParam!=NULL?(HWND)*((CLocateAppWnd*)dwParam):NULL,str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
				return FALSE;
			}
			break;
		case ueCannotIncrement:
			if (((CLocateAppWnd*)dwParam)->GetProgramFlags()&CLocateAppWnd::pfShowNonCriticalErrors)
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

	CString Path(GetApp()->GetExeName(),GetApp()->GetExeName().FindLast('\\')+1);
	
	

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
// CLocateAppWnd

int CLocateAppWnd::OnCreate(LPCREATESTRUCT lpcs)
{
	// Loading menu
	m_Menu.LoadMenu(IDR_SYSTEMTRAYMENU);

	// Loading registry settings
	LoadRegistry();
	
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
	
	// Turn on explorer's Ctrl+F and F3 overriding
	CRegKey RegKey;
	CString Path(IDS_REGPLACE,CommonResource);
	Path<<"\\General";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD dwValue=0;
		RegKey.QueryValue("OverrideExplorer",dwValue);
		if (dwValue)
			m_hHook=SetHook(*this);			
	}
	BOOL bRet=CFrameWnd::OnCreateClient(lpcs);
	if (bDoOpen)
		OnLocate();
	return bRet;
}

void CLocateAppWnd::SaveRegistry() const
{
	CRegKey RegKey;
	if(RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",
		CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		RegKey.SetValue("General Flags",m_dwProgramFlags&pfSave);
	}
}

void CLocateAppWnd::LoadRegistry()
{
	// When modifications are done, check whether 
	// function is applicable for UpdateSettings
	
	
	CRegKey RegKey;
	DWORD temp;

	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		// Loading dwFlags
		temp=m_dwProgramFlags;
		RegKey.QueryValue("General Flags",temp);
		m_dwProgramFlags&=~pfSave;
		m_dwProgramFlags|=temp&pfSave;
	}
}

BOOL CLocateAppWnd::UpdateSettings()
{
	LoadRegistry();

	// TODO: Close update tooltip if it present
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
	
	if (m_dwProgramFlags&pfEnableUpdateTooltip && 
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
	case IDM_GLOBALUPDATEDBACCEL:
		OnUpdate(FALSE);
		break;
	case IDM_UPDATEDATABASES:
	case IDM_UPDATEDATABASESACCEL:
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
			UpdateSettings();
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
	
	DeleteTaskbarIcon();

	PostQuitMessage(0);
	m_Menu.DestroyMenu();

	KillTimer(ID_CHECKSCHEDULES);
	
	// Ensure that update animation and status window are stopped
	StopUpdateStatusNotification();
	if (m_pUpdateStatusWnd!=NULL)
		m_pUpdateStatusWnd->DestroyWindow();
	
	
	// Unhookin if necessary
	if (m_hHook!=NULL)
		UnsetHook(m_hHook);

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

		DebugNumMessage("Trying to close locate dialog thread %X",(DWORD)m_pLocateDlgThread);
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
	SaveRegistry();
	
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
		if (m_pLocateDlgThread==NULL)
			return NULL;
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
	default:
		if (msg==nHFCInstallationMessage)
		{
			if (lParam!=NULL)
			{	
				char szAppLine[257];
				GlobalGetAtomName((ATOM)lParam,szAppLine,256);
				if (GetApp()->GetAppName().CompareNoCase(szAppLine)==0)
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

