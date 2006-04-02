/* Copyright (c) 1997-2006 Janne Huttunen
   database updater v2.99.6.3190                  */

#include <HFCLib.h>
#include "Locatedb.h"

CDatabase::CDatabase(CDatabase& src)
{
	sMemCopy(this,&src,sizeof(CDatabase));
	// This cleares m_aExcludedDirectories to null to prevent invalid memory handling
    m_aExcludedDirectories.GiveBuffer();
	
	if (m_szName!=NULL)
		m_szName=alloccopy(m_szName);
	if (m_szCreator!=NULL)
		m_szCreator=alloccopy(m_szCreator);
	if (m_szDescription!=NULL)
		m_szDescription=alloccopy(m_szDescription);
	if (m_szArchiveName!=NULL)
		m_szArchiveName=alloccopy(m_szArchiveName);

	
	if (m_szRoots!=NULL)
	{
		DWORD iLength;
		for (iLength=0;m_szRoots[iLength]!=L'\0' || m_szRoots[iLength+1]!=L'\0';iLength++);
		iLength+=2;
        m_szRoots=new WCHAR[iLength];
		sMemCopy(m_szRoots,src.m_szRoots,iLength);
	}

	for (int i=0;i<src.m_aExcludedDirectories.GetSize();i++)
		m_aExcludedDirectories.Add(alloccopy(src.m_aExcludedDirectories[i]));
}

BOOL CDatabase::LoadFromRegistry(HKEY hKeyRoot,LPCSTR szPath,CArray<CDatabase*>& aDatabases)
{
	CRegKey RegKey;
	CString Path(szPath);
	Path<<'\\';
	
	
	if (RegKey.OpenKey(hKeyRoot,Path,CRegKey::defRead|CRegKey::samEnumerateSubkeys)!=ERROR_SUCCESS)
		return FALSE;

    CString key;
	for (int i=0;RegKey.EnumKey(i,key);i++)
	{
		CDatabase* pDatabase=CDatabase::FromKey(RegKey,"",key);
		if (pDatabase->m_wID==0)
			pDatabase->m_wID=GetUniqueIndex(aDatabases);

		if (pDatabase!=NULL)
			aDatabases.Add(pDatabase);
	}

	return TRUE;
}

CDatabase* CDatabase::FromName(HKEY hKeyRoot,LPCSTR szPath,LPCWSTR szName,SIZE_T iNameLength)
{
	CRegKey RegKey,RegKey2;
	CString Path(szPath);
	Path<<'\\';
	
	
	if (RegKey.OpenKey(hKeyRoot,Path,CRegKey::defRead|CRegKey::samEnumerateSubkeys)!=ERROR_SUCCESS)
		return NULL;

	if (iNameLength==SIZE_T(-1))
		iNameLength=istrlenw(szName);

    CString key;
	for (int i=0;RegKey.EnumKey(i,key);i++)
	{
		if (RegKey2.OpenKey(RegKey,key,CRegKey::defRead)==ERROR_SUCCESS)
		{
			DWORD dwLength=RegKey2.QueryValueLength();
			if (dwLength>1)
			{
				CAllocArrayTmpl<WCHAR> pName(dwLength);
				RegKey2.QueryValue(L"",pName,dwLength);
                
				if (strcasencmp(pName,szName,iNameLength)==0)
					return CDatabase::FromKey(RegKey,"",key);		
			}
			RegKey2.CloseKey();
		}
	}
	return NULL;
}

BOOL CDatabase::SaveToRegistry(HKEY hKeyRoot,LPCSTR szPath,const PDATABASE* ppDatabases,int nDatabases)
{
	CRegKey RegKey;
	CString Path(szPath);
	Path<<'\\';
	
	if (RegKey.OpenKey(hKeyRoot,Path,CRegKey::samAll|CRegKey::createNew)!=ERROR_SUCCESS)
		return FALSE;
	
	CString name;
	// First, delete all existing keys
	for (;RegKey.EnumKey(0,name);)
		RegKey.DeleteKey(name);

	
	// Retrieving digits;
	int nDigits=1;
	for (int i=10;i<=nDatabases;i*=10,nDigits++);
	char* pNum=new char[nDigits+1];
		

    for (int i=0;i<nDatabases;i++)
	{
		char* pKeyName=ppDatabases[i]->GetValidKey(i+1);
		SIZE_T dwLength=istrlen(pKeyName);
		
		// Copying name
		char* pKey=new char[nDigits+dwLength+2];

        sMemCopy(pKey+nDigits+1,pKeyName,dwLength+1);
		pKey[nDigits]='_';
        
        // Formating number
		_itoa_s(i+1,pNum,nDigits+1,10);
		ASSERT(strlen(pNum)<=size_t(nDigits));

		dwLength=istrlen(pNum);
        sMemCopy(pKey+nDigits-dwLength,pNum,dwLength);
		sMemSet(pKey,'0',nDigits-dwLength);
        
		ppDatabases[i]->SaveToRegistry(hKeyRoot,szPath,pKey);
		delete[] pKey;
	}

	delete[] pNum;
	return TRUE;
}


