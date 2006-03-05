////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

LONGLONG GetDiskFreeSpace(LPCSTR szDrive)
{
#ifdef WIN32
	ULARGE_INTEGER i64FreeBytesToCaller;
	BOOL (CALLBACK *pGetDiskFreeSpaceEx)(LPCTSTR lpDirectoryName,
		PULARGE_INTEGER lpFreeBytesAvailable,PULARGE_INTEGER lpTotalNumberOfBytes,
		PULARGE_INTEGER lpTotalNumberOfFreeBytes);
	*((FARPROC*)&pGetDiskFreeSpaceEx) = GetProcAddress( GetModuleHandle("kernel32.dll"),
                         "GetDiskFreeSpaceExA");
	if (pGetDiskFreeSpaceEx!=NULL)
	{
		ULARGE_INTEGER i64TotalBytes,i64FreeBytes;
		if (!pGetDiskFreeSpaceEx (szDrive,&i64FreeBytesToCaller,&i64TotalBytes,&i64FreeBytes))
			return 0;
	}
	else
	{
		DWORD nSectors,nBytes,nFreeClusters,nTolalClusters;
		if (!GetDiskFreeSpace(szDrive,&nSectors,
			&nBytes,&nFreeClusters,&nTolalClusters))
			return 0;
		i64FreeBytesToCaller.QuadPart=nBytes*nSectors*nFreeClusters;
	}
	return i64FreeBytesToCaller.QuadPart;
#else
	UCHAR drive[2];
	drive[0]=szDrive[0];
	drive[1]='\0';
	strupr((char*)drive);
	
	struct dfree df;
	getdfree(drive[0]-'A'+1,&df);
	return df.df_bsec*df.df_sclus*df.df_avail;
#endif
}

/* OBSOLETE, use CFile::IsFile
BYTE IsFile(LPCTSTR File)
{
	HANDLE hFile;
#ifdef WIN32
	hFile=CreateFile(File,0,
		FILE_SHARE_READ,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return FALSE;
	CloseHandle(hFile);
#else
	hFile=(FILE*)fopen(File,"rb");
	if (hFile==NULL)
		return FALSE;
	fclose((FILE*)hFile);
#endif
	return TRUE;
}
*/

#ifndef WIN32

BYTE CopyFile(LPCTSTR src,LPCTSTR dst)
{
	LPSTR buffer;
	FILE *fp;
	DWORD Size;
	UINT temp,temp2;

	fp=fopen(src,"rb");
	if (fp==NULL)
	{
		SetHFCError(HFC_CANNOTOPEN);
		return FALSE;
	}
	Size=filelength(fileno(fp));
	buffer=new char[Size+2];
	if (buffer==NULL)
	{
		fclose(fp);
		SetHFCError(HFC_CANNOTALLOC);
		return FALSE;
	}
	if (!fread(buffer,1,Size,fp))
	{
		delete[] buffer;
		fclose(fp);
		SetHFCError(HFC_CANNOTREAD);
		return FALSE;
	}
	_dos_getftime(fileno(fp),&temp,&temp2);
	fclose(fp);
	fp=fopen(src,"wb");
	if (fp==NULL)
	{
		delete[] buffer;
		SetHFCError(HFC_CANNOTCREATE);
		return FALSE;
	}
	if (fwrite(buffer,1,Size,fp))
	{
		delete[] buffer;
		SetHFCError(HFC_CANNOTWRITE);
		return FALSE;
	}
	_dos_setftime(fileno(fp),temp,temp2);
	fclose(fp);
	_dos_getfileattr(src,&temp);
	_dos_setfileattr(dst,temp);
	return TRUE;
}

#endif

/////////////////////////////////////////////
// CFile

#ifndef WIN32

CFile::CFile()
:	m_bCloseOnDelete(FALSE),m_nOpenFlags(0)
{
	m_hFile=(HANDLE)hFileNull;
}

CFile::CFile(BOOL bThrowExceptions)
:	m_bCloseOnDelete(FALSE),m_nOpenFlags(0),CExceptionObject(bThrowExceptions)
{
	m_hFile=(HANDLE)hFileNull;
}

CFile::CFile(HANDLE hFile,BOOL bThrowExceptions)
:	m_hFile(hFile),m_bCloseOnDelete(FALSE),m_nOpenFlags(0),
	CExceptionObject(bThrowExceptions)
{
}

CFile::CFile(FILE* pFile,BOOL bThrowExceptions)
:	m_hFile(HANDLE(pFile)),m_bCloseOnDelete(FALSE),m_nOpenFlags(0),
	CExceptionObject(bThrowExceptions)
{
}

CFile::CFile(LPCSTR lpszFileName,int nOpenFlags,CFileException* e)
:	m_hFile(HANDLE(hFileNull)),m_bCloseOnDelete(FALSE)
{
	Open(lpszFileName,nOpenFlags,e);
}

CFile::CFile(LPCWSTR lpszFileName,int nOpenFlags,CFileException* e)
:	m_hFile(HANDLE(hFileNull)),m_bCloseOnDelete(FALSE)
{
	Open(lpszFileName,nOpenFlags,e);
}

CFile::CFile(LPCSTR lpszFileName,int nOpenFlags,bThrowExceptions)
:	m_hFile(HANDLE(hFileNull)),m_bCloseOnDelete(FALSE),
	CExceptionObject(bThrowExceptions)
{
	Open(lpszFileName,nOpenFlags,e);
}

CFile::CFile(LPCWSTR lpszFileName,int nOpenFlags,bThrowExceptions)
:	m_hFile(HANDLE(hFileNull)),m_bCloseOnDelete(FALSE),
	CExceptionObject(bThrowExceptions)
{
	Open(lpszFileName,nOpenFlags,e);
}

#endif




BOOL CFile::GetStatus(CFileStatus& rStatus) const
{
#ifdef WIN32
	BY_HANDLE_FILE_INFORMATION fi;
	if (!GetFileInformationByHandle(m_hFile,&fi))
		return FALSE;
	rStatus.m_ctime=fi.ftCreationTime;
	rStatus.m_mtime=fi.ftLastWriteTime;
	rStatus.m_atime=fi.ftLastAccessTime;
	rStatus.m_size=fi.nFileSizeLow;
	rStatus.m_attribute=fi.dwFileAttributes;
#else
	struct ffblk find;
	if (findfirst((LPCTSTR)m_strFileName,&find,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_LABEL|FA_DIREC|FA_ARCH))
		return FALSE;
	CTime tim(find.ff_fdate,find.ff_ftime);
	rStatus.m_ctime=tim;
	rStatus.m_mtime=tim;
	rStatus.m_atime=tim;
	rStatus.m_size=find.ff_fsize;
	rStatus.m_attribute=find.ff_attrib;
#endif
	StringCbCopy(rStatus.m_szFullName,MAX_PATH,m_strFileName);
	return TRUE;
}


