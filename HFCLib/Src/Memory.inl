////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
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

/////////////////////////////////////////////////
// CBufferAllocator

#ifndef _DEBUG
inline CAllocator::CAllocator()
{
}

inline CAllocator::~CAllocator()
{
}

inline void CAllocator::FreeAll()
{
}

inline BYTE* CAllocator::Allocate(SIZE_T size)
{
	return new BYTE[size];
}

inline BYTE* CAllocator::AllocateFast(SIZE_T size)
{
	return Allocate(size);
}

inline void CAllocator::Free(void* pBlock)
{
	delete [] (BYTE*) pBlock;
}

inline void CAllocator::DebugInformation(CString& str)
{
}

inline CString CAllocator::GetAllocatorID() const
{
	CString id;
	id.Format("CAllocator on 0x%X",(void*)(this));
	return id;
}
#endif

/////////////////////////////////////////////////
// CDebugAllocator

#ifdef _DEBUG
template <DWORD EXTRA_ALLOC>
CDebugAllocator<EXTRA_ALLOC>::CDebugAllocator()
:	pFirstAlloc(NULL),pLastAlloc(NULL)
{
}

template <DWORD EXTRA_ALLOC>
CDebugAllocator<EXTRA_ALLOC>::~CDebugAllocator()
{
	FreeAll();
}

template <DWORD EXTRA_ALLOC>
void CDebugAllocator<EXTRA_ALLOC>::FreeAll()
{
	while (pFirstAlloc!=NULL)
	{
		pLastAlloc=pFirstAlloc;
		pFirstAlloc=pFirstAlloc->pNext;
		DebugNumMessage("CDebugAllocator::FreeAll(): 0x%X not freed properly",DWORD(pLastAlloc));
		free(pLastAlloc);
	}
	pLastAlloc=NULL;
}

template <DWORD EXTRA_ALLOC>
BYTE* CDebugAllocator<EXTRA_ALLOC>::Allocate(SIZE_T size)
{
	ALLOC* tmp=(ALLOC*)malloc(sizeof(ALLOC)+size+EXTRA_ALLOC);
	if (tmp==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return NULL;
	}
	tmp->dwLength=size;
	tmp->line=-1;
	tmp->szFile=NULL;
	tmp->pNext=NULL;
	tmp->pPrev=pLastAlloc;
	if (pLastAlloc!=NULL)
		pLastAlloc->pNext=tmp;
	else
		pFirstAlloc=tmp;
	pLastAlloc=tmp;

	for (int i=0;i<EXTRA_ALLOC;i++)
		((BYTE*)tmp)[i+sizeof(ALLOC)+size]=0xab;

	return LPBYTE(tmp)+sizeof(ALLOC);
}

template <DWORD EXTRA_ALLOC>
NDEBUGINLINE BYTE* CDebugAllocator<EXTRA_ALLOC>::AllocateFast(SIZE_T size)
{
	return Allocate(size);
}

template <DWORD EXTRA_ALLOC>
BYTE* CDebugAllocator<EXTRA_ALLOC>::Allocate(SIZE_T size,int line,char* szFile)
{
	ALLOC* tmp=(ALLOC*)malloc(sizeof(ALLOC)+size+EXTRA_ALLOC);
	if (tmp==NULL)
	{
		SetHFCError(HFC_CANNOTALLOC);
		return NULL;
	}
	tmp->dwLength=size;
	tmp->line=line;
	tmp->szFile=szFile;
	tmp->pNext=NULL;
	tmp->pPrev=pLastAlloc;
	if (pLastAlloc!=NULL)
		pLastAlloc->pNext=tmp;
	else
		pFirstAlloc=tmp;
	pLastAlloc=tmp;

	for (int i=0;i<EXTRA_ALLOC;i++)
		((BYTE*)tmp)[i+sizeof(ALLOC)+size]=0xab;

	return LPBYTE(tmp)+sizeof(ALLOC);
}

template <DWORD EXTRA_ALLOC>
void CDebugAllocator<EXTRA_ALLOC>::Free(void* pBlock)
{
	ALLOC* alloc=(ALLOC*)(LPBYTE(pBlock)-sizeof(ALLOC));
	if (alloc->pNext!=NULL)
	{
		if (alloc->pPrev!=NULL)
		{
			alloc->pPrev->pNext=alloc->pNext;
			alloc->pNext->pPrev=alloc->pPrev;
		}
		else
		{
			pFirstAlloc=alloc->pNext;	
			alloc->pNext->pPrev=NULL;
		}
	}
	else
	{
		if (alloc->pPrev!=NULL)
		{
			pLastAlloc=alloc->pPrev;
			alloc->pPrev->pNext=NULL;
		}
		else
		{
			pFirstAlloc=NULL;
			pLastAlloc=NULL;
		}
	}
	
	for (int i=0;i<EXTRA_ALLOC;i++)
	{
		if (((BYTE*)alloc)[i+sizeof(ALLOC)+alloc->dwLength]!=0xab)
			break;
	}
	if (i!=EXTRA_ALLOC)
	{
		if (alloc->szFile!=NULL)
			DebugFormatMessage("CDebugAllocator: buffer overrun, %s:%d length=%d",
			alloc->szFile,alloc->line,alloc->dwLength);
		else
			DebugFormatMessage("CDebugAllocator: buffer overrun, length=%d",
			alloc->dwLength);
	}
    	
	free(alloc);
}

template <DWORD EXTRA_ALLOC>
void CDebugAllocator<EXTRA_ALLOC>::DebugInformation(CString& str)
{
	DWORD dwAllocations=0;
	DWORD dwAllocatedSpace=0;

	CString str2,str3;

	ALLOC* pAlloc=pFirstAlloc;
	while (pAlloc!=NULL)
	{
		dwAllocatedSpace+=pAlloc->dwLength;
		dwAllocations++;
		if (pAlloc->szFile!=NULL)
			str3.Format("\nBlock on 0x%X, Length: %d. (allocated @ line %d in %s).",
			DWORD(pAlloc),pAlloc->dwLength,pAlloc->line,pAlloc->szFile);
		else
			str3.Format("\nBlock on 0x%X, Length: %d.",
			DWORD(pAlloc),pAlloc->dwLength);

		str2 << str3;
		pAlloc=pAlloc->pNext;
	}

	str.Format("Allocations: %d, Allocated space: %d",dwAllocations,dwAllocatedSpace);
	str << str2;
}

template <DWORD EXTRA_ALLOC>
CString CDebugAllocator<EXTRA_ALLOC>::GetAllocatorID() const
{
	CString id;
	id.Format("CDebugAllocator on 0x%X",DWORD(this));
	return id;
}

#else
template <DWORD EXTRA_ALLOC>
inline CDebugAllocator<EXTRA_ALLOC>::CDebugAllocator()
{
}
template <DWORD EXTRA_ALLOC>
inline CDebugAllocator<EXTRA_ALLOC>::~CDebugAllocator()
{
}
template <DWORD EXTRA_ALLOC>
inline void CDebugAllocator<EXTRA_ALLOC>::FreeAll()
{
	// Cannot do that
}
template <DWORD EXTRA_ALLOC>
inline BYTE* CDebugAllocator<EXTRA_ALLOC>::Allocate(SIZE_T size)
{
	return (BYTE*)malloc(size+EXTRA_ALLOC);
}
template <DWORD EXTRA_ALLOC>
inline void CDebugAllocator<EXTRA_ALLOC>::Free(void* pBlock)
{
	free(pBlock);
}

#endif


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

