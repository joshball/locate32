/* Copyright (c) 1997-2005 Janne Huttunen
   database locater v2.99.5.5070                  */

#if !defined(LOCATER_INL)
#define LOCATER_INL


#ifdef WIN32
#define CLOCATERINITIALIATIONS   \
:	m_dwMaxFoundFiles(DWORD(-1)), \
	m_dwMinSize(DWORD(-1)),m_dwMaxSize(DWORD(-1)), \
	m_wMinDate(WORD(-1)),m_wMaxDate(WORD(-1)), \
	m_dwFlags(LOCATE_FOLDERNAMES),szBuffer(NULL), \
	m_pContentSearcher(NULL), \
	m_hThread(NULL),dbFile(NULL)
#else
#define CLOCATERINITIALIATIONS   \
:	m_dwMaxFoundFiles(DWORD(-1)), \
	m_dwMinSize(DWORD(-1)),m_dwMaxSize(DWORD(-1)), \
	m_wMinDate(WORD(-1)),m_wMaxDate(WORD(-1)), \
	m_dwFlags(LOCATE_FOLDERNAMES),szBuffer(NULL), \
	m_pContentSearcher(NULL),dbFile(NULL)
#endif


inline CLocater::CLocater() 
CLOCATERINITIALIATIONS
{
}

inline CLocater::CLocater(LPCSTR szDatabaseFile)
CLOCATERINITIALIATIONS
{
	SetDatabases(szDatabaseFile);
}

inline CLocater::CLocater(const PDATABASE* pDatabases,int nDatabases)
CLOCATERINITIALIATIONS
{
	SetDatabases(pDatabases,nDatabases);
}

inline CLocater::CLocater(const CDatabase* pDatabases,int nDatabases)
CLOCATERINITIALIATIONS
{
	SetDatabases(pDatabases,nDatabases);
}

inline CLocater::CLocater(const CArray<PDATABASE>& aDatabases)
CLOCATERINITIALIATIONS
{
	SetDatabases(aDatabases);
}

inline CLocater::CLocater(const CArray<CDatabase>& aDatabases)
CLOCATERINITIALIATIONS
{
	SetDatabases(aDatabases);
}

inline void CLocater::SetFunctions(LOCATEPROC pProc,LOCATEFOUNDPROC pFoundProc,DWORD dwParam)
{
	m_pProc=pProc;
	for (int i=0;i<m_aDatabases.GetSize();i++)
		m_aDatabases[i]->m_pFoundProc=pFoundProc;
	m_dwData=dwParam;
}

inline void CLocater::SetFunctions(LOCATEPROC pProc,LOCATEFOUNDPROC* pFoundProcs,DWORD dwParam)
{
	m_pProc=pProc;
	for (int i=0;i<m_aDatabases.GetSize();i++)
		m_aDatabases[i]->m_pFoundProc=pFoundProcs[i];
	m_dwData=dwParam;
}

inline void CLocater::SetDatabases(LPCSTR szDatabaseFile)
{
	m_aDatabases.RemoveAll();
	m_aDatabases.Add(new DBArchive("custom",CDatabase::archiveFile,szDatabaseFile,0,TRUE));
}

inline void CLocater::SetDatabases(const PDATABASE* pDatabases,int nDatabases)
{
	m_aDatabases.RemoveAll();
	for (int i=0;i<nDatabases;i++)
	{
		m_aDatabases.Add(new DBArchive(pDatabases[i]->GetName(),
			pDatabases[i]->GetArchiveType(),pDatabases[i]->GetArchiveName(),
			pDatabases[i]->GetID(),pDatabases[i]->IsEnabled()));
	}
}

inline void CLocater::SetDatabases(const CDatabase* pDatabases,int nDatabases)
{
	m_aDatabases.RemoveAll();
	for (int i=0;i<nDatabases;i++)
	{
		m_aDatabases.Add(new DBArchive(pDatabases[i].GetName(),
			pDatabases[i].GetArchiveType(),pDatabases[i].GetArchiveName(),
			pDatabases[i].GetID(),pDatabases[i].IsEnabled()));
	}
}

inline void CLocater::SetDatabases(const CArray<PDATABASE>& aDatabases)
{
	m_aDatabases.RemoveAll();
	for (int i=0;i<aDatabases.GetSize();i++)
	{
		m_aDatabases.Add(new DBArchive(aDatabases[i]->GetName(),aDatabases[i]->GetArchiveType(),
			aDatabases[i]->GetArchiveName(),aDatabases[i]->GetID(),aDatabases[i]->IsEnabled()));
	}
}