CString CFile::GetFileTitle() const
{
#ifdef WIN32
	CString str;
	::GetFileTitle(m_strFileName,str.GetBuffer(2000),2000);
	str.FreeExtra();
#else
	CString str(m_strFileName+LastCharIndex(m_strFileName,'\\')+1);
#endif
	return str;
}

CString CFile::GetFilePath() const
{
#ifdef WIN32
	LPTSTR temp;
	CString str;
	GetFullPathName(m_strFileName,2000,str.GetBuffer(2000),&temp);
	str.FreeExtra();
	return str;
#else
	return m_strFileName;	
#endif
}

BOOL CFile::Open(LPCSTR lpszFileName, int nOpenFlags,CFileException* pError)
{
	if (m_bCloseOnDelete)
		Close();
	m_bCloseOnDelete=TRUE;
	m_nOpenFlags=nOpenFlags;
#ifdef WIN32
	
	// Obtaining full path name
	LPSTR szTemp;
	DWORD dwLength=GetFullPathName(lpszFileName,MAX_PATH,m_strFileName.GetBuffer(MAX_PATH),&szTemp);
    if (dwLength==0)
		m_strFileName=lpszFileName;
	else if (dwLength>MAX_PATH)
		dwLength=GetFullPathName(lpszFileName,dwLength+2,m_strFileName.GetBuffer(dwLength+2),&szTemp);
	m_strFileName.FreeExtra(dwLength);
	

	DWORD dwShare;
	switch (nOpenFlags&shareFlags)
	{
	case shareCompat:
		dwShare=0;
		break;
	case shareDenyWrite:
		dwShare=FILE_SHARE_READ;
		break;
	case shareDenyRead:
		dwShare=FILE_SHARE_WRITE;
		break;
	case shareDenyNone:
		dwShare=FILE_SHARE_WRITE|FILE_SHARE_READ;
		break;
	default:
		if (m_bThrow)
			throw CException(CException::invalidParameter);
		dwShare=0;
		break;
	}


	if ((nOpenFlags&openFlags)==0)
	{
		if (nOpenFlags&modeWrite)
			nOpenFlags|=openCreateAlways;
		else
			nOpenFlags|=openExisting;
	}

	if (nOpenFlags&otherInherit)
	{
		SECURITY_ATTRIBUTES sa;
		sa.nLength=sizeof(sa);
		sa.lpSecurityDescriptor=NULL;
		sa.bInheritHandle=TRUE;
		m_hFile=::CreateFile(lpszFileName,nOpenFlags&modeFlags,dwShare,&sa,nOpenFlags&openFlags,FILE_ATTRIBUTE_NORMAL,NULL);
	}
	else
		m_hFile=::CreateFile(lpszFileName,nOpenFlags&modeFlags,dwShare,NULL,nOpenFlags&openFlags,FILE_ATTRIBUTE_NORMAL,NULL);

	
	if (m_hFile==INVALID_HANDLE_VALUE)
	{
		SetHFCError(HFC_CANNOTOPEN);
		if (pError!=NULL)
		{
			pError->m_lOsError = ::GetLastError();
			pError->m_cause = CFileException::OsErrorToException(pError->m_lOsError);
			pError->m_strFileName = lpszFileName;
			if (m_bThrow)
				throw *pError;
		}
		else if (m_bThrow)
			throw CFileException(CFileException::OsErrorToException(::GetLastError()),::GetLastError(),lpszFileName);
		return FALSE;
	}
	else if (pError!=NULL)
		pError->m_cause=CException::none;
	return TRUE;
#else
	char mode[4]={ 0,0,0,0 };

	m_strFileName = lpszFileName;
	
	switch (nOpenFlags&(openFlags|modeFlags))
	{
	case modeWrite|openCreateNew:
		if (IsFile(lpszFileName))
			return FALSE;
	case modeWrite|openCreateAlways:
		mode[0]='w';
		break;
	case modeWrite|openTruncate:
		if (!IsFile(lpszFileName))
			return FALSE;
		mode[0]='w';
		break;
	case modeWrite|openExisting:
		if (!IsFile(lpszFileName))
			return FALSE;
	case modeWrite|openAlways:
		mode[0]='a';
		break;
	case modeReadWrite|openTruncate:
		if (!IsFile(lpszFileName))
			return FALSE;
	case modeReadWrite|openCreateAlways:
		mode[0]='w';
		mode[1]='+';
        break;
	case modeReadWrite|openCreateNew:
		if (IsFile(lpszFileName))
			return FALSE;
		mode[0]='w';
		mode[1]='+';
        break;
	case modeReadWrite|openExisting:
		if (!IsFile(lpszFileName))
			return FALSE;
	case modeReadWrite|openAlways:
		mode[0]='r';
		mode[1]='+';
		break;
	case modeReadWrite|modeNoTruncate|modeCreate:
		mode[0]='a';
		mode[1]='+';
		break;
	default:
		mode[0]='r';
		break;
	}
	if (nOpenFlags&typeText)
		strcat(mode,"t");
	else
		strcat(mode,"b");

        m_hFile=(HANDLE)fopen(lpszFileName,mode);
	if (m_hFile==NULL)
	{
		SetHFCError(HFC_CANNOTCREATE);
		if (pError != NULL)
		{
			if (nOpenFlags&modeWrite)
				pError->m_cause = CFileException::invalidFile;
			else
				pError->m_cause = CFileException::fileNotFound;
			pError->m_strFileName = lpszFileName;
			if (m_bThrow)
				throw *pError;
		}
		else if (m_bThrow)
		{
			if (nOpenFlags&modeWrite)
				throw CFileException(CFileException::invalidFile,lpszFileName);
			else
				throw CFileException(CFileException::fileNotFound,lpszFileName);
		}
		return FALSE;
	}
	return TRUE;
#endif
}

#ifdef DEF_WCHAR

