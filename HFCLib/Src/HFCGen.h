////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////


#ifndef HFCGENERAL_H
#define HFCGENERAL_H

class CTime
{
public:
	static CTime PASCAL GetCurrentTime();
	
	CTime();
	CTime(time_t time,WORD ms=0);
	CTime(int nYear,int nMonth,int nDay,int nHour,int nMin,int nSec,int nMSec,int nDST=-1);
	CTime(WORD wDosDate, WORD wDosTime, int nDST=-1);
	CTime(const CTime& timeSrc);

#ifdef WIN32
	CTime(const SYSTEMTIME& sysTime, int nDST=-1);
	CTime(const FILETIME& fileTime, int nDST=-1);
#endif

	time_t m_time;
	WORD msec;
	
	operator time_t() const;
#ifdef WIN32
	operator SYSTEMTIME() const;
	operator FILETIME() const;
#endif

	const CTime& operator=(const CTime& timeSrc);
	const CTime& operator=(time_t t);
#ifdef WIN32
	const CTime& operator=(SYSTEMTIME& timeSrc);
	const CTime& operator=(FILETIME& timeSrc);
#endif

	struct tm* GetGmtTm(struct tm* ptm = NULL) const;
	struct tm* GetLocalTm(struct tm* ptm = NULL) const;

	time_t GetTime() const;
	int GetYear() const;
	int GetMonth() const;
	int GetDay() const;
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetMilliSecond() const;
	int GetDayOfWeek() const;   // 1=Sun, 2=Mon, ..., 7=Sat
	
	BOOL IsLeapYear() const;
	WORD DayOfYear() const;
	DWORD GetIndex() const;
	BYTE GetWeek(BYTE nFirstIsMonday=TRUE) const;
	BYTE GetDaysInMonth() const;
	
	static BOOL IsLeapYear(WORD nYear);
	static WORD LeapYears(WORD nYear);
	static WORD DayOfYear(WORD nDay,WORD nMonth,WORD nYear);
	static DWORD GetIndex(WORD nDay,WORD nMonth,WORD nYear);
	static CTime GetDayFromDayOfYear(WORD nDay,WORD nYear);
	static CTime GetDayFromIndex(DWORD nIndex);
	static BYTE GetDaysInMonth(BYTE nMonth,WORD nYear);
	static BYTE GetWeek(WORD nDay,WORD nMonth,WORD nYear,BYTE nFirstIsMonday=TRUE);
	static int GetDayOfWeek(WORD nDay,WORD nMonth,WORD nYear);  // 0=Sun, 1=Mon, ..., 6=Sat
			
	BOOL operator==(CTime time) const;
	BOOL operator!=(CTime time) const;
	BOOL operator<(CTime time) const;
	BOOL operator>(CTime time) const;
	BOOL operator<=(CTime time) const;
	BOOL operator>=(CTime time) const;

	CString Format(LPCSTR pFormat) const;
	CString FormatGmt(LPCSTR pFormat) const;
#ifdef DEF_RESOURCES
	CString Format(UINT nFormatID) const;
	CString FormatGmt(UINT nFormatID) const;
#endif

#ifdef DEF_WCHAR
	CStringW Format(LPCWSTR pFormat) const;
	CStringW FormatGmt(LPCWSTR pFormat) const;

#ifdef DEF_RESOURCES
	CStringW FormatW(UINT nFormatID) const;
	CStringW FormatGmtW(UINT nFormatID) const;
#endif

#endif

public:
	static BYTE Wait(DWORD nTime);
};

class CFileStatus
{
public:
	CTime m_ctime;
	CTime m_mtime;
	CTime m_atime;
	LONG m_size;
	DWORD m_attribute;
	TCHAR m_szFullName[MAX_PATH];
};

class CObject
{
};

class CExceptionObject : public CObject
{
public:
	CExceptionObject();
	CExceptionObject(BOOL bThrow);
	void SetToThrow(BOOL bThrow=TRUE);
	BOOL IsThrowing() const;

protected:
	BOOL m_bThrow;
};

#ifdef WIN32
#define FILE_NULL	INVALID_HANDLE_VALUE
#else
#define FILE_NULL	NULL
#endif


class CCmdTarget : public CObject
{
};

