#include <HFCLib.h>
#include "Locate32.h"

#define DEF_FORMATS		7



HRESULT STDMETHODCALLTYPE CFileObject::CEnumFORMATETC::Next(ULONG celt,FORMATETC __RPC_FAR *rgelt,ULONG __RPC_FAR *pceltFetched)
{
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
			break;
		case 2:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_FILENAME);
			break;
		case 3:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_FILENAMEW);
			break;
		case 4:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_FILENAMEMAPA);
			break;
		case 5:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_FILENAMEMAPW);
			break;
		case 6:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_SHELLIDLIST);
			break;
		case 7:
			rgelt[counter].cfFormat=RegisterClipboardFormat(CFSTR_SHELLIDLISTOFFSET);
			break;
		}
		m_format++;
		counter++;
	}
	if (pceltFetched!=NULL)
		*pceltFetched=counter;
	if (counter!=celt)
		return S_FALSE;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileObject::CEnumFORMATETC::Skip(ULONG celt)
{
	m_format+=celt;
	if (m_format>DEF_FORMATS)
		return S_FALSE;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileObject::CEnumFORMATETC::Reset(void)
{
	m_format=1;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileObject::CEnumFORMATETC::Clone(IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenum)
{
	*ppenum=new CEnumFORMATETC;
	if (*ppenum==NULL)
		return E_OUTOFMEMORY;
	((CEnumFORMATETC*)*ppenum)->AddRef();
	((CEnumFORMATETC*)*ppenum)->m_format=m_format;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileObject::GetData(FORMATETC *pformatetcIn,STGMEDIUM *pmedium)
{
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
			sMemCopy((LPSTR)pmedium->hGlobal,(LPCSTR)*m_Files[0],len+1);
		}
		else if (strcasecmp(CFSTR_FILENAMEW,szFormat)==0)
		{
			int len=m_Files[0]->GetLength();
			pmedium->hGlobal=GlobalAlloc(GMEM_DDESHARE|GMEM_FIXED,len*2+4);
			MultiByteToWideChar(CP_ACP,0,(LPCSTR)*m_Files[0],-1,(LPWSTR)pmedium->hGlobal,len+4);
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
	if (pformatetc==NULL)
		return E_INVALIDARG;
	if (pformatetc->lindex!=-1)
		return DV_E_LINDEX;
	if (pformatetc->tymed!=TYMED_HGLOBAL)
		return DV_E_TYMED;
	if (pformatetc->dwAspect!=DVASPECT_CONTENT)
		return DV_E_DVASPECT;
	if (pformatetc->cfFormat==CF_HDROP)
		return S_OK;
	if (pformatetc->cfFormat==CF_TEXT)
		return S_OK;
	char szFormat[100];
	if (GetClipboardFormatName(pformatetc->cfFormat,szFormat,100))
	{
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
	return DV_E_FORMATETC;
}

HRESULT STDMETHODCALLTYPE CFileObject::GetCanonicalFormatEtc(FORMATETC *pformatectIn,FORMATETC *pformatetcOut)
{
	return 0;
}

HRESULT STDMETHODCALLTYPE CFileObject::SetData(FORMATETC *pformatetc,STGMEDIUM *pmedium,BOOL fRelease)
{	
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
			m_Files.Add(new CString(szPath));
		}
		DragFinish((HDROP)pmedium->hGlobal);
	}
	else if (pformatetc->cfFormat>0x100)
	{
		char szFormat[100];
		GetClipboardFormatName(pformatetc->cfFormat,szFormat,100);
		if (strcasecmp(CFSTR_FILENAME,szFormat)==0)
		{
			m_Files.RemoveAll();
			m_Points.RemoveAll();
			m_Files.Add(new CString((LPCSTR)pmedium->hGlobal));
		}
		else if (strcasecmp(CFSTR_FILENAMEW,szFormat)==0)
		{
			m_Files.RemoveAll();
			m_Points.RemoveAll();
			m_Files.Add(new CString((LPCWSTR)pmedium->hGlobal));
		}
		else if (strcasecmp(CFSTR_TARGETCLSID,szFormat)==0)
			return S_OK;
		else if (strcasecmp(CFSTR_LOGICALPERFORMEDDROPEFFECT,szFormat)==0)
			return S_OK;
		else if (strcasecmp(CFSTR_PERFORMEDDROPEFFECT,szFormat)==0)
			return S_OK;
		else if (strcasecmp(CFSTR_PREFERREDDROPEFFECT,szFormat)==0)
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
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CFileObject::DUnadvise(DWORD dwConnection)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CFileObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
	return E_NOTIMPL;
}

BYTE CFileObject::SetFile(LPCSTR szFile)
{
	m_Files.RemoveAll();
	m_Points.RemoveAll();
	m_Files.Add(new CString(szFile));
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
			m_Files.Add(new CString(pData->GetPath()));
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
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	
	HGLOBAL hGlobal;
	DWORD nDataLength=sizeof(DROPFILES)+2;
	
	if (osvi.dwPlatformId==VER_PLATFORM_WIN32_NT)
	{
		// Win2000/XP needs Unicode
		for (int i=0;i<m_Files.GetSize();i++)
			nDataLength+=(m_Files[i]->GetLength()+1)*2;
		hGlobal=GlobalAlloc(GPTR,nDataLength);
		if (hGlobal==NULL)
			return NULL;
		((DROPFILES*)hGlobal)->fNC=TRUE;
		GetCursorPos(&((DROPFILES*)hGlobal)->pt);
		((DROPFILES*)hGlobal)->fWide=TRUE;
		((DROPFILES*)hGlobal)->pFiles=sizeof(DROPFILES);
		LPSTR pDst=(LPSTR)hGlobal+sizeof(DROPFILES);
		for (i=0;i<m_Files.GetSize();i++)
		{
			MemCopyAtoW(pDst,*m_Files[i],m_Files[i]->GetLength()+1);
			pDst+=(m_Files[i]->GetLength()+1)*2;
		}
		*pDst='\0';

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
		for (i=0;i<m_Files.GetSize();i++)
		{
			DebugFormatMessage("HDROP: file: %s",(LPCSTR)*m_Files[i]);
			sMemCopy(pDst,(LPCSTR)*m_Files[i],m_Files[i]->GetLength()+1);
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
	sMemCopy((LPSTR)hGlobal,m_Files.GetAt(0)->GetBuffer(),m_Files.GetAt(0)->GetLength()+1);
	return hGlobal;
}

HGLOBAL CFileObject::GetFileNameW()
{
	if (m_Files.GetSize()==0)
		return NULL;

	HGLOBAL hGlobal=GlobalAlloc(GPTR,(m_Files.GetAt(0)->GetLength()+1)*2);
	MemCopyAtoW((LPSTR)hGlobal,m_Files.GetAt(0)->GetBuffer(),m_Files.GetAt(0)->GetLength()+1);
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
		sMemCopy(pDst,(LPCSTR)(*m_Files[i])+pNameStartPos[i],m_Files[i]->GetLength()+1-pNameStartPos[i]);
		pDst+=m_Files[i]->GetLength()+1-pNameStartPos[i];
	}
	*pDst='\0';
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
		MultiByteToWideChar(CP_ACP,0,(LPCSTR)(*m_Files[i])+pNameStartPos[i],-1,pDst,m_Files[i]->GetLength()+1-pNameStartPos[i]);
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
		if (pDataObj->QueryGetData(&FormatEtc)==S_OK)
		{
			*pdwEffect=DROPEFFECT_MOVE;
			return S_OK;
		}
	}
	*pdwEffect=DROPEFFECT_NONE;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileTarget::DragOver(DWORD grfKeyState,
		POINTL pt,DWORD __RPC_FAR *pdwEffect)
{
	*pdwEffect=DROPEFFECT_MOVE;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileTarget::DragLeave(void)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CFileTarget::Drop(IDataObject __RPC_FAR *pDataObj,
		DWORD grfKeyState,POINTL pt,DWORD __RPC_FAR *pdwEffect)
{
	DebugMessage("CFileTarget::Drop(IDataObject __RPC_FAR *pDataObj,DWORD grfKeyState,POINTL pt,DWORD __RPC_FAR *pdwEffect)");
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
