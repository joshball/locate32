////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#ifdef WIN32
int strcasecmp(LPCSTR s1,LPCSTR s2)
{
	CHAR *tmp1,*tmp2;	
	int ret;
	size_t nLen1=istrlen(s1);
	size_t nLen2=istrlen(s2);
	
	tmp1=new CHAR[nLen1+2];
	if (tmp1==NULL)
		SetHFCError(HFC_CANNOTALLOC);
	CopyMemory(tmp1,s1,nLen1+1);
	
	tmp2=new CHAR[nLen2+2];
	if (tmp2==NULL)
		SetHFCError(HFC_CANNOTALLOC);
	CopyMemory(tmp2,s2,nLen2+1);
	
	CharLowerBuff(tmp1,nLen1);
	CharLowerBuff(tmp2,nLen2);
	ret=strcmp(tmp1,tmp2);
	delete[] tmp1;
	delete[] tmp2;
	return ret;
}
#endif

#ifdef DEF_WCHAR
int strcasecmp(LPCWSTR s1,LPCWSTR s2)
{
	WCHAR *tmp1,*tmp2;	
	int ret;
	size_t nLen1=istrlenw(s1);
	size_t nLen2=istrlenw(s2);
	
	tmp1=new WCHAR[nLen1+2];
	if (tmp1==NULL)
		SetHFCError(HFC_CANNOTALLOC);
	CopyMemory(tmp1,s1,nLen1*2+2);
	
	tmp2=new WCHAR[nLen2+2];
	if (tmp2==NULL)
		SetHFCError(HFC_CANNOTALLOC);
	CopyMemory(tmp2,s2,nLen2*2+2);
	
	CharLowerBuffW(tmp1,nLen1);
	CharLowerBuffW(tmp2,nLen2);
	ret=wcscmp(tmp1,tmp2);
	delete[] tmp1;
	delete[] tmp2;
	return ret;
}
#endif

int strcasencmp(LPCSTR s1,LPCSTR s2,DWORD n)
{
	if (int(n)<1)
		return 0;

	TCHAR *tmp1,*tmp2;	
	int ret;

	for (DWORD n1=0;n1<n && s1[n1]!='\0';n1++);
	for (DWORD n2=0;n2<n && s2[n2]!='\0';n2++);

	tmp1=new CHAR[n+1];
	if (tmp1==NULL)
		SetHFCError(HFC_CANNOTALLOC);
	tmp2=new CHAR[n+1];
	if (tmp2==NULL)
		SetHFCError(HFC_CANNOTALLOC);
	dMemCopy(tmp1,s1,n1);
	dMemCopy(tmp2,s2,n2);
    
	tmp1[n1]='\0';
	tmp2[n2]='\0';
	CharLower(tmp1);
	CharLower(tmp2);
	ret=strcmp(tmp1,tmp2);
	delete[] tmp1;
	delete[] tmp2;
	return ret;
}

int strcasencmp(LPCWSTR s1,LPCWSTR s2,DWORD n)
{
	if (int(n)<1)
		return 0;

	WCHAR *tmp1,*tmp2;	
	int ret;

	for (DWORD n1=0;n1<n && s1[n1]!='\0';n1++);
	for (DWORD n2=0;n2<n && s2[n2]!='\0';n2++);

	tmp1=new WCHAR[n+1];
	if (tmp1==NULL)
		SetHFCError(HFC_CANNOTALLOC);
	tmp2=new WCHAR[n+1];
	if (tmp2==NULL)
		SetHFCError(HFC_CANNOTALLOC);
	dMemCopy(tmp1,s1,n1*2);
	dMemCopy(tmp2,s2,n2*2);
    
	tmp1[n1]='\0';
	tmp2[n2]='\0';
	CharLowerW(tmp1);
	CharLowerW(tmp2);
	ret=wcscmp(tmp1,tmp2);
	delete[] tmp1;
	delete[] tmp2;
	return ret;
}