class CFile : public CExceptionObject
{
public:
	enum OpenFlags {
		// Flags corresponds with dwCreationDisposition param of CreateFile
		openCreateNew=			0x1,
		openCreateAlways=		0x2,
		openExisting=			0x3,
		openAlways=				0x4,
		openTruncate=			0x5,
		openFlags=				0xF,
		
		// Shage flags
		shareCompat=			0x0,
		shareDenyWrite=			0x10,
		shareDenyRead=			0x20,
		shareDenyNone=			0x30,
		shareFlags=				0xF0,
		
		// Flags corresponds with dwDesiredAccess param of CreateFile
		modeWrite=				0x40000000,
		modeRead=				0x80000000,
		modeReadWrite=			0x40000000|0x80000000,
		modeFlags=				0xF0000000,
		
		
		typeText=				0x4000,
		typeBinary=				0,
		typeFlags=				0xF000,
		
		otherInherit=			0x100,
		otherErrorWhenEOF=		0x200,
		otherStrNullTerminated= 0x400,
		
		otherFlags=				0xF00,

		defRead=				modeRead|shareDenyWrite|openExisting,
		defWrite=				modeWrite|shareDenyWrite|openCreateAlways
	};

	enum Attribute {
		readOnly=	0x1,
		hidden=		0x2,
		system=		0x04,
		volume=		0x08,
		directory=	0x10,
		archive=	0x20,
		normal=		0x80,
		tempotary=	0x100,
		comressed=	0x200,
		encrypted=	0x400
	};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };
	
	CFile();
	CFile(BOOL bThrowExceptions);
	CFile(HANDLE hFile,BOOL bThrowExceptions=FALSE);
	CFile(LPCSTR lpszFileName,int nOpenFlags,CFileException* e);
	CFile(LPCSTR lpszFileName,int nOpenFlags,BOOL bThrowExceptions=FALSE);
#ifdef DEF_WCHAR
	CFile(LPCWSTR lpszFileName,int nOpenFlags,CFileException* e);
	CFile(LPCWSTR lpszFileName,int nOpenFlags,BOOL bThrowExceptions=FALSE);
#endif

#ifndef WIN32
	CFile(FILE* pFile,BOOL bThrowExceptions=FALSE);
#endif
	virtual ~CFile();
	
	HANDLE m_hFile;
	BOOL m_bCloseOnDelete;
	CString m_strFileName;
	int m_nOpenFlags;

	operator HANDLE() const;
#ifndef WIN32
	operator FILE*() const;
#endif
	BOOL IsOpen() const { return m_hFile!=FILE_NULL; }

#ifdef WIN32
	virtual ULONG_PTR GetPosition(PLONG pHigh=NULL) const;
	ULONGLONG GetPosition64() const;
#else
	virtual ULONG_PTR GetPosition() const;
#endif

	BOOL GetStatus(CFileStatus& rStatus) const;
	virtual CString GetFileName() const;
	virtual CString GetFileTitle() const;
	virtual CString GetFilePath() const;
	virtual void SetFilePath(LPCTSTR lpszNewName);

	virtual BOOL Open(LPCSTR lpszFileName, int nOpenFlags,CFileException* pError = NULL);
#ifdef DEF_WCHAR
	virtual BOOL Open(LPCWSTR lpszFileName, int nOpenFlags,CFileException* pError = NULL);
#endif	
	BOOL OpenRead(LPCSTR lpszFileName,CFileException* pError = NULL) { return Open(lpszFileName,CFile::defRead,pError); }
	BOOL OpenRead(LPCWSTR lpszFileName,CFileException* pError = NULL) { return Open(lpszFileName,CFile::defRead,pError); }
	BOOL OpenWrite(LPCSTR lpszFileName,CFileException* pError = NULL) { return Open(lpszFileName,CFile::defWrite,pError); }
	BOOL OpenWrite(LPCWSTR lpszFileName,CFileException* pError = NULL) { return Open(lpszFileName,CFile::defWrite,pError); }

	static BOOL Rename(LPCSTR lpszOldName,LPCSTR lpszNewName);
	static BOOL Remove(LPCSTR lpszFileName);
	static BOOL GetStatus(LPCSTR lpszFileName,CFileStatus& rStatus);
	static BOOL SetStatus(LPCSTR lpszFileName,const CFileStatus& status);

	ULONG_PTR SeekToEnd() { return this->Seek(0,end); }
	BOOL SeekToBegin() { return this->Seek(0,begin)>0; }