BOOL CFile::Open(LPCWSTR lpszFileName, int nOpenFlags,CFileException* pError)
{
	if (m_bCloseOnDelete)
		Close();
	m_bCloseOnDelete=TRUE;
	m_nOpenFlags=nOpenFlags;
	
	// Obtaining full path name
	LPSTR szTemp;
	char szAnsiPath[MAX_PATH];
	WideCharToMultiByte(CP_ACP,0,lpszFileName,-1,szAnsiPath,MAX_PATH,NULL,NULL);

	DWORD dwLength=GetFullPathName(szAnsiPath,MAX_PATH,m_strFileName.GetBuffer(MAX_PATH),&szTemp);
    if (dwLength==0)
		m_strFileName=lpszFileName;
	else if (dwLength>MAX_PATH)
		dwLength=GetFullPathName(szAnsiPath,dwLength+2,m_strFileName.GetBuffer(dwLength+2),&szTemp);
	m_strFileName.FreeExtra(dwLength);
	

	DWORD dwShare;
	switch (nOpenFlags&shareFlags)
	{
	case shareCompat:
		dwShare=0;
		break;
	case shareDenyWrite:
		dwShare=FILE_SHARE_READ;
		break;
	case shareDenyRead:
		dwShare=FILE_SHARE_WRITE;
		break;
	case shareDenyNone:
		dwShare=FILE_SHARE_WRITE|FILE_SHARE_READ;
		break;
	default:
		if (m_bThrow)
			throw CException(CException::invalidParameter);
		dwShare=0;
		break;
	}


	if ((nOpenFlags&openFlags)==0)
	{
		if (nOpenFlags&modeWrite)
			nOpenFlags|=openCreateAlways;
		else
			nOpenFlags|=openExisting;
	}

	if (nOpenFlags&otherInherit)
	{
		SECURITY_ATTRIBUTES sa;
		sa.nLength=sizeof(sa);
		sa.lpSecurityDescriptor=NULL;
		sa.bInheritHandle=TRUE;
		m_hFile=::CreateFileW(lpszFileName,nOpenFlags&modeFlags,dwShare,&sa,nOpenFlags&openFlags,FILE_ATTRIBUTE_NORMAL,NULL);
	}
	else
		m_hFile=::CreateFileW(lpszFileName,nOpenFlags&modeFlags,dwShare,NULL,nOpenFlags&openFlags,FILE_ATTRIBUTE_NORMAL,NULL);

	
	if (m_hFile==INVALID_HANDLE_VALUE)
	{
		SetHFCError(HFC_CANNOTOPEN);
		if (pError!=NULL)
		{
			pError->m_lOsError = ::GetLastError();
			pError->m_cause = CFileException::OsErrorToException(pError->m_lOsError);
			pError->m_strFileName = lpszFileName;
			if (m_bThrow)
				throw *pError;
		}
		else if (m_bThrow)
			throw CFileException(CFileException::OsErrorToException(::GetLastError()),::GetLastError(),m_strFileName);
		return FALSE;
	}
	else if (pError!=NULL)
		pError->m_cause=CException::none;
	return TRUE;
}
#endif

