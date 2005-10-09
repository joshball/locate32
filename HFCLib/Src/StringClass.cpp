////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"
#include <stdarg.h>

//#define OUTPUT_LENGTHS

#define STR_EXTRAALLOC					64
#define STR_LOADSTRINGBUFLEN			1024

#if defined(_DEBUG) && defined(OUTPUT_LENGTHS)
#define OUTPUT(len)		DebugNumMessage("string length=%d",(int)(len));
#else
#define OUTPUT(len)
#endif

#define sMemCopy(dst,src,len)	CopyMemory(dst,src,len)
#define sMemZero(dst,len)		ZeroMemory(dst,len)
#define sMemSet(dst,val,len)	iMemSet(dst,val,len)
#define sstrlen(str,len)		dstrlen(str,len)

#define sMemCopyW				iMemCopyW
#define sstrlenW				dwstrlen

CString::CString(const CString& str)
:	m_nBase(10)
{
	if (&str!=this && str.m_pData!=NULL)
	{
		m_nDataLen=str.m_nDataLen;
		OUTPUT(m_nDataLen)
		m_nBase=str.m_nBase;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		dMemCopy(m_pData,str.m_pData,m_nDataLen+1);
	}
	else
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
		m_nBase=10;
	}
}

CString::CString(CHAR ch,DWORD nRepeat)
:	m_nBase(10)
{
	if (!nRepeat)
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
	else
	{
		m_nDataLen=nRepeat;
		OUTPUT(m_nDataLen)
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		for (m_nDataLen=0;m_nDataLen<nRepeat;m_nDataLen++)
			m_pData[m_nDataLen]=ch;
		m_pData[m_nDataLen]='\0';
	}
}

CString::CString(LPCSTR lpsz)
:	m_nBase(10)
{
	if (lpsz!=NULL)
	{
		//m_nDataLen=strlen(lpsz);
		sstrlen(lpsz,m_nDataLen);
		
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		sMemCopy(m_pData,lpsz,m_nDataLen+1);
	}
	else
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
}

CString::CString(LPCSTR lpsz,DWORD nLength)
:	m_nBase(10)
{
	if (lpsz==NULL || nLength<=0)
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
	else
	{
		//m_nDataLen=strlen(lpsz);
		sstrlen(lpsz,m_nDataLen);

		OUTPUT(m_nDataLen)
		if (m_nDataLen>nLength)
			m_nDataLen=nLength;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		sMemCopy(m_pData,lpsz,m_nDataLen);
		m_pData[m_nDataLen]='\0';
	}
}

CString::CString(const unsigned char * lpsz)
:	m_nBase(10)
{
	if (lpsz==NULL)
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
	else
	{
		//m_nDataLen=strlen((LPSTR)lpsz);
		sstrlen(lpsz,m_nDataLen);

		OUTPUT(m_nDataLen)
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		sMemCopy(m_pData,lpsz,m_nDataLen+1);
	}
}

#ifdef DEF_WCHAR
CString::CString(const CStringW& str)
:	m_nBase(10)
{
	if (str.m_pData!=NULL)
	{
		m_nDataLen=str.m_nDataLen;
		OUTPUT(m_nDataLen)
		m_nBase=str.m_nBase;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		MemCopyWtoA(m_pData,str.m_pData,m_nDataLen+1);
	}
	else
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
		m_nBase=10;
	}
}

CString::CString(LPCWSTR lpsz)
	:m_nBase(10)
{
	if (lpsz!=NULL)
	{
		//m_nDataLen=wcslen(lpsz);
		sstrlen(lpsz,m_nDataLen);

		OUTPUT(m_nDataLen)
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		MemCopyWtoA(m_pData,lpsz,m_nDataLen+1);
	}
	else
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
}

CString::CString(WCHAR ch,DWORD nRepeat)
	:m_nBase(10)
{
	if (!nRepeat)
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
	else
	{
		m_nDataLen=nRepeat;
		OUTPUT(m_nDataLen)
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		
		CHAR nch;
		MemCopyWtoA(&nch,&ch,2);

		for (m_nDataLen=0;m_nDataLen<nRepeat;m_nDataLen++)
			m_pData[m_nDataLen]=nch;
		m_pData[m_nDataLen]='\0';
	}
}
#endif

CHAR& CString::operator[](DWORD nIndex)
{
	if (m_pData==NULL)
		return (char)*szEmpty;
	if (nIndex<m_nAllocLen)
		return m_pData[nIndex];
	else
		return m_pData[m_nAllocLen-1];
}

CHAR& CString::operator[](int nIndex)
{
	if (m_pData==NULL)
		return (char)*szEmpty;
	if (nIndex>=0 && (DWORD)nIndex<m_nAllocLen)
		return m_pData[nIndex];
	else
		return m_pData[m_nAllocLen-1];
}
	
CHAR& CString::operator[](UINT nIndex)
{
	if (m_pData==NULL)
		return (char)*szEmpty;
	if (nIndex>=0 && (DWORD)nIndex<m_nAllocLen)
		return m_pData[nIndex];
	else
		return m_pData[m_nAllocLen-1];
}

CString& CString::Copy(LPCSTR str)
{
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	sstrlen(str,m_nDataLen);
	m_pData=new char[m_nAllocLen=m_nDataLen+1];
	sMemCopy(m_pData,str,m_nDataLen);
	m_pData[m_nDataLen]='\0';
	return *this;
}

CString& CString::Copy(LPCSTR str,int iLength)
{
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	if (iLength<0)
		sstrlen(str,m_nDataLen);
	else
		m_nDataLen=iLength;
	m_pData=new char[m_nAllocLen=m_nDataLen+1];
	sMemCopy(m_pData,str,m_nDataLen);
	m_pData[m_nDataLen]='\0';
	return *this;
}

#ifdef DEF_WCHAR
CString& CString::Copy(LPCWSTR str)
{
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	for (m_nDataLen=0;str[m_nDataLen]!='\0';m_nDataLen++); //dstrlen(str,m_nDataLen);
	m_pData=new char[m_nAllocLen=m_nDataLen+1];
	MemCopyWtoA(m_pData,str,m_nDataLen+1);
	return *this;
}

CString& CString::Copy(LPCWSTR str,int iLength)
{
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	if (iLength<0)
		dstrlen(str,m_nDataLen);
	else
		m_nDataLen=iLength;
	m_pData=new char[m_nAllocLen=m_nDataLen+1];
	MemCopyWtoA(m_pData,str,m_nDataLen);
	m_pData[m_nDataLen]='\0';
	return *this;
}
#endif

CString& CString::Copy(const BYTE* str)
{ 
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	for (m_nDataLen=0;str[m_nDataLen]!='\0';m_nDataLen++); // dstrlen
	m_pData=new char[m_nAllocLen=m_nDataLen+1];
	sMemCopy(m_pData,str,m_nDataLen);
	m_pData[m_nDataLen]='\0';

	return *this;
}

CString& CString::Copy(const BYTE* str,int iLength)
{
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	if (iLength<0)
		sstrlen(str,m_nDataLen);
	else
		m_nDataLen=iLength;
	m_pData=new char[m_nAllocLen=m_nDataLen+1];
	sMemCopy(m_pData,str,m_nDataLen);
	m_pData[m_nDataLen]='\0';
	return *this;
}

const CString& CString::operator=(const CString& str)
{
	if (str.m_nDataLen==0)
	{
		if (m_pData!=NULL)
		{
			delete[] m_pData;
			m_pData=NULL;
			m_nDataLen=0;
			m_nAllocLen=0;
		}
		return *this;
	}
	return Copy(str,str.GetLength());
}

const CString& CString::operator=(CHAR ch)
{
	if (m_nAllocLen<2 || m_nAllocLen>STR_EXTRAALLOC)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new CHAR[m_nAllocLen=STR_EXTRAALLOC];
		OUTPUT(m_nDataLen)
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
	}
	m_pData[0]=ch;
	m_pData[1]='\0';
	m_nDataLen=1;
	return *this;
}

const CString& CString::operator=(DWORD iNum)
{
	CHAR szBuffer[34];
	itoa(iNum,szBuffer,m_nBase);
	
	//m_nDataLen=strlen(szBuffer);
	sstrlen(szBuffer,m_nDataLen);
	
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
	}
	sMemCopy(m_pData,szBuffer,m_nDataLen+1);
	return *this;
}

const CString& CString::operator=(int iNum)
{
	CHAR szBuffer[34];
	itoa(iNum,szBuffer,m_nBase);
	//m_nDataLen=strlen(szBuffer);
	sstrlen(szBuffer,m_nDataLen);

	OUTPUT(m_nDataLen)
		
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
	}
	sMemCopy(m_pData,szBuffer,m_nDataLen+1);
	return *this;
}

#ifdef DEF_WCHAR
const CString& CString::operator=(const CStringW& str)
{
	if (str.m_nDataLen==0)
	{
		if (m_pData!=NULL)
		{
			delete[] m_pData;
			m_pData=NULL;
			m_nDataLen=0;
			m_nAllocLen=0;
		}
		return *this;
	}
	return Copy(str,str.GetLength());
}

const CString& CString::operator=(WCHAR ch)
{
	if (m_nAllocLen<2 || m_nAllocLen>10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new CHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
	}
	MemCopyWtoA(m_pData,&ch,2);
	m_pData[1]='\0';
	m_nDataLen=1;
	return *this;
}

#endif

