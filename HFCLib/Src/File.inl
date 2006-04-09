////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////

#ifndef HFCFILE_INL
#define HFCFILE_INL

/////////////////////////////////////////////////
// CFile

#ifdef WIN32
inline CFile::CFile()
:	m_bCloseOnDelete(FALSE),m_nOpenFlags(0),m_hFile(FILE_NULL)
{
}

inline CFile::CFile(BOOL bThrowExceptions)
:	m_bCloseOnDelete(FALSE),m_nOpenFlags(0),m_hFile(FILE_NULL),
	CExceptionObject(bThrowExceptions)
{
}

inline CFile::CFile(HANDLE hFile,BOOL bThrowExceptions)
:	m_hFile(hFile),m_bCloseOnDelete(FALSE),m_nOpenFlags(0),
	CExceptionObject(bThrowExceptions)
{
}

inline CFile::CFile(LPCSTR lpszFileName,int nOpenFlags,CFileException* e)
:	m_hFile(FILE_NULL),m_bCloseOnDelete(FALSE)
{
	Open(lpszFileName,nOpenFlags,e);
}

inline CFile::CFile(LPCSTR lpszFileName,int nOpenFlags,BOOL bThrowExceptions)
:	m_hFile(FILE_NULL),m_bCloseOnDelete(FALSE),
	CExceptionObject(bThrowExceptions)
{
	Open(lpszFileName,nOpenFlags,NULL);
}
#endif

#ifdef DEF_WCHAR
inline CFile::CFile(LPCWSTR lpszFileName,int nOpenFlags,CFileException* e)
:	m_hFile(FILE_NULL),m_bCloseOnDelete(FALSE)
{
	Open(lpszFileName,nOpenFlags,e);
}

inline CFile::CFile(LPCWSTR lpszFileName,int nOpenFlags,BOOL bThrowExceptions)
:	m_hFile(FILE_NULL),m_bCloseOnDelete(FALSE),
	CExceptionObject(bThrowExceptions)
{
	Open(lpszFileName,nOpenFlags,NULL);
}
#endif


inline CFile::~CFile()
{
	if (m_hFile!=FILE_NULL && m_bCloseOnDelete)
#ifdef WIN32
		CloseHandle(m_hFile);
#else
		fclose((FILE*)m_hFile);
#endif
}

inline CFile::operator HANDLE() const
{
	return m_hFile;
}

#ifndef WIN32
inline CFile::operator FILE*() const
{
	return (FILE*)m_hFile; 
}
#endif

inline CString CFile::GetFilePath() const
{
	return m_strFileName;
}

inline void CFile::SetFilePath(LPCSTR lpszNewName)
{
	m_strFileName=lpszNewName;
}

#ifdef DEF_WCHAR
inline CStringW CFile::GetFilePathW() const
{
	return m_strFileName;
}

inline void CFile::SetFilePath(LPCWSTR lpszNewName)
{
	m_strFileName=lpszNewName;
}
#endif


inline BOOL CFile::Read(BYTE& bNum,CFileException* pError)
{ 
	return this->Read(&bNum,sizeof(BYTE),pError)==sizeof(BYTE); 
}

inline BOOL CFile::Read(WORD& wNum,CFileException* pError) 
{ 
	return this->Read(&wNum,sizeof(WORD),pError)==sizeof(WORD); 
}

inline BOOL CFile::Read(DWORD& dwNum,CFileException* pError) 
{ 
	return this->Read(&dwNum,sizeof(DWORD),pError)==sizeof(DWORD); 
}

inline BOOL CFile::Write(const CStringA& str,CFileException* pError) 
{ 
	return this->Write((LPCSTR)str,str.GetLength()+(m_nOpenFlags&otherStrNullTerminated?1:0),pError); 
}

#ifdef DEF_WCHAR
inline BOOL CFile::Write(const CStringW& str,CFileException* pError) 
{ 
	return this->Write((LPCWSTR)str,(str.GetLength()+(m_nOpenFlags&otherStrNullTerminated?1:0))*2,pError); 
}
#endif



inline BOOL CFile::Write(BYTE bNum,CFileException* pError) 
{ 
	return this->Write(&bNum,sizeof(BYTE),pError); 
}

inline BOOL CFile::Write(WORD wNum,CFileException* pError)
{ 
	return this->Write(&wNum,sizeof(WORD),pError); 
}