BOOL CFile::GetStatus(LPCTSTR lpszFileName,CFileStatus& rStatus)
{
#ifdef WIN32
	HANDLE hFile;
	BY_HANDLE_FILE_INFORMATION fi;
	LPTSTR temp;
	if (!GetFullPathName(lpszFileName,_MAX_PATH,rStatus.m_szFullName,&temp))
		return FALSE;
	
	hFile=CreateFile(rStatus.m_szFullName,0,
		FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return FALSE;
	
	if (!GetFileInformationByHandle(hFile,&fi))
	{
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	rStatus.m_ctime=fi.ftCreationTime;
	rStatus.m_mtime=fi.ftLastWriteTime;
	rStatus.m_atime=fi.ftLastAccessTime;
	rStatus.m_size=fi.nFileSizeLow;
	rStatus.m_attribute=fi.dwFileAttributes;
#else
	struct ffblk find;
	if (findfirst(lpszFileName,&find,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_LABEL|FA_DIREC|FA_ARCH))
		return FALSE;
	CTime tim(find.ff_fdate,find.ff_ftime);
	rStatus.m_ctime=tim;
	rStatus.m_mtime=tim;
	rStatus.m_atime=tim;
	rStatus.m_size=find.ff_fsize;
	rStatus.m_attribute=find.ff_attrib;
	strcpy(rStatus.m_szFullName,lpszFileName);
#endif
	return TRUE;
}

BOOL  CFile::SetStatus(LPCTSTR lpszFileName,const CFileStatus& status)
{
#ifdef WIN32
	HANDLE hFile;
	hFile=CreateFile(lpszFileName,GENERIC_WRITE,
		FILE_SHARE_READ,NULL,OPEN_EXISTING,
		status.m_attribute,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return FALSE;
	if (!SetFileTime(hFile,&((FILETIME)status.m_ctime),&((FILETIME)status.m_atime),&((FILETIME)status.m_mtime)))
	{
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	return SetFileAttributes(lpszFileName,status.m_attribute);
#else
	UINT ftime,fdate;
	FILE* fp=fopen(lpszFileName,"ab");
	if (fp==NULL)
		return FALSE;
	fdate=(status.m_ctime.GetYear()-80)<<9+status.m_ctime.GetMonth()<<5+status.m_ctime.GetDay();
	ftime=status.m_ctime.GetHour()<<11+status.m_ctime.GetMinute()<<5+status.m_ctime.GetSecond()>>1;
	_dos_setfileattr(lpszFileName,status.m_attribute);
	_dos_setftime(fileno(fp),fdate,ftime);
	fclose(fp);
	return TRUE;
#endif
}
#ifdef WIN32
LONG_PTR CFile::Seek(ULONG_PTR lOff, ULONG_PTR nFrom,CFileException* pError,LONG* pHighPos)
{
	LONG_PTR dwNew=::SetFilePointer(m_hFile,lOff,pHighPos,(DWORD)nFrom);
	if (dwNew  == (LONG_PTR)-1)
	{
		SetHFCError(HFC_BADSEEK);
		if (pError != NULL)
		{
			pError->m_lOsError = ::GetLastError();
			pError->m_cause = CFileException::OsErrorToException(pError->m_lOsError);
			pError->m_strFileName = m_strFileName;
			if (m_bThrow)
				throw *pError;
		}
		else if (m_bThrow)
			throw CFileException(CFileException::OsErrorToException(::GetLastError()),::GetLastError(),m_strFileName);
		return FALSE;
	}
	return dwNew;
}
#else
LONG_PTR CFile::Seek(ULONG_PTR lOff,ULONG_PTR nFrom,CFileException* pError)
{
	LONG_PTR ret=fseek((FILE*)m_hFile,lOff,nFrom);
	if (ret)
	{
		SetHFCError(HFC_BADSEEK);
		if (pError != NULL)
		{
			pError->m_cause=CFileException::badSeek;
			pError->m_strFileName=m_strFileName;
			if (m_bThrow)
				throw *pError;
		}
		else if (m_bThrow)
			throw CFileException(CFileException::badSeek,m_strFileName);
		return FALSE;
	}
	return TRUE;
}
#endif

#ifdef WIN32
BOOL CFile::SetLength(DWORD dwNewLen,LONG* pHigh)
{
	this->Seek((LONG)dwNewLen,(UINT)begin,NULL,pHigh);
	return ::SetEndOfFile(m_hFile);
}
BOOL CFile::SetLength(ULONGLONG dwNewLen)
{
	LONG l=(LONG)(dwNewLen>>32);
	return SetLength((SIZE_T)dwNewLen,&l);
}
#endif

SIZE_T CFile::Read(void* lpBuf, SIZE_T nCount,CFileException* pError)
{
	if (nCount == 0)
		return 0;
#ifdef WIN32
	DWORD dwRead;
	BOOL ret=::ReadFile(m_hFile,lpBuf,nCount,&dwRead,NULL);
	if (ret && dwRead<nCount && m_nOpenFlags&otherErrorWhenEOF) 
	{
		SetHFCError(HFC_ENDOFFILE);
		if (pError!=NULL)
		{
			pError->m_lOsError = ERROR_HANDLE_EOF;
			pError->m_cause = (CException::exceptionCode)CFileException::endOfFile;
			pError->m_strFileName = m_strFileName;
			if (m_bThrow)
				throw *pError;
		}
		else if (m_bThrow)
			throw CFileException(CFileException::endOfFile,ERROR_HANDLE_EOF,m_strFileName);
		return 0;
	}
	else if (!ret)
	{
		SetHFCError(HFC_CANNOTREAD);
		if (pError!=NULL)
		{
			pError->m_lOsError = ::GetLastError();
			pError->m_cause = CFileException::OsErrorToException(pError->m_lOsError);
			pError->m_strFileName = m_strFileName;
			if (m_bThrow)
				throw *pError;
		}
		else if (m_bThrow)
			throw CFileException (CFileException::OsErrorToException(::GetLastError()),::GetLastError(),m_strFileName);
		return 0;
	}
	return (UINT)dwRead;
#else
	DWORD ret=fread(lpBuf,1,nCount,(FILE*)m_hFile);
	if (ret<nCount) // Error occured
	{
		if (feof((FILE*)m_hFile))
		{
			if (m_nOpenFlags&otherErrorWhenEOF)
			{
				if (pError!=NULL)
				{
					pError->m_cause = CFileException::endOfFile;
					pError->m_strFileName = m_strFileName;
				}
				else if (m_bThrow)
					throw CFileException (CFileException::endOfFile,m_strFileName);
			}
			return 0;
		}

		if (pError!=NULL)
		{
			pError->m_cause = CFileException::readFault;
			pError->m_strFileName = m_strFileName;
		}
		else if (m_bThrow)
			throw CFileException (CFileException::readFault,m_strFileName);
		return 0;
	}
	return ret;
#endif
}

BOOL CFile::Read(CStringA& str,CFileException* pError)
{
	str.Empty();
	CHAR bChar;

#ifdef WIN32
	DWORD dwRead;
	
	if (m_nOpenFlags&typeText)
	{
		BOOL ret=::ReadFile(m_hFile,&bChar,1,&dwRead,NULL);
		if (ret && dwRead)
		{
			do {
				if (bChar=='\r' || bChar=='\n' || bChar=='\t' || bChar==' ')
					return TRUE;	
				str<< bChar;
				ret=::ReadFile(m_hFile,&bChar,1,&dwRead,NULL);
			}
			while (ret && dwRead);        			
			return TRUE;
		}
		if (ret) // End of file
		{
			if (str.IsEmpty() && m_nOpenFlags&otherErrorWhenEOF)
			{
				SetHFCError(HFC_ENDOFFILE);
				if (pError!=NULL)
				{
					pError->m_lOsError = ERROR_HANDLE_EOF;
					pError->m_cause = (CException::exceptionCode)CFileException::endOfFile;
					pError->m_strFileName = m_strFileName;
					if (m_bThrow)
						throw *pError;
				}
				else if (m_bThrow)
					throw CFileException(CFileException::endOfFile,ERROR_HANDLE_EOF,m_strFileName);
                return 0;
			}
			return TRUE;
		}
	}
	else
	{
		BOOL ret=::ReadFile(m_hFile,&bChar,1,&dwRead,NULL);
		while (ret && dwRead)
		{
			if (bChar=='\0')
				return TRUE;
			str << bChar;
			ret=::ReadFile(m_hFile,&bChar,1,&dwRead,NULL);
		}
		if (ret) // End of file
		{
			if (str.IsEmpty() && m_nOpenFlags&otherErrorWhenEOF)
			{
				SetHFCError(HFC_ENDOFFILE);
				if (pError!=NULL)
				{
					pError->m_lOsError = ERROR_HANDLE_EOF;
					pError->m_cause = (CException::exceptionCode)CFileException::endOfFile;
					pError->m_strFileName = m_strFileName;
					if (m_bThrow)
						throw *pError;
				}
				else if (m_bThrow)
					throw CFileException(CFileException::endOfFile,ERROR_HANDLE_EOF,m_strFileName);
                return 0;
			}
			return TRUE;
		}
	}

	if (pError!=NULL)
	{
		pError->m_lOsError = ::GetLastError();
		pError->m_cause = CFileException::OsErrorToException(pError->m_lOsError);
		pError->m_strFileName = m_strFileName;
		if (m_bThrow)
			throw *pError;
	}
	else if (m_bThrow)
		throw CFileException (CFileException::OsErrorToException(::GetLastError()),::GetLastError(),m_strFileName);
	
	return FALSE;
#else
	if (m_nOpenFlags&typeText)
	{
		BOOL ret=::fread(&bChar,1,1,(FILE*)m_hFile);
		if (ret)
		{
			do {
				if (bChar=='\r' || bChar=='\n' || bChar=='\t' || bChar==' ')
					return TRUE;	
				str<< bChar;
				ret=::fread(&bChar,1,1,(FILE*)m_hFile);
			}
			while (ret);       
			return TRUE;
		}
	}
	else
	{
		DWORD ret=fread(&bChar,1,1,(FILE*)m_hFile);
		while (ret)
		{
			if (bChar=='\0')
				return TRUE;
			str << bChar;
			ret=fread(&bChar,1,1,(FILE*)m_hFile);
		}
	}
	if (pError!=NULL)
	{
		pError->m_cause = feof(FILE*)m_hFile)?CFileException::endOfFile:CFileException::readFault;
		pError->m_strFileName = m_strFileName;
		if (m_bThrow)
			throw *pError;
	}
	else if (m_bThrow)
		throw CFileException (feof(FILE*)m_hFile)?CFileException::endOfFile:CFileException::readFault,m_strFileName);
	return FALSE;
#endif
}

#ifdef DEF_WCHAR
BOOL CFile::Read(CStringW& str,CFileException* pError)
{
	str.Empty();
	WCHAR bChar;

#ifdef WIN32
	DWORD dwRead;
	
	if (m_nOpenFlags&typeText)
	{
		BOOL ret=::ReadFile(m_hFile,&bChar,2,&dwRead,NULL);
		if (ret && dwRead)
		{
			do {
				if (bChar==L'\r' || bChar==L'\n' || bChar==L'\t' || bChar==L' ')
					return TRUE;	
				str<< bChar;
				ret=::ReadFile(m_hFile,&bChar,2,&dwRead,NULL);
			}
			while (ret && dwRead);           			
			return TRUE;
		}
		if (ret) // End of file
		{
			if (str.IsEmpty() && m_nOpenFlags&otherErrorWhenEOF)
			{
				SetHFCError(HFC_ENDOFFILE);
				if (pError!=NULL)
				{
					pError->m_lOsError = ERROR_HANDLE_EOF;
					pError->m_cause = (CException::exceptionCode)CFileException::endOfFile;
					pError->m_strFileName = m_strFileName;
					if (m_bThrow)
						throw *pError;
				}
				else if (m_bThrow)
					throw CFileException(CFileException::endOfFile,ERROR_HANDLE_EOF,m_strFileName);
                return 0;
			}
			return TRUE;
		}
	}
	else
	{
		BOOL ret=::ReadFile(m_hFile,&bChar,2,&dwRead,NULL);
		while (ret && dwRead)
		{
			if (bChar=='\0')
				return TRUE;
			str << bChar;
			ret=::ReadFile(m_hFile,&bChar,2,&dwRead,NULL);
		}
		if (ret) // End of file
		{
			if (str.IsEmpty() && m_nOpenFlags&otherErrorWhenEOF)
			{
				SetHFCError(HFC_ENDOFFILE);
				if (pError!=NULL)
				{
					pError->m_lOsError = ERROR_HANDLE_EOF;
					pError->m_cause = (CException::exceptionCode)CFileException::endOfFile;
					pError->m_strFileName = m_strFileName;
					if (m_bThrow)
						throw *pError;
				}
				else if (m_bThrow)
					throw CFileException(CFileException::endOfFile,ERROR_HANDLE_EOF,m_strFileName);
                return 0;
			}
			return TRUE;
		}
	}

	if (pError!=NULL)
	{
		pError->m_lOsError = ::GetLastError();
		pError->m_cause = CFileException::OsErrorToException(pError->m_lOsError);
		pError->m_strFileName = m_strFileName;
		if (m_bThrow)
			throw *pError;
	}
	else if (m_bThrow)
		throw CFileException (CFileException::OsErrorToException(::GetLastError()),::GetLastError(),m_strFileName);
	
	return FALSE;
#else
	
	if (m_nOpenFlags&typeText)
	{
		BOOL ret=fread(&bChar,2,1,(FILE*)m_hFile);
		if (ret)
		{
			do {
				if (bChar==L'\r' || bChar==L'\n' || bChar==L'\t' || bChar==L' ')
					return TRUE;	
				str<< bChar;
				ret=fread(&bChar,2,1,(FILE*)m_hFile);
			}
			while (ret);         			
			return TRUE;
		}
	}
	else
	{
		DWORD ret=fread(&bChar,2,1,(FILE*)m_hFile);
		while (ret)
		{
			if (bChar=='\0')
				return TRUE;
			str << bChar;
			ret=fread(&bChar,2,1,(FILE*)m_hFile);
		}
	}

	if (pError!=NULL)
	{
		pError->m_cause = feof(FILE*)m_hFile)?CFileException::endOfFile:CFileException::readFault;
		pError->m_strFileName = m_strFileName;
		if (m_bThrow)
			throw *pError;
	}
	else if (m_bThrow)
		throw CFileException (feof(FILE*)m_hFile)?CFileException::endOfFile:CFileException::readFault,m_strFileName);
	return FALSE;
#endif
}
#endif

