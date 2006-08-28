/* Copyright (c) 1997-2006 Janne Huttunen
   database updater v2.99.6.6040                 */

#include <HFCLib.h>
#include "Locatedb.h"

#ifndef WIN32
#include "dtype.h"
#include "fcntl.h"
#endif

CDatabaseUpdater::CDatabaseUpdater(LPCWSTR szDatabaseFile,LPCWSTR szAuthor,LPCWSTR szComment,
		LPCWSTR* pszRoots,DWORD nNumberOfRoots,UPDATEPROC pProc,DWORD_PTR dwParam)
:	m_pCurrentRoot(NULL),sStatus(statusInitializing),m_dwFiles(0),m_dwDirectories(0),
	m_pProc(pProc),m_dwData(dwParam),m_dwCurrentDatabase(DWORD(-1)),dbFile(NULL)
#ifdef WIN32	
	,m_hThread(NULL),m_lForceQuit(FALSE)
#endif
{
	DebugMessage("CDatabaseUpdater::CDatabaseUpdater(1)");

	m_aDatabases.Add(new DBArchive(szDatabaseFile,CDatabase::archiveFile,
		szAuthor,szComment,pszRoots,nNumberOfRoots,0,NULL,NULL,0));
}

CDatabaseUpdater::CDatabaseUpdater(const PDATABASE* ppDatabases,
		int nDatabases,UPDATEPROC pProc,DWORD_PTR dwParam)
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
		int nDatabases,UPDATEPROC pProc,WORD wThread,DWORD_PTR dwParam)
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
	DebugFormatMessage("CDatabaseUpdater::UpdatingProc() BEGIN this=%lX m_pProc=%lX",ULONG_PTR(this),ULONG_PTR(m_pProc));
	
	UpdateError ueResult=ueSuccess;

	class StopUpdating{};

#ifdef WIN32
	InterlockedExchange(&m_lForceQuit,FALSE);
#endif

	ASSERT(dbFile==NULL);

	for (m_dwCurrentDatabase=0;m_dwCurrentDatabase<DWORD(m_aDatabases.GetSize());m_dwCurrentDatabase++)
	{
		BOOL bUnicode=m_aDatabases[m_dwCurrentDatabase]->IsFlagged(DBArchive::Unicode);


		// Starting to scan database
		try {
			m_pCurrentRoot=m_aDatabases[m_dwCurrentDatabase]->m_pFirstRoot;
			
			DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): m_pCurrentDatabase=%lX szName=%s",ULONG_PTR(m_aDatabases[m_dwCurrentDatabase]),m_aDatabases[m_dwCurrentDatabase]->m_szName);
				
			m_pProc(m_dwData,StartedDatabase,ueResult,this);
			
			// Initilizating file and directory count
			m_dwFiles=0;
			m_dwDirectories=0;

			while (m_pCurrentRoot!=NULL
	#ifdef WIN32
				&& !m_lForceQuit
	#endif
				)
			{
				DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): m_pCurrentRoot=%lX path=%s",ULONG_PTR(m_pCurrentRoot),(LPCSTR)W2A(m_pCurrentRoot->m_Path));
							
				sStatus=statusScanning;
				m_pProc(m_dwData,RootChanged,ueResult,this);
				
				
				// Scannin root
				if (bUnicode)
					ueResult=m_pCurrentRoot->ScanRootW(m_lForceQuit);
				else
					ueResult=m_pCurrentRoot->ScanRoot(m_lForceQuit);

				if (ueResult==ueFolderUnavailable)
				{
					m_pProc(m_dwData,ErrorOccured,ueResult,this);
					if (m_aDatabases[m_dwCurrentDatabase]->IsFlagged(DBArchive::StopIfUnuavailable))
						throw ueFolderUnavailable; // Next database
				}
				else if (ueResult!=ueSuccess)
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
			
			
			BOOL bWriteHeader=TRUE;

			// Opening file
			switch (m_aDatabases[m_dwCurrentDatabase]->m_nArchiveType)
			{
			case CDatabase::archiveFile:
				if (m_aDatabases[m_dwCurrentDatabase]->IsFlagged(DBArchive::IncrementalUpdate))
				{
					DebugMessage("CDatabaseUpdater::UpdatingProc(): trying to open database for incremental update");
					
					dbFile=OpenDatabaseFileForIncrementalUpdate(
						m_aDatabases[m_dwCurrentDatabase]->m_szArchive,
						m_dwFiles,m_dwDirectories,bUnicode);
					
					DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): returns: %lX",(ULONG_PTR)dbFile);
					
					
					if (dbFile==(CFile*)-1)
					{
						dbFile=NULL;
						if (!m_pProc(m_dwData,ErrorOccured,ueCannotIncrement,this))
							throw ueStillWorking;
					}
					else if (dbFile==(CFile*)-2)
					{
						dbFile=NULL;
						if (!m_pProc(m_dwData,ErrorOccured,ueWrongCharset,this))
							throw ueStillWorking;
					}
					else if (dbFile!=NULL)
					{
						bWriteHeader=FALSE;
						break;
					}
				}
				
				if (dbFile==NULL)
				{
					DebugMessage("CDatabaseUpdater::UpdatingProc(): trying to open database");
					dbFile=new CFile(m_aDatabases[m_dwCurrentDatabase]->m_szArchive,
						CFile::defWrite|CFile::otherStrNullTerminated,TRUE);
				}
				break;
			default:
				throw CFileException(CFileException::notImplemented,
					-1,m_aDatabases[m_dwCurrentDatabase]->m_szArchive);
			}
			
			if (bWriteHeader)
			{
				//////////////////////////////////
				// Writing header
				DebugMessage("CDatabaseUpdater::UpdatingProc(): writing header");

		#ifdef WIN32
				// Writing identification, '\17=0x11=0x10|0x1' 0x1 = Long filenames and 0x10 = ANSI
				dbFile->Write("LOCATEDB20",10);
				dbFile->Write(BYTE(bUnicode?0x21:0x11));
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
				DWORD dwExtraSize1=1,dwExtraSize2=1;
				if (m_aDatabases[m_dwCurrentDatabase]->m_szExtra1!=NULL)
					dwExtraSize1+=(DWORD)istrlenw(m_aDatabases[m_dwCurrentDatabase]->m_szExtra1);
				if (m_aDatabases[m_dwCurrentDatabase]->m_szExtra2!=NULL)
					dwExtraSize2+=(DWORD)istrlenw(m_aDatabases[m_dwCurrentDatabase]->m_szExtra2);
				
				

				if (bUnicode)
				{
					// Writing header size
					dbFile->Write(DWORD(
						(m_aDatabases[m_dwCurrentDatabase]->m_sAuthor.GetLength()+1)*2+ // Author data
						(m_aDatabases[m_dwCurrentDatabase]->m_sComment.GetLength()+1)*2+ // Comments data
						dwExtraSize1*2+dwExtraSize2*2+ // Extra
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
					if (m_aDatabases[m_dwCurrentDatabase]->m_szExtra1!=NULL)
						dbFile->Write(m_aDatabases[m_dwCurrentDatabase]->m_szExtra1,dwExtraSize1*2);
					else
						dbFile->Write((WORD)0);

					if (m_aDatabases[m_dwCurrentDatabase]->m_szExtra2!=NULL)
						dbFile->Write(m_aDatabases[m_dwCurrentDatabase]->m_szExtra2,dwExtraSize2*2);
					else
						dbFile->Write((WORD)0);
				}
				else
				{
					// Writing header size
					dbFile->Write(DWORD(
						m_aDatabases[m_dwCurrentDatabase]->m_sAuthor.GetLength()+1+ // Author data
						m_aDatabases[m_dwCurrentDatabase]->m_sComment.GetLength()+1+ // Comments data
						dwExtraSize1+dwExtraSize2+ // Extra
						4+ // Time
						4+ // Number of files
						4  // Number of directories
						)
					);
					// Writing author
					dbFile->Write(W2A(m_aDatabases[m_dwCurrentDatabase]->m_sAuthor),
						(DWORD)m_aDatabases[m_dwCurrentDatabase]->m_sAuthor.GetLength()+1);
			
					// Writing comments
					dbFile->Write(W2A(m_aDatabases[m_dwCurrentDatabase]->m_sComment),
						(DWORD)m_aDatabases[m_dwCurrentDatabase]->m_sComment.GetLength()+1);
					
					// Writing free data
					if (m_aDatabases[m_dwCurrentDatabase]->m_szExtra1!=NULL)
						dbFile->Write(W2A(m_aDatabases[m_dwCurrentDatabase]->m_szExtra1,dwExtraSize1),dwExtraSize1);
					else
						dbFile->Write((BYTE)0);
					if (m_aDatabases[m_dwCurrentDatabase]->m_szExtra2!=NULL)
						dbFile->Write(W2A(m_aDatabases[m_dwCurrentDatabase]->m_szExtra2,dwExtraSize2),dwExtraSize2);
					else
						dbFile->Write((BYTE)0);
				}


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
			
				DebugMessage("CDatabaseUpdater::UpdatingProc(): writing header end");
			}

			DebugMessage("CDatabaseUpdater::UpdatingProc(): writing directory data");

			// Writing root directory datas
			m_pCurrentRoot=m_aDatabases[m_dwCurrentDatabase]->m_pFirstRoot;
			while (m_pCurrentRoot!=NULL && (ueResult==ueSuccess || ueResult==ueFolderUnavailable))
			{
				sStatus=statusWritingDB;
				
				// Writing root data
				if (bUnicode)
					ueResult=m_pCurrentRoot->WriteW(dbFile);
				else
					ueResult=m_pCurrentRoot->Write(dbFile);

				m_pCurrentRoot=m_pCurrentRoot->m_pNext;
			}

			DebugMessage("CDatabaseUpdater::UpdatingProc(): end of directory data");

			// End mark
			dbFile->Write((DWORD)0);

			delete dbFile;
			dbFile=NULL;

			DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): DB %lX OK",ULONG_PTR(m_aDatabases[m_dwCurrentDatabase]));
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

	sStatus=statusFinishing;
	m_dwCurrentDatabase=DWORD(-1);
	
	// Closing file if needed
	if (dbFile!=NULL)
	{
		delete dbFile;
		dbFile=NULL;
	}
	m_pProc(m_dwData,FinishedUpdating,ueResult,this);
	m_pProc(m_dwData,ClassShouldDelete,ueResult,this);

	// This class is deleted in the previous call, do not access this anymore

	DebugFormatMessage("CDatabaseUpdater::UpdatingProc(): ALL DONE ueResult=%d",DWORD(ueResult));

	return ueResult;
}