inline BOOL CFile::Write(DWORD dwNum,CFileException* pError) 
{ 
	return this->Write(&dwNum,sizeof(DWORD),pError); 
}

inline BOOL CFile::Write(char ch,CFileException* pError) 
{ 
	return this->Write(&ch,sizeof(char),pError); 
}

inline BOOL CFile::Write(LPCSTR szNullTerminatedString,CFileException* pError) 
{ 
	return this->Write(szNullTerminatedString,istrlen(szNullTerminatedString)+(m_nOpenFlags&otherStrNullTerminated?1:0),pError); 
}

#ifdef DEF_WCHAR
inline BOOL CFile::Write(LPCWSTR szNullTerminatedString,CFileException* pError) 
{ 
	return this->Write(szNullTerminatedString,2*istrlenw(szNullTerminatedString)+(m_nOpenFlags&otherStrNullTerminated?2:0),pError); 
}
#endif

////////////////////////////////////////
// namespace FileSystem

inline BOOL FileSystem::Rename(LPCSTR lpszOldName,LPCSTR lpszNewName)
{
#ifdef WIN32
	return ::MoveFile(lpszOldName,lpszNewName);
#else
	return (BYTE)_rename(lpszOldName,lpszNewName);	
#endif
}

inline BOOL FileSystem::Remove(LPCSTR lpszFileName)
{
#ifdef WIN32
	return ::DeleteFile(lpszFileName);
#else
	return (BYTE)::remove(lpszFileName);
#endif
}

#ifdef DEF_WCHAR
inline BOOL FileSystem::Rename(LPCWSTR lpszOldName,LPCWSTR lpszNewName)
{
	if (IsFullUnicodeSupport())
		return ::MoveFileW(lpszOldName,lpszNewName);
	else
		return ::MoveFile(W2A(lpszOldName),W2A(lpszNewName));
}

inline BOOL FileSystem::Remove(LPCWSTR lpszFileName)
{
	if (IsFullUnicodeSupport())
		return ::DeleteFileW(lpszFileName);
	else
		return ::DeleteFile(W2A(lpszFileName));
}
#endif

inline BOOL FileSystem::CreateDirectory(LPCSTR lpPathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	return ::CreateDirectory(lpPathName,lpSecurityAttributes);
}

inline BOOL FileSystem::RemoveDirectory(LPCSTR lpPathName)
{
	return ::RemoveDirectory(lpPathName);
}

inline DWORD FileSystem::GetFullPathName(LPCSTR lpFileName,DWORD nBufferLength,LPSTR lpBuffer,LPTSTR* lpFilePart)
{
	return ::GetFullPathName(lpFileName,nBufferLength,lpBuffer,lpFilePart);
}

inline DWORD FileSystem::GetShortPathName(LPCSTR lpszLongPath,LPSTR lpszShortPath,DWORD cchBuffer)
{
	return ::GetShortPathName(lpszLongPath,lpszShortPath,cchBuffer);
}

inline DWORD FileSystem::GetCurrentDirectory(DWORD nBufferLength,LPSTR lpBuffer)
{
	return ::GetCurrentDirectory(nBufferLength,lpBuffer);
}

inline DWORD FileSystem::GetLongPathName(LPCSTR lpszShortPath,LPSTR lpszLongPath,DWORD cchBuffer)
{
	return ::GetLongPathName(lpszShortPath,lpszLongPath,cchBuffer);
}
inline DWORD FileSystem::GetTempPath(DWORD nBufferLength,LPSTR lpBuffer)
{
	return ::GetTempPath(nBufferLength,lpBuffer);
}

inline UINT FileSystem::GetTempFileName(LPCSTR lpPathName,LPCSTR lpPrefixString,UINT uUnique,LPSTR lpTempFileName)
{
	return ::GetTempFileName(lpPathName,lpPrefixString,uUnique,lpTempFileName);
}
	
inline BOOL FileSystem::MoveFile(LPCSTR lpExistingFileName,LPCSTR lpNewFileName,DWORD dwFlags)
{
	return ::MoveFileEx(lpExistingFileName,lpNewFileName,dwFlags);	
}
	
inline UINT FileSystem::GetDriveType(LPCSTR lpRootPathName)
{
	return ::GetDriveType(lpRootPathName);
}