BOOL CDatabase::SaveToRegistry(HKEY hKeyRoot,LPCSTR szPath,LPCSTR szKey)
{
	CRegKey RegKey;
	CString Path(szPath);
	Path<<'\\'<<szKey;
	
	if (RegKey.OpenKey(hKeyRoot,Path,CRegKey::defWrite)!=ERROR_SUCCESS)
		return FALSE;

	RegKey.SetValue(NULL,m_szName);
	RegKey.SetValue("Flags",DWORD(m_wFlags));
	RegKey.SetValue("Thread",DWORD(m_wThread));
	RegKey.SetValue("ID",DWORD(m_wID));

	RegKey.SetValue("ArchiveType",DWORD(m_ArchiveType));
	if (m_szArchiveName!=NULL)
		RegKey.SetValue(L"ArchiveName",m_szArchiveName);
	else
		RegKey.SetValue("ArchiveName",szEmpty);

	if (m_szCreator!=NULL)
		RegKey.SetValue(L"Creator",m_szCreator);
	else
		RegKey.SetValue("Creator",szEmpty);

	if (m_szDescription!=NULL)
		RegKey.SetValue(L"Description",m_szDescription);
	else
		RegKey.SetValue("Description",szEmpty);

	if (m_szRoots!=NULL)
	{
		int i;
		for (i=0;m_szRoots[i]!='\0' || m_szRoots[i+1]!='\0';i++);
		RegKey.SetValue(L"Roots",m_szRoots,i+2,REG_MULTI_SZ);
		
	}
	else
		RegKey.SetValue("Roots",szEmpty);

	// Excluded directories
	DWORD dwLength=1;
	for (int i=0;i<m_aExcludedDirectories.GetSize();i++)
		dwLength+=istrlenw(m_aExcludedDirectories[i])+1;

	WCHAR* pString=new WCHAR[dwLength+1];
	LPWSTR pPtr=pString;
	for (int i=0;i<m_aExcludedDirectories.GetSize();i++)
	{
		if (i>0)
			*(pPtr++)=L';';

		int iLength=istrlenw(m_aExcludedDirectories[i]);
		MemCopyW(pPtr,m_aExcludedDirectories[i],iLength);
		pPtr+=iLength;
	}
	*pPtr=L'\0';
		
	RegKey.SetValue(L"Excluded Directories",pString);
    delete[] pString;
	return TRUE;
}

CDatabase* CDatabase::FromKey(HKEY hKeyRoot,LPCSTR szPath,LPCSTR szKey)
{
	CRegKey RegKey;
	CString Path(szPath);
	if (!Path.IsEmpty() && Path.LastChar()!='\\')
		Path<<'\\';
	Path << szKey;
	
	if (RegKey.OpenKey(hKeyRoot,Path,CRegKey::defRead)!=ERROR_SUCCESS)
		return NULL;

	DWORD dwLength=RegKey.QueryValueLength();
	if (int(dwLength)<=1)
		return NULL;

	CDatabase* pDatabase=new CDatabase;
	
	// Copying name
	pDatabase->m_szName=new WCHAR[dwLength];
	RegKey.QueryValue(L"",pDatabase->m_szName,dwLength);
		
	// Retrieving flags and archive type
	DWORD dwTemp=0;
	RegKey.QueryValue("ID",dwTemp);
	pDatabase->m_wID=LOWORD(dwTemp);
	RegKey.QueryValue("Flags",dwTemp);
	pDatabase->m_wFlags=LOWORD(dwTemp);
	RegKey.QueryValue("Thread",dwTemp);
	pDatabase->m_wThread=LOWORD(dwTemp);
	
	RegKey.QueryValue("ArchiveType",*((DWORD*)&pDatabase->m_ArchiveType));
	
	
	// Copying archive name info
	dwLength=RegKey.QueryValueLength("ArchiveName");
	if (dwLength>1)
	{
		pDatabase->m_szArchiveName=new WCHAR[dwLength];
		RegKey.QueryValue(L"ArchiveName",pDatabase->m_szArchiveName,dwLength);
	}

    // Copying creator info
	dwLength=RegKey.QueryValueLength(L"Creator");
	if (dwLength>1)
	{
		pDatabase->m_szCreator=new WCHAR[dwLength];
		RegKey.QueryValue(L"Creator",pDatabase->m_szCreator,dwLength);
	}

	// Copying description
	dwLength=RegKey.QueryValueLength("Description");
	if (dwLength>1)
	{
		pDatabase->m_szDescription=new WCHAR[dwLength];
		RegKey.QueryValue(L"Description",pDatabase->m_szDescription,dwLength);
	}

	// Copying roots
	dwLength=RegKey.QueryValueLength(L"Roots");
	if (dwLength>1)
	{
		pDatabase->m_szRoots=new WCHAR[dwLength];
		RegKey.QueryValue(L"Roots",pDatabase->m_szRoots,dwLength);
	}

	// Excluded directories
	dwLength=RegKey.QueryValueLength("Excluded Directories");
	if (dwLength>1)
	{
		LPWSTR pString=new WCHAR[dwLength];
		RegKey.QueryValue(L"Excluded Directories",pString,dwLength);

		LPCWSTR pPtr=pString;
		while (*pPtr!=L'\0')
		{
			int nIndex=FirstCharIndex(pPtr,L';');
			if (nIndex!=-1)
			{
				pDatabase->m_aExcludedDirectories.Add(alloccopy(pPtr,nIndex));
                pPtr+=nIndex+1;
			}
			else
			{
				pDatabase->m_aExcludedDirectories.Add(alloccopy(pPtr));
				break;
			}
		}

		delete[] pString;
	}

	return pDatabase;
}

