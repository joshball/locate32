#if !defined(SETTINGSDLG_INL)
#define SETTINGSDLG_INL

#if _MSC_VER >= 1000
#pragma once
#endif 

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
:	COptionsPropertyPage(IDD_ADVANCEDSETTINGS,IDS_ADVANCEDSETTINGS,IDC_SETTINGS)
{
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



#endif