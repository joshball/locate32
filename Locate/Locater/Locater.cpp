/* Copyright (c) 1997-2005 Janne Huttunen
   database locater v2.99.5.10220                 */

#include <HFCLib.h>

// Including pcre
#include <pcre.h>
#ifdef HFC_MTLIBS
#pragma comment(lib, "libpcremt.lib")
#else
#pragma comment(lib, "libpcre.lib")
#endif
/*extern "C" void *(*pcre_malloc)(size_t);
extern "C" void  (*pcre_free)(void *);
extern "C" void *(*pcre_stack_malloc)(size_t);
extern "C" void  (*pcre_stack_free)(void *);
extern "C" int   (*pcre_callout)(pcre_callout_block *);*/




#include "Locater.h"

#include "../locatedb/locatedb.h"


#define OVECCOUNT 30  /* should be a multiple of 3 */


void InitLocaterLibrary()
{

	pcre_malloc = malloc;
	pcre_free = free;
	pcre_stack_malloc = malloc;
	pcre_stack_free = free;
	pcre_callout = NULL;
}

class CSearchRegExpFromFile : public CSearchFromFile
{
public:
	CSearchRegExpFromFile(LPCSTR szRegularExpression,DWORD dwLength=DWORD(-1));
	virtual ~CSearchRegExpFromFile();
	
	virtual BOOL Search(LPCSTR szFile); // if return value is TRUE if any found
	virtual DWORD GetFoundPosition() const;
	
public:
	virtual void OpenFile(LPCSTR szFile);
	virtual void CloseFile();

private:
	pcre* pRegExp;
	pcre_extra* pRegExtra;
};

CSearchRegExpFromFile::CSearchRegExpFromFile(LPCSTR szRegularExpression,DWORD dwLength)
:	CSearchFromFile(),pRegExp(NULL),pRegExtra(NULL)
{
	const char *error;
	int erroffset;

	if (dwLength==DWORD(-1))
		pRegExp=pcre_compile(szRegularExpression,0,&error,&erroffset,NULL);
	else
	{
		char* pText=new char[dwLength+1];
		sMemCopy(pText,szRegularExpression,dwLength);
		pText[dwLength]='\0';
		pRegExp=pcre_compile(pText,0,&error,&erroffset,NULL);
		delete[] pText;
	}


	if (pRegExp==NULL)
	{
		DebugFormatMessage("pcre_compile returns error: %s",error);
#ifdef _CONSOLE
		fprintf(stderr,"pcre_compile returns error: %s\n",error);
#endif
		return;
	}

	pRegExtra=pcre_study(pRegExp,0,&error); 
}

CSearchRegExpFromFile::~CSearchRegExpFromFile()
{
	if (pRegExp!=NULL)
	{
		free(pRegExp);
		pRegExp=NULL;
	}
	if (pRegExtra!=NULL)
	{
		free(pRegExtra);
		pRegExtra=NULL;
	}

	CloseFile();
}
	
BOOL CSearchRegExpFromFile::Search(LPCSTR szFile)
{
	int offsets[10];
	char buffer[BUFSIZ];
	
	if (hFile==NULL)
	{
		OpenFile(szFile);
		if (hFile==NULL)
			return FALSE;
	}

	while (fgets(buffer, sizeof(buffer), (FILE*)hFile) != NULL)
	{
		int length = istrlen(buffer);
		
		// removing '\n' and '\r'
		if (length > 0 && (buffer[length-1] == '\n' || buffer[length-1] == '\r'))  
			buffer[--length] = '\0';
		
		if (pcre_exec(pRegExp, pRegExtra, buffer, length, 0, 0,	offsets, 10) >= 0)
			return TRUE;
	}

	return FALSE;
}

DWORD CSearchRegExpFromFile::GetFoundPosition() const
{
	return 0;
}

void CSearchRegExpFromFile::CloseFile()
{
	if (hFile!=NULL)
	{
		fclose((FILE*)hFile);
		hFile=NULL;
	}
}

void CSearchRegExpFromFile::OpenFile(LPCSTR szFile)
{
	if (hFile!=NULL)
		fclose((FILE*)hFile);

	if (fopen_s((FILE**)&hFile,szFile,"rt")!=0)
		hFile=NULL;

}

CLocater::~CLocater()
{
	LocaterDebugMessage("CLocater::~CLocater() BEGIN");


	m_aDatabases.RemoveAll();
	m_aDirectories.RemoveAll();

	if (dbFile!=NULL)
	{
		delete dbFile;
		dbFile=NULL;
	}

	if (m_dwFlags&LOCATE_REGULAREXPRESSION)
	{
		if (m_regexp!=NULL)
		{
			free(m_regexp);
			m_regexp=NULL;
		}
		if (m_regextra!=NULL)
		{
			free(m_regextra);
			m_regextra=NULL;
		}
	}
	else
	{
		m_aNames.RemoveAll();
		m_aExtensions.RemoveAll();
	}


#ifdef WIN32
	if (m_hThread!=NULL)
	{
		CloseHandle(m_hThread);
		m_hThread=NULL;
	}
#endif

	if (m_pContentSearcher!=NULL)
		delete m_pContentSearcher;



	LocaterDebugMessage("CLocater::~CLocater() END");
}

