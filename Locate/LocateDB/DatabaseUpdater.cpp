/* Copyright (c) 1997-2004 Janne Huttunen
   database updater v2.98.4.9200                 */

#include <HFCLib.h>
#include "Locatedb.h"

#ifndef WIN32
#include "dtype.h"
#include "fcntl.h"
#endif

CDatabaseUpdater::CDatabaseUpdater(LPCSTR szDatabaseFile,LPCSTR szAuthor,LPCSTR szComment,
		LPCSTR* pszRoots,DWORD nNumberOfRoots,UPDATEPROC pProc,DWORD dwParam)
:	m_pCurrentRoot(NULL),sStatus(statusInitializing),m_dwFiles(0),m_dwDirectories(0),
	m_pProc(pProc),m_dwData(dwParam),m_dwCurrentDatabase(DWORD(-1)),dbFile(NULL)
#ifdef WIN32	
	,m_hThread(NULL),m_lForceQuit(FALSE)
#endif
{
	DebugMessage("CDatabaseUpdater::CDatabaseUpdater(1)");

	m_aDatabases.Add(new DBArchive(szDatabaseFile,CDatabase::archiveFile,
		szAuthor,szComment,pszRoots,nNumberOfRoots,0,NULL,0));
}

CDatabaseUpdater::CDatabaseUpdater(const PDATABASE* ppDatabases,
		int nDatabases,UPDATEPROC pProc,DWORD dwParam)
:	m_pCurrentRoot(NULL),sStatus(statusInitializing),m_dwFiles(0),m_dwDirectories(0),
	m_pProc(pProc),m_dwData(dwParam),m_dwCurrentDatabase(DWORD(-1)),dbFile(NULL)
#ifdef WIN32	
	,m_hThread(NULL),m_lForceQuit(FALSE)
#endif
{
	DebugMessage("CDatabaseUpdater::CDatabaseUpdater(2)");

	for (int i=0;i<nDatabases;i++)
		m_aDatabases.Add(new DBArchive(ppDatabases[i]));
}
	
CDatabaseUpdater::CDatabaseUpdater(const PDATABASE* ppDatabases,
		int nDatabases,UPDATEPROC pProc,WORD wThread,DWORD dwParam)
:	m_pCurrentRoot(NULL),sStatus(statusInitializing),m_dwFiles(0),m_dwDirectories(0),
	m_pProc(pProc),m_dwData(dwParam),m_dwCurrentDatabase(DWORD(-1)),dbFile(NULL)
#ifdef WIN32	
	,m_hThread(NULL),m_lForceQuit(FALSE)
#endif
{
	DebugMessage("CDatabaseUpdater::CDatabaseUpdater(3)");

	for (int i=0;i<nDatabases;i++)
	{
		if (ppDatabases[i]->GetThreadId()==wThread)	
			m_aDatabases.Add(new DBArchive(ppDatabases[i]));
	}
}


CDatabaseUpdater::~CDatabaseUpdater()
{
	DebugMessage("CDatabaseUpdater::~CDatabaseUpdater()");

	if (dbFile!=NULL)
	{
		delete dbFile;
		dbFile=NULL;
	}
	

#ifdef WIN32
	if (m_hThread!=NULL)
	{
		CloseHandle(m_hThread);
		m_hThread=NULL;
	}
#endif
}