CDatabase* CDatabase::FromOldStyleDatabase(HKEY hKeyRoot,LPCSTR szPath)
{
	CRegKey RegKey;
	
	// Try to open key
	if (RegKey.OpenKey(hKeyRoot,szPath,CRegKey::defRead)!=ERROR_SUCCESS)
		return NULL;
	
	// Try to retrieve database file
	DWORD dwLength=RegKey.QueryValueLength("DatabaseFile");
	if (dwLength==0)
		return NULL;

    // Old style database info found, loading it
	CDatabase* pDatabase=new CDatabase;
	
	pDatabase->m_ArchiveType=CDatabase::archiveFile;
	pDatabase->m_szArchiveName=new WCHAR[dwLength];
	RegKey.QueryValue(L"DatabaseFile",pDatabase->m_szArchiveName,dwLength);

	
	// Setting name to "default"
	pDatabase->m_szName=new WCHAR[8];
	MemCopyW(pDatabase->m_szName,L"default",8);
	
	// Copying creator info
	dwLength=RegKey.QueryValueLength("Default Creator");
	if (dwLength>1)
	{
		pDatabase->m_szCreator=new WCHAR[dwLength];
		RegKey.QueryValue(L"Default Creator",pDatabase->m_szCreator,dwLength);
	}
	
	// Copying description
	dwLength=RegKey.QueryValueLength("Default Description");
	if (dwLength>1)
	{
		pDatabase->m_szDescription=new WCHAR[dwLength];
		RegKey.QueryValue(L"Default Description",pDatabase->m_szDescription,dwLength);
	}

	// Copying roots
	dwLength=RegKey.QueryValueLength(L"Drives");
	if (dwLength>1)
	{
		pDatabase->m_szRoots=new WCHAR[dwLength];
		RegKey.QueryValue(L"Drives",pDatabase->m_szRoots,dwLength);
	}
	
	return pDatabase;
}

CDatabase* CDatabase::FromFile(LPCWSTR szFileName,int dwNameLength)
{
	LPWSTR szFile=GetCorrertFileName(szFileName,dwNameLength);
	if (szFile==NULL)
		return NULL;

    CDatabase* pDatabase=new CDatabase;

	
	
	pDatabase->m_szName=new WCHAR[6];
	MemCopyW(pDatabase->m_szName,L"param",6);

	pDatabase->m_ArchiveType=archiveFile;
	pDatabase->m_szArchiveName=szFile;

	return pDatabase;
}

CDatabase* CDatabase::FromDefaults(BOOL bDefaultFileName,LPCWSTR szAppDir,SIZE_T iAppDirLength)
{
	CDatabase* pDatabase=new CDatabase; // This default dwFlags and description and drives to NULL

	pDatabase->m_szName=new WCHAR[8];
	MemCopyW(pDatabase->m_szName,L"default",8);

	pDatabase->m_ArchiveType=CDatabase::archiveFile;
	
	if (bDefaultFileName)
	{
		if (iAppDirLength==SIZE_T(-1))
			iAppDirLength=istrlenw(szAppDir);

		pDatabase->m_szArchiveName=new WCHAR[iAppDirLength+10];
		MemCopyW(pDatabase->m_szArchiveName,szAppDir,iAppDirLength);
		MemCopyW(pDatabase->m_szArchiveName+iAppDirLength,L"files.dbs",10);

	}
	else
		pDatabase->m_szArchiveName=NULL;
	return pDatabase;
}

/* Example
 $$LDBSET$T:0000$F:E1G1S0I0$N:local$AF:C:\Utils\db\files.dbs$C:jmj@iik$D:All local files$R:1$$
*/

CDatabase* CDatabase::FromExtraBlock(LPCWSTR szExtraBlock)
{
	
	for (int i=0;szExtraBlock[i]!='\0';i++)
	{
		// Finding "$$LDBSET$"
		if (wcsncmp(szExtraBlock+i,L"$$LDBSET$",9)!=0)
			continue;
		
        // Found
		LPCWSTR pPtr=szExtraBlock+i+9;
        int length,keylen;

		CDatabase* pDatabase=new CDatabase;
		CArrayFAP<LPWSTR> aRoots;
		

		for (;;)
		{
			// Counting field length
            for (length=0;pPtr[length]!=L'$' && pPtr[length]!=L'$';length++);
			if (length==0)
				break;

			// Find ':'
			for (keylen=0;keylen<length && pPtr[keylen]!=L':';keylen++);
			if (pPtr[keylen]!=L':')
			{
				// Not correct field
				pPtr+=length+1;
				continue;
			}
			CStringW sValue(pPtr+keylen+1,length-keylen-1);

			switch (*pPtr)
			{
			case L'T': // Thread
			case L't': 
				{
					// Checking zeroes
					int i;
					for (i=0;sValue[i]=='0';i++);
					LPWSTR pTemp;
					pDatabase->m_wThread=(WORD)wcstoul(LPCWSTR(sValue)+i,&pTemp,16);
                    break;
				}
			case L'F': // Flags
			case L'f':
				{
					LPCWSTR pTemp=sValue;
					while (*pTemp!=L'\0')
					{
						switch (*(pTemp++))
						{
						case L'E':
						case L'e':
							if (*pTemp==L'1')
							{
								pDatabase->m_wFlags|=flagEnabled;
								pTemp++;
							}
							else if (*pTemp=='0')
							{
								pDatabase->m_wFlags&=~flagEnabled;
								pTemp++;
							}
							else
								pDatabase->m_wFlags|=flagEnabled;
							break;
						case L'G':
						case L'g':
							if (*pTemp==L'1')
							{
								pDatabase->m_wFlags|=flagGlobalUpdate;
								pTemp++;
							}
							else if (*pTemp==L'0')
							{
								pDatabase->m_wFlags&=~flagGlobalUpdate;
								pTemp++;
							}
							else
								pDatabase->m_wFlags|=flagGlobalUpdate;
							break;
							break;
						case L'S':
						case L's':
							if (*pTemp==L'1')
							{
								pDatabase->m_wFlags|=flagStopIfRootUnavailable;
								pTemp++;
							}
							else if (*pTemp==L'0')
							{
								pDatabase->m_wFlags&=~flagStopIfRootUnavailable;
								pTemp++;
							}
							else
								pDatabase->m_wFlags|=flagStopIfRootUnavailable;
							break;
						case L'I':
						case L'i':
							if (*pTemp==L'1')
							{
								pDatabase->m_wFlags|=flagIncrementalUpdate;
								pTemp++;
							}
							else if (*pTemp==L'0')
							{
								pDatabase->m_wFlags&=~flagIncrementalUpdate;
								pTemp++;
							}
							else
								pDatabase->m_wFlags|=flagIncrementalUpdate;
							break;
						}
					}
					break;
				}
			case L'N': // Name
			case L'n':
				pDatabase->m_szName=sValue.GiveBuffer();
				break;
			case L'A': // Archive
			case L'a':
				if (pPtr[1]==L'F')
				{
					// File:
					pDatabase->m_ArchiveType=CDatabase::archiveFile;
					pDatabase->m_szArchiveName=sValue.GiveBuffer();
				}
				break;
			case L'C': // Creator
			case L'c':
				pDatabase->m_szCreator=sValue.GiveBuffer();
				break;
			case L'D': // Description
			case L'd':
				pDatabase->m_szDescription=sValue.GiveBuffer();
				break;
			case L'R':
			case L'r':
				if (sValue.Compare(L"1")==0)
					aRoots.RemoveAll();
				else
					aRoots.Add(sValue.GiveBuffer());
				break;
			case L'E':
			case L'e':
                pDatabase->m_aExcludedDirectories.Add(sValue.GiveBuffer());
				break;
			}

			pPtr+=length+1;
				
		}

		pDatabase->SetRoots(aRoots);

		return pDatabase;
	}
	return NULL;
}

