////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
////////////////////////////////////////////////////////////////////


#ifndef CMNCTLEX_INL
#define CMNCTLEX_INL

inline CListCtrlEx::CListCtrlEx()
{
}

inline CListCtrlEx::CListCtrlEx(HWND hWnd)
:	CListCtrl(hWnd)
{
}

inline int CListCtrlEx::GetColumnCount() const
{
	return aColumns.GetSize();
}

inline int CListCtrlEx::GetVisibleColumnCount() const
{
	return aSubItems.GetSize();
}

inline int CListCtrlEx::GetColumnIDFromSubItem(int nSubItem) const
{
	ASSERT(nSubItem<aSubItems.GetSize());
	return aSubItems[nSubItem];
}

inline int CListCtrlEx::GetColumnID(int nCol) const
{
	ASSERT(nCol<aColumns.GetSize());
	return aColumns[nCol]->nID;
}
	
inline int CListCtrlEx::GetVisibleColumn(int nCol) const
{
	ASSERT(nCol<aColumns.GetSize());
	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagVisible)
		return GetVisibleColumnFromSubItem(aColumns[nCol]->lc.iSubItem);
	return -1;
}

inline CListCtrlEx::COLUMNDATA::~COLUMNDATA()
{
	if (!(bFlags&FlagTitleIsResource) && pStrTitle!=NULL)
	{
		delete[] pStrTitle;
	}
}

#endif