#if !defined(SETTINGSDLG_INL)
#define SETTINGSDLG_INL

#if _MSC_VER >= 1000
#pragma once
#endif 

inline COptionsPropertyPage::COptionsPropertyPage()
:	m_pTree(NULL),m_pItems(NULL)
{
}

inline COptionsPropertyPage::COptionsPropertyPage(const OPTIONPAGE* pOptionPage)
:	m_pTree(NULL),m_pItems(NULL)
{
	Construct(pOptionPage);
}

inline COptionsPropertyPage::Item::Item(
	ItemType nType_,Item* pParent_,Item** pChilds_,LPWSTR pString_,
	CALLBACKPROC pProc_,DWORD wParam_,void* lParam_)
:	nType(nType_),pParent(pParent_),bChecked(0),bEnabled(TRUE),
	wParam(wParam_),lParam(lParam_),pProc(pProc_),
	m_nStateIcon(-1),hControl(NULL),hControl2(NULL)
{
	if (pString_!=NULL)
		pString=alloccopy(pString_);
	else 
		pString=NULL;
	
	if (pChilds_!=NULL)
	{
		for (int i=0;pChilds_[i]!=NULL;i++);
		if (i>0)
		{
			pChilds=new Item*[i+1];
			CopyMemory(pChilds,pChilds_,sizeof(Item*)*(i+1));
			return;
		}
	}
	pChilds=NULL;
}

inline COptionsPropertyPage::Item::Item(
	ItemType nType_,Item* pParent_,Item** pChilds_,UINT nStringID,
	CALLBACKPROC pProc_,DWORD wParam_,void* lParam_)
:	nType(nType_),pParent(pParent_),bChecked(0),bEnabled(TRUE),
	wParam(wParam_),lParam(lParam_),pProc(pProc_),
	m_nStateIcon(-1),hControl(NULL),hControl2(NULL)
{
	int nCurLen=50;
	int iLength;
	
	if (!IsFullUnicodeSupport())
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
		for (int i=0;pChilds_[i]!=NULL;i++);
		if (i>0)
		{
			pChilds=new Item*[i+1];
			CopyMemory(pChilds,pChilds_,sizeof(Item*)*(i+1));
			return;
		}
	}
	pChilds=NULL;
}


inline COptionsPropertyPage::Item::~Item()
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
		if (pData!=NULL)
			delete[] pData;
		break;
	case Font:
		if (pLogFont!=NULL)
			delete pLogFont;
		break;
	}
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateRoot(LPWSTR szText,Item** pChilds)
{
	return new Item(Item::Root,NULL,pChilds,szText,NULL,0,0);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateRoot(UINT nTextID,Item** pChilds)
{
	return new Item(Item::Root,NULL,pChilds,nTextID,NULL,0,0);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateCheckBox(
	LPWSTR szText,Item** pChilds,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::CheckBox,NULL,pChilds,szText,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateCheckBox(
	UINT nTextID,Item** pChilds,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::CheckBox,NULL,pChilds,nTextID,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateRadioBox(
	LPWSTR szText,Item** pChilds,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::RadioBox,NULL,pChilds,szText,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateRadioBox(
	UINT nTextID,Item** pChilds,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::RadioBox,NULL,pChilds,nTextID,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateComboBox(
	LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::Combo,NULL,NULL,szText,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateComboBox(
	UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::Combo,NULL,NULL,nTextID,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateEdit(
	LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::Edit,NULL,NULL,szText,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateEdit(
	UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::Edit,NULL,NULL,nTextID,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateListBox(
	LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::List,NULL,NULL,szText,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateListBox(
	UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::List,NULL,NULL,nTextID,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateNumeric(
	LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::Numeric,NULL,NULL,szText,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateNumeric(
	UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::Numeric,NULL,NULL,nTextID,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateColor(
	LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::Color,NULL,NULL,szText,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateColor(
	UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::Color,NULL,NULL,nTextID,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateFont(
	LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::Font,NULL,NULL,szText,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateFont(
	UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::Font,NULL,NULL,nTextID,pProc,wParam,lParam);
}

inline int COptionsPropertyPage::Item::GetStateImage(CImageList* pImageList) const
{
	switch (nType)
	{
	case CheckBox:
		return bChecked?2:1;
	case RadioBox:
		return bChecked?4:3;
	case Root:
		return 5;
	case Color:
		if (m_nStateIcon==-1)
			m_nStateIcon=IconFromColor(pImageList);
		return m_nStateIcon;
	default:
		return 0;
	}
}

inline void COptionsPropertyPage::Item::GetValuesFromBasicParams(const COptionsPropertyPage::BASICPARAMS* pParams)
{
	switch (nType)
	{
	case RadioBox:
	case CheckBox:
		bChecked=pParams->bChecked;
		break;
	case Numeric:
	case List:
    	lValue=pParams->lValue;
		break;
	case Edit:
	case Combo:
		if (pData!=NULL)
			delete[] pData;
		pData=pParams->pData;
		break;
	case Color:
		cColor=pParams->cColor;
        break;
	case Font:
		if (pLogFont==NULL)
			pLogFont=pParams->pLogFont;
		ASSERT (pParams->pLogFont==pLogFont);
		break;
	case Root:
	default:
		break;
	}
}

inline void COptionsPropertyPage::Item::SetValuesForBasicParams(COptionsPropertyPage::BASICPARAMS* pParams)
{
	pParams->lParam=lParam;
	pParams->wParam=wParam;
	
	switch (nType)
	{
	case RadioBox:
	case CheckBox:
		pParams->bChecked=bChecked;
		break;
	case Numeric:
	case List:
    	pParams->lValue=lValue;
		break;
	case Edit:
	case Combo:
		pParams->pData=pData;
		break;
	case Color:
		pParams->cColor=cColor;
		break;
	case Font:
		pParams->pLogFont=pLogFont;
		break;
	case Root:
	default:
		pParams->pData=NULL;
		break;
	}
}

inline void COptionsPropertyPage::Item::FreeText(LPWSTR pText) const
{
    if (pText!=pString)
		delete[] pText;
}











inline BOOL CSettingsProperties::IsFlagSet(DWORD dwFlags)
{
	return (m_dwSettingsFlags&dwFlags)?TRUE:FALSE;
}

inline BOOL CSettingsProperties::IsAllFlagsSet(DWORD dwFlags)
{
	return (m_dwSettingsFlags&dwFlags)==dwFlags?TRUE:FALSE;
}

inline void CSettingsProperties::SetFlags(DWORD dwFlags,BOOL bState)
{
	if (bState)
		m_dwSettingsFlags|=dwFlags;
	else
		m_dwSettingsFlags&=~dwFlags;
}

inline void CSettingsProperties::SetFlags(DWORD dwFlags)
{
	m_dwSettingsFlags|=dwFlags;
}

inline void CSettingsProperties::ClearFlags(DWORD dwFlags)
{
	m_dwSettingsFlags&=~dwFlags;
}

inline CSettingsProperties::CGeneralSettingsPage::CGeneralSettingsPage()
:	CPropertyPage(IDD_GENERALSETTINGS,IDS_GENERALSETTINGS)
{
}

inline CSettingsProperties::CAdvancedSettingsPage::CAdvancedSettingsPage()
{	
	OPTIONPAGE op;
	op.dwFlags=COptionsPropertyPage::OPTIONPAGE::opCaptionIsID|
		COptionsPropertyPage::OPTIONPAGE::opTemplateIsID|
		COptionsPropertyPage::OPTIONPAGE::opChangeIsID;
		
	op.nIDTemplate=IDD_ADVANCEDSETTINGS;
	op.nIDCaption=IDS_ADVANCEDSETTINGS;
	op.nTreeCtrlID=IDC_SETTINGS;
	op.nIDChangeText=IDS_ADVSETCHANGE;


	Construct(&op);
}

inline CSettingsProperties::CLanguageSettingsPage::CLanguageSettingsPage()
:	CPropertyPage(IDD_LANGUAGESETTINGS,IDS_LANGUAGESETTINGS),m_pList(NULL),nLastSel(0)
{
}

inline CSettingsProperties::CDatabasesSettingsPage::CDatabasesSettingsPage()
:	CPropertyPage(IDD_DATABASESETTINGS,IDS_DATABASESETTINGS),m_pList(NULL)
{
}

inline CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CDatabaseDialog()
:	CDialog(IDD_DATABASEDIALOG),m_pList(NULL),m_bDontEditName(FALSE)
{
}

inline CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::CExcludeDirectoryDialog()
:	CDialog(IDD_EXCLUDEDIRECTORIES)
{
}

inline CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::CExcludeDirectoryDialog(const CArrayFAP<LPSTR>& rDirectories)
:	CDialog(IDD_EXCLUDEDIRECTORIES),m_bTextChanged(TRUE)
{
	for (int i=0;i<rDirectories.GetSize();i++)
		m_aDirectories.Add(alloccopy(rDirectories[i]));
}

inline CSettingsProperties::CAutoUpdateSettingsPage::CAutoUpdateSettingsPage()
:	CPropertyPage(IDD_AUTOUPDATESETTINGS,IDS_AUTOUPDATESETTINGS)
{
}

inline CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::CCheduledUpdateDlg()
:	CDialog(IDD_SCHEDULEDUPDATE),m_pCombo(NULL),m_pSchedule(NULL),m_bChanged(FALSE)
{
}

inline CSettingsProperties::CKeyboardShortcutsPage::CKeyboardShortcutsPage()
:	CPropertyPage(IDD_KEYBOARDSHORTCUTS,IDS_LANGUAGESETTINGS),
	m_pList(NULL),m_pToolBar(NULL),m_ToolBarBitmaps(NULL),
	m_ToolBarBitmapsDisabled(NULL),m_ToolBarBitmapsHot(NULL)
{
}


#endif