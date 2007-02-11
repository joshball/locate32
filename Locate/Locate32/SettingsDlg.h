#if !defined(SETTINGSDLG_H)
#define SETTINGSDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif 

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
			ChangingValue,
			BrowseFile
		} crReason;

		// Input
		DWORD wParam;
		void* lParam;
		COptionsPropertyPage* pPage;

		// Output
		union {
			BOOL bChecked;
			LPWSTR pData;
			LONG lValue;
			COLORREF cColor;
			LOGFONT* pLogFont;
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
			LPWSTR pNewData;
			LONG lNewValue;
			COLORREF cNewColor;
			LOGFONT* pNewLogFont;
		};
	};
	struct BROWSEDLGPARAMS : BASICPARAMS {
		LPWSTR szTitle; // Can be identiefier to resource
		LPWSTR szFilters; // Can be identiefier to resource
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

	// lParam is pointer to CStringW which will be set
	static BOOL CALLBACK DefaultEditStrWProc(BASICPARAMS* pParams); 

	// lParam is pointer to COLORREF which will be set
	static BOOL CALLBACK DefaultColorProc(BASICPARAMS* pParams); 

	// lParam is pointer to LOGFONT which will be set
	static BOOL CALLBACK DefaultFontProc(BASICPARAMS* pParams); 


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
			Numeric,
			Color,
			Font,
			File
		} nType;
		Item* pParent;
		Item** pChilds; // NULL terminated array
		LPWSTR pString;

		HWND hControl; // Control associated for item
		HWND hControl2; // Another control associated for item

		// Data
		union {
			BOOL bChecked;
			LPWSTR pData;
			LONG lValue;
			COLORREF cColor;
			LOGFONT* pLogFont;
		};

		// Callback
		CALLBACKPROC pProc;
		DWORD wParam;
		void* lParam;

		mutable int m_nStateIcon;

	private:
		Item(ItemType nType,Item* pParent,Item** pChilds,LPWSTR pString,
			CALLBACKPROC pProc,DWORD wParam,void* lParam);
		Item(ItemType nType,Item* pParent,Item** pChilds,UINT nStringID,
			CALLBACKPROC pProc,DWORD wParam,void* lParam);
		~Item();

		int GetStateImage(CImageList* pImageList) const;
		
		void SetValuesForBasicParams(COptionsPropertyPage::BASICPARAMS* pParams);
		void GetValuesFromBasicParams(const COptionsPropertyPage::BASICPARAMS* pParams);

		LPWSTR GetText(BOOL bActive=FALSE) const;
		void FreeText(LPWSTR pText) const;

		int IconFromColor(CImageList* pImageList,int nReplace=-1) const;
		
		friend COptionsPropertyPage;


	};
	
	