BOOL CLocater::LocatingProc()
{
	LocaterDebugMessage("CLocater::LocatingProc() BEGIN");
	
	// Initializing
	UpdateError ueResult=ueStillWorking;
	
#ifdef WIN32
	InterlockedExchange(&m_lForceQuit,FALSE);
#endif
	m_dwFoundFiles=0;
	m_dwFoundDirectories=0;

	m_pProc(m_dwData,Initializing,ueStillWorking,0,this);
	DWORD nPathLen;

	for (int i=0;i<m_aDirectories.GetSize();i++)
		m_aDirectories.GetAt(i)->MakeLower();

	// Opening database file
	
	szBuffer=NULL;
	ASSERT(dbFile==NULL);
	
	
	for (int i=0;i<m_aDatabases.GetSize() && ueResult==ueStillWorking;i++)
	{
		LocaterDebugMessage("CLocater::LocatingProc() DBSTART");
	
		if (!m_aDatabases[i]->bEnable)
			continue;
		
		m_pCurrentDatabase=m_aDatabases[i];
		m_wCurrentDatabaseID=m_pCurrentDatabase->wID;
		m_wCurrentRootIndex=0;
        m_pCurrentFoundProc=m_pCurrentDatabase->m_pFoundProc;

		ueResult=ueStillWorking;
		m_pProc(m_dwData,BeginningDatabase,ueStillWorking,(DWORD)m_pCurrentDatabase->szName,this);


		
		try 
		{
			LocaterDebugMessage("CLocater::LocatingProc() OPENINGDB");
	
			// Opening file
			switch (m_pCurrentDatabase->nArchiveType)
			{
			case CDatabase::archiveFile:
				dbFile=new CFile(m_pCurrentDatabase->szArchive,CFile::defRead,TRUE);
				break;
			default:
				throw CFileException(CFileException::notImplemented,
					-1,m_pCurrentDatabase->szArchive);
			}

			LocaterDebugMessage("CLocater::LocatingProc() READINGDB");
	
			// Reading and verifing header
			szBuffer=new BYTE[11];
			dbFile->Read(szBuffer,11);

			if (szBuffer[0]!='L' || szBuffer[1]!='O' || 
				szBuffer[2]!='C' || szBuffer[3]!='A' || 
				szBuffer[4]!='T' || szBuffer[5]!='E' || 
				szBuffer[6]!='D' || szBuffer[7]!='B' ||
				szBuffer[8]!='2' || szBuffer[9]!='0' )
				throw CFileException(CFileException::invalidFile,
	#ifdef WIN32
				-1,
	#endif
				m_pCurrentDatabase->szArchive);

			// Setting OEM info
			m_pCurrentDatabase->bOEM=!(szBuffer[10]&0x10);

			// Skipping database header
			DWORD dwBlockSize;
			dbFile->Read(dwBlockSize);
			dbFile->Seek(dwBlockSize,CFile::current);
			
			
			
			// Reading root data
			LocaterDebugMessage("CLocater::LocatingProc() READINGROOTDATA");
			
			if (m_aDirectories.GetSize()>0)
			{
				// There is directories in which files should be

				dbFile->Read(dwBlockSize);

				LocaterDebugMessage("CLocater::LocatingProc() block size readed A");

				while (dwBlockSize>0
	#ifdef WIN32
					&& !m_lForceQuit
	#endif
				)
				{
					LocaterDebugMessage("CLocater::LocatingProc() new root");

					m_pProc(m_dwData,RootChanged,ueStillWorking,0,this);
					
					// Reading type data
					LocaterDebugMessage("CLocater::LocatingProc() read block");
					dbFile->Read(m_bCurrentRootType);
					LocaterDebugMessage("CLocater::LocatingProc() block readed");
			
					
					// Reading path
					{
						BYTE cTemp;
						dbFile->Read(cTemp);
						for (nPathLen=0;cTemp!='\0';nPathLen++)
						{
							szCurrentPath[nPathLen]=cTemp;
							szCurrentPathLower[nPathLen]=cTemp;
							dbFile->Read(cTemp);
						}
						szCurrentPath[nPathLen]='\0';
						szCurrentPathLower[nPathLen]='\0';
	#ifdef WIN32
						CharLower(szCurrentPathLower);
	#else
						strlwr(szCurrentPathLower);
	#endif
						dwBlockSize-=nPathLen+1+1; // second 1 is type
					}
					
					ValidType vtType=IsRootValid(nPathLen);
					LocaterDebugNumMessage("CLocater::LocatingProc() vtType=%X",DWORD(vtType));
					
					if (vtType!=NoValidFolders)
					{
						// Reading data to buffer
						delete[] szBuffer;
						pPoint=szBuffer=new BYTE[dwBlockSize];
						dbFile->Read(szBuffer,dwBlockSize);

						// Resolving volume name 
						m_szVolumeName=(LPCSTR)pPoint;
						for (;*pPoint!='\0';pPoint++);
						pPoint++;
					
						// Resolving volume serial
						m_dwVolumeSerial=*((DWORD*)pPoint);
						pPoint+=sizeof(DWORD); // 1 == '\0' in volumename
					
						// Resolving file system
						m_szFileSystem=(LPCSTR)pPoint;
						for (;*pPoint!='\0';pPoint++);
						pPoint++;

						// Skipping the number of files and directories
						pPoint+=2*sizeof(DWORD);

						// Telling that volume information is available
						m_pProc(m_dwData,RootInformationAvail,ueStillWorking,0,this);
					
						// OK, now we are beginning of folder data
						if (vtType==ValidFolders)
							CheckFolder(nPathLen);
						else
							LocateValidFolder(nPathLen);
					
					}
					else // Skipping root
						dbFile->Seek(dwBlockSize,CFile::current);
				
					LocaterDebugMessage("CLocater::LocatingProc() reading next root");
				
					// New root
					m_wCurrentRootIndex++;
					dbFile->Read(dwBlockSize);
				}
			}
			else
			{
				// No restrinctions about directories
				dbFile->Read(dwBlockSize);

				LocaterDebugMessage("CLocater::LocatingProc() block size readed B");
				
				while (dwBlockSize>0
	#ifdef WIN32
					&& !m_lForceQuit
	#endif
				)
				{
					LocaterDebugMessage("CLocater::LocatingProc() 1");

					m_pProc(m_dwData,RootChanged,ueStillWorking,0,this);
				
					// Reading type data
					dbFile->Read(m_bCurrentRootType);
								
					// Reading path
					{
						BYTE cTemp;
						dbFile->Read(cTemp);
						for (nPathLen=0;cTemp!='\0';nPathLen++)
						{
							szCurrentPath[nPathLen]=cTemp;
							dbFile->Read(cTemp);
						}
						szCurrentPath[nPathLen]='\0';
						
						dwBlockSize-=nPathLen+1+1; // second 1 is type
					}
					
					// Reading data to buffer
					delete[] szBuffer;
					pPoint=szBuffer=new BYTE[dwBlockSize];
					dbFile->Read(szBuffer,dwBlockSize);

					LocaterDebugMessage("CLocater::LocatingProc() 2");

					// Resolving volume name 
					m_szVolumeName=(LPCSTR)pPoint;
					for (;*pPoint!='\0';pPoint++);
					pPoint++;
					
					// Resolving volume serial
					m_dwVolumeSerial=*((DWORD*)pPoint);
					pPoint+=sizeof(DWORD); // 1 == '\0' in volumename
					
					// Resolving file system
					m_szFileSystem=(LPCSTR)pPoint;
					for (;*pPoint!='\0';pPoint++);
					pPoint++;

					LocaterDebugMessage("CLocater::LocatingProc() 3");
					
					// Skipping the number of files and directories
					pPoint+=2*sizeof(DWORD);

					// Telling that volume information is available
					m_pProc(m_dwData,RootInformationAvail,ueStillWorking,0,this);
					
					// OK, now we are at the beginning of folder data
					CheckFolder(nPathLen);
					
					LocaterDebugMessage("CLocater::LocatingProc() reading next root");

					// New root
					m_wCurrentRootIndex++;
					dbFile->Read(dwBlockSize);
				}
			}

			LocaterDebugMessage("CLocater::LocatingProc() DBEND");
			
			delete dbFile;
			dbFile=NULL;
		}

		catch (CFileException ex)
		{
			switch (ex.m_cause)
			{
			case CFileException::fileOpen:
			case CFileException::badPath:
			case CFileException::fileNotFound:
#ifdef _DEBUG
				if (m_pCurrentDatabase!=NULL)
					DebugFormatMessage("FILEOPEN/BADPATH/NOTFOUND: %s",m_pCurrentDatabase->szArchive);
#endif
				m_pProc(m_dwData,ErrorOccured,ueResult=ueOpen,0,this);
				break;
			case CFileException::readFault:
			case CFileException::fileCorrupt:
			case CFileException::sharingViolation:
			case CFileException::lockViolation:
			case CFileException::accessDenied:
#ifdef _DEBUG
				if (m_pCurrentDatabase!=NULL)
					DebugFormatMessage("READFAULT/CORRUPT/SHARING/LOCK: %s",m_pCurrentDatabase->szArchive);
#endif
				m_pProc(m_dwData,ErrorOccured,ueResult=ueRead,0,this);
				break;
			case CFileException::endOfFile:
			case CFileException::badSeek:
			case CFileException::invalidFile:
#ifdef _DEBUG
				if (m_pCurrentDatabase!=NULL)
					DebugFormatMessage("EOF/BADSEEK/INVALID: %s",m_pCurrentDatabase->szArchive);
#endif
				m_pProc(m_dwData,ErrorOccured,ueResult=ueInvalidDatabase,0,this);
				break;
			default:
#ifdef _DEBUG
				if (m_pCurrentDatabase!=NULL)
					DebugFormatMessage("UNKNOWN: %s",m_pCurrentDatabase->szArchive);
#endif
				m_pProc(m_dwData,ErrorOccured,ueResult=ueInvalidDatabase,0,this);
				m_pProc(m_dwData,ErrorOccured,ueResult=ueUnknown,0,this);
				break;
			}
		}
		catch (CException ex)
		{
			switch (ex.m_cause)
			{
			case CException::cannotAllocate:
				m_pProc(m_dwData,ErrorOccured,ueResult=ueAlloc,0,this);
				break;
			case CException::none:
				// No error, LocateFoundProc returned FALSE
				break;
			default:
				m_pProc(m_dwData,ErrorOccured,ueResult=ueUnknown,0,this);
				break;
			}
		}
		catch (UpdateError ue)
		{
			if (ue==ueLimitReached)
				ueResult=ueLimitReached;
			else if (ue!=ueSuccess && ue!=ueStillWorking && ue!=ueFolderUnavailable)
				m_pProc(m_dwData,ErrorOccured,ueResult=ue,0,this);
		}
		catch (...)
		{
			m_pProc(m_dwData,ErrorOccured,ueResult=ueUnknown,0,this);
		}
		
		
		m_pProc(m_dwData,FinishedDatabase,ueResult,(DWORD)m_pCurrentDatabase->szName,this);

		// Finishing
		if (szBuffer!=NULL)
			delete[] szBuffer;
		szBuffer=NULL;
	}
	
	
	if (dbFile!=NULL)
	{
		delete dbFile;
		dbFile=NULL;
	}
	
	m_pProc(m_dwData,FinishedLocating,ueResult,0,this);
	m_pProc(m_dwData,ClassShouldDelete,ueResult,0,this);
	
	// This class is deleted in the previous call, do not access this
	
	LocaterDebugMessage("CLocater::LocatingProc() END");
	return TRUE;
}

