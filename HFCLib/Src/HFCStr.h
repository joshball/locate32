////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
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
// len is parameter! Works with WCHAR and BYTE too!
#define dstrlen(str,len)			for (len=0;(str)[len]!='\0';len++)
#define dwstrlen(str,len)			dstrlen(str,len)
#define dstrlento(str,len,ch)		for (len=0;(str)[len]!='\0' && (str)[len]!=(ch);len++)

inline size_t istrlen(const char* str)
{
	size_t len;
	for (len=0;(str)[len]!='\0';len++);
	return len;
}


#ifdef DEF_WCHAR
inline size_t istrlenw(const WCHAR* str)
{
	size_t len;
	for (len=0;(str)[len]!=L'\0';len++);
	return len;
}
#endif

#define dreplacech(str,from,to) \
	{ for (register UINT i=0;(str)[i]!='\0';i++) if ((str)[i]==(from)) (str)[i]=(to); }
#define dwreplacech(str,from,to)	dreplacech(str,from,to)

#define dparseto(str,len,ch)				for (len=0;(str)[len]!=(ch);len++)
#define dwparseto(str,len,ch)				dparseto(str,len,ch)
#define dparseto2(str,len,ch1,ch2)			for (len=0;(str)[len]!=(ch1) && (str)[len]!=(ch2);len++)
#define dparseto3(str,len,ch1,ch2,ch3)		for (len=0;(str)[len]!=(ch1) && (str)[len]!=(ch2) && (str)[len]!=(ch3);len++)
#define dparseto4(str,len,ch1,ch2,ch3,ch4)	for (len=0;(str)[len]!=(ch1) && (str)[len]!=(ch2) && (str)[len]!=(ch3) && (str)[len]!=(ch4);len++)

// Extended sprintf style handlers
int vsprintfex( char *buffer, size_t buffersize, const char *format, va_list argptr );
inline int sprintfex( char *buffer, size_t buffersize, const char *format,...)
{
	va_list argList;
	va_start(argList,format);
	int nRet=vsprintfex(buffer,buffersize,format,argList);
	va_end(argList);
	return nRet;
}

#ifdef DEF_WCHAR
int vswprintfex( wchar_t *buffer, size_t buffersize, const wchar_t *format, va_list argptr );
inline int swprintfex( wchar_t *buffer, size_t buffersize, const wchar_t *format,...)
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

BYTE ContainString(LPCSTR,LPCSTR);
int FirstCharIndex(LPCSTR,const CHAR);
int LastCharIndex(LPCSTR,const CHAR);
int NextCharIndex(LPCSTR,const CHAR,int);

#ifdef DEF_WCHAR
BYTE ContainString(LPCWSTR,LPCWSTR);
int FirstCharIndex(LPCWSTR,const WCHAR);
int LastCharIndex(LPCWSTR,const WCHAR);
int NextCharIndex(LPCWSTR,const WCHAR,int);
#endif

#ifdef WIN32
int strcasecmp(LPCTSTR,LPCTSTR);
#endif

int strcasencmp(LPCTSTR s1,LPCTSTR s2,DWORD n);
int _readnum(int base,LPCSTR& str,DWORD length=DWORD(-1));

BYTE* dataparser(LPCSTR pString,DWORD dwStrLen,MALLOC_FUNC pMalloc,DWORD* pdwDataLength=NULL);
BYTE* dataparser2(LPCSTR pStr,DWORD* pdwDataLength);
BOOL IsCharNumeric(char cChar,BYTE bBase);


//////////////////////////////////////////////////
// Class CString

#define CStringA CString
class CString
{
protected:
	LPSTR m_pData;
	DWORD m_nDataLen;
	DWORD m_nAllocLen;
	BYTE m_nBase;
public:
//Constructors
	CString();
	CString(const CString& str);
	CString(CHAR ch,DWORD nRepeat=1);
	CString(LPCSTR lpsz);
	CString(LPCSTR lpsz,DWORD nLength);
	CString(const unsigned char * lpsz);
	~CString();

public:
//Attributes & Operations
	DWORD GetLength() const { return m_nDataLen; }
	BOOL IsEmpty() const { return (m_pData==NULL || !m_nDataLen); }
	void Empty();

	CString& SetBase(BYTE nBase);
	BYTE GetBase() { return m_nBase; }

	CHAR GetAt(DWORD nIndex) const;
	CHAR operator[](DWORD nIndex) const;
	CHAR& operator[](DWORD nIndex);
	CHAR operator[](int nIndex) const;
	CHAR& operator[](int nIndex);
	CHAR operator[](UINT nIndex) const;
	CHAR& operator[](UINT nIndex);
	void SetAt(DWORD nIndex,CHAR ch);
	
	inline operator LPCSTR() const	{ if (m_pData==NULL)	return szEmpty;	return m_pData;}
	LPSTR GetPCHData() const { return m_pData; } // Use with caution, may be null
	operator FREEDATA(); // use delete[] (LPSTR) pObject to release memory
	void TakeBack(FREEDATA pData);


	CString& Copy(LPCSTR src);
	CString& Copy(LPCSTR src,int iLength);
	CString& Copy(const BYTE* src);
	CString& Copy(const BYTE* src,int iLength);

	
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

	void Append(LPCSTR str,int iLength=-1);
    void Append(const CString& str);
    void Append(CHAR ch);

	int Compare(LPCSTR lpsz) const;
	int CompareNoCase(LPCSTR lpsz) const;

	BOOL operator==(const CString& str);
	BOOL operator==(LPCSTR str);
	BYTE ContainString(LPCSTR str,DWORD start=0);

