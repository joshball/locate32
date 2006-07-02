/* Copyright (c) 1997-2006 Janne Huttunen
   locate.exe v2.99.6.6040                 */

const char* szVersionStr="locate 3.0 beta 6.6040";

#include <hfclib.h>
#ifndef WIN32
#include <go32.h>
#include <sys/farptr.h>
#include <conio.h>
#endif
      
#include "locater/locater.h"
#include "lan_resources.h"




int Lines=0;

enum {
	flagWholeWord=0x1,
	flagReplaceSpaces=0x2,
	flagShowWhatAreWeLookingFor=0x4,
	flagOutputIsPaged=0x8,
	flagNoSubDirectories=0x10
};
BYTE dwMainFlags=flagReplaceSpaces;


#ifdef WIN32
HANDLE hStdOut;
HANDLE hStdIn;
volatile BOOL nShouldQuit=FALSE;

DWORD GetConsoleLines()
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	if (!GetConsoleScreenBufferInfo(hStdOut,&csbiInfo))
		return 24;		
	return csbiInfo.srWindow.Bottom-csbiInfo.srWindow.Top;
}
#endif

BOOL SetLanguageSpecifigHandles(LPCWSTR szAppPath)
{
	CRegKey RegKey;
	CStringW LangFile;
	if (RegKey.OpenKey(HKCU,"Software\\Update",
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue(L"Language",LangFile);
		RegKey.CloseKey();
	}
	if (LangFile.IsEmpty())
		LangFile=L"lan_en.dll";

	CStringW Path(szAppPath,LastCharIndex(szAppPath,'\\')+1);
	
	HINSTANCE hLib=FileSystem::LoadLibrary(Path+LangFile);
	if (hLib==NULL)
	{
		hLib=FileSystem::LoadLibrary(Path+L"lan_en.dll");

		if (hLib==NULL)
		{
			fwprintf(stderr,L"Cannot load language file '%s'\n",(LPCWSTR)LangFile);
			return FALSE;
		}

		fwprintf(stderr,L"Cannot load language file '%s', using 'lan_en.dll'\n",(LPCWSTR)LangFile);
	}

	SetResourceHandle(hLib,LanguageSpecificResource);
	return TRUE;
}

/*
void ReadIniFile(LPSTR szData,LPSTR szPath)
{
    int i;
    while (1)
    {
        while (szData[0]==' ' || szData[0]=='\n' || szData[0]=='\r') szData++;
        for (i=0;szData[i]!='=' && szData[i]!='\0';i++);
        if (szData[i]=='\0')
            return;    
        szData[i]='\0';
        if (strcasecmp(szData,"DatabaseFile")==0)
        {
            szData+=i+1;
            for (i=0;szData[i]!='\0' && szData[i]!='\n';i++);
            if (szData[i-1]=='\r')
               szData[i-1]='\0';
            else
               szData[i]='\0';
            strcpy(szPath,szData);
            return;
        }
        while (szData[0]=='\n' || szData[0]=='\r')
        {
            if (szData[0]=='\0')
                return;
            szData++;   
        }     
    }
}


#ifndef WIN32
void GetFromDB(char* own,char* szPath)
{
   int i;
   for (i=strlen(own);i>=0 && own[i]!='\\' && own[i]!='/' ;i--);
   MemCopy(szPath,own,i);
   MemCopy(szPath+i,"\\locate.ini",12);
   FILE* fp;
   fp=fopen(szPath,"rb");
   szPath[0]='\0';
   if (fp!=NULL)
   {
       LPSTR szFile;
       int nSize=filelength(fileno(fp));
	       szFile=new char[nSize+2];
       if (szFile!=NULL)
       {
           if (0!=fread(szFile,1,nSize,fp))
           {
               szFile[nSize]='\0';
               ReadIniFile(szFile,szPath);	
           }
           delete[] szFile; 
       }
       fclose(fp);
   }
   if (!IsFile(szPath)) 
   {
       MemCopy(szPath,own,i);
       MemCopy(szPath+i,"\\files.dbs",11);
   }
}
#endif
*/
    
BOOL CALLBACK LocateProc(DWORD dwParam,CallingReason crReason,UpdateError ue,DWORD dwInfo,const CLocater* pLocater)
{          
	switch (crReason)
	{
	case ErrorOccured:
		switch (ue)
		{
		case ueUnknown:
			fwprintf(stderr,(LPCWSTR)ID2W(IDS_LOCATEUNKNOWNERROR));
			break;
		case ueOpen:
			fwprintf(stderr,(LPCWSTR)ID2W(IDS_LOCATECANNOTOPEN),pLocater->GetCurrentDatabaseFile());
			break;
		case ueRead:
			fwprintf(stderr,(LPCWSTR)ID2W(IDS_LOCATECANNOTREAD),pLocater->GetCurrentDatabaseFile());
			break;
		case ueInvalidDatabase:
			fwprintf(stderr,(LPCWSTR)ID2W(IDS_LOCATEINVALIDDATABASE),pLocater->GetCurrentDatabaseFile());
			break;
		}
		break;
	}
    return TRUE;
}

BOOL CALLBACK LocateFoundProc(DWORD dwParam,BOOL bFolder,const CLocater* pLocater)
{
	if (nShouldQuit)
		return FALSE;

	if (dwMainFlags&flagOutputIsPaged)
	{
#ifdef WIN32
		if (Lines>int(GetConsoleLines()-1))
#else
		if (Lines>(_farpeekb(_dos_ds,0x484)-1))
#endif
		{
			DWORD nTmp=1;char szbuf[2];
			SetConsoleMode(hStdIn,0);
			ReadConsole(hStdIn,szbuf,1,&nTmp,NULL);
			SetConsoleMode(hStdIn,ENABLE_PROCESSED_INPUT);
			if (szbuf[0]==3) // Ctrl+C
				return FALSE;
			Lines=0;
		}
		Lines++;
	}
	if (bFolder)
		printf("%s\\%s\n",pLocater->GetCurrentPath(),pLocater->GetFolderName());
	else
		printf("%s\\%s\n",pLocater->GetCurrentPath(),pLocater->GetFileName());
	return TRUE;
}

#ifdef WIN32
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
		nShouldQuit=TRUE;
		break;
	}
	return 1;
}
#endif