const CString& CString::operator+=(const CString& str)
{
	if (str.m_nDataLen==0 || &str==this)
		return *this;	
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+str.m_nDataLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopy(&m_pData[m_nDataLen],str.m_pData,str.m_nDataLen+1);
		m_nDataLen+=str.m_nDataLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=str.m_nDataLen;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopy(m_pData,str.m_pData,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

const CString& CString::operator+=(CHAR ch)
{
	if (m_pData!=NULL)
	{
		if (m_nDataLen+1>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		m_pData[m_nDataLen++]=ch;
		m_pData[m_nDataLen]='\0';
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=1;
		m_pData=new CHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		m_pData[0]=ch;
		m_pData[1]='\0';
		OUTPUT(m_nDataLen)
		return *this;
	}
}

void CString::Append(LPCSTR str,int nStrLen)
{
	if (str==NULL)
		return;
	if (nStrLen==-1)
		sstrlen(str,nStrLen);

	if (nStrLen==0)
		return;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopy(&m_pData[m_nDataLen],str,nStrLen);
		m_nDataLen+=nStrLen;
		m_pData[m_nDataLen]='\0';
		OUTPUT(m_nDataLen)
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		sMemCopy(m_pData,str,m_nDataLen);
		m_pData[m_nDataLen]='\0';
		OUTPUT(m_nDataLen)
	}
}

const CString& CString::operator+=(LPCSTR str)
{
	if (str==NULL)
		return *this;	
	DWORD nStrLen;
	sstrlen(str,nStrLen);

	if (nStrLen==0)
		return *this;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopy(&m_pData[m_nDataLen],str,nStrLen+1);
		m_nDataLen+=nStrLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopy(m_pData,str,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

const CString& CString::operator+=(DWORD iNum)
{
	CHAR szBuffer[34];
	itoa(iNum,szBuffer,m_nBase);
	if (m_pData!=NULL)
	{
		DWORD nStrLen;
		sstrlen(szBuffer,nStrLen);

		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopy(&m_pData[m_nDataLen],szBuffer,nStrLen+1);
		m_nDataLen+=nStrLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		//m_nDataLen=strlen((LPSTR)szBuffer);
		sstrlen(szBuffer,m_nDataLen);
		
		m_pData=new CHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopy(m_pData,szBuffer,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

const CString& CString::operator+=(int iNum)
{
	CHAR szBuffer[34];
	itoa(iNum,szBuffer,m_nBase);
	if (m_pData!=NULL)
	{
		DWORD nStrLen; 
		sstrlen(szBuffer,nStrLen);

		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopy(&m_pData[m_nDataLen],szBuffer,nStrLen+1);
		m_nDataLen+=nStrLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		//m_nDataLen=strlen((LPSTR)szBuffer);
		sstrlen(szBuffer,m_nDataLen);

		m_pData=new CHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopy(m_pData,szBuffer,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

#ifdef DEF_WCHAR
const CString& CString::operator+=(const CStringW& str)
{
	if (str.m_nDataLen==0)
		return *this;	
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+str.m_nDataLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyWtoA(&m_pData[m_nDataLen],str.m_pData,str.m_nDataLen+1);
		m_nDataLen+=str.m_nDataLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=str.m_nDataLen;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyWtoA(m_pData,str.m_pData,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

const CString& CString::operator+=(WCHAR ch)
{
	if (m_pData!=NULL)
	{
		if (m_nDataLen+1>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyWtoA(m_pData+m_nDataLen,&ch,2);
		m_pData[++m_nDataLen]='\0';
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=1;
		m_pData=new CHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyWtoA(m_pData,&ch,2);
		m_pData[1]='\0';
		OUTPUT(m_nDataLen)
		return *this;
	}
}

const CString& CString::operator+=(LPCWSTR str)
{
	if (str==NULL)
		return *this;	
	
	DWORD nStrLen;
	sstrlen(str,nStrLen);

	if (nStrLen==0)
		return *this;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyWtoA(&m_pData[m_nDataLen],str,nStrLen+1);
		m_nDataLen+=nStrLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyWtoA(m_pData,str,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

void CString::Append(LPCWSTR str,int nStrLen)
{
	if (str==NULL)
		return;	
	
	if (nStrLen!=-1)
		sstrlen(str,nStrLen);

	if (nStrLen==0)
		return;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyWtoA(&m_pData[m_nDataLen],str,nStrLen+1);
		m_nDataLen+=nStrLen;
		OUTPUT(m_nDataLen)
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		MemCopyWtoA(m_pData,str,m_nDataLen+1);
		OUTPUT(m_nDataLen)
	}
}
#endif


CString& CString::operator<<(const CString& str)
{
	if (str.m_nDataLen==0 || &str==this)
		return *this;	
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+str.m_nDataLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopy(&m_pData[m_nDataLen],str.m_pData,str.m_nDataLen+1);
		m_nDataLen+=str.m_nDataLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=str.m_nDataLen;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopy(m_pData,str.m_pData,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

CString& CString::operator<<(CHAR ch)
{
	if (m_pData!=NULL)
	{
		if (m_nDataLen+1>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		m_pData[m_nDataLen++]=ch;
		m_pData[m_nDataLen]='\0';
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=1;
		m_pData=new CHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		m_pData[0]=ch;
		m_pData[1]='\0';
		OUTPUT(m_nDataLen)
		return *this;
	}
}

CString& CString::operator<<(LPCSTR str)
{
	if (str==NULL)
		return *this;	
	DWORD nStrLen;
	sstrlen(str,nStrLen);

	if (nStrLen==0)
		return *this;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopy(&m_pData[m_nDataLen],str,nStrLen+1);
		m_nDataLen+=nStrLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopy(m_pData,str,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

CString& CString::operator<<(DWORD iNum)
{
	CHAR szBuffer[34];
	itoa(iNum,szBuffer,m_nBase);
	if (m_pData!=NULL)
	{
		DWORD nStrLen;
		sstrlen(szBuffer,nStrLen);

		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopy(&m_pData[m_nDataLen],szBuffer,nStrLen+1);
		m_nDataLen+=nStrLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		//m_nDataLen=strlen((LPSTR)szBuffer);
		sstrlen(szBuffer,m_nDataLen);

		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopy(m_pData,szBuffer,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

CString& CString::operator<<(int iNum)
{
	CHAR szBuffer[34];
	itoa(iNum,szBuffer,m_nBase);
	if (m_pData!=NULL)
	{
		DWORD nStrLen;
		sstrlen(szBuffer,nStrLen);

		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopy(&m_pData[m_nDataLen],szBuffer,nStrLen+1);
		m_nDataLen+=nStrLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		//m_nDataLen=strlen((LPSTR)szBuffer);
		sstrlen(szBuffer,m_nDataLen);
		
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopy(m_pData,szBuffer,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

#ifdef DEF_WCHAR
CString& CString::operator<<(const CStringW& str)
{
	if (str.m_nDataLen==0)
		return *this;	
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+str.m_nDataLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyWtoA(&m_pData[m_nDataLen],str.m_pData,str.m_nDataLen+1);
		m_nDataLen+=str.m_nDataLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=str.m_nDataLen;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyWtoA(m_pData,str.m_pData,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}

CString& CString::operator<<(WCHAR ch)
{
	if (m_pData!=NULL)
	{
		if (m_nDataLen+1>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyWtoA(m_pData+m_nDataLen,&ch,2);
		m_pData[m_nDataLen]='\0';
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=1;
		m_pData=new CHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyWtoA(m_pData,&ch,2);
		m_pData[1]='\0';
		OUTPUT(m_nDataLen)
		return *this;
	}
}

CString& CString::operator<<(LPCWSTR str)
{
	if (str==NULL)
		return *this;	
	
	DWORD nStrLen;
	sstrlen(str,nStrLen);

	if (nStrLen==0)
		return *this;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopy(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyWtoA(&m_pData[m_nDataLen],str,nStrLen+1);
		m_nDataLen+=nStrLen;
		OUTPUT(m_nDataLen)
		return *this;	
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyWtoA(m_pData,str,m_nDataLen+1);
		OUTPUT(m_nDataLen)
		return *this;
	}
}
#endif

int CString::Compare(LPCSTR lpsz) const
{
	if (m_pData==NULL)
	{
		if (lpsz==NULL)
			return 0;
		else
		{
			if (lpsz[0]=='\0')
				return 0;
			else
				return -1;
		}
	}
	if (lpsz==NULL)
		return 1;
	return strcmp(m_pData,lpsz);
}

int CString::CompareNoCase(LPCSTR lpsz) const
{
	if (m_pData==NULL)
	{
		if (lpsz==NULL)
			return 0;
		else
		{
			if (lpsz[0]=='\0')
				return 0;
			else
				return -1;
		}
	}
	if (lpsz==NULL)
		return 1;
	CHAR *tmp1,*tmp2;
	int ret;
	tmp1=new CHAR[m_nDataLen+2];
	
	sstrlen(lpsz,ret);
	tmp2=new CHAR[ret+2];

	if (tmp1==NULL || tmp2==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return -1;
	}
	sMemCopy(tmp1,m_pData,m_nDataLen+1);
	sMemCopy(tmp2,lpsz,ret+1);
	::CharLower(tmp1);
	::CharLower(tmp2);
	ret=strcmp(tmp1,tmp2);
	delete[] tmp1;
	delete[] tmp2;
	return ret;
}

#ifdef DEF_WCHAR

int CString::Compare(LPCWSTR lpsz) const
{
	if (m_pData==NULL)
	{
		if (lpsz==NULL)
			return 0;
		else
		{
			if (lpsz[0]=='\0')
				return 0;
			else
				return -1;
		}
	}
	if (lpsz==NULL)
		return 1;
	WCHAR* lpwsz=new WCHAR[m_nDataLen+2];
	if (lpwsz==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return -1;
	}
	MemCopyAtoW(lpwsz,m_pData,m_nDataLen);
	int ret=wcsncmp(lpwsz,lpsz,m_nDataLen);
	delete[] lpwsz;
	return ret;
}

int CString::CompareNoCase(LPCWSTR lpsz) const
{
	if (m_pData==NULL)
	{
		if (lpsz==NULL)
			return 0;
		else
		{
			if (lpsz[0]=='\0')
				return 0;
			else
				return -1;
		}
	}
	if (lpsz==NULL)
		return 1;
	WCHAR *tmp1,*tmp2;
	DWORD ret;
	tmp1=new WCHAR[m_nDataLen+2];
	sstrlenW(lpsz,ret);
	tmp2=new WCHAR[ret+2];
	if (tmp1==NULL || tmp2==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return -1;
	}
	MemCopyAtoW(tmp1,m_pData,m_nDataLen);
	sMemCopy(tmp2,lpsz,ret);
	::CharLowerBuffW(tmp1,m_nDataLen);
	::CharLowerBuffW(tmp2,ret);
	int ret2=wcsncmp(tmp1,tmp2,min(m_nDataLen,ret));
	delete[] tmp1;
	delete[] tmp2;
	return ret2!=0?ret2:ret-m_nDataLen;
}

#endif

BOOL CString::operator==(const CString& str)
{
	if (m_pData==NULL)
	{
		if (str.m_nDataLen==0)
			return TRUE;
		else
			return FALSE;
	}
	if (str.m_pData==NULL)
		return FALSE;
	if (strcmp(m_pData,str.m_pData)==0)
		return TRUE;
	return FALSE;
}

#ifdef DEF_WCHAR
BOOL CString::operator==(const CStringW& str)
{
	if (m_pData==NULL)
	{
		if (str.m_nDataLen==0)
			return TRUE;
		else
			return FALSE;
	}
	if (str.m_pData==NULL)
		return FALSE;
	if (m_nDataLen!=str.m_nDataLen)
		return FALSE;
	WCHAR* lpwsz=new WCHAR[m_nDataLen+2];
	if (lpwsz==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return FALSE;
	}
	MemCopyAtoW(lpwsz,m_pData,m_nDataLen);
	if (wcsncmp(lpwsz,str.m_pData,m_nDataLen)==0)
	{
		delete[] lpwsz;
		return TRUE;
	}
	delete[] lpwsz;
	return FALSE;
}

#endif

BYTE CString::ContainString(LPCSTR str,DWORD start)
{
	DWORD i,j=0;
	if (m_pData==NULL)
	{
		if (str==NULL)
			return TRUE;
		else
		{
			if (str[0]=='\0')
				return TRUE;
			else
				return FALSE;
		}
	}
	for (i=start;i<m_nDataLen;i++)
	{
		j=0;
		while ((m_pData[i+j]==str[j] || str[j]=='?') && m_pData[i+j]!='\0' ) 
			j++;
		if (str[j]=='\0')
			return 1;
		if (str[j]=='*')
			return ContainString(str+j+1,j+i+1);
	}
	return 0;
}

void CString::MakeReverse()
{
	if (m_pData==NULL)
		return;
	CHAR* pchNewData=new CHAR[m_nDataLen];
	for (UINT i=0;i<m_nDataLen;i++)
		pchNewData[i]=m_pData[m_nDataLen-i-1];
	pchNewData[m_nDataLen]='\0';
	delete[] m_pData;
	m_pData=pchNewData;
}

int CString::Find(CHAR ch) const
{
	if (m_pData==NULL)
		return -1;
	DWORD i;
	for (i=0;i<m_nDataLen;i++)
	{
		if (m_pData[i]==ch)
			return i;
	}
	return -1;
}

int CString::FindFirst(CHAR ch) const
{
	if (m_pData==NULL)
		return -1;
	DWORD i;
	for (i=0;i<m_nDataLen;i++)
	{
		if (m_pData[i]==ch)
			return i;
	}
	return -1;
}

int CString::FindLast(CHAR ch) const
{
	if (m_pData==NULL)
		return -1;
	int i;
	for (i=m_nDataLen-1;i>=0;i--)
	{
		if (m_pData[i]==ch)
			return i;
	}
	return -1;
}
int CString::FindNext(CHAR ch,int idx) const
{
	if (m_pData==NULL)
		return -1;
	DWORD i;
	for (i=idx+1;i<m_nDataLen;i++)
	{
		if (m_pData[i]==ch)
			return i;	
	}
   	return -1;
}	

int CString::FindOneOf(LPCSTR lpszCharSet) const
{
	if (m_pData==NULL)
		return -1;
	for (DWORD i=0;i<m_nDataLen;i++)
	{
		for (DWORD j=0;lpszCharSet[j]!='\0';j++)
		{
			if (m_pData[i]==lpszCharSet[j])
				return i;
		}
	}
	return -1;
}

int CString::Find(LPCSTR lpszSub) const
{
	if (m_pData==NULL)
		return -1;
	LPSTR pret=strstr(m_pData,lpszSub);
	if (pret!=NULL)
		return (int)(pret-m_pData);
	return -1;
}

LPSTR CString::GetBuffer(int nMinBufLength,BYTE bStoreData)
{
	if (nMinBufLength==-1)
	{
		if (m_pData==NULL)
		{
			m_pData=new CHAR[m_nAllocLen=2];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return NULL;
			}
			m_nDataLen=0;
			m_pData[0]='\0';
		}
		return m_pData;
	}
	if ((DWORD)nMinBufLength>=m_nAllocLen)
	{
		if (bStoreData && m_pData!=NULL)
		{
			LPSTR temp=m_pData;
			m_pData=new CHAR[m_nAllocLen=nMinBufLength+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return NULL;
			}
			sMemCopy(m_pData,temp,nMinBufLength);
			m_nDataLen=nMinBufLength;
			m_pData[nMinBufLength]='\0';
			delete[] temp;
		}
		else
		{
			if (m_pData!=NULL)
				delete[] m_pData;
			m_pData=new CHAR[m_nAllocLen=nMinBufLength+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOCATE);
				return NULL;
			}
			m_nDataLen=0;
			m_pData[0]='\0';
		}
	}
	m_nDataLen=nMinBufLength;
	return m_pData;
}

void CString::FreeExtra(int nNewLength)
{
	if (m_pData==NULL)
		return;
	LPSTR temp=m_pData;
	if (nNewLength==-1)
		m_nDataLen=fstrlen(m_pData);
	else
		m_nDataLen=nNewLength;
	m_pData=new CHAR[m_nAllocLen=max(m_nDataLen+1,2)];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	sMemCopy(m_pData,temp,m_nDataLen);
	m_pData[m_nDataLen]='\0';
	delete[] temp;
}

void CString::Compact()
{
	if (m_pData==NULL)
		return;
	if (m_pData[0]=='\0')
	{
		delete[] m_pData;
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
		return;
	}
	if (m_nDataLen<=m_nAllocLen+1)
		return;
	char* pchTemp=new char[m_nAllocLen=m_nDataLen+1];
	sMemCopy(pchTemp,m_pData,m_nDataLen+1);
	delete [] m_pData;
	m_pData=pchTemp;
}
	
BOOL CString::InsChar(DWORD idx,CHAR ch)
{
	if (m_nDataLen+1>=m_nAllocLen)
	{
		LPSTR temp=m_pData;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return FALSE;
		}
		sMemCopy(m_pData,temp,idx);
		m_pData[idx]=ch;
		sMemCopy(&m_pData[idx+1],&temp[idx],m_nDataLen-idx);
		m_nDataLen++;
		m_pData[m_nDataLen]='\0';
		delete[] temp;
		return TRUE;
	}
	for (DWORD i=m_nDataLen;i>idx;i--)
		m_pData[i]=m_pData[i-1];
	m_pData[idx]=ch;
	m_nDataLen++;
	m_pData[m_nDataLen]='\0';
	return TRUE;
}

/* Deletes character */
	
BOOL CString::DelChar(DWORD idx)
{
	if (idx>=m_nDataLen)
		return FALSE;
	for (DWORD i=idx;i<m_nDataLen;i++)
		m_pData[i]=m_pData[i+1];
	m_nDataLen--;
	return TRUE;
}

void CString::ReplaceChars(char from,char to)
{
	for (DWORD i=0;i<m_nDataLen;i++)
	{
		if (m_pData[i]==from)
			m_pData[i]=to;
	}
}


BOOL CString::Delete(DWORD idx,int nCount)
{
	if (idx>=m_nDataLen)
		return FALSE;
	if (nCount<1)
		return FALSE;
	for (DWORD i=idx;i<=m_nDataLen-nCount;i++)
		m_pData[i]=m_pData[i+nCount];
	m_nDataLen-=nCount;
	return TRUE;
}

void CString::Trim()
{
	if (m_nDataLen==0)
		return;
	
	if (m_pData[0]!=' ')
	{
		for (;m_pData[m_nDataLen-1]==' ';m_nDataLen--);
		m_pData[m_nDataLen]='\0';
		return;
	}

	DWORD nStart=0;
	for (;m_pData[nStart]==' ';nStart++);
	if (m_pData[nStart]==' ')
	{
		delete[] m_pData;
		m_pData=NULL;
		m_nDataLen=m_nAllocLen=0;
		return;
	}
	for (;m_pData[m_nDataLen-1]==' ';m_nDataLen--);
	
	
	char* pszTemp=new char[m_nAllocLen=m_nDataLen-nStart+1];
	sMemCopy(pszTemp,m_pData+nStart,m_nDataLen-nStart);
	delete[] m_pData;
	m_pData=pszTemp;
	m_nDataLen-=nStart;
	m_pData[m_nDataLen]='\0';
}
	
void CString::Swap(CString& str)
{
	LPSTR temp;
	DWORD templen;
	BYTE tempbase;
	
	temp=m_pData;
	m_pData=str.m_pData;
	str.m_pData=temp;
	
	templen=m_nDataLen;
	m_nDataLen=str.m_nDataLen;
	str.m_nDataLen=templen;
	
	templen=m_nAllocLen;
	m_nAllocLen=str.m_nAllocLen;
	str.m_nAllocLen=templen;

	tempbase=m_nBase;
	m_nBase=str.m_nBase;
	m_nBase=tempbase;
}

void CString::FormatV(LPCSTR lpszFormat,va_list argList)
{
	int nBufferSize=1024;
	LPSTR temp;
	LPSTR end;
	for (;;)
	{
		temp=new char[nBufferSize];
		HRESULT hRet=StringCbVPrintfEx(temp,nBufferSize,&end,NULL,STRSAFE_IGNORE_NULLS,lpszFormat,argList);
        
		if (hRet==S_OK)
			break;
		if (hRet!=STRSAFE_E_INSUFFICIENT_BUFFER)
			return;

		delete[] temp;
		nBufferSize*=2;
	}

	m_nDataLen=DWORD(end)-DWORD(temp);
    if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	}
	sMemCopy(m_pData,temp,m_nDataLen+1);
	
	delete[] temp;
}
	

void CString::Format(LPCSTR lpszFormat,...)
{
	va_list argList;
	va_start(argList,lpszFormat);
	
	FormatV(lpszFormat,argList);
	va_end(argList);
}

void CString::FormatEx(LPCSTR lpszFormat,...)
{
	va_list argList;
	va_start(argList,lpszFormat);
	
	LPSTR temp=new CHAR[STR_LOADSTRINGBUFLEN];
	
	m_nDataLen=vsprintfex(temp,STR_LOADSTRINGBUFLEN,lpszFormat,argList);
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	}
	sMemCopy(m_pData,temp,m_nDataLen+1);
	
	delete[] temp;
	va_end(argList);
}



#ifdef DEF_RESOURCES

void CString::Format(UINT nFormatID,...)
{
	va_list argList;
	va_start(argList,nFormatID);
	
	LPSTR lpszFormat=new CHAR[STR_LOADSTRINGBUFLEN];
	if (lpszFormat==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadString(nFormatID,lpszFormat,STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete lpszFormat;
			lpszFormat=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadString(nFormatID,lpszFormat,i*STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
		}
	}
	
	FormatV(lpszFormat,argList);
	delete[] lpszFormat;
	
	va_end(argList);
}

void CString::FormatEx(UINT nFormatID,...)
{
	va_list argList;
	va_start(argList,nFormatID);
	
	LPSTR temp=new CHAR[STR_LOADSTRINGBUFLEN];
	LPSTR lpszFormat=new CHAR[STR_LOADSTRINGBUFLEN];
	if (temp==NULL || lpszFormat==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadString(nFormatID,lpszFormat,STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete lpszFormat;
			lpszFormat=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadString(nFormatID,lpszFormat,i*STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
		}
	}
	
	m_nDataLen=vsprintfex(temp,STR_LOADSTRINGBUFLEN,lpszFormat,argList);
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
	}
	sMemCopy(m_pData,temp,m_nDataLen+1);
	
	delete[] lpszFormat;
	delete[] temp;
	va_end(argList);
}

void CString::FormatC(UINT nFormatID,...)
{
	va_list argList;
	va_start(argList,nFormatID);
	
	char temp[STR_LOADSTRINGBUFLEN];
	LPSTR lpszFormat=new CHAR[STR_LOADSTRINGBUFLEN];
	if (temp==NULL || lpszFormat==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadString(nFormatID,lpszFormat,STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete lpszFormat;
			lpszFormat=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadString(nFormatID,lpszFormat,i*STR_LOADSTRINGBUFLEN,CommonResource);
		}
	}
	
	FormatV(lpszFormat,argList);
	delete[] lpszFormat;
	
	va_end(argList);
}


void CString::FormatExC(UINT nFormatID,...)
{
	va_list argList;
	va_start(argList,nFormatID);
	
	LPSTR temp=new CHAR[STR_LOADSTRINGBUFLEN];
	LPSTR lpszFormat=new CHAR[STR_LOADSTRINGBUFLEN];
	if (temp==NULL || lpszFormat==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadString(nFormatID,lpszFormat,STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete lpszFormat;
			lpszFormat=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadString(nFormatID,lpszFormat,i*STR_LOADSTRINGBUFLEN,CommonResource);
		}
	}
	
	m_nDataLen=vsprintfex(temp,STR_LOADSTRINGBUFLEN,lpszFormat,argList);
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
	}
	sMemCopy(m_pData,temp,m_nDataLen+1);
	
	delete[] lpszFormat;
	delete[] temp;
	va_end(argList);
}

CString::CString(int nID)
:	m_nBase(10)
{
	LPSTR szBuffer;
	szBuffer=new CHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadStringA(GetLanguageSpecificResourceHandle(),nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringA(GetLanguageSpecificResourceHandle(),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;;
	}
	sMemCopy(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
}


CString::CString(int nID,TypeOfResourceHandle bType)
:	m_nBase(10)
{
	LPSTR szBuffer;
	szBuffer=new CHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadStringA(GetResourceHandle(bType),nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringA(GetResourceHandle(bType),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;;
	}
	sMemCopy(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
}


CString::CString(UINT nID)
:	m_nBase(10)
{
	LPSTR szBuffer;
	szBuffer=new CHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadStringA(GetLanguageSpecificResourceHandle(),nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringA(GetLanguageSpecificResourceHandle(),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;;
	}
	sMemCopy(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
}

CString::CString(UINT nID,TypeOfResourceHandle bType)
:	m_nBase(10)
{
	LPSTR szBuffer;
	szBuffer=new CHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadStringA(GetResourceHandle(bType),nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringA(GetResourceHandle(bType),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;;
	}
	sMemCopy(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
}

CString::CString(int nID,BOOL bLoadAsUnicode)
:	m_nBase(10)
{
	if (bLoadAsUnicode && IsFullUnicodeSupport())
	{
		LPWSTR szBuffer;
		szBuffer=new WCHAR[STR_LOADSTRINGBUFLEN];
		if (szBuffer==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		m_nDataLen=::LoadStringW(GetLanguageSpecificResourceHandle(),nID,szBuffer,STR_LOADSTRINGBUFLEN);
		if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
		{
			for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
			{
				delete szBuffer;
				szBuffer=new WCHAR[i*STR_LOADSTRINGBUFLEN];
				m_nDataLen=::LoadStringW(GetLanguageSpecificResourceHandle(),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
			}
		}
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;;
		}
		MemCopyWtoA(m_pData,szBuffer,m_nDataLen+1);
		delete[] szBuffer;
		return;
	}
	
	LPSTR szBuffer;
	szBuffer=new CHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadStringA(GetLanguageSpecificResourceHandle(),nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringA(GetLanguageSpecificResourceHandle(),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;;
	}
	sMemCopy(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
}

CString::CString(UINT nID,BOOL bLoadAsUnicode)
:	m_nBase(10)
{
	if (bLoadAsUnicode && IsFullUnicodeSupport())
	{
		LPWSTR szBuffer;
		szBuffer=new WCHAR[STR_LOADSTRINGBUFLEN];
		if (szBuffer==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		m_nDataLen=::LoadStringW(GetLanguageSpecificResourceHandle(),nID,szBuffer,STR_LOADSTRINGBUFLEN);
		if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
		{
			for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
			{
				delete szBuffer;
				szBuffer=new WCHAR[i*STR_LOADSTRINGBUFLEN];
				m_nDataLen=::LoadStringW(GetLanguageSpecificResourceHandle(),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
			}
		}
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;;
		}
		MemCopyWtoA(m_pData,szBuffer,m_nDataLen+1);
		delete[] szBuffer;
		return;
	}
	
	LPSTR szBuffer;
	szBuffer=new CHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadStringA(GetLanguageSpecificResourceHandle(),nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringA(GetLanguageSpecificResourceHandle(),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;;
	}
	sMemCopy(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
}

BOOL CString::LoadString(UINT nID)
{
	LPSTR szBuffer;
	szBuffer=new CHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return FALSE;
	}
	m_nDataLen=::LoadStringA(GetLanguageSpecificResourceHandle(),nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringA(GetLanguageSpecificResourceHandle(),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return FALSE;
		}
	}
	sMemCopy(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
	return TRUE;
}
BOOL CString::LoadString(UINT nID,TypeOfResourceHandle bType)
{
	LPSTR szBuffer;
	szBuffer=new CHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return FALSE;
	}
	m_nDataLen=::LoadStringA(GetResourceHandle(bType),nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new CHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringA(GetResourceHandle(bType),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new CHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return FALSE;
		}
	}
	sMemCopy(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
	return TRUE;
}

#endif

#ifdef WIN32
void CString::AnsiToOem()
{
	if (m_pData==NULL)
		return;
	CString help(m_pData);
	::CharToOemA(help,m_pData);
}

void CString::OemToAnsi()
{
	if (m_pData==NULL)
		return;
	CString help(m_pData);
	::OemToCharA(help,m_pData);
}
#endif


/////////////////////////
// CStringW

#ifdef DEF_WCHAR

CStringW::CStringW(const CStringW& str)
:	m_nBase(10)
{
	if (&str!=this && str.m_pData!=NULL)
	{
		m_nDataLen=str.m_nDataLen;
		m_nBase=str.m_nBase;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		sMemCopyW(m_pData,str.m_pData,m_nDataLen+1);
	}
	else
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
		m_nBase=10;
	}
}

CStringW::CStringW(WCHAR ch,DWORD nRepeat)
	:m_nBase(10)
{
	if (!nRepeat)
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
	else
	{
		m_nDataLen=nRepeat;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		for (m_nDataLen=0;m_nDataLen<nRepeat;m_nDataLen++)
			m_pData[m_nDataLen]=ch;
		m_pData[m_nDataLen]='\0';
	}
}

CStringW::CStringW(LPCWSTR lpsz)
	:m_nBase(10)
{
	if (lpsz!=NULL)
	{
		//m_nDataLen=wcslen(lpsz);
		sstrlenW(lpsz,m_nDataLen);
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		sMemCopyW(m_pData,lpsz,m_nDataLen+1);
	}
	else
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
}

CStringW::CStringW(LPCWSTR lpsz,DWORD nLength)
	:m_nBase(10)
{
	if (lpsz==NULL || nLength<=0)
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
	else
	{
		//m_nDataLen=wcslen(lpsz);
		sstrlenW(lpsz,m_nDataLen);

		if (m_nDataLen>nLength)
			m_nDataLen=nLength;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		sMemCopyW(m_pData,lpsz,m_nDataLen);
		m_pData[m_nDataLen]='\0';
	}
}

CStringW::CStringW(const short * lpsz)
:	m_nBase(10)
{
	if (lpsz==NULL)
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
	else
	{
		//m_nDataLen=wcslen((LPWSTR)lpsz);
		sstrlenW(lpsz,m_nDataLen);

		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		sMemCopyW(m_pData,lpsz,m_nDataLen+1);
	}
}

CStringW::CStringW(const CString& str)
:	m_nBase(10)
{
	if (str.m_pData!=NULL)
	{
		m_nDataLen=str.m_nDataLen;
		m_nBase=str.m_nBase;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		MemCopyAtoW(m_pData,str.m_pData,m_nDataLen);
		m_pData[m_nDataLen]='\0';
	}
	else
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
		m_nBase=10;
	}
}

CStringW::CStringW(LPCSTR lpsz)
:	m_nBase(10)
{
	if (lpsz!=NULL)
	{
		//m_nDataLen=strlen(lpsz);
		sstrlen(lpsz,m_nDataLen);

		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		MemCopyAtoW(m_pData,lpsz,m_nDataLen);
		m_pData[m_nDataLen]='\0';
	}
	else
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
}

CStringW::CStringW(CHAR ch,DWORD nRepeat)
:	m_nBase(10)
{
	if (!nRepeat)
	{
		m_pData=NULL;
		m_nDataLen=0;
		m_nAllocLen=0;
	}
	else
	{
		m_nDataLen=nRepeat;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		WCHAR wch;
		MemCopyAtoW(&wch,&ch,1);
		for (m_nDataLen=0;m_nDataLen<nRepeat;m_nDataLen++)
			m_pData[m_nDataLen]=wch;
		m_pData[m_nDataLen]='\0';
	}
}

WCHAR& CStringW::operator[](DWORD nIndex)
{
	if (m_pData==NULL)
		return (WCHAR)*szwEmpty;
	if (nIndex<m_nAllocLen)
		return m_pData[nIndex];
	else
		return m_pData[m_nAllocLen-1];
}

WCHAR& CStringW::operator[](int nIndex)
{
	if (m_pData==NULL)
		return (WCHAR)*szwEmpty;
	if (nIndex>=0 && (DWORD)nIndex<m_nAllocLen)
		return m_pData[nIndex];
	else
		return m_pData[m_nAllocLen-1];
}

WCHAR& CStringW::operator[](UINT nIndex)
{
	if (m_pData==NULL)
		return (WCHAR)*szwEmpty;
	if (nIndex>=0 && (DWORD)nIndex<m_nAllocLen)
		return m_pData[nIndex];
	else
		return m_pData[m_nAllocLen-1];
}
	
CStringW& CStringW::Copy(LPCWSTR str)
{
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	sstrlenW(str,m_nDataLen);
	m_pData=new WCHAR[m_nAllocLen=m_nDataLen+1];
	sMemCopyW(m_pData,str,m_nDataLen+1);
	
	return *this;
}

CStringW& CStringW::Copy(LPCWSTR str,int iLength)
{
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	if (iLength<0)
		sstrlenW(str,m_nDataLen);
	else
		m_nDataLen=iLength;
	m_pData=new WCHAR[m_nAllocLen=m_nDataLen+1];
	sMemCopyW(m_pData,str,m_nDataLen);
	m_pData[m_nDataLen]='\0';
	return *this;
}

CStringW& CStringW::Copy(LPCSTR str)
{
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	for (m_nDataLen=0;str[m_nDataLen]!='\0';m_nDataLen++);
	m_pData=new WCHAR[m_nAllocLen=m_nDataLen+1];
	MemCopyAtoW(m_pData,str,m_nDataLen+1);
	return *this;
}

CStringW& CStringW::Copy(LPCSTR str,int iLength)
{
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	if (iLength<0)
		sstrlen(str,m_nDataLen);
	else
		m_nDataLen=iLength;
	m_pData=new WCHAR[m_nAllocLen=m_nDataLen+1];
	MemCopyAtoW(m_pData,str,m_nDataLen+1);
	return *this;
}

CStringW& CStringW::Copy(const BYTE* str) 
{ 
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	for (m_nDataLen=0;str[m_nDataLen]!='\0';m_nDataLen++);
	m_pData=new WCHAR[m_nAllocLen=m_nDataLen+1];
	MemCopyAtoW(m_pData,str,m_nDataLen);
	m_pData[m_nDataLen]=L'\0';
	return *this;
}

CStringW& CStringW::Copy(const BYTE* str,int iLength)
{
	if (str==NULL)
	{
		Empty();
		return *this;
	}
	if (m_pData!=NULL)
		delete[] m_pData;
	if (iLength<0)
		sstrlen(str,m_nDataLen);
	else
		m_nDataLen=iLength;
	m_pData=new WCHAR[m_nAllocLen=m_nDataLen+1];
	
	MemCopyAtoW(m_pData,str,m_nDataLen);
	m_pData[m_nDataLen]=L'\0';
	return *this;
}
	
const CStringW& CStringW::operator=(const CStringW& str)
{
	if (str.m_nDataLen==0)
	{
		if (m_pData!=NULL)
		{
			delete[] m_pData;
			m_pData=NULL;
			m_nDataLen=0;
			m_nAllocLen=0;
		}
		return *this;
	}
	return Copy(str,str.GetLength());
}

const CStringW& CStringW::operator=(WCHAR ch)
{
	if (m_nAllocLen<2 || m_nAllocLen>10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new WCHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
	}
	m_pData[0]=ch;
	m_pData[1]='\0';
	m_nDataLen=1;
	return *this;
}

const CStringW& CStringW::operator=(DWORD iNum)
{
	WCHAR szBuffer[34];
	_itow(iNum,szBuffer,m_nBase);
	
	//m_nDataLen=wcslen(szBuffer);'
	sstrlenW(szBuffer,m_nDataLen);
	
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
	}
	sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
	return *this;
}

const CStringW& CStringW::operator=(int iNum)
{
	WCHAR szBuffer[34];
	_itow(iNum,szBuffer,m_nBase);
	
	//m_nDataLen=wcslen(szBuffer);
	sstrlenW(szBuffer,m_nAllocLen);
	
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
	}
	sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
	return *this;
}

const CStringW& CStringW::operator=(const CString& str)
{
	if (str.m_nDataLen==0)
	{
		if (m_pData!=NULL)
		{
			delete[] m_pData;
			m_pData=NULL;
			m_nDataLen=0;
			m_nAllocLen=0;
		}
		return *this;
	}
	return Copy(str,str.GetLength());
}

const CStringW& CStringW::operator=(CHAR ch)
{
	if (m_nAllocLen<2 || m_nAllocLen>10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new WCHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
	}
	MemCopyAtoW(m_pData,&ch,2);
	m_pData[1]='\0';
	m_nDataLen=1;
	return *this;
}

const CStringW& CStringW::operator+=(const CStringW& str)
{
	if (str.m_nDataLen==0 || &str==this)
		return *this;	
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+str.m_nDataLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopyW(&m_pData[m_nDataLen],str.m_pData,str.m_nDataLen+1);
		m_nDataLen+=str.m_nDataLen;
		return *this;	
	}
	else
	{
		m_nDataLen=str.m_nDataLen;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		sMemCopyW(m_pData,str.m_pData,m_nDataLen+1);
		return *this;
	}
}

const CStringW& CStringW::operator+=(WCHAR ch)
{
	if (m_pData!=NULL)
	{
		if (m_nDataLen+1>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		m_pData[m_nDataLen++]=ch;
		m_pData[m_nDataLen]='\0';
		return *this;	
	}
	else
	{
		m_nDataLen=1;
		m_pData=new WCHAR[m_nAllocLen=STR_EXTRAALLOC];
		m_pData[0]=ch;
		m_pData[1]='\0';
		return *this;
	}
}


void CStringW::Append(LPCWSTR str,int nStrLen)
{
	if (str==NULL)
		return;	
	if (nStrLen==-1)
		sstrlenW(str,nStrLen);

	if (nStrLen==0)
		return;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopyW(&m_pData[m_nDataLen],str,nStrLen);
		m_nDataLen+=nStrLen;
		m_pData[m_nDataLen]=L'\0';
		
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		sMemCopyW(m_pData,str,m_nDataLen);
		m_pData[m_nDataLen]=L'\0';
	}
}


const CStringW& CStringW::operator+=(LPCWSTR str)
{
	if (str==NULL)
		return *this;	
	DWORD nStrLen; //=wcslen(str);	
	sstrlenW(str,nStrLen);

	if (nStrLen==0)
		return *this;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopyW(&m_pData[m_nDataLen],str,nStrLen+1);
		m_nDataLen+=nStrLen;
		return *this;	
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopyW(m_pData,str,m_nDataLen+1);
		return *this;
	}
}

const CStringW& CStringW::operator+=(DWORD iNum)
{
	WCHAR szBuffer[34];
	_itow(iNum,szBuffer,m_nBase);
	if (m_pData!=NULL)
	{
		DWORD nStrLen;	
		sstrlenW(szBuffer,nStrLen);

		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopyW(&m_pData[m_nDataLen],szBuffer,nStrLen+1);
		m_nDataLen+=nStrLen;
		return *this;	
	}
	else
	{
		//m_nDataLen=strlen((LPSTR)szBuffer); // should that work anyway
		sstrlenW(szBuffer,m_nDataLen);

		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
		return *this;
	}
}

const CStringW& CStringW::operator+=(int iNum)
{
	WCHAR szBuffer[34];
	_itow(iNum,szBuffer,m_nBase);
	if (m_pData!=NULL)
	{
		DWORD nStrLen;//=wcslen(szBuffer);	
		sstrlenW(szBuffer,nStrLen);

		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopyW(&m_pData[m_nDataLen],szBuffer,nStrLen+1);
		m_nDataLen+=nStrLen;
		return *this;	
	}
	else
	{
		//m_nDataLen=strlen((LPSTR)szBuffer);
		sstrlenW(szBuffer,m_nDataLen);

		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
		return *this;
	}
}

const CStringW& CStringW::operator+=(const CString& str)
{
	if (str.m_nDataLen==0)
		return *this;	
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+str.m_nDataLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyAtoW(&m_pData[m_nDataLen],str.m_pData,str.m_nDataLen);
		m_nDataLen+=str.m_nDataLen;
		m_pData[m_nDataLen]='\0';
		return *this;	
	}
	else
	{
		m_nDataLen=str.m_nDataLen;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyAtoW(m_pData,str.m_pData,m_nDataLen);
		m_pData[m_nDataLen]='\0';
		return *this;
	}
}

const CStringW& CStringW::operator+=(CHAR ch)
{
	if (m_pData!=NULL)
	{
		if (m_nDataLen+1>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyAtoW(m_pData+m_nDataLen,&ch,1);
		m_pData[++m_nDataLen]='\0';
		return *this;	
	}
	else
	{
		m_nDataLen=1;
		m_pData=new WCHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyAtoW(m_pData,&ch,1);
		m_pData[1]='\0';
		return *this;
	}
}

const CStringW& CStringW::operator+=(LPCSTR str)
{
	if (str==NULL)
		return *this;	
	DWORD nStrLen;	
	sstrlen(str,nStrLen);

	if (nStrLen==0)
		return *this;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyAtoW(&m_pData[m_nDataLen],str,nStrLen);
		m_nDataLen+=nStrLen;
		m_pData[m_nDataLen]='\0';
		return *this;	
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyAtoW(m_pData,str,m_nDataLen);
		m_pData[m_nDataLen]='\0';
		return *this;
	}
}

void CStringW::Append(LPCSTR str,int nStrLen)
{
	if (str==NULL)
		return;	
	if (nStrLen!=-1)
		sstrlen(str,nStrLen);

	if (nStrLen==0)
		return;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyAtoW(&m_pData[m_nDataLen],str,nStrLen);
		m_nDataLen+=nStrLen;
		m_pData[m_nDataLen]='\0';
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
		MemCopyAtoW(m_pData,str,m_nDataLen);
		m_pData[m_nDataLen]='\0';
	}
}

CStringW& CStringW::operator<<(const CStringW& str)
{
	if (str.m_nDataLen==0 || &str==this)
		return *this;	
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+str.m_nDataLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopyW(&m_pData[m_nDataLen],str.m_pData,str.m_nDataLen+1);
		m_nDataLen+=str.m_nDataLen;
		return *this;	
	}
	else
	{
		m_nDataLen=str.m_nDataLen;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopyW(m_pData,str.m_pData,m_nDataLen+1);
		return *this;
	}
}

CStringW& CStringW::operator<<(WCHAR ch)
{
	if (m_pData!=NULL)
	{
		if (m_nDataLen+1>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		m_pData[m_nDataLen++]=ch;
		m_pData[m_nDataLen]='\0';
		return *this;	
	}
	else
	{
		m_nDataLen=1;
		m_pData=new WCHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		m_pData[0]=ch;
		m_pData[1]='\0';
		return *this;
	}
}

CStringW& CStringW::operator<<(LPCWSTR str)
{
	if (str==NULL)
		return *this;	
	DWORD nStrLen;	
	sstrlenW(str,nStrLen);

	if (nStrLen==0)
		return *this;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopyW(&m_pData[m_nDataLen],str,nStrLen+1);
		m_nDataLen+=nStrLen;
		return *this;	
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopyW(m_pData,str,m_nDataLen+1);
		return *this;
	}
}

CStringW& CStringW::operator<<(DWORD iNum)
{
	WCHAR szBuffer[34];
	_itow(iNum,szBuffer,m_nBase);
	if (m_pData!=NULL)
	{
		DWORD nStrLen;
		sstrlenW(szBuffer,nStrLen);

		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopyW(&m_pData[m_nDataLen],szBuffer,nStrLen+1);
		m_nDataLen+=nStrLen;
		return *this;	
	}
	else
	{
		//m_nDataLen=strlen((LPSTR)szBuffer);
		sstrlenW(szBuffer,m_nDataLen);

		m_pData=new WCHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
		return *this;
	}
}

CStringW& CStringW::operator<<(int iNum)
{
	WCHAR szBuffer[34];
	_itow(iNum,szBuffer,m_nBase);
	if (m_pData!=NULL)
	{
		DWORD nStrLen;
		sstrlenW(szBuffer,nStrLen);

		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		sMemCopyW(&m_pData[m_nDataLen],szBuffer,nStrLen+1);
		m_nDataLen+=nStrLen;
		return *this;	
	}
	else
	{
		//m_nDataLen=strlen((LPSTR)szBuffer);
		sstrlenW(szBuffer,m_nDataLen);

		m_pData=new WCHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
		return *this;
	}
}

CStringW& CStringW::operator<<(const CString& str)
{
	if (str.m_nDataLen==0)
		return *this;	
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+str.m_nDataLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyAtoW(&m_pData[m_nDataLen],str.m_pData,str.m_nDataLen);
		m_nDataLen+=str.m_nDataLen;
		m_pData[m_nDataLen]='\0';
		return *this;	
	}
	else
	{
		m_nDataLen=str.m_nDataLen;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyAtoW(m_pData,str.m_pData,m_nDataLen);
		m_pData[m_nDataLen]='\0';
		return *this;
	}
}

CStringW& CStringW::operator<<(CHAR ch)
{
	if (m_pData!=NULL)
	{
		if (m_nDataLen+1>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyAtoW(m_pData+m_nDataLen,&ch,1);
		m_pData[++m_nDataLen]='\0';
		return *this;	
	}
	else
	{
		m_nDataLen=1;
		m_pData=new WCHAR[m_nAllocLen=STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyAtoW(m_pData,&ch,1);
		m_pData[1]='\0';
		return *this;
	}
}

CStringW& CStringW::operator<<(LPCSTR str)
{
	if (str==NULL)
		return *this;	
	DWORD nStrLen;	
	sstrlen(str,nStrLen);

	if (nStrLen==0)
		return *this;
	if (m_pData!=NULL)
	{
		DWORD templen=m_nDataLen+nStrLen;
		if (templen>=m_nAllocLen)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=templen+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return *this;
			}
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
		MemCopyAtoW(&m_pData[m_nDataLen],str,nStrLen);
		m_nDataLen+=nStrLen;
		m_pData[m_nDataLen]='\0';
		return *this;	
	}
	else
	{
		m_nDataLen=nStrLen;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return *this;
		}
		MemCopyAtoW(m_pData,str,m_nDataLen);
		m_pData[m_nDataLen]='\0';
		return *this;
	}
}

int CStringW::Compare(LPCWSTR lpsz) const
{
	if (m_pData==NULL)
	{
		if (lpsz==NULL)
			return 0;
		else
		{
			if (lpsz[0]=='\0')
				return 0;
			else
				return -1;
		}
	}
	if (lpsz==NULL)
		return 1;
	return wcscmp(m_pData,lpsz);
}

int CStringW::CompareNoCase(LPCWSTR lpsz) const
{
	if (m_pData==NULL)
	{
		if (lpsz==NULL)
			return 0;
		else
		{
			if (lpsz[0]=='\0')
				return 0;
			else
				return -1;
		}
	}
	if (lpsz==NULL)
		return 1;
	WCHAR *tmp1,*tmp2;
	int ret;
	tmp1=new WCHAR[m_nDataLen+2];
	
	sstrlenW(lpsz,ret);
	tmp2=new WCHAR[ret+2];
	if (tmp1==NULL || tmp2==NULL)
	{	
		SetHFCError(HFC_CANNOTALLOCATE);
		return -1;
	}
	sMemCopyW(tmp1,m_pData,m_nDataLen+1);
	sMemCopyW(tmp2,lpsz,ret+1);
	::CharLowerW(tmp1);
	::CharLowerW(tmp2);
	ret=wcscmp(tmp1,tmp2);
	delete[] tmp1;
	delete[] tmp2;
	return ret;
}

int CStringW::Compare(LPCSTR lpsz) const
{
	if (m_pData==NULL)
	{
		if (lpsz==NULL)
			return 0;
		else
		{
			if (lpsz[0]=='\0')
				return 0;
			else
				return -1;
		}
	}
	if (lpsz==NULL)
		return 1;
	CHAR* lpasz=new CHAR[m_nDataLen+2];
	if (lpasz==NULL)
	{	
		SetHFCError(HFC_CANNOTALLOCATE);
		return -1;
	}
	MemCopyWtoA(lpasz,m_pData,m_nDataLen+1);
	int ret=strcmp(lpasz,lpsz);
	delete[] lpasz;
	return ret;
}

int CStringW::CompareNoCase(LPCSTR lpsz) const
{
	if (m_pData==NULL)
	{
		if (lpsz==NULL)
			return 0;
		else
		{
			if (lpsz[0]=='\0')
				return 0;
			else
				return -1;
		}
	}
	if (lpsz==NULL)
		return 1;
	CHAR *tmp1,*tmp2;
	int ret;
	tmp1=new CHAR[m_nDataLen+2];

	sstrlen(lpsz,ret);
	tmp2=new CHAR[ret+2];
	
	if (tmp1==NULL || tmp2==NULL)
	{	
		SetHFCError(HFC_CANNOTALLOCATE);
		return -1;
	}
	MemCopyWtoA(tmp1,m_pData,m_nDataLen+1);
	sMemCopy(tmp2,lpsz,ret+1);
	::CharLowerA(tmp1);
	::CharLowerA(tmp2);
	ret=strcmp(tmp1,tmp2);
	delete[] tmp1;
	delete[] tmp2;
	return ret;
}

BOOL CStringW::operator==(const CStringW& str)
{
	if (m_pData==NULL)
	{
		if (str.m_nDataLen==0)
			return TRUE;
		else
			return FALSE;
	}
	if (str.m_pData==NULL)
		return FALSE;
	if (wcscmp(m_pData,str.m_pData)==0)
		return TRUE;
	return FALSE;
}

BOOL CStringW::operator==(const CString& str)
{
	if (m_pData==NULL)
	{
		if (str.m_nDataLen==0)
			return TRUE;
		else
			return FALSE;
	}
	if (str.m_pData==NULL)
		return FALSE;
	if (m_nDataLen!=str.m_nDataLen)
		return FALSE;
	CHAR* lpasz=new CHAR[m_nDataLen+2];
	if (lpasz==NULL)
	{	
		SetHFCError(HFC_CANNOTALLOCATE);
		return FALSE;
	}
	MemCopyWtoA(lpasz,m_pData,m_nDataLen+1);
	if (strcmp(lpasz,str.m_pData)==0)
	{
		delete[] lpasz;
		return TRUE;
	}
	delete[] lpasz;
	return FALSE;
}

BYTE CStringW::ContainString(LPCWSTR str,DWORD start)
{
	DWORD i,j=0;
	if (m_pData==NULL)
	{
		if (str==NULL)
			return TRUE;
		else
		{
			if (str[0]=='\0')
				return TRUE;
			else
				return FALSE;
		}
	}
	for (i=start;i<m_nDataLen;i++)
	{
		j=0;
		while ((m_pData[i+j]==str[j] || str[j]=='?') && m_pData[i+j]!='\0' ) 
			j++;
		if (str[j]=='\0')
			return 1;
		if (str[j]=='*')
			return ContainString(str+j+1,j+i+1);
	}
	return 0;
}

void CStringW::MakeReverse()
{
	if (m_pData==NULL)
		return;
	WCHAR* pchNewData=new WCHAR[m_nDataLen];
	for (UINT i=0;i<m_nDataLen;i++)
		pchNewData[i]=m_pData[m_nDataLen-i-1];
	pchNewData[m_nDataLen]='\0';
	delete[] m_pData;
	m_pData=pchNewData;
}

int CStringW::Find(WCHAR ch) const
{
	if (m_pData==NULL)
		return -1;
	DWORD i;
	for (i=0;i<m_nDataLen;i++)
	{
		if (m_pData[i]==ch)
			return i;
	}
	return -1;
}

int CStringW::FindFirst(WCHAR ch) const
{
	if (m_pData==NULL)
		return -1;
	DWORD i;
	for (i=0;i<m_nDataLen;i++)
	{
		if (m_pData[i]==ch)
			return i;
	}
	return -1;
}

int CStringW::FindLast(WCHAR ch) const
{
	if (m_pData==NULL)
		return -1;
	int i;
	for (i=m_nDataLen-1;i>=0;i--)
	{
		if (m_pData[i]==ch)
			return i;
	}
	return -1;
}

int CStringW::FindNext(WCHAR ch,int idx) const
{
	if (m_pData==NULL)
		return -1;
	DWORD i;
	for (i=idx+1;i<m_nDataLen;i++)
	{
		if (m_pData[i]==ch)
			return i;	
	}
   	return -1;
}	

int CStringW::FindOneOf(LPCWSTR lpszCharSet) const
{
	if (m_pData==NULL)
		return -1;
	
	for (DWORD i=0;i<m_nDataLen;i++)
	{
		for (DWORD j=0;lpszCharSet[j]!='\0';j++)
		{
			if (m_pData[i]==lpszCharSet[j])
				return i;
		}
	}
	return -1;
}

int CStringW::Find(LPCWSTR lpszSub) const
{
	if (m_pData==NULL)
		return -1;
	if (wcsstr(m_pData,lpszSub)!=NULL)
		return TRUE;
	else
		return FALSE;
}

LPWSTR CStringW::GetBuffer(int nMinBufLength,BYTE bStoreData)
{
	if (nMinBufLength==-1)
	{
		if (m_pData==NULL)
		{
			m_pData=new WCHAR[m_nAllocLen=2];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return NULL;
			}
			m_nDataLen=0;
			m_pData[0]='\0';
		}
		return m_pData;
	}
	if ((DWORD)nMinBufLength>m_nAllocLen)
	{
		if (bStoreData && m_pData!=NULL)
		{
			LPWSTR temp=m_pData;
			m_pData=new WCHAR[m_nAllocLen=nMinBufLength+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return NULL;
			}
			sMemCopyW(m_pData,temp,nMinBufLength);
			m_nDataLen=nMinBufLength;
			m_pData[nMinBufLength]='\0';
			delete[] temp;
		}
		else
		{
			if (m_pData!=NULL)
				delete[] m_pData;
			m_pData=new WCHAR[m_nAllocLen=nMinBufLength+STR_EXTRAALLOC];
			if (m_pData==NULL)
			{	
				SetHFCError(HFC_CANNOTALLOCATE);
				return NULL;
			}
			m_nDataLen=0;
			m_pData[0]='\0';
		}
	}
	m_nDataLen=nMinBufLength;
	return m_pData;
}

void CStringW::Compact()
{
	if (m_pData==NULL)
		return;
	if (m_nDataLen<=m_nAllocLen+1)
		return;
	WCHAR* pchTemp=new WCHAR[m_nAllocLen=m_nDataLen+1];
	sMemCopyW(pchTemp,m_pData,m_nDataLen+1);
	delete[] m_pData;
	m_pData=pchTemp;
}
	
void CStringW::FreeExtra(int nNewLength)
{
	if (m_pData==NULL)
		return;
	LPWSTR temp=m_pData;
	if (nNewLength==-1)
		m_nDataLen=fwstrlen(m_pData);
	else
		m_nDataLen=nNewLength;
	m_pData=new WCHAR[m_nAllocLen=max(m_nDataLen+1,2)];
	if (m_pData==NULL)
	{	
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	sMemCopyW(m_pData,temp,m_nDataLen);
	m_pData[m_nDataLen]='\0';
	delete[] temp;
}

BOOL CStringW::InsChar(DWORD idx,WCHAR ch)
{
	if (m_nDataLen+1>=m_nAllocLen)
	{
		LPWSTR temp=m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return FALSE;
		}
		sMemCopyW(m_pData,temp,idx);
		m_pData[idx]=ch;
		sMemCopyW(&m_pData[idx+1],&temp[idx],m_nDataLen-idx);
		m_nDataLen++;
		m_pData[m_nDataLen]='\0';
		return TRUE;
	}
	for (DWORD i=m_nDataLen;i>idx;i--)
		m_pData[i]=m_pData[i-1];
	m_pData[idx]=ch;
	m_nDataLen++;
	m_pData[m_nDataLen]='\0';
	return TRUE;
}
	
BOOL CStringW::DelChar(DWORD idx)
{
	if (idx>=m_nDataLen)
		return FALSE;
	for (DWORD i=idx;i<m_nDataLen;i++)
		m_pData[i]=m_pData[i+1];
	m_nDataLen--;
	return TRUE;
}

void CStringW::ReplaceChars(char from,char to)
{
	for (DWORD i=0;i<m_nDataLen;i++)
	{
		if (m_pData[i]==from)
			m_pData[i]=to;
	}
}

BOOL CStringW::Delete(DWORD idx,int nCount)
{
	if (idx>=m_nDataLen)
		return FALSE;
	if (nCount<1)
		return FALSE;
	for (DWORD i=idx;i<=m_nDataLen-nCount;i++)
		m_pData[i]=m_pData[i+nCount];
	m_nDataLen-=nCount;
	return TRUE;
}



void CStringW::Trim()
{
	if (m_nDataLen==0)
		return;
	
	if (m_pData[0]!=' ')
	{
		for (;m_pData[m_nDataLen-1]==' ';m_nDataLen--);
		m_pData[m_nDataLen]='\0';
		return;
	}

	for (DWORD nStart=0;m_pData[nStart]==' ';nStart++);
	if (m_pData[nStart]==' ')
	{
		delete[] m_pData;
		m_pData=NULL;
		m_nDataLen=m_nAllocLen=0;
		return;
	}
	for (m_nDataLen;m_pData[m_nDataLen-1]==' ';m_nDataLen--);
	
	
	WCHAR* pszTemp=new WCHAR[m_nAllocLen=m_nDataLen-nStart+1];
	sMemCopyW(pszTemp,m_pData+nStart,m_nDataLen-nStart);
	delete[] m_pData;
	m_pData=pszTemp;
	m_nDataLen-=nStart;
	m_pData[m_nDataLen]='\0';
}
	
void CStringW::Swap(CStringW& str)
{
	LPWSTR temp;
	DWORD templen;
	BYTE tempbase;
	
	temp=m_pData;
	m_pData=str.m_pData;
	str.m_pData=temp;
	
	templen=m_nDataLen;
	m_nDataLen=str.m_nDataLen;
	str.m_nDataLen=templen;
	
	templen=m_nAllocLen;
	m_nAllocLen=str.m_nAllocLen;
	str.m_nAllocLen=templen;

	tempbase=m_nBase;
	m_nBase=str.m_nBase;
	m_nBase=tempbase;
}


void CStringW::FormatV(LPCWSTR lpszFormat,va_list argList)
{
	int nBufferSize=1024;
	LPWSTR temp;
	LPWSTR end;
	for (;;)
	{
		temp=new WCHAR[nBufferSize];
		HRESULT hRet=StringCbVPrintfExW(temp,nBufferSize,&end,NULL,STRSAFE_IGNORE_NULLS,lpszFormat,argList);
        
		if (hRet==S_OK)
			break;
		if (hRet!=STRSAFE_E_INSUFFICIENT_BUFFER)
			return;

		delete[] temp;
		nBufferSize*=2;
	}

	m_nDataLen=(DWORD(end)-DWORD(temp))/sizeof(WCHAR);
    if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	}
	sMemCopyW(m_pData,temp,m_nDataLen+1);
	
	delete[] temp;
}
	

void CStringW::Format(LPCWSTR lpszFormat,...)
{
	va_list argList;
	va_start(argList,lpszFormat);
	
	FormatV(lpszFormat,argList);
	va_end(argList);
}

void CStringW::FormatEx(LPCWSTR lpszFormat,...)
{
	va_list argList;
	va_start(argList,lpszFormat);
	
	LPWSTR temp=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (temp==NULL)
	{	
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}

	m_nDataLen=vswprintfex(temp,STR_LOADSTRINGBUFLEN,lpszFormat,argList);
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData==NULL)
			delete[] m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
	}
	sMemCopyW(m_pData,temp,m_nDataLen+1);
	
	delete[] temp;
	va_end(argList);
}

#ifdef DEF_RESOURCES

void CStringW::Format(UINT nFormatID,...)
{
	va_list argList;
	va_start(argList,nFormatID);
	
	LPWSTR lpszFormat=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (lpszFormat==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadString(nFormatID,lpszFormat,STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete lpszFormat;
			lpszFormat=new WCHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadString(nFormatID,lpszFormat,i*STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
		}
	}
	
	FormatV(lpszFormat,argList);
	delete[] lpszFormat;
	
	va_end(argList);
}

void CStringW::FormatEx(UINT nFormatID,...)
{
	va_list argList;
	va_start(argList,nFormatID);
	
	
	LPWSTR lpszFormat=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (lpszFormat==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadString(nFormatID,lpszFormat,STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete lpszFormat;
			lpszFormat=new WCHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadString(nFormatID,lpszFormat,i*STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
		}
	}

	
	LPWSTR temp=new WCHAR[STR_LOADSTRINGBUFLEN];
	m_nDataLen=vswprintfex(temp,STR_LOADSTRINGBUFLEN,lpszFormat,argList);
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData==NULL)
			delete[] m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
	}
	sMemCopyW(m_pData,temp,m_nDataLen+1);
	
	delete[] lpszFormat;
	delete[] temp;
	va_end(argList);
}

void CStringW::FormatC(UINT nFormatID,...)
{
	va_list argList;
	va_start(argList,nFormatID);
	
	LPWSTR lpszFormat=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (lpszFormat==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadString(nFormatID,lpszFormat,STR_LOADSTRINGBUFLEN,CommonResource);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete lpszFormat;
			lpszFormat=new WCHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadString(nFormatID,lpszFormat,i*STR_LOADSTRINGBUFLEN,CommonResource);
		}
	}
	
	FormatV(lpszFormat,argList);
	delete[] lpszFormat;
	
	va_end(argList);
}

void CStringW::FormatExC(UINT nFormatID,...)
{
	va_list argList;
	va_start(argList,nFormatID);
	
	
	LPWSTR lpszFormat=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (lpszFormat==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadString(nFormatID,lpszFormat,STR_LOADSTRINGBUFLEN,CommonResource);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete lpszFormat;
			lpszFormat=new WCHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadString(nFormatID,lpszFormat,i*STR_LOADSTRINGBUFLEN,CommonResource);
		}
	}


	LPWSTR temp=new WCHAR[STR_LOADSTRINGBUFLEN];
	m_nDataLen=vswprintfex(temp,STR_LOADSTRINGBUFLEN,lpszFormat,argList);
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData==NULL)
			delete[] m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return;
		}
	}
	sMemCopyW(m_pData,temp,m_nDataLen+1);
	
	delete[] lpszFormat;
	delete[] temp;
	va_end(argList);
}


CStringW::CStringW(int nID)
:	m_nBase(10)
{
	LPWSTR szBuffer;
	szBuffer=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadStringW(GetLanguageSpecificResourceHandle(),nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new WCHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringW(GetLanguageSpecificResourceHandle(),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;;
	}
	sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
}

CStringW::CStringW(int nID,TypeOfResourceHandle bType)
:	m_nBase(10)
{
	LPWSTR szBuffer;
	szBuffer=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadStringW(GetResourceHandle(bType),nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new WCHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringW(GetResourceHandle(bType),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;;
	}
	sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
}


CStringW::CStringW(UINT nID)
:	m_nBase(10)
{
	LPWSTR szBuffer;
	szBuffer=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadString(nID,szBuffer,STR_LOADSTRINGBUFLEN,LanguageSpecificResource);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new WCHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringW(GetLanguageSpecificResourceHandle(),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;;
	}
	MemCopyW(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
}

CStringW::CStringW(UINT nID,TypeOfResourceHandle bType)
:	m_nBase(10)
{
	LPWSTR szBuffer;
	szBuffer=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;
	}
	m_nDataLen=::LoadString(nID,szBuffer,STR_LOADSTRINGBUFLEN,bType);
	if (m_nDataLen>=STR_LOADSTRINGBUFLEN-2)
	{
		for (DWORD i=2;m_nDataLen>=i*STR_LOADSTRINGBUFLEN-2;i++)
		{
			delete szBuffer;
			szBuffer=new WCHAR[i*STR_LOADSTRINGBUFLEN];
			m_nDataLen=::LoadStringW(GetResourceHandle(bType),nID,szBuffer,i*STR_LOADSTRINGBUFLEN);
		}
	}
	m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
	if (m_pData==NULL)
	{
		SetHFCError(HFC_CANNOTALLOCATE);
		return;;
	}
	sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
}

BOOL CStringW::LoadString(UINT nID)
{
	LPWSTR szBuffer=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{	
		SetHFCError(HFC_CANNOTALLOCATE);
		return FALSE;
	}
	m_nDataLen=::LoadString(nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return FALSE;
		}
	}
	sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
	return TRUE;
}

BOOL CStringW::LoadString(UINT nID,TypeOfResourceHandle bType)
{
	LPWSTR szBuffer=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{	
		SetHFCError(HFC_CANNOTALLOCATE);
		return FALSE;
	}
	m_nDataLen=::LoadString(nID,szBuffer,STR_LOADSTRINGBUFLEN,bType);
	if (m_nDataLen>=m_nAllocLen || m_nDataLen<m_nAllocLen-10)
	{
		if (m_pData!=NULL)
			delete[] m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return FALSE;
		}
	}
	sMemCopyW(m_pData,szBuffer,m_nDataLen+1);
	delete[] szBuffer;
	return TRUE;
}

BOOL CStringW::AddString(UINT nID)
{
	LPWSTR szBuffer=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{	
		SetHFCError(HFC_CANNOTALLOCATE);
		return FALSE;
	}
	int nStrLen=::LoadString(nID,szBuffer,STR_LOADSTRINGBUFLEN);
	if (nStrLen+m_nDataLen>=m_nAllocLen)
	{
		LPWSTR temp=m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+nStrLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return FALSE;
		}
		if (temp!=NULL)
		{
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
	}
	sMemCopyW(m_pData+m_nDataLen,szBuffer,nStrLen+1);
	m_nDataLen+=nStrLen;
	delete[] szBuffer;
	return TRUE;
}

BOOL CStringW::AddString(UINT nID,TypeOfResourceHandle bType)
{
	LPWSTR szBuffer=new WCHAR[STR_LOADSTRINGBUFLEN];
	if (szBuffer==NULL)
	{	
		SetHFCError(HFC_CANNOTALLOCATE);
		return FALSE;
	}
	int nStrLen=::LoadString(nID,szBuffer,STR_LOADSTRINGBUFLEN,bType);
	if (nStrLen+m_nDataLen>=m_nAllocLen)
	{
		LPWSTR temp=m_pData;
		m_pData=new WCHAR[m_nAllocLen=m_nDataLen+nStrLen+STR_EXTRAALLOC];
		if (m_pData==NULL)
		{	
			SetHFCError(HFC_CANNOTALLOCATE);
			return FALSE;
		}
		if (temp!=NULL)
		{
			sMemCopyW(m_pData,temp,m_nDataLen);
			delete[] temp;
		}
	}
	sMemCopyW(m_pData+m_nDataLen,szBuffer,nStrLen+1);
	m_nDataLen+=nStrLen;
	delete[] szBuffer;
	return TRUE;
}

#endif

#endif