inline void CLocater::SetDatabases(const CArray<CDatabase>& aDatabases)
{
	m_aDatabases.RemoveAll();
	for (int i=0;i<aDatabases.GetSize();i++)
	{
		m_aDatabases.Add(new DBArchive(aDatabases[i].GetName(),aDatabases[i].GetArchiveType(),
			aDatabases[i].GetArchiveName(),aDatabases[i].GetID(),aDatabases[i].IsEnabled()));
	}
}

inline void CLocater::SetSizeAndDate(DWORD dwFlags,DWORD dwMinSize,DWORD dwMaxSize,
									 WORD wMinDate,WORD wMaxDate)
{
	m_dwFlags=(m_dwFlags&~(LOCATE_MINCREATIONDATE|LOCATE_MINACCESSDATE|LOCATE_MAXCREATIONDATE|LOCATE_MAXACCESSDATE))|dwFlags;
	m_dwMinSize=dwMinSize;
	m_dwMaxSize=dwMaxSize;
	m_wMinDate=wMinDate;
	m_wMaxDate=wMaxDate;
}


inline bool _strncmp(const char* p1,const char* p2,int a)
{
	for (a--;a>=0;a--)
		if (p1[a]!=p2[a])
			return FALSE;
	return TRUE;
}

inline bool _strfcmp(const char* p1,const char* p2,int a)
{
	for (a--;p1[a]!='\\';a--)
		if (p1[a]!=p2[a])
			return FALSE;
	return TRUE;
}

inline CLocater::ValidType CLocater::IsFolderValid(DWORD nPathLen)
{
	if (m_aDirectories.GetSize())
	{
		for (int i=0;i<m_aDirectories.GetSize();i++)
		{
			if (m_aDirectories.GetAt(i)->GetLength()>nPathLen)
			{
				if (_strfcmp(*m_aDirectories.GetAt(i),szCurrentPathLower,nPathLen))
					return SomeValidFolders;
			}
			else
			{
				if (_strfcmp(*m_aDirectories.GetAt(i),szCurrentPathLower,
					m_aDirectories.GetAt(i)->GetLength()))
					return ValidFolders;
			}
		}
		return NoValidFolders;
	}
	else
		return ValidFolders;					
}

inline CLocater::ValidType CLocater::IsRootValid(DWORD nPathLen)
{
	for (int i=0;i<m_aDirectories.GetSize();i++)
	{
		if (m_aDirectories.GetAt(i)->GetLength()>nPathLen)
		{
			if (_strncmp(*m_aDirectories.GetAt(i),szCurrentPathLower,nPathLen))
				return SomeValidFolders;
		}
		else
		{
			if (_strncmp(*m_aDirectories.GetAt(i),szCurrentPathLower,
				m_aDirectories.GetAt(i)->GetLength()))
				return ValidFolders;
		}
	}
	return NoValidFolders;
}

inline LPCSTR CLocater::GetFolderName() const
{
	return LPCSTR(pPoint);
}

inline DWORD CLocater::GetFolderNameLen() const
{
	return (DWORD)*(pPoint-1);
}

inline WORD CLocater::GetFolderModifiedTime() const
{
	return *(WORD*)(pPoint+pPoint[-1]+3);
}

inline WORD CLocater::GetFolderModifiedDate() const
{
	return *(WORD*)(pPoint+pPoint[-1]+1);
}

inline WORD CLocater::GetFolderCreatedDate() const
{
	return *(WORD*)(pPoint+pPoint[-1]+5);
}

inline WORD CLocater::GetFolderCreatedTime() const
{
	return WORD(-1);
}

inline WORD CLocater::GetFolderAccessedDate() const
{
	return *(WORD*)(pPoint+pPoint[-1]+7);
}

inline WORD CLocater::GetFolderAccessedTime() const
{
	return WORD(-1);
}

inline LPCSTR CLocater::GetFileName() const
{
	return LPCSTR(pPoint+3);
}

inline DWORD CLocater::GetFileNameLen() const
{
	return pPoint[1];
}

inline BYTE CLocater::GetFileExtensionPos() const
{
	return pPoint[2];
}

inline DWORD CLocater::GetFileSizeLo() const
{
	return *(DWORD*)(pPoint+pPoint[1]+4);
}

inline BYTE CLocater::GetFileSizeHi() const
{
	return *(pPoint+pPoint[1]+4+sizeof(DWORD));
}

