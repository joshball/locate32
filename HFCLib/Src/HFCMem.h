////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2004 Janne Huttunen
////////////////////////////////////////////////////////////////////

#ifndef HFCMEMORY_H
#define HFCMEMORY_H

/* Benchmark results:

Copyers:
Buffer/count	1st				2nd				3rd
10/10000000		CopyMemory:70	dMemCopy:150	fMemCopy:160
100/1000000 	CopyMemory:111	iMemCopy:130	MemCopy:160
1000/100000		CopyMemory		iMemCopy		MemCopy
1000000/100		CopyMemory		iMemCopy		MemCopy

Setters (to 0):
Buffer/count	1st				2nd				3rd
10/10000000 	ZeroMemory:30	fMemSet:30  	dMemSet:40
100/1000000 	dMemSet:70		fMemSet:80  	ZeroMemory:81
1000/100000		ZeroMemory		iMemSet			fMemSet
1000000/100		ZeroMemory		fMemSet			fMemSet

Setters (to random):
Buffer/count	1st				2nd				3rd
10/10000000 	dMemSet:70		fMemSet:70  	iMemSet:
100/1000000 	dMemSet:80		fMemSet:81  	iMemSet:110
1000/100000		iMemSet			dMemSet			fMemSet
1000000/100		iMemSet			fMemSet			dMemSet

String length:
Buffer/count	1st				2nd				3rd
10/10000000 	dstrlen:280		fstrlen:290		istrlen:290
100/1000000 	dstrlen:321		istrlen:331		fstrlen:340
1000/100000		dstrlen			fstrlen			istrlen
1000000/100		istrlen			dstrlen			fstrlen
*/


/*
	 XXXXXXXX is normal version
	iXXXXXXXX is inline version
	fXXXXXXXX is for loop version
	dXXXXXXXX is define version
*/
// Memory copiers

void MemCopy(LPVOID dst,LPCVOID src,DWORD len);
#define dMemCopy(dst,src,len) \
{for (register UINT __i_=0;__i_<(len);__i_++) \
((BYTE*)(dst))[__i_]=((BYTE*)(src))[__i_];}

// Memory setters

void MemSet(LPVOID dst,BYTE byte,DWORD count);
#define dMemSet(dst,byte,count) \
{for (register UINT __i_=0;__i_<(count);__i_++) \
((BYTE*)(dst))[__i_]=(BYTE)(byte);}

//////////////////////////////////////
// CLASSES

class CAlloc : public CExceptionObject
{
public:
	CAlloc(DWORD nSize) { m_pData=new BYTE[nSize]; if (m_bThrow && m_pData==NULL) throw CException(CException::cannotAllocate); }
	CAlloc(BYTE* pData) { m_pData=pData; }
	~CAlloc() { if (m_pData!=NULL) delete[] m_pData; }
	
	void Free() { if (m_pData!=NULL) delete[] m_pData; m_pData=NULL; }
	BOOL IsAllocated() const { return m_pData!=NULL; }
	void ReAlloc(DWORD nSize) { Free(); m_pData=new BYTE[nSize];  if (m_bThrow && m_pData==NULL) throw CException(CException::cannotAllocate); }
	
	operator BYTE*() { return m_pData; }
	operator LPCSTR() const { return (LPCSTR)m_pData; }
	operator LPSTR() { return (LPSTR)m_pData; }
	operator INT*() { return (INT*)m_pData; }
	operator UINT*() { return (UINT*)m_pData; }
	operator DWORD*() { return (DWORD*)m_pData; }
	operator LONG*() { return (LONG*)m_pData; }
	operator HANDLE*() { return (HANDLE*)m_pData; }
	operator void*() { return (void*)m_pData; }

	operator FREEDATA() { FREEDATA pRet=(FREEDATA)m_pData; m_pData=NULL; return pRet; }

public:
	BYTE* m_pData;
};

