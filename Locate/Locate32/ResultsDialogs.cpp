/* Locate32 - Copyright (c) 1997-2009 Janne Huttunen */

#include <HFCLib.h>
#include "Locate32.h"
#include <wchar.h>

BOOL CResults::Value::IsInteger() const
{
	if (nType==Operator)
		return FALSE;
	if (nType==Integer || pString==NULL) // NULL is considered as 0
		return TRUE;
	
	// Skip leading spaces
	int i=0;
	for (;pString[i]==' ' || pString[i]=='\t';i++);
		
	for (;pString[i]!='\0';i++)
	{
		if (pString[i]<'0' || pString[i]>'9')
		{
			// Rest string is spaces?
			if (i>0)
			{
				for (;pString[i]==' ' || pString[i]=='\t';i++);
				return pString[i]=='\0';
			}
			
			return FALSE;
		}
	}
	return TRUE;
}

CResults::Value::operator LPCWSTR() const
{
	if (nType==String)
		return pString;
	return NULL;
}


void CResults::Value::GetString(CStringW& str) const
{
	ASSERT(nType!=Operator);

	if (nType==String && pString!=NULL)
		str.Copy(pString);
	else if (nType==Integer)
	{
		WCHAR buffer[20];
		_itow_s(nInteger,buffer,20,10);
		str.Copy(buffer);
	}
}

BOOL CResults::Value::AddString(LPCWSTR pAdd)
{
	if (nType==Operator || nType==Integer)
		return FALSE;

	int nOldLen=istrlen(pString);
	int nLen=istrlen(pAdd);
	LPWSTR pNewValue=new WCHAR[nOldLen+nLen+1];
	MemCopyW(pNewValue,pString,nOldLen);
	MemCopyW(pNewValue+nOldLen,pAdd,nLen);
	pNewValue[nOldLen+nLen]='\0';
	delete[] pString;
	pString=pNewValue;
	return TRUE;
}

BOOL CResults::Value::AddString(LPCWSTR pAdd,size_t nLen)
{
	if (IsOperator() || IsInteger())
		return FALSE;

	int nOldLen=istrlen(pString);
	LPWSTR pNewValue=new WCHAR[nOldLen+nLen+1];
	MemCopyW(pNewValue,pString,nOldLen);
	MemCopyW(pNewValue+nOldLen,pAdd,nLen);
	pNewValue[nOldLen+nLen]='\0';
	delete[] pString;
	pString=pNewValue;
	return TRUE;
}

UINT CResults::CValueStream::ParseOperatorsFromString(LPCWSTR pString,BOOL bCanHavePtr)
{
	LPCWSTR pPtr=pString;
	while (*pPtr!='\0')
	{
		int nLen;
		CResults::Value::OperatorType nOperator=CResults::Value::None;
		int nOperatorLen=1;

		for (nLen=0;pPtr[nLen]!='\0';nLen++)
		{
			switch (pPtr[nLen])
			{
			case '!':
				if (pPtr[nLen+1]=='=')
				{
					nOperator=CResults::Value::NotEqual;
					nOperatorLen=2;
				}
				else
					nOperator=CResults::Value::Not;
				break;
			case '=':
				if (pPtr[nLen+1]=='=')
					nOperatorLen=2;
				nOperator=CResults::Value::Equal;
				break;
			case '<':
				if (pPtr[nLen+1]=='=')
				{
					nOperator=CResults::Value::LessOrEqual;
					nOperatorLen=2;
				}
				else
					nOperator=CResults::Value::Less;
				break;
			case '>':
				if (pPtr[nLen+1]=='=')
				{
					nOperator=CResults::Value::GreaterOrEqual;
					nOperatorLen=2;
				}
				else
					nOperator=CResults::Value::Greater;
				break;
			case '+':
				nOperator=CResults::Value::Add;
				break;
			case '-':
				nOperator=CResults::Value::Subtract;
				break;
			case '*':
				nOperator=CResults::Value::Multiply;
				break;
			case '/':
				if (pPtr[nLen+1]=='/')
				{
					nOperator=CResults::Value::DivideAndRoundToInfinity;
					nOperatorLen=2;
				}
				else
					nOperator=CResults::Value::Divide;
				break;
			case '%':
				nOperator=CResults::Value::Remainder;
				break;
			case '&':
				if (pPtr[nLen+1]=='&')
					nOperatorLen=2;
				nOperator=CResults::Value::And;
				break;
			case '|':
				if (pPtr[nLen+1]=='|')
					nOperatorLen=2;
				nOperator=CResults::Value::Or;
				break;
			}

			if (nOperator!=CResults::Value::None)
				break;
		}

		if (nOperator==CResults::Value::None) // End of string?
		{
			if (pPtr==pString && bCanHavePtr)
			{
				// Just one pointer
				AddToListPtr(pString);
				return 2;
			}
			else
			{
				AddToList(pPtr,nLen);
				return 1;
			}
		}
		
		if (nLen>0)
			AddToList(pPtr,nLen);
			
		AddToList(nOperator);
		pPtr+=nLen+nOperatorLen;		
	}
	return 1;
}

void CResults::Value::Write(CStream& stream,BOOL bGivePtr) 
{
	CResults::CValueStream* pValueStream=dynamic_cast<CResults::CValueStream*>(&stream);
	if (pValueStream!=NULL)
	{
		switch (nType)
		{
		case Integer:
			pValueStream->AddToList(nInteger);
			return;
		case Operator:
			pValueStream->AddToList(nOperator);
			return;
		case String:
			if (pValueStream->ParseOperatorsFromString(pString,bGivePtr)==2)
				pString=NULL;
			return;
		}
	}

	ASSERT(nType!=Operator);

	if (nType==String && pString!=NULL)
		stream.Write(pString);
	else
	{
		WCHAR buffer[20];
		_itow_s(nInteger,buffer,20,10);
		stream.Write(buffer);
	}	
}

void CResults::Value::ToString()
{
	ASSERT(nType!=Operator);

	if (nType==Integer)
	{
		WCHAR buffer[20];
		_itow_s(nInteger,buffer,20,10);
		nType=String;
		pString=alloccopy(buffer);
	}	
}

BOOL CResults::Value::OnlySpaces() const
{
	if (nType==String && pString!=NULL)
	{
		int i=0;
		for (i=0;pString[i]==' ' || pString[i]=='\t';i++);
		return pString[i]=='\0';
	}
	return FALSE;
}


BOOL CResults::Value::ToInteger(BOOL bForce)
{	
	ASSERT(nType!=Operator);


	if (nType==String)
	{
		if (!bForce)
		{
			for (int i=0;pString[i]!='\0';i++)
			{
				if (pString[i]<'0' || pString[i]>'9')
					return FALSE;
			}
		}

		int nValue=_wtoi(pString);
		delete[] pString;
		nType=Integer;
		nInteger=nValue;
	}

	return TRUE;
}

CResults::CValueStream::~CValueStream()
{
}
	
DWORD CResults::CValueStream::GetLength(DWORD* pHigh) const
{
	// No need to implement
	return 0;
}

ULONGLONG CResults::CValueStream::GetLength64() const
{
	// No need to implement
	return 0;
}

BOOL CResults::CValueStream::SetLength(DWORD dwNewLen,LONG* pHigh)
{
	// No need to implement
	return 0;
}

BOOL CResults::CValueStream::SetLength64(ULONGLONG dwNewLen)
{
	// No need to implement
	return 0;
}

DWORD CResults::CValueStream::Seek(LONG lOff, SeekPosition nFrom,LONG* pHighPos)
{
	// No need to implement
	return 0;
}

DWORD CResults::CValueStream::Seek64(ULONGLONG lOff, SeekPosition nFrom)
{
	// No need to implement
	return 0;
}

ULONG CResults::CValueStream::GetPosition(PLONG pHigh) const
{
	// No need to implement
	return 0;
}

ULONGLONG CResults::CValueStream::GetPosition64() const
{
	// No need to implement
	return 0;
}

DWORD CResults::CValueStream::Read(void* lpBuf, DWORD nCount) const
{
	// No need to implement
	return 0;
}

BOOL CResults::CValueStream::Write(const void* lpBuf, DWORD nCount)
{
	// No need to implement
	return FALSE;
}

BOOL CResults::CValueStream::Write(LPCWSTR lpString, DWORD nCount)
{
	LPWSTR pCopy=alloccopy(lpString,nCount);
	if (ParseOperatorsFromString(pCopy,TRUE)==2)
		return TRUE;
	delete[] pCopy;
	return TRUE;
}