LPWSTR CDatabase::GetCorrertFileName(LPCWSTR szFileName,SIZE_T dwNameLength)
{
	if (dwNameLength==SIZE_T(-1))
		dwNameLength=istrlenw(szFileName);
	
	LPWSTR szFile;
	if (szFileName[0]!=L'\\' && szFileName[1]!=L':')
	{
		DWORD dwLength=GetCurrentDirectory(0,NULL);
        if (dwLength==0)
			return NULL;
	
		szFile=new WCHAR[dwLength+dwNameLength+2];
		CFile::GetCurrentDirectory(dwLength,szFile);
		dwLength--;
		if (szFile[dwLength-1]!='\\')
			szFile[dwLength++]='\\';
		sMemCopy(szFile+dwLength,szFileName,DWORD(dwNameLength));
		szFile[dwLength+dwNameLength]='\0';
	}
	else
	{
		szFile=new WCHAR[dwNameLength+1];
		MemCopyW(szFile,szFileName,DWORD(dwNameLength));
		szFile[dwNameLength]=L'\0';
	}

	// Checking whether file exists
	if (!CFile::IsValidFileName(szFile))
	{
		delete[] szFile;
		return NULL;
	}
	return szFile;
}

void CDatabase::CheckDoubleNames(PDATABASE* ppDatabases,int nDatabases)
{
	for (int i=1;i<nDatabases;i++)
	{
		SIZE_T dwLength=istrlenw(ppDatabases[i]->m_szName);
		
		for (int j=0;j<i;j++)
		{
			if (strcasencmp(ppDatabases[i]->m_szName,ppDatabases[j]->m_szName,dwLength+1)==0)
			{
				if (ppDatabases[i]->m_szName[dwLength-1]>='0' && ppDatabases[i]->m_szName[dwLength-1]<'9')
					ppDatabases[i]->m_szName[dwLength-1]++;
				else if (ppDatabases[i]->m_szName[dwLength-1]=='9')
				{
					if (ppDatabases[i]->m_szName[dwLength-2]>='1' && ppDatabases[i]->m_szName[dwLength-2]<'9')
						ppDatabases[i]->m_szName[dwLength-2]++;
					else
					{
						WCHAR* tmp=new WCHAR[dwLength+2];
						MemCopyW(tmp,ppDatabases[i]->m_szName,dwLength-1);
						delete[] ppDatabases[i]->m_szName;
						tmp[dwLength-1]=L'1';
						tmp[dwLength]=L'0';
						tmp[dwLength+1]=L'\0';
						ppDatabases[i]->m_szName=tmp;
					}
				}
				else
				{
					WCHAR* tmp=new WCHAR[dwLength+2];
					MemCopyW(tmp,ppDatabases[i]->m_szName,dwLength);
					tmp[dwLength++]=L'1';
                    tmp[dwLength]=L'\0';
					delete[] ppDatabases[i]->m_szName;
                    ppDatabases[i]->m_szName=tmp;
				}
				
				dwLength=istrlenw(ppDatabases[i]->m_szName);
				j=-1;
			}			
		}
	}
}



void CDatabase::CheckValidNames(PDATABASE* ppDatabases,int nDatabases)
{
	for (int i=0;i<nDatabases;i++)
	{
		if (!IsNameValid(ppDatabases[i]->m_szName))
		{
			MakeNameValid(ppDatabases[i]->m_szName);
			
			// Unallocaling unnecessary memory
			WCHAR* pNew=alloccopy(ppDatabases[i]->m_szName);
			delete[] ppDatabases[i]->m_szName;
			ppDatabases[i]->m_szName=pNew;
		}
	}
}


#define ISVALIDFORNAME(a) \
	( (a)!=L'\\' && (a)!=L'\"' && (a)!=L'\'' )

