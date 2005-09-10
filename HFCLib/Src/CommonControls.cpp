////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#ifdef DEF_WINDOWS

///////////////////////////
// Class CImageList
///////////////////////////

#ifdef DEF_RESOURCES
BOOL CImageList::DeleteImageList()
{
	if (m_hImageList==NULL)
		return TRUE;
	if (!ImageList_Destroy(m_hImageList))
		return FALSE;
	m_hImageList=NULL;
	return TRUE;
}

#endif

///////////////////////////
// Class CStatusBarCtrl
///////////////////////////

CString CStatusBarCtrl::GetText(int nPane,int* pType) const
{
	CString str;
	int ret=::SendMessage(m_hWnd,SB_GETTEXT,(WPARAM)nPane,(LPARAM)str.GetBuffer(LOWORD(::SendMessage(m_hWnd,SB_GETTEXTLENGTH,(WPARAM)nPane,0))));
	if (pType!=NULL)
		*pType=HIWORD(ret);
	return str;
}

int CStatusBarCtrl::GetText(LPTSTR lpszText,int nPane,int* pType) const
{
	int ret=::SendMessage(m_hWnd,SB_GETTEXT,(WPARAM)nPane,(LPARAM)lpszText);	
	if (pType!=NULL)
		*pType=HIWORD(ret);
	return LOWORD(ret);
}

int CStatusBarCtrl::GetTextLength(int nPane,int* pType) const
{
	int ret=::SendMessage(m_hWnd,SB_GETTEXTLENGTH,(WPARAM)nPane,0);
	if (pType!=NULL)
		*pType=HIWORD(ret);
	return LOWORD(ret);
}

///////////////////////////
// Class CToolTipCtrl
///////////////////////////

void CToolTipCtrl::GetText(CString& str,HWND hWnd,UINT nIDTool) const
{
	CToolInfo ti;
	ti.cbSize=sizeof (TOOLINFO);
	ti.uFlags=0;
	ti.hwnd=hWnd;
	ti.uId=nIDTool;
	ti.lpszText=ti.szText;
	::SendMessage(m_hWnd,TTM_GETTEXT,0,(LPARAM)&ti);
	str=ti.szText;
}

BOOL CToolTipCtrl::GetToolInfo(LPTOOLINFO pToolInfo,HWND hWnd,UINT nIDTool) const
{
	pToolInfo->cbSize=sizeof(TOOLINFO);
	pToolInfo->hwnd=hWnd;
	pToolInfo->uId=nIDTool;
	return ::SendMessage(m_hWnd,TTM_GETTOOLINFO,0,(LPARAM)pToolInfo);
}

void CToolTipCtrl::SetToolRect(HWND hWnd,UINT nIDTool,LPCRECT lpRect)
{
	CToolInfo ti;
	ti.cbSize=sizeof (TOOLINFO);
	ti.uFlags=0;
	ti.hwnd=hWnd;
	ti.uId=nIDTool;
	ti.rect=*lpRect;
	::SendMessage(m_hWnd,TTM_NEWTOOLRECT,0,(LPARAM)&ti);
}

#ifdef DEF_RESOURCES
BOOL CToolTipCtrl::AddTool(HWND hWnd,UINT nIDText,LPCRECT lpRectTool,UINT nIDTool,LPARAM lParam)
{
	TOOLINFO ti;
	ti.cbSize=sizeof (TOOLINFO);
	ti.uFlags=TTF_SUBCLASS;
	ti.hwnd=hWnd;
	ti.uId=nIDTool;
	ti.hinst=GetLanguageSpecificResourceHandle();
	ti.lpszText=MAKEINTRESOURCE(nIDText);
	ti.lParam=lParam;	
	if (lpRectTool!=NULL)
		ti.rect=*lpRectTool;
	else if (hWnd!=NULL)
		::GetClientRect(hWnd,&ti.rect);
	else
	{
		ti.rect.left=0;
		ti.rect.right=0;
		ti.rect.top=0;
		ti.rect.bottom=0;
	}
	return ::SendMessage(m_hWnd,TTM_ADDTOOL,0,(LPARAM)&ti);
}
#endif

BOOL CToolTipCtrl::AddTool(HWND hWnd,LPCTSTR lpszText,LPCRECT lpRectTool,UINT nIDTool,LPARAM lParam)
{
	TOOLINFO ti;
	ti.cbSize=sizeof (TOOLINFO);
	ti.uFlags=TTF_SUBCLASS;
	ti.hwnd=hWnd;
	ti.uId=nIDTool;
	ti.lpszText=(LPSTR)lpszText;
	ti.lParam=lParam;	
	if (lpRectTool!=NULL)
		ti.rect=*lpRectTool;
	else if (hWnd!=NULL)
		::GetClientRect(hWnd,&ti.rect);
	else
	{
		ti.rect.left=0;
		ti.rect.right=0;
		ti.rect.top=0;
		ti.rect.bottom=0;
	}
	return ::SendMessage(m_hWnd,TTM_ADDTOOL,0,(LPARAM)&ti);
}

void CToolTipCtrl::DelTool(HWND hWnd,UINT nIDTool)
{
	CToolInfo ti;
	ti.cbSize=sizeof (TOOLINFO);
	ti.uFlags=0;
	ti.hwnd=hWnd;
	ti.uId=nIDTool;
	::SendMessage(m_hWnd,TTM_DELTOOL,0,(LPARAM)&ti);
}

void CToolTipCtrl::DelTool(HWND hWnd,HWND hToolWnd)
{
	CToolInfo ti;
	ti.cbSize=sizeof (TOOLINFO);
	ti.uFlags=TTF_IDISHWND;
	ti.hwnd=hWnd;
	ti.uId=(UINT)hToolWnd;
	::SendMessage(m_hWnd,TTM_DELTOOL,0,(LPARAM)&ti);
}


BOOL CToolTipCtrl::HitTest(HWND hWnd,LPPOINT pt,LPTOOLINFO lpToolInfo) const
{
	TTHITTESTINFO hi;
	hi.hwnd=hWnd;
	hi.pt=*pt;
	hi.ti.cbSize=sizeof(TOOLINFO);
	BOOL nRet=::SendMessage(m_hWnd,TTM_HITTEST,0,(LPARAM)&hi);
	*lpToolInfo=hi.ti;
	return nRet;
}

void CToolTipCtrl::UpdateTipText(LPCTSTR lpszText,HWND hWnd,UINT nIDTool)
{
	CToolInfo ti;
	ti.cbSize=sizeof (TOOLINFO);
	ti.uFlags=TTF_SUBCLASS;
	ti.hwnd=hWnd;
	ti.uId=nIDTool;
	ti.lpszText=ti.szText;
	StringCbCopy(ti.szText,256,lpszText);
	::SendMessage(m_hWnd,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
}

#ifdef DEF_RESOURCES
void CToolTipCtrl::UpdateTipText(UINT nIDText,HWND hWnd,UINT nIDTool)
{
	CToolInfo ti;
	ti.cbSize=sizeof (TOOLINFO);
	ti.uFlags=TTF_SUBCLASS;
	ti.hwnd=hWnd;
	ti.uId=nIDTool;
	ti.hinst=GetLanguageSpecificResourceHandle();
	ti.lpszText=MAKEINTRESOURCE(nIDText);
	::SendMessage(m_hWnd,TTM_UPDATETIPTEXT,0,(LPARAM)&ti);
}
#endif

void CToolTipCtrl::FillInToolInfo(LPTOOLINFO ti,HWND hWnd,UINT nIDTool) const
{
	ti->cbSize=sizeof (TOOLINFO);
	ti->uFlags=TTF_SUBCLASS;
	ti->hwnd=hWnd;
	ti->uId=nIDTool;
}

///////////////////////////
// Class CReBarCtrl
/////////////////////


///////////////////////////
// Class CToolBarCtrl
///////////////////////////

BOOL CToolBarCtrl::Create(DWORD dwStyle,const RECT* rect,HWND hWndParent,UINT nID)
{
	m_hWnd=CreateWindowEx(0L,TOOLBARCLASSNAME,szEmpty,
      dwStyle,rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,
      hWndParent,(HMENU)nID,GetInstanceHandle(),NULL);
	if (m_hWnd==NULL)
		return FALSE;
	::SendMessage(m_hWnd,TB_BUTTONSTRUCTSIZE,sizeof(TBBUTTON),0);
	return TRUE;
}

#ifdef DEF_RESOURCES
int CToolBarCtrl::AddBitmap(int nNumButtons,UINT nBitmapID)
{
	TBADDBITMAP ab;
	ab.hInst=GetLanguageSpecificResourceHandle();
	ab.nID=nBitmapID;
	return ::SendMessage(m_hWnd,TB_ADDBITMAP,(WPARAM)nNumButtons,(LPARAM)&ab);
}
#endif

int CToolBarCtrl::AddBitmap(int nNumButtons,HBITMAP hBitmap)
{
	TBADDBITMAP ab;
	ab.hInst=NULL;
	ab.nID=(UINT)hBitmap;
	return ::SendMessage(m_hWnd,TB_ADDBITMAP,(WPARAM)nNumButtons,(LPARAM)&ab);
}

void CToolBarCtrl::SaveState(HKEY hKeyRoot,LPCTSTR lpszSubKey,LPCTSTR lpszValueName)
{
	TBSAVEPARAMS sp;
	sp.hkr=hKeyRoot;
	sp.pszSubKey=lpszSubKey;
	sp.pszValueName=lpszValueName;
	::SendMessage(m_hWnd,TB_SAVERESTORE,TRUE,(LPARAM)&sp);
}

void CToolBarCtrl::RestoreState(HKEY hKeyRoot,LPCTSTR lpszSubKey,LPCTSTR lpszValueName)
{
	TBSAVEPARAMS sp;
	sp.hkr=hKeyRoot;
	sp.pszSubKey=lpszSubKey;
	sp.pszValueName=lpszValueName;
	::SendMessage(m_hWnd,TB_SAVERESTORE,FALSE,(LPARAM)&sp);
}

///////////////////////////
// Class CSpinButtonCtrl
///////////////////////////

void CSpinButtonCtrl::GetRange(int &lower,int& upper) const
{
	int ret=::SendMessage(m_hWnd,UDM_GETRANGE,0,0);
	lower=HIWORD(ret);
	upper=LOWORD(ret);
}

///////////////////////////
// Class CTabCtrl
///////////////////////////

BOOL CTabCtrl::Create(DWORD dwStyle,const RECT* rect,HWND hParentWnd,UINT nID)
{
	m_hWnd=CreateWindowEx(0L,WC_TABCONTROL,szEmpty,dwStyle,
		rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,
		hParentWnd,(HMENU)nID,GetInstanceHandle(),NULL);
	if (m_hWnd==NULL)
		return FALSE;
	return TRUE;
}

CSize CTabCtrl::SetItemSize(CSize size)
{
	int ret;
	CSize temp;
	ret=::SendMessage(m_hWnd,TCM_SETITEMSIZE,0,MAKELPARAM(size.cx,size.cy));
	temp.cx=LOWORD(ret);
	temp.cy=HIWORD(ret);
	return temp;
}


///////////////////////////////////////////
// CRichEditCtrl
///////////////////////////////////////////

CPoint CRichEditCtrl::GetCharPos(long lChar) const
{
	CPoint pt;
	int ret=::SendMessage(m_hWnd,EM_POSFROMCHAR,(WPARAM)lChar,0);
	pt.x=LOWORD(ret);
	pt.y=HIWORD(ret);
	return pt;
}

CString CRichEditCtrl::GetSelText() const
{
	CString str;
	::SendMessage(m_hWnd,EM_GETSELTEXT,0,(LPARAM)str.GetBuffer(1000));
	str.FreeExtra();
	return str;
}

BOOL CRichEditCtrl::SetAlignmentForSelection(WORD wAlignment)
{
	PARAFORMAT2 pf;

	pf.cbSize = sizeof(PARAFORMAT2);

	::SendMessage(m_hWnd,EM_GETPARAFORMAT,0,(LPARAM)&pf);
	if(!(pf.dwMask&PFM_ALIGNMENT) || pf.wAlignment!=wAlignment)
	{
		pf.dwMask=PFM_ALIGNMENT;		// only change the alignment
		pf.wAlignment=wAlignment;
		return ::SendMessage(m_hWnd,EM_SETPARAFORMAT,0,(LPARAM) &pf);
	}
	return TRUE;
}

BOOL CRichEditCtrl::SetLineSpacingRuleForSelection(BYTE bLineSpacingRule,LONG dyLineSpacing)
{
	PARAFORMAT2 pf;

	pf.cbSize = sizeof(PARAFORMAT2);

	::SendMessage(m_hWnd,EM_GETPARAFORMAT,0,(LPARAM)&pf);
	if(!(pf.dwMask&PFM_LINESPACING) || pf.bLineSpacingRule!=bLineSpacingRule)
	{
		pf.dwMask=PFM_LINESPACING;		// only change the alignment
		pf.bLineSpacingRule=bLineSpacingRule;
		pf.dyLineSpacing=dyLineSpacing;
		return ::SendMessage(m_hWnd,EM_SETPARAFORMAT,0,(LPARAM) &pf);
	}
	return TRUE;
}
	
BOOL CRichEditCtrl::SetEffectForSelection(DWORD dwEffects,DWORD dwMask)
{
	CHARFORMAT2 cf;
	cf.cbSize=sizeof(CHARFORMAT2);
	cf.dwMask=dwMask;
	cf.dwEffects=dwEffects;
	return ::SendMessage(m_hWnd,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf);
}
	
BOOL CRichEditCtrl::SetUnderlineTypeForSelection(BYTE bUnderlineType)
{
	CHARFORMAT2 cf;
	cf.cbSize=sizeof(CHARFORMAT2);
	cf.dwMask=CFM_UNDERLINETYPE;
	cf.bUnderlineType=bUnderlineType;
	return ::SendMessage(m_hWnd,EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf);
}
	
///////////////////////////
// Class CListCtrl
///////////////////////////

BOOL CListCtrl::SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,
	int nImage, UINT nState, UINT nStateMask, LPARAM lParam)
{
	LV_ITEM li;
	li.mask=nMask;
	li.iItem=nItem;
	li.iSubItem=nSubItem;
	li.state=nState;
	li.stateMask=nStateMask;
	li.pszText=(LPSTR)lpszItem;
	li.iImage=nImage;
	li.lParam=lParam;
	return ::SendMessage(m_hWnd,LVM_SETITEM,0,(LPARAM)&li);
}

BOOL CListCtrl::GetItemRect(int nItem, LPRECT lpRect, UINT nCode) const
{
	if (lpRect!=NULL)
		lpRect->left=nCode;
	return ::SendMessage(m_hWnd,LVM_GETITEMRECT,(WPARAM)nItem,(LPARAM)lpRect);
}

BOOL CListCtrl::SetItemState(int nItem, UINT nState, UINT nMask)
{
	LV_ITEM li;
	ZeroMemory(&li,sizeof(LV_ITEM));
	li.mask=LVIF_STATE;
	li.iItem=nItem;
	li.iSubItem=0;
	li.state=nState;
	li.stateMask=nMask;
	return ::SendMessage(m_hWnd,LVM_SETITEMSTATE,nItem,(LPARAM)&li);
}	

CString CListCtrl::GetItemText(int nItem, int nSubItem) const
{
	CString str;
	LV_ITEM li;
	ZeroMemory(&li,sizeof(LV_ITEM));
	li.mask=LVIF_TEXT;
	li.iItem=nItem;
	li.iSubItem=nSubItem;
	li.pszText=str.GetBuffer(1000);
	li.cchTextMax=1000;
	::SendMessage(m_hWnd,LVM_GETITEMTEXT,nItem,(LPARAM)&li);
	str.FreeExtra();
	return str;
}
	
int CListCtrl::GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const
{
	LV_ITEM li;
	ZeroMemory(&li,sizeof(LV_ITEM));
	li.mask=LVIF_TEXT;
	li.iItem=nItem;
	li.iSubItem=nSubItem;
	li.pszText=lpszText;
	li.cchTextMax=nLen;
	return ::SendMessage(m_hWnd,LVM_GETITEMTEXT,nItem,(LPARAM)&li);
}

BOOL CListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
	LV_ITEM li;
	ZeroMemory(&li,sizeof(LV_ITEM));
	li.mask=LVIF_TEXT;
	li.iItem=nItem;
	li.iSubItem=nSubItem;
	li.pszText=(LPSTR)lpszText;
	return ::SendMessage(m_hWnd,LVM_SETITEMTEXT,nItem,(LPARAM)&li);
}

BOOL CListCtrl::SetItemData(int nItem, DWORD dwData)
{
	LV_ITEM li;
	li.mask=LVIF_PARAM;
	li.lParam=(LPARAM)dwData;
	li.iItem=nItem;
	li.iSubItem=0;
	return ::SendMessage(m_hWnd,LVM_SETITEM,0,(LPARAM)&li);
}

DWORD CListCtrl::GetItemData(int nItem) const
{
	LV_ITEM li;
	li.mask=LVIF_PARAM;
	li.iItem=nItem;
	li.iSubItem=0;
	::SendMessage(m_hWnd,LVM_GETITEM,0,(LPARAM)&li);
	return li.lParam;
}

int CListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
	LV_ITEM li;
	li.mask=LVIF_TEXT;
	li.iItem=nItem;
	li.pszText=(LPSTR)lpszItem;
	li.iSubItem=0;
	return ::SendMessage(m_hWnd,LVM_INSERTITEM,0,(LPARAM)&li);
}

int CListCtrl::InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
{
	LV_ITEM li;
	li.mask=LVIF_TEXT|LVIF_IMAGE;
	li.iImage=nImage;
	li.iItem=nItem;
	li.iSubItem=0;
	li.pszText=(LPSTR)lpszItem;
	return ::SendMessage(m_hWnd,LVM_INSERTITEM,0,(LPARAM)&li);
}

int CListCtrl::HitTest(POINT pt, UINT* pFlags) const
{
	LV_HITTESTINFO lh;
	lh.pt=pt;
	if (pFlags!=NULL)
		lh.flags=*pFlags;
	::SendMessage(m_hWnd,LVM_HITTEST,0,(LPARAM)&lh);
	if (pFlags!=NULL)
		*pFlags=lh.flags;
	return lh.iItem;
}

int CListCtrl::InsertColumn(int nCol, LPCTSTR lpszColumnHeading,
	int nFormat, int nWidth, int nSubItem)
{
	LV_COLUMN lc;
	lc.mask=LVCF_TEXT|LVCF_FMT;
    lc.fmt=nFormat;
	if (nWidth>=0)
	{
		lc.mask|=LVCF_WIDTH;
		lc.cx=nWidth;
	}
	lc.pszText=(LPSTR)lpszColumnHeading;
	if (nSubItem>=0)
	{
		lc.mask|=LVCF_SUBITEM;
		lc.iSubItem=nSubItem;
	}
	return ::SendMessage(m_hWnd,LVM_INSERTCOLUMN,nCol,(LPARAM)&lc);
}

int CListCtrl::InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState,
	UINT nStateMask,int nImage, LPARAM lParam)
{
	LV_ITEM li;
	li.mask=nMask;
	li.iImage=nImage;
	li.iItem=nItem;
	li.state=nState;
	li.stateMask=nStateMask;
	li.iSubItem=0;
	li.pszText=(LPSTR)lpszItem;
	li.lParam=lParam;
	return ::SendMessage(m_hWnd,LVM_INSERTITEM,0,(LPARAM)&li);
}

BOOL CListCtrl::LoadColumnsState(HKEY hRootKey,LPCSTR lpKey,LPCSTR lpSubKey)
{
	CRegKey RegKey;
	if (lpKey==NULL)
		RegKey.m_hKey=hRootKey;
	else if (RegKey.OpenKey(hRootKey,lpKey,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return FALSE;
	DWORD nDataLength=RegKey.QueryValueLength(lpSubKey);
	if (nDataLength<4)
	{
		if (lpKey==NULL)
			RegKey.m_hKey=NULL;
		return FALSE;
	}
	int* pData=(int*)new BYTE[nDataLength];
	DWORD dwRet=RegKey.QueryValue(lpSubKey,(LPSTR)pData,nDataLength);
	if (lpKey==NULL)
		RegKey.m_hKey=NULL;
	if (dwRet<nDataLength)
	{
		delete[] (BYTE*)pData;
		return FALSE;
	}

	if ((pData[0]*2+1)*sizeof(int)!=nDataLength)
	{
		delete[] (BYTE*)pData;
		return FALSE;
	}
	BOOL nOrderOK=FALSE;
	for (int i=0;i<pData[0];i++)
	{
		SetColumnWidth(i,pData[1+i]);
		if (pData[1+pData[0]+i]!=0)
			nOrderOK=TRUE;
	}	
	if (nOrderOK)
		SetColumnOrderArray(pData[0],pData+1+pData[0]);
	delete[] (BYTE*)pData;
	return TRUE;
}

BOOL CListCtrl::SaveColumnsState(HKEY hRootKey,LPCSTR lpKey,LPCSTR lpSubKey) const
{
	CRegKey RegKey;
	if (lpKey==NULL)
		RegKey.m_hKey=hRootKey;
	else if (RegKey.OpenKey(hRootKey,lpKey,CRegKey::createNew|CRegKey::samAll)!=ERROR_SUCCESS)
		return FALSE;
	LVCOLUMN lc;
	fMemSet(&lc,0,sizeof(LVCOLUMN));
	lc.mask=LVCF_WIDTH|LVCF_ORDER;
	CIntArray ColumnWidths;
	CIntArray ColumnOrders;
	for (int i=0;;i++)
	{
		if (!GetColumn(i,&lc))
			break;
		ColumnWidths.Add(lc.cx);
		ColumnOrders.Add(lc.iOrder);
	}
	if (i==0)
	{
		if (lpKey==NULL)
			RegKey.m_hKey=NULL;
		return FALSE;
	}
	int* pData=new int[1+2*i];
	if (pData==NULL)
	{
		if (lpKey==NULL)
			RegKey.m_hKey=NULL;
		return FALSE;
	}
	pData[0]=i;
	iMemCopy(pData+1,ColumnWidths.m_pData,sizeof(int)*i);
	iMemCopy(pData+1+i,ColumnOrders.m_pData,sizeof(int)*i);
	BOOL bRet=RegKey.SetValue(lpSubKey,(LPCSTR)pData,sizeof(int)*(1+2*i),REG_BINARY)==ERROR_SUCCESS;
	RegKey.CloseKey();
	delete[] pData;
	if (lpKey==NULL)
		RegKey.m_hKey=NULL;
	return bRet;
}

///////////////////////////////////////////
// CTreeCtrl
///////////////////////////////////////////

CString CTreeCtrl::GetItemText(HTREEITEM hItem) const
{
	CString str;
	TV_ITEM ti;
	ti.mask=TVIF_TEXT;
	ti.hItem=hItem;
	ti.pszText=str.GetBuffer(1000);
	ti.cchTextMax=1000;
	::SendMessage(m_hWnd,TVM_GETITEM,0,(LPARAM)&ti);
	str.FreeExtra();
	return str;
}

BOOL CTreeCtrl::GetItemImage(HTREEITEM hItem,int& nImage, int& nSelectedImage) const
{
	TV_ITEM ti;
	BOOL ret;
	ti.mask=TVIF_SELECTEDIMAGE|TVIF_IMAGE;
	ti.hItem=hItem;
	ret=::SendMessage(m_hWnd,TVM_GETITEM,0,(LPARAM)&ti);
	nImage=ti.iImage;
	nSelectedImage=ti.iSelectedImage;
	return ret;
}

UINT CTreeCtrl::GetItemState(HTREEITEM hItem,UINT nStateMask) const
{
	TV_ITEM ti;
	ti.mask=TVIF_STATE;
	ti.hItem=hItem;
	ti.stateMask=nStateMask;
	::SendMessage(m_hWnd,TVM_GETITEM,0,(LPARAM)&ti);
	return ti.state;
}

DWORD CTreeCtrl::GetItemData(HTREEITEM hItem) const
{
	TV_ITEM ti;
	ti.mask=TVIF_PARAM	;
	ti.hItem=hItem;
	::SendMessage(m_hWnd,TVM_GETITEM,0,(LPARAM)&ti);
	return ti.lParam;
}

BOOL CTreeCtrl::SetItem(HTREEITEM hItem,UINT nMask,LPCTSTR lpszItem,int nImage,int nSelectedImage,UINT nState,UINT nStateMask,LPARAM lParam)
{
	TV_ITEM ti;
	ti.mask=nMask;
	ti.hItem=hItem;
	ti.state=nState;
	ti.stateMask=nStateMask;
	ti.pszText=(LPSTR)lpszItem;
	ti.iImage=nImage;
	ti.iSelectedImage=nSelectedImage;
	ti.cChildren=0;
	ti.lParam=lParam;
	return ::SendMessage(m_hWnd,TVM_SETITEM,0,(LPARAM)&ti);
}

BOOL CTreeCtrl::SetItemText(HTREEITEM hItem,LPCSTR lpszItem)
{
	TV_ITEMA ti;
	ti.mask=TVIF_TEXT;
	ti.pszText=(LPSTR)lpszItem;
	ti.hItem=hItem;
	return ::SendMessage(m_hWnd,TVM_SETITEMA,0,(LPARAM)&ti);
}

BOOL CTreeCtrl::SetItemText(HTREEITEM hItem,LPCWSTR lpszItem)
{
	TV_ITEMW ti;
	ti.mask=TVIF_TEXT;
	ti.pszText=(LPWSTR)lpszItem;
	ti.hItem=hItem;
	return ::SendMessage(m_hWnd,TVM_SETITEMW,0,(LPARAM)&ti);
}

BOOL CTreeCtrl::SetItemImage(HTREEITEM hItem,int nImage,int nSelectedImage)
{
	TV_ITEM ti;
	ti.mask=TVIF_IMAGE|TVIF_SELECTEDIMAGE;
	ti.iImage=nImage;
	ti.iSelectedImage=nSelectedImage;
	ti.hItem=hItem;
	return ::SendMessage(m_hWnd,TVM_SETITEM,0,(LPARAM)&ti);
}

BOOL CTreeCtrl::SetItemState(HTREEITEM hItem,UINT nState,UINT nStateMask)
{
	TV_ITEM ti;
	ti.mask=TVIF_STATE;
	ti.state=nState;
	ti.stateMask=nStateMask;
	ti.hItem=hItem;
	return ::SendMessage(m_hWnd,TVM_SETITEM,0,(LPARAM)&ti);
}

BOOL CTreeCtrl::SetItemData(HTREEITEM hItem,DWORD dwData)
{
	TV_ITEM ti;
	ti.mask=TVIF_PARAM;
	ti.lParam=dwData;
	ti.hItem=hItem;
	return ::SendMessage(m_hWnd,TVM_SETITEM,0,(LPARAM)&ti);
}

HTREEITEM CTreeCtrl::InsertItem(UINT nMask,LPCTSTR lpszItem,int nImage,int nSelectedImage,UINT nState,UINT nStateMask,LPARAM lParam,HTREEITEM hParent,HTREEITEM hInsertAfter)
{
	TV_INSERTSTRUCT is;
	is.item.mask=nMask;
	is.item.hItem=NULL;
	is.item.state=nState;
	is.item.stateMask=nStateMask;
	is.item.pszText=(LPSTR)lpszItem;
	is.item.iImage=nImage;
	is.item.iSelectedImage=nSelectedImage;
	is.item.cChildren=0;
	is.item.lParam=lParam;
	is.hParent=hParent;
	is.hInsertAfter=hInsertAfter;
	return (HTREEITEM)::SendMessage(m_hWnd,TVM_INSERTITEM,0,(LPARAM)&is);
}

HTREEITEM CTreeCtrl::InsertItem(LPCSTR lpszItem,HTREEITEM hParent,HTREEITEM hInsertAfter)
{
	TV_INSERTSTRUCTA is;
	is.item.mask=TVIF_TEXT;
	is.item.hItem=NULL;
	is.item.pszText=(LPSTR)lpszItem;
	is.hParent=hParent;
	is.hInsertAfter=hInsertAfter;
	return (HTREEITEM)::SendMessage(m_hWnd,TVM_INSERTITEM,0,(LPARAM)&is);
}

HTREEITEM CTreeCtrl::InsertItem(LPCWSTR lpszItem,HTREEITEM hParent,HTREEITEM hInsertAfter)
{
	TV_INSERTSTRUCTW is;
	is.item.mask=TVIF_TEXT;
	is.item.hItem=NULL;
	is.item.pszText=(LPWSTR)lpszItem;
	is.hParent=hParent;
	is.hInsertAfter=hInsertAfter;
	return (HTREEITEM)::SendMessage(m_hWnd,TVM_INSERTITEMW,0,(LPARAM)&is);
}

HTREEITEM CTreeCtrl::InsertItem(LPCSTR lpszItem,int nImage,int nSelectedImage,HTREEITEM hParent,HTREEITEM hInsertAfter)
{
	TV_INSERTSTRUCT is;
	is.item.mask=TVIF_TEXT|TVIF_IMAGE;
	is.item.hItem=NULL;
	is.item.pszText=(LPSTR)lpszItem;
	is.item.iImage=nImage;
	is.item.iSelectedImage=nSelectedImage;
	is.hParent=hParent;
	is.hInsertAfter=hInsertAfter;
	return (HTREEITEM)::SendMessage(m_hWnd,TVM_INSERTITEM,0,(LPARAM)&is);
}

HTREEITEM CTreeCtrl::InsertItem(LPCWSTR lpszItem,int nImage,int nSelectedImage,HTREEITEM hParent,HTREEITEM hInsertAfter)
{
	TV_INSERTSTRUCTW is;
	is.item.mask=TVIF_TEXT|TVIF_IMAGE;
	is.item.hItem=NULL;
	is.item.pszText=(LPWSTR)lpszItem;
	is.item.iImage=nImage;
	is.item.iSelectedImage=nSelectedImage;
	is.hParent=hParent;
	is.hInsertAfter=hInsertAfter;
	return (HTREEITEM)::SendMessage(m_hWnd,TVM_INSERTITEMW,0,(LPARAM)&is);
}

HTREEITEM CTreeCtrl::HitTest(CPoint pt,UINT* pFlags) const
{
	TV_HITTESTINFO hi;
	iMemCopy(&hi.pt,&pt,sizeof(POINT));
	::SendMessage(m_hWnd,TVM_HITTEST,0,(LPARAM)&hi);
	if (pFlags!=NULL)
		*pFlags=hi.flags;
	return hi.hItem;
}

///////////////////////////////////////////
// CComboBoxEx
///////////////////////////////////////////

BOOL CComboBoxEx::Create(DWORD dwStyle, const RECT* rect, HWND hParentWnd, UINT nID)
{
	CCommonCtrl::m_hWnd=CreateWindowEx(0L,WC_COMBOBOXEX,szEmpty,dwStyle,
		rect->left,rect->top,rect->right-rect->left,rect->bottom-rect->top,
		hParentWnd,(HMENU)nID,GetInstanceHandle(),NULL);
	if (CCommonCtrl::m_hWnd==NULL)
		return FALSE;
	CComboBox::m_hWnd=(HWND)::SendMessage(CCommonCtrl::m_hWnd,CBEM_GETCOMBOCONTROL,0,0);
	CEdit::m_hWnd=(HWND)::SendMessage(CCommonCtrl::m_hWnd,CBEM_GETEDITCONTROL,0,0);
	return TRUE;
}

int CComboBoxEx::InsertItem(LPCSTR pszText,int iImage,int iSelectedImage,int iOverlay,int iIndent,LPARAM lParam,UINT mask)
{
	COMBOBOXEXITEM cbei;
	if (mask)
		cbei.mask=mask;
	else
	{
		cbei.mask=CBEIF_LPARAM;
		if (pszText!=NULL)
			cbei.mask|=CBEIF_TEXT;
		if (iImage!=-1)
			cbei.mask|=CBEIF_IMAGE;
		if (iSelectedImage!=-1)
			cbei.mask|=CBEIF_IMAGE;
		if (iOverlay!=-1)
			cbei.mask|=CBEIF_OVERLAY;
		if (iIndent!=-1)
			cbei.mask|=CBEIF_INDENT;
	}
	cbei.iItem=::SendMessage(CComboBox::m_hWnd,CB_GETCOUNT,0,0);
	cbei.iImage=iImage;
	cbei.iSelectedImage=iSelectedImage;
	cbei.iOverlay=iOverlay;
	cbei.iIndent=iIndent;
	cbei.lParam=lParam;
	cbei.pszText=(LPSTR)pszText;
	return ::SendMessage(CCommonCtrl::m_hWnd,CBEM_INSERTITEM,0,(LPARAM)&cbei);
}
	
int CComboBoxEx::InsertItem(LPCWSTR pszText,int iImage,int iSelectedImage,int iOverlay,int iIndent,LPARAM lParam,UINT mask)
{
	COMBOBOXEXITEMW cbei;
	if (mask)
		cbei.mask=mask;
	else
	{
		cbei.mask=CBEIF_LPARAM;
		if (pszText!=NULL)
			cbei.mask|=CBEIF_TEXT;
		if (iImage!=-1)
			cbei.mask|=CBEIF_IMAGE;
		if (iSelectedImage!=-1)
			cbei.mask|=CBEIF_IMAGE;
		if (iOverlay!=-1)
			cbei.mask|=CBEIF_OVERLAY;
		if (iIndent!=-1)
			cbei.mask|=CBEIF_INDENT;
	}
	cbei.iItem=::SendMessage(CComboBox::m_hWnd,CB_GETCOUNT,0,0);
	cbei.iImage=iImage;
	cbei.iSelectedImage=iSelectedImage;
	cbei.iOverlay=iOverlay;
	cbei.iIndent=iIndent;
	cbei.lParam=lParam;
	cbei.pszText=(LPWSTR)pszText;
	return ::SendMessage(CCommonCtrl::m_hWnd,CBEM_INSERTITEMW,0,(LPARAM)&cbei);
}
	
CString CComboBoxEx::GetItemText(int nItem) const
{
	CString str;
	COMBOBOXEXITEM ce;
	ce.iItem=nItem;
	ce.cchTextMax=2000;
	ce.pszText=str.GetBuffer(2000);
	ce.mask=CBEIF_TEXT;
	::SendMessage(CCommonCtrl::m_hWnd,CBEM_GETITEM,0,(LPARAM)&ce);
	str.FreeExtra();
	return str;
}

CStringW CComboBoxEx::GetItemTextW(int nItem) const
{
	CStringW str;
	COMBOBOXEXITEMW ce;
	ce.iItem=nItem;
	ce.cchTextMax=2000;
	ce.pszText=str.GetBuffer(2000);
	ce.mask=CBEIF_TEXT;
	::SendMessageW(CCommonCtrl::m_hWnd,CBEM_GETITEMW,0,(LPARAM)&ce);
	str.FreeExtra();
	return str;
}

int CComboBoxEx::GetItemText(int nItem,LPSTR lpszText,int nLen) const
{
	COMBOBOXEXITEM ce;
	ce.iItem=nItem;
	ce.cchTextMax=nLen;
	ce.pszText=lpszText;
	ce.mask=CBEIF_TEXT;
	return ::SendMessage(CCommonCtrl::m_hWnd,CBEM_GETITEM,0,(LPARAM)&ce);
}

int CComboBoxEx::GetItemText(int nItem,LPWSTR lpszText,int nLen) const
{
	COMBOBOXEXITEMW ce;
	ce.iItem=nItem;
	ce.cchTextMax=nLen;
	ce.pszText=lpszText;
	ce.mask=CBEIF_TEXT;
	return ::SendMessageW(CCommonCtrl::m_hWnd,CBEM_GETITEMW,0,(LPARAM)&ce);
}

BOOL CComboBoxEx::SetItemText(int nItem,LPCSTR lpszText)
{
	COMBOBOXEXITEM ce;
	ce.pszText=(LPSTR)lpszText;
	ce.iItem=nItem;
	ce.mask=CBEIF_TEXT;
	return ::SendMessage(CCommonCtrl::m_hWnd,CBEM_SETITEM,0,(LPARAM)&ce);
}

BOOL CComboBoxEx::SetItemText(int nItem,LPCWSTR lpszText)
{
	COMBOBOXEXITEMW ce;
	ce.pszText=(LPWSTR)lpszText;
	ce.iItem=nItem;
	ce.mask=CBEIF_TEXT;
	return ::SendMessageW(CCommonCtrl::m_hWnd,CBEM_SETITEMW,0,(LPARAM)&ce);
}

DWORD CComboBoxEx::GetItemData(int nIndex) const
{
	COMBOBOXEXITEM ce;
	ce.mask=CBEIF_LPARAM;
	ce.iItem=nIndex;
	if (!::SendMessage(CCommonCtrl::m_hWnd,CBEM_GETITEM,0,(LPARAM)&ce))
		return 0;
	return ce.lParam;
}

int CComboBoxEx::SetItemData(int nIndex, DWORD dwItemData)
{
	COMBOBOXEXITEM ce;
	ce.mask=CBEIF_LPARAM;
	ce.iItem=nIndex;
	ce.lParam=dwItemData;
	return ::SendMessage(CCommonCtrl::m_hWnd,CBEM_SETITEM,0,(LPARAM)&ce);
}

void* CComboBoxEx::GetItemDataPtr(int nIndex) const
{
	COMBOBOXEXITEM ce;
	ce.mask=CBEIF_LPARAM;
	ce.iItem=nIndex;
	if (!::SendMessage(CCommonCtrl::m_hWnd,CBEM_GETITEM,0,(LPARAM)&ce))
		return 0;
	return (void*)ce.lParam;
}


#endif