inline DWORD FileSystem::GetLogicalDriveStrings(DWORD nBufferLength,LPSTR lpBuffer)
{
	return ::GetLogicalDriveStringsA(nBufferLength,lpBuffer);
}

inline BOOL FileSystem::GetVolumeInformation(LPCSTR lpRootPathName,LPSTR lpVolumeNameBuffer,
		DWORD nVolumeNameSize,LPDWORD lpVolumeSerialNumber,LPDWORD lpMaximumComponentLength,
		LPDWORD lpFileSystemFlags,LPSTR lpFileSystemNameBuffer,DWORD nFileSystemNameSize)
{
	return ::GetVolumeInformation(lpRootPathName,lpVolumeNameBuffer,
		nVolumeNameSize,lpVolumeSerialNumber,lpMaximumComponentLength,
		lpFileSystemFlags,lpFileSystemNameBuffer,nFileSystemNameSize);
}


	
#ifdef DEF_WCHAR
inline BOOL FileSystem::CreateDirectory(LPCWSTR lpPathName,LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	if (IsFullUnicodeSupport())
		return ::CreateDirectoryW(lpPathName,lpSecurityAttributes);
	else
		return ::CreateDirectoryA(W2A(lpPathName),lpSecurityAttributes);
}

inline BOOL FileSystem::RemoveDirectory(LPCWSTR lpPathName)
{
	if (IsFullUnicodeSupport())
		return ::RemoveDirectoryW(lpPathName);
	else
		return ::RemoveDirectoryA(W2A(lpPathName));
}

inline BOOL FileSystem::MoveFile(LPCWSTR lpExistingFileName,LPCWSTR lpNewFileName,DWORD dwFlags)
{
	if (!IsFullUnicodeSupport())
		return ::MoveFileExW(lpExistingFileName,lpNewFileName,dwFlags);	
	else
		return ::MoveFileExA(W2A(lpExistingFileName),W2A(lpNewFileName),dwFlags);	
}

inline UINT FileSystem::GetDriveType(LPCWSTR lpRootPathName)
{
	if (IsFullUnicodeSupport())
		return ::GetDriveTypeW(lpRootPathName);
	else
		return ::GetDriveTypeA(W2A(lpRootPathName));
}

inline DWORD FileSystem::GetLogicalDriveStrings(DWORD nBufferLength,LPWSTR lpBuffer)
{
	if (IsFullUnicodeSupport())
		return ::GetLogicalDriveStringsW(nBufferLength,lpBuffer);
	else
	{
		DWORD nLen=::GetLogicalDriveStringsA(0,NULL)+1;
		if (nLen==0)
			return 0;
		CHAR* pBuffer=new CHAR[nLen+1];
		nLen=::GetLogicalDriveStringsA(nLen,pBuffer);
		MultiByteToWideChar(CP_ACP,0,pBuffer,nLen+1,lpBuffer,nBufferLength);
		delete[] pBuffer;
		return nLen;
	}
}

inline BOOL FileSystem::GetVolumeInformation(LPCWSTR lpRootPathName,LPWSTR lpVolumeNameBuffer,
		DWORD nVolumeNameSize,LPDWORD lpVolumeSerialNumber,LPDWORD lpMaximumComponentLength,
		LPDWORD lpFileSystemFlags,LPWSTR lpFileSystemNameBuffer,DWORD nFileSystemNameSize)
{
	if (IsFullUnicodeSupport())
		return ::GetVolumeInformationW(lpRootPathName,lpVolumeNameBuffer,
			nVolumeNameSize,lpVolumeSerialNumber,lpMaximumComponentLength,
			lpFileSystemFlags,lpFileSystemNameBuffer,nFileSystemNameSize);
	else
	{
		char* lpVolumeNameBufferA=NULL,*lpFileSystemNameBufferA=NULL;
		if (lpVolumeNameBuffer!=NULL)
			lpVolumeNameBufferA=new char[max(nVolumeNameSize,2)];
		if (lpFileSystemNameBuffer!=NULL)
			lpFileSystemNameBufferA=new char[max(nFileSystemNameSize,2)];
		BOOL bRet=::GetVolumeInformationA(W2A(lpRootPathName),lpVolumeNameBufferA,
			nVolumeNameSize,lpVolumeSerialNumber,lpMaximumComponentLength,
			lpFileSystemFlags,lpFileSystemNameBufferA,nFileSystemNameSize);
		
		if (lpVolumeNameBufferA!=NULL)
		{
			if (bRet)			
				MultiByteToWideChar(CP_ACP,0,lpVolumeNameBufferA,-1,lpVolumeNameBuffer,nVolumeNameSize);
			delete[] lpVolumeNameBufferA;
		}
		if (lpFileSystemNameBufferA!=NULL)
		{
			if (bRet)			
				MultiByteToWideChar(CP_ACP,0,lpFileSystemNameBufferA,-1,lpFileSystemNameBuffer,nFileSystemNameSize);
			delete[] lpFileSystemNameBufferA;
		}
		return bRet;
	}
}