#define ISVALIDFORKEY(a) \
	( ((a)>=L'0' && (a)<=L'9') || \
	  ((a)>=L'a' && (a)<=L'z') || \
	  ((a)>=L'A' && (a)<=L'A') || \
	  (a)==L' ' || \
	  (a)==L'#' || \
	  (a)==L'-' || \
	  (a)==L'_' )
	
BOOL CDatabase::IsNameValid(LPCWSTR szName)
{
	int i;
	for (i=0;szName[i]!=L'\0';i++)
	{
		if (!ISVALIDFORNAME(szName[i]))
			return FALSE;
	}
	return i>0;
}

void CDatabase::MakeNameValid(LPWSTR szName)
{
	int i,j=0;
	for (i=0;szName[i]!=L'\0';i++)
	{
		if (ISVALIDFORNAME(szName[i]))
			szName[j++]=szName[i];
	}
	szName[j]=L'\0';
}

LPSTR CDatabase::GetValidKey(DWORD dwUniqueNum) const
{
	DWORD dwValid=0;
	for (int i=0;m_szName[i]!='\0';i++)
	{
		if (ISVALIDFORKEY(m_szName[i]))
			dwValid++;
	}

	if (dwValid==0)
	{
		if (dwUniqueNum>0)
		{
			char* key=new char[25];
			StringCbPrintf(key,25,"db%d",dwUniqueNum);
			return key;            
		}
		else
			return alloccopy("db");
	}

	CHAR* key=new char[dwValid+1];
	int i,j;
	for (i=0,j=0;m_szName[i]!='\0';i++)
	{
		if (ISVALIDFORKEY(m_szName[i]))
			MemCopyWtoA(key+(j++),m_szName+i,1);
	}
	key[j]='\0';
	return key;
}

WORD CDatabase::CheckIDs(PDATABASE* ppDatabases,int nDatabases)
{
	int nCurrentlyMustBe=0,index;
	
	for (index=0;index<nDatabases;index++)
	{
		if (ppDatabases[index]->m_wID==0)
			ppDatabases[index]->m_wID=GetUniqueIndex(ppDatabases,nDatabases);
	}

	index=0;
	while (index<nDatabases)
	{
		int nCurrentThreadID=ppDatabases[index]->m_wThread;
		if (nCurrentThreadID!=nCurrentlyMustBe)
		{
			// Make databases with current ID to nCurrentlyMustBe
			ppDatabases[index++]->m_wThread=nCurrentlyMustBe;

            while (index<nDatabases && ppDatabases[index]->m_wThread==nCurrentThreadID)
				ppDatabases[index++]->m_wThread=nCurrentlyMustBe;
		}
		else
		{
			for (;index<nDatabases && ppDatabases[index]->m_wThread==nCurrentThreadID;index++);
		}
		nCurrentlyMustBe++;
	}
	return nCurrentlyMustBe;
}

void CDatabase::GetRoots(CArray<LPWSTR>& aRoots) const
{
	if (m_szRoots!=NULL)
	{
		LPCWSTR pPtr=m_szRoots;
		while (*pPtr!=L'\0')
		{
			SIZE_T dwLength=istrlenw(pPtr);
			aRoots.Add(alloccopy(pPtr,dwLength));
			pPtr+=dwLength;
		}
	}
}

void CDatabase::SetRoots(LPWSTR* pRoots,int nCount)
{
	if (m_szRoots!=NULL)
		delete[] m_szRoots;

	if (nCount==0)
	{
		m_szRoots=NULL;
		return;
	}

	// Counting required buffer size
	DWORD dwBufferSize=0;
	SIZE_T* dwLengths=new SIZE_T[nCount];

	int i;
	for (i=0;i<nCount;i++)
	{
		dwLengths[i]=istrlenw(pRoots[i]);
		dwBufferSize+=++dwLengths[i];
	}
	
	m_szRoots=new WCHAR[dwBufferSize+1];
	LPWSTR pPtr=m_szRoots;
	for (i=0;i<nCount;i++)
	{
		MemCopyW(pPtr,pRoots[i],dwLengths[i]);
		pPtr+=dwLengths[i];
	}
	*pPtr=L'\0';

	delete[] dwLengths;
}

CDatabase* CDatabase::FindByName(PDATABASE* ppDatabases,int nDatabases,LPCWSTR szName,SIZE_T iLength)
{
	CStringW sName(szName,iLength);
	sName.MakeLower();

	for (int i=0;i<nDatabases;i++)
	{
		CStringW str(ppDatabases[i]->GetName());
		str.MakeLower();        

		if (sName.Compare(str)==0)
			return ppDatabases[i];
	}
	return NULL;
}

