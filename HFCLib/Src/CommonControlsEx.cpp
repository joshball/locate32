////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2004 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"


#if defined(DEF_WINDOWS) && defined(DEF_RESOURCES)

CListCtrlEx::COLUMNDATA::COLUMNDATA(int nID_,int nWidth,int nFormat,int nTitleID_,TypeOfResourceHandle bResourceType_)
:	bFlags(FlagTitleIsResource),nID(nID_),
	nTitleID(nTitleID_),bResourceType(bResourceType_)
{
	lc.mask=LVCF_FMT;
	lc.fmt=nFormat;
	if (nWidth>=0)
	{
		lc.cx=nWidth;
		lc.mask|=LVCF_WIDTH;
	}
}

CListCtrlEx::COLUMNDATA::COLUMNDATA(int nID_,int nWidth,int nFormat,LPCSTR pTitle)
:	bFlags(0),nID(nID_)
{
	lc.mask=LVCF_FMT;
	lc.fmt=nFormat;
	if (nWidth>=0)
	{
		lc.cx=nWidth;
		lc.mask|=LVCF_WIDTH;
	}

	DWORD dwTitleLen;
	dstrlen(pTitle,dwTitleLen);
	pStrTitle=new char[max(dwTitleLen+1,2)];
	dMemCopy(pStrTitle,pTitle,dwTitleLen+1);
}

CListCtrlEx::COLUMNDATA::COLUMNDATA(int nID_,const LVCOLUMN* _lc)
:	bFlags(0),nID(nID_)
{
	dMemCopy(&lc,_lc,sizeof(LVCOLUMN));

	if (lc.mask&LVCF_TEXT)
	{
		DWORD dwTitleLen;
		dstrlen(_lc->pszText,dwTitleLen);
		pStrTitle=new char[max(dwTitleLen+1,2)];
		dMemCopy(pStrTitle,_lc->pszText,dwTitleLen+1);
	}
	else
	{
		pStrTitle=new char[2];
		pStrTitle[0]='\0';
	}
}

int CListCtrlEx::InsertColumn(int nID,LPCTSTR lpszColumnHeading,BOOL bShow,int nFormat,int nWidth)
{
	COLUMNDATA* pNew=new COLUMNDATA(nID,nWidth,nFormat,lpszColumnHeading);
    aColumns.Add(pNew);

	if (bShow)
		ShowColumn(aColumns.GetSize()-1);
	return aColumns.GetSize()-1;
}

int CListCtrlEx::InsertColumn(int nID,int nTitleID,BOOL bShow,int nFormat,int nWidth,TypeOfResourceHandle bType)
{
	COLUMNDATA* pNew=new COLUMNDATA(nID,nWidth,nFormat,nTitleID,bType);
	aColumns.Add(pNew);

	if (bShow)
		ShowColumn(aColumns.GetSize()-1);
	return aColumns.GetSize()-1;
}

int CListCtrlEx::InsertColumn(int nID,BOOL bShow,const LVCOLUMN* lc)
{
	COLUMNDATA* pNew=new COLUMNDATA(nID,lc);
	aColumns.Add(pNew);

	if (bShow)
		ShowColumn(aColumns.GetSize()-1);
	return aColumns.GetSize()-1;
}

BOOL CListCtrlEx::DeleteColumn(int nCol)
{
	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagVisible)
	{
		CListCtrl::DeleteColumn(GetVisibleColumnFromSubItem(
			aColumns[nCol]->lc.iSubItem));
	    RemoveSubitem(aColumns[nCol]->lc.iSubItem);
	}
	aColumns.RemoveAt(nCol);
	return TRUE;
}

BOOL CListCtrlEx::ShowColumn(int nCol)
{
	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagVisible)
		return TRUE;

	aColumns[nCol]->lc.mask|=LVCF_SUBITEM|LVCF_TEXT;
	aColumns[nCol]->lc.iSubItem=aSubItems.GetSize();
	aSubItems.Add(aColumns[nCol]->nID);

	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagTitleIsResource)
	{
		CString txt(UINT(aColumns[nCol]->nTitleID),aColumns[nCol]->bResourceType);
		aColumns[nCol]->lc.pszText=txt.GiveBuffer();
	}
	else
		aColumns[nCol]->lc.pszText=aColumns[nCol]->pStrTitle;

	CListCtrl::InsertColumn(aSubItems.GetSize(),&aColumns[nCol]->lc);
	aColumns[nCol]->bFlags|=COLUMNDATA::FlagVisible;

			
	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagTitleIsResource)
		delete[] aColumns[nCol]->lc.pszText;
	return TRUE;
}

BOOL CListCtrlEx::HideColumn(int nCol)
{
	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagVisible)
	{
		aColumns[nCol]->lc.mask&=~LVCF_TEXT;
		int nActualID=GetVisibleColumnFromSubItem(aColumns[nCol]->lc.iSubItem);
		ASSERT(nActualID>=0);
		
		CListCtrl::GetColumn(nActualID,&aColumns[nCol]->lc);
		CListCtrl::DeleteColumn(nActualID);
		ASSERT(aColumns[nCol]->lc.iSubItem<aSubItems.GetSize());
		ASSERT(aSubItems[aColumns[nCol]->lc.iSubItem]==aColumns[nCol]->nID);

		RemoveSubitem(aColumns[nCol]->lc.iSubItem);

				
		aColumns[nCol]->bFlags&=~int(COLUMNDATA::FlagVisible);
	}
	return TRUE;
}


BOOL CListCtrlEx::LoadColumnsState(HKEY hRootKey,LPCSTR lpKey,LPCSTR lpSubKey)
{
	CRegKey RegKey;
	if (lpKey==NULL)
		RegKey.m_hKey=hRootKey;
	else if (RegKey.OpenKey(hRootKey,lpKey,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return FALSE;
	DWORD nDataLength=RegKey.QueryValueLength(lpSubKey);
	if (nDataLength<sizeof(int)*(2+GetColumnCount()) && 
		nDataLength%sizeof(int)!=0)
	{
		if (lpKey==NULL)
			RegKey.m_hKey=NULL;
		return FALSE;
	}
	nDataLength/=sizeof(int);

	int* pData=new int[nDataLength];
	DWORD dwRet=RegKey.QueryValue(lpSubKey,(LPSTR)pData,nDataLength*sizeof(int));
	if (lpKey==NULL)
		RegKey.m_hKey=NULL;
	if (dwRet<nDataLength || pData[0]!=~GetColumnCount() ||
		nDataLength!=~pData[0]+pData[1]+2)
	{
		delete[] pData;
		return FALSE;
	}

    SetColumnOrderArray(pData[1],pData+2+GetColumnCount());
	SetColumnWidthArray(GetColumnCount(),pData+2);

	delete[] pData;
	return TRUE;
}

BOOL CListCtrlEx::SaveColumnsState(HKEY hRootKey,LPCSTR lpKey,LPCSTR lpSubKey) const
{
	CRegKey RegKey;
	if (lpKey==NULL)
		RegKey.m_hKey=hRootKey;
	else if (RegKey.OpenKey(hRootKey,lpKey,CRegKey::createNew|CRegKey::samAll)!=ERROR_SUCCESS)
		return FALSE;
	int* pData=new int[2+GetColumnCount()+GetVisibleColumnCount()];
	pData[0]=~GetColumnCount(); 
	pData[1]=GetVisibleColumnCount();
    GetColumnWidthArray(GetColumnCount(),pData+2);
	GetColumnOrderArray(GetVisibleColumnCount(),pData+2+GetColumnCount());

    RegKey.SetValue(lpSubKey,(LPCSTR)pData,sizeof(int)*(2+GetColumnCount()+GetVisibleColumnCount()),REG_BINARY);
	RegKey.CloseKey();
	delete[] pData;
	if (lpKey==NULL)
		RegKey.m_hKey=NULL;
	return TRUE;
}

HMENU CListCtrlEx::CreateColumnSelectionMenu(int nFirstID) const
{
	HMENU hMenu=CreatePopupMenu();

	MENUITEMINFO mii;
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_STATE|MIIM_ID|MIIM_TYPE;
	mii.fType=MFT_STRING;
	for (int i=0;i<aColumns.GetSize();i++)
	{
		if (aColumns[i]->bFlags&COLUMNDATA::FlagTitleIsResource)
		{
			CString txt(UINT(aColumns[i]->nTitleID),aColumns[i]->bResourceType);
			mii.dwTypeData=txt.GiveBuffer();
		}
		else
			mii.dwTypeData=aColumns[i]->pStrTitle;
		mii.wID=nFirstID+aColumns[i]->nID;
		if (aColumns[i]->bFlags&COLUMNDATA::FlagVisible)
			mii.fState=MFS_CHECKED;
		else
			mii.fState=MFS_ENABLED;
		InsertMenuItem(hMenu,i,TRUE,&mii);
		if (aColumns[i]->bFlags&COLUMNDATA::FlagTitleIsResource)
			delete[] (char*) mii.dwTypeData;
	
		mii.wID++;
	}
	return hMenu;		
}

void CListCtrlEx::ColumnSelectionMenuProc(int nID,int nFirstID)
{
	nID-=nFirstID;

	for (int i=0;i<aColumns.GetSize();i++)
	{
		if (aColumns[i]->nID==nID)
		{
			if (aColumns[i]->bFlags&COLUMNDATA::FlagVisible)
			{
				if (GetVisibleColumnCount()>1) // Do not hide last column
					HideColumn(i);
			}
			else
				ShowColumn(i);
			break;
		}
	}
}


int CListCtrlEx::GetVisibleColumnFromSubItem(int nSubItem) const
{
	LVCOLUMN lc;
	lc.mask=LVCF_SUBITEM;
	
	for (int i=0;CListCtrl::GetColumn(i,&lc);i++)
	{
		if (lc.iSubItem==nSubItem)
		{
			// TODO: Jos tästä ei tule valitusta palauta suoraan nSubItem
			ASSERT(i==nSubItem);
			return i;
		}
	}
	return -1;
}

CString CListCtrlEx::GetColumnTitle(int nCol)
{
	ASSERT(nCol>=0);

	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagTitleIsResource)
		return CString(UINT(aColumns[nCol]->nTitleID),aColumns[nCol]->bResourceType);
	else
		return CString(aColumns[nCol]->pStrTitle);
}


void CListCtrlEx::RemoveSubitem(int nSubItem)
{
	for (int i=0;i<aColumns.GetSize();i++)
	{
		if (aColumns[i]->bFlags&COLUMNDATA::FlagVisible &&
			aColumns[i]->lc.iSubItem>nSubItem)
			aColumns[i]->lc.iSubItem--;
	}

	LVCOLUMN lc;
	lc.mask=LVCF_SUBITEM;
	for (int i=0;CListCtrl::GetColumn(i,&lc);i++)
	{
		if (lc.iSubItem>nSubItem)
		{
			lc.iSubItem--;
			CListCtrl::SetColumn(i,&lc);
		}
	}
	aSubItems.RemoveAt(nSubItem);
}


BOOL CListCtrlEx::SetColumnOrderArray(int iCount,LPINT pi)
{
	// First show/hide correct columns
	for (int i=0;i<aColumns.GetSize();i++)
	{
		for (int j=0;j<iCount;j++)
		{
			if (pi[j]==i)
				break;
		}
		if (j<iCount)
			ShowColumn(i);
		else
			HideColumn(i);
	}
	
	for (int i=0;i<iCount;i++)
		pi[i]=GetVisibleColumn(pi[i]);		

	CListCtrl::SetColumnOrderArray(iCount,pi);
	return TRUE;
}

BOOL CListCtrlEx::GetColumnOrderArray(int iCount,LPINT pi) const
{
	if (iCount<GetVisibleColumnCount())
		return FALSE;
	iCount=GetVisibleColumnCount();
	
	// Obtaining actual IDs
	CListCtrl::GetColumnOrderArray(iCount,pi);

	// Converting actual ID to column index
	for (int i=0;i<iCount;i++)
		pi[i]=GetColumnFromSubItem(pi[i]);
	return TRUE;
}

int CListCtrlEx::GetColumnWidth(int nCol) const
{
	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagVisible)
		return CListCtrl::GetColumnWidth(GetVisibleColumnFromSubItem(aColumns[nCol]->lc.iSubItem));
	else
		return aColumns[nCol]->lc.cx;
}

BOOL CListCtrlEx::SetColumnWidth(int nCol, int cx)
{
	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagVisible)
		return CListCtrl::SetColumnWidth(GetVisibleColumnFromSubItem(aColumns[nCol]->lc.iSubItem),cx);
	else
		aColumns[nCol]->lc.cx=cx;
	return TRUE;
}

BOOL CListCtrlEx::SetColumnWidthArray(int iCount,LPINT pi)
{
	for (int i=min(iCount,aColumns.GetSize())-1;i>=0;i--)
	{
		if (aColumns[i]->bFlags&COLUMNDATA::FlagVisible)
			CListCtrl::SetColumnWidth(GetVisibleColumnFromSubItem(aColumns[i]->lc.iSubItem),pi[i]);
		else
			aColumns[i]->lc.cx=pi[i];
	}
	return TRUE;
}

BOOL CListCtrlEx::GetColumnWidthArray(int iCount,LPINT pi) const
{
	for (int i=min(iCount,aColumns.GetSize())-1;i>=0;i--)
	{
		if (aColumns[i]->bFlags&COLUMNDATA::FlagVisible)
			pi[i]=CListCtrl::GetColumnWidth(GetVisibleColumnFromSubItem(aColumns[i]->lc.iSubItem));
		else
			pi[i]=aColumns[i]->lc.cx;
	}
	return TRUE;
}

int CListCtrlEx::GetColumnFromID(int nID) const
{
	for (int i=0;i<aColumns.GetSize();i++)
	{
		if (aColumns[i]->nID==nID)
			return i;
	}
	return -1;
}

BOOL CListCtrlEx::GetColumnIDArray(int iCount,LPINT pi) const
{
	for (int i=min(aColumns.GetSize(),iCount)-1;i>=0;i--)
		pi[i]=aColumns[i]->nID;
	return TRUE;
}

int CListCtrlEx::GetColumnFromSubItem(int nSubItem) const
{
	for (int i=0;i<aColumns.GetSize();i++)
	{
		if (aColumns[i]->bFlags&COLUMNDATA::FlagVisible &&
			aColumns[i]->lc.iSubItem==nSubItem)
			return i;
	}
	return -1;
}

BOOL CListCtrlEx::GetColumn(int nCol, LV_COLUMN* pColumn) const
{
	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagVisible)
		return CListCtrl::GetColumn(GetVisibleColumnFromSubItem(aColumns[nCol]->lc.iSubItem),pColumn);
	else
	{
		if (pColumn->mask&LVCF_ORDER)
			pColumn->iOrder=-1;
		if (pColumn->mask&LVCF_IMAGE)
			pColumn->iImage=aColumns[nCol]->lc.mask&LVCF_IMAGE?aColumns[nCol]->lc.iImage:0;
		if (pColumn->mask&LVCF_WIDTH)
			pColumn->cx=aColumns[nCol]->lc.mask&LVCF_WIDTH?aColumns[nCol]->lc.cx:100;
		if (pColumn->mask&LVCF_SUBITEM)
			pColumn->iSubItem=-1;
		if (pColumn->mask&LVCF_FMT)
			pColumn->fmt=aColumns[nCol]->lc.mask&LVCF_FMT?aColumns[nCol]->lc.fmt:LVCFMT_LEFT;
		if (pColumn->mask&LVCF_TEXT)
		{
			if (aColumns[nCol]->bFlags&COLUMNDATA::FlagTitleIsResource)
			{
				::LoadString(GetResourceHandle(aColumns[nCol]->bResourceType),
					aColumns[nCol]->nTitleID,pColumn->pszText,pColumn->cchTextMax);
			}
			else
			{
				int nLength=max(strlen(aColumns[nCol]->pStrTitle),size_t(pColumn->cchTextMax-1));
				strncmp(pColumn->pszText,aColumns[nCol]->pStrTitle,nLength);
				pColumn->pszText[nLength]='\0';
			}
		}
		return TRUE;
	}
}

BOOL CListCtrlEx::SetColumn(int nCol, const LV_COLUMN* pColumn)
{
	if (aColumns[nCol]->bFlags&COLUMNDATA::FlagVisible)
		return CListCtrl::SetColumn(GetVisibleColumnFromSubItem(aColumns[nCol]->lc.iSubItem),pColumn);
	else
	{
		if (pColumn->mask&LVCF_IMAGE)
		{
			aColumns[nCol]->lc.mask|=LVCF_IMAGE;
			aColumns[nCol]->lc.iImage=pColumn->iImage;
		}
		if (pColumn->mask&LVCF_WIDTH)
		{
			aColumns[nCol]->lc.mask|=LVCF_WIDTH;
			aColumns[nCol]->lc.cx=pColumn->cx;
		}
		if (pColumn->mask&LVCF_FMT)
		{
			aColumns[nCol]->lc.mask|=LVCF_FMT;
			aColumns[nCol]->lc.fmt=pColumn->fmt;
		}
		if (pColumn->mask&LVCF_TEXT)
		{
			if (!(aColumns[nCol]->bFlags&COLUMNDATA::FlagTitleIsResource) &&
				aColumns[nCol]->pStrTitle!=NULL)
				delete[] aColumns[nCol]->pStrTitle;

			aColumns[nCol]->bFlags&=~DWORD(COLUMNDATA::FlagTitleIsResource);
            
			aColumns[nCol]->pStrTitle=new char[max(strlen(pColumn->pszText+1)+1,2)];
			strcpy(aColumns[nCol]->pStrTitle,pColumn->pszText);
		}
		return TRUE;
	}
}


#endif