BYTE ContainString(LPCSTR string,LPCSTR pattern)
{
   unsigned int i,j=0;
   for (i=0;i<strlen(string);i++)
   {
      j=0;
      while ((string[i+j]==pattern[j] || pattern[j]=='?') && string[i+j]!='\0' ) j++;
      if (pattern[j]=='\0')
         return 1;
      if (pattern[j]=='*')
         return ContainString(&(string[j+i+1]),&(pattern[j+1]));
   }
   return 0;
}

BYTE ContainString(LPCWSTR string,LPCWSTR pattern)
{
   unsigned int i,j=0;
   for (i=0;i<istrlenw(string);i++)
   {
      j=0;
      while ((string[i+j]==pattern[j] || pattern[j]==L'?') && string[i+j]!=L'\0' ) j++;
      if (pattern[j]==L'\0')
         return 1;
      if (pattern[j]==L'*')
         return ContainString(&(string[j+i+1]),&(pattern[j+1]));
   }
   return 0;
}

int FirstCharIndex(LPCSTR str,const CHAR ch)
{
	int i;
	for (i=0;str[i]!='\0';i++)
	{
		if (str[i]==ch)
			return i;
	}
	return -1;
}

int FirstCharIndex(LPCWSTR str,const WCHAR ch)
{
	int i;
	for (i=0;str[i]!=L'\0';i++)
	{
		if (str[i]==ch)
			return i;
	}
	return -1;
}

int LastCharIndex(LPCSTR str,const CHAR ch)
{
	int i,ret=-1;
	for (i=0;str[i]!='\0';i++)
	{
		if (str[i]==ch)
			ret=i;
	}
	return ret;
}

int LastCharIndex(LPCWSTR str,const WCHAR ch)
{
	int i,ret=-1;
	for (i=0;str[i]!=L'\0';i++)
	{
		if (str[i]==ch)
			ret=i;
	}
	return ret;
}

int NextCharIndex(LPCSTR str,const CHAR ch,int oldidx)
{
   int i;
   for (i=oldidx+1;str[i]!='\0';i++)
   {
      if (str[i]==ch)
         return i;
   }
   return -1;
}

int NextCharIndex(LPCWSTR str,const WCHAR ch,int oldidx)
{
   int i;
   for (i=oldidx+1;str[i]!=L'\0';i++)
   {
      if (str[i]==ch)
         return i;
   }
   return -1;
}

static int _getbase(LPCSTR& str)
{
	if (*str==':')
		return 16;
	
	int base=0;
	if (*str=='(')
	{
		for (;*str=='(';str++);
		for (;*str>='0' && *str<='9';str++)
		{
			base*=10;
			base+=*str-'0';

		}
		for (;*str==')';str++);
	}
	else
	{
		for (;*str>='0' && *str<='9';str++)
		{
			base*=10;
			base+=*str-'0';
		}
	}
	return base>0?base:16;
}

// szString will change
int _readnum(int base,LPCSTR& str,DWORD length)
{
	int num=0;
	BOOL bToNegative=FALSE;
	for (;*str=='-';str++)
		bToNegative=!bToNegative;

	if (base==16)
	{
		for (;*str!='\0' && length>0;length--,str++)
		{
			if (*str>='0' && *str<='9')
			{
				num<<=4;
				num+=*str-'0';
			}
			else if (*str>='a' && *str<='f')
			{
				num<<=4;
				num+=*str-'a'+0xa;
			}
			else if (*str>='A' && *str<='F')
			{
				num<<=4;
				num+=*str-'A'+0xa;
			}
			else
				break;
		}
		if (bToNegative)
			return -num;
		return num;
	}
	for (;*str!='\0' && length>0;length--,str++)
	{
		int n=chartonum(*str);
        if (n>=base)
			break;
		if (n==0 && *str!='0')
			break;
		num*=base;
		num+=n;
	}
	if (bToNegative)
		return -num;
	return num;
}

