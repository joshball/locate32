////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2007 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#if defined(HFC_USEDEBUGNEW)
	#define new DEBUG_NEW
#endif


#ifdef WIN32
////////////////////////////////////////////
// CGlobalAlloc
////////////////////////////////////////////


BOOL CGlobalAlloc::Alloc(SIZE_T nSize,allocFlags nFlags)
{
	if (m_hGlobal!=NULL)
		Free();
	m_hGlobal=GlobalAlloc(nFlags,nSize);
	DebugOpenHandle(dhtMemoryBlock,m_hGlobal,STRNULL);
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

BOOL CGlobalAlloc::ReAlloc(SIZE_T nSize,allocFlags nFlags)
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
	DebugCloseHandle(dhtMemoryBlock,m_hGlobal,STRNULL);
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

BOOL CHeap::Create(SIZE_T dwInitialSize,SIZE_T dwMaximumSize,attributes nAttributes)
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