BOOL CFile::Write(const void* lpBuf, ULONG_PTR nCount,CFileException* pError)
{
	if (nCount == 0)
		return TRUE;
#ifdef WIN32
	DWORD nWritten;
	BOOL ret=::WriteFile(m_hFile,lpBuf,nCount,&nWritten,NULL);
	if (!ret || nWritten<nCount)
	{
		if (pError!=NULL)
		{
			pError->m_lOsError = ::GetLastError();
			pError->m_cause = CFileException::OsErrorToException(pError->m_lOsError);
			pError->m_strFileName = m_strFileName;
			if (m_bThrow)
				throw *pError;
		}
		else if (m_bThrow)
			throw CFileException(CFileException::OsErrorToException(::GetLastError()),::GetLastError(),m_strFileName);
		return FALSE;
	}
	return TRUE;
#else
	DWORD ret=fwrite(lpBuf,1,nCount,(FILE*)m_hFile);
	if (!ret)
	{
		if (pError!=NULL)
		{
			pError->m_cause = CFileException::writeFault;
			pError->m_strFileName = m_strFileName;
			if (m_bThrow)
				throw *pError;
		}
		else if (m_bThrow)
			throw CFileException(CFileException::writeFault,m_strFileName);
		return FALSE;
	}
	return TRUE;
#endif
}

BOOL CFile::Close()
{
	BOOL bError = FALSE;
	if (m_hFile != FILE_NULL)
#ifdef WIN32
		bError = !::CloseHandle(m_hFile);
#else
		bError = !fclose((FILE*)m_hFile);
#endif
	m_hFile = FILE_NULL;
	m_bCloseOnDelete = FALSE;
	m_strFileName.Empty();
	return bError;
}

BOOL CFile::IsEndOfFile() const
{
#ifdef WIN32
	DWORD dwPos=::SetFilePointer(m_hFile,0,NULL,FILE_CURRENT);
    if (dwPos==(DWORD)-1)
	{
		SetHFCError(HFC_ERROR);
		if (m_bThrow)
			throw CFileException(CFileException::badSeek,GetLastError(),m_strFileName);
		return FALSE;
	}
	return dwPos==::GetFileSize(m_hFile,NULL);
#else
	return feof((FILE*)m_hFile);
#endif
}

#ifdef WIN32
SIZE_T CFile::GetLength(PSIZE_T pHigh) const
{
	return ::GetFileSize(m_hFile,pHigh);
}
ULONGLONG CFile::GetLength64() const
{
	SIZE_T high;
	SIZE_T low=::GetFileSize(m_hFile,&high);

	return (((ULONGLONG)high)<<32)|((ULONGLONG)low);
}
#else
SIZE_T CFile::GetLength() const
{
	return filelength(fileno((FILE*)m_hFile));
}
#endif

#ifdef WIN32
void CFile::LockRange(DWORD dwPos, DWORD dwCount)
{
	::LockFile(m_hFile,dwPos,0,dwCount,0);
}

void CFile::UnlockRange(DWORD dwPos, DWORD dwCount)
{
	::UnlockFile(m_hFile,dwPos,0,dwCount,0);
}
#endif

BOOL CFile::Flush()
{
#ifdef WIN32
	return ::FlushFileBuffers(m_hFile);
#else
	return (BYTE)::fflush((FILE*)m_hFile);
#endif
}


	

ULONG_PTR CFile::GetPosition(PLONG pHigh) const
{
#ifdef WIN32
	DWORD dwPos=::SetFilePointer(m_hFile,0,pHigh,FILE_CURRENT);
	if (dwPos==(DWORD)-1)
	{
		SetHFCError(HFC_ERROR);
		if (m_bThrow)
			throw CFileException(CFileException::badSeek,GetLastError(),m_strFileName);
		return FALSE;
	}
	return dwPos;
#else
	return (DWORD)ftell((FILE*)m_hFile);
#endif
}



#ifdef WIN32
ULONGLONG CFile::GetPosition64() const
{
	LONG high;
	DWORD dwPos=::SetFilePointer(m_hFile,0,&high,FILE_CURRENT);
	if (dwPos==(DWORD)-1)
	{
		SetHFCError(HFC_ERROR);
		if (m_bThrow)
			throw CFileException(CFileException::badSeek,GetLastError(),m_strFileName);
		return 0;
	}
	return (((ULONGLONG)high)<<32)|((ULONGLONG)dwPos);
}
#endif