public:

	struct OPTIONPAGE
	{
		union {
			UINT nIDTemplate;
			LPCWSTR lpszTemplateName;
        };
		union {
			UINT nIDCaption;
			LPCWSTR lpszCaption;
		};
		union {
			UINT nIDChangeText;
			LPCWSTR lpszChangeText;
		};
		UINT nTreeCtrlID;

		enum OptionPageFlags {
			opTemplateIsID=0x1,
			opCaptionIsID=0x2,
			opChangeIsID=0x4
		};
		DWORD dwFlags;
	};



	COptionsPropertyPage();
	COptionsPropertyPage(const COptionsPropertyPage::OPTIONPAGE* pOptionPage,TypeOfResourceHandle bType=LanguageSpecificResource);
	
	void Construct(const OPTIONPAGE* pOptionPage,TypeOfResourceHandle bType=LanguageSpecificResource);
	

	virtual BOOL OnApply();
	virtual void OnDestroy();
	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual void OnActivate(WORD fActive,BOOL fMinimized,HWND hwnd);
	//virtual void OnTimer(DWORD wTimerID); 
	
	virtual LRESULT WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);

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
	static Item* CreateColor(LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateColor(UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateFont(LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateFont(UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateFile(LPWSTR szText,CALLBACKPROC pProc,DWORD wParam,void* lParam);
	static Item* CreateFile(UINT nTextID,CALLBACKPROC pProc,DWORD wParam,void* lParam);

private:
    BOOL InsertItemsToTree(HTREEITEM hParent,Item** pItems,Item* pParent=NULL);
	BOOL TreeNotifyHandler(NMTVDISPINFO *pTvdi);
	BOOL SetCheckState(HTREEITEM hItem,Item* pItem,NewState nNewState);
	BOOL SetNumericValue(Item* pItem);
	BOOL SetTextValue(Item* pItem);
	BOOL SetListValue(Item* pItem);
	BOOL SetColorValue(Item* pItem,COLORREF cNewColor);
	BOOL SetFontValue(Item* pItem,LOGFONT* pLogFont);
	void EnableChilds(HTREEITEM hItem,BOOL bEnable);
	void UncheckOtherRadioButtons(HTREEITEM hItem,HTREEITEM hParent);
	void CallApply(Item** pItems);
	
	struct UserData {
		WNDPROC pOldWndProc;
		COptionsPropertyPage* pDialog;
	};

	static LRESULT CALLBACK TreeSubClassFunc(HWND hWnd,UINT uMessage,
		WPARAM wParam,LPARAM lParam);
	static LRESULT CALLBACK ButtonSubClassFunc(HWND hWnd,UINT uMessage,
		WPARAM wParam,LPARAM lParam);
	static LRESULT CALLBACK EditSubClassFunc(HWND hWnd,UINT uMessage,
		WPARAM wParam,LPARAM lParam);
	static LRESULT CALLBACK ComboSubClassFunc(HWND hWnd,UINT uMessage,
		WPARAM wParam,LPARAM lParam);
	

protected:
	CTreeCtrl* m_pTree;
	mutable CImageList m_Images;
	Item** m_pItems;
	UINT m_nTreeID;
	CStringW m_ChangeText;

	
		
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

	void SetSettingsFlags(DWORD dwFlags,BOOL bState);
	void SetSettingsFlags(DWORD dwFlags);
	void ClearSettingsFlags(DWORD dwFlags);
	BOOL IsSettingsFlagSet(DWORD dwFlags);
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
		static BOOL CALLBACK ExternalCommandProc(BASICPARAMS* pParams); 
		static BOOL CALLBACK TrayIconProc(BASICPARAMS* pParams); 


		static BOOL CALLBACK EnumDateFormatsProc(LPTSTR lpDateFormatString);
		static BOOL CALLBACK EnumTimeFormatsProc(LPTSTR lpDateFormatString);

		CArrayFAP<LPSTR> m_aBuffer;

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
			int AddDirectoryToListWithVerify(LPCWSTR szPath,INT iLength=-1);
			int AddDirectoryToList(LPCWSTR szPath,int iLength=-1);
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
				CExcludeDirectoryDialog(LPCWSTR szFiles,const CArrayFAP<LPWSTR>& rDirectories);

				virtual BOOL OnInitDialog(HWND hwndFocus);
				virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
				virtual BOOL OnClose();
				
				void OnOK();
				BOOL OnAddFolder(BOOL bShowErrorMessageIfExists);
				void OnRemove();
				void OnBrowse();

				void EnableControls();

			public:
				CStringW m_sFiles;
				CArrayFAP<LPWSTR> m_aDirectories;
				
				BOOL m_bDirectoryFieldChanged;
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
		void OnEdit();

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
			CComboBox* m_pTypeCombo;
			BYTE m_bChanged;


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

		

	private:
		CListCtrl* m_pList,*m_pWhenPressedList;
		CToolBarCtrl* m_pToolBar;
		CImageList m_ToolBarBitmaps;
		CImageList m_ToolBarBitmapsDisabled;
		CImageList m_ToolBarBitmapsHot;

		CShortcut* m_pCurrentShortcut;
		int m_nCurrentAction;
		
		CAction::ActionControls* m_pPossibleControlsToActivate;
		CAction::ActionControls* m_pPossibleControlsToChange;
		CAction::ActionMenuCommands* m_pPossibleMenuCommands;
		CShortcut::VirtualKeyName* m_pVirtualKeyNames;
		CArrayFAP<LPWSTR> m_aPossiblePresets;

		
		CComboBox m_ActionCombo,m_SubActionCombo,m_VerbCombo,m_WhichFileCombo;

		HWND hDialogs[5];
		BOOL bFreeDialogs;
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
	CStringW m_CustomTrayIcon;
	
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
		settingsCustomUseTrayIcon = 0x0200,
				
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

	CStringW m_strLangFile;
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


};



#endif 