CDatabase* CDatabase::FindByFile(PDATABASE* ppDatabases,int nDatabases,LPCWSTR szFile,SIZE_T iLength)
{
	WCHAR* pPath1=NULL;
	WCHAR szPath1[MAX_PATH]; 

	if (iLength==0)
		return NULL;

	DWORD dwRet;

	if (szFile[0]!=L'\\' && szFile[1]!=L':')
	{
		if (iLength==SIZE_T(-1))
			iLength=istrlenw(szFile);
			
		SIZE_T dwLength=GetCurrentDirectory(0,NULL);
		if (dwLength==0)
			return NULL;
       
		pPath1=new WCHAR[dwLength+iLength+2];
		CFile::GetCurrentDirectory(dwLength,pPath1);
		dwLength--;
		if (pPath1[dwLength-1]!=L'\\')
			pPath1[dwLength++]=L'\\';

		MemCopyW(pPath1+dwLength,szFile,DWORD(iLength));
		pPath1[dwLength+iLength]=L'\0';

		dwRet=CFile::GetShortPathName(pPath1,szPath1,MAX_PATH);
	}
	else if (iLength!=-1)
	{
		pPath1=new WCHAR[iLength+1];
		MemCopyW(pPath1,szFile,DWORD(iLength));
		pPath1[iLength]=L'\0';
		dwRet=CFile::GetShortPathName(pPath1,szPath1,MAX_PATH);
	}
	else
		dwRet=CFile::GetShortPathName(szFile,szPath1,MAX_PATH);
	
	// File does not exists
	if (dwRet==0)
	{
		if (pPath1==NULL)
			pPath1=const_cast<LPWSTR>(szFile);
		
		for (int i=0;i<nDatabases;i++)
		{
			if (!CFile::IsFile(ppDatabases[i]->m_szArchiveName))
			{
				if (strcasecmp(pPath1,ppDatabases[i]->m_szArchiveName)==0)
				{
					if (pPath1!=szFile)
						delete[] pPath1;
					return ppDatabases[i];
				}
			}
		}
		if (pPath1!=szFile)
			delete[] pPath1;
		return NULL;
	}

	if (pPath1!=NULL)
		delete[] pPath1;
	for (int i=0;i<nDatabases;i++)
	{
		WCHAR szPath2[MAX_PATH];
		dwRet=CFile::GetShortPathName(ppDatabases[i]->m_szArchiveName,szPath2,MAX_PATH);

		if (dwRet!=NULL)
		{
			if (strcasecmp(szPath1,szPath2)==0)
				return ppDatabases[i];
		}
	}
	return NULL;
}

WORD CDatabase::GetUniqueIndex(PDATABASE* ppDatabases,int nDatabases)
{
	WORD wNewIndex,i;

	do
	{
		wNewIndex=LOWORD(GetTickCount());

		if (wNewIndex==0)
			continue;

        for (i=0;i<WORD(nDatabases);i++)
		{
			if (ppDatabases[i]->m_wID==wNewIndex)
				break;
		}
	}
	while (i<WORD(nDatabases));
	return wNewIndex;
}

BOOL CDatabase::IsFileNamesOEM() const
{
#ifdef WIN32
	BYTE* szBuffer=new BYTE[11];
	BOOL bOEM=-1;
	CFile* dbFile=NULL;

	try 
	{
		switch (m_ArchiveType)
		{
		case archiveFile:
			dbFile=new CFile(GetArchiveName(),CFile::defRead,TRUE);
			break;
		default:
			return -1;
		}
		
		// Reading and verifing header
		dbFile->Read(szBuffer,11);
		if (szBuffer[0]=='L' && szBuffer[1]=='O' &&
			szBuffer[2]=='C' && szBuffer[3]=='A' && 
			szBuffer[4]=='T' && szBuffer[5]=='E' && 
			szBuffer[6]=='D' && szBuffer[7]=='B' &&
			szBuffer[8]=='2' && szBuffer[9]=='0' )
		{		
			bOEM=!(szBuffer[10]&0x10);
		}
		dbFile->Close();
	}
	catch (...)
	{
	}
	
	delete[] szBuffer;
	
	if (dbFile!=NULL)
		delete dbFile;
	return bOEM;
#else
	return TRUE;
#endif
}

void CDatabase::AddLocalRoots()
{
	CArrayFAP<LPWSTR> aLocalRoots;
	GetLogicalDrives(&aLocalRoots);

	if (m_szRoots==NULL)
	{
		CIntArray aLengths;
		SIZE_T dwDataLength=0;
		int i;
		for (i=0;i<aLocalRoots.GetSize();i++)
		{
			SIZE_T iLength=istrlenw(aLocalRoots[i]);
			aLengths.Add(++iLength);
			dwDataLength+=iLength;
		}
		m_szRoots=new WCHAR[++dwDataLength];
		LPWSTR ptr=m_szRoots;
		for (i=0;i<aLocalRoots.GetSize();i++)
		{
			CopyMemory(ptr,aLocalRoots[i],aLengths[i]);
			ptr+=aLengths[i];
		}
		*ptr=L'\0';
		return;
	}

	for (int i=0;i<aLocalRoots.GetSize();i++)
		AddRoot(aLocalRoots[i]);
}

void CDatabase::AddRoot(LPCWSTR pRoot)
{
	SIZE_T dwLength=istrlenw(pRoot);
	
	if (m_szRoots==NULL)
	{
		m_szRoots=new WCHAR[dwLength+2];
		MemCopyW(m_szRoots,pRoot,dwLength);
		m_szRoots[dwLength++]=L'\0';
		m_szRoots[dwLength++]=L'\0';
		return;
	}
	
	// Counting previous data length;
	DWORD dwDataLength;
	for (dwDataLength=0;m_szRoots[dwDataLength]!='\0' || m_szRoots[dwDataLength+1]!='\0';dwDataLength++)
	{
		if (strcasecmp(m_szRoots+dwDataLength,pRoot)==0)
			return; // Already exist
	}
	dwDataLength++;

	LPWSTR pNew=new WCHAR[dwDataLength+dwLength+2];
    MemCopyW(pNew,m_szRoots,dwDataLength);
	MemCopyW(pNew+dwDataLength,pRoot,dwLength);
	dwDataLength+=dwLength;
	pNew[dwDataLength++]='\0';
	pNew[dwDataLength++]='\0';

	delete[] m_szRoots;
	m_szRoots=pNew;
}

#ifdef WIN32