DWORD WINAPI CLocater::LocateThreadProc(LPVOID lpParameter)
{
	return ((CLocater*)lpParameter)->LocatingProc();
}

BOOL _ContainString(LPCSTR s1,LPCSTR s2) // Is s2 in the s1
{
#ifdef _DEBUG
	LPCSTR orig1=s1,orig2=s2;
#endif 

    BOOL bBreakIfNotMatch;
	if (s2[0]=='*')
	{
		if (s2[1]=='\0')
			return TRUE;
		s2++;
		bBreakIfNotMatch=FALSE;
	}
	else
		bBreakIfNotMatch=TRUE;

	while (*s1!='\0')
	{
		for (int i=0;;i++)
		{
			// is s1 too short?
			if (s1[i]=='\0')
			{
				if (s2[i]=='\0')
					return TRUE;
				return s2[i]=='*' && s2[i+1]=='\0';
			}
			// string differ
			if (s1[i]!=s2[i])
			{
				if (s2[i]=='?')
					continue;
				
				if (s2[i]=='*')
				{
					if (s2[i+1]=='\0')
						return TRUE;
					s2+=i+1;
					s1+=i-1;
					bBreakIfNotMatch=FALSE;
					break;
				}
				break;
			}
		}
		if (bBreakIfNotMatch)
			return FALSE;
		s1++;
	}
	return FALSE;
}