CResults::Value CResults::CValueStream::ToSingleValue()
{
	if (!EvaluateOperators())
		return Value(L"!INVALIDFORMAT!");
	if (m_Values.GetCount()==0)
		return Value(L"");
	else if (m_Values.GetCount()==1)
		return m_Values.GetHead()->SwapToNew();
	else {
		CMemoryStream str;
		POSITION pPos=m_Values.GetHeadPosition();
		while (pPos!=NULL)
		{
			m_Values.GetAt(pPos)->Write(str,FALSE);
			pPos=m_Values.GetNextPosition(pPos);
		}
		m_Values.RemoveAll();
		return Value((LPCWSTR)str.GetData(),str.GetLength()/2);
	}	
}

BOOL CResults::CValueStream::EvaluateOperators()
{
	// First, if there is two non-operators in row, combine them
	POSITION pPos=m_Values.GetHeadPosition();
	while (pPos!=NULL)
	{
		Value* pThis=m_Values.GetAt(pPos);
		if (!pThis->IsOperator())
		{
			Value* pNext=m_Values.GetNext(pPos);
			while (pNext!=NULL && !pNext->IsOperator())
			{
				pThis->ToString();
				pNext->ToString();
				pThis->AddString((LPCWSTR)*pNext);
				m_Values.RemoveAt(m_Values.GetNextPosition(pPos));
				pNext=m_Values.GetNext(pPos);
			}
		}

		pPos=m_Values.GetNextPosition(pPos);
	}

	// Calculate *, /, //, and % operators
	Value* pPrev=NULL;
	pPos=m_Values.GetHeadPosition();
	while (pPos!=NULL)
	{
		Value* pThis=m_Values.GetAt(pPos);
		Value* pNext=m_Values.GetNext(pPos);

		switch(pThis->GetOperator())
		{
		case CResults::Value::Multiply:
		case CResults::Value::Divide:
		case CResults::Value::DivideAndRoundToInfinity:
		case CResults::Value::Remainder:
			if (pNext==NULL || pPrev==NULL)
				return FALSE;
			if (pPrev->IsInteger() && pNext->IsInteger())
			{
				int lVal=(int)*pPrev;
				int rVal=(int)*pNext;
				switch (pThis->GetOperator())
				{
				case CResults::Value::Multiply:
					pThis->Set(lVal*rVal);
					break;
				case CResults::Value::Divide:
					pThis->Set(lVal/rVal);
					break;
				case CResults::Value::DivideAndRoundToInfinity:
					pThis->Set(lVal/rVal+(lVal%rVal!=0?1:0));
					break;
				case CResults::Value::Remainder:
					pThis->Set(lVal%rVal);
					break;
				}
				m_Values.RemoveAt(m_Values.GetPrevPosition(pPos));
				m_Values.RemoveAt(m_Values.GetNextPosition(pPos));
			}
			else
				return FALSE;
			break;
		}

		pPrev=pThis;
		pPos=m_Values.GetNextPosition(pPos);
	}

	
	// Then !, + and -
	pPrev=NULL;
	pPos=m_Values.GetHeadPosition();
	while (pPos!=NULL)
	{
		Value* pThis=m_Values.GetAt(pPos);
		Value* pNext=m_Values.GetNext(pPos);

		switch(pThis->GetOperator())
		{
		case CResults::Value::Not:
			if (pNext==NULL)
				return FALSE; 
			if (!pNext->IsInteger())
				return FALSE;
			pThis->Set(!(int)*pNext);
			m_Values.RemoveAt(m_Values.GetNextPosition(pPos));
			break;
		case CResults::Value::Add:
		case CResults::Value::Subtract:
			if (pNext==NULL || pPrev==NULL)
				return FALSE;
			if (pPrev->IsInteger() && pNext->IsInteger())
			{
				int lVal=(int)*pPrev;
				int rVal=(int)*pNext;
				
				pThis->Set(pThis->GetOperator()==CResults::Value::Add?lVal+rVal:lVal-rVal);
				
				m_Values.RemoveAt(m_Values.GetPrevPosition(pPos));
				m_Values.RemoveAt(m_Values.GetNextPosition(pPos));
			}
			else
				return FALSE;
			break;
		}

		pPrev=pThis;
		pPos=m_Values.GetNextPosition(pPos);
	}

	// Then equalities
	pPrev=NULL;
	pPos=m_Values.GetHeadPosition();
	while (pPos!=NULL)
	{
		Value* pThis=m_Values.GetAt(pPos);
		Value* pNext=m_Values.GetNext(pPos);

		switch(pThis->GetOperator())
		{
		case CResults::Value::Equal:
		case CResults::Value::NotEqual:
			if (pNext==NULL || pPrev==NULL)
				return FALSE;
			if (pPrev->IsInteger())
			{
				if (!pNext->IsInteger())
					return FALSE;

				int bEqual=(int)*pPrev==(int)*pNext;
				pThis->Set(pThis->GetOperator()==CResults::Value::NotEqual?!bEqual:bEqual);
			}
			else 
			{
				int bEqual=FALSE;
				LPCWSTR pszPrev=(LPCWSTR)*pPrev;
				LPCWSTR pszNext=(LPCWSTR)*pNext;
				if (pszNext!=NULL)
					bEqual=wcscmp(pszPrev,pszNext)==0;
				pThis->Set(pThis->GetOperator()==CResults::Value::NotEqual?!bEqual:bEqual);
				
			}
			m_Values.RemoveAt(m_Values.GetPrevPosition(pPos));
			m_Values.RemoveAt(m_Values.GetNextPosition(pPos));
			break;
		case CResults::Value::Less:
		case CResults::Value::Greater:
		case CResults::Value::LessOrEqual:
		case CResults::Value::GreaterOrEqual:
			if (pNext==NULL || pPrev==NULL)
				return FALSE;
			if (pPrev->IsInteger() && pNext->IsInteger())
			{
				int lVal=(int)*pPrev;
				int rVal=(int)*pNext;
				switch (pThis->GetOperator())
				{
				case CResults::Value::Less:
					pThis->Set(lVal<rVal);
					break;
				case CResults::Value::Greater:
					pThis->Set(lVal>rVal);
					break;
				case CResults::Value::LessOrEqual:
					pThis->Set(lVal<=rVal);
					break;
				case CResults::Value::GreaterOrEqual:
					pThis->Set(lVal>=rVal);
					break;
				}
				m_Values.RemoveAt(m_Values.GetPrevPosition(pPos));
				m_Values.RemoveAt(m_Values.GetNextPosition(pPos));
			}
			else
				return FALSE;
			break;
		}

		pPrev=pThis;
		pPos=m_Values.GetNextPosition(pPos);
	}

	// And finally AND and OR
	pPrev=NULL;
	pPos=m_Values.GetHeadPosition();
	while (pPos!=NULL)
	{
		Value* pThis=m_Values.GetAt(pPos);
		Value* pNext=m_Values.GetNext(pPos);

		switch(pThis->GetOperator())
		{
		case CResults::Value::And:
		case CResults::Value::Or:
			if (pNext==NULL || pPrev==NULL)
				return FALSE;
			if (pPrev->IsInteger() && pNext->IsInteger())
			{
				int lVal=(int)*pPrev;
				int rVal=(int)*pNext;
				
				pThis->Set(pThis->GetOperator()==CResults::Value::And?(lVal&&rVal):(lVal||rVal));
				 
				m_Values.RemoveAt(m_Values.GetPrevPosition(pPos));
				m_Values.RemoveAt(m_Values.GetNextPosition(pPos));
			}
			else
				return FALSE;
			break;
		}

		pPrev=pThis;
		pPos=m_Values.GetNextPosition(pPos);
	}


	return TRUE;
}
	




BOOL CResults::Initialize(DWORD dwFlags,LPCWSTR szDescription)
{
	m_dwFlags=dwFlags;
	m_strDescription=szDescription;
	m_nResults=0;
	m_nFiles=0;
	m_nDirectories=0;

	m_AllDetails.Attach(CLocateDlg::GetDefaultDetails());
	
	// Initializing temp file
	WCHAR szTempPath[MAX_PATH];
	if (!FileSystem::GetTempPath(MAX_PATH,szTempPath))
	{
		if (m_bThrow)
			throw CFileException(CFileException::badPath,GetLastError(),"TEMP");
		else
			return FALSE;
	}
	
	if (!FileSystem::GetTempFileName(szTempPath,L"lsr",0,m_sTempFile.GetBuffer(MAX_PATH)))
	{
		if (m_bThrow)
			throw CFileException(CFileException::fileCreate,GetLastError(),"temp file");
		else
			return FALSE;
	}

	return TRUE;
}