inline WORD CLocater::GetFileModifiedTime() const
{
	return *(WORD*)(pPoint+pPoint[1]+3+4+2+2);
}

inline WORD CLocater::GetFileModifiedDate() const
{
	return *(WORD*)(pPoint+pPoint[1]+3+4+2);
}

inline WORD CLocater::GetFileCreatedDate() const
{
	return *(WORD*)(pPoint+pPoint[1]+3+4+2+4);
}

inline WORD CLocater::GetFileCreatedTime() const
{
	return WORD(-1);
}

inline WORD CLocater::GetFileAccessedDate() const
{
	return *(WORD*)(pPoint+pPoint[1]+3+4+2+4+2);
}

inline WORD CLocater::GetFileAccessedTime() const
{
	return WORD(-1);
}

inline LPCSTR CLocater::GetCurrentPath() const
{
	return szCurrentPath;
}

inline DWORD CLocater::GetCurrentPathLen() const
{
	return dwCurrentPathLen;
}
	
inline DWORD CLocater::GetFoundFiles() const
{
	return m_dwFoundFiles;
}

inline BYTE CLocater::GetFolderAttributes() const
{
	return *(pPoint-6);
}

inline BYTE CLocater::GetFileAttributes() const
{
	return pPoint[0];
}

inline BOOL CLocater::HaveFileExtension() const
{
	//return GetFileExtensionPos()!=0 ||*GetFileName()[0]=='.';
	return pPoint[2]!=0 || pPoint[3]=='.';
}

inline DWORD CLocater::GetAdvancedFlags() const
{
	return m_dwFlags;
}

inline DWORD CLocater::SetAdvancedFlags(DWORD dwFlags)
{
	return (m_dwFlags=dwFlags);
}

inline DWORD CLocater::AddAdvancedFlags(DWORD dwNewFlag)
{
	return (m_dwFlags|=dwNewFlag);
}

inline DWORD CLocater::RemoveAdvancedFlags(DWORD dwRemoveFlag)
{
	return (m_dwFlags&=~dwRemoveFlag);
}


#if defined(WIN32) & !defined(LOCATER_NOTHREAD)
inline void CLocater::CouldStop()
{
	InterlockedExchange(&m_lForceQuit,TRUE);
}
#endif


inline CLocater::DBArchive::DBArchive(LPCSTR szName_,CDatabase::ArchiveType nArchiveType_,LPCSTR szArchive_,WORD wID_,BOOL bEnable_)
:	nArchiveType(nArchiveType_),wID(wID_),bEnable(bEnable_),bOEM(FALSE)
{
	dwNameLength=strlen(szName_)+1;
	szName=new char[dwNameLength];
	sMemCopy(szName,szName_,dwNameLength);
	szArchive=alloccopy(szArchive_);
}

inline CLocater::DBArchive::~DBArchive()
{
	if (szName!=NULL)
		delete[] szName;
	if (szArchive!=NULL)
		delete[] szArchive;
}

inline WORD CLocater::GetCurrentDatabaseID() const
{
	return m_wCurrentDatabaseID;
}

inline WORD CLocater::GetCurrentDatabaseRootID() const
{
	return m_wCurrentRootIndex;
}	
	
inline LPCSTR CLocater::GetCurrentDatabaseName() const
{
	return m_pCurrentDatabase->szName;
}

inline void CLocater::GetCurrentDatabaseName(LPSTR& szName) const
{
	szName=new char[m_pCurrentDatabase->dwNameLength];
	sMemCopy(szName,m_pCurrentDatabase->szName,m_pCurrentDatabase->dwNameLength);
}

inline BOOL CLocater::IsCurrentDatabaseNamesOEM() const
{
	return m_pCurrentDatabase->bOEM;
}

inline LPCSTR CLocater::GetCurrentDatabaseFile() const
{
	return m_pCurrentDatabase->szArchive;
}

inline DWORD CLocater::GetNumberOfDatabases() const
{
	return m_aDatabases.GetSize();
}

inline BYTE CLocater::GetCurrentDatabaseRootType() const
{
	return m_bCurrentRootType;
}

inline LPCSTR CLocater::GetCurrentDatabaseVolumeLabel() const
{
	return m_szVolumeName;
}

inline DWORD CLocater::GetCurrentDatabaseVolumeSerial() const
{
	return m_dwVolumeSerial;
}

inline LPCSTR CLocater::GetCurrentDatabaseFileSystem() const
{
	return m_szFileSystem;
}
	
#endif