template<class TYPE>
class CAllocTmpl : public CExceptionObject
{
public:
	CAllocTmpl() { m_pData=new TYPE;  }
	CAllocTmpl(BOOL bThrow) { m_pData=new TYPE; m_bThrow=bThrow; if (m_bThrow && m_pData==NULL) throw CException(CException::cannotAllocate); }
	CAllocTmpl(TYPE* pData) { m_pData=pData; }
	~CAllocTmpl() { if (m_pData!=NULL) delete m_pData; }
	
	void Free() { if (m_pData!=NULL) delete m_pData; m_pData=NULL; }
	BOOL IsAllocated() const { return m_pData!=NULL; }
	
	operator void*() { return (void*)m_pData; }
	operator TYPE*() { return m_pData; }
	TYPE* operator ->() {return m_pData; }

	operator FREEDATA() { FREEDATA pRet=(FREEDATA)m_pData; m_pData=NULL; return pRet; }

public:
	TYPE* m_pData;
};

template<class TYPE>
class CAllocArrayTmpl : public CExceptionObject
{
public:
	CAllocArrayTmpl(DWORD dwSize) { m_pData=new TYPE[dwSize];  }
	CAllocArrayTmpl(DWORD dwSize,BOOL bThrow) { m_pData=new TYPE[dwSize]; m_bThrow=bThrow; if (m_bThrow && m_pData==NULL) throw CException(CException::cannotAllocate); }
	CAllocArrayTmpl(TYPE* pData) { m_pData=pData; }
	~CAllocArrayTmpl() { if (m_pData!=NULL) delete[] m_pData; }
	
	void Free() { if (m_pData!=NULL) delete[] m_pData; m_pData=NULL; }
	BOOL IsAllocated() const { return m_pData!=NULL; }
	
	operator void*() { return (void*)m_pData; }
	operator TYPE*() { return m_pData; }

	TYPE* operator ->() {return m_pData; }
	TYPE operator[](int nIndex) const { return m_pData[nIndex]; }
	TYPE& operator[](int nIndex) { return m_pData[nIndex]; }
	TYPE operator[](UINT nIndex) const { return m_pData[nIndex]; }
	TYPE& operator[](UINT nIndex) { return m_pData[nIndex]; }
	TYPE operator[](DWORD nIndex) const { return m_pData[nIndex]; }
	TYPE& operator[](DWORD nIndex) { return m_pData[nIndex]; }
	
	operator FREEDATA() { FREEDATA pRet=(FREEDATA)m_pData; m_pData=NULL; return pRet; }

public:
	TYPE* m_pData;
};

#ifdef WIN32
class CGlobalAlloc : public CExceptionObject
{
public:
	enum allocFlags { 
		fixed=GMEM_FIXED,
		moveable=GMEM_MOVEABLE,
		zeroinit=GMEM_ZEROINIT,
		ptr=GPTR,
		hnd=GHND,
		modify=GMEM_MODIFY ,
		discardable=GMEM_DISCARDABLE,
		noCompact=GMEM_NOCOMPACT,
		noDiscard=GMEM_NODISCARD,
		notBanked=GMEM_NOT_BANKED,
		share=GMEM_SHARE,
		ddeshare=GMEM_DDESHARE,
		notify=GMEM_NOTIFY
	};


	CGlobalAlloc(DWORD nSize,allocFlags nFlags=moveable);
	CGlobalAlloc(HGLOBAL hGlobal,allocFlags nFlags=allocFlags::ptr);
	~CGlobalAlloc();
	
	BOOL Alloc(DWORD nSize,allocFlags nFlags=moveable);
	BOOL ReAlloc(DWORD nSize,allocFlags nFlags=moveable);
	void Free();
	BOOL IsAllocated() const;
	DWORD GetSize() const;
	
	void Lock();
	void Unlock();
	BOOL IsLocked() const;

	allocFlags GetFlags() const;
	BOOL Discard();
	