inline BOOL CLocater::SetDirectoriesAndStartToLocate(BOOL bThreaded,LPCSTR* szDirectories,DWORD nDirectories)
{
	for (DWORD i=0;i<nDirectories;i++)
	{
		CString* pTmp=new CString(szDirectories[i]);
		while (pTmp->LastChar()=='\\')
			pTmp->DelChar(pTmp->GetLength()-1);
		m_aDirectories.Add(pTmp);
	}
	
#ifdef WIN32
	if (bThreaded)
	{
		DWORD dwThreadID;
		LocaterDebugMessage("CLocater::LocateFiles CREATING THREAD");
		m_hThread=CreateThread(NULL,0,CLocater::LocateThreadProc,this,0,&dwThreadID);
		LocaterDebugMessage("CLocater::LocateFiles CREATING THREAD OK?");
		return m_hThread!=NULL;
	}
	else
#endif
		return LocatingProc();
}

void CLocater::SetAdvanced(DWORD dwFlags,BYTE* pContainData,DWORD dwContainDataLength,
						   DWORD dwMaxFoundFiles)
{
	m_dwFlags=(m_dwFlags&~(LOCATE_FILENAMES|LOCATE_FOLDERNAMES|LOCATE_CONTAINTEXTISMATCHCASE))|dwFlags;
	m_dwMaxFoundFiles=dwMaxFoundFiles;
	
	if (pContainData!=NULL)
	{
		if (m_pContentSearcher!=NULL)
			delete m_pContentSearcher;

		if (dwFlags&LOCATE_REGULAREXPRESSIONSEARCH)
			m_pContentSearcher=new CSearchRegExpFromFile(LPCSTR(pContainData),dwContainDataLength);
		else
		{
			m_pContentSearcher=new CSearchHexFromFile(LPCSTR(pContainData),dwContainDataLength,
				(m_dwFlags&LOCATE_CONTAINTEXTISMATCHCASE)==LOCATE_CONTAINTEXTISMATCHCASE);
		}

	}
}

BOOL CLocater::LocateFiles(BOOL bThreaded,LPCSTR* szNames,DWORD nNames,
								LPCSTR* szExtensions,DWORD nExtensions,
								LPCSTR* szDirectories,DWORD nDirectories)
{

	DWORD i;
	for (i=0;i<nNames;i++)
	{
		if (szNames[i]!=NULL && szNames[i][0]!='\0')
		{
			CString* pName=new CString(szNames[i]);
			pName->MakeLower();
            m_aNames.Add(pName);
		}
	}
	for (i=0;i<nExtensions;i++)
		m_aExtensions.Add(new CString(szExtensions[i]));

	return SetDirectoriesAndStartToLocate(bThreaded,szDirectories,nDirectories);	
}