void CResults::Close()
{
	if (m_pSelectedDetails!=NULL)
	{
		delete[] m_pSelectedDetails;
		m_pSelectedDetails=NULL;
	}
	if (m_pLengths!=NULL)
	{
		delete[] m_pLengths;
		m_pLengths=NULL;
	}
	if (!m_sTempFile.IsEmpty())
	{
		FileSystem::Remove(m_sTempFile);
		m_sTempFile.Empty();
	}
}


BOOL CResults::Create(CListCtrl* pList,int* pSelectedDetails,int nSelectedDetails,BOOL bDataToTmpFile)
{

	m_nSelectedDetails=nSelectedDetails;
	if (nSelectedDetails>0)
	{
		m_pSelectedDetails=new int[nSelectedDetails];
		m_pLengths=new DWORD[nSelectedDetails];
		
		sMemCopy(m_pSelectedDetails,pSelectedDetails,nSelectedDetails*sizeof(int));
		sMemSet(m_pLengths,0,nSelectedDetails*sizeof(int));
	}

	int mask=(m_dwFlags&RESULT_INCLUDESELECTEDITEMS)?LVNI_SELECTED:LVNI_ALL;


	if (bDataToTmpFile)
	{
		// Save details to temporary file
		CFile tmpFile(m_sTempFile,CFile::defWrite,TRUE);
		int nItem=pList->GetNextItem(-1,mask);
		while (nItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)pList->GetItemData(nItem);
			if (pItem!=NULL)
			{
				for (int i=0;i<nSelectedDetails;i++)
				{
					// Updating if necessary
					if (pItem->ShouldUpdateByDetail((DetailType)m_pSelectedDetails[i]))
						pItem->UpdateByDetail((DetailType)m_pSelectedDetails[i]);
					
					// Retrieving detail text
					LPWSTR szDetail=pItem->GetDetailText((DetailType)m_pSelectedDetails[i]);
					DWORD dwLength=(DWORD)istrlenw(szDetail);

					// Checking length
					if (dwLength>m_pLengths[i])
						m_pLengths[i]=dwLength;
					
					//Writing detail to temp file
					tmpFile.Write(dwLength);
					tmpFile.Write(szDetail,dwLength);
				}
				
				// Count results
				m_nResults++;
				if (!pItem->IsDeleted())
				{
					if (pItem->IsFolder())
						m_nDirectories++;
					else
						m_nFiles++;
				}

				// Checking database IDs
				WORD wID=pItem->GetDatabaseID();
				int i;
				for (i=m_aFromDatabases.GetSize()-1;i>=0;i--)
				{
					if (m_aFromDatabases[i]==wID)
						break;
				}
				if (i<0)
					m_aFromDatabases.Add(wID);
			}
			nItem=pList->GetNextItem(nItem,mask);
		}
	}
	else
	{
		int nItem=pList->GetNextItem(-1,mask);
		while (nItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)pList->GetItemData(nItem);
			if (pItem!=NULL)
			{
				m_Items.Add(pItem);

				// Count results
				m_nResults++;
				if (!pItem->IsDeleted())
				{
					if (pItem->IsFolder())
						m_nDirectories++;
					else
						m_nFiles++;
				}

				// Checking database IDs
				WORD wID=pItem->GetDatabaseID();
				int i;
				for (i=m_aFromDatabases.GetSize()-1;i>=0;i--)
				{
					if (m_aFromDatabases[i]==wID)
						break;
				}
				if (i<0)
					m_aFromDatabases.Add(wID);
			}
			nItem=pList->GetNextItem(nItem,mask);
		}
	}
	return TRUE;
}