/*
BYTE* dataparser(LPCSTR pString,DWORD dwStrLen,DWORD* pdwDataLength);

Converts str to BYTE* pointer and returns it. 
Returned pointer should delete with delete[](BYTE*) operator.

if str is:
  "Hello", data will be "Hello" and *pdwDataLength will be 5
  "str:Hello", data will be "Hello" and *pdwDataLength will be 5
  "str:Hello\0", data will be "Hello\0" and *pdwDataLength will be 6
  "oem:Hello", data will be "Hello" and *pdwDataLength will be 5, ansi to oem conversion will be done
  "unicode:Hello", data will be L"Hello" and *pdwDataLength will be 10, ansi to unicode conversion will be done
  "dword(16):12345678", *(DWORD*)data will be 0x12345678 (hex) and *pdwDataLength will be 4
  "dword(16):12345678", *(DWORD*)data will be 0x12345678 (hex) and *pdwDataLength will be 4
  "dword(10):12345678", *(DWORD*)data will be 12345678 (decimal) and *pdwDataLength will be 4
  "dword:10" and "dword16:10" are same as "dword(16):10" (16 is default)
  "int:10" is same as "dword(10):10"
  "word(16):1234", *(WORD*)data will be 0x1234 and *pdwDataLength will be 2
  "byte(16):12", *(BYTE*)data will be 0x12 and *pdwDataLength will be 1

if str begins with "hex:" or "bin:" following data will be hex data e.g:
  "hex:12 34 56 78" (same as "dword(16):78563412")
  "hex:12 34" (same as "word(16):3412")
  "hex:123456789abcde"
  "hex:0000000000" (5 zero bytes)
  "hex:0 0 0 0 0" (5 zero bytes)

*/

