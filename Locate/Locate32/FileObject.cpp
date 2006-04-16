#include <HFCLib.h>
#include "Locate32.h"

#define DEF_FORMATS		7



HRESULT STDMETHODCALLTYPE CFileObject::CEnumFORMATETC::Next(ULONG celt,FORMATETC __RPC_FAR *rgelt,ULONG __RPC_FAR *pceltFetched)
{
	FoDebugFormatMessage4("CFileObject::CEnumFORMATETC::Next(%X,%X,%X)",celt,rgelt,pceltFetched,NULL);
	
	ULONG counter=0;
	while (counter<celt)
	{
		rgelt[counter].lindex=-1;
		rgelt[counter].ptd=NULL;
		rgelt[counter].dwAspect=DVASPECT_CONTENT;
		rgelt[counter].tymed=1;

		if (m_format>DEF_FORMATS)
		{
			rgelt[counter].tymed=0;
			rgelt[counter].cfFormat=0;
			break;
		}
		
		switch (m_format)
		{
		case 1:
			rgelt[counter].cfFormat=CF_HDROP;
			FoDebugFormatMessage2("CFileObject::CEnumFORMATETC::Next, rgelt[%d].cfFormat=CF_HDROP",counter,0);
			break;
		case 2:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_FILENAME);
			FoDebugFormatMessage2("CFileObject::CEnumFORMATETC::Next, rgelt[%d].cfFormat=%X='CFSTR_FILENAME'",counter,rgelt[counter].cfFormat);
			break;
		case 3:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_FILENAMEW);
			FoDebugFormatMessage2("CFileObject::CEnumFORMATETC::Next, rgelt[%d].cfFormat=%X='CFSTR_FILENAMEW'",counter,rgelt[counter].cfFormat);
			break;
		case 4:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_FILENAMEMAPA);
			FoDebugFormatMessage2("CFileObject::CEnumFORMATETC::Next, rgelt[%d].cfFormat=%X='CFSTR_FILENAMEMAPA'",counter,rgelt[counter].cfFormat);
			break;
		case 5:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_FILENAMEMAPW);
			FoDebugFormatMessage2("CFileObject::CEnumFORMATETC::Next, rgelt[%d].cfFormat=%X='CFSTR_FILENAMEMAPW'",counter,rgelt[counter].cfFormat);
			break;
		case 6:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_SHELLIDLIST);
			FoDebugFormatMessage2("CFileObject::CEnumFORMATETC::Next, rgelt[%d].cfFormat=%X='CFSTR_SHELLIDLIST'",counter,rgelt[counter].cfFormat);
			break;
		case 7:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_SHELLIDLISTOFFSET);
			FoDebugFormatMessage2("CFileObject::CEnumFORMATETC::Next, rgelt[%d].cfFormat=%X='CFSTR_SHELLIDLISTOFFSET'",counter,rgelt[counter].cfFormat);
			break;
		default:
			FoDebugFormatMessage2("CFileObject::CEnumFORMATETC::Next, format %d is unknown",m_format,0);
			break;
		}
		m_format++;
		counter++;
	}
	if (pceltFetched!=NULL)
		*pceltFetched=counter;
	if (counter!=celt)
	{
		FoDebugMessage("CFileObject::CEnumFORMATETC::Next, will return S_FALSE");
		return S_FALSE;
	}
	FoDebugMessage("CFileObject::CEnumFORMATETC::Next, will return S_OK");
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileObject::CEnumFORMATETC::Skip(ULONG celt)
{
	FoDebugFormatMessage2("CFileObject::CEnumFORMATETC::Skip(%X)",celt,NULL);
	
	m_format+=celt;
	if (m_format>DEF_FORMATS)
		return S_FALSE;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileObject::CEnumFORMATETC::Reset(void)
{
	FoDebugMessage("CFileObject::CEnumFORMATETC::Reset");

	m_format=1;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileObject::CEnumFORMATETC::Clone(IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenum)
{
	FoDebugFormatMessage2("CFileObject::CEnumFORMATETC::Clone(%X)",ppenum,NULL);

	*ppenum=new CEnumFORMATETC;
	if (*ppenum==NULL)
		return E_OUTOFMEMORY;
	((CEnumFORMATETC*)*ppenum)->AddRef();
	((CEnumFORMATETC*)*ppenum)->m_format=m_format;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileObject::GetData(FORMATETC *pformatetcIn,STGMEDIUM *pmedium)
{
	FoDebugFormatMessage4("CFileObject::GetData(%X,%X), cfFormat=%X",pformatetcIn,pmedium,pformatetcIn->cfFormat,NULL);

	if (m_Files.GetSize()==0)
		return DV_E_FORMATETC;
	if (pformatetcIn==NULL || pmedium==NULL)
		return E_INVALIDARG;
	if (pformatetcIn->lindex!=-1)
		return DV_E_LINDEX;
	if ((pformatetcIn->tymed&TYMED_HGLOBAL)==0)
		return DV_E_TYMED;
	if (pformatetcIn->dwAspect!=DVASPECT_CONTENT)
		return DV_E_DVASPECT;
	
	pmedium->tymed=TYMED_HGLOBAL;
	pmedium->pUnkForRelease=NULL;
	
	if (pformatetcIn->cfFormat==0)
		pformatetcIn->cfFormat=CF_HDROP;
	else if (pformatetcIn->cfFormat==CF_HDROP)
		pmedium->hGlobal=GetHDrop();
	else if (pformatetcIn->cfFormat>0x100)
	{
		char szFormat[100];
		GetClipboardFormatName(pformatetcIn->cfFormat,szFormat,100);
		if (strcasecmp(CFSTR_SHELLIDLIST,szFormat)==0)
			pmedium->hGlobal=GetItemIDList();
		else if (strcasecmp(CFSTR_SHELLIDLISTOFFSET,szFormat)==0 && m_Points.GetSize()>0)
			pmedium->hGlobal=GetItemIDListOffset();
		else if (strcasecmp(CFSTR_FILENAME,szFormat)==0)
		{
			int len=m_Files[0]->GetLength();
			pmedium->hGlobal=GlobalAlloc(GMEM_DDESHARE|GMEM_FIXED,len+2);
			WideCharToMultiByte(CP_ACP,0,(LPCWSTR)*m_Files[0],len+1,(LPSTR)pmedium->hGlobal,len+2,NULL,0);
		}
		else if (strcasecmp(CFSTR_FILENAMEW,szFormat)==0)
		{
			int len=m_Files[0]->GetLength();
			pmedium->hGlobal=GlobalAlloc(GMEM_DDESHARE|GMEM_FIXED,len*2+2);
			MemCopyW((LPWSTR)pmedium->hGlobal,(LPCWSTR)*m_Files[0],len+1);
		}
		else if (strcasecmp(CFSTR_FILENAMEMAPA,szFormat)==0)
			pmedium->hGlobal=GetFileNameMapA();
		else if (strcasecmp(CFSTR_FILENAMEMAPW,szFormat)==0)
			pmedium->hGlobal=GetFileNameMapW();
		else if (strcasecmp("Locate Item positions",szFormat)==0)
		{
			pmedium->hGlobal=GlobalAlloc(GMEM_FIXED,sizeof(POINT));
			sMemCopy(pmedium->hGlobal,&m_StartPosition,sizeof(POINT));
		}
		else
			return DV_E_FORMATETC;
	}
	else
		return DV_E_FORMATETC;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileObject::GetDataHere(FORMATETC *pformatetc,STGMEDIUM *pmedium)
{
	FoDebugFormatMessage4("CFileObject::GetDataHere(%X,%X), cfFormat=%X",pformatetc,pmedium,pformatetc->cfFormat,NULL);

	HRESULT hRes;
	STGMEDIUM med;
	if (pformatetc->tymed!=TYMED_HGLOBAL)
		return DV_E_TYMED;
	med.pUnkForRelease=pmedium->pUnkForRelease;
	hRes=GetData(pformatetc,&med);
	pmedium->tymed=med.tymed;
	sMemCopy(pmedium->hGlobal,med.hGlobal,GlobalSize(med.hGlobal));
	GlobalFree(med.hGlobal);
	return hRes;
}
	
HRESULT STDMETHODCALLTYPE CFileObject::QueryGetData(FORMATETC *pformatetc)
{
	FoDebugFormatMessage2("CFileObject::QueryGetData(%X), cfFormat=%X",pformatetc,pformatetc->cfFormat);

	if (pformatetc==NULL)
	{
		FoDebugMessage("CFileObject::QueryGetData: invalid argument");
		return E_INVALIDARG;
	}
	
	if (pformatetc->lindex!=-1)
	{
		FoDebugMessage("CFileObject::QueryGetData: invalid lindex");
		return DV_E_LINDEX;
	}
	

	if (pformatetc->tymed==0xFFFFFFFF)
		pformatetc->tymed=TYMED_HGLOBAL;
	else if (pformatetc->tymed!=TYMED_HGLOBAL)
	{
		FoDebugFormatMessage2("CFileObject::QueryGetData: invalid tymed %X",pformatetc->tymed,NULL);
		return DV_E_TYMED;
	}

	if (pformatetc->dwAspect!=DVASPECT_CONTENT)
	{
		FoDebugMessage("CFileObject::QueryGetData: invalid dwAspect");
		return DV_E_DVASPECT;
	}
	if (pformatetc->cfFormat==CF_HDROP)
	{
		FoDebugMessage("CFileObject::QueryGetData: format is CF_HDROP");
		return S_OK;
	}
	if (pformatetc->cfFormat==CF_TEXT)
	{
		FoDebugMessage("CFileObject::QueryGetData: format is CF_TEXT");
		return S_OK;
	}
	char szFormat[100];
	if (GetClipboardFormatName(pformatetc->cfFormat,szFormat,100))
	{
		FoDebugFormatMessage2("CFileObject::QueryGetData:, format is '%s'",szFormat,NULL);

		if (strcasecmp(szFormat,CFSTR_SHELLIDLIST)==0)
			return S_OK;
		if (strcasecmp(szFormat,CFSTR_SHELLIDLISTOFFSET)==0)
			return S_OK;
		if (strcasecmp(szFormat,CFSTR_FILENAME)==0)
			return S_OK;
		if (strcasecmp(szFormat,CFSTR_FILENAMEW)==0)
			return S_OK;
		if (strcasecmp(szFormat,"Locate Item positions")==0)
			return S_OK;
	}
	else
	{
		FoDebugFormatMessage2("CFileObject::QueryGetData:Failed to get clipboard format name for %X",pformatetc->cfFormat,NULL);
	}
	return DV_E_FORMATETC;
}

HRESULT STDMETHODCALLTYPE CFileObject::GetCanonicalFormatEtc(FORMATETC *pformatectIn,FORMATETC *pformatetcOut)
{
	FoDebugFormatMessage2("CFileObject::QueryGetData(%X,%X)",pformatectIn,pformatetcOut);
	return 0;
}

HRESULT STDMETHODCALLTYPE CFileObject::SetData(FORMATETC *pformatetc,STGMEDIUM *pmedium,BOOL fRelease)
{	
	FoDebugFormatMessage4("CFileObject::SetData(%X,%X,%X), pformatetc->cfFormat=%X",pformatetc,pmedium,fRelease,pformatetc->cfFormat);

	if (pformatetc==NULL || pmedium==NULL)
		return E_INVALIDARG;
	if (pformatetc->lindex!=-1)
		return DV_E_LINDEX;
	if (pformatetc->tymed!=TYMED_HGLOBAL)
		return DV_E_TYMED;
	if (pformatetc->dwAspect!=DVASPECT_CONTENT)
		return DV_E_DVASPECT;
	if (pformatetc->cfFormat==CF_HDROP)
	{
		char szPath[_MAX_PATH];
		m_Files.RemoveAll();
		m_Points.RemoveAll();
		for (int i=DragQueryFile((HDROP)pmedium->hGlobal,0xFFFFFFFF,NULL,0)-1;i>=0;i--)
		{
			DragQueryFile((HDROP)pmedium->hGlobal,i,szPath,_MAX_PATH);
			m_Files.Add(new CStringW(szPath));
		}
		DragFinish((HDROP)pmedium->hGlobal);
	}
	else if (pformatetc->cfFormat>0x100)
	{
		char szFormat[100];
		GetClipboardFormatName(pformatetc->cfFormat,szFormat,100);
		if (_stricmp(CFSTR_FILENAME,szFormat)==0)
		{
			m_Files.RemoveAll();
			m_Points.RemoveAll();
			m_Files.Add(new CStringW((LPCSTR)pmedium->hGlobal));
		}
		else if (_stricmp(CFSTR_FILENAMEW,szFormat)==0)
		{
			m_Files.RemoveAll();
			m_Points.RemoveAll();
			m_Files.Add(new CStringW((LPCWSTR)pmedium->hGlobal));
		}
		else if (_stricmp(CFSTR_TARGETCLSID,szFormat)==0)
			return S_OK;
		else if (_stricmp(CFSTR_LOGICALPERFORMEDDROPEFFECT,szFormat)==0)
			return S_OK;
		else if (_stricmp(CFSTR_PERFORMEDDROPEFFECT,szFormat)==0)
			return S_OK;
		else if (_stricmp(CFSTR_PREFERREDDROPEFFECT,szFormat)==0)
			return S_OK;
		else
			return DV_E_FORMATETC;
	}
	else
		return DV_E_FORMATETC;
	if (fRelease)
		ReleaseStgMedium(pmedium);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileObject::EnumFormatEtc(DWORD dwDirection,IEnumFORMATETC** ppenumFormatEtc)
{
	FoDebugFormatMessage2("CFileObject::EnumFormatEtc(%X,%X)",dwDirection,ppenumFormatEtc);

	if (dwDirection==DATADIR_GET)
	{
		*ppenumFormatEtc=new CEnumFORMATETC;
		((CEnumFORMATETC*)*ppenumFormatEtc)->AutoDelete();
		if (*ppenumFormatEtc==NULL)
			return E_OUTOFMEMORY;
		(*ppenumFormatEtc)->AddRef();
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CFileObject::DAdvise(FORMATETC *pformatetc,DWORD advf,IAdviseSink *pAdvSink,DWORD *pdwConnection)
{
	FoDebugFormatMessage4("CFileObject::DAdvise(%X,%X,%X,%X)",pformatetc,advf,pAdvSink,pdwConnection);
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CFileObject::DUnadvise(DWORD dwConnection)
{
	FoDebugFormatMessage2("CFileObject::DUnadvise(%X)",dwConnection,NULL);
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CFileObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
	FoDebugFormatMessage2("CFileObject::EnumDAdvise(%X)",ppenumAdvise,NULL);
	return E_NOTIMPL;
}

BYTE CFileObject::SetFile(LPCWSTR szFile)
{
	m_Files.RemoveAll();
	m_Points.RemoveAll();
	m_Files.Add(new CStringW(szFile));
	return TRUE;
}

BYTE CFileObject::SetFiles(CListCtrl* pList)
{
	m_Files.RemoveAll();
	m_Points.RemoveAll();
	int nItem=pList->GetNextItem(-1,LVNI_SELECTED);
	BOOL bGetPoints=((pList->GetStyle() & LVS_TYPEMASK)==LVS_ICON) || (pList->GetSelectedCount()==1);

	while (nItem!=-1)
	{
		CLocatedItem* pData=(CLocatedItem*)pList->GetItemData(nItem);
		if (pData!=NULL)
		{
			m_Files.Add(new CStringW(pData->GetPath()));
			if (bGetPoints)
			{
				CPoint* pt=new CPoint;
				pList->GetItemPosition(nItem,pt);
				pList->ClientToScreen(pt);
				m_Points.Add(pt);
			}
		}
		nItem=pList->GetNextItem(nItem,LVNI_SELECTED);
	}
	return TRUE;
}

HGLOBAL CFileObject::GetHDrop()
{
	HGLOBAL hGlobal;
	DWORD nDataLength=sizeof(DROPFILES)+2;
	
	if (IsFullUnicodeSupport())
	{
		// Win2000/XP needs Unicode
		int i;
		for (i=0;i<m_Files.GetSize();i++)
			nDataLength+=(m_Files[i]->GetLength()+1)*2;
		hGlobal=GlobalAlloc(GPTR,nDataLength);
		BYTE* pLock=(BYTE*)GlobalLock(hGlobal);


		if (pLock==NULL)
			return NULL;
		((DROPFILES*)pLock)->fNC=TRUE;
		GetCursorPos(&((DROPFILES*)pLock)->pt);
		((DROPFILES*)pLock)->fWide=TRUE;
		((DROPFILES*)pLock)->pFiles=sizeof(DROPFILES);
		LPWSTR pDst=(LPWSTR)(pLock+sizeof(DROPFILES));
		for (i=0;i<m_Files.GetSize();i++)
		{
			MemCopyW(pDst,*m_Files[i],m_Files[i]->GetLength()+1);
			pDst+=m_Files[i]->GetLength()+1;
		}
		*pDst=L'\0';
		
		GlobalUnlock(hGlobal);
	}
	else
	{
		for (int i=0;i<m_Files.GetSize();i++)
			nDataLength+=m_Files[i]->GetLength()+1;
		hGlobal=GlobalAlloc(GPTR,nDataLength);
		if (hGlobal==NULL)
			return NULL;
		((DROPFILES*)hGlobal)->fNC=TRUE;
		GetCursorPos(&((DROPFILES*)hGlobal)->pt);
		((DROPFILES*)hGlobal)->fWide=FALSE;
		((DROPFILES*)hGlobal)->pFiles=sizeof(DROPFILES);
		LPSTR pDst=(LPSTR)hGlobal+sizeof(DROPFILES);
		for (int i=0;i<m_Files.GetSize();i++)
		{
			DebugFormatMessage(L"HDROP: file: %s",(LPCWSTR)*m_Files[i]);
			MemCopyWtoA(pDst,(LPCWSTR)*m_Files[i],m_Files[i]->GetLength()+1);
			pDst+=m_Files[i]->GetLength()+1;
		}
		*pDst='\0';
	}
	
	
	return hGlobal;
}

HGLOBAL CFileObject::GetFileNameA()
{
	if (m_Files.GetSize()==0)
		return NULL;

	HGLOBAL hGlobal=GlobalAlloc(GPTR,m_Files.GetAt(0)->GetLength()+1);
	MemCopyWtoA((LPSTR)hGlobal,m_Files.GetAt(0)->GetBuffer(),m_Files.GetAt(0)->GetLength()+1);
	return hGlobal;
}

HGLOBAL CFileObject::GetFileNameW()
{
	if (m_Files.GetSize()==0)
		return NULL;

	HGLOBAL hGlobal=GlobalAlloc(GPTR,(m_Files.GetAt(0)->GetLength()+1)*2);
	MemCopyW((LPWSTR)hGlobal,m_Files.GetAt(0)->GetBuffer(),m_Files.GetAt(0)->GetLength()+1);
	return hGlobal;
}


HGLOBAL CFileObject::GetItemIDList()
{
	int i;
	LPITEMIDLIST* pItemLists=new LPITEMIDLIST[m_Files.GetSize()+1];
	DWORD* pItemLengths=new DWORD[m_Files.GetSize()+1];
	UINT nDataLength=sizeof(CIDA)+m_Files.GetSize()*sizeof(UINT);
	
	pItemLists[0]=NULL;
	nDataLength+=pItemLengths[0]=2;
	
	for (i=1;i<=m_Files.GetSize();i++)
	{
		pItemLists[i]=GetFileIDList(*m_Files[i-1]);
		nDataLength+=pItemLengths[i]=GetIDListSize(pItemLists[i]);
	}
	CIDA* pida=(CIDA*)GlobalAlloc(GPTR,nDataLength);
	pida->cidl=m_Files.GetSize();
	DWORD nOffset=pida->aoffset[0]=sizeof(CIDA)+m_Files.GetSize()*sizeof(UINT);
	if (pItemLists[0]==NULL)
	{
		*(WORD*)((LPSTR)pida+nOffset)=0;
		nOffset+=2;
		i=1;
	}
	else
		i=0;
	
	for (;i<=m_Files.GetSize();i++)
	{
		pida->aoffset[i]=nOffset;
		sMemCopy((LPSTR)pida+nOffset,pItemLists[i],pItemLengths[i]);
		nOffset+=pItemLengths[i];
	}
	if (pItemLists[0]!=NULL)
	{
		IMalloc* pMalloc;
		if (SHGetMalloc(&pMalloc)==NOERROR)
			pMalloc->Free(pItemLists[0]);
	}
	delete[] pItemLists;
	delete[] pItemLengths;
	return (HGLOBAL)pida;
}

HGLOBAL CFileObject::GetItemIDListOffset()
{
	POINT* pPoints=(POINT*)GlobalAlloc(GPTR,sizeof(POINT)*(m_Files.GetSize()+1));
	
	GetCursorPos(pPoints);
	for (int i=1;i<=m_Files.GetSize();i++)
	{
		pPoints[i].x=m_Points[i-1]->x-m_StartPosition.x;
		pPoints[i].y=m_Points[i-1]->y-m_StartPosition.y;
	}
	return (HGLOBAL)pPoints;
}

HGLOBAL CFileObject::GetFileNameMapA()
{
	DWORD nDataLength=2;
	int* pNameStartPos=new int[m_Files.GetSize()];
	int i;
	for (i=0;i<m_Files.GetSize();i++)
	{
		pNameStartPos[i]=m_Files[i]->FindLast('\\')+1;
		nDataLength+=m_Files[i]->GetLength()+1-pNameStartPos[i];
	}
	HGLOBAL hGlobal=GlobalAlloc(GPTR,nDataLength);
	if (hGlobal==NULL)
		return NULL;
	LPSTR pDst=(LPSTR)hGlobal;
	for (i=0;i<m_Files.GetSize();i++)
	{
		MemCopyWtoA(pDst,(LPCWSTR)(*m_Files[i])+pNameStartPos[i],m_Files[i]->GetLength()+1-pNameStartPos[i]);
		pDst+=m_Files[i]->GetLength()+1-pNameStartPos[i];
	}
	*pDst=L'\0';
	delete[] pNameStartPos;
	return hGlobal;
}

HGLOBAL CFileObject::GetFileNameMapW()
{
	DWORD nDataLength=2;
	int* pNameStartPos=new int[m_Files.GetSize()];
	int i;
	for (i=0;i<m_Files.GetSize();i++)
	{
		pNameStartPos[i]=m_Files[i]->FindLast('\\')+1;
		nDataLength+=m_Files[i]->GetLength()+1-pNameStartPos[i];
	}
	HGLOBAL hGlobal=GlobalAlloc(GPTR,nDataLength*2);
	if (hGlobal==NULL)
		return NULL;
	LPWSTR pDst=(LPWSTR)hGlobal;
	for (i=0;i<m_Files.GetSize();i++)
	{
		MemCopyW(pDst,(LPCWSTR)(*m_Files[i])+pNameStartPos[i],m_Files[i]->GetLength()+1-pNameStartPos[i]);
		pDst+=m_Files[i]->GetLength()+1-pNameStartPos[i];
	}
	*pDst='\0';
	delete[] pNameStartPos;
	return hGlobal;
}

HRESULT STDMETHODCALLTYPE CFileSource::QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState)
{
	if (fEscapePressed)
		return DRAGDROP_S_CANCEL;
	if (!(grfKeyState&MK_LBUTTON) && !(grfKeyState&MK_RBUTTON))
		return DRAGDROP_S_DROP;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileSource::GiveFeedback(DWORD dwEffect)
{
	return DRAGDROP_S_USEDEFAULTCURSORS;
}


HRESULT STDMETHODCALLTYPE CFileTarget::DragEnter(IDataObject __RPC_FAR *pDataObj,
		DWORD grfKeyState,POINTL pt,DWORD __RPC_FAR *pdwEffect)
{
	if (grfKeyState&MK_LBUTTON)
	{
		FORMATETC FormatEtc;
		FormatEtc.cfFormat=RegisterClipboardFormat("Locate Item positions");
		FormatEtc.ptd=NULL;
		FormatEtc.dwAspect=DVASPECT_CONTENT;
		FormatEtc.lindex=-1;
		FormatEtc.tymed=TYMED_HGLOBAL;
		if (pDataObj->QueryGetData(&FormatEtc)!=S_OK)
		{
			*pdwEffect=DROPEFFECT_NONE;
			return S_OK;
		}

		m_pDataObjectInWindow=pDataObj;
		m_pDataObjectInWindow->AddRef();

		*pdwEffect=DROPEFFECT_MOVE;
		return S_OK;			
	}
	*pdwEffect=DROPEFFECT_NONE;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileTarget::DragOver(DWORD grfKeyState,
		POINTL pt,DWORD __RPC_FAR *pdwEffect)
{
	if (m_pDataObjectInWindow!=NULL && grfKeyState&MK_LBUTTON)
	{
		FORMATETC FormatEtc;
		FormatEtc.cfFormat=RegisterClipboardFormat("Locate Item positions");
		FormatEtc.ptd=NULL;
		FormatEtc.dwAspect=DVASPECT_CONTENT;
		FormatEtc.lindex=-1;
		FormatEtc.tymed=TYMED_HGLOBAL;
		if (m_pDataObjectInWindow->QueryGetData(&FormatEtc)==S_OK)
		{
			*pdwEffect=DROPEFFECT_MOVE;
			return S_OK;
		}
	}
	*pdwEffect=DROPEFFECT_NONE;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileTarget::DragLeave(void)
{
	if (m_pDataObjectInWindow!=NULL)
	{
		m_pDataObjectInWindow->Release();
		m_pDataObjectInWindow=NULL;
	}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileTarget::Drop(IDataObject __RPC_FAR *pDataObj,
		DWORD grfKeyState,POINTL pt,DWORD __RPC_FAR *pdwEffect)
{
	DebugMessage("CFileTarget::Drop(IDataObject __RPC_FAR *pDataObj,DWORD grfKeyState,POINTL pt,DWORD __RPC_FAR *pdwEffect)");
	
	if (m_pDataObjectInWindow!=NULL)
	{
		ASSERT(m_pDataObjectInWindow==pDataObj);

		m_pDataObjectInWindow->Release();
		m_pDataObjectInWindow=NULL;
	}

	
	
	CListCtrl* pList=GetLocateDlg()->m_pListCtrl;
	if (pList->GetStyle()&LVS_REPORT)
		return S_OK;
	FORMATETC FormatEtc;
	FormatEtc.cfFormat=RegisterClipboardFormat("Locate Item positions");
	FormatEtc.ptd=NULL;
	FormatEtc.dwAspect=DVASPECT_CONTENT;
	FormatEtc.lindex=-1;
	FormatEtc.tymed=TYMED_HGLOBAL;
	STGMEDIUM Medium;
	if (pDataObj->GetData(&FormatEtc,&Medium)!=S_OK)
		return E_UNEXPECTED;
	POINT* pPoint=(POINT*)Medium.hGlobal;
	int nItem=pList->GetNextItem(-1,LVNI_SELECTED);
	while (nItem!=-1)
	{
		POINT pos;
		pList->GetItemPosition(nItem,&pos);
		pos.x+=pt.x-pPoint->x;
		pos.y+=pt.y-pPoint->y;
		pList->SetItemPosition(nItem,pos);
		nItem=pList->GetNextItem(nItem,LVNI_SELECTED);
	}
	ReleaseStgMedium(&Medium);
	return S_OK;
}
