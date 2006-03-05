////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2006 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

void MemCopy(LPVOID dst,LPCVOID src,UINT len)
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

void MemSet(LPVOID pDst,BYTE nByte,UINT nCount)
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

#ifdef WIN32
////////////////////////////////////////////
// CGlobalAlloc
////////////////////////////////////////////


BOOL CGlobalAlloc::Alloc(DWORD nSize,allocFlags nFlags)
{
	if (m_hGlobal!=NULL)
		Free();
	m_hGlobal=GlobalAlloc(nFlags,nSize);
	if (m_hGlobal==NULL)
	{
		if (m_bThrow)
			throw CException(CException::cannotAllocate);
		return FALSE;
	}
	m_nFlags=nFlags;
	if ((m_nFlags&moveable)==0)
		m_pData=(BYTE*)m_hGlobal;
	else
		m_pData=NULL;
	return TRUE;
}

BOOL CGlobalAlloc::ReAlloc(DWORD nSize,allocFlags nFlags)
{
	if (nFlags&moveable && m_pData!=NULL)
		GlobalUnlock(m_hGlobal);
	HGLOBAL hGlobal=GlobalReAlloc(m_hGlobal,nSize,nFlags);
	if (hGlobal==NULL)
	{
		if (m_bThrow)
			throw CException(CException::cannotAllocate);
		return FALSE;
	}
	m_hGlobal=hGlobal;
	m_nFlags=nFlags;
	if (m_nFlags&moveable)
		m_pData=NULL;
	else
		m_pData=(BYTE*)hGlobal;
	return TRUE;
}

void CGlobalAlloc::Free()
{
	if ((m_nFlags&moveable)==0 && m_pData!=NULL)
		GlobalUnlock(m_hGlobal);
	GlobalFree(m_hGlobal);
	m_pData=NULL;
	m_hGlobal=NULL;
}

BOOL CGlobalAlloc::Discard()
{
	Unlock();
	HGLOBAL hGlobal=GlobalDiscard(m_hGlobal);
	if (hGlobal==NULL)
	{
		if (m_bThrow)
			throw CException(CException::cannotDiscard);
		return FALSE;
	}
	m_hGlobal=hGlobal;
	return TRUE;
}

////////////////////////////////////////////
// CHeap
////////////////////////////////////////////

BOOL CHeap::Create(DWORD dwInitialSize,DWORD dwMaximumSize,attributes nAttributes)
{
	m_hHeap=HeapCreate(nAttributes|m_bThrow?HEAP_GENERATE_EXCEPTIONS:0,dwInitialSize,dwMaximumSize);
	if (m_hHeap==NULL)
	{
		if (m_bThrow)
			throw CException(CException::cannotCreateHeap);
		return FALSE;
	}
	m_bDestroy=TRUE;
	return TRUE;
}

#endif //WIN32

	
////////////////////////////////////////////
// CAllocators
////////////////////////////////////////////

#ifdef _DEBUG
CArray<CAllocator*>* paAllocators=NULL;

CAllocator::CAllocator()
{
	if (paAllocators==NULL)
		paAllocators=new CArray<CAllocator*>;

	paAllocators->Add(this);
}

CAllocator::~CAllocator()
{
	if (paAllocators!=NULL)
	{
		int nIndex=paAllocators->Find(this);
		if (nIndex!=-1)
			paAllocators->RemoveAt(nIndex);
		if (paAllocators->GetSize()==0)
		{
			delete paAllocators;
			paAllocators=NULL;
		}
	}
}


void CAllocator::FreeAll()
{
}

BYTE* CAllocator::Allocate(DWORD size)
{
	return new BYTE[size];
}

BYTE* CAllocator::AllocateFast(DWORD size)
{
	return new BYTE[size];
}

BYTE* CAllocator::Allocate(DWORD size,int line,char* szFile)
{
	return new BYTE[size];
}

void CAllocator::Free(void* pBlock)
{
	delete [] (BYTE*) pBlock;
}

void CAllocator::DebugInformation(CString& str)
{
}

inline CString CAllocator::GetAllocatorID() const
{
	CString id;
	id.Format("CAllocator on 0x%X",DWORD(this));
	return id;
}

void AddAllocatorsIDtoCombo(HWND hCombo)
{
	if (paAllocators==NULL)
		return;
	for (int i=0;i<paAllocators->GetSize();i++)
		SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)(LPCSTR)paAllocators->GetAt(i)->GetAllocatorID());
}

void GetAllocatorString(int iIndex,CString& str)
{
	if (paAllocators==NULL)
	{
		str="No allocators available";
		return;
	}
	else if (iIndex==-1 || iIndex>=paAllocators->GetSize())
	{
		str="No allocator available";
		return;
	}

	CAllocator* pAllocator=paAllocators->GetAt(iIndex);
	if (pAllocator==NULL)
	{
		str="No allocator available";
		return;
	}
	pAllocator->DebugInformation(str);
}
	
void CheckAllocators()
{
	if (paAllocators==NULL)
		return;
	if (paAllocators->GetSize()==0)
		return;

	CString msg("Allocator check begins"),str;
	for (int i=0;i<paAllocators->GetSize();i++)
	{
		paAllocators->GetAt(i)->DebugInformation(str);
		msg << '\n' << paAllocators->GetAt(i)->GetAllocatorID();
		if (!str.IsEmpty())
			msg << ":\n" << str;
	}
	DebugMessage(msg);
	DebugMessage("Allocator check ended");
}




#endif