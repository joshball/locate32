/* Copyright (c) 1997-2004 Janne Huttunen
   database updater v2.98.4.9200                */

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
		
		CRootDirectory* m_pNext;

		// Excluded directories
		CArray<LPSTR> m_aExcludedDirectories; 

		friend CDatabaseUpdater;
		friend DBArchive;
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
			StopIfUnuavailable = 0x1 // Stops updating if root is unavailable
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
	
	const LPCSTR GetCurrentDatabaseName() const;
	const LPCSTR GetCurrentDatabaseFile() const;
	LPSTR GetCurrentDatabaseNameStr() const;

#ifdef WIN32
	static BYTE GetAttribFlag(DWORD dwAttribs);
#endif

private:

	CArrayFP<DBArchive*> m_aDatabases;
	DBArchive* m_pCurrentDatabase;

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

	// Status flags (statusNotChanged is not by CDatabaseUpdater)
	enum {
		statusNotChanged,
		statusInitializing,
		statusScanning,
		statusWritingDB
	} sStatus;
	CRootDirectory* m_pCurrentRoot;

	friend CRootDirectory;
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
:	m_szArchive(NULL),m_nArchiveType(CDatabase::archiveFile),m_pFirstRoot(NULL),m_nFlags(0)
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

inline const CDatabaseUpdater::CRootDirectory* CDatabaseUpdater::GetCurrentRoot() const
{
	return m_pCurrentRoot;
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
	if (m_pCurrentDatabase==NULL)
		return NULL;
	return m_pCurrentDatabase->m_szName;
}

inline const LPCSTR CDatabaseUpdater::GetCurrentDatabaseFile() const
{
	if (m_pCurrentDatabase==NULL)
		return NULL;
	return m_pCurrentDatabase->m_szArchive;
}

inline LPSTR CDatabaseUpdater::GetCurrentDatabaseNameStr() const
{
	LPSTR szRet=new char[m_pCurrentDatabase->m_dwNameLength+1];
	ASSERT(strlen(m_pCurrentDatabase->m_szName)==m_pCurrentDatabase->m_dwNameLength);
	
	CopyMemory(szRet,m_pCurrentDatabase->m_szName,m_pCurrentDatabase->m_dwNameLength+1);
	return szRet;
}


#if defined(WIN32) && !defined(DBUPD_NOFORCELIBS)
	#pragma comment(lib, "Mpr.lib")
#endif

#endif