BYTE* dataparser(LPCSTR pStr,DWORD dwStrLen,MALLOC_FUNC pMalloc,DWORD* pdwDataLength)
{
	if (pStr[0]=='\0')
		return NULL;
	
	if (_1stcontain2nd(pStr,"int:"))
	{
		pStr+=4;
		BYTE* pRet=(BYTE*)pMalloc(4);
		*((DWORD*)pRet)=DWORD(_readnum(10,pStr));
		if (pdwDataLength!=NULL)
			*pdwDataLength=4;
		return pRet;
	}
	else if (_1stcontain2nd(pStr,"dword"))
	{
		pStr+=5;
		int base=_getbase(pStr);
		if (*pStr!=':')
			return NULL;
		pStr++;
		BYTE* pRet=(BYTE*)pMalloc(4);
		*((DWORD*)pRet)=DWORD(_readnum(base,pStr));
		if (pdwDataLength!=NULL)
			*pdwDataLength=4;
		return pRet;
	}
	else if (_1stcontain2nd(pStr,"word"))
	{
		pStr+=4;		
		int base=_getbase(pStr);
		if (*pStr!=':')
			return NULL;
		pStr++;
		BYTE* pRet=(BYTE*)pMalloc(2);
		*((WORD*)pRet)=WORD(_readnum(base,pStr));
		if (pdwDataLength!=NULL)
			*pdwDataLength=2;
		return pRet;
	}
	else if (_1stcontain2nd(pStr,"byte"))
	{
		pStr+=4;		
			int base=_getbase(pStr);
		if (*pStr!=':')
			return NULL;
		pStr++;
		BYTE* pRet=(BYTE*)pMalloc(2);
		*pRet=BYTE(_readnum(base,pStr));
		if (pdwDataLength!=NULL)
			*pdwDataLength=1;
		return pRet;
	}
	else if (_1stcontain2nd(pStr,"hex:") || _1stcontain2nd(pStr,"bin:"))
	{
		pStr+=4;
		DWORD i=0;
		
		// Calculating reqiured size
		for (LPCSTR pStr2=pStr;*pStr2!='\0';i++)
		{
			if (!((*pStr2>='0' && *pStr2<='9') || 
				(*pStr2>='a' && *pStr2<='f') ||
				(*pStr2>='A' && *pStr2<='F')))
				break;
				
			pStr2++;

			if ((*pStr2>='0' && *pStr2<='9') || 
				(*pStr2>='a' && *pStr2<='f') ||
				(*pStr2>='A' && *pStr2<='F'))
				pStr2++;

			for (;*pStr2==' ';pStr2++);
		}

		if (i==0)
			return NULL;
			
		BYTE* pRet=(BYTE*)pMalloc(max(i,2));

		for (i=0;*pStr!='\0';i++)
		{
			if (*pStr>='0' && *pStr<='9')
				pRet[i]=*pStr-'0';
			else if (*pStr>='a' && *pStr<='f')
				pRet[i]=*pStr-'a'+0xa;
			else if (*pStr>='A' && *pStr<='F')
				pRet[i]=*pStr-'A'+0xa;
			else
				break;

			pStr++;

			if (*pStr>='0' && *pStr<='9')
			{
				pRet[i]<<=4;
				pRet[i]+=*pStr-'0';
				pStr++;
			}
			else if (*pStr>='a' && *pStr<='f')
			{
				pRet[i]<<=4;
				pRet[i]+=*pStr-'a'+0xa;
				pStr++;
			}
			else if (*pStr>='A' && *pStr<='F')
			{
				pRet[i]<<=4;
				pRet[i]+=*pStr-'A'+0xa;
				pStr++;
			}

			for (;*pStr==' ';pStr++);
		}
		if (pdwDataLength!=NULL)
			*pdwDataLength=i;
		return pRet;
	}
	else if (_1stcontain2nd(pStr,"str:"))
	{
		dwStrLen-=4;
		pStr+=4;
		BYTE* pRet;
		if (int(dwStrLen)<=0)
			return NULL;

		pRet=(BYTE*)pMalloc(dwStrLen);
		int i;
		for (i=0;*pStr!='\0';i++,pStr++)
		{
			if (*pStr=='\\' && pStr[1]!='\0')
			{
				pStr++;
				switch (*pStr)
				{
				case '0':
					pRet[i]='\0';
					break;
				case 'n':
					pRet[i]='\n';
					break;
				case 'r':
					pRet[i]='\r';
					break;
				case 't':
					pRet[i]='\t';
					break;
				case 'b':
					pRet[i]='\b';
					break;
				default:
					pRet[i]=BYTE(_readnum(16,pStr,2));
					pStr--;
					break;
				}
			}
			else
				pRet[i]=*pStr;
		}
		if (pdwDataLength!=NULL)
			*pdwDataLength=i;
		return pRet;
	}
#ifdef WIN32
	else if (_1stcontain2nd(pStr,"oem:"))
	{
		dwStrLen-=4;
		pStr+=4;
		if (int(dwStrLen)<=0)
			return NULL;
		
		BYTE* pRet;
		pRet=(BYTE*)pMalloc(dwStrLen);
		int i;
		for (i=0;*pStr!='\0';i++,pStr++)
		{
			if (*pStr=='\\' && pStr[1]!='\0')
			{
				pStr++;
				switch (*pStr)
				{
				case '0':
					pRet[i]='\0';
					break;
				case 'n':
					pRet[i]='\n';
					break;
				case 'r':
					pRet[i]='\r';
					break;
				case 't':
					pRet[i]='\t';
					break;
				case 'b':
					pRet[i]='\b';
					break;
				default:
					pRet[i]=BYTE(_readnum(16,pStr,2));
					pStr--;
					break;
				}
			}
			else
				pRet[i]=*pStr;
		}
		AnsiToOemBuff(LPSTR(pRet),LPSTR(pRet),i);
		if (pdwDataLength!=NULL)
			*pdwDataLength=i;
		return pRet;
	}
#endif
#ifdef DEF_WCHAR
	else if (_1stcontain2nd(pStr,"wstr:") || _1stcontain2nd(pStr,"uni:"))
	{
		for (pStr+=3,dwStrLen-=3;*pStr!=':';pStr++,dwStrLen--);
		pStr++;dwStrLen--;

		if (int(dwStrLen)<=0)
			return NULL;
	
		WCHAR* pRet;
		pRet=(WCHAR*)pMalloc(dwStrLen*2);


		int i;
		for (i=0;*pStr!='\0';i++,pStr++)
		{
			if (*pStr=='\\' && pStr[1]!='\0')
			{
				pStr++;
				switch (*pStr)
				{
				case '0':
					pRet[i]=L'\0';
					break;
				case 'n':
					pRet[i]=L'\n';
					break;
				case 'r':
					pRet[i]=L'\r';
					break;
				case 't':
					pRet[i]=L'\t';
					break;
				case 'b':
					pRet[i]=L'\b';
					break;
				default:
					pRet[i]=WCHAR(_readnum(16,pStr,4));
					pStr--;
					break;
				}
			}
			else
				MemCopyAtoW(pRet+i,pStr,1);
		}
		if (pdwDataLength!=NULL)
			*pdwDataLength=i*2;
		return (BYTE*)pRet;
	}
#endif
	else
	{
		if (int(dwStrLen)<=0)
			return NULL;
		
		BYTE* pRet;
		pRet=(BYTE*)pMalloc(dwStrLen);
		dMemCopy(pRet,pStr,dwStrLen);
		if (pdwDataLength!=NULL)
			*pdwDataLength=dwStrLen;
		return pRet;
	}
}

