/* Copyright (c) 1997-2005 Janne Huttunen
   database updater v2.99.5.7100                */

#if !defined(DATABASEUPDATER_H)
#define DATABASEUPDATER_H

#if _MSC_VER >= 1000
#pragma once
#endif



class CLocater;
class CDatabaseUpdater;

typedef BOOL (CALLBACK* UPDATEPROC)(DWORD dwParam,CallingReason crReason,UpdateError ueCode,CDatabaseUpdater* pUpdater);

#define BFSIZE 200000


// Maybe usefull
#define IS_UPDATER_EXITED(ptr)		((((DWORD)(ptr))&0xFFFF0000)==0xFFFF0000?TRUE:FALSE)
#define GET_UPDATER_CODE(ptr)		(IS_UPDATER_EXITED(ptr)?(UpdateError)(~((DWORD)(ptr))):ueStillWorking)
#define UPDATER_EXITED(code)		((CDatabaseUpdater*)(~((DWORD)(code))))

// String copyers
#define sMemCopy(dst,src,len)	CopyMemory(dst,src,len)
#define sMemZero(dst,len)		ZeroMemory(dst,len)
#define sMemSet(dst,val,len)	iMemSet(dst,val,len)
#define sstrlen(str,len)		dstrlen(str,len)

#define sMemCopyW				iMemCopyW
#define sstrlenW				dwstrlen


class CDatabaseUpdater
{
public:
	class DBArchive;

	class CRootDirectory
	{
	private:
		CRootDirectory(LPCSTR szPath);
		CRootDirectory(LPCSTR szPath,int iPathLen);
		
		~CRootDirectory();
	
		UpdateError ScanRoot(volatile LONG& lForceQuit);
		UpdateError ScanFolder(LPSTR szFolder,DWORD nFolderNameLength,volatile LONG& lForceQuit);
		UpdateError Write(CFile* dbFile);

	public:
		CString m_Path;
		DWORD m_dwFiles;
		DWORD m_dwDirectories;
	
	private:
		class CBuffer
		{
		public:
			CBuffer() { pNext=NULL; pData=new BYTE[BFSIZE]; }
			~CBuffer() { delete[] pData; }

		public:
			operator BYTE*() {return pData; }
			
			BYTE* pData;
			DWORD nLength;
			CBuffer* pNext;
		};
		CBuffer *m_pFirstBuffer,*pCurrentBuffer;
		BYTE* pPoint;
		

		// Excluded directories
		CArray<LPSTR> m_aExcludedDirectories; 

		friend CDatabaseUpdater;
		friend DBArchive;
	public:
		CRootDirectory* m_pNext;

	};

	class DBArchive {
	public:
		DBArchive();
		DBArchive(LPCSTR szArchiveName,CDatabase::ArchiveType nArchiveType,
			LPCSTR szAuthor,LPCSTR szComment,LPCSTR* pszRoots,DWORD nNumberOfRoots,BYTE nFlags,
			LPCSTR* ppExcludedDirectories,int nExcludedDirectories);
		DBArchive(const CDatabase* pDatabase);
		~DBArchive();

	public:
		enum DBFlags {
			StopIfUnuavailable = 0x1, // Stops updating if root is unavailable
			IncrementalUpdate = 0x2 // Incremental update
		};

		BOOL IsFlagged(DBFlags flag);
		void SetFlag(DBFlags flag,BOOL bSet=TRUE);
		void ParseExcludedDirectories(const LPCSTR* ppExcludedDirectories,int nExcludedDirectories);
		

	protected:

		LPSTR m_szArchive;
		CDatabase::ArchiveType m_nArchiveType;
		
		LPSTR m_szName;
		DWORD m_dwNameLength;

		CString m_sAuthor;
		CString m_sComment;
		CRootDirectory* m_pFirstRoot;

		BYTE m_nFlags;

		friend CDatabaseUpdater;

		LPSTR m_szExtra1;
		LPSTR m_szExtra2;
		
		// For progress estimation
		DWORD m_dwExpectedDirectories;
		DWORD m_dwExpectedFiles;


	};

protected:
	// Thread procs
#ifdef WIN32
	static DWORD WINAPI UpdateThreadProc(LPVOID lpParameter);
#endif
	UpdateError UpdatingProc();

public:
	CDatabaseUpdater(LPCSTR szDatabaseFile,LPCSTR szAuthor,LPCSTR szComment,
		LPCSTR* pszRoots,DWORD nNumberOfRoots,
		UPDATEPROC pProc,DWORD dwParam=0);	
					// Scan strings of pszRoots