BOOL CLocater::LocateFiles(BOOL bThreaded,LPCSTR szRegExp,LPCSTR* szDirectories,DWORD nDirectories)
{
	LocaterDebugMessage5("CLocater::LocateFiles BEGIN, \r\nszRegExp:%s\r\nszDirectories:%X\r\nnDirectories:%d",
		szRegExp,(DWORD)szDirectories,nDirectories,0,0,0);

	if (szRegExp[0]!='\0')
	{
		m_dwFlags|=LOCATE_REGULAREXPRESSION;
		
		const char *error;
		int erroffset;

		m_regexp=pcre_compile(szRegExp,0,&error,&erroffset,NULL);

		if (m_regexp==NULL)
		{
			DebugFormatMessage("pcre_compile returns error: %s",error);
#ifdef _CONSOLE
			fprintf(stderr,"pcre_compile returns error: %s\n",error);
#endif
			return FALSE;
		}

		m_regextra=pcre_study(m_regexp,0,&error); 

	}
	return SetDirectoriesAndStartToLocate(bThreaded,szDirectories,nDirectories);	
}



#if WIN32 & !defined(LOCATER_NOTHREAD)
BOOL CLocater::StopLocating()
{
	LocaterDebugMessage("CLocater::StopLocating() BEGIN");
		
	HANDLE hThread=m_hThread;
	DWORD status;
	if (hThread==NULL)
		return FALSE;
	
	BOOL bRet=::GetExitCodeThread(m_hThread,&status);
	if (bRet && status==STILL_ACTIVE)
	{
		InterlockedExchange(&m_lForceQuit,TRUE);
		
		WaitForSingleObject(hThread,50);
		CWinThread* pThread=GetCurrentWinThread();
		if (pThread!=NULL)
		{
			// Processing messages
			PostQuitMessage(0);
			pThread->ModalLoop();			
		}
		WaitForSingleObject(hThread,50);
		bRet=::GetExitCodeThread(hThread,&status);
		
		if (bRet && status==STILL_ACTIVE)
		{
			::TerminateThread(hThread,1);
		
			bRet=::GetExitCodeThread(hThread,&status);
			while (bRet && status==STILL_ACTIVE)
			{
				::TerminateThread(hThread,1);
				Sleep(100);
				bRet=::GetExitCodeThread(hThread,&status);
			}
			
			m_pProc(m_dwData,FinishedDatabase,ueStopped,0,this);
			m_pProc(m_dwData,FinishedLocating,ueStopped,0,this);
			
			if (m_hThread!=NULL)
				m_pProc(m_dwData,ClassShouldDelete,ueStopped,0,this);
			
		}
	}

	LocaterDebugMessage("CLocater::StopLocating() END");
	return TRUE;
}
#endif