static int _getbase2(LPCSTR& str,int def)
{
	if (*str=='(')
		return def;
	
	int base=0;
	for (;*str>='0' && *str<='9';str++)
	{
		base*=10;
		base+=*str-'0';
	}
	return base>0?base:def;
}

/*
BYTE* dataparser2(LPCSTR pString,DWORD* pdwDataLength);

Converts str to BYTE* pointer and returns it. 
Returned pointer should delete with delete[](BYTE*) operator.

hex is default style, usage e.g. "FF EE oem(Hello)" of "11 int(10) dword(1234,16)"
if str is:

  "str(Hello)", data will be "Hello" and *pdwDataLength will be 5
  "str(Hello\0)", data will be "Hello\0" and *pdwDataLength will be 6
  "oem(Hello)", data will be "Hello" and *pdwDataLength will be 5, ansi to oem conversion will be done
  "unicode(Hello)", data will be L"Hello" and *pdwDataLength will be 10, ansi to unicode conversion will be done
  "dword16(12345678)", *(DWORD*)data will be 0x12345678 (hex) and *pdwDataLength will be 4
  "dword10(12345678)", *(DWORD*)data will be 12345678 (decimal) and *pdwDataLength will be 4
  "dword(X)" is same as "dword16(X)" (16 is default in case of dword)
  "int(X)" is same as "int10(X)" (10 is default in case of int)
  "word16(1234)", *(WORD*)data will be 0x1234 and *pdwDataLength will be 2
  "byte16(12)", *(BYTE*)data will be 0x12 and *pdwDataLength will be 1
*/

inline void _allocmore(BYTE*& pStr,BYTE*& pStrPtr,DWORD& nAllocLen,DWORD nRequired)
{
	DWORD nLen=DWORD(pStrPtr-pStr);
	if (nAllocLen<nLen+nRequired)
	{
		BYTE* pNewPtr=new BYTE[nAllocLen=nLen+nRequired+10];
        MemCopy(pNewPtr,pStr,nLen);
		delete[] pStr;
		pStr=pNewPtr;
		pStrPtr=pStr+nLen;
	}
}

