/* Copyright (c) 1997-2003 Janne Huttunen
   Updatedb.exe v2.98.4.9200                    */

#include <HFCLib.h>
#include "locatedb/locatedb.h"

#include "lan_resources.h"

#ifdef WIN32
		LPCSTR szVersionStr="updtdb32 v2.98.4.9200";
#else
		LPCSTR szVersionStr="updatedb v2.98.4.9200";
#endif


#if !defined(LANG_FI) && !defined(LANG_EN)
#define LANG_EN
#endif

CDatabaseUpdater** ppUpdaters=NULL;

BOOL nQuiet=FALSE;

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
			fprintf(stderr,"Cannot load language file '%s'\n",(LPCSTR)Path);
			return FALSE;
		}

		fprintf(stderr,"Cannot load language file '%s', using 'lan_en.dll'\n",(LPCSTR)LangFile);
	}

	SetResourceHandle(hLib,LanguageSpecificResource);
	return TRUE;
}

BOOL CALLBACK UpdateProc(DWORD dwParam,CallingReason crReason,UpdateError ueCode,CDatabaseUpdater* pUpdater)
{
	switch (crReason)
	{
	case StartedDatabase:
		if (!nQuiet)
		{
			if (strncmp(pUpdater->GetCurrentDatabaseName(),"PARAMX",6)==0 ||
				strncmp(pUpdater->GetCurrentDatabaseName(),"DEFAULTX",8)==0)
			    printf(CString(IDS_UPDATEDB32UPDATINGDATABASE2),pUpdater->GetCurrentDatabaseFile());
			else
			{
                printf(CString(IDS_UPDATEDB32UPDATINGDATABASE),
					pUpdater->GetCurrentDatabaseName(),pUpdater->GetCurrentDatabaseFile());
			}
		}
		break;
	case RootChanged:
		if (!nQuiet)
		{
			if (pUpdater->GetCurrentRoot()!=NULL)
				printf(CString(IDS_UPDATEDB32SCANNING),(LPCSTR)pUpdater->GetCurrentRoot()->m_Path);
			else	
				printf(CString(IDS_UPDATEDB32WRITINGDB));
		}
		break;
	case ErrorOccured:
		if (!nQuiet)
		{	
			switch (ueCode)
			{
			case ueAlloc:
				fprintf(stderr,CString(IDS_UPDATEDB32ALLOCATEMEM));
				break;
			case ueOpen:
			case ueCreate:
				fprintf(stderr,CString(IDS_UPDATEDB32CANNOTCREATEFILE),(LPCSTR)pUpdater->GetCurrentDatabaseFile());
				break;
			case ueWrite:
				fprintf(stderr,CString(IDS_UPDATEDB32CANNOTWRITE),(LPCSTR)pUpdater->GetCurrentDatabaseFile());
				break;
			case ueUnknown:
				fprintf(stderr,CString(IDS_UPDATEDB32ALLOCATEMEM));
				break;
			case ueFolderUnavailable:
				fprintf(stderr,CString(IDS_UPDATEDB32ROOTUNAVAILABLE),
					pUpdater->GetCurrentRootPath()!=NULL?pUpdater->GetCurrentRootPath():"(NULL)");
				break;
			}
		}
		if (ueCode==ueSuccess)
			return 0;
		break;
	case ClassShouldDelete:
		{
			DWORD dwRunning=0;
			if (ppUpdaters==NULL)
				return TRUE; // One thread mode

			for (int i=0;ppUpdaters[i]!=NULL;i++)
			{
				if (ppUpdaters[i]==pUpdater)
					ppUpdaters[i]=UPDATER_EXITED(ueCode);
				else if (!IS_UPDATER_EXITED(ppUpdaters[i]))
					dwRunning++;
			}
            delete pUpdater;
			
			if (dwRunning==0)
			{
				delete[] ppUpdaters;
				ppUpdaters=NULL;
			}
			break;
		}
	}
	return TRUE;
}