#ifdef WIN32
DWORD WINAPI CDatabaseUpdater::UpdateThreadProc(LPVOID lpParameter)
{
	DebugFormatMessage("CDatabaseUpdater::UpdateThreadProc thread=%lX",GetCurrentThreadId());
	return ((CDatabaseUpdater*)lpParameter)->UpdatingProc()==ueSuccess?0:1;	
}
#endif

// Start updating database
#ifdef WIN32
UpdateError CDatabaseUpdater::Update(BOOL bThreaded,int nThreadPriority)
{
	m_pProc(m_dwData,Initializing,ueSuccess,this);
	if (bThreaded)
	{
		DWORD dwThreadID;
		DebugFormatMessage("CDatabaseUpdater::Update this=%lX",ULONG_PTR(this));
		m_hThread=CreateThread(NULL,0,UpdateThreadProc,this,CREATE_SUSPENDED,&dwThreadID);
		
		if (m_hThread!=NULL)
		{
			SetThreadPriority(m_hThread,nThreadPriority);
			ResumeThread(m_hThread);
			return ueSuccess;
		}
		m_pProc(m_dwData,ErrorOccured,ueCannotCreateThread,this);
		return ueCannotCreateThread;
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
	MemCopyWtoA(szPath,(LPCWSTR)m_Path,m_Path.GetLength()+1);
	
	DebugMessage("CDatabaseUpdater::CRootDirectory::ScanRoot: started to scan root folder");
	UpdateError ueResult=ScanFolder(szPath,m_Path.GetLength(),lForceQuit);
	DebugMessage("CDatabaseUpdater::CRootDirectory::ScanRoot: ended to scan root folder");
	
	
	pCurrentBuffer->nLength=(DWORD)(pPoint-pCurrentBuffer->pData);

	DebugFormatMessage("CDatabaseUpdater::CRootDirectory::ScanRoot: END scanning root %s",m_Path);
	return ueResult;
}

UpdateError CDatabaseUpdater::CRootDirectory::ScanRootW(volatile LONG& lForceQuit)
{
	DebugFormatMessage("CDatabaseUpdater::CRootDirectory::ScanRoot: scanning root %s",m_Path);

	// Initializing buffer
	pCurrentBuffer=m_pFirstBuffer=new CBuffer;
	if (pCurrentBuffer==NULL)
		throw CException(CException::cannotAllocate);

	pPoint=*pCurrentBuffer;

	ASSERT(m_Path.GetLength()<=MAX_PATH);

	// Scanning folder
	WCHAR szPath[MAX_PATH+20];
	MemCopyW(szPath,(LPCWSTR)m_Path,m_Path.GetLength()+1);
	
	DebugMessage("CDatabaseUpdater::CRootDirectory::ScanRootW: started to scan root folder");
	UpdateError ueResult=ScanFolder(szPath,m_Path.GetLength(),lForceQuit);
	DebugMessage("CDatabaseUpdater::CRootDirectory::ScanRootW: ended to scan root folder");
	
	
	pCurrentBuffer->nLength=(DWORD)(pPoint-pCurrentBuffer->pData);

	DebugFormatMessage("CDatabaseUpdater::CRootDirectory::ScanRoot: END scanning root %s",m_Path);
	return ueResult;
}
#ifdef WIN32
typedef	HANDLE HFIND;
typedef WIN32_FIND_DATAA FIND_DATA;
typedef WIN32_FIND_DATAW FIND_DATAW;

#define VALID_HFIND(h)		((h)!=INVALID_HANDLE_VALUE)

inline HFIND _FindFirstFile(LPCSTR szFolder,FIND_DATA* fd)
{
	return FindFirstFileA(szFolder,fd);
}
inline HFIND _FindFirstFile(LPCWSTR szFolder,FIND_DATAW * fd)
{
	return FindFirstFileW(szFolder,fd);
}
inline BOOL _FindNextFile(HFIND hFind,FIND_DATA* fd)
{
	return FindNextFileA(hFind,fd);
}
inline BOOL _FindNextFile(HFIND hFind,FIND_DATAW* fd)
{
	return FindNextFileW(hFind,fd);
}
inline void _FindClose(HFIND hFind)
{
	FindClose(hFind);
}
inline BOOL _FindIsFolder(FIND_DATA* fd)
{
	return (fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY;
}
inline BOOL _FindIsFolder(FIND_DATAW* fd)
{
	return (fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY;
}
inline LPCSTR _FindGetName(FIND_DATA* fd)
{
	return fd->cFileName;
}
inline LPCWSTR _FindGetName(FIND_DATAW* fd)
{
	return fd->cFileName;
}

inline BYTE _FindGetAttribFlag(FIND_DATA* fd)
{
	return CDatabaseUpdater::GetAttribFlag(fd->dwFileAttributes);
}
inline BYTE _FindGetAttribFlag(FIND_DATAW* fd)
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


inline void _FindGetLastWriteDosDateTime(FIND_DATAW* fd,WORD* pwDate,WORD* pwTime)
{
	FILETIME ft;
	FileTimeToLocalFileTime(&fd->ftLastWriteTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,pwTime);
}
inline void _FindGetCreationDosDate(FIND_DATAW* fd,WORD* pwDate,WORD* pwTime)
{
	FILETIME ft;
	FileTimeToLocalFileTime(&fd->ftCreationTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,pwTime);
}
inline void _FindGetLastAccessDosDate(FIND_DATAW* fd,WORD* pwDate,WORD* pwTime)
{
	FILETIME ft;
	FileTimeToLocalFileTime(&fd->ftLastAccessTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,pwTime);
}

inline DWORD _FindGetFileSizeLo(FIND_DATA* fd)
{
	return fd->nFileSizeLow;
}
inline DWORD _FindGetFileSizeLo(FIND_DATAW* fd)
{
	return fd->nFileSizeLow;
}

inline DWORD _FindGetFileSizeHi(FIND_DATA* fd)
{
	return fd->nFileSizeHigh;
}
inline DWORD _FindGetFileSizeHi(FIND_DATAW* fd)
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
		WCHAR* pLowerFolder=alloccopyAtoW(szFolder,nLength);
		MakeLower(pLowerFolder);

		for (int i=0;i<m_aExcludedDirectories.GetSize();i++)
		{
			if (wcscmp(m_aExcludedDirectories[i],pLowerFolder)==0)
			{
				delete[] pLowerFolder;
				return ueSuccess;
			}
		}
		delete[] pLowerFolder;
	}	
	
	szFolder[nLength++]='\\';
	szFolder[nLength]='*';
	szFolder[nLength+1]='.';
	szFolder[nLength+2]='*';
	szFolder[nLength+3]='\0';

	

	FIND_DATA fd;
	
	HFIND hFind=_FindFirstFile(szFolder,&fd);
	

	if (!VALID_HFIND(hFind))
		return ueFolderUnavailable;

	for(;;)
	{
		if (_FindGetName(&fd)[0]=='.' && (_FindGetName(&fd)[1]=='\0' || _FindGetName(&fd)[1]=='.'))
		{
			if(!_FindNextFile(hFind,&fd))
				break;
			continue;
		}


		if (lForceQuit)
		{
			_FindClose(hFind);
			throw ueStopped;
		}

		if (m_aExcludeFilesPatternsA!=NULL)
		{
			BOOL bExcluded=FALSE;
			LPSTR* pPtr=m_aExcludeFilesPatternsA;
			while (*pPtr!=NULL)
			{
				if (ContainString(_FindGetName(&fd),*pPtr))
				{
					bExcluded=TRUE;
					break;
				}

				pPtr++;
			}

			if (bExcluded)
			{
				if(!_FindNextFile(hFind,&fd))
					break;
				continue;
			}				
		}

		DWORD sNameLength=istrlen(_FindGetName(&fd));
		ASSERT(sNameLength<256);
			

		if (_FindIsFolder(&fd))
		{
			// Get the length of directory name and checks that length is 
			// less than MAX_PATH, otherwise ignore
			if (nLength+sNameLength<MAX_PATH)
			{
				
				// Check whether new buffer is needed
				if (pPoint+_MAX_PATH+10>pCurrentBuffer->pData+BFSIZE)
				{
					// New buffer
					pCurrentBuffer->nLength=(DWORD)(pPoint-pCurrentBuffer->pData);
					pCurrentBuffer=pCurrentBuffer->pNext=new CBuffer;
					if (pCurrentBuffer==NULL)
						throw CException(CException::cannotAllocate);
					
					pPoint=*pCurrentBuffer;
				}
				
				*pPoint=UDBATTRIB_DIRECTORY|_FindGetAttribFlag(&fd); // Directory
				BYTE* pSizePointer=pPoint+1;
				pPoint+=5;

				
				

				// Set file name length
				*(pPoint++)=(BYTE)sNameLength;
				
				// Copy path
				sMemCopy(szFolder+nLength,_FindGetName(&fd),sNameLength);
				sMemCopy(pPoint,_FindGetName(&fd),sNameLength+1);

				// Move pointer
				pPoint+=sNameLength+1;


				// File time
				_FindGetLastWriteDosDateTime(&fd,(WORD*)pPoint,(WORD*)pPoint+1);
				_FindGetCreationDosDate(&fd,(WORD*)pPoint+2);
				_FindGetLastAccessDosDate(&fd,(WORD*)pPoint+3);
				pPoint+=sizeof(WORD)*4;
			
				ScanFolder(szFolder,nLength+sNameLength,lForceQuit);
				//szFolder[nLength+sTemp]='\0';
				*pPoint='\0'; // No more files and directories
				pPoint++;

				// Calculating directory data size
				if (pSizePointer>=pCurrentBuffer->pData && pSizePointer<pCurrentBuffer->pData+BFSIZE)
					((DWORD*)pSizePointer)[0]=(DWORD)(pPoint-pSizePointer);
				else
				{
					// Buffer has changed
					CBuffer* pTmp=m_pFirstBuffer;
					
					// old buffer to pTmp
					while (pSizePointer<pTmp->pData || pSizePointer>=pTmp->pData+BFSIZE)
						pTmp=pTmp->pNext;
					
					// Decreasing first buffer
					((LONG*)pSizePointer)[0]=(LONG)(pTmp->pData-pSizePointer);
					
					// Adding length between pCurrentbuffer and pTmp
					for (;pTmp!=pCurrentBuffer;pTmp=pTmp->pNext)
						((LONG*)pSizePointer)[0]+=pTmp->nLength;

					// Adding this buffer len
					((LONG*)pSizePointer)[0]+=(DWORD)(pPoint-pCurrentBuffer->pData);
				}

				// Increase directories count
				m_dwDirectories++;
			}
		}
		else
		{
			// File name
			
			if(nLength+sNameLength<MAX_PATH)
			{
				if (pPoint+MAX_PATH+15>pCurrentBuffer->pData+BFSIZE)
				{
					// New buffer
					pCurrentBuffer->nLength=(DWORD)(pPoint-pCurrentBuffer->pData);
					pCurrentBuffer=pCurrentBuffer->pNext=new CBuffer;
					pPoint=*pCurrentBuffer;
				}

				*(pPoint++)=UDBATTRIB_FILE|_FindGetAttribFlag(&fd); // File
				
				// File name length
				*(pPoint++)=(BYTE)sNameLength; 
				// Extension position (or 0 if no extension)
				for (*pPoint=BYTE(sNameLength)-1;*pPoint>0 && _FindGetName(&fd)[*pPoint]!='.';(*pPoint)--); 
				pPoint++;
				
				// Copying filename
				sMemCopy(pPoint,_FindGetName(&fd),sNameLength+1);
				pPoint+=sNameLength+1;
				
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
		
		if(!_FindNextFile(hFind,&fd))
			break;
	}
	_FindClose(hFind);

	return ueSuccess;
}


UpdateError CDatabaseUpdater::CRootDirectory::ScanFolder(LPWSTR szFolder,DWORD nLength,volatile LONG& lForceQuit)
{
	if (m_aExcludedDirectories.GetSize()>0)
	{
		// Checking whether directory is excluded
		WCHAR* pLowerFolder=alloccopy(szFolder,nLength);
		MakeLower(pLowerFolder);

		for (int i=0;i<m_aExcludedDirectories.GetSize();i++)
		{
			if (wcscmp(m_aExcludedDirectories[i],pLowerFolder)==0)
			{
				delete[] pLowerFolder;
				return ueSuccess;
			}
		}
		delete[] pLowerFolder;
	}	
	
	szFolder[nLength++]=L'\\';
	szFolder[nLength]=L'*';
	szFolder[nLength+1]=L'.';
	szFolder[nLength+2]=L'*';
	szFolder[nLength+3]=L'\0';

	

	FIND_DATAW fd;
	
	HFIND hFind=_FindFirstFile(szFolder,&fd);
	

	if (!VALID_HFIND(hFind))
		return ueFolderUnavailable;

	for(;;)
	{
		if (_FindGetName(&fd)[0]==L'.' && (_FindGetName(&fd)[1]==L'\0' || _FindGetName(&fd)[1]==L'.'))
		{
			if(!_FindNextFile(hFind,&fd))
				break;
			continue;
		}

		if (lForceQuit)
		{
			_FindClose(hFind);
			throw ueStopped;
		}


		if (m_aExcludeFilesPatternsW!=NULL)
		{
			BOOL bExcluded=FALSE;
			LPWSTR* pPtr=m_aExcludeFilesPatternsW;
			while (*pPtr!=NULL)
			{
				if (ContainString(_FindGetName(&fd),*pPtr))
				{
					bExcluded=TRUE;
					break;
				}

				pPtr++;
			}

			if (bExcluded)
			{
				if(!_FindNextFile(hFind,&fd))
					break;
				continue;
			}				
		}

		DWORD sNameLength=istrlenw(_FindGetName(&fd));
		ASSERT(sNameLength<256);

		
			

		if (_FindIsFolder(&fd))
		{
			// Get the length of directory name and checks that length is 
			// less than MAX_PATH, otherwise ignore
			if (nLength+sNameLength<MAX_PATH)
			{
				
				// Check whether new buffer is needed
				if (pPoint+MAX_PATH*2+14>pCurrentBuffer->pData+BFSIZE)
				{
					// New buffer
					pCurrentBuffer->nLength=(DWORD)(pPoint-pCurrentBuffer->pData);
					pCurrentBuffer=pCurrentBuffer->pNext=new CBuffer;
					if (pCurrentBuffer==NULL)
						throw CException(CException::cannotAllocate);
					
					pPoint=*pCurrentBuffer;
				}
				
				*pPoint=UDBATTRIB_DIRECTORY|_FindGetAttribFlag(&fd); // Directory
				BYTE* pSizePointer=pPoint+1;
				pPoint+=5;

				
				

				// Set file name length
				*(pPoint++)=(BYTE)sNameLength;
				
				// Copy path
				MemCopyW(szFolder+nLength,_FindGetName(&fd),sNameLength);
				MemCopyW(pPoint,_FindGetName(&fd),sNameLength+1);

				// Move pointer
				pPoint+=sNameLength*2+2;


				// File time
				_FindGetLastWriteDosDateTime(&fd,(WORD*)pPoint,(WORD*)pPoint+1);
				_FindGetCreationDosDate(&fd,(WORD*)pPoint+2,(WORD*)pPoint+3);
				_FindGetLastAccessDosDate(&fd,(WORD*)pPoint+4,(WORD*)pPoint+5);
				pPoint+=sizeof(WORD)*6;
			
				ScanFolder(szFolder,nLength+sNameLength,lForceQuit);
				//szFolder[nLength+sTemp]='\0';
				*pPoint='\0'; // No more files and directories
				pPoint++;

				// Calculating directory data size
				if (pSizePointer>=pCurrentBuffer->pData && pSizePointer<pCurrentBuffer->pData+BFSIZE)
					((DWORD*)pSizePointer)[0]=(DWORD)(pPoint-pSizePointer);
				else
				{
					// Buffer has changed
					CBuffer* pTmp=m_pFirstBuffer;
					
					// old buffer to pTmp
					while (pSizePointer<pTmp->pData || pSizePointer>=pTmp->pData+BFSIZE)
						pTmp=pTmp->pNext;
					
					// Decreasing first buffer
					((LONG*)pSizePointer)[0]=(LONG)(pTmp->pData-pSizePointer);
					
					// Adding length between pCurrentbuffer and pTmp
					for (;pTmp!=pCurrentBuffer;pTmp=pTmp->pNext)
						((LONG*)pSizePointer)[0]+=pTmp->nLength;

					// Adding this buffer len
					((LONG*)pSizePointer)[0]+=(DWORD)(pPoint-pCurrentBuffer->pData);
				}

				// Increase directories count
				m_dwDirectories++;
			}
		}
		else
		{
			// File name
			
			if(nLength+sNameLength<MAX_PATH)
			{
				if (pPoint+MAX_PATH*2+20>pCurrentBuffer->pData+BFSIZE)
				{
					// New buffer
					pCurrentBuffer->nLength=(DWORD)(pPoint-pCurrentBuffer->pData);
					pCurrentBuffer=pCurrentBuffer->pNext=new CBuffer;
					pPoint=*pCurrentBuffer;
				}

				*(pPoint++)=UDBATTRIB_FILE|_FindGetAttribFlag(&fd); // File
				
				// File name length
				*(pPoint++)=(BYTE)sNameLength; 
				// Extension position (or 0 if no extension)
				for (*pPoint=BYTE(sNameLength)-1;*pPoint>0 && _FindGetName(&fd)[*pPoint]!='.';(*pPoint)--); 
				pPoint++;

				// Copying filename
				MemCopyW(pPoint,_FindGetName(&fd),sNameLength+1);
				pPoint+=sNameLength*2+2;
				
				// File size
				*((DWORD*)pPoint)=_FindGetFileSizeLo(&fd);
				*((WORD*)(pPoint+4))=(WORD)_FindGetFileSizeHi(&fd);
				pPoint+=6;

				// File time
				_FindGetLastWriteDosDateTime(&fd,(WORD*)pPoint,(WORD*)pPoint+1);
				_FindGetCreationDosDate(&fd,(WORD*)pPoint+2,(WORD*)pPoint+3);
				_FindGetLastAccessDosDate(&fd,(WORD*)pPoint+4,(WORD*)pPoint+4);
				
				pPoint+=sizeof(WORD)*6;
				
				// Increase files count
				m_dwFiles++;
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
	switch (FileSystem::GetDriveType(sPath+'\\'))
	{
	case DRIVE_UNKNOWN:
		return 0x00;
	case DRIVE_NO_ROOT_DIR:
		return 0xF0;
	case DRIVE_REMOVABLE:
		return 0x20;
	case DRIVE_FIXED:
		return 0x10;
	case DRIVE_REMOTE:
		return 0x40;
	case DRIVE_CDROM:
		return 0x30;
	case DRIVE_RAMDISK:
		return 0x50;
	}
	return 0;
}

inline BYTE _GetDriveType(CStringW& sPath)
{
	switch (FileSystem::GetDriveType(sPath+L'\\'))
	{
	case DRIVE_UNKNOWN:
		return 0x00;
	case DRIVE_NO_ROOT_DIR:
		return 0xF0;
	case DRIVE_REMOVABLE:
		return 0x20;
	case DRIVE_FIXED:
		return 0x10;
	case DRIVE_REMOTE:
		return 0x40;
	case DRIVE_CDROM:
		return 0x30;
	case DRIVE_RAMDISK:
		return 0x50;
	}
	return 0;
}
inline BYTE _GetDriveTypeW(CStringW& sPath)
{
	switch (GetDriveTypeW(sPath+L'\\'))
	{
	case DRIVE_UNKNOWN:
		return 0x00;
	case DRIVE_NO_ROOT_DIR:
		return 0xF0;
	case DRIVE_REMOVABLE:
		return 0x20;
	case DRIVE_FIXED:
		return 0x10;
	case DRIVE_REMOTE:
		return 0x40;
	case DRIVE_CDROM:
		return 0x30;
	case DRIVE_RAMDISK:
		return 0x50;
	}
	return 0;
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
		if (m_Path[0]!=L'\\')
		{
			char szDrive[4]="X:\\";
			szDrive[0]=W2Ac(m_Path[0]);
			nRet=GetVolumeInformation(szDrive,sVolumeName.GetBuffer(20),20,&dwSerial,
				&dwTemp,&dwTemp,sFSName.GetBuffer(20),20);
		}
		else // network, UNC path
			nRet=GetVolumeInformation(W2A(m_Path),sVolumeName.GetBuffer(20),20,&dwSerial,
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
		+(DWORD)m_Path.GetLength()+1 // Path
		+(DWORD)sVolumeName.GetLength()+1 // Volume name
		+sizeof(DWORD)	//Serial
		+(DWORD)sFSName.GetLength()+1 // Filesystem
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
	dbFile->Write(W2A(m_Path));

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

UpdateError CDatabaseUpdater::CRootDirectory::WriteW(CFile* dbFile)
{
	CStringW sVolumeName;
	CStringW sFSName;

	DWORD dwSerial=0;
	BYTE nType;
	
	{
		if (m_Path.GetLength()==2 || (m_Path[0]==L'\\' && m_Path[1]==L'\\'))
			nType=_GetDriveTypeW(m_Path);
		else
			nType=0xF0;

		// resolving label,fs and serial
		// Resolving information
		DWORD dwTemp;
		UINT nOldMode=SetErrorMode(SEM_FAILCRITICALERRORS);
		BOOL nRet;
		if (m_Path[0]!=L'\\')
		{
			WCHAR szDrive[4]=L"X:\\";
			szDrive[0]=m_Path[0];
			nRet=GetVolumeInformationW(szDrive,sVolumeName.GetBuffer(20),20,&dwSerial,
				&dwTemp,&dwTemp,sFSName.GetBuffer(20),20);
		}
		else // network, UNC path
			nRet=GetVolumeInformationW(m_Path,sVolumeName.GetBuffer(20),20,&dwSerial,
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
	}

	// Calculating data length
	DWORD nLength=1 // Type
		+(DWORD)((m_Path.GetLength()+1)*2) // Path
		+(DWORD)((sVolumeName.GetLength()+1)*2) // Volume name
		+sizeof(DWORD)	//Serial
		+(DWORD)((sFSName.GetLength()+1)*2) // Filesystem
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
	m_nArchiveType(pDatabase->GetArchiveType()),m_pFirstRoot(NULL),m_nFlags(0),
	m_szExtra1(NULL),m_szExtra2(NULL),m_aExcludeFilesPatternsA(NULL)
{
	m_szArchive=alloccopy(pDatabase->GetArchiveName());
	m_dwNameLength=(DWORD)wcslen(pDatabase->GetName());
	m_szName=alloccopy(pDatabase->GetName(),m_dwNameLength);

	// Retrieving flags
	if (pDatabase->IsFlagged(CDatabase::flagStopIfRootUnavailable))
		m_nFlags|=StopIfUnuavailable;
	if (pDatabase->IsFlagged(CDatabase::flagIncrementalUpdate))
		m_nFlags|=IncrementalUpdate;
	if (IsUnicodeSystem() && !pDatabase->IsFlagged(CDatabase::flagAnsiCharset))
		m_nFlags|=Unicode;

	LPCWSTR pPtr=pDatabase->GetRoots();
	if (pPtr==NULL)
	{
#ifdef WIN32
		WCHAR drive[3]=L"X:";
		CRootDirectory* tmp;
			
		DWORD dwBufferLen=GetLogicalDriveStrings(0,NULL)+1;
		WCHAR* szDrives=new WCHAR[dwBufferLen];
		FileSystem::GetLogicalDriveStrings(dwBufferLen,szDrives);

		for (int i=0;szDrives[i*4]!=L'\0';i++)
		{
			if (FileSystem::GetDriveType(szDrives+i*4)==DRIVE_FIXED)
			{
				drive[0]=szDrives[i*4];
				
				if (m_pFirstRoot==NULL)
					tmp=m_pFirstRoot=new CRootDirectory(drive);
				else
					tmp=tmp->m_pNext=new CRootDirectory(drive);
			}
		}
		delete[] szDrives;




#else
		char drive[3]="X:";
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
		delete[] szDrives;
#endif
        
		if (m_pFirstRoot!=NULL)
			tmp->m_pNext=NULL;
		
		
	}
	else
	{
		CRootDirectory* tmp;
			
		while (*pPtr!=L'\0')
		{
			DWORD dwLength=(DWORD)istrlenw(pPtr);

			if ((dwLength==2 || dwLength==3) && pPtr[1]==L':') // Root is drive
			{
				WCHAR root[]=L"X:\\";
				root[0]=pPtr[0];
				if (m_pFirstRoot==NULL)
					tmp=m_pFirstRoot=new CRootDirectory(root,dwLength);
				else
					tmp=tmp->m_pNext=new CRootDirectory(root,dwLength);
			}
			else if (pPtr[0]=='\\' && pPtr[1]=='\\')
			{
				if (FileSystem::IsDirectory(pPtr))
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
					DWORD dwEntries=-1,cbBuffer=16384,dwRet;
					HANDLE hEnum;
					
					if (IsUnicodeSystem())
					{
						NETRESOURCEW nr;
						nr.dwScope=RESOURCE_CONNECTED;
						nr.dwType=RESOURCETYPE_DISK;
						nr.dwDisplayType=RESOURCEDISPLAYTYPE_SHARE;
						nr.dwUsage=RESOURCEUSAGE_CONTAINER;
						nr.lpLocalName=NULL;
						nr.lpRemoteName=const_cast<LPWSTR>(pPtr);
						nr.lpComment=NULL;
						nr.lpProvider=NULL;
						dwRet=WNetOpenEnumW(RESOURCE_GLOBALNET,RESOURCETYPE_DISK,0,&nr,&hEnum);
					}
					else
					{
						NETRESOURCE nr;
						nr.dwScope=RESOURCE_CONNECTED;
						nr.dwType=RESOURCETYPE_DISK;
						nr.dwDisplayType=RESOURCEDISPLAYTYPE_SHARE;
						nr.dwUsage=RESOURCEUSAGE_CONTAINER;
						nr.lpLocalName=NULL;
						nr.lpRemoteName=alloccopyWtoA(pPtr);
						nr.lpComment=NULL;
						nr.lpProvider=NULL;
						dwRet=WNetOpenEnumA(RESOURCE_GLOBALNET,RESOURCETYPE_DISK,0,&nr,&hEnum);
						delete[] nr.lpRemoteName;
					}

					if (dwRet!=NOERROR)
					{
						if (m_pFirstRoot==NULL)
							tmp=m_pFirstRoot=new CRootDirectory(pPtr,dwLength);
						else
							tmp=tmp->m_pNext=new CRootDirectory(pPtr,dwLength);

						pPtr+=dwLength+1;
						continue;
					}


					if (IsUnicodeSystem())
					{
						NETRESOURCEW* nro;
							
						for(;;)
						{
							nro=(NETRESOURCEW*)GlobalAlloc(GPTR,cbBuffer);
							dwRet=WNetEnumResourceW(hEnum,&dwEntries,nro,&cbBuffer);
							if (dwRet==ERROR_NO_MORE_ITEMS)
								break;

							if (dwRet!=ERROR_MORE_DATA && dwRet!=NOERROR)
							{
								if (m_pFirstRoot==NULL)
									tmp=m_pFirstRoot=new CRootDirectory(pPtr,dwLength);
								else
									tmp=tmp->m_pNext=new CRootDirectory(pPtr,dwLength);

								break;
							}

							for (DWORD i=0;i<dwEntries;i++)
							{
								if (m_pFirstRoot==NULL)
									tmp=m_pFirstRoot=new CRootDirectory(nro[i].lpRemoteName);
								else
									tmp=tmp->m_pNext=new CRootDirectory(nro[i].lpRemoteName);
							}
							GlobalFree((HGLOBAL)nro);
						}
					}
					else
					{
						NETRESOURCE* nro;
							
						for(;;)
						{
							nro=(NETRESOURCE*)GlobalAlloc(GPTR,cbBuffer);
							dwRet=WNetEnumResource(hEnum,&dwEntries,nro,&cbBuffer);
							if (dwRet==ERROR_NO_MORE_ITEMS)
								break;

							if (dwRet!=ERROR_MORE_DATA && dwRet!=NOERROR)
							{
								if (m_pFirstRoot==NULL)
									tmp=m_pFirstRoot=new CRootDirectory(pPtr,dwLength);
								else
									tmp=tmp->m_pNext=new CRootDirectory(pPtr,dwLength);

								break;
							}

							for (DWORD i=0;i<dwEntries;i++)
							{
								if (m_pFirstRoot==NULL)
									tmp=m_pFirstRoot=new CRootDirectory(A2W(nro[i].lpRemoteName));
								else
									tmp=tmp->m_pNext=new CRootDirectory(A2W(nro[i].lpRemoteName));
							}
							GlobalFree((HGLOBAL)nro);
						}
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


	// Resolve expected directories and files
	CDatabaseInfo::ReadFilesAndDirectoriesCount(m_nArchiveType,m_szArchive,
		m_dwExpectedFiles,m_dwExpectedDirectories);

	// Excluded directories
	ParseExcludedFilesAndDirectories(pDatabase->GetExcludedFiles(),
		pDatabase->GetExcludedDirectories().GetData(),pDatabase->GetExcludedDirectories().GetSize());
	
	m_szExtra2=pDatabase->ConstructExtraBlock();
}
		
CDatabaseUpdater::DBArchive::DBArchive(LPCWSTR szArchiveName,CDatabase::ArchiveType nArchiveType,
											  LPCWSTR szAuthor,LPCWSTR szComment,LPCWSTR* pszRoots,DWORD nNumberOfRoots,BYTE nFlags,
											  LPCWSTR szExcludedFiles,LPCWSTR* ppExcludedDirectories,int nExcludedDirectories)
:	m_sAuthor(szAuthor),m_sComment(szComment),m_nArchiveType(nArchiveType),
	m_szName(NULL),m_dwNameLength(0),m_nFlags(nFlags),
	m_szExtra1(NULL),m_szExtra2(NULL),m_aExcludeFilesPatternsA(NULL)
{
	m_szArchive=alloccopy(szArchiveName);
	
	if (!IsUnicodeSystem())
		m_nFlags&=~Unicode;

	if (nNumberOfRoots==0)
	{
		m_pFirstRoot=NULL;
		return;
	}

	CRootDirectory* tmp=m_pFirstRoot=new CRootDirectory(pszRoots[0]);
	for (DWORD i=1;i<nNumberOfRoots;i++)
		tmp=tmp->m_pNext=new CRootDirectory(pszRoots[i]);
	tmp->m_pNext=NULL;

	ParseExcludedFilesAndDirectories(szExcludedFiles,ppExcludedDirectories,nExcludedDirectories);

	CDatabaseInfo::ReadFilesAndDirectoriesCount(m_nArchiveType,m_szArchive,
		m_dwExpectedFiles,m_dwExpectedDirectories);

	
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

	if (m_szExtra1!=NULL)
		delete[] m_szExtra1;
	if (m_szExtra2!=NULL)
		delete[] m_szExtra2;

	if (m_aExcludeFilesPatternsA!=NULL)
	{
		if (m_nFlags&Unicode)
		{
			for (int i=0;m_aExcludeFilesPatternsW[i]!=NULL;i++)
				delete[] m_aExcludeFilesPatternsW[i];
			delete[] m_aExcludeFilesPatternsW;
		}
		else
		{
			for (int i=0;m_aExcludeFilesPatternsA[i]!=NULL;i++)
				delete[] m_aExcludeFilesPatternsA[i];
			delete[] m_aExcludeFilesPatternsA;
		}
	}
}


void CDatabaseUpdater::DBArchive::ParseExcludedFilesAndDirectories(LPCWSTR szExcludedFiles,
																   const LPCWSTR* ppExcludedDirs,
																   int nExcludedDirectories)
{
	// Parsing files
	if (szExcludedFiles!=NULL)
	{
		if (szExcludedFiles[0]!='\0')
		{
			// Counting
			DWORD nPatterns=1;
			LPCWSTR pPtr;

			for (pPtr=szExcludedFiles;*pPtr!='\0';pPtr++)
			{
				if (*pPtr==';' || *pPtr==',')
					nPatterns++;
			}

			if (m_nFlags&Unicode)
				m_aExcludeFilesPatternsW=new WCHAR*[nPatterns+1];
			else
				m_aExcludeFilesPatternsA=new CHAR*[nPatterns+1];

			nPatterns=0;
			pPtr=szExcludedFiles;

			for (;;)
			{
				DWORD nLength;
				for (nLength=0;pPtr[nLength]!='\0' && pPtr[nLength]!=';' && pPtr[nLength]!=',';nLength++);

				if (m_nFlags&Unicode)
					m_aExcludeFilesPatternsW[nPatterns]=alloccopy(pPtr,nLength);
				else
					m_aExcludeFilesPatternsA[nPatterns]=alloccopyWtoA(pPtr,nLength);
				nPatterns++;

				pPtr+=nLength;

				if (*pPtr=='\0')
					break;

				pPtr++;
			}

			m_aExcludeFilesPatternsA[nPatterns]=NULL;
		}
	}

	if (nExcludedDirectories==0)
	{
		// Just place excluded files and exit
		if (m_aExcludeFilesPatternsA!=NULL)
		{
			CRootDirectory* pRoot=m_pFirstRoot;
			while (pRoot!=NULL)
			{
				pRoot->m_aExcludeFilesPatternsA=m_aExcludeFilesPatternsA;
				pRoot=pRoot->m_pNext;
			}
		}
		return;
	}

	// Parsing directories
	// First, check that directory names are valid
	LPWSTR* ppExcludedDirectories=new LPWSTR[max(nExcludedDirectories,2)];
	DWORD* pdwExcludedDirectoriesLen=new DWORD[max(nExcludedDirectories,2)];
	int i,j;
	for (i=0,j=0;i<nExcludedDirectories;i++)
	{
		if (ppExcludedDirs[i][0]==L'\0')
			continue;
		if (ppExcludedDirs[i][1]==L'\0')
			continue;

		if (ppExcludedDirs[i][1]==L':' && ppExcludedDirs[i][2]==L'\0')
		{
			ppExcludedDirectories[j]=new WCHAR[3];
			ppExcludedDirectories[j][0]=ppExcludedDirs[i][0];
			ppExcludedDirectories[j][1]=':';
			ppExcludedDirectories[j][2]='\0';
			MakeLower(ppExcludedDirectories[j]);
			
			pdwExcludedDirectoriesLen[j]=2;
		
			j++;
			continue;
		}


		WCHAR szBuffer[400];
		int nRet=FileSystem::GetFullPathName(ppExcludedDirs[i],400,szBuffer,NULL);

		if (!nRet)
			continue;

		if (szBuffer[nRet-1]==L'\\')
			szBuffer[--nRet]=L'\0';

		MakeLower(szBuffer);
		pdwExcludedDirectoriesLen[j]=nRet;
		ppExcludedDirectories[j++]=alloccopy(szBuffer,nRet);
	}
	nExcludedDirectories=j;



	// Then set excluded files and directories for roots
	CRootDirectory* pRoot=m_pFirstRoot;
	
	while (pRoot!=NULL)
	{
		DWORD nRootLength=(DWORD)pRoot->m_Path.GetLength();
		if (pRoot->m_Path.LastChar()=='\\')
			nRootLength--;

		// Copying text to another buffer to get lowercase version
		WCHAR* pLowerRoot=alloccopy(pRoot->m_Path,nRootLength);
		MakeLower(pLowerRoot);

		BOOL bDeleteRoot=FALSE;

		for (int i=0;i<nExcludedDirectories;i++)
		{
			// Excluded path is shorter than root, excluded dir cannot be subdir of root
			if (pdwExcludedDirectoriesLen[i]<nRootLength)
				continue;

			// Beginning of paths are not same
			if (wcsncmp(pLowerRoot,ppExcludedDirectories[i],nRootLength)!=0)
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
			continue;
		}
		
		// Inserting excluded files pattern
		pRoot->m_aExcludeFilesPatternsA=m_aExcludeFilesPatternsA;
			
		// Next root
		pRoot=pRoot->m_pNext;
	}

	for (int i=0;i<nExcludedDirectories;i++)
		delete[] ppExcludedDirectories[i];
	delete[] ppExcludedDirectories;
	delete[] pdwExcludedDirectoriesLen;
}

CFile* CDatabaseUpdater::OpenDatabaseFileForIncrementalUpdate(LPCWSTR szArchive,DWORD dwFiles,DWORD dwDirectories,BOOL bUnicode)
{
	DebugFormatMessage("CDatabaseUpdater::OpenDatabaseFileForIncrementalUpdate(): BEGIN, archive='%s'",szArchive);
	
	CFile* dbFile=NULL;
	
	try {
		dbFile=new CFile(szArchive,
			CFile::modeReadWrite|CFile::openExisting|CFile::shareDenyWrite|CFile::shareDenyRead|CFile::otherStrNullTerminated,TRUE);

		if (dbFile==NULL)
		{
			// Cannot open file, incremental update not possible
			return NULL;
		}
	
		dbFile->SeekToBegin();
	
		// Reading and verifing header
		char szBuffer[11];
		dbFile->Read(szBuffer,11);

		if (szBuffer[0]!='L' || szBuffer[1]!='O' || 
			szBuffer[2]!='C' || szBuffer[3]!='A' || 
			szBuffer[4]!='T' || szBuffer[5]!='E' || 
			szBuffer[6]!='D' || szBuffer[7]!='B' ||
			szBuffer[8]!='2' || szBuffer[9]!='0' )
		{
			throw CFileException(CFileException::invalidFile,
#ifdef WIN32
				-1,
#endif
				szArchive);
		}

		if ((bUnicode && !(szBuffer[10]&0x20)) ||
			(!bUnicode && (szBuffer[10]&0x20)))
		{
			throw CFileException(CFileException::invalidFormat,
#ifdef WIN32
				-1,
#endif
				szArchive);

		}

	
		// Updating file and directory counts
		DWORD dwBlockSize;
		dbFile->Read(dwBlockSize);
		dbFile->Seek(dwBlockSize-2*sizeof(DWORD),CFile::current);
		
		DWORD dwTemp1,dwTemp2;
		dbFile->Read(dwTemp1);
		dbFile->Read(dwTemp2);

		dwTemp1+=dwFiles;
		dwTemp2+=dwDirectories;
		dbFile->Seek(-2*LONG(sizeof(DWORD)),CFile::current);

		dbFile->Write(dwTemp1);
		dbFile->Write(dwTemp2);

		// Searching enf of file
		dbFile->Read(dwBlockSize);
		while (dwBlockSize>0)
		{
			dbFile->Seek(dwBlockSize,CFile::current);
			dbFile->Read(dwBlockSize);
		}


		// Now we should be in the end of file
		ASSERT(dbFile->IsEndOfFile());

		dbFile->Seek(-LONG(sizeof(DWORD)),CFile::current);
	}
	catch (CFileException fe)
	{
		DebugFormatMessage("CDatabaseUpdater::OpenDatabaseFileForIncrementalUpdate(): fe.m_cause=%d",fe.m_cause);
			
		if (dbFile!=NULL)
			delete dbFile;
		switch (fe.m_cause)
		{
		case CFileException::invalidFile:
		case CFileException::sharingViolation:
		case CFileException::endOfFile:
		case CFileException::writeProtected:
		case CFileException::writeFault:
		case CFileException::lockViolation:
		case CFileException::accessDenied:
		case CFileException::readFault:
			DebugMessage("CDatabaseUpdater::OpenDatabaseFileForIncrementalUpdate(): END2");
			return (CFile*)-1;
		case CFileException::invalidFormat:
			DebugMessage("CDatabaseUpdater::OpenDatabaseFileForIncrementalUpdate(): END4");
			return (CFile*)-2;
		default:
			DebugMessage("CDatabaseUpdater::OpenDatabaseFileForIncrementalUpdate(): END3");
			return NULL;
		}
	}
	catch (...)
	{
		DebugFormatMessage("CDatabaseUpdater::OpenDatabaseFileForIncrementalUpdate(): caugh unknown exception");

		if (dbFile!=NULL)
			delete dbFile;
	
		DebugMessage("CDatabaseUpdater::OpenDatabaseFileForIncrementalUpdate(): END4");
		return NULL;
	}

	DebugFormatMessage("CDatabaseUpdater::OpenDatabaseFileForIncrementalUpdate(): END, will return='%X'",dbFile);
	return dbFile;
}

