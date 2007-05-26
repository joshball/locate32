////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2007 Janne Huttunen
////////////////////////////////////////////////////////////////////

#ifndef HFCMEMORY_INL
#define HFCMEMORY_INL



inline void fMemCopy(void* dst,const void* src,SIZE_T len) 
{
	for (register ULONG_PTR i=0;i<len;i++)
		((BYTE*)dst)[i]=((BYTE*)src)[i];
}


inline void fMemSet(void* dst,BYTE byte,SIZE_T count) // faster when count is small
{
	for (register ULONG_PTR i=0;i<count;i++)
		((BYTE*)dst)[i]=byte;
}

/////////////////////////////////////////////////
// Inline functions
/////////////////////////////////////////////////

////////////////////////////////////////////////
// CAutoPtr

template<class TYPE> 
inline CAutoPtr<TYPE>::~CAutoPtr()
{
	if (m_data!=NULL)
		delete m_data;
}

template<class TYPE> 
inline TYPE* CAutoPtr<TYPE>::Release()
{
	TYPE* tmp=m_data;
	m_data=NULL;
	return tmp;
}

template<class TYPE> 
inline void CAutoPtr<TYPE>::Assign(TYPE* data)
{
	if (m_data!=NULL)
		delete m_data;
	m_data=data;
}

template<class TYPE> 
inline void CAutoPtr<TYPE>::Assign(CAutoPtr<TYPE>& another)
{
	if (m_data!=NULL)
		delete m_data;
	m_data=another.Release();
}

template<class TYPE> 
inline CAutoPtr<TYPE>& CAutoPtr<TYPE>::operator=(TYPE* data)
{
	Assign(data);
	return *this;
}

	
template<class TYPE> 
inline CAutoPtr<TYPE>& CAutoPtr<TYPE>::operator=(CAutoPtr<TYPE>& another)
{
	Assign(another);
	return *this;
}



////////////////////////////////////////////////
// CAutoPtrA

template<class TYPE> 
inline CAutoPtrA<TYPE>::~CAutoPtrA()
{
	if (m_data!=NULL)
		delete[] m_data;
}

template<class TYPE> 
inline TYPE* CAutoPtrA<TYPE>::Release()
{
	TYPE* tmp=m_data;
	m_data=NULL;
	return tmp;
}

template<class TYPE> 
inline void CAutoPtrA<TYPE>::Assign(TYPE* data)
{
	if (m_data!=NULL)
		delete[] m_data;
	m_data=data;
}

template<class TYPE> 
inline void CAutoPtrA<TYPE>::Assign(CAutoPtr<TYPE>& another)
{
	if (m_data!=NULL)
		delete[] m_data;
	m_data=another.Release();
}

template<class TYPE> 
inline CAutoPtrA<TYPE>& CAutoPtrA<TYPE>::operator=(TYPE* data)
{
	Assign(data);
	return *this;
}
	
template<class TYPE> 
inline CAutoPtrA<TYPE>& CAutoPtrA<TYPE>::operator=(CAutoPtrA<TYPE>& another)
{
	Assign(another);
	return *this;
}






////////////////////////////////////////////
// CGlobalAlloc

#ifdef WIN32

inline CGlobalAlloc::CGlobalAlloc(SIZE_T nSize,allocFlags nType)
:	m_hGlobal(NULL)
{
	Alloc(nSize,nType);
}

inline CGlobalAlloc::CGlobalAlloc(HGLOBAL hGlobal,allocFlags nFlags)
:	m_hGlobal(hGlobal),m_nFlags(nFlags)
{
	m_pData=m_nFlags&moveable?NULL:(BYTE*)m_hGlobal;
}
	
inline CGlobalAlloc::~CGlobalAlloc()
{
	if (m_nFlags&moveable && m_pData!=NULL)
		GlobalUnlock(m_hGlobal);
	GlobalFree(m_hGlobal);
}

inline BOOL CGlobalAlloc::IsAllocated() const
{
	return m_hGlobal!=NULL;
}

inline void CGlobalAlloc::Lock()
{
	if (m_pData==NULL && m_hGlobal!=NULL)
		m_pData=(BYTE*)GlobalLock(m_hGlobal);
}

inline SIZE_T CGlobalAlloc::GetSize() const
{
	return GlobalSize(m_hGlobal);
}

inline void CGlobalAlloc::Unlock()
{
	if (m_nFlags&moveable && m_pData!=NULL)
	{
		GlobalUnlock(m_hGlobal);
		m_pData=NULL;
	}
}

inline BOOL CGlobalAlloc::IsLocked() const
{
	return m_pData!=NULL;
}

inline CGlobalAlloc::allocFlags CGlobalAlloc::GetFlags() const
{
	return (CGlobalAlloc::allocFlags)GlobalFlags(m_hGlobal);
}

////////////////////////////////////////////
// CHeap

inline CHeap::~CHeap()
{
	if (m_bDestroy)
		::HeapDestroy(m_hHeap);
}

inline CHeap::CHeap()
:	m_bDestroy(FALSE),m_hHeap(NULL)
{
}

inline CHeap::CHeap(SIZE_T dwInitialSize,SIZE_T dwMaximumSize,attributes nAttributes)
:	m_bDestroy(FALSE)
{
	Create(dwInitialSize,dwMaximumSize,nAttributes);
}

inline BOOL CHeap::IsAllocated()
{
	return m_hHeap!=NULL;
}

inline SIZE_T CHeap::Compact(attributes nAttributes)
{
	return HeapCompact(m_hHeap,nAttributes);
}

inline BOOL CHeap::Lock()
{
	return HeapLock(m_hHeap);
}

inline BOOL CHeap::Unlock()
{
	return HeapUnlock(m_hHeap);
}

inline BOOL CHeap::Validate(LPCVOID lpBlock,attributes nAttributes)
{
	return HeapValidate(m_hHeap,nAttributes,lpBlock);
}

inline BOOL CHeap::Walk(LPPROCESS_HEAP_ENTRY lpEntry)
{
	return HeapWalk(m_hHeap,lpEntry);
}

inline CHeap::CHeapBlock* CHeap::Alloc(SIZE_T nSize,attributes nAttributes)
{
	return new CHeapBlock(HeapAlloc(m_hHeap,nAttributes|m_bThrow?HEAP_GENERATE_EXCEPTIONS:0,nSize));
}

inline CHeap CHeap::GetProcessHeap()
{
	return CHeap(::GetProcessHeap());
}

inline BOOL CHeap::Destroy()
{
	if (m_hHeap==NULL)
		return FALSE;
	BOOL nRet=HeapDestroy(m_hHeap);
	m_hHeap=NULL;
	return nRet;
}

#endif

#endif