	CDatabaseUpdater(const PDATABASE* ppDatabases,
		int nDatabases,UPDATEPROC pProc,DWORD dwParam=0);
	
	// Takes databases with thread ID wThread
	CDatabaseUpdater(const PDATABASE* ppDatabases,
		int nDatabases,UPDATEPROC pProc,WORD wThread,DWORD dwParam=0);

	~CDatabaseUpdater();
	
	void SetExtras(LPCSTR szExtra1,LPCSTR szExtra2);

#ifdef WIN32
	UpdateError Update(BOOL bThreaded=FALSE);

	BOOL StopUpdating(BOOL bForce=TRUE);
	void CouldStop();

#else
	UpdateError Update();
#endif

	const CRootDirectory* GetCurrentRoot() const;
	LPCSTR GetCurrentRootPath() const;
	LPSTR GetCurrentRootPathStr() const;

	BOOL EnumDatabases(int iDatabase,LPCSTR& szName,LPCSTR& szFile,CDatabase::ArchiveType& nArchiveType,CRootDirectory*& pFirstRoot);
	
	const LPCSTR GetCurrentDatabaseName() const;
	const LPCSTR GetCurrentDatabaseFile() const;
	LPSTR GetCurrentDatabaseNameStr() const;
	
	DWORD GetNumberOfDatabases() const;
	DWORD GetCurrentDatabase() const;
	WORD GetProgressStatus() const; // Estimated progress status between 0-1000

	BOOL IsIncrementUpdate() const;

#ifdef WIN32
	static BYTE GetAttribFlag(DWORD dwAttribs);
#endif

private:

	static CFile* OpenDatabaseFileForIncrementalUpdate(LPCSTR szArchive,DWORD dwFiles,DWORD dwDirectories);

	CArrayFP<DBArchive*> m_aDatabases;
	DWORD m_dwCurrentDatabase;

	DWORD m_dwFiles;
	DWORD m_dwDirectories;
	

	

private:
	CFile* dbFile;
	UPDATEPROC m_pProc;
	DWORD m_dwData;
#ifdef WIN32
	HANDLE m_hThread;
	volatile LONG m_lForceQuit;
#endif

	
	CRootDirectory* m_pCurrentRoot;

	friend CRootDirectory;

public:
	// Status flags (statusNotChanged is not by CDatabaseUpdater)
	enum UpdateStatus {
		statusNotChanged,
		statusInitializing,
		statusScanning,
		statusWritingDB,
		statusFinishing
	};	
	UpdateStatus GetStatus() const { return sStatus; }
private:
	UpdateStatus sStatus;
};

typedef CDatabaseUpdater* PDATABASEUPDATER;

inline CDatabaseUpdater::CRootDirectory::CRootDirectory(LPCSTR szPath)
:	m_Path(szPath),m_dwFiles(0),m_dwDirectories(0),m_pFirstBuffer(NULL)
{
}

inline CDatabaseUpdater::CRootDirectory::CRootDirectory(LPCSTR szPath,int iLength)
:	m_Path(szPath,iLength),m_dwFiles(0),m_dwDirectories(0),m_pFirstBuffer(NULL)
{
}

inline CDatabaseUpdater::DBArchive::DBArchive()
:	m_szArchive(NULL),m_nArchiveType(CDatabase::archiveFile),m_pFirstRoot(NULL),m_nFlags(0),
	m_dwExpectedDirectories(DWORD(-1)),m_dwExpectedFiles(DWORD(-1)),
	m_szExtra1(NULL),m_szExtra2(NULL)
{
}

inline BOOL CDatabaseUpdater::DBArchive::IsFlagged(DBFlags flag)
{
	return m_nFlags&flag;
}

inline void CDatabaseUpdater::DBArchive::SetFlag(DBFlags flag,BOOL bSet)
{
	if (bSet)
		m_nFlags|=flag;
	else
		m_nFlags&=~BYTE(flag);
}


#ifdef WIN32
inline void CDatabaseUpdater::CouldStop()
{
	InterlockedExchange(&m_lForceQuit,TRUE);
}