UpdateError CDatabaseUpdater::UpdatingProc()
{
	DebugFormatMessage("CDatabaseUpdater::UpdatingProc() BEGIN this=%X m_pProc=%X",DWORD(this),DWORD(m_pProc));
	
	UpdateError ueResult=ueSuccess;

	class StopUpdating{};

#ifdef WIN32
	InterlockedExchange(&m_lForceQuit,FALSE);
#endif

	ASSERT(dbFile==NULL);

	for (m_dwCurrentDatabase=0;m_dwCurrentDatabase<DWORD(m_aDatabases.GetSize());m_dwCurrentDatabase++)
	{
		try {
			m_pCurrentRoot=m_aDatabases[m_dwCurrentDatabase]->m_pFirstRoot;
			
			DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): m_pCurrentDatabase=%X szName=%s",DWORD(m_aDatabases[m_dwCurrentDatabase]),m_aDatabases[m_dwCurrentDatabase]->m_szName);
				
			m_pProc(m_dwData,StartedDatabase,ueResult,this);
			
			

			while (m_pCurrentRoot!=NULL
	#ifdef WIN32
				&& !m_lForceQuit
	#endif
				)
			{
				DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): m_pCurrentRoot=%X path=%s",DWORD(m_pCurrentRoot),LPCSTR(m_pCurrentRoot->m_Path));
							
				sStatus=statusScanning;
				m_pProc(m_dwData,RootChanged,ueResult,this);
				
				
				// Scannin root
				ueResult=m_pCurrentRoot->ScanRoot(m_lForceQuit);
				if (ueResult==ueFolderUnavailable)
				{
					m_pProc(m_dwData,ErrorOccured,ueResult,this);
					if (m_aDatabases[m_dwCurrentDatabase]->IsFlagged(DBArchive::StopIfUnuavailable))
						throw ueFolderUnavailable; // Next database
				}
				else if (ueResult!=ueSuccess && ueResult!=ueFolderUnavailable)
					throw ueResult;
				
				m_dwFiles+=m_pCurrentRoot->m_dwFiles;
				m_dwDirectories+=m_pCurrentRoot->m_dwDirectories;
				
				// Next root
				m_pCurrentRoot=m_pCurrentRoot->m_pNext;
			}

	#ifdef WIN32
			if (m_lForceQuit)
				throw ueResult=ueStopped;
	#endif

			DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): writing to %s",m_aDatabases[m_dwCurrentDatabase]->m_szArchive);
		
			// Start writing database
			m_pProc(m_dwData,RootChanged,ueResult,this);
			m_pCurrentRoot=NULL; // Writing header
			sStatus=statusWritingDB;
			
			
			// Opening file
			switch (m_aDatabases[m_dwCurrentDatabase]->m_nArchiveType)
			{
			case CDatabase::archiveFile:
				dbFile=new CFile(m_aDatabases[m_dwCurrentDatabase]->m_szArchive,
					CFile::defWrite|CFile::otherStrNullTerminated,TRUE);
				break;
			default:
				throw CFileException(CFileException::notImplemented,
					-1,m_aDatabases[m_dwCurrentDatabase]->m_szArchive);
			}
			
	#ifdef WIN32
			// Writing identification, '\17=0x11=0x10|0x1' 0x1 = Long filenames and 0x10 = ANSI
			dbFile->Write("LOCATEDB20",10);
			dbFile->Write(BYTE(0x11));
	#else
			// Writing identification, '\0x0' = Short filenames and OEM
			dbFile->Write("LOCATEDB20",10);
			m_pCurrentRoot=m_pFirstRoot;
			BOOL bLFN=FALSE;
			while (m_pCurrentRoot!=NULL)
			{
				if (_use_lfn(m_pCurrentRoot->m_Path))
				{
					bLFN=TRUE;
					break;
				}
				m_pCurrentRoot=m_pCurrentRoot->m_pNext;
			}
			if (bLFN)
				File.Write(BYTE(1));
			else
				File.Write(BYTE(0));
	#endif
				
			// Writing header size
			dbFile->Write(DWORD(
				m_aDatabases[m_dwCurrentDatabase]->m_sAuthor.GetLength()+1+ // Author data
				m_aDatabases[m_dwCurrentDatabase]->m_sComment.GetLength()+1+ // Comments data
				1+1+ // Extra
				4+ // Time
				4+ // Number of files
				4  // Number of directories
				)
			);

			// Writing author
			dbFile->Write(m_aDatabases[m_dwCurrentDatabase]->m_sAuthor);
	
			// Writing comments
			dbFile->Write(m_aDatabases[m_dwCurrentDatabase]->m_sComment);
			
			// Writing free data
			dbFile->Write((BYTE)0);

			dbFile->Write((BYTE)0);
			
			// Writing filedate
			{
				WORD wTime,wDate;
	#ifdef WIN32
				SYSTEMTIME st;
				FILETIME ft;
				GetLocalTime(&st);
				SystemTimeToFileTime(&st,&ft);
				FileTimeToDosDateTime(&ft,&wDate,&wTime);
	#else
				time_t tt;
				struct tm *t;
				time(&tt);
				t=localtime(&tt);
				wDate=(t->tm_mday&0x1F)|(((t->tm_mon+1)&0x0F)<<5)|(((t->tm_year-80)&0x7F)<<9);
				wTime=((t->tm_sec/2)&0x1F)|((t->tm_min&0x3F)<<5)|((t->tm_hour&0x1F)<<11);
	#endif
				dbFile->Write(wDate);
				dbFile->Write(wTime);
			}

			// Writing number of files and directories
			dbFile->Write(m_dwFiles);
			dbFile->Write(m_dwDirectories);
			
			// Writing root directory datas
			m_pCurrentRoot=m_aDatabases[m_dwCurrentDatabase]->m_pFirstRoot;
			while (m_pCurrentRoot!=NULL && ueResult==ueSuccess)
			{
				sStatus=statusWritingDB;
				
				// Writing root data
				ueResult=m_pCurrentRoot->Write(dbFile);

				m_pCurrentRoot=m_pCurrentRoot->m_pNext;
			}

			// End mark
			dbFile->Write((DWORD)0);

			delete dbFile;
			dbFile=NULL;

			DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): DB %X OK",DWORD(m_aDatabases[m_dwCurrentDatabase]));
		}
		catch (CFileException fe)
		{
			DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): catch CFileException exception with cause %d",fe.m_cause);

			switch (fe.m_cause)
			{
			case CFileException::badPath:
			case CFileException::accessDenied:
			case CFileException::fileExist:
			case CFileException::sharingViolation:
			case CFileException::fileCreate:
			case CFileException::directoryFull:
			case CFileException::tooManyOpenFiles:
				m_pProc(m_dwData,ErrorOccured,ueResult=ueCreate,this);
				break;
			case CFileException::invalidFile:
			case CFileException::fileOpen:
				m_pProc(m_dwData,ErrorOccured,ueResult=ueOpen,this);
				break;
			case CFileException::endOfFile:
			case CFileException::writeProtected:
			case CFileException::writeFault:
			case CFileException::diskFull:
			case CFileException::lockViolation:
				m_pProc(m_dwData,ErrorOccured,ueResult=ueWrite,this);
				break;
			case CFileException::readFault:
				m_pProc(m_dwData,ErrorOccured,ueResult=ueRead,this);
				break;
			default:
				m_pProc(m_dwData,ErrorOccured,ueResult=ueUnknown,this);
				break;
			}
			break;
		}
		catch (CException ex)
		{
			DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): catch CException exception with cause %d",ex.m_cause);

			switch (ex.m_cause)
			{
			case CException::cannotAllocate:
				m_pProc(m_dwData,ErrorOccured,ueResult=ueAlloc,this);
				break;
			default:
				m_pProc(m_dwData,ErrorOccured,ueResult=ueUnknown,this);
				break;
			}
			break;
		}
		catch (UpdateError ue)
		{
			DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): catch uerror %d",DWORD(ue));

			if (ue!=ueStillWorking && ue!=ueSuccess && ue!=ueFolderUnavailable)
			{
				m_pProc(m_dwData,ErrorOccured,ueResult=ue,this);
				break;
			}
		}
		catch (...)
		{
			DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): catch unknown exception");

			m_pProc(m_dwData,ErrorOccured,ueResult=ueUnknown,this);
			break;
		}
		m_pProc(m_dwData,FinishedDatabase,ueResult,this);
	}

	// Closing file if needed
	if (dbFile!=NULL)
	{
		delete dbFile;
		dbFile=NULL;
	}
	m_dwCurrentDatabase=DWORD(-1);
	m_pProc(m_dwData,FinishedUpdating,ueResult,this);
	m_pProc(m_dwData,ClassShouldDelete,ueResult,this);

	// This class is deleted in the previous call, do not access this anymore

	DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): ALL DONE ueResult=%d",DWORD(ueResult));

	return ueResult;
}