#endif

////////////////////////////////////////
// CFileFind

#ifdef WIN32

inline CFileFind::CFileFind()
{
#ifdef WIN32
	m_hFind=NULL;
#endif
}

#ifndef WIN32
inline void CFileFind::Close()
{
}
#endif

inline CFileFind::~CFileFind()
{
	Close();
}

inline void CFileFind::GetFileName(LPSTR szName,SIZE_T nMaxLen) const
{
#ifdef WIN32
	strcpy_s(szName,nMaxLen,m_fd.cFileName);
#else
	strcpy_s(szName,nMaxLen,m_fd.ff_name);
#endif
}

inline void CFileFind::GetFileName(CString& name) const
{
#ifdef WIN32
	name=m_fd.cFileName;
#else
	name=m_fd.ff_name;
#endif
}

inline DWORD CFileFind::GetFileSize() const
{
#ifdef WIN32
	return m_fd.nFileSizeLow;
#else
	return m_fd.ff_fsize;
#endif
}



inline BOOL CFileFind::GetLastWriteTime(FILETIME& rTimeStamp) const
{
	rTimeStamp=m_fd.ftLastWriteTime;
	return TRUE;
}

inline BOOL CFileFind::GetLastAccessTime(FILETIME& rTimeStamp) const
{
	rTimeStamp=m_fd.ftLastAccessTime;
	return TRUE;
}

inline BOOL CFileFind::GetCreationTime(FILETIME& rTimeStamp) const
{
	rTimeStamp=m_fd.ftCreationTime;
	return TRUE;
}

inline BOOL CFileFind::GetLastWriteTime(CTime& refTime) const
{
	refTime=m_fd.ftLastWriteTime;
	return TRUE;
}

inline BOOL CFileFind::GetLastAccessTime(CTime& refTime) const
{
	refTime=m_fd.ftLastAccessTime;
	return TRUE;
}

inline BOOL CFileFind::GetCreationTime(CTime& refTime) const
{
	refTime=m_fd.ftCreationTime;
	return TRUE;
}
#else

inline USHORT CFileFind::GetFileTime() const
{
	return m_fd.ff_ftime;
}

inline USHORT CFileFind::GetFileDate() const
{
	return m_fd.ff_fdate;
}
#endif

inline BOOL CFileFind::MatchesMask(DWORD dwMask) const
{
#ifdef WIN32
	return (m_fd.dwFileAttributes&dwMask);
#else
	return (m_fd.ff_attrib&dwMask);
#endif
}

inline BOOL CFileFind::IsReadOnly() const
{
#ifdef WIN32
	return MatchesMask(FILE_ATTRIBUTE_READONLY);
#else
	return MatchesMask(FA_RDONLY);
#endif
}

inline BOOL CFileFind::IsDirectory() const
{
#ifdef WIN32
	return MatchesMask(FILE_ATTRIBUTE_DIRECTORY);
#else
	return MatchesMask(FA_DIREC);
#endif
}

inline BOOL CFileFind::IsSystem() const
{
#ifdef WIN32
	return MatchesMask(FILE_ATTRIBUTE_SYSTEM);
#else
	return MatchesMask(FA_SYSTEM);
#endif
}

inline BOOL CFileFind::IsHidden() const
{
#ifdef WIN32
	return MatchesMask(FILE_ATTRIBUTE_HIDDEN);
#else
	return MatchesMask(FA_HIDDEN);
#endif
}

inline BOOL CFileFind::IsNormal() const
{
#ifdef WIN32
	return MatchesMask(FILE_ATTRIBUTE_NORMAL);
#else
	return MatchesMask(0);
#endif
}