BOOL CFile::IsFile(LPCTSTR szFileName)
{
	if (szFileName[0]=='\0')
		return FALSE;
#ifdef WIN32
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	int ret=TRUE;
	hFind=FindFirstFile(szFileName,&fd);
	if (hFind!=INVALID_HANDLE_VALUE)
	{
		while (fd.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY && ret)
			ret=FindNextFile(hFind,&fd);
		FindClose(hFind);	
		return ret;
	}
	return FALSE;
#else
	struct ffblk fd;
	return !findfirst(szFileName,&fd,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_ARCH);
#endif
}

INT CFile::IsDirectory(LPCTSTR szDirectoryName)
{
#ifdef WIN32
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	int ret=TRUE;
	if (szDirectoryName[0]=='\0')
		return 0;
	if (szDirectoryName[1]=='\0')
		return 0;
	if (szDirectoryName[2]=='\0')
		return 0;
	if (szDirectoryName[1]==':' && szDirectoryName[2]=='\\' && szDirectoryName[3]=='\0')
	{
		switch (GetDriveType(szDirectoryName))
		{
		case DRIVE_UNKNOWN:
		case DRIVE_NO_ROOT_DIR:
			return 0;
		case DRIVE_FIXED:
			return 1;
		default:
			return 2;
		}
	}
	
	// Taking last '\\' 
	LPSTR szPath;
	DWORD dwPathLen;
	dstrlen(szDirectoryName,dwPathLen);
	if (szDirectoryName[dwPathLen-1]=='\\' && dwPathLen>3)
	{
		szPath=new char[dwPathLen+5];
		iMemCopy(szPath,szDirectoryName,--dwPathLen);
		szPath[dwPathLen]='\0';
	}
	else
		szPath=LPSTR(szDirectoryName);

	hFind=FindFirstFile(szPath,&fd);
	if (hFind!=INVALID_HANDLE_VALUE)
	{
		while (!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && ret)
			ret=FindNextFile(hFind,&fd);
	
		if (szPath!=szDirectoryName)
			delete[] szPath;

		FindClose(hFind);	
		if (ret)
		{
			if (szDirectoryName[0]=='\\')
			{
				if (szDirectoryName[1]=='\\')
					return 2;
				switch (GetDriveType(NULL))
				{
				case DRIVE_UNKNOWN:
				case DRIVE_NO_ROOT_DIR:
					return 0;
				case DRIVE_FIXED:
					return 1;
				default:
					return 2;
				}
 
			}
			if (szDirectoryName[1]==':' && szDirectoryName[2]=='\\')
			{
				char szTemp[4]="X:\\";
				szTemp[0]=szDirectoryName[0];

				switch (GetDriveType(szTemp))
				{
				case DRIVE_UNKNOWN:
					return 0;
				case DRIVE_FIXED:
					return 1;
				default:
					return 2;
				}
			}
		}
		return 0;
	}
	else if (szDirectoryName[0]=='\\' && szDirectoryName[1]=='\\')
	{
		// UNC share name
		if (szPath==szDirectoryName)
		{
			szPath=new char[dwPathLen+5];
			dMemCopy(szPath,szDirectoryName,dwPathLen);
		}		
		dMemCopy(szPath+dwPathLen,"\\*.*",5);

		hFind=FindFirstFile(szPath,&fd);
		delete[] szPath;
		if (hFind==INVALID_HANDLE_VALUE)
			return 0;
		// Is this really needed, e.g. \\pc\c$ does not have '.' in directory list
		//ret=1;
		//while ((fd.cFileName[0]!='.' || fd.cFileName[1]!='\0') && ret==1)
		//	ret=FindNextFile(hFind,&fd);
		FindClose(hFind);
		//return ret?2:0;
		return 2;
	}
	
	
	if (szPath!=szDirectoryName)
		delete[] szPath;
	return 0;
#else
	struct ffblk fd;
	return !findfirst(szDirectoryName,&fd,FA_DIREC);
#endif
}

