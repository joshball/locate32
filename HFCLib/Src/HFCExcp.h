////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2004 Janne Huttunen
////////////////////////////////////////////////////////////////////


#ifndef HFCEXCEPTIONS_H
#define HFCEXCEPTIONS_H

class CException
{
public:
	enum exceptionCode {
		none=0,
		cannotAllocate=1,
		cannotReadResource=2,
		invalidFormat=3,
		cannotExecute=4,
		cannotCreateHeap=5,
		cannotDiscard=6,
		obsoleteFunction=7,
		notImplemented=8,
		corruptData=9,
		invalidParameter=10,
		noAccess=11,
		unknown=0xFFFF
	};

#ifdef WIN32
	LONG    m_lOsError;
	virtual int ReportError(UINT nType=MB_OK,UINT nMessageID=0);
#else
	virtual int ReportError();
#endif

#ifdef WIN32
	CException(exceptionCode cause = CException::none,LONG lOsError = -1);
#else
	CException(exceptionCode cause = CException::none);
#endif

	virtual HFCERROR GetHFCErrorCode() const;

	virtual BOOL GetErrorMessage(LPTSTR lpszError,UINT nMaxError);
	exceptionCode     m_cause;

};




class CFileException : public CException
{
public:
	enum exceptionCode {
		none=0,
		generic=0x1000,
		fileNotFound=0x1001,
		badPath=0x1002,
		tooManyOpenFiles=0x1003,
		accessDenied=0x1004,
		invalidFile=0x1005,
		fileCorrupt=0x1006,
		fileExist=0x1007,
		removeCurrentDir=0x1008,
		directoryFull=0x1009,
		badSeek=0x100A,
		hardIO=0x100B,
		sharingViolation=0x100C,
		lockViolation=0x100D,
		writeProtected=0x100E,
		writeFault=0x100F,
		readFault=0x1010,
		fileCreate=0x1011,
		diskFull=0x1012,
		endOfFile=0x1013,
		fileOpen=0x1014,

		unknown=0xFFFF
	};

	

#ifdef WIN32
	CFileException(exceptionCode cause = CFileException::none, LONG lOsError = -1,
		LPCTSTR lpszArchiveName = NULL);
	CFileException(CException::exceptionCode cause, LONG lOsError = -1,
		LPCTSTR lpszArchiveName = NULL);
#else
	CFileException(exceptionCode cause = CFileException::none, LPCSTR lpszArchiveName = NULL);
	CFileException(CException::exceptionCode cause, LPCSTR lpszArchiveName = NULL);
#endif
	virtual ~CFileException();

	virtual HFCERROR GetHFCErrorCode() const;

	CString m_strFileName;

#ifdef WIN32
	static CException::exceptionCode OsErrorToException(LONG lOsError);
#endif
	virtual BOOL GetErrorMessage(LPTSTR lpszError,UINT nMaxError);
};



#ifdef WIN32
inline CException::CException(CException::exceptionCode cause,LONG lOsError)
:	m_cause(cause),m_lOsError(lOsError)
{
}

inline CFileException::CFileException(CFileException::exceptionCode cause,LONG lOsError,LPCTSTR lpszArchiveName)
:	CException((CException::exceptionCode)cause,lOsError)
{
	m_strFileName=lpszArchiveName;
}
inline CFileException::CFileException(CException::exceptionCode cause,LONG lOsError,LPCTSTR lpszArchiveName)
:	CException(cause,lOsError)
{
	m_strFileName=lpszArchiveName;
}
#else
inline CFileException::CFileException(CFileException::exceptionCode cause,LPCSTR lpszArchiveName)
:	CException((CException::exceptionCode)cause)
{
        m_strFileName=lpszArchiveName;
}
inline CFileException::CFileException(CException::exceptionCode cause,LPCSTR lpszArchiveName)
:	CException(cause)
{
        m_strFileName=lpszArchiveName;
}
#endif



#endif