inline BOOL CFileFind::IsArchived() const
{
#ifdef WIN32
	return MatchesMask(FILE_ATTRIBUTE_ARCHIVE);
#else
	return MatchesMask(FA_ARCH);
#endif
}

#ifdef WIN32
inline BOOL CFileFind::IsCompressed() const
{
	return MatchesMask(FILE_ATTRIBUTE_COMPRESSED);
}

inline BOOL CFileFind::IsTemporary() const
{
	return MatchesMask(FILE_ATTRIBUTE_TEMPORARY);
}

#endif


////////////////////////////////////////
// CSearchFromFile

HFCERROR SetHFCError2(HFCERROR,int,char*);

inline CSearchFromFile::CSearchFromFile()
:	hFile(NULL)
{
}

inline CSearchHexFromFile::CSearchHexFromFile(const BYTE* pData,DWORD dwLength)
:	CSearchFromFile(),pBuffer(NULL),bMatchCase(TRUE)
{
	// Setting data
	pSearchValue=new BYTE[this->dwLength=dwLength];
	for (register int i=dwLength-1;i>=0;i--)
		pSearchValue[i]=pData[i];
}

inline CSearchHexFromFile::CSearchHexFromFile(LPCSTR szString,BOOL bMatchCase)
:	CSearchFromFile(),pBuffer(NULL)
{
	this->bMatchCase=bMatchCase;

	// Setting data
	dwLength=istrlen(szString);
#ifdef WIN32
	pSearchValue=new BYTE[dwLength];
	for (register LONG_PTR i=dwLength-1;i>=0;i--)
		pSearchValue[i]=szString[i];
	if (!bMatchCase)
		CharLowerBuff(LPSTR(pSearchValue),(DWORD)dwLength);
#else
	if (!bMatchCase)
	{
		pSearchValue=new BYTE[dwLength+1];
		for (register int i=dwLength;i>=0;i--)
			pSearchValue[i]=szString[i];
		strlwr(LPSTR(pSearchValue));
		return;
	}

	pSearchValue=new BYTE[dwLength];
	for (register int i=dwLength-1;i>=0;i--)
		pSearchValue[i]=szString[i];
#endif
}

inline CSearchHexFromFile::CSearchHexFromFile(LPCSTR szString,DWORD dwLength,BOOL bMatchCase)
:	CSearchFromFile(),pBuffer(NULL),bMatchCase(TRUE)
{
	this->bMatchCase=bMatchCase;

#ifdef WIN32
	// Setting data
	pSearchValue=new BYTE[this->dwLength=dwLength];
	for (register LONG_PTR i=this->dwLength-1;i>=0;i--)
		pSearchValue[i]=szString[i];
	if (!bMatchCase)
		CharLowerBuff(LPSTR(pSearchValue),dwLength);
#else
	// Setting data
	if (!bMatchCase)
	{
		this->dwLength=dwLength;
		pSearchValue=new BYTE[dwLength+1];
		for (register int i=this->dwLength-1;i>=0;i--)
			pSearchValue[i]=szString[i];  
		pSearchValue[dwLength]='\0';
		strlwr(LPSTR(pSearchValue));
		return;
	}	
	pSearchValue=new BYTE[this->dwLength=dwLength];
	for (register int i=this->dwLength-1;i>=0;i--)
		pSearchValue[i]=szString[i];
#endif
}

inline CSearchHexFromFile::CSearchHexFromFile(BYTE bNumber)
:	CSearchFromFile(),pBuffer(NULL),bMatchCase(TRUE)
{
	// Setting data
	pSearchValue=new BYTE[2];
	dwLength=1;
	*pSearchValue=bNumber;
}

inline CSearchHexFromFile::CSearchHexFromFile(WORD wNumber)
:	CSearchFromFile(),pBuffer(NULL),bMatchCase(TRUE)
{
	// Setting data
	pSearchValue=new BYTE[2];
	dwLength=2;
	*PWORD(pSearchValue)=wNumber;
}

inline CSearchHexFromFile::CSearchHexFromFile(DWORD dwNumber)
:	CSearchFromFile(),pBuffer(NULL),bMatchCase(TRUE)
{
	// Setting data
	pSearchValue=new BYTE[4];
	dwLength=4;
	*PDWORD(pSearchValue)=dwNumber;
}


#endif