inline BOOL CLocater::IsFileNameWhatAreWeLookingFor() const
{
	if (m_dwFlags&LOCATE_REGULAREXPRESSION)
	{
		int ovector[OVECCOUNT];
		if (m_dwFlags&LOCATE_CHECKWHOLEPATH)
		{
			szCurrentPath[dwCurrentPathLen]='\\';
			dMemCopy(szCurrentPath+dwCurrentPathLen+1,GetFileName(),GetFileNameLen()+1);
			int rc = pcre_exec(m_regexp,m_regextra,szCurrentPath,dwCurrentPathLen+GetFileNameLen()+1,
				0,0,ovector,OVECCOUNT);

			szCurrentPath[dwCurrentPathLen]='\0';
			return rc>=0;
		}
		else
		{
			int rc = pcre_exec(m_regexp,m_regextra,GetFileName(),GetFileNameLen(),
				0,0,ovector,OVECCOUNT);
			return rc>=0;
		}
	}

	

	
	if (m_dwFlags&LOCATE_CHECKWHOLEPATH)
	{
		if (m_aNames.GetSize()==0)
			return TRUE;

		// Copying to buffer
		DWORD dwPathLen=dwCurrentPathLen+1+GetFileNameLen();
		char* szName=new char[dwPathLen+2];
		
		sMemCopy(szName,szCurrentPath,dwCurrentPathLen);
		szName[dwCurrentPathLen]='\\';
		sMemCopy(szName+dwCurrentPathLen+1,GetFileName(),GetFileNameLen());
		szName[dwPathLen]='\0';

	#ifdef WIN32
		CharLowerBuff(szName,dwPathLen);
	#else 
		strlwr(szName);
	#endif	

		for (int i=0;i<m_aNames.GetSize();i++)
		{
			if (m_aNames[i]->LastChar()=='.')
			{
				if (!HaveFileExtension())
				{
					// No extension, that is what are we looking for
					LPSTR szCondition=m_aNames[i]->GetBuffer();
					szCondition[m_aNames[i]->GetLength()-1]='\0';

					if (_ContainString(szName,szCondition))
					{
						delete[] szName;
						szCondition[m_aNames[i]->GetLength()-1]='.';
						return TRUE;
					}
					szCondition[m_aNames[i]->GetLength()-1]='.';
				}
			}
			else if (_ContainString(szName,*m_aNames[i]))
			{
				delete[] szName;
				return TRUE;
			}
		}	
		delete[] szName;
		return FALSE;
	}
	else
	{	
		if (m_aNames.GetSize()==0 && m_aExtensions.GetSize()==0)
			return TRUE;

		// Checking extension first
		if (m_aExtensions.GetSize()>0)
		{
			BOOL bFound=FALSE;
			
			if (!HaveFileExtension())
			{
				// No extension
				for (int i=0;i<m_aExtensions.GetSize();i++)
				{
					if (m_aExtensions[i]->GetLength()==0)
					{
						bFound=TRUE;
						break;
					}
				}
			}
			else
			{

				
				// Resolving extension length
				DWORD dwExtensionLen=GetFileNameLen()-GetFileExtensionPos()-1;
				char* szExtension=NULL;

				for (int i=0;i<m_aExtensions.GetSize();i++)
				{
					if (dwExtensionLen!=m_aExtensions[i]->GetLength())
						continue;

					if (szExtension==NULL)
					{
			#ifdef WIN32
						// Copying extension to buffer
						szExtension=new char[dwExtensionLen];
						sMemCopy(szExtension,GetFileName()+GetFileExtensionPos()+1,dwExtensionLen);
						CharLowerBuff(szExtension,dwExtensionLen);
			#else
						szExtension=new char[dwExtensionLen+1];
						sMemCopy(szExtension,GetFileName()+GetFileExtensionPos()+1,dwExtensionLen+1);
						strlwr(szExtension);
			#endif
					}
					
					if (_strncmp(szExtension,*m_aExtensions[i],dwExtensionLen))
					{
						bFound=TRUE;
						break;
					}
				}
				if (szExtension!=NULL)
					delete[] szExtension;

				
			}

			if (!bFound)
				return FALSE;
		}

		
		
		if (m_aNames.GetSize()==0)
			return TRUE;

		DWORD dwNameLength;
		// If whole name is what are we looking for, copy all name to szName
		if (m_dwFlags&LOCATE_EXTENSIONWITHNAME)
			dwNameLength=GetFileNameLen();
		else
		{
			dwNameLength=GetFileExtensionPos();
			if (dwNameLength==0) // No extension
				dwNameLength=GetFileNameLen();
		}
		
		// Copying to buffer
		char* szName=new char[dwNameLength+2];
		sMemCopy(szName,GetFileName(),dwNameLength);
		szName[dwNameLength]='\0';
	#ifdef WIN32
		CharLowerBuff(szName,dwNameLength);
	#else 
		strlwr(szName);
	#endif	
		for (int i=0;i<m_aNames.GetSize();i++)
		{
			if (m_aNames[i]->LastChar()=='.')
			{
				if (GetFileExtensionPos()==0 && GetFileName()[0]!='.')
				{
					// No extension, that is what are we looking for
					LPSTR szCondition=m_aNames[i]->GetBuffer();
					szCondition[m_aNames[i]->GetLength()-1]='\0';

					if (_ContainString(szName,szCondition))
					{
						delete[] szName;
						szCondition[m_aNames[i]->GetLength()-1]='.';
						return TRUE;
					}
					szCondition[m_aNames[i]->GetLength()-1]='.';
				}
			}
			else if (_ContainString(szName,*m_aNames[i]))
			{
				delete[] szName;
				return TRUE;
			}
		}	
		delete[] szName;
		return FALSE;
	}
}

