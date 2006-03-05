////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////
// All definitions for manipulating strings
////////////////////////////////////////////////////////////////////

#ifndef HFCSTR_H
#define HFCSTR_H

////////////////////////////////////////
// Definitions

#ifdef DEF_WCHAR
inline BOOL IsFullUnicodeSupport()
{
#ifdef MICROSOFT_LAYER_FOR_UNICODE 
	return TRUE;
#else
	extern BOOL bIsFullUnicodeSupport;
	return bIsFullUnicodeSupport;
#endif
}
#endif

inline SIZE_T istrlen(const char* str)
{
	SIZE_T len;
	for (len=0;(str)[len]!='\0';len++);
	return len;
}


#ifdef DEF_WCHAR
inline SIZE_T istrlenw(const WCHAR* str)
{
	SIZE_T len;
	for (len=0;(str)[len]!=L'\0';len++);
	return len;
}
#endif

inline SIZE_T parseto(LPCSTR str,CHAR ch)
{
	SIZE_T len;
	for (len=0;(str)[len]!=(ch);len++);
	return len;
}

#ifdef DEF_WCHAR
inline SIZE_T parseto(LPCWSTR str,WCHAR ch)
{
	SIZE_T len;
	for (len=0;(str)[len]!=(ch);len++);
	return len;
}
#endif

inline SIZE_T parseto2(LPCSTR str,CHAR ch1,CHAR ch2)
{
	SIZE_T len;
	for (len=0;str[len]!=ch1 && str[len]!=ch2;len++);
	return len;
}

inline SIZE_T parseto3(LPCSTR str,CHAR ch1,CHAR ch2,CHAR ch3)
{
	SIZE_T len;
	for (len=0;str[len]!=ch1 && str[len]!=ch2 && str[len]!=ch3;len++);
	return len;
}

inline SIZE_T parseto4(LPCSTR str,CHAR ch1,CHAR ch2,CHAR ch3,CHAR ch4)
{
	SIZE_T len;
	for (len=0;str[len]!=ch1 && str[len]!=ch2 && str[len]!=ch3 && str[len]!=ch4;len++);
	return len;
}


// Extended sprintf style handlers
int vsprintfex( char *buffer, SIZE_T buffersize, const char *format, va_list argptr );
inline int sprintfex( char *buffer, SIZE_T buffersize, const char *format,...)
{
	va_list argList;
	va_start(argList,format);
	int nRet=vsprintfex(buffer,buffersize,format,argList);
	va_end(argList);
	return nRet;
}

#ifdef DEF_WCHAR
int vswprintfex( wchar_t *buffer, SIZE_T buffersize, const wchar_t *format, va_list argptr );
inline int swprintfex( wchar_t *buffer, SIZE_T buffersize, const wchar_t *format,...)
{
	va_list argList;
	va_start(argList,format);
	int nRet=vswprintfex(buffer,buffersize,format,argList);
	va_end(argList);
	return nRet;
}
#endif

#ifdef DEF_WCHAR
#define MemCopyW(dst,src,len) \
	MemCopy((dst),(src),(len)*sizeof(WCHAR))
#define iMemCopyW(dst,src,len) \
	iMemCopy((dst),(src),(len)*sizeof(WCHAR))
#define MemCopyWtoA(dst,src,len) \
	WideCharToMultiByte(CP_ACP,0,(LPCWSTR)(src),(int)(len),(LPSTR)(dst),(int)(len),NULL,NULL)
#define MemCopyAtoW(dst,src,len) \
	MultiByteToWideChar(CP_ACP,0,(LPCSTR)(src),(int)(len),(LPWSTR)(dst),(int)(len))
#endif

////////////////////////////////////////
// String functions

BOOL ContainString(LPCSTR,LPCSTR);
LONG_PTR FirstCharIndex(LPCSTR,const CHAR);
LONG_PTR LastCharIndex(LPCSTR,const CHAR);
LONG_PTR NextCharIndex(LPCSTR,const CHAR,LONG_PTR);

#ifdef DEF_WCHAR
BOOL ContainString(LPCWSTR,LPCWSTR);
LONG_PTR FirstCharIndex(LPCWSTR,const WCHAR);
LONG_PTR LastCharIndex(LPCWSTR,const WCHAR);
LONG_PTR NextCharIndex(LPCWSTR,const WCHAR,LONG_PTR);
#endif

#ifdef WIN32
int strcasecmp(LPCTSTR,LPCTSTR);
#endif

int strcasencmp(LPCTSTR s1,LPCTSTR s2,DWORD n);
int _readnum(int base,LPCSTR& str,SIZE_T length=SIZE_T(-1));