#ifdef WIN32
DWORD WINAPI CDatabaseUpdater::UpdateThreadProc(LPVOID lpParameter)
{
	DebugNumMessage("CDatabaseUpdater::UpdateThreadProc: lParam=%X",DWORD(lpParameter));
	return ((CDatabaseUpdater*)lpParameter)->UpdatingProc()==ueSuccess?0:1;	
}
#endif

// Start updating database
#ifdef WIN32
UpdateError CDatabaseUpdater::Update(BOOL bThreaded)
{
	m_pProc(m_dwData,Initializing,ueSuccess,this);
	if (bThreaded)
	{
		DWORD dwThreadID;
		DebugNumMessage("CDatabaseUpdater::Update this=%X",DWORD(this));
		m_hThread=CreateThread(NULL,0,UpdateThreadProc,this,0,&dwThreadID);
		return m_hThread!=NULL?ueSuccess:ueCannotCreateThread;
	}
	else
		return UpdatingProc();
}

BOOL CDatabaseUpdater::StopUpdating(BOOL bForce)
{
	HANDLE hThread=m_hThread;
	DWORD status;
	if (hThread==NULL)
		return TRUE;
	
	BOOL bRet=::GetExitCodeThread(m_hThread,&status);
	if (bRet && status==STILL_ACTIVE)
	{
		InterlockedExchange(&m_lForceQuit,TRUE);
		WaitForSingleObject(hThread,200);
		bRet=::GetExitCodeThread(hThread,&status);
		
		if (!bForce)
			return !(bRet && status==STILL_ACTIVE);

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

			m_pProc(m_dwData,FinishedUpdating,ueStopped,this);
			if (m_hThread!=NULL)
				m_pProc(m_dwData,ClassShouldDelete,ueStopped,this);
		}
	}
	return TRUE;
}
#else

UpdateError CDatabaseUpdater::Update()
{
	m_pProc(m_dwData,Initializing,ueSuccess,this);
	return UpdatingProc();
}
#endif




CDatabaseUpdater::CRootDirectory::~CRootDirectory()
{
	while (m_pFirstBuffer!=NULL)
	{
		pCurrentBuffer=m_pFirstBuffer->pNext;
		delete m_pFirstBuffer;
		m_pFirstBuffer=pCurrentBuffer;
	}
}

UpdateError CDatabaseUpdater::CRootDirectory::ScanRoot(volatile LONG& lForceQuit)
{
	DebugFormatMessage("CDatabaseUpdater::CRootDirectory::ScanRoot: scanning root %s",m_Path);

	// Initializing buffer
	pCurrentBuffer=m_pFirstBuffer=new CBuffer;
	if (pCurrentBuffer==NULL)
		throw CException(CException::cannotAllocate);

	pPoint=*pCurrentBuffer;

	ASSERT(m_Path.GetLength()<=MAX_PATH);

	// Scanning folder
	char szPath[MAX_PATH+20];
	sMemCopy(szPath,(LPCSTR)m_Path,m_Path.GetLength()+1);
	
	DebugMessage("CDatabaseUpdater::CRootDirectory::ScanRoot: started to scan root folder");
	UpdateError ueResult=ScanFolder(szPath,m_Path.GetLength(),lForceQuit);
	DebugMessage("CDatabaseUpdater::CRootDirectory::ScanRoot: ended to scan root folder");
	
	
	pCurrentBuffer->nLength=pPoint-pCurrentBuffer->pData;

	DebugFormatMessage("CDatabaseUpdater::CRootDirectory::ScanRoot: END scanning root %s",m_Path);
	return ueResult;
}

#ifdef WIN32
typedef	HANDLE HFIND;
typedef WIN32_FIND_DATA FIND_DATA;