inline BOOL CLocater::IsFolderNameWhatAreWeLookingFor() const
{
	if (m_dwFlags&LOCATE_REGULAREXPRESSION)
	{
		int ovector[OVECCOUNT];
		if (m_dwFlags&LOCATE_CHECKWHOLEPATH)
		{
			szCurrentPath[dwCurrentPathLen]='\\';
			dMemCopy(szCurrentPath+dwCurrentPathLen+1,GetFolderName(),GetFolderNameLen()+1);
			int rc = pcre_exec(m_regexp,m_regextra,szCurrentPath,dwCurrentPathLen+GetFolderNameLen()+1,
				0,0,ovector,OVECCOUNT);

			szCurrentPath[dwCurrentPathLen]='\0';
			return rc>=0;
		}
		else
		{
			int rc = pcre_exec(m_regexp,m_regextra,GetFolderName(),GetFolderNameLen(),
				0,0,ovector,OVECCOUNT);
			return rc>=0;
		}
		
	}
	if (m_dwFlags&LOCATE_CHECKWHOLEPATH)
	{
		if (m_aNames.GetSize()==0)
			return TRUE;

		// Copying to buffer
		DWORD dwPathLen=dwCurrentPathLen+1+GetFolderNameLen();
		char* szName=new char[dwPathLen+2];
		
		sMemCopy(szName,szCurrentPath,dwCurrentPathLen);
		szName[dwCurrentPathLen]='\\';
		sMemCopy(szName+dwCurrentPathLen+1,GetFolderName(),GetFolderNameLen());
		szName[dwPathLen]='\0';

	#ifdef WIN32
		CharLowerBuff(szName,dwPathLen);
	#else 
		strlwr(szName);
	#endif	

		for (int i=0;i<m_aNames.GetSize();i++)
		{
			if (_ContainString(szName,*m_aNames[i]))
			{
				delete[] szName;
				return TRUE;
			}
		}	
		delete[] szName;
		return FALSE;
	}
	else
	{	
	
		if (m_aNames.GetSize()==0 && m_aExtensions.GetSize())
			return TRUE;

		// Checking extension first
		if (m_aExtensions.GetSize()>0)
		{
			BOOL bFound=FALSE;

			for (int i=0;i<m_aExtensions.GetSize();i++)
			{
				// Resolving extension length
				DWORD dwExtensionPos=LastCharIndex(GetFolderName(),'.')+1;
				if (dwExtensionPos==-1)
					continue;

				DWORD dwExtensionLen=GetFolderNameLen()-dwExtensionPos;
				
				if (dwExtensionLen!=m_aExtensions[i]->GetLength())
					continue;

				// Copying extension to buffer
	#ifdef WIN32
				char* szExtension=new char[dwExtensionLen];
				sMemCopy(szExtension,GetFolderName()+dwExtensionPos,dwExtensionLen);
				CharLowerBuff(szExtension,dwExtensionLen);
	#else
				char* szExtension=new char[dwExtensionLen+1];
				sMemCopy(szExtension,GetFolderName()+dwExtensionPos,dwExtensionLen+1);
				strlwr(szExtension);
	#endif
				if (_strncmp(szExtension,*m_aExtensions[i],dwExtensionLen))
				{
					delete[] szExtension;
					bFound=TRUE;
					break;
				}
				delete[] szExtension;
			}

			if (!bFound)
				return FALSE;
		}
		
		if (m_aNames.GetSize()==0)
			return TRUE;

		if (GetFolderNameLen()==0)
			return FALSE;

	
		// Copying to buffer
		char* szName=new char[GetFolderNameLen()+1];
		sMemCopy(szName,GetFolderName(),GetFolderNameLen());
		szName[GetFolderNameLen()]='\0';
	#ifdef WIN32
		CharLower(szName);
	#else
		strlwr(szName);
	#endif		

		for (int i=0;i<m_aNames.GetSize();i++)
		{
			if (_ContainString(szName,*m_aNames[i]))
			{
				delete[] szName;
				return TRUE;
			}
		}
		delete[] szName;
		return FALSE;
	}
}

inline BOOL CLocater::IsFileAdvancedWhatAreWeLookingFor() const
{
	if (m_dwMinSize!=DWORD(-1))
	{
		if (GetFileSizeHi()==0 && GetFileSizeLo()<m_dwMinSize)
			return FALSE;
	}
	if (m_dwMaxSize!=DWORD(-1))
	{
		if (GetFileSizeHi()!=0 || GetFileSizeLo()>m_dwMaxSize)
			return FALSE;
	}
	if (m_wMaxDate!=WORD(-1))
	{
		if (m_dwFlags&LOCATE_MAXCREATIONDATE)
		{
			if (GetFileCreatedDate()>m_wMaxDate)
				return FALSE;
		}
		else if (m_dwFlags&LOCATE_MAXACCESSDATE)
		{
			if (GetFileAccessedDate()>m_wMaxDate)
				return FALSE;
		}
		else
		{
			if (GetFileModifiedDate()>m_wMaxDate)
				return FALSE;
		}
	}
	if (m_wMinDate!=WORD(-1))
	{
		if (m_dwFlags&LOCATE_MINCREATIONDATE)
		{
			if (GetFileCreatedDate()<m_wMinDate)
				return FALSE;
		}
		else if (m_dwFlags&LOCATE_MINACCESSDATE)
		{
			if (GetFileAccessedDate()<m_wMinDate)
				return FALSE;
		}
		else
		{
			if (GetFileModifiedDate()<m_wMinDate)
				return FALSE;
		}
	}
	if (m_pContentSearcher!=NULL)
	{
		// Forming path
		char szPath[MAX_PATH];
		CopyMemory(szPath,GetCurrentPath(),GetCurrentPathLen());
		szPath[GetCurrentPathLen()]='\\';
		CopyMemory(szPath+GetCurrentPathLen()+1,GetFileName(),GetFileNameLen()+1);
		
		// Searching
		m_pProc(m_dwData,SearchingStarted,ueStillWorking,0,this);
		BOOL bRet=m_pContentSearcher->Search(szPath);
		m_pProc(m_dwData,SearchingEnded,ueSuccess,0,this);
		m_pContentSearcher->CloseFile();
		return bRet;
	}
	return TRUE;
}

inline BOOL CLocater::IsFolderAdvancedWhatAreWeLookingFor() const
{
	if (m_wMaxDate!=WORD(-1))
	{
		if (m_dwFlags&LOCATE_MAXCREATIONDATE)
		{
			if (GetFolderCreatedDate()>m_wMaxDate)
				return FALSE;
		}
		else if (m_dwFlags&LOCATE_MAXACCESSDATE)
		{
			if (GetFolderAccessedDate()>m_wMaxDate)
				return FALSE;
		}
		else
		{
			if (GetFolderModifiedDate()>m_wMaxDate)
				return FALSE;
		}
	}
	if (m_wMinDate!=WORD(-1))
	{
		if (m_dwFlags&LOCATE_MINCREATIONDATE)
		{
			if (GetFolderCreatedDate()<m_wMinDate)
				return FALSE;
		}
		else if (m_dwFlags&LOCATE_MINACCESSDATE)
		{
			if (GetFolderAccessedDate()<m_wMinDate)
				return FALSE;
		}
		else
		{
			if (GetFolderModifiedDate()<m_wMinDate)
				return FALSE;
		}
	}
	return TRUE;
}

