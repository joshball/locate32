#if !defined(SETTINGSDLG_H)
#define SETTINGSDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif 

#define WM_REDRAWSELITEMCONTROL		WM_APP+100
#define WM_FOCUSSELITEMCONTROL		WM_APP+101

class COptionsPropertyPage : public CPropertyPage
{
public:
	struct Item;

	enum NewState {
		Unchecked,
		Checked,
		Toggle 
	};
	// Callback systtem
	struct BASICPARAMS {
		enum Reason {
			Initialize,
			SetSpinRange,
			Get,
			Set,
			Apply,
			ChangingValue
		} crReason;

		// Input
		DWORD wParam;
		void* lParam;
		COptionsPropertyPage* pPage;

		// Output
		union {
			BOOL bChecked;
			LPSTR pData;
			LONG lValue;
		};
	};

	struct INITIALIZEPARAMS : public BASICPARAMS {
		HWND hControl;
	};
	struct COMBOAPPLYPARAMS : public BASICPARAMS {
		LONG nCurSel;
	};
	struct SPINPOXPARAMS : public BASICPARAMS {
		int iLow;
		int iHigh;
	};
	struct CHANGINGVALPARAMS : public BASICPARAMS {
		union {
			NewState nNewState;
			LPSTR pNewData;
			LONG lNewValue;
		};
	};

	typedef BOOL (CALLBACK* CALLBACKPROC)(BASICPARAMS* pParams);

	// lParam is pointer to DWORD value which is will be set
	// wParam is used mask
	static BOOL CALLBACK DefaultCheckBoxProc(BASICPARAMS* pParams); 
	
	// lParam is pointer to DWORD value which is will be set
	// HIWORD of wParam is mask to be setted, LOWORD is value
	static BOOL CALLBACK DefaultRadioBoxProc(BASICPARAMS* pParams); 

	// lParam is pointer to DWORD value which is will be set
	// HIWORD of wParam is mask (shifted 16 bits) to be setted, LOWORD is value (shifted 16 bit)
	static BOOL CALLBACK DefaultRadioBoxShiftProc(BASICPARAMS* pParams); 

	// lParam is pointer to DWORD value which is will be set
	// if wParam==0, all values are accepted
	// if wParam==-1, only nonnegative values are accepted
	// otherwise HIWORD is maximum, LOWORD is minimum
	static BOOL CALLBACK DefaultNumericProc(BASICPARAMS* pParams); 

	// lParam is pointer to string class which will be set
	static BOOL CALLBACK DefaultEditStrProc(BASICPARAMS* pParams); 


public:
	// Item class
	struct Item {
	private:
		BOOL bEnabled;

		// Visualization
		enum ItemType {
			Root,
			CheckBox,
			RadioBox,
			Edit,
			Combo,
			List,
			Numeric
		} nType;
		Item* pParent;
		Item** pChilds; // NULL terminated array
		LPWSTR pString;

		HWND hControl; // Control associated for item
		HWND hControl2; // Another control associated for item

		// Data
		union {
			BOOL bChecked;
			LPSTR pData;
			LONG lValue;
		};

		// Callback
		CALLBACKPROC pProc;
		DWORD wParam;
		void* lParam;


	private:
		Item(ItemType nType,Item* pParent,Item** pChilds,LPWSTR pString,
			CALLBACKPROC pProc,DWORD wParam,void* lParam);
		Item(ItemType nType,Item* pParent,Item** pChilds,UINT nStringID,
			CALLBACKPROC pProc,DWORD wParam,void* lParam);
		~Item();

		int GetStateImage() const;
		
		void SetValuesForBasicParams(COptionsPropertyPage::BASICPARAMS* pParams);
		void GetValuesFromBasicParams(const COptionsPropertyPage::BASICPARAMS* pParams);

		LPWSTR GetText(BOOL bActive=FALSE) const;
		void FreeText(LPWSTR pText) const;

		friend COptionsPropertyPage;
	};
	
	
public:
	COptionsPropertyPage(UINT nIDTemplate,UINT nIDCaption,UINT nTreeCtrlID);
	COptionsPropertyPage(LPCTSTR lpszTemplateName,UINT nIDCaption,UINT nTreeCtrlID);


	virtual BOOL OnApply();
	virtual void OnDestroy();
	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual void OnActivate(WORD fActive,BOOL fMinimized,HWND hwnd);
	//virtual void OnTimer(DWORD wTimerID); 
	
	virtual BOOL WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);

	BOOL Initialize(Item** pItems);
	
	static Item* CreateRoot(LPWSTR szText,Item** pChilds);
	static Item* CreateRoot(UINT nTextID,Item** pChilds);
	static Item* CreateCheckBox(LPWSTR szText,Item** pChilds,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateCheckBox(UINT nTextID,Item** pChilds,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateRadioBox(LPWSTR szText,Item** pChilds,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateRadioBox(UINT nTextID,Item** pChilds,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateEdit(LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateEdit(UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateListBox(LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateListBox(UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateComboBox(LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateComboBox(UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateNumeric(LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateNumeric(UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam);

private:
    BOOL InsertItemsToTree(HTREEITEM hParent,Item** pItems,Item* pParent=NULL);
	BOOL TreeNotifyHandler(NMTVDISPINFO *pTvdi,NMTREEVIEW *pNm);
	BOOL SetCheckState(HTREEITEM hItem,Item* pItem,NewState nNewState);
	BOOL SetNumericValue(Item* pItem);
	BOOL SetTextValue(Item* pItem);
	BOOL SetListValue(Item* pItem);
	void EnableChilds(HTREEITEM hItem,BOOL bEnable);
	void UncheckOtherRadioButtons(HTREEITEM hItem,HTREEITEM hParent);
	void CallApply(Item** pItems);
	
private:
	CTreeCtrl* m_pTree;
	CImageList m_Images;
	Item** m_pItems;
	UINT m_nTreeID;
		
};

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

		void OnNew();
		void OnEdit();
		void OnRemove();
		void OnEnable(BOOL nEnable);
		void OnUpdate();
		void OnRestore();
		void OnThreads();
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