#define VALID_HFIND(h)		((h)!=INVALID_HANDLE_VALUE)

inline HFIND _FindFirstFile(LPCSTR szFolder,FIND_DATA* fd)
{
	return FindFirstFile(szFolder,fd);
}
inline BOOL _FindNextFile(HFIND hFind,FIND_DATA* fd)
{
	return FindNextFile(hFind,fd);
}
inline void _FindClose(HFIND hFind)
{
	FindClose(hFind);
}
inline BOOL _FindIsFolder(FIND_DATA* fd)
{
	return (fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY;
}
inline LPCSTR _FindGetName(FIND_DATA* fd)
{
	return fd->cFileName;
}

inline BYTE _FindGetAttribFlag(FIND_DATA* fd)
{
	return CDatabaseUpdater::GetAttribFlag(fd->dwFileAttributes);
}

inline void _FindGetLastWriteDosDateTime(FIND_DATA* fd,WORD* pwDate,WORD* pwTime)
{
	FILETIME ft;
	FileTimeToLocalFileTime(&fd->ftLastWriteTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,pwTime);
}
inline void _FindGetCreationDosDate(FIND_DATA* fd,WORD* pwDate)
{
	FILETIME ft;
	WORD wTemp;
	FileTimeToLocalFileTime(&fd->ftCreationTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,&wTemp);
}
inline void _FindGetLastAccessDosDate(FIND_DATA* fd,WORD* pwDate)
{
	FILETIME ft;
	WORD wTemp;
	FileTimeToLocalFileTime(&fd->ftLastAccessTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,&wTemp);
}
inline DWORD _FindGetFileSizeLo(FIND_DATA* fd)
{
	return fd->nFileSizeLow;
}
inline DWORD _FindGetFileSizeHi(FIND_DATA* fd)
{
	return fd->nFileSizeHigh;
}

#else
typedef int HFIND;
typedef struct ffblk FIND_DATA;

#define VALID_HFIND(h)		((h)==0)

inline HFIND _FindFirstFile(LPCSTR szFolder,FIND_DATA* fd)
{
	return findfirst(szFolder,fd,FA_HIDDEN|FA_SYSTEM|FA_DIREC);
}
inline BOOL _FindNextFile(HFIND hFind,FIND_DATA* fd)
{
	return !findnext(fd);
}
inline void _FindClose(HFIND hFind)
{
}
inline BOOL _FindIsFolder(FIND_DATA* fd)
{
	return (fd->ff_attrib&FA_DIREC)==FA_DIREC;
}
inline LPCSTR _FindGetName(FIND_DATA* fd)
{
	return fd->ff_name;
}
inline BYTE _FindGetAttribFlag(FIND_DATA* fd)
{
	BYTE bRet=0;
	if (fd->ff_attrib&FA_ARCH)
		bRet|=UDBATTRIB_ARCHIVE;
	if (fd->ff_attrib&FA_HIDDEN)
		bRet|=UDBATTRIB_HIDDEN;
	if (fd->ff_attrib&FA_RDONLY)
		bRet|=UDBATTRIB_READONLY;
	if (fd->ff_attrib&FA_SYSTEM)
		bRet|=UDBATTRIB_SYSTEM;
	return bRet;
}
inline void _FindGetLastWriteDosDateTime(FIND_DATA* fd,WORD* pwDate,WORD* pwTime)
{
	*pwDate=fd->ff_fdate;
	*pwTime=fd->ff_ftime;
}
inline void _FindGetCreationDosDate(FIND_DATA* fd,WORD* pwDate)
{
	*pwDate=fd->lfn_cdate;
}
inline void _FindGetLastAccessDosDate(FIND_DATA* fd,WORD* pwDate)
{
	*pwDate=fd->lfn_adate;
}
inline DWORD _FindGetFileSizeLo(FIND_DATA* fd)
{
	return fd->ff_fsize;
}
inline DWORD _FindGetFileSizeHi(FIND_DATA* fd)
{
	return 0;
}
#endif

UpdateError CDatabaseUpdater::CRootDirectory::ScanFolder(LPSTR szFolder,DWORD nLength,volatile LONG& lForceQuit)
{
	if (m_aExcludedDirectories.GetSize()>0)
	{
		// Checking whether directory is excluded
		char* pLowerFolder=new char [nLength+1];
		CopyMemory(pLowerFolder,szFolder,nLength);
		pLowerFolder[nLength]='\0';
		CharLower(pLowerFolder);

		for (int i=0;i<m_aExcludedDirectories.GetSize();i++)
		{
			if (strcmp(m_aExcludedDirectories[i],pLowerFolder)==0)
			{
				delete[] pLowerFolder;
				return ueSuccess;
			}
		}
		delete[] pLowerFolder;
	}	
	
	szFolder[nLength]='\\';
	szFolder[nLength+1]='*';
	szFolder[nLength+2]='.';
	szFolder[nLength+3]='*';
	szFolder[nLength+4]='\0';

	

	FIND_DATA fd;
	
	HFIND hFind=_FindFirstFile(szFolder,&fd);
	
	DWORD dwTemp;

	if (!VALID_HFIND(hFind))
		return ueFolderUnavailable;

	for(;;)
	{
		if (_FindGetName(&fd)[0]!='.' || (_FindGetName(&fd)[1]!='\0' && _FindGetName(&fd)[1]!='.'))
		{
			if (lForceQuit)
			{
				_FindClose(hFind);
				throw ueStopped;
			}


			if (_FindIsFolder(&fd))
			{
				// Get the length of directory name and checks that length is 
				// less than MAX_PATH, otherwise ignore
				dstrlen(_FindGetName(&fd),dwTemp);
				if (nLength+dwTemp+1<MAX_PATH)
				{
					
					// Check whether new buffer is needed
					if (pPoint+_MAX_PATH+10>pCurrentBuffer->pData+BFSIZE)
					{
						// New buffer
						pCurrentBuffer->nLength=pPoint-pCurrentBuffer->pData;
						pCurrentBuffer=pCurrentBuffer->pNext=new CBuffer;
						if (pCurrentBuffer==NULL)
							throw CException(CException::cannotAllocate);
						
						pPoint=*pCurrentBuffer;
					}
					
					*pPoint=UDBATTRIB_DIRECTORY|_FindGetAttribFlag(&fd); // Directory
					BYTE* pSizePointer=pPoint+1;
					pPoint+=5;

					
					

					// Copying cFilename to pPoint and szFolder, and setting *pPoint to name length
					for (*pPoint=0;_FindGetName(&fd)[*pPoint]!='\0';(*pPoint)++)
					{
						szFolder[nLength+1+*pPoint]=_FindGetName(&fd)[*pPoint];
						pPoint[1+*pPoint]=_FindGetName(&fd)[*pPoint];
					}
					// '\0' to end
					szFolder[nLength+1+*pPoint]='\0';
					pPoint[1+*pPoint]='\0';
					
					pPoint+=*pPoint+1+1;


					// File time
					_FindGetLastWriteDosDateTime(&fd,(WORD*)pPoint,(WORD*)pPoint+1);
					_FindGetCreationDosDate(&fd,(WORD*)pPoint+2);
					_FindGetLastAccessDosDate(&fd,(WORD*)pPoint+3);
					pPoint+=sizeof(WORD)*4;
				
					ScanFolder(szFolder,nLength+1+dwTemp,lForceQuit);
					szFolder[nLength+1+dwTemp]='\0';
					*pPoint='\0';
					pPoint++;

					// Calculating directory data size
					if (pSizePointer>=pCurrentBuffer->pData && pSizePointer<pCurrentBuffer->pData+BFSIZE)
						((DWORD*)pSizePointer)[0]=pPoint-pSizePointer;
					else
					{
						// Buffer has changed
						CBuffer* pTmp=m_pFirstBuffer;
						
						// old buffer to pTmp
						while (pSizePointer<pTmp->pData || pSizePointer>=pTmp->pData+BFSIZE)
							pTmp=pTmp->pNext;
						
						// Decreasing first buffer
						((LONG*)pSizePointer)[0]=(LONG)pTmp->pData-(LONG)pSizePointer;
						
						// Adding length between pCurrentbuffer and pTmp
						for (;pTmp!=pCurrentBuffer;pTmp=pTmp->pNext)
							((LONG*)pSizePointer)[0]+=pTmp->nLength;

						// Adding this buffer len
						((LONG*)pSizePointer)[0]+=pPoint-pCurrentBuffer->pData;
					}

					// Increase directories count
					m_dwDirectories++;
				}
			}
			else
			{
				DWORD dwTemp;
				// File name
				dstrlen(_FindGetName(&fd),dwTemp);
				
				if(nLength+dwTemp+1<MAX_PATH)
				{
					if (pPoint+275>pCurrentBuffer->pData+BFSIZE)
					{
						// New buffer
						pCurrentBuffer->nLength=pPoint-pCurrentBuffer->pData;
						pCurrentBuffer=pCurrentBuffer->pNext=new CBuffer;
						pPoint=*pCurrentBuffer;
					}

					*pPoint=UDBATTRIB_FILE|_FindGetAttribFlag(&fd); // File
					pPoint++;
				
					// File name length
					pPoint[0]=(BYTE)(dwTemp>255?255:dwTemp); 
					// Extension position (or 0 if no extension)
					for (pPoint[1]=pPoint[0]-1;pPoint[1]>0 && _FindGetName(&fd)[pPoint[1]]!='.';pPoint[1]--); 

					// Copying filename
					sMemCopy(pPoint+2,_FindGetName(&fd),pPoint[0]);
					pPoint+=((DWORD)pPoint[0])+3;
					pPoint[-1]='\0';
					
					// File size
					((DWORD*)pPoint)[0]=_FindGetFileSizeLo(&fd);
					pPoint[4]=(BYTE)_FindGetFileSizeHi(&fd);
					pPoint+=5;

					// File time
					_FindGetLastWriteDosDateTime(&fd,(WORD*)pPoint,(WORD*)pPoint+1);
					_FindGetCreationDosDate(&fd,(WORD*)pPoint+2);
					_FindGetLastAccessDosDate(&fd,(WORD*)pPoint+3);
					
					pPoint+=sizeof(WORD)*4;
					
					// Increase files count
					m_dwFiles++;
				}
			}
		}		
		if(!_FindNextFile(hFind,&fd))
			break;
	}
	_FindClose(hFind);

	return ueSuccess;
}

#ifndef WIN32
BYTE _GetDriveType(LPCSTR szPath)
{
	return getdrivetype(szPath);
}
#else
inline BYTE _GetDriveType(CString& sPath)
{
	BYTE nType;
	switch (GetDriveType(sPath+'\\'))
	{
	case DRIVE_UNKNOWN:
		nType=0x00;
		break;
	case DRIVE_NO_ROOT_DIR:
		nType=0xF0;
		break;
	case DRIVE_REMOVABLE:
		nType=0x20;
		break;
	case DRIVE_FIXED:
		nType=0x10;
		break;
	case DRIVE_REMOTE:
		nType=0x40;
		break;
	case DRIVE_CDROM:
		nType=0x30;
		break;
	case DRIVE_RAMDISK:
		nType=0x50;
		break;
	}
	return nType;
}
#endif

UpdateError CDatabaseUpdater::CRootDirectory::Write(CFile* dbFile)
{
	CString sVolumeName,sFSName;
	DWORD dwSerial=0;
	BYTE nType;
	
	{
		if (m_Path.GetLength()==2 || (m_Path[0]=='\\' && m_Path[1]=='\\'))
			nType=_GetDriveType(m_Path);
		else
			nType=0xF0;
		// resolving label,fs and serial
#ifdef WIN32
		// Resolving information
		DWORD dwTemp;
		UINT nOldMode=SetErrorMode(SEM_FAILCRITICALERRORS);
		BOOL nRet;
		if (m_Path[0]!='\\')
		{
			char szDrive[4]="X:\\";
			szDrive[0]=m_Path[0];
			nRet=GetVolumeInformation(szDrive,sVolumeName.GetBuffer(20),20,&dwSerial,
				&dwTemp,&dwTemp,sFSName.GetBuffer(20),20);
		}
		else // network, UNC path
			nRet=GetVolumeInformation(m_Path,sVolumeName.GetBuffer(20),20,&dwSerial,
				&dwTemp,&dwTemp,sFSName.GetBuffer(20),20);
		
		if (nRet)
		{
			sVolumeName.FreeExtra();
			sFSName.FreeExtra();
		}
		else
		{
			sVolumeName.Empty();
			sFSName.Empty();
			dwSerial=0;
		}

		SetErrorMode(nOldMode);
#else
		// getting fs
		if (getvolumeinfo(m_Path,&dwSerial,sVolumeName.GetBuffer(12),sFSName.GetBuffer(10)))
		{
			sVolumeName.FreeExtra();
			sFSName.FreeExtra();
		}
		else
		{
			sVolumeName.Empty();
			sFSName.Empty();
			dwSerial=0;
		}
#endif
	}

	// Calculating data length
	DWORD nLength=1 // Type
		+m_Path.GetLength()+1 // Path
		+sVolumeName.GetLength()+1 // Volume name
		+sizeof(DWORD)	//Serial
		+sFSName.GetLength()+1 // Filesystem
		+2*sizeof(DWORD) // Number of files and directories
		+2; // End, 0x0000
	
	pCurrentBuffer=m_pFirstBuffer;
	while (pCurrentBuffer!=NULL)
	{
		nLength+=pCurrentBuffer->nLength;
		pCurrentBuffer=pCurrentBuffer->pNext;
	}
	
	// Writing data length
	dbFile->Write(nLength);

	// Writing volume type
	dbFile->Write(nType);

	// Writing path
	dbFile->Write(m_Path);

	// Writing volume name
	dbFile->Write(sVolumeName);

	// Writing volume serial
	dbFile->Write(dwSerial);

	// Writing filesystem
	dbFile->Write(sFSName);

	// Writing number of files and directories
	dbFile->Write(m_dwFiles);
	dbFile->Write(m_dwDirectories);
       
	// Why?
	dbFile->Flush();

	while (m_pFirstBuffer!=NULL)
	{
		pCurrentBuffer=m_pFirstBuffer->pNext;
		dbFile->Write(m_pFirstBuffer->pData,m_pFirstBuffer->nLength);
		delete m_pFirstBuffer;
		m_pFirstBuffer=pCurrentBuffer;
	}

	// End mark
	dbFile->Write((WORD)0);

	return ueSuccess;
}

CDatabaseUpdater::DBArchive::DBArchive(const CDatabase* pDatabase)
:	m_sAuthor(pDatabase->GetCreator()),m_sComment(pDatabase->GetDescription()),
	m_nArchiveType(pDatabase->GetArchiveType()),m_pFirstRoot(NULL),m_nFlags(0)
{
	DWORD dwTemp;
	dstrlen(pDatabase->GetArchiveName(),dwTemp);
	m_szArchive=new char[dwTemp+1];
	CopyMemory(m_szArchive,pDatabase->GetArchiveName(),dwTemp+1);

	dstrlen(pDatabase->GetName(),m_dwNameLength);
	m_szName=new char[m_dwNameLength+1];
	CopyMemory(m_szName,pDatabase->GetName(),m_dwNameLength+1);

	if (pDatabase->IsFlagged(CDatabase::flagStopIfRootUnavailable))
		m_nFlags|=DBFlags::StopIfUnuavailable;


	LPCSTR pPtr=pDatabase->GetRoots();
	if (pPtr==NULL)
	{
		char drive[3]="X:";
		
#ifdef WIN32
		DWORD dwBufferLen=GetLogicalDriveStrings(0,NULL)+1;
		char* szDrives=new char[dwBufferLen];
		GetLogicalDriveStrings(dwBufferLen,szDrives);


		CRootDirectory* tmp;
		for (int i=0;szDrives[i*4]!='\0';i++)
		{
			if (GetDriveType(szDrives+i*4)==DRIVE_FIXED)
			{
				drive[0]=szDrives[i*4];
				
				if (m_pFirstRoot==NULL)
					tmp=m_pFirstRoot=new CRootDirectory(drive);
				else
					tmp=tmp->m_pNext=new CRootDirectory(drive);
			}
		}
#else
		char szDrives[27];
		getlocaldrives(szDrives);
		for (int i=0;i<27;i++)
		{
			if (szDrives[i])
			{
				drive[0]=i+'A';

				if (m_pFirstRoot==NULL)
					tmp=m_pFirstRoot=new CRootDirectory(drive);
				else
					tmp=tmp->m_pNext=new CRootDirectory(drive);
			}
		}
#endif
        
		if (m_pFirstRoot!=NULL)
			tmp->m_pNext=NULL;
		
		delete[] szDrives;
	}
	else
	{
		CRootDirectory* tmp;
			
		while (*pPtr!='\0')
		{
			DWORD dwLength;
			dstrlen(pPtr,dwLength);

			if ((dwLength==2 || dwLength==3) && pPtr[1]==':') // Root is drive
			{
				char root[]="X:\\";
				root[0]=pPtr[0];
				if (m_pFirstRoot==NULL)
					tmp=m_pFirstRoot=new CRootDirectory(root,dwLength);
				else
					tmp=tmp->m_pNext=new CRootDirectory(root,dwLength);
			}
			else if (pPtr[0]=='\\' && pPtr[1]=='\\')
			{
				if (CFile::IsDirectory(pPtr))
				{
					if (m_pFirstRoot==NULL)
						tmp=m_pFirstRoot=new CRootDirectory(pPtr,dwLength);
					else
						tmp=tmp->m_pNext=new CRootDirectory(pPtr,dwLength);
				}
				else
				{
					// May be computer, since it is not directory
					// So retrieve shares
					DWORD dwEntries=-1,cbBuffer=16384;
						
					NETRESOURCE nr;
					nr.dwScope=RESOURCE_CONNECTED;
					nr.dwType=RESOURCETYPE_DISK;
					nr.dwDisplayType=RESOURCEDISPLAYTYPE_SHARE;
					nr.dwUsage=RESOURCEUSAGE_CONTAINER;
					nr.lpLocalName=NULL;
					nr.lpRemoteName=const_cast<LPSTR>(pPtr);
					nr.lpComment=NULL;
					nr.lpProvider=NULL;

					HANDLE hEnum;
					DWORD dwRet=WNetOpenEnum(RESOURCE_GLOBALNET,RESOURCETYPE_DISK,0,&nr,&hEnum);
										
					if (dwRet!=NOERROR)
					{
						if (m_pFirstRoot==NULL)
							tmp=m_pFirstRoot=new CRootDirectory(pPtr,dwLength);
						else
							tmp=tmp->m_pNext=new CRootDirectory(pPtr,dwLength);

						pPtr+=dwLength+1;
						continue;
					}
						
					NETRESOURCE* nro;
						
					for(;;)
					{
						nro=(NETRESOURCE*)GlobalAlloc(GPTR,cbBuffer);
						dwRet=WNetEnumResource(hEnum,&dwEntries,nro,&cbBuffer);
						if (dwRet!=ERROR_MORE_DATA && dwRet!=NOERROR)
							break;

						for (DWORD i=0;i<dwEntries;i++)
						{
							if (m_pFirstRoot==NULL)
								tmp=m_pFirstRoot=new CRootDirectory(nro[i].lpRemoteName);
							else
								tmp=tmp->m_pNext=new CRootDirectory(nro[i].lpRemoteName);
						}
						GlobalFree((HGLOBAL)nro);
					}
						
					WNetCloseEnum(hEnum);	
				}
			}
			else
			{
				if (m_pFirstRoot==NULL)
					tmp=m_pFirstRoot=new CRootDirectory(pPtr,dwLength);
				else
					tmp=tmp->m_pNext=new CRootDirectory(pPtr,dwLength);
			}

			pPtr+=dwLength+1;
		}

		if (m_pFirstRoot!=NULL)
			tmp->m_pNext=NULL;
	}

	// Excluded directories
	if (pDatabase->GetExcludedDirectories().GetSize()>0)
		ParseExcludedDirectories(pDatabase->GetExcludedDirectories().GetData(),pDatabase->GetExcludedDirectories().GetSize());
}
		
CDatabaseUpdater::DBArchive::DBArchive(LPCSTR szArchiveName,CDatabase::ArchiveType nArchiveType,
											  LPCSTR szAuthor,LPCSTR szComment,LPCSTR* pszRoots,DWORD nNumberOfRoots,BYTE nFlags,
											  LPCSTR* ppExcludedDirectories,int nExcludedDirectories)
:	m_sAuthor(szAuthor),m_sComment(szComment),m_nArchiveType(nArchiveType),
	m_szName(NULL),m_dwNameLength(0),m_nFlags(nFlags)
{
	DWORD dwTemp;
	dstrlen(szArchiveName,dwTemp);
	m_szArchive=new char[dwTemp+1];
	sMemCopy(m_szArchive,szArchiveName,dwTemp+1);

	if (nNumberOfRoots==0)
	{
		m_pFirstRoot=NULL;
		return;
	}

	CRootDirectory* tmp=m_pFirstRoot=new CRootDirectory(pszRoots[0]);
	for (DWORD i=1;i<nNumberOfRoots;i++)
		tmp=tmp->m_pNext=new CRootDirectory(pszRoots[i]);
	tmp->m_pNext=NULL;

	if (nExcludedDirectories>0)
		ParseExcludedDirectories(ppExcludedDirectories,nExcludedDirectories);
}

CDatabaseUpdater::DBArchive::~DBArchive()
{
	CRootDirectory* m_pCurrentRoot;

	// Cleaning data
	while (m_pFirstRoot!=NULL)
	{
		m_pCurrentRoot=m_pFirstRoot->m_pNext;
		delete m_pFirstRoot;
		m_pFirstRoot=m_pCurrentRoot;
	}

	if (m_szArchive!=NULL)
        delete[] m_szArchive;
}


void CDatabaseUpdater::DBArchive::ParseExcludedDirectories(const LPCSTR* ppExcludedDirs,int nExcludedDirectories)
{
	// First, check that directory names are valid
	LPSTR* ppExcludedDirectories=new LPSTR[max(nExcludedDirectories,2)];
	DWORD* pdwExcludedDirectoriesLen=new DWORD[max(nExcludedDirectories,2)];
	for (int i=0,j=0;i<nExcludedDirectories;i++)
	{
		if (ppExcludedDirs[i][0]=='\0')
			continue;
		if (ppExcludedDirs[i][1]=='\0')
			continue;

		if (ppExcludedDirs[i][1]==':' && ppExcludedDirs[i][2]=='\0')
		{
			ppExcludedDirectories[j]=new char[3];
			ppExcludedDirectories[j][0]=ppExcludedDirs[i][0];
			CharLowerBuff(ppExcludedDirectories[j],1);
			ppExcludedDirectories[j][1]=':';
			ppExcludedDirectories[j][2]='\0';

			pdwExcludedDirectoriesLen[j]=2;
		
			j++;
			continue;
		}


		char szBuffer[400];
        int nRet=GetFullPathName(ppExcludedDirs[i],400,szBuffer,NULL);

		if (!nRet)
			continue;

		if (szBuffer[nRet-1]=='\\')
			szBuffer[--nRet]='\0';

		CharLower(szBuffer);
		pdwExcludedDirectoriesLen[j]=nRet;
		ppExcludedDirectories[j++]=alloccopy(szBuffer,nRet);

	}
	nExcludedDirectories=j;



	// Then set excluded directories for corresponding roots
	CRootDirectory* pRoot=m_pFirstRoot;
	
	while (pRoot!=NULL)
	{
		DWORD nRootLength=pRoot->m_Path.GetLength();
		if (pRoot->m_Path.LastChar()=='\\')
			nRootLength--;

		// Copying text to another buffer to get lowercase version
		char* pLowerRoot=new char[nRootLength+1];
		CopyMemory(pLowerRoot,LPCSTR(pRoot->m_Path),nRootLength);
		pLowerRoot[nRootLength]='\0';
		CharLower(pLowerRoot);

		BOOL bDeleteRoot=FALSE;

		for (int i=0;i<nExcludedDirectories;i++)
		{
			// Excluded path is shorter than root, excluded dir cannot be subdir of root
			if (pdwExcludedDirectoriesLen[i]<nRootLength)
				continue;

			// Beginning of paths are not same
			if (strncmp(pLowerRoot,ppExcludedDirectories[i],nRootLength)!=0)
				continue;
			
			if (nRootLength==pdwExcludedDirectoriesLen[i])
			{
				bDeleteRoot=TRUE;
				break;
			}
			
			if (ppExcludedDirectories[i][nRootLength]=='\\')
			{
				// Excluded directory is inside of root
				pRoot->m_aExcludedDirectories.Add(alloccopy(ppExcludedDirectories[i],
					pdwExcludedDirectoriesLen[i]));
			}
		}
		delete[] pLowerRoot;

		if (bDeleteRoot)
		{
			// Root is excluded, removing whole root
			if (pRoot==m_pFirstRoot)
			{
				m_pFirstRoot=pRoot->m_pNext;
				delete pRoot;
				pRoot=m_pFirstRoot;
			}
			else
			{
				// Resolving previous item
				CRootDirectory* pPrevRoot=m_pFirstRoot;
				while (pPrevRoot->m_pNext!=pRoot)
				pPrevRoot=pPrevRoot->m_pNext;
		
				pPrevRoot->m_pNext=pRoot->m_pNext;
				delete pRoot;
				pRoot=pPrevRoot->m_pNext;
			}
		}
		else // Next root
			pRoot=pRoot->m_pNext;
	}

	for (int i=0;i<nExcludedDirectories;i++)
		delete[] ppExcludedDirectories[i];
	delete[] ppExcludedDirectories;
	delete[] pdwExcludedDirectoriesLen;
}

WORD CDatabaseUpdater::GetProgressStatus() const
{
	return (WORD)(GetTickCount()%1000);
}