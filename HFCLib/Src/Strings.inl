////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////
// Inline funktions for manipulating strings
////////////////////////////////////////////////////////////////////

#ifndef HFCSTR_INL
#define HFCSTR_INL

// copyers with allocation
inline LPSTR alloccopy(LPCSTR szString)
{
	SIZE_T nLength=istrlen(szString);
	char* psz=new char[max(nLength,1)+1];
	CopyMemory(psz,szString,nLength+1);
	return psz;
}

inline LPSTR alloccopy(LPCSTR szString,SIZE_T dwLength)
{
	char* psz=new char[max(dwLength,1)+1];
	CopyMemory(psz,szString,dwLength);
	psz[dwLength]='\0';
	return psz;
}

inline LPSTR allocempty()
{
	char* psz=new char[2];
	*psz='\0';
	return psz;
}

#ifdef DEF_WCHAR
inline LPWSTR alloccopy(LPCWSTR szString)
{
	SIZE_T nLength=istrlenw(szString);
	WCHAR* psz=new WCHAR[max(nLength,1)+1];
	for (register SIZE_T i=0;i<=nLength;i++)
		psz[i]=szString[i];
	return psz;
}

inline LPWSTR alloccopy(LPCWSTR szString,SIZE_T dwLength)
{
	WCHAR* psz=new WCHAR[max(dwLength,1)+1];
	for (register SIZE_T i=0;i<dwLength;i++)
		psz[i]=szString[i];
	psz[dwLength]=L'\0';
	return psz;
}

inline LPSTR alloccopyWtoA(LPCWSTR szString)
{
	SIZE_T dwLength=istrlenw(szString);
	CHAR* psz=new CHAR[max(dwLength,1)+1];
	MemCopyWtoA(psz,szString,dwLength+1);
	return psz;
}

inline LPSTR alloccopyWtoA(LPCWSTR szString,SIZE_T dwLength)
{
	CHAR* psz=new CHAR[max(dwLength,1)+1];
	MemCopyWtoA(psz,szString,dwLength);
	psz[dwLength]=L'\0';
	return psz;
}

inline LPWSTR alloccopyAtoW(LPCSTR szString)
{
	SIZE_T dwLength=istrlen(szString);
	WCHAR* psz=new WCHAR[max(dwLength,1)+1];
	MemCopyAtoW(psz,szString,dwLength+1);
	return psz;
}

inline LPWSTR alloccopyAtoW(LPCSTR szString,SIZE_T dwLength)
{
	WCHAR* psz=new WCHAR[max(dwLength,1)+1];
	MemCopyAtoW(psz,szString,dwLength);
	psz[dwLength]='\0';
	return psz;
}
inline LPWSTR allocemptyW()
{
	WCHAR* psz=new WCHAR[2];
	*psz=L'\0';
	return psz;
}


#endif


inline void MakeLower(LPSTR szStr)
{
#ifdef WIN32
	CharLower(szStr);
#else
	strlwr(szStr);
#endif
}

inline void MakeUpper(LPSTR szStr)
{
#ifdef WIN32
	CharUpper(szStr);
#else
	strupr(szStr);
#endif
}

#ifdef DEF_WCHAR
inline void MakeLower(LPWSTR szStr)
{
	if (IsFullUnicodeSupport())
		CharLowerW(szStr);
	else
		_wcslwr_s(szStr,istrlenw(szStr)+1);
}

inline void MakeUpper(LPWSTR szStr)
{
	if (IsFullUnicodeSupport())
		CharLowerW(szStr);
	else
		_wcsupr_s(szStr,istrlenw(szStr)+1);
}
#endif

// char replacers
inline void replacech(LPSTR str,char from,char to)
{
	for (register UINT i=0;str[i]!='\0';i++)
	{
		if (str[i]==from)
			str[i]=to;
	}
}

#ifdef DEF_WCHAR
inline void replacech(LPWSTR str,WCHAR from,WCHAR to)
{
	for (register UINT i=0;str[i]!='\0';i++)
	{
		if (str[i]==from)
			str[i]=to;
	}
}
#endif

// parsers
inline UINT fparseto(LPCSTR str,char ch)
{
	register UINT i=0;
	for (;str[i]!=ch;i++);
	return i;
}