BOOL CResults::SaveToFile(LPCWSTR szFile) const
{
	ASSERT(m_Items.GetSize()==0);

	// Opening files
	CFileEncode outFile(szFile,CFile::defWrite,TRUE);
	CFile tmpFile(m_sTempFile,CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
	outFile.CloseOnDelete();
	tmpFile.CloseOnDelete();

	if (m_dwFlags&RESULT_ENCODINGUTF8)
		outFile.SetEncoding(CFileEncode::UTF8);
	else if (m_dwFlags&RESULT_ENCODINGUNICODE)
		outFile.SetEncoding(CFileEncode::Unicode);
	else
		outFile.SetEncoding(CFileEncode::ANSI);
	


	
	// Writing header
	{
		CStringW str;
		str.Format(IDS_SAVERESULTSHEADER,m_nResults,m_nFiles,m_nDirectories);
		outFile.Write(str);
		outFile.Write(L"\r\n",2);
	}



	// Checking width of labels if necessary
	CAllocArrayTmpl<CStringW> pLabels(max(m_nSelectedDetails,1));


	if (m_dwFlags&RESULT_INCLUDELABELS)
	{
		for (int i=0;i<m_nSelectedDetails;i++)
		{
			pLabels[i].LoadString(m_AllDetails[m_pSelectedDetails[i]].nString,LanguageSpecificResource);
		    
			if ((DWORD)pLabels[i].GetLength()>m_pLengths[i])
				m_pLengths[i]=(DWORD)pLabels[i].GetLength();
		}
	}

	
	// Checking the fields tallest length
	DWORD dwMaxLength=0;
	for (int i=0;i<m_nSelectedDetails;i++)
	{
		if (m_pLengths[i]>dwMaxLength)
			dwMaxLength=m_pLengths[i];
	}

	// Initializing buffers
	CAllocArrayTmpl<WCHAR> szBuffer(dwMaxLength+3,TRUE);
	CAllocArrayTmpl<WCHAR> szSpaces(dwMaxLength+2,TRUE);
	wmemset(szSpaces,L' ',dwMaxLength+2);

	if (m_dwFlags&RESULT_INCLUDEDATE)
	{
		WCHAR szDate[200];
		outFile.Write(ID2W(IDS_SAVERESULTSDATE));
		outFile.Write(L' ');
		DWORD dwLength;
		if (IsUnicodeSystem())
			dwLength=GetDateFormatW(NULL,DATE_SHORTDATE,NULL,NULL,szDate,200);
		else
		{
			char szDateA[200];
			dwLength=GetDateFormat(NULL,DATE_SHORTDATE,NULL,NULL,szDateA,200);
			MemCopyAtoW(szDate,szDateA,dwLength);
		}
		szDate[dwLength-1]=' ';
		outFile.Write(szDate,dwLength);
		if (IsUnicodeSystem())
			dwLength=GetTimeFormatW(NULL,0,NULL,NULL,szDate,200)-1;
		else
		{
			char szDateA[200];
			dwLength=GetTimeFormat(NULL,0,NULL,NULL,szDateA,200)-1;
			MemCopyAtoW(szDate,szDateA,dwLength);
		}
		szDate[dwLength++]='\r';
		szDate[dwLength++]='\n';
		outFile.Write(szDate,dwLength);
	}

	if (m_dwFlags&RESULT_INCLUDEDESCRIPTION)
	{
		outFile.Write(m_strDescription);
		outFile.Write(L"\r\n",2);
	}

	if (m_dwFlags&RESULT_INCLUDEDBINFO && m_aFromDatabases.GetSize()>0)
	{
		CStringW str(IDS_SAVERESULTSDBCAPTION);
		outFile.Write(str);
		outFile.Write(L"\r\n",2);

		for (int i=0;i<m_aFromDatabases.GetSize();i++)
		{
			const CDatabase* pDatabase=GetLocateApp()->GetDatabase(m_aFromDatabases[i]);
			str.Format(IDS_SAVERESULTSDB,pDatabase->GetName(),
				pDatabase->GetCreator(),pDatabase->GetDescription(),
				pDatabase->GetArchiveName());
			outFile.Write(str);
			outFile.Write(L"\r\n",2);
		}
	}

	outFile.Write(L"\r\n",2);

	if (m_dwFlags&RESULT_INCLUDELABELS && m_nSelectedDetails>0)
	{
		int i;
		for (i=0;i<m_nSelectedDetails-1;i++)
		{
			outFile.Write((LPCWSTR)pLabels[i],(DWORD)pLabels[i].GetLength());
			outFile.Write(szSpaces,m_pLengths[i]-(DWORD)pLabels[i].GetLength()+2);
		}

		outFile.Write(pLabels[i]);
		outFile.Write(L"\r\n",2);
	}

	if (m_nSelectedDetails==0)
		return TRUE;

	// Saving data to files
	for (int nRes=0;nRes<m_nResults;nRes++)
	{
		DWORD dwLength;
		    
		for (int i=0;i<m_nSelectedDetails-1;i++)
		{
			// Reading length and data
			tmpFile.Read(dwLength);
			tmpFile.Read((void*)(LPWSTR)szBuffer,dwLength*2);

			outFile.Write(szBuffer,dwLength);
			outFile.Write(szSpaces,m_pLengths[i]-dwLength+2);
		}

		tmpFile.Read(dwLength);
		tmpFile.Read((void*)(LPWSTR)szBuffer,dwLength*2);

		szBuffer[dwLength++]='\r';
		szBuffer[dwLength++]='\n';
		outFile.Write(szBuffer,dwLength);
	}

	return TRUE;
}

BOOL CResults::SaveToHtmlFile(LPCWSTR szFile) const
{
	ASSERT(m_Items.GetSize()==0);

	// Opening files
	CFileEncode outFile(szFile,CFile::defWrite,TRUE);
	CFile tmpFile(m_sTempFile,CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
	outFile.CloseOnDelete();
	tmpFile.CloseOnDelete();

	LPCWSTR pCharset;
	if (m_dwFlags&RESULT_ENCODINGUTF8)
	{
		outFile.SetEncoding(CFileEncode::UTF8);
		pCharset=L"UTF-8";
	}
	else if (m_dwFlags&RESULT_ENCODINGUNICODE)
	{
		outFile.SetEncoding(CFileEncode::Unicode);
		pCharset=L"UTF-16";
	}
	else
	{
		outFile.SetEncoding(CFileEncode::ANSI);
		pCharset=L"iso-8859-1";
	}


	CStringW str;
	
	/* Header section BEGIN */
	{
		WCHAR pStr[]=L"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";
		outFile.Write(pStr,sizeof(pStr)/sizeof(WCHAR)-1);
	}
	{
		WCHAR pStr[]=L"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n<head>\n";
		outFile.Write(pStr,sizeof(pStr)/sizeof(WCHAR)-1);
	}
	
	str.Format(L"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\" />\n<style type=\"text/css\">\n",pCharset);
	outFile.Write(str);
	
	{
		WCHAR pStr[]=L"body { font-size: 11pt; }\ntable { border-style: none; margin-left: -10pt;} \n";
		outFile.Write(pStr,sizeof(pStr)/sizeof(WCHAR)-1);
	}
	{
		WCHAR pStr[]=L"td { spacing: 10pt 2pt; padding: 1pt 10pt;}\n#databasetable_header { font-size: 12pt; font-weight: bold;}\n";
		outFile.Write(pStr,sizeof(pStr)/sizeof(WCHAR)-1);
	}
	{
		WCHAR pStr[]=L"#resultstable_header { font-size: 12pt; font-weight: bold;}\n</style>\n";
		outFile.Write(pStr,sizeof(pStr)/sizeof(WCHAR)-1);
	}
	{
		WCHAR pStr[]=L"<link rel=\"stylesheet\" href=\"loc_res.css\" type=\"text/css\" />\n<title>";
		outFile.Write(pStr,sizeof(pStr)/sizeof(WCHAR)-1);
	}
	str.LoadString(IDS_SAVERESULTSTITLE);
	outFile.Write(str);

	{
		WCHAR pStr[]=L"</title>\n</head>\n<body>\n<div id=\"header\">\n<h1>";
		outFile.Write(pStr,sizeof(pStr)/sizeof(WCHAR)-1);
	}

	str.LoadString(IDS_SAVERESULTSTITLE2);
	outFile.Write(str);

	{
		WCHAR pStr[]=L"</h1>\n<ul>\n<li id=\"head_results\">";
		outFile.Write(pStr,sizeof(pStr)/sizeof(WCHAR)-1);
	}

	str.Format(IDS_SAVERESULTSHEADER,m_nResults,m_nFiles,m_nDirectories);
	outFile.Write(str);
	outFile.Write(L"</li>\n",6);

	/* Header section END */

	/* Date section BEGIN */
	
	if (m_dwFlags&RESULT_INCLUDEDATE)
	{
		outFile.Write(L"<li id=\"head_date\">");

		WCHAR szDate[200];
		str.LoadString(IDS_SAVERESULTSDATE);
		outFile.Write(str);
		DWORD dwLength;
		if (IsUnicodeSystem())
			dwLength=GetDateFormatW(NULL,DATE_SHORTDATE,NULL,NULL,szDate,200);
		else
		{
			char szDateA[200];
			dwLength=GetDateFormat(NULL,DATE_SHORTDATE,NULL,NULL,szDateA,200);
			MemCopyAtoW(szDate,szDateA,dwLength);
		}

        szDate[dwLength-1]=L' ';
		outFile.Write(szDate,dwLength);
		if (IsUnicodeSystem())
			dwLength=GetTimeFormatW(NULL,0,NULL,NULL,szDate,200)-1;
		else
		{
			char szDateA[200];
			dwLength=GetTimeFormatA(NULL,0,NULL,NULL,szDateA,200)-1;
			MemCopyAtoW(szDate,szDateA,dwLength);
		}
		outFile.Write(szDate,dwLength);
		outFile.Write(L"</li>\n");
		
	}
	/* Date section END */
	
	/* Description section BEGIN */
	if (m_dwFlags&RESULT_INCLUDEDESCRIPTION)
	{
		outFile.Write(L"<li id=\"head_description\">");
		outFile.Write(m_strDescription);
		outFile.Write(L"</li>\n");
	}
	/* Description section END */
	
	/* Database section BEGIN */
	if (m_dwFlags&RESULT_INCLUDEDBINFO && m_aFromDatabases.GetSize()>0)
	{
		outFile.Write(L"<li id=\"head_databases\">");
		str.LoadString(IDS_SAVERESULTSDBCAPTION);
		outFile.Write(str);
		outFile.Write(L"\n<table id=\"databasetable\">\n<tr id=\"databasetable_header\">\n</td><td>");
		str.LoadString(IDS_SAVERESULTSDBNAME);
		outFile.Write(str);
		outFile.Write(L"</td><td>",9);
		str.LoadString(IDS_SAVERESULTSDBCREATOR);
		outFile.Write(str);
		outFile.Write(L"</td><td>",9);
		str.LoadString(IDS_SAVERESULTSDBDESCRIPTION);
		outFile.Write(str);
		outFile.Write(L"</td><td>",9);
		str.LoadString(IDS_SAVERESULTSDBFILE);
		outFile.Write(str);
		outFile.Write(L"</td></tr>\n",11);
		
		
		
		for (int i=0;i<m_aFromDatabases.GetSize();i++)
		{
			const CDatabase* pDatabase=GetLocateApp()->GetDatabase(m_aFromDatabases[i]);

			outFile.Write(L"<tr><td>",8);
			outFile.Write(pDatabase->GetName());
			outFile.Write(L"</td><td>",9);
			outFile.Write(pDatabase->GetCreator());
			outFile.Write(L"</td><td>",9);
			outFile.Write(pDatabase->GetDescription());
			outFile.Write(L"</td><td>",9);
			outFile.Write(pDatabase->GetArchiveName());
			outFile.Write(L"</td></tr>",10);

		}
		
		outFile.Write(L"</table>\n</li>\n");
		
	}
	outFile.Write(L"</ul>\n</div>\n");
	/* Database section END */
	

	/* Results section BEGIN  */
	outFile.Write(L"<div id=\"resultslist\">\n<table id=\"resulttable\">\n");
	
	if (m_dwFlags&RESULT_INCLUDELABELS && m_nSelectedDetails>0)
	{
		CAutoPtrA<CLocateDlg::ViewDetails> pDetails(CLocateDlg::GetDefaultDetails());
		outFile.Write(L"<tr id=\"resultstable_header\">\n");
		
		for (int i=0;i<m_nSelectedDetails;i++)
		{
			outFile.Write(L"<td>",4);
			str.LoadString(m_AllDetails[m_pSelectedDetails[i]].nString,LanguageSpecificResource);
			outFile.Write(str);
			outFile.Write(L"</td>",5);
		}
		
		outFile.Write(L"\n</tr>\n",7);
	}

	
	for (int nRes=0;nRes<m_nResults;nRes++)
	{
		outFile.Write(L"<tr>",4);
		DWORD dwLength;
			
		for (int i=0;i<m_nSelectedDetails;i++)
		{
			outFile.Write(L"<td>",4);

			// Reading length and data
			tmpFile.Read(dwLength);
			
			if (dwLength>0)
			{
				WCHAR* szBuffer=new WCHAR[dwLength];
				tmpFile.Read((LPSTR)szBuffer,dwLength*2);
				outFile.Write(szBuffer,dwLength);
				delete[] szBuffer;
			}
			outFile.Write(L"</td>",5);			
		}

		outFile.Write(L"</tr>\n",6);
	}
	
	outFile.Write(L"</table>\n</div>\n</body>\n</html>\n");
	
	/* Results section END */
	
	return TRUE;
}


BOOL CResults::SaveToHtmlFile(LPCWSTR szFile,LPCWSTR szTemplate)
{	
	ASSERT(m_nResults==m_Items.GetSize());

	// Opening files
	CStringW str;
	CAutoPtrA<WCHAR> pBuffer;
	DWORD dwTemplateLen=0;
	CFileEncode outFile(szFile,CFile::defWrite,TRUE);
	CFile tmpFile(m_sTempFile,CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
	outFile.CloseOnDelete();
	tmpFile.CloseOnDelete();
	
	if (m_dwFlags&RESULT_ENCODINGUTF8)
	{
		outFile.SetEncoding(CFileEncode::UTF8);
		SetVariable("CHARSET",L"UTF-8");
	}
	else if (m_dwFlags&RESULT_ENCODINGUNICODE)
	{
		outFile.SetEncoding(CFileEncode::Unicode);
		SetVariable("CHARSET",L"UTF-16");
	}
	else
	{
		outFile.SetEncoding(CFileEncode::ANSI);
		SetVariable("CHARSET",L"iso-8859-1");
	}

	// Initialize variables
	
	// Labels
	SetVariable("TOOLBARTITLE",ID2W(IDS_SAVERESULTSTITLE));
	SetVariable("TITLE",ID2W(IDS_SAVERESULTSTITLE2));
	SetVariable("DATELABEL",ID2W(IDS_SAVERESULTSDATE));
	SetVariable("RESULTSFROMDATABASES",ID2W(IDS_SAVERESULTSDBCAPTION));
	SetVariable("DBNAMELABEL",ID2W(IDS_SAVERESULTSDBNAME));
	SetVariable("DBCREATORLABEL",ID2W(IDS_SAVERESULTSDBCREATOR));
	SetVariable("DBDESCRIPTIONLABEL",ID2W(IDS_SAVERESULTSDBDESCRIPTION));
	SetVariable("DBFILELABEL",ID2W(IDS_SAVERESULTSDBFILE));
	
	
	SetVariable("INCLUDEDATE",(m_dwFlags&RESULT_INCLUDEDATE)?TRUE:FALSE);
	SetVariable("INCLUDEDESCRIPTION",(m_dwFlags&RESULT_INCLUDEDESCRIPTION)?TRUE:FALSE);
	SetVariable("INCLUDEDBINFO",(m_dwFlags&RESULT_INCLUDEDBINFO)?TRUE:FALSE);
	SetVariable("INCLUDECOLUMNLABELS",(m_dwFlags&RESULT_INCLUDELABELS)?TRUE:FALSE);


	SetVariable("NUM_RESULTS",m_nResults);
	SetVariable("NUM_FILES",m_nFiles);
	SetVariable("NUM_DIRECTORIES",m_nDirectories);
	SetVariable("NUM_SELECTEDDETAILS",m_nSelectedDetails);
	SetVariable("NUM_ALLDETAILS",TypeCount);
	SetVariable("NUM_DATABASES",m_aFromDatabases.GetSize());
	SetVariable("DESCRIPTION",m_strDescription);
	

	
	str.Format(IDS_SAVERESULTSHEADER,m_nResults,m_nFiles,m_nDirectories);
	SetVariable("RESULTSSTRING",str);
	
	// Date and time
	{
		WCHAR szDate[200];
		DWORD dwLength;
		if (IsUnicodeSystem())
			dwLength=GetDateFormatW(NULL,DATE_SHORTDATE,NULL,NULL,szDate,200);
		else
		{
			char szDateA[200];
			dwLength=GetDateFormat(NULL,DATE_SHORTDATE,NULL,NULL,szDateA,200);
			MemCopyAtoW(szDate,szDateA,dwLength);
		}
        szDate[dwLength-1]=L'\0';
		SetVariable("DATE",szDate);

		if (IsUnicodeSystem())
			dwLength=GetTimeFormatW(NULL,0,NULL,NULL,szDate,200)-1;
		else
		{
			char szDateA[200];
			dwLength=GetTimeFormatA(NULL,0,NULL,NULL,szDateA,200)-1;
			MemCopyAtoW(szDate,szDateA,dwLength);
		}

        szDate[dwLength]=L'\0';
		SetVariable("TIME",szDate);
	}

	




	{
		// Read template file to memory
		CFile tmlFile(szTemplate,CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
		tmlFile.CloseOnDelete();
		dwTemplateLen=tmlFile.GetLength();
		
		// Check encoding
		BYTE pTestBuffer[4];
		tmlFile.Read(pTestBuffer,4);
		if (pTestBuffer[0]==0xFF && pTestBuffer[1]==0xFE)
		{
			// Unicode file
			tmlFile.Seek(2,CFile::begin);
			dwTemplateLen-=2;
			dwTemplateLen/=2;
			pBuffer.Attach(new WCHAR[dwTemplateLen+2]);
			tmlFile.Read((BYTE*)(WCHAR*)pBuffer,dwTemplateLen*2);
		}
		else if (pTestBuffer[0]!=0 && pTestBuffer[1]==0 && pTestBuffer[2]!=0 && pTestBuffer[3]==0)
		{
			// Probable Unicode
			tmlFile.SeekToBegin();
			dwTemplateLen/=2;
			pBuffer.Attach(new WCHAR[dwTemplateLen+2]);
			tmlFile.Read((BYTE*)(WCHAR*)pBuffer,dwTemplateLen*2);
		}
		else
		{
			// ANSI
			CAutoPtrA<char> pBufferA;
			tmlFile.SeekToBegin();
			pBufferA.Attach(new char[dwTemplateLen+2]);
			pBuffer.Attach(new WCHAR[dwTemplateLen+2]);
			tmlFile.Read((char*)pBufferA,dwTemplateLen);
			MultiByteToWideChar(CP_ACP,0,pBufferA,dwTemplateLen,pBuffer,dwTemplateLen);
		}
		
		pBuffer[dwTemplateLen]='\0';
		pBuffer[dwTemplateLen+1]='\0';
		tmlFile.Close();
	}

	return ParseBuffer(outFile,pBuffer,dwTemplateLen,FALSE);
}

BOOL CResults::ParseBuffer(CStream& stream,LPCWSTR pBuffer,int iBufferLen,BOOL bHandleParenthesis)
{
	LPCWSTR pPtr=pBuffer;
	int nLength;

	while (iBufferLen>0)
	{
		// Find next '$' or '(' 
		for (nLength=0;nLength<iBufferLen && pPtr[nLength]!=L'$' && (pPtr[nLength]!=L'(' || !bHandleParenthesis);nLength++);

		if (nLength>0)
		{
			// Write buffer to output file
			stream.Write(pPtr,nLength);
			pPtr+=nLength;
			iBufferLen-=nLength;
		}

		// If no '$' anymore, all done
		if (iBufferLen==0)
			return TRUE;

		// Handle parenthesis
		if (pPtr[nLength]==L'(' && bHandleParenthesis)
		{
			int iBlockLength;
			if (!ParseBlockLength(pPtr,iBufferLen,iBlockLength))
				throw CFileException(CFileException::invalidFile);

			EvaluateCondition(pPtr+1,iBlockLength).Write(stream,TRUE);

			pPtr+=iBlockLength+2;
			iBufferLen-=iBlockLength+2;
			continue;
		}
		

		// Skip '$'
		pPtr++;iBufferLen--;
		
		// Find '$' or '['
		for (nLength=0;nLength<iBufferLen && pPtr[nLength]!=L'$' && pPtr[nLength]!=L'[';nLength++);

		if (nLength==iBufferLen)
		{
			// No '$' or '[', write the rest of file and exit.
			stream.Write(pPtr,nLength);
			return FALSE;
		}
		
		if (pPtr[nLength]==L'$')
		{
			// Variable
			W2A variable(pPtr,nLength);
			Value* pValue=GetVariable(variable);
			if (pValue!=NULL)
				pValue->Write(stream,FALSE);
			else
				stream.Write(L"!UNKNOWNVARIABLE!");
			
			pPtr+=nLength+1;
			iBufferLen-=nLength+1;
		}
		else 
		{
			// IF/FOR or function
			if (nLength==2 && _wcsnicmp(pPtr,L"IF",2)==0)
			{
				// IF
				pPtr+=2;
				iBufferLen-=2;

				int iConditionLength,iBlockLength;
				if (!ParseBlockLength(pPtr,iBufferLen,iConditionLength))
					throw CFileException(CFileException::invalidFile);

				LPCWSTR pCondition=pPtr+1;
				pPtr+=2+iConditionLength;
				iBufferLen-=iConditionLength+2;

				if (!ParseBlockLength(pPtr,iBufferLen,iBlockLength))
					throw CFileException(CFileException::invalidFile);
				
				Value Condition=EvaluateCondition(pCondition,iConditionLength);
				if (Condition.IsInteger())
				{
					if (int(Condition))
					{
						if (!ParseBuffer(stream,pPtr+1,iBlockLength,TRUE))
							throw CFileException(CFileException::invalidFile);
					}
				}
				else
					stream.Write(L"!INVALIDARGUMENT!");

				pPtr+=iBlockLength+2;
				iBufferLen-=iBlockLength+2;

			}
			else if (nLength==3 && _wcsnicmp(pPtr,L"FOR",3)==0)
			{
				// FOR
				pPtr+=3;
				iBufferLen-=3;

				int iConditionLength,iBlockLength;
				if (!ParseBlockLength(pPtr,iBufferLen,iConditionLength))
					throw CFileException(CFileException::invalidFile);

				CStringW sVariable(pPtr+1,iConditionLength);
				pPtr+=2+iConditionLength;
				iBufferLen-=iConditionLength+2;

				if (!ParseBlockLength(pPtr,iBufferLen,iBlockLength))
					throw CFileException(CFileException::invalidFile);
				
				// Separate min and max from variable
				int nVariableLen=sVariable.FindFirst(L'=');
				if (nVariableLen==-1)
					throw CFileException(CFileException::invalidFile);

				int nMinLen=LastCharIndex(LPCWSTR(sVariable)+nVariableLen+1,L':');
				if (nVariableLen==-1)
					throw CFileException(CFileException::invalidFile);
			
				
				
				int nMin=EvaluateCondition(LPCWSTR(sVariable)+nVariableLen+1,nMinLen);
				int nMax=EvaluateCondition(LPCWSTR(sVariable)+nVariableLen+nMinLen+1+1,iConditionLength-nVariableLen-nMinLen-2);

				sVariable.FreeExtra(nVariableLen);
				sVariable.Trim(); // Remove spaces

				for (int i=nMin;i<=nMax;i++)
				{
					SetVariable(W2A(sVariable),i);
					if (!ParseBuffer(stream,pPtr+1,iBlockLength,TRUE))
						throw CFileException(CFileException::invalidFile);
				}

				pPtr+=iBlockLength+2;
				iBufferLen-=iBlockLength+2;

			}
			else if (nLength>5 && _wcsnicmp(pPtr,L"GETDB",5)==0)
			{
				// GETDBNAME etc
				pPtr+=5;
				iBufferLen-=5;
				nLength-=5;

				int iParametersLen;
				LPCWSTR pName=pPtr;
				pPtr+=nLength;
				iBufferLen-=nLength;

				if (!ParseBlockLength(pPtr,iBufferLen,iParametersLen))
					throw CFileException(CFileException::invalidFile);


				Value Var=EvaluateCondition(pPtr+1,iParametersLen);
				int iDB=int(Var)-1;
				
				if (iDB>=0 && iDB<m_aFromDatabases.GetSize() && Var.IsInteger())
				{
					const CDatabase* pDatabase=GetLocateApp()->GetDatabase(m_aFromDatabases[iDB]);
					if (nLength==4 && _wcsnicmp(pName,L"NAME",4)==0)
						stream.Write(pDatabase->GetName());
					else if (nLength==7 && _wcsnicmp(pName,L"CREATOR",7)==0)
						stream.Write(pDatabase->GetCreator());
					else if (nLength==11 && _wcsnicmp(pName,L"DESCRIPTION",11)==0)
						stream.Write(pDatabase->GetDescription());
					else if (nLength==4 && _wcsnicmp(pName,L"FILE",4)==0)
						stream.Write(pDatabase->GetArchiveName());
					else
						stream.Write(L"!INVALIDFUNCTION!");
				}
				else
					stream.Write(L"!INVALIDARGUMENT!");

				pPtr+=iParametersLen+2;
				iBufferLen-=iParametersLen+2;

			}
			else if (nLength==13 && _wcsnicmp(pPtr,L"GETDETAILNAME",13)==0)
			{
				// GETDETAILNAME
				pPtr+=13;
				iBufferLen-=13;
				
				int iParametersLen,iDetail=-1;
				if (!ParseBlockLength(pPtr,iBufferLen,iParametersLen))
					throw CFileException(CFileException::invalidFile);

				Value Var=EvaluateCondition(pPtr+1,iParametersLen);
				if (Var.IsInteger())
					iDetail=int(Var)-1;
				
				if (iDetail>=0 && iDetail<TypeCount)
					stream.Write(ID2W(m_AllDetails[iDetail].nString));
				else
					stream.Write(L"!INVALIDARGUMENT!");

				pPtr+=iParametersLen+2;
				iBufferLen-=iParametersLen+2;

			}
			else if (nLength==17 && _wcsnicmp(pPtr,L"GETSELECTEDDETAIL",17)==0)
			{
				// GETSELECTEDDETAIL
				pPtr+=17;
				iBufferLen-=17;
				
				int iParametersLen,iDetail=-1;
				if (!ParseBlockLength(pPtr,iBufferLen,iParametersLen))
					throw CFileException(CFileException::invalidFile);

				Value Var=EvaluateCondition(pPtr+1,iParametersLen);
				if (Var.IsInteger())
					iDetail=int(Var)-1;
				
				if (iDetail>=0 && iDetail<m_nSelectedDetails)
					Value(m_pSelectedDetails[iDetail]+1).Write(stream,TRUE);
				else
					stream.Write(L"!INVALIDARGUMENT!");

				pPtr+=iParametersLen+2;
				iBufferLen-=iParametersLen+2;

			}
			else if (nLength==9 && _wcsnicmp(pPtr,L"GETDETAIL",9)==0)
			{
				// GETDETAIL
				pPtr+=9;
				iBufferLen-=9;
				
				int iParametersLen;
				if (!ParseBlockLength(pPtr,iBufferLen,iParametersLen))
					throw CFileException(CFileException::invalidFile);

				int iFirstArgLen,iFile=-1,iDetail=-1;
				for (iFirstArgLen=0;pPtr[iFirstArgLen+1]!=L',' && iFirstArgLen<iParametersLen;iFirstArgLen++);

				Value vFile=EvaluateCondition(pPtr+1,iFirstArgLen);
				if (vFile.IsInteger())
				{
					iFile=int(vFile)-1,iDetail=0;
					if (iFirstArgLen<iParametersLen)
					{
						Value vDetail=EvaluateCondition(pPtr+iFirstArgLen+1+1,iParametersLen-iFirstArgLen-1);
						if (vDetail.IsInteger())
							iDetail=int(vDetail)-1;
					}
				}

				if (iFile>=0 && iFile<m_nResults && iDetail>=0 && iDetail<TypeCount)
				{
					// Updating if necessary
					if (m_Items[iFile]->ShouldUpdateByDetail((DetailType)iDetail))
						m_Items[iFile]->UpdateByDetail((DetailType)iDetail);
					
					// Format detail and write to the file
					stream.Write(m_Items[iFile]->GetDetailText((DetailType)iDetail));
				}
				else
					stream.Write(L"!INVALIDARGUMENT!");

				pPtr+=iParametersLen+2;
				iBufferLen-=iParametersLen+2;

			}
			else if (nLength==3 && _wcsnicmp(pPtr,L"SET",3)==0)
			{
				// SET
				pPtr+=3;
				iBufferLen-=3;
				
				int iParametersLen;
				if (!ParseBlockLength(pPtr,iBufferLen,iParametersLen))
					throw CFileException(CFileException::invalidFile);

				int iVariableLen;
				for (iVariableLen=0;pPtr[iVariableLen+1]!=L'=' && iVariableLen<iParametersLen;iVariableLen++);
				if (iVariableLen==iParametersLen)
					stream.Write(L"!INVALIDARGUMENT!");
				else
				{
					Value sValue=EvaluateCondition(pPtr+iVariableLen+1+1,iParametersLen-iVariableLen-1);
					SetVariable(W2A(pPtr+1,iVariableLen),sValue,TRUE);
				}

				pPtr+=iParametersLen+2;
				iBufferLen-=iParametersLen+2;

			}
			else if (nLength==4 && _wcsnicmp(pPtr,L"EVAL",4)==0)
			{
				// EVALueate
				pPtr+=4;
				iBufferLen-=4;
				
				int iConditionLength;
				if (!ParseBlockLength(pPtr,iBufferLen,iConditionLength))
					throw CFileException(CFileException::invalidFile);

				Value Condition=EvaluateCondition(pPtr+1,iConditionLength);
				Condition.Write(stream,TRUE);

				pPtr+=2+iConditionLength;
				iBufferLen-=iConditionLength+2;

			}
			else
			{
				pPtr+=nLength;
				iBufferLen-=nLength;
			}
		}
		
	}
	return TRUE;
}



BOOL CResults::ParseBlockLength(LPCWSTR& pPtr,int& iLen,int& riBlockLen) const
{
	// Find block start
	while (iLen>0 && (*pPtr==' ' || *pPtr=='\t' || *pPtr=='\n' || *pPtr=='\r'))
	{
		pPtr++;
		iLen--;
	}
		

	LPCWSTR pBuffer=pPtr;
	int iBufferLen=iLen;
	ASSERT(*pBuffer==L'[' || *pBuffer==L'{' || *pBuffer==L'(');
	WCHAR cBeginChar=*pBuffer;
	WCHAR cEndChar;
	
	switch (cBeginChar)
	{
	case L'{':
		cEndChar=L'}';
		break;
	case L'[':
		cEndChar=L']';
		break;
	case L'(':
		cEndChar=L')';
		break;
	default:
		return FALSE;
	}	
	int nCharsToIgnore=0;

	pBuffer++;
	iBufferLen--;
	riBlockLen=0;

	while (iBufferLen>0)
	{
		if (*pBuffer==cEndChar)
		{
			if (nCharsToIgnore<=0)
				return TRUE;
			else
				nCharsToIgnore--;
		}
		else if (*pBuffer==cBeginChar)
			nCharsToIgnore++;
		
		
		riBlockLen++;
		pBuffer++;
		iBufferLen--;
	}
	return FALSE;
}



CResults::Value CResults::EvaluateCondition(LPCWSTR pBuffer,int iConditionLength)
{
	CValueStream stream;
	if (!ParseBuffer(stream,pBuffer,iConditionLength,TRUE))
		throw CFileException(CFileException::invalidFile);
	return stream.ToSingleValue();
}

CSaveResultsDlg::CSaveResultsDlg()
:	CFileDialog(FALSE,L"*",szwEmpty,OFN_EXPLORER|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_NOREADONLYRETURN|OFN_ENABLESIZING,IDS_SAVERESULTSFILTERS,TRUE),
	m_nFlags(IDC_TITLE|RESULT_INCLUDEDATE|RESULT_INCLUDELABELS),m_pList(NULL)
{
	DWORD nFlags=GetSystemFeaturesFlag();
	EnableFeatures(nFlags);
	if (nFlags&(efWin2000|efWinME))
		SetTemplate(IDD_RESULTSAVEDIALOG2000);
	else
		SetTemplate(IDD_RESULTSAVEDIALOG);

	
	SetTitle(ID2W(IDS_SAVERESULTS));
	
	// Setting default details
	m_aDetails.Add(FullPath);
	m_aDetails.Add(FileSize);
	m_aDetails.Add(DateModified);


}

CSaveResultsDlg::~CSaveResultsDlg()
{
		if (m_pofn->lpstrTitle!=NULL)
	{
		delete[] (LPSTR)m_pofn->lpstrTitle;
		m_pofn->lpstrTitle=NULL;
	}

	if (m_pList!=NULL)
		delete m_pList;
}

BOOL CSaveResultsDlg::OnInitDialog(HWND hwndFocus)
{
	// Initializing detail list view
	m_pList=new CListCtrl(GetDlgItem(IDC_DETAILS));
	m_pList->SetExtendedListViewStyle(LVS_EX_CHECKBOXES,LVS_EX_CHECKBOXES);
	m_pList->InsertColumn(0,"",LVCFMT_LEFT,330,0);
	if (IsUnicodeSystem())
		m_pList->SetUnicodeFormat(TRUE);
		
	// Initialing toolbar
	m_ToolbarIL.Create(IDB_SAVERESTOOLBARBITMAPS,14,2,RGB(255,255,255));
	m_ToolbarILHover.Create(IDB_SAVERESTOOLBARBITMAPSH,14,2,RGB(255,255,255));
	m_ToolbarILDisabled.Create(IDB_SAVERESTOOLBARBITMAPSD,14,2,RGB(255,255,255));
	SendDlgItemMessage(IDC_TOOLBAR,TB_SETIMAGELIST,0,LPARAM(HIMAGELIST(m_ToolbarIL)));
	SendDlgItemMessage(IDC_TOOLBAR,TB_SETHOTIMAGELIST,0,LPARAM(HIMAGELIST(m_ToolbarILHover)));
	SendDlgItemMessage(IDC_TOOLBAR,TB_SETDISABLEDIMAGELIST,0,LPARAM(HIMAGELIST(m_ToolbarILDisabled)));
	
	TBBUTTON tb;
	memset(&tb,0,sizeof(TBBUTTON));
	tb.idCommand=IDC_UP;
	tb.fsStyle=TBSTYLE_BUTTON;
	tb.fsState=TBSTATE_ENABLED;
	tb.iBitmap=0;
	SendDlgItemMessage(IDC_TOOLBAR,TB_INSERTBUTTON,0,LPARAM(&tb));
	tb.idCommand=IDC_DOWN;
	tb.iBitmap=1;
	SendDlgItemMessage(IDC_TOOLBAR,TB_INSERTBUTTON,1,LPARAM(&tb));
	EnableDlgItem(IDC_TOOLBAR,m_pList->GetNextItem(-1,LVNI_SELECTED)!=-1);

	CComboBox cEncoding(GetDlgItem(IDC_ENCODING));
	cEncoding.AddString(ID2W(IDS_SAVERESULTSANSI));
	cEncoding.AddString(ID2W(IDS_SAVERESULTSUNICODE));
	cEncoding.AddString(ID2W(IDS_SAVERESULTSUTF8));
	if (m_nFlags&RESULT_ENCODINGUTF8)
		cEncoding.SetCurSel(2);
	else if (m_nFlags&RESULT_ENCODINGUNICODE)
		cEncoding.SetCurSel(1);
	else
		cEncoding.SetCurSel(0);


	AddTemplates();
	
	// Setting dialog items to correspond with m_nFlags
	if (m_nFlags&RESULT_INCLUDEDATE)
		CheckDlgButton(IDC_DATE,1);
	if (m_nFlags&RESULT_INCLUDELABELS)
		CheckDlgButton(IDC_LABELS,1);
	if (m_nFlags&RESULT_INCLUDEDBINFO)
		CheckDlgButton(IDC_DBINFO,1);
	if (m_nFlags&RESULT_INCLUDEDESCRIPTION)
	{
		CheckDlgButton(IDC_DESCRIPTIONTOGGLE,1);
		EnableDlgItem(IDC_DESCRIPTION,1);
		SetDlgItemText(IDC_DESCRIPTION,m_strDescription);
	}
	else
		EnableDlgItem(IDC_DESCRIPTION,0);

	if (m_nFlags&RESULT_ACTIVATESELECTEDITEMS)
	{
		if (m_nFlags&RESULT_INCLUDESELECTEDITEMS)
			CheckDlgButton(IDC_SELECTEDITEMS,1);
		else
			CheckDlgButton(IDC_ALLITEMS,1);
	}
	else
	{
		CheckDlgButton(IDC_ALLITEMS,1);
		EnableDlgItem(IDC_SELECTEDITEMS,FALSE);
	}

	CLocateDlg::ViewDetails* pDetails=CLocateDlg::GetDefaultDetails();

	// Inserting details to list view and checking selected
	int nItem;
	CString Title;
	for (nItem=0;nItem<m_aDetails.GetSize();nItem++)
	{
		Title.LoadString(pDetails[m_aDetails[nItem]].nString,LanguageSpecificResource);
		m_pList->InsertItem(LVIF_TEXT|LVIF_PARAM,nItem,
			Title,0,0,0,LPARAM(m_aDetails[nItem]));
		m_pList->SetCheckState(nItem,TRUE);
	}

	for (int nDetail=0;nDetail<=LastType;nDetail++)
	{
		if (m_aDetails.Find(nDetail)==-1)
		{
			Title.LoadString(pDetails[nDetail].nString,LanguageSpecificResource);
			m_pList->InsertItem(LVIF_TEXT|LVIF_PARAM,nItem++,
				Title,0,0,0,LPARAM(nDetail));
			m_pList->SetCheckState(nItem,FALSE);
		}
	}
	delete[] pDetails;
	
	return CFileDialog::OnInitDialog(hwndFocus);
}


void CSaveResultsDlg::AddTemplates()
{
	CComboBox cTemplate(GetDlgItem(IDC_TEMPLATE));
	cTemplate.ResetContent();
	cTemplate.AddString(ID2W(IDS_SAVERESULTSNOTEMPLATE));
	cTemplate.SetCurSel(0);
	
	CStringW TemplatePath(GetLocateApp()->GetExeNameW());
	TemplatePath.FreeExtra(TemplatePath.FindLast(L'\\')+1);
	TemplatePath << L"templates\\";
	
	CFileFind ff;
	BOOL bRet=ff.FindFile(TemplatePath+L"*.ret");
	while (bRet)
	{
		CStringW Name;
		ff.GetFileName(Name);
		if (!Name.IsEmpty())
		{
			int iLen=Name.FindLast(L'.');
			if (iLen!=-1 && Name.GetLength()-iLen==4)
			{
				m_TemplateFiles.Add((TemplatePath+Name).GiveBuffer());
			
				BOOL bActivateThis=m_strTemplate==Name;

				Name.FreeExtra(iLen);
				int iRet=cTemplate.AddString(Name);

				if (iRet!=CB_ERR && bActivateThis)
					cTemplate.SetCurSel(iRet);

			}
		}

		bRet=ff.FindNextFile();
	}

}

BOOL CSaveResultsDlg::ItemUpOrDown(BOOL bUp)
{
	int nSelected=m_pList->GetNextItem(-1,LVNI_SELECTED);
	ASSERT(nSelected!=-1);

	int nOther=m_pList->GetNextItem(nSelected,bUp?LVNI_ABOVE:LVNI_BELOW);
	if (nOther==-1 || nOther==nSelected)
		return FALSE;

	// This is found to be the best way to do this
	LVITEM li;
	BOOL bSelected=m_pList->GetCheckState(nSelected);
	li.mask=LVIF_STATE|LVIF_PARAM;
	li.stateMask=0xFFFFFFFF;
	li.iItem=nSelected;
	li.iSubItem=0;
	m_pList->GetItem(&li);
	m_pList->SetItemData(nSelected,NULL);
	m_pList->DeleteItem(nSelected);
	li.iItem=nOther;
	li.mask=LVIF_PARAM|LVIF_STATE|LVIF_TEXT;
	li.pszText=LPSTR_TEXTCALLBACK;
	nOther=m_pList->InsertItem(&li);
	m_pList->SetCheckState(nOther,bSelected);
	m_pList->EnsureVisible(nOther,FALSE);
	m_pList->SetFocus();
	return TRUE;
}

BOOL CSaveResultsDlg::OnFileNameOK()
{
	m_nFlags=0;
	if (IsDlgButtonChecked(IDC_DATE))
		m_nFlags=RESULT_INCLUDEDATE;
	if (IsDlgButtonChecked(IDC_LABELS))
		m_nFlags|=RESULT_INCLUDELABELS;
	if (IsDlgButtonChecked(IDC_DBINFO))
		m_nFlags|=RESULT_INCLUDEDBINFO;
	if (IsDlgButtonChecked(IDC_DESCRIPTIONTOGGLE))
	{
		m_nFlags|=RESULT_INCLUDEDESCRIPTION;
		GetDlgItemText(IDC_DESCRIPTION,m_strDescription);
	}
	else
		m_strDescription.Empty();
	if (IsDlgButtonChecked(IDC_SELECTEDITEMS))
		m_nFlags|=RESULT_INCLUDESELECTEDITEMS;

	int nTemplateSel=(int)SendDlgItemMessage(IDC_TEMPLATE,CB_GETCURSEL)-1;
	if (nTemplateSel>=0 && nTemplateSel<m_TemplateFiles.GetSize())
		m_strTemplate=m_TemplateFiles[nTemplateSel];
	else
		m_strTemplate.Empty();


	switch (SendDlgItemMessage(IDC_ENCODING,CB_GETCURSEL))
	{
	case 2:
		m_nFlags|=RESULT_ENCODINGUTF8;
		break;
	case 1:
		m_nFlags|=RESULT_ENCODINGUNICODE;
		break;
	}	
	

	int nItem=m_pList->GetNextItem(-1,LVNI_ALL);
	m_aDetails.RemoveAll();
	while (nItem!=-1)
	{
		if (m_pList->GetCheckState(nItem))
			m_aDetails.Add((INT)m_pList->GetItemData(nItem));	
		nItem=m_pList->GetNextItem(nItem,LVNI_ALL);
	}

	return TRUE;
}


BOOL CSaveResultsDlg::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	switch (idCtrl)
	{
	case IDC_DETAILS:
		return ListNotifyHandler((NMLISTVIEW*)pnmh);
	}
	return CFileDialog::OnNotify(idCtrl,pnmh);
}

BOOL CSaveResultsDlg::ListNotifyHandler(NMLISTVIEW *pNm)
{
	switch(pNm->hdr.code)
	{
	case LVN_GETDISPINFO:
		{
			LV_DISPINFO *pLvdi=(LV_DISPINFO *)pNm;
			if (pLvdi->item.lParam<=LastType)
			{
				if (g_szBuffer!=NULL)
					delete[] g_szBuffer;

				g_szBuffer=allocstring(IDS_LISTNAME+(UINT)pLvdi->item.lParam,LanguageSpecificResource);
				pLvdi->item.pszText=g_szBuffer;
			}
			break;
		}
	case LVN_GETDISPINFOW:
		{
			LV_DISPINFOW *pLvdi=(LV_DISPINFOW*)pNm;
			if (pLvdi->item.lParam<=LastType)
			{
				if (g_szwBuffer!=NULL)
					delete[] g_szwBuffer;

				g_szwBuffer=allocstringW(IDS_LISTNAME+(UINT)pLvdi->item.lParam,LanguageSpecificResource);
				pLvdi->item.pszText=g_szwBuffer;
			}
			break;
		}
	case LVN_ITEMCHANGED:
		if (pNm->uNewState&LVIS_SELECTED)
		{
			EnableDlgItem(IDC_TOOLBAR,TRUE);

			SendDlgItemMessage(IDC_TOOLBAR,TB_SETSTATE,IDC_UP,m_pList->GetNextItem(pNm->iItem,LVNI_ABOVE)!=-1?TBSTATE_ENABLED:0);
			SendDlgItemMessage(IDC_TOOLBAR,TB_SETSTATE,IDC_DOWN,m_pList->GetNextItem(pNm->iItem,LVNI_BELOW)!=-1?TBSTATE_ENABLED:0);
			
		}
		else if (pNm->uOldState&LVNI_SELECTED)
			EnableDlgItem(IDC_TOOLBAR,FALSE);

		break;
	}
	return FALSE;
}


BOOL CSaveResultsDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch (wID)
	{
	case IDC_DESCRIPTIONTOGGLE:
		EnableDlgItem(IDC_DESCRIPTION,IsDlgButtonChecked(IDC_DESCRIPTIONTOGGLE));
		break;
	case IDC_UP:
		ItemUpOrDown(TRUE);
		break;
	case IDC_DOWN:
		ItemUpOrDown(FALSE);
		break;
	}
	return CFileDialog::OnCommand(wID,wNotifyCode,hControl);
}