BYTE* dataparser2(LPCSTR pStr,DWORD* pdwDataLength)
{
	if (pStr[0]=='\0')
		return NULL;
	
	BYTE* pData=new BYTE[10];
	BYTE* pDataPtr=pData;
	DWORD nAllocLen=10;

	// Removing spaces 
	while (*pStr==' ') pStr++;

	while (*pStr!='\0')
	{
		if (_1stcontain2nd(pStr,"int"))
		{
			pStr+=3;
			int base=_getbase2(pStr,10);
			if (*pStr!='(')
				break;
			pStr++;
			
			_allocmore(pData,pDataPtr,nAllocLen,4);
			*((DWORD*)pDataPtr)=DWORD(_readnum(base,pStr));
			while (*pStr!=')' && *pStr!='\0') pStr++;
			if (*pStr==')') pStr++;
			pDataPtr+=4;
		}
		else if (_1stcontain2nd(pStr,"dword"))
		{
			pStr+=5;
			int base=_getbase2(pStr,16);
			if (*pStr!='(')
				break;
			pStr++;
			
			_allocmore(pData,pDataPtr,nAllocLen,4);
			*((DWORD*)pDataPtr)=DWORD(_readnum(base,pStr));
			while (*pStr!=')' && *pStr!='\0') pStr++;
			if (*pStr==')') pStr++;
			pDataPtr+=4;
		}
		else if (_1stcontain2nd(pStr,"word"))
		{
			pStr+=4;
			int base=_getbase2(pStr,16);
			if (*pStr!='(')
				break;
			pStr++;
			
			_allocmore(pData,pDataPtr,nAllocLen,2);
			*((WORD*)pDataPtr)=WORD(_readnum(base,pStr));
			while (*pStr!=')' && *pStr!='\0') pStr++;
			if (*pStr==')') pStr++;
			pDataPtr+=2;
		}
		else if (_1stcontain2nd(pStr,"byte"))
		{
			pStr+=4;
			int base=_getbase2(pStr,16);
			if (*pStr!='(')
				break;
			pStr++;
			
			_allocmore(pData,pDataPtr,nAllocLen,1);
			*((BYTE*)pDataPtr)=BYTE(_readnum(base,pStr));
			while (*pStr!=')' && *pStr!='\0') pStr++;
			if (*pStr==')') pStr++;
			pDataPtr+=1;
		}
		else if (_1stcontain2nd(pStr,"str"))
		{
			pStr+=3;
			if (*pStr!='(')
				break;
			pStr++;
			
			for (;*pStr!=')' && *pStr!='\0';pStr++)
			{
				_allocmore(pData,pDataPtr,nAllocLen,1);
				
				if (*pStr=='\\' && pStr[1]!='\0')
				{
					pStr++;
					switch (*pStr)
					{
					case '0':
						*(pDataPtr++)='\0';
						break;
					case 'n':
						*(pDataPtr++)='\n';
						break;
					case 'r':
						*(pDataPtr++)='\r';
						break;
					case 't':
						*(pDataPtr++)='\t';
						break;
					case 'b':
						*(pDataPtr++)='\b';
						break;
					case ')':
						*(pDataPtr++)=')';
						break;
					default:
						*(pDataPtr++)=BYTE(_readnum(16,pStr,2));;
						pStr--;
						break;
					}
				}
				else
					*(pDataPtr++)=*pStr;
			}
			if (*pStr==')') pStr++;
		}
	#ifdef WIN32
		else if (_1stcontain2nd(pStr,"oem"))
		{
			pStr+=3;
			if (*pStr!='(')
				break;
			pStr++;
			
			int iStart=DWORD(pDataPtr-pData);
			
			for (;*pStr!=')' && *pStr!='\0';pStr++)
			{
				_allocmore(pData,pDataPtr,nAllocLen,1);
				
				if (*pStr=='\\' && pStr[1]!='\0')
				{
					pStr++;
					switch (*pStr)
					{
					case '0':
						*(pDataPtr++)='\0';
						break;
					case 'n':
						*(pDataPtr++)='\n';
						break;
					case 'r':
						*(pDataPtr++)='\r';
						break;
					case 't':
						*(pDataPtr++)='\t';
						break;
					case 'b':
						*(pDataPtr++)='\b';
						break;
					case ')':
						*(pDataPtr++)=')';
						break;
					default:
						*(pDataPtr++)=BYTE(_readnum(16,pStr,2));;
						pStr--;
						break;
					}
				}
				else
					*(pDataPtr++)=*pStr;
			}
			CharToOemBuff(LPSTR(pData+iStart),LPSTR(pData+iStart),DWORD(pDataPtr-pData)-iStart);
			if (*pStr==')') pStr++;
		}
	#endif
	#ifdef DEF_WCHAR
		else if (_1stcontain2nd(pStr,"wstr") || _1stcontain2nd(pStr,"uni"))
		{
			pStr+=3;
			while (*pStr!='(') pStr++;
			pStr++;
			
			for (;*pStr!=')' && *pStr!='\0';pStr++)
			{
				_allocmore(pData,pDataPtr,nAllocLen,2);
				
				if (*pStr=='\\' && pStr[1]!='\0')
				{
					pStr++;
					switch (*pStr)
					{
					case '0':
						*LPWORD(pDataPtr)=L'\0';
						break;
					case 'n':
						*LPWORD(pDataPtr)=L'\n';
						break;
					case 'r':
						*LPWORD(pDataPtr)=L'\r';
						break;
					case 't':
						*LPWORD(pDataPtr)=L'\t';
						break;
					case 'b':
						*LPWORD(pDataPtr)=L'\b';
						break;
					case ')':
						*LPWORD(pDataPtr)=L')';
						break;
					default:
						*LPWORD(pDataPtr)=WORD(_readnum(16,pStr,4));;
						pStr--;
						break;
					}
					pDataPtr+=2;
				}
				else
				{	
					MemCopyAtoW(pDataPtr,pStr,1);
					pDataPtr+=2;
				}
			}
			if (*pStr==')') pStr++;
		}
	#endif
		else
		{
			_allocmore(pData,pDataPtr,nAllocLen,1);
			LPCSTR pStrOld=pStr;
			*pDataPtr=_readnum(16,pStr,2);
			if (pStr==pStrOld)
			{
				*pdwDataLength=DWORD(pDataPtr-pData);
				return pData;
			}
			pDataPtr++;
		}
		while (*pStr==' ') pStr++;
	}
	*pdwDataLength=DWORD(pDataPtr-pData);
	return pData;
}