void showverbose(LPCWSTR* ppStrings,UINT nStrings,LPCWSTR* ppExtensions,UINT nExtension,LPCWSTR* ppDirectories,UINT nDirectories)
{
	if (nStrings>0)
	{
		wprintf(ID2W(IDS_LOCATEVERBOSESTR));
		wprintf(ppStrings[0]);

		for (UINT i=1;i<nStrings;i++)
			wprintf(L", %s",ppStrings[i]);
		putchar('\n');
	}

	if (nExtension>0)
	{
		wprintf(ID2W(IDS_LOCATEVERBOSEEXT));
		wprintf(ppExtensions[0]);

		for (UINT i=1;i<nExtension;i++)
			wprintf(L", %s",ppExtensions[i]);
		putchar('\n');
	}


	if (nDirectories>0)
	{
		wprintf(ID2W(IDS_LOCATEVERBOSEDIR));
		for (UINT i=0;i<nDirectories;i++)
			wprintf(L"%s\n",ppDirectories[i]);
	}
	putchar('\n');
}

BOOL CALLBACK LocateFoundProcW(DWORD dwParam,BOOL bFolder,const CLocater* pLocater)
{
	if (nShouldQuit)
		return FALSE;

	if (dwMainFlags&flagOutputIsPaged)
	{
#ifdef WIN32
		if (Lines>int(GetConsoleLines()-1))
#else
		if (Lines>(_farpeekb(_dos_ds,0x484)-1))
#endif
		{
			DWORD nTmp=1;char szbuf[2];
			SetConsoleMode(hStdIn,0);
			ReadConsole(hStdIn,szbuf,1,&nTmp,NULL);
			SetConsoleMode(hStdIn,ENABLE_PROCESSED_INPUT);
			if (szbuf[0]==3) // Ctrl+C
				return FALSE;
			Lines=0;
		}
		Lines++;
	}
	if (bFolder)
		wprintf(L"%s\\%s\n",pLocater->GetCurrentPathW(),pLocater->GetFolderNameW());
	else
		wprintf(L"%s\\%s\n",pLocater->GetCurrentPathW(),pLocater->GetFileNameW());
	return TRUE;
}