void CLocater::LocateValidFolder(DWORD nPathLen)
{
	while (*pPoint!='\0'
#ifdef WIN32
	   && !m_lForceQuit
#endif
      )

	{
		if ((*pPoint&UDBATTRIB_TYPEFLAG)==UDBATTRIB_DIRECTORY)
		{
			DWORD dwNameLen=pPoint[5];
			pPoint+=6;
			
			BOOL bSkipOver=FALSE;
			
			LPSTR pPath=szCurrentPath+nPathLen;
			LPSTR pPathLower=szCurrentPathLower+nPathLen;
	
			*pPath='\\';
			*pPathLower='\\';
			pPath++;
			pPathLower++;
			
			// Copying paths
			for (int j=dwNameLen;j>=0;j--)
			{
				pPath[j]=pPoint[j];
				pPathLower[j]=pPoint[j];
			}
#ifdef WIN32
			CharLower(pPathLower);
#else
			strlwr(pPathLower);
#endif

			ValidType vtType=IsFolderValid(nPathLen+dwNameLen+1);			

			switch (vtType)
			{
			case SomeValidFolders:
				pPoint+=9+dwNameLen;
				LocateValidFolder(nPathLen+dwNameLen+1);
				pPoint++;
				break;
			case ValidFolders:
				pPoint+=9+dwNameLen;
				CheckFolder(nPathLen+dwNameLen+1);
				pPoint++;
				break;
			default:
				pPoint+=*((LONG*)(pPoint+1-6))+(1-6);
				break;
			}
		}
		else if ((*pPoint&UDBATTRIB_TYPEFLAG)==UDBATTRIB_FILE)
		{
			// Skipping all files
			pPoint+=1+ // Type
				1+ // name length
				1+ // extension index
				pPoint[1]+1+ // filename + '\0'
				sizeof(DWORD)+sizeof(BYTE) + // Filesize
				sizeof(DWORD) + // Mofified date and time
				sizeof(WORD) + sizeof(WORD); // Creation and access date
		}
		else
			throw CFileException(CFileException::invalidFile,
#ifdef WIN32
			      -1,
#endif
				  m_pCurrentDatabase->szArchive);
	}
}

void CLocater::CheckFolder(DWORD nPathLen)
{
	szCurrentPath[nPathLen]='\0';
	dwCurrentPathLen=nPathLen;
				
	while (*pPoint!='\0'
#ifdef WIN32
       && !m_lForceQuit
#endif
       )
	{
		if ((*pPoint&UDBATTRIB_TYPEFLAG)==UDBATTRIB_DIRECTORY)
		{
			DWORD dwNameLen=pPoint[5];
			pPoint+=6;
			
			// This item is folder
			if (m_dwFlags&LOCATE_FOLDERNAMES)
			{
				if (IsFolderNameWhatAreWeLookingFor())
				{
					szCurrentPath[nPathLen]='\0';
					dwCurrentPathLen=nPathLen;
					if (IsFolderAdvancedWhatAreWeLookingFor())
					{
						if (!m_pCurrentFoundProc(m_dwData,TRUE,this))
							throw CException(CException::none);

						m_dwFoundDirectories++;
						if (m_dwFoundFiles+m_dwFoundDirectories>=m_dwMaxFoundFiles)
							throw ueLimitReached;
					}
				}
			}

			szCurrentPath[nPathLen]='\\';
			sMemCopy(szCurrentPath+nPathLen+1,pPoint,dwNameLen+1);
			
			if (m_aDirectories.GetSize()>0 && m_dwFlags&LOCATE_NOSUBDIRECTORIES)
				pPoint+=*((DWORD*)(pPoint-5))-6;
			else
			{
				pPoint+=9+dwNameLen;
				CheckFolder(nPathLen+dwNameLen+1);
			}

			
			// Putting these back to correct ones
			szCurrentPath[nPathLen]='\0';
			dwCurrentPathLen=nPathLen;


			pPoint++;
		}
		else if ((*pPoint&UDBATTRIB_TYPEFLAG)==UDBATTRIB_FILE)
		{
			// This item is file
			if (m_dwFlags&LOCATE_FILENAMES)
			{
				if (IsFileNameWhatAreWeLookingFor())
				{	
					if (IsFileAdvancedWhatAreWeLookingFor())
					{
					
						if (!m_pCurrentFoundProc(m_dwData,FALSE,this))
							throw CException(CException::none);
						
						m_dwFoundFiles++;
						if (m_dwFoundFiles+m_dwFoundDirectories>=m_dwMaxFoundFiles)
							throw ueLimitReached;
					}
				}
			}
			
			pPoint+=1+ // Type
				1+ // name length
				1+ // extension index
				pPoint[1]+1+ // filename + '\0'
				sizeof(DWORD)+sizeof(BYTE) + // Filesize
				sizeof(DWORD) + // Mofified date and time
				sizeof(WORD) + sizeof(WORD); // Creation and access date
		}
		else
			throw CFileException(CFileException::invalidFile,
#ifdef WIN32
			-1,
#endif
			m_pCurrentDatabase->szArchive);
	}
}