#ifdef DEF_WCHAR
inline UINT fwparse(LPCWSTR str,WCHAR ch)
{
	register UINT i=0;
	for (;str[i]!=ch;i++);
	return i;
}
#endif

///////////////////////////////////////////
// inlines for CString

inline CString::CString()
:	m_pData(NULL),m_nDataLen(0),m_nAllocLen(0),m_nBase(10)
{
}

inline CString::~CString()
{
	if (m_pData!=NULL)
		delete[] m_pData;
}



inline void CString::Empty()
{
	if (m_pData!=NULL)
	{
		delete[] m_pData;
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
}

inline CString& CString::SetBase(BYTE nBase)
{
	if (nBase>=2 && nBase<=36)
		m_nBase=nBase;
	return *this;
}

inline CHAR CString::GetAt(ULONG_PTR nIndex) const
{
	if (nIndex<m_nAllocLen && m_pData!=NULL)
		return m_pData[nIndex];
	else
		return 0;
}

inline CHAR CString::operator[](ULONG_PTR nIndex) const
{
	if (nIndex<m_nAllocLen && m_pData!=NULL)
		return m_pData[nIndex];
	else
		return 0;
}

inline CHAR CString::operator[](LONG_PTR nIndex) const
{
	if ((ULONG_PTR)nIndex<m_nAllocLen && m_pData!=NULL)
		return m_pData[nIndex];
	else
		return 0;
}

inline CHAR CString::operator[](int nIndex) const
{
	if (nIndex>=0 && (DWORD)nIndex<m_nAllocLen && m_pData!=NULL)
		return m_pData[nIndex];
	else
		return 0;
}




inline CHAR CString::operator[](UINT nIndex) const
{
	if (nIndex>=0 && (DWORD)nIndex<m_nAllocLen && m_pData!=NULL)
		return m_pData[nIndex];
	else
		return 0;
}

#ifdef DEF_WCHAR
inline const CString& CString::operator=(LPCWSTR str)
{
	return Copy(str);
}
#endif

inline const CString& CString::operator=(LPCSTR str)
{
	return Copy(str);
}

inline const CString& CString::operator=(unsigned char * str)
{
	return Copy(str);
}

inline void CString::SetAt(DWORD nIndex,CHAR ch)
{
	if (nIndex<m_nAllocLen)
		m_pData[nIndex]=ch;
}

inline CString CString::operator+(const CString& str)
{
	CString temp(m_pData);
	temp+=str;
	return temp;
}

inline CString CString::operator+(const LPCSTR str)
{
	CString temp(m_pData);
	temp+=str;
	return temp;
}

inline CString CString::operator+(const CHAR ch)
{
	CString temp(m_pData);
	temp+=ch;
	return temp;
}

#ifdef DEF_WCHAR
inline CString CString::operator+(const CStringW& str)
{
	CString temp(m_pData);
	temp+=str;
	return temp;
}

inline CString CString::operator+(const LPCWSTR str)
{
	CString temp(m_pData);
	temp+=str;
	return temp;
}

inline CString CString::operator+(const WCHAR ch)
{
	CString temp(m_pData);
	temp+=ch;
	return temp;
}
#endif
 

inline void CString::Append(const CString& str)
{
	*this << str;
}


inline void CString::Append(CHAR ch)
{
	*this << ch;
}

#ifdef DEF_WCHAR
inline void CString::Append(const CStringW& str)
{
	*this << str;
}

inline void CString::Append(WCHAR ch)
{
	*this << ch;
}
#endif 

inline void CString::MakeUpper()
{
	if (m_pData!=NULL)
		CharUpperA(m_pData);
}

inline void CString::MakeLower()
{
	if (m_pData!=NULL)
		CharLower(m_pData);
}

inline BOOL CString::operator==(LPCSTR str)
{
	return (Compare(str)==0);
}

#ifdef DEF_WCHAR
inline BOOL CString::operator==(LPCWSTR str)
{
	return (Compare(str)==0);
}
#endif

inline CString CString::Mid(DWORD nFirst,DWORD nCount) const
{
	if (nFirst+nCount<m_nDataLen)
		return CString (&m_pData[nFirst],nCount);
	else
		return CString (&m_pData[m_nDataLen-nCount]);
}

inline CString CString::Mid(DWORD nFirst) const
{
	if (m_nDataLen>nFirst)
		return CString(&m_pData[nFirst]);
	else
		return CString(&m_pData[m_nDataLen]);
}

inline CString CString::Left(DWORD nCount) const
{
	return CString(m_pData,nCount);
}

inline CString CString::Right(DWORD nCount) const
{
	return CString(&m_pData[m_nDataLen-nCount]);
}

#ifdef DEF_RESOURCES
inline BOOL CString::AddString(UINT nID)
{
	CString str((UINT)nID);
	*this << str;
	return TRUE;
}

inline BOOL CString::AddString(UINT nID,TypeOfResourceHandle bType)
{
	CString str((UINT)nID,bType);
	*this << str;
	return TRUE;
}
#endif

inline CHAR CString::LastChar() const
{
	return m_nDataLen>0?m_pData[m_nDataLen-1]:'\0';
}

inline void CString::DelLastChar()
{
	m_pData[--m_nDataLen]='\0';
}	

inline LPSTR CString::GiveBuffer()
{
	LPSTR pRet=m_pData;
	m_pData=NULL;
	m_nDataLen=0;
	m_nAllocLen=0;
	return pRet;
}

///////////////////////////////////////////
// inlines for CStringW

#ifdef DEF_WCHAR

inline CStringW::CStringW()
:	m_pData(NULL),m_nDataLen(0),m_nAllocLen(0),m_nBase(10)
{
}

inline CStringW::~CStringW()
{
	if (m_pData!=NULL)
		delete[] m_pData;
}

inline void CStringW::Empty()
{
	if (m_pData!=NULL)
	{
		delete[] m_pData;
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
}

inline CStringW& CStringW::SetBase(BYTE nBase)
{
	if (nBase>=2 && nBase<=36)
		m_nBase=nBase;
	return *this;
}

inline WCHAR CStringW::GetAt(DWORD nIndex) const
{
	if (nIndex<m_nAllocLen && m_pData!=NULL)
		return m_pData[nIndex];
	else
		return 0;
}

inline WCHAR CStringW::operator[](DWORD nIndex) const
{
	if (nIndex<m_nAllocLen && m_pData!=NULL)
		return m_pData[nIndex];
	else
		return 0;
}

inline WCHAR CStringW::operator[](int nIndex) const
{
	if (nIndex>=0 && (DWORD)nIndex<m_nAllocLen && m_pData!=NULL)
		return m_pData[nIndex];
	else
		return 0;
}

inline WCHAR CStringW::operator[](UINT nIndex) const
{
	if (nIndex>=0 && (DWORD)nIndex<m_nAllocLen && m_pData!=NULL)
		return m_pData[nIndex];
	else
		return 0;
}

inline void CStringW::SetAt(DWORD nIndex,WCHAR ch)
{
	if (nIndex<m_nAllocLen)
		m_pData[nIndex]=ch;
}

inline CStringW::operator LPCWSTR() const
{
	if (m_pData==NULL)
		return szwEmpty;
	return m_pData;
}

inline CStringW CStringW::operator+(const CStringW& str)
{
	CStringW temp(m_pData);
	temp+=str;
	return temp;
}

inline CStringW CStringW::operator+(const LPCWSTR str)
{
	CStringW temp(m_pData);
	temp+=str;
	return temp;
}

inline CStringW CStringW::operator+(const WCHAR ch)
{
	CStringW temp(m_pData);
	temp+=ch;
	return temp;
}

inline CStringW CStringW::operator+(const CString& str)
{
	CStringW temp(m_pData);
	temp+=str;
	return temp;
}

inline CStringW CStringW::operator+(const LPCSTR str)
{
	CStringW temp(m_pData);
	temp+=str;
	return temp;
}

inline CStringW CStringW::operator+(const CHAR ch)
{
	CStringW temp(m_pData);
	temp+=ch;
	return temp;
}


inline void CStringW::Append(const CString& str)
{
	*this << str;
}

inline void CStringW::Append(const CStringW& str)
{
	*this << str;
}

inline void CStringW::Append(CHAR ch)
{
	*this << ch;
}

inline void CStringW::Append(WCHAR ch)
{
	*this << ch;
}

inline const CStringW& CStringW::operator=(LPCSTR str)
{
	return Copy(str);
}

inline const CStringW& CStringW::operator=(LPCWSTR str)
{
	return Copy(str);
}

inline const CStringW& CStringW::operator=(unsigned short * str)
{
	return Copy((LPWSTR)str);
}

inline BOOL CStringW::operator==(LPCWSTR str)
{
	return (Compare(str)==0);
}

inline BOOL CStringW::operator==(LPCSTR str)
{
	return (Compare(str)==0);
}

inline CStringW CStringW::Mid(DWORD nFirst,DWORD nCount) const
{
	if (nFirst+nCount<m_nDataLen)
		return CStringW (&m_pData[nFirst],nCount);
	else
		return CStringW (&m_pData[m_nDataLen-nCount]);
}

inline CStringW CStringW::Mid(DWORD nFirst) const
{
	if (m_nDataLen>nFirst)
		return CStringW(&m_pData[nFirst]);
	else
		return CStringW(&m_pData[m_nDataLen]);
}

inline CStringW CStringW::Left(DWORD nCount) const
{
	return CStringW(m_pData,nCount);
}

inline CStringW CStringW::Right(DWORD nCount) const
{
	return CStringW(&m_pData[m_nDataLen-nCount]);
}


inline WCHAR CStringW::LastChar() const
{
	return m_nDataLen>0?m_pData[m_nDataLen-1]:'\0';
}

inline void CStringW::DelLastChar()
{
	m_pData[--m_nDataLen]='\0';
}	

inline LPWSTR CStringW::GiveBuffer()
{
	LPWSTR pRet=m_pData;
	m_pData=NULL;
	m_nDataLen=0;
	m_nAllocLen=0;
	return pRet;
}

#endif 

// dataparsers

inline bool _1stcontain2nd(const char* s1,const char* s2)
{
	for (register int i=0;s2[i]!='\0';i++)
		if (s1[i]!=s2[i])
			return FALSE;
	return TRUE;
}

inline int chartonum(char ch)
{
	if (ch>='0' && ch<='9')
		return ch-'0';
	if (ch>='a' && ch<='z')
		return ch-'a'+10;
	if (ch>='A' && ch<='Z')
		return ch-'a'+10;
	return 0;
}

// szString will not change
inline int readnum(int base,LPCSTR szString,SIZE_T length=SIZE_T(-1))
{
	return _readnum(base,szString,length);
}

inline BYTE* dataparser(LPCSTR pString,SIZE_T dwStrLen,SIZE_T* pdwDataLength=NULL)
{
	return dataparser(pString,dwStrLen,malloc,pdwDataLength);
}
inline BYTE* dataparser(const CString& str,SIZE_T* pdwDataLength=NULL)
{
	return dataparser(LPCSTR(str),str.GetLength(),pdwDataLength);
}

inline BYTE* dataparser(LPCSTR str,SIZE_T* pdwDataLength=NULL)
{
	return dataparser(str,istrlen(str),pdwDataLength);
}






inline W2A::W2A(LPCWSTR sA)
{
	pAStr=alloccopyWtoA(sA);
}

inline W2A::W2A(LPCWSTR sA,SIZE_T len)
{
	pAStr=alloccopyWtoA(sA,len);
}

inline W2A::W2A(LPCWSTR sA,int len)
{
	pAStr=alloccopyWtoA(sA,len);
}

inline W2A::W2A(CStringW& sA)
{
	pAStr=alloccopyWtoA(sA,sA.GetLength());
}


inline W2A::~W2A()
{
	delete pAStr;
}

inline W2A::operator LPCSTR() const
{
	return pAStr;
}

inline A2W::A2W(LPCSTR sA)
{
	pWStr=alloccopyAtoW(sA);
}

inline A2W::A2W(LPCSTR sA,SIZE_T len)
{
	pWStr=alloccopyAtoW(sA,len);
}

inline A2W::A2W(LPCSTR sA,int len)
{
	pWStr=alloccopyAtoW(sA,len);
}

inline A2W::A2W(CString& sA)
{
	pWStr=alloccopyAtoW(sA,sA.GetLength());
}

inline A2W::~A2W()
{
	delete pWStr;
}

inline A2W::operator LPCWSTR() const
{
	return pWStr;
}

#endif