int wmain (int argc,wchar_t * argv[])
{
#ifdef _DEBUG
	CAppData::stdfunc();
#endif

#ifdef WIN32
	hStdOut=GetStdHandle(STD_OUTPUT_HANDLE);
	hStdIn=GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(hStdIn,ENABLE_PROCESSED_INPUT);
	SetConsoleCtrlHandler(HandlerRoutine,TRUE);
#endif

	InitLocaterLibrary();

	if (!SetLanguageSpecifigHandles(argv[0]))
	{
#ifdef _DEBUG
		fprintf(stderr,"No language dll file");
#else
		return 1;
#endif
	}

	CArrayFP<CDatabase*> aDatabases;
	struct {
		BYTE helps:3;
		BYTE verbose:1;
	} options={0,0};

	DWORD dwMinSize=DWORD(-1);
	DWORD dwMaxSize=DWORD(-1);
	WORD wMaxDate=WORD(-1);
	WORD wMinDate=WORD(-1);

	DWORD dwMaxFoundFiles=DWORD(-1);
	DWORD dwFlags=LOCATE_FILENAMES|LOCATE_CONTAINTEXTISMATCHCASE;
	DWORD dwExtraFlags=0;
	BYTE* pContainData=NULL;
	SIZE_T dwContainDataLength=0;

	CStringW String;
	CArrayFAP<LPWSTR> aDirectories;
	CArrayFAP<LPWSTR> aExtensions;
	
	int i;
	for (i=1;i<argc;i++)
	{
		if (argv[i][0]==L'-' || argv[i][0]==L'/')
		{
			switch (argv[i][1])
			{
			case L's':
			case L'S':
				dwMainFlags|=flagOutputIsPaged;
				if (argv[i][2]=='c' || argv[i][2]=='C')
	           			dwFlags|=LOCATE_CONTAINTEXTISMATCHCASE;
				break;
			case L'c':
			case L'C':
           		dwFlags|=LOCATE_CONTAINTEXTISMATCHCASE;
                if (argv[i][2]=='s' || argv[i][2]=='S')
					dwMainFlags|=flagOutputIsPaged;
    			break;
			case L'i':
				dwMainFlags|=flagShowWhatAreWeLookingFor;
				break;
            case L'd':
				{
					// Using database file
					CStringW sFile;
					if (argv[i][2]==L'\0')
					{
						if (i>=argc-1)
							sFile=L"";
						else
							sFile=argv[++i];
					}
					else
	                    sFile=(argv[i]+2);
	
					if (CDatabase::FindByFile(aDatabases,sFile)==NULL)
					{
						CDatabase* pDatabase=CDatabase::FromFile(sFile);
						if (pDatabase!=NULL)
							aDatabases.Add(pDatabase);
					}
					break;
				}
			case L'D':
				{
					CStringW sName;
					if (argv[i][2]==L'\0')
					{
						if (i>=argc-1)
							sName=L"";
						else
							sName=argv[++i];
					}
					else
	                    sName=(argv[i]+2);
	
					if (CDatabase::FindByName(aDatabases,sName)==NULL)
					{
						CDatabase* pDatabase=CDatabase::FromName(HKCU,
							"Software\\Update\\Databases",sName);
						if (pDatabase!=NULL)
						{
							pDatabase->SetThreadId(0);
							aDatabases.Add(pDatabase);
						}
					}
					break;
				}
			case L'p':
			case L'P':
                if (argv[i][2]==L'\0')
				{
					if (i>=argc-1)
						aDirectories.Add(allocemptyW());
					else
					{
						++i;
						if (argv[i][0]=='.' && argv[i][1]==L'\0')
						{
							WCHAR* pPath=new WCHAR[MAX_PATH];
							FileSystem::GetCurrentDirectory(MAX_PATH,pPath);
							aDirectories.Add(pPath);
						}
						else
							aDirectories.Add(alloccopy(argv[i]));
					}
				}
                else
				{
					int j=argv[i][2]==L':'?3:2;
					if (argv[i][j]==L'.' && argv[i][j+1]==L'\0')
					{
						WCHAR* pPath=new WCHAR[MAX_PATH];
						FileSystem::GetCurrentDirectory(MAX_PATH,pPath);
						aDirectories.Add(pPath);
					}
					else
						aDirectories.Add(alloccopy(argv[i]+j));
				}
				break;
			case 't':
			case 'T':
                if (argv[i][2]==L'\0')
				{
					if (i>=argc-1)
						aExtensions.Add(allocemptyW());
					else
						aExtensions.Add(alloccopy(argv[++i]));
				}
                else
                    aExtensions.Add(alloccopy(argv[i]+2));
				break;
			case L'r':
				dwFlags|=LOCATE_REGULAREXPRESSION;
				break;
			case L'w':
			case L'W':
				dwFlags|=LOCATE_CHECKWHOLEPATH;
				break;
			case L'v':
				options.verbose=1;
				break;
			case L'V':
				options.helps=2;
				break;
			case L'?':
			case L'h':
			case L'H':
				options.helps=1;
				break;
			case L'R':
				if (argv[i][2]==L'n' || argv[i][2]==L'N')
					dwFlags|=LOCATE_NOSUBDIRECTORIES;
				else
					dwFlags&=~LOCATE_NOSUBDIRECTORIES;
				break;
			case L'l':
			case L'L':
			{
				WCHAR* ep;
            	switch (argv[i][2])
            	{
				case L'w':
					if (argv[i][3]==L'n')
						dwMainFlags&=~flagWholeWord;
					else
						dwMainFlags|=flagWholeWord;
					break;
				case L'r':
					if (argv[i][3]==L'n')
						dwMainFlags&=~flagReplaceSpaces;
					else
						dwMainFlags|=flagReplaceSpaces;
					break;
				case L'm':
            		if (argv[i][3]==L':')
            			dwMinSize=wcstoul(argv[i]+4,&ep,0);
            		else
            			dwMinSize=wcstoul(argv[i]+3,&ep,0);
                    if (*ep==L'k' || *ep==L'K')
                       dwMinSize*=1024;
                    else if (*ep==L'M' || *ep==L'm')
                         dwMinSize*=1024*1024;
	           		break;
            	case L'M':
            		if (argv[i][3]==L':')
            			dwMaxSize=wcstoul(argv[i]+4,&ep,0);
            		else
            			dwMaxSize=wcstoul(argv[i]+3,&ep,0);
                    if (*ep==L'k' || *ep==L'K')
                       dwMaxSize*=1024;
                    else if (*ep==L'M' || *ep==L'm')
                         dwMaxSize*=1024*1024;
	           		break;
            	case L'f':
            		dwFlags&=~LOCATE_FOLDERNAMES;
            		dwFlags|=LOCATE_FILENAMES;
            		if (argv[i][3]==L'd' || argv[i][3]==L'D') 
						dwFlags|=LOCATE_FOLDERNAMES;
            		break;
            	case L'd':                      
            		dwFlags&=~LOCATE_FILENAMES;
            		dwFlags|=LOCATE_FOLDERNAMES;
            		if (argv[i][3]==L'f' || argv[i][3]==L'F')
						dwFlags|=LOCATE_FILENAMES;
            		break;
            	case L'n':
            		if (argv[i][3]==':')
            			dwMaxFoundFiles=_wtol(argv[i]+4);
            		else
            			dwMaxFoundFiles=_wtol(argv[i]+3);
	           		break;
	           	case L'c':
	           	case L'C':
	           		if (argv[i][3]==L'm')
	           			dwFlags|=LOCATE_CONTAINTEXTISMATCHCASE;
            		else if (argv[i][3]==L'n')
	           			dwFlags&=~LOCATE_CONTAINTEXTISMATCHCASE;
            		else if (argv[i][3]==L':')
					{
	            		if (pContainData!=NULL)
							free(pContainData);
						dwFlags&=~LOCATE_REGULAREXPRESSIONSEARCH;
            			pContainData=dataparser(argv[i]+4,&dwContainDataLength);
					}
            		else
					{
	            		if (pContainData!=NULL)
							free(pContainData);
            			dwFlags&=~LOCATE_REGULAREXPRESSIONSEARCH;
            			pContainData=dataparser(argv[i]+3,&dwContainDataLength);
					}
            		break;
	           	case L'R':
	           		{
					
						dwFlags|=LOCATE_REGULAREXPRESSIONSEARCH;
            			if (pContainData!=NULL)
							delete[] pContainData;
					
						int j=argv[i][3]==L':'?4:3;
						if (argv[i][j]==L'\0')
						{
							i++;j=0;
						}

						if (argv[i][j]==L'\"')
						{						
							j++;
							dwContainDataLength=(DWORD)FirstCharIndex(argv[i]+j,L'\"');
							if (dwContainDataLength==DWORD(-1))
								dwContainDataLength=istrlenw(argv[i]+j);
							pContainData=(PBYTE)alloccopy(argv[i]+j,dwContainDataLength);
						}
						else
						{
							dwContainDataLength=(DWORD)istrlenw(argv[i]+j);
							pContainData=(PBYTE)alloccopy(argv[i]+j,dwContainDataLength);
						}
					}
					break;
				case L'D': // dates
				{
					const WCHAR* lpCmdLine=argv[i]+3;

					int nLength=(int)istrlenw(lpCmdLine);
					if (nLength<7)
                        break;
					WCHAR szBuf[]=L"XX";
					szBuf[0]=lpCmdLine[1];
					szBuf[1]=lpCmdLine[2];
					WORD bYear=_wtoi(szBuf);
					if (bYear<60)
						bYear+=2000;
					else
						bYear+=1900;
					bYear-=1980;
					szBuf[0]=lpCmdLine[3];
					szBuf[1]=lpCmdLine[4];
					BYTE bMonth=_wtoi(szBuf);
					if (bMonth<1 || bMonth>12)
						bMonth=1;
					szBuf[0]=lpCmdLine[5];
					szBuf[1]=lpCmdLine[6];
					BYTE bDay=_wtoi(szBuf);
					if (bDay<1 || bDay>CTime::GetDaysInMonth(bMonth,bYear))
						bDay=1;					
					
					if (isupper(lpCmdLine[0])) // max date
					{
						switch (lpCmdLine[0])
						{
						case L'A':
							dwFlags|=LOCATE_MAXACCESSDATE;
							break;
						case L'C':
							dwFlags|=LOCATE_MAXCREATIONDATE;
							break;
						}
						wMaxDate=(BYTE(bYear)<<9)|(bMonth<<5)|(bDay);
					}
					else
					{
						switch (lpCmdLine[0])
						{
						case L'a':
							dwFlags|=LOCATE_MINACCESSDATE;
							break;
						case L'c':
							dwFlags|=LOCATE_MINCREATIONDATE;
							break;
						}
						wMinDate=(bYear<<9)|(bMonth<<5)|(bDay);
					}

					break;
				}
				}
            }
			}
      }
      else
      {
		  if (dwFlags&LOCATE_REGULAREXPRESSION)
		  {
				if (argv[i][0]==L'\"')
				{
					String << (argv[i]+1);
					if (String.LastChar()==L'\"')
						String.DelLastChar();
				}
				else
					String << argv[i];
		  }
		  else
		  {
				if (!String.IsEmpty())
				{
					// Inserting '/' or space 
					if (dwMainFlags&flagReplaceSpaces)
						String << L'*';
					else
						String << L' ';
	
				}
				
				String << argv[i];
		  }
		}
	}

	

   
	if (options.helps==1)
	{
		fprintf(stdout,"%s\n",szVersionStr);
		
		HRSRC hRc=FindResource(GetLanguageSpecificResourceHandle(),MAKEINTRESOURCE(IDR_LOCATEHELP),"HELPTEXT");
		HGLOBAL hGlobal=LoadResource(GetLanguageSpecificResourceHandle(),hRc);
		LPCSTR pStr=(LPCSTR)LockResource(hGlobal);
		
		// Counting length
		int len;
		for (len=0;pStr[len]!='\0';len++)
		{
			if (pStr[len]=='E' && pStr[len+1]=='O' && pStr[len+2]=='F')
				break;
		}


		fwrite(pStr,1,len,stdout);
		
		FreeLibrary(GetLanguageSpecificResourceHandle());
		return 1;
	}
	else if (options.helps==2)
	{
		puts(szVersionStr);

		FreeLibrary(GetLanguageSpecificResourceHandle());
		return 1;
	}

	// Checking databases
	// First, check that there is database 
	if (aDatabases.GetSize()==0)
		CDatabase::LoadFromRegistry(HKCU,"Software\\Update\\Databases",aDatabases);   
	
	// If there is still no any available database, try to load old style db
	if (aDatabases.GetSize()==0)
	{
		CDatabase* pDatabase=CDatabase::FromOldStyleDatabase(HKCU,"Software\\Update\\Database");
		if (pDatabase==NULL)
			pDatabase=CDatabase::FromDefaults(TRUE,argv[0],(int)LastCharIndex(argv[0],'\\')+1); // Nothing else can be done?
		aDatabases.Add(pDatabase);
	}
	CDatabase::CheckValidNames(aDatabases);
	CDatabase::CheckDoubleNames(aDatabases);

	
	if (aExtensions.GetSize()==0)
		dwFlags|=LOCATE_EXTENSIONWITHNAME;

#ifdef _DEBUG
	wprintf (L"flags: %X name: %s\n",dwMainFlags,(LPCWSTR)String);
	Lines++;
#endif
	if (dwMainFlags&flagShowWhatAreWeLookingFor)
	{
		wprintf(ID2W(IDS_LOCATESTRING),(LPCWSTR)String);
		Lines++;
	}
	
   
	CLocater locater(aDatabases);
	locater.SetSizeAndDate(dwFlags,dwMinSize,dwMaxSize,wMinDate,wMaxDate);
	locater.SetAdvanced(dwFlags,pContainData,(DWORD)min(dwContainDataLength,MAXDWORD),dwMaxFoundFiles);

	locater.SetFunctions(LocateProc,LocateFoundProc,LocateFoundProcW,NULL);

	
	if (dwFlags&LOCATE_REGULAREXPRESSION)
	{
		if (options.verbose)
		{
			wprintf(ID2W(IDS_LOCATEVERBOSEREG),(LPCWSTR)String);
			putchar(' ');

			if (aDirectories.GetSize()>0)
			{
				wprintf(ID2W(IDS_LOCATEVERBOSEDIR));
				for (int i=0;i<aDirectories.GetSize();i++)
					printf("%s\n",aDirectories[i]);
			}
			putchar('\n');
		}

		locater.LocateFiles(FALSE,W2A(String),
			(LPCWSTR*)aDirectories.GetData(),aDirectories.GetSize());
	}
	else if (!String.IsEmpty())
	{
		int nIndex=(int)String.FindFirst(',');
		if (nIndex==-1)
		{
			// Inserting '*':s if needed
			if (!(dwMainFlags&flagWholeWord))
			{
				if (String[0]!='*')
					String.InsChar(0,'*');
				if (String.LastChar()!='*')
					String << '*';
			}

			LPCWSTR s=String;

			if (options.verbose)
			{
				showverbose(&s,1,
					(LPCWSTR*)aExtensions.GetData(),aExtensions.GetSize(),
					(LPCWSTR*)aDirectories.GetData(),aDirectories.GetSize());
			}

			locater.LocateFiles(FALSE,&s,1,
				(LPCWSTR*)aExtensions.GetData(),aExtensions.GetSize(),
				(LPCWSTR*)aDirectories.GetData(),aDirectories.GetSize());
		}
		else
		{
			// Separate strings
			
			CArrayFAP<LPWSTR> aStrings;
			LPCWSTR pStr=String;
			BOOL bContinue=TRUE;

			while (bContinue)
			{
				
				if (nIndex==-1)
				{
					bContinue=FALSE;
					nIndex=(int)istrlenw(pStr);
				}

				if (nIndex>0)
				{
					if (dwMainFlags&flagWholeWord)
						aStrings.Add(alloccopy(pStr,nIndex));
					else
					{
						// Inserting '*'
						WCHAR* pTemp=new WCHAR[nIndex+3];
						if (pStr[0]!=L'*')
						{
							pTemp[0]=L'*';
							MemCopyW(pTemp+1,pStr,nIndex);
							if (pStr[nIndex-1]!=L'*')
							{	
								pTemp[nIndex+1]=L'*';
								pTemp[nIndex+2]=L'\0';
							}
							else
								pTemp[nIndex+1]=L'\0';
						}
						else
						{
							MemCopyW(pTemp,pStr,nIndex);
							if (pStr[nIndex-1]!=L'*')
							{	
								pTemp[nIndex]=L'*';
								pTemp[nIndex+1]=L'\0';
							}
							else
								pTemp[nIndex]='\0';
						}
						aStrings.Add(pTemp);


					}

					pStr+=nIndex+1;
					nIndex=(int)FirstCharIndex(pStr,',');
				}
			
			}

			if (options.verbose)
			{
				showverbose((LPCWSTR*)aStrings.GetData(),aStrings.GetSize(),
					(LPCWSTR*)aExtensions.GetData(),aExtensions.GetSize(),
					(LPCWSTR*)aDirectories.GetData(),aDirectories.GetSize());
			}

			locater.LocateFiles(FALSE,(LPCWSTR*)aStrings.GetData(),aStrings.GetSize(),
				(LPCWSTR*)aExtensions.GetData(),aExtensions.GetSize(),
				(LPCWSTR*)aDirectories.GetData(),aDirectories.GetSize());

		}

		
	
	}
	else
	{
		if (options.verbose)
		{
			showverbose(NULL,0,
				(LPCWSTR*)aExtensions.GetData(),aExtensions.GetSize(),
				(LPCWSTR*)aDirectories.GetData(),aDirectories.GetSize());
		}

		locater.LocateFiles(FALSE,NULL,0,
			(LPCWSTR*)aExtensions.GetData(),aExtensions.GetSize(),
			(LPCWSTR*)aDirectories.GetData(),aDirectories.GetSize());
	}


	if (options.verbose)
		wprintf(ID2W(IDS_LOCATEVERBOSEFOUNDFILES),locater.GetNumberOfResults());

	if (pContainData!=NULL)
		delete[] pContainData;

	FreeLibrary(GetLanguageSpecificResourceHandle());
	return 0;
}