void CDatabase::GetLogicalDrives(CArrayFAP<LPWSTR>* paRoots)
{
	paRoots->RemoveAll();
	DWORD dwBufferLen=GetLogicalDriveStrings(0,NULL)+1;
	
	if (IsFullUnicodeSupport())
	{
		WCHAR* szDrives=new WCHAR[dwBufferLen];
		GetLogicalDriveStringsW(dwBufferLen,szDrives);

		for (int i=0;szDrives[i*4]!=L'\0';i++)
		{
			if (GetDriveTypeW(szDrives+i*4)==DRIVE_FIXED)
			{
				WCHAR* tmp=new WCHAR[3];
				tmp[0]=szDrives[i*4];
				tmp[1]=':';
				tmp[2]='\0';
				paRoots->Add(tmp);
			}
		}
		delete[] szDrives;
	}
	else
	{
		CHAR* szDrives=new CHAR[dwBufferLen];
		GetLogicalDriveStrings(dwBufferLen,szDrives);

		for (int i=0;szDrives[i*4]!='\0';i++)
		{
			if (GetDriveTypeA(szDrives+i*4)==DRIVE_FIXED)
			{
				WCHAR* tmp=new WCHAR[3];
				MemCopyAtoW(tmp,szDrives+i*4,1);
				tmp[1]=L':';
				tmp[2]=L'\0';
				paRoots->Add(tmp);
			}
		}
		delete[] szDrives;
	}

}

#else

void CDatabase::GetLogicalDrives(CArrayFAP<LPSTR>* paRoots)
{            
	char szDrives[27];
	getlocaldrives(szDrives);
	for (int i=0;i<27;i++)
	{
		if (szDrives[i])
		{
			char* pRoot=new char[3];
			pRoot[0]=i+'A';
			pRoot[1]=':';
			pRoot[2]='\0';
			paRoots->Add(pRoot);
		}
	}
}


/*
void CDatabase::ReadIniFile(BYTE* pData,CArrayFAP<LPSTR>* paRoots,CString* pCreator,CString* pDescription,CString* pDatabaseFile)
{
    int i;
    while (1)
    {
        LPCSTR szData=LPCSTR(pData);
    	while (szData[0]==' ' || szData[0]=='\n' || szData[0]=='\r') szData++;
        for (i=0;szData[i]!='=' && szData[i]!='\0';i++);
        if (szData[i]=='\0')
            return;    
        pData[i]='\0';
        if (strcasecmp(szData,"DatabaseFile")==0)
        {
            szData+=i+1;
            if (pDatabaseFile==NULL)
            {
				while (szData[0]!='\n' && szData[0]!='\0') szData++;
			}
            else
            {
				pDatabaseFile->Empty();
				for (i=0;szData[i]!='\0' && szData[i]!='\n'&& szData[i]!='\r';i++)
                    *pDatabaseFile<<szData[i];
            }
            if (szData[0]=='\0')
                return;
            szData++;
        }
        else if (strcasecmp(szData,"Creator")==0)
        {
            szData+=i+1;
            if (pCreator==NULL)
            {
				while (szData[0]!='\n' && szData[0]!='\0') szData++;
			}
            else
            {
				pCreator->Empty();
				for (i=0;szData[i]!='\0' && szData[i]!='\n'&& szData[i]!='\r';i++)
                    *pCreator<<szData[i];
            }
            if (szData[0]=='\0')
                return;
            szData++;
        }
        else if (strcasecmp(szData,"Description")==0)
        {
            szData+=i+1;
            if (pDescription==NULL)
            {
				while (szData[0]!='\n' && szData[0]!='\0') szData++;
			}
            else
            {
				pDescription->Empty();
				for (i=0;szData[i]!='\0' && szData[i]!='\n'&& szData[i]!='\r';i++)
                    *pDescription<<szData[i];
            }
            if (szData[0]=='\0')
                return;
            szData++;
        }
        else if (strcasecmp(szData,"Drives")==0)
        {
            szData+=i+1;
            if (paRoots==NULL)
            {
                while (szData[0]!='\n' && szData[0]!='\0') szData++;
            }
            else
            {
                paRoots->RemoveAll();

				while (szData[i]!='\n' && szData[i]!='\r' && szData[i]!='\0')
				{
					DWORD dwLength;
					for (dwLength=0;szData[dwLength]!='\r' && szData[dwLength]!='\n' && szData[dwLength]!='\0' && szData[dwLength]!=';';dwLength++);
					LPSTR pRoot=new char[dwLength+1];
					sMemCopy(pRoot,szData,dwLength);
					pRoot[dwLength]='\0';
					paRoots->Add(pRoot);
					szData+=dwLength;
					if (szData[i]==';')
						szData++;
				}
            }
            if (szData[0]=='\0')
                return;
            szData++;
        }
        while (szData[0]=='\n' || szData[0]=='\r')
        {
            if (szData[0]=='\0')
                return;
            szData++;   
        }     
    }
}
*/

#endif