BOOL CFile::IsValidFileName(LPCSTR szFile,LPSTR szShortName)
{
#ifdef WIN32
	if (szFile[0]=='\0')
		return FALSE;
	if (CFile::IsFile(szFile))
	{
		if (szShortName!=NULL)
			GetShortPathName(szFile,szShortName,_MAX_PATH);
		return TRUE;
	}
	HANDLE hFile=CreateFile(szFile,GENERIC_WRITE,
		FILE_SHARE_READ,NULL,CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return FALSE;
	CloseHandle(hFile);
	if (szShortName!=NULL)
		GetShortPathName(szFile,szShortName,_MAX_PATH);
	DeleteFile(szFile);
	return TRUE;
#else
	if (szFile[0]=='\0')
		return FALSE;
	if (CFile::IsFile(szFile))
		return TRUE;
	FILE* fp=fopen(szFile,"wb");
	if (fp==NULL)
		return FALSE;
	fclose(fp);
	remove(szFile);
	return TRUE;
#endif
}


// Last '//' is not counted, if exists
DWORD CFile::ParseExistingPath(LPCSTR szPath)
{
	DWORD dwLength=strlen(szPath);

	if (dwLength<2)
		return 0;
	if (dwLength==2 || dwLength==3)
	{
		if (szPath[1]==':')
		{
			char szTemp[]="X:\\";
			szTemp[0]=szPath[0];
			return GetDriveType(szTemp)==DRIVE_NO_ROOT_DIR?0:2;
		}
		return 0;
	}
	
	char* pTempPath=new char[dwLength+2];
	CopyMemory(pTempPath,szPath,dwLength+1);
	if (pTempPath[dwLength-1]!='\\')
	{
		pTempPath[dwLength]='\\';
		pTempPath[++dwLength]='\0';
	}
	
	while (!IsDirectory(pTempPath))
	{
		dwLength--;
		while (dwLength>0 && pTempPath[dwLength]=='\\')
			dwLength--;

		// Findling next '\\'
		while (dwLength>0 && pTempPath[dwLength]!='\\')
			dwLength--;
        if (dwLength==0)
		{
			delete[] pTempPath;
			return 0;
		}

		pTempPath[dwLength+1]='\0';
	}

	while (dwLength>0 && pTempPath[dwLength-1]=='\\')
		dwLength--;
	delete[] pTempPath;
	return dwLength;
}

BOOL CFile::IsValidPath(LPCSTR szPath,BOOL bAsDirectory)
{
	BOOL bRet=FALSE;

	int nExisting=ParseExistingPath(szPath);
	if (nExisting==0)
		return FALSE;

	if (szPath[nExisting]!='\\')
	{
		if (szPath[nExisting]!='\0')
			return FALSE; // Should not be possible
		return TRUE;
	}

	int nStart=nExisting,nFirstNonExisting=-1;
	for(;;)
	{
		// Next '\\' or '\0'
        int i;
		for (i=nStart+1;szPath[i]!='\0' && szPath[i]!='\\';i++);
			
		if (szPath[i]=='\0')
		{
			// Is possible to create file?
			if (bAsDirectory)
			{
				if (CreateDirectory(szPath,NULL))
				{
					bRet=TRUE;
					RemoveDirectory(szPath);
				}
			}
			else	
			{
				HANDLE hFile=CreateFile(szPath,GENERIC_WRITE,
					FILE_SHARE_READ,NULL,CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,NULL);
				if (hFile!=INVALID_HANDLE_VALUE)
				{
					CloseHandle(hFile);
					DeleteFile(szPath);

					bRet=TRUE;
				}
			}
			break;
		}

		char* pTemp=new char[i+2];
		CopyMemory(pTemp,szPath,i);
		pTemp[i]='\0';

		if (!CreateDirectory(pTemp,NULL))
		{
			delete[] pTemp;
			break;
		}

		delete[] pTemp;
		nStart=i;

	}
	
	// Removing created directories
	while (nStart>nExisting)
	{
		for (;szPath[nStart]!='\\' && nExisting<nStart;nStart--);

		if (nStart<=nExisting)
			break;

		char* pTemp=new char[nStart+2];
		CopyMemory(pTemp,szPath,nStart);
		pTemp[nStart]='\0';
		RemoveDirectory(pTemp);
		delete[] pTemp;
        
		nStart--;
	}

	return bRet;
}

BOOL CFile::CreateDirectoryRecursive(LPCSTR szPath,LPSECURITY_ATTRIBUTES plSecurityAttributes)
{
	BOOL bRet=FALSE;

	int nExisting=ParseExistingPath(szPath);
	if (nExisting==0)
		return FALSE;

	if (szPath[nExisting]!='\\')
	{
		if (szPath[nExisting]!='\0')
			return FALSE; // Should not be possible
		return TRUE;
	}



	int nStart=nExisting,nFirstNonExisting=-1;
	for(;;)
	{
		// Next '\\' or '\0'
		int i;
        for (i=nStart+1;szPath[i]!='\0' && szPath[i]!='\\';i++);
					
		if (szPath[i]=='\0')
			return CreateDirectory(szPath,NULL);

		char* pTemp=new char[i+2];
		CopyMemory(pTemp,szPath,i);
		pTemp[i]='\0';

		if (!CreateDirectory(pTemp,NULL))
		{
			delete[] pTemp;
			return FALSE;
		}

		delete[] pTemp;
		nStart=i;

	}
	return TRUE;
}

BOOL CFile::IsSamePath(LPCSTR szDir1,LPCSTR szDir2)
{
#ifdef WIN32
	char path1[MAX_PATH],path2[MAX_PATH];
	int nRet1,nRet2;
	nRet1=GetShortPathName(szDir1,path1,MAX_PATH);
	if (!nRet1)
		return strcasecmp(szDir1,szDir2)==0;
	nRet2=GetShortPathName(szDir2,path2,MAX_PATH);
	if (!nRet2 || nRet1<nRet2-1 || nRet1>nRet2+1)
		return FALSE;
	if (path1[nRet1-1]=='\\')
		path1[nRet1-1]='\0';
	if (path2[nRet2-1]=='\\')
		path2[nRet2-1]='\0';
	return strcasecmp(path1,path2)==0;	
#else
	char path1[MAX_PATH],path2[MAX_PATH];
	int nRet1,nRet2;
	dstrlen(szDir1,nRet1);
	dstrlen(szDir2,nRet2);

	iMemCopy(path1,szDir1,nRet1);
	iMemCopy(path2,szDir2,nRet2);
	if (path1[nRet1-1]=='\\')
		path1[nRet1-1]='\0';
	if (path2[nRet2-1]=='\\')
		path2[nRet2-1]='\0';
	return strcasecmp(path1,path2)==0;	
#endif
}

// Checking whether szSubDir is sub directory of the szPath
BOOL CFile::IsSubDirectory(LPCSTR szSubDir,LPCSTR szPath)
{
	char sSubDir[MAX_PATH],sPath[MAX_PATH];
#ifdef WIN32
	if (!GetShortPathName(szSubDir,sSubDir,MAX_PATH))
		StringCbCopy(sSubDir,MAX_PATH,szSubDir);
	if (!GetShortPathName(szPath,sPath,MAX_PATH))
		StringCbCopy(sPath,MAX_PATH,szPath);
#else
	{
		char* p1;
		for (p1=szSubDir,int i=0;*p1!=0;p1++,i++)
		{
			if (*p1=='\\')
			{
				while (*p1=='\\')
					p1++;
				sSubDir[i]=='\';
			}
			else
				sSubDir[i]=*p1;
		}
		for (p1=szPath,int i=0;*p1!=0;p1++,i++)
		{
			if (*p1=='\\')
			{
				while (*p1=='\\')
					p1++;
				sPath[i]=='\';
			}
			else
				sPath[i]=*p1;
		}
	}
#endif
    int nSlashes=0,i;
	// Counting '\\' characters in szPath
	for (i=0;sPath[i]!='\0';i++)
	{
		if (sPath[i]=='\\')
			nSlashes++;
	}

	// Break szSubDir after nSlashes pcs of '\\'
	for (i=0;sSubDir[i]!='\0';i++)
	{
		if (sSubDir[i]=='\\')
		{
			nSlashes--;
			if (nSlashes<0)
				break;
		}
	}

	if (nSlashes>=0) // Not enough '\\' directories, cannot be subdir
		return FALSE;
	sSubDir[i]='\0';

	return strcasecmp(sSubDir,sPath)==0;
}

///////////////////////////////////////////
// CFileFind
///////////////////////////////////////////

#ifdef WIN32
void CFileFind::Close()
{
	if (m_hFind!=NULL)
	{
		FindClose(m_hFind);
		m_hFind=NULL;
	}
}
#endif

BOOL CFileFind::FindFile(LPCTSTR pstrName)
{
	strRoot.Copy(pstrName,LastCharIndex(pstrName,'\\')+1);
#ifdef WIN32
	if (m_hFind!=NULL)
		::FindClose(m_hFind);
	if (pstrName!=NULL)
		m_hFind=::FindFirstFile(pstrName,&m_fd);
	else
		m_hFind=::FindFirstFile("*.*",&m_fd);
	if (m_hFind==INVALID_HANDLE_VALUE)
		return FALSE;
	return TRUE;
#else
	if (pstrName==NULL)
        return !findfirst("*.*",&m_fd,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC|FA_ARCH);
	else
        return !findfirst(pstrName,&m_fd,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC|FA_ARCH);
#endif
}

BOOL CFileFind::FindNextFile()
{
#ifdef WIN32
	if (m_hFind==NULL)
		return FindFile();
	return ::FindNextFile(m_hFind,&m_fd);
#else
	return !findnext(&m_fd);
#endif
}

CString CFileFind::GetFilePath() const
{
	CString str(strRoot);
#ifdef WIN32
	str<<m_fd.cFileName;
#else
	str<<m_fd.ff_name;
#endif
	return str;
}

#ifdef WIN32

CString CFileFind::GetFileTitle() const
{
	CString temp(strRoot);
	temp << m_fd.cFileName;
	CString title;
	::GetFileTitle(temp,title.GetBuffer(_MAX_PATH),_MAX_PATH);
	title.FreeExtra();
	return title;
}

CString CFileFind::GetFileURL() const
{
	CString url("file://");
	url << strRoot << m_fd.cFileName;
	return url;
}
#endif

#ifndef WIN32

BOOL CFileFind::GetFileTime(CTime& refTime) const
{
	struct tm lt;
	lt.tm_sec=(m_fd.ff_ftime&~0xFFE0)<<1;
	lt.tm_min=(m_fd.ff_ftime&~0xF800)>>5;
	lt.tm_hour=m_fd.ff_ftime>>11;
	lt.tm_mday=m_fd.ff_fdate&~0xFFE0;
	lt.tm_mon=((m_fd.ff_fdate&~0xFE00)>>5)-1;
	lt.tm_year=(m_fd.ff_fdate>>9)+80;
	lt.tm_isdst=0;
	refTime.msec=0;
	refTime.m_time=mktime(&lt);
	return TRUE;
}
#endif

///////////////////////////////////////////
// CSearchFromFile
///////////////////////////////////////////

void CSearchFromFile::OpenFile(LPCSTR szFile)
{
	if (hFile!=NULL)
		this->CloseFile();
#ifdef WIN32
	hFile=CreateFile(szFile,GENERIC_READ,FILE_SHARE_READ,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		hFile=NULL;
#else
	hFile=fopen(szFile,"rb");
#endif

	if (hFile==NULL)
		SetHFCError2(HFC_CANNOTOPEN,__LINE__,__FILE__);

}

void CSearchFromFile::CloseFile()
{
#ifdef WIN32
	if (hFile!=NULL)
		CloseHandle(hFile);
#else
	if (hFile!=NULL)
		fclose(hFile);
#endif
	hFile=NULL;

}



///////////////////////////////////////////
// CSearchHexFromFile
///////////////////////////////////////////

#define SEARCH_BUFFERLEN	10000

CSearchHexFromFile::~CSearchHexFromFile()
{
	CloseFile();
	if (pSearchValue!=NULL)
		delete[] pSearchValue;
}

CSearchFromFile::~CSearchFromFile()
{
	this->CloseFile();
}

DWORD CSearchHexFromFile::GetFoundPosition() const
{
	return dwFilePtr+dwBufferPtr;
}

void CSearchHexFromFile::CloseFile()
{
#ifdef WIN32
	if (hFile!=NULL)
		CloseHandle(hFile);
#else
	if (hFile!=NULL)
		fclose(hFile);
#endif
	hFile=NULL;

	if (pBuffer!=NULL)
	{
		// Ensures that buffer is clear
		delete[] pBuffer;
		pBuffer=NULL;
	}
}

BOOL CSearchHexFromFile::Search(LPCSTR szFile)
{
    if (hFile==NULL)
	{
		OpenFile(szFile);
				
		
		if (hFile==NULL)
			return FALSE;
	}

	if (pBuffer==NULL)
	{
#ifdef WIN32
		dwFileSize=GetFileSize(hFile,NULL);
#else
		dwFileSize=filelength(fileno(hFile));
#endif
		
		if (dwFileSize<dwLength)
			return FALSE; // File size if smaller than length of search value

		dwBufferLen=min(SEARCH_BUFFERLEN,dwFileSize);
		pBuffer=new BYTE[max(dwBufferLen,1)+1];
		DWORD dwReaded;
#ifdef WIN32
		BOOL bRet=ReadFile(hFile,pBuffer,dwBufferLen,&dwReaded,NULL);
		if (!bRet || dwReaded<dwBufferLen)
		{
			SetHFCError(HFC_CANNOTREAD);
			return FALSE;
		}
#else
		dwReaded=fread(pBuffer,1,dwBufferLen,hFile);
		if (dwReaded<dwBufferLen)
		{
			SetHFCError(HFC_CANNOTREAD);
			return FALSE;
		}
#endif
		if (!bMatchCase)
		{
#ifdef WIN32
			CharLowerBuff(LPSTR(pBuffer),dwBufferLen);
#else
			pBuffer[dwBufferLen]='\0';
			strlwr(LPSTR(pBuffer));
#endif
		}
		
		dwBufferLen-=(dwLength-1);
#ifdef WIN32
		SetFilePointer(hFile,1-int(dwLength),NULL,FILE_CURRENT);
#else
		fseek(hFile,1-int(dwLength),SEEK_CUR);
#endif
		dwFilePtr=0;
		dwBufferPtr=0;
	}
	else
		dwBufferPtr++;

	while (1)
	{
		for (;dwBufferPtr<dwBufferLen;dwBufferPtr++)
		{
			DWORD i=0;
			for (;i<dwLength;i++)
			{
				if (pBuffer[dwBufferPtr+i]!=pSearchValue[i])
					break;
			}
			if (i==dwLength)
				return TRUE;
		}

		// Marking buffer readed
		dwFilePtr+=dwBufferLen;

		// Not enough of file left
		if (dwFileSize-dwFilePtr<dwLength)
			return FALSE;
		
		// Allocating new buffer if necessary
		if (dwFilePtr+SEARCH_BUFFERLEN>dwFileSize)
		{
			delete[] pBuffer;
			dwBufferLen=dwFileSize-dwFilePtr;
			pBuffer=new BYTE[max(1,dwBufferLen)+1];
		}
		else
			dwBufferLen=SEARCH_BUFFERLEN;
		
		DWORD dwReaded;
#ifdef WIN32
		BOOL bRet=ReadFile(hFile,pBuffer,dwBufferLen,&dwReaded,NULL);
		if (!bRet || dwReaded<dwBufferLen)
		{
			SetHFCError(HFC_CANNOTREAD);
			return FALSE;
		}
#else
		dwReaded=fread(pBuffer,1,dwBufferLen,hFile);
		if (dwReaded<dwBufferLen)
		{
			SetHFCError(HFC_CANNOTREAD);
			return FALSE;
		}

#endif
		if (!bMatchCase)
		{
#ifdef WIN32
			CharLowerBuff(LPSTR(pBuffer),dwBufferLen);
#else
            pBuffer[dwBufferLen]='\0';
            strlwr(LPSTR(pBuffer));
#endif
		}
		
		dwBufferLen-=(dwLength-1);
#ifdef WIN32
		SetFilePointer(hFile,1-int(dwLength),NULL,FILE_CURRENT);
#else
		fseek(hFile,1-int(dwLength),SEEK_CUR);
#endif
		dwBufferPtr=0;
	}
	return FALSE;
}
