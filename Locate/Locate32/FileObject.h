#if !defined(FILEOBJECT_H)
#define FILEOBJECT_H

#if _MSC_VER >= 1000
#pragma once
#endif


class CFileObject : public CCoDataObject
{
protected:
	class CEnumFORMATETC : public CCoEnumFORMATETC
	{
	protected:
		ULONG	m_format;

	public:
		CEnumFORMATETC();
		
		virtual HRESULT STDMETHODCALLTYPE Next(ULONG celt,FORMATETC __RPC_FAR *rgelt,ULONG __RPC_FAR *pceltFetched);
		virtual HRESULT STDMETHODCALLTYPE Skip(ULONG celt);
		virtual HRESULT STDMETHODCALLTYPE Reset(void);
		virtual HRESULT STDMETHODCALLTYPE Clone(IEnumFORMATETC __RPC_FAR *__RPC_FAR *ppenum);
	
#ifdef _DEBUG
	public:
		inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
		inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
		inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
	};

	CArrayFP<CStringW*> m_Files;
	CArrayFP<CPoint*> m_Points;
	CPoint m_StartPosition;

public:
	CFileObject();
	virtual ~CFileObject();

	BYTE SetFile(LPCWSTR szFile); 
	BYTE SetFiles(CListCtrl* pList);
	HGLOBAL GetFileNameA();
	HGLOBAL GetFileNameW();

	HGLOBAL GetHDrop();
	HGLOBAL GetItemIDList();
	HGLOBAL GetItemIDListOffset();
	HGLOBAL GetFileNameMapA();
	HGLOBAL GetFileNameMapW();

	virtual HRESULT STDMETHODCALLTYPE GetData(FORMATETC *pformatetcIn,STGMEDIUM *pmedium);
	virtual HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC *pformatetc,STGMEDIUM *pmedium);
	virtual HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC *pformatetc);

	virtual HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC *pformatectIn,FORMATETC *pformatetcOut);
	virtual HRESULT STDMETHODCALLTYPE SetData(FORMATETC *pformatetc,STGMEDIUM *pmedium,BOOL fRelease);
	virtual HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection,IEnumFORMATETC** ppenumFormatEtc);

	virtual HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC *pformatetc,DWORD advf,IAdviseSink *pAdvSink,DWORD *pdwConnection);
	virtual HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection);

	virtual HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA** ppenumAdvise);

#ifdef _DEBUG
public:
	void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
};

class CFileSource : public CCoDropSource
{
public:
	virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState);
    virtual HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect);
#ifdef _DEBUG
public:
	void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
};

class CFileTarget : public CCoDropTarget
{
public:
	CFileTarget();
	virtual ~CFileTarget();

	virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject __RPC_FAR *pDataObj,
		DWORD grfKeyState,POINTL pt,DWORD __RPC_FAR *pdwEffect);
	virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState,
		POINTL pt,DWORD __RPC_FAR *pdwEffect);
	virtual HRESULT STDMETHODCALLTYPE DragLeave(void);
	virtual HRESULT STDMETHODCALLTYPE Drop(IDataObject __RPC_FAR *pDataObj,
		DWORD grfKeyState,POINTL pt,DWORD __RPC_FAR *pdwEffect);
#ifdef _DEBUG
public:
	void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif

	IDataObject* m_pDataObjectInWindow;
};



inline CFileObject::CEnumFORMATETC::CEnumFORMATETC()
:	m_format(1)
{
}

inline CFileObject::CFileObject()
{
	GetCursorPos(&m_StartPosition);
	DebugFormatMessage("FileObject %X created.",(DWORD)this);
}

inline CFileObject::~CFileObject()
{
	//m_Files.RemoveAll();
	//m_Points.RemoveAll();
	DebugFormatMessage("FileObject %X destroyed.",(DWORD)this);
}


inline CFileTarget::CFileTarget()
:	m_pDataObjectInWindow(NULL)
{
}

inline CFileTarget::~CFileTarget()
{
	if (m_pDataObjectInWindow!=NULL)
	{
		m_pDataObjectInWindow->Release();
		m_pDataObjectInWindow=NULL;
	}
}

#endif