LPWSTR CDatabase::ConstructExtraBlock(DWORD* pdwLen) const
{
	CStringW str;
	str.Format(L"$$LDBSET$T:%04X$",m_wThread);
	
	// Flags
	if (m_wFlags!=0)
	{
		str << L"F:";
		
		if (m_wFlags&flagEnabled)
			str << L"E1";
		else
			str << L"E0";
		if (m_wFlags&flagGlobalUpdate)
			str << L"G1";
		else
			str << L"G0";
		if (m_wFlags&flagStopIfRootUnavailable)
			str << L"S1";
		else
			str << L"S0";
		if (m_wFlags&flagIncrementalUpdate)
			str << L"I1";
		else
			str << L"I0";
		
		str << L'$';
	}

	if (m_szName!=NULL)
	{
		// Name
		str << L"N:";
		for (int i=0;m_szName[i]!=L'\0';i++)
		{
			if (m_szName[i]==L'$')
				str << L'\\';
			str << m_szName[i];
		}
		str << L'$';
	}

	if (m_ArchiveType==archiveFile && m_szArchiveName!=NULL)
	{
		// Archive file
		str << L"AF:";
		for (int i=0;m_szArchiveName[i]!=L'\0';i++)
		{
			if (m_szArchiveName[i]==L'$')
				str << L'\\';
			str << m_szArchiveName[i];
		}
		str << L'$';
	}

	if (m_szCreator!=NULL)
	{
		// Creator
		str << L"C:";
		for (int i=0;m_szCreator[i]!=L'\0';i++)
		{
			if (m_szCreator[i]==L'$')
				str << L'\\';
			str << m_szCreator[i];
		}
		str << L'$';
	}
	if (m_szDescription!=NULL)
	{
		// Description
		str << L"D:";
		for (int i=0;m_szDescription[i]!=L'\0';i++)
		{
			if (m_szDescription[i]==L'$')
				str << L'\\';
			str << m_szDescription[i];
		}
		str << L'$';
	}

	// Roots
	if (m_szRoots==NULL)
		str << L"R:1$";
	else
	{
		LPWSTR pStr=m_szRoots;
		while (*pStr!=L'\0')
		{
			str << L"R:";
			while (*pStr!=L'\0')
			{
				if (*pStr==L'$')
					str << L'$';
				str << *pStr;
				pStr++;
			}
			str << L'$';
			pStr++;
		}
	}

	// Excluded directories
	for (int i=0;i<m_aExcludedDirectories.GetSize();i++)
	{
		str << L"E:";
		for (int j=0;m_aExcludedDirectories[i][j]!=L'\0';j++)
		{
			if (m_aExcludedDirectories[i][j]==L'$')
				str << L'\\';
			str << m_aExcludedDirectories[i][j];
		}
		str << L'$';
	
	}

	str << L'$';
	str.FreeExtra();

	if (pdwLen!=NULL)
		*pdwLen=str.GetLength();
	return str.GiveBuffer();
}



BOOL CDatabase::SaveExtraBlockToDbFile(LPCWSTR szArchive)
{
	
	CFile *pInFile=NULL,*pOutFile=NULL;
	char* szBuffer=NULL;

	LPWSTR szExtra=ConstructExtraBlock();
	DWORD iExtraLen=istrlenw(szExtra)+1;

	// Constructing temp file name	
	WCHAR szTempFile[MAX_PATH];
	{
		CStringW CurPath(szArchive,LastCharIndex(szArchive,L'\\'));
		
		if (!CFile::GetTempFileName(CurPath,L"_dbtmp",0,szTempFile))
			return FALSE;
	}

	BOOL bRet=TRUE;
	DWORD dwTemp;
	CString Creator,Description,Extra1,Extra2;
	

	try {
		// Opening database and trying to read header
		pInFile=new CFile(szArchive,CFile::defRead|CFile::otherStrNullTerminated,TRUE);

		szBuffer=new char[12];
		pInFile->Read(szBuffer,11);

		if (szBuffer[0]!='L' || szBuffer[1]!='O' || 
			szBuffer[2]!='C' || szBuffer[3]!='A' || 
			szBuffer[4]!='T' || szBuffer[5]!='E' || 
			szBuffer[6]!='D' || szBuffer[7]!='B' ||
			szBuffer[8]!='2' || szBuffer[9]!='0' )
		{
			throw CFileException(CFileException::invalidFile,
#ifdef WIN32
				-1,
#endif
				szArchive);
		}

		// Reading fields
		pInFile->Read(dwTemp); // Header size
		pInFile->Read(Creator);
		pInFile->Read(Description);
		pInFile->Read(Extra1);
		pInFile->Read(Extra2);

		pOutFile=new CFile(szTempFile,CFile::defWrite|CFile::otherStrNullTerminated,TRUE);
		
		// Writing identification, '\17=0x11=0x10|0x1' 0x1 = Long filenames and 0x10 = ANSI
		pOutFile->Write("LOCATEDB20",10);
		pOutFile->Write(BYTE(0x11));

        // Writing header size
		pOutFile->Write(DWORD(
			Creator.GetLength()+1+ // Author data
			Description.GetLength()+1+ // Comments data
			Extra1.GetLength()+1+
			iExtraLen+ // Extra
			4+ // Time
			4+ // Number of files
			4  // Number of directories
			));

        pOutFile->Write(Creator);
		pOutFile->Write(Description);
		pOutFile->Write(Extra1);
		pOutFile->Write(szExtra,iExtraLen);
		
		// TIME, FILE and DIRECTORY counts
		pInFile->Read(szBuffer,3*sizeof(DWORD));
		pOutFile->Write(szBuffer,3*sizeof(DWORD));

        // Roots
		pInFile->Read(dwTemp);
		while (dwTemp!=0)
		{
			pOutFile->Write(dwTemp);

			delete[] szBuffer;
			szBuffer=new char[dwTemp];
			pInFile->Read(szBuffer,dwTemp);
			pOutFile->Write(szBuffer,dwTemp);

			pInFile->Read(dwTemp);
		}

		pOutFile->Write((DWORD)0);

		pInFile->Close();
		pOutFile->Close();

		CFile::Remove(szArchive);
		CFile::MoveFile(szTempFile,szArchive);
	}
	catch (...)
	{
		bRet=FALSE;
	}
    
	if (pInFile!=NULL)
		delete pInFile;
	if (pOutFile!=NULL)
		delete pOutFile;
	if (szBuffer!=NULL)
		delete[] szBuffer;
	delete[] szExtra;

	
	// Delete temp file 
	CFile::Remove(szTempFile);
	return bRet;
}