BOOL IsCharNumeric(char cChar,BYTE bBase)
{
	if (bBase<=10)
	{
		if (cChar>='0' && cChar<='0'+bBase-1)
			return TRUE;
		return FALSE;
	}
	if (cChar>='0' && cChar<='9')
		return TRUE;
	bBase-=11;
	if (cChar>='A' && cChar<='A'+bBase)
		return TRUE;
	if (cChar>='a' && cChar<='a'+bBase)
		return TRUE;
	return FALSE;
}


inline void* va_getarg(va_list argList,int count)
{
	for (int i=0;i<count-1;i++)
		va_arg(argList,void*);
	return va_arg(argList,void*);
}



int vsprintfex( char *buffer, size_t buffersize,const char *format, va_list argList )
{
	int ptr=0;
	const char* in=format;
	LPSTR end;

	int nNextArg=0;

    while (*in!='\0')
	{
		if (*in=='%')
		{
			in++;
			
			// Finding first non number
			for(int index=0;in[index]>='0' && in[index]<='9';index++);

			// Now index points to nonnumberic character
			if (in[index]==':')
			{
				// ok number was argument place
				if (index==0)
					return 0;
				nNextArg=atoi(in);
                
				// finding next '%'
				in+=index+1;
				for (int length=0;in[length]!='\0' && in[length]!='%';length++);

				char* pTemp=new char[length+2];
				pTemp[0]='%';
				CopyMemory(pTemp+1,in,length);
				pTemp[length+1]='\0';
				if (StringCbPrintfExA(buffer+ptr,buffersize-ptr,&end,NULL,STRSAFE_IGNORE_NULLS,pTemp,va_getarg(argList,nNextArg))!=S_OK)
					return 0;
                ptr=end-buffer;
				delete[] pTemp;

				if (in[length]=='\0')
					break;
				
				in+=length;
			}
			else
			{
				nNextArg++;
				for (;in[index]!='\0' && in[index]!='%';index++);
			

				char* pTemp=new char[index+2];
				pTemp[0]='%';
				CopyMemory(pTemp+1,in,index);
				pTemp[index+1]='\0';
				if (StringCbPrintfExA(buffer+ptr,buffersize-ptr,&end,NULL,STRSAFE_IGNORE_NULLS,pTemp,va_getarg(argList,nNextArg))!=S_OK)
					return 0;
                ptr=end-buffer;
				delete[] pTemp;
				
				if (in[index]=='\0')
					break;
				
				in+=index;                
			}
		}
		else
			buffer[ptr++]=*(in++);
	}
	return ptr;
}


