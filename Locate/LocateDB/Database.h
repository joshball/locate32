/* Copyright (c) 1997-2004 Janne Huttunen
   database updater v2.98.4.9200                */

#if !defined(DATABASE_H)
#define DATABASE_H

#if _MSC_VER >= 1000
#pragma once
#endif




class CDatabase;
typedef CDatabase* PDATABASE;

class CDatabase
{
public:
	enum DatabaseFlags {
		flagEnabled=0x1,
		flagGlobalUpdate=0x2,
		flagStopIfRootUnavailable=0x4
	};
		
	enum ArchiveType {
		archiveFile
	};

private:
    CDatabase();

public:
	
	// Copyer
	CDatabase(CDatabase& src);
	
	~CDatabase();

	

	WORD GetID() const;
	LPCSTR GetName() const;
	LPCSTR GetCreator() const;
	LPCSTR GetDescription() const;
	LPCSTR GetRoots() const;
	void GetRoots(CArray<LPSTR>& aRoots) const;
	WORD GetFlags() const;
	WORD GetThreadId() const;
	
	void SetFlags(WORD wFlags);
	void SetThreadId(WORD wThreadID);

	void SetNamePtr(LPSTR szName);
	void SetCreatorPtr(LPSTR szCreator);
	void SetDescriptionPtr(LPSTR szDescription);

	void SetRootsPtr(LPSTR szRoots);
	void SetRoots(CArray<LPSTR>& aRoots);
	void SetRoots(LPSTR* pRoots,int nCount);
	void AddRoot(LPCSTR pRoot);
	void AddLocalRoots();

	BOOL IsEnabled() const;
	BOOL IsGloballyUpdated() const;
	
	BOOL IsFlagged(DatabaseFlags nFlag) const;
	void SetFlag(DatabaseFlags nFlag,BOOL bSet=TRUE);

	void Enable(BOOL bEnable=TRUE);
	void UpdateGlobally(BOOL bUpdate=TRUE);

	ArchiveType GetArchiveType() const;
	LPCSTR GetArchiveName() const;

	void SetArchiveName(LPCSTR szArchiveName);
	void SetArchiveNamePtr(LPSTR szArchiveName);
	void SetArchiveType(ArchiveType nType);

	LPSTR GetValidKey(DWORD dwUniqueNum=0) const; // free returned pointer with delete[]

	const CArrayFAP<LPSTR>& GetExcludedDirectories() const;
	void SetExcludedDirectories(const CArrayFAP<LPSTR>& aExcludedDirectories);
	BOOL AddExcludedDirectory(LPCSTR szDirectory);


	BOOL IsFileNamesOEM() const;

	BOOL DoDatabaseFileExist() const;

private:
	BOOL SaveToRegistry(HKEY hKeyRoot,LPCSTR szPath,LPCSTR szKey);

public:
	static BOOL LoadFromRegistry(HKEY hKeyRoot,LPCSTR szPath,CArray<PDATABASE>& aDatabases); // Returns the number or retrieved dbs
	static INT LoadFromRegistry(HKEY hKeyRoot,LPCSTR szPath,PDATABASE*& ppDatabases); // Returns the number or retrieved dbs
	static BOOL SaveToRegistry(HKEY hKeyRoot,LPCSTR szPath,const CArray<PDATABASE>& aDatabases);
	static BOOL SaveToRegistry(HKEY hKeyRoot,LPCSTR szPath,const PDATABASE* ppDatabases,int nDatabases);

	static CDatabase* FromName(HKEY hKeyRoot,LPCSTR szPath,LPCSTR szName,int dwNameLength=-1);
	static CDatabase* FromKey(HKEY hKeyRoot,LPCSTR szPath,LPCSTR szKey);
	static CDatabase* FromFile(LPCSTR szFileName,int dwNameLength=-1);
    static CDatabase* FromOldStyleDatabase(HKEY hKeyRoot,LPCSTR szPath);
	static CDatabase* FromDefaults(BOOL bDefaultFileName,LPCSTR szAppDir,int iAppDirLength);
	
	static void CheckValidNames(CArray<PDATABASE>& aDatabases);
	static void CheckValidNames(PDATABASE* ppDatabases,int nDatabases);
	static void CheckDoubleNames(CArray<PDATABASE>& aDatabases);
	static void CheckDoubleNames(PDATABASE* ppDatabases,int nDatabases);
	static WORD CheckIDs(CArray<PDATABASE>& aDatabases);
	static WORD CheckIDs(PDATABASE* ppDatabases,int nDatabases);
	