#ifdef WIN32
	virtual LONG_PTR Seek(ULONG_PTR lOff, ULONG_PTR nFrom,CFileException* pError=NULL,LONG* pHighPos=NULL);

	virtual BOOL SetLength(SIZE_T dwNewLen,LONG* pHigh=NULL);
	BOOL SetLength(ULONGLONG dwNewLen);
#else
	virtual LONG_PTR Seek(ULONG_PTR lOff, ULONG_PTR nFrom,CFileException* pError=NULL);
#endif

	
#ifdef WIN32
	virtual SIZE_T GetLength(PSIZE_T pHigh=NULL) const;
	ULONGLONG GetLength64() const;
#else
	virtual SIZE_T GetLength() const;
#endif

	virtual SIZE_T Read(void* lpBuf, SIZE_T nCount,CFileException* pError=NULL);
	virtual BOOL Write(const void* lpBuf, SIZE_T nCount,CFileException* pError=NULL);
	
	// Helpers
	BOOL Read(CStringA& str,CFileException* pError=NULL);
#ifdef DEF_WCHAR
	BOOL Read(CStringW& str,CFileException* pError=NULL);
#endif
	BOOL Read(BYTE& bNum,CFileException* pError=NULL);
	BOOL Read(WORD& wNum,CFileException* pError=NULL);
	BOOL Read(DWORD& dwNum,CFileException* pError=NULL);

	BOOL Write(const CStringA& str,CFileException* pError=NULL);
#ifdef DEF_WCHAR
	BOOL Write(const CStringW& str,CFileException* pError=NULL);
#endif
	BOOL Write(BYTE bNum,CFileException* pError=NULL);
	BOOL Write(WORD wNum,CFileException* pError=NULL);
	BOOL Write(DWORD dwNum,CFileException* pError=NULL);
	BOOL Write(char ch,CFileException* pError=NULL);
	BOOL Write(LPCSTR szNullTerminatedString,CFileException* pError=NULL);
#ifdef DEF_WCHAR
	BOOL Write(LPCWSTR szNullTerminatedString,CFileException* pError=NULL);
#endif

	virtual BOOL IsEndOfFile() const;

#ifdef WIN32
	virtual void LockRange(ULONG_PTR dwPos, SIZE_T dwCount);
	virtual void UnlockRange(ULONG_PTR dwPos, SIZE_T dwCount);
#endif
	virtual BOOL Flush();
	virtual BOOL Close();

	static BOOL IsFile(LPCTSTR szFileName);
	static INT IsDirectory(LPCSTR szDirectoryName); // return: 0 not dir, 1 fixed, 2 remote
#ifdef DEF_WCHAR
	static INT IsDirectory(LPCWSTR szDirectoryName); // return: 0 not dir, 1 fixed, 2 remote
#endif
	static BOOL IsValidFileName(LPCSTR szFile,LPSTR szShortName=NULL); // Parent directory must exist
	static BOOL IsValidPath(LPCSTR szPath,BOOL bAsDirectory=FALSE);

	static BOOL IsSamePath(LPCSTR szDir1,LPCSTR szDir2);
	static BOOL IsSubDirectory(LPCSTR szSubDir,LPCSTR szPath);

	// Last '//' is not counted, if exists
	static DWORD ParseExistingPath(LPCSTR szPath);
	static BOOL CreateDirectoryRecursive(LPCSTR szPath,LPSECURITY_ATTRIBUTES plSecurityAttributes);

	
};

class CFileFind : public CObject
{
public:
#ifdef WIN32
	HANDLE m_hFind;
	WIN32_FIND_DATA m_fd;
#else
	struct ffblk m_fd;
#endif
	CString strRoot;

public:
	CFileFind();
	~CFileFind();

	void Close();
	BOOL FindFile(LPCTSTR pstrName=NULL);
	BOOL FindNextFile();
	
