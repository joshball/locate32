////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////

#ifndef HFCMEMORY_INL
#define HFCMEMORY_INL

inline void iMemCopy(LPVOID dst,LPCVOID src,UINT len) // Exactly same as MemCopy()
{
#ifdef WIN32
	__asm
	{
		push edi
		push esi
		mov edi,[dst]
		mov esi,[src]
		mov ebx,[len]
		mov ecx,ebx
		shr ecx,2
		cld
		rep movsd
		mov ecx,ebx
		and ecx,3
		rep movsb
		pop esi
		pop edi
	}
#else
	__asm__("
		movl %%ebx,%%ecx
		shrl $2,%%ecx
		cld
		rep 
		movsl
		movl %%ebx,%%ecx
		andl $3,%%ecx
		rep 
		movsb
		"
		:
		:"S" (src),"D" (dst),"b" (len)
		:"ecx","memory");
#endif
}

inline void fMemCopy(void* dst,const void* src,SIZE_T len) // faster when src is very short
{
	for (register SIZE_T i=0;i<len;i++)
		((BYTE*)dst)[i]=((BYTE*)src)[i];
}

inline void iMemSet(LPVOID pDst,BYTE nByte,UINT nCount) // exactly same as MemSet()
{
#ifdef WIN32
	__asm
	{
		push edi
		mov edi,[pDst]
		mov bl,[nByte]
		mov al,bl
		mov ah,bl
		shl eax,16
		mov al,bl
		mov ah,bl
		mov ebx,[nCount]
		mov ecx,ebx
		shr ecx,2
		cld
		rep stosd
		mov ecx,ebx
		and ecx,3
		rep stosb
		pop edi
	}
#else
	__asm__("
		movl %%ebx,%%ecx
		shrl $2,%%ecx
		movb %%dl,%%al
		movb %%dl,%%ah
		shrl $16,%%eax
		movb %%dl,%%al
		movb %%dl,%%ah
		cld
		rep 
		stosl
		movl %%ebx,%%ecx
		andl $3,%%ecx
		rep 
		stosb
		"
		:
                :"D" (pDst),"d" (nByte),"b" (nCount)
		:"ax","cx","memory");
#endif
}

