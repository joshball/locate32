#if !defined(SETTINGSDLG_H)
#define SETTINGSDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif 





class CSettingsProperties : public CPropertySheet  
{
public:
	CSettingsProperties(HWND hParent);
	virtual ~CSettingsProperties();

public:
	BOOL LoadSettings();
	BOOL SaveSettings();
	CList<CSchedule*>* GetSchedules() { return &m_Schedules; }

	void SetFlags(DWORD dwFlags,BOOL bState);
	void SetFlags(DWORD dwFlags);
	void ClearFlags(DWORD dwFlags);
	BOOL IsFlagSet(DWORD dwFlags);
	BOOL IsAllFlagsSet(DWORD dwFlags);
    
public:
	class CGeneralSettingsPage : public CPropertyPage 
	{
	public:
		CGeneralSettingsPage();
		virtual BOOL OnInitDialog(HWND hwndFocus);
		virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
		virtual BOOL OnApply();
		virtual void OnCancel();
				
	protected:
		BYTE OnSystemSettings();
		BYTE OnPointToSelect();
		BYTE OnClickToSelect();
		BYTE OnNeverUnderline();
		BYTE OnPointUnderline();
		BYTE OnAlwaysUnderline();
		
	protected:
		friend CSettingsProperties;
		CSettingsProperties* m_pSettings;
	

#ifdef _DEBUG
	public:
		inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
		inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
		inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }

#endif
	};
	
	class CAdvancedSettingsPage : public COptionsPropertyPage 
	{
	public:
		CAdvancedSettingsPage();
		virtual BOOL OnInitDialog(HWND hwndFocus);
		virtual void OnCancel();
			
	protected:
		friend CSettingsProperties;
		CSettingsProperties* m_pSettings;

		static BOOL CALLBACK TimeFormatComboProc(COptionsPropertyPage::BASICPARAMS* pParams);
		static BOOL CALLBACK DateFormatComboProc(COptionsPropertyPage::BASICPARAMS* pParams);
		static BOOL CALLBACK UpdateThreadPriorityProc(COptionsPropertyPage::BASICPARAMS* pParams);
		static BOOL CALLBACK FileSizeListProc(COptionsPropertyPage::BASICPARAMS* pParams);
		static BOOL CALLBACK LimitResultsCheckBoxProc(COptionsPropertyPage::BASICPARAMS* pParams); 
		static BOOL CALLBACK UpdateTooltipPositionProc(COptionsPropertyPage::BASICPARAMS* pParams);
		static BOOL CALLBACK UpdateTooltipTopmostProc(COptionsPropertyPage::BASICPARAMS* pParams);
		

		static BOOL CALLBACK EnumDateFormatsProc(LPTSTR lpDateFormatString);
		static BOOL CALLBACK EnumTimeFormatsProc(LPTSTR lpDateFormatString);

		CArrayFAP<LPSTR> m_aBuffer;

#ifdef _DEBUG
	public:
		inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
		inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
		inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
	};
	
	class CLanguageSettingsPage : public CPropertyPage 
	{
	public:
		CLanguageSettingsPage();
		virtual BOOL OnInitDialog(HWND hwndFocus);
		virtual BOOL OnApply();
		virtual void OnCancel();
		virtual void OnDestroy();
		virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
		virtual void OnTimer(DWORD wTimerID); 
			
		BOOL ListNotifyHandler(NMLISTVIEW *pNm);
				
	protected:
		friend CSettingsProperties;
		CSettingsProperties* m_pSettings;

		void FindLanguages();
        
		struct LanguageItem {
			CStringW Language;
			CStringW File;
			CStringW Description;
		};


#ifdef _DEBUG
	public:
		inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
		inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
		inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
	private:
		CListCtrl* m_pList;
		int nLastSel;
	};

	class CDatabasesSettingsPage : public CPropertyPage 
	{
	public:
		CDatabasesSettingsPage();
		
		virtual BOOL OnInitDialog(HWND hwndFocus);
		virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
		virtual void OnDestroy();
		virtual BOOL OnApply();
		virtual void OnCancel();
		virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
			
	protected:
        BOOL ListNotifyHandler(NMLISTVIEW *pNm);
		static int CALLBACK ThreadSortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

		void OnNew(CDatabase* pDatabaseTempl=NULL);
		void OnEdit();
		void OnRemove();
		void OnEnable(BOOL nEnable);
		void OnUpdate();
		void OnRestore();
		void OnThreads();
		void OnImport();
		void OnExport();

		void SetDatabasesToList();
		

		BOOL ItemUpOrDown(BOOL bUp);		
		BOOL IncreaseThread(int nItem,CDatabase* pDatabase,BOOL bDecrease=FALSE);

		void EnableThreadGroups(int nThreadGroups);
		void RemoveThreadGroups();
		void ChangeNumberOfThreads(int nThreads);
	
		void EnableButtons();

		
		class CDatabaseDialog : public CDialog
		{
		public:
			CDatabaseDialog();

			virtual BOOL OnInitDialog(HWND hwndFocus);
			virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
			virtual void OnDestroy();
			virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
			virtual BOOL OnClose();
			
			BOOL ListNotifyHandler(NMLISTVIEW *pNm);
		
		protected:
			
			void OnOK();
			void OnBrowse();
			void OnAddFolder();
			void OnRemoveFolder();
			void OnExcludeDirectories();
			
			BOOL EnableRemoveButton();
			int AddDriveToList(LPWSTR szDrive);
			int AddDirectoryToListWithVerify(LPCWSTR szPath,SIZE_T iLength=-1);
			int AddDirectoryToList(LPCWSTR szPath,SIZE_T iLength=-1);
			int AddComputerToList(LPCWSTR szName);
			
		public:
			BOOL m_bDontEditName;
			
			CDatabase* m_pDatabase;
			int m_nMaximumNumbersOfThreads;

			// For checking whether name or database file already exist
			CArray<CDatabase*> m_aOtherDatabases; 

		protected:
			CListCtrl* m_pList;

			class CExcludeDirectoryDialog : public CDialog
			{
			public:
				CExcludeDirectoryDialog();
				CExcludeDirectoryDialog(const CArrayFAP<LPWSTR>& rDirectories);

				virtual BOOL OnInitDialog(HWND hwndFocus);
				virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
				virtual BOOL OnClose();
				
				void OnOK();
				BOOL OnAddFolder(BOOL bShowErrorMessageIfExists);
				void OnRemove();
				void OnBrowse();

				void EnableControls();

			public:
				CArrayFAP<LPWSTR> m_aDirectories;

				BOOL m_bTextChanged;
			};

		};

		

	protected:
		
		friend CSettingsProperties;
		friend class CSelectDatabasesDlg;

		CSettingsProperties* m_pSettings;
		CListCtrl* m_pList;
		int m_nThreadsCurrently;

	};

	class CAutoUpdateSettingsPage : public CPropertyPage 
	{
	public:
		CAutoUpdateSettingsPage();
		virtual BOOL OnInitDialog(HWND hwndFocus);
		virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
		virtual void OnDrawItem(UINT idCtl,LPDRAWITEMSTRUCT lpdis);
		virtual void OnMeasureItem(int nIDCtl,LPMEASUREITEMSTRUCT lpMeasureItemStruct);
		virtual void OnDestroy();
		virtual BOOL OnApply();
		virtual void OnCancel();

		void EnableItems();
				
	public:
		class CCheduledUpdateDlg : public CDialog
		{
		public:
			CCheduledUpdateDlg();

		public:
			virtual BOOL OnInitDialog(HWND hwndFocus);
			virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
			virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
			virtual void OnDestroy();
			virtual BOOL OnClose();

		private:
			BOOL OnOK();
			BOOL OnTypeChanged();
			BOOL OnDatabases();

		public:
			CSchedule* m_pSchedule;
		
		private:
			CComboBox* m_pCombo;
			BYTE m_bChanged;

#ifdef _DEBUG
		public:
			inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
			inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
			inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
		};

	protected:
		friend CSettingsProperties;
		CSettingsProperties* m_pSettings;
		CListBox* m_pSchedules;

	};

	class CKeyboardShortcutsPage : public CPropertyPage 
	{
	protected:
		class CAdvancedDlg: public CDialog  
		{
		public:
			CAdvancedDlg(CShortcut* pShortcut);

			virtual BOOL OnInitDialog(HWND hwndFocus);
			virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
			virtual BOOL OnClose();
			
			void OnOK();
			void EnableItems();

			CShortcut* m_pShortcut;

		};



	public:
		CKeyboardShortcutsPage();
		virtual ~CKeyboardShortcutsPage();

		virtual BOOL OnInitDialog(HWND hwndFocus);
		virtual BOOL OnApply();
		virtual void OnCancel();
		virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
		virtual void OnDestroy();
		virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
		virtual void OnTimer(DWORD wTimerID); 
			
		BOOL ListNotifyHandler(NMLISTVIEW *pNm);
		BOOL WherePressedNotifyHandler(NMLISTVIEW *pNm);
		BOOL ItemUpOrDown(BOOL bUp);

		void InsertSubActions();
		void InsertKeysToVirtualKeyCombo();
		void RefreshShortcutListLabels();
		void SetShortcutKeyWhenVirtualKeyChanged();
		void SetVirtualKeyWhenShortcutKeyChanged();
		
		
		void OnNewShortcut();
		void OnRemoveShortcut();
		void OnResetToDefaults();
		void OnAdvanced();
		void OnAddAction();
		void OnRemoveAction();
		void OnNextAction(BOOL bNext);
		void OnSwapAction(BOOL bWithNext);
		
		void OnChangeItem(NMLISTVIEW *pNm);
		void OnChangingItem(NMLISTVIEW *pNm);

		CAction::Action GetSelectedAction() const;
		void SetFieldsForShortcut(CShortcut* pShortcut);
		void SaveFieldsForShortcut(CShortcut* pShortcut);
		void SetFieldsForAction(CAction* pAction);
		void SaveFieldsForAction(CAction* pAction);
		void ClearActionFields();
		void EnableItems();
		void SetFieldsRelativeToMnemonics();
		void InsertShortcuts();


		
		void FormatActionLabel(CStringW& str,CAction::Action nAction,UINT uSubAction) const;
		BOOL GetSubActionLabel(CStringW& str,CAction::Action nAction,UINT uSubAction) const;
		UINT IndexToSubAction(CAction::Action nAction,UINT nIndex) const;
		UINT SubActionToIndex(CAction::Action nAction,UINT nSubAction) const;

		void SetVirtualCode(BYTE bCode,BOOL bScanCode);
		BYTE GetVirtualCode(BOOL bScanCode) const;
		void SetHotKeyForShortcut(CShortcut* pShortcut);
		void GetHotKeyForShortcut(CShortcut* pShortcut) const;
		
		void SetHotKey(BYTE bKey,BYTE bModifiers);
		




	protected:
		friend CSettingsProperties;
		CSettingsProperties* m_pSettings;

		
#ifdef _DEBUG
	public:
		inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
		inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
		inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
	private:
		CListCtrl* m_pList,*m_pWhenPressedList;
		CToolBarCtrl* m_pToolBar;
		CImageList m_ToolBarBitmaps;
		CImageList m_ToolBarBitmapsDisabled;
		CImageList m_ToolBarBitmapsHot;

		CShortcut* m_pCurrentShortcut;
		int m_nCurrentAction;
		
		CAction::ActionActivateControls* m_pPossibleControls;
		CAction::ActionMenuCommands* m_pPossibleMenuCommands;
		CShortcut::VirtualKeyName* m_pVirtualKeyNames;

		CComboBox m_ActionCombo,m_SubActionCombo,m_VerbCombo,m_WhichFileCombo;

		HWND hDialogs[5];	
		HMENU hMainMenu;
		HMENU hPopupMenu;



	};

public:
	//General
	DWORD m_dwLocateDialogFlags;  // Used with CLocateDlg::LocateDialogFlags
	DWORD m_dwLocateDialogExtraFlags;  // Used with CLocateDlg::LocateDialogExtraFlags
	DWORD m_dwProgramFlags; // Used with CLocateAppWnd::ProgramFlags
	
	DWORD m_nMaximumFoundFiles;
	DWORD m_nNumberOfNames; // Directoried in NameDlg 
	DWORD m_nNumberOfTypes; // Directoried in NameDlg 
	DWORD m_nNumberOfDirectories; // Directoried in NameDlg 
	int  m_nUpdateThreadPriority;
	
	LOGFONT m_lResultListFont;
	

	CStringW m_TimeFormat;
	CStringW m_DateFormat;
	CLocateApp::FileSizeFormats m_nFileSizeFormat;
	
	CStringW m_OpenFoldersWith;
	
 	// Database
	CArrayFP<PDATABASE> m_aDatabases;
	
	// Shortcuts
	CArrayFP <CShortcut*> m_aShortcuts;

	// Schedules
	CListFP <CSchedule*> m_Schedules;
	

	// General settings flags	
	enum SettingsFlags {
		settingsCancelled=0x0001,
		settingsUseLanguageWithConsoleApps=0x0002,
		settingsIsUsedDatabaseChanged=0x0004,
		settingsUseOtherProgramsToOpenFolders=0x0008,
		settingsDatabasesOverridden=0x0010,
		settingsStartLocateAtStartup=0x0020,
		settingsSetTooltipDelays=0x0040,
		settingsUseCustomResultListFont=0x0080,
		settingsDontShowExtensionInRenameDialog=0x0100,
				
		settingsDefault=settingsUseLanguageWithConsoleApps
	};


	// For m_bAdvancedAndContextMenuFlag
	enum AdvancedAndContextMenuFlags {
		cmLocateOnMyComputer=0x1,
		cmLocateOnMyDocuments=0x2,
		cmLocateOnDrives=0x4,
		cmLocateOnFolders=0x8,
		cmUpdateOnMyComputer=0x10
	};

	// For m_bDefaultFlag
	enum DefaultFlags {
		defaultFileNames=0,
		defaultFileAndFolderNames=1,
		defaultFolderNames=2,
		defaultCheckInFlag=0x3,
		defaultWholeName=0x4,
		defaultMatchCase=0x8,
		defaultReplaceSpaces=0x10,
		defaultTextIsCaseSensetive=0x20,
		defaultUseWholePath=0x40,

		defaultDefault=defaultMatchCase|defaultFileAndFolderNames
	};
	DWORD m_dwSettingsFlags;
	BYTE m_bAdvancedAndContextMenuFlag;
	BYTE m_bDefaultFlag;
	BYTE m_bSorting;

	CString m_strLangFile;
	DWORD m_nTransparency;
    
	// Update tooltip
	DWORD m_nToolTipTransparency;
    COLORREF m_cToolTipTextColor;
    COLORREF m_cToolTipTitleColor;
    COLORREF m_cToolTipErrorColor;
	COLORREF m_cToolTipBackColor;
	LOGFONT m_lToolTipTextFont;
	LOGFONT m_lToolTipTitleFont;

	DWORD m_dwTooltipDelayInitial;
	DWORD m_dwTooltipDelayAutopop;
	

private:
	CGeneralSettingsPage* m_pGeneral;
	CAdvancedSettingsPage* m_pAdvanced;
	CLanguageSettingsPage* m_pLanguage;
	CDatabasesSettingsPage* m_pDatabases;
	CAutoUpdateSettingsPage* m_pAutoUpdate;
	CKeyboardShortcutsPage* m_pKeyboardShortcuts;

	friend CGeneralSettingsPage;

#ifdef _DEBUG
public:
	inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
};



#endif 
