/* Copyright (c) 1997-2004 Janne Huttunen
   database updater v2.98.4.9200                  */

#if !defined(DATABASEINFO_H)
#define DATABASEINFO_H

#if _MSC_VER >= 1000
#pragma once
#endif

class CDatabaseInfo
{
public:
	enum RootType {
		Unknown=0,
		Fixed=0x10,
		Removable=0x20,
		CDRom=0x30,
		Remote=0x40,
		Ramdisk=0x50,
		Directory=0xF0
	};
		
	struct CRoot
	{
		CRoot() : rtType(Unknown) {}
		CRoot(char drive) 
		:	sPath(drive),dwVolumeSerial(0),dwNumberOfFiles(DWORD(-1)),
			dwNumberOfDirectories(DWORD(-1)),rtType(Unknown) 
		{
			sPath <<':';
		}
		
		CString sPath;
		CString sVolumeName;
		DWORD dwVolumeSerial;
		CString sFileSystem;
		DWORD dwNumberOfFiles;
		DWORD dwNumberOfDirectories;
		RootType rtType;
	};
	
protected:
	CDatabaseInfo();

	BOOL GetInfo(const CDatabase* pDatabase);

public:
	BYTE bVersion;
	BYTE bLongFilenames;
	BYTE bAnsi;
	CString sCreator;
	CString sDescription;
	CTime tCreationTime;
	DWORD dwNumberOfFiles;
	DWORD dwNumberOfDirectories;
	CArrayFP<CRoot*> aRootFolders;
	DWORD dwFileSize;

public:
	static CDatabaseInfo* GetFromDatabase(const CDatabase* pDatabase);

		
	static BOOL GetRootsFromDatabase(CArray<LPSTR>& aRoots,const CDatabase* pDatabase);
	
	// Fonctions do not clear aRoots
	static BOOL GetRootsFromDatabases(CArray<LPSTR>& aRoots,const PDATABASE* pDatabases,int nDatabases,BOOL bOnlyEnabled=FALSE);
	static BOOL GetRootsFromDatabases(CArray<LPSTR>& aRoots,const CArray<CDatabase*>& aDatabases,BOOL bOnlyEnabled=FALSE);

	static BOOL ReadFilesAndDirectoriesCount(CDatabase::ArchiveType,LPCSTR szArchive,DWORD& dwFiles,DWORD& dwDirectories);
};

inline 	CDatabaseInfo::CDatabaseInfo()
:	bVersion(0), bLongFilenames(0),dwNumberOfDirectories(DWORD(-1)),dwNumberOfFiles(DWORD(-1))
{
}

inline CDatabaseInfo* CDatabaseInfo::GetFromDatabase(const CDatabase* pDatabase)
{
	CDatabaseInfo* pRet=new CDatabaseInfo;
	
	if (!pRet->GetInfo(pDatabase))
	{
		delete pRet;
		return NULL;
	}
	return pRet;
}

inline BOOL CDatabaseInfo::GetRootsFromDatabases(CArray<LPSTR>& aRoots,const CArray<CDatabase*>& aDatabases,BOOL bOnlyEnabled)
{
	return GetRootsFromDatabases(aRoots,aDatabases,aDatabases.GetSize(),bOnlyEnabled);
}

#endif