/*
void GetFromDB(LPCSTR szOwnName,CString* pDatabaseFile,CArrayFAP<LPSTR>* aRoots,CString* pCreator,CString* pDescription)
{
   	CString Temp;

#ifdef WIN32
   	CRegKey RegKey;
	CDatabaseUpdater::GetDBSettings(aRoots,pCreator,pDescription,pDatabaseFile);
#else
	// Retrieving ini file name
	char szIniFile[_MAX_PATH]; 
	int i;
	dstrlen(szOwnName,i);
	for (i--;i>=0 && szOwnName[i]!='\\' && szOwnName[i]!='/' ;i--);
	sMemCopy(szIniFile,szOwnName,i);
	sMemCopy(szIniFile+i,"\\locate.ini",12);
	CDatabaseUpdater::GetDBSettings(szIniFile,aRoots,pCreator,pDescription,pDatabaseFile);
#endif
	
	if (pDatabaseFile!=NULL)
    {
        if (!CFile::IsValidFileName(*pDatabaseFile))
		{
    		pDatabaseFile->Copy(szOwnName,LastCharIndex(szOwnName,'\\')+1);
			*pDatabaseFile<<"Files.dbs";
    	}
    }
}
*/


int main (int argc,char ** argv)
{
	CAppData::stdfunc();
	
	if (!SetLanguageSpecifigHandles(argv[0]))
		return 1;

	CArrayFP<CDatabase*> aDatabases;
	
	WORD wCurrentThread=0;

	aDatabases.Add(CDatabase::FromDefaults(TRUE,argv[0],LastCharIndex(argv[0],'\\')+1));
	aDatabases[0]->SetNamePtr(alloccopy("DEFAULTX"));
	aDatabases[0]->SetThreadId(wCurrentThread);

    int i,helps=0;
#ifndef WIN32
    if (getenv("TZ")==NULL)
		fprintf(stderr,"Timezone is not set. Database may contain invalid file times.\nFor example type "SET TZ=GMT+1" for central european time.\n");
#endif
    for (i=1;i<argc;i++)
    {
        if (argv[i][0]=='-' || argv[i][0]=='/')
        {
            switch (argv[i][1])
			{
			case 'l':
			case 'L':
				if (strncmp(aDatabases.GetLast()->GetName(),"PARAMX",6)!=0 && 
					strncmp(aDatabases.GetLast()->GetName(),"DEFAULTX",8)!=0)
					printf(CString(IDS_UPDATEDB32CANNOTCHANGELOADED),aDatabases.GetLast()->GetName());
				else if (argv[i][2]=='1')
					aDatabases.GetLast()->AddLocalRoots();
				else 
				{
					CString* pStr;
					if (argv[i][2]=='\0' && i+1<argc)
						pStr=new CString(argv[++i]);
					else
						pStr=new CString(argv[i]+2);
					
					if ((*pStr)[0]=='\"')
						pStr->DelChar(0);
					if (pStr->LastChar()=='\"')
						pStr->DelLastChar();
					while (pStr->LastChar()=='\\')
						pStr->DelLastChar();
					
					if (pStr->GetLength()>1)
					{
						if ((*pStr)[1]==':' && pStr->GetLength()==2)
							aDatabases.GetLast()->AddRoot(alloccopy(*pStr));
						else if (CFile::IsDirectory(*pStr))
							aDatabases.GetLast()->AddRoot(alloccopy(*pStr));
						else
							fprintf(stderr,CString(IDS_UPDATEDB32DIRECTORYISNOTVALID),(LPCSTR)*pStr);
					}
					else
						fprintf(stderr,CString(IDS_UPDATEDB32DIRECTORYISNOTVALID),(LPCSTR)*pStr);
					delete pStr;
				}
				break;
			case 'e':
			case 'E':
				if (strncmp(aDatabases.GetLast()->GetName(),"PARAMX",6)!=0 && 
					strncmp(aDatabases.GetLast()->GetName(),"DEFAULTX",8)!=0)
					printf(CString(IDS_UPDATEDB32CANNOTCHANGELOADED),aDatabases.GetLast()->GetName());
				else 
				{
					CString* pStr;
					if (argv[i][2]=='\0' && i+1<argc)
						pStr=new CString(argv[++i]);
					else
						pStr=new CString(argv[i]+2);
					
					if ((*pStr)[0]=='\"')
						pStr->DelChar(0);
					if (pStr->LastChar()=='\"')
						pStr->DelLastChar();
					while (pStr->LastChar()=='\\')
						pStr->DelLastChar();
					
					if (pStr->GetLength()>1)
					{
						if (!aDatabases.GetLast()->AddExcludedDirectory(*pStr))
							fprintf(stderr,CString(IDS_UPDATEDB32DIRECTORYISNOTVALID),(LPCSTR)*pStr);
					}
					else
						fprintf(stderr,CString(IDS_UPDATEDB32DIRECTORYISNOTVALID),(LPCSTR)*pStr);
					delete pStr;
				}
				break;
			case 't':
			case 'T':
				if (strncmp(aDatabases.GetLast()->GetName(),"PARAMX",6)!=0 &&
					strncmp(aDatabases.GetLast()->GetName(),"DEFAULTX",8)!=0)
					printf(CString(IDS_UPDATEDB32CANNOTCHANGELOADED),aDatabases.GetLast()->GetName());
				else if (argv[i][2]=='c' || argv[i][2]=='C')
				{
                       if (argv[i][3]=='\0')
                           aDatabases.GetLast()->SetCreatorPtr(alloccopy(argv[++i]));
                       else
                           aDatabases.GetLast()->SetCreatorPtr(alloccopy(argv[i]+2));
				}
				else if (argv[i][2]=='d' || argv[i][2]=='D')
				{
                       if (argv[i][3]=='\0')
                           aDatabases.GetLast()->SetDescriptionPtr(alloccopy(argv[++i]));
                       else
                           aDatabases.GetLast()->SetDescriptionPtr(alloccopy(argv[i]+2));
				}
				break;
			case 'N':
			case 'n':
				wCurrentThread++;
				break;
			case 'q':
			case 'Q':
				nQuiet=TRUE;
				break;
			case 'v':
			case 'V':
				printf("%s\n",szVersionStr);
				return 0;
			case 'd':
				{
					// Using database file
					LPCSTR szFile;
					if (argv[i][2]=='\0')
						szFile=argv[++i];
					else
						szFile=(argv[i]+2);
					
					if (aDatabases.GetSize()==1 && strcmp(aDatabases[0]->GetName(),"DEFAULTX")==0)
					{
						aDatabases[0]->SetNamePtr(alloccopy("PARAMX"));
						aDatabases[0]->SetArchiveNamePtr(alloccopy(szFile));
					}
					else if (CDatabase::FindByFile(aDatabases,szFile)==NULL)
					{
						CDatabase* pDatabase=CDatabase::FromFile(szFile);
						if (pDatabase!=NULL)
						{
                            aDatabases.Add(pDatabase);
							pDatabase->SetNamePtr(alloccopy("PARAMX"));
							pDatabase->SetThreadId(wCurrentThread);
						}
					}
			
				}
				break;
			case 'D':
				{
					// Loading database 'name' from registry, cannot be changed 
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
							// Is only default loaded
							if (aDatabases.GetSize()==1 && strcmp(aDatabases[0]->GetName(),"DEFAULTX")==0)
							{
								delete aDatabases[0];
								aDatabases[0]=pDatabase;
							}
							else
							{
								aDatabases.Add(pDatabase);
								pDatabase->SetThreadId(wCurrentThread);
							}
						}
					}
				}
				break;
			case 'h':
			case 'H':
			case '?':
				helps=1;
				break;
			default:
				helps=1;
				break;
			}
       }
    }
    if (helps==1)
    {
#ifdef WIN32
		fprintf(stderr,CString(IDS_UPDATEDB32USAGE),szVersionStr);
#else
        fprintf(stderr,"%s\nusage updatedb",szVersionStr);
#endif

		fprintf(stderr,CString(IDS_UPDATEDB32USAGE2));
		return 1;
    }

	// Checking databases
	// First, check that there is database 
	if (aDatabases.GetSize()==0)
		CDatabase::LoadFromRegistry(HKCU,"Software\\Update\\Databases",aDatabases);   
	
	CDatabase::CheckValidNames(aDatabases);
	CDatabase::CheckDoubleNames(aDatabases);
	
	
	for (int i=0;i<aDatabases.GetSize();)
	{
		if (!aDatabases[i]->IsGloballyUpdated())
			aDatabases.RemoveAt(i);
		else
			i++;
	}

	// Starting to update
	WORD dwTheads=CDatabase::CheckIDs(aDatabases);
    if (dwTheads==0)
		return FALSE;
	if (dwTheads==1)
	{
		CDatabaseUpdater Updater(aDatabases,aDatabases.GetSize(),UpdateProc);
		Updater.Update(FALSE);
	}
	else
	{
		ppUpdaters=new CDatabaseUpdater*[dwTheads+1];

		for (WORD wThread=0;wThread<dwTheads;wThread++)
		{
			ppUpdaters[wThread]=new CDatabaseUpdater(aDatabases,aDatabases.GetSize(),							
				UpdateProc,wThread,(DWORD)0);
		}
		ppUpdaters[dwTheads]=NULL;

		// Starting
		for (wThread=0;wThread<dwTheads;wThread++)
			ppUpdaters[wThread]->Update(TRUE);
		
		while (ppUpdaters!=NULL)
			Sleep(100);
	}
	return 1;
}


