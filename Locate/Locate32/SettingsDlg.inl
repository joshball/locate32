#if !defined(SETTINGSDLG_INL)
#define SETTINGSDLG_INL

#if _MSC_VER >= 1000
#pragma once
#endif 


inline COptionsPropertyPage::COptionsPropertyPage()
:	m_pTree(NULL),m_pItems(NULL),CPropertyPage()
{
}

inline COptionsPropertyPage::COptionsPropertyPage(const COptionsPropertyPage::OPTIONPAGE* pOptionPage,TypeOfResourceHandle bType)
:	m_pTree(NULL),m_pItems(NULL),CPropertyPage()
{
	Construct(pOptionPage,bType);
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

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateFile(
	LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::File,NULL,NULL,szText,pProc,wParam,lParam);
}

inline COptionsPropertyPage::Item* COptionsPropertyPage::CreateFile(
	UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam)
{
	return new Item(Item::File,NULL,NULL,nTextID,pProc,wParam,lParam);
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
	case File:
		if (pData!=NULL && pData!=pParams->pData)
			delete[] pData;
		pData=pParams->pData;
		//DebugFormatMessage("GetValuesFromBasicParams, pData=%X",pData);
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
	case File:
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








inline BOOL CSettingsProperties::IsSettingsFlagSet(DWORD dwFlags)
{
	return (m_dwSettingsFlags&dwFlags)?TRUE:FALSE;
}

inline BOOL CSettingsProperties::IsAllFlagsSet(DWORD dwFlags)
{
	return (m_dwSettingsFlags&dwFlags)==dwFlags?TRUE:FALSE;
}

inline void CSettingsProperties::SetSettingsFlags(DWORD dwFlags,BOOL bState)
{
	if (bState)
		m_dwSettingsFlags|=dwFlags;
	else
		m_dwSettingsFlags&=~dwFlags;
}

inline void CSettingsProperties::SetSettingsFlags(DWORD dwFlags)
{
	m_dwSettingsFlags|=dwFlags;
}

inline void CSettingsProperties::ClearSettingsFlags(DWORD dwFlags)
{
	m_dwSettingsFlags&=~dwFlags;
}

inline CSettingsProperties::CGeneralSettingsPage::CGeneralSettingsPage()
:	CPropertyPage(IDD_GENERALSETTINGS,IDS_GENERALSETTINGS)
{
}

inline CSettingsProperties::CAdvancedSettingsPage::CAdvancedSettingsPage()
{	
	DebugMessage("CSettingsProperties::CAdvancedSettingsPage::CAdvancedSettingsPage()"); 

	OPTIONPAGE op;
	op.dwFlags=COptionsPropertyPage::OPTIONPAGE::opTemplateIsID|
		COptionsPropertyPage::OPTIONPAGE::opCaptionIsID|
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

inline CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::CExcludeDirectoryDialog(LPCWSTR szFiles,const CArrayFAP<LPWSTR>& rDirectories)
:	CDialog(IDD_EXCLUDEDIRECTORIES),m_bDirectoryFieldChanged(TRUE),m_sFiles(szFiles)
{
	for (int i=0;i<rDirectories.GetSize();i++)
		m_aDirectories.Add(alloccopy(rDirectories[i]));
}

inline CSettingsProperties::CAutoUpdateSettingsPage::CAutoUpdateSettingsPage()
:	CPropertyPage(IDD_AUTOUPDATESETTINGS,IDS_AUTOUPDATESETTINGS)
{
}

inline CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::CCheduledUpdateDlg()
:	CDialog(IDD_SCHEDULEDUPDATE),m_pTypeCombo(NULL),m_pSchedule(NULL),m_bChanged(FALSE)
{
}



inline CSettingsProperties::CKeyboardShortcutsPage::CAdvancedDlg::CAdvancedDlg(CShortcut* pShortcut)
:	CDialog(IDD_SHORTCUTADVANCED),m_pShortcut(pShortcut)
{
}

inline void CSettingsProperties::CKeyboardShortcutsPage::RefreshShortcutListLabels()
{
	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	m_pList->RedrawItems(nItem,nItem);
}

inline void CSettingsProperties::CKeyboardShortcutsPage::SetHotKey(BYTE bKey,BYTE bModifiers)
{
	SendDlgItemMessage(IDC_SHORTCUTKEY,HKM_SETHOTKEY,MAKEWORD(bKey,bModifiers),0);
	
	// Check whether MOD_EXT is needed
	if (LOBYTE(SendDlgItemMessage(IDC_SHORTCUTKEY,HKM_GETHOTKEY))!=bKey)
		SendDlgItemMessage(IDC_SHORTCUTKEY,HKM_SETHOTKEY,MAKEWORD(bKey,bModifiers|HOTKEYF_EXT),0);
}

inline void CSettingsProperties::CKeyboardShortcutsPage::SetHotKeyForShortcut(CShortcut* pShortcut)
{
	return SetHotKey(pShortcut->m_bVirtualKey,pShortcut->GetHotkeyModifiers());
}

inline void CSettingsProperties::CKeyboardShortcutsPage::GetHotKeyForShortcut(CShortcut* pShortcut) const
{
	// Using hotkey control
	WORD wKey=(WORD)SendDlgItemMessage(IDC_SHORTCUTKEY,HKM_GETHOTKEY,0,0);
	pShortcut->m_bVirtualKey=LOBYTE(wKey);
    pShortcut->SetHotkeyModifiers(HIBYTE(wKey));
}

inline CAction::Action CSettingsProperties::CKeyboardShortcutsPage::GetSelectedAction() const
{
	int nAction=(CAction::Action)SendDlgItemMessage(IDC_ACTION,CB_GETCURSEL,0,0);
	if ((INT)nAction==CB_ERR || nAction==0)
		return CAction::None;
	
	return (CAction::Action)(nAction-1);
}
#endif