	operator BYTE*() { Lock(); return m_pData; }
	operator LPCSTR() { Lock(); return (LPCSTR)m_pData; }
	operator LPSTR() { Lock(); return (LPSTR)m_pData; }
	operator INT*() { Lock(); return (INT*)m_pData; }
	operator UINT*() { Lock(); return (UINT*)m_pData; }
	operator DWORD*() { Lock(); return (DWORD*)m_pData; }
	operator LONG*() { Lock(); return (LONG*)m_pData; }
	operator HANDLE*() { Lock(); return (HANDLE*)m_pData; }
	operator void*() { Lock(); return (void*)m_pData; }
	HGLOBAL GetGlobal() const { return m_hGlobal;};

	operator FREEDATA() { FREEDATA pRet=(FREEDATA)m_hGlobal; m_hGlobal=NULL; return pRet; } // use GlobalFree((HGLOBAL)pObject) to release memory

public:
	BYTE* m_pData;
	HGLOBAL m_hGlobal;
	allocFlags m_nFlags;
};

class CHeap : CExceptionObject
{
public:
	enum attributes {
		none=0,
		noSerialize=HEAP_NO_SERIALIZE,
		generateExceptions=HEAP_GENERATE_EXCEPTIONS,
		growable=HEAP_GROWABLE,
		zeroMemory=HEAP_ZERO_MEMORY,
		reallocInPlaceOnly=HEAP_REALLOC_IN_PLACE_ONLY,
		tailCheckingEnabled=HEAP_TAIL_CHECKING_ENABLED,
		freeCheckingEnabled=HEAP_FREE_CHECKING_ENABLED,
		disableCoalesceOnFree=HEAP_DISABLE_COALESCE_ON_FREE,
		createAlign16=HEAP_CREATE_ALIGN_16,
		createEnableTracing=HEAP_CREATE_ENABLE_TRACING 
	};

	class CHeapBlock
	{
	public:
		enum attributes {
			none=0,
			noSerialize=HEAP_NO_SERIALIZE,
			generateExceptions=HEAP_GENERATE_EXCEPTIONS,
			growable=HEAP_GROWABLE,
			zeroMemory=HEAP_ZERO_MEMORY,
			reallocInPlaceOnly=HEAP_REALLOC_IN_PLACE_ONLY,
			tailCheckingEnabled=HEAP_TAIL_CHECKING_ENABLED,
			freeCheckingEnabled=HEAP_FREE_CHECKING_ENABLED,
			disableCoalesceOnFree=HEAP_DISABLE_COALESCE_ON_FREE,
			createAlign16=HEAP_CREATE_ALIGN_16,
			createEnableTracing=HEAP_CREATE_ENABLE_TRACING 
		};
		
		CHeapBlock(void* pBlock);
		CHeapBlock(HANDLE hHeap,attributes nAttributes);
		~CHeapBlock();
		
		DWORD GetSize();
		BOOL Free();
		BOOL Release() { Free(); delete this; }

		operator BYTE*() { return (BYTE*)m_pBlock; }
		operator LPCSTR() { return (LPCSTR)m_pBlock; }
		operator LPSTR() { return (LPSTR)m_pBlock; }
		operator INT*() { return (INT*)m_pBlock; }
		operator UINT*() { return (UINT*)m_pBlock; }
		operator DWORD*() { return (DWORD*)m_pBlock; }
		operator LONG*() { return (LONG*)m_pBlock; }
		operator HANDLE*() { return (HANDLE*)m_pBlock; }
		operator void*() { return (void*)m_pBlock; }

	public:
		void* m_pBlock;
	};


	CHeap();
	CHeap(DWORD dwInitialSize,DWORD dwMaximumSize,attributes nAttributes=attributes::none);
	~CHeap();
	
	BOOL Create(DWORD dwInitialSize,DWORD dwMaximumSize,attributes nAttributes=attributes::none);
	BOOL Destroy();
	BOOL IsAllocated();

	UINT Compact(attributes nAttributes=attributes::none);
	BOOL Lock();
	BOOL Unlock();
	BOOL Validate(LPCVOID lpBlock=NULL,attributes nAttributes=attributes::none);
	BOOL Walk(LPPROCESS_HEAP_ENTRY lpEntry);

	CHeapBlock* Alloc(DWORD nSize,attributes nAttributes=attributes::none);
	
	static CHeap GetProcessHeap();

	operator FREEDATA() { FREEDATA pRet=(FREEDATA)m_hHeap; m_hHeap=NULL; return pRet; } // use ::HeapDestroy() to release memery

private:
	CHeap(HANDLE hHeap) { m_hHeap=hHeap; m_bDestroy=FALSE; }

public:
	BOOL m_bDestroy;
	HANDLE m_hHeap;
};

#endif

// Data keepers

template<class TYPE> class CDataKeeperFP
{
public:
	CDataKeeperFP(TYPE* data) { m_data=data; m_nRef=1; }
	~CDataKeeperFP() { delete m_data; }

	void AddRef() { m_nRef++; }
	CDataKeeperFP* Release()
	{
		m_nRef--;
		if (m_nRef==0)
		{
			delete this; 
			return NULL;
		}
		else
			return this;
	}

	operator TYPE*() const { return m_data; }
	operator TYPE*&() { return m_data; }
	operator TYPE() const { return *m_data; }
	operator TYPE&() { return *m_data; }
	TYPE* operator ->() { return m_data; }

private:
	TYPE* m_data;
	int m_nRef;
};

template<class TYPE> class CDataKeeperFAP
{
public:
	CDataKeeperFAP(TYPE* data) { m_data=data; m_nRef=1; }
	~CDataKeeperFAP() { delete[] m_data; }
	
	void AddRef() { m_nRef++; }
	CDataKeeperFAP* Release()
	{
		if (--m_nRef==0)
		{
			delete this; 
			return NULL;
		}
		else
			return this;
	}

	operator TYPE*() const { return m_data; }
	operator TYPE*&() { return m_data; }
	operator TYPE() const { return *m_data; }
	operator TYPE&() { return *m_data; }
	TYPE* operator ->() { return m_data; }

private:
	TYPE* m_data;
	int m_nRef;
};

class CAllocator // Abstract class for Allocators
{
public:
	CAllocator();
	DEBUGVIRTUAL ~CAllocator();

	DEBUGVIRTUAL void FreeAll();
	
	DEBUGVIRTUAL BYTE* Allocate(DWORD size);
	DEBUGVIRTUAL BYTE* AllocateFast(DWORD size);
	DEBUGVIRTUAL void Free(void* pBlock);

#ifdef _DEBUG
	BYTE* Allocate(DWORD size,int line,char* szFile);
#endif
	
	DEBUGVIRTUAL void DebugInformation(CString& str);
	DEBUGVIRTUAL CString GetAllocatorID() const;
};

template <DWORD EXTRA_ALLOC>
class CDebugAllocator : public CAllocator
{
public:
	CDebugAllocator();
	DEBUGVIRTUAL ~CDebugAllocator();

	DEBUGVIRTUAL void FreeAll();
	
	DEBUGVIRTUAL BYTE* Allocate(DWORD size);
	DEBUGVIRTUAL BYTE* AllocateFast(DWORD size);

#ifdef _DEBUG
	BYTE* Allocate(DWORD size,int line,char* szFile);
#endif
	
	DEBUGVIRTUAL void Free(void* pBlock);
	
#ifdef _DEBUG
	DEBUGVIRTUAL void DebugInformation(CString& str);
	DEBUGVIRTUAL CString GetAllocatorID() const;

	struct ALLOC 
	{
		ALLOC* pNext;
		ALLOC* pPrev;
		DWORD dwLength;
		char* szFile;
		int line;
	};
	ALLOC* pFirstAlloc;
	ALLOC* pLastAlloc;
#endif
};


template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
class CBufferAllocator : public CAllocator
{
public:
	CBufferAllocator();
	DEBUGVIRTUAL ~CBufferAllocator();

	DEBUGVIRTUAL void FreeAll(); // Caution!

public:
	// Allocators
	DEBUGVIRTUAL BYTE* Allocate(DWORD size);
	OUTTYPE Allocate2(DWORD size) { return (OUTTYPE)Allocate(size); }
	DEBUGVIRTUAL OUTTYPE AllocateFast(DWORD size); // Faster allocation, does not check unallocated memory
	
#ifdef _DEBUG
	BYTE* Allocate(DWORD size,int line,char* szFile);
#endif

	// Free
	DEBUGVIRTUAL void Free(void* pBlock);
	
	BOOL IsPointerAllocated(const void* pBlock) const { return GetAlloc(pBlock)!=NULL; } 
	DWORD GetAllocSize(const void* pBlock) const { return ((DWORD*)pBlock)[-1]&~(1<<31); }

	void ReArrange(void** pBlocks[],int nBlocks);

#ifdef _DEBUG
	DEBUGVIRTUAL void DebugInformation(CString& str);
	DEBUGVIRTUAL CString GetAllocatorID() const;
#else
	void DebugInformation(CString& str);
#endif


protected:
	struct ALLOC
	{
	public:
		DWORD dwLength;
		DWORD dwAllocations;
		DWORD dwFreeSpace;
		BYTE* pPtr;

		DWORD GetFree() const { return dwFreeSpace; }

		BYTE* AllocBlock(DWORD size)
		{
			size+=EXTRA_ALLOC;

			if (size+sizeof(DWORD)>int(dwFreeSpace))
				return NULL;

			BYTE* pRet=PBYTE(this)+sizeof(ALLOC);
			
			while (*((DWORD*)pRet)!=0)
			{
				if ((*((DWORD*)pRet)&0x80000000)==0)
				{
					DWORD* pLen=LPDWORD(pRet);
					DWORD* pNextLen=LPDWORD(pRet+*pLen+sizeof(DWORD));

					// Calculating available free size
					while (((*pNextLen)&0x80000000)==0)
					{
						if (*pNextLen==0)
						{
							pPtr=pRet;
							if (MaximumAvailableBlockInEnd()<LONG(size))
								return NULL;
							return AllocBlockFromEnd(size);
						}
						*pLen+=*pNextLen+sizeof(DWORD);
						pNextLen=LPDWORD(pRet+*pLen+sizeof(DWORD));
					}
							
					if (*pLen>=size)
					{
						if (*pLen>size+sizeof(DWORD))
						{
							*((DWORD*)(pRet+size+sizeof(DWORD)))=*pLen-(size+sizeof(DWORD));
							*pLen=size|0x80000000;
						}
						else
							*pLen|=0x80000000;

						dwAllocations++;
						
						// allocated block might be larger than size
						dwFreeSpace-=(*pLen&~0x80000000)+sizeof(DWORD);

						pRet+=sizeof(DWORD);
						if (pPtr<pRet)
						{
							pPtr=pRet+size;
							*((DWORD*)pPtr)=0;
						}
						return pRet;
					}
				}
				pRet+=((*((DWORD*)pRet))&~0x80000000)+sizeof(DWORD);
			}	
			if (MaximumAvailableBlockInEnd()>=LONG(size))
				return AllocBlockFromEnd(size);
			return NULL;
		}

		BYTE* AllocBlockFromEnd(DWORD size)
		{
			size+=EXTRA_ALLOC;

			*((DWORD*)pPtr)=size|(1<<31);
			BYTE* pRet=pPtr+sizeof(DWORD);
			pPtr=pRet+size;
			*((DWORD*)pPtr)=0;
			dwAllocations++;
			dwFreeSpace-=size+sizeof(DWORD);
			return pRet;
		}

		void FreeBlock(void* pBlock)
		{
			if (((DWORD*)pBlock)[-1]&(1<<31)) // Checking whether allocated
			{
				dwFreeSpace+=(((DWORD*)pBlock)[-1]&~(1<<31))+sizeof(DWORD);
				((DWORD*)pBlock)[-1]&=~(1<<31);
				dwAllocations--;
			}
		}

		BOOL IsBlockInAlloc(const void* pBlock) const { return DWORD(pBlock)>DWORD(this) && DWORD(pBlock)<DWORD(this)+dwLength; }



		LONG MaximumAvailableBlockInEnd() const { return dwLength-(int(pPtr)-int(this))-2*sizeof(DWORD); }
		
	public:
		ALLOC* pNext;
	};
	
	ALLOC* GetAlloc(const void* pBlock) const
	{
		if (m_pFirstAlloc==NULL)
			return NULL;

		if (m_pCurrentAlloc->IsBlockInAlloc(pBlock))
			return m_pCurrentAlloc;
		
		ALLOC* pTmp=m_pFirstAlloc;
		while (pTmp!=NULL)
		{
			if (pTmp->IsBlockInAlloc(pBlock))
				return pTmp;
			pTmp=pTmp->pNext;
		}
		return NULL;
	}

	ALLOC* NewAlloc(ALLOC* pNext=NULL)
	{
		ALLOC* pAlloc=(ALLOC*)new BYTE[ALLOC_SIZE];
		pAlloc->dwAllocations=0;
		pAlloc->dwFreeSpace=pAlloc->dwLength=ALLOC_SIZE;
		pAlloc->pPtr=LPBYTE(pAlloc)+sizeof(ALLOC);
		*((DWORD*)pAlloc->pPtr)=0;
		pAlloc->pNext=pNext;	
		DebugNumMessage("CBufferAllocator<OUTTYPE,ALLOC_SIZE>::NewAlloc(ALLOC* pNext): New ALLOC %X created.",DWORD(pAlloc));
		return pAlloc;
	}

	ALLOC* NewAlloc(DWORD size,ALLOC* pNext=NULL)
	{
		ALLOC* pAlloc=(ALLOC*)new BYTE[size+sizeof(ALLOC)+sizeof(DWORD)];
		pAlloc->dwFreeSpace=pAlloc->dwLength=size+sizeof(ALLOC)+2*sizeof(DWORD);
		pAlloc->dwAllocations=0;
		pAlloc->pPtr=LPBYTE(pAlloc)+sizeof(ALLOC);
		*((DWORD*)pAlloc->pPtr)=0;
		pAlloc->pNext=pNext;	
		DebugNumMessage("CBufferAllocator<OUTTYPE,ALLOC_SIZE>::NewAlloc(DWORD size,ALLOC* pNext): New ALLOC %X created.",DWORD(pAlloc));
		return pAlloc;
	}

	ALLOC* m_pFirstAlloc;
	ALLOC* m_pCurrentAlloc;
};


template <class OUTTYPE,DWORD ALLOC_SIZE,DWORD EXTRA_ALLOC> 
class CBufferAllocatorThreadSafe : public CBufferAllocator<OUTTYPE,ALLOC_SIZE,EXTRA_ALLOC>
{
public:
	CBufferAllocatorThreadSafe();
	~CBufferAllocatorThreadSafe();

	DEBUGVIRTUAL BYTE* Allocate(DWORD size);
	OUTTYPE Allocate2(DWORD size) { return (OUTTYPE)Allocate(size); }
	OUTTYPE AllocateFast(DWORD size); // Faster allocation, does not check unallocated memory

	DEBUGVIRTUAL void Free(void* pBlock);

	void ReArrange(void** pBlocks[],int nBlocks);

#ifdef _DEBUG
	DEBUGVIRTUAL void DebugInformation(CString& str);
	DEBUGVIRTUAL CString GetAllocatorID() const;
#else
	void DebugInformation(CString& str);
#endif

protected:
	HANDLE m_hMutex;
};

#include "Memory.inl"

#endif