BYTE* dataparser(LPCSTR pString,SIZE_T dwStrLen,MALLOC_FUNC pMalloc,DWORD* pdwDataLength=NULL);
BYTE* dataparser2(LPCSTR pStr,SIZE_T* pdwDataLength);
BOOL IsCharNumeric(char cChar,BYTE bBase);


//////////////////////////////////////////////////
// Class CString

#define CStringA CString
class CString
{
protected:
	LPSTR m_pData;
	SIZE_T m_nDataLen;
	SIZE_T m_nAllocLen;
	BYTE m_nBase;
public:
//Constructors
	CString();
	CString(const CString& str);
	CString(CHAR ch,SIZE_T nRepeat=1);
	CString(LPCSTR lpsz);
	CString(LPCSTR lpsz,DWORD nLength);
	CString(const unsigned char * lpsz);
	~CString();

public:
//Attributes & Operations
	SIZE_T GetLength() const { return m_nDataLen; }
	BOOL IsEmpty() const { return (m_pData==NULL || !m_nDataLen); }
	void Empty();

	CString& SetBase(BYTE nBase);
	BYTE GetBase() { return m_nBase; }

	CHAR GetAt(DWORD nIndex) const;
	CHAR operator[](LONG_PTR nIndex) const;
	CHAR& operator[](LONG_PTR nIndex);
	CHAR operator[](ULONG_PTR nIndex) const;
	CHAR& operator[](ULONG_PTR nIndex);
	CHAR operator[](int nIndex) const;
	CHAR& operator[](int nIndex);
	CHAR operator[](UINT nIndex) const;
	CHAR& operator[](UINT nIndex);
	void SetAt(ULONG_PTR nIndex,CHAR ch);
	
	inline operator LPCSTR() const	{ if (m_pData==NULL)	return szEmpty;	return m_pData;}
	LPSTR GetPCHData() const { return m_pData; } // Use with caution, may be null
	

	CString& Copy(LPCSTR src);
	CString& Copy(LPCSTR src,SIZE_T iLength);
	CString& Copy(const BYTE* src);
	CString& Copy(const BYTE* src,SIZE_T iLength);

	
	const CString& operator=(const CString& str);
	const CString& operator=(CHAR ch);
	const CString& operator=(LPCSTR str);
	const CString& operator=(unsigned char * str);
	const CString& operator=(DWORD iNum);
	const CString& operator=(int iNum);

	const CString& operator+=(const CString& str);
	const CString& operator+=(CHAR ch);
	const CString& operator+=(LPCSTR str);
	const CString& operator+=(DWORD iNum);
	const CString& operator+=(int iNum);

	CString operator+(const CString& str);
	CString operator+(const LPCSTR str);
	CString operator+(const CHAR ch);

	CString& operator<<(const CString& str);
	CString& operator<<(CHAR ch);
	CString& operator<<(LPCSTR str);
	CString& operator<<(DWORD iNum);
	CString& operator<<(int iNum);

	void Append(LPCSTR str,SIZE_T iLength=SIZE_T(-1));
    void Append(const CString& str);
    void Append(CHAR ch);

	int Compare(LPCSTR lpsz) const;
	int CompareNoCase(LPCSTR lpsz) const;

	BOOL operator==(const CString& str);
	BOOL operator==(LPCSTR str);
	BOOL ContainString(LPCSTR str,ULONG_PTR start=0);

	CString Mid(ULONG_PTR nFirst,ULONG_PTR nCount) const;
	CString Mid(ULONG_PTR nFirst) const;
	CString Left(SIZE_T nCount) const;
	CString Right(SIZE_T nCount) const;
	CHAR LastChar() const;
	void DelLastChar();

	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	LONG_PTR Find(CHAR ch) const;
	LONG_PTR FindOneOf(LPCSTR lpszCharSet) const;
	LONG_PTR FindFirst(CHAR ch) const;
	LONG_PTR FindLast(CHAR ch) const;
	LONG_PTR FindNext(CHAR ch,LONG_PTR idx) const;
	LONG_PTR Find(LPCSTR lpszSub) const;

	LPSTR GetBuffer(SIZE_T nMinBufLength=SIZE_T(-1),BOOL bStoreData=FALSE);
	SIZE_T GetAllocLength() const { return m_nAllocLen; }
	void FreeExtra(SIZE_T nNewLength=SIZE_T(-1)); // Call after GetBuffer()
	void Compact(); // Makes memory usage as small as possible
					// don't call after GetBuffer()
	LPSTR GiveBuffer();
	
	
		
	BOOL InsChar(ULONG_PTR idx,CHAR ch);
	BOOL Insert(ULONG_PTR idx,CHAR ch) { return InsChar(idx,ch); }
	BOOL DelChar(ULONG_PTR idx);
	BOOL Delete(ULONG_PTR idx,SIZE_T nCount=1);
	void ReplaceChars(char from,char to);