	void GetFileName(LPSTR szName,SIZE_T nMaxLen) const;
	void GetFileName(CString& name) const;
	void GetFilePath(LPSTR szPath,SIZE_T nMaxLen) const;
	void GetFilePath(CString& path) const;
	SIZE_T GetFileSize() const;
	
#ifdef WIN32
	SIZE_T GetFileTitle(CString title) const;
	SIZE_T GetFileTitle(LPSTR szFileTitle,SIZE_T nMaxLen) const;

#endif

#ifdef WIN32
	BOOL GetLastWriteTime(FILETIME& rTimeStamp) const;
	BOOL GetLastAccessTime(FILETIME& rTimeStamp) const;
	BOOL GetCreationTime(FILETIME& rTimeStamp) const;
	BOOL GetLastWriteTime(CTime& refTime) const;
	BOOL GetLastAccessTime(CTime& refTime) const;
	BOOL GetCreationTime(CTime& refTime) const;
#else
	USHORT GetFileTime() const;
	USHORT GetFileDate() const;
	BOOL GetFileTime(CTime& refTime) const;
#endif

	BOOL MatchesMask(DWORD dwMask) const;

	BOOL IsReadOnly() const;
	BOOL IsDirectory() const;
	BOOL IsSystem() const;
	BOOL IsHidden() const;
	BOOL IsNormal() const;
	BOOL IsArchived() const;
#ifdef WIN32
	BOOL IsTemporary() const;
	BOOL IsCompressed() const;
#endif
};

class CSearchFromFile // Pure virtual class
{
public:
	CSearchFromFile();
	virtual ~CSearchFromFile();
	
	virtual BOOL Search(LPCSTR szFile)=0; // if return value is TRUE if any found
	virtual ULONG_PTR GetFoundPosition() const=0;
	
	virtual void OpenFile(LPCSTR szFile);
	virtual void CloseFile();

protected:
#ifdef WIN32
	HANDLE hFile;
#else
	FILE* hFile;
#endif

};

class CSearchHexFromFile : public CSearchFromFile
{
public:
	CSearchHexFromFile(const BYTE* pData,DWORD dwLength);
	CSearchHexFromFile(LPCSTR szString,BOOL bMatchCase=TRUE);
	CSearchHexFromFile(LPCSTR szString,DWORD dwLength,BOOL bMatchCase=TRUE);
	CSearchHexFromFile(BYTE bNumber);
	CSearchHexFromFile(WORD wNumber);
	CSearchHexFromFile(DWORD dwNumber);
	virtual ~CSearchHexFromFile();
	virtual void CloseFile();

	
	virtual BOOL Search(LPCSTR szFile); // if return value is TRUE if any found
	virtual ULONG_PTR GetFoundPosition() const;
private:

	BYTE* pSearchValue;
	SIZE_T dwLength;


	BYTE* pBuffer;
	SIZE_T dwBufferLen;
	ULONG_PTR dwBufferPtr;
	ULONG_PTR dwFilePtr; 
	SIZE_T dwFileSize;
	BOOL bMatchCase;
};

class CSize : public tagSIZE
{
public:
	CSize();
	CSize(int initCX,int initCY);
	CSize(SIZE initSize);
	CSize(DWORD dwSize);
	
	int GetCX() const;
	void SetCX(int initCX);
	int GetCY() const;
	void SetCY(int initCY);
	void SetSize(int initCX,int initCY);

	void MakeLarger(CSize& sz);
	void MakeSmaller(CSize& sz);

};

class CPoint : public tagPOINT
{
public:
	CPoint();
	CPoint(int initX, int initY);
	CPoint(const POINT& initPt);
	CPoint(const SIZE& initSize);
	CPoint(DWORD dwPoint);

	BOOL IsInRect(const RECT& rc);

	int GetX() const;
	void SetX(int initX);
	int GetY() const;
	void SetY(int initY);
	void SetPoint(int initX,int initY);
};

class CRect : public tagRECT
{
public:
	CRect();
	CRect(int l, int t, int r, int b);
	CRect(const RECT& rc);
	CRect(const LPRECT rc);
	CRect(const POINT& pt, const SIZE& sz);
	CRect(const POINT& topLeft, const POINT& bottomRight);

	int Width() const;
	int Height() const;
	CSize Size() const;
	
	BOOL IsPtInRect(const POINT& pt);
	BOOL IsPtInRect(int x,int y);

	CPoint TopLeft();
	CPoint BottomRight();
};


#ifdef WIN32

class CGdiObject : public CObject
{
public:
	HGDIOBJ m_hObject;
	
public:
	CGdiObject();
	CGdiObject(HGDIOBJ hObject);
	virtual ~CGdiObject();

	BOOL DeleteObject();
	
	operator HGDIOBJ() const;
	HGDIOBJ GetSafeHandle() const;
	
	
	int GetObject(int nCount,LPVOID lpObject) const;
	DWORD GetObjectType() const;
	BOOL CreateStockObject(int nIndex);
	BOOL UnrealizeObject();
	BOOL operator==(const CGdiObject& obj) const;
	BOOL operator!=(const CGdiObject& obj) const;
};

class CPen : public CGdiObject
{
public:
	CPen();
	CPen(HPEN hPen);
	CPen(int nPenStyle,int nWidth,COLORREF crColor);
	CPen(int nPenStyle,int nWidth,const LOGBRUSH* pLogBrush,
		int nStyleCount=0,const DWORD* lpStyle=NULL);
	
	BOOL CreatePen(int nPenStyle,int nWidth,COLORREF crColor);
	BOOL CreatePen(int nPenStyle,int nWidth,const LOGBRUSH* pLogBrush,
		int nStyleCount=0,const DWORD* lpStyle=NULL);
	BOOL CreatePenIndirect(LPLOGPEN lpLogPen);

	operator HPEN() const;
	int GetLogPen(LOGPEN* pLogPen);
	int GetExtLogPen(EXTLOGPEN* pLogPen);
};

class CBrush : public CGdiObject
{
public:
	CBrush();
	CBrush(HBRUSH hObject);
	CBrush(COLORREF crColor);
	CBrush(int nIndex,COLORREF crColor);
	
	BOOL CreateSolidBrush(COLORREF crColor);
	BOOL CreateHatchBrush(int nIndex,COLORREF crColor);
	BOOL CreateBrushIndirect(const LOGBRUSH* lpLogBrush);
	BOOL CreatePatternBrush(HBITMAP hBitmap);
	BOOL CreateDIBPatternBrush(HGLOBAL hPackedDIB,UINT nUsage);
	BOOL CreateDIBPatternBrush(const void* lpPackedDIB,UINT nUsage);
	BOOL CreateSysColorBrush(int nIndex);

	operator HBRUSH() const;
	int GetLogBrush(LOGBRUSH* pLogBrush);
};

class CFont : public CGdiObject
{
public:
	CFont();
	CFont(HFONT hObject);
	
	BOOL CreateFontIndirect(const LOGFONTA* lpLogFont);
	BOOL CreateFontIndirect(const LOGFONTW* lpLogFont);
	
	BOOL CreateFont(int nHeight,int nWidth,int nEscapement,
		int nOrientation,int nWeight,BYTE bItalic,BYTE bUnderline,
		BYTE cStrikeOut,BYTE nCharSet,BYTE nOutPrecision,
		BYTE nClipPrecision,BYTE nQuality,BYTE nPitchAndFamily,
		LPCTSTR lpszFacename);
	BOOL CreatePointFont(int nPointSize,LPCTSTR lpszFaceName,HDC hDC=NULL);
	BOOL CreatePointFontIndirect(const LOGFONT* lpLogFont,HDC hDC=NULL);

	operator HFONT() const;
	int GetLogFont(LOGFONT* pLogFont);
};

class CBitmap : public CGdiObject
{
public:
	CBitmap();
	CBitmap(HBITMAP hObject);

	BOOL LoadBitmap(LPCTSTR lpszResourceName);
	BOOL LoadBitmap(UINT nIDResource);
	BOOL LoadOEMBitmap(UINT nIDBitmap);

	BOOL CreateBitmap(int nWidth,int nHeight,UINT nPlanes,UINT nBitcount,const void* lpBits);
	BOOL CreateBitmapIndirect(LPBITMAP lpBitmap);
	BOOL CreateCompatibleBitmap(HDC hDC,int nWidth,int nHeight);

	operator HBITMAP() const;
	
	DWORD SetBitmapBits(DWORD dwCount, const void* lpBits);
	DWORD GetBitmapBits(DWORD dwCount, LPVOID lpBits) const;
	CSize SetBitmapDimension(int nWidth, int nHeight);
	CSize GetBitmapDimension() const;
	BOOL GetBitmapDimension(CSize& dim) const;
	CSize GetBitmapSize(HDC hDC=NULL) const;
	BOOL GetBitmapSize(CSize& sz,HDC hDC=NULL) const;

};

class CPalette : public CGdiObject
{
public:
	CPalette();
	CPalette(HPALETTE hObject);
	
	BOOL CreatePalette(LPLOGPALETTE lpLogPalette);
	BOOL CreateHalftonePalette(HDC hDC);

	operator HPALETTE() const;
	int GetEntryCount();
	UINT GetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
			LPPALETTEENTRY lpPaletteColors) const;
	UINT SetPaletteEntries(UINT nStartIndex, UINT nNumEntries,
			LPPALETTEENTRY lpPaletteColors);

	void AnimatePalette(UINT nStartIndex, UINT nNumEntries,
			LPPALETTEENTRY lpPaletteColors);
	UINT GetNearestPaletteIndex(COLORREF crColor) const;
	BOOL ResizePalette(UINT nNumEntries);
};

class CRgn : public CGdiObject
{
public:
	CRgn();
	CRgn(HRGN hObject);

	operator HRGN() const;

	BOOL CreateRectRgn(int x1, int y1, int x2, int y2);
	BOOL CreateRectRgnIndirect(LPCRECT lpRect);
	BOOL CreateEllipticRgn(int x1, int y1, int x2, int y2);
	BOOL CreateEllipticRgnIndirect(LPCRECT lpRect);
	BOOL CreatePolygonRgn(LPPOINT lpPoints, int nCount, int nMode);
	BOOL CreatePolyPolygonRgn(LPPOINT lpPoints, LPINT lpPolyCounts,
			int nCount, int nPolyFillMode);
	BOOL CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3);
	BOOL CreateFromPath(HDC hDC);
	BOOL CreateFromData(const XFORM* lpXForm, int nCount,
		const RGNDATA* pRgnData);

	void SetRectRgn(int x1, int y1, int x2, int y2);
	void SetRectRgn(LPCRECT lpRect);
	int CombineRgn(HRGN hRgn1,HRGN hRgn2,int nCombineMode);
	BOOL EqualRgn(HRGN hRgn) const;
	int OffsetRgn(int x, int y);
	int OffsetRgn(POINT point);
	int GetRgnBox(LPRECT lpRect) const;
	BOOL PtInRegion(int x, int y) const;
	BOOL PtInRegion(POINT point) const;
	BOOL RectInRegion(LPCRECT lpRect) const;
	int GetRegionData(LPRGNDATA lpRgnData, int nCount) const;
};

class CWaitCursor
{
public:
	CWaitCursor();
	~CWaitCursor();
	void Restore();
protected:
	HCURSOR m_hOldCursor;
};

class CRegKey
{
public:
	enum Status {
		createNew=0x0,
		openExist=0x1,
		samCreateLink=0x2,
		samCreateSubkey=0x4,
		samEnumerateSubkeys=0x8,
		samExecute=0x10,
		samNotify=0x20,
		samQueryValue=0x40,
		samSetValue=0x80,
		
		samRead=samQueryValue|samNotify|samEnumerateSubkeys,
		samWrite=samCreateSubkey|samSetValue,
		samAll=0xFE,
		
		optionVolatile=0x100,
		optionNoVolatile=0x200,

		defRead=openExist|samRead,
		defWrite=createNew|samAll
	};

	CRegKey();
	CRegKey(HKEY hKey);
	CRegKey(HKEY hKey,LPCSTR lpszSubKey,DWORD fStatus=CRegKey::defWrite,LPSECURITY_ATTRIBUTES lpSecurityAttributes=NULL);
	virtual ~CRegKey();

	LONG OpenKey(HKEY hKey,LPCSTR lpszSubKey,DWORD fStatus=CRegKey::createNew|CRegKey::samAll,LPSECURITY_ATTRIBUTES lpSecurityAttributes=NULL);
	BOOL OpenRead(HKEY hKey,LPCSTR lpszSubKey) { return OpenKey(hKey,lpszSubKey,CRegKey::defRead,NULL)==ERROR_SUCCESS; }
	BOOL OpenWrite(HKEY hKey,LPCSTR lpszSubKey) { return OpenKey(hKey,lpszSubKey,CRegKey::defWrite,NULL)==ERROR_SUCCESS; }
	
	
	LONG CloseKey();
	LONG UnLoadKey(LPCTSTR lpszSubKey);
	BYTE FlushKey();
	
	HKEY m_hKey;
	HKEY GetHandleKey() const;
	operator HKEY() const;

	DWORD QueryValue(LPCSTR lpszValueName,LPSTR lpbData,SIZE_T cbData,LPDWORD lpdwType=NULL) const;	
	BOOL QueryValue(LPCSTR lpszValueName,CString& strData) const;	
	BOOL QueryValue(LPCSTR lpszValueName,DWORD& dwData) const;	
	
	SIZE_T QueryValueLength(LPCSTR lpszValueName=NULL) const;
	SIZE_T QueryValueLength(LPCSTR lpszValueName,BOOL& bIsOk) const;
	
	LONG SetValue(LPCSTR lpValueName,LPCSTR lpData,SIZE_T cbData,DWORD dwType=REG_BINARY);
	BOOL SetValue(LPCSTR lpValueName,CString& strData);
	LONG SetValue(LPCSTR lpValueName,LPCSTR strData);
	BOOL SetValue(LPCSTR lpValueName,DWORD dwData);
	
	

	DWORD EnumKey(DWORD iSubkey,LPSTR lpszName,SIZE_T cchName,LPSTR lpszClass=NULL,PSIZE_T lpcchClass=NULL,PFILETIME lpftLastWrite=NULL) const;
	BOOL EnumKey(DWORD iSubkey,CString& strName,LPSTR lpszClass=NULL,PSIZE_T lpcchClass=NULL,PFILETIME lpftLastWrite=NULL) const;
	DWORD EnumValue(DWORD iValue,LPSTR lpszValue,SIZE_T cchValue,LPDWORD lpdwType=NULL,LPBYTE lpbData=NULL,PSIZE_T lpcbData=0) const;
	BOOL EnumValue(DWORD iValue,CString& strName,LPDWORD lpdwType=NULL,LPBYTE lpbData=NULL,PSIZE_T lpcbData=0) const;
	
	

	LONG DeleteKey(LPCSTR lpszSubKey);
	LONG DeleteValue(LPCSTR lpszValue); // does not remove subkeys on WinNT
	static BOOL DeleteKey(HKEY hKey,LPCSTR szKey); // Removes also subkeys
	
	LONG GetKeySecurity(SECURITY_INFORMATION SecInf,PSECURITY_DESCRIPTOR pSecDesc,LPDWORD lpcbSecDesc) const;
	LONG SetKeySecurity(SECURITY_INFORMATION si,PSECURITY_DESCRIPTOR psd);

	LONG LoadKey(LPCSTR lpszSubKey,LPCSTR lpszFile);
	LONG SaveKey(LPCSTR lpszFile,LPSECURITY_ATTRIBUTES lpsa);
	LONG ReplaceKey(LPCSTR lpSubKey,LPCSTR lpNewFile,LPCSTR lpOldFile);
	LONG RestoreKey(LPCSTR lpszFile,DWORD fdw);

	
	static LONG CopyKey(HKEY hSource,HKEY hDestination);
	static LONG RenameSubKey(HKEY hKey,LPCSTR szOldName,LPCSTR szNewName);
	
	LONG CopyKey(HKEY hDestination);
	LONG RenameSubKey(LPCSTR szOldName,LPCSTR szNewName);

	LONG NotifyChangeKeyValue(BOOL fWatchSubTree,DWORD fdwNotifyFilter,HANDLE hEvent,BOOL fAsync);
	LONG QueryInfoKey(LPTSTR lpszClass,PSIZE_T lpcchClass,LPDWORD lpcSubKeys,PSIZE_T lpcchMaxSubkey,
		PSIZE_T lpcchMaxClass,LPDWORD lpcValues,PSIZE_T lpcchMaxValueName,PSIZE_T lpcbMaxValueData,
		LPDWORD lpcbSecurityDescriptor,PFILETIME lpftLastWriteTime) const;

#ifdef DEF_WCHAR
	CRegKey(HKEY hKey,LPCWSTR lpszSubKey,DWORD fStatus=CRegKey::defWrite,LPSECURITY_ATTRIBUTES lpSecurityAttributes=NULL);

	LONG OpenKey(HKEY hKey,LPCWSTR lpszSubKey,DWORD fStatus=CRegKey::createNew|CRegKey::samAll,LPSECURITY_ATTRIBUTES lpSecurityAttributes=NULL);
	BOOL OpenRead(HKEY hKey,LPCWSTR lpszSubKey) { return OpenKey(hKey,lpszSubKey,CRegKey::defRead,NULL)==ERROR_SUCCESS; }
	BOOL OpenWrite(HKEY hKey,LPCWSTR lpszSubKey) { return OpenKey(hKey,lpszSubKey,CRegKey::defWrite,NULL)==ERROR_SUCCESS; }
	
	DWORD QueryValue(LPCWSTR lpszValueName,LPWSTR lpbData,SIZE_T cbData,LPDWORD lpdwType=NULL) const;	
	BOOL QueryValue(LPCWSTR lpszValueName,CStringW& strData) const;	
	BOOL QueryValue(LPCWSTR lpszValueName,DWORD& dwData) const;	
	
	SIZE_T QueryValueLength(LPCWSTR lpszValueName) const;
	SIZE_T QueryValueLength(LPCWSTR lpszValueName,BOOL& bIsOk) const;

	LONG SetValue(LPCWSTR lpValueName,LPCSTR lpData,SIZE_T cbData,DWORD dwType=REG_BINARY);
	BOOL SetValue(LPCWSTR lpValueName,CStringW& strData);
	LONG SetValue(LPCWSTR lpValueName,LPCWSTR strData);
	BOOL SetValue(LPCWSTR lpValueName,DWORD dwData);

	DWORD EnumKey(DWORD iSubkey,LPWSTR lpszName,SIZE_T cchName,LPWSTR lpszClass=NULL,PSIZE_T lpcchClass=NULL,PFILETIME lpftLastWrite=NULL) const;
	BOOL EnumKey(DWORD iSubkey,CStringW& strName,LPWSTR lpszClass=NULL,LPDWORD lpcchClass=NULL,PFILETIME lpftLastWrite=NULL) const;
	DWORD EnumValue(DWORD iValue,LPWSTR lpszValue,SIZE_T cchValue,LPDWORD lpdwType=NULL,LPBYTE lpbData=NULL,PSIZE_T lpcbData=0) const;
	BOOL EnumValue(DWORD iValue,CStringW& strName,LPDWORD lpdwType=NULL,LPBYTE lpbData=NULL,PSIZE_T lpcbData=0) const;

	LONG DeleteKey(LPCWSTR lpszSubKey);
	LONG DeleteValue(LPCWSTR lpszValue); // does not remove subkeys on WinNT
	static BOOL DeleteKey(HKEY hKey,LPCWSTR szKey); // Removes also subkeys

	LONG LoadKey(LPCWSTR lpszSubKey,LPCWSTR lpszFile);
	LONG SaveKey(LPCWSTR lpszFile,LPSECURITY_ATTRIBUTES lpsa);
	LONG ReplaceKey(LPCWSTR lpSubKey,LPCWSTR lpNewFile,LPCWSTR lpOldFile);
	LONG RestoreKey(LPCWSTR lpszFile,DWORD fdw);

	static LONG RenameSubKey(HKEY hKey,LPCWSTR szOldName,LPCWSTR szNewName);
	
	LONG RenameSubKey(LPCWSTR szOldName,LPCWSTR szNewName);

#endif

};

#endif

/////////////////////////////////////////////////
// Inline functions
/////////////////////////////////////////////////

#include "General.inl"
#include "GdiObject.inl"
#include "File.inl"

///////////////////////////////////////
// CExceptionObject

inline CExceptionObject::CExceptionObject()
:	m_bThrow(FALSE)
{
}

inline CExceptionObject::CExceptionObject(BOOL bThrow)
:	m_bThrow(bThrow)
{
}

inline void CExceptionObject::SetToThrow(BOOL bThrow)
{
	m_bThrow=bThrow;
}

inline BOOL CExceptionObject::IsThrowing() const
{
	return m_bThrow;
}

#endif
