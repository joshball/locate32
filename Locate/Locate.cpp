/* Copyright (c) 1997-2005 Janne Huttunen
   locate.exe v2.99.5.1020                 */

const char* szVersionStr="locate 3.0 beta 5.1020";

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
	flagOutputIsPaged=0x8
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

BOOL SetLanguageSpecifigHandles(LPCSTR szAppPath)
{
	CRegKey RegKey;
	CString LangFile;
	if (RegKey.OpenKey(HKCU,"Software\\Update",
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("Language",LangFile);
		RegKey.CloseKey();
	}
	if (LangFile.IsEmpty())
		LangFile="lan_en.dll";

	CString Path(szAppPath,LastCharIndex(szAppPath,'\\')+1);
	
	HINSTANCE hLib=LoadLibrary(Path+LangFile);
	if (hLib==NULL)
	{
		hLib=LoadLibrary(Path+"lan_en.dll");

		if (hLib==NULL)
		{
			fprintf(stderr,"Cannot load language file '%s'\n",(LPCSTR)LangFile);
			return FALSE;
		}

		fprintf(stderr,"Cannot load language file '%s', using 'lan_en.dll'\n",(LPCSTR)LangFile);
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
    
BOOL CALLBACK LocateProc(DWORD dwParam,CallingReason crReason,UpdateError ue,DWORD dwFoundFiles,const CLocater* pLocater)
{          
	switch (crReason)
	{
	case ErrorOccured:
		switch (ue)
		{
		case ueUnknown:
			fprintf(stderr,(LPCSTR)CString(IDS_LOCATEUNKNOWNERROR));
			break;
		case ueOpen:
			fprintf(stderr,(LPCSTR)CString(IDS_LOCATECANNOTOPEN),pLocater->GetCurrentDatabaseFile());
			break;
		case ueRead:
			fprintf(stderr,(LPCSTR)CString(IDS_LOCATECANNOTREAD),pLocater->GetCurrentDatabaseFile());
			break;
		case ueInvalidDatabase:
			fprintf(stderr,(LPCSTR)CString(IDS_LOCATEINVALIDDATABASE),pLocater->GetCurrentDatabaseFile());
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

int main (int argc,char * argv[])
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

	if (!SetLanguageSpecifigHandles(argv[0]))
		return 1;

	CArrayFP<CDatabase*> aDatabases;
	int helps=0;

	DWORD dwMinSize=DWORD(-1);
	DWORD dwMaxSize=DWORD(-1);
	WORD wMaxDate=WORD(-1);
	WORD wMinDate=WORD(-1);

	DWORD dwMaxFoundFiles=DWORD(-1);
	DWORD dwFlags=LOCATE_FILENAMES|LOCATE_CONTAINTEXTISMATCHCASE;
	BYTE* pContainData=NULL;
	DWORD dwContainDataLength=0;

	CString String;
	CArrayFAP<LPSTR> aDirectories;
	CArrayFAP<LPSTR> aExtensions;
	int i;
	for (i=1;i<argc;i++)
	{
		if (argv[i][0]=='-' || argv[i][0]=='/')
		{
			switch (argv[i][1])
			{
			case 's':
			case 'S':
				dwMainFlags|=flagOutputIsPaged;
				if (argv[i][2]=='c' || argv[i][2]=='C')
	           			dwFlags|=LOCATE_CONTAINTEXTISMATCHCASE;
				break;
			case 'c':
			case 'C':
           		dwFlags|=LOCATE_CONTAINTEXTISMATCHCASE;
                if (argv[i][2]=='s' || argv[i][2]=='S')
					dwMainFlags|=flagOutputIsPaged;
    			break;
			case 'i':
				dwMainFlags|=flagShowWhatAreWeLookingFor;
				break;
            case 'd':
                // Using database file
                LPCSTR szFile;
				if (argv[i][2]=='\0')
                    szFile=argv[++i];
                else
                    szFile=(argv[i]+2);

				if (CDatabase::FindByFile(aDatabases,szFile)==NULL)
				{
					CDatabase* pDatabase=CDatabase::FromFile(szFile);
					if (pDatabase!=NULL)
						aDatabases.Add(pDatabase);
				}
				break;
			case 'D':
				LPCSTR szName;
				if (argv[i][2]=='\0')
                    szName=argv[++i];
                else
                    szName=(argv[i]+2);

				if (CDatabase::FindByName(aDatabases,szName)==NULL)
				{
					CDatabase* pDatabase=CDatabase::FromName(HKCU,
						"Software\\Update\\Databases",szName);
					if (pDatabase!=NULL)
					{
						pDatabase->SetThreadId(0);
						aDatabases.Add(pDatabase);
					}
				}
				break;
			case 'p':
			case 'P':
                if (argv[i][2]=='\0')
                    aDirectories.Add(alloccopy(argv[++i]));
                else
                    aDirectories.Add(alloccopy(argv[i]+2));
				break;
			case 't':
			case 'T':
                if (argv[i][2]=='\0')
                    aExtensions.Add(alloccopy(argv[++i]));
                else
                    aExtensions.Add(alloccopy(argv[i]+2));
				break;
			case 'r':
				dwFlags|=LOCATE_REGULAREXPRESSION;
				break;
			case 'R':
				dwFlags|=LOCATE_REGULAREXPRESSION|LOCATE_REGULAREXPRESSIONINPATH;
				break;
			case 'v':
				helps=2;
				break;
			case '?':
			case 'h':
			case 'H':
				helps=1;
				break;
			case 'l':
			case 'L':
			{
				char* ep;
            	switch (argv[i][2])
            	{
				case 'w':
					if (argv[i][3]=='n')
						dwMainFlags&=~flagWholeWord;
					else
						dwMainFlags|=flagWholeWord;
					break;
				case 'r':
					if (argv[i][3]=='n')
						dwMainFlags&=~flagReplaceSpaces;
					else
						dwMainFlags|=flagReplaceSpaces;
					break;
				case 'm':
            		if (argv[i][3]==':')
            			dwMinSize=strtoul(argv[i]+4,&ep,0);
            		else
            			dwMinSize=strtoul(argv[i]+3,&ep,0);
                    if (*ep=='k' || *ep=='K')
                       dwMinSize*=1024;
                    else if (*ep=='M' || *ep=='m')
                         dwMinSize*=1024*1024;
	           		break;
            	case 'M':
            		if (argv[i][3]==':')
            			dwMaxSize=strtoul(argv[i]+4,&ep,0);
            		else
            			dwMaxSize=strtoul(argv[i]+3,&ep,0);
                    if (*ep=='k' || *ep=='K')
                       dwMaxSize*=1024;
                    else if (*ep=='M' || *ep=='m')
                         dwMaxSize*=1024*1024;
	           		break;
            	case 'f':
            		dwFlags&=~LOCATE_FOLDERNAMES;
            		dwFlags|=LOCATE_FILENAMES;
            		if (argv[i][3]=='d' || argv[i][3]=='D') 
						dwFlags|=LOCATE_FOLDERNAMES;
            		break;
            	case 'd':                      
            		dwFlags&=~LOCATE_FILENAMES;
            		dwFlags|=LOCATE_FOLDERNAMES;
            		if (argv[i][3]=='f' || argv[i][3]=='F')
						dwFlags|=LOCATE_FILENAMES;
            		break;
            	case 'n':
            		if (argv[i][3]==':')
            			dwMaxFoundFiles=atol(argv[i]+4);
            		else
            			dwMaxFoundFiles=atol(argv[i]+3);
	           		break;
	           	case 'c':
	           	case 'C':
	           		if (argv[i][3]=='m')
	           			dwFlags|=LOCATE_CONTAINTEXTISMATCHCASE;
            		else if (argv[i][3]=='n')
	           			dwFlags&=~LOCATE_CONTAINTEXTISMATCHCASE;
            		else if (argv[i][3]==':')
					{
	            		if (pContainData!=NULL)
							delete[] pContainData;
						dwFlags&=~LOCATE_REGULAREXPRESSIONSEARCH;
            			pContainData=dataparser(argv[i]+4,&dwContainDataLength);
					}
            		else
					{
	            		if (pContainData!=NULL)
							delete[] pContainData;
            			dwFlags&=~LOCATE_REGULAREXPRESSIONSEARCH;
            			pContainData=dataparser(argv[i]+3,&dwContainDataLength);
					}
            		break;
	           	case 'R':
	           		{
					
						dwFlags|=LOCATE_REGULAREXPRESSIONSEARCH;
            			if (pContainData!=NULL)
							delete[] pContainData;
					
						int j=argv[i][3]==':'?4:3;
						if (argv[i][j]=='\0')
						{
							i++;j=0;
						}

						if (argv[i][j]=='\"')
						{						
							j++;
							dwContainDataLength=(DWORD)FirstCharIndex(argv[i]+j,'\"');
							if (dwContainDataLength==DWORD(-1))
								dwContainDataLength=istrlen(argv[i]+j);
							pContainData=(PBYTE)alloccopy(argv[i]+j,dwContainDataLength);
						}
						else
						{
							dwContainDataLength=(DWORD)istrlen(argv[i]+j);
							pContainData=(PBYTE)alloccopy(argv[i]+j,dwContainDataLength);
						}
					}
					break;
				case 'D': // dates
				{
					const char* lpCmdLine=argv[i]+3;

					int nLength=strlen(lpCmdLine);
					if (nLength<7)
                        break;
					char szBuf[]="XX";
					szBuf[0]=lpCmdLine[1];
					szBuf[1]=lpCmdLine[2];
					WORD bYear=atoi(szBuf);
					if (bYear<60)
						bYear+=2000;
					else
						bYear+=1900;
					bYear-=1980;
					szBuf[0]=lpCmdLine[3];
					szBuf[1]=lpCmdLine[4];
					BYTE bMonth=atoi(szBuf);
					if (bMonth<1 || bMonth>12)
						bMonth=1;
					szBuf[0]=lpCmdLine[5];
					szBuf[1]=lpCmdLine[6];
					BYTE bDay=atoi(szBuf);
					if (bDay<1 || bDay>CTime::GetDaysInMonth(bMonth,bYear))
						bDay=1;					
					
					if (isupper(lpCmdLine[0])) // max date
					{
						switch (lpCmdLine[0])
						{
						case 'A':
							dwFlags|=LOCATE_MAXACCESSDATE;
							break;
						case 'C':
							dwFlags|=LOCATE_MAXCREATIONDATE;
							break;
						}
						wMaxDate=(BYTE(bYear)<<9)|(bMonth<<5)|(bDay);
					}
					else
					{
						switch (lpCmdLine[0])
						{
						case 'a':
							dwFlags|=LOCATE_MINACCESSDATE;
							break;
						case 'c':
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
				if (argv[i][0]=='\"')
				{
					String << (argv[i]+1);
					if (String.LastChar()=='\"')
						String.DelLastChar();
				}
				else
					String << argv[i];
		  }
		  else
		  {
				if (!String.IsEmpty())
				{
					if (dwMainFlags&flagReplaceSpaces)
						String << '*';
					else
						String << ' ';
	
				}
				else if (!(dwMainFlags&flagWholeWord))
					String << '*';
		  
				String << argv[i];
		  }
		}
	}

	if (!(dwFlags&LOCATE_REGULAREXPRESSION))
	{
		if (!(dwMainFlags&flagWholeWord) && String.LastChar()!='*' && !String.IsEmpty())
			String << '*';
	}

   
	if (helps==1)
	{
		fprintf(stderr,"%s\n",szVersionStr);
		
		HRSRC hRc=FindResource(GetLanguageSpecificResourceHandle(),MAKEINTRESOURCE(IDR_LOCATEHELP),"HELPTEXT");
		HGLOBAL hGlobal=LoadResource(GetLanguageSpecificResourceHandle(),hRc);
		fprintf(stderr,(LPCSTR)LockResource(hGlobal));
		
		return 1;
	}
	else if (helps==2)
	{
		puts(szVersionStr);
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
			pDatabase=CDatabase::FromDefaults(TRUE,argv[0],LastCharIndex(argv[0],'\\')+1); // Nothing else can be done?
		aDatabases.Add(pDatabase);
	}
	CDatabase::CheckValidNames(aDatabases);
	CDatabase::CheckDoubleNames(aDatabases);

	
	if (aExtensions.GetSize()==0)
		dwFlags|=LOCATE_EXTENSIONWITHNAME;

#ifdef _DEBUG
	printf ("flags: %X name: %s\n",dwMainFlags,(LPCSTR)String);
	Lines++;
#endif
	if (dwMainFlags&flagShowWhatAreWeLookingFor)
	{
		printf (CString(IDS_LOCATESTRING),(LPCSTR)String);
		Lines++;
	}
	
   
	CLocater locater(aDatabases);
	locater.SetSizeAndDate(dwFlags,dwMinSize,dwMaxSize,wMinDate,wMaxDate);
	locater.SetAdvanced(dwFlags,pContainData,dwContainDataLength,dwMaxFoundFiles);

	locater.SetFunctions(LocateProc,LocateFoundProc,NULL);

	if (dwFlags&LOCATE_REGULAREXPRESSION)
	{
		locater.LocateFiles(FALSE,String,(dwFlags&LOCATE_REGULAREXPRESSIONINPATH)?TRUE:FALSE,
			(LPCSTR*)aDirectories.GetData(),aDirectories.GetSize());
	}
	else
	{
		locater.LocateFiles(FALSE,String,
			(LPCSTR*)aExtensions.GetData(),aExtensions.GetSize(),
			(LPCSTR*)aDirectories.GetData(),aDirectories.GetSize());
	}


	if (pContainData!=NULL)
		delete[] pContainData;
	return 0;
}