	CString Mid(DWORD nFirst,DWORD nCount) const;
	CString Mid(DWORD nFirst) const;
	CString Left(DWORD nCount) const;
	CString Right(DWORD nCount) const;
	CHAR LastChar() const;
	void DelLastChar();

	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	int Find(CHAR ch) const;
	int FindOneOf(LPCSTR lpszCharSet) const;
	int FindFirst(CHAR ch) const;
	int FindLast(CHAR ch) const;
	int FindNext(CHAR ch,int idx) const;
	int Find(LPCSTR lpszSub) const;

	LPSTR GetBuffer(int nMinBufLength=-1,BYTE bStoreData=FALSE);
	int GetAllocLength() const { return m_nAllocLen; }
	void FreeExtra(int nNewLength=-1); // Call after GetBuffer()
	void Compact(); // Makes memory usage as small as possible
					// don't call after GetBuffer()
	LPSTR GiveBuffer();
	
	
		
	BOOL InsChar(DWORD idx,CHAR ch);
	BOOL Insert(DWORD idx,CHAR ch) { return InsChar(idx,ch); }
	BOOL DelChar(DWORD idx);
	BOOL Delete(DWORD idx,int nCount=1);
	void ReplaceChars(char from,char to);

	void Trim(); // Deletes spaces from begin and end

	void Swap(CString& str);

	void Format(LPCSTR lpszFormat,...);
	void Format(LPCSTR lpszFormat,va_list);
	void FormatEx(LPCSTR lpszFormat,...);

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
	CString(WCHAR ch,DWORD nRepeat=1);
	
	CString& Copy(LPCWSTR src);
	CString& Copy(LPCWSTR src,int iLength);

	const CString& operator=(const CStringW& str);
	const CString& operator=(WCHAR ch);
	const CString& operator=(LPCWSTR str);
	const CString& operator+=(const CStringW& str);
	const CString& operator+=(WCHAR ch);
	const CString& operator+=(LPCWSTR str);

	void Append(LPCWSTR str,int iLength=-1);
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
};


//////////////////////////////////////////////////
// Class CStringW

#ifdef DEF_WCHAR	

class CStringW
{
protected:
	LPWSTR m_pData;
	DWORD m_nDataLen;
	DWORD m_nAllocLen;
	BYTE m_nBase;
public:
//Constructors
	CStringW();
	CStringW(const CStringW& str);
	CStringW(WCHAR ch,DWORD nRepeat=1);
	CStringW(LPCWSTR lpsz);
	CStringW(LPCWSTR lpsz,DWORD nLength);
	CStringW(const short * lpsz);
	
	CStringW(const CString& str);
	CStringW(CHAR ch,DWORD nRepeat=1);
	CStringW(LPCSTR lpsz);
	
	~CStringW();

public:
//Attributes & Operations
	DWORD GetLength() const { return m_nDataLen; }
	BOOL IsEmpty() const { return (m_pData==NULL || !m_nDataLen); }
	void Empty();

	CStringW& SetBase(BYTE nBase);
	BYTE GetBase() { return m_nBase; }

	WCHAR GetAt(DWORD nIndex) const;
	WCHAR operator[](DWORD nIndex) const;
	WCHAR& operator[](DWORD nIndex);
	WCHAR operator[](int nIndex) const;
	WCHAR& operator[](int nIndex);
	WCHAR operator[](UINT nIndex) const;
	WCHAR& operator[](UINT nIndex);
	void SetAt(DWORD nIndex,WCHAR ch);
	operator LPCWSTR() const;
	LPWSTR GetPCHData() const { return m_pData; } // Use with caution, may be null
	operator FREEDATA();  // use delete[] (LPWSTR) pObject to release memory
	void TakeBack(FREEDATA pData);

	CStringW& Copy(LPCSTR src);
	CStringW& Copy(LPCSTR src,int iLength);
	CStringW& Copy(const BYTE* src);
	CStringW& Copy(const BYTE* src,int iLength);
	CStringW& Copy(LPCWSTR src);
	CStringW& Copy(LPCWSTR src,int iLength);
	
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
	
	void Append(LPCSTR str,int iLength=-1);
    void Append(LPCWSTR str,int iLength=-1);
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
	BYTE ContainString(LPCWSTR str,DWORD start=0);

	CStringW Mid(DWORD nFirst,DWORD nCount) const;
	CStringW Mid(DWORD nFirst) const;
	CStringW Left(DWORD nCount) const;
	CStringW Right(DWORD nCount) const;
	WCHAR LastChar() const;
	void DelLastChar();

	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	int Find(WCHAR ch) const;
	int FindOneOf(LPCWSTR lpszCharSet) const;
	int FindFirst(WCHAR ch) const;
	int FindLast(WCHAR ch) const;
	int FindNext(WCHAR ch,int idx) const;
	int Find(LPCWSTR lpszSub) const;

	LPWSTR GetBuffer(int nMinBufLength=-1,BYTE bStoreData=FALSE);
	void FreeExtra(int nNewLength=-1);
	int GetAllocLength() const { return m_nAllocLen; }
	void Compact();
	LPWSTR GiveBuffer(); 

	BOOL InsChar(DWORD idx,WCHAR ch);
	BOOL Insert(DWORD idx,WCHAR ch) { return InsChar(idx,ch); }
	BOOL DelChar(DWORD idx);
	BOOL Delete(DWORD idx,int nCount=1);
	void ReplaceChars(char from,char to);
	void Trim(); // Deletes spaces from begin and end

	void Swap(CStringW& str);

	void Format(LPCWSTR lpszFormat,...);
	void Format(LPCWSTR lpszFormat,va_list);
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