inline BYTE CDatabaseUpdater::GetAttribFlag(DWORD dwAttribs)
{
	BYTE bRet=0;
	if (dwAttribs&FILE_ATTRIBUTE_ARCHIVE)
		bRet|=UDBATTRIB_ARCHIVE;
	if (dwAttribs&FILE_ATTRIBUTE_HIDDEN)
		bRet|=UDBATTRIB_HIDDEN;
	if (dwAttribs&FILE_ATTRIBUTE_READONLY)
		bRet|=UDBATTRIB_READONLY;
	if (dwAttribs&FILE_ATTRIBUTE_SYSTEM)
		bRet|=UDBATTRIB_SYSTEM;
	return bRet;
}

#endif

inline DWORD CDatabaseUpdater::GetNumberOfDatabases() const
{
	return m_aDatabases.GetSize();
}

inline DWORD CDatabaseUpdater::GetCurrentDatabase() const
{
	return m_dwCurrentDatabase;
}

inline const CDatabaseUpdater::CRootDirectory* CDatabaseUpdater::GetCurrentRoot() const
{
	return m_pCurrentRoot;
}


inline WORD CDatabaseUpdater::GetProgressStatus() const
{
	if (m_dwCurrentDatabase==DWORD(-1))
		return WORD(-1);
	
	
	if (m_aDatabases[m_dwCurrentDatabase]->m_dwExpectedDirectories==DWORD(-1))
		return WORD(-1);

	DWORD dwDirectoriesCurrently=m_dwDirectories;
	if (m_pCurrentRoot!=NULL)
		dwDirectoriesCurrently+=m_pCurrentRoot->m_dwDirectories;

	if (dwDirectoriesCurrently>m_aDatabases[m_dwCurrentDatabase]->m_dwExpectedDirectories)
		return 990;

    return WORD((dwDirectoriesCurrently*1000)/m_aDatabases[m_dwCurrentDatabase]->m_dwExpectedDirectories);
}

inline LPCSTR CDatabaseUpdater::GetCurrentRootPath() const
{
	if (m_pCurrentRoot==NULL)
		return NULL;
	return m_pCurrentRoot->m_Path;
}

inline LPSTR CDatabaseUpdater::GetCurrentRootPathStr() const
{
	char* pStr=new char[m_pCurrentRoot->m_Path.GetLength()+1];
	CopyMemory(pStr,(LPCSTR)m_pCurrentRoot->m_Path,m_pCurrentRoot->m_Path.GetLength()+1);
	return pStr;
}

inline const LPCSTR CDatabaseUpdater::GetCurrentDatabaseName() const
{
	if (m_dwCurrentDatabase==DWORD(-1))
		return NULL;
	return m_aDatabases[m_dwCurrentDatabase]->m_szName;
}

inline const LPCSTR CDatabaseUpdater::GetCurrentDatabaseFile() const
{
	if (m_dwCurrentDatabase==DWORD(-1))
		return NULL;
	return m_aDatabases[m_dwCurrentDatabase]->m_szArchive;
}

inline LPSTR CDatabaseUpdater::GetCurrentDatabaseNameStr() const
{
	if (m_dwCurrentDatabase==DWORD(-1))
		return 0;
	
	LPSTR szRet=new char[m_aDatabases[m_dwCurrentDatabase]->m_dwNameLength+1];
	ASSERT(strlen(m_aDatabases[m_dwCurrentDatabase]->m_szName)==m_aDatabases[m_dwCurrentDatabase]->m_dwNameLength);
	
	CopyMemory(szRet,m_aDatabases[m_dwCurrentDatabase]->m_szName,m_aDatabases[m_dwCurrentDatabase]->m_dwNameLength+1);
	return szRet;
}

inline BOOL CDatabaseUpdater::IsIncrementUpdate() const
{
	if (m_dwCurrentDatabase==DWORD(-1))
		return FALSE;
	
	return m_aDatabases[m_dwCurrentDatabase]->IsFlagged(DBArchive::IncrementalUpdate);
}

inline BOOL CDatabaseUpdater::EnumDatabases(int iDatabase,LPCSTR& szName,LPCSTR& szFile,CDatabase::ArchiveType& nArchiveType,CRootDirectory*& pFirstRoot)
{
	if (iDatabase<0 || iDatabase>=m_aDatabases.GetSize())
		return FALSE;

	szName=m_aDatabases[iDatabase]->m_szName;
	szFile=m_aDatabases[iDatabase]->m_szArchive;
	nArchiveType=m_aDatabases[iDatabase]->m_nArchiveType;
	pFirstRoot=m_aDatabases[iDatabase]->m_pFirstRoot;
	return TRUE;
}

#if defined(WIN32) && !defined(DBUPD_NOFORCELIBS)
	#pragma comment(lib, "Mpr.lib")
#endif

#endif