#ifdef DEF_WCHAR
int vswprintfex( wchar_t *buffer, size_t buffersize, const wchar_t *format, va_list argList )
{
	int ptr=0;
	const wchar_t* in=format;
	LPWSTR end;

	int nNextArg=0;

    while (*in!='\0')
	{
		if (*in=='%')
		{
			in++;
			
			// Finding first non number
			for(int index=0;in[index]>=L'0' && in[index]<=L'9';index++);

			// Now index points to nonnumberic character
			if (in[index]==L':')
			{
				// ok number was argument place
				if (index==0)
					return 0;
				nNextArg=_wtoi(in);
                
				// finding next '%'
				in+=index+1;
				for (int length=0;in[length]!=L'\0' && in[length]!=L'%';length++);

				wchar_t* pTemp=new wchar_t[length+2];
				pTemp[0]=L'%';
				MemCopyW(pTemp+1,in,length);
				pTemp[length+1]=L'\0';
				if (StringCbPrintfExW(buffer+ptr,buffersize-ptr,&end,NULL,STRSAFE_IGNORE_NULLS,pTemp,va_getarg(argList,nNextArg))!=S_OK)
					return 0;
                ptr=end-buffer;
				delete[] pTemp;

				if (in[length]==L'\0')
					break;
				
				in+=length;
			}
			else
			{
				nNextArg++;
				for (;in[index]!=L'\0' && in[index]!=L'%';index++);
			

				wchar_t* pTemp=new wchar_t[index+2];
				pTemp[0]='%';
				MemCopyW(pTemp+1,in,index);
				pTemp[index+1]=L'\0';
				
                
				if (StringCbPrintfExW(buffer+ptr,buffersize-ptr,&end,NULL,STRSAFE_IGNORE_NULLS,pTemp,va_getarg(argList,nNextArg))!=S_OK)
					return 0;
                ptr=end-buffer;

				delete[] pTemp;
				
				if (in[index]==L'\0')
					break;
				
				in+=index;                
			}
		}
		else
			buffer[ptr++]=*(in++);
	}
	return ptr;
}
#endif


#ifdef DEF_WCHAR
int LoadString(UINT uID,LPWSTR lpBuffer,int nBufferMax)
{
	if (!IsFullUnicodeSupport())
	{
		char* pStr=new char[nBufferMax+1];
		int nRet=::LoadStringA(GetLanguageSpecificResourceHandle(),uID,pStr,nBufferMax);
		MultiByteToWideChar(CP_ACP,0,pStr,nRet+1,lpBuffer,nBufferMax);
		delete[] pStr;
		return nRet;
	}
	return (int)::LoadStringW(GetLanguageSpecificResourceHandle(),uID,lpBuffer,nBufferMax);
}

int LoadString(UINT uID,LPWSTR lpBuffer,int nBufferMax,TypeOfResourceHandle bType)
{
	if (!IsFullUnicodeSupport())
	{
		char* pStr=new char[nBufferMax+1];
		int nRet=::LoadStringA(GetResourceHandle(bType),uID,pStr,nBufferMax);
		MultiByteToWideChar(CP_ACP,0,pStr,nRet+1,lpBuffer,nBufferMax);
		delete[] pStr;
		return nRet;
	}
	return (int)::LoadStringW(GetResourceHandle(bType),uID,lpBuffer,nBufferMax);
}
#endif