	static BOOL IsNameValid(LPCSTR szName);
	static void MakeNameValid(LPSTR szName);
	static LPSTR GetCorrertFileName(LPCSTR szFileName,int dwNameLength=-1);

	static WORD GetUniqueIndex(CArray<PDATABASE>& aDatabases);
	static WORD GetUniqueIndex(PDATABASE* ppDatabases,int nDatabases);


	static CDatabase* FindByName(CArray<PDATABASE>& aDatabases,LPCSTR szName,int iLength=-1);
	static CDatabase* FindByName(PDATABASE* ppDatabases,int nDatabases,LPCSTR szName,int iLength=1);

	static CDatabase* FindByFile(CArray<PDATABASE>& aDatabases,LPCSTR szFile,int iLength=-1);
	static CDatabase* FindByFile(PDATABASE* ppDatabases,int nDatabases,LPCSTR szFile,int iLength=1);

	static void GetLogicalDrives(CArrayFAP<LPSTR>* paRoots);

private:
	LPSTR m_szName;
	WORD m_wFlags;
	WORD m_wThread;
	WORD m_wID;

	LPSTR m_szCreator;    
	LPSTR m_szDescription;

	LPSTR m_szRoots; // Roots included, NULL means all local
	
	// Archive type, at that moment only file is supported
	// in future, possiple types may be tcp/ip stream, SQL, ...
	ArchiveType m_ArchiveType;
	LPSTR m_szArchiveName;

	CArrayFAP<LPSTR> m_aExcludedDirectories;
};


// Inline functions
inline CDatabase::CDatabase()
// Default values:
:	m_szCreator(NULL),m_szDescription(NULL),m_szRoots(NULL),
	m_wFlags(flagEnabled|flagGlobalUpdate),m_wThread(0),
	m_ArchiveType(archiveFile),m_wID(0)
{
}

inline CDatabase::~CDatabase()
{
	if (m_szName!=NULL)
		delete[] m_szName;
	if (m_szDescription!=NULL)
		delete[] m_szDescription;
	if (m_szCreator!=NULL)
		delete[] m_szCreator;
	if (m_szArchiveName!=NULL)
		delete[] m_szArchiveName;
	if (m_szRoots!=NULL)
		delete[] m_szRoots;
}

inline LPCSTR CDatabase::GetName() const 
{ 
	return m_szName; 
}

inline LPCSTR CDatabase::GetRoots() const 
{ 
	return m_szRoots; 
}

inline LPCSTR CDatabase::GetCreator() const 
{
	if (m_szCreator==NULL)
		return szEmpty;
	return m_szCreator; 
}

inline LPCSTR CDatabase::GetDescription() const 
{ 
	if (m_szDescription==NULL)
		return szEmpty;
	return m_szDescription; 
}

inline WORD CDatabase::GetFlags() const 
{ 
	return m_wFlags; 
}

inline WORD CDatabase::GetThreadId() const 
{ 
	return m_wThread; 
}

inline CDatabase::ArchiveType CDatabase::GetArchiveType() const 
{ 
	return m_ArchiveType; 
}

inline LPCSTR CDatabase::GetArchiveName() const 
{ 
	return m_szArchiveName; 
}

inline INT CDatabase::LoadFromRegistry(HKEY hKeyRoot,LPCSTR szPath,CDatabase**& ppDatabases)
{
	CArray<CDatabase*> aDatabases;
	INT nDBs=LoadFromRegistry(hKeyRoot,szPath,aDatabases);
	if (nDBs)
		ppDatabases=aDatabases;
	return nDBs;
}

inline BOOL CDatabase::SaveToRegistry(HKEY hKeyRoot,LPCSTR szPath,const CArray<CDatabase*>& aDatabases)
{
	return SaveToRegistry(hKeyRoot,szPath,aDatabases,aDatabases.GetSize());
}

inline void CDatabase::CheckValidNames(CArray<PDATABASE>& aDatabases)
{
	CheckValidNames(aDatabases,aDatabases.GetSize());
}

inline void CDatabase::CheckDoubleNames(CArray<PDATABASE>& aDatabases)
{
	CheckDoubleNames(aDatabases,aDatabases.GetSize());
}

inline WORD CDatabase::CheckIDs(CArray<PDATABASE>& aDatabases)
{
	return CheckIDs(aDatabases,aDatabases.GetSize());
}

inline void CDatabase::Enable(BOOL bEnable)
{
	if (bEnable)
		m_wFlags|=flagEnabled;
	else
		m_wFlags&=~flagEnabled;
}