inline void fMemSet(void* dst,BYTE byte,SIZE_T count) // faster when count is small
{
	for (register SIZE_T i=0;i<count;i++)
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

inline BYTE* CAllocator::Allocate(DWORD size)
{
	return new BYTE[size];
}

inline BYTE* CAllocator::AllocateFast(DWORD size)
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
BYTE* CDebugAllocator<EXTRA_ALLOC>::Allocate(DWORD size)
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
NDEBUGINLINE BYTE* CDebugAllocator<EXTRA_ALLOC>::AllocateFast(DWORD size)
{
	return Allocate(size);
}

template <DWORD EXTRA_ALLOC>
BYTE* CDebugAllocator<EXTRA_ALLOC>::Allocate(DWORD size,int line,char* szFile)
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
inline BYTE* CDebugAllocator<EXTRA_ALLOC>::Allocate(DWORD size)
{
	return (BYTE*)malloc(size+EXTRA_ALLOC);
}
template <DWORD EXTRA_ALLOC>
inline void CDebugAllocator<EXTRA_ALLOC>::Free(void* pBlock)
{
	free(pBlock);
}

#endif

/////////////////////////////////////////////////
// CBufferAllocator


template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
inline CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::CBufferAllocator()
:	m_pFirstAlloc(NULL)
{
}

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
NDEBUGINLINE CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::~CBufferAllocator()
{ 
	FreeAll(); 
}


template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
void CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::FreeAll()
{
	while (m_pFirstAlloc!=NULL)
	{
		m_pCurrentAlloc=m_pFirstAlloc;
		m_pFirstAlloc=m_pFirstAlloc->pNext;
		DebugNumMessage("CBufferAllocator<OUTTYPE,ALLOC_SIZE>::FreeAll(): ALLOC %X will be deleted.",DWORD(m_pCurrentAlloc));
		delete[] (BYTE*)m_pCurrentAlloc;
	}
	m_pCurrentAlloc=NULL;
}
	
#ifdef _DEBUG
template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
inline BYTE* CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::Allocate(DWORD size,int line,char* szFile)
{
	Allocate(size);
}
#endif

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
BYTE* CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::Allocate(DWORD size)
{
	size+=EXTRA_ALLOC;

	if (size>3*ALLOC_SIZE/2)
	{
		m_pCurrentAlloc=m_pFirstAlloc=NewAlloc(size,m_pFirstAlloc);
		return (OUTTYPE)m_pCurrentAlloc->AllocBlockFromEnd(size);
	}

	if (m_pFirstAlloc==NULL)
	{
		// This allocation is first
		m_pCurrentAlloc=m_pFirstAlloc=NewAlloc();
		return (OUTTYPE)m_pCurrentAlloc->AllocBlockFromEnd(size);
	}

	BYTE* pRet=m_pCurrentAlloc->AllocBlock(size);
	if (pRet!=NULL)
		return pRet;

	if (m_pCurrentAlloc!=m_pFirstAlloc)
	{
		ALLOC* pAlloc=m_pFirstAlloc;
		while (pAlloc->pNext!=m_pCurrentAlloc)
		{
			pRet=pAlloc->AllocBlock(size);
			if (pRet!=NULL)
			{
				m_pCurrentAlloc=pAlloc;
				return (OUTTYPE)pRet;
			}
			pAlloc=pAlloc->pNext;
		}	
	}

	while (m_pCurrentAlloc->pNext!=NULL)
	{
		m_pCurrentAlloc=m_pCurrentAlloc->pNext;
		pRet=m_pCurrentAlloc->AllocBlock(size);
		if (pRet!=NULL)
			return (OUTTYPE)pRet;
	}	

	m_pCurrentAlloc=m_pCurrentAlloc->pNext=NewAlloc();
	return (OUTTYPE)m_pCurrentAlloc->AllocBlockFromEnd(size);
}

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
OUTTYPE CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::AllocateFast(DWORD size)
{
	size+=EXTRA_ALLOC;

	if (size>3*ALLOC_SIZE/2)
	{
		m_pCurrentAlloc=m_pFirstAlloc=NewAlloc(size,m_pFirstAlloc);
		return (OUTTYPE)m_pCurrentAlloc->AllocBlockFromEnd(size);
	}
		
	if (m_pFirstAlloc==NULL)
	{
		// This allocation is first
		m_pCurrentAlloc=m_pFirstAlloc=NewAlloc();
		return (OUTTYPE)m_pCurrentAlloc->AllocBlockFromEnd(size);
	}
	if (m_pCurrentAlloc->MaximumAvailableBlockInEnd()<LONG(size))
	{
		// Not enough memory, allocating more
		m_pCurrentAlloc=m_pCurrentAlloc->pNext=NewAlloc(m_pCurrentAlloc->pNext);
		//return m_pCurrentAlloc->AllocBlockFromEnd(size);
	}
	return (OUTTYPE)m_pCurrentAlloc->AllocBlockFromEnd(size);
}

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
void CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::Free(void* pBlock)
{
	if (m_pFirstAlloc==NULL)
	{
		DebugMessage("CBufferAllocator<OUTTYPE,ALLOC_SIZE>::Free(): Trying to free when m_pFirstAlloc==NULL");
		return;
	}
	
	if (m_pCurrentAlloc->IsBlockInAlloc(pBlock))
	{
		m_pCurrentAlloc->FreeBlock(pBlock);
		if (m_pCurrentAlloc->dwAllocations>0)
			return; // Do not remove ALLOC

		if (m_pFirstAlloc!=m_pCurrentAlloc)
		{
			
			ALLOC* pPrevAlloc=m_pFirstAlloc;
			while (pPrevAlloc->pNext!=m_pCurrentAlloc)
				pPrevAlloc=pPrevAlloc->pNext;

			pPrevAlloc->pNext=m_pCurrentAlloc->pNext;
			
			DebugNumMessage("CBufferAllocator<OUTTYPE,ALLOC_SIZE>::Free(1): ALLOC %X will be deleted.",DWORD(m_pCurrentAlloc));
			delete[] (BYTE*)m_pCurrentAlloc;
			m_pCurrentAlloc=pPrevAlloc;
			
			return;
		}
	}
	else if (m_pFirstAlloc->IsBlockInAlloc(pBlock))
	{
		m_pFirstAlloc->FreeBlock(pBlock);
		if (m_pFirstAlloc->dwAllocations>0)
			return; // Do not remove ALLOC
		m_pCurrentAlloc=m_pFirstAlloc->pNext;
		DebugNumMessage("CBufferAllocator<OUTTYPE,ALLOC_SIZE>::Free(2): ALLOC %X will be deleted.",DWORD(m_pFirstAlloc));
		delete[] (BYTE*) m_pFirstAlloc;
		m_pFirstAlloc=m_pCurrentAlloc;
		return;
	}
	else
	{
		m_pCurrentAlloc=m_pFirstAlloc;
		ALLOC* pTmp;
		while ((pTmp=m_pCurrentAlloc->pNext)!=NULL)
		{
			if (pTmp->IsBlockInAlloc(pBlock))
			{
				pTmp->FreeBlock(pBlock);
				if (pTmp->dwAllocations==0)
				{
					m_pCurrentAlloc->pNext=pTmp->pNext;
					DebugNumMessage("CBufferAllocator<OUTTYPE,ALLOC_SIZE>::Free(3): ALLOC %X will be deleted.",DWORD(pTmp));
					delete[] (BYTE*) pTmp;
				}
				return;
			}
			m_pCurrentAlloc=pTmp;
		}
		DebugNumMessage("CBufferAllocator<OUTTYPE,ALLOC_SIZE>::Free(): pBlock %X does not found.",DWORD(pBlock));
		return;
	}

	// Removing first ALLOC, m_pCurrentAlloc==m_pFirstAlloc
	m_pFirstAlloc=m_pFirstAlloc->pNext;
	DebugNumMessage("CBufferAllocator<OUTTYPE,ALLOC_SIZE>::Free(4): ALLOC %X will be deleted.",DWORD(m_pCurrentAlloc));
	delete[] (BYTE*) m_pCurrentAlloc;
	m_pCurrentAlloc=m_pFirstAlloc;
}


#ifdef _DEBUG
template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
void CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::DebugInformation(CString& str)
{
	CString str2;
	
	ALLOC* pAlloc=m_pFirstAlloc;
	DWORD dwBlocks=0;
	DWORD dwAllocations=0;
	DWORD dwAllocatedSpace=0;
	DWORD dwUnallocated=0;
	while (pAlloc!=NULL)
	{
		dwAllocations+=pAlloc->dwAllocations;
		dwAllocatedSpace+=pAlloc->dwLength-pAlloc->dwFreeSpace;
		dwUnallocated+=pAlloc->dwFreeSpace;
		dwBlocks++;
		pAlloc=pAlloc->pNext;
	}
	str.Format("Blocks: %d,Allocations: %d, AllocatedSpace: %d, Unallocated: %d",
		dwBlocks,dwAllocations,dwAllocatedSpace,dwUnallocated);

	pAlloc=m_pFirstAlloc;
	int i=0;
	while (pAlloc!=NULL)
	{

		BYTE* pRet=LPBYTE(pAlloc)+sizeof(CBufferAllocator::ALLOC);
		DWORD* pLen;
		DWORD dwAllocation=0;
		DWORD dwUsed=0;
		while (*(pLen=LPDWORD(pRet))!=0)
		{
			if ((*pLen)&0x80000000)
			{
				dwAllocation++;
				dwUsed+=((*pLen)&~0x80000000)+sizeof(DWORD);
			}			
			pRet=LPBYTE(pLen)+((*pLen)&~0x80000000)+sizeof(DWORD);
		}		
		str2.Format("\nBlock %d on 0x%x Len %d, Allocs %d. Free: %d, Used: %d, pPtr=%X, Availlen end: %d",
			i,DWORD(pAlloc),pAlloc->dwLength,pAlloc->dwAllocations,pAlloc->dwFreeSpace,
			dwUsed,DWORD(pAlloc->pPtr),pAlloc->MaximumAvailableBlockInEnd());
		str << str2;
		
		if (LONG(pLen)>=LONG(pAlloc+pAlloc->dwLength))
		{
			str2.Format("\nBlock %X is damaged: allocated over dwLength",DWORD(pAlloc));
			str << str2;
		}
		if (dwUsed+pAlloc->dwFreeSpace!=pAlloc->dwLength)
		{
			str2.Format("\nBlock %X is damaged: used+freespace!=length",DWORD(pAlloc));
			str << str2;
		}
		if (pRet!=pAlloc->pPtr)
		{
			str2.Format("\nBlock %X is damaged: pPtr should be %X instead of %X",DWORD(pAlloc),DWORD(pRet),DWORD(pAlloc->pPtr));
			str << str2;
		}
		if (dwAllocation!=pAlloc->dwAllocations)
		{
			str2.Format("\nBlock %X is damaged: dwAllocation!=pAlloc->dwAllocations",DWORD(pAlloc));
			str << str2;
		}
		
		i++;
		pAlloc=pAlloc->pNext;
	}
}



template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
CString CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::GetAllocatorID() const
{
	CString id;
	id.Format("CBufferAllocator<OUTTYPE,%d> on 0x%X",ALLOC_SIZE,DWORD(this));
	return id;
}

#endif

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
void CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::ReArrange(void** pBlocks[],int nBlocks)
{
	for (int i=0;i<nBlocks;i++)
	{
		BYTE*& pBlock=*((BYTE**)pBlocks[i]);
		DWORD dwLength=GetAllocSize(pBlock);
		
		ALLOC* pAlloc=m_pFirstAlloc;
		while (pAlloc!=NULL && !pAlloc->IsBlockInAlloc(pBlock))
		{
			BYTE* pNewBlock=pAlloc->AllocBlock(dwLength);
			if (pNewBlock!=NULL)
			{
				dMemCopy(pNewBlock,pBlock,dwLength);
				Free(pBlock);
				pBlock=pNewBlock;
				break;
			}
			pAlloc=pAlloc->pNext;
		}
	}
}

////////////////////////////////////////////
// CBufferAllocatorThreadSafe

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
inline CBufferAllocatorThreadSafe<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::CBufferAllocatorThreadSafe()
:	CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>()
{
	m_hMutex=CreateMutex(NULL,FALSE,NULL);
}

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
inline CBufferAllocatorThreadSafe<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::~CBufferAllocatorThreadSafe()
{
	if (m_hMutex!=NULL)
	{
		CloseHandle(m_hMutex);
		m_hMutex=NULL;
	}
}

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
NDEBUGINLINE BYTE* CBufferAllocatorThreadSafe<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::Allocate(DWORD size)
{
	if (WaitForMutex(m_hMutex))
		return NULL;

	BYTE* pRet=CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::Allocate(size);
	ReleaseMutex(m_hMutex);
	return pRet;
}

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
inline OUTTYPE CBufferAllocatorThreadSafe<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::AllocateFast(DWORD size)
{
	if (WaitForMutex(m_hMutex))
		return NULL;
	
	BYTE* pRet=CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::AllocateFast(size);
	
	ReleaseMutex(m_hMutex);
	return pRet;
}

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> NDEBUGINLINE 
void CBufferAllocatorThreadSafe<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::Free(void* pBlock)
{
	if (WaitForMutex(m_hMutex))
		return;
	
	CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::Free(pBlock);
	
	ReleaseMutex(m_hMutex);
}

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC>
void CBufferAllocatorThreadSafe<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::ReArrange(void** pBlocks[],int nBlocks)
{
	if (WaitForMutex(m_hMutex))
		return;
	
	for (int i=0;i<nBlocks;i++)
	{
		BYTE*& pBlock=*((BYTE**)pBlocks[i]);
		DWORD dwLength=GetAllocSize(pBlock);
		
		ALLOC* pAlloc=m_pFirstAlloc;
		while (pAlloc!=NULL && !pAlloc->IsBlockInAlloc(pBlock))
		{
			BYTE* pNewBlock=pAlloc->AllocBlock(dwLength);
			if (pNewBlock!=NULL)
			{
				dMemCopy(pNewBlock,pBlock,dwLength);
				CBufferAllocator<OUTTYPE,ALLOC_SIZE>::Free(pBlock);
				pBlock=pNewBlock;
				break;
			}
			pAlloc=pAlloc->pNext;
		}
	}
	ReleaseMutex(m_hMutex);
}

#ifdef _DEBUG
template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
void CBufferAllocatorThreadSafe<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::DebugInformation(CString& str)
{
	if (WaitForMutex(m_hMutex))
		return;
	CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::DebugInformation(str);
	ReleaseMutex(m_hMutex);
}

template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
CString CBufferAllocatorThreadSafe<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>::GetAllocatorID() const
{
	CString id;
	id.Format("CBufferAllocatorThreadSafe<OUTTYPE,%d> on 0x%X",ALLOC_SIZE,DWORD(this));
	return id;
}
#endif
////////////////////////////////////////////
// CGlobalAlloc

#ifdef WIN32

inline CGlobalAlloc::CGlobalAlloc(DWORD nSize,allocFlags nType)
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

inline CHeap::CHeap(DWORD dwInitialSize,DWORD dwMaximumSize,attributes nAttributes)
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

inline CHeap::CHeapBlock* CHeap::Alloc(DWORD nSize,attributes nAttributes)
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

