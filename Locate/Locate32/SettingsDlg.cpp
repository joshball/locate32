#include <HFCLib.h>
#include "Locate32.h"
#include <shlwapi.h>

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



////////////////////////////////////////
// CSettingsProperties
////////////////////////////////////////

CSettingsProperties::CSettingsProperties(HWND hParent)
:	CPropertySheet(IDS_SETTINGS,hParent,0),
	m_nMaximumFoundFiles(0),
	m_dwLocateDialogFlags(CLocateDlg::fgDefault),
	m_dwLocateDialogExtraFlags(CLocateDlg::efDefault),
	m_bDefaultFlag(defaultDefault),	m_dwSettingsFlags(settingsDefault),
	m_nNumberOfDirectories(DEFAULT_NUMBEROFDIRECTORIES),
	m_nTransparency(0),m_nToolTipTransparency(0)
{
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
	AddPage((CPropertyPage*)m_pAutoUpdate);
	AddPage((CPropertyPage*)m_pKeyboardShortcuts);
	
	m_pGeneral->m_pSettings=m_pAdvanced->m_pSettings=this;
	m_pLanguage->m_pSettings=m_pAutoUpdate->m_pSettings=this;
	m_pDatabases->m_pSettings=m_pKeyboardShortcuts->m_pSettings=this;

	m_psh.dwFlags|=PSH_NOAPPLYNOW|PSH_NOCONTEXTHELP;

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
	
	m_Schedules.RemoveAll();

	
}

BOOL CSettingsProperties::LoadSettings()
{
	//DebugMessage("CSettingsProperties::LoadSettings()");
	CRegKey RegKey;
	
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
	else if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD temp=m_dwLocateDialogFlags;
		RegKey.QueryValue("Program Status",temp);
		m_dwLocateDialogFlags&=~CLocateDlg::fgSave;
		m_dwLocateDialogFlags|=temp&CLocateDlg::fgSave;

		temp=m_dwLocateDialogExtraFlags;
		RegKey.QueryValue("Program StatusExtra",temp);
		m_dwLocateDialogExtraFlags&=~CLocateDlg::efSave;
		m_dwLocateDialogExtraFlags|=temp&CLocateDlg::efSave;

	}
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Recent Strings",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		RegKey.QueryValue("NumberOfDirectories",m_nNumberOfDirectories);

	// Initializing values
	if (GetLocateDlg()==NULL)
	{
		if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
			RegKey.QueryValue("MaximumFoundFiles",m_nMaximumFoundFiles);
	}
	else
		m_nMaximumFoundFiles=GetLocateDlg()->GetMaxFoundFiles();
    	
	
	// Retrieving databases
	m_aDatabases.RemoveAll();
	const CArray<PDATABASE>& rOrigDatabases=GetLocateApp()->GetDatabases();
	for (int i=0;i<rOrigDatabases.GetSize();i++)
		m_aDatabases.Add(new CDatabase(*rOrigDatabases[i]));
	
	SetFlags(settingsDatabasesOverridden,
		((CLocateApp*)GetApp())->GetStartupFlags()&CLocateApp::CStartData::startupDatabasesOverridden);
	
	
	// Loading schedules
	POSITION pPos=GetLocateAppWnd()->m_Schedules.GetHeadPosition();
	while (pPos!=NULL)
	{
		m_Schedules.AddTail(new CSchedule(GetLocateAppWnd()->m_Schedules.GetAt(pPos)));
		pPos=GetLocateAppWnd()->m_Schedules.GetNextPosition(pPos);
	}

	// Loading some general settings
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::defRead)==ERROR_SUCCESS)
	{
		m_bDefaultFlag=0;
		DWORD nTemp=1;
		RegKey.QueryValue("Default CheckIn",nTemp);
		m_bDefaultFlag|=nTemp;
		SendDlgItemMessage(IDC_CHECKIN,CB_SETCURSEL,nTemp);
		
		nTemp=0;
		RegKey.QueryValue("Default MatchWholeName",nTemp);
		if (nTemp) m_bDefaultFlag|=defaultWholeName;
		
		nTemp=1;
		RegKey.QueryValue("Default DataMatchCase",nTemp);
		if (nTemp) m_bDefaultFlag|=defaultMatchCase;
		
		nTemp=0;
		RegKey.QueryValue("Default ReplaceSpaces",nTemp);
		if (nTemp) m_bDefaultFlag|=defaultReplaceSpaces;


		// Overrinding explorer for opening folders
		RegKey.QueryValue("Use other program to open folders",nTemp);
		SetFlags(settingsUseOtherProgramsToOpenFolders,nTemp);
		RegKey.QueryValue("Open folders with",m_OpenFoldersWith);

		if (RegKey.QueryValue("Transparency",nTemp))
			m_nTransparency=min(nTemp,255);
	}

	// m_bAdvancedAndContextMenuFlag
	m_bAdvancedAndContextMenuFlag=GetLocateAppWnd()->m_hHook!=NULL?hookExplorer:0;
	if (RegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		m_bAdvancedAndContextMenuFlag|=cmLocateOnMyComputer;
	if (RegKey.OpenKey(HKCR,"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		m_bAdvancedAndContextMenuFlag|=cmLocateOnMyDocuments;
	if (RegKey.OpenKey(HKCR,"Drive\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		m_bAdvancedAndContextMenuFlag|=cmLocateOnDrives;
	if (RegKey.OpenKey(HKCR,"Directory\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		m_bAdvancedAndContextMenuFlag|=cmLocateOnFolders;
	if (RegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\updatedb",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
		m_bAdvancedAndContextMenuFlag|=cmUpdateOnMyComputer;
	
	
	// m_strLanguage
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource),
		CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("Language",m_strLangFile);
		RegKey.CloseKey();
	}
	if (m_strLangFile.IsEmpty())
	{
		m_strLangFile="lan_en.dll";
		SetFlags(settingsUseLanguageWithConsoleApps);
	}
	else if (RegKey.OpenKey(HKCU,"Software\\Update",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		CString tmp;
		RegKey.QueryValue("Language",tmp);
		SetFlags(settingsUseLanguageWithConsoleApps,tmp.CompareNoCase(m_strLangFile)==0);
		RegKey.CloseKey();
	}
		
	// Checking wheter locate is runned at system startup
	if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		CString Path;
		if (RegKey.QueryValue("Startup",Path))
		{
			if (Path.LastChar()!='\\')
				Path << '\\';
			Path<<"Locate32 Autorun.lnk";
			
			SetFlags(settingsStartLocateAtStartup,CFile::IsFile(Path));
			
		}
	}
	
	// Update status tooltip
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs\\Updatestatus",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		DWORD dwTemp;
		if (RegKey.QueryValue("Transparency",dwTemp))
			m_nToolTipTransparency=min(dwTemp,255);
		if (RegKey.QueryValue("TextColor",dwTemp))
			m_cToolTipTextColor=dwTemp;
		if (RegKey.QueryValue("TitleColor",dwTemp))
			m_cToolTipTitleColor=dwTemp;
		if (RegKey.QueryValue("ErrorColor",dwTemp))
			m_cToolTipErrorColor=dwTemp;
		if (RegKey.QueryValue("BackColor",dwTemp))
			m_cToolTipBackColor=dwTemp;

	
		if (RegKey.QueryValue("TextFont",(LPSTR)&m_lToolTipTextFont,sizeof(LOGFONT))<sizeof(LOGFONT))
			CLocateAppWnd::CUpdateStatusWnd::FillFontStructs(&m_lToolTipTextFont,NULL);
		
		if (RegKey.QueryValue("TitleFont",(LPSTR)&m_lToolTipTitleFont,sizeof(LOGFONT))<sizeof(LOGFONT))
			CLocateAppWnd::CUpdateStatusWnd::FillFontStructs(NULL,&m_lToolTipTitleFont);
		
	}
	return TRUE;
}


BOOL CSettingsProperties::SaveSettings()
{
	//DebugMessage("CSettingsProperties::SaveSettings()");
	CRegKey RegKey;
	CString Path;
	
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\General";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		RegKey.SetValue("Program Status",m_dwLocateDialogFlags&CLocateDlg::fgSave);
		RegKey.SetValue("Program StatusExtra",m_dwLocateDialogExtraFlags&CLocateDlg::efSave);
		RegKey.SetValue("General Flags",m_dwProgramFlags&CLocateApp::pfSave);
		
		RegKey.SetValue("DateFormat",m_DateFormat);
		RegKey.SetValue("TimeFormat",m_TimeFormat);
		RegKey.SetValue("OverrideExplorer",DWORD(m_bAdvancedAndContextMenuFlag&hookExplorer?TRUE:FALSE));
	}
	((CLocateApp*)GetApp())->m_strDateFormat=m_DateFormat;
	((CLocateApp*)GetApp())->m_strTimeFormat=m_TimeFormat;
	((CLocateApp*)GetApp())->m_nFileSizeFormat=m_nFileSizeFormat;

	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Recent Strings";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		RegKey.SetValue("NumberOfDirectories",m_nNumberOfDirectories);
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Locate";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		RegKey.SetValue("MaximumFoundFiles",(LPTSTR)&m_nMaximumFoundFiles,4,REG_DWORD);
	if (GetLocateDlg()!=NULL)
	{
		GetLocateDlg()->SetMaxFoundFiles(m_nMaximumFoundFiles);
		GetLocateDlg()->m_NameDlg.ChangeNumberOfDirectories(m_nNumberOfDirectories);
	}

	
	// Settings databases
	CDatabase::CheckIDs(m_aDatabases);
	if (!IsFlagSet(settingsDatabasesOverridden))
	{
		GetLocateApp()->SetDatabases(m_aDatabases);
		CDatabase::SaveToRegistry(HKCU,"Software\\Update\\Databases",GetLocateApp()->GetDatabases());

		GetLocateApp()->ClearStartupFlag(CLocateApp::CStartData::startupDatabasesOverridden);
	}
	else
		GetLocateApp()->SetDatabases(m_aDatabases);
	
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::defWrite)==ERROR_SUCCESS)
	{
		RegKey.SetValue("Default CheckIn",m_bDefaultFlag&defaultCheckInFlag);
		RegKey.SetValue("Default MatchWholeName",m_bDefaultFlag&defaultWholeName?1:0);
		RegKey.SetValue("Default DataMatchCase",m_bDefaultFlag&defaultMatchCase?1:0);
		RegKey.SetValue("Default ReplaceSpaces",m_bDefaultFlag&defaultReplaceSpaces?1:0);

		// Overrinding explorer for opening folders
		RegKey.SetValue("Use other program to open folders",(DWORD)IsFlagSet(settingsUseOtherProgramsToOpenFolders));
		RegKey.SetValue("Open folders with",m_OpenFoldersWith);

		RegKey.SetValue("Transparency",m_nTransparency);
	}

	// Start/Stop hooking
	if (m_bAdvancedAndContextMenuFlag&hookExplorer)
	{
		if (GetLocateAppWnd()->m_hHook==NULL)
			GetLocateAppWnd()->m_hHook=SetHook(*GetLocateAppWnd());
	}
	else
	{
		if (GetLocateAppWnd()->m_hHook!=NULL)
		{
			UnsetHook(GetLocateAppWnd()->m_hHook);
			GetLocateAppWnd()->m_hHook=NULL;
		}
	}


	// Insert/Remove context menu items
	
	// Locate... on My Computer
	if (RegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.CloseKey();
		if ((m_bAdvancedAndContextMenuFlag&cmLocateOnMyComputer)==0)
			CRegKey::DeleteKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\locate");
	}
	else if (m_bAdvancedAndContextMenuFlag&cmLocateOnMyComputer)
	{
		if (RegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\locate",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			RegKey.SetValue("",CString(IDS_EXPLORERLOCATE));
			CRegKey CommandKey;
			if (CommandKey.OpenKey(RegKey,"command",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
			{
				CString command;
				command.Format("\"%s\" /P4",GetApp()->GetExeName());
				CommandKey.SetValue("",command);
			}
		}
	}
	
	// Locate... on My Documents
	if (RegKey.OpenKey(HKCR,"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.CloseKey();
		if ((m_bAdvancedAndContextMenuFlag&cmLocateOnMyDocuments)==0)
			CRegKey::DeleteKey(HKCR,"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\shell\\locate");
	}
	else if (m_bAdvancedAndContextMenuFlag&cmLocateOnMyDocuments)
	{
		if (RegKey.OpenKey(HKCR,"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\shell\\locate",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			RegKey.SetValue("",CString(IDS_EXPLORERLOCATE));
			CRegKey CommandKey;
			if (CommandKey.OpenKey(RegKey,"command",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
			{
				CString command;
				command.Format("\"%s\" /P3",GetApp()->GetExeName());
				CommandKey.SetValue("",command);
			}
		}
	}

	// Locate... on Drives
	if (RegKey.OpenKey(HKCR,"Drive\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.CloseKey();
		if ((m_bAdvancedAndContextMenuFlag&cmLocateOnDrives)==0)
			CRegKey::DeleteKey(HKCR,"Drive\\shell\\locate");
	}
	else if (m_bAdvancedAndContextMenuFlag&cmLocateOnDrives)
	{
		if (RegKey.OpenKey(HKCR,"Drive\\shell\\locate",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			RegKey.SetValue("",CString(IDS_EXPLORERLOCATE));
			CRegKey CommandKey;
			if (CommandKey.OpenKey(RegKey,"command",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
			{
				CString command;
				command.Format("\"%s\" /p \"%%1\"",GetApp()->GetExeName());
				CommandKey.SetValue("",command);
			}
		}
	}

	// Locate... on Directories
	if (RegKey.OpenKey(HKCR,"Directory\\shell\\locate",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.CloseKey();
		if ((m_bAdvancedAndContextMenuFlag&cmLocateOnFolders)==0)
			CRegKey::DeleteKey(HKCR,"Directory\\shell\\locate");
	}
	else if (m_bAdvancedAndContextMenuFlag&cmLocateOnFolders)
	{
		if (RegKey.OpenKey(HKCR,"Directory\\shell\\locate",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			RegKey.SetValue("",CString(IDS_EXPLORERLOCATE));
			CRegKey CommandKey;
			if (CommandKey.OpenKey(RegKey,"command",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
			{
				CString command;
				command.Format("\"%s\" /p \"%%1\"",GetApp()->GetExeName());
				CommandKey.SetValue("",command);
			}
		}
	}

	// Update Database... on My Computer
	if (RegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\updatedb",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.CloseKey();
		if ((m_bAdvancedAndContextMenuFlag&cmUpdateOnMyComputer)==0)
			CRegKey::DeleteKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\updatedb");
	}
	else if (m_bAdvancedAndContextMenuFlag&cmUpdateOnMyComputer)
	{
		if (RegKey.OpenKey(HKCR,"CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\shell\\updatedb",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			RegKey.SetValue("",CString(IDS_EXPLORERUPDATE));
			CRegKey CommandKey;
			if (CommandKey.OpenKey(RegKey,"command",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
			{
				CString command(GetApp()->GetExeName(),GetApp()->GetExeName().FindLast('\\')+1);
				command << "updtdb32.exe\"";
				command.Insert(0,'\"');
				CommandKey.SetValue("",command);
			}
		}
	}

	// Language
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource),
		CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		RegKey.SetValue("Language",m_strLangFile);
		RegKey.CloseKey();
		
		if (RegKey.OpenKey(HKCU,"Software\\Update",
				CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
		{
			if (IsFlagSet(settingsUseLanguageWithConsoleApps))
				RegKey.SetValue("Language",m_strLangFile);
			else
				RegKey.DeleteValue("Language");
		}

	}	

	// Creating or deleting shortcut to Startup mene if necessary
	if (RegKey.OpenKey(HKCU,"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		CString Path;
		if (RegKey.QueryValue("Startup",Path))
		{
			if (Path.LastChar()!='\\')
				Path << '\\';
			Path<<"Locate32 Autorun.lnk";
			
			if (IsFlagSet(settingsStartLocateAtStartup) && !CFile::IsFile(Path))
				CreateShortcut(Path,GetApp()->GetExeName(),""," /S");
			else if (CFile::IsFile(Path))
				DeleteFile(Path);

		}	
	}

	// Update status tooltip
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Dialogs\\Updatestatus";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		
		RegKey.SetValue("Transparency",m_nToolTipTransparency);
		RegKey.SetValue("TextFont",(LPSTR)&m_lToolTipTextFont,sizeof(LOGFONT));
		RegKey.SetValue("TitleFont",(LPSTR)&m_lToolTipTitleFont,sizeof(LOGFONT));
		RegKey.SetValue("TextColor",(DWORD)m_cToolTipTextColor);
		RegKey.SetValue("TitleColor",(DWORD)m_cToolTipTitleColor);
		RegKey.SetValue("ErrorColor",(DWORD)m_cToolTipErrorColor);
		RegKey.SetValue("BackColor",(DWORD)m_cToolTipBackColor);
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
	if (m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgLVStyleSingleClick)
	{
		CheckDlgButton(IDC_SINGLECLICK,1);
		OnSingleClick();
		if ((m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgLVStyleAlwaysUnderline)==CLocateDlg::fgLVStyleAlwaysUnderline)
			CheckDlgButton(IDC_ALWAYSUNDERLINE,1);
		else if ((m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgLVStyleAlwaysUnderline)==CLocateDlg::fgLVStyleUnderLine)
			CheckDlgButton(IDC_POINTUNDERLINE,1);
		else
			CheckDlgButton(IDC_NEVERUNDERLINE,1);
	}
	else
	{
		CheckDlgButton(IDC_DOUBLECLICK,1);
		CheckDlgButton(IDC_NEVERUNDERLINE,1);
		OnDoubleClick();
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
	// Close to ST
	if (GetLocateApp()->IsStartupFlagSet(CLocateApp::CStartData::startupLeaveBackground))
	{
		EnableDlgItem(IDC_CLOSETOSYSTEMTRAY,FALSE);
		CheckDlgButton(IDC_CLOSETOSYSTEMTRAY,1);
	}
	else if (m_pSettings->m_dwLocateDialogFlags&CLocateDlg::fgDialogLeaveLocateBackground)
		CheckDlgButton(IDC_CLOSETOSYSTEMTRAY,1);
	
	// Defaults
	if (IsFullUnicodeSupport())
	{
		SendDlgItemMessageW(IDC_CHECKIN,CB_ADDSTRING,0,(LPARAM)(LPCWSTR)CStringW(IDS_FILENAMESONLY));
		SendDlgItemMessageW(IDC_CHECKIN,CB_ADDSTRING,0,(LPARAM)(LPCWSTR)CStringW(IDS_FILEANDFOLDERNAMES));
		SendDlgItemMessageW(IDC_CHECKIN,CB_ADDSTRING,0,(LPARAM)(LPCWSTR)CStringW(IDS_FOLDERNAMESONLY));
	}
	else
	{
		SendDlgItemMessage(IDC_CHECKIN,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_FILENAMESONLY));
		SendDlgItemMessage(IDC_CHECKIN,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_FILEANDFOLDERNAMES));
		SendDlgItemMessage(IDC_CHECKIN,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_FOLDERNAMESONLY));
	}
	SendDlgItemMessage(IDC_CHECKIN,CB_SETCURSEL,m_pSettings->m_bDefaultFlag&CSettingsProperties::defaultCheckInFlag);
	
	CheckDlgButton(IDC_MATCHWHOLEFILENAMEONLY,m_pSettings->m_bDefaultFlag&CSettingsProperties::defaultWholeName);
	CheckDlgButton(IDC_REPLACESPACES,m_pSettings->m_bDefaultFlag&CSettingsProperties::defaultReplaceSpaces);
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
	case IDC_SINGLECLICK:
		OnSingleClick();
		break;
	case IDC_DOUBLECLICK:
		OnDoubleClick();
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
		CLocateDlg::fgDialogRememberFields|CLocateDlg::fgDialogMinimizeToST);

	// Setting tree view mouse behaviour
	if (IsDlgButtonChecked(IDC_SYSTEMSETTINGS))
		m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgLVStyleSystemDefine;
	else
	{
		if (IsDlgButtonChecked(IDC_SINGLECLICK))
		{
			m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgLVStyleSingleClick;
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
	// Load file types from registry
	if (!GetLocateApp()->IsStartupFlagSet(CLocateApp::CStartData::startupLeaveBackground))
	{
		if (IsDlgButtonChecked(IDC_CLOSETOSYSTEMTRAY))
			m_pSettings->m_dwLocateDialogFlags|=CLocateDlg::fgDialogLeaveLocateBackground;
	}
		
	// Defaults:
	m_pSettings->m_bDefaultFlag=(BYTE)SendDlgItemMessage(IDC_CHECKIN,CB_GETCURSEL);
	if (IsDlgButtonChecked(IDC_MATCHWHOLEFILENAMEONLY))
		m_pSettings->m_bDefaultFlag|=CSettingsProperties::defaultWholeName;
	if (IsDlgButtonChecked(IDC_REPLACESPACES))
		m_pSettings->m_bDefaultFlag|=CSettingsProperties::defaultReplaceSpaces;
	if (IsDlgButtonChecked(IDC_MATCHCASE))
		m_pSettings->m_bDefaultFlag|=CSettingsProperties::defaultMatchCase;
	
	CPropertyPage::OnApply();
	return TRUE;
}

void CSettingsProperties::CGeneralSettingsPage::OnCancel()
{
	m_pSettings->SetFlags(CSettingsProperties::settingsCancelled);
	
	CPropertyPage::OnCancel();
}


BYTE CSettingsProperties::CGeneralSettingsPage::OnSystemSettings()
{
	if (IsDlgButtonChecked(IDC_SYSTEMSETTINGS))
	{
		EnableDlgItem(IDC_SINGLECLICK,FALSE);
		EnableDlgItem(IDC_DOUBLECLICK,FALSE);
		EnableDlgItem(IDC_NEVERUNDERLINE,FALSE);
		EnableDlgItem(IDC_POINTUNDERLINE,FALSE);
		EnableDlgItem(IDC_ALWAYSUNDERLINE,FALSE);
	}
	else
	{
		EnableDlgItem(IDC_SINGLECLICK,TRUE);
		EnableDlgItem(IDC_DOUBLECLICK,TRUE);
		if (IsDlgButtonChecked(IDC_SINGLECLICK))
		{
			EnableDlgItem(IDC_NEVERUNDERLINE,TRUE);
			EnableDlgItem(IDC_POINTUNDERLINE,TRUE);
			EnableDlgItem(IDC_ALWAYSUNDERLINE,TRUE);
		}
	}
	return TRUE;
}

BYTE CSettingsProperties::CGeneralSettingsPage::OnSingleClick()
{
	CheckDlgButton(IDC_SINGLECLICK,1);
	CheckDlgButton(IDC_DOUBLECLICK,0);
	EnableDlgItem(IDC_NEVERUNDERLINE,TRUE);
	EnableDlgItem(IDC_POINTUNDERLINE,TRUE);
	EnableDlgItem(IDC_ALWAYSUNDERLINE,TRUE);
	return TRUE;
}

BYTE CSettingsProperties::CGeneralSettingsPage::OnDoubleClick()
{
	CheckDlgButton(IDC_SINGLECLICK,0);
	CheckDlgButton(IDC_DOUBLECLICK,1);
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
		CreateEdit(IDS_ADVSETOPENFOLDERWITH,DefaultEditStrProc,0,&m_pSettings->m_OpenFoldersWith),
		NULL
	};

	Item* FileViewItems[]={
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
		CreateCheckBox(IDS_ADVSETDONTSHOWHIDDENFILES,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVDontShowHiddenFiles,&m_pSettings->m_dwLocateDialogFlags),
		CreateCheckBox(IDS_ADVSETNODOUBLERESULTS,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVNoDoubleItems,&m_pSettings->m_dwLocateDialogFlags),
		CreateComboBox(IDS_ADVSETSHOWDATESINFORMAT,DateFormatComboProc,0,0),
		CreateComboBox(IDS_ADVSETSHOWTIMESINFORMAT,TimeFormatComboProc,0,0),
		CreateListBox(IDS_ADVSETSHOWFILESIZESINFORMAT,FileSizeListProc,0,&m_pSettings->m_nFileSizeFormat),
		CreateCheckBox(IDS_ADVSETFORMATWITHUSERLOCALE,NULL,DefaultCheckBoxProc,
			CLocateApp::pfFormatUseLocaleFormat,&m_pSettings->m_dwProgramFlags),
		CreateCheckBox(IDS_ADVSETUSEPROGRAMFORFOLDERS,OtherExplorerProgram,DefaultCheckBoxProc,
			CSettingsProperties::settingsUseOtherProgramsToOpenFolders,&m_pSettings->m_dwSettingsFlags),
		CreateCheckBox(IDS_ADVSETCOMPUTEMD5SUMS,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLVComputeMD5Sums,&m_pSettings->m_dwLocateDialogFlags),
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
		NULL,
		NULL
	};

	OSVERSIONINFOEX oi;
	oi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
	BOOL bRet=GetVersionEx((LPOSVERSIONINFO) &oi);
	if (!(bRet && oi.dwPlatformId!=VER_PLATFORM_WIN32_NT ||
		!(oi.dwMajorVersion>=5 || (oi.dwMajorVersion==4 && oi.wServicePackMajor>=3) )))
	{
		// Overwriding does not work in Win9x or old WinNt
		SystemItems[2]=CreateCheckBox(IDS_ADVSETOVERWRITEKEYS,NULL,
			DefaultCheckBoxProc,CSettingsProperties::hookExplorer,&m_pSettings->m_bAdvancedAndContextMenuFlag);
	}
	
	Item* LookInItems[]={
		CreateNumeric(IDS_ADVSETNUMBEROFDIRECTORIES,DefaultNumericProc,
			MAKELONG(0,100),&m_pSettings->m_nNumberOfDirectories),
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
		CreateCheckBox(IDS_ADVSETLOADTYPES,NULL,DefaultCheckBoxProc,
			CLocateDlg::fgLoadRegistryTypes,&m_pSettings->m_dwLocateDialogFlags),
		CreateRoot(IDS_ADVSETLOOKINCOMBO,LookInItems),
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
		NULL, // For transparency
		NULL
	};
	if (GetProcAddress(GetModuleHandle("user32.dll"),"SetLayeredWindowAttributes")!=NULL)
	{
		// Needs at least Win2k
		DialogItems[4]=CreateNumeric(IDS_ADVSETTRANSPARENCY,DefaultNumericProc,
			MAKELONG(0,255),&m_pSettings->m_nTransparency);
		StatusTooltipItems[6]=CreateNumeric(IDS_ADVSETTOOLTIPTRANSPARENCY,DefaultNumericProc,
			MAKELONG(0,255),&m_pSettings->m_nToolTipTransparency);

	}
		

	Item* UpdateProcessItems[]={
		CreateCheckBox(IDS_ADVSETSHOWCRITICALERRORS,NULL,
			DefaultCheckBoxProc,CLocateApp::pfShowCriticalErrors,&m_pSettings->m_dwProgramFlags),
		CreateCheckBox(IDS_ADVSETSHOWNONCRITICALERRORS,NULL,
			DefaultCheckBoxProc,CLocateApp::pfShowNonCriticalErrors,&m_pSettings->m_dwProgramFlags),
		CreateCheckBox(IDS_ADVSETSHOWUPDATESTATUSTOOLTIP,StatusTooltipItems,
			DefaultCheckBoxProc,CLocateApp::pfEnableUpdateTooltip,&m_pSettings->m_dwProgramFlags),
		NULL
	};
	

	Item* Parents[]={
		CreateRoot(IDS_ADVSETRESULTS,ResultsItems),
		CreateRoot(IDS_ADVSETDIALOGS,DialogItems),
		CreateRoot(IDS_ADVSETUPDATEPROCESS,UpdateProcessItems),
		CreateRoot(IDS_ADVSETSYSTEM,SystemItems),
		NULL};

	Initialize(Parents);
	return FALSE;
}

void CSettingsProperties::CAdvancedSettingsPage::OnCancel()
{
	m_pSettings->SetFlags(CSettingsProperties::settingsCancelled);

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
		
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_ADVSETSYSTEMDEFAULT));		
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
			((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer.RemoveAll();
			EnumDateFormats(EnumDateFormatsProc,LOCALE_USER_DEFAULT,DATE_SHORTDATE);
		
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_ADVSETSYSTEMDEFAULT));		
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_SETCURSEL,0,0);		

			for (int i=0;i<((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer.GetSize();i++)
				::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)((CAdvancedSettingsPage*)pParams->pPage)->m_aBuffer[i]);
			
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
			CString text(IDS_ADVSETFILESIZEFORMATLESS1KB);
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATDEPENGINGSIZE);
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATBYTES);
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATBYTENOUNITS);
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATKB);
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATKBNOUNITS);
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATMB);
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)text);		
			text.LoadString(IDS_ADVSETFILESIZEFORMATMBNOUNITS);
			::SendMessage(((INITIALIZEPARAMS*)pParams)->hControl,CB_ADDSTRING,0,(LPARAM)(LPCSTR)text);		
	
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


////////////////////////////////////////
// CLanguageSettingsPage
////////////////////////////////////////


BOOL CSettingsProperties::CLanguageSettingsPage::OnInitDialog(HWND hwndFocus)
{
	CPropertyPage::OnInitDialog(hwndFocus);

	m_pList=new CListCtrl(GetDlgItem(IDC_LANGUAGE));

	m_pList->InsertColumn(0,CString(IDS_LANGUAGE),LVCFMT_LEFT,130);
	m_pList->InsertColumn(1,CString(IDS_LANGUAGEFILE),LVCFMT_LEFT,80);
	m_pList->InsertColumn(2,CString(IDS_LANGUAGEDESC),LVCFMT_LEFT,100);
	
	m_pList->SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT ,LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT );
	m_pList->LoadColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs","Language Settings List Widths");

	FindLanguages();
	
	CheckDlgButton(IDC_USEWITHCONSOLEAPPS,
		m_pSettings->IsFlagSet(CSettingsProperties::settingsUseLanguageWithConsoleApps));
	return FALSE;
}

BOOL CSettingsProperties::CLanguageSettingsPage::OnApply()
{
	CPropertyPage::OnApply();
	
    m_pSettings->SetFlags(settingsUseLanguageWithConsoleApps,IsDlgButtonChecked(IDC_USEWITHCONSOLEAPPS));

	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);

	ASSERT(nItem!=-1);

	LanguageItem* pli=(LanguageItem*)m_pList->GetItemData(nItem);
	m_pSettings->m_strLangFile=pli->File;
	return TRUE;
}

void CSettingsProperties::CLanguageSettingsPage::OnDestroy()
{
	CPropertyPage::OnDestroy();

	if (m_pList!=NULL)
	{
		m_pList->SaveColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs","Language Settings List Widths");
		delete m_pList;
		m_pList=NULL;
	}
}
		
void CSettingsProperties::CLanguageSettingsPage::OnCancel()
{
	m_pSettings->SetFlags(CSettingsProperties::settingsCancelled);

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
		ListNotifyHandler((LV_DISPINFO*)pnmh,(NMLISTVIEW*)pnmh);
		break;
	}
	return CPropertyPage::OnNotify(idCtrl,pnmh);
}


BOOL CSettingsProperties::CLanguageSettingsPage::ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm)
{
	switch(pLvdi->hdr.code)
	{
	case LVN_DELETEITEM:
		delete (LanguageItem*)pNm->lParam;
		break;
	case LVN_GETDISPINFO:
		{
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

	CString Path(GetApp()->GetExeName(),GetApp()->GetExeName().FindLast('\\')+1);
	Path<<"*.dll";

	
	LVITEM li;
	li.mask=LVIF_PARAM|LVIF_STATE|LVIF_TEXT;
	li.iItem=0;
	li.pszText=LPSTR_TEXTCALLBACK;
	li.stateMask=LVIS_SELECTED;

	CFileFind ff;
	BOOL bRet=ff.FindFile(Path);

    while (bRet)
	{
		HINSTANCE hLib=LoadLibrary(ff.GetFilePath());
        if (hLib!=NULL)
		{
			LANGCALL pFunc=(LANGCALL)GetProcAddress(hLib,"GetLocateLanguageFileInfo");
			if (pFunc==NULL) // Watcom style
				pFunc=(LANGCALL)GetProcAddress(hLib,"_GetLocateLanguageFileInfo");
			if (pFunc!=NULL)
			{
				LanguageItem* pli=new LanguageItem;
				pFunc(pli->Language.GetBuffer(200),200,pli->Description.GetBuffer(1000),1000);
				pli->File=ff.GetFileName();
				li.lParam=(LPARAM)pli;
				li.iSubItem=0;
				if (m_pSettings->m_strLangFile.CompareNoCase(ff.GetFileName())==0)
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
	m_pList->InsertColumn(0,CString(IDS_DATABASENAME),LVCFMT_LEFT,100,0);
	m_pList->InsertColumn(1,CString(IDS_DATABASEFILE),LVCFMT_LEFT,130,0);
	m_pList->InsertColumn(2,CString(IDS_GLOBALUPDATE),LVCFMT_LEFT,70,0);
	if (((CLocateApp*)GetApp())->m_wComCtrlVersion<0x0600)
	{
		m_pList->InsertColumn(3,CString(IDS_THREADID),LVCFMT_LEFT,40,0);
		int oa[]={3,0,1,2};
		m_pList->SetColumnOrderArray(4,oa);
	}
		
	m_pList->LoadColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs","Databases Settings List Widths");
	
	// Setting threads counter
	CSpinButtonCtrl Spin(GetDlgItem(IDC_THREADSPIN));
	Spin.SetBuddy(GetDlgItem(IDC_THREADS));
	Spin.SetRange(1,99);

	SetDatabasesToList();


	if (m_pSettings->IsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
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
		m_pList->SaveColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs","Databases Settings List Widths");
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
		dbd.m_pDatabase=CDatabase::FromDefaults(FALSE,NULL,0);
	dbd.m_nMaximumNumbersOfThreads=m_nThreadsCurrently;
	
	int iItem=m_pList->GetNextItem(-1,LVNI_ALL);
	while (iItem!=-1)
	{
		CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(iItem);
		if (pDatabase!=NULL)
			dbd.m_aOtherDatabases.Add(pDatabase);
		iItem=m_pList->GetNextItem(iItem,LVNI_ALL);
	}

	if (dbd.DoModal(*this,LanguageSpecificResource))
	{
		LVITEM li;
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

	CDatabaseDialog dbd;

	if (m_pSettings->IsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
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

	EnableButtons();
	m_pList->SetFocus();

	if (m_nThreadsCurrently>1 && GetLocateApp()->m_wComCtrlVersion<0x0600)
		m_pList->SortItems(ThreadSortProc,NULL);
}

void CSettingsProperties::CDatabasesSettingsPage::OnRemove()
{
	int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	if (nItem==-1)
		return;

	CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(nItem);
	if (pDatabase->GetArchiveType()==CDatabase::archiveFile)
	{
		if (CFile::IsFile(pDatabase->GetArchiveName()))
		{
			CString str;
			str.Format(IDS_DELETEDATABASEQUESTION,pDatabase->GetArchiveName());
			int nRet=MessageBox(str,CString(IDS_DELETEDATABASE),MB_ICONQUESTION|MB_YESNO);
			if (nRet==IDYES)
				DeleteFile(pDatabase->GetArchiveName());
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
	
	CDatabase::LoadFromRegistry(HKCU,"Software\\Update\\Databases",m_pSettings->m_aDatabases);

	// If there is still no any available database, try to load old style db
	if (m_pSettings->m_aDatabases.GetSize()==0)
	{
		CDatabase* pDatabase=CDatabase::FromOldStyleDatabase(HKCU,"Software\\Update\\Database");
		if (pDatabase==NULL)
		{
			CString& sExe=GetApp()->GetExeName();
			pDatabase=CDatabase::FromDefaults(TRUE,sExe,sExe.FindLast('\\')+1); // Nothing else can be done?
		}
		else
		{
			if (CDatabase::SaveToRegistry(HKCU,"Software\\Update\\Databases",&pDatabase,1))
				CRegKey::DeleteKey(HKCU,"Software\\Update\\Database");
		}
		m_pSettings->m_aDatabases.Add(pDatabase);
	}
	
	
	// Databases are not anymore overridden
	m_pSettings->ClearFlags(CSettingsProperties::settingsDatabasesOverridden);
	
	
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
		return ListNotifyHandler((LV_DISPINFO*)pnmh,(NMLISTVIEW*)pnmh);
	}
	return CPropertyPage::OnNotify(idCtrl,pnmh);
}

void CSettingsProperties::CDatabasesSettingsPage::OnImport()
{
	// Set wait cursor
	CWaitCursor wait;
	
	// Initializing file name dialog
	CString Title;
	CFileDialog fd(TRUE,"*","",OFN_EXPLORER|OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_ENABLESIZING,CString(IDS_IMPORTDATABASEFILTERS));
	fd.EnableFeatures();
	Title.LoadString(IDS_IMPORTDATABASESETTINGS);
	fd.m_pofn->lpstrTitle=Title;
	
	// Ask file name
	if (!fd.DoModal(*this))
		return;

	// First, check whether file is database and read information
	CDatabase* pDatabase;
	CDatabaseInfo* pDatabaseInfo=CDatabaseInfo::GetFromFile(fd.GetPathName());
	if (pDatabaseInfo!=NULL)
	{
		if (!pDatabaseInfo->szExtra2.IsEmpty())
			pDatabase=CDatabase::FromExtraBlock(pDatabaseInfo->szExtra2);
		if (pDatabase==NULL && !pDatabaseInfo->szExtra1.IsEmpty())
			pDatabase=CDatabase::FromExtraBlock(pDatabaseInfo->szExtra1);

		if (pDatabase!=NULL)
		{
			pDatabase->SetArchiveType(CDatabase::archiveFile);
			pDatabase->SetArchiveName(fd.GetPathName());
		}
	}
	else
	{
		CFile* pFile=NULL;
		char* pFileContent=NULL;
		BOOL bError=FALSE;

		try {
			pFile=new CFile(fd.GetPathName(),CFile::defRead,TRUE);

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
			pDatabase=CDatabase::FromExtraBlock(pFileContent);

		if (pFile!=NULL)
			delete pFile;
		if (pFileContent!=NULL)
			delete[] pFileContent;
	}

	if (pDatabase==NULL)
	{
		CString msg;
		msg.Format(IDS_UNABLEREADSETTINGS,(LPCSTR)fd.GetPathName());
		MessageBox(msg,Title,MB_OK|MB_ICONERROR);
		return;
	}

	if (pDatabase->GetThreadId()>=m_nThreadsCurrently)
	{
		if (MessageBox(CString(IDS_INCREASETHREADCOUNT),Title,MB_ICONQUESTION|MB_YESNO)==IDYES)
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
	
	LPCSTR pCurFile=szEmpty;
	if (pDatabase->GetArchiveType()==CDatabase::archiveFile)
		pCurFile=pDatabase->GetArchiveName();

	// Initializing file name dialog
	CString Title;
	
	CFileDialog fd(FALSE,"*",pCurFile,OFN_EXPLORER|OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_ENABLESIZING,CString(IDS_EXPORTDATABASEFILTERS));
	fd.EnableFeatures();
	Title.LoadString(IDS_EXPORTDATABASESETTINGS);
	fd.m_pofn->lpstrTitle=Title;
	
	
	// Ask file name
	if (!fd.DoModal(*this))
		return;
	
	if (CFile::IsFile(fd.GetPathName()))
	{
		if (pDatabase->SaveExtraBlockToDbFile(fd.GetPathName()))
			return;

		CString msg;
		msg.Format(IDS_FILEISNOTDATABASE,LPCSTR(fd.GetPathName()));
		if (MessageBox(msg,Title,MB_ICONQUESTION|MB_YESNO)==IDNO)
			return;			
	}

	CFile* pFile=NULL;
	LPSTR pExtra=pDatabase->ConstructExtraBlock();
	DWORD dwExtraLen=istrlen(pExtra);

	try {
		pFile=new CFile(fd.GetPathName(),CFile::defWrite,TRUE);
		pFile->Write(pExtra,dwExtraLen);
	}
	catch (...)
	{
		MessageBox(CString(IDS_CANNOTWRITESETTINGS),Title,MB_ICONERROR|MB_OK);
	}
	if (pFile!=NULL)
		delete pFile;
	delete[] pExtra;
    
}

BOOL CSettingsProperties::CDatabasesSettingsPage::ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm)
{
	switch(pLvdi->hdr.code)
	{
	case LVN_ITEMCHANGING:
		if (m_pSettings->IsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
			return TRUE;
		return FALSE;
	case LVN_ITEMCHANGED:
		if (pNm->lParam!=NULL && (pNm->uNewState&0x00002000)!=(pNm->uOldState&0x00002000))
		{
			if (m_pSettings->IsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
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
	case LVN_GETDISPINFO:
		{
			CDatabase* pDatabase=(CDatabase*)pLvdi->item.lParam;
			if (pDatabase==NULL)
				break;
				
			switch (pLvdi->item.iSubItem)
			{
			case 0:
				if (!m_pSettings->IsFlagSet(CSettingsProperties::settingsDatabasesOverridden))
					pLvdi->item.pszText=const_cast<LPSTR>(pDatabase->GetName());
				else
				{
					ISDLGTHREADOK
					if (g_szBuffer!=NULL)
						delete[] g_szBuffer;
					g_szBuffer=new char[40];
					LoadString(IDS_COMMANDLINEARGUMENT,g_szBuffer,40);
					pLvdi->item.pszText=g_szBuffer;
				}
				break;
			case 1:
				pLvdi->item.pszText=const_cast<LPSTR>(pDatabase->GetArchiveName());
				break;
			case 2:
				ISDLGTHREADOK
				if (g_szBuffer!=NULL)
					delete[] g_szBuffer;
				g_szBuffer=new char[100];
				LoadString(pDatabase->IsGloballyUpdated()?IDS_YES:IDS_NO,g_szBuffer,100);
				pLvdi->item.pszText=g_szBuffer;
				break;
			case 3:
				ISDLGTHREADOK
				if (g_szBuffer!=NULL)
					delete[] g_szBuffer;
				g_szBuffer=new char[20];
				itoa(pDatabase->GetThreadId()+1,g_szBuffer,10);
				pLvdi->item.pszText=g_szBuffer;
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
	int nNext;
	int nItem=m_pList->GetNextItem(-1,LVNI_ALL);

	while ((nNext=m_pList->GetNextItem(nItem,LVNI_ABOVE))!=-1)
	{
		if (nNext==nItem)
			break; // This should not be like that, why is it?
		nItem=nNext;
	}
	
	while (nItem!=-1)
	{
		PDATABASE pDatabase=(PDATABASE)m_pList->GetItemData(nItem);
		ASSERT(pDatabase!=NULL);

		m_pSettings->m_aDatabases.Add(pDatabase);
		m_pList->SetItemData(nItem,NULL);

		nNext=m_pList->GetNextItem(nItem,LVNI_BELOW);
		if (nNext==nItem)
			break;
		nItem=nNext;
	}
	
	return TRUE;
}

void CSettingsProperties::CDatabasesSettingsPage::OnCancel()
{
	m_pSettings->SetFlags(CSettingsProperties::settingsCancelled);


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
	
	BOOL bEnable=nSelectedItem!=-1 && !m_pSettings->IsFlagSet(CSettingsProperties::settingsDatabasesOverridden);
	
	EnableDlgItem(IDC_EDIT,nSelectedItem!=-1);
	EnableDlgItem(IDC_REMOVE,bEnable);
	EnableDlgItem(IDC_UPDATE,bEnable);
	
	
	EnableDlgItem(IDC_EXPORT,bEnable);
	EnableDlgItem(IDC_IMPORT,!m_pSettings->IsFlagSet(CSettingsProperties::settingsDatabasesOverridden));
	
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
	m_pList->InsertColumn(0,CString(IDS_VOLUMELABEL),LVCFMT_LEFT,95,0);
	m_pList->InsertColumn(1,CString(IDS_VOLUMEPATH),LVCFMT_LEFT,75,1);
	m_pList->InsertColumn(2,CString(IDS_VOLUMETYPE),LVCFMT_LEFT,70,2);
	m_pList->InsertColumn(3,CString(IDS_VOLUMEFILESYSTEM),LVCFMT_LEFT,65,3);
	m_pList->LoadColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs","Database Dialog List Widths");
	
	for (int i=1;i<=m_nMaximumNumbersOfThreads;i++)
	{
		char num[5];
		itoa(i,num,10);
		SendDlgItemMessage(IDC_USEDTHREAD,CB_ADDSTRING,0,LPARAM(num));
	}

    // Setting current information	
	if (m_bDontEditName)
	{
		EnableDlgItem(IDC_NAME,FALSE);
		SetDlgItemText(IDC_NAME,CString(IDS_COMMANDLINEARGUMENT));
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
		
	// Inserting local drives to drive list
	DWORD nLength=GetLogicalDriveStrings(0,NULL)+1;
	if (nLength<2)
		return FALSE;

	char* szDrives=new char[nLength+1];
	GetLogicalDriveStrings(nLength,szDrives);
	for (LPSTR szDrive=szDrives;szDrive[0]!='\0';szDrive+=4)
		AddDriveToList(szDrive);
	delete[] szDrives;

	
	
	// Setting local drives
	if (m_pDatabase->GetRoots()!=NULL)
	{
		CheckDlgButton(IDC_CUSTOMDRIVES,1);
		EnableDlgItem(IDC_FOLDERS,TRUE);
		EnableDlgItem(IDC_ADDFOLDER,TRUE);
		EnableDlgItem(IDC_REMOVEFOLDER,EnableRemoveButton());

		LPCSTR pPtr=m_pDatabase->GetRoots();

		while (*pPtr!='\0')
		{
			DWORD dwLength;
			dstrlen(pPtr,dwLength);

			if (dwLength>2)
			{
				if (pPtr[0]=='\\' && pPtr[1]=='\\')
				{
					if (!CFile::IsDirectory(pPtr))
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
				
				char root[]="X:\\";
				root[0]=pPtr[0];
				UINT uRet=GetDriveType(root);
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
		m_pList->SaveColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs","Database Dialog List Widths");
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
		ListNotifyHandler((LV_DISPINFO*)pnmh,(NMLISTVIEW*)pnmh);
		break;
	}
	return CDialog::OnNotify(idCtrl,pnmh);
}

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm)
{
	switch(pLvdi->hdr.code)
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
	char* pText=new char[iLength];
	GetDlgItemText(IDC_NAME,pText,iLength);
	
	if (!m_bDontEditName)
	{
		if (strncmp(pText,"DEFAULTX",8)==0 || strncmp(pText,"PARAMX",6)==0)
		{
			CString msg;
			msg.Format(IDS_INVALIDDBNAME,pText);
			MessageBox(msg,CString(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
			SetFocus(IDC_NAME);
			delete[] pText;
			return;
		}
		if (!CDatabase::IsNameValid(pText))
		{
			CString msg;
			msg.Format(IDS_INVALIDDBNAME,pText);
			MessageBox(msg,CString(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
			SetFocus(IDC_NAME);
			delete[] pText;
			return;
		}
		if (CDatabase::FindByName(m_aOtherDatabases,pText)!=NULL)
		{
			CString msg;
			msg.Format(IDS_NAMEALREADYEXISTS,pText);
			MessageBox(msg,CString(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
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
		CString msg;
		msg.Format(IDS_INVALIDFILENAME,"");
		MessageBox(msg,CString(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
		SetFocus(IDC_DBFILE);
		return;
	}
	pText=new char[iLength];
	GetDlgItemText(IDC_DBFILE,pText,iLength);
	m_pDatabase->SetArchiveNamePtr(CDatabase::GetCorrertFileName(pText));
	
	if (m_pDatabase->GetArchiveName()==NULL)
	{
		// Path was not ok, is this intended
		CString msg;
		msg.Format(IDS_INVALIDFILENAMEISOK,pText);
		if (MessageBox(msg,CString(IDS_DATABASESETTINGS),MB_YESNO|MB_ICONINFORMATION)==IDNO)
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
		CString msg;
		msg.Format(IDS_FILEALREADYEXISTS,m_pDatabase->GetArchiveName());
		MessageBox(msg,CString(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
		SetFocus(IDC_DBFILE);
		return;
	}
	
    // Setting creator
	iLength=GetDlgItemTextLength(IDC_CREATOR)+1;
	pText=new char[iLength];
	GetDlgItemText(IDC_CREATOR,pText,iLength);
	m_pDatabase->SetCreatorPtr(pText);

	// Setting description
	iLength=GetDlgItemTextLength(IDC_DESCRIPTION)+1;
	pText=new char[iLength];
	GetDlgItemText(IDC_DESCRIPTION,pText,iLength);
	m_pDatabase->SetDescriptionPtr(pText);

	// Settings flags
	m_pDatabase->Enable(IsDlgButtonChecked(IDC_ENABLE));
	m_pDatabase->UpdateGlobally(IsDlgButtonChecked(IDC_GLOBALUPDATE));
	m_pDatabase->SetFlag(CDatabase::flagStopIfRootUnavailable,IsDlgButtonChecked(IDC_STOPIFROOTUNAVAILABLE));
	m_pDatabase->SetFlag(CDatabase::flagIncrementalUpdate,IsDlgButtonChecked(IDC_INCREMENTALUPDATE));

	// Setting thread ID
	m_pDatabase->SetThreadId((WORD)SendDlgItemMessage(IDC_USEDTHREAD,CB_GETCURSEL));
    
	if (IsDlgButtonChecked(IDC_CUSTOMDRIVES))
	{
		CArrayFAP<LPSTR> aRoots;
		
		LVITEM li;
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
			char* szPath=new char[MAX_PATH];
			li.mask=LVIF_TEXT;
			li.iSubItem=1;
			li.pszText=szPath;
			li.cchTextMax=MAX_PATH;
			m_pList->GetItem(&li);
			DWORD nPathLen;
			dstrlen(szPath,nPathLen);
			for (int i=0;i<aRoots.GetSize();i++)
			{
				LPSTR pAddedPath=aRoots.GetAt(i);
				if (CFile::IsSubDirectory(szPath,pAddedPath))
				{
					CString str;
					str.Format(IDS_SUBFOLDER,szPath,pAddedPath,szPath);
					MessageBox(str,CString(IDS_DATABASESETTINGS),MB_ICONINFORMATION|MB_OK);
					bDoNotAdd=TRUE;
					break;
				}
				else if (CFile::IsSubDirectory(pAddedPath,szPath))
				{
					CString str;
					str.Format(IDS_SUBFOLDER,pAddedPath,szPath,pAddedPath);
					MessageBox(str,CString(IDS_DATABASESETTINGS),MB_ICONINFORMATION|MB_OK);
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
		CString Folder;
		if (!fd.GetFolder(Folder))
			AddComputerToList(fd.m_lpil);
		else
			AddDirectoryToListWithVerify(Folder);
		
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
	CString Temp,Title;
	Temp.LoadString(IDS_DATABASEFILTERS);
	GetDlgItemText(IDC_DBFILE,Title);
	CFileDialog fd(FALSE,"*",Title,OFN_EXPLORER|OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_ENABLESIZING,Temp);
	fd.EnableFeatures();
	Title.LoadString(IDS_SELECTDATABASE);
	fd.m_pofn->lpstrTitle=Title;
	
	// Ask file name
	if (!fd.DoModal(*this))
		return;

	// Check filename and set
	Temp=fd.GetPathName();
	int i=Temp.Find('*');
	if (i==-1)
		SetDlgItemText(IDC_DBFILE,Temp);
	else
	{
		if (Temp[i-1]=='.')
			i--;
		Title.Copy(Temp,i);
		SetDlgItemText(IDC_DBFILE,Title);
	}
	
	// Set focus to file name edit box
	SetFocus(IDC_DBFILE);
	SendDlgItemMessage(IDC_DBFILE,EM_SETSEL,0,-1);
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::OnExcludeDirectories()
{
	CExcludeDirectoryDialog edd(m_pDatabase->GetExcludedDirectories());
	
	if (edd.DoModal(*this))
		m_pDatabase->SetExcludedDirectories(edd.m_aDirectories);
}

int CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::AddDriveToList(LPSTR szDrive)
{
	DWORD nType=GetDriveType(szDrive);
	DebugFormatMessage("CSettingsProperties::CDatabaseSettingsPage::AddDriveToList(%s),type:%X",szDrive,nType);

	LVITEM li;
	li.iItem=m_pList->GetItemCount();

	CString Temp;
	char szLabel[20],szFileSystem[20];
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
	UINT nOldMode=SetErrorMode(SEM_FAILCRITICALERRORS);
	
	DWORD dwTemp;
	if (!GetVolumeInformation(szDrive,szLabel,20,&dwTemp,&dwTemp,&dwTemp,szFileSystem,20))
	{
		switch (nType)
		{
		case DRIVE_REMOVABLE:
			LoadString(IDS_FLOPPYSTRING,szLabel,20);
			break;
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

	// Resolving icon,
	SHFILEINFO fi;
	if (SHGetFileInfo(szDrive,0,&fi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME|SHGFI_SMALLICON|SHGFI_SYSICONINDEX))
		li.iImage=fi.iIcon;
	else
		li.iImage=DEL_IMAGE;
	// Label
	li.iSubItem=0;
	li.mask=LVIF_TEXT|LVIF_IMAGE;
	li.pszText=szLabel;
	m_pList->InsertItem(&li);

	// Path
	li.mask=LVIF_TEXT;
	li.iSubItem=1;
	szDrive[2]='\0';
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
		LPCSTR pPtr=m_pDatabase->GetRoots();

		while (*pPtr!='\0')
		{
			DWORD dwLength;
			dstrlen(pPtr,dwLength);

			// Check if selected
			if (strcasecmp(pPtr,szDrive)==0)
				m_pList->SetCheckState(li.iItem,TRUE);

			pPtr+=dwLength+1;
		}
	}
	
	return li.iItem;
}

int CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::AddDirectoryToListWithVerify(LPCSTR szFolder,int iLength)
{
	CString rFolder(szFolder,iLength);

	// Checks  wherther rPath is OK i.e. removes \ at end
	// Further, checks whether folder is already in list 
	// or folder is subfolder for already exists folder
	
		
	// No \ at end
	while (rFolder.LastChar()=='\\')
		rFolder.DelLastChar();

	LVITEM li;
	li.iItem=m_pList->GetNextItem(-1,LVNI_ALL);
	while (li.iItem!=-1)
	{
		char szPath[MAX_PATH];
		li.mask=LVIF_TEXT;
		li.iSubItem=1;
		li.pszText=szPath;
		li.cchTextMax=_MAX_PATH;
		m_pList->GetItem(&li);
		if (rFolder.CompareNoCase(szPath)==0)
		{
			CString str;
			str.Format(IDS_FOLDEREXIST,(LPCSTR)rFolder);
			MessageBox(str,CString(IDS_ADDFOLDER),MB_ICONINFORMATION|MB_OK);
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

		
int CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::AddDirectoryToList(LPCSTR szPath,int iLength)
{
	if (iLength==-1)
		dstrlen(szPath,iLength);
	
	char szLabel[20],szFileSystem[20];
	
	// Resolving label
	UINT nOldMode=SetErrorMode(SEM_FAILCRITICALERRORS);
	DWORD dwTemp;
	LVITEM li;
	li.iItem=m_pList->GetItemCount();

	CString Drive;
	if (szPath[1]==':')
		Drive << szPath[0] << ":\\";
	else
	{
		int nIndex=FirstCharIndex(szPath,'\\');
		if (nIndex==-1 || szPath[nIndex+1]!='\\')
			return -1;
		
		nIndex=NextCharIndex(szPath,'\\',nIndex+1);
		if (nIndex==-1)
		{
			Drive.Copy(szPath,iLength);
			Drive << '\\';
		}
		else
			Drive.Copy(szPath,nIndex+1);
	}
	if (!GetVolumeInformation(Drive,szLabel,20,&dwTemp,&dwTemp,&dwTemp,szFileSystem,20))
		szFileSystem[0]='\0';
	SetErrorMode(nOldMode);

	// Resolving icon,
	SHFILEINFO fi;
	if (SHGetFileInfo(szPath,0,&fi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME|SHGFI_SMALLICON|SHGFI_SYSICONINDEX))
	{
		
		// Label
		li.mask=LVIF_TEXT|LVIF_IMAGE;
		li.iImage=fi.iIcon;
		li.iSubItem=0;
		li.pszText=fi.szDisplayName;
		m_pList->InsertItem(&li);

		// Path
		li.mask=LVIF_TEXT;
		li.iSubItem=1;
		li.pszText=const_cast<LPSTR>(szPath);
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
	}
	else
	{
		// Label
		li.mask=LVIF_TEXT|LVIF_IMAGE;
		li.iImage=DEL_IMAGE;
		li.iSubItem=0;
		li.pszText=const_cast<LPSTR>(szEmpty);
		m_pList->InsertItem(&li);

		// Path
		li.mask=LVIF_TEXT;
		li.iSubItem=1;
		li.pszText=const_cast<LPSTR>(szPath);
		m_pList->SetItem(&li);
	}
	
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




int CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::AddComputerToList(LPITEMIDLIST lpiil)
{
	char szName[500];
	if (!GetDisplayNameFromIDList(lpiil,szName,500))
		return -1;


	// if szName does not start with "\\", name is not server
	if (szName[0]!='\\' && szName[1]!='\\')
	{
		CString str;
		if (szName[0]==':' && szName[1]==':')
			str.LoadString(IDS_ERRORCANNOTADDITEM);
		else
			str.Format(IDS_ERRORCANNOTADDITEM2,szName);
		MessageBox(str,CString(IDS_ERROR),MB_ICONERROR|MB_OK);
		return -1;
	}

	return AddComputerToList(szName);
}

int CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::AddComputerToList(LPCSTR szName)
{
	char szLabel[100];
	
	// Resolving label
	LVITEM li;
	li.iItem=m_pList->GetItemCount();

	// Resolving icon,
	SHFILEINFO fi;
	if (SHGetFileInfo(szName,0,&fi,sizeof(SHFILEINFO),SHGFI_DISPLAYNAME|SHGFI_SMALLICON|SHGFI_SYSICONINDEX))
		li.iImage=fi.iIcon;
	else
		return -1;

	// Setting data
	// Label
	li.iSubItem=0;
	li.mask=LVIF_TEXT|LVIF_IMAGE;
	li.pszText=fi.szDisplayName;
	m_pList->InsertItem(&li);

	// Path
	li.mask=LVIF_TEXT;
	li.iSubItem=1;
	li.pszText=const_cast<LPSTR>(szName);
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

	// Inserting strings
	for (int i=0;i<m_aDirectories.GetSize();i++)
		SendDlgItemMessage(IDC_DIRECTORIES,LB_ADDSTRING,0,LPARAM(m_aDirectories[i]));
	
	EnableControls();
	SetFocus(IDC_DIRECTORYNAME);


	return FALSE;
}

BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);
	switch(wID)
	{
	case IDOK:
		if (m_bTextChanged)
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
			m_bTextChanged=TRUE;
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
	
	m_aDirectories.RemoveAll();

	int nCount=SendDlgItemMessage(IDC_DIRECTORIES,LB_GETCOUNT);
	for (int i=0;i<nCount;i++)
	{
		int iLength=SendDlgItemMessage(IDC_DIRECTORIES,LB_GETTEXTLEN,i)+1;
		char* pText=new char[max(iLength,2)];
		SendDlgItemMessage(IDC_DIRECTORIES,LB_GETTEXT,i,LPARAM(pText));
		m_aDirectories.Add(pText);
	}

}
		
BOOL CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::OnAddFolder(BOOL bShowErrorMessageIfExists)
{
	m_bTextChanged=FALSE;

	CString sDirectory;
	int iLength=GetDlgItemTextLength(IDC_DIRECTORYNAME);
	char* pText=new char[iLength+1];
	GetDlgItemText(IDC_DIRECTORYNAME,pText,iLength+1);
	if (!GetFullPathName(pText,400,sDirectory.GetBuffer(400),NULL))
	{
		CString str;
		MessageBox(str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
		delete[] pText;
		return FALSE;	
	}
	delete[] pText;
	sDirectory.FreeExtra();


	if (!CFile::IsDirectory(sDirectory))
	{
		CString str;
		str.Format(IDS_ERRORDIRECTORYNOTFOUND,LPCSTR(sDirectory));
		MessageBox(str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
		return FALSE;
	}

	CString sLower(sDirectory);
	sLower.MakeLower();

	
	for (int i=SendDlgItemMessage(IDC_DIRECTORIES,LB_GETCOUNT)-1;i>=0;i--)
	{
		int iLength=SendDlgItemMessage(IDC_DIRECTORIES,LB_GETTEXTLEN,i);
        char* szText=new char[iLength+2];
		SendDlgItemMessage(IDC_DIRECTORIES,LB_GETTEXT,i,(LPARAM)szText);
		CharLower(szText);

		if (sLower.Compare(szText)==0)
		{
			if (bShowErrorMessageIfExists)
				ShowErrorMessage(IDS_ALREADYEXCLUDED,IDS_ERROR);
			return TRUE;
		}		
	}
    
	SendDlgItemMessage(IDC_DIRECTORIES,LB_ADDSTRING,0,(LPARAM)(LPCSTR)sDirectory);

	SetFocus(IDC_DIRECTORIES);
	EnableControls();
	return FALSE;
}

void CSettingsProperties::CDatabasesSettingsPage::CDatabaseDialog::CExcludeDirectoryDialog::OnRemove()
{
	int nSel=SendDlgItemMessage(IDC_DIRECTORIES,LB_GETCURSEL);
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
		CString Folder;
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
			
			
			nCurSel=m_pSchedules->GetCurSel();
			EnableDlgItem(IDC_EDIT,nCurSel!=-1);
			EnableDlgItem(IDC_DELETE,nCurSel!=-1);
			break;
		}
	case IDC_EDIT:
		{
			CCheduledUpdateDlg sud;
			int nCurSel=m_pSchedules->GetCurSel();
			if (nCurSel==-1)
				break;
			sud.m_pSchedule=(CSchedule*)m_pSchedules->GetItemData(nCurSel);
			sud.DoModal(*this);
			m_pSchedules->SetCurSel(nCurSel);
			break;
		}
	case IDC_UPDATES:
		if (wNotifyCode==LBN_SELCHANGE)
		{
			
			CString txt;
			int nCurSel=m_pSchedules->GetCurSel();
			if (nCurSel==-1)
			{
				EnableDlgItem(IDC_EDIT,FALSE);
				EnableDlgItem(IDC_DELETE,FALSE);
				break;
			}
			EnableDlgItem(IDC_EDIT,TRUE);
			EnableDlgItem(IDC_DELETE,TRUE);
			CSchedule* pSchedule=(CSchedule*)m_pSchedules->GetItemData(nCurSel);
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
	}
	return FALSE;
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
		if (!IsFullUnicodeSupport())
		{
            // Windows 9x
			CStringA str;
			pSchedule->GetString(str);
			dc.DrawText(str,&rc,DT_LEFT|DT_VCENTER);
		}
		else
		{
            // Windows NT
			CStringW str;
			pSchedule->GetString(str);
			dc.DrawText(str,&rc,DT_LEFT|DT_VCENTER);
		}

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
			if (!IsFullUnicodeSupport())
			{
				// Windows 9x
				CStringA str;
				((CSchedule*)lpMeasureItemStruct->itemData)->GetString(str);
				sz=dc.GetTextExtent(str);
			}
			else
			{
				// Windows NT
				CStringW str;
				((CSchedule*)lpMeasureItemStruct->itemData)->GetString(str);
				sz=dc.GetTextExtent(str);
			}
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
	m_pSettings->SetFlags(CSettingsProperties::settingsCancelled);

	CPropertyPage::OnCancel();
}


/////////////////////////////////////////////////
// CAutoUpdateSettingsPage::CCheduledUpdateDlg

BOOL CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::OnInitDialog(HWND hwndFocus)
{
	m_pCombo=new CComboBox(GetDlgItem(IDC_TYPE));
	CString txt;
	txt.LoadString(IDS_MINUTELY);
	m_pCombo->AddString(txt);
	txt.LoadString(IDS_HOURLY);
	m_pCombo->AddString(txt);
	txt.LoadString(IDS_DAILY);
	m_pCombo->AddString(txt);
	txt.LoadString(IDS_WEEKLY);
	m_pCombo->AddString(txt);
	txt.LoadString(IDS_MONTHLY);
	m_pCombo->AddString(txt);
	txt.LoadString(IDS_ONCE);
	m_pCombo->AddString(txt);
	txt.LoadString(IDS_ATSTARTUP);
	m_pCombo->AddString(txt);
	CComboBox MonthType(GetDlgItem(IDC_MTYPE));
	txt.LoadString(IDS_FIRST);
	MonthType.AddString(txt);
	txt.LoadString(IDS_SECOND);
	MonthType.AddString(txt);
	txt.LoadString(IDS_THIRD);
	MonthType.AddString(txt);
	txt.LoadString(IDS_FOURTH);
	MonthType.AddString(txt);
	txt.LoadString(IDS_LAST);
	MonthType.AddString(txt);
	CComboBox MDays(GetDlgItem(IDC_MDAYS));
	txt.LoadString(IDS_MONDAY);
	MDays.AddString(txt);
	txt.LoadString(IDS_TUESDAY);
	MDays.AddString(txt);
	txt.LoadString(IDS_WEDNESDAY);
	MDays.AddString(txt);
	txt.LoadString(IDS_THURSDAY);
	MDays.AddString(txt);
	txt.LoadString(IDS_FRIDAY);
	MDays.AddString(txt);
	txt.LoadString(IDS_SATURDAY);
	MDays.AddString(txt);
	txt.LoadString(IDS_SUNDAY);
	MDays.AddString(txt);
	SYSTEMTIME st;
	GetLocalTime(&st);
	st.wHour=m_pSchedule->m_tStartTime.bHour;
	st.wMinute=m_pSchedule->m_tStartTime.bMinute;
	st.wSecond=m_pSchedule->m_tStartTime.bSecond;
	CDateTimeCtrl TimeCtrl(GetDlgItem(IDC_TIME));
	TimeCtrl.SetSystemtime(GDT_VALID,&st);
	CSpinButtonCtrl spin1(GetDlgItem(IDC_SPIN)),spin2(GetDlgItem(IDC_MSPIN)),spin3(GetDlgItem(IDC_MINUTESPIN));
	spin1.SetRange(1,32000);
	spin2.SetRange(1,31);
	spin3.SetRange(0,59);
	spin1.SetBuddy(GetDlgItem(IDC_EVERY));
	spin2.SetBuddy(GetDlgItem(IDC_MEVERY));
	spin3.SetBuddy(GetDlgItem(IDC_MINUTEONHOUR));

	// Setting Every spin (and maybe other things)
	switch (m_pSchedule->m_nType)
	{
	case CSchedule::typeMinutely:
		spin1.SetPos(m_pSchedule->m_tMinutely.wEvery);
		SetDlgItemInt(IDC_EVERY,m_pSchedule->m_tMinutely.wEvery);
		break;
	case CSchedule::typeHourly:
		spin1.SetPos(m_pSchedule->m_tHourly.wEvery);
		SetDlgItemInt(IDC_EVERY,m_pSchedule->m_tHourly.wEvery);
		break;
	case CSchedule::typeDaily:
		spin1.SetPos(m_pSchedule->m_tDaily.wEvery);
		SetDlgItemInt(IDC_EVERY,m_pSchedule->m_tDaily.wEvery);
		break;
	case CSchedule::typeWeekly:
		spin1.SetPos(m_pSchedule->m_tWeekly.wEvery);
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
        spin1.SetPos(1);
		break;
	}

	if (m_pSchedule->m_nType==CSchedule::typeMonthly)
	{
		if (m_pSchedule->m_tMonthly.nType==CSchedule::SMONTHLYTYPE::Type::Day)
		{
			CheckDlgButton(IDC_MDAY,BST_CHECKED);
			OnCommand(IDC_MDAY,CBN_SELCHANGE,NULL);
			spin2.SetPos(m_pSchedule->m_tMonthly.bDay);
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
			spin2.SetPos(1);
		}	
	}
	else
	{
		CheckDlgButton(IDC_MDAY,BST_CHECKED);
		OnCommand(IDC_MDAY,CBN_SELCHANGE,NULL);
		spin2.SetPos(1);
		SendDlgItemMessage(IDC_MTYPE,CB_SETCURSEL,0,0);
		SendDlgItemMessage(IDC_MDAYS,CB_SETCURSEL,0,0);
	}
	
	if (m_pSchedule->m_nType==CSchedule::typeOnce)
	{
		GetLocalTime(&st);
		st.wYear=m_pSchedule->m_dStartDate.wYear;
		st.wMonth=m_pSchedule->m_dStartDate.bMonth;
		st.wDay=m_pSchedule->m_dStartDate.bDay;
		CDateTimeCtrl DateCtrl(GetDlgItem(IDC_ONCETIME));
		DateCtrl.SetSystemtime(GDT_VALID,&st);
	}
	
	// Setting minute spin
	if (m_pSchedule->m_nType==CSchedule::typeHourly)
	{
		spin3.SetPos(m_pSchedule->m_tHourly.wMinute);
		SetDlgItemInt(IDC_MINUTEONHOUR,m_pSchedule->m_tHourly.wMinute);
	}
	else
		spin3.SetPos(0);
		
	int nSel[]={2,3,4,5,6,1,0};

	m_pCombo->SetCurSel(nSel[m_pSchedule->m_nType]);
	OnTypeChanged();
	if (m_pSchedule->m_bFlags&CSchedule::flagEnabled)
		CheckDlgButton(IDC_ENABLED,BST_CHECKED);
	if (m_pSchedule->m_bFlags&CSchedule::flagDeleteAfterRun)
		CheckDlgButton(IDC_DELETEAFTERRUN,BST_CHECKED);
	if (m_pSchedule->m_bFlags&CSchedule::flagAtThisTime)
		CheckDlgButton(IDC_ATTHISTIME,BST_CHECKED);
	if (m_pSchedule->m_bFlags&CSchedule::flagRunned &&
		!(m_pSchedule->m_tLastStartTime.bHour==0 && m_pSchedule->m_tLastStartTime.bMinute==0 && m_pSchedule->m_tLastStartTime.bSecond==0 &&
		m_pSchedule->m_tLastStartDate.wYear<1995 && m_pSchedule->m_tLastStartDate.bMonth==0 && m_pSchedule->m_tLastStartDate.bDay==0))
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
		txt.Format(IDS_LASTRUN,szDate,szTime);
	}
	else
		txt.LoadString(IDS_LASTRUNNEVER);
	
#ifdef _DEBUG
	CString txt2;
	txt2.Format(" F:%X STD:%d.%d.%d STT: %d:%d:",m_pSchedule->m_bFlags,
		m_pSchedule->m_tLastStartDate.bDay,m_pSchedule->m_tLastStartDate.bMonth,m_pSchedule->m_tLastStartDate.wYear,
		m_pSchedule->m_tLastStartTime.bHour,m_pSchedule->m_tLastStartTime.bMinute,m_pSchedule->m_tLastStartTime.bSecond);
	txt << txt2;
#endif
	SetDlgItemText(IDC_LASTRUN,(LPCSTR)txt);
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
	case IDC_EVERY:
	case IDC_MEVERY:
	case IDC_MTYPE:
	case IDC_MDAYS:
	case IDC_ONCETIME:
	case IDC_ENABLED:
	case IDC_DELETEAFTERRUN:
		m_bChanged=TRUE;
		break;
	case IDC_DATABASES:
		OnDatabases();
		break;
	}
	return FALSE;
}

BOOL CSettingsProperties::CAutoUpdateSettingsPage::CCheduledUpdateDlg::OnDatabases()
{
	CArray<PDATABASE> aDatabases;
		
	CSelectDatabasesDlg dbd(GetLocateApp()->GetDatabases(),aDatabases,0,
		CString(IDS_REGPLACE,CommonResource)+"\\Dialogs\\SelectDatabases/Schedule");
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
			for (int i=0;i<aDatabases.GetSize();i++)
				dwLength+=istrlen(aDatabases[i]->GetName())+1;

			m_pSchedule->m_pDatabases=new char[dwLength];
			LPSTR pPtr=m_pSchedule->m_pDatabases;
			for (i=0;i<aDatabases.GetSize();i++)
			{
				int iStrlen=istrlen(aDatabases[i]->GetName())+1;
				CopyMemory(pPtr,aDatabases[i]->GetName(),iStrlen);
				pPtr+=iStrlen;
			}
			*pPtr='\0';
		}
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
	if (m_pCombo!=NULL)
	{
		delete m_pCombo;
		m_pCombo=NULL;
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
	CSchedule::ScheduleType nType[]={
		CSchedule::typeMinutely,
		CSchedule::typeHourly,
		CSchedule::typeDaily,
		CSchedule::typeWeekly,
		CSchedule::typeMonthly,
		CSchedule::typeOnce,
		CSchedule::typeAtStartup,
	};
	CSchedule::ScheduleType nNewType=nType[m_pCombo->GetCurSel()];
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
		m_pSchedule->m_tMinutely.wEvery=GetDlgItemInt(IDC_EVERY);
		if ((int)m_pSchedule->m_tMinutely.wEvery<1)
			m_pSchedule->m_tMinutely.wEvery=1;
		break;
	case CSchedule::typeHourly:
		m_pSchedule->m_tHourly.wEvery=GetDlgItemInt(IDC_EVERY);
		m_pSchedule->m_tHourly.wMinute=GetDlgItemInt(IDC_MINUTEONHOUR);
		if (m_pSchedule->m_tHourly.wMinute>59)
			m_pSchedule->m_tHourly.wMinute=59;
		if ((int)m_pSchedule->m_tHourly.wEvery<1)
			m_pSchedule->m_tHourly.wEvery=1;
		break;		
	case CSchedule::typeDaily:
		m_pSchedule->m_tDaily.wEvery=GetDlgItemInt(IDC_EVERY);
		if ((int)m_pSchedule->m_tDaily.wEvery<1)
			m_pSchedule->m_tDaily.wEvery=1;
		break;
	case CSchedule::typeWeekly:
		m_pSchedule->m_tWeekly.wEvery=GetDlgItemInt(IDC_EVERY);
		if ((int)m_pSchedule->m_tWeekly.wEvery<1)
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
			m_pSchedule->m_tMonthly.nType=CSchedule::SMONTHLYTYPE::Type::Day;
			m_pSchedule->m_tMonthly.bDay=GetDlgItemInt(IDC_MEVERY);
			if ((int)m_pSchedule->m_tMonthly.bDay<1)
				m_pSchedule->m_tMonthly.bDay=1;
		}
		else
		{
			m_pSchedule->m_tMonthly.nType=CSchedule::SMONTHLYTYPE::Type::WeekDay;
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
		
	CSchedule::ScheduleType nType=nTypes[m_pCombo->GetCurSel()];

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















///////////////////////////
// Class COptionsPropertyPage
///////////////////////////
#define IDC_EDITCONTROLFORSELECTEDITEM  1300
#define IDC_SPINCONTROLFORSELECTEDITEM  1301
#define IDC_COMBOCONTROLFORSELECTEDITEM 1302
#define IDC_COLORBUTTONFORSELECTEDITEM	1303
#define IDC_FONTBUTTONFORSELECTEDITEM	1304
//#define sMemCopyW	MemCopyW


void COptionsPropertyPage::Construct(const OPTIONPAGE* pOptionPage)
{
	CPropertyPage::Construct(pOptionPage->dwFlags&OPTIONPAGE::opTemplateIsID?
		MAKEINTRESOURCE(pOptionPage->nIDTemplate):pOptionPage->lpszTemplateName,
		pOptionPage->dwFlags&OPTIONPAGE::opCaptionIsID?pOptionPage->nIDCaption:0,
		LanguageSpecificResource);

	if (!(pOptionPage->dwFlags&OPTIONPAGE::opCaptionIsID))
	{
		m_psp.pszTitle=pOptionPage->lpszCaption;
		m_psp.dwFlags|=PSP_USETITLE;
	}
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

		
	}

	if (pItems==NULL)
		return FALSE;
	
	// Counting items
	for (int iItems=0;pItems[iItems]!=NULL;iItems++);
	
	m_pItems=new Item*[iItems+1];
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
	
	if (!IsFullUnicodeSupport())
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
        			
		if (!IsFullUnicodeSupport())
		{
			DWORD iStrLen;
			dstrlen(pItems[i]->pString,iStrLen);
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
			InsertItemsToTree(!IsFullUnicodeSupport()?tisa.hInsertAfter:tisw.hInsertAfter,pItems[i]->pChilds,pItems[i]);

		// Type specified actions
		switch (pItems[i]->nType)
		{
		case Item::RadioBox:
			if (pItems[i]->bChecked)
				hSelectedRadioItem=!IsFullUnicodeSupport()?tisa.hInsertAfter:tisw.hInsertAfter;
			// Continuing
		case Item::CheckBox:
			EnableChilds(!IsFullUnicodeSupport()?tisa.hInsertAfter:tisw.hInsertAfter,pItems[i]->bChecked);
			break;
		case Item::Edit:
            pItems[i]->hControl=CreateWindow("EDIT","",
				ES_AUTOHSCROLL|WS_TABSTOP|WS_CHILDWINDOW|WS_BORDER,
				10,10,100,13,*this,(HMENU)IDC_EDITCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
			::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);
			
			// Initializing
			if (pItems[i]->pProc!=NULL)
			{
				bp.crReason=BASICPARAMS::Initialize;
				bp.hControl=pItems[i]->hControl;
				pItems[i]->pProc(&bp);
			}
			if (pItems[i]->pData!=NULL)
				::SendMessage(pItems[i]->hControl,WM_SETTEXT,0,LPARAM(pItems[i]->pData));
			break;
		case Item::Numeric:
			{
				pItems[i]->hControl=CreateWindow("EDIT","",
					ES_AUTOHSCROLL|WS_TABSTOP|WS_CHILDWINDOW|WS_BORDER|ES_NUMBER,
					10,10,50,20,*this,(HMENU)IDC_EDITCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
				::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);
	
				// Initializing
				if (pItems[i]->pProc!=NULL)
				{
					bp.crReason=BASICPARAMS::Initialize;
					bp.hControl=pItems[i]->hControl;
					pItems[i]->pProc(&bp);
				}
				char szText[100];
				itoa(pItems[i]->lValue,szText,10);
				::SendMessage(pItems[i]->hControl,WM_SETTEXT,0,LPARAM(szText));
			}
			break;
		case Item::List:
			pItems[i]->hControl=CreateWindow("COMBOBOX","",
				CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP|WS_CHILDWINDOW|WS_BORDER,
				10,10,100,100,*this,(HMENU)IDC_COMBOCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
			::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

			// Initializing
			if (pItems[i]->pProc!=NULL)
			{
				bp.crReason=BASICPARAMS::Initialize;
				bp.hControl=pItems[i]->hControl;
				pItems[i]->pProc(&bp);
			}
			::SendMessage(pItems[i]->hControl,CB_SETCURSEL,pItems[i]->lValue,NULL);
		

			break;
		case Item::Combo:
			pItems[i]->hControl=CreateWindow("COMBOBOX","",
				CBS_DROPDOWN|WS_VSCROLL|WS_TABSTOP|WS_CHILDWINDOW|WS_BORDER,
				10,10,100,100,*this,(HMENU)IDC_COMBOCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
			::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

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
				int nFind=::SendMessage(pItems[i]->hControl,CB_FINDSTRINGEXACT,0,LPARAM(pItems[i]->pData));
				::SendMessage(pItems[i]->hControl,CB_SETCURSEL,nFind,0);
				if (nFind==CB_ERR)
					::SendMessage(pItems[i]->hControl,WM_SETTEXT,0,LPARAM(pItems[i]->pData));
			}
			break;
		case Item::Font:
			pItems[i]->hControl=CreateWindow("BUTTON",m_ChangeText,BS_PUSHBUTTON|WS_TABSTOP|WS_CHILDWINDOW,
				10,10,100,13,*this,(HMENU)IDC_FONTBUTTONFORSELECTEDITEM,GetInstanceHandle(),NULL);
			::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

			// Initializing
			if (pItems[i]->pProc!=NULL)
			{
				bp.crReason=BASICPARAMS::Initialize;
				bp.hControl=pItems[i]->hControl;
				pItems[i]->pProc(&bp);
			}
			break;
		case Item::Color:
			pItems[i]->hControl=CreateWindow("BUTTON",m_ChangeText,BS_PUSHBUTTON|WS_TABSTOP|WS_CHILDWINDOW,
				10,10,100,13,*this,(HMENU)IDC_COLORBUTTONFORSELECTEDITEM,GetInstanceHandle(),NULL);
			::SendMessage(pItems[i]->hControl,WM_SETFONT,SendMessage(WM_GETFONT),TRUE);

			// Initializing
			if (pItems[i]->pProc!=NULL)
			{
				bp.crReason=BASICPARAMS::Initialize;
				bp.hControl=pItems[i]->hControl;
				pItems[i]->pProc(&bp);
			}
			break;
			
		}
		
		// Setting text
		LPWSTR pCurText=pItems[i]->GetText();
		if (pCurText!=pItems[i]->pString)
		{
			if (!IsFullUnicodeSupport())
			{
				DWORD iStrLen;
				dstrlen(pCurText,iStrLen);
				tisa.itemex.pszText=new char[iStrLen+2];
				MemCopyWtoA(tisa.itemex.pszText,pCurText,iStrLen+1);
				m_pTree->SetItemText(tisa.hInsertAfter,tisa.itemex.pszText);
				delete[] tisa.itemex.pszText;
			}
			else
				m_pTree->SetItemText(tisw.hInsertAfter,pCurText);
			
		}


		
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
				else if (pItem->nType==Item::Edit)
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
				cd.DoModal(*this);
	
				SetColorValue(pItem,cd.GetColor());

				m_pTree->RedrawWindow();
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
                
				fd.DoModal(*this);
	
				SetFontValue(pItem,&fd.m_lf);

				WCHAR* pText=pItem->GetText(TRUE);
				m_pTree->SetItemText(hItem,pText);
				pItem->FreeText(pText);

				break;
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
					bp.nCurSel=::SendMessage(pItems[i]->hControl,CB_GETCURSEL,0,0);
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
	
BOOL COptionsPropertyPage::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
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
				RECT rcItem,rcOrig,rcTree,rcOther;
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


				if (rcOrig.left!=rcItem.right+1 || rcOrig.top!=rcItem.top-1)
				{
					// Moving control to correct place
					ScreenToClient(&rcItem);
					::SetWindowPos(pItem->hControl,HWND_TOP,rcItem.right+1,rcItem.top-1,0,0,
						(bNotVisible?SWP_HIDEWINDOW:SWP_SHOWWINDOW)|SWP_NOSIZE|SWP_NOACTIVATE);
					

					if (pItem->hControl2!=NULL)
					{
						::SetWindowPos(pItem->hControl2,HWND_TOP,rcItem.right+1+rcOther.left,rcItem.top-1+rcOther.top,0,0,
							(bNotVisible?SWP_HIDEWINDOW:SWP_SHOWWINDOW)|SWP_NOSIZE|SWP_NOACTIVATE);
						::InvalidateRect(pItem->hControl2,NULL,FALSE);
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
		BOOL bRet=TreeNotifyHandler((NMTVDISPINFO*)pnmh,(NMTREEVIEW*)pnmh);
		if (bRet)
			SetWindowLong(dwlMsgResult,bRet);
		return bRet;
	}			
	return CPropertyPage::OnNotify(idCtrl,pnmh);
}

BOOL COptionsPropertyPage::TreeNotifyHandler(NMTVDISPINFO *pTvdi,NMTREEVIEW *pNm)
{
	switch (pNm->hdr.code)
	{
	case TVN_KEYDOWN:
		if (((NMTVKEYDOWN*)pNm)->wVKey==VK_SPACE)
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
				if (pItem->bEnabled && (ht.flags&TVHT_ONITEMSTATEICON || pNm->hdr.code==NM_DBLCLK))
					SetCheckState(hItem,pItem,Toggle);
			}
			break;
		}
	case TVN_SELCHANGING:
		// Checking if selection cannot be vhanged
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
				if (!IsFullUnicodeSupport())
				{
					DWORD iStrLen;
					dstrlen(pText,iStrLen);
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
				if (!IsFullUnicodeSupport())
				{
					DWORD iStrLen;
					dstrlen(pText,iStrLen);
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
				}
				::SetWindowPos(pItem->hControl,HWND_TOP,0,0,nWidth,20,SWP_SHOWWINDOW|SWP_NOMOVE);


				
				if (pItem->nType==Item::Numeric)
				{
					// Creating Up/Down control
					pItem->hControl2=CreateWindow("msctls_updown32","",
						UDS_SETBUDDYINT|UDS_ALIGNRIGHT|UDS_ARROWKEYS|WS_TABSTOP|WS_CHILDWINDOW|WS_VISIBLE,
						rc.right+20,rc.top-1,10,10,*this,(HMENU)IDC_SPINCONTROLFORSELECTEDITEM,GetInstanceHandle(),NULL);
					::SendMessage(pItem->hControl2,UDM_SETBUDDY,WPARAM(pItem->hControl),NULL);
					
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

				PostMessage(WM_FOCUSSELITEMCONTROL);
			}
		}
		break;
	case TVN_SELCHANGED:
		/*if (pNm->itemNew.lParam!=NULL)
		{
			if (((Item*)pNm->itemNew.lParam)->hControl!=NULL)
				::SetFocus(((Item*)pNm->itemNew.lParam)->hControl);
		}*/
		break;
	case TVN_ITEMEXPANDING:
		if (pNm->action==TVE_COLLAPSE || pNm->action==TVE_TOGGLE)
			return TRUE;
		return FALSE;
	case NM_CUSTOMDRAW:
		{
			NMTVCUSTOMDRAW* pCustomDraw=(NMTVCUSTOMDRAW*)pNm;
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
	int iTextLen=::SendMessage(pItem->hControl,WM_GETTEXTLENGTH,0,0)+1;
	char* szText=new char[iTextLen];
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
		iCurSel=::SendMessage(pItem->hControl,CB_GETCURSEL,0,0);
		if (iCurSel!=CB_ERR)
		{
			iTextLen=::SendMessage(pItem->hControl,CB_GETLBTEXTLEN,iCurSel,0)+2;			
			cp.pNewData=new char[iTextLen];
			::SendMessage(pItem->hControl,CB_GETLBTEXT,iCurSel,LPARAM(cp.pNewData));
		}
		else
		{
			iTextLen=::SendMessage(pItem->hControl,WM_GETTEXTLENGTH,iCurSel,0)+2;			
			cp.pNewData=new char[iTextLen];
			::SendMessage(pItem->hControl,WM_GETTEXT,iTextLen,LPARAM(cp.pNewData));
		}

		break;
	default:
		iTextLen=::SendMessage(pItem->hControl,WM_GETTEXTLENGTH,0,0)+1;
		cp.pNewData=new char[iTextLen];
		::SendMessage(pItem->hControl,WM_GETTEXT,iTextLen,LPARAM(cp.pNewData));
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
	cp.lNewValue=::SendMessage(pItem->hControl,CB_GETCURSEL,0,0);
	
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
			_itow(lValue,szText,10);
			int iLength=istrlenw(szText)+1;
			int iLabelLen=istrlenw(pString);
			
			WCHAR* pText=new WCHAR[iLabelLen+iLength+2];
			sMemCopyW(pText,pString,iLabelLen);
			pText[iLabelLen++]=' ';
			sMemCopyW(pText+iLabelLen,szText,iLength);
			return pText;
		}
		return pString;
	case List:
    case Combo:
		if (hControl!=NULL && !bActive)
		{
			int nCurSel=::SendMessage(hControl,CB_GETCURSEL,0,0);
			int iLength=(nCurSel!=-1)?
				::SendMessage(hControl,CB_GETLBTEXTLEN,nCurSel,0)+1:
				::SendMessage(hControl,WM_GETTEXTLENGTH,0,0)+1;
			int iLabelLen=istrlenw(pString);
			
			WCHAR* pText=new WCHAR[iLabelLen+iLength+2];
			sMemCopyW(pText,pString,iLabelLen);
			pText[iLabelLen++]=' ';
				
			if (nCurSel!=-1)
			{
				char* pTemp=new char[iLength+2];
				::SendMessage(hControl,CB_GETLBTEXT,nCurSel,LPARAM(pTemp));
				MemCopyAtoW(pText+iLabelLen,pTemp,iLength+1);
				delete[] pTemp;
			}
			else
			{
				if (!IsFullUnicodeSupport())
				{
					// 9x
					char* pTemp=new char[iLength+2];
					::GetWindowText(hControl,pTemp,iLength);
					MemCopyAtoW(pText+iLabelLen,pTemp,iLength+1);
					delete[] pTemp;
				}
				else
					::GetWindowTextW(hControl,pText+iLabelLen,iLength);
			}

			return pText;
		}
		return pString;
	case Edit:
		if (hControl!=NULL && !bActive)
		{
			int iLength=::SendMessage(hControl,WM_GETTEXTLENGTH,0,0)+1;
			int iLabelLen=istrlenw(pString);
			
			WCHAR* pText=new WCHAR[iLabelLen+iLength+1];
			sMemCopyW(pText,pString,iLabelLen);
			pText[iLabelLen++]=' ';
				
			if (!IsFullUnicodeSupport())
			{
				// 9x
				char* pTemp=new char[iLength+2];
				::GetWindowText(hControl,pTemp,iLength);
				MemCopyAtoW(pText+iLabelLen,pTemp,iLength+1);
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
			str << ' ' << pLogFont->lfFaceName;

			if (pLogFont->lfHeight<0)
			{
				// Getting device caps
				HDC hScreenDC=::GetDC(NULL);
				int pt=MulDiv(-pLogFont->lfHeight, 72,::GetDeviceCaps(hScreenDC,LOGPIXELSY));
				::ReleaseDC(NULL,hScreenDC);
				
				str << ' ' << pt;
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
		pParams->pData=alloccopy(*(CString*)pParams->lParam);
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


		
////////////////////////////////////////
// CKeyboardShortcutsPage
////////////////////////////////////////


BOOL CSettingsProperties::CKeyboardShortcutsPage::OnInitDialog(HWND hwndFocus)
{
	CPropertyPage::OnInitDialog(hwndFocus);

	m_pList=new CListCtrl(GetDlgItem(IDC_KEYLIST));

	m_pList->InsertColumn(0,"Shortcut",LVCFMT_LEFT,130);
	m_pList->InsertColumn(1,"Action",LVCFMT_LEFT,150);
	m_pList->InsertColumn(2,"Global",LVCFMT_LEFT,80);
	
	m_pList->SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT ,LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT );
	m_pList->LoadColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs","Shortcuts Settings List Widths");

	m_ToolBarBitmaps.Create(IDB_SHORTCUTACTIONSBITMAPS,15,0,RGB(255,255,255));
	m_ToolBarBitmapsHot.Create(IDB_SHORTCUTACTIONSBITMAPSH,15,0,RGB(255,255,255));
	m_ToolBarBitmapsDisabled.Create(IDB_SHORTCUTACTIONSBITMAPSD,15,0,RGB(255,255,255));

	m_pToolBar=new CToolBarCtrl(GetDlgItem(IDC_ACTIONTOOLBAR));
	m_pToolBar->SetImageList(m_ToolBarBitmaps);
	m_pToolBar->SetDisabledImageList(m_ToolBarBitmapsDisabled);
	m_pToolBar->SetHotImageList(m_ToolBarBitmapsHot);
	
	TBBUTTON toolbuttons[]={
#pragma warning (disable :4305 4309)
		{0,IDC_ADD,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0},
		{1,IDC_REMOVE,0,TBSTYLE_BUTTON,0,0,0,0},
		{2,IDC_NEXT,0,TBSTYLE_BUTTON,0,0,0,0},
		{3,IDC_PREV,0,TBSTYLE_BUTTON,0,0,0,0},
		{4,IDC_SWAPWITHPREVIOUS,0,TBSTYLE_BUTTON,0,0,0,0},
		{5,IDC_SWAPWITHNEXT,TBSTATE_ENABLED,TBSTYLE_BUTTON,0,0,0,0}
#pragma warning (default :4305 4309)
	};
	m_pToolBar->AddButtons(6,toolbuttons);

	m_pToolBar->SetWindowPos(HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
	return FALSE;
}

BOOL CSettingsProperties::CKeyboardShortcutsPage::OnApply()
{
	CPropertyPage::OnApply();

	
	return TRUE;
}

void CSettingsProperties::CKeyboardShortcutsPage::OnDestroy()
{
	CPropertyPage::OnDestroy();

	if (m_pList!=NULL)
	{
		m_pList->SaveColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs","Shortcuts Settings List Widths");
		delete m_pList;
		m_pList=NULL;
	}

	if (m_pToolBar!=NULL)
	{
		delete m_pToolBar;
		m_pToolBar=NULL;
	}

}
		
void CSettingsProperties::CKeyboardShortcutsPage::OnCancel()
{
	m_pSettings->SetFlags(CSettingsProperties::settingsCancelled);

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
	switch (idCtrl)
	{
	case IDC_SHORTCUTKEY:
		ListNotifyHandler((LV_DISPINFO*)pnmh,(NMLISTVIEW*)pnmh);
		break;
	default:
		if (pnmh->code==TTN_NEEDTEXT)
		{
			switch (pnmh->idFrom)
			{
			case IDC_ADD:
				((LPTOOLTIPTEXT)pnmh)->lpszText="Add new action";
				break;
			case IDC_REMOVE:
				((LPTOOLTIPTEXT)pnmh)->lpszText="Remove action";
				break;
			case IDC_NEXT:
				((LPTOOLTIPTEXT)pnmh)->lpszText="Next action";
				break;
			case IDC_PREV:
				((LPTOOLTIPTEXT)pnmh)->lpszText="Previous action";
				break;
			case IDC_SWAPWITHPREVIOUS:
				((LPTOOLTIPTEXT)pnmh)->lpszText="Swap with previous";
				break;
			case IDC_SWAPWITHNEXT:
				((LPTOOLTIPTEXT)pnmh)->lpszText="Swap with next";
				break;
			}
		}
	}
	return CPropertyPage::OnNotify(idCtrl,pnmh);
}


BOOL CSettingsProperties::CKeyboardShortcutsPage::ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm)
{
	/*switch(pLvdi->hdr.code)
	{
	case LVN_DELETEITEM:
		delete (LanguageItem*)pNm->lParam;
		break;
	case LVN_GETDISPINFO:
		{
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
	*/
	return TRUE;
}