	void Trim(); // Deletes spaces from begin and end

	void Swap(CString& str);

	void Format(LPCSTR lpszFormat,...);
	void FormatEx(LPCSTR lpszFormat,...);

	void FormatV(LPCSTR lpszFormat,va_list);
	
#ifdef DEF_RESOURCES
	CString(int nID);
	CString(int nID,TypeOfResourceHandle bType);
	CString(UINT nID);
	CString(UINT nID,TypeOfResourceHandle bType);
	
	CString(int nID,BOOL bLoadAsUnicodeIfPossible);
	CString(UINT nID,BOOL bLoadAsUnicodeIfPossible);
	
	void Format(UINT nFormatID,...);
	void FormatEx(UINT nFormatID,...);
	void FormatC(UINT nFormatID,...);
	void FormatExC(UINT nFormatID,...);
	
	BOOL LoadString(UINT nID);
	BOOL LoadString(UINT nID,TypeOfResourceHandle bType);
	BOOL AddString(UINT nID);
	BOOL AddString(UINT nID,TypeOfResourceHandle bType);
	
#endif
#ifdef WIN32
	void AnsiToOem();
	void OemToAnsi();
#endif	

#ifdef DEF_WCHAR	
	// Widechar support

	CString(const CStringW& str);
	CString(LPCWSTR lpsz);
	CString(WCHAR ch,SIZE_T nRepeat=1);
	
	CString& Copy(LPCWSTR src);
	CString& Copy(LPCWSTR src,SIZE_T iLength);

	const CString& operator=(const CStringW& str);
	const CString& operator=(WCHAR ch);
	const CString& operator=(LPCWSTR str);
	const CString& operator+=(const CStringW& str);
	const CString& operator+=(WCHAR ch);
	const CString& operator+=(LPCWSTR str);

	void Append(LPCWSTR str,SIZE_T iLength=SIZE_T(-1));
	void Append(const CStringW& str);
	void Append(WCHAR ch);

	CString operator+(const CStringW& str);
	CString operator+(const LPCWSTR str);
	CString operator+(const WCHAR ch);

	CString& operator<<(const CStringW& str);
	CString& operator<<(WCHAR ch);
	CString& operator<<(LPCWSTR str);

	int Compare(LPCWSTR lpsz) const;
	int CompareNoCase(LPCWSTR lpsz) const;

	BOOL operator==(const CStringW& str);
	BOOL operator==(LPCWSTR str);

	friend class CStringW;
#endif

#ifdef _DEBUG_LOGGING
	void DebugDumpInfo();
#else
	void DebugDumpInfo() {};
#endif
};


//////////////////////////////////////////////////
// Class CStringW

#ifdef DEF_WCHAR	

class CStringW
{
protected:
	LPWSTR m_pData;
	SIZE_T m_nDataLen;
	SIZE_T m_nAllocLen;
	BYTE m_nBase;
public:
//Constructors
	CStringW();
	CStringW(const CStringW& str);
	CStringW(WCHAR ch,SIZE_T nRepeat=1);
	CStringW(LPCWSTR lpsz);
	CStringW(LPCWSTR lpsz,DWORD nLength);
	CStringW(const short * lpsz);
	
	CStringW(const CString& str);
	CStringW(CHAR ch,SIZE_T nRepeat=1);
	CStringW(LPCSTR lpsz);
	
	~CStringW();

public:
//Attributes & Operations
	SIZE_T GetLength() const { return m_nDataLen; }
	BOOL IsEmpty() const { return (m_pData==NULL || !m_nDataLen); }
	void Empty();

	CStringW& SetBase(BYTE nBase);
	BYTE GetBase() { return m_nBase; }

	WCHAR GetAt(DWORD nIndex) const;
	WCHAR operator[](LONG_PTR nIndex) const;
	WCHAR& operator[](LONG_PTR nIndex);
	WCHAR operator[](ULONG_PTR nIndex) const;
	WCHAR& operator[](ULONG_PTR nIndex);
	WCHAR operator[](int nIndex) const;
	WCHAR& operator[](int nIndex);
	WCHAR operator[](UINT nIndex) const;
	WCHAR& operator[](UINT nIndex);
	void SetAt(ULONG_PTR nIndex,WCHAR ch);
	operator LPCWSTR() const;
	LPWSTR GetPCHData() const { return m_pData; } // Use with caution, may be null
	
	CStringW& Copy(LPCSTR src);
	CStringW& Copy(LPCSTR src,SIZE_T iLength);
	CStringW& Copy(const BYTE* src);
	CStringW& Copy(const BYTE* src,SIZE_T iLength);
	CStringW& Copy(LPCWSTR src);
	CStringW& Copy(LPCWSTR src,SIZE_T iLength);
	
	const CStringW& operator=(const CStringW& str);
	const CStringW& operator=(WCHAR ch);
	const CStringW& operator=(LPCWSTR str);
	const CStringW& operator=(unsigned short * str);
	const CStringW& operator=(DWORD iNum);
	const CStringW& operator=(int iNum);
	const CStringW& operator=(const CString& str);
	const CStringW& operator=(CHAR ch);
	const CStringW& operator=(LPCSTR str);
	
	const CStringW& operator+=(const CStringW& str);
	const CStringW& operator+=(WCHAR ch);
	const CStringW& operator+=(LPCWSTR str);
	const CStringW& operator+=(DWORD iNum);
	const CStringW& operator+=(int iNum);
	const CStringW& operator+=(const CString& str);
	const CStringW& operator+=(CHAR ch);
	const CStringW& operator+=(LPCSTR str);
	
	void Append(LPCSTR str,SIZE_T iLength=SIZE_T(-1));
    void Append(LPCWSTR str,SIZE_T iLength=SIZE_T(-1));
    void Append(const CString& str);
    void Append(const CStringW& str);
    void Append(CHAR ch);
	void Append(WCHAR ch);

	CStringW operator+(const CStringW& str);
	CStringW operator+(const LPCWSTR str);
	CStringW operator+(const WCHAR ch);
	CStringW operator+(const CString& str);
	CStringW operator+(const LPCSTR str);
	CStringW operator+(const CHAR ch);
	
	CStringW& operator<<(const CStringW& str);
	CStringW& operator<<(WCHAR ch);
	CStringW& operator<<(LPCWSTR str);
	CStringW& operator<<(DWORD iNum);
	CStringW& operator<<(int iNum);
	CStringW& operator<<(const CString& str);
	CStringW& operator<<(CHAR ch);
	CStringW& operator<<(LPCSTR str);

	int Compare(LPCWSTR lpsz) const;
	int CompareNoCase(LPCWSTR lpsz) const;
	int Compare(LPCSTR lpsz) const;
	int CompareNoCase(LPCSTR lpsz) const;
	
	BOOL operator==(const CStringW& str);
	BOOL operator==(LPCWSTR str);
	BOOL operator==(const CString& str);
	BOOL operator==(LPCSTR str);
	BOOL ContainString(LPCWSTR str,ULONG_PTR start=0);

	CStringW Mid(ULONG_PTR nFirst,SIZE_T nCount) const;
	CStringW Mid(ULONG_PTR nFirst) const;
	CStringW Left(SIZE_T nCount) const;
	CStringW Right(SIZE_T nCount) const;
	WCHAR LastChar() const;
	void DelLastChar();

	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	LONG_PTR Find(WCHAR ch) const;
	LONG_PTR FindOneOf(LPCWSTR lpszCharSet) const;
	LONG_PTR FindFirst(WCHAR ch) const;
	LONG_PTR FindLast(WCHAR ch) const;
	LONG_PTR FindNext(WCHAR ch,LONG_PTR idx) const;
	LONG_PTR Find(LPCWSTR lpszSub) const;

	LPWSTR GetBuffer(SIZE_T nMinBufLength=SIZE_T(-1),BOOL bStoreData=FALSE);
	void FreeExtra(SIZE_T nNewLength=SIZE_T(-1));
	SIZE_T GetAllocLength() const { return m_nAllocLen; }
	void Compact();
	LPWSTR GiveBuffer(); 

	BOOL InsChar(ULONG_PTR idx,WCHAR ch);
	BOOL Insert(ULONG_PTR idx,WCHAR ch) { return InsChar(idx,ch); }
	BOOL DelChar(ULONG_PTR idx);
	BOOL Delete(ULONG_PTR idx,SIZE_T nCount=1);
	void ReplaceChars(char from,char to);
	void Trim(); // Deletes spaces from begin and end

	void Swap(CStringW& str);

	void Format(LPCWSTR lpszFormat,...);
	void FormatV(LPCWSTR lpszFormat,va_list);
	void FormatEx(LPCWSTR lpszFormat,...);

#ifdef DEF_RESOURCES
	CStringW(int nID);
	CStringW(int nID,TypeOfResourceHandle bType);
	CStringW(UINT nID);
	CStringW(UINT nID,TypeOfResourceHandle bType);

	void Format(UINT nFormatID,...);
	void FormatC(UINT nFormatID,...);
	void FormatEx(UINT nFormatID,...);
	void FormatExC(UINT nFormatID,...);

	BOOL LoadString(UINT nID);
	BOOL LoadString(UINT nID,TypeOfResourceHandle bType);
	BOOL AddString(UINT nID);
	BOOL AddString(UINT nID,TypeOfResourceHandle bType);
#endif
	friend class CString;

};

#endif

#include "Strings.inl"

#endif