inline void CDatabase::UpdateGlobally(BOOL bUpdate)
{
	if (bUpdate)
		m_wFlags|=flagGlobalUpdate;
	else
		m_wFlags&=~flagGlobalUpdate;
}
	
inline void CDatabase::SetFlags(WORD wFlags)
{
	m_wFlags=wFlags;
}

inline void CDatabase::SetThreadId(WORD wThreadID)
{
	m_wThread=wThreadID;
}

inline BOOL CDatabase::IsEnabled() const
{
	return (m_wFlags&flagEnabled)?1:0;
}

inline BOOL CDatabase::IsGloballyUpdated() const
{
	return (m_wFlags&flagGlobalUpdate)?1:0;
}

inline void CDatabase::SetNamePtr(LPSTR szName)
{
	if (m_szName!=NULL)
		delete[] m_szName;
	m_szName=szName;
}

inline void CDatabase::SetCreatorPtr(LPSTR szCreator)
{
	if (m_szCreator!=NULL)
		delete[] m_szCreator;
	m_szCreator=szCreator;
}

inline void CDatabase::SetDescriptionPtr(LPSTR szDescription)
{
	if (m_szDescription!=NULL)
		delete[] m_szDescription;
	m_szDescription=szDescription;
}

inline void CDatabase::SetArchiveName(LPCSTR szArchiveName)
{
	if (m_szArchiveName!=NULL)
		delete[] m_szArchiveName;
	m_szArchiveName=alloccopy(szArchiveName);
}

inline void CDatabase::SetArchiveNamePtr(LPSTR szArchiveName)
{
	if (m_szArchiveName!=NULL)
		delete[] m_szArchiveName;
	m_szArchiveName=szArchiveName;
}

inline void CDatabase::SetArchiveType(ArchiveType nType)
{
	m_ArchiveType=nType;
}

inline void CDatabase::SetRootsPtr(LPSTR szRoots)
{
	if (m_szRoots!=NULL)
		delete[] m_szRoots;
	m_szRoots=szRoots;
}

inline void CDatabase::SetRoots(CArray<LPSTR>& aRoots)
{
	return SetRoots(aRoots,aRoots.GetSize());
}

inline CDatabase* CDatabase::FindByName(CArray<PDATABASE>& aDatabases,LPCSTR szName,int iLength)
{
	return FindByName(aDatabases,aDatabases.GetSize(),szName,iLength);
}

inline CDatabase* CDatabase::FindByFile(CArray<PDATABASE>& aDatabases,LPCSTR szFile,int iLength)
{
	return FindByFile(aDatabases,aDatabases.GetSize(),szFile,iLength);
}

inline WORD CDatabase::GetID() const
{
	return m_wID;
}

inline WORD CDatabase::GetUniqueIndex(CArray<PDATABASE>& aDatabases)
{
	return GetUniqueIndex(aDatabases,aDatabases.GetSize());
}
	
inline BOOL CDatabase::DoDatabaseFileExist() const
{
	switch (m_ArchiveType)
	{
	case archiveFile:
		return CFile::IsFile(m_szArchiveName);
	}
	return FALSE;
}

inline BOOL CDatabase::IsFlagged(DatabaseFlags nFlag) const
{
	return m_wFlags&nFlag;
}

inline void CDatabase::SetFlag(DatabaseFlags nFlag,BOOL bSet)
{
	if (bSet)
		m_wFlags|=nFlag;
	else
		m_wFlags&=~WORD(nFlag);
}

inline const CArrayFAP<LPSTR>& CDatabase::GetExcludedDirectories() const
{
	return m_aExcludedDirectories;
}

inline void CDatabase::SetExcludedDirectories(const CArrayFAP<LPSTR>& aExcludedDirectories)
{
	m_aExcludedDirectories.RemoveAll();
	for (int i=0;i<aExcludedDirectories.GetSize();i++)
		m_aExcludedDirectories.Add(alloccopy(aExcludedDirectories[i]));
}

inline BOOL CDatabase::AddExcludedDirectory(LPCSTR szDirectory)
{
	if (szDirectory[1]!=':' || szDirectory[2]!='\0')
	{
		if (!CFile::IsDirectory(szDirectory))
			return FALSE;	
	}
	
	for (int i=0;i<m_aExcludedDirectories.GetSize();i++)
	{
		if (strcasecmp(m_aExcludedDirectories[i],szDirectory)==0)
			return TRUE;
	}
	m_aExcludedDirectories.Add(alloccopy(szDirectory));
	return TRUE;
}

#endif