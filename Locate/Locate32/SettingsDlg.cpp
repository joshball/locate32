#include <HFCLib.h>
#include "Locate32.h"

inline BOOL operator==(const SYSTEMTIME& s1,const SYSTEMTIME& s2)
{
	return (s1.wYear==s2.wYear && s1.wMonth==s2.wMonth && s1.wDay==s2.wDay &&
		s1.wHour==s2.wHour && s1.wMinute==s2.wMinute && s1.wSecond==s2.wSecond &&
		s1.wMilliseconds==s2.wMilliseconds);
}

inline BOOL operator!=(const SYSTEMTIME& s1,const SYSTEMTIME& s2)
{
	return !(s1.wYear==s2.wYear && s1.wMonth==s2.wMonth && s1.wDay==s2.wDay &&
		s1.wHour==s2.wHour && s1.wMinute==s2.wMinute && s1.wSecond==s2.wSecond &&
		s1.wMilliseconds==s2.wMilliseconds);
}

#define IDC_EDITCONTROLFORSELECTEDITEM  1300
#define IDC_SPINCONTROLFORSELECTEDITEM  1301
#define IDC_COMBOCONTROLFORSELECTEDITEM 1302
#define IDC_COLORBUTTONFORSELECTEDITEM	1303
#define IDC_FONTBUTTONFORSELECTEDITEM	1304
#define IDC_BROWSEBUTTONFORSELECTEDITEM	1305
//#define sMemCopyW	MemCopyW


COptionsPropertyPage::Item::Item(
	ItemType nType_,Item* pParent_,Item** pChilds_,LPWSTR pString_,
	CALLBACKPROC pProc_,DWORD wParam_,void* lParam_)
:	nType(nType_),pParent(pParent_),pData(NULL),bEnabled(TRUE),
	wParam(wParam_),lParam(lParam_),pProc(pProc_),
	m_nStateIcon(-1),hControl(NULL),hControl2(NULL)
{
	if (pString_!=NULL)
		pString=alloccopy(pString_);
	else 
		pString=NULL;
	
	if (pChilds_!=NULL)
	{
		int i;
		for (i=0;pChilds_[i]!=NULL;i++);
		if (i>0)
		{
			pChilds=new Item*[i+1];
			CopyMemory(pChilds,pChilds_,sizeof(Item*)*(i+1));
			return;
		}
	}
	pChilds=NULL;
}

COptionsPropertyPage::Item::Item(
	ItemType nType_,Item* pParent_,Item** pChilds_,UINT nStringID,
	CALLBACKPROC pProc_,DWORD wParam_,void* lParam_)
:	nType(nType_),pParent(pParent_),pData(NULL),bEnabled(TRUE),
	wParam(wParam_),lParam(lParam_),pProc(pProc_),
	m_nStateIcon(-1),hControl(NULL),hControl2(NULL)
{
	int nCurLen=50;
	int iLength;
	
	if (!IsUnicodeSystem())
	{
		// Non-unicode
		char* szText=new char[nCurLen];
		while ((iLength=::LoadString(GetResourceHandle(LanguageSpecificResource),nStringID,szText,nCurLen)+1)>=nCurLen)
		{
			delete[] szText;
			nCurLen+=50;
			szText=new char[nCurLen];
		}
		pString=new WCHAR[iLength];
		MemCopyAtoW(pString,szText,iLength);
		delete[] szText;
	}
	else
	{
		// Unicode
		WCHAR* szText=new WCHAR[nCurLen];
		while ((iLength=::LoadStringW(GetResourceHandle(LanguageSpecificResource),nStringID,szText,nCurLen)+1)>=nCurLen)
		{
			delete[] szText;
			nCurLen+=50;
			szText=new WCHAR[nCurLen];
		}
		pString=new WCHAR[iLength];
		MemCopyW(pString,szText,iLength);
		delete[] szText;
	}

	if (pChilds_!=NULL)
	{
		int i;
		for (i=0;pChilds_[i]!=NULL;i++);
		if (i>0)
		{
			pChilds=new Item*[i+1];
			CopyMemory(pChilds,pChilds_,sizeof(Item*)*(i+1));
			return;
		}
	}
	pChilds=NULL;

	

}


COptionsPropertyPage::Item::~Item()
{
	if (pChilds!=NULL)
	{
		for (int i=0;pChilds[i]!=NULL;i++)
			delete pChilds[i];
		delete[] pChilds;
	}
	if (pString!=NULL)
		delete[] pString;

	switch (nType)
	{
	case Combo:
	case Edit:
	case File:
		if (pData!=NULL)
			delete[] pData;
		break;
	case Font:
		if (pLogFont!=NULL)
			delete pLogFont;
		break;
	}
}

void COptionsPropertyPage::Construct(const OPTIONPAGE* pOptionPage,TypeOfResourceHandle bType)
{
	if (pOptionPage->dwFlags&OPTIONPAGE::opTemplateIsID)
		m_lpszTemplateName=MAKEINTRESOURCE(pOptionPage->nIDTemplate);
	else
	{
		if (IsUnicodeSystem())
			m_lpszTemplateNameW=alloccopy(pOptionPage->lpszTemplateName);
		else
			m_lpszTemplateName=alloccopyWtoA(pOptionPage->lpszTemplateName);
	}


	if (pOptionPage->dwFlags&OPTIONPAGE::opCaptionIsID)
		CPropertyPage::Construct(pOptionPage->nIDCaption,bType);
	else
		CPropertyPage::Construct(pOptionPage->lpszCaption,bType);

	m_nTreeID=pOptionPage->nTreeCtrlID;


	if (pOptionPage->dwFlags&OPTIONPAGE::opChangeIsID)
		m_ChangeText.LoadString(pOptionPage->nIDChangeText);
	else
		m_ChangeText=pOptionPage->lpszChangeText;


}

BOOL COptionsPropertyPage::Initialize(COptionsPropertyPage::Item** pItems)
{
	if (m_pTree==NULL)
	{
		m_pTree=new CTreeCtrl(GetDlgItem(IDC_SETTINGS));
		m_Images.Create(IDB_OPTIONSPROPERTYPAGEBITMAPS,16,256,RGB(255,255,255),IMAGE_BITMAP,LR_SHARED|LR_CREATEDIBSECTION);
		m_pTree->SetImageList(m_Images,TVSIL_STATE);
		if (IsUnicodeSystem())
			m_pTree->SetUnicodeFormat(TRUE);

		// Subclassing tree control
		UserData* pUserData=new UserData;
		pUserData->pDialog=this;
		pUserData->pOldWndProc=(WNDPROC)m_pTree->SetWindowLong(gwlWndProc,(LONG_PTR)TreeSubClassFunc);

		if (pUserData->pOldWndProc==NULL)
		{
			// Subclassing didn't success
			delete pUserData;
		}
		else
			m_pTree->SetWindowLong(gwlUserData,(LONG_PTR)pUserData);

		
	}

	if (pItems==NULL)
		return FALSE;
	
	// Counting items
	int iItems;
	for (iItems=0;pItems[iItems]!=NULL;iItems++);
	
	m_pItems=new Item*[max(iItems+1,2)];
	m_pItems[iItems]=NULL;
	CopyMemory(m_pItems,pItems,sizeof(Item*)*(iItems+1));
	return InsertItemsToTree(NULL,m_pItems,NULL);
	
}



BOOL COptionsPropertyPage::InsertItemsToTree(HTREEITEM hParent,COptionsPropertyPage::Item** pItems,COptionsPropertyPage::Item* pParent)
{
	INITIALIZEPARAMS bp;
	bp.pPage=this;

	

	union {
		TVINSERTSTRUCTA tisa;
		TVINSERTSTRUCTW tisw;
	};
	
	if (!IsUnicodeSystem())
	{
		// Windows 9x
		tisa.hInsertAfter=TVI_LAST;
		tisa.hParent=hParent;
		tisa.itemex.stateMask=TVIS_STATEIMAGEMASK|TVIS_EXPANDED;
		tisa.itemex.mask=TVIF_STATE|TVIF_CHILDREN|TVIF_TEXT|TVIF_PARAM;
	}
	else
	{
		// Windows NT/2000/XP
		tisw.hInsertAfter=TVI_LAST;
		tisw.hParent=hParent;
		tisw.itemex.stateMask=TVIS_STATEIMAGEMASK|TVIS_EXPANDED;
		tisw.itemex.mask=TVIF_STATE|TVIF_CHILDREN|TVIF_TEXT|TVIF_PARAM;
	}

	HTREEITEM hSelectedRadioItem=NULL;
	
	int nItemHeight=18;
    for (int i=0;pItems[i]!=NULL;i++)
	{
		if (pItems[i]->pProc!=NULL)
		{
			bp.crReason=BASICPARAMS::Get;
			pItems[i]->SetValuesForBasicParams(&bp);
			if (pItems[i]->pProc(&bp))
				pItems[i]->GetValuesFromBasicParams(&bp);
		}
        			
		if (!IsUnicodeSystem())
		{
			SIZE_T iStrLen=istrlenw(pItems[i]->pString);
			tisa.itemex.pszText=new char [iStrLen+2];
			MemCopyWtoA(tisa.itemex.pszText,pItems[i]->pString,iStrLen+1);
			tisa.itemex.cChildren=pItems[i]->pChilds==0?0:1;
			tisa.itemex.lParam=LPARAM(pItems[i]);
			tisa.itemex.state=TVIS_EXPANDED|INDEXTOSTATEIMAGEMASK(pItems[i]->GetStateImage(&m_Images));
			tisa.hInsertAfter=m_pTree->InsertItem(&tisa);
			delete[] tisa.itemex.pszText;
		}
		else
		{
			tisw.itemex.pszText=pItems[i]->pString;
			tisw.itemex.cChildren=pItems[i]->pChilds==0?0:1;
			tisw.itemex.lParam=LPARAM(pItems[i]);
			tisw.itemex.state=TVIS_EXPANDED|INDEXTOSTATEIMAGEMASK(pItems[i]->GetStateImage(&m_Images));
			tisw.hInsertAfter=m_pTree->InsertItem(&tisw);
		}


		if (pItems[i]->pChilds!=NULL)
			InsertItemsToTree(!IsUnicodeSystem()?tisa.hInsertAfter:tisw.hInsertAfter,pItems[i]->pChilds,pItems[i]);

		// Type specified actions
		switch (pItems[i]->nType)
		{
		case Item::RadioBox:
			if (pItems[i]->bChecked)
				hSelectedRadioItem=!IsUnicodeSystem()?tisa.hInsertAfter:tisw.hInsertAfter;
			// Continuing
		case Item::CheckBox:
			EnableChilds(!IsUnicodeSystem()?tisa.hInsertAfter:tisw.hInsertAfter,pItems[i]->bChecked);
			break;
		case Item::Edit:
		case Item::File:
			{
				pItems[i]->hControl=CreateWindow("EDIT","",
					ES_AUTOHSCROLL|WS_CHILDWINDOW|WS_BORDER,
					10,10,100,13,*this,(HMENU)IDC_EDITCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
				::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

				// Subclassing control
				UserData* pUserData=new UserData;
				pUserData->pDialog=this;
				pUserData->pOldWndProc=(WNDPROC)::SetWindowLongPtr(pItems[i]->hControl,
					GWLP_WNDPROC,(LONG_PTR)EditSubClassFunc);

				if (pUserData->pOldWndProc==NULL)
				{
					// Subclassing didn't success
					delete pUserData;
				}
				else
					::SetWindowLongPtr(pItems[i]->hControl,GWLP_USERDATA,(LONG_PTR)pUserData);

				
				// Initializing
				if (pItems[i]->pProc!=NULL)
				{
					bp.crReason=BASICPARAMS::Initialize;
					bp.hControl=pItems[i]->hControl;
					pItems[i]->pProc(&bp);
				}
				if (pItems[i]->pData!=NULL)
				{
					if (IsUnicodeSystem())
						::SendMessageW(pItems[i]->hControl,WM_SETTEXT,0,LPARAM(pItems[i]->pData));
					else
						::SendMessage(pItems[i]->hControl,WM_SETTEXT,0,LPARAM((LPCSTR)W2A(pItems[i]->pData)));
				}
				break;
			}
		case Item::Numeric:
			{
				pItems[i]->hControl=CreateWindow("EDIT","",
					ES_AUTOHSCROLL|WS_CHILDWINDOW|WS_BORDER|ES_NUMBER,
					10,10,50,20,*this,(HMENU)IDC_EDITCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
				::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

				// Setting subclass info struct
				UserData* pUserData=new UserData;
				pUserData->pDialog=this;
				pUserData->pOldWndProc=NULL;
				::SetWindowLongPtr(pItems[i]->hControl,GWLP_USERDATA,(LONG_PTR)pUserData);
	
				// Initializing
				if (pItems[i]->pProc!=NULL)
				{
					bp.crReason=BASICPARAMS::Initialize;
					bp.hControl=pItems[i]->hControl;
					pItems[i]->pProc(&bp);
				}
				if (IsUnicodeSystem())
				{
					WCHAR szText[100];
					_itow_s(pItems[i]->lValue,szText,100,10);
					::SendMessageW(pItems[i]->hControl,WM_SETTEXT,0,LPARAM(szText));
				}
				else
				{
					char szText[100];
					_itoa_s(pItems[i]->lValue,szText,100,10);
					::SendMessage(pItems[i]->hControl,WM_SETTEXT,0,LPARAM(szText));
				}
			}
			break;
		case Item::List:
			{
				pItems[i]->hControl=CreateWindow("COMBOBOX","",
					CBS_DROPDOWNLIST|WS_VSCROLL|WS_CHILDWINDOW|WS_BORDER,
					10,10,100,100,*this,(HMENU)IDC_COMBOCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
				::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);
				
				// Subclassing control
				UserData* pUserData=new UserData;
				pUserData->pDialog=this;
				pUserData->pOldWndProc=(WNDPROC)::SetWindowLongPtr(pItems[i]->hControl,
					GWLP_WNDPROC,(LONG_PTR)ComboSubClassFunc);

				if (pUserData->pOldWndProc==NULL)
				{
					// Subclassing didn't success
					delete pUserData;
				}
				else
					::SetWindowLongPtr(pItems[i]->hControl,GWLP_USERDATA,(LONG_PTR)pUserData);

				// Initializing
				if (pItems[i]->pProc!=NULL)
				{
					bp.crReason=BASICPARAMS::Initialize;
					bp.hControl=pItems[i]->hControl;
					pItems[i]->pProc(&bp);
				}
				::SendMessage(pItems[i]->hControl,CB_SETCURSEL,pItems[i]->lValue,NULL);
				break;
			}			
		case Item::Combo:
			{
				pItems[i]->hControl=CreateWindow("COMBOBOX","",
					CBS_DROPDOWN|WS_VSCROLL|WS_CHILDWINDOW|WS_BORDER,
					10,10,100,100,*this,(HMENU)IDC_COMBOCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
				::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

				// Subclassing control
				UserData* pUserData=new UserData;
				pUserData->pDialog=this;
				pUserData->pOldWndProc=(WNDPROC)::SetWindowLongPtr(pItems[i]->hControl,
					GWLP_WNDPROC,(LONG_PTR)ComboSubClassFunc);

				if (pUserData->pOldWndProc==NULL)
				{
					// Subclassing didn't success
					delete pUserData;
				}
				else
					::SetWindowLongPtr(pItems[i]->hControl,GWLP_USERDATA,(LONG_PTR)pUserData);

				HWND hEdit=GetWindow(pItems[i]->hControl,GW_CHILD);
				if (hEdit!=NULL)
				{
					UserData* pUserData=new UserData;
					pUserData->pDialog=this;
					pUserData->pOldWndProc=(WNDPROC)::SetWindowLongPtr(hEdit,
						GWLP_WNDPROC,(LONG_PTR)ComboSubClassFunc);

					if (pUserData->pOldWndProc==NULL)
					{
						// Subclassing didn't success
						delete pUserData;
					}
					else
						::SetWindowLongPtr(hEdit,GWLP_USERDATA,(LONG_PTR)pUserData);
				}

				// Initializing
				if (pItems[i]->pProc!=NULL)
				{
					bp.crReason=BASICPARAMS::Initialize;
					bp.hControl=pItems[i]->hControl;
					pItems[i]->pProc(&bp);
				}

				
				if (pItems[i]->pData!=NULL)
				{
					// Checking whether value is found in combo
					int nFind=(int)::SendMessage(pItems[i]->hControl,CB_FINDSTRINGEXACT,0,LPARAM(pItems[i]->pData));
					::SendMessage(pItems[i]->hControl,CB_SETCURSEL,nFind,0);
					if (nFind==CB_ERR)
					{
						if (IsUnicodeSystem())
							::SendMessageW(pItems[i]->hControl,WM_SETTEXT,0,LPARAM(pItems[i]->pData));
						else
							::SendMessage(pItems[i]->hControl,WM_SETTEXT,0,LPARAM((LPCSTR)W2A(pItems[i]->pData)));
					}
				}
				break;
			}
		case Item::Font:
			{
				if (IsUnicodeSystem())
					pItems[i]->hControl=CreateWindowW(L"BUTTON",m_ChangeText,BS_PUSHBUTTON|WS_CHILDWINDOW,
						10,10,100,13,*this,(HMENU)IDC_FONTBUTTONFORSELECTEDITEM,GetInstanceHandle(),NULL);
				else
					pItems[i]->hControl=CreateWindow("BUTTON",W2A(m_ChangeText),BS_PUSHBUTTON|WS_CHILDWINDOW,
						10,10,100,13,*this,(HMENU)IDC_FONTBUTTONFORSELECTEDITEM,GetInstanceHandle(),NULL);

				::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

				// Subclassing control
				UserData* pUserData=new UserData;
				pUserData->pDialog=this;
				pUserData->pOldWndProc=(WNDPROC)::SetWindowLongPtr(pItems[i]->hControl,
					GWLP_WNDPROC,(LONG_PTR)ButtonSubClassFunc);

				if (pUserData->pOldWndProc==NULL)
				{
					// Subclassing didn't success
					delete pUserData;
				}
				else
					::SetWindowLongPtr(pItems[i]->hControl,GWLP_USERDATA,(LONG_PTR)pUserData);

				// Initializing
				if (pItems[i]->pProc!=NULL)
				{
					bp.crReason=BASICPARAMS::Initialize;
					bp.hControl=pItems[i]->hControl;
					pItems[i]->pProc(&bp);
				}
				break;
			}		
		case Item::Color:
			{
				if (IsUnicodeSystem())
					pItems[i]->hControl=CreateWindowW(L"BUTTON",m_ChangeText,BS_PUSHBUTTON|WS_CHILDWINDOW,
						10,10,100,13,*this,(HMENU)IDC_COLORBUTTONFORSELECTEDITEM,GetInstanceHandle(),NULL);
				else
					pItems[i]->hControl=CreateWindow("BUTTON",W2A(m_ChangeText),BS_PUSHBUTTON|WS_CHILDWINDOW,
						10,10,100,13,*this,(HMENU)IDC_COLORBUTTONFORSELECTEDITEM,GetInstanceHandle(),NULL);

				::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

				// Subclassing control
				UserData* pUserData=new UserData;
				pUserData->pDialog=this;
				pUserData->pOldWndProc=(WNDPROC)::SetWindowLongPtr(pItems[i]->hControl,
					GWLP_WNDPROC,(LONG_PTR)ButtonSubClassFunc);

				if (pUserData->pOldWndProc==NULL)
				{
					// Subclassing didn't success
					delete pUserData;
				}
				else
					::SetWindowLongPtr(pItems[i]->hControl,GWLP_USERDATA,(LONG_PTR)pUserData);

				// Initializing
				if (pItems[i]->pProc!=NULL)
				{
					bp.crReason=BASICPARAMS::Initialize;
					bp.hControl=pItems[i]->hControl;
					pItems[i]->pProc(&bp);
				}
				break;
			}			
		}
		
		// Setting text
		LPWSTR pCurText=pItems[i]->GetText();
		if (pCurText!=pItems[i]->pString)
		{
			if (!IsUnicodeSystem())
			{
				int iStrLen=istrlenw(pCurText);
				tisa.itemex.pszText=new char[iStrLen+2];
				MemCopyWtoA(tisa.itemex.pszText,pCurText,iStrLen+1);
				m_pTree->SetItemText(tisa.hInsertAfter,tisa.itemex.pszText);
				delete[] tisa.itemex.pszText;
			}
			else
				m_pTree->SetItemText(tisw.hInsertAfter,pCurText);
			
		}
		pItems[i]->FreeText(pCurText);


		
		if (pItems[i]->hControl!=NULL)
		{
			CRect rc;
			::GetWindowRect(pItems[i]->hControl,&rc);
			if (rc.Height()-2>nItemHeight)
				nItemHeight=rc.Height()-2;
		}
	}

	// Ensuring that one radio is at least selected
	if (hSelectedRadioItem==NULL)
	{
		HTREEITEM hItem;
		if (hParent==NULL)
			hItem=m_pTree->GetNextItem(NULL,TVGN_ROOT);
		else
			hItem=m_pTree->GetNextItem(hParent,TVGN_CHILD);

		while (hItem!=NULL)
		{
			Item* pItem=(Item*)m_pTree->GetItemData(hItem);
			if (pItem!=NULL)
			{
				if (pItem->nType==Item::RadioBox)
				{
					hSelectedRadioItem=hItem;
					SetCheckState(hItem,pItem,Checked);					
					break;
				}
			}            			
			hItem=m_pTree->GetNextItem(hItem,TVGN_NEXT);
		}
	}
	else
		UncheckOtherRadioButtons(hSelectedRadioItem,hParent);

	if (nItemHeight>m_pTree->GetItemHeight())
		m_pTree->SetItemHeight(nItemHeight);
	return TRUE;
}

BOOL COptionsPropertyPage::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CPropertyPage::OnCommand(wID,wNotifyCode,hControl);

	switch (wID)
	{
	case IDC_EDITCONTROLFORSELECTEDITEM:
		switch (wNotifyCode)
		{
		case EN_CHANGE:
			{
				HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
				if (hItem==NULL)
					break;
				
				Item* pItem=(Item*)m_pTree->GetItemData(hItem);
				if (pItem==NULL)
					break;
				if (pItem->hControl!=hControl)
					break;
				
				if (pItem->nType==Item::Numeric)
					SetNumericValue(pItem);
				else if (pItem->nType==Item::Edit || pItem->nType==Item::File)
					SetTextValue(pItem);
				
				break;
			}
		case EN_SETFOCUS:
			::SendMessage(hControl,EM_SETSEL,0,MAKELPARAM(0,-1));
			break;
		}
		break;
	case IDC_SPINCONTROLFORSELECTEDITEM:
		break;
	case IDC_COMBOCONTROLFORSELECTEDITEM:
		switch (wNotifyCode)
		{
		case CBN_SELCHANGE:
			{
				HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
				if (hItem==NULL)
					break;
				Item* pItem=(Item*)m_pTree->GetItemData(hItem);
				if (pItem==NULL)
					break;
				if (pItem->hControl!=hControl)
					break;
				if (pItem->nType==Item::Combo)
					SetTextValue(pItem);
				else if (pItem->nType==Item::List)
					SetListValue(pItem);
				break;
			}
		case CBN_EDITCHANGE:
			{
				HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
				if (hItem==NULL)
					break;
				Item* pItem=(Item*)m_pTree->GetItemData(hItem);
				if (pItem==NULL)
					break;
				if (pItem->hControl!=hControl)
					break;

				if (pItem->nType==Item::Combo)
					SetTextValue(pItem);
				break;
			}
		case CBN_SETFOCUS:
			::SendMessage(hControl,CB_SETEDITSEL,0,MAKELPARAM(0,-1));
			break;
		default:
			CAppData::stdfunc();
			break;
		}
		break;
	case IDC_COLORBUTTONFORSELECTEDITEM:
		{
			HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
			if (hItem==NULL)
				break;
			Item* pItem=(Item*)m_pTree->GetItemData(hItem);
			if (pItem==NULL)
				break;
			if (pItem->hControl!=hControl)
				break;

			if (pItem->nType==Item::Color)
			{
				CColorDialog cd(pItem->cColor);
				if (cd.DoModal(*this))
				{
					SetColorValue(pItem,cd.GetColor());

					m_pTree->RedrawWindow();
				}
				break;
			}
			break;
		}
	case IDC_FONTBUTTONFORSELECTEDITEM:
		{
			HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
			if (hItem==NULL)
				break;
			Item* pItem=(Item*)m_pTree->GetItemData(hItem);
			if (pItem==NULL)
				break;
			if (pItem->hControl!=hControl)
				break;

			if (pItem->nType==Item::Font)
			{
				CFontDialog fd(pItem->pLogFont,CF_SCREENFONTS);
                
				if (fd.DoModal(*this))
				{
					SetFontValue(pItem,&fd.m_lf);

					WCHAR* pText=pItem->GetText(TRUE);
					m_pTree->SetItemText(hItem,pText);
					pItem->FreeText(pText);
				}

				break;
			}
			break;
		}
	case IDC_BROWSEBUTTONFORSELECTEDITEM:
		{
			HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
			if (hItem==NULL)
				break;
			Item* pItem=(Item*)m_pTree->GetItemData(hItem);
			if (pItem==NULL)
				break;
			
			if (pItem->nType==Item::File)
			{
				BROWSEDLGPARAMS bp;
				pItem->SetValuesForBasicParams(&bp);
				bp.crReason=BASICPARAMS::BrowseFile;
				bp.pPage=this;
				bp.szTitle=NULL;
				bp.szFilters=NULL;
				pItem->pProc(&bp);
				
				CFileDialog* pfd;
				if (IS_INTRESOURCE(bp.szFilters))
				{
					pfd=new CFileDialog(TRUE,L"*",szwEmpty,OFN_EXPLORER|OFN_HIDEREADONLY|
						OFN_NOREADONLYRETURN|OFN_ENABLESIZING,(UINT)(ULONG_PTR)(bp.szFilters));
				}
				else
				{
					pfd=new CFileDialog(TRUE,L"*",szwEmpty,OFN_EXPLORER|OFN_HIDEREADONLY|
						OFN_NOREADONLYRETURN|OFN_ENABLESIZING,bp.szFilters);
				}
				
				pfd->EnableFeatures();
				if (IS_INTRESOURCE(bp.szTitle))
					pfd->SetTitle(ID2W((UINT)(ULONG_PTR)bp.szTitle));
				else
					pfd->SetTitle(bp.szTitle);
	
				if (pfd->DoModal(*this))
				{
					WCHAR szPath[MAX_PATH];
					if (pfd->GetFilePath(szPath,MAX_PATH))
					{
						if (IsUnicodeSystem())
							::SendMessageW(pItem->hControl,WM_SETTEXT,0,(LPARAM)szPath);
						else
							::SendMessage(pItem->hControl,WM_SETTEXT,0,(LPARAM)(LPCSTR)W2A(szPath));
						::SendMessage(pItem->hControl,EM_SETSEL,0,-1);
						::SetFocus(pItem->hControl);
					}
				}


				delete pfd;
			}
			break;
		}
	}
	return FALSE;
}

BOOL COptionsPropertyPage::OnApply()
{
	CPropertyPage::OnApply();
	if (m_pItems!=NULL)
		CallApply(m_pItems);
	return TRUE;
}

void COptionsPropertyPage::CallApply(Item** pItems)
{
	COMBOAPPLYPARAMS bp;
	bp.pPage=this;
	bp.crReason=BASICPARAMS::Apply;

	for (int i=0;pItems[i]!=NULL;i++)
	{
		if (pItems[i]->bEnabled)
		{
			if (pItems[i]->pProc!=NULL)
			{
				pItems[i]->SetValuesForBasicParams(&bp);
				if (pItems[i]->nType==Item::Combo || pItems[i]->nType==Item::List)
					bp.nCurSel=(LONG)::SendMessage(pItems[i]->hControl,CB_GETCURSEL,0,0);
				pItems[i]->pProc(&bp);
			}
			if (pItems[i]->pChilds!=NULL)
				CallApply(pItems[i]->pChilds);
		}
	}
}

void COptionsPropertyPage::OnDestroy()
{
	CPropertyPage::OnDestroy();
	
	if (m_pTree!=NULL)
	{
		m_pTree->DeleteAllItems();
		delete m_pTree;
		m_pTree=NULL;
	}
	
	if (m_pItems!=NULL)
	{
		for (int i=0;m_pItems[i]!=NULL;i++)
			delete m_pItems[i];
		delete[] m_pItems;
	}

	
}

	
void COptionsPropertyPage::OnActivate(WORD fActive,BOOL fMinimized,HWND hwnd)
{
	CPropertyPage::OnActivate(fActive,fMinimized,hwnd);

	if (fActive!=WA_INACTIVE)
		PostMessage(WM_REDRAWSELITEMCONTROL);
}

/*void COptionsPropertyPage::OnTimer(DWORD wTimerID)
{
	switch (wTimerID)
	{
	case 0:
		KillTimer(0);
		PostMessage(WM_REDRAWSELITEMCONTROL);
		break;
	}
	CPropertyPage::OnTimer(wTimerID);
}*/

int COptionsPropertyPage::Item::IconFromColor(CImageList* pImageList,int nReplace) const
{
	int cx=16,cy=16;
	pImageList->GetIconSize(&cx,&cy);

	HDC hScreenDC=::GetDC(NULL);
	HDC memDC=::CreateCompatibleDC(hScreenDC);
    HBITMAP memBM=CreateCompatibleBitmap(hScreenDC,cx,cy);
    HBITMAP memBM2=CreateCompatibleBitmap(hScreenDC,cx,cy);
    
	// Creating first image
	SelectObject(memDC,memBM);
    HBRUSH hBrush=CreateSolidBrush(cColor);
	FillRect(memDC,&CRect(2,0,cx-2,cy-3),hBrush);
	DeleteObject(hBrush);
	
	// Crating second image
	SelectObject(memDC,memBM2);
    hBrush=CreateSolidBrush(RGB(255,255,255));
	FillRect(memDC,&CRect(0,0,cx,cy),hBrush);
	DeleteObject(hBrush);
	hBrush=CreateSolidBrush(RGB(0,0,0));
	FillRect(memDC,&CRect(2,0,cx-2,cy-3),hBrush);
	DeleteObject(hBrush);
	
	DeleteDC(memDC);
	
	
	int nImage=-1;
	
	if (nReplace==-1)
		nImage=pImageList->Add(memBM,memBM2);
	else
		nImage=pImageList->Replace(nReplace,memBM,memBM2)?nReplace:-1;
    
	DeleteObject(memBM);
	DeleteObject(memBM2);
	::ReleaseDC(NULL,hScreenDC);
	
	return nImage;
}
	
LRESULT COptionsPropertyPage::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
	case WM_REDRAWSELITEMCONTROL:
		{
			HTREEITEM hActiveItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
			if (hActiveItem==NULL)
				return TRUE;

			Item* pItem=(Item*)m_pTree->GetItemData(hActiveItem);
			if (pItem==NULL)
				return TRUE;

			if (pItem->hControl!=NULL)
			{
				// Checking that should position change
				CRect rcItem,rcOrig,rcTree,rcOther;
				m_pTree->GetClientRect(&rcTree);
				m_pTree->GetItemRect(hActiveItem,&rcItem,TRUE);
				BOOL bNotVisible=rcItem.top<0 || rcItem.bottom>rcTree.bottom;
				m_pTree->ClientToScreen(&rcItem);
				
				::GetWindowRect(pItem->hControl,&rcOrig);
				if (pItem->hControl2!=NULL)
				{
					::GetWindowRect(pItem->hControl2,&rcOther);
					rcOther.left-=rcOrig.left;
					rcOther.top-=rcOrig.top;
				}


				//if (rcOrig.left!=rcItem.right+1 || rcOrig.top!=rcItem.top-1)
				{
					// Moving control to correct place
					ScreenToClient(&rcItem);
					::SetWindowPos(pItem->hControl,HWND_TOP,rcItem.right+1,rcItem.top-1,0,0,
						(bNotVisible?SWP_HIDEWINDOW:SWP_SHOWWINDOW)|SWP_NOSIZE|SWP_NOACTIVATE);
					

					if (pItem->hControl2!=NULL)
					{
						if (pItem->nType==Item::Numeric)
						{
							::SetWindowPos(pItem->hControl2,HWND_TOP,rcItem.right+1+rcOther.left,rcItem.top-1+rcOther.top,0,0,
								(bNotVisible?SWP_HIDEWINDOW:SWP_SHOWWINDOW)|SWP_NOSIZE|SWP_NOACTIVATE);
							::InvalidateRect(pItem->hControl2,NULL,FALSE);
						}
						else if (pItem->nType==Item::File)
						{
							::SetWindowPos(pItem->hControl2,HWND_TOP,rcItem.right+1+rcOrig.Width(),rcItem.top-1,0,0,
								(bNotVisible?SWP_HIDEWINDOW:SWP_SHOWWINDOW)|SWP_NOSIZE|SWP_NOACTIVATE);
							::InvalidateRect(pItem->hControl2,NULL,FALSE);
						}
					}

				}

				::InvalidateRect(pItem->hControl,NULL,FALSE);
				if (pItem->hControl2!=NULL)
					::InvalidateRect(pItem->hControl2,NULL,FALSE);
			}
			break;
		}
	case WM_FOCUSSELITEMCONTROL:
		{
			HTREEITEM hActiveItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
			if (hActiveItem==NULL)
				return TRUE;

			Item* pItem=(Item*)m_pTree->GetItemData(hActiveItem);
			if (pItem==NULL)
				return TRUE;
			
			if (pItem->hControl!=NULL)
			{
				::InvalidateRect(pItem->hControl,NULL,FALSE);
				::SetFocus(pItem->hControl);
			}

			break;
		}
	}
	return CPropertyPage::WindowProc(msg,wParam,lParam);
}

BOOL COptionsPropertyPage::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	if (idCtrl==m_nTreeID)
	{
		CPropertyPage::OnNotify(idCtrl,pnmh);
		BOOL bRet=TreeNotifyHandler((NMTVDISPINFO*)pnmh);
		if (bRet)
			SetWindowLong(dwlMsgResult,bRet);
		return bRet;
	}			
	return CPropertyPage::OnNotify(idCtrl,pnmh);
}

BOOL COptionsPropertyPage::TreeNotifyHandler(NMTVDISPINFO *pTvdi)
{
	switch (pTvdi->hdr.code)
	{
	case TVN_KEYDOWN:
		if (((NMTVKEYDOWN*)pTvdi)->wVKey==VK_SPACE)
		{
			HTREEITEM hItem=m_pTree->GetNextItem(NULL,TVGN_CARET);
			if (hItem==NULL)
				break;
			Item* pItem=(Item*)m_pTree->GetItemData(hItem);
			if (pItem==NULL)
				break;
			if (pItem->bEnabled && (pItem->nType==Item::RadioBox || pItem->nType==Item::CheckBox))
				SetCheckState(hItem,pItem,Toggle);
			
		}
		break;
	case NM_CLICK:
	case NM_DBLCLK:
		{
			TVHITTESTINFO ht;
			GetCursorPos(&ht.pt);
			m_pTree->ScreenToClient(&ht.pt);
			HTREEITEM hItem=m_pTree->HitTest(&ht);
			if (hItem==NULL)
				break;
			Item* pItem=(Item*)m_pTree->GetItemData(hItem);
			if (pItem==NULL)
				break;
			if (pItem->nType==Item::RadioBox || pItem->nType==Item::CheckBox)
			{
				if (pItem->bEnabled && (ht.flags&TVHT_ONITEMSTATEICON || pTvdi->hdr.code==NM_DBLCLK))
					SetCheckState(hItem,pItem,Toggle);
			}
			break;
		}
	case TVN_SELCHANGINGA:
	case TVN_SELCHANGINGW:
		{
			NMTREEVIEWA *pNm=(NMTREEVIEWA*)pTvdi;
			// Procedure does not access to any LPTSTR elements of NMTREEVIEWW,
			// so no need for different unicode implementation

			// Checking if selection cannot be changed
			if (pNm->itemNew.lParam!=NULL)
			{
				if (!((Item*)pNm->itemNew.lParam)->bEnabled)
					return TRUE;
			}

			// Hiding control for previous item
			if (pNm->itemOld.lParam!=NULL)
			{
				Item* pItem=(Item*)pNm->itemOld.lParam;
				if (pItem->hControl!=NULL)
				{
					// Hiding window and ensuring that that part of tree is redrawn
					RECT rc;
					::GetWindowRect(pItem->hControl,&rc);
					::ShowWindow(pItem->hControl,SW_HIDE);
					m_pTree->ScreenToClient(&rc);
					::InvalidateRect(*m_pTree,&rc,FALSE);
					
					// Setting text
					WCHAR* pText=pItem->GetText(FALSE);
					if (!IsUnicodeSystem())
					{
						int iStrLen=istrlenw(pText);
						char* paText=new char [iStrLen+2];
						MemCopyWtoA(paText,pText,iStrLen+1);
						m_pTree->SetItemText(pNm->itemOld.hItem,paText);
						delete[] paText;
					
					}
					else
						m_pTree->SetItemText(pNm->itemOld.hItem,pText);
					pItem->FreeText(pText);
					
					// Deleting another control
					if (pItem->hControl2!=NULL)
					{
						::DestroyWindow(pItem->hControl2);
						pItem->hControl2=NULL;
					}

					if (pItem->nType==Item::Numeric)
					{
						// Desubclassing
						UserData* pUserData=(UserData*)::GetWindowLong(pItem->hControl,GWLP_USERDATA);
						if (pUserData!=NULL)
						{
							if (pUserData->pOldWndProc!=NULL)
								::SetWindowLongPtr(pItem->hControl,GWLP_WNDPROC,(LONG_PTR)pUserData->pOldWndProc);
							pUserData->pOldWndProc=NULL;
						}
					}
				}
			}
			// Showing control for previous item
			if (pNm->itemNew.lParam!=NULL)
			{
				Item* pItem=(Item*)pNm->itemNew.lParam;
				if (pItem->hControl!=NULL)
				{
					// Changing text
					// Setting text
					WCHAR* pText=pItem->GetText(TRUE);
					if (!IsUnicodeSystem())
					{
						int iStrLen=istrlenw(pText);
						char* paText=new char [iStrLen+2];
						MemCopyWtoA(paText,pText,iStrLen+1);
						m_pTree->SetItemText(pNm->itemNew.hItem,paText);
						delete[] paText;
					
					}
					else
						m_pTree->SetItemText(pNm->itemNew.hItem,pText);
					
					
					// Show control
					::ShowWindow(((Item*)pNm->itemNew.lParam)->hControl,SW_SHOW);
	                
					// Moving it
					RECT rc;
					m_pTree->GetItemRect(pNm->itemNew.hItem,&rc,TRUE);
					m_pTree->ClientToScreen(&rc);
					ScreenToClient(&rc);
					
					int nWidth=60; // 60 is for numeric
					if (pItem->nType==Item::Font || pItem->nType==Item::Color) 
					{
						CDC dc(this);
						HGDIOBJ hOldFont=dc.SelectObject((HFONT)SendMessage(WM_GETFONT));
						CSize sz=dc.GetTextExtent(m_ChangeText);
						nWidth=sz.cx+10;
						dc.SelectObject(hOldFont);
					}
					else if (pItem->nType!=Item::Numeric)
					{
						RECT rcTree;
						m_pTree->GetClientRect(&rcTree);
						nWidth=rcTree.right-rc.right;
						if (pItem->nType==Item::File)
							nWidth-=20;
					}
					::SetWindowPos(pItem->hControl,HWND_TOP,0,0,nWidth,20,SWP_SHOWWINDOW|SWP_NOMOVE);

							

					
					if (pItem->nType==Item::Numeric)
					{
						// Creating Up/Down control
						pItem->hControl2=CreateWindow("msctls_updown32","",
							UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_ARROWKEYS|WS_CHILDWINDOW|WS_VISIBLE|UDS_NOTHOUSANDS,
							rc.right+20,rc.top-1,10,10,*this,(HMENU)IDC_SPINCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
						::SendMessage(pItem->hControl2,UDM_SETBUDDY,WPARAM(pItem->hControl),NULL);
						
						
						// Subclassing edit control
						UserData* pUserData=(UserData*)::GetWindowLongPtr(pItem->hControl,GWLP_USERDATA);
						if (pUserData!=NULL)
						{
							pUserData->pOldWndProc=(WNDPROC)::SetWindowLongPtr(pItem->hControl,
								GWLP_WNDPROC,(LONG_PTR)EditSubClassFunc);
						}


						// Subclassing updown control
						pUserData=new UserData;
						pUserData->pDialog=this;
						pUserData->pOldWndProc=(WNDPROC)::SetWindowLongPtr(pItem->hControl2,
							GWLP_WNDPROC,(LONG_PTR)EditSubClassFunc);

						if (pUserData->pOldWndProc==NULL)
						{
							// Subclassing didn't success
							delete pUserData;
						}
						else
							::SetWindowLongPtr(pItem->hControl2,GWLP_USERDATA,(LONG_PTR)pUserData);

						if (pItem->pProc!=NULL)
						{
							SPINPOXPARAMS spb;
							spb.iLow=0;
							spb.iHigh=MAXLONG;
							pItem->SetValuesForBasicParams(&spb);
							spb.crReason=SPINPOXPARAMS::SetSpinRange;
							spb.pPage=this;
							pItem->pProc(&spb);
							::SendMessage(pItem->hControl2,UDM_SETRANGE32,spb.iLow,spb.iHigh);
						}
						else
							::SendMessage(pItem->hControl2,UDM_SETRANGE32,0,MAXLONG);

						::SetWindowPos(pItem->hControl2,HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
					}
					else if (pItem->nType==Item::File)
					{
						// Browse control
						pItem->hControl2=CreateWindow("BUTTON","...",
							BS_PUSHBUTTON|WS_TABSTOP|WS_CHILDWINDOW|WS_VISIBLE,
							rc.right+20,rc.top-1,20,21,*this,(HMENU)IDC_BROWSEBUTTONFORSELECTEDITEM,GetInstanceHandle(),NULL);
						::SendMessage(pItem->hControl2,WM_SETFONT,SendMessage(WM_GETFONT),0);
						
						::SetWindowPos(pItem->hControl2,HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);

						// Subclassing control
						UserData* pUserData=new UserData;
						pUserData->pDialog=this;
						pUserData->pOldWndProc=(WNDPROC)::SetWindowLongPtr(pItem->hControl2,
							GWLP_WNDPROC,(LONG_PTR)EditSubClassFunc);

						if (pUserData->pOldWndProc==NULL)
						{
							// Subclassing didn't success
							delete pUserData;
						}
						else
							::SetWindowLongPtr(pItem->hControl2,GWLP_USERDATA,(LONG_PTR)pUserData);
					}

					if (pItem->nType!=Item::Color && pItem->nType!=Item::Font)
						PostMessage(WM_FOCUSSELITEMCONTROL);
				}
			}
			break;
		}
	case TVN_SELCHANGEDA:
	case TVN_SELCHANGEDW:
		/*if (pNm->itemNew.lParam!=NULL)
		{
			if (((Item*)pNm->itemNew.lParam)->hControl!=NULL)
				::SetFocus(((Item*)pNm->itemNew.lParam)->hControl);
		}*/
		break;
	case TVN_ITEMEXPANDINGA:
	case TVN_ITEMEXPANDINGW:
		if (((NMTREEVIEWA*)pTvdi)->action==TVE_COLLAPSE || ((NMTREEVIEWA*)pTvdi)->action==TVE_TOGGLE)
			return TRUE;
		return FALSE;
	case TVN_DELETEITEMA:
	case TVN_DELETEITEMW:
		if (((NMTREEVIEW*)pTvdi)->itemOld.lParam!=NULL)
		{
			Item* pItem=(Item*)((NMTREEVIEW*)pTvdi)->itemOld.lParam;
			if (pItem->hControl2!=NULL)
			{
				::DestroyWindow(pItem->hControl2);
				pItem->hControl2=NULL;
			}

			if (pItem->hControl!=NULL)
			{
				if (pItem->nType==Item::Numeric)
				{
					UserData* pData=(UserData*)::GetWindowLong(pItem->hControl,GWLP_USERDATA);
					if (pData!=NULL)
					{
						::SetWindowLongPtr(pItem->hControl,GWLP_USERDATA,NULL);
						if (pData->pOldWndProc!=NULL)
							::SetWindowLongPtr(pItem->hControl,GWLP_WNDPROC,(LONG_PTR)pData->pOldWndProc);

						delete pData;
					}
					
				}
				::DestroyWindow(pItem->hControl);

				pItem->hControl=NULL;
			}
		}
		break;
	case NM_CUSTOMDRAW:
		{
			NMTVCUSTOMDRAW* pCustomDraw=(NMTVCUSTOMDRAW*)pTvdi;
			if (pCustomDraw->nmcd.dwDrawStage==CDDS_PREPAINT)
				return CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
			else if (pCustomDraw->nmcd.dwDrawStage==CDDS_POSTPAINT)
			{
				PostMessage(WM_REDRAWSELITEMCONTROL);
				return CDRF_NOTIFYITEMDRAW;
			}
			else if (pCustomDraw->nmcd.dwDrawStage&CDDS_ITEMPREPAINT)
			{
				Item* pItem=(Item*)pCustomDraw->nmcd.lItemlParam;
				if (!pItem->bEnabled)
					pCustomDraw->clrText=GetSysColor(COLOR_GRAYTEXT);
				return CDRF_DODEFAULT;
			}
			break;
		}
	}
	return FALSE;
}


BOOL COptionsPropertyPage::SetCheckState(HTREEITEM hItem,COptionsPropertyPage::Item* pItem,
										 COptionsPropertyPage::NewState nNewState)
{
	if (nNewState==Toggle && pItem->nType==Item::RadioBox)
		nNewState=Checked;

	if (pItem->pProc!=NULL)
	{
		CHANGINGVALPARAMS cp;
		cp.crReason=BASICPARAMS::ChangingValue;
		cp.pPage=this;
		pItem->SetValuesForBasicParams(&cp);
		cp.nNewState=nNewState;
		if (!pItem->pProc(&cp))
			return FALSE;
	}

	if (pItem->nType==Item::CheckBox || pItem->nType==Item::RadioBox)
	{
		if (nNewState==Toggle)
		    pItem->bChecked=!pItem->bChecked;
		else if (nNewState==Checked)
		{
			if (pItem->bChecked)
				return FALSE;
			pItem->bChecked=TRUE;
		}
		else
		{
			if (!pItem->bChecked)
				return FALSE;
			pItem->bChecked=FALSE;
		}
		m_pTree->SetItemState(hItem,INDEXTOSTATEIMAGEMASK(pItem->GetStateImage(&m_Images)),TVIS_STATEIMAGEMASK);

		if (pItem->nType==Item::RadioBox && pItem->bChecked)
			UncheckOtherRadioButtons(hItem,m_pTree->GetParentItem(hItem));
		
		EnableChilds(hItem,pItem->bChecked);
		
		
		if (pItem->pProc!=NULL)
		{
			BASICPARAMS bp;
			bp.crReason=BASICPARAMS::Set;
			bp.pPage=this;
			pItem->SetValuesForBasicParams(&bp);
			pItem->pProc(&bp);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL COptionsPropertyPage::SetNumericValue(Item* pItem)
{
	int iTextLen=(int)::SendMessage(pItem->hControl,WM_GETTEXTLENGTH,0,0)+1;
	char* szText=new char[iTextLen+1];
	::SendMessage(pItem->hControl,WM_GETTEXT,iTextLen,LPARAM(szText));

	CHANGINGVALPARAMS cp;
	cp.lNewValue=atol(szText);
	delete[] szText;

	// Asking wheter value can be changed
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::ChangingValue;
		pItem->SetValuesForBasicParams(&cp);
		cp.pPage=this;
		if (!pItem->pProc(&cp))
			return FALSE;
	}
	pItem->lValue=cp.lNewValue;
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::Set;
		cp.lValue=pItem->lValue;
		pItem->pProc(&cp);
	}
	return TRUE;
}

BOOL COptionsPropertyPage::SetTextValue(Item* pItem)
{
	

	CHANGINGVALPARAMS cp;
	int iTextLen,iCurSel;
	switch (pItem->nType)
	{
	case Item::Combo:
	case Item::List:
		iCurSel=(int)::SendMessage(pItem->hControl,CB_GETCURSEL,0,0);
		if (iCurSel!=CB_ERR)
		{
			iTextLen=(int)::SendMessage(pItem->hControl,CB_GETLBTEXTLEN,iCurSel,0)+2;			
			cp.pNewData=new WCHAR[iTextLen];
				
			if (IsUnicodeSystem())
				::SendMessageW(pItem->hControl,CB_GETLBTEXT,iCurSel,LPARAM(cp.pNewData));
			else
			{
				char* pAText=new char[iTextLen];
				::SendMessage(pItem->hControl,CB_GETLBTEXT,iCurSel,LPARAM(pAText));
				MultiByteToWideChar(CP_ACP,0,pAText,-1,cp.pNewData,iTextLen);
				delete[] pAText;
			}			
		}
		else
		{
			iTextLen=(int)::SendMessage(pItem->hControl,WM_GETTEXTLENGTH,iCurSel,0)+2;			
			cp.pNewData=new WCHAR[iTextLen];
			
			
			if (IsUnicodeSystem())
				::SendMessageW(pItem->hControl,WM_GETTEXT,iTextLen,LPARAM(cp.pNewData));
			else
			{
				char* pAText=new char[iTextLen];
				::SendMessage(pItem->hControl,WM_GETTEXT,iTextLen,LPARAM(pAText));
				MultiByteToWideChar(CP_ACP,0,pAText,-1,cp.pNewData,iTextLen);
				delete[] pAText;
			}		
			
			
		}

		break;
	default:
		iTextLen=(int)::SendMessage(pItem->hControl,WM_GETTEXTLENGTH,0,0)+1;
		cp.pNewData=new WCHAR[max(iTextLen,2)];
		if (IsUnicodeSystem())
			::SendMessageW(pItem->hControl,WM_GETTEXT,iTextLen,LPARAM(cp.pNewData));
		else
		{
			char* pAText=new char[iTextLen];
			::SendMessage(pItem->hControl,WM_GETTEXT,iTextLen,LPARAM(pAText));
			MultiByteToWideChar(CP_ACP,0,pAText,-1,cp.pNewData,iTextLen);
			delete[] pAText;
		}	
		
		break;
	}
	
	

	// Asking wheter value can be changed
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::ChangingValue;
		pItem->SetValuesForBasicParams(&cp);
		cp.pPage=this;
		if (!pItem->pProc(&cp))
		{
			delete[] cp.pNewData;
			return FALSE;
		}
	}

	
	if (pItem->pData!=NULL)
		delete[] pItem->pData;

	
	pItem->pData=cp.pNewData;
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::Set;
		cp.pData=cp.pData;
		pItem->pProc(&cp);
	}
	
	return TRUE;
}

BOOL COptionsPropertyPage::SetListValue(Item* pItem)
{
	CHANGINGVALPARAMS cp;
	cp.lNewValue=(LONG)::SendMessage(pItem->hControl,CB_GETCURSEL,0,0);
	
	// Asking wheter value can be changed
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::ChangingValue;
		pItem->SetValuesForBasicParams(&cp);
		cp.pPage=this;
		if (!pItem->pProc(&cp))
			return FALSE;
	}
	pItem->lValue=cp.lNewValue;
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::Set;
		cp.lValue=pItem->lValue;
		pItem->pProc(&cp);
	}
	return TRUE;
}

BOOL COptionsPropertyPage::SetColorValue(Item* pItem,COLORREF cNewColor)
{
	CHANGINGVALPARAMS cp;
	cp.cNewColor=cNewColor;
	
	// Asking wheter value can be changed
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::ChangingValue;
		pItem->SetValuesForBasicParams(&cp);
		cp.pPage=this;
		if (!pItem->pProc(&cp))
			return FALSE;
	}
	pItem->cColor=cp.cNewColor;
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::Set;
		cp.cColor=pItem->cColor;
		pItem->pProc(&cp);
	}

	pItem->m_nStateIcon=pItem->IconFromColor(&m_Images,pItem->m_nStateIcon);
	return TRUE;
}

BOOL COptionsPropertyPage::SetFontValue(Item* pItem,LOGFONT* pLogFont)
{
	CHANGINGVALPARAMS cp;
	cp.pNewLogFont=pLogFont;
	
	// Asking wheter value can be changed
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::ChangingValue;
		pItem->SetValuesForBasicParams(&cp);
		cp.pPage=this;
		if (!pItem->pProc(&cp))
			return FALSE;
	}
	if (pItem->pLogFont==NULL)
		pItem->pLogFont=new LOGFONT;
    CopyMemory(pItem->pLogFont,cp.pNewLogFont,sizeof(LOGFONT));
	
	if (pItem->pProc!=NULL)
	{
		cp.crReason=BASICPARAMS::Set;
		cp.pLogFont=pItem->pLogFont;
		pItem->pProc(&cp);
	}
	return TRUE;
}
	
	
void COptionsPropertyPage::EnableChilds(HTREEITEM hItem,BOOL bEnable)
{
	HTREEITEM hChildItem=m_pTree->GetNextItem(hItem,TVGN_CHILD);
    while (hChildItem!=NULL)
	{
		Item* pItem=(Item*)m_pTree->GetItemData(hChildItem);
		if (pItem!=NULL)
			pItem->bEnabled=bEnable;
		
		m_pTree->SetItemState(hChildItem,bEnable?0:TVIS_CUT,TVIS_CUT);
		
		hChildItem=m_pTree->GetNextItem(hChildItem,TVGN_NEXT);
	}
}

void COptionsPropertyPage::UncheckOtherRadioButtons(HTREEITEM hItem,HTREEITEM hParent)
{
	if (hParent==NULL)
		return;
	
	HTREEITEM hChilds;
	if (hParent==NULL)
		hChilds=m_pTree->GetNextItem(NULL,TVGN_ROOT);
	else
		hChilds=m_pTree->GetNextItem(hParent,TVGN_CHILD);



	while (hChilds!=NULL)
	{
		Item* pItem=(Item*)m_pTree->GetItemData(hChilds);
		if (pItem!=NULL)
		{
			if (pItem->nType==Item::RadioBox && hChilds!=hItem)
				SetCheckState(hChilds,(Item*)m_pTree->GetItemData(hChilds),Unchecked);
		}
		hChilds=m_pTree->GetNextItem(hChilds,TVGN_NEXT);
	}
}

WCHAR* COptionsPropertyPage::Item::GetText(BOOL bActive) const
{
	switch (nType)
	{
	case Numeric:
		if (hControl!=NULL && !bActive)
		{
			WCHAR szText[100];
			_itow_s(lValue,szText,100,10);
			int iLength=(int)istrlenw(szText)+1;
			int iLabelLen=(int)istrlenw(pString);
			
			WCHAR* pText=new WCHAR[iLabelLen+iLength+2];
			MemCopyW(pText,pString,iLabelLen);
			pText[iLabelLen++]=' ';
			MemCopyW(pText+iLabelLen,szText,iLength);
			return pText;
		}
		return pString;
	case List:
    case Combo:
		if (hControl!=NULL && !bActive)
		{
			
			CComboBox cb(hControl);

			int nCurSel=cb.GetCurSel();
			int iLength=(int)((nCurSel!=-1)?cb.GetLBTextLen(nCurSel)+1:cb.GetTextLength()+1);
			int iLabelLen=(int)istrlenw(pString);
			
			WCHAR* pText=new WCHAR[iLabelLen+iLength+2];
			MemCopyW(pText,pString,iLabelLen);
			pText[iLabelLen++]=' ';
				
			if (nCurSel!=-1)
				cb.GetLBText(nCurSel,pText+iLabelLen);
			else
				cb.GetWindowText(pText+iLabelLen,iLength+1);

			return pText;
		}
		return pString;
	case Edit:
	case File:
		if (hControl!=NULL && !bActive)
		{
			
			int iLength=(int)::SendMessage(hControl,WM_GETTEXTLENGTH,0,0)+1;
			int iLabelLen=(int)istrlenw(pString);
			
			WCHAR* pText=new WCHAR[iLabelLen+iLength+1];
			MemCopyW(pText,pString,iLabelLen);
			pText[iLabelLen++]=' ';
				
			if (!IsUnicodeSystem())
			{
				// 9x
				char* pTemp=new char[iLength+2];
				::GetWindowText(hControl,pTemp,iLength);
				MemCopyAtoW(pText+iLabelLen,pTemp,iLength);
				delete[] pTemp;
			}
			else
				::GetWindowTextW(hControl,pText+iLabelLen,iLength);
				
			return pText;
		}
		return pString;
	case Font:
		if (pLogFont!=NULL)
		{
			
			CStringW str(pString);
			str << L' ' << pLogFont->lfFaceName;

			if (pLogFont->lfHeight<0)
			{
				// Getting device caps
				HDC hScreenDC=::GetDC(NULL);
				int pt=MulDiv(-pLogFont->lfHeight, 72,::GetDeviceCaps(hScreenDC,LOGPIXELSY));
				::ReleaseDC(NULL,hScreenDC);
				
				str << L' ' << pt;
			}

			
			return str.GiveBuffer();
		}
		return pString;
	case RadioBox:
	case CheckBox:
	case Root:
	case Color:
	default:
		return pString;
	}
}

// lParam is pointer to DWORD value which is will be set
// wParam is used mask
BOOL CALLBACK COptionsPropertyPage::DefaultCheckBoxProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch (pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		pParams->bChecked=(*((DWORD*)pParams->lParam))&pParams->wParam?TRUE:FALSE;
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		if (pParams->bChecked)
			*((DWORD*)pParams->lParam)|=DWORD(pParams->wParam);
		else
			*((DWORD*)pParams->lParam)&=~DWORD(pParams->wParam);
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}
	return TRUE;
}

// lParam is pointer to DWORD value which is will be set
// HIWORD of wParam is mask to be setted, LOWORD is value
BOOL CALLBACK COptionsPropertyPage::DefaultRadioBoxProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch (pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		if (pParams->lParam!=NULL)
			pParams->bChecked=((*((DWORD*)pParams->lParam))&(HIWORD(pParams->wParam)))==LOWORD(pParams->wParam);
		else
			pParams->bChecked=0;
		break;
	case BASICPARAMS::Set:
		/*
		*((DWORD*)pParams->lParam)&=~HIWORD(pParams->wParam);
		if (pParams->bChecked)
			*((DWORD*)pParams->lParam)|=LOWORD(pParams->wParam);
		*/
		break;
	case BASICPARAMS::Apply:
		if (pParams->bChecked && pParams->lParam!=NULL)
		{
			*((DWORD*)pParams->lParam)&=~HIWORD(pParams->wParam);
			*((DWORD*)pParams->lParam)|=LOWORD(pParams->wParam);
		}
		break;		
	case BASICPARAMS::ChangingValue:
		break;
	}
	return TRUE;
}

// lParam is pointer to DWORD value which is will be set
// HIWORD of wParam is mask (shifted 16 bits) to be setted, LOWORD is value (shifted 16 bit)
BOOL CALLBACK COptionsPropertyPage::DefaultRadioBoxShiftProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch (pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		pParams->bChecked=((*((DWORD*)pParams->lParam))&(HIWORD(pParams->wParam)<<16))==LOWORD(pParams->wParam)<<16;
		break;
	case BASICPARAMS::Set:
		/*
		*((DWORD*)pParams->lParam)&=~HIWORD(pParams->wParam);
		if (pParams->bChecked)
			*((DWORD*)pParams->lParam)|=LOWORD(pParams->wParam);
		*/
		break;
	case BASICPARAMS::Apply:
		if (pParams->bChecked)
		{
			*((DWORD*)pParams->lParam)&=~(HIWORD(pParams->wParam)<<16);
			*((DWORD*)pParams->lParam)|=LOWORD(pParams->wParam)<<16;
		}
		break;		
	case BASICPARAMS::ChangingValue:
		break;
	}
	return TRUE;
}

// lParam is pointer to DWORD value which is will be set
// if wParam==0, all values are accepted
// if wParam==-1, positive values are accepted
// otherwise HIWORD is maximum, LOWORD is minimum
BOOL CALLBACK COptionsPropertyPage::DefaultNumericProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch (pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		pParams->lValue=*((DWORD*)pParams->lParam);
		if (pParams->wParam==0) 
			break; // Accept all values
		else if (pParams->wParam==DWORD(-1))
		{
			// -1: Accept only nonnegative values
			if (pParams->lValue<0)
				pParams->lValue=0;
		}
		else if (pParams->lValue>int(HIWORD(pParams->wParam)))
			pParams->lValue=int(HIWORD(pParams->wParam));
		else if (pParams->lValue<int(LOWORD(pParams->wParam)))
			pParams->lValue=int(LOWORD(pParams->wParam));
		break;
	case BASICPARAMS::Set:
		/*
		*((DWORD*)pParams->lParam)&=~HIWORD(pParams->wParam);
		if (pParams->bChecked)
			*((DWORD*)pParams->lParam)|=LOWORD(pParams->wParam);
		*/
		break;
	case BASICPARAMS::Apply:
		*((DWORD*)pParams->lParam)=pParams->lValue;
		break;		
	case BASICPARAMS::SetSpinRange:
		if (pParams->wParam==0)
		{
			((SPINPOXPARAMS*)pParams)->iLow=MINLONG;
			((SPINPOXPARAMS*)pParams)->iHigh=MAXLONG;
		}
		else if (pParams->wParam==DWORD(-1))
		{
			((SPINPOXPARAMS*)pParams)->iLow=0;
			((SPINPOXPARAMS*)pParams)->iHigh=MAXLONG;
		}
		else
		{
			((SPINPOXPARAMS*)pParams)->iLow=LOWORD(pParams->wParam);
			((SPINPOXPARAMS*)pParams)->iHigh=HIWORD(pParams->wParam);
		}
		break;
	case BASICPARAMS::ChangingValue:
		if (pParams->wParam==0) // 
			break;
		else if (pParams->wParam==DWORD(-1))
		{
			if (((CHANGINGVALPARAMS*)pParams)->lNewValue<0)
				return FALSE;
		}
		else if (((CHANGINGVALPARAMS*)pParams)->lNewValue<int(LOWORD(pParams->wParam)) || 
			((CHANGINGVALPARAMS*)pParams)->lNewValue>int(HIWORD(pParams->wParam)))
			return FALSE;
		break;
	}
	return TRUE;
}

// lParam is pointer to string class which will be set
BOOL CALLBACK COptionsPropertyPage::DefaultEditStrProc(BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		ASSERT(pParams->pData==NULL);
		pParams->pData=alloccopyAtoW(*(CString*)pParams->lParam);
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		if (pParams->pData==NULL)
			((CString*)pParams->lParam)->Empty();
		else
			((CString*)pParams->lParam)->Copy(pParams->pData);
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}		
	return TRUE;
}

// lParam is pointer to CStringW which will be set
BOOL CALLBACK COptionsPropertyPage::DefaultEditStrWProc(BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		ASSERT(pParams->pData==NULL);
		pParams->pData=alloccopy(*(CStringW*)pParams->lParam);
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		if (pParams->pData==NULL)
			((CStringW*)pParams->lParam)->Empty();
		else
			((CStringW*)pParams->lParam)->Copy(pParams->pData);
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}		
	return TRUE;
}

// lParam is pointer to LOGFONT strcut
BOOL CALLBACK COptionsPropertyPage::DefaultFontProc(BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		if (pParams->pLogFont==NULL)
			pParams->pLogFont=new LOGFONT;
		CopyMemory(pParams->pLogFont,pParams->lParam,sizeof(LOGFONT));
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		CopyMemory(pParams->lParam,pParams->pLogFont,sizeof(LOGFONT));	
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}		
	return TRUE;
}

// lParam is pointer to COLORREF
BOOL CALLBACK COptionsPropertyPage::DefaultColorProc(BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		pParams->cColor=*((COLORREF*)pParams->lParam);
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		*((COLORREF*)pParams->lParam)=pParams->cColor;
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}		
	return TRUE;
}


LRESULT CALLBACK COptionsPropertyPage::TreeSubClassFunc(HWND hWnd,UINT uMsg,
															  WPARAM wParam,LPARAM lParam)
{
	UserData* pUserData=(UserData*)::GetWindowLongPtr(hWnd,GWLP_USERDATA);
	LRESULT lRet=FALSE;

	if (pUserData==NULL)
		return FALSE;

	
	switch (uMsg)
	{
	case WM_DESTROY:
		// Calling original window procedure
		lRet=CallWindowProc(pUserData->pOldWndProc,hWnd,uMsg,wParam,lParam);

		// Desubclassing
		::SetWindowLongPtr(hWnd,GWLP_WNDPROC,(LONG_PTR)pUserData->pOldWndProc);
		
		// Free memory
		delete pUserData;
		return lRet;
	case WM_KEYDOWN:
		if ((wParam==VK_DOWN || wParam==VK_UP) &&
			!(GetKeyState(VK_CONTROL)&0x8000))
		{
			HTREEITEM hCurrentItem=TreeView_GetSelection(hWnd);
			if (hCurrentItem!=NULL)
			{
				HTREEITEM hNextItem;
				
				for (;;)
				{
					if (wParam==VK_DOWN)
						hNextItem=TreeView_GetNextVisible(hWnd,hCurrentItem);
					else
						hNextItem=TreeView_GetPrevVisible(hWnd,hCurrentItem);

					if (hNextItem==NULL)
						break;

					TVITEM tv;
					tv.hItem=hNextItem;
					tv.mask=TVIF_HANDLE|TVIF_PARAM;
					TreeView_GetItem(hWnd,&tv);					

					if (((Item*)tv.lParam)->bEnabled)
						break;

					hCurrentItem=hNextItem;
				}
				
				if (hNextItem!=NULL)
				{
					::SetFocus(hWnd);
					TreeView_SelectItem(hWnd,hNextItem);
				}
				break;
			}			
		}
		// Continue to default
	default:
		lRet=CallWindowProc(pUserData->pOldWndProc,hWnd,uMsg,wParam,lParam);
		break;
	}

	return lRet;
}

LRESULT CALLBACK COptionsPropertyPage::ButtonSubClassFunc(HWND hWnd,UINT uMsg,
															  WPARAM wParam,LPARAM lParam)
{
	

	UserData* pUserData=(UserData*)::GetWindowLongPtr(hWnd,GWLP_USERDATA);
	LRESULT lRet=FALSE;

	if (pUserData==NULL)
		return FALSE;

	
	switch (uMsg)
	{
	case WM_DESTROY:
		// Calling original window procedure
		lRet=CallWindowProc(pUserData->pOldWndProc,hWnd,uMsg,wParam,lParam);

		// Desubclassing
		::SetWindowLongPtr(hWnd,GWLP_WNDPROC,(LONG_PTR)pUserData->pOldWndProc);
		
		// Free memory
		delete pUserData;
		return lRet;
	case WM_KEYDOWN:
		if ((wParam==VK_DOWN || wParam==VK_UP) &&
			!(GetKeyState(VK_CONTROL)&0x8000))
			pUserData->pDialog->m_pTree->SendMessage(WM_KEYDOWN,wParam,lParam);

		// Continue to default
	default:
		lRet=CallWindowProc(pUserData->pOldWndProc,hWnd,uMsg,wParam,lParam);
		break;
	}

	return lRet;
}

LRESULT CALLBACK COptionsPropertyPage::EditSubClassFunc(HWND hWnd,UINT uMsg,
															  WPARAM wParam,LPARAM lParam)
{
	UserData* pUserData=(UserData*)::GetWindowLongPtr(hWnd,GWLP_USERDATA);
	LRESULT lRet=FALSE;

	if (pUserData==NULL)
		return FALSE;

	
	switch (uMsg)
	{
	case WM_DESTROY:
		// Calling original window procedure
		lRet=CallWindowProc(pUserData->pOldWndProc,hWnd,uMsg,wParam,lParam);

		// Desubclassing
		::SetWindowLongPtr(hWnd,GWLP_WNDPROC,(LONG_PTR)pUserData->pOldWndProc);
		
		// Free memory
		delete pUserData;
		return lRet;
	case WM_KEYDOWN:
		if (wParam==VK_DOWN || wParam==VK_UP)
			return pUserData->pDialog->m_pTree->SendMessage(WM_KEYDOWN,wParam,lParam);


		// Continue to default
	default:
		lRet=CallWindowProc(pUserData->pOldWndProc,hWnd,uMsg,wParam,lParam);
		break;
	}

	return lRet;
}

LRESULT CALLBACK COptionsPropertyPage::ComboSubClassFunc(HWND hWnd,UINT uMsg,
															  WPARAM wParam,LPARAM lParam)
{
	UserData* pUserData=(UserData*)::GetWindowLongPtr(hWnd,GWLP_USERDATA);
	LRESULT lRet=FALSE;

	if (pUserData==NULL)
		return FALSE;

	
	switch (uMsg)
	{
	case WM_DESTROY:
		// Calling original window procedure
		lRet=CallWindowProc(pUserData->pOldWndProc,hWnd,uMsg,wParam,lParam);

		// Desubclassing
		::SetWindowLongPtr(hWnd,GWLP_WNDPROC,(LONG_PTR)pUserData->pOldWndProc);
		
		// Free memory
		delete pUserData;
		return lRet;
	case WM_KEYDOWN:
		if ((wParam==VK_DOWN || wParam==VK_UP) && 
			!(GetKeyState(VK_MENU)&0x8000))
		{
			if (GetWindow(hWnd,GW_CHILD)==NULL)
			{
				// Edit control
				if (!::SendMessage(::GetParent(hWnd),CB_GETDROPPEDSTATE,0,0))	
					return pUserData->pDialog->m_pTree->SendMessage(WM_KEYDOWN,wParam,lParam);
			}
			else if (!::SendMessage(hWnd,CB_GETDROPPEDSTATE,0,0))	
				return pUserData->pDialog->m_pTree->SendMessage(WM_KEYDOWN,wParam,lParam);
		}

		// Continue to default
	default:
		lRet=CallWindowProc(pUserData->pOldWndProc,hWnd,uMsg,wParam,lParam);
		break;
	}

	return lRet;
}

////////////////////////////////////////
// CSettingsProperties
////////////////////////////////////////

CSettingsProperties::CSettingsProperties(HWND hParent)
:	CPropertySheet(IDS_SETTINGS,hParent,0),
	m_nMaximumFoundFiles(0),m_nUpdateThreadPriority(THREAD_PRIORITY_NORMAL),
	m_dwLocateDialogFlags(CLocateDlg::fgDefault),
	m_dwLocateDialogExtraFlags(CLocateDlg::efDefault),
	m_bDefaultFlag(defaultDefault),m_bSorting(BYTE(-1)),
	m_dwSettingsFlags(settingsDefault),
	m_nNumberOfDirectories(DEFAULT_NUMBEROFDIRECTORIES),
	m_nNumberOfNames(DEFAULT_NUMBEROFNAMES),
	m_nNumberOfTypes(DEFAULT_NUMBEROFTYPES),
	m_nTransparency(0),m_nToolTipTransparency(0),
	m_dwTooltipDelayAutopop(DWORD(-1)),
	m_dwTooltipDelayInitial(DWORD(-1))
{
	AddFlags(PSH_NOAPPLYNOW|PSH_NOCONTEXTHELP);

	m_pGeneral=new CGeneralSettingsPage;
	m_pAdvanced=new CAdvancedSettingsPage;
	m_pLanguage=new CLanguageSettingsPage;
	m_pDatabases=new CDatabasesSettingsPage;
	m_pAutoUpdate=new CAutoUpdateSettingsPage;
	m_pKeyboardShortcuts=new CKeyboardShortcutsPage;
	
	
	AddPage((CPropertyPage*)m_pGeneral);
	AddPage((CPropertyPage*)m_pAdvanced);
	AddPage((CPropertyPage*)m_pLanguage);
	AddPage((CPropertyPage*)m_pDatabases);
	if (GetLocateApp()->m_nInstance==0)
		AddPage((CPropertyPage*)m_pAutoUpdate);
	AddPage((CPropertyPage*)m_pKeyboardShortcuts);
	
	
	m_pGeneral->m_pSettings=m_pAdvanced->m_pSettings=this;
	m_pLanguage->m_pSettings=m_pAutoUpdate->m_pSettings=this;
	m_pDatabases->m_pSettings=m_pKeyboardShortcuts->m_pSettings=this;



	
	int nDeviceCaps;
	{
		// Getting device caps
		HDC hScreenDC=::GetDC(NULL);
		nDeviceCaps=::GetDeviceCaps(hScreenDC,LOGPIXELSY);
		::ReleaseDC(NULL,hScreenDC);
	}
	ZeroMemory(&m_lResultListFont,sizeof(LOGFONT));
	m_lResultListFont.lfHeight=-MulDiv(9, nDeviceCaps, 72);
	m_lResultListFont.lfWeight=FW_NORMAL;
	StringCbCopy(m_lResultListFont.lfFaceName,LF_FACESIZE,"Tahoma");
	

	CLocateAppWnd::CUpdateStatusWnd::FillFontStructs(&m_lToolTipTextFont,&m_lToolTipTitleFont);



	m_cToolTipBackColor=GetSysColor(COLOR_INFOBK);
	m_cToolTipTextColor=GetSysColor(COLOR_INFOTEXT);
	m_cToolTipTitleColor=GetSysColor(COLOR_INFOTEXT);
	m_cToolTipErrorColor=GetSysColor(COLOR_INFOTEXT);


}

CSettingsProperties::~CSettingsProperties()
{
	delete m_pGeneral;
	delete m_pAdvanced;
	delete m_pLanguage;
	delete m_pDatabases;
	delete m_pAutoUpdate;
	delete m_pKeyboardShortcuts;
	
	m_Schedules.RemoveAll();

	// Free buffers
	if (g_szBuffer!=NULL)
	{
		delete[] g_szBuffer;
		g_szBuffer=NULL;
	}
	if (g_szwBuffer!=NULL)
	{
		delete[] g_szwBuffer;
		g_szwBuffer=NULL;
	}
}

BOOL CSettingsProperties::LoadSettings()
{
	DebugMessage("CSettingsProperties::LoadSettings()");
	
	CRegKey GenRegKey;
	CRegKey2 LocRegKey;
	
	m_DateFormat=((CLocateApp*)GetApp())->m_strDateFormat;
	m_TimeFormat=((CLocateApp*)GetApp())->m_strTimeFormat;
	m_nFileSizeFormat=((CLocateApp*)GetApp())->m_nFileSizeFormat;
	
	// GetLocateAppWnd() is alwyas present
	m_dwProgramFlags=CLocateApp::GetProgramFlags();

	if (GetLocateDlg()!=NULL)
	{
		m_dwLocateDialogFlags=GetLocateDlg()->GetFlags();
		m_dwLocateDialogExtraFlags=GetLocateDlg()->GetExtraFlags();
	}
	else if (LocRegKey.OpenKey(HKCU,"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD temp=m_dwLocateDialogFlags;
		LocRegKey.QueryValue("Program Status",temp);
		m_dwLocateDialogFlags&=~CLocateDlg::fgSave;
		m_dwLocateDialogFlags|=temp&CLocateDlg::fgSave;

		temp=m_dwLocateDialogExtraFlags;
		LocRegKey.QueryValue("Program StatusExtra",temp);
		m_dwLocateDialogExtraFlags&=~CLocateDlg::efSave;
		m_dwLocateDialogExtraFlags|=temp&CLocateDlg::efSave;


	}
	if (LocRegKey.OpenKey(HKCU,"\\Recent Strings",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		LocRegKey.QueryValue("NumberOfDirectories",m_nNumberOfDirectories);
		LocRegKey.QueryValue("NumberOfNames",m_nNumberOfNames);
		LocRegKey.QueryValue("NumberOfTypes",m_nNumberOfTypes);
	}

	// Initializing values
	if (GetLocateDlg()==NULL)
	{
		if (LocRegKey.OpenKey(HKCU,"\\Locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
			LocRegKey.QueryValue("MaximumFoundFiles",m_nMaximumFoundFiles);
	}
	else
		m_nMaximumFoundFiles=GetLocateDlg()->GetMaxFoundFiles();
    	
	
	// Retrieving databases
	m_aDatabases.RemoveAll();
	const CArray<PDATABASE>& rOrigDatabases=GetLocateApp()->GetDatabases();
	for (int i=0;i<rOrigDatabases.GetSize();i++)
		m_aDatabases.Add(new CDatabase(*rOrigDatabases[i]));
	
	SetSettingsFlags(settingsDatabasesOverridden,
		((CLocateApp*)GetApp())->GetStartupFlags()&CLocateApp::CStartData::startupDatabasesOverridden);
	
	
	// Loading shortcuts
	m_aShortcuts.RemoveAll();
	if (!CShortcut::LoadShortcuts(m_aShortcuts))
	{
        m_aShortcuts.RemoveAll();
		if (!CShortcut::GetDefaultShortcuts(m_aShortcuts))
		{
			ShowErrorMessage(IDS_ERRORCANNOTLOADDEFAULTSHORTUCS,IDS_ERROR);
			m_aShortcuts.RemoveAll();
		}		
	}

	
	if (GetLocateApp()->m_nInstance==0)
	{
		// Loading schedules
		POSITION pPos=GetLocateAppWnd()->m_Schedules.GetHeadPosition();
		while (pPos!=NULL)
		{
			m_Schedules.AddTail(new CSchedule(GetLocateAppWnd()->m_Schedules.GetAt(pPos)));
			pPos=GetLocateAppWnd()->m_Schedules.GetNextPosition(pPos);
		}	
	}

	// Loading some general settings
	if (LocRegKey.OpenKey(HKCU,"\\General",CRegKey::defRead)==ERROR_SUCCESS)
	{
		m_bDefaultFlag=0;
		DWORD nTemp=1;
		LocRegKey.QueryValue("Default CheckIn",nTemp);
		m_bDefaultFlag|=nTemp;
		SendDlgItemMessage(IDC_CHECKIN,CB_SETCURSEL,nTemp);
		
		nTemp=0;
		LocRegKey.QueryValue("Default MatchWholeName",nTemp);
		if (nTemp) m_bDefaultFlag|=defaultWholeName;
		
		nTemp=1;
		LocRegKey.QueryValue("Default DataMatchCase",nTemp);
		if (nTemp) m_bDefaultFlag|=defaultMatchCase;
		
		nTemp=0;
		LocRegKey.QueryValue("Default ReplaceSpaces",nTemp);
		if (nTemp) m_bDefaultFlag|=defaultReplaceSpaces;

		nTemp=0;
		LocRegKey.QueryValue("Default UseWholePath",nTemp);
		if (nTemp) m_bDefaultFlag|=defaultUseWholePath;


		if (LocRegKey.QueryValue("Default Sorting",nTemp))
			m_bSorting=(BYTE)nTemp;		


		// Overrinding explorer for opening folders
		LocRegKey.QueryValue("Use other program to open folders",nTemp);
		SetSettingsFlags(settingsUseOtherProgramsToOpenFolders,nTemp);
		LocRegKey.QueryValue(L"Open folders with",m_OpenFoldersWith);

		if (LocRegKey.QueryValue("Transparency",nTemp))
			m_nTransparency=min(nTemp,255);


		if (LocRegKey.QueryValue("TooltipDelayAutopop",m_dwTooltipDelayAutopop))
			m_dwSettingsFlags|=settingsSetTooltipDelays;
		if (LocRegKey.QueryValue("TooltipDelayInitial",m_dwTooltipDelayInitial))
			m_dwSettingsFlags|=settingsSetTooltipDelays;

		if (LocRegKey.QueryValue("Update Process Priority",nTemp))
			m_nUpdateThreadPriority=nTemp;


		if (LocRegKey.QueryValue("ResultListFont",(LPSTR)&m_lResultListFont,sizeof(LOGFONT))==sizeof(LOGFONT))
			m_dwSettingsFlags|=settingsUseCustomResultListFont;


		if (LocRegKey.QueryValue(L"CustomTrayIcon",m_CustomTrayIcon))
			m_dwSettingsFlags|=settingsCustomUseTrayIcon;

	}

	// m_bAdvancedAndContextMenuFlag
	m_bAdvancedAndContextMenuFlag=0;
	if (GenRegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		m_bAdvancedAndContextMenuFlag|=cmLocateOnMyComputer;
	if (GenRegKey.OpenKey(HKCR,"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		m_bAdvancedAndContextMenuFlag|=cmLocateOnMyDocuments;
	if (GenRegKey.OpenKey(HKCR,"Drive\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		m_bAdvancedAndContextMenuFlag|=cmLocateOnDrives;
	if (GenRegKey.OpenKey(HKCR,"Directory\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		m_bAdvancedAndContextMenuFlag|=cmLocateOnFolders;
	if (GenRegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\updatedb",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		m_bAdvancedAndContextMenuFlag|=cmUpdateOnMyComputer;
	
	
	// m_strLanguage
	if (LocRegKey.OpenKey(HKCU,"",
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		LocRegKey.QueryValue(L"Language",m_strLangFile);
		LocRegKey.CloseKey();
	}
	if (m_strLangFile.IsEmpty())
	{
		m_strLangFile=L"lan_en.dll";
		SetSettingsFlags(settingsUseLanguageWithConsoleApps);
	}
	else if (GenRegKey.OpenKey(HKCU,CLocateApp::GetRegKey(""),
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		CStringW tmp;
		GenRegKey.QueryValue(L"Language",tmp);
		SetSettingsFlags(settingsUseLanguageWithConsoleApps,tmp.CompareNoCase(m_strLangFile)==0);
		GenRegKey.CloseKey();
	}
		
	// Checking wheter locate is runned at system startup
	if (GenRegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		CStringW Path;
		if (GenRegKey.QueryValue(L"Startup",Path))
		{
			if (Path.LastChar()!='\\')
				Path << '\\';
			Path<<L"Locate32 Autorun.lnk";
			
			SetSettingsFlags(settingsStartLocateAtStartup,FileSystem::IsFile(Path));
			
		}
	}
	
	// Update status tooltip
	if (LocRegKey.OpenKey(HKCU,"\\Dialogs\\Updatestatus",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD dwTemp;
		if (LocRegKey.QueryValue("Transparency",dwTemp))
			m_nToolTipTransparency=min(dwTemp,255);
		if (LocRegKey.QueryValue("TextColor",dwTemp))
			m_cToolTipTextColor=dwTemp;
		if (LocRegKey.QueryValue("TitleColor",dwTemp))
			m_cToolTipTitleColor=dwTemp;
		if (LocRegKey.QueryValue("ErrorColor",dwTemp))
			m_cToolTipErrorColor=dwTemp;
		if (LocRegKey.QueryValue("BackColor",dwTemp))
			m_cToolTipBackColor=dwTemp;

	
		if (LocRegKey.QueryValue("TextFont",(LPSTR)&m_lToolTipTextFont,sizeof(LOGFONT))<sizeof(LOGFONT))
			CLocateAppWnd::CUpdateStatusWnd::FillFontStructs(&m_lToolTipTextFont,NULL);
		
		if (LocRegKey.QueryValue("TitleFont",(LPSTR)&m_lToolTipTitleFont,sizeof(LOGFONT))<sizeof(LOGFONT))
			CLocateAppWnd::CUpdateStatusWnd::FillFontStructs(NULL,&m_lToolTipTitleFont);
		
	}

	// Retrieve tooltip default times
	if (m_dwTooltipDelayAutopop==DWORD(-1))
		m_dwTooltipDelayAutopop=GetDoubleClickTime()*10;
	if (m_dwTooltipDelayInitial==DWORD(-1))
		m_dwTooltipDelayInitial=GetDoubleClickTime();



	// Update status tooltip
	if (LocRegKey.OpenKey(HKCU,"\\Misc",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD dwTemp=0;;
		LocRegKey.QueryValue("NoExtensionInRename",dwTemp);
		if (dwTemp)
			m_dwSettingsFlags|=settingsDontShowExtensionInRenameDialog;
	}
			
	
	return TRUE;
}


BOOL CSettingsProperties::SaveSettings()
{
	DebugMessage("CSettingsProperties::SaveSettings()");
	
	CRegKey GenRegKey;
	CRegKey2 LocRegKey;

	if (LocRegKey.OpenKey(HKCU,"\\General",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		LocRegKey.SetValue("Program Status",m_dwLocateDialogFlags&CLocateDlg::fgSave);
		LocRegKey.SetValue("Program StatusExtra",m_dwLocateDialogExtraFlags&CLocateDlg::efSave);
		LocRegKey.SetValue("General Flags",m_dwProgramFlags&CLocateApp::pfSave);
		
		LocRegKey.SetValue(L"DateFormat",m_DateFormat);
		LocRegKey.SetValue(L"TimeFormat",m_TimeFormat);


		// Default flags
		LocRegKey.SetValue("Default CheckIn",m_bDefaultFlag&defaultCheckInFlag);
		LocRegKey.SetValue("Default MatchWholeName",m_bDefaultFlag&defaultWholeName?1:0);
		LocRegKey.SetValue("Default DataMatchCase",m_bDefaultFlag&defaultMatchCase?1:0);
		LocRegKey.SetValue("Default ReplaceSpaces",m_bDefaultFlag&defaultReplaceSpaces?1:0);
		LocRegKey.SetValue("Default UseWholePath",m_bDefaultFlag&defaultUseWholePath?1:0);
		LocRegKey.SetValue("Default Sorting",DWORD(m_bSorting));
			
		// Overrinding explorer for opening folders
		LocRegKey.SetValue("Use other program to open folders",(DWORD)IsSettingsFlagSet(settingsUseOtherProgramsToOpenFolders));
		LocRegKey.SetValue(L"Open folders with",m_OpenFoldersWith);

		LocRegKey.SetValue("Transparency",m_nTransparency);

		if (m_dwSettingsFlags&settingsSetTooltipDelays)
		{
			LocRegKey.SetValue("TooltipDelayAutopop",m_dwTooltipDelayAutopop);
			LocRegKey.SetValue("TooltipDelayInitial",m_dwTooltipDelayInitial);
		}
		else
		{
			LocRegKey.DeleteValue("TooltipDelayAutopop");
			LocRegKey.DeleteValue("TooltipDelayInitial");
		}


		LocRegKey.SetValue("Update Process Priority",(DWORD)m_nUpdateThreadPriority);




		if (m_dwSettingsFlags&settingsUseCustomResultListFont)
			LocRegKey.SetValue("ResultListFont",(LPSTR)&m_lResultListFont,sizeof(LOGFONT));
		else
			LocRegKey.DeleteValue("ResultListFont");

		if (m_dwSettingsFlags&settingsCustomUseTrayIcon)
			LocRegKey.SetValue(L"CustomTrayIcon",m_CustomTrayIcon);
		else
			LocRegKey.DeleteValue("CustomTrayIcon");

	}


	((CLocateApp*)GetApp())->m_strDateFormat=m_DateFormat;
	((CLocateApp*)GetApp())->m_strTimeFormat=m_TimeFormat;
	((CLocateApp*)GetApp())->m_nFileSizeFormat=m_nFileSizeFormat;

	if (LocRegKey.OpenKey(HKCU,"\\Recent Strings",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		LocRegKey.SetValue("NumberOfDirectories",m_nNumberOfDirectories);
		LocRegKey.SetValue("NumberOfNames",m_nNumberOfNames);
		LocRegKey.SetValue("NumberOfTypes",m_nNumberOfTypes);
	}

	if (LocRegKey.OpenKey(HKCU,"\\Locate",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		LocRegKey.SetValue("MaximumFoundFiles",(LPTSTR)&m_nMaximumFoundFiles,4,REG_DWORD);
	if (GetLocateDlg()!=NULL)
	{
		GetLocateDlg()->SetMaxFoundFiles(m_nMaximumFoundFiles);
		GetLocateDlg()->m_NameDlg.ChangeNumberOfItemsInLists(m_nNumberOfNames,m_nNumberOfTypes,m_nNumberOfDirectories);
	}

	
	// Settings databases
	CDatabase::CheckIDs(m_aDatabases);
	if (!IsSettingsFlagSet(settingsDatabasesOverridden))
	{
		GetLocateApp()->SetDatabases(m_aDatabases);
		CDatabase::SaveToRegistry(HKCU,CLocateApp::GetRegKey("Databases"),GetLocateApp()->GetDatabases());

		GetLocateApp()->ClearStartupFlag(CLocateApp::CStartData::startupDatabasesOverridden);
	}
	else
		GetLocateApp()->SetDatabases(m_aDatabases);

	// Save shortcuts
	if (!CShortcut::SaveShortcuts(m_aShortcuts))
	{
		ShowErrorMessage(IDS_ERRORCANNOTSAVESHORTCUTS,IDS_ERROR);
	}
	GetLocateAppWnd()->PostMessage(WM_RESETSHORTCUTS);
	if (GetLocateDlg()!=NULL)
		GetLocateDlg()->PostMessage(WM_RESETSHORTCUTS);
	
    
	
	// Insert/Remove context menu items
	
	// Locate... on My Computer
	if (GenRegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		GenRegKey.CloseKey();
		if ((m_bAdvancedAndContextMenuFlag&cmLocateOnMyComputer)==0)
			CRegKey::DeleteKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\locate");
	}
	else if (m_bAdvancedAndContextMenuFlag&cmLocateOnMyComputer)
	{
		if (GenRegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\locate",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			GenRegKey.SetValue("",ID2A(IDS_EXPLORERLOCATE));
			CRegKey CommandKey;
			if (CommandKey.OpenKey(GenRegKey,"command",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
			{
				CStringW command;
				command.Format(L"\"%s\" /P4",(LPCWSTR)GetApp()->GetExeNameW());
				CommandKey.SetValue(L"",command);
			}
		}
	}
	
	// Locate... on My Documents
	if (GenRegKey.OpenKey(HKCR,"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		GenRegKey.CloseKey();
		if ((m_bAdvancedAndContextMenuFlag&cmLocateOnMyDocuments)==0)
			CRegKey::DeleteKey(HKCR,"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\shell\\locate");
	}
	else if (m_bAdvancedAndContextMenuFlag&cmLocateOnMyDocuments)
	{
		if (GenRegKey.OpenKey(HKCR,"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\shell\\locate",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			GenRegKey.SetValue("",ID2A(IDS_EXPLORERLOCATE));
			CRegKey CommandKey;
			if (CommandKey.OpenKey(GenRegKey,"command",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
			{
				CStringW command;
				command.Format(L"\"%s\" /P3",(LPCWSTR)GetApp()->GetExeNameW());
				CommandKey.SetValue(L"",command);
			}
		}
	}

	// Locate... on Drives
	if (GenRegKey.OpenKey(HKCR,"Drive\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		GenRegKey.CloseKey();
		if ((m_bAdvancedAndContextMenuFlag&cmLocateOnDrives)==0)
			CRegKey::DeleteKey(HKCR,"Drive\\shell\\locate");
	}
	else if (m_bAdvancedAndContextMenuFlag&cmLocateOnDrives)
	{
		if (GenRegKey.OpenKey(HKCR,"Drive\\shell\\locate",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			GenRegKey.SetValue("",ID2A(IDS_EXPLORERLOCATE));
			CRegKey CommandKey;
			if (CommandKey.OpenKey(GenRegKey,"command",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
			{
				CStringW command;
				command.Format(L"\"%s\" /p \"%%1\"",(LPCWSTR)GetApp()->GetExeNameW());
				CommandKey.SetValue(L"",command);
			}
		}
	}

	// Locate... on Directories
	if (GenRegKey.OpenKey(HKCR,"Directory\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		GenRegKey.CloseKey();
		if ((m_bAdvancedAndContextMenuFlag&cmLocateOnFolders)==0)
			CRegKey::DeleteKey(HKCR,"Directory\\shell\\locate");
	}
	else if (m_bAdvancedAndContextMenuFlag&cmLocateOnFolders)
	{
		if (GenRegKey.OpenKey(HKCR,"Directory\\shell\\locate",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			GenRegKey.SetValue("",ID2A(IDS_EXPLORERLOCATE));
			CRegKey CommandKey;
			if (CommandKey.OpenKey(GenRegKey,"command",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
			{
				CStringW command;
				command.Format(L"\"%s\" /p \"%%1\"",(LPCWSTR)GetApp()->GetExeNameW());
				CommandKey.SetValue(L"",command);
			}
		}
	}

	// Update Database... on My Computer
	if (GenRegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\updatedb",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		GenRegKey.CloseKey();
		if ((m_bAdvancedAndContextMenuFlag&cmUpdateOnMyComputer)==0)
			CRegKey::DeleteKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\updatedb");
	}
	else if (m_bAdvancedAndContextMenuFlag&cmUpdateOnMyComputer)
	{
		if (GenRegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\updatedb",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			GenRegKey.SetValue("",ID2A(IDS_EXPLORERUPDATE));
			CRegKey CommandKey;
			if (CommandKey.OpenKey(GenRegKey,"command",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
			{
				CStringW sExeName(GetApp()->GetExeNameW());
				CStringW command(sExeName,sExeName.FindLast(L'\\')+1);
				command << L"updtdb32.exe\"";
				command.Insert(0,L'\"');
				CommandKey.SetValue(L"",command);
			}
		}
	}

	// Language
	if (LocRegKey.OpenKey(HKCU,"",
		CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		LocRegKey.SetValue(L"Language",m_strLangFile);
		LocRegKey.CloseKey();
		
		if (GenRegKey.OpenKey(HKCU,CLocateApp::GetRegKey(""),
				CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			if (IsSettingsFlagSet(settingsUseLanguageWithConsoleApps))
				GenRegKey.SetValue(L"Language",m_strLangFile);
			else
				GenRegKey.DeleteValue("Language");
		}

	}	

	// Creating or deleting shortcut to Startup mene if necessary
	if (GenRegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		CStringW Path;
		if (GenRegKey.QueryValue(L"Startup",Path))
		{
			if (Path.LastChar()!=L'\\')
				Path << L'\\';
			Path<<L"Locate32 Autorun.lnk";
			
			if (IsSettingsFlagSet(settingsStartLocateAtStartup))
			{
				if (!FileSystem::IsFile(Path))
					CreateShortcut(Path,GetApp()->GetExeNameW(),L"",L" /S");
			}
			else 
			{
				if (FileSystem::IsFile(Path))
					FileSystem::Remove(Path);
			}

		}	
	}

	// Update status tooltip
	if (LocRegKey.OpenKey(HKCU,"\\Dialogs\\Updatestatus",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		
		LocRegKey.SetValue("Transparency",m_nToolTipTransparency);
		LocRegKey.SetValue("TextFont",(LPSTR)&m_lToolTipTextFont,sizeof(LOGFONT));
		LocRegKey.SetValue("TitleFont",(LPSTR)&m_lToolTipTitleFont,sizeof(LOGFONT));
		LocRegKey.SetValue("TextColor",(DWORD)m_cToolTipTextColor);
		LocRegKey.SetValue("TitleColor",(DWORD)m_cToolTipTitleColor);
		LocRegKey.SetValue("ErrorColor",(DWORD)m_cToolTipErrorColor);
		LocRegKey.SetValue("BackColor",(DWORD)m_cToolTipBackColor);
	}

	// Misc settings
	if (LocRegKey.OpenKey(HKCU,"\\Misc",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		LocRegKey.SetValue("NoExtensionInRename",m_dwSettingsFlags&settingsDontShowExtensionInRenameDialog?1:0);
	}
		
	
	return TRUE;
}

////////////////////////////////////////
// CGeneralSettingsPage
////////////////////////////////////////


BOOL CSettingsProperties::CGeneralSettingsPage::OnInitDialog(HWND hwndFocus)
{
	CPropertyPage::OnInitDialog(hwndFocus);
	
	// Setting mouse behaviour options
	if (m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgLVStylePointToSelect)
	{
		CheckDlgButton(IDC_POINTTOSELECT,1);
		OnPointToSelect();
		if ((m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgLVStyleAlwaysUnderline)==CLocateDlg::fgLVStyleAlwaysUnderline)
			CheckDlgButton(IDC_ALWAYSUNDERLINE,1);
		else if ((m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgLVStyleAlwaysUnderline)==CLocateDlg::fgLVStyleUnderLine)
			CheckDlgButton(IDC_POINTUNDERLINE,1);
		else
			CheckDlgButton(IDC_NEVERUNDERLINE,1);
	}
	else
	{
		CheckDlgButton(IDC_CLICKTOSELECT,1);
		CheckDlgButton(IDC_NEVERUNDERLINE,1);
		OnClickToSelect();
	}
		
	
	if (m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgLVStyleSystemDefine)
	{
		CheckDlgButton(IDC_SYSTEMSETTINGS,1);
		OnSystemSettings();
	}


	// Remember states
	if (m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgDialogRememberFields)
		CheckDlgButton(IDC_REMEMBERSTATES,1);
	// Minimize to ST
	if (m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgDialogMinimizeToST)
		CheckDlgButton(IDC_MINIMIZETOSYSTEMTRAY,1);
	if (m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgDialogCloseMinimizesDialog)
		CheckDlgButton(IDC_CLOSEBUTTONMINIMIZESWINDOW,1);

	

	// Close to ST
	if (GetLocateApp()->IsStartupFlagSet(CLocateApp::CStartData::startupLeaveBackground))
	{
		EnableDlgItem(IDC_CLOSETOSYSTEMTRAY,FALSE);
		CheckDlgButton(IDC_CLOSETOSYSTEMTRAY,1);
	}
	else if (m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgDialogLeaveLocateBackground)
		CheckDlgButton(IDC_CLOSETOSYSTEMTRAY,1);
	
	// Adding details to sorting box
	if (IsUnicodeSystem())
	{
		SendDlgItemMessageW(IDC_SORTING,CB_ADDSTRING,0,(LPARAM)(LPCWSTR)ID2W(IDS_NOSORTNG));
		for (int iDetail=0;iDetail<=CLocateDlg::LastType;iDetail++)
			SendDlgItemMessageW(IDC_SORTING,CB_ADDSTRING,0,(LPARAM)(LPCWSTR)ID2W(IDS_LISTNAME+iDetail));
	}
	else
	{
		SendDlgItemMessage(IDC_SORTING,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_NOSORTNG));
		for (int iDetail=0;iDetail<=CLocateDlg::LastType;iDetail++)
			SendDlgItemMessage(IDC_SORTING,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_LISTNAME+iDetail));
	}	
	
	// Defaults
	if (m_pSettings->m_bSorting!=BYTE(-1))
	{
		SendDlgItemMessage(IDC_SORTING,CB_SETCURSEL,(m_pSettings->m_bSorting&127)+1);
		CheckDlgButton((m_pSettings->m_bSorting&128)?IDC_DESCENDINGORDER:IDC_ASCENDINGORDER,TRUE);
	}
	else
	{
		SendDlgItemMessage(IDC_SORTING,CB_SETCURSEL,0);
		CheckDlgButton(IDC_ASCENDINGORDER,TRUE);
	}

	CComboBox Checkin(GetDlgItem(IDC_CHECKIN));
	Checkin.AddString(ID2W(IDS_FILENAMESONLY));
	Checkin.AddString(ID2W(IDS_FILEANDFOLDERNAMES));
	Checkin.AddString(ID2W(IDS_FOLDERNAMESONLY));
	Checkin.SetCurSel(m_pSettings->m_bDefaultFlag&CSettingsProperties::defaultCheckInFlag);
	
	CheckDlgButton(IDC_MATCHWHOLEFILENAMEONLY,m_pSettings->m_bDefaultFlag&CSettingsProperties::defaultWholeName);
	CheckDlgButton(IDC_REPLACESPACES,m_pSettings->m_bDefaultFlag&CSettingsProperties::defaultReplaceSpaces);
	CheckDlgButton(IDC_USEWHOLEPATH,m_pSettings->m_bDefaultFlag&CSettingsProperties::defaultUseWholePath);
	CheckDlgButton(IDC_MATCHCASE,m_pSettings->m_bDefaultFlag&CSettingsProperties::defaultMatchCase);
	
	return FALSE;
}

BOOL CSettingsProperties::CGeneralSettingsPage::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CPropertyPage::OnCommand(wID,wNotifyCode,hControl);
	switch (wID)
	{
	case IDC_SYSTEMSETTINGS:
		OnSystemSettings();
		break;
	case IDC_POINTTOSELECT:
		OnPointToSelect();
		break;
	case IDC_CLICKTOSELECT:
		OnClickToSelect();
		break;
	case IDC_NEVERUNDERLINE:
		OnNeverUnderline();
		break;
	case IDC_POINTUNDERLINE:
		OnPointUnderline();
		break;
	case IDC_ALWAYSUNDERLINE:
		OnAlwaysUnderline();
		break;
	}
	return FALSE;
}

BOOL CSettingsProperties::CGeneralSettingsPage::OnApply()
{
	m_pSettings->m_dwLocateDialogFlags&=~(CLocateDlg::fgLVStyleFlag|CLocateDlg::fgDialogLeaveLocateBackground|
		CLocateDlg::fgDialogRememberFields|CLocateDlg::fgDialogMinimizeToST|CLocateDlg::fgDialogCloseMinimizesDialog);

	// Setting tree view mouse behaviour
	if (IsDlgButtonChecked(IDC_SYSTEMSETTINGS))
		m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgLVStyleSystemDefine;
	else
	{
		if (IsDlgButtonChecked(IDC_POINTTOSELECT))
		{
			m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgLVStylePointToSelect;
			if (IsDlgButtonChecked(IDC_ALWAYSUNDERLINE))
				m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgLVStyleAlwaysUnderline;
			else if (IsDlgButtonChecked(IDC_POINTUNDERLINE))
				m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgLVStyleUnderLine;
		}
	}
	
	// Remember dialog states
	if (IsDlgButtonChecked(IDC_REMEMBERSTATES))
		m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgDialogRememberFields;
	// Minimize to system tray
	if (IsDlgButtonChecked(IDC_MINIMIZETOSYSTEMTRAY))
		m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgDialogMinimizeToST;
	// Close minimizes window
	if (IsDlgButtonChecked(IDC_CLOSEBUTTONMINIMIZESWINDOW))
		m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgDialogCloseMinimizesDialog;
	// Load file types from registry
	if (!GetLocateApp()->IsStartupFlagSet(CLocateApp::CStartData::startupLeaveBackground))
	{
		if (IsDlgButtonChecked(IDC_CLOSETOSYSTEMTRAY))
			m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgDialogLeaveLocateBackground;
	}
		
	// Defaults
	int nSel=(int)SendDlgItemMessage(IDC_SORTING,CB_GETCURSEL);
	if (nSel<=0)
		m_pSettings->m_bSorting=BYTE(-1);
	else
	{
		m_pSettings->m_bSorting=nSel-1;
		if (IsDlgButtonChecked(IDC_DESCENDINGORDER))
			m_pSettings->m_bSorting|=128;
	}

	m_pSettings->m_bDefaultFlag=(BYTE)SendDlgItemMessage(IDC_CHECKIN,CB_GETCURSEL);
	if (IsDlgButtonChecked(IDC_MATCHWHOLEFILENAMEONLY))
		m_pSettings->m_bDefaultFlag|=CSettingsProperties::defaultWholeName;
	if (IsDlgButtonChecked(IDC_REPLACESPACES))
		m_pSettings->m_bDefaultFlag|=CSettingsProperties::defaultReplaceSpaces;
	if (IsDlgButtonChecked(IDC_USEWHOLEPATH))
		m_pSettings->m_bDefaultFlag|=CSettingsProperties::defaultUseWholePath;
	if (IsDlgButtonChecked(IDC_MATCHCASE))
		m_pSettings->m_bDefaultFlag|=CSettingsProperties::defaultMatchCase;
	
	CPropertyPage::OnApply();
	return TRUE;
}

void CSettingsProperties::CGeneralSettingsPage::OnCancel()
{
	m_pSettings->SetSettingsFlags(CSettingsProperties::settingsCancelled);
	
	CPropertyPage::OnCancel();
}


BYTE CSettingsProperties::CGeneralSettingsPage::OnSystemSettings()
{
	if (IsDlgButtonChecked(IDC_SYSTEMSETTINGS))
	{
		EnableDlgItem(IDC_POINTTOSELECT,FALSE);
		EnableDlgItem(IDC_CLICKTOSELECT,FALSE);
		EnableDlgItem(IDC_NEVERUNDERLINE,FALSE);
		EnableDlgItem(IDC_POINTUNDERLINE,FALSE);
		EnableDlgItem(IDC_ALWAYSUNDERLINE,FALSE);
	}
	else
	{
		EnableDlgItem(IDC_POINTTOSELECT,TRUE);
		EnableDlgItem(IDC_CLICKTOSELECT,TRUE);
		if (IsDlgButtonChecked(IDC_POINTTOSELECT))
		{
			EnableDlgItem(IDC_NEVERUNDERLINE,TRUE);
			EnableDlgItem(IDC_POINTUNDERLINE,TRUE);
			EnableDlgItem(IDC_ALWAYSUNDERLINE,TRUE);
		}
	}
	return TRUE;
}

BYTE CSettingsProperties::CGeneralSettingsPage::OnPointToSelect()
{
	CheckDlgButton(IDC_POINTTOSELECT,1);
	CheckDlgButton(IDC_CLICKTOSELECT,0);
	EnableDlgItem(IDC_NEVERUNDERLINE,TRUE);
	EnableDlgItem(IDC_POINTUNDERLINE,TRUE);
	EnableDlgItem(IDC_ALWAYSUNDERLINE,TRUE);
	return TRUE;
}

BYTE CSettingsProperties::CGeneralSettingsPage::OnClickToSelect()
{
	CheckDlgButton(IDC_POINTTOSELECT,0);
	CheckDlgButton(IDC_CLICKTOSELECT,1);
	EnableDlgItem(IDC_NEVERUNDERLINE,FALSE);
	EnableDlgItem(IDC_POINTUNDERLINE,FALSE);
	EnableDlgItem(IDC_ALWAYSUNDERLINE,FALSE);
	return TRUE;
}
	
BYTE CSettingsProperties::CGeneralSettingsPage::OnNeverUnderline()
{
	CheckDlgButton(IDC_NEVERUNDERLINE,1);
	CheckDlgButton(IDC_POINTUNDERLINE,0);
	CheckDlgButton(IDC_ALWAYSUNDERLINE,0);
	return TRUE;
}

BYTE CSettingsProperties::CGeneralSettingsPage::OnPointUnderline()
{
	CheckDlgButton(IDC_NEVERUNDERLINE,0);
	CheckDlgButton(IDC_POINTUNDERLINE,1);
	CheckDlgButton(IDC_ALWAYSUNDERLINE,0);
	return TRUE;
}

BYTE CSettingsProperties::CGeneralSettingsPage::OnAlwaysUnderline()
{
	CheckDlgButton(IDC_NEVERUNDERLINE,0);
	CheckDlgButton(IDC_POINTUNDERLINE,0);
	CheckDlgButton(IDC_ALWAYSUNDERLINE,1);
	return TRUE;
}


////////////////////////////////////////
// CAdvancedSettingsPage
////////////////////////////////////////


BOOL CSettingsProperties::CAdvancedSettingsPage::OnInitDialog(HWND hwndFocus)
{
	COptionsPropertyPage::OnInitDialog(hwndFocus);

	Item* TitleMethodItems[]={
		CreateCheckBox(IDS_ADVSETFIRSTCHARUPPER,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLV1stCharUpper,&m_pSettings->m_dwLocateDialogFlags),
		CreateRadioBox(IDS_ADVSETALWAYSSHOWEXT,NULL,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::fgLVAlwaysShowExtensions,CLocateDlg::fgLVExtensionFlag),&m_pSettings->m_dwLocateDialogFlags),
		CreateRadioBox(IDS_ADVSETHIDEEXTFORKNOWN,NULL,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::fgLVHideKnownExtensions,CLocateDlg::fgLVExtensionFlag),&m_pSettings->m_dwLocateDialogFlags),
		CreateRadioBox(IDS_ADVSETNEVERSHOWEXT,NULL,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::fgLVNeverShowExtensions,CLocateDlg::fgLVExtensionFlag),&m_pSettings->m_dwLocateDialogFlags),
		NULL
	};
	Item* TypeMethodItems[]={
		CreateRadioBox(IDS_ADVSETUSESHELLFORTYPE,NULL,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::fgLVShowShellType,CLocateDlg::fgLVShowShellType),&m_pSettings->m_dwLocateDialogFlags),
		CreateRadioBox(IDS_ADVSETUSEOWNMETHODFORTYPE,NULL,DefaultRadioBoxProc,
			MAKELONG(0,CLocateDlg::fgLVShowShellType),&m_pSettings->m_dwLocateDialogFlags),
		NULL
	};
	Item* OtherExplorerProgram[]={
		CreateFile(IDS_ADVSETOPENFOLDERWITH,ExternalCommandProc,0,&m_pSettings->m_OpenFoldersWith),
		NULL
	};

	Item* TooltipDelayItems[]={
		CreateNumeric(IDS_ADVSETSHOWTOOLTIPDELAY,DefaultNumericProc,
			MAKELONG(0,1000000),&m_pSettings->m_dwTooltipDelayInitial),
		CreateNumeric(IDS_ADVSETSHOWTOOLTIPDURATION,DefaultNumericProc,
			MAKELONG(0,1000000),&m_pSettings->m_dwTooltipDelayAutopop),
		NULL
	};
	Item* ResultListFontItems[]={
		CreateFont(IDS_ADVSETRESULTLISTFONT,DefaultFontProc,NULL,&m_pSettings->m_lResultListFont),
		NULL
	};
	Item* FileViewItems[]={
		CreateCheckBox(IDS_ADVSETUSECUSTOMFONTINRESULTLIST,ResultListFontItems,DefaultCheckBoxProc,
			CSettingsProperties::settingsUseCustomResultListFont,&m_pSettings->m_dwSettingsFlags),
		CreateCheckBox(IDS_ADVSETNODELAYEDUPDATING,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVNoDelayedUpdate,&m_pSettings->m_dwLocateDialogFlags),
		CreateRadioBox(IDS_ADVSETUSEGETTITLE,NULL,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::fgLVUseGetFileTitle,CLocateDlg::fgLVMethodFlag),&m_pSettings->m_dwLocateDialogFlags),
		CreateRadioBox(IDS_ADVSETUSEOWNMETHODFORTITLE,TitleMethodItems,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::fgLVUseOwnMethod,CLocateDlg::fgLVMethodFlag),&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETSHOWTYPEICONS,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVShowIcons,&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETSHOWFILETYPES,TypeMethodItems,DefaultCheckBoxProc,
			CLocateDlg::fgLVShowFileTypes,&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETDONTSHOWTOOLTIPS,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVDontShowTooltips,&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETSETTOOLTIPDELAYS,TooltipDelayItems,DefaultCheckBoxProc,
			CSettingsProperties::settingsSetTooltipDelays,&m_pSettings->m_dwSettingsFlags),
		CreateCheckBox(IDS_ADVSETDONTSHOWHIDDENFILES,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVDontShowHiddenFiles,&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETDONTSHOWDELETEDFILES,NULL,DefaultCheckBoxProc,
			CLocateDlg::efLVDontShowDeletedFiles,&m_pSettings->m_dwLocateDialogExtraFlags),
		CreateCheckBox(IDS_ADVSETNODOUBLERESULTS,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVNoDoubleItems,&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETFOLDERSFIRST,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVFoldersFirst,&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETFULLROWSELECT,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVSelectFullRow,&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETACTIVATEFIRSTITEM,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVActivateFirstResult,&m_pSettings->m_dwLocateDialogFlags),
		CreateComboBox(IDS_ADVSETSHOWDATESINFORMAT,DateFormatComboProc,0,0),
		CreateComboBox(IDS_ADVSETSHOWTIMESINFORMAT,TimeFormatComboProc,0,0),
		CreateListBox(IDS_ADVSETSHOWFILESIZESINFORMAT,FileSizeListProc,0,&m_pSettings->m_nFileSizeFormat),
		CreateCheckBox(IDS_ADVSETFORMATWITHUSERLOCALE,NULL,DefaultCheckBoxProc,
			CLocateApp::pfFormatUseLocaleFormat,&m_pSettings->m_dwProgramFlags),
		CreateCheckBox(IDS_ADVSETUSEPROGRAMFORFOLDERS,OtherExplorerProgram,DefaultCheckBoxProc,
			CSettingsProperties::settingsUseOtherProgramsToOpenFolders,&m_pSettings->m_dwSettingsFlags),
		CreateCheckBox(IDS_ADVSETCOMPUTEMD5SUMS,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVComputeMD5Sums,&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETALLOWINPLACERENAMING,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVAllowInPlaceRenaming,&m_pSettings->m_dwLocateDialogFlags),
		NULL
	};
	Item* LimitMaximumResults[]={
		CreateNumeric(IDS_ADVSETMAXNUMBEROFRESULTS,DefaultNumericProc,
			DWORD(-1),&m_pSettings->m_nMaximumFoundFiles),
		NULL
	};		
	
	
	Item* FileBackgroundOperations[]={
		CreateRadioBox(IDS_ADVSETDISABLEFSCHANGETRACKING,NULL,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::efDisableFSTracking,CLocateDlg::efTrackingMask),&m_pSettings->m_dwLocateDialogExtraFlags),
		CreateRadioBox(IDS_ADVSETENABLEFSCHANGETRACKING,NULL,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::efEnableFSTracking,CLocateDlg::efTrackingMask),&m_pSettings->m_dwLocateDialogExtraFlags),
		NULL,
		NULL
	};
	if (GetProcAddress(GetModuleHandle("kernel32.dll"),"ReadDirectoryChangesW")!=NULL)
	{
		FileBackgroundOperations[2]=CreateRadioBox(IDS_ADVSETENABLEFSCHANGETRACKINGOLD,NULL,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::efEnableFSTrackingOld,CLocateDlg::efTrackingMask),&m_pSettings->m_dwLocateDialogExtraFlags);
	}

	Item* UpdateResults[]={
		CreateRadioBox(IDS_ADVSETDISABLEUPDATING,NULL,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::efDisableItemUpdating,CLocateDlg::efItemUpdatingMask),&m_pSettings->m_dwLocateDialogExtraFlags),
		CreateRadioBox(IDS_ADVSETENABLEUPDATING,FileBackgroundOperations,DefaultRadioBoxProc,
			MAKELONG(CLocateDlg::efEnableItemUpdating,CLocateDlg::efItemUpdatingMask),&m_pSettings->m_dwLocateDialogExtraFlags),
		NULL
	};
		

	Item* ResultsItems[]={
		CreateCheckBox(IDS_ADVSETLIMITRESULTS,LimitMaximumResults,LimitResultsCheckBoxProc,
			0,&m_pSettings->m_nMaximumFoundFiles),
		CreateRoot(IDS_ADVSETRESULTSLIST,FileViewItems),
		CreateRoot(IDS_ADVSETUPDATERESULTS,UpdateResults),
		NULL
	};
		

	Item* ShellContextMenuItems[]={
		CreateCheckBox(IDS_ADVSETLOCATEINMYCOMPUTER,NULL,
			DefaultCheckBoxProc,CSettingsProperties::cmLocateOnMyComputer,&m_pSettings->m_bAdvancedAndContextMenuFlag),
		CreateCheckBox(IDS_ADVSETLOCATEINMYDOCUMENTS,NULL,
			DefaultCheckBoxProc,CSettingsProperties::cmLocateOnMyDocuments,&m_pSettings->m_bAdvancedAndContextMenuFlag),
		CreateCheckBox(IDS_ADVSETLOCATEINDRIVES,NULL,
			DefaultCheckBoxProc,CSettingsProperties::cmLocateOnDrives,&m_pSettings->m_bAdvancedAndContextMenuFlag),
		CreateCheckBox(IDS_ADVSETLOCATEINFOLFERS,NULL,
			DefaultCheckBoxProc,CSettingsProperties::cmLocateOnFolders,&m_pSettings->m_bAdvancedAndContextMenuFlag),
		CreateCheckBox(IDS_ADVSETUPDATEINMYCOMPUTER,NULL,
			DefaultCheckBoxProc,CSettingsProperties::cmUpdateOnMyComputer,&m_pSettings->m_bAdvancedAndContextMenuFlag),
		NULL
	};

	

	Item* SystemItems[]={
		CreateCheckBox(IDS_ADVSETRUNATSYSTEMSTARTUP,NULL,
			DefaultCheckBoxProc,CSettingsProperties::settingsStartLocateAtStartup,&m_pSettings->m_dwSettingsFlags),
		CreateRoot(IDS_ADVSETSHELLCONTEXTMENU,ShellContextMenuItems),
		NULL
	};

	
	Item* LookInItems[]={
		CreateNumeric(IDS_ADVSETNUMBEROFDIRECTORIES,DefaultNumericProc,
			MAKELONG(0,100),&m_pSettings->m_nNumberOfDirectories),
		CreateCheckBox(IDS_ADVSETDONTSAVENETWORKDRIVES,NULL,DefaultCheckBoxProc,
			CLocateDlg::efNameDontSaveNetworkDrivesAndDirectories,&m_pSettings->m_dwLocateDialogExtraFlags),
		CreateRadioBox(IDS_ADVSETADDSELECTEDROOTS,NULL,DefaultRadioBoxShiftProc,
			MAKELONG(CLocateDlg::fgNameAddEnabledRoots>>16,CLocateDlg::fgNameRootFlag>>16),&m_pSettings->m_dwLocateDialogFlags),
		CreateRadioBox(IDS_ADVSETADDALLROOTS,NULL,DefaultRadioBoxShiftProc,
			MAKELONG(CLocateDlg::fgNameAddAllRoots>>16,CLocateDlg::fgNameRootFlag>>16),&m_pSettings->m_dwLocateDialogFlags),
		CreateRadioBox(IDS_ADVSETDONTADDANYROOTS,NULL,DefaultRadioBoxShiftProc,
			MAKELONG(CLocateDlg::fgNameDontAddRoots>>16,CLocateDlg::fgNameRootFlag>>16),&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETMULTIPLEDIRECTORIES,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgNameMultibleDirectories,&m_pSettings->m_dwLocateDialogFlags),
		NULL
	};

	Item* DialogItems[]={
		CreateCheckBox(IDS_ADVSETLARGEMODEONLY,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgDialogLargeModeOnly,&m_pSettings->m_dwLocateDialogFlags),
		CreateNumeric(IDS_ADVSETNUMBEROFNAMES,DefaultNumericProc,
			MAKELONG(0,256),&m_pSettings->m_nNumberOfNames),
		CreateNumeric(IDS_ADVSETNUMBEROFTYPES,DefaultNumericProc,
			MAKELONG(0,256),&m_pSettings->m_nNumberOfTypes),
		CreateRoot(IDS_ADVSETLOOKINCOMBO,LookInItems),
		CreateCheckBox(IDS_ADVSETLOADTYPES,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLoadRegistryTypes,&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETTOPMOST,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgDialogTopMost,&m_pSettings->m_dwLocateDialogFlags),
		NULL, // For transparency
		NULL
	};
	
	Item* StatusTooltipItems[]={
		CreateColor(IDS_ADVSETTOOLTIPTEXTCOLOR,DefaultColorProc,NULL,&m_pSettings->m_cToolTipTextColor),
		CreateColor(IDS_ADVSETTOOLTIPTITLECOLOR,DefaultColorProc,NULL,&m_pSettings->m_cToolTipTitleColor),
		CreateColor(IDS_ADVSETTOOLTIPERRORCOLOR,DefaultColorProc,NULL,&m_pSettings->m_cToolTipErrorColor),
		CreateColor(IDS_ADVSETTOOLTIPBACKCOLOR,DefaultColorProc,NULL,&m_pSettings->m_cToolTipBackColor),
		CreateFont(IDS_ADVSETTOOLTIPTEXTFONT,DefaultFontProc,NULL,&m_pSettings->m_lToolTipTextFont),
		CreateFont(IDS_ADVSETTOOLTIPTITLEFONT,DefaultFontProc,NULL,&m_pSettings->m_lToolTipTitleFont),
		CreateListBox(IDS_ADVSETTOOLTIPWINDOWPOSITION,UpdateTooltipPositionProc,0,&m_pSettings->m_dwProgramFlags),
		CreateListBox(IDS_ADVSETTOOLTIPWINDOWONTOP,UpdateTooltipTopmostProc,0,&m_pSettings->m_dwProgramFlags),
		NULL, // For transparency
		NULL
	};
	if (GetProcAddress(GetModuleHandle("user32.dll"),"SetLayeredWindowAttributes")!=NULL)
	{
		// Needs at least Win2k
		DialogItems[6]=CreateNumeric(IDS_ADVSETTRANSPARENCY,DefaultNumericProc,
			MAKELONG(0,255),&m_pSettings->m_nTransparency);
		StatusTooltipItems[8]=CreateNumeric(IDS_ADVSETTOOLTIPTRANSPARENCY,DefaultNumericProc,
			MAKELONG(0,255),&m_pSettings->m_nToolTipTransparency);

	}
		

	Item* UpdateProcessItems[]={
		CreateCheckBox(IDS_ADVSETSHOWUPDATESTATUSTOOLTIP,StatusTooltipItems,
			DefaultCheckBoxProc,CLocateApp::pfEnableUpdateTooltip,&m_pSettings->m_dwProgramFlags),
		CreateListBox(IDS_ADVSETUPDATETHREADPRIORITY,UpdateThreadPriorityProc,0,&m_pSettings->m_nUpdateThreadPriority),
		NULL
	};
	
	Item* SystemTrayIconItems[]={
		CreateFile(IDS_ADVSETICONFILE,TrayIconProc,0,&m_pSettings->m_CustomTrayIcon),
		NULL
	};
	Item* MiscItems[]={
		CreateCheckBox(IDS_ADVSETDONTSHOWEXTINRENAME,NULL,DefaultCheckBoxProc,
			CSettingsProperties::settingsDontShowExtensionInRenameDialog,&m_pSettings->m_dwSettingsFlags),
		CreateCheckBox(IDS_ADVSETSHOWCRITICALERRORS,NULL,
			DefaultCheckBoxProc,CLocateApp::pfShowCriticalErrors,&m_pSettings->m_dwProgramFlags),
		CreateCheckBox(IDS_ADVSETSHOWNONCRITICALERRORS,NULL,
			DefaultCheckBoxProc,CLocateApp::pfShowNonCriticalErrors,&m_pSettings->m_dwProgramFlags),
		CreateCheckBox(IDS_ADVSETCLICKACTIVATETRAYICON,NULL,
			DefaultCheckBoxProc,CLocateApp::pfTrayIconClickActivate,&m_pSettings->m_dwProgramFlags),
		CreateCheckBox(IDS_ADVSETCUSTOMTRAYICON,SystemTrayIconItems,DefaultCheckBoxProc,
			CSettingsProperties::settingsCustomUseTrayIcon,&m_pSettings->m_dwSettingsFlags),
		CreateCheckBox(IDS_ADVSETUSEDEFDIRECTORYICON,NULL,DefaultCheckBoxProc,
			CLocateApp::pfUseDefaultIconForDirectories,&m_pSettings->m_dwProgramFlags),
		NULL
	};

	
	Item* Parents[]={
		CreateRoot(IDS_ADVSETRESULTS,ResultsItems),
		CreateRoot(IDS_ADVSETDIALOGS,DialogItems),
		CreateRoot(IDS_ADVSETUPDATEPROCESS,UpdateProcessItems),
		CreateRoot(IDS_ADVSETMISCELLANEOUS,MiscItems),
		CreateRoot(IDS_ADVSETSYSTEM,SystemItems),
		NULL};

	Initialize(Parents);
	return FALSE;
}

void CSettingsProperties::CAdvancedSettingsPage::OnCancel()
{
	m_pSettings->SetSettingsFlags(CSettingsProperties::settingsCancelled);
	COptionsPropertyPage::OnCancel();
}


BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::LimitResultsCheckBoxProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch (pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		break;
	case BASICPARAMS::Get:
		pParams->bChecked=(*((LONG*)pParams->lParam))>0;
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		if (!pParams->bChecked)
			*((DWORD*)pParams->lParam)=0;
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}
	return TRUE;
}

BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::UpdateThreadPriorityProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		if (((INITIALIZEPARAMS*)pParams)->hControl!=NULL)
		{
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_PRIORITYHIGH));		
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_PRIORITYABOVENORMAL));		
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_PRIORITYNORMAL));		
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_PRIORITYBELOWNORMAL));		
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_PRIORITYLOW));		
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_PRIORITYIDLE));		
		}
		break;
	case BASICPARAMS::Get:
		switch ( *((int*)pParams->lParam))
		{
		case THREAD_PRIORITY_HIGHEST:
            pParams->lValue=0;
			break;
		case THREAD_PRIORITY_ABOVE_NORMAL:
            pParams->lValue=1;
			break;
		case THREAD_PRIORITY_NORMAL:
            pParams->lValue=2;
			break;
		case THREAD_PRIORITY_BELOW_NORMAL:
            pParams->lValue=3;
			break;
		case THREAD_PRIORITY_LOWEST:
            pParams->lValue=4;
			break;
		case THREAD_PRIORITY_IDLE:
            pParams->lValue=5;
			break;
		default:
			pParams->lValue=2;
			break;
		}
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		switch (((COMBOAPPLYPARAMS*)pParams)->nCurSel)
		{
		case 0:
            *((int*)pParams->lParam)=THREAD_PRIORITY_HIGHEST;
			break;
		case 1:
            *((int*)pParams->lParam)=THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case 2:
            *((int*)pParams->lParam)=THREAD_PRIORITY_NORMAL;
			break;
		case 3:
            *((int*)pParams->lParam)=THREAD_PRIORITY_BELOW_NORMAL;
			break;
		case 4:
            *((int*)pParams->lParam)=THREAD_PRIORITY_LOWEST;
			break;
		case 5:
            *((int*)pParams->lParam)=THREAD_PRIORITY_IDLE;
			break;
		}
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}		
	return TRUE;
}

BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::UpdateTooltipPositionProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		if (((INITIALIZEPARAMS*)pParams)->hControl!=NULL)
		{
			CComboBox cb(((INITIALIZEPARAMS*)pParams)->hControl);

			cb.AddString(ID2W(IDS_ADVSETTOOLTIPPOSNEARCLOCK));		
			cb.AddString(ID2W(IDS_ADVSETTOOLTIPPOSUPPERLEFT));		
			cb.AddString(ID2W(IDS_ADVSETTOOLTIPPOSUPPERRIGHT));		
			cb.AddString(ID2W(IDS_ADVSETTOOLTIPPOSLOWERLEFT));		
			cb.AddString(ID2W(IDS_ADVSETTOOLTIPPOSLOWERRIGHT));		
			cb.AddString(ID2W(IDS_ADVSETTOOLTIPPOSLASTPOSITION));		
		}
		break;
	case BASICPARAMS::Get:
		switch ((*((DWORD*)pParams->lParam))&CLocateApp::pfUpdateTooltipPositionMask)
		{
		case CLocateApp::pfUpdateTooltipPositionUpLeft:
            pParams->lValue=1;
			break;
		case CLocateApp::pfUpdateTooltipPositionUpRight:
            pParams->lValue=2;
			break;
		case CLocateApp::pfUpdateTooltipPositionDownLeft:
            pParams->lValue=3;
			break;
		case CLocateApp::pfUpdateTooltipPositionDownRight:
            pParams->lValue=4;
			break;
		case CLocateApp::pfUpdateTooltipPositionLastPosition:
			pParams->lValue=5;
			break;
		default:
			pParams->lValue=0;
			break;
		}
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		(*((DWORD*)pParams->lParam))&=~CLocateApp::pfUpdateTooltipPositionMask;
		switch (((COMBOAPPLYPARAMS*)pParams)->nCurSel)
		{
		case 0:
            *((int*)pParams->lParam)|=CLocateApp::pfUpdateTooltipPositionDefault;
			break;
		case 1:
            *((int*)pParams->lParam)|=CLocateApp::pfUpdateTooltipPositionUpLeft;
			break;
		case 2:
            *((int*)pParams->lParam)|=CLocateApp::pfUpdateTooltipPositionUpRight;
			break;
		case 3:
            *((int*)pParams->lParam)|=CLocateApp::pfUpdateTooltipPositionDownLeft;
			break;
		case 4:
            *((int*)pParams->lParam)|=CLocateApp::pfUpdateTooltipPositionDownRight;
			break;
		case 5:
			*((int*)pParams->lParam)|=CLocateApp::pfUpdateTooltipPositionLastPosition;
			break;
		}
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}		
	return TRUE;
}

BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::UpdateTooltipTopmostProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		if (((INITIALIZEPARAMS*)pParams)->hControl!=NULL)
		{
			CComboBox cb(((INITIALIZEPARAMS*)pParams)->hControl);
			
			cb.AddString(ID2W(IDS_ADVSETTOOLTIPALWAYSONTOP));		
			cb.AddString(ID2W(IDS_ADVSETTOOLTIPTOPIFFOREGROUNDNOTMAXIMIZED));		
			cb.AddString(ID2W(IDS_ADVSETTOOLTIPTOPIFFOREGROUNDNOTFULLSCREEN));		
			cb.AddString(ID2W(IDS_ADVSETTOOLTIPNEVERONTOP));		
		}
		break;
	case BASICPARAMS::Get:
		switch ((*((DWORD*)pParams->lParam))&CLocateApp::pfUpdateTooltipTopmostMask)
		{
		case CLocateApp::pfUpdateTooltipNoTopmostWhenForegroundWndMaximized:
            pParams->lValue=1;
			break;
		case CLocateApp::pfUpdateTooltipNoTopmostWhdnForegroundWndInFullScreen:
            pParams->lValue=2;
			break;
		case CLocateApp::pfUpdateTooltipNeverTopmost:
            pParams->lValue=3;
			break;
		default:
			pParams->lValue=0;
			break;
		}
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		(*((DWORD*)pParams->lParam))&=~CLocateApp::pfUpdateTooltipTopmostMask;
		switch (((COMBOAPPLYPARAMS*)pParams)->nCurSel)
		{
		case 0:
            *((int*)pParams->lParam)|=CLocateApp::pfUpdateTooltipAlwaysTopmost;
			break;
		case 1:
            *((int*)pParams->lParam)|=CLocateApp::pfUpdateTooltipNoTopmostWhenForegroundWndMaximized;
			break;
		case 2:
            *((int*)pParams->lParam)|=CLocateApp::pfUpdateTooltipNoTopmostWhdnForegroundWndInFullScreen;
			break;
		case 3:
            *((int*)pParams->lParam)|=CLocateApp::pfUpdateTooltipNeverTopmost;
			break;
		}
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}		
	return TRUE;
}

BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::TimeFormatComboProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		// Date and time formats
		if (((INITIALIZEPARAMS*)pParams)->hControl!=NULL)
		{
			((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer.RemoveAll();
			EnumTimeFormats(EnumTimeFormatsProc,LOCALE_USER_DEFAULT,0);
			//EnumDateFormats(DateFormatsProc,LOCALE_USER_DEFAULT,DATE_SHORTDATE);
		
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_ADVSETSYSTEMDEFAULT));		
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_SETCURSEL,0,0);		
	
			for (int i=0;i<((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer.GetSize();i++)
				::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer[i]);
			
			((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer.RemoveAll();
		}
		break;
	case BASICPARAMS::Get:
		if (!((CAdvancedSettingsPage*)pParams->pPage)->m_pSettings->m_TimeFormat.IsEmpty())
			pParams->pData=alloccopy(((CAdvancedSettingsPage*)pParams->pPage)->m_pSettings->m_TimeFormat);
		else
			pParams->pData=NULL;
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		if (pParams->pData==NULL || ((COMBOAPPLYPARAMS*)pParams)->nCurSel==0)
			((CAdvancedSettingsPage*)pParams->pPage)->m_pSettings->m_TimeFormat.Empty();
		else
			((CAdvancedSettingsPage*)pParams->pPage)->m_pSettings->m_TimeFormat.Copy(pParams->pData);
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}		
	return TRUE;
}

BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::DateFormatComboProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		// Date and time formats
		if (((INITIALIZEPARAMS*)pParams)->hControl!=NULL)
		{
			CComboBox cb(((INITIALIZEPARAMS*)pParams)->hControl);
			((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer.RemoveAll();
			EnumDateFormats(EnumDateFormatsProc,LOCALE_USER_DEFAULT,DATE_SHORTDATE);
		
			cb.AddString(ID2W(IDS_ADVSETSYSTEMDEFAULT));		
			cb.SetCurSel(0);		

			for (int i=0;i<((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer.GetSize();i++)
				cb.AddString(((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer[i]);
			
			((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer.RemoveAll();
		}
		break;
	case BASICPARAMS::Get:
		if (!((CAdvancedSettingsPage*)pParams->pPage)->m_pSettings->m_DateFormat.IsEmpty())
			pParams->pData=alloccopy(((CAdvancedSettingsPage*)pParams->pPage)->m_pSettings->m_DateFormat);
		else
			pParams->pData=NULL;
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		if (pParams->pData==NULL || ((COMBOAPPLYPARAMS*)pParams)->nCurSel==0)
			((CAdvancedSettingsPage*)pParams->pPage)->m_pSettings->m_DateFormat.Empty();
		else
			((CAdvancedSettingsPage*)pParams->pPage)->m_pSettings->m_DateFormat.Copy(pParams->pData);
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}
	return TRUE;
}


BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::FileSizeListProc(COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch(pParams->crReason)
	{
	case BASICPARAMS::Initialize:
		// File size formats
		if (((INITIALIZEPARAMS*)pParams)->hControl!=NULL)
		{
			CComboBox cb(((INITIALIZEPARAMS*)pParams)->hControl);
			CStringW text(IDS_ADVSETFILESIZEFORMATLESS1KB);
			cb.AddString(text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATDEPENGINGSIZE);
			cb.AddString(text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATBYTES);
			cb.AddString(text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATBYTENOUNITS);
			cb.AddString(text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATKB);
			cb.AddString(text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATKBNOUNITS);
			cb.AddString(text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATMB);
			cb.AddString(text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATMBNOUNITS);
			cb.AddString(text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATLESS1MB);
			cb.AddString(text);		
			
		}

		break;
	case BASICPARAMS::Get:
		pParams->lValue=*((CLocateApp::FileSizeFormats*)pParams->lParam);
		break;
	case BASICPARAMS::Set:
		break;
	case BASICPARAMS::Apply:
		*((LONG*)pParams->lParam)=((COMBOAPPLYPARAMS*)pParams)->nCurSel;
		break;
	case BASICPARAMS::ChangingValue:
		break;
	}
	return TRUE;
}

BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::EnumTimeFormatsProc(LPTSTR lpTimeFormatString)
{
	GetLocateAppWnd()->m_pSettings->m_pAdvanced->m_aBuffer.Add(alloccopy(lpTimeFormatString));
	return TRUE;
}

BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::EnumDateFormatsProc(LPTSTR lpDateFormatString)
{
	GetLocateAppWnd()->m_pSettings->m_pAdvanced->m_aBuffer.Add(alloccopy(lpDateFormatString));
	return TRUE;
}

BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::ExternalCommandProc(BASICPARAMS* pParams)
{
	if (pParams->crReason==BASICPARAMS::BrowseFile)
	{
		((BROWSEDLGPARAMS*)pParams)->szTitle=MAKEINTRESOURCEW(IDS_ADVSETSELECTPROGRAM);
		((BROWSEDLGPARAMS*)pParams)->szFilters=MAKEINTRESOURCEW(IDS_PROGRAMSFILTERS);
		return TRUE;
	}
	return DefaultEditStrWProc(pParams);
}


BOOL CALLBACK CSettingsProperties::CAdvancedSettingsPage::TrayIconProc(BASICPARAMS* pParams)
{
	if (pParams->crReason==BASICPARAMS::BrowseFile)
	{
		((BROWSEDLGPARAMS*)pParams)->szTitle=MAKEINTRESOURCEW(IDS_ADVSETSELECTICONFILE);
		((BROWSEDLGPARAMS*)pParams)->szFilters=MAKEINTRESOURCEW(IDS_ICONFILTERS);
		return TRUE;
	}
	return DefaultEditStrWProc(pParams);
}

////////////////////////////////////////
// CLanguageSettingsPage
////////////////////////////////////////


BOOL CSettingsProperties::CLanguageSettingsPage::OnInitDialog(HWND hwndFocus)
{
	CPropertyPage::OnInitDialog(hwndFocus);

	m_pList=new CListCtrl(GetDlgItem(IDC_LANGUAGE));
	if (IsUnicodeSystem())
		m_pList->SetUnicodeFormat(TRUE);
	
	m_pList->InsertColumn(0,ID2W(IDS_LANGUAGE),LVCFMT_LEFT,130);
	m_pList->InsertColumn(1,ID2W(IDS_LANGUAGEFILE),LVCFMT_LEFT,80);
	m_pList->InsertColumn(2,ID2W(IDS_LANGUAGEDESC),LVCFMT_LEFT,100);
	
	m_pList->SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT ,LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT );
	m_pList->LoadColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\Dialogs","Language Settings List Widths");

	FindLanguages();
	
	CheckDlgButton(IDC_USEWITHCONSOLEAPPS,
		m_pSettings->IsSettingsFlagSet(CSettingsProperties::settingsUseLanguageWithConsoleApps));
	return FALSE;
}

BOOL CSettingsProperties::CLanguageSettingsPage::OnApply()
{
	CPropertyPage::OnApply();
	
    m_pSettings->SetSettingsFlags(settingsUseLanguageWithConsoleApps,IsDlgButtonChecked(IDC_USEWITHCONSOLEAPPS));

	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	if (nItem!=-1)
	{
		LanguageItem* pli=(LanguageItem*)m_pList->GetItemData(nItem);
		m_pSettings->m_strLangFile=pli->File;
	}
	return TRUE;
}

void CSettingsProperties::CLanguageSettingsPage::OnDestroy()
{
	CPropertyPage::OnDestroy();

	if (m_pList!=NULL)
	{
		m_pList->SaveColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\Dialogs","Language Settings List Widths");
		delete m_pList;
		m_pList=NULL;
	}
}
		
void CSettingsProperties::CLanguageSettingsPage::OnCancel()
{
	m_pSettings->SetSettingsFlags(CSettingsProperties::settingsCancelled);

	CPropertyPage::OnCancel();
}

void CSettingsProperties::CLanguageSettingsPage::OnTimer(DWORD wTimerID)
{
	KillTimer(wTimerID);

	if (m_pList->GetNextItem(-1,LVNI_SELECTED)==-1)
		m_pList->SetItemState(nLastSel,LVIS_SELECTED,LVIS_SELECTED);
}

BOOL CSettingsProperties::CLanguageSettingsPage::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	switch (idCtrl)
	{
	case IDC_LANGUAGE:
		ListNotifyHandler((NMLISTVIEW*)pnmh);
		break;
	}
	return CPropertyPage::OnNotify(idCtrl,pnmh);
}


BOOL CSettingsProperties::CLanguageSettingsPage::ListNotifyHandler(NMLISTVIEW *pNm)
{
	switch(pNm->hdr.code)
	{
	case LVN_DELETEITEM:
		delete (LanguageItem*)pNm->lParam;
		break;
	case LVN_GETDISPINFO:
		{
			LV_DISPINFO *pLvdi=(LV_DISPINFO *)pNm;
			LanguageItem* li=(LanguageItem*)pLvdi->item.lParam;
            if (li==NULL)
				break;
			
			pLvdi->item.mask=LVIF_TEXT|LVIF_DI_SETITEM;

			if (g_szBuffer!=NULL)
				delete[] g_szBuffer;
			switch (pLvdi->item.iSubItem)
			{
			case 0:
				pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(li->Language,li->Language.GetLength());
				break;
			case 1:
				pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(li->File,li->File.GetLength());
				break;
			case 2:
				pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(li->Description,li->Description.GetLength());
				break;
			}
			break;
		}
	case LVN_GETDISPINFOW:
		{
			LV_DISPINFOW *pLvdi=(LV_DISPINFOW *)pNm;
			LanguageItem* li=(LanguageItem*)pLvdi->item.lParam;
            if (li==NULL)
				break;
			
			pLvdi->item.mask=LVIF_TEXT|LVIF_DI_SETITEM;

			switch (pLvdi->item.iSubItem)
			{
			case 0:
				pLvdi->item.pszText=li->Language.GetBuffer();
				break;
			case 1:
				pLvdi->item.pszText=li->File.GetBuffer();
				break;
			case 2:
				pLvdi->item.pszText=li->Description.GetBuffer();
				break;
			}
			break;
		}
	case LVN_ITEMCHANGED:
		if ((pNm->uOldState&LVIS_SELECTED)==0 && (pNm->uNewState&LVIS_SELECTED))
		{
			nLastSel=pNm->iItem;
			KillTimer(0);
		}
		if ((pNm->uOldState&LVIS_SELECTED) && (pNm->uNewState&LVIS_SELECTED)==0)
			SetTimer(0,100,NULL);
		break;
	}
	return TRUE;
}

void CSettingsProperties::CLanguageSettingsPage::FindLanguages()
{
	typedef void  (*LANGCALL)(
		LPSTR /* OUT */ szLanguage,
		DWORD /* IN  */ dwMaxLanguageLength,
		LPSTR /* OUT */ szDescription,
		DWORD /* IN  */ dwMaxDescriptionLength);
	typedef void  (*LANGCALLW)(
		LPWSTR /* OUT */ szLanguage,
		DWORD /* IN  */ dwMaxLanguageLength,
		LPWSTR /* OUT */ szDescription,
		DWORD /* IN  */ dwMaxDescriptionLength);

	CStringW Path(GetApp()->GetExeNameW());
	Path.FreeExtra(Path.FindLast('\\')+1);
	Path<<L"*.dll";

	
	LVITEM li;
	li.mask=LVIF_PARAM|LVIF_STATE|LVIF_TEXT;
	li.iItem=0;
	li.pszText=LPSTR_TEXTCALLBACK;
	li.stateMask=LVIS_SELECTED;

	CFileFind ff;
	BOOL bRet=ff.FindFile(Path);

    while (bRet)
	{
		char szPathTemp[MAX_PATH];
		ff.GetFilePath(szPathTemp,MAX_PATH);
		HINSTANCE hLib=LoadLibrary(szPathTemp);
        if (hLib!=NULL)
		{
			LANGCALL pFunc=(LANGCALL)GetProcAddress(hLib,"GetLocateLanguageFileInfo");
			if (pFunc==NULL) // Watcom style
				pFunc=(LANGCALL)GetProcAddress(hLib,"_GetLocateLanguageFileInfo");
			LANGCALLW pFuncW=(LANGCALLW)GetProcAddress(hLib,"GetLocateLanguageFileInfoW");
			if (pFuncW==NULL) // Watcom style
				pFuncW=(LANGCALLW)GetProcAddress(hLib,"_GetLocateLanguageFileInfoW");

			if (pFuncW!=NULL && IsUnicodeSystem())
			{
				LanguageItem* pli=new LanguageItem;
				pFuncW(pli->Language.GetBuffer(200),200,pli->Description.GetBuffer(1000),1000);
				pli->Language.FreeExtra();
				pli->Description.FreeExtra();

				ff.GetFileName(pli->File);
				li.lParam=(LPARAM)pli;
				li.iSubItem=0;
				if (m_pSettings->m_strLangFile.CompareNoCase(pli->File)==0)
				{
					li.state=LVIS_SELECTED;
					nLastSel=li.iItem;
				}
				else
					li.state=0;

				m_pList->InsertItem(&li);
				li.iItem++;
			}
			else if (pFunc!=NULL)
			{
				LanguageItem* pli=new LanguageItem;
				char szLanguage[200],szDescription[1000];
				pFunc(szLanguage,200,szDescription,1000);
				pli->Language=szLanguage;
				pli->Description=szDescription;
				
				ff.GetFileName(pli->File);
				li.lParam=(LPARAM)pli;
				li.iSubItem=0;
				if (m_pSettings->m_strLangFile.CompareNoCase(pli->File)==0)
				{
					li.state=LVIS_SELECTED;
					nLastSel=li.iItem;
				}
				else
					li.state=0;

				m_pList->InsertItem(&li);
				li.iItem++;
			}
			FreeLibrary(hLib);
		}
		bRet=ff.FindNextFile();
	}
}




////////////////////////////////////////
// CDatabasesSettingsPage
////////////////////////////////////////

BOOL CSettingsProperties::CDatabasesSettingsPage::OnInitDialog(HWND hwndFocus)
{
	CPropertyPage::OnInitDialog(hwndFocus);

	// Initializing list control
	m_pList=new CListCtrl(GetDlgItem(IDC_DATABASES));
	m_pList->SetExtendedListViewStyle(LVS_EX_CHECKBOXES|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT,
		LVS_EX_CHECKBOXES|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	m_pList->InsertColumn(0,ID2A(IDS_DATABASENAME),LVCFMT_LEFT,100,0);
	m_pList->InsertColumn(1,ID2A(IDS_DATABASEFILE),LVCFMT_LEFT,130,0);
	m_pList->InsertColumn(2,ID2A(IDS_GLOBALUPDATE),LVCFMT_LEFT,70,0);
	if (((CLocateApp*)GetApp())->m_wComCtrlVersion<0x0600)
	{
		m_pList->InsertColumn(3,ID2A(IDS_THREADID),LVCFMT_LEFT,40,0);
		int oa[]={3,0,1,2};
		m_pList->SetColumnOrderArray(4,oa);
	}
	if (IsUnicodeSystem())
		m_pList->SetUnicodeFormat(TRUE);

	m_pList->LoadColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\Dialogs","Databases Settings List Widths");
	
	// Setting threads counter
	CSpinButtonCtrl Spin(GetDlgItem(IDC_THREADSPIN));
	Spin.SetBuddy(GetDlgItem(IDC_THREADS));
	Spin.SetRange(1,99);

	SetDatabasesToList();


	if (m_pSettings->IsSettingsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
	{
		// Databases are overridden via command line parameters
		// Disabling new button
		EnableDlgItem(IDC_NEW,FALSE);

		EnableDlgItem(IDC_THREADS,FALSE);
		EnableDlgItem(IDC_THREADSPIN,FALSE);

		ShowDlgItem(IDC_THREADS,swHide);
		ShowDlgItem(IDC_THREADSPIN,swHide);
	}
	else
	{

		ShowDlgItem(IDC_OVERRIDETXT,swHide);
		ShowDlgItem(IDC_RESTORE,swHide);
	}

	EnableButtons();	
	return FALSE;
}

void CSettingsProperties::CDatabasesSettingsPage::SetDatabasesToList()
{
	if (m_nThreadsCurrently>1)
		RemoveThreadGroups();
	
	m_nThreadsCurrently=CDatabase::CheckIDs(m_pSettings->m_aDatabases);
	
	SendDlgItemMessage(IDC_THREADSPIN,UDM_SETPOS,0,MAKELONG(m_nThreadsCurrently,0));
	

	LVITEM li;
	li.pszText=LPSTR_TEXTCALLBACK;
	if (m_nThreadsCurrently>1 && ((CLocateApp*)GetApp())->m_wComCtrlVersion>=0x0600)
	{
        li.mask=LVIF_TEXT|LVIF_PARAM|LVIF_GROUPID;

		EnableThreadGroups(m_nThreadsCurrently);
	}
	else
		li.mask=LVIF_TEXT|LVIF_PARAM;
			
	li.iSubItem=0;
	
	for (li.iItem=0;li.iItem<m_pSettings->m_aDatabases.GetSize();li.iItem++)
	{
		CDatabase* pDatabase=m_pSettings->m_aDatabases[li.iItem];
		li.lParam=LPARAM(pDatabase);
		li.iGroupId=pDatabase->GetThreadId();

		m_pList->SetCheckState(
			m_pList->InsertItem(&li),
			pDatabase->IsEnabled());
	}
	
	// Removing m_aDatabases without freeing cells
	delete[] m_pSettings->m_aDatabases.GiveBuffer();
}

void CSettingsProperties::CDatabasesSettingsPage::OnDestroy()
{
	if (m_pList!=NULL)
	{
		m_pList->SaveColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\Dialogs","Databases Settings List Widths");
		delete m_pList;
		m_pList=NULL;
	}
	CPropertyPage::OnDestroy();
}

BOOL CSettingsProperties::CDatabasesSettingsPage::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch (wID)
	{
	case IDC_NEW:
		OnNew();
		break;
	case IDC_EDIT:
		OnEdit();
		break;
	case IDC_REMOVE:
		OnRemove();
		break;
	case IDC_ENABLE:
		OnEnable(TRUE);
		break;
	case IDC_DISABLE:
		OnEnable(FALSE);
		break;
	case IDC_UP:
		ItemUpOrDown(TRUE);
		break;
	case IDC_DOWN:
		ItemUpOrDown(FALSE);
		break;
	case IDC_UPDATE:
		OnUpdate();
		break;
	case IDC_RESTORE:
		OnRestore();
		break;
	case IDC_IMPORT:
		OnImport();
		break;
	case IDC_EXPORT:
		OnExport();
		break;
	case IDC_THREADS:
		if (wNotifyCode==EN_CHANGE)
			OnThreads();
		else if (wNotifyCode==EN_SETFOCUS)
			SendDlgItemMessage(IDC_THREADS,EM_SETSEL,0,-1);
		break;
	}
	return CPropertyPage::OnCommand(wID,wNotifyCode,hControl);
}

void CSettingsProperties::CDatabasesSettingsPage::OnNew(CDatabase* pDatabaseTempl)
{
	CWaitCursor wait;

	CDatabaseDialog dbd;
	if (pDatabaseTempl!=NULL)
		dbd.m_pDatabase=pDatabaseTempl;
	else
		dbd.m_pDatabase=CDatabase::FromDefaults(FALSE);
	dbd.m_nMaximumNumbersOfThreads=m_nThreadsCurrently;
	
	if (m_pList->GetItemCount()>0)
	{
		int iItem=m_pList->GetNextItem(-1,LVNI_ALL);
		while (iItem!=-1)
		{
			CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(iItem);
			if (pDatabase!=NULL)
				dbd.m_aOtherDatabases.Add(pDatabase);
			iItem=m_pList->GetNextItem(iItem,LVNI_ALL);
		}
	}

	if (dbd.DoModal(*this,LanguageSpecificResource))
	{
		LVITEM li;

		if (m_pList->GetItemCount()>0)
		{
			li.iItem=m_pList->GetNextItem(-1,LVNI_ALL);
			while (li.iItem!=-1)
			{
				CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(li.iItem);
				ASSERT(pDatabase!=NULL);

				if (pDatabase->GetThreadId()>dbd.m_pDatabase->GetThreadId())
					break;

				li.iItem=m_pList->GetNextItem(li.iItem,LVNI_ALL);
			}

			// Inserting to the end of list
			if (li.iItem==-1)
				li.iItem=m_pList->GetItemCount();

		}
		else
			li.iItem=0;

		
		li.pszText=LPSTR_TEXTCALLBACK;
		if (m_nThreadsCurrently>1 && ((CLocateApp*)GetApp())->m_wComCtrlVersion>=0x0600)
			li.mask=LVIF_TEXT|LVIF_PARAM|LVIF_STATE|LVIF_GROUPID;
		else
			li.mask=LVIF_TEXT|LVIF_PARAM|LVIF_STATE;
		li.iSubItem=0;
		li.lParam=LPARAM(dbd.m_pDatabase);
		li.iGroupId=dbd.m_pDatabase->GetThreadId();
		li.state=LVIS_SELECTED;
		li.stateMask=LVIS_SELECTED;
		li.iItem=m_pList->InsertItem(&li);
		m_pList->SetCheckState(li.iItem,dbd.m_pDatabase->IsEnabled());

		m_pList->EnsureVisible(li.iItem,FALSE);
	}
	else
		delete dbd.m_pDatabase;


	EnableButtons();
	m_pList->SetFocus();
}

void CSettingsProperties::CDatabasesSettingsPage::OnEdit()
{
	CWaitCursor wait;

	ASSERT(m_pList->GetItemCount()>0);

	CDatabaseDialog dbd;

	if (m_pSettings->IsSettingsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
		dbd.m_bDontEditName=TRUE;

	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	if (nItem==-1)
		return;
	
	CDatabase* pOrigDatabase=(CDatabase*)m_pList->GetItemData(nItem);
	ASSERT(pOrigDatabase!=NULL);

	dbd.m_pDatabase=new CDatabase(*pOrigDatabase);
	dbd.m_nMaximumNumbersOfThreads=m_nThreadsCurrently;
	
	int iOtherItem=m_pList->GetNextItem(-1,LVNI_ALL);
	while (iOtherItem!=-1)
	{
		CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(iOtherItem);
		if (pDatabase!=NULL && pDatabase!=pOrigDatabase)
			dbd.m_aOtherDatabases.Add(pDatabase);
		iOtherItem=m_pList->GetNextItem(iOtherItem,LVNI_ALL);
	}


	if (dbd.DoModal(*this,LanguageSpecificResource))
	{
		delete (CDatabase*)m_pList->GetItemData(nItem);
		m_pList->SetItemData(nItem,LPARAM(dbd.m_pDatabase));

		m_pList->RedrawItems(nItem,nItem);
		m_pList->SetCheckState(nItem,dbd.m_pDatabase->IsEnabled());

		if (((CLocateApp*)GetApp())->m_wComCtrlVersion>=0x0600 && m_nThreadsCurrently>1)
		{
			LVITEM li;
			li.iItem=nItem;
			li.iSubItem=0;
			li.mask=LVIF_GROUPID;
			li.iGroupId=dbd.m_pDatabase->GetThreadId();
			m_pList->SetItem(&li);
		}
	}
	else
		delete dbd.m_pDatabase;


	if (m_nThreadsCurrently>1 && GetLocateApp()->m_wComCtrlVersion<0x0600)
		m_pList->SortItems(ThreadSortProc,NULL);

	EnableButtons();
	m_pList->SetFocus();
}

void CSettingsProperties::CDatabasesSettingsPage::OnRemove()
{
	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	if (nItem==-1)
		return;

	CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(nItem);
	if (pDatabase->GetArchiveType()==CDatabase::archiveFile)
	{
		if (FileSystem::IsFile(pDatabase->GetArchiveName()))
		{
			CStringW str;
			str.Format(IDS_DELETEDATABASEQUESTION,pDatabase->GetArchiveName());
			int nRet=MessageBox(str,ID2W(IDS_DELETEDATABASE),MB_ICONQUESTION|MB_YESNO);
			if (nRet==IDYES)
				FileSystem::Remove(pDatabase->GetArchiveName());
		}
	}
	m_pList->DeleteItem(nItem);

	EnableButtons();
	m_pList->SetFocus();
}

void CSettingsProperties::CDatabasesSettingsPage::OnEnable(BOOL bEnable)
{
	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	if (nItem==-1)
		return;

	CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(nItem);
	ASSERT(pDatabase!=NULL);

	pDatabase->Enable(bEnable);
	m_pList->SetCheckState(nItem,bEnable);

	EnableDlgItem(IDC_ENABLE,!bEnable);
	EnableDlgItem(IDC_DISABLE,bEnable);

	m_pList->SetFocus();
}

BOOL CSettingsProperties::CDatabasesSettingsPage::ItemUpOrDown(BOOL bUp)
{
	int nSelected=m_pList->GetNextItem(-1,LVNI_SELECTED);
	ASSERT(nSelected!=-1);
	CDatabase* pSelected=(CDatabase*)m_pList->GetItemData(nSelected);
	
	int nOther=m_pList->GetNextItem(nSelected,bUp?LVNI_ABOVE:LVNI_BELOW);
	if (nOther==-1 || nOther==nSelected)
	{
		if (bUp && pSelected->GetThreadId()>0)
			return IncreaseThread(nSelected,pSelected,TRUE);
		else if (!bUp && pSelected->GetThreadId()<m_nThreadsCurrently-1)
			return IncreaseThread(nSelected,pSelected,FALSE);
		return FALSE;
	}

	CDatabase* pOther=(CDatabase*)m_pList->GetItemData(nOther);
	if (pOther->GetThreadId()!=pSelected->GetThreadId())
	{
		ASSERT(bUp?pSelected->GetThreadId()>0:pSelected->GetThreadId()<m_nThreadsCurrently-1);
		return IncreaseThread(nSelected,pSelected,bUp);
	}

	// This is working in this dialog! Wou
	LPARAM pParam=m_pList->GetItemData(nSelected);
	m_pList->SetItemData(nSelected,m_pList->GetItemData(nOther));
	m_pList->SetItemData(nOther,pParam);
	UINT nState=m_pList->GetItemState(nSelected,0xFFFFFFFF);
	m_pList->SetItemState(nSelected,m_pList->GetItemState(nOther,0xFFFFFFFF),0xFFFFFFFF);
	m_pList->SetItemState(nOther,nState,0xFFFFFFFF);

	m_pList->EnsureVisible(nOther,FALSE);
	m_pList->RedrawItems(min(nSelected,nOther),max(nSelected,nOther));

	m_pList->UpdateWindow();
	
	EnableButtons();
	m_pList->SetFocus();
	return TRUE;
}

BOOL CSettingsProperties::CDatabasesSettingsPage::IncreaseThread(int nItem,CDatabase* pDatabase,BOOL bDecrease)
{
	if ((bDecrease && pDatabase->GetThreadId()<1) || 
		(!bDecrease && pDatabase->GetThreadId()>=m_nThreadsCurrently-1))
		return FALSE;

	pDatabase->SetThreadId(pDatabase->GetThreadId()+(bDecrease?-1:1));

	if (GetLocateApp()->m_wComCtrlVersion>=0x0600)
	{
		LVITEM li;
		li.mask=LVIF_GROUPID;
		li.iItem=nItem;
		li.iSubItem=0;
		li.iGroupId=pDatabase->GetThreadId();
		m_pList->SetItem(&li);
	}
	else
		m_pList->RedrawItems(nItem,nItem);
	
	m_pList->EnsureVisible(nItem,FALSE);
	m_pList->SetFocus();
	EnableButtons();
	return TRUE;
}

int CALLBACK CSettingsProperties::CDatabasesSettingsPage::ThreadSortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (((CDatabase*)lParam1)->GetThreadId()>((CDatabase*)lParam2)->GetThreadId())
		return 1;
	if (((CDatabase*)lParam1)->GetThreadId()<((CDatabase*)lParam2)->GetThreadId())
		return -1;
	return 0;
}

void CSettingsProperties::CDatabasesSettingsPage::OnUpdate()
{
	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	if (nItem==-1)
		return;

	CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(nItem);
	pDatabase->UpdateGlobally(!pDatabase->IsGloballyUpdated());
	m_pList->RedrawItems(nItem,nItem);

	m_pList->SetFocus();
}

void CSettingsProperties::CDatabasesSettingsPage::OnRestore()
{
	// Removing current items
	m_pList->DeleteAllItems();

	// Loading databases from registry
	ASSERT(m_pSettings->m_aDatabases.GetSize()==0);
	
	CDatabase::LoadFromRegistry(HKCU,CLocateApp::GetRegKey("Databases"),m_pSettings->m_aDatabases);

	// If there is still no any available database, try to load old style db
	if (m_pSettings->m_aDatabases.GetSize()==0)
	{
		CDatabase* pDatabase=CDatabase::FromOldStyleDatabase(HKCU,"Software\\Update\\Database");
		if (pDatabase==NULL)
		{
			CStringW sApp(GetApp()->GetExeNameW());
			pDatabase=CDatabase::FromDefaults(TRUE); 
		}
		else
		{
			if (CDatabase::SaveToRegistry(HKCU,CLocateApp::GetRegKey("Databases"),&pDatabase,1))
				CRegKey::DeleteKey(HKCU,"Software\\Update\\Database");
		}
		m_pSettings->m_aDatabases.Add(pDatabase);
	}
	
	
	// Databases are not anymore overridden
	m_pSettings->ClearSettingsFlags(CSettingsProperties::settingsDatabasesOverridden);
	
	
	// Setting databases to list
	SetDatabasesToList();
	
	// Enabling dlg buttons
	EnableDlgItem(IDC_NEW,TRUE);
	
	ShowDlgItem(IDC_OVERRIDETXT,swHide);
	ShowDlgItem(IDC_RESTORE,swHide);
	
	ShowDlgItem(IDC_THREADS,swShow);
	ShowDlgItem(IDC_THREADSPIN,swShow);
	EnableDlgItem(IDC_THREADS,TRUE);
	EnableDlgItem(IDC_THREADSPIN,TRUE);
	
	EnableButtons();
	m_pList->SetFocus();
}

void CSettingsProperties::CDatabasesSettingsPage::OnThreads()
{
	int nThreads=GetDlgItemInt(IDC_THREADS);

	if (nThreads<1)
		SetDlgItemInt(IDC_THREADS,nThreads=1,FALSE);

	ChangeNumberOfThreads(nThreads);
	
	EnableButtons();
}

BOOL CSettingsProperties::CDatabasesSettingsPage::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	switch (idCtrl)
	{
	case IDC_DATABASES:
		return ListNotifyHandler((NMLISTVIEW*)pnmh);
	}
	return CPropertyPage::OnNotify(idCtrl,pnmh);
}

void CSettingsProperties::CDatabasesSettingsPage::OnImport()
{
	// Set wait cursor
	CWaitCursor wait;
	
	// Initializing file name dialog
	CFileDialog fd(TRUE,L"*",szwEmpty,OFN_EXPLORER|OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_ENABLESIZING,IDS_IMPORTDATABASEFILTERS);
	fd.EnableFeatures();
	fd.SetTitle(ID2W(IDS_IMPORTDATABASESETTINGS));
	
	
	// Ask file name
	if (!fd.DoModal(*this))
		return;

	// First, check whether file is database and read information
	CDatabase* pDatabase;
	CStringW Path;
	fd.GetFilePath(Path);
	CDatabaseInfo* pDatabaseInfo=CDatabaseInfo::GetFromFile(Path);
	if (pDatabaseInfo!=NULL)
	{
		if (!pDatabaseInfo->sExtra2.IsEmpty())
			pDatabase=CDatabase::FromExtraBlock(pDatabaseInfo->sExtra2);
		if (pDatabase==NULL && !pDatabaseInfo->sExtra1.IsEmpty())
			pDatabase=CDatabase::FromExtraBlock(pDatabaseInfo->sExtra1);

		if (pDatabase!=NULL)
		{
			pDatabase->SetArchiveType(CDatabase::archiveFile);
			pDatabase->SetArchiveName(Path);
		}
	}
	else
	{
		CFile* pFile=NULL;
		char* pFileContent=NULL;
		BOOL bError=FALSE;

		try {
			CStringW Path;
			fd.GetFilePath(Path);
			pFile=new CFile(Path,CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
			pFile->CloseOnDelete();

			DWORD dwLength=pFile->GetLength();
			pFileContent=new char[dwLength+1];
			pFile->Read(pFileContent,dwLength);
			pFileContent[dwLength]='\0';
			pFile->Close();
		}
		catch (...)
		{
			bError=TRUE;
		}
        
		if (!bError)
			pDatabase=CDatabase::FromExtraBlock(A2W(pFileContent));

		if (pFile!=NULL)
			delete pFile;
		if (pFileContent!=NULL)
			delete[] pFileContent;
	}

	if (pDatabase==NULL)
	{
		CStringW msg,path;
		fd.GetFilePath(path);
		msg.Format(IDS_UNABLEREADSETTINGS,(LPCWSTR)path);
		MessageBox(msg,ID2W(IDS_ERROR),MB_OK|MB_ICONERROR);
		return;
	}

	if (pDatabase->GetThreadId()>=m_nThreadsCurrently)
	{
		if (MessageBox(ID2W(IDS_INCREASETHREADCOUNT),ID2W(IDS_IMPORTDATABASESETTINGS),MB_ICONQUESTION|MB_YESNO)==IDYES)
		{
			ChangeNumberOfThreads(pDatabase->GetThreadId()+1);
			SetDlgItemInt(IDC_THREADS,pDatabase->GetThreadId()+1,FALSE);
		}
		else
			pDatabase->SetThreadId(0);
	}

	OnNew(pDatabase);
}

void CSettingsProperties::CDatabasesSettingsPage::OnExport()
{
	int iItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	if (iItem==-1)
		return;
    
	CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(iItem);
	if (pDatabase==NULL)
		return;

	// Set wait cursor
	CWaitCursor wait;
	
	LPCWSTR pCurFile=szwEmpty;
	if (pDatabase->GetArchiveType()==CDatabase::archiveFile)
		pCurFile=pDatabase->GetArchiveName();

	// Initializing file name dialog
	CFileDialog fd(FALSE,L"*",pCurFile,OFN_EXPLORER|OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_ENABLESIZING,
		IDS_EXPORTDATABASEFILTERS);
	fd.EnableFeatures();
	fd.SetTitle(ID2W(IDS_EXPORTDATABASESETTINGS));
	
	
	// Ask file name
	if (!fd.DoModal(*this))
		return;
	
	CStringW Path;
	fd.GetFilePath(Path);
	
	if (FileSystem::IsFile(Path))
	{
		if (pDatabase->SaveExtraBlockToDbFile(Path))
			return;

		CStringW msg;
		msg.Format(IDS_FILEISNOTDATABASE,LPCWSTR(Path));
		if (MessageBox(msg,ID2W(IDS_EXPORTDATABASESETTINGS),MB_ICONQUESTION|MB_YESNO)==IDNO)
			return;			
	}

	CFile* pFile=NULL;
	LPWSTR pExtra=pDatabase->ConstructExtraBlock();
	DWORD dwExtraLen=(int)istrlenw(pExtra);

	try {
		pFile=new CFile(Path,CFile::defWrite,TRUE);
		pFile->CloseOnDelete();
		pFile->Write(pExtra,dwExtraLen);
	}
	catch (...)
	{
		MessageBox(ID2A(IDS_CANNOTWRITESETTINGS),szError,MB_ICONERROR|MB_OK);
	}
	if (pFile!=NULL)
		delete pFile;
	delete[] pExtra;
    
}

BOOL CSettingsProperties::CDatabasesSettingsPage::ListNotifyHandler(NMLISTVIEW *pNm)
{
	switch(pNm->hdr.code)
	{
	case LVN_ITEMCHANGING:
		if (m_pSettings->IsSettingsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
			return TRUE;
		return FALSE;
	case LVN_ITEMCHANGED:
		if (pNm->lParam!=NULL && (pNm->uNewState&0x00002000)!=(pNm->uOldState&0x00002000))
		{
			if (m_pSettings->IsSettingsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
			{
				m_pList->SetCheckState(pNm->iItem,((CDatabase*)pNm->lParam)->IsEnabled());
				m_pList->SetItemState(pNm->iItem,0,LVIS_SELECTED);
			}
			else
				((CDatabase*)pNm->lParam)->Enable(m_pList->GetCheckState(pNm->iItem));

			EnableButtons();
		}	
		break;
	case NM_CLICK:
		EnableButtons();
		break;
	case NM_DBLCLK:
		OnEdit();
		break;
	case LVN_DELETEITEM:
		if (pNm->lParam!=NULL)
			delete (CDatabase*)pNm->lParam;
		break;
	case LVN_GETDISPINFOA:
		{
			LV_DISPINFO *pLvdi=(LV_DISPINFO*)pNm;

			CDatabase* pDatabase=(CDatabase*)pLvdi->item.lParam;
			if (pDatabase==NULL)
				break;
				
			ISDLGTHREADOK
			if (g_szBuffer!=NULL)
				delete[] g_szBuffer;
			
			switch (pLvdi->item.iSubItem)
			{
			case 0:
				if (!m_pSettings->IsSettingsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
					pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(pDatabase->GetName());
				else
				{
					g_szBuffer=new char[40];
					LoadString(IDS_COMMANDLINEARGUMENT,g_szBuffer,40);
					pLvdi->item.pszText=g_szBuffer;
				}
				break;
			case 1:
				pLvdi->item.pszText=g_szBuffer=alloccopyWtoA(pDatabase->GetArchiveName());
				break;
			case 2:
				g_szBuffer=new char[100];
				LoadString(pDatabase->IsGloballyUpdated()?IDS_YES:IDS_NO,g_szBuffer,100);
				pLvdi->item.pszText=g_szBuffer;
				break;
			case 3:
				g_szBuffer=new char[20];
				_itoa_s(pDatabase->GetThreadId()+1,g_szBuffer,20,10);
				pLvdi->item.pszText=g_szBuffer;
				break;
			}
			break;
		}
	case LVN_GETDISPINFOW:
		{
			LV_DISPINFOW *pLvdi=(LV_DISPINFOW*)pNm;

			CDatabase* pDatabase=(CDatabase*)pLvdi->item.lParam;
			if (pDatabase==NULL)
				break;
				
			switch (pLvdi->item.iSubItem)
			{
			case 0:
				if (!m_pSettings->IsSettingsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
					pLvdi->item.pszText=const_cast<LPWSTR>(pDatabase->GetName());
				else
				{
					ISDLGTHREADOK
					if (g_szwBuffer!=NULL)
						delete[] g_szwBuffer;
					g_szwBuffer=new WCHAR[40];
					LoadString(IDS_COMMANDLINEARGUMENT,g_szwBuffer,40);
					pLvdi->item.pszText=g_szwBuffer;
				}
				break;
			case 1:
				pLvdi->item.pszText=const_cast<LPWSTR>(pDatabase->GetArchiveName());
				break;
			case 2:
				if (g_szwBuffer!=NULL)
					delete[] g_szwBuffer;
				g_szwBuffer=new WCHAR[100];
				LoadString(pDatabase->IsGloballyUpdated()?IDS_YES:IDS_NO,g_szwBuffer,100);
				pLvdi->item.pszText=g_szwBuffer;
				break;
			case 3:
				if (g_szwBuffer!=NULL)
					delete[] g_szwBuffer;
				g_szwBuffer=new WCHAR[20];
				_itow_s(pDatabase->GetThreadId()+1,g_szwBuffer,20,10);
				pLvdi->item.pszText=g_szwBuffer;
				break;
			}
			break;
		}
	}
	return 0;
}
	


BOOL CSettingsProperties::CDatabasesSettingsPage::OnApply()
{
	CPropertyPage::OnApply();

	ASSERT(m_pSettings->m_aDatabases.GetSize()==0);

	// Get the first item
	//int nNext;
	int nItem=m_pList->GetNextItem(-1,LVNI_ALL);

	/*while ((nNext=m_pList->GetNextItem(nItem,LVNI_ABOVE))!=-1)
	{
		if (nNext==nItem)
			break; // This should not be like that, why is it?
		nItem=nNext;
	}*/
	
	while (nItem!=-1)
	{
		PDATABASE pDatabase=(PDATABASE)m_pList->GetItemData(nItem);
		ASSERT(pDatabase!=NULL);

		m_pSettings->m_aDatabases.Add(pDatabase);
		m_pList->SetItemData(nItem,NULL);

		/*nNext=m_pList->GetNextItem(nItem,LVNI_BELOW);
		if (nNext==nItem)
			break;
		nItem=nNext;*/

		nItem=m_pList->GetNextItem(nItem,LVNI_ALL);
	}
	
	return TRUE;
}

void CSettingsProperties::CDatabasesSettingsPage::OnCancel()
{
	m_pSettings->SetSettingsFlags(CSettingsProperties::settingsCancelled);


	CPropertyPage::OnCancel();
}

void CSettingsProperties::CDatabasesSettingsPage::EnableThreadGroups(int nThreadGroups)
{
	if (m_pList->IsGroupViewEnabled())
		return;

	m_pList->EnableGroupView(TRUE);
	m_nThreadsCurrently=nThreadGroups;
	
	// Creating groups
	CStringW str;
	LVGROUP lg;
	dMemSet(&lg,0,sizeof(LVGROUP));
	lg.cbSize=sizeof(LVGROUP);
	lg.mask=LVGF_HEADER|LVGF_GROUPID|LVGF_ALIGN;
	lg.state=LVGS_NORMAL;
	lg.uAlign=LVGA_HEADER_LEFT;

	for (lg.iGroupId=0;lg.iGroupId<nThreadGroups;lg.iGroupId++)
	{
		str.Format(IDS_THREADNAME,lg.iGroupId+1);
		lg.pszHeader=str.GetBuffer();

		m_pList->InsertGroup(lg.iGroupId,&lg);
	}

	// Setting groups IDs
	if (m_pList->GetItemCount()>0)
	{
		LVITEM li;
		li.mask=LVIF_GROUPID;
		li.iItem=m_pList->GetNextItem(-1,LVNI_ALL);
		li.iSubItem=0;
		while (li.iItem!=-1)
		{
			CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(li.iItem);
			ASSERT(pDatabase!=NULL);

			li.iGroupId=pDatabase->GetThreadId();
			
			m_pList->SetItem(&li);
			li.iItem=m_pList->GetNextItem(li.iItem,LVNI_ALL);
		}
	}
}

void CSettingsProperties::CDatabasesSettingsPage::RemoveThreadGroups()
{
	if (!m_pList->IsGroupViewEnabled())
		return;

	m_pList->RemoveAllGroups();
	m_pList->EnableGroupView(FALSE);
}

void CSettingsProperties::CDatabasesSettingsPage::ChangeNumberOfThreads(int nThreads)
{
	ASSERT(nThreads>=1);
	
	if (nThreads>m_nThreadsCurrently)
	{
		// Number is increased
		if (((CLocateApp*)GetApp())->m_wComCtrlVersion<0x0600)
		{
			m_nThreadsCurrently=nThreads;
			return;
		}

		if (m_nThreadsCurrently==1)
			EnableThreadGroups(nThreads);
		else
		{
			// Insertig new thread groups
			CStringW str;
			LVGROUP lg;
			dMemSet(&lg,0,sizeof(LVGROUP));
			lg.cbSize=sizeof(LVGROUP);
			lg.mask=LVGF_HEADER|LVGF_GROUPID|LVGF_ALIGN;
			lg.state=LVGS_NORMAL;
			lg.uAlign=LVGA_HEADER_LEFT;

			for (lg.iGroupId=m_nThreadsCurrently;lg.iGroupId<nThreads;lg.iGroupId++)
			{
				str.Format(IDS_THREADNAME,lg.iGroupId+1);
				lg.pszHeader=str.GetBuffer();

				m_pList->InsertGroup(lg.iGroupId,&lg);
			}
		
		}
		m_nThreadsCurrently=nThreads;
		m_pList->RedrawItems(0,m_pList->GetItemCount());
	}
	else if (nThreads<m_nThreadsCurrently)
	{
		// Ensuring that there is no any items with higher thread ID than available
		if (m_pList->GetItemCount()>0)
		{
			int nItem=m_pList->GetNextItem(-1,LVNI_ALL);
			while (nItem!=-1)
			{
				CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(nItem);
				ASSERT(pDatabase!=NULL);

				if (pDatabase->GetThreadId()>=nThreads)
				{
					pDatabase->SetThreadId(nThreads-1);
					
					LVITEM li;
					li.iItem=nItem;
					li.iSubItem=0;
					li.mask=LVIF_GROUPID;
					li.iGroupId=nThreads-1;
					m_pList->SetItem(&li);
				}
				nItem=m_pList->GetNextItem(nItem,LVNI_ALL);
			}
		}

		if (((CLocateApp*)GetApp())->m_wComCtrlVersion>=0x0600)
		{
			if (nThreads==1)
				RemoveThreadGroups();
			else
			{
				// Removing unused thread groups
				while (m_nThreadsCurrently>nThreads)
					m_pList->RemoveGroup(--m_nThreadsCurrently);
			}
		}
		m_nThreadsCurrently=nThreads;

		m_pList->RedrawItems(0,m_pList->GetItemCount());
	}
}

void CSettingsProperties::CDatabasesSettingsPage::EnableButtons()
{
	int nSelectedItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	
	BOOL bEnable=nSelectedItem!=-1 && !m_pSettings->IsSettingsFlagSet(CSettingsProperties::settingsDatabasesOverridden);
	
	EnableDlgItem(IDC_EDIT,nSelectedItem!=-1);
	EnableDlgItem(IDC_REMOVE,bEnable);
	EnableDlgItem(IDC_UPDATE,bEnable);
	
	
	EnableDlgItem(IDC_EXPORT,bEnable);
	EnableDlgItem(IDC_IMPORT,!m_pSettings->IsSettingsFlagSet(CSettingsProperties::settingsDatabasesOverridden));
	
	if (bEnable)
	{
		CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(nSelectedItem);
		ASSERT(pDatabase!=NULL);

		EnableDlgItem(IDC_ENABLE,!pDatabase->IsEnabled());
		EnableDlgItem(IDC_DISABLE,pDatabase->IsEnabled());

		// Checking item above
		int nAnother=m_pList->GetNextItem(nSelectedItem,LVNI_ABOVE);
		if (nAnother==-1 || nAnother==nSelectedItem)
			EnableDlgItem(IDC_UP,pDatabase->GetThreadId()>0);
		else
			EnableDlgItem(IDC_UP,TRUE);
		
		// Checking item below
		nAnother=m_pList->GetNextItem(nSelectedItem,LVNI_BELOW);
		if (nAnother==-1 || nAnother==nSelectedItem)
			EnableDlgItem(IDC_DOWN,pDatabase->GetThreadId()<m_nThreadsCurrently-1); 
		else
			EnableDlgItem(IDC_DOWN,TRUE);
	}
	else
	{
		EnableDlgItem(IDC_ENABLE,FALSE);
		EnableDlgItem(IDC_DISABLE,FALSE);
		EnableDlgItem(IDC_UP,FALSE);
		EnableDlgItem(IDC_DOWN,FALSE);
	}
}
	
/////////////////////////////////////////////////
// CDatabasesSettingsPage::CDatabaseDialog

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	
	// Initializing list control
	m_pList=new CListCtrl(GetDlgItem(IDC_FOLDERS));
	CLocateDlg::SetSystemImagelists(m_pList);
	m_pList->SetExtendedListViewStyle(LVS_EX_CHECKBOXES|LVS_EX_HEADERDRAGDROP,
		LVS_EX_CHECKBOXES|LVS_EX_HEADERDRAGDROP);
	if (IsUnicodeSystem())
		m_pList->SetUnicodeFormat(TRUE);
	m_pList->InsertColumn(0,ID2W(IDS_VOLUMELABEL),LVCFMT_LEFT,95,0);
	m_pList->InsertColumn(1,ID2W(IDS_VOLUMEPATH),LVCFMT_LEFT,75,1);
	m_pList->InsertColumn(2,ID2W(IDS_VOLUMETYPE),LVCFMT_LEFT,70,2);
	m_pList->InsertColumn(3,ID2W(IDS_VOLUMEFILESYSTEM),LVCFMT_LEFT,65,3);
	m_pList->LoadColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\Dialogs","Database Dialog List Widths");
	
	for (int i=1;i<=m_nMaximumNumbersOfThreads;i++)
	{
		char num[5];
		_itoa_s(i,num,5,10);
		SendDlgItemMessage(IDC_USEDTHREAD,CB_ADDSTRING,0,LPARAM(num));
	}

    // Setting current information	
	if (m_bDontEditName)
	{
		EnableDlgItem(IDC_NAME,FALSE);
		SetDlgItemText(IDC_NAME,ID2A(IDS_COMMANDLINEARGUMENT));
	}
	else
		SetDlgItemText(IDC_NAME,m_pDatabase->GetName());
	SetDlgItemText(IDC_DBFILE,m_pDatabase->GetArchiveName());
	SetDlgItemText(IDC_CREATOR,m_pDatabase->GetCreator());
	SetDlgItemText(IDC_DESCRIPTION,m_pDatabase->GetDescription());
	SendDlgItemMessage(IDC_USEDTHREAD,CB_SETCURSEL,m_pDatabase->GetThreadId());

	CheckDlgButton(IDC_ENABLE,m_pDatabase->IsEnabled());
	CheckDlgButton(IDC_GLOBALUPDATE,m_pDatabase->IsGloballyUpdated());
	CheckDlgButton(IDC_STOPIFROOTUNAVAILABLE,m_pDatabase->IsFlagged(CDatabase::flagStopIfRootUnavailable));
	CheckDlgButton(IDC_INCREMENTALUPDATE,m_pDatabase->IsFlagged(CDatabase::flagIncrementalUpdate));
	
	if (IsUnicodeSystem())
		CheckDlgButton(IDC_UNICODE,!m_pDatabase->IsFlagged(CDatabase::flagAnsiCharset));
	else
		EnableDlgItem(IDC_UNICODE,FALSE);

	// Inserting local drives to drive list
	DWORD nLength=GetLogicalDriveStrings(0,NULL)+1;
	if (nLength<2)
		return FALSE;

	WCHAR* szDrives=new WCHAR[nLength+1];
	FileSystem::GetLogicalDriveStrings(nLength,szDrives);
	for (LPWSTR szDrive=szDrives;szDrive[0]!=L'\0';szDrive+=4)
		AddDriveToList(szDrive);
	delete[] szDrives;

	
	
	// Setting local drives
	if (m_pDatabase->GetRoots()!=NULL)
	{
		CheckDlgButton(IDC_CUSTOMDRIVES,1);
		EnableDlgItem(IDC_FOLDERS,TRUE);
		EnableDlgItem(IDC_ADDFOLDER,TRUE);
		EnableDlgItem(IDC_REMOVEFOLDER,EnableRemoveButton());

		LPCWSTR pPtr=m_pDatabase->GetRoots();

		while (*pPtr!='\0')
		{
			DWORD dwLength=istrlenw(pPtr);

			if (dwLength>2)
			{
				if (pPtr[0]=='\\' && pPtr[1]=='\\')
				{
					if (!FileSystem::IsDirectory(pPtr))
						AddComputerToList(pPtr);
					else
						AddDirectoryToList(pPtr,dwLength);
				}			
				else
					AddDirectoryToList(pPtr,dwLength);
			}
			else if (pPtr[1]==':')
			{
				// Checking whether we point unavailable drive
				WCHAR root[]=L"X:\\";
				root[0]=pPtr[0];
				UINT uRet=FileSystem::GetDriveType(root);
				if (uRet==DRIVE_UNKNOWN || uRet==DRIVE_NO_ROOT_DIR)
					AddDriveToList(root); // Unavailable
				
			}
			pPtr+=dwLength+1;
		}
	}
	else
	{
		CheckDlgButton(IDC_LOCALDRIVES,1);
		EnableDlgItem(IDC_FOLDERS,FALSE);
		EnableDlgItem(IDC_ADDFOLDER,FALSE);
		EnableDlgItem(IDC_REMOVEFOLDER,FALSE);
	}
	return FALSE;
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnDestroy()
{
	if (m_pList!=NULL)
	{
		m_pList->SaveColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\Dialogs","Database Dialog List Widths");
		delete m_pList;
		m_pList=NULL;
	}
	CDialog::OnDestroy();
}

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnClose()
{
	CDialog::OnClose();
	EndDialog(0);
	return 0;
}

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	switch (idCtrl)
	{
	case IDC_FOLDERS:
		ListNotifyHandler((NMLISTVIEW*)pnmh);
		break;
	}
	return CDialog::OnNotify(idCtrl,pnmh);
}

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::ListNotifyHandler(NMLISTVIEW *pNm)
{
	switch(pNm->hdr.code)
	{
	case NM_CLICK:
		EnableDlgItem(IDC_REMOVEFOLDER,EnableRemoveButton());
		break;
	}
	return TRUE;
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnOK()
{
	// Setting name
	int iLength=GetDlgItemTextLength(IDC_NAME)+1;
	WCHAR* pText=new WCHAR[iLength];
	GetDlgItemText(IDC_NAME,pText,iLength);
	
	if (!m_bDontEditName)
	{
		if (wcsncmp(pText,L"DEFAULTX",8)==0 || wcsncmp(pText,L"PARAMX",6)==0)
		{
			CStringW msg;
			msg.Format(IDS_INVALIDDBNAME,pText);
			MessageBox(msg,ID2W(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
			SetFocus(IDC_NAME);
			delete[] pText;
			return;
		}
		if (!CDatabase::IsNameValid(pText))
		{
			CStringW msg;
			msg.Format(IDS_INVALIDDBNAME,pText);
			MessageBox(msg,ID2W(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
			SetFocus(IDC_NAME);
			delete[] pText;
			return;
		}
		if (CDatabase::FindByName(m_aOtherDatabases,pText)!=NULL)
		{
			CStringW msg;
			msg.Format(IDS_NAMEALREADYEXISTS,pText);
			MessageBox(msg,ID2W(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
			SetFocus(IDC_NAME);
			delete[] pText;
			return;
		}
		m_pDatabase->SetNamePtr(pText);
	}
	

    // Setting db file
	m_pDatabase->SetArchiveType(CDatabase::archiveFile);
	iLength=GetDlgItemTextLength(IDC_DBFILE)+1;
	if (iLength==2) // Specified text is too short
	{
		CStringW msg;
		msg.Format(IDS_INVALIDFILENAME,"");
		MessageBox(msg,ID2W(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
		SetFocus(IDC_DBFILE);
		return;
	}
	pText=new WCHAR[iLength];
	GetDlgItemText(IDC_DBFILE,pText,iLength);
	m_pDatabase->SetArchiveNamePtr(CDatabase::GetCorrertFileName(pText));
	
	if (m_pDatabase->GetArchiveName()==NULL)
	{
		// Path was not ok, is this intended
		CStringW msg;
		msg.Format(IDS_INVALIDFILENAMEISOK,pText);
		if (MessageBox(msg,ID2W(IDS_DATABASESETTINGS),MB_YESNO|MB_ICONINFORMATION)==IDNO)
		{
			SetFocus(IDC_DBFILE);
			delete[] pText;
			return;
		}

		m_pDatabase->SetArchiveNamePtr(pText);
	}
	else
		delete[] pText;

	if (CDatabase::FindByFile(m_aOtherDatabases,m_pDatabase->GetArchiveName())!=NULL)
	{
		CStringW msg;
		msg.Format(IDS_FILEALREADYEXISTS,m_pDatabase->GetArchiveName());
		MessageBox(msg,ID2W(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
		SetFocus(IDC_DBFILE);
		return;
	}
	
    // Setting creator
	iLength=GetDlgItemTextLength(IDC_CREATOR)+1;
	pText=new WCHAR[iLength];
	GetDlgItemText(IDC_CREATOR,pText,iLength);
	m_pDatabase->SetCreatorPtr(pText);

	// Setting description
	iLength=GetDlgItemTextLength(IDC_DESCRIPTION)+1;
	pText=new WCHAR[iLength];
	GetDlgItemText(IDC_DESCRIPTION,pText,iLength);
	m_pDatabase->SetDescriptionPtr(pText);
	

	// Settings flags
	m_pDatabase->Enable(IsDlgButtonChecked(IDC_ENABLE));
	m_pDatabase->UpdateGlobally(IsDlgButtonChecked(IDC_GLOBALUPDATE));
	m_pDatabase->SetFlag(CDatabase::flagStopIfRootUnavailable,IsDlgButtonChecked(IDC_STOPIFROOTUNAVAILABLE));
	m_pDatabase->SetFlag(CDatabase::flagIncrementalUpdate,IsDlgButtonChecked(IDC_INCREMENTALUPDATE));
	m_pDatabase->SetFlag(CDatabase::flagAnsiCharset,!IsDlgButtonChecked(IDC_UNICODE));

	// Setting thread ID
	m_pDatabase->SetThreadId((WORD)SendDlgItemMessage(IDC_USEDTHREAD,CB_GETCURSEL));
    
	if (IsDlgButtonChecked(IDC_CUSTOMDRIVES))
	{
		CArrayFAP<LPWSTR> aRoots;
		
		LVITEMW li;
		li.iItem=m_pList->GetNextItem(-1,LVNI_ALL);
		while (li.iItem!=-1)
		{
			// Not selected
			if (!m_pList->GetCheckState(li.iItem))
			{
				li.iItem=m_pList->GetNextItem(li.iItem,LVNI_ALL);
				continue;
			}
			
			BOOL bDoNotAdd=FALSE;
			WCHAR* szPath=new WCHAR[MAX_PATH];
			li.mask=LVIF_TEXT;
			li.iSubItem=1;
			li.pszText=szPath;
			li.cchTextMax=MAX_PATH;
			m_pList->GetItem(&li);
			DWORD nPathLen=istrlenw(szPath);
			for (int i=0;i<aRoots.GetSize();i++)
			{
				LPWSTR pAddedPath=aRoots.GetAt(i);
				if (FileSystem::IsSubDirectory(szPath,pAddedPath))
				{
					CStringW str;
					str.Format(IDS_SUBFOLDER,szPath,pAddedPath,szPath);
					MessageBox(str,ID2W(IDS_DATABASESETTINGS),MB_ICONINFORMATION|MB_OK);
					bDoNotAdd=TRUE;
					break;
				}
				else if (FileSystem::IsSubDirectory(pAddedPath,szPath))
				{
					CStringW str;
					str.Format(IDS_SUBFOLDER,pAddedPath,szPath,pAddedPath);
					MessageBox(str,ID2W(IDS_DATABASESETTINGS),MB_ICONINFORMATION|MB_OK);
					aRoots.RemoveAt(i--);
					continue;
				}
			}
			if (!bDoNotAdd)
				aRoots.Add(szPath);
			else
				delete[] szPath;
			li.iItem=m_pList->GetNextItem(li.iItem,LVNI_ALL);
		}

		if (aRoots.GetSize()==0)
		{
			ShowErrorMessage(IDS_ERRORNODRIVES,IDS_DATABASESETTINGS);
			return;
		}
		m_pDatabase->SetRoots(aRoots);
	}
	else
		m_pDatabase->SetRootsPtr(NULL);

	EndDialog(1);
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnAddFolder()
{
	CWaitCursor wait;
	
	// Ask folder
	CFolderDialog fd(IDS_ADDFOLDER,BIF_RETURNONLYFSDIRS|BIF_USENEWUI);
	if (fd.DoModal(*this))
	{
		// Insert folder to list
		CStringW Folder;
		if (fd.GetFolder(Folder))
		{
			WCHAR szNetHood[MAX_PATH];
			switch (GetNethoodTarget(Folder,szNetHood,MAX_PATH))
			{
			case 1:
				AddComputerToList(szNetHood);
				break;
			case 2:
				AddDirectoryToListWithVerify(szNetHood);
				break;
			case 0:
				AddDirectoryToListWithVerify(Folder);
				break;
			}
		}
		else
		{
			// Checking type of folder
			IShellFolder *psf=NULL;
			
			try {
				HRESULT hRes=SHGetDesktopFolder(&psf);
	            if (!SUCCEEDED(hRes))
					throw COleException(hRes);
				
					
				SHDESCRIPTIONID di;
				hRes=SHGetDataFromIDList(psf,fd.m_lpil,SHGDFIL_DESCRIPTIONID,&di,sizeof(SHDESCRIPTIONID));
				if (!SUCCEEDED(hRes))
					throw COleException(hRes);

				
			
				if (di.clsid!=CLSID_NetworkPlaces)
					throw FALSE;

				

				STRRET str;
				LPWSTR pComputer;
				hRes=psf->GetDisplayNameOf(fd.m_lpil,SHGDN_FORPARSING,&str);
				if (FAILED(hRes))
					throw COleException(hRes);
				
				pComputer=StrRetToPtrW(str,fd.m_lpil);
				if (pComputer==NULL)
					throw COleException(hRes);
					
			
				if (pComputer[0]!='\\' && pComputer[1]!='\\')
				{
					if (pComputer[0]==':' && pComputer[1]==':')
						ShowErrorMessage(IDS_ERRORCANNOTADDITEM,IDS_ERROR,MB_ICONERROR|MB_OK);
					else
					{
						CStringW s;
						s.Format(IDS_ERRORCANNOTADDITEM2,pComputer);
						MessageBox(s,ID2W(IDS_ERROR),MB_ICONERROR|MB_OK);
					}
				}
				else
					AddComputerToList(pComputer);
				
				delete[] pComputer;
		
			}
	#ifdef _DEBUG_LOGGING
			catch (COleException exp)
			{
				char error[1000];
				exp.GetErrorMessage(error,1000);
				DebugFormatMessage("CDatabaseDialog::OnAddFolder() throwed OLE exception: %s",error);
				ShowErrorMessage(IDS_ERRORCANNOTADDITEM,IDS_ERROR,MB_ICONERROR|MB_OK);
			}
	#endif
			catch (...)
			{
				DebugMessage("CDatabaseDialog::OnAddFolder() throwed unknown exception");

				ShowErrorMessage(IDS_ERRORCANNOTADDITEM,IDS_ERROR,MB_ICONERROR|MB_OK);
			}

			if (psf!=NULL)
				psf->Release();

			
			
		}

		// Setting focus to list
		m_pList->SetFocus();
	}

	// Enable "Remove folder" button is needed
	EnableDlgItem(IDC_REMOVEFOLDER,EnableRemoveButton());
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnRemoveFolder()
{
	// Removes current item from list
	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	if (nItem!=-1)
		m_pList->DeleteItem(nItem);
	m_pList->SetFocus();


	// Enable "Remove folder" button is needed
	EnableDlgItem(IDC_REMOVEFOLDER,EnableRemoveButton());
}

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);
	switch (wID)
	{
	case IDOK:
	case IDC_OK:
		OnOK();
		break;
	case IDCANCEL:
	case IDC_CANCEL:
		EndDialog(0);
		break;
	case IDC_LOCALDRIVES:
		CheckDlgButton(IDC_CUSTOMDRIVES,0);
		EnableDlgItem(IDC_FOLDERS,FALSE);
		EnableDlgItem(IDC_ADDFOLDER,FALSE);
		EnableDlgItem(IDC_REMOVEFOLDER,FALSE);
		break;
	case IDC_CUSTOMDRIVES:
		CheckDlgButton(IDC_LOCALDRIVES,0);
		EnableDlgItem(IDC_FOLDERS,TRUE);
		EnableDlgItem(IDC_ADDFOLDER,TRUE);
		EnableDlgItem(IDC_REMOVEFOLDER,EnableRemoveButton());
		break;
	case IDC_BROWSE:
		OnBrowse();
		break;
	case IDC_ADDFOLDER:
		OnAddFolder();
		break;
	case IDC_REMOVEFOLDER:
		OnRemoveFolder();
		break;
	case IDC_EXCLUDEDIRECTORIES:
		OnExcludeDirectories();
		break;
	case IDC_NAME:
	case IDC_DBFILE:
	case IDC_CREATOR:
	case IDC_DESCRIPTION:
		if (wNotifyCode==EN_SETFOCUS)
			SendDlgItemMessage(wID,EM_SETSEL,0,-1);
		break;
	}
	return FALSE;
}


void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnBrowse()
{
	// Set wait cursor
	CWaitCursor wait;

	// Initializing file name dialog
	CStringW File;
	GetDlgItemText(IDC_DBFILE,File);

	CFileDialog fd(FALSE,L"*",File,OFN_EXPLORER|OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_ENABLESIZING,IDS_DATABASEFILTERS);
	fd.EnableFeatures();
	fd.SetTitle(ID2A(IDS_SELECTDATABASE));
	
	// Ask file name
	if (!fd.DoModal(*this))
		return;
	
	// Check filename and set
	fd.GetFilePath(File);
	int i=File.Find('*');
	if (i==-1)
		SetDlgItemText(IDC_DBFILE,File);
	else
	{
		if (File[i-1]=='.')
			i--;
		File.FreeExtra(i);
		SetDlgItemText(IDC_DBFILE,File);
	}

	// Set focus to file name edit box
	SetFocus(IDC_DBFILE);
	SendDlgItemMessage(IDC_DBFILE,EM_SETSEL,0,-1);
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnExcludeDirectories()
{
	CExcludeDirectoryDialog edd(m_pDatabase->GetExcludedFiles(),
		m_pDatabase->GetExcludedDirectories());
	
	if (edd.DoModal(*this))
	{
		m_pDatabase->SetExcludedFiles(edd.m_sFiles);
		m_pDatabase->SetExcludedDirectories(edd.m_aDirectories);
	}
}

int CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::AddDriveToList(LPWSTR szDrive)
{
	DWORD nType=FileSystem::GetDriveType(szDrive);
	//DebugFormatMessage(L"CSettingsProperties::CDatabaseSettingsPage::AddDriveToList(%s),type:%X",szDrive,nType);

	LVITEMW li;
	li.iItem=m_pList->GetItemCount();

	CStringW Temp;
	WCHAR szLabel[20],szFileSystem[20]=L"";
	switch (nType)
	{
	case DRIVE_FIXED:
		Temp.LoadString(IDS_VOLUMETYPEFIXED);
		break;
	case DRIVE_REMOVABLE:
		Temp.LoadString(IDS_VOLUMETYPEREMOVABLE);
		break;
	case DRIVE_CDROM:
		Temp.LoadString(IDS_VOLUMETYPECDROM);
		break;
	case DRIVE_REMOTE:
		Temp.LoadString(IDS_VOLUMETYPEREMOTE);
		break;
	case DRIVE_RAMDISK:
		Temp.LoadString(IDS_VOLUMETYPERAMDISK);
		break;
	case DRIVE_NO_ROOT_DIR:
	default:
		Temp.LoadString(IDS_VOLUMETYPEUNKNOWN);
		break;
	}

	// Resolving label
	
	
	if (nType==DRIVE_REMOVABLE)
        LoadString(IDS_FLOPPYSTRING,szLabel,20);
	else
	{
		DWORD dwTemp;
		UINT nOldMode=SetErrorMode(SEM_FAILCRITICALERRORS);
	
		if (!FileSystem::GetVolumeInformation(szDrive,szLabel,20,&dwTemp,&dwTemp,&dwTemp,szFileSystem,20))
		{
			switch (nType)
			{
			case DRIVE_CDROM:
				LoadString(IDS_CDROMSTRING,szLabel,20);
				break;
			default:	
				szLabel[0]='\0';
				break;
			}
			szFileSystem[0]='\0';
		}

		SetErrorMode(nOldMode);
	}

	// Resolving icon,
	if (GetLocateApp()->GetProgramFlags()&CLocateApp::pfUseDefaultIconForDirectories &&
		FileSystem::GetDriveType(szDrive)==DRIVE_REMOTE)
		li.iImage=DIR_IMAGE;
	else
	{
		SHFILEINFOW fi;
		if (GetFileInfo(szDrive,0,&fi,SHGFI_SMALLICON|SHGFI_SYSICONINDEX))
			li.iImage=fi.iIcon;
		else
			li.iImage=DEL_IMAGE;
	}
	// Label
	li.iSubItem=0;
	li.mask=LVIF_TEXT|LVIF_IMAGE;
	li.pszText=szLabel;
	m_pList->InsertItem(&li);

	// Path
	li.mask=LVIF_TEXT;
	li.iSubItem=1;
	szDrive[2]=L'\0';
	li.pszText=szDrive;
	m_pList->SetItem(&li);
	
	// Type
	li.pszText=Temp.GetBuffer();
	li.iSubItem=2;
	m_pList->SetItem(&li);
	
	// FS
	li.pszText=szFileSystem;
	li.iSubItem=3;
	m_pList->SetItem(&li);
	
	if (m_pDatabase->GetRoots()==NULL) 
	{
		// Set if local drives	
		if (nType==DRIVE_FIXED)
			m_pList->SetCheckState(li.iItem,TRUE);
	}
	else
	{
		LPCWSTR pPtr=m_pDatabase->GetRoots();

		while (*pPtr!='\0')
		{
			DWORD dwLength=istrlenw(pPtr);

			// Check if selected
			if (strcasecmp(pPtr,szDrive)==0)
				m_pList->SetCheckState(li.iItem,TRUE);

			pPtr+=dwLength+1;
		}
	}
	
	return li.iItem;
}

int CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::AddDirectoryToListWithVerify(LPCWSTR szFolder,INT iLength)
{
	CStringW rFolder(szFolder,iLength);

	// Checks  wherther rPath is OK i.e. removes \ at end
	// Further, checks whether folder is already in list 
	// or folder is subfolder for already exists folder
	
		
	// No \ at end
	while (rFolder.LastChar()=='\\')
		rFolder.DelLastChar();

	LVITEMW li;
	li.iItem=m_pList->GetNextItem(-1,LVNI_ALL);
	while (li.iItem!=-1)
	{
		WCHAR szPath[MAX_PATH];
		li.mask=LVIF_TEXT;
		li.iSubItem=1;
		li.pszText=szPath;
		li.cchTextMax=_MAX_PATH;
		m_pList->GetItem(&li);
		if (rFolder.CompareNoCase(szPath)==0)
		{
			CStringW str;
			str.Format(IDS_FOLDEREXIST,(LPCWSTR)rFolder);
			MessageBox(str,ID2W(IDS_ADDFOLDER),MB_ICONINFORMATION|MB_OK);
			m_pList->SetFocus();
			m_pList->SetCheckState(li.iItem,TRUE);
			m_pList->EnsureVisible(li.iItem,FALSE);
			return -1;
		}
		li.iItem=m_pList->GetNextItem(li.iItem,LVNI_ALL);
	}
	li.iItem=AddDirectoryToList(rFolder);
	m_pList->SetItemState(li.iItem,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
	m_pList->EnsureVisible(li.iItem,FALSE);
	return li.iItem;
}

		
int CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::AddDirectoryToList(LPCWSTR szPath,int iLength)
{
	if (iLength==-1)
		iLength=istrlenw(szPath);
	
	WCHAR szLabel[20],szFileSystem[20];
	
	// Resolving label
	DWORD dwTemp;
	LVITEMW li;
	li.iItem=m_pList->GetItemCount();

	CStringW Drive;
	if (szPath[1]==':')
		Drive << szPath[0] << ":\\";
	else
	{
		int nIndex=FirstCharIndex(szPath,L'\\');
		if (nIndex==-1 || szPath[nIndex+1]!=L'\\')
			return -1;
		
		nIndex=NextCharIndex(szPath,L'\\',nIndex+1);
		if (nIndex==-1)
		{
			Drive.Copy(szPath,iLength);
			Drive << '\\';
		}
		else
			Drive.Copy(szPath,nIndex+1);
	}
	
	
	
	UINT nOldMode=SetErrorMode(SEM_FAILCRITICALERRORS);
	if (!FileSystem::GetVolumeInformation(Drive,szLabel,20,&dwTemp,&dwTemp,&dwTemp,szFileSystem,20))
		szFileSystem[0]='\0';
	SetErrorMode(nOldMode);

	// Resolving icon,
	li.mask=LVIF_TEXT|LVIF_IMAGE;
	li.iSubItem=0;
		
	// Resolving icon,
	if (GetLocateApp()->GetProgramFlags()&CLocateApp::pfUseDefaultIconForDirectories)
	{
		li.iImage=DIR_IMAGE;
		li.pszText=const_cast<LPWSTR>(szPath+LastCharIndex(szPath,L'\\')+1);
	}
	else
	{
		SHFILEINFOW fi;
		if (GetFileInfo(szPath,0,&fi,SHGFI_DISPLAYNAME|SHGFI_SMALLICON|SHGFI_SYSICONINDEX))
		{
			// Label
			li.iImage=fi.iIcon;
			li.pszText=fi.szDisplayName;
		}
		else
		{
			// Label
			li.iImage=DEL_IMAGE;
			li.pszText=const_cast<LPWSTR>(szwEmpty);
		}
	}
	m_pList->InsertItem(&li);

	// Path
	li.mask=LVIF_TEXT;
	li.iSubItem=1;
	li.pszText=const_cast<LPWSTR>(szPath);
	m_pList->SetItem(&li);

	// Type
	LoadString(IDS_VOLUMETYPEDIRECTORY,szLabel,20);
	li.pszText=szLabel;
	li.iSubItem=2;
	m_pList->SetItem(&li);
	
	// FS
	li.pszText=szFileSystem;
	li.iSubItem=3;
	m_pList->SetItem(&li);

	m_pList->SetCheckState(li.iItem,TRUE);
	return li.iItem;
}

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::EnableRemoveButton()
{
	
	// Check current item whether it is drive (drives cannot be removed)
	LVITEM li;
	li.iItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	if (li.iItem==-1)
		return FALSE; // Any drive is not selected

	char szPath[_MAX_PATH];
	li.mask=LVIF_TEXT;
	li.iSubItem=1;
	li.pszText=szPath;
	li.cchTextMax=_MAX_PATH;
	m_pList->GetItem(&li);
	
	// Is the path form of "X:"?
	if (szPath[1]==':' && szPath[2]=='\0')
		return FALSE;
	return TRUE;
}







int CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::AddComputerToList(LPCWSTR szName)
{
	WCHAR szLabel[100];
	
	// Resolving label
	LVITEMW li;
	li.iItem=m_pList->GetItemCount();

	// Setting data
	// Label
	if (GetLocateApp()->GetProgramFlags()&CLocateApp::pfUseDefaultIconForDirectories)
	{
		li.iImage=DIR_IMAGE;
		li.pszText=const_cast<LPWSTR>(szName);
	}
	else
	{
		// Resolving icon,
		SHFILEINFOW fi;
		if (GetFileInfo(szName,0,&fi,SHGFI_DISPLAYNAME|SHGFI_SMALLICON|SHGFI_SYSICONINDEX))
		{
			li.iImage=fi.iIcon;
			li.pszText=fi.szDisplayName;
		}
		else
			return -1;
	}

	li.iSubItem=0;
	li.mask=LVIF_TEXT|LVIF_IMAGE;
	m_pList->InsertItem(&li);

	// Path
	li.mask=LVIF_TEXT;
	li.iSubItem=1;
	li.pszText=const_cast<LPWSTR>(szName);
	m_pList->SetItem(&li);
	
	// Type
	LoadString(IDS_VOLUMETYPECOMPUTER,szLabel,100);
	li.pszText=szLabel;
	li.iSubItem=2;
	m_pList->SetItem(&li);
	
	// FS
	m_pList->SetItemState(li.iItem,0x2000,LVIS_STATEIMAGEMASK);
	return li.iItem;
}

/////////////////////////////////////////////////
// CDatabasesSettingsPage::CDatabaseDialog::CExcludedDirectoryDialog

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	CListBox List(GetDlgItem(IDC_DIRECTORIES));

	SetDlgItemText(IDC_EXCLUDEFILES,m_sFiles);

	// Inserting strings
	for (int i=0;i<m_aDirectories.GetSize();i++)
		List.AddString(m_aDirectories[i]);
	
	EnableControls();
	SetFocus(IDC_EXCLUDEFILES);


	return FALSE;
}

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);
	switch(wID)
	{
	case IDOK:
		if (m_bDirectoryFieldChanged && GetFocus()==GetDlgItem(IDC_DIRECTORYNAME))
		{
			OnAddFolder(TRUE);
			SetFocus(IDC_DIRECTORYNAME);
		}
		else
			OnOK();
		break;
	case IDC_DIRECTORYNAME:
		if (wNotifyCode==EN_SETFOCUS)
			::SendMessage(hControl,EM_SETSEL,0,MAKELPARAM(0,-1));
		else if (wNotifyCode==EN_CHANGE)
		{
			EnableControls();
			m_bDirectoryFieldChanged=TRUE;
		}
		break;
	case IDC_OK:
		OnOK();
		break;
	case IDCANCEL:
	case IDC_CANCEL:
		EndDialog(0);
		break;
	case IDC_BROWSE:
		OnBrowse();
		break;
	case IDC_ADDFOLDER:
		OnAddFolder(TRUE);
		break;
	case IDC_REMOVEFOLDER:
		OnRemove();
		break;
	case IDC_DIRECTORIES:
		if (wNotifyCode==LBN_SELCHANGE)
			EnableControls();
break;
	}
	return FALSE;
}

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::OnClose()
{
	CDialog::OnClose();
	EndDialog(0);
	return 0;
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::EnableControls()
{
	EnableDlgItem(IDC_REMOVEFOLDER,SendDlgItemMessage(IDC_DIRECTORIES,LB_GETSEL)!=LB_ERR?TRUE:FALSE);
	EnableDlgItem(IDC_ADDFOLDER,GetDlgItemTextLength(IDC_DIRECTORYNAME)>0);
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::OnOK()
{
	EndDialog(1);

	GetDlgItemText(IDC_EXCLUDEFILES,m_sFiles);
	
	m_aDirectories.RemoveAll();

	CListBox Directories(GetDlgItem(IDC_DIRECTORIES));
	int nCount=Directories.GetCount();
	for (int i=0;i<nCount;i++)
	{
		DWORD iLength=Directories.GetTextLen(i)+1;
		WCHAR* pText=new WCHAR[max(iLength,2)];
		Directories.GetText(i,pText);
		m_aDirectories.Add(pText);
	}

}
		
BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::OnAddFolder(BOOL bShowErrorMessageIfExists)
{
	m_bDirectoryFieldChanged=FALSE;

	CStringW sDirectoryPre,sDirectory;
	CEdit DirectoryName(GetDlgItem(IDC_DIRECTORYNAME));
	CListBox Directories(GetDlgItem(IDC_DIRECTORIES));
	
	DirectoryName.GetText(sDirectoryPre);

	if (sDirectoryPre.Find('*')==-1 && sDirectory.Find('.')==-1)
	{
		// Exact path
		if (!FileSystem::GetFullPathName(sDirectoryPre,400,sDirectory.GetBuffer(400),NULL))
		{
			CStringW str;
			str.Format(IDS_ERRORDIRECTORYNOTFOUND,LPCWSTR(sDirectory));
			MessageBox(str,ID2W(IDS_ERROR),MB_OK|MB_ICONERROR);
			DirectoryName.SetFocus();
			return FALSE;	
		}
		sDirectory.FreeExtra();


		if (!FileSystem::IsDirectory(sDirectory))
		{
			CStringW str;
			str.Format(IDS_ERRORDIRECTORYNOTFOUND,LPCWSTR(sDirectory));
			MessageBox(str,ID2W(IDS_ERROR),MB_OK|MB_ICONERROR);
			DirectoryName.SetFocus();
			return FALSE;
		}

		sDirectoryPre=sDirectory;
	}
	else
		sDirectory=sDirectoryPre;
		
	sDirectoryPre.MakeLower();

	
	for (int i=Directories.GetCount()-1;i>=0;i--)
	{
		int iLength=Directories.GetTextLen(i);
        WCHAR* szText=new WCHAR[iLength+2];
		Directories.GetText(i,szText);
		MakeLower(szText);

		if (sDirectoryPre.Compare(szText)==0)
		{
			if (bShowErrorMessageIfExists)
				ShowErrorMessage(IDS_ALREADYEXCLUDED,IDS_ERROR);
			return TRUE;
		}		
	}
    
	Directories.AddString(sDirectory);

	SetFocus(IDC_DIRECTORIES);
	EnableControls();
	return FALSE;
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::OnRemove()
{
	int nSel=(int)SendDlgItemMessage(IDC_DIRECTORIES,LB_GETCURSEL);
	if (nSel==LB_ERR)
		return;

	SendDlgItemMessage(IDC_DIRECTORIES,LB_DELETESTRING,nSel);

	EnableControls();
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::OnBrowse()
{
	CWaitCursor;

	// Ask folder
	CFolderDialog fd(IDS_ADDFOLDER,BIF_RETURNONLYFSDIRS|BIF_USENEWUI|BIF_NONEWFOLDERBUTTON);
	if (fd.DoModal(*this))
	{
		// Insert folder to list
		CStringW Folder;
		if (!fd.GetFolder(Folder))
			ShowErrorMessage(IDS_CANNOTEXCLUDESELECTED,IDS_ERROR);
		else
			SetDlgItemText(IDC_DIRECTORYNAME,Folder);
		
		// Setting focus to list
		SetFocus(IDC_DIRECTORYNAME);
	}
}

////////////////////////////////////////
// CAutoUpdateSettingsPage
////////////////////////////////////////

BOOL CSettingsProperties::CAutoUpdateSettingsPage::OnInitDialog(HWND hwndFocus)
{
	CPropertyPage::OnInitDialog(hwndFocus);
	m_pSchedules=new CListBox(GetDlgItem(IDC_UPDATES));
	POSITION pPos=m_pSettings->m_Schedules.GetHeadPosition();
	while (pPos!=NULL)
	{
		m_pSchedules->InsertString(-1,(LPCSTR)m_pSettings->m_Schedules.GetAt(pPos));
		pPos=m_pSettings->m_Schedules.GetNextPosition(pPos);
	}
	return FALSE;
}

void CSettingsProperties::CAutoUpdateSettingsPage::EnableItems()
{
	int nCurSel=m_pSchedules->GetCurSel();
	if (nCurSel==-1)
	{
		EnableDlgItem(IDC_EDIT,FALSE);
		EnableDlgItem(IDC_DELETE,FALSE);

		EnableDlgItem(IDC_DOWN,FALSE);
		EnableDlgItem(IDC_UP,FALSE);
	}
	else
	{
		EnableDlgItem(IDC_EDIT,TRUE);
		EnableDlgItem(IDC_DELETE,TRUE);	

		EnableDlgItem(IDC_DOWN,nCurSel!=m_pSchedules->GetCount()-1);
		EnableDlgItem(IDC_UP,nCurSel!=0);
	}
}

BOOL CSettingsProperties::CAutoUpdateSettingsPage::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CPropertyPage::OnCommand(wID,wNotifyCode,hControl);
	switch (wID)
	{
	case IDC_ADD:
		{
			CCheduledUpdateDlg sud;
			sud.m_pSchedule=new CSchedule;
			if (sud.DoModal(*this))
			{
				m_pSettings->m_Schedules.AddTail(sud.m_pSchedule);
				m_pSchedules->InsertString(-1,(LPCSTR)sud.m_pSchedule);
			}
			else
				delete sud.m_pSchedule;

			EnableItems();
			break;
		}
	case IDC_DELETE:
		{
			int nCurSel=m_pSchedules->GetCurSel();
			if (nCurSel==-1)
				break;
			CSchedule* tmp=(CSchedule*)m_pSchedules->GetItemData(nCurSel);
			m_pSchedules->DeleteString(nCurSel);
			m_pSettings->m_Schedules.RemoveAt(m_pSettings->m_Schedules.Find(tmp));

			EnableItems();
						
			break;
		}
	case IDC_EDIT:
		OnEdit();
		break;
	case IDC_UPDATES:
		if (wNotifyCode==LBN_DBLCLK)
			OnEdit();
		else if (wNotifyCode==LBN_SELCHANGE)
		{
			CString txt;
			EnableItems();

			int nCurSel=m_pSchedules->GetCurSel();
			if (nCurSel==-1)
				break;
		
			CSchedule* pSchedule=(CSchedule*)m_pSchedules->GetItemData(nCurSel);
			if (pSchedule==NULL)
				break;
			if (pSchedule->m_bFlags&CSchedule::flagRunned)
			{
				SYSTEMTIME st;
				char szDate[100],szTime[100];
				st.wYear=pSchedule->m_tLastStartDate.wYear;
				st.wMonth=pSchedule->m_tLastStartDate.bMonth;
				st.wDay=pSchedule->m_tLastStartDate.bDay;
				st.wHour=pSchedule->m_tLastStartTime.bHour;
				st.wMinute=pSchedule->m_tLastStartTime.bMinute;
				st.wSecond=pSchedule->m_tLastStartTime.bSecond;
				st.wMilliseconds=0;
				GetTimeFormat(LOCALE_USER_DEFAULT,0,&st,NULL,szTime,100);
				GetDateFormat(LOCALE_USER_DEFAULT,DATE_LONGDATE,&st,NULL,szDate,100);
				txt.FormatEx(IDS_LASTRUN,szDate,szTime);
			}
			else
				txt.LoadString(IDS_LASTRUNNEVER);
			SetDlgItemText(IDC_LASTRUN,(LPCSTR)txt);
		}
		break;
	case IDC_UP:
	case IDC_DOWN:
		{
			int nSelItem=m_pSchedules->GetCurSel();
			int nAltItem=nSelItem+(wID==IDC_DOWN?1:-1);



			if (nSelItem==-1 ||
				(wID==IDC_UP && nSelItem==0) ||
				(wID==IDC_DOWN && nSelItem==m_pSchedules->GetCount()-1))
				break;
			
			CSchedule* pSchedule=(CSchedule*)m_pSchedules->GetItemData(nSelItem);
			CSchedule* pAltSchedule=(CSchedule*)m_pSchedules->GetItemData(nAltItem);

			if (pSchedule==NULL || pAltSchedule==NULL)
				break;
            
			POSITION pPos=m_pSettings->m_Schedules.Find(pSchedule);
			POSITION pAltPos=m_pSettings->m_Schedules.Find(pAltSchedule);

			if (pPos==NULL || pAltPos==NULL)
				break;
            
			m_pSettings->m_Schedules.SetAt(pPos,pAltSchedule);
			m_pSchedules->SetItemData(nSelItem,DWORD(pAltSchedule));
			
			m_pSettings->m_Schedules.SetAt(pAltPos,pSchedule);
			m_pSchedules->SetItemData(nAltItem,DWORD(pSchedule));

			m_pSchedules->UpdateWindow();
			m_pSchedules->InvalidateRect(NULL,TRUE);
			m_pSchedules->SetCurSel(nAltItem);
			
			EnableItems();

			break;
		}


	}
	return FALSE;
}

void CSettingsProperties::CAutoUpdateSettingsPage::OnEdit()
{
	CCheduledUpdateDlg sud;
	int nCurSel=m_pSchedules->GetCurSel();
	if (nCurSel==-1)
		return;
	sud.m_pSchedule=(CSchedule*)m_pSchedules->GetItemData(nCurSel);
	sud.DoModal(*this);
	m_pSchedules->SetCurSel(nCurSel);
}

void CSettingsProperties::CAutoUpdateSettingsPage::OnDestroy()
{
	CPropertyPage::OnDestroy();
	if (m_pSchedules!=NULL)
	{
		delete m_pSchedules;
		m_pSchedules=NULL;
	}
}

void CSettingsProperties::CAutoUpdateSettingsPage::OnDrawItem(UINT idCtl,LPDRAWITEMSTRUCT lpdis)
{
	if (idCtl==IDC_UPDATES)
	{
		CDC dc(lpdis->hDC);
		HICON hIcon;
		CSchedule* pSchedule=(CSchedule*)lpdis->itemData;
		if (pSchedule==NULL)
			return;
		CBrush brush(GetSysColor(COLOR_WINDOW));
		HPEN hOldPen=(HPEN)dc.SelectObject(GetStockObject(WHITE_PEN));
		HBRUSH hOldBrush=(HBRUSH)dc.SelectObject(brush);
		dc.SetBkMode(TRANSPARENT);
		dc.Rectangle(&(lpdis->rcItem));
		if (lpdis->itemState&ODS_SELECTED)
		{
			CBrush Brush(GetSysColor(COLOR_HIGHLIGHT));
			CPen Pen(PS_SOLID,1,GetSysColor(COLOR_HIGHLIGHT));
			dc.SelectObject(Brush);
			dc.SelectObject(Pen);
			dc.Rectangle(lpdis->rcItem.left+1,lpdis->rcItem.top+1,lpdis->rcItem.right-1,lpdis->rcItem.bottom-1);
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			dc.SelectObject(brush);
		}
		else
			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		
		if (pSchedule->m_bFlags&CSchedule::flagEnabled)
			hIcon=(HICON)LoadImage(IDI_YES,IMAGE_ICON,16,16,0);
		else
			hIcon=(HICON)LoadImage(IDI_NO,IMAGE_ICON,16,16,0);
		CRect rc(lpdis->rcItem);
		dc.DrawState(CPoint(rc.left,rc.top),CSize(16,16),hIcon,DST_ICON);
		rc.left+=16;
		rc.top++;
	
		CStringW str;
		pSchedule->GetString(str);
		dc.DrawText(str,&rc,DT_LEFT|DT_VCENTER);

		dc.SelectObject(hOldPen);
		dc.SelectObject(hOldBrush);
	}
	CPropertyPage::OnDrawItem(idCtl,lpdis);
}

void CSettingsProperties::CAutoUpdateSettingsPage::OnMeasureItem(int nIDCtl,LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	switch(nIDCtl)
	{
	case IDC_UPDATES:
		{
			CDC dc(this);
			CSize sz;
			CStringW str;
			
			((CSchedule*)lpMeasureItemStruct->itemData)->GetString(str);
			sz=dc.GetTextExtent(str);
			
			lpMeasureItemStruct->itemWidth=sz.cx+2;
			lpMeasureItemStruct->itemHeight=max(sz.cy,16)+3;
			break;
		}
	}
	CPropertyPage::OnMeasureItem(nIDCtl,lpMeasureItemStruct);
}
		

BOOL CSettingsProperties::CAutoUpdateSettingsPage::OnApply()
{
	CPropertyPage::OnApply();
	return TRUE;
}

void CSettingsProperties::CAutoUpdateSettingsPage::OnCancel()
{
	m_pSettings->SetSettingsFlags(CSettingsProperties::settingsCancelled);

	CPropertyPage::OnCancel();
}


/////////////////////////////////////////////////
// CAutoUpdateSettingsPage::CCheduledUpdateDlg

BOOL CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::OnInitDialog(HWND hwndFocus)
{
	// Type combo
	int nSel[]={2,3,4,5,6,1,0};
	m_pTypeCombo=new CComboBox(GetDlgItem(IDC_TYPE));
	m_pTypeCombo->AddString(ID2W(IDS_MINUTELY));
	m_pTypeCombo->AddString(ID2W(IDS_HOURLY));
	m_pTypeCombo->AddString(ID2W(IDS_DAILY));
	m_pTypeCombo->AddString(ID2W(IDS_WEEKLY));
	m_pTypeCombo->AddString(ID2W(IDS_MONTHLY));
	m_pTypeCombo->AddString(ID2W(IDS_ONCE));
	m_pTypeCombo->AddString(ID2W(IDS_ATSTARTUP));
	m_pTypeCombo->SetCurSel(nSel[m_pSchedule->m_nType]);
	
	// Month type combo
	CComboBox Combo(GetDlgItem(IDC_MTYPE));
	Combo.AddString(ID2W(IDS_FIRST));
	Combo.AddString(ID2W(IDS_SECOND));
	Combo.AddString(ID2W(IDS_THIRD));
	Combo.AddString(ID2W(IDS_FOURTH));
	Combo.AddString(ID2W(IDS_LAST));
	
	// Days combo
	Combo.AssignToDlgItem(*this,IDC_MDAYS);
	Combo.AddString(ID2W(IDS_MONDAY));
	Combo.AddString(ID2W(IDS_TUESDAY));
	Combo.AddString(ID2W(IDS_WEDNESDAY));
	Combo.AddString(ID2W(IDS_THURSDAY));
	Combo.AddString(ID2W(IDS_FRIDAY));
	Combo.AddString(ID2W(IDS_SATURDAY));
	Combo.AddString(ID2W(IDS_SUNDAY));

	// Time control
	SYSTEMTIME st;
	GetLocalTime(&st);
	st.wHour=m_pSchedule->m_tStartTime.bHour;
	st.wMinute=m_pSchedule->m_tStartTime.bMinute;
	st.wSecond=m_pSchedule->m_tStartTime.bSecond;
	CDateTimeCtrl TimeCtrl(GetDlgItem(IDC_TIME));
	TimeCtrl.SetSystemtime(GDT_VALID,&st);

	
	// Setting Every spin (and maybe other things)
	CSpinButtonCtrl SpinControl(GetDlgItem(IDC_SPIN));
	SpinControl.SetRange(1,32000);
	SpinControl.SetBuddy(GetDlgItem(IDC_EVERY));
	switch (m_pSchedule->m_nType)
	{
	case CSchedule::typeMinutely:
		SpinControl.SetPos(m_pSchedule->m_tMinutely.wEvery);
		SetDlgItemInt(IDC_EVERY,m_pSchedule->m_tMinutely.wEvery);
		break;
	case CSchedule::typeHourly:
		SpinControl.SetPos(m_pSchedule->m_tHourly.wEvery);
		SetDlgItemInt(IDC_EVERY,m_pSchedule->m_tHourly.wEvery);
		break;
	case CSchedule::typeDaily:
		SpinControl.SetPos(m_pSchedule->m_tDaily.wEvery);
		SetDlgItemInt(IDC_EVERY,m_pSchedule->m_tDaily.wEvery);
		break;
	case CSchedule::typeWeekly:
		SpinControl.SetPos(m_pSchedule->m_tWeekly.wEvery);
		SetDlgItemInt(IDC_EVERY,m_pSchedule->m_tWeekly.wEvery);
		if (m_pSchedule->m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Monday)
			CheckDlgButton(IDC_MON,BST_CHECKED);
		if (m_pSchedule->m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Tuesday)
			CheckDlgButton(IDC_TUE,BST_CHECKED);
		if (m_pSchedule->m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Wednesday)
			CheckDlgButton(IDC_WED,BST_CHECKED);
		if (m_pSchedule->m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Thursday)
			CheckDlgButton(IDC_THU,BST_CHECKED);
		if (m_pSchedule->m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Friday)
			CheckDlgButton(IDC_FRI,BST_CHECKED);
		if (m_pSchedule->m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Saturday)
			CheckDlgButton(IDC_SAT,BST_CHECKED);
		if (m_pSchedule->m_tWeekly.bDays&CSchedule::SWEEKLYTYPE::Sunday)
			CheckDlgButton(IDC_SUN,BST_CHECKED);
		break;
	default:
        SpinControl.SetPos(1);
		break;
	}

	// "Every Month" spin
	SpinControl.AssignToDlgItem(*this,IDC_MSPIN);
	SpinControl.SetRange(1,31);
	SpinControl.SetBuddy(GetDlgItem(IDC_MEVERY));
	if (m_pSchedule->m_nType==CSchedule::typeMonthly)
	{
		if (m_pSchedule->m_tMonthly.nType==CSchedule::SMONTHLYTYPE::Day)
		{
			CheckDlgButton(IDC_MDAY,BST_CHECKED);
			OnCommand(IDC_MDAY,CBN_SELCHANGE,NULL);
			SpinControl.SetPos(m_pSchedule->m_tMonthly.bDay);
			SetDlgItemInt(IDC_MEVERY,m_pSchedule->m_tMonthly.bDay);
			SendDlgItemMessage(IDC_MTYPE,CB_SETCURSEL,0,0);
			SendDlgItemMessage(IDC_MDAYS,CB_SETCURSEL,0,0);
		}
		else
		{
			CheckDlgButton(IDC_MTHE,BST_CHECKED);
			OnCommand(IDC_MTHE,CBN_SELCHANGE,NULL);
			SendDlgItemMessage(IDC_MTYPE,CB_SETCURSEL,m_pSchedule->m_tMonthly.nWeek,0);
			SendDlgItemMessage(IDC_MDAYS,CB_SETCURSEL,m_pSchedule->m_tMonthly.bDay,0);
			SpinControl.SetPos(1);
		}	
	}
	else
	{
		CheckDlgButton(IDC_MDAY,BST_CHECKED);
		OnCommand(IDC_MDAY,CBN_SELCHANGE,NULL);
		SpinControl.SetPos(1);
		SendDlgItemMessage(IDC_MTYPE,CB_SETCURSEL,0,0);
		SendDlgItemMessage(IDC_MDAYS,CB_SETCURSEL,0,0);
	}
	


	// Setting minute spin
	SpinControl.AssignToDlgItem(*this,IDC_MINUTESPIN);
	SpinControl.SetRange(0,59);
	SpinControl.SetBuddy(GetDlgItem(IDC_MINUTEONHOUR));
	if (m_pSchedule->m_nType==CSchedule::typeHourly)
	{
		SpinControl.SetPos(m_pSchedule->m_tHourly.wMinute);
		SetDlgItemInt(IDC_MINUTEONHOUR,m_pSchedule->m_tHourly.wMinute);
	}
	else
		SpinControl.SetPos(0);
		
	
	// Once time control
	if (m_pSchedule->m_nType==CSchedule::typeOnce)
	{
		GetLocalTime(&st);
		st.wYear=m_pSchedule->m_dStartDate.wYear;
		st.wMonth=m_pSchedule->m_dStartDate.bMonth;
		st.wDay=m_pSchedule->m_dStartDate.bDay;
		CDateTimeCtrl DateCtrl(GetDlgItem(IDC_ONCETIME));
		DateCtrl.SetSystemtime(GDT_VALID,&st);
	}
	
	
	
	// Enabled, "delete after run" and "at this time only"	
	if (m_pSchedule->m_bFlags&CSchedule::flagEnabled)
		CheckDlgButton(IDC_ENABLED,BST_CHECKED);
	if (m_pSchedule->m_bFlags&CSchedule::flagDeleteAfterRun)
		CheckDlgButton(IDC_DELETEAFTERRUN,BST_CHECKED);
	if (m_pSchedule->m_bFlags&CSchedule::flagAtThisTime)
		CheckDlgButton(IDC_ATTHISTIME,BST_CHECKED);

	
	// CPU usage
	SpinControl.AssignToDlgItem(*this,IDC_CPUUSAGESPIN);
	SpinControl.SetRange(0,100);
	SpinControl.SetBuddy(GetDlgItem(IDC_CPUUSAGE));
	if (m_pSchedule->m_wCpuUsageTheshold!=WORD(-1))
	{
		CheckDlgButton(IDC_CPUUSAGECHECK,TRUE);
		SetDlgItemInt(IDC_CPUUSAGE,m_pSchedule->m_wCpuUsageTheshold,FALSE);
		SpinControl.SetPos(m_pSchedule->m_wCpuUsageTheshold);
	}
	else
	{
		EnableDlgItem(IDC_CPUUSAGE,FALSE);
		EnableDlgItem(IDC_CPUUSAGESPIN,FALSE);
		EnableDlgItem(IDC_CPUUSAGEEXTRALABEL,FALSE);
		SetDlgItemInt(IDC_CPUUSAGE,0,FALSE);
		SpinControl.SetPos(100);
	}


	// Thread priority
	Combo.AssignToDlgItem(*this,IDC_THREADPRIORITY);
	Combo.AddString(ID2W(IDS_PRIORITYHIGH));
	Combo.AddString(ID2W(IDS_PRIORITYABOVENORMAL));
	Combo.AddString(ID2W(IDS_PRIORITYNORMAL));
	Combo.AddString(ID2W(IDS_PRIORITYBELOWNORMAL));
	Combo.AddString(ID2W(IDS_PRIORITYLOW));
	Combo.AddString(ID2W(IDS_PRIORITYIDLE));
	switch (m_pSchedule->m_nThreadPriority)
	{
	case THREAD_PRIORITY_HIGHEST:
		Combo.SetCurSel(0);
		break;
	case THREAD_PRIORITY_ABOVE_NORMAL:
		Combo.SetCurSel(1);
		break;
	case THREAD_PRIORITY_NORMAL:
		Combo.SetCurSel(2);
		break;
	case THREAD_PRIORITY_BELOW_NORMAL:
		Combo.SetCurSel(3);
		break;
	case THREAD_PRIORITY_LOWEST:
		Combo.SetCurSel(4);
		break;
	case THREAD_PRIORITY_IDLE:
		Combo.SetCurSel(5);
		break;
	default:
		Combo.SetCurSel(2);
		break;
	}
		


	// Last run text
	CStringW LastRun;
	if (m_pSchedule->m_bFlags&CSchedule::flagRunned &&
		!(m_pSchedule->m_tLastStartTime.bHour==0 && m_pSchedule->m_tLastStartTime.bMinute==0 && 
		m_pSchedule->m_tLastStartTime.bSecond==0 && m_pSchedule->m_tLastStartDate.wYear<1995 && 
		m_pSchedule->m_tLastStartDate.bMonth==0 && m_pSchedule->m_tLastStartDate.bDay==0))
	{
		if (IsUnicodeSystem())
		{
			WCHAR szDate[100],szTime[100];
			st.wYear=m_pSchedule->m_tLastStartDate.wYear;
			st.wMonth=m_pSchedule->m_tLastStartDate.bMonth;
			st.wDay=m_pSchedule->m_tLastStartDate.bDay;
			st.wHour=m_pSchedule->m_tLastStartTime.bHour;
			st.wMinute=m_pSchedule->m_tLastStartTime.bMinute;
			st.wSecond=m_pSchedule->m_tLastStartTime.bSecond;
			st.wMilliseconds=0;
			GetTimeFormatW(LOCALE_USER_DEFAULT,0,&st,NULL,szTime,100);
			GetDateFormatW(LOCALE_USER_DEFAULT,DATE_LONGDATE,&st,NULL,szDate,100);
			LastRun.Format(IDS_LASTRUN,szDate,szTime);
		}
		else
		{
			char szDate[100],szTime[100];
			st.wYear=m_pSchedule->m_tLastStartDate.wYear;
			st.wMonth=m_pSchedule->m_tLastStartDate.bMonth;
			st.wDay=m_pSchedule->m_tLastStartDate.bDay;
			st.wHour=m_pSchedule->m_tLastStartTime.bHour;
			st.wMinute=m_pSchedule->m_tLastStartTime.bMinute;
			st.wSecond=m_pSchedule->m_tLastStartTime.bSecond;
			st.wMilliseconds=0;
			GetTimeFormat(LOCALE_USER_DEFAULT,0,&st,NULL,szTime,100);
			GetDateFormat(LOCALE_USER_DEFAULT,DATE_LONGDATE,&st,NULL,szDate,100);
			LastRun.Format(IDS_LASTRUN,(LPCWSTR)A2W(szDate),(LPCWSTR)A2W(szTime));
		}
	}
	else
		LastRun.LoadString(IDS_LASTRUNNEVER);
	
/*#ifdef _DEBUG
	CStringW txt2;
	txt2.Format(L" F:%X STD:%d.%d.%d STT: %d:%d:",m_pSchedule->m_bFlags,
		m_pSchedule->m_tLastStartDate.bDay,m_pSchedule->m_tLastStartDate.bMonth,m_pSchedule->m_tLastStartDate.wYear,
		m_pSchedule->m_tLastStartTime.bHour,m_pSchedule->m_tLastStartTime.bMinute,m_pSchedule->m_tLastStartTime.bSecond);
	LastRun << txt2;
#endif*/

	SetDlgItemText(IDC_LASTRUN,LastRun);
	
	
	OnTypeChanged();
		
	m_bChanged=FALSE;
	return CDialog::OnInitDialog(hwndFocus);
}

BOOL CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);
	switch (wID)
	{
	case IDC_OK:
		OnOK();
		break;
	case IDCANCEL:
	case IDC_CANCEL:
		EndDialog(0);
		break;
	case IDC_TYPE:
		if (wNotifyCode==CBN_SELCHANGE)
			OnTypeChanged();
		break;
	case IDC_MDAY:
	case IDC_MTHE:
		CheckDlgButton(IDC_MDAY,wID==IDC_MDAY);
		CheckDlgButton(IDC_MTHE,wID==IDC_MTHE);
		
		EnableDlgItem(IDC_MEVERY,wID==IDC_MDAY);
		EnableDlgItem(IDC_MSPIN,wID==IDC_MDAY);
		EnableDlgItem(IDC_MTYPE,wID==IDC_MTHE);
		EnableDlgItem(IDC_MDAYS,wID==IDC_MTHE);
		m_bChanged=TRUE;
		break;
	case IDC_TIME:
	case IDC_MON:
	case IDC_TUE:
	case IDC_WED:
	case IDC_THU:
	case IDC_FRI:
	case IDC_SAT:
	case IDC_SUN:	
	case IDC_MTYPE:
	case IDC_MDAYS:
	case IDC_ONCETIME:
	case IDC_ENABLED:
	case IDC_DELETEAFTERRUN:
	case IDC_THREADPRIORITY:
		m_bChanged=TRUE;
		break;
	case IDC_DATABASES:
		OnDatabases();
		break;
	case IDC_CPUUSAGECHECK:
		{
			BOOL bEnable=IsDlgButtonChecked(IDC_CPUUSAGECHECK);
			EnableDlgItem(IDC_CPUUSAGE,bEnable);
			EnableDlgItem(IDC_CPUUSAGESPIN,bEnable);
			EnableDlgItem(IDC_CPUUSAGEEXTRALABEL,bEnable);
			
			SetFocus(IDC_CPUUSAGE);
			m_bChanged=TRUE;
			break;
		}
	case IDC_EVERY:
		if (wNotifyCode==EN_SETFOCUS)
			::SendMessage(hControl,EM_SETSEL,0,-1);
		else if (wNotifyCode==EN_CHANGE)
		{
			BOOL bTranslated=FALSE;
			UINT nVal=GetDlgItemInt(IDC_EVERY,&bTranslated,FALSE);
			if (nVal<1)
				SetDlgItemInt(IDC_EVERY,1,FALSE);
		
			m_bChanged=TRUE;
		}
		break;
	case IDC_MEVERY:
		if (wNotifyCode==EN_SETFOCUS)
			::SendMessage(hControl,EM_SETSEL,0,-1);
		else if (wNotifyCode==EN_CHANGE)
		{
			BOOL bTranslated=FALSE;
			UINT nVal=GetDlgItemInt(IDC_MEVERY,&bTranslated,FALSE);
			if (nVal<1 && bTranslated)
				SetDlgItemInt(IDC_MEVERY,1,FALSE);
			else if (nVal>31 && bTranslated)
				SetDlgItemInt(IDC_MEVERY,31,FALSE);

			m_bChanged=TRUE;
		}
		break;
	case IDC_MINUTEONHOUR:
		if (wNotifyCode==EN_SETFOCUS)
			::SendMessage(hControl,EM_SETSEL,0,-1);
		else if (wNotifyCode==EN_CHANGE)
		{
			BOOL bTranslated=FALSE;
			UINT nVal=GetDlgItemInt(IDC_MINUTEONHOUR,&bTranslated,FALSE);
			if (nVal>59  && bTranslated)
				SetDlgItemInt(IDC_MINUTEONHOUR,59,FALSE);
			m_bChanged=TRUE;
		}
		break;
	case IDC_CPUUSAGE:
		if (wNotifyCode==EN_SETFOCUS)
			::SendMessage(hControl,EM_SETSEL,0,-1);
		else if (wNotifyCode==EN_CHANGE)
		{
			BOOL bTranslated=FALSE;
			UINT nVal=GetDlgItemInt(IDC_CPUUSAGE,&bTranslated,FALSE);
			if (nVal>100 && bTranslated)
				SetDlgItemInt(IDC_CPUUSAGE,100,FALSE);
			m_bChanged=TRUE;
		}
		break;
	}
	return FALSE;
}

BOOL CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::OnDatabases()
{
	CArray<PDATABASE> aDatabases;
		
	CSelectDatabasesDlg dbd(GetLocateApp()->GetDatabases(),aDatabases,0,
		CRegKey2::GetCommonKey()+"\\Dialogs\\SelectDatabases/Schedule");
	dbd.SelectDatabases(m_pSchedule->m_pDatabases);

	if (dbd.DoModal(*this))
	{
		if ((dbd.m_bFlags&CSelectDatabasesDlg::flagSelectedMask)==CSelectDatabasesDlg::flagLasestIsSelected)
			return TRUE;
		
		if ((dbd.m_bFlags&CSelectDatabasesDlg::flagSelectedMask)==CSelectDatabasesDlg::flagGlobalIsSelected)
		{
			if (m_pSchedule->m_pDatabases!=NULL)
			{
				delete[] m_pSchedule->m_pDatabases;
				m_pSchedule->m_pDatabases=NULL;
			}
		}
		else
		{
			if (m_pSchedule->m_pDatabases!=NULL)
				delete[] m_pSchedule->m_pDatabases;
			
			DWORD dwLength=1;
			int i=0;
			for (i=0;i<aDatabases.GetSize();i++)
				dwLength+=istrlenw(aDatabases[i]->GetName())+1;

			m_pSchedule->m_pDatabases=new WCHAR[dwLength];
			LPWSTR pPtr=m_pSchedule->m_pDatabases;
			for (i=0;i<aDatabases.GetSize();i++)
			{
				LONG_PTR iStrlen=istrlenw(aDatabases[i]->GetName())+1;
				MemCopyW(pPtr,aDatabases[i]->GetName(),iStrlen);
				pPtr+=iStrlen;
			}
			*pPtr='\0';
		}

		m_bChanged=TRUE;
		return TRUE;
	}
	return FALSE;
}

BOOL CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	switch (idCtrl)
	{
	case IDC_TIME:
	case IDC_ONCETIME:
		m_bChanged=TRUE;
		break;
	}
	return CDialog::OnNotify(idCtrl,pnmh);
}

void CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::OnDestroy()
{
	if (m_pTypeCombo!=NULL)
	{
		delete m_pTypeCombo;
		m_pTypeCombo=NULL;
	}
	CDialog::OnDestroy();
}

BOOL CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::OnClose()
{
	CDialog::OnClose();
	EndDialog(0);
	return 0;
}

BOOL CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::OnOK()
{
	BOOL bTranslated=FALSE;

	CSchedule::ScheduleType nType[]={
		CSchedule::typeMinutely,
		CSchedule::typeHourly,
		CSchedule::typeDaily,
		CSchedule::typeWeekly,
		CSchedule::typeMonthly,
		CSchedule::typeOnce,
		CSchedule::typeAtStartup,
	};
	CSchedule::ScheduleType nNewType=nType[m_pTypeCombo->GetCurSel()];
	if (m_pSchedule->m_nType!=nNewType)
	{
		m_bChanged=TRUE;
		m_pSchedule->m_nType=nNewType;
	}
	
	if (m_bChanged)
	{
		if (m_pSchedule->m_nType!=CSchedule::typeAtStartup)
			m_pSchedule->m_bFlags&=~CSchedule::flagRunned;
		CSchedule::GetCurrentDateAndTime(&m_pSchedule->m_tLastStartDate,&m_pSchedule->m_tLastStartTime);
	}

	if (IsDlgButtonChecked(IDC_ENABLED))
		m_pSchedule->m_bFlags|=CSchedule::flagEnabled;
	else
		m_pSchedule->m_bFlags&=~CSchedule::flagEnabled;
	if (IsDlgButtonChecked(IDC_DELETEAFTERRUN))
		m_pSchedule->m_bFlags|=CSchedule::flagDeleteAfterRun;
	else
		m_pSchedule->m_bFlags&=~CSchedule::flagDeleteAfterRun;
	if (IsDlgButtonChecked(IDC_ATTHISTIME))
		m_pSchedule->m_bFlags|=CSchedule::flagAtThisTime;
	else
		m_pSchedule->m_bFlags&=~CSchedule::flagAtThisTime;

	
	SYSTEMTIME st;
	CDateTimeCtrl TimeCtrl(GetDlgItem(IDC_TIME));
	TimeCtrl.GetSystemtime(&st);
	m_pSchedule->m_tStartTime=st;
	
	switch (m_pSchedule->m_nType)
	{
	case CSchedule::typeMinutely:
		m_pSchedule->m_tMinutely.wEvery=GetDlgItemInt(IDC_EVERY,&bTranslated,FALSE);
		if (!bTranslated || (int)m_pSchedule->m_tMinutely.wEvery<1)
			m_pSchedule->m_tMinutely.wEvery=1;
		break;
	case CSchedule::typeHourly:
		m_pSchedule->m_tHourly.wEvery=GetDlgItemInt(IDC_EVERY,&bTranslated,FALSE);
		if (!bTranslated || (int)m_pSchedule->m_tHourly.wEvery<1)
			m_pSchedule->m_tHourly.wEvery=1;
		
		m_pSchedule->m_tHourly.wMinute=GetDlgItemInt(IDC_MINUTEONHOUR,&bTranslated,FALSE);
		if (!bTranslated)
			m_pSchedule->m_tHourly.wMinute=0;
		else if (m_pSchedule->m_tHourly.wMinute>59)
			m_pSchedule->m_tHourly.wMinute=59;
		break;		
	case CSchedule::typeDaily:
		m_pSchedule->m_tDaily.wEvery=GetDlgItemInt(IDC_EVERY,&bTranslated,FALSE);
		if (!bTranslated || (int)m_pSchedule->m_tDaily.wEvery<1)
			m_pSchedule->m_tDaily.wEvery=1;
		break;
	case CSchedule::typeWeekly:
		m_pSchedule->m_tWeekly.wEvery=GetDlgItemInt(IDC_EVERY,&bTranslated,FALSE);
		if (!bTranslated || (int)m_pSchedule->m_tWeekly.wEvery<1)
			m_pSchedule->m_tWeekly.wEvery=1;
		m_pSchedule->m_tWeekly.bDays=0;
		if (IsDlgButtonChecked(IDC_MON))
			m_pSchedule->m_tWeekly.bDays|=CSchedule::SWEEKLYTYPE::Monday;
		if (IsDlgButtonChecked(IDC_TUE))
			m_pSchedule->m_tWeekly.bDays|=CSchedule::SWEEKLYTYPE::Tuesday;
		if (IsDlgButtonChecked(IDC_WED))
			m_pSchedule->m_tWeekly.bDays|=CSchedule::SWEEKLYTYPE::Wednesday;
		if (IsDlgButtonChecked(IDC_THU))
			m_pSchedule->m_tWeekly.bDays|=CSchedule::SWEEKLYTYPE::Thursday;
		if (IsDlgButtonChecked(IDC_FRI))
			m_pSchedule->m_tWeekly.bDays|=CSchedule::SWEEKLYTYPE::Friday;
		if (IsDlgButtonChecked(IDC_SAT))
			m_pSchedule->m_tWeekly.bDays|=CSchedule::SWEEKLYTYPE::Saturday;
		if (IsDlgButtonChecked(IDC_SUN))
			m_pSchedule->m_tWeekly.bDays|=CSchedule::SWEEKLYTYPE::Sunday;
		break;
	case CSchedule::typeMonthly:
		if (IsDlgButtonChecked(IDC_MDAY))
		{
			m_pSchedule->m_tMonthly.nType=CSchedule::SMONTHLYTYPE::Day;
			m_pSchedule->m_tMonthly.bDay=GetDlgItemInt(IDC_MEVERY,&bTranslated,FALSE);
			if (!bTranslated || (int)m_pSchedule->m_tMonthly.bDay<1)
				m_pSchedule->m_tMonthly.bDay=1;
		}
		else
		{
			m_pSchedule->m_tMonthly.nType=CSchedule::SMONTHLYTYPE::WeekDay;
			m_pSchedule->m_tMonthly.nWeek=(CSchedule::SMONTHLYTYPE::Week)SendDlgItemMessage(IDC_MTYPE,CB_GETCURSEL);
			m_pSchedule->m_tMonthly.bDay=(BYTE)SendDlgItemMessage(IDC_MDAYS,CB_GETCURSEL);
		}
		break;
	case CSchedule::typeOnce:
		{
			CDateTimeCtrl DateCtrl(GetDlgItem(IDC_ONCETIME));
			SYSTEMTIME st;
			DateCtrl.GetSystemtime(&st);
			m_pSchedule->m_dStartDate=st;
			break;
		}
	case CSchedule::typeAtStartup:
		break;
	}

	if (IsDlgButtonChecked(IDC_CPUUSAGECHECK))
	{
		m_pSchedule->m_wCpuUsageTheshold=GetDlgItemInt(IDC_CPUUSAGE,&bTranslated,FALSE);
		if (!bTranslated)
			m_pSchedule->m_wCpuUsageTheshold=WORD(-1);
	}
	else
		m_pSchedule->m_wCpuUsageTheshold=WORD(-1);


	switch (SendDlgItemMessage(IDC_THREADPRIORITY,CB_GETCURSEL))
	{
	case 0:
		m_pSchedule->m_nThreadPriority=THREAD_PRIORITY_HIGHEST;
        break;
	case 1:
        m_pSchedule->m_nThreadPriority=THREAD_PRIORITY_ABOVE_NORMAL;
        break;
	case 3:
		m_pSchedule->m_nThreadPriority=THREAD_PRIORITY_BELOW_NORMAL;
        break;
	case 4:
        m_pSchedule->m_nThreadPriority=THREAD_PRIORITY_LOWEST;
        break;
	case 5:
        m_pSchedule->m_nThreadPriority=THREAD_PRIORITY_IDLE;
        break;
	default:
		m_pSchedule->m_nThreadPriority=THREAD_PRIORITY_NORMAL;
        break;
	}

	EndDialog(1);
	return TRUE;
}
	
BOOL CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::OnTypeChanged()
{
	CString Title;
	CString txt;
	
	CSchedule::ScheduleType nTypes[]={
		CSchedule::typeMinutely,
		CSchedule::typeHourly,
		CSchedule::typeDaily,
		CSchedule::typeWeekly,
		CSchedule::typeMonthly,
		CSchedule::typeOnce,
		CSchedule::typeAtStartup,
	};
		
	CSchedule::ScheduleType nType=nTypes[m_pTypeCombo->GetCurSel()];

	EnableDlgItem(IDC_TIME,nType!=CSchedule::typeMinutely && 
		nType!=CSchedule::typeHourly && nType!=CSchedule::typeAtStartup);
	EnableDlgItem(IDC_ATTHISTIME,nType!=CSchedule::typeMinutely && nType!=CSchedule::typeAtStartup);
	
    switch (nType)
	{
	case CSchedule::typeMinutely:
		txt.LoadString(IDS_MINUTELY);
		ShowDlgItem(IDC_EVERYTXT,swShow);
		ShowDlgItem(IDC_EVERY,swShow);
		ShowDlgItem(IDC_SPIN,swShow);
		ShowDlgItem(IDC_MINUTEONHOUR,swHide);
		ShowDlgItem(IDC_MINUTEONHOURLBL,swHide);
		ShowDlgItem(IDC_MINUTESPIN,swHide);
		ShowDlgItem(IDC_MINUTE,swShow);
		ShowDlgItem(IDC_DAYS,swHide);
		ShowDlgItem(IDC_HOURS,swHide);
		ShowDlgItem(IDC_WEEKS,swHide);
		ShowDlgItem(IDC_MON,swHide);
		ShowDlgItem(IDC_TUE,swHide);
		ShowDlgItem(IDC_WED,swHide);
		ShowDlgItem(IDC_THU,swHide);
		ShowDlgItem(IDC_FRI,swHide);
		ShowDlgItem(IDC_SAT,swHide);
		ShowDlgItem(IDC_SUN,swHide);
		ShowDlgItem(IDC_MDAY,swHide);
		ShowDlgItem(IDC_MTHE,swHide);
		ShowDlgItem(IDC_MEVERY,swHide);
		ShowDlgItem(IDC_MSPIN,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS2,swHide);
		ShowDlgItem(IDC_MTYPE,swHide);
		ShowDlgItem(IDC_MDAYS,swHide);
		ShowDlgItem(IDC_ONCETIME,swHide);
		ShowDlgItem(IDC_RUNON,swHide);
		::SetWindowPos(GetDlgItem(IDC_FRAME),HWND_TOP,0,0,328,50,SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		break;
	case CSchedule::typeHourly:
		txt.LoadString(IDS_HOURLY);
		ShowDlgItem(IDC_EVERYTXT,swShow);
		ShowDlgItem(IDC_EVERY,swShow);
		ShowDlgItem(IDC_SPIN,swShow);
		ShowDlgItem(IDC_MINUTEONHOUR,swShow);
		ShowDlgItem(IDC_MINUTEONHOURLBL,swShow);
		ShowDlgItem(IDC_MINUTESPIN,swShow); 
		ShowDlgItem(IDC_MINUTE,swHide);
		ShowDlgItem(IDC_DAYS,swHide);
		ShowDlgItem(IDC_HOURS,swShow);
		ShowDlgItem(IDC_WEEKS,swHide);
		ShowDlgItem(IDC_MON,swHide);
		ShowDlgItem(IDC_TUE,swHide);
		ShowDlgItem(IDC_WED,swHide);
		ShowDlgItem(IDC_THU,swHide);
		ShowDlgItem(IDC_FRI,swHide);
		ShowDlgItem(IDC_SAT,swHide);
		ShowDlgItem(IDC_SUN,swHide);
		ShowDlgItem(IDC_MDAY,swHide);
		ShowDlgItem(IDC_MTHE,swHide);
		ShowDlgItem(IDC_MEVERY,swHide);
		ShowDlgItem(IDC_MSPIN,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS2,swHide);
		ShowDlgItem(IDC_MTYPE,swHide);
		ShowDlgItem(IDC_MDAYS,swHide);
		ShowDlgItem(IDC_ONCETIME,swHide);
		ShowDlgItem(IDC_RUNON,swHide);
		::SetWindowPos(GetDlgItem(IDC_FRAME),HWND_TOP,0,0,328,50,SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		break;
	case CSchedule::typeDaily:
		txt.LoadString(IDS_DAILY);
		ShowDlgItem(IDC_EVERYTXT,swShow);
		ShowDlgItem(IDC_EVERY,swShow);
		ShowDlgItem(IDC_SPIN,swShow);
		ShowDlgItem(IDC_MINUTEONHOUR,swHide);
		ShowDlgItem(IDC_MINUTEONHOURLBL,swHide);
		ShowDlgItem(IDC_MINUTESPIN,swHide);
		ShowDlgItem(IDC_MINUTE,swHide);
		ShowDlgItem(IDC_DAYS,swShow);
		ShowDlgItem(IDC_HOURS,swHide);
		ShowDlgItem(IDC_WEEKS,swHide);
		ShowDlgItem(IDC_MON,swHide);
		ShowDlgItem(IDC_TUE,swHide);
		ShowDlgItem(IDC_WED,swHide);
		ShowDlgItem(IDC_THU,swHide);
		ShowDlgItem(IDC_FRI,swHide);
		ShowDlgItem(IDC_SAT,swHide);
		ShowDlgItem(IDC_SUN,swHide);
		ShowDlgItem(IDC_MDAY,swHide);
		ShowDlgItem(IDC_MTHE,swHide);
		ShowDlgItem(IDC_MEVERY,swHide);
		ShowDlgItem(IDC_MSPIN,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS2,swHide);
		ShowDlgItem(IDC_MTYPE,swHide);
		ShowDlgItem(IDC_MDAYS,swHide);
		ShowDlgItem(IDC_ONCETIME,swHide);
		ShowDlgItem(IDC_RUNON,swHide);
		::SetWindowPos(GetDlgItem(IDC_FRAME),HWND_TOP,0,0,328,50,SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		break;
	case CSchedule::typeWeekly:
		txt.LoadString(IDS_WEEKLY);
		ShowDlgItem(IDC_EVERYTXT,swShow);
		ShowDlgItem(IDC_EVERY,swShow);
		ShowDlgItem(IDC_SPIN,swShow);
		ShowDlgItem(IDC_MINUTEONHOUR,swHide);
		ShowDlgItem(IDC_MINUTEONHOURLBL,swHide);
		ShowDlgItem(IDC_MINUTESPIN,swHide);
		ShowDlgItem(IDC_MINUTE,swHide);
		ShowDlgItem(IDC_DAYS,swHide);
		ShowDlgItem(IDC_HOURS,swHide);
		ShowDlgItem(IDC_WEEKS,swShow);
		ShowDlgItem(IDC_MON,swShow);
		ShowDlgItem(IDC_TUE,swShow);
		ShowDlgItem(IDC_WED,swShow);
		ShowDlgItem(IDC_THU,swShow);
		ShowDlgItem(IDC_FRI,swShow);
		ShowDlgItem(IDC_SAT,swShow);
		ShowDlgItem(IDC_SUN,swShow);
		ShowDlgItem(IDC_MDAY,swHide);
		ShowDlgItem(IDC_MTHE,swHide);
		ShowDlgItem(IDC_MEVERY,swHide);
		ShowDlgItem(IDC_MSPIN,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS2,swHide);
		ShowDlgItem(IDC_MTYPE,swHide);
		ShowDlgItem(IDC_MDAYS,swHide);
		ShowDlgItem(IDC_ONCETIME,swHide);
		ShowDlgItem(IDC_RUNON,swHide);
		::SetWindowPos(GetDlgItem(IDC_FRAME),HWND_TOP,0,0,328,115,SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		break;
	case CSchedule::typeMonthly:
		txt.LoadString(IDS_MONTHLY);
		ShowDlgItem(IDC_EVERYTXT,swHide);
		ShowDlgItem(IDC_EVERY,swHide);
		ShowDlgItem(IDC_SPIN,swHide);
		ShowDlgItem(IDC_MINUTEONHOUR,swHide);
		ShowDlgItem(IDC_MINUTEONHOURLBL,swHide);
		ShowDlgItem(IDC_MINUTESPIN,swHide);
		ShowDlgItem(IDC_MINUTE,swHide);
		ShowDlgItem(IDC_DAYS,swHide);
		ShowDlgItem(IDC_HOURS,swHide);
		ShowDlgItem(IDC_WEEKS,swHide);
		ShowDlgItem(IDC_MON,swHide);
		ShowDlgItem(IDC_TUE,swHide);
		ShowDlgItem(IDC_WED,swHide);
		ShowDlgItem(IDC_THU,swHide);
		ShowDlgItem(IDC_FRI,swHide);
		ShowDlgItem(IDC_SAT,swHide);
		ShowDlgItem(IDC_SUN,swHide);
		ShowDlgItem(IDC_MDAY,swShow);
		ShowDlgItem(IDC_MTHE,swShow);
		ShowDlgItem(IDC_MEVERY,swShow);
		ShowDlgItem(IDC_MSPIN,swShow);
		ShowDlgItem(IDC_OFTHEMONTHS,swShow);
		ShowDlgItem(IDC_OFTHEMONTHS2,swShow);
		ShowDlgItem(IDC_MTYPE,swShow);
		ShowDlgItem(IDC_MDAYS,swShow);
		ShowDlgItem(IDC_ONCETIME,swHide);
		ShowDlgItem(IDC_RUNON,swHide);
		::SetWindowPos(GetDlgItem(IDC_FRAME),HWND_TOP,0,0,328,115,SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		break;
	case CSchedule::typeOnce:
		txt.LoadString(IDS_ONCE);
		ShowDlgItem(IDC_EVERYTXT,swHide);
		ShowDlgItem(IDC_EVERY,swHide);
		ShowDlgItem(IDC_SPIN,swHide);
		ShowDlgItem(IDC_MINUTEONHOUR,swHide);
		ShowDlgItem(IDC_MINUTEONHOURLBL,swHide);
		ShowDlgItem(IDC_MINUTESPIN,swHide);
		ShowDlgItem(IDC_MINUTE,swHide);
		ShowDlgItem(IDC_DAYS,swHide);
		ShowDlgItem(IDC_HOURS,swHide);
		ShowDlgItem(IDC_WEEKS,swHide);
		ShowDlgItem(IDC_MON,swHide);
		ShowDlgItem(IDC_TUE,swHide);
		ShowDlgItem(IDC_WED,swHide);
		ShowDlgItem(IDC_THU,swHide);
		ShowDlgItem(IDC_FRI,swHide);
		ShowDlgItem(IDC_SAT,swHide);
		ShowDlgItem(IDC_SUN,swHide);
		ShowDlgItem(IDC_MDAY,swHide);
		ShowDlgItem(IDC_MTHE,swHide);
		ShowDlgItem(IDC_MEVERY,swHide);
		ShowDlgItem(IDC_MSPIN,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS2,swHide);
		ShowDlgItem(IDC_MTYPE,swHide);
		ShowDlgItem(IDC_MDAYS,swHide);
		ShowDlgItem(IDC_ONCETIME,swShow);
		ShowDlgItem(IDC_RUNON,swShow);
		::SetWindowPos(GetDlgItem(IDC_FRAME),HWND_TOP,0,0,328,50,SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		break;
	case CSchedule::typeAtStartup:
		txt.LoadString(IDS_ATSTARTUP);
		ShowDlgItem(IDC_EVERYTXT,swHide);
		ShowDlgItem(IDC_EVERY,swHide);
		ShowDlgItem(IDC_SPIN,swHide);
		ShowDlgItem(IDC_MINUTEONHOUR,swHide);
		ShowDlgItem(IDC_MINUTEONHOURLBL,swHide);
		ShowDlgItem(IDC_MINUTESPIN,swHide);
		ShowDlgItem(IDC_MINUTE,swHide);
		ShowDlgItem(IDC_DAYS,swHide);
		ShowDlgItem(IDC_HOURS,swHide);
		ShowDlgItem(IDC_WEEKS,swHide);
		ShowDlgItem(IDC_MON,swHide);
		ShowDlgItem(IDC_TUE,swHide);
		ShowDlgItem(IDC_WED,swHide);
		ShowDlgItem(IDC_THU,swHide);
		ShowDlgItem(IDC_FRI,swHide);
		ShowDlgItem(IDC_SAT,swHide);
		ShowDlgItem(IDC_SUN,swHide);
		ShowDlgItem(IDC_MDAY,swHide);
		ShowDlgItem(IDC_MTHE,swHide);
		ShowDlgItem(IDC_MEVERY,swHide);
		ShowDlgItem(IDC_MSPIN,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS,swHide);
		ShowDlgItem(IDC_OFTHEMONTHS2,swHide);
		ShowDlgItem(IDC_MTYPE,swHide);
		ShowDlgItem(IDC_MDAYS,swHide);
		ShowDlgItem(IDC_ONCETIME,swHide);
		ShowDlgItem(IDC_RUNON,swHide);
		::SetWindowPos(GetDlgItem(IDC_FRAME),HWND_TOP,0,0,0,0,SWP_HIDEWINDOW|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOSIZE);
		break;
	}
	Title.Format(IDS_SCHEDULEUPDATE,(LPCSTR)txt);
	SetDlgItemText(IDC_FRAME,Title);
	return TRUE;
}

















		
////////////////////////////////////////
// CKeyboardShortcutsPage
////////////////////////////////////////



CSettingsProperties::CKeyboardShortcutsPage::CKeyboardShortcutsPage()
:	CPropertyPage(IDD_KEYBOARDSHORTCUTS,IDS_SHORTCUTSETTINGS),
	m_pList(NULL),m_pToolBar(NULL),m_ToolBarBitmaps(NULL),
	m_ToolBarBitmapsDisabled(NULL),m_ToolBarBitmapsHot(NULL),
	m_pCurrentShortcut(NULL)
{
	m_pPossibleControlsToActivate=CAction::GetPossibleControlValuesToActivate();
	m_pPossibleControlsToChange=CAction::GetPossibleControlValuesToChange();
	m_pPossibleMenuCommands=CAction::GetPossibleMenuCommands();
	m_pVirtualKeyNames=CShortcut::GetVirtualKeyNames();

	

	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg!=NULL)
	{
		bFreeDialogs=FALSE;
		hDialogs[0]=*pLocateDlg;
		hDialogs[1]=pLocateDlg->m_NameDlg;
		hDialogs[2]=pLocateDlg->m_SizeDateDlg;
		hDialogs[3]=pLocateDlg->m_AdvancedDlg;
		hDialogs[4]=NULL;
	}
	else
	{
		bFreeDialogs=TRUE;
		hDialogs[0]=CreateDialog(GetLanguageSpecificResourceHandle(),
			MAKEINTRESOURCE(IDD_MAIN),NULL,(DLGPROC)CLocateApp::DummyDialogProc);
		hDialogs[1]=CreateDialog(GetLanguageSpecificResourceHandle(),
			MAKEINTRESOURCE(IDD_NAME),hDialogs[0],(DLGPROC)CLocateApp::DummyDialogProc);
		hDialogs[2]=CreateDialog(GetLanguageSpecificResourceHandle(),
			MAKEINTRESOURCE(IDD_SIZEDATE),hDialogs[0],(DLGPROC)CLocateApp::DummyDialogProc);
		hDialogs[3]=CreateDialog(GetLanguageSpecificResourceHandle(),
			MAKEINTRESOURCE(IDD_ADVANCED),hDialogs[0],(DLGPROC)CLocateApp::DummyDialogProc);
		hDialogs[4]=NULL;
	}

	hMainMenu=::LoadMenu(IDR_MAINMENU);
	hPopupMenu=::LoadMenu(IDR_POPUPMENU);
}

CSettingsProperties::CKeyboardShortcutsPage::~CKeyboardShortcutsPage()
{

	delete[] m_pPossibleControlsToActivate;
	delete[] m_pPossibleControlsToChange;
	delete[] m_pPossibleMenuCommands;
	for (int i=0;m_pVirtualKeyNames[i].bKey!=0;i++)
		delete[] m_pVirtualKeyNames[i].pName;
	delete[] m_pVirtualKeyNames;

	m_aPossiblePresets.RemoveAll();

	if (bFreeDialogs)
	{
		for (int i=0;hDialogs[i]!=NULL;i++)
            ::DestroyWindow(hDialogs[i]);
	}

	::DestroyMenu(hMainMenu);
	::DestroyMenu(hPopupMenu);
}

BOOL CSettingsProperties::CKeyboardShortcutsPage::OnInitDialog(HWND hwndFocus)
{
	CPropertyPage::OnInitDialog(hwndFocus);

	m_pList=new CListCtrl(GetDlgItem(IDC_KEYLIST));

	m_pList->InsertColumn(0,ID2A(IDS_SHORTCUTLISTLABELSHORTCUT),LVCFMT_LEFT,80);
	m_pList->InsertColumn(1,ID2A(IDS_SHORTCUTLISTLABELTYPE),LVCFMT_LEFT,60);
	m_pList->InsertColumn(2,ID2A(IDS_SHORTCUTLISTLABELACTION),LVCFMT_LEFT,200);
	
	m_pList->SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT ,LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT );
	m_pList->LoadColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\Dialogs","Shortcuts Settings List Widths");

	m_ToolBarBitmaps.Create(IDB_SHORTCUTACTIONSBITMAPS,15,0,RGB(255,255,255));
	m_ToolBarBitmapsHot.Create(IDB_SHORTCUTACTIONSBITMAPSH,15,0,RGB(255,255,255));
	m_ToolBarBitmapsDisabled.Create(IDB_SHORTCUTACTIONSBITMAPSD,15,0,RGB(255,255,255));

	m_pToolBar=new CToolBarCtrl(GetDlgItem(IDC_ACTIONTOOLBAR));
	m_pToolBar->SetImageList(m_ToolBarBitmaps);
	m_pToolBar->SetDisabledImageList(m_ToolBarBitmapsDisabled);
	m_pToolBar->SetHotImageList(m_ToolBarBitmapsHot);
	
	TBBUTTON toolbuttons[]={
#pragma warning (disable :4305 4309)
		{0,IDC_ADDACTION,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
		{1,IDC_REMOVEACTION,0,TBSTYLE_BUTTON,0,0,0,0},
		{2,IDC_PREV,0,TBSTYLE_BUTTON,0,0,0,0},
		{3,IDC_NEXT,0,TBSTYLE_BUTTON,0,0,0,0},
		{4,IDC_SWAPWITHPREVIOUS,0,TBSTYLE_BUTTON,0,0,0,0},
		{5,IDC_SWAPWITHNEXT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0}
#pragma warning (default :4305 4309)
	};
	m_pToolBar->AddButtons(6,toolbuttons);
	m_pToolBar->SetWindowPos(HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);

	
	m_pWhenPressedList=new CListCtrl(GetDlgItem(IDC_WHENPRESSED));
	m_pWhenPressedList->InsertColumn(0,"",LVCFMT_LEFT,150);
	m_pWhenPressedList->SetExtendedListViewStyle(LVS_EX_CHECKBOXES,LVS_EX_CHECKBOXES);
	if (IsUnicodeSystem())
		m_pWhenPressedList->SetUnicodeFormat(TRUE);
	
	m_pWhenPressedList->InsertItem(LVIF_TEXT|LVIF_PARAM,0,ID2W(IDS_SHORTCUTWHENFOCUSINRESULTLIST),0,0,0,CShortcut::wpFocusInResultList);
	m_pWhenPressedList->InsertItem(LVIF_TEXT|LVIF_PARAM,1,ID2W(IDS_SHORTCUTWHENFOCUSNOTINRESULTLIST),0,0,0,CShortcut::wpFocusNotInResultList);
	m_pWhenPressedList->InsertItem(LVIF_TEXT|LVIF_PARAM,2,ID2W(IDS_SHORTCUTWHENNAMETABSHOWN),0,0,0,CShortcut::wpNameTabShown);
	m_pWhenPressedList->InsertItem(LVIF_TEXT|LVIF_PARAM,3,ID2W(IDS_SHORTCUTWHENSIZEDATETABSHOWN),0,0,0,CShortcut::wpSizeDateTabShown);
	m_pWhenPressedList->InsertItem(LVIF_TEXT|LVIF_PARAM,4,ID2W(IDS_SHORTCUTWHENADVANCEDTABSHOWN),0,0,0,CShortcut::wpAdvancedTabShown);
	
	
	// Check wheter Advanced items should be added
	m_ActionCombo.AssignToDlgItem(*this,IDC_ACTION);
	m_SubActionCombo.AssignToDlgItem(*this,IDC_SUBACTION);
	m_VerbCombo.AssignToDlgItem(*this,IDC_VERB);
	m_WhichFileCombo.AssignToDlgItem(*this,IDC_WHICHFILE);

	// Insert action categories
	m_ActionCombo.AddString(ID2W(IDS_NONE));
	m_ActionCombo.AddString(ID2W(IDS_ACTIONCATACTIVATECONTROL));
	m_ActionCombo.AddString(ID2W(IDS_ACTIONCATACTIVATETAB));
	m_ActionCombo.AddString(ID2W(IDS_ACTIONCATMENUCOMMAND));
	m_ActionCombo.AddString(ID2W(IDS_ACTIONCATSHOWHIDEDIALOG));
	m_ActionCombo.AddString(ID2W(IDS_ACTIONCATTRESULTLIST));
	m_ActionCombo.AddString(ID2W(IDS_ACTIONCATMISC));
	m_ActionCombo.AddString(ID2W(IDS_ACTIONCATCHANGEVALUE));
	m_ActionCombo.AddString(ID2W(IDS_ACTIONCATPRESETS));

	m_VerbCombo.AddString(ID2W(IDS_DEFAULT));

	// Insert "next/prev file"s
	m_WhichFileCombo.AddString(ID2W(IDS_ACTIONRESITEMNEXTFILE));
	m_WhichFileCombo.AddString(ID2W(IDS_ACTIONRESITEMPREVFILE));
	m_WhichFileCombo.AddString(ID2W(IDS_ACTIONRESITEMNEXTNONDELETEDFILE));
	m_WhichFileCombo.AddString(ID2W(IDS_ACTIONRESITEMPREVNONDELETEDFILE));


	// Insert verbs
	m_VerbCombo.AddString("open");
	m_VerbCombo.AddString("edit");
	m_VerbCombo.AddString("explore");
	m_VerbCombo.AddString("find");
	m_VerbCombo.AddString("print");

	// Enumerate presets
	CRegKey2 RegKey;
	if (RegKey.OpenKey(HKCU,"\\Dialogs\\SearchPresets",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		CRegKey PresetKey;
		CComboBox Combo(GetDlgItem(IDC_PRESETS));
		char szBuffer[30];

		for (int nPreset=0;nPreset<1000;nPreset++)
		{
			StringCbPrintf(szBuffer,30,"Preset %03d",nPreset);
	
			if (PresetKey.OpenKey(RegKey,szBuffer,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
				break;
	
			DWORD dwLength=PresetKey.QueryValueLength("");
			if (dwLength>0)
			{
				CStringW PresetName;
				//WCHAR* pPresetName=new WCHAR[dwLength+1];
				//PresetKey.QueryValue(L"",pPresetName,dwLength);
				//m_aPossiblePresets.Add(pPresetName);
				PresetKey.QueryValue(L"",PresetName);
				m_aPossiblePresets.Add(PresetName.GiveBuffer());				
			}
	
			PresetKey.CloseKey();
		}		
	}

	// Current selections
	m_ActionCombo.SetCurSel(0);
	m_VerbCombo.SetCurSel(0);
	m_WhichFileCombo.SetCurSel(0);

	// Inserting items
	InsertShortcuts();
	
	ClearActionFields();
	EnableItems();


		
	return FALSE;
}



void CSettingsProperties::CKeyboardShortcutsPage::InsertShortcuts()
{
	LVITEM li;
	li.pszText=LPSTR_TEXTCALLBACK;
	li.mask=LVIF_TEXT|LVIF_PARAM;
	li.iSubItem=0;
	li.iItem=0;
	
	for (int i=0;i<m_pSettings->m_aShortcuts.GetSize();i++)
	{
		li.lParam=LPARAM(m_pSettings->m_aShortcuts[i]);
		
		m_pList->InsertItem(&li);
		li.iItem++;
	}
	delete[] m_pSettings->m_aShortcuts.GiveBuffer();
}

void CSettingsProperties::CKeyboardShortcutsPage::InsertSubActions()
{
	ASSERT(m_pCurrentShortcut!=NULL);
	ASSERT(m_nCurrentAction>=0 && m_nCurrentAction<m_pCurrentShortcut->m_apActions.GetSize());
	
	m_SubActionCombo.ResetContent();

	CAction::Action nAction=GetSelectedAction();
	
	CStringW SubActionLabel;
	switch (nAction)
	{
	case CAction::ActivateControl:
	case CAction::ChangeValue:
		SubActionLabel.LoadString(IDS_SHORTCUTSUBACTIONCONTROL);
		break;
	case CAction::ActivateTab:
		SubActionLabel.LoadString(IDS_SHORTCUTSUBACTIONTAB);
		break;
	case CAction::MenuCommand:
		SubActionLabel.LoadString(IDS_SHORTCUTSUBACTIONMENUITEM);
		break;
	case CAction::Presets:
		SubActionLabel.LoadString(IDS_SHORTCUTSUBACTIONPRESET);
		break;
	default:
		SubActionLabel.LoadString(IDS_SHORTCUTSUBACTION);
		break;
	}
	SetDlgItemText(IDC_STATICSUBACTION,SubActionLabel);

	UINT nSubAction;
	for (int nIndex=0;(nSubAction=IndexToSubAction(nAction,nIndex))!=(UINT)-1;nIndex++)
	{
		CStringW Title;
		if (GetSubActionLabel(Title,nAction,nSubAction))
			m_SubActionCombo.AddString(Title);
		else
			m_SubActionCombo.AddString(ID2W(IDS_UNKNOWN));
	}

	m_SubActionCombo.SetCurSel(0);

	if (m_pCurrentShortcut->m_apActions[m_nCurrentAction]->m_nAction==nAction)
	{
		UINT nIndex=SubActionToIndex(nAction,m_pCurrentShortcut->m_apActions[m_nCurrentAction]->m_nSubAction);
		if (nIndex!=(UINT)-1)
			m_SubActionCombo.SetCurSel(nIndex);
	}
}


UINT CSettingsProperties::CKeyboardShortcutsPage::IndexToSubAction(CAction::Action nAction,UINT nIndex) const
{
	switch (nAction)
	{
	case CAction::None:
		break;
	case CAction::ActivateControl:
		if (m_pPossibleControlsToActivate[nIndex]==CAction::NullControl)
			return (UINT)-1;
		return m_pPossibleControlsToActivate[nIndex];
	case CAction::ActivateTab:
		if (nIndex>CAction::ActivateTabsLast)
			return (UINT)-1;
		return nIndex;
	case CAction::MenuCommand:
		if (m_pPossibleMenuCommands[nIndex]==CAction::NullMenuCommand)
			return (UINT)-1;
		return m_pPossibleMenuCommands[nIndex];
	case CAction::ShowHideDialog:
		if (nIndex>CAction::ShowHideDialogLast)
			return (UINT)-1;
		return nIndex;
	case CAction::ResultListItems:
		if (nIndex>CAction::ResultListLast)
			return (UINT)-1;
		return nIndex;
	case CAction::Misc:
		if (nIndex>CAction::MiscLast)
			return (UINT)-1;
		return nIndex;
	case CAction::ChangeValue:
		if (m_pPossibleControlsToChange[nIndex]==CAction::NullControl)
			return (UINT)-1;
		return m_pPossibleControlsToChange[nIndex];
	case CAction::Presets:
		if (nIndex>=(UINT)m_aPossiblePresets.GetSize())
			return (UINT)-1;
		return nIndex;
	}
	return (UINT)-1;
}

UINT CSettingsProperties::CKeyboardShortcutsPage::SubActionToIndex(CAction::Action nAction,UINT nSubAction) const
{
	switch (nAction)
	{
	case CAction::None:
		return (UINT)-1;
	case CAction::ActivateControl:
		{
			for (int nIndex=0;m_pPossibleControlsToActivate[nIndex]!=CAction::NullControl;nIndex++)
			{
				if (m_pPossibleControlsToActivate[nIndex]==nSubAction)
					return nIndex;
			}
			return (UINT)-1;
		}
	case CAction::ActivateTab:
		if (nSubAction>CAction::ActivateTabsLast)
			return (UINT)-1;
		return nSubAction;
	case CAction::MenuCommand:
		{
			for (int nIndex=0;m_pPossibleMenuCommands[nIndex]!=CAction::NullMenuCommand;nIndex++)
			{
				if (m_pPossibleMenuCommands[nIndex]==nSubAction)
					return nIndex;
			}
			return (UINT)-1;
		}
	case CAction::ShowHideDialog:
		if (nSubAction>CAction::ShowHideDialogLast)
			return (UINT)-1;
		return nSubAction;
	case CAction::ResultListItems:
		if (nSubAction>CAction::ResultListLast)
			return (UINT)-1;
		return nSubAction;
	case CAction::Misc:
		if (nSubAction>CAction::MiscLast)
			return (UINT)-1;
		return nSubAction;
	case CAction::Presets:
		return nSubAction;
	case CAction::ChangeValue:
		{
			for (int nIndex=0;m_pPossibleControlsToChange[nIndex]!=CAction::NullControl;nIndex++)
			{
				if (m_pPossibleControlsToChange[nIndex]==nSubAction)
					return nIndex;
			}
			return (UINT)-1;
		}
	default:
		ASSERT(0);
		break;
	}
	return (UINT)-1;
	
}

BOOL CSettingsProperties::CKeyboardShortcutsPage::GetSubActionLabel(CStringW& str,CAction::Action nAction,UINT uSubAction) const
{
	switch (nAction)
	{
	case CAction::None:
		str.LoadString(IDS_NONE);
		return TRUE;
	case CAction::ActivateControl:
	case CAction::ChangeValue:
		{
			// Check which dialog contains control
			WORD wControlID=HIWORD(uSubAction);
			
			if (wControlID&(1<<15)) 
			{
				// First bit up, id corresponds to string
				str.LoadString(~wControlID);
				return !str.IsEmpty();
			}
			else if (wControlID!=0)
			{
				// Checking wheter dialog contains control 
				CWnd Control;
				for (int i=0;hDialogs[i]!=NULL;i++)
				{
					Control.AssignToDlgItem(hDialogs[i],wControlID);
					if (HWND(Control)!=NULL)
						break;
				}

				if (HWND(Control)!=NULL)
				{
					int nIndex;
					Control.GetWindowText(str);
					while ((nIndex=str.Find('&'))!=-1)
						str.DelChar(nIndex);
					
					
					return TRUE;
				}
			}
			return FALSE;
		}
	case CAction::ActivateTab:
		{
			str.LoadString(CAction::GetActivateTabsActionLabelStringId(
				(CAction::ActionActivateTabs)IndexToSubAction(CAction::ActivateTab,uSubAction)));		

			int nIndex;
			while ((nIndex=str.Find(L"&&"))!=-1)
				str.DelChar(nIndex);
			return TRUE;
		}
	case CAction::MenuCommand:
		{
			WCHAR szLabel[1000];
			MENUITEMINFOW mii;
			BYTE nSubMenu=CAction::GetMenuAndSubMenu((CAction::ActionMenuCommands)uSubAction);
			CMenu SubMenu(GetSubMenu((nSubMenu&128)?hMainMenu:hPopupMenu,nSubMenu&~128));
			
			mii.cbSize=sizeof(MENUITEMINFOW);
			mii.fMask=MIIM_ID|MIIM_TYPE;
			mii.dwTypeData=szLabel;
			mii.cch=1000;
				
			
			BOOL bRet=SubMenu.GetMenuItemInfo(LOWORD(uSubAction),FALSE,&mii);

			if (bRet && mii.fType==MFT_STRING)
			{
				str.Format((INT)HIWORD(uSubAction),szLabel);
				int nIndex=str.FindFirst(L'\t');
				if (nIndex!=-1)
					str.FreeExtra(nIndex);

				while ((nIndex=str.Find(L'&'))!=-1)
                    str.DelChar(nIndex);
				return TRUE;
			}
			return FALSE;
		}
	case CAction::ShowHideDialog:
		str.LoadString(CAction::GetShowHideDialogActionLabelStringId(
			(CAction::ActionShowHideDialog)IndexToSubAction(CAction::ShowHideDialog,uSubAction)));		
		break;
	case CAction::ResultListItems:
		str.LoadString(CAction::GetResultItemActionLabelStringId(
			(CAction::ActionResultList)IndexToSubAction(CAction::ResultListItems,uSubAction)));		
		break;
	case CAction::Misc:
		str.LoadString(CAction::GetMiscActionStringLabelId(
			(CAction::ActionMisc)IndexToSubAction(CAction::Misc,uSubAction)));
		break;
	case CAction::Presets:
		ASSERT(uSubAction<(UINT)m_aPossiblePresets.GetSize());
		str.Copy(m_aPossiblePresets[(int)uSubAction]);
		break;
	default:
		ASSERT(0);
		return FALSE;
	}

	return TRUE;
}


BOOL CSettingsProperties::CKeyboardShortcutsPage::OnApply()
{
	CPropertyPage::OnApply();
	
	DebugMessage("CKeyboardShortcutsPage::OnApply() BEGIN");

	if (m_pCurrentShortcut!=NULL)
		SaveFieldsForShortcut(m_pCurrentShortcut);

	m_pSettings->m_aShortcuts.RemoveAll();
	
	if (m_pList->GetItemCount()>0)
	{
		int nItem=m_pList->GetNextItem(-1,LVNI_ALL);
		while (nItem!=-1)
		{
			CShortcut* pShortcut=(CShortcut*)m_pList->GetItemData(nItem);
			if (pShortcut!=NULL)
			{
				m_pSettings->m_aShortcuts.Add(pShortcut);
				m_pList->SetItemData(nItem,NULL);
			}
			nItem=m_pList->GetNextItem(nItem,LVNI_ALL);
		}
	}

	DebugMessage("CKeyboardShortcutsPage::OnApply() END");
	
	return TRUE;
}

void CSettingsProperties::CKeyboardShortcutsPage::OnDestroy()
{
	DebugMessage("CKeyboardShortcutsPage::OnDestroy() BEGIN");

	CPropertyPage::OnDestroy();

	

	if (m_pList!=NULL)
	{
		m_pList->SaveColumnsState(HKCU,CRegKey2::GetCommonKey()+"\\Dialogs","Shortcuts Settings List Widths");
		delete m_pList;
		m_pList=NULL;
	}

	if (m_pToolBar!=NULL)
	{
		delete m_pToolBar;
		m_pToolBar=NULL;
	}

	if (m_pWhenPressedList!=NULL)
	{
		delete m_pWhenPressedList;
		m_pWhenPressedList=NULL;
	}

	DebugMessage("CKeyboardShortcutsPage::OnDestroy() END");
}
		
void CSettingsProperties::CKeyboardShortcutsPage::OnCancel()
{
	m_pSettings->SetSettingsFlags(CSettingsProperties::settingsCancelled);

	CPropertyPage::OnCancel();
}

void CSettingsProperties::CKeyboardShortcutsPage::OnTimer(DWORD wTimerID)
{
	/*KillTimer(wTimerID);

	if (m_pList->GetNextItem(-1,LVNI_SELECTED)==-1)
		m_pList->SetItemState(nLastSel,LVIS_SELECTED,LVIS_SELECTED);*/
}

BOOL CSettingsProperties::CKeyboardShortcutsPage::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	ASSERT(idCtrl==pnmh->idFrom);
	
	switch (idCtrl)
	{
	case IDC_KEYLIST:
		ListNotifyHandler((NMLISTVIEW*)pnmh);
		break;
	case IDC_WHEREPRESSED:
		WherePressedNotifyHandler((NMLISTVIEW*)pnmh);
		break;
	default:
		if (pnmh->code==TTN_NEEDTEXT)
		{
			if (g_szBuffer!=NULL)
				delete[] g_szBuffer;

			switch (pnmh->idFrom)
			{
			case IDC_ADDACTION:
				g_szBuffer=allocstring(IDS_SHORTCUTADDACTION);
				break;
			case IDC_REMOVEACTION:
				g_szBuffer=allocstring(IDS_SHORTCUTREMOVEACTION);
				break;
			case IDC_NEXT:
				g_szBuffer=allocstring(IDS_SHORTCUTNEXTACTION);
				break;
			case IDC_PREV:
				g_szBuffer=allocstring(IDS_SHORTCUTPREVACTION);
				break;
			case IDC_SWAPWITHPREVIOUS:
				g_szBuffer=allocstring(IDS_SHORTCUTSWAPWITHPREVIOUS);
				break;
			case IDC_SWAPWITHNEXT:
				g_szBuffer=allocstring(IDS_SHORTCUTSWAPWITHNEXT);
				break;
			default:
				g_szBuffer=allocstring(IDS_UNKNOWN);
				break;
			}
			((LPTOOLTIPTEXT)pnmh)->lpszText=g_szBuffer;				
		}
		else if (pnmh->code==TTN_NEEDTEXTW)
		{
			if (g_szwBuffer!=NULL)
				delete[] g_szwBuffer;

			switch (pnmh->idFrom)
			{
			case IDC_ADDACTION:
				g_szwBuffer=allocstringW(IDS_SHORTCUTADDACTION);
				break;
			case IDC_REMOVEACTION:
				g_szwBuffer=allocstringW(IDS_SHORTCUTREMOVEACTION);
				break;
			case IDC_NEXT:
				g_szwBuffer=allocstringW(IDS_SHORTCUTNEXTACTION);
				break;
			case IDC_PREV:
				g_szwBuffer=allocstringW(IDS_SHORTCUTPREVACTION);
				break;
			case IDC_SWAPWITHPREVIOUS:
				g_szwBuffer=allocstringW(IDS_SHORTCUTSWAPWITHPREVIOUS);
				break;
			case IDC_SWAPWITHNEXT:
				g_szwBuffer=allocstringW(IDS_SHORTCUTSWAPWITHNEXT);
				break;
			default:
				g_szwBuffer=allocstringW(IDS_UNKNOWN);
				break;
			}
			((LPTOOLTIPTEXTW)pnmh)->lpszText=g_szwBuffer;				
		}
	}
	return CPropertyPage::OnNotify(idCtrl,pnmh);
}


		

BOOL CSettingsProperties::CKeyboardShortcutsPage::ListNotifyHandler(NMLISTVIEW *pNm)
{
	switch(pNm->hdr.code)
	{
	case LVN_DELETEITEM:
		if (pNm->lParam!=NULL)
			delete (CShortcut*)pNm->lParam;
		break;
	case LVN_GETDISPINFO:
		{
			LV_DISPINFO *pLvdi=(LV_DISPINFO *)pNm;
			//CShortcut* pShortcut=(CShortcut*)pLvdi->item.lParam;
			if (pLvdi->item.lParam==NULL)
				break;
			if (g_szBuffer!=NULL)
				delete[] g_szBuffer;
			
			// Retrieving state
			pLvdi->item.mask=LVIF_STATE;
			pLvdi->item.stateMask=LVIS_SELECTED;
			m_pList->GetItem(&pLvdi->item);
			pLvdi->item.mask=LVIF_TEXT;

			switch (pLvdi->item.iSubItem)
			{
			case 0:// Shortcut
				if (pLvdi->item.state&LVIS_SELECTED)
				{
					// Item is selected and not overrided, retrieving shortcut from hotkey control
					if (IsDlgButtonChecked(IDC_HOTKEYRADIO))
					{
						CStringW Buffer;
						WORD wKey=(WORD)SendDlgItemMessage(IDC_SHORTCUTKEY,HKM_GETHOTKEY,0,0);
						CShortcut::FormatKeyLabel(m_pVirtualKeyNames,LOBYTE(wKey),CShortcut::HotkeyModifiersToModifiers(HIBYTE(wKey)),
							(m_pCurrentShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode)?TRUE:FALSE,Buffer);
						g_szBuffer=alloccopyWtoA(Buffer,Buffer.GetLength());
					}
					else
					{
						BOOL bScancode=(m_pCurrentShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode)?TRUE:FALSE;
						BYTE bVKey=GetVirtualCode(m_pCurrentShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode);
						BYTE bModifiers=0;
							
						if (IsDlgButtonChecked(IDC_MODCTRL))
							bModifiers|=CShortcut::ModifierControl;
						if (IsDlgButtonChecked(IDC_MODALT))
							bModifiers|=CShortcut::ModifierAlt;
						if (IsDlgButtonChecked(IDC_MODSHIFT))
							bModifiers|=CShortcut::ModifierShift;
						if (IsDlgButtonChecked(IDC_MODWIN))
							bModifiers|=CShortcut::ModifierWin;
						CStringW Buffer;
						CShortcut::FormatKeyLabel(m_pVirtualKeyNames,bVKey,bModifiers,bScancode,Buffer);
						g_szBuffer=alloccopyWtoA(Buffer,Buffer.GetLength());
					}
				}
				else
				{
					CStringW Buffer;
					((CShortcut*)pLvdi->item.lParam)->FormatKeyLabel(m_pVirtualKeyNames,Buffer);
					g_szBuffer=alloccopyWtoA(Buffer,Buffer.GetLength());
				}
				break;
			case 1: // Type
				switch (((CShortcut*)pLvdi->item.lParam)->m_dwFlags&CShortcut::sfKeyTypeMask)
				{
				case CShortcut::sfGlobalHotkey:
					g_szBuffer=allocstring(IDS_ADVSHORTCUTGLOBALHOTKEY);
					break;
				case CShortcut::sfGlobalHook:
					g_szBuffer=allocstring(IDS_ADVSHORTCUTGLOBALHOOK);
					break;
				case CShortcut::sfLocal:
					g_szBuffer=allocstring(IDS_ADVSHORTCUTLOCAL);
					break;
				}
				break;
			case 2: // Action
				if (((CShortcut*)pLvdi->item.lParam)->m_apActions.GetSize()>1)
					g_szBuffer=allocstring(IDS_SHORTCUTMULTIPLEACTIONS);
				else if (pLvdi->item.state&LVIS_SELECTED)
				{
					int nCurSel=m_SubActionCombo.GetCurSel();
					UINT nSubAction=0;
					CAction::Action nAction=GetSelectedAction();
					if (nAction==CAction::None)
					{
						g_szBuffer=allocstring(IDS_NONE);
						break;
					}
					if ((nSubAction=IndexToSubAction(nAction,nCurSel))!=UINT(-1))
					{
						CStringW Buffer;
						FormatActionLabel(Buffer,nAction,nSubAction);
						g_szBuffer=alloccopyWtoA(Buffer,Buffer.GetLength());
						break;
					}
					
					g_szBuffer=allocstring(IDS_UNKNOWN);
				}				
				else
				{
					CStringW Buffer;
					FormatActionLabel(Buffer,(CAction::Action)((CShortcut*)pLvdi->item.lParam)->m_apActions[0]->m_nAction,
						((CShortcut*)pLvdi->item.lParam)->m_apActions[0]->m_nSubAction);
					g_szBuffer=alloccopyWtoA(Buffer,Buffer.GetLength());
				}
				break;
			default:
				g_szBuffer=allocstring(IDS_UNKNOWN);
				break;
			}
			pLvdi->item.pszText=g_szBuffer;
			break;
		}
	case LVN_GETDISPINFOW:
		{
			LV_DISPINFOW *pLvdi=(LV_DISPINFOW *)pNm;
			//CShortcut* pShortcut=(CShortcut*)pLvdi->item.lParam;
			if (pLvdi->item.lParam==NULL)
				break;
			if (g_szwBuffer!=NULL)
				delete[] g_szwBuffer;
			
			// Retrieving state
			pLvdi->item.mask=LVIF_STATE;
			pLvdi->item.stateMask=LVIS_SELECTED;
			m_pList->GetItem(&pLvdi->item);
			pLvdi->item.mask=LVIF_TEXT;

			switch (pLvdi->item.iSubItem)
			{
			case 0:// Shortcut
				if (pLvdi->item.state&LVIS_SELECTED)
				{
					// Item is selected and not overrided, retrieving shortcut from hotkey control
					if (IsDlgButtonChecked(IDC_HOTKEYRADIO))
					{
						WORD wKey=(WORD)SendDlgItemMessage(IDC_SHORTCUTKEY,HKM_GETHOTKEY,0,0);
						CStringW Buffer;
						CShortcut::FormatKeyLabel(m_pVirtualKeyNames,LOBYTE(wKey),CShortcut::HotkeyModifiersToModifiers(HIBYTE(wKey)),
							(m_pCurrentShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode)?TRUE:FALSE,Buffer);
						g_szwBuffer=Buffer.GiveBuffer();
					}
					else
					{
						BOOL bScancode=(m_pCurrentShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode)?TRUE:FALSE;
						BYTE bVKey=GetVirtualCode(m_pCurrentShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode);
						BYTE bModifiers=0;
							
						if (IsDlgButtonChecked(IDC_MODCTRL))
							bModifiers|=CShortcut::ModifierControl;
						if (IsDlgButtonChecked(IDC_MODALT))
							bModifiers|=CShortcut::ModifierAlt;
						if (IsDlgButtonChecked(IDC_MODSHIFT))
							bModifiers|=CShortcut::ModifierShift;
						if (IsDlgButtonChecked(IDC_MODWIN))
							bModifiers|=CShortcut::ModifierWin;
						CStringW Buffer;
						CShortcut::FormatKeyLabel(m_pVirtualKeyNames,bVKey,bModifiers,bScancode,Buffer);
						g_szwBuffer=Buffer.GiveBuffer();
					}
				}
				else
				{
					CStringW Buffer;
					((CShortcut*)pLvdi->item.lParam)->FormatKeyLabel(m_pVirtualKeyNames,Buffer);
					g_szwBuffer=Buffer.GiveBuffer();
				}
				break;
			case 1: // Type
				switch (((CShortcut*)pLvdi->item.lParam)->m_dwFlags&CShortcut::sfKeyTypeMask)
				{
				case CShortcut::sfGlobalHotkey:
					g_szwBuffer=allocstringW(IDS_ADVSHORTCUTGLOBALHOTKEY);
					break;
				case CShortcut::sfGlobalHook:
					g_szwBuffer=allocstringW(IDS_ADVSHORTCUTGLOBALHOOK);
					break;
				case CShortcut::sfLocal:
					g_szwBuffer=allocstringW(IDS_ADVSHORTCUTLOCAL);
					break;
				}
				break;
			case 2: // Action
				if (((CShortcut*)pLvdi->item.lParam)->m_apActions.GetSize()>1)
					g_szwBuffer=allocstringW(IDS_SHORTCUTMULTIPLEACTIONS);
				else if (pLvdi->item.state&LVIS_SELECTED)
				{
					int nCurSel=m_SubActionCombo.GetCurSel();
					UINT nSubAction=0;
					CAction::Action nAction=GetSelectedAction();
					if (nAction==CAction::None)
					{
						g_szwBuffer=allocstringW(IDS_NONE);
						break;
					}
					if ((nSubAction=IndexToSubAction(nAction,nCurSel))!=UINT(-1))
					{
						CStringW Buffer;
						FormatActionLabel(Buffer,nAction,nSubAction);
						g_szwBuffer=Buffer.GiveBuffer();
						break;
					}
					
					g_szwBuffer=allocstringW(IDS_UNKNOWN);
				}				
				else
				{
					CStringW Buffer;
					FormatActionLabel(Buffer,(CAction::Action)((CShortcut*)pLvdi->item.lParam)->m_apActions[0]->m_nAction,
						((CShortcut*)pLvdi->item.lParam)->m_apActions[0]->m_nSubAction);
					g_szwBuffer=Buffer.GiveBuffer();
				}
				break;
			default:
				g_szwBuffer=allocstringW(IDS_UNKNOWN);
				break;
			}
			pLvdi->item.pszText=g_szwBuffer;
			break;
		}
	case LVN_ITEMCHANGED:
		OnChangeItem(pNm);
		break;
	case LVN_ITEMCHANGING:
		OnChangingItem(pNm);
		break;
	}
	return TRUE;
}

BOOL CSettingsProperties::CKeyboardShortcutsPage::WherePressedNotifyHandler(NMLISTVIEW *pNm)
{
	switch(pNm->hdr.code)
	{
	case LVN_ITEMCHANGING:
		if (pNm->uNewState&LVIS_SELECTED && !(pNm->uOldState&LVIS_SELECTED))
		{
			pNm->uNewState&=~LVIS_SELECTED;
			
			SetWindowLong(CWnd::dwlMsgResult,TRUE);
		}
		break;
	case LVN_ITEMCHANGED:
		if (pNm->uNewState&LVIS_SELECTED)
			m_pWhenPressedList->SetItemState(pNm->iItem,0,LVIS_SELECTED);
		break;
	}
	return TRUE;
}
BOOL CSettingsProperties::CKeyboardShortcutsPage::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);

	switch (wID)
	{
	case IDC_NEW:
		OnNewShortcut();
		break;
	case IDC_REMOVE:
		OnRemoveShortcut();
		break;
	case IDC_UP:
	case IDC_DOWN:
		ItemUpOrDown(wID==IDC_UP);
		break;
	case IDC_RESET:
		OnResetToDefaults();
		break;
	case IDC_ADVANCED:
		OnAdvanced();
		break;
	case IDC_ADDACTION:
		OnAddAction();
		break;
	case IDC_FROMMNEMONIC:
		SetFieldsRelativeToMnemonics();
		EnableItems();
		RefreshShortcutListLabels();
		break;
	case IDC_REMOVEACTION:
		OnRemoveAction();
		break;
	case IDC_NEXT:
	case IDC_PREV:
		OnNextAction(wID==IDC_NEXT);
		break;
	case IDC_SWAPWITHPREVIOUS:
	case IDC_SWAPWITHNEXT:
		OnSwapAction(wID==IDC_SWAPWITHNEXT);
		break;
	case IDC_ACTION:
	case IDC_SUBACTION:
		if (wNotifyCode==CBN_SELCHANGE)
		{
			SaveFieldsForAction(m_pCurrentShortcut->m_apActions[m_nCurrentAction]);
			if (wID==IDC_ACTION)
				InsertSubActions();

			
			if (m_pCurrentShortcut->m_dwFlags&CShortcut::sfUseMemonic)
			{
				if (m_pCurrentShortcut->GetMnemonicForAction(hDialogs)==0)
				{
					m_pCurrentShortcut->m_dwFlags&=~CShortcut::sfUseMemonic;
					CheckDlgButton(IDC_FROMMNEMONIC,FALSE);
				}
				else
					SetFieldsRelativeToMnemonics();
			}

			EnableItems();
			RefreshShortcutListLabels();
		}
		break;		
	case IDC_HOTKEYRADIO:
	case IDC_CODERADIO:
		if (IsDlgButtonChecked(wID))
			break;

		CheckDlgButton(IDC_HOTKEYRADIO,wID==IDC_HOTKEYRADIO);
		CheckDlgButton(IDC_CODERADIO,wID==IDC_CODERADIO);
		

		EnableDlgItem(IDC_SHORTCUTKEY,wID==IDC_HOTKEYRADIO);
		EnableDlgItem(IDC_CODE,wID==IDC_CODERADIO);
		EnableDlgItem(IDC_MODCTRL,wID==IDC_CODERADIO);
		EnableDlgItem(IDC_MODALT,wID==IDC_CODERADIO);
		EnableDlgItem(IDC_MODSHIFT,wID==IDC_CODERADIO);
		EnableDlgItem(IDC_MODWIN,wID==IDC_CODERADIO);

		InsertKeysToVirtualKeyCombo();
		RefreshShortcutListLabels();

		SetFocus(wID==IDC_HOTKEYRADIO?IDC_SHORTCUTKEY:IDC_CODE);
		break;
	case IDC_MODSHIFT:
	case IDC_MODCTRL:
	case IDC_MODALT:
	case IDC_MODWIN:
	case IDC_CODE:
		SetShortcutKeyWhenVirtualKeyChanged();
		RefreshShortcutListLabels();
		break;
	case IDC_SHORTCUTKEY:
		SetVirtualKeyWhenShortcutKeyChanged();
		RefreshShortcutListLabels();
		break;
	case IDC_VALUEHELPTEXT:
		{
			HRSRC hRc=FindResource(GetLanguageSpecificResourceHandle(),MAKEINTRESOURCE(IDR_CHANGEVALUEHELP),"HELPTEXT");
			HGLOBAL hGlobal=LoadResource(GetLanguageSpecificResourceHandle(),hRc);
			LPCSTR pStr=(LPCSTR)LockResource(hGlobal);

			// Counting length
			int len;
			for (len=0;pStr[len]!='\0';len++)
			{
				if (pStr[len]=='E' && pStr[len+1]=='O' && pStr[len+2]=='F')
					break;
			}


			MessageBox(CString(pStr,len),ID2A(IDS_HELPINFO),MB_OK|MB_ICONINFORMATION);
			break;
		}
	}
	return FALSE;
}

BOOL CSettingsProperties::CKeyboardShortcutsPage::ItemUpOrDown(BOOL bUp)
{
	ASSERT(m_pCurrentShortcut!=NULL);
	SaveFieldsForShortcut(m_pCurrentShortcut);

	int nSelected=m_pList->GetNextItem(-1,LVNI_SELECTED);
	ASSERT(nSelected!=-1);
	ASSERT(m_pCurrentShortcut==(CShortcut*)m_pList->GetItemData(nSelected));
	CShortcut* pSelected=m_pCurrentShortcut;
	
	
	int nOther=m_pList->GetNextItem(nSelected,bUp?LVNI_ABOVE:LVNI_BELOW);
	CShortcut* pOther=(CShortcut*)m_pList->GetItemData(nOther);
	ASSERT(pOther!=NULL);
	
	// Swapping

    // First set data to NULL
	m_pList->SetItemData(nSelected,NULL);
	m_pList->SetItemData(nOther,NULL);
	
	// Swap states
    UINT nState=m_pList->GetItemState(nSelected,0xFFFFFFFF);
	m_pList->SetItemState(nSelected,m_pList->GetItemState(nOther,0xFFFFFFFF),0xFFFFFFFF);
	m_pList->SetItemState(nOther,nState,0xFFFFFFFF);

	// Changes data
	m_pList->SetItemData(nSelected,(LPARAM)pOther);
	m_pList->SetItemData(nOther,(LPARAM)pSelected);
	
	// At this moment m_pCurrentShortcut points to pOther
	SetFieldsForShortcut(m_pCurrentShortcut=pSelected);
	
	
	
	m_pList->EnsureVisible(nOther,FALSE);
	m_pList->RedrawItems(min(nSelected,nOther),max(nSelected,nOther));
	m_pList->UpdateWindow();
	
	EnableItems();
	RefreshShortcutListLabels();
	return TRUE;
}


void CSettingsProperties::CKeyboardShortcutsPage::SetShortcutKeyWhenVirtualKeyChanged()
{
	ASSERT(m_pCurrentShortcut!=NULL);

	if (m_pCurrentShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode)
		return;

	
	BYTE bKey=GetVirtualCode(FALSE);
    BYTE bHotkey=0;
	if (IsDlgButtonChecked(IDC_MODALT))
		bHotkey=HOTKEYF_ALT;
	if (IsDlgButtonChecked(IDC_MODCTRL))
		bHotkey=HOTKEYF_CONTROL;
	if (IsDlgButtonChecked(IDC_MODSHIFT))
		bHotkey=HOTKEYF_SHIFT;

	SetHotKey(bKey,bHotkey);
	
}

void CSettingsProperties::CKeyboardShortcutsPage::SetVirtualKeyWhenShortcutKeyChanged()
{
	
	DWORD dwKey=(DWORD)SendDlgItemMessage(IDC_SHORTCUTKEY,HKM_GETHOTKEY,0,0);

    if (dwKey==0)
		return;


	SetVirtualCode(LOBYTE(dwKey),FALSE);
	
	CheckDlgButton(IDC_MODALT,(dwKey&(HOTKEYF_ALT<<8))?1:0);
	CheckDlgButton(IDC_MODSHIFT,(dwKey&(HOTKEYF_SHIFT<<8))?1:0);
	CheckDlgButton(IDC_MODCTRL,(dwKey&(HOTKEYF_CONTROL<<8))?1:0);
}

void CSettingsProperties::CKeyboardShortcutsPage::SetVirtualCode(BYTE bCode,BOOL bScanCode)
{
	if (!bScanCode)
	{
		// Check for VK_ code
		for (int i=0;m_pVirtualKeyNames[i].bKey!=0;i++)
		{
			if (m_pVirtualKeyNames[i].bKey==bCode)
			{
				SendDlgItemMessage(IDC_CODE,CB_SETCURSEL,i);
				SetDlgItemText(IDC_CODE,m_pVirtualKeyNames[i].pName);
				return;
			}
		}


		// First, check whether code is just ascii item
		BYTE pKeyState[256];
		GetKeyboardState(pKeyState);
		WORD wChar;
		int nRet=ToAscii(bCode,0,pKeyState,&wChar,0);
		if (nRet==1)
		{
			char text[]="\"X\"";
			text[1]=BYTE(wChar);
			MakeUpper(text+1,1);

			SendDlgItemMessage(IDC_CODE,CB_SETCURSEL,-1);
			SetDlgItemText(IDC_CODE,text);
			return;
		}

	}

	
	SetDlgItemInt(IDC_CODE,bCode,FALSE);
}

BYTE CSettingsProperties::CKeyboardShortcutsPage::GetVirtualCode(BOOL bScanCode) const
{
	if (!bScanCode)
	{
		int nSel=(int)SendDlgItemMessage(IDC_CODE,CB_GETCURSEL);
		if (nSel!=CB_ERR)
			return m_pVirtualKeyNames[nSel].bKey;
		
		DWORD dwTextLen=GetDlgItemTextLength(IDC_CODE);
        if (dwTextLen>2) 
		{
            char* pText=new char[dwTextLen+2];
			GetDlgItemText(IDC_CODE,pText,dwTextLen+2);
			
			if (pText[0]=='\"') // Has form "X"
			{
				SHORT sRet=VkKeyScan(pText[1]);
				return BYTE(sRet);
			}
		}
	}

	BOOL pTranslated;
	UINT nKey=GetDlgItemInt(IDC_CODE,&pTranslated,FALSE);
	if (!pTranslated || nKey>255)
		return 0;
	
	return (BYTE)nKey;
}


void CSettingsProperties::CKeyboardShortcutsPage::InsertKeysToVirtualKeyCombo()
{
	if (m_pCurrentShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode)
	{
		if (SendDlgItemMessage(IDC_CODE,CB_GETCURSEL)!=CB_ERR)
		{
			// Just reset
			SendDlgItemMessage(IDC_CODE,CB_RESETCONTENT);
			return;
		}

		// Save old value
		BYTE bCode=GetVirtualCode(FALSE);
		
		// Reset content
		SendDlgItemMessage(IDC_CODE,CB_RESETCONTENT);
		
		// Set old value
		SetVirtualCode(bCode,TRUE);
	}
	else
	{
		if (SendDlgItemMessage(IDC_CODE,CB_GETCOUNT)>0)
			return; // Do nothing, items are already listed

		// Save old value
		BYTE bCode=GetVirtualCode(TRUE);
		
		// Reset content
		SendDlgItemMessage(IDC_CODE,CB_RESETCONTENT);

		// Insert items
		for (int i=0;m_pVirtualKeyNames[i].bKey!=0;i++)
			SendDlgItemMessage(IDC_CODE,CB_ADDSTRING,0,(LPARAM)m_pVirtualKeyNames[i].pName);
		
		// Set old value
		SetVirtualCode(bCode,FALSE);

	}
	
}

void CSettingsProperties::CKeyboardShortcutsPage::OnNewShortcut()
{
	LVITEM li;
	li.pszText=LPSTR_TEXTCALLBACK;
	li.mask=LVIF_TEXT|LVIF_PARAM|LVIF_STATE;
	li.iSubItem=0;
	li.iItem=m_pList->GetItemCount();
	li.lParam=LPARAM(new CShortcut);
	li.stateMask=li.state=LVIS_SELECTED;
	
	int nItem=m_pList->InsertItem(&li);
	
	if (nItem!=-1)
		m_pList->EnsureVisible(nItem,FALSE);

	EnableItems();

	SetFocus(IDC_SHORTCUTKEY);

}

void CSettingsProperties::CKeyboardShortcutsPage::OnRemoveShortcut()
{
	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	ASSERT(nItem!=-1);
	
	m_pCurrentShortcut=NULL;
	
	CShortcut* pShortcut=(CShortcut*)m_pList->GetItemData(nItem);
	m_pList->SetItemData(nItem,NULL);
	if (pShortcut!=NULL)
		delete pShortcut;

	m_pList->DeleteItem(nItem);
	
	EnableItems();
}

void CSettingsProperties::CKeyboardShortcutsPage::OnResetToDefaults()
{
	m_pCurrentShortcut=NULL;
	m_pList->DeleteAllItems();

	m_pSettings->m_aShortcuts.RemoveAll();

	OSVERSIONINFOEX oi;
	oi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
	BOOL bRet=GetVersionEx((LPOSVERSIONINFO) &oi);
	BOOL bCanHook=!(bRet && oi.dwPlatformId!=VER_PLATFORM_WIN32_NT ||
		!(oi.dwMajorVersion>=5 || (oi.dwMajorVersion==4 && oi.wServicePackMajor>=3) ));


	if (!CShortcut::GetDefaultShortcuts(m_pSettings->m_aShortcuts,
		CShortcut::loadLocal|CShortcut::loadGlobalHotkey|(bCanHook?CShortcut::loadGlobalHook:0)))
	{
		ShowErrorMessage(IDS_ERRORCANNOTLOADDEFAULTSHORTUCS,IDS_ERROR);
		m_pSettings->m_aShortcuts.RemoveAll();
	}		
	else
		InsertShortcuts();


	RefreshShortcutListLabels();
	EnableItems();
}

void CSettingsProperties::CKeyboardShortcutsPage::OnAdvanced()
{
	ASSERT(m_pCurrentShortcut!=NULL);
	
    CAdvancedDlg dlg(m_pCurrentShortcut);
	SaveFieldsForShortcut(m_pCurrentShortcut);
	
	if (dlg.DoModal(*this))
	{
		InsertKeysToVirtualKeyCombo();
		SetFieldsForShortcut(m_pCurrentShortcut);
	}

	RefreshShortcutListLabels();
	EnableItems();
}

void CSettingsProperties::CKeyboardShortcutsPage::SetFieldsRelativeToMnemonics()
{
	ASSERT(m_pCurrentShortcut!=NULL);
	
	if (IsDlgButtonChecked(IDC_FROMMNEMONIC))
	{
		char cMnemonic=m_pCurrentShortcut->GetMnemonicForAction(hDialogs);
		ASSERT(cMnemonic!=0);

		m_pCurrentShortcut->m_dwFlags|=CShortcut::sfUseMemonic;
		


		// Convert to virtual key
		m_pCurrentShortcut->m_bVirtualKey=LOBYTE(VkKeyScan(cMnemonic));
		if (m_pCurrentShortcut->m_bVirtualKey==0)
			return;
		m_pCurrentShortcut->m_bModifiers=CShortcut::ModifierAlt;

        // Turn off scancode		
		m_pCurrentShortcut->m_dwFlags&=~CShortcut::sfVirtualKeyIsScancode;
		CheckDlgButton(IDC_VKISSCANCODE,FALSE);

        
		SetVirtualCode(m_pCurrentShortcut->m_bVirtualKey,FALSE);
		SetHotKeyForShortcut(m_pCurrentShortcut);

		
		CheckDlgButton(IDC_MODCTRL,FALSE);
		CheckDlgButton(IDC_MODALT,TRUE);
		CheckDlgButton(IDC_MODSHIFT,FALSE);
		CheckDlgButton(IDC_MODWIN,FALSE);

		
	}
	else
		m_pCurrentShortcut->m_dwFlags&=~CShortcut::sfUseMemonic;

}


void CSettingsProperties::CKeyboardShortcutsPage::OnAddAction()
{
	ASSERT(m_pCurrentShortcut!=NULL);
	ASSERT(m_nCurrentAction>=0 && m_nCurrentAction<m_pCurrentShortcut->m_apActions.GetSize());
	
	// Saving current fields
	SaveFieldsForAction(m_pCurrentShortcut->m_apActions[m_nCurrentAction]);

    // Creating new action
	m_pCurrentShortcut->m_apActions.Add(new CAction);

	// Activating it
	m_nCurrentAction=m_pCurrentShortcut->m_apActions.GetSize()-1;
	SetFieldsForAction(m_pCurrentShortcut->m_apActions[m_nCurrentAction]);

	EnableItems();
	RefreshShortcutListLabels();

	m_ActionCombo.SetFocus();
}

void CSettingsProperties::CKeyboardShortcutsPage::OnRemoveAction()
{
	ASSERT(m_pCurrentShortcut!=NULL);
	ASSERT(m_pCurrentShortcut->m_apActions.GetSize()>1);
	ASSERT(m_nCurrentAction>=0 && m_nCurrentAction<m_pCurrentShortcut->m_apActions.GetSize());
	
	
	// Delete shortcut
	m_pCurrentShortcut->m_apActions.RemoveAt(m_nCurrentAction);

	// If not the first action, activate previous
	if (m_nCurrentAction>0)
		m_nCurrentAction--;
	SetFieldsForAction(m_pCurrentShortcut->m_apActions[m_nCurrentAction]);

	EnableItems();
	RefreshShortcutListLabels();
}

void CSettingsProperties::CKeyboardShortcutsPage::OnNextAction(BOOL bNext)
{
	ASSERT(m_pCurrentShortcut!=NULL);
	ASSERT(m_nCurrentAction>=0 && m_nCurrentAction<m_pCurrentShortcut->m_apActions.GetSize());
	// If swap with previous, check that not first action
	ASSERT(m_nCurrentAction>0 || bNext);
	// If swap with next, check that not last action
	ASSERT(m_nCurrentAction<m_pCurrentShortcut->m_apActions.GetSize()-1 || !bNext);

	// Save current fields
	SaveFieldsForAction(m_pCurrentShortcut->m_apActions[m_nCurrentAction]);

	// Activate next/previous
	m_nCurrentAction+=bNext?+1:-1;
    SetFieldsForAction(m_pCurrentShortcut->m_apActions[m_nCurrentAction]);

	EnableItems();
}

void CSettingsProperties::CKeyboardShortcutsPage::OnSwapAction(BOOL bWithNext)
{
	ASSERT(m_pCurrentShortcut!=NULL);
	ASSERT(m_nCurrentAction>=0 && m_nCurrentAction<m_pCurrentShortcut->m_apActions.GetSize());
	// If swap with previous, check that not first action
	ASSERT(m_nCurrentAction>0 || bWithNext);
	// If swap with next, check that not last action
	ASSERT(m_nCurrentAction<m_pCurrentShortcut->m_apActions.GetSize()-1 || !bWithNext);

	// Save current fields
	SaveFieldsForAction(m_pCurrentShortcut->m_apActions[m_nCurrentAction]);

	// Swapping
	int m_nAnotherAction=m_nCurrentAction+(bWithNext?+1:-1);
	CAction* pTmp=m_pCurrentShortcut->m_apActions[m_nAnotherAction];
	m_pCurrentShortcut->m_apActions[m_nAnotherAction]=m_pCurrentShortcut->m_apActions[m_nCurrentAction];
	m_pCurrentShortcut->m_apActions[m_nCurrentAction]=pTmp;

	// Set current fields
	SetFieldsForAction(m_pCurrentShortcut->m_apActions[m_nCurrentAction=m_nAnotherAction]);
	
	EnableItems();
}


void CSettingsProperties::CKeyboardShortcutsPage::EnableItems()
{
	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);

	// Enable/disable remove button
	EnableDlgItem(IDC_REMOVE,nItem!=-1);
	
	// Enable/disable static group box
	EnableDlgItem(IDC_STATICSHORTCUT,nItem!=-1);
	
	


	// Enable/disable advanced button
	EnableDlgItem(IDC_ADVANCED,nItem!=-1);
	
	// Enable/disable action items
	EnableDlgItem(IDC_STATICACTIONS,nItem!=-1); // Groupbox
	EnableDlgItem(IDC_ACTIONTOOLBAR,nItem!=-1); // Toolbar
	EnableDlgItem(IDC_STATICACTION,nItem!=-1); // Action static text
	m_ActionCombo.EnableWindow(nItem!=-1); // Action combo
	EnableDlgItem(IDC_STATICSUBACTION,nItem!=-1); // Subaction static text
	m_SubActionCombo.EnableWindow(nItem!=-1); // Subaction combo

	ShowState ssVerb=swHide,ssMessage=swHide,ssCommand=swHide,ssWhichFile=swHide,ssChangeValue=swHide;
	
	if (nItem!=-1)
	{
	
		ASSERT(m_pCurrentShortcut==(CShortcut*)m_pList->GetItemData(nItem));
		ASSERT(m_nCurrentAction>=0 && m_nCurrentAction<m_pCurrentShortcut->m_apActions.GetSize());

		EnableDlgItem(IDC_FROMMNEMONIC,m_pCurrentShortcut->GetMnemonicForAction(hDialogs)!=0);
		
		BOOL bCodeChecked=IsDlgButtonChecked(IDC_CODERADIO);	
		// Enable/disable shortcut
		EnableDlgItem(IDC_HOTKEYRADIO,!(m_pCurrentShortcut->m_dwFlags&CShortcut::sfUseMemonic));
		EnableDlgItem(IDC_CODERADIO,!(m_pCurrentShortcut->m_dwFlags&CShortcut::sfUseMemonic));
		EnableDlgItem(IDC_SHORTCUTKEY,!bCodeChecked && !(m_pCurrentShortcut->m_dwFlags&CShortcut::sfUseMemonic));
		EnableDlgItem(IDC_CODE,bCodeChecked && !(m_pCurrentShortcut->m_dwFlags&CShortcut::sfUseMemonic));
		EnableDlgItem(IDC_MODCTRL,bCodeChecked && !(m_pCurrentShortcut->m_dwFlags&CShortcut::sfUseMemonic));
		EnableDlgItem(IDC_MODALT,bCodeChecked && !(m_pCurrentShortcut->m_dwFlags&CShortcut::sfUseMemonic));
		EnableDlgItem(IDC_MODSHIFT,bCodeChecked && !(m_pCurrentShortcut->m_dwFlags&CShortcut::sfUseMemonic));
		EnableDlgItem(IDC_MODWIN,bCodeChecked && !(m_pCurrentShortcut->m_dwFlags&CShortcut::sfUseMemonic));

		
		m_pToolBar->EnableButton(IDC_REMOVEACTION,m_pCurrentShortcut->m_apActions.GetSize()>=2);
		m_pToolBar->EnableButton(IDC_NEXT,m_nCurrentAction<=m_pCurrentShortcut->m_apActions.GetSize()-2);
		m_pToolBar->EnableButton(IDC_PREV,m_nCurrentAction>0);
		m_pToolBar->EnableButton(IDC_SWAPWITHNEXT,m_nCurrentAction<=m_pCurrentShortcut->m_apActions.GetSize()-2);
		m_pToolBar->EnableButton(IDC_SWAPWITHPREVIOUS,m_nCurrentAction>0);

		EnableDlgItem(IDC_STATICWHEREPRESSED,
			(m_pCurrentShortcut->m_dwFlags&CShortcut::sfKeyTypeMask)==CShortcut::sfLocal);
		EnableDlgItem(IDC_WHEREPRESSED,
			(m_pCurrentShortcut->m_dwFlags&CShortcut::sfKeyTypeMask)==CShortcut::sfLocal);

		int nOther=m_pList->GetNextItem(nItem,LVNI_ABOVE);
		EnableDlgItem(IDC_UP,nOther!=-1 && nOther!=nItem);
		nOther=m_pList->GetNextItem(nItem,LVNI_BELOW);
		EnableDlgItem(IDC_DOWN,nOther!=-1 && nOther!=nItem);

		switch (GetSelectedAction())
		{
		case CAction::None:
			EnableDlgItem(IDC_STATICSUBACTION,FALSE);
			m_SubActionCombo.EnableWindow(FALSE);
			break;
		case CAction::ResultListItems:
			switch (m_SubActionCombo.GetCurSel())
			{
			case CAction::Execute:
				ssVerb=swShow;
				break;
			case CAction::ExecuteCommand:
                ssCommand=swShow;
				break;
			case CAction::SelectFile:
				ssWhichFile=swShow;
				break;
			}
            break;
		case CAction::Misc:
			switch (m_SubActionCombo.GetCurSel())
			{
			case CAction::SendMessage:
			case CAction::PostMessage:
				ssMessage=swShow;
				break;
			case CAction::ExecuteCommandMisc:
				ssCommand=swShow;
				break;				
			}
			break;
		case CAction::ChangeValue:
			ssChangeValue=swShow;
			break;
		}
	}
	else
	{


		EnableDlgItem(IDC_HOTKEYRADIO,FALSE);
		EnableDlgItem(IDC_CODERADIO,FALSE);
		
		EnableDlgItem(IDC_SHORTCUTKEY,FALSE);
		EnableDlgItem(IDC_CODE,FALSE);
		EnableDlgItem(IDC_MODCTRL,FALSE);
		EnableDlgItem(IDC_MODALT,FALSE);
		EnableDlgItem(IDC_MODSHIFT,FALSE);
		EnableDlgItem(IDC_MODWIN,FALSE);

		EnableDlgItem(IDC_STATICWHEREPRESSED,FALSE);
		EnableDlgItem(IDC_WHEREPRESSED,FALSE);
		EnableDlgItem(IDC_FROMMNEMONIC,FALSE);

		EnableDlgItem(IDC_UP,FALSE);
		EnableDlgItem(IDC_DOWN,FALSE);


	}

	ShowDlgItem(IDC_STATICVERB,ssVerb);
	m_VerbCombo.ShowWindow(ssVerb);
		
	ShowDlgItem(IDC_STATICWINDOW,ssMessage);
	ShowDlgItem(IDC_WINDOW,ssMessage);
	ShowDlgItem(IDC_STATICMESSAGE,ssMessage);
	ShowDlgItem(IDC_MESSAGE,ssMessage);
	ShowDlgItem(IDC_STATICWPARAM,ssMessage);
	ShowDlgItem(IDC_WPARAM,ssMessage);
	ShowDlgItem(IDC_STATICLPARAM,ssMessage);
	ShowDlgItem(IDC_LPARAM,ssMessage);

	ShowDlgItem(IDC_STATICCOMMAND,ssCommand);
	ShowDlgItem(IDC_COMMAND,ssCommand);

	ShowDlgItem(IDC_STATICVALUE,ssChangeValue);
	ShowDlgItem(IDC_VALUE,ssChangeValue);
	ShowDlgItem(IDC_VALUEHELPTEXT,ssChangeValue);

	ShowDlgItem(IDC_STATICWHICHFILE,ssWhichFile);
	m_WhichFileCombo.ShowWindow(ssWhichFile);

}

void CSettingsProperties::CKeyboardShortcutsPage::OnChangeItem(NMLISTVIEW *pNm)
{
	if (pNm->iItem==-1)
		return;

	if (!(pNm->uChanged&LVIF_STATE))
		return;
	
    if (!(pNm->uOldState&LVIS_SELECTED) &&
		pNm->uNewState&LVIS_SELECTED)
	{
        // Item is selected
		CShortcut* pShortcut=(CShortcut*)m_pList->GetItemData(pNm->iItem);
		if (pShortcut==NULL)
			return;

		SetFieldsForShortcut(m_pCurrentShortcut=pShortcut);
	}

	EnableItems();
}

void CSettingsProperties::CKeyboardShortcutsPage::OnChangingItem(NMLISTVIEW *pNm)
{
	if (pNm->iItem==-1)
		return;

	if (!(pNm->uChanged&LVIF_STATE))
		return;

	
    if (pNm->uOldState&LVIS_SELECTED &&
		!(pNm->uNewState&LVIS_SELECTED))
	{
		// Item is deselected
		CShortcut* pShortcut=(CShortcut*)m_pList->GetItemData(pNm->iItem);
		if (pShortcut==NULL)
			return;
		SaveFieldsForShortcut(pShortcut);
		ClearActionFields();

		m_pList->RedrawItems(pNm->iItem,pNm->iItem);
	}

}

void CSettingsProperties::CKeyboardShortcutsPage::SetFieldsForShortcut(CShortcut* pShortcut)
{
	if (pShortcut->m_dwFlags&CShortcut::sfVirtualKeySpecified)
	{
		CheckDlgButton(IDC_HOTKEYRADIO,FALSE);
		CheckDlgButton(IDC_CODERADIO,TRUE);
	}
	else
	{
		CheckDlgButton(IDC_CODERADIO,FALSE);
		CheckDlgButton(IDC_HOTKEYRADIO,TRUE);
	}

		
	InsertKeysToVirtualKeyCombo();
	SetVirtualCode(pShortcut->m_bVirtualKey,(pShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode)?TRUE:FALSE);
	if (!(pShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode))
		SetHotKeyForShortcut(pShortcut);

	CheckDlgButton(IDC_MODCTRL,pShortcut->m_bModifiers&CShortcut::ModifierControl?1:0);
	CheckDlgButton(IDC_MODALT,pShortcut->m_bModifiers&CShortcut::ModifierAlt?1:0);
	CheckDlgButton(IDC_MODSHIFT,pShortcut->m_bModifiers&CShortcut::ModifierShift?1:0);
	CheckDlgButton(IDC_MODWIN,pShortcut->m_bModifiers&CShortcut::ModifierWin?1:0);

	CheckDlgButton(IDC_FROMMNEMONIC,(pShortcut->m_dwFlags&CShortcut::sfUseMemonic)?TRUE:FALSE);
	SetFieldsRelativeToMnemonics();
	

	m_nCurrentAction=0;
	ASSERT(pShortcut->m_apActions.GetSize()>0);
	SetFieldsForAction(pShortcut->m_apActions[0]);

	ASSERT(m_pWhenPressedList->GetItemCount()>0);

	int nItem=m_pWhenPressedList->GetNextItem(-1,LVNI_ALL);
	while (nItem!=-1)
	{
		if ((pShortcut->m_dwFlags&CShortcut::sfKeyTypeMask)==CShortcut::sfLocal)
		{
			CShortcut::WhenPresssed wpMask=(CShortcut::WhenPresssed)m_pWhenPressedList->GetItemData(nItem);
			m_pWhenPressedList->SetCheckState(nItem,(pShortcut->m_wWhenPressed&wpMask)?TRUE:FALSE);
		}
		else
			m_pWhenPressedList->SetCheckState(nItem,TRUE);
			nItem=m_pWhenPressedList->GetNextItem(nItem,LVNI_ALL);
	}

}

void CSettingsProperties::CKeyboardShortcutsPage::SaveFieldsForShortcut(CShortcut* pShortcut)
{
	if (IsDlgButtonChecked(IDC_HOTKEYRADIO))
	{
		GetHotKeyForShortcut(pShortcut);
		pShortcut->m_dwFlags&=~(CShortcut::sfVirtualKeySpecified|CShortcut::sfVirtualKeyIsScancode);
	}
	else
	{
		pShortcut->m_dwFlags|=CShortcut::sfVirtualKeySpecified;

		// Set modifiers
		pShortcut->m_bModifiers=0;
		if (IsDlgButtonChecked(IDC_MODCTRL))
			pShortcut->m_bModifiers|=CShortcut::ModifierControl;
		if (IsDlgButtonChecked(IDC_MODALT))
			pShortcut->m_bModifiers|=CShortcut::ModifierAlt;
		if (IsDlgButtonChecked(IDC_MODSHIFT))
			pShortcut->m_bModifiers|=CShortcut::ModifierShift;
		if (IsDlgButtonChecked(IDC_MODWIN))
			pShortcut->m_bModifiers|=CShortcut::ModifierWin;

		
		pShortcut->m_bVirtualKey=GetVirtualCode((pShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode)?TRUE:FALSE);
	}


	ASSERT(m_nCurrentAction>=0 && m_nCurrentAction<pShortcut->m_apActions.GetSize());
	SaveFieldsForAction(pShortcut->m_apActions[m_nCurrentAction]);


	if ((pShortcut->m_dwFlags&CShortcut::sfKeyTypeMask)==CShortcut::sfLocal)
	{
		ASSERT(m_pWhenPressedList->GetItemCount()>0);
		
		int nItem=m_pWhenPressedList->GetNextItem(-1,LVNI_ALL);
		while (nItem!=-1)
		{	
			CShortcut::WhenPresssed wpMask=(CShortcut::WhenPresssed)m_pWhenPressedList->GetItemData(nItem);
			
			if (m_pWhenPressedList->GetCheckState(nItem))
				pShortcut->m_wWhenPressed|=wpMask;
			else
				pShortcut->m_wWhenPressed&=~wpMask;

			
			nItem=m_pWhenPressedList->GetNextItem(nItem,LVNI_ALL);
		}
	}

}

void CSettingsProperties::CKeyboardShortcutsPage::SetFieldsForAction(CAction* pAction)
{
	ASSERT(m_pCurrentShortcut!=NULL);
	ASSERT(m_nCurrentAction>=0 && m_nCurrentAction<m_pCurrentShortcut->m_apActions.GetSize());
	
	// Change groupbox title
	if (m_pCurrentShortcut->m_apActions.GetSize()>1)
	{
		CString str;
		str.Format(IDS_SHORTCUTACTION2,m_nCurrentAction+1,m_pCurrentShortcut->m_apActions.GetSize());
		SetDlgItemText(IDC_STATICACTIONS,str);
	}
	else
		SetDlgItemText(IDC_STATICACTIONS,ID2A(IDS_SHORTCUTACTION));

	// Setting action, InsertSubActions does selection of subaction
	m_ActionCombo.SetCurSel(pAction->m_nAction+1);
	InsertSubActions();

	// Clear sub action fields fisrt
	m_VerbCombo.SetCurSel(0);
	SetDlgItemText(IDC_WINDOW,szEmpty);
	SetDlgItemInt(IDC_MESSAGE,0,FALSE);
	SetDlgItemText(IDC_WPARAM,szEmpty);
	SetDlgItemText(IDC_LPARAM,szEmpty);
	SetDlgItemText(IDC_COMMAND,szEmpty);
	SetDlgItemText(IDC_VALUE,szEmpty);
	m_WhichFileCombo.SetCurSel(0);


	switch (pAction->m_nAction)
	{
	case CAction::ResultListItems:
		if (pAction->m_nResultList==CAction::Execute)
		{
			if (pAction->m_szVerb==NULL)
				m_VerbCombo.SetCurSel(0);
			else
			{
				m_VerbCombo.SetCurSel(-1);
				m_VerbCombo.SetText(pAction->m_szVerb);
			}
		}
		else if (pAction->m_nResultList==CAction::ExecuteCommand && pAction->m_szCommand!=NULL)
			SetDlgItemText(IDC_COMMAND,pAction->m_szCommand);
		else if (pAction->m_nResultList==CAction::SelectFile)
			m_WhichFileCombo.SetCurSel(pAction->m_nSelectFileType);	
		break;
	case CAction::Misc:
		if ((pAction->m_nMisc==CAction::SendMessage || pAction->m_nMisc==CAction::PostMessage) &&
			pAction->m_pSendMessage!=NULL)
		{
			SetDlgItemText(IDC_WINDOW,pAction->m_pSendMessage->szWindow!=NULL?
				pAction->m_pSendMessage->szWindow:szwEmpty);
			SetDlgItemInt(IDC_MESSAGE,pAction->m_pSendMessage->nMessage,FALSE);
			SetDlgItemText(IDC_WPARAM,pAction->m_pSendMessage->szWParam!=NULL?
				pAction->m_pSendMessage->szWParam:szwEmpty);
			SetDlgItemText(IDC_LPARAM,pAction->m_pSendMessage->szLParam!=NULL?
				pAction->m_pSendMessage->szLParam:szwEmpty);
		}
		else if (pAction->m_nMisc==CAction::ExecuteCommandMisc && pAction->m_szCommand!=NULL)
			SetDlgItemText(IDC_COMMAND,pAction->m_szCommand);
		
		break;
	case CAction::Presets:
		if (pAction->m_szPreset!=NULL)
		{
			for (int i=0;i<m_aPossiblePresets.GetSize();i++)
			{
				if (wcscmp(m_aPossiblePresets[i],pAction->m_szPreset)==0)
					m_SubActionCombo.SetCurSel(i);
			}
		}
		break;
	case CAction::ChangeValue:
		if (pAction->m_szValue!=NULL)
			SetDlgItemText(IDC_VALUE,pAction->m_szValue);
		break;
	}
}

void CSettingsProperties::CKeyboardShortcutsPage::SaveFieldsForAction(CAction* pAction)
{
	// Clearing previous extra information
	pAction->ClearExtraInfo();
	
	// Setting action
	pAction->m_nAction=GetSelectedAction();;
	
	switch (pAction->m_nAction)
	{
	case CAction::None:
		pAction->m_nSubAction=0;
		break;
	case CAction::ActivateControl:
		{
			int nCurSel=m_SubActionCombo.GetCurSel();
			if (nCurSel!=CB_ERR)
				pAction->m_nControl=m_pPossibleControlsToActivate[nCurSel];
			else
				pAction->m_nControl=m_pPossibleControlsToActivate[0];
			break;
		}
	case CAction::MenuCommand:
		{
			int nCurSel=m_SubActionCombo.GetCurSel();
			if (nCurSel!=CB_ERR)
				pAction->m_nMenuCommand=m_pPossibleMenuCommands[nCurSel];
			else
				pAction->m_nMenuCommand=m_pPossibleMenuCommands[0];
			break;
		}
	case CAction::ActivateTab:
	case CAction::ShowHideDialog:
		pAction->m_nSubAction=m_SubActionCombo.GetCurSel();
		if ((int)pAction->m_nSubAction==CB_ERR)
			pAction->m_nSubAction=0;
		break;
	case CAction::ResultListItems:
		pAction->m_nResultList=(CAction::ActionResultList)m_SubActionCombo.GetCurSel();
		if ((int)pAction->m_nResultList==CB_ERR)
			pAction->m_nResultList=CAction::Execute;
		else if (pAction->m_nResultList==CAction::Execute)
		{
			int nSelection=m_VerbCombo.GetCurSel();
			if (nSelection==CB_ERR)
			{
				UINT nLen=(UINT)m_VerbCombo.GetTextLength();
				pAction->m_szVerb=new WCHAR[nLen+1];
				m_VerbCombo.GetText(pAction->m_szVerb,nLen+1);
			}
			else if (nSelection!=0)
			{
				UINT nLen=(UINT)m_VerbCombo.GetLBTextLen(nSelection);
				pAction->m_szVerb=new WCHAR[nLen+1];
				m_VerbCombo.GetLBText(nSelection,pAction->m_szVerb);
			}
		}
		else if (pAction->m_nResultList==CAction::ExecuteCommand)
		{
			// Get command
			UINT nLen=GetDlgItemTextLength(IDC_COMMAND);
			if (nLen>0)
			{
				pAction->m_szCommand=new WCHAR[nLen+1];
				GetDlgItemText(IDC_COMMAND,pAction->m_szCommand,nLen+1);
			}
		}
		else if (pAction->m_nResultList==CAction::SelectFile)
		{
			pAction->m_nSelectFileType=(CSubAction::SelectFileType)m_WhichFileCombo.GetCurSel();
			if (int(pAction->m_nSelectFileType)==CB_ERR)
				pAction->m_nSelectFileType=CSubAction::NextFile;
		}
		break;
	case CAction::Misc:
		pAction->m_nMisc=(CAction::ActionMisc)m_SubActionCombo.GetCurSel();
		if ((int)pAction->m_nMisc==CB_ERR)
			pAction->m_nMisc=CAction::SendMessage;

		pAction->m_pSendMessage=new CAction::SendMessageInfo;
		
		if (pAction->m_nMisc==CAction::SendMessage || 
			pAction->m_nMisc==CAction::PostMessage)
		{
			// Get message
			BOOL bTranslated;
			pAction->m_pSendMessage->nMessage=GetDlgItemInt(IDC_MESSAGE,&bTranslated,FALSE);
			if (!bTranslated)
				pAction->m_pSendMessage->nMessage=0;
			
			// Get window			
			UINT nLen=GetDlgItemTextLength(IDC_WINDOW);
			if (nLen>0)
			{
				pAction->m_pSendMessage->szWindow=new WCHAR[nLen+1];
				GetDlgItemText(IDC_WINDOW,pAction->m_pSendMessage->szWindow,nLen+1);
			}
		
			// Get WParam
			nLen=GetDlgItemTextLength(IDC_WPARAM);
			if (nLen>0)
			{
				pAction->m_pSendMessage->szWParam=new WCHAR[nLen+1];
				GetDlgItemText(IDC_WPARAM,pAction->m_pSendMessage->szWParam,nLen+1);
			}
			
			// Get LParam
			nLen=GetDlgItemTextLength(IDC_LPARAM);
			if (nLen>0)
			{
				pAction->m_pSendMessage->szLParam=new WCHAR[nLen+1];
				GetDlgItemText(IDC_LPARAM,pAction->m_pSendMessage->szLParam,nLen+1);
			}
		}
		else if (pAction->m_nMisc==CAction::ExecuteCommandMisc)
		{
			// Get command
			UINT nLen=GetDlgItemTextLength(IDC_COMMAND);
			if (nLen>0)
			{
				pAction->m_szCommand=new WCHAR[nLen+1];
				GetDlgItemText(IDC_COMMAND,pAction->m_szCommand,nLen+1);
			}
		}
		break;
	case CAction::Presets:
		{
			pAction->m_nSubAction=m_SubActionCombo.GetCurSel();
			if((int)pAction->m_nSubAction>=0 && (int)pAction->m_nSubAction<m_aPossiblePresets.GetSize())
				pAction->m_szPreset=alloccopy(m_aPossiblePresets[(int)pAction->m_nSubAction]);
			break;
		}
	case CAction::ChangeValue:
		{
			int nCurSel=m_SubActionCombo.GetCurSel();
			if (nCurSel!=CB_ERR)
				pAction->m_nControl=m_pPossibleControlsToChange[nCurSel];
			else
				pAction->m_nControl=m_pPossibleControlsToChange[0];
			
			// Get command
			UINT nLen=GetDlgItemTextLength(IDC_VALUE);
			if (nLen>0)
			{
				pAction->m_szCommand=new WCHAR[nLen+1];
				GetDlgItemText(IDC_VALUE,pAction->m_szValue,nLen+1);
			}
			break;

		}
	default:
		ASSERT(0);
		break;
	
	};

}

void CSettingsProperties::CKeyboardShortcutsPage::ClearActionFields()
{
	// Change groupbox title
	SetDlgItemText(IDC_STATICACTIONS,ID2A(IDS_SHORTCUTACTION));
}


void CSettingsProperties::CKeyboardShortcutsPage::FormatActionLabel(CStringW& str,CAction::Action nAction,UINT uSubAction) const
{
	// Insert action code
	switch (nAction)
	{
	case CAction::None:
		str.LoadString(IDS_NONE);
		return;
	case CAction::ActivateControl:
		str.LoadString(IDS_ACTIONCATACTIVATECONTROL);
		break;
	case CAction::ActivateTab:
		str.LoadString(IDS_ACTIONCATACTIVATETAB);
		break;
	case CAction::MenuCommand:
		str.LoadString(IDS_ACTIONCATMENUCOMMAND);
		break;
	case CAction::ShowHideDialog:
		break;
	case CAction::ResultListItems:
		break;
	case CAction::Misc:
		break;
	case CAction::Presets:
		str.LoadString(IDS_ACTIONCATPRESETS);
		break;
	case CAction::ChangeValue:
		str.LoadString(IDS_ACTIONCATCHANGEVALUE);
		break;
	}

	// Insert subaction code
	CStringW subaction;
	if (GetSubActionLabel(subaction,nAction,uSubAction))
	{
		if (str.IsEmpty())
			str=subaction;
		else
			str << L'/' << subaction;
	}
}

///////////////////////////////////////////////////////////////
// CSettingsProperties::CKeyboardShortcutsPage::CAdvancedDlg

BOOL CSettingsProperties::CKeyboardShortcutsPage::CAdvancedDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);

	// Adding items to combo
	SendDlgItemMessage(IDC_TYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_ADVSHORTCUTLOCAL));
	SendDlgItemMessage(IDC_TYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_ADVSHORTCUTGLOBALHOTKEY));

	OSVERSIONINFOEX oi;
	oi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
	BOOL bRet=GetVersionEx((LPOSVERSIONINFO) &oi);
	if (!(bRet && oi.dwPlatformId!=VER_PLATFORM_WIN32_NT ||
		!(oi.dwMajorVersion>=5 || (oi.dwMajorVersion==4 && oi.wServicePackMajor>=3) )))
		SendDlgItemMessage(IDC_TYPE,CB_ADDSTRING,0,(LPARAM)(LPCSTR)ID2A(IDS_ADVSHORTCUTGLOBALHOOK));

	// Set where
	switch (m_pShortcut->m_dwFlags&CShortcut::sfKeyTypeMask)
	{
	case CShortcut::sfGlobalHotkey:
		SendDlgItemMessage(IDC_TYPE,CB_SETCURSEL,1);
		break;
	case CShortcut::sfGlobalHook:
		SendDlgItemMessage(IDC_TYPE,CB_SETCURSEL,2);
		break;
	default:
		SendDlgItemMessage(IDC_TYPE,CB_SETCURSEL,0);
		break;
	}

	// Set virtual key is scancode
	CheckDlgButton(IDC_VKISSCANCODE,(m_pShortcut->m_dwFlags&CShortcut::sfVirtualKeyIsScancode)?TRUE:FALSE);

	// Where pressed field (works only with global keys)
	if ((m_pShortcut->m_dwFlags&CShortcut::sfKeyTypeMask)!=CShortcut::sfLocal)
	{
		if (m_pShortcut->m_pClass==LPSTR(-1))
			CheckDlgButton(IDC_LOCATEDIALOG,1);
		else
		{
			if (m_pShortcut->m_pTitle!=NULL)
				SetDlgItemText(IDC_WINDOWTITLE,m_pShortcut->m_pTitle);
			if (m_pShortcut->m_pClass!=NULL)
				SetDlgItemText(IDC_WINDOWCLASS,m_pShortcut->m_pClass);
		}
	}		

	
	// Global hook specifig stuff
	if ((m_pShortcut->m_dwFlags&CShortcut::sfKeyTypeMask)==CShortcut::sfGlobalHook)
	{
		if ((m_pShortcut->m_dwFlags&CShortcut::sfExecuteMask)!=CShortcut::sfExecuteWhenDown)
			CheckDlgButton(IDC_EXECUTEWHENKEYISUP,1);
		else
			CheckDlgButton(IDC_EXECUTEWHENKEYISDOWN,1);

		if (m_pShortcut->m_dwFlags&CShortcut::sfRemoveKeyDownMessage)
			CheckDlgButton(IDC_REMOVEDOWNMESSAGE,TRUE);
		if (m_pShortcut->m_dwFlags&CShortcut::sfRemoveKeyUpMessage)
			CheckDlgButton(IDC_REMOVEUPMESSAGE,TRUE);
	}
	else
	{
		CheckDlgButton(IDC_EXECUTEWHENKEYISDOWN,1);
	}
	

	if (m_pShortcut->m_nDelay==0)
		CheckDlgButton(IDC_WAITNONE,1);
	else if (m_pShortcut->m_nDelay==UINT(-1))
		CheckDlgButton(IDC_WAITPOST,1);
	else
	{
		CheckDlgButton(IDC_WAITDELAY,1);
		SetDlgItemInt(IDC_WAITMS,m_pShortcut->m_nDelay,FALSE);
	}



	EnableItems();
	return FALSE;
}


void CSettingsProperties::CKeyboardShortcutsPage::CAdvancedDlg::EnableItems()
{
	if (SendDlgItemMessage(IDC_TYPE,CB_GETCURSEL)!=0)
	{
		//Type is not local
		EnableDlgItem(IDC_LOCATEDIALOG,TRUE);
		
		BOOL bEnableWindowAndClass=!IsDlgButtonChecked(IDC_LOCATEDIALOG);

		EnableDlgItem(IDC_WINDOWTITLE,bEnableWindowAndClass);
		EnableDlgItem(IDC_WINDOWCLASS,bEnableWindowAndClass);
		EnableDlgItem(IDC_STATICWINDOWTITLE,bEnableWindowAndClass);
		EnableDlgItem(IDC_STATICCLASS,bEnableWindowAndClass);

		EnableDlgItem(IDC_EXECUTEWHENSTATIC,TRUE);
		EnableDlgItem(IDC_EXECUTEWHENKEYISDOWN,TRUE);
		EnableDlgItem(IDC_EXECUTEWHENKEYISUP,TRUE);
		EnableDlgItem(IDC_REMOVEMESSAGESTATIC,TRUE);
		EnableDlgItem(IDC_REMOVEDOWNMESSAGE,TRUE);
		EnableDlgItem(IDC_REMOVEUPMESSAGE,TRUE);
	}
	else
	{
		EnableDlgItem(IDC_LOCATEDIALOG,FALSE);
		EnableDlgItem(IDC_WINDOWTITLE,FALSE);
		EnableDlgItem(IDC_WINDOWCLASS,FALSE);
		EnableDlgItem(IDC_STATICWINDOWTITLE,FALSE);
		EnableDlgItem(IDC_STATICCLASS,FALSE);

		EnableDlgItem(IDC_EXECUTEWHENSTATIC,FALSE);
		EnableDlgItem(IDC_EXECUTEWHENKEYISDOWN,FALSE);
		EnableDlgItem(IDC_EXECUTEWHENKEYISUP,FALSE);
		EnableDlgItem(IDC_REMOVEMESSAGESTATIC,FALSE);
		EnableDlgItem(IDC_REMOVEDOWNMESSAGE,FALSE);
		EnableDlgItem(IDC_REMOVEUPMESSAGE,FALSE);
	}

	EnableDlgItem(IDC_WAITMS,IsDlgButtonChecked(IDC_WAITDELAY));
	EnableDlgItem(IDC_VKISSCANCODE,(m_pShortcut->m_dwFlags&CShortcut::sfVirtualKeySpecified)?TRUE:FALSE);

}

BOOL CSettingsProperties::CKeyboardShortcutsPage::CAdvancedDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);

	switch (wID)
	{
	case IDC_OK:
		OnOK();
		break;
	case IDCANCEL:
	case IDC_CANCEL:
		EndDialog(0);
		break;
	case IDC_LOCATEDIALOG:
	case IDC_WAITNONE:
	case IDC_WAITPOST:
	case IDC_TYPE:
		EnableItems();
		break;
	case IDC_WAITDELAY:
		EnableItems();
		SetFocus(IDC_WAITMS);
		break;
	}	
	return FALSE;
}

BOOL CSettingsProperties::CKeyboardShortcutsPage::CAdvancedDlg::OnClose()
{
	CDialog::OnClose();
	EndDialog(0);
	return FALSE;
}


void CSettingsProperties::CKeyboardShortcutsPage::CAdvancedDlg::OnOK()
{
	// Clear old values
	m_pShortcut->ClearExtraInfo();
	
	// Type
	m_pShortcut->m_dwFlags&=~CShortcut::sfKeyTypeMask;
	switch (SendDlgItemMessage(IDC_TYPE,CB_GETCURSEL))
	{
	case 1:
		m_pShortcut->m_dwFlags|=CShortcut::sfGlobalHotkey;
		break;
	case 2:
		m_pShortcut->m_dwFlags|=CShortcut::sfGlobalHook;
		break;
	default:
		m_pShortcut->m_dwFlags|=CShortcut::sfLocal;
		break;
	}


	if ((m_pShortcut->m_dwFlags&CShortcut::sfKeyTypeMask)!=CShortcut::sfLocal)
	{
		if (IsDlgButtonChecked(IDC_LOCATEDIALOG))
		{
			m_pShortcut->m_pClass=LPSTR(-1);
			m_pShortcut->m_pTitle=NULL;
		}
		else
		{
			CString Text;
			if (GetDlgItemText(IDC_WINDOWTITLE,Text)>0)
				m_pShortcut->m_pTitle=alloccopy(Text,Text.GetLength());
			else
				m_pShortcut->m_pTitle=NULL;

			if (GetDlgItemText(IDC_WINDOWCLASS,Text)>0)
				m_pShortcut->m_pClass=alloccopy(Text,Text.GetLength());
			else
				m_pShortcut->m_pClass=NULL;

		}
	}		
	
	// Global hook specifig stuff
	if ((m_pShortcut->m_dwFlags&CShortcut::sfKeyTypeMask)==CShortcut::sfGlobalHook)
	{
		m_pShortcut->m_dwFlags&=~CShortcut::sfExecuteMask;
		if (IsDlgButtonChecked(IDC_EXECUTEWHENKEYISUP))
			m_pShortcut->m_dwFlags|=CShortcut::sfExecuteWhenUp;


		if (IsDlgButtonChecked(IDC_REMOVEDOWNMESSAGE))
			m_pShortcut->m_dwFlags|=CShortcut::sfRemoveKeyDownMessage;
		else
			m_pShortcut->m_dwFlags&=~CShortcut::sfRemoveKeyDownMessage;
			
		if (IsDlgButtonChecked(IDC_REMOVEUPMESSAGE))
			m_pShortcut->m_dwFlags|=CShortcut::sfRemoveKeyUpMessage;
		else
			m_pShortcut->m_dwFlags&=~CShortcut::sfRemoveKeyUpMessage;
	}
	

	// Setting wait time
	if (IsDlgButtonChecked(IDC_WAITNONE))
		m_pShortcut->m_nDelay=0;
	else if (IsDlgButtonChecked(IDC_WAITPOST))
		m_pShortcut->m_nDelay=UINT(-1);
	else
	{
		BOOL bTranslated;
		m_pShortcut->m_nDelay=GetDlgItemInt(IDC_WAITMS,&bTranslated,FALSE);
		if (!bTranslated)
			m_pShortcut->m_nDelay=0;
	}

	// VK is scancode
	if (IsDlgButtonChecked(IDC_VKISSCANCODE))
		m_pShortcut->m_dwFlags|=CShortcut::sfVirtualKeyIsScancode;
	else
		m_pShortcut->m_dwFlags&=~CShortcut::sfVirtualKeyIsScancode;

	EndDialog(1);
}