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
		BYTE OnSingleClick();
		BYTE OnDoubleClick();
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
		static BOOL CALLBACK LimitResultsCheckBoxProc(COptionsPropertyPage::BASICPARAMS* pParams); 
	

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
			
		BOOL ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm);
				
	protected:
		friend CSettingsProperties;
		CSettingsProperties* m_pSettings;

		void FindLanguages();
        
		struct LanguageItem {
			CString Language;
			CString File;
			CString Description;
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
        BOOL ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm);
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
			
			BOOL ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm);
		
		protected:
			
			void OnOK();
			void OnBrowse();
			void OnAddFolder();
			void OnRemoveFolder();
			void OnExcludeDirectories();
			
			BOOL EnableRemoveButton();
			int AddDriveToList(LPSTR szDrive);
			int AddDirectoryToListWithVerify(LPCSTR szPath,int iLength=-1);
			int AddDirectoryToList(LPCSTR szPath,int iLength=-1);
			int AddComputerToList(LPCSTR szName);
			int AddComputerToList(LPITEMIDLIST lpiil);

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
				CExcludeDirectoryDialog(const CArrayFAP<LPSTR>& rDirectories);

				virtual BOOL OnInitDialog(HWND hwndFocus);
				virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
				virtual BOOL OnClose();
				
				void OnOK();
				BOOL OnAddFolder(BOOL bShowErrorMessageIfExists);
				void OnRemove();
				void OnBrowse();

				void EnableControls();

			public:
				CArrayFAP<LPSTR> m_aDirectories;

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

public:
	//General
	DWORD m_dwLocateDialogFlags;  // Used with CLocateDlg::LocateDialogFlags
	DWORD m_dwLocateDialogExtraFlags;  // Used with CLocateDlg::LocateDialogExtraFlags
	DWORD m_dwProgramFlags; // Used with CLocateAppWnd::ProgramFlags
	
	DWORD m_nMaximumFoundFiles;
	DWORD m_nNumberOfDirectories; // Directoried in NameDlg 
	
	CString m_TimeFormat;
	CString m_DateFormat;
	
	CString m_OpenFoldersWith;
	
 	//Database
	CArrayFP<PDATABASE> m_aDatabases;
	
	// General settings flags	
	enum SettingsFlags {
		settingsCancelled=0x1,
		
		settingsUseLanguageWithConsoleApps=0x2,
		
		settingsStartLocateAtStartup=0x20,
		settingsUseOtherProgramsToOpenFolders=0x8,
		
		settingsIsUsedDatabaseChanged=0x4,
		settingsDatabasesOverridden=0x10,
		
		
		settingsDefault=settingsUseLanguageWithConsoleApps
	};


	// For m_bAdvancedAndContextMenuFlag
	enum AdvancedAndContextMenuFlags {
		cmLocateOnMyComputer=0x1,
		cmLocateOnMyDocuments=0x2,
		cmLocateOnDrives=0x4,
		cmLocateOnFolders=0x8,
		cmUpdateOnMyComputer=0x10,
		hookExplorer=0x80
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

		defaultDefault=defaultMatchCase|defaultFileAndFolderNames
	};
	DWORD m_dwSettingsFlags;
	BYTE m_bAdvancedAndContextMenuFlag;
	BYTE m_bDefaultFlag;
	
	CString m_strLangFile;
	
	CListFP <CSchedule*> m_Schedules;

private:
	CGeneralSettingsPage* m_pGeneral;
	CAdvancedSettingsPage* m_pAdvanced;
	CLanguageSettingsPage* m_pLanguage;
	CDatabasesSettingsPage* m_pDatabases;
	CAutoUpdateSettingsPage* m_pAutoUpdate;

	friend CGeneralSettingsPage;

#ifdef _DEBUG
public:
	inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
};



#endif 
