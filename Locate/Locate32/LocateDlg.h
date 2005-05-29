#if !defined(LOCATEDLG_H)
#define LOCATEDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif 

// This should be in SmallDialogs.h, but SmallDialog.h needs LocateDlg.h to be specified
class CSavePresetDlg: public CDialog  
{
public:
	CSavePresetDlg();

	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	
	virtual void OnOK();
	virtual void OnCancel();

public:
	CString m_sReturnedPreset;
};


class CLocateDlgThread : public CWinThread  
{
public:
	CLocateDlgThread();
	
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	virtual BOOL OnThreadMessage(MSG* pMsg);


public:
	CLocateDlg* m_pLocate;

#ifdef _DEBUG
public:
	inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
};


extern CBufferAllocator<BYTE*,2000,BUFFERALLOC_EXTRALEN> FileTypeAllocator;

class CLocateDlg : public CDialog  
{
public:
	// If more than 64 details, make m_nSorting bigger
	enum DetailType {
		Title=0,
		InFolder=1,
		FullPath=2,
		ShortFileName=3,
		ShortFilePath=4,
		FileSize=5,
		FileType=6,
		DateModified=7,
		DateCreated=8,
		DateAccessed=9,
		Attributes=10,
		ImageDimensions=11,
		Owner=12,
		Database=13,
		DatabaseDescription=14,
		DatabaseArchive=15,
		VolumeLabel=16,
		VolumeSerial=17,
		VOlumeFileSystem=18,
		MD5sum=19,
		LastType=19,


		Needed=255
	};
	
public:
	class CNameDlg : public CDialog  
	{
	public:
		enum TypeOfItem
		{
			Everywhere=0,
			Special=1,
			Drive=2,
			Custom=3,
			Root=4,
			NotSelected=0xFFFF // This is used to indicate that any 
							   // item in LookIn combo is not selected
		};

		enum SpecialType
		{
			Documents=1,
			Desktop=2,
			Personal=3,
			MyComputer=4
		};

		enum EveryWhereType
		{
			Original=0,
			RootTitle=1
		};
	private:
		class DirSelection {
		public:
			DirSelection(BYTE bSelected);
			~DirSelection();

			void FreeData();
			void SetValuesFromControl(HWND hControl,const CString* pBrowse,int nBrowseDirs);
			LPARAM GetLParam(const CString* pBrowse,int nBrowseDirs) const;
			

			CLocateDlg::CNameDlg::TypeOfItem nType;
			union {
				SpecialType nSpecialType;
				EveryWhereType nEverywhereType;
				char cDriveLetter;
			};
			LPSTR pTitleOrDirectory;
			BYTE bSelected:1;
		};


	public:
			
		CNameDlg();
		
		virtual BOOL OnInitDialog(HWND hwndFocus);
		virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
		virtual void OnDestroy();
		virtual void OnSize(UINT nType, int cx, int cy);	
		
	public:
		BOOL OnOk(CString& sName,CArray<LPSTR>& aExtensions,CArrayFAP<LPSTR>& aDirectories);
		void OnClear();

		void OnMoreDirectories();
		void OnLookInNewSelection();
		void OnLookInRemoveSelection();
		void OnLookInSelection(int nSelection);
		void OnLookInNextSelection(BOOL bNext);
		void LookInChangeSelection(int nCurrentSelection,int nNewSelection);



		BOOL SetPath(LPCTSTR szPath);

		void EnableItems(BOOL bEnable=TRUE);
		BOOL InitDriveBox(BYTE nFirstTime=FALSE);
		void SetStartData(const CLocateApp::CStartData* pStartData);
		void ChangeNumberOfDirectories(int iNumberOfDirectories);
		
		BOOL EnableMultiDirectorySupport(BOOL bEnable);
		BOOL SelectByLParam(LPARAM lParam);
		BOOL SelectByRootName(LPCSTR szDirectory);
		BOOL SelectByCustomName(LPCSTR szDirectory);
		DWORD GetCurrentlySelectedComboItem(CComboBoxEx& LookIn) const;

		BOOL GetDirectoriesForActiveSelection(CArray<LPSTR>& aDirectories,TypeOfItem* pType=NULL,BOOL bNoWarningIfNotExists=FALSE);
		BOOL GetDirectoriesFromCustomText(CArray<LPSTR>& aDirectories,LPCSTR szCustomText,DWORD dwLength,BOOL bCurrentSelection,BOOL bNoWarningIfNotExists=FALSE);
		BOOL GetDirectoriesFromLParam(CArray<LPSTR>& aDirectories,LPARAM lParam);
		BOOL GetDirectoriesForSelection(CArray<LPSTR>& aDirectories,const DirSelection* pSelection,BOOL bNoWarnings=FALSE);

		static void AddDirectoryToList(CArray<LPSTR>& aDirectories,LPCSTR szDirectory);
		static void AddDirectoryToList(CArray<LPSTR>& aDirectories,LPCSTR szDirectory,DWORD dwLength);
		static void AddDirectoryToList(CArray<LPSTR>& aDirectories,FREEDATA szDirectory);
		
		BOOL CheckAndAddDirectory(LPCSTR pFolder,DWORD dwLength,BOOL bAlsoSet=TRUE,BOOL bNoWarning=FALSE);

		static WORD ComputeChecksumFromDir(LPCSTR szDir);

		void LoadControlStates(CRegKey& RegKey);
		void SaveControlStates(CRegKey& RegKey);

	
	protected:
		
		void OnBrowse();
		void SaveRegistry() const;
		void LoadRegistry();

	public:
		CString* m_pBrowse;
		int m_nMaxBrowse;

		WORD m_nFieldLeft;
		WORD m_nButtonWidth;
		WORD m_nBrowseTop;
		WORD m_nMoreDirsTop;
		BYTE m_nMoreDirsWidth;
		
#ifdef _DEBUG
	public:
		inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
		inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
		inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif

	
		DirSelection** m_pMultiDirs;

	};

	class CSizeDateDlg : public CDialog  
	{
	public:
		CSizeDateDlg();
		
		virtual BOOL OnInitDialog(HWND hwndFocus);
		virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
		
	public:
		BOOL OnOk(CLocater* pLocater);
		void OnClear();
		
		void EnableItems(BOOL bEnable);
		BOOL LookOnlyFiles() const;
		void SetStartData(const CLocateApp::CStartData* pStartData);

		void LoadControlStates(CRegKey& RegKey);
		void SaveControlStates(CRegKey& RegKey);

#ifdef _DEBUG
	public:
		inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
		inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
		inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
	};

	class CAdvancedDlg : public CDialog  
	{
	public:
		CAdvancedDlg();
		
		virtual BOOL OnInitDialog(HWND hwndFocus);
		virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
		virtual void OnSize(UINT nType, int cx, int cy);	
		virtual void OnDrawItem(UINT idCtl,LPDRAWITEMSTRUCT lpdis);
		virtual void OnMeasureItem(int nIDCtl,LPMEASUREITEMSTRUCT lpMeasureItemStruct);
		virtual void OnDestroy();
		virtual BOOL WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);
		
	public:
		// Return codes for OnOk
		enum {
			flagMatchCase=0x1,
			flagReplaceSpaces=0x2,
			flagNameIsRegularExpression=0x4, // These are not returned, 
			flagUseWholePath=0x8
		};
		DWORD OnOk(CLocater* pLocater);
		void OnClear();
		
		void EnableItems(BOOL bEnable);

		void ChangeEnableStateForCheck();
		void UpdateTypeList();
		void AddBuildInFileTypes();
		void ReArrangeAllocatedData();
		void SetStartData(const CLocateApp::CStartData* pStartData);

		static DWORD WINAPI UpdaterProc(CAdvancedDlg* pAdvancedDlg);
	
		void LoadControlStates(CRegKey& RegKey);
		void SaveControlStates(CRegKey& RegKey);

	private:
		struct FileType
		{
			char* szExtensions;
			char* szTitle;
			char* szType;
			DWORD dwExtensionLength;
			
			HICON hIcon;
			LPSTR szIconPath;
			

			FileType();
			FileType(FREEDATA frType,FREEDATA frTitle);
			FileType(LPCSTR& szBuildIn,HIMAGELIST hImageList);
			~FileType();
			
			void AddExtension(LPCSTR szExtension,DWORD dwExtensionLength);
			void SetIcon(CRegKey& rKey,BOOL toHandle=FALSE);
			void ExtractIconFromPath();

		private:
			
		public:
			inline void* operator new(size_t size) { return FileTypeAllocator.AllocateFast(size); }
			inline void operator delete(void* pObject) { FileTypeAllocator.Free(pObject); }
			inline void operator delete(void* pObject,size_t size) { FileTypeAllocator.Free(pObject); }
		};

		int AddTypeToList(LPCSTR szKey,DWORD dwKeyLength,CArray<FileType*>& aFileTypes);
		int AddTypeToList(LPCSTR pTypeAndExtensions);
		
	private:
		HANDLE m_hTypeUpdaterThread;
		HICON m_hDefaultTypeIcon;
		CImageList m_ToolbarIL,m_ToolbarILHover,m_ToolbarILDisabled;

		friend FileType;
		friend CLocateDlg;

		WORD m_nMatchCaseWidth;
		WORD m_nMatchCaseTop;
		WORD m_nCheckPos;
		
		enum {
			fgBuildInTypesAdded=0x1,
			fgOtherTypeAdded=0x2
		};
		BYTE m_dwFlags;

#ifdef _DEBUG
	public:
		inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
		inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
		inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
	};

private:
	class CSavePresetDlg: public ::CSavePresetDlg  
	{
	public:
		virtual void OnOK();
	};

	struct ContextMenuStuff {
		ContextMenuStuff();
		~ContextMenuStuff();
		

        IContextMenu* pContextMenu;
		IContextMenu2* pContextMenu2;
		IContextMenu3* pContextMenu3;
		IShellFolder* pParentFolder;

		LPITEMIDLIST pParentIDList;
		LPITEMIDLIST* apidl;
		BYTE** apcidl;
		int nIDlistCount;

	};

	
public:

	CLocateDlg();
	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();
	virtual void OnDestroy();
	virtual void OnSize(UINT nType, int cx, int cy);	
	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
	virtual void OnChangeCbChain(HWND hWndRemove,HWND hWndAfter );
	virtual void OnDrawClipboard();
	virtual void OnInitMenuPopup(HMENU hPopupMenu,UINT nIndex,BOOL bSysMenu);
	virtual void OnDrawItem(UINT idCtl,LPDRAWITEMSTRUCT lpdis);
	virtual void OnTimer(DWORD wTimerID); 
	virtual void OnContextMenu(HWND hWnd,CPoint& pos);
	virtual void OnMeasureItem(int nIDCtl,LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual BOOL WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);
	
	void OnInitMainMenu(HMENU hPopupMenu,UINT nIndex);
	void OnInitFileMenu(HMENU hPopupMenu);
	void OnInitSendToMenu(HMENU hPopupMenu);

	void SortItems(DetailType nColumn,BYTE bDescending=-1); // bDescending:0=ascending order, 1=desc, -1=default

	static UINT AddSendToMenuItems(HMENU hMenu,CString& sSendToPath,UINT wStartID);
	static void FreeSendToMenuItems(HMENU hMenu);
	static BOOL IsSendToMenu(HMENU hMenu);
	static BOOL InsertMenuItemsFromTemplate(HMENU hMenu,HMENU hTemplate,UINT uStartPosition,int nDefaultItem=-1);
	
	//void EnsureFocus();
	BOOL ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm);
	static int CALLBACK ListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int SortNewItem(CListCtrl* pList,CLocatedItem* pNewItem,BYTE bSorting);
	void SetSortArrowToHeader(DetailType nColumn,BOOL bRemove,BOOL bDownArrow);
	HMENU CreateFileContextMenu(HMENU hFileMenu);
	ContextMenuStuff* GetContextMenuForFiles(LPCSTR szParent,CArrayFP<CString*>& aFiles);
	


	void ClearMenuVariables();     
	static int GetSendToMenuPos(HMENU hMenu);
	BOOL CheckClipboard();
	void EnableItems();
	void DisableItems();
	void ResetFileNotificators();
	void StartBackgroundOperations();
	void StopBackgroundOperations();
	void ChangeBackgroundOperationsPriority(BOOL bLower);

	BOOL GetFileClassID(CString& file,CLSID& clsid,LPCSTR szType); 
	BOOL SendFiles(CString& dst,CListCtrl* pList,CLSID& clsid);
	
	BYTE BeginDragFiles(CListCtrl* pList);
	BOOL ResolveSystemLVStatus();
	BOOL SetListSelStyle();
	BOOL UpdateSettings();
	void SetStartData(const CLocateApp::CStartData* pStartData);
	BOOL StartLocateAnimation();
	BOOL StopLocateAnimation();
	BOOL StartUpdateAnimation();
	BOOL StopUpdateAnimation();

	void LoadDialogTexts();
	void SaveDialogTexts();
	void SetControlPositions(UINT nType,int cx, int cy);

	void OpenFolder(LPCSTR szFolder);

	void OnActivateTab(int nIndex);

protected:
	void OnOk(BOOL bForceSelectDatabases=FALSE);
	void OnStop();
	void OnNewSearch();
	void OnMenuCommands(WORD wID);
	void OnSendToCommand(WORD wID);
	void OnAutoArrange();
	void OnAlignToGrid();
	void OnRefresh();
	void OnSettings() { GetLocateAppWnd()->OnSettings(); }
	void OnProperties();
	void OnCopy(BOOL bCut);
	void OnOpenContainingFolder();
	void OnCreateShortcut();
	void OnDelete();
	void OnRemoveFromThisList();
	void OnSelectAll();
	void OnInvertSelection();
	void OnSaveResults();
	void OnSelectDetails();
	void OnCopyPathToClipboard(BOOL bShortPath);
	void OnChangeFileName();
	void OnChangeFileNameCase();
	void OnUpdateLocatedItem();
	void OnComputeMD5Sums(BOOL bForSameSizeFilesOnly);
	void OnShowFileInformation();

	BOOL SetListStyle(int id,BOOL bInit=FALSE);
	void SetVisibleWindowInTab();
	void SaveRegistry();
	void LoadRegistry();

	void SetDialogMode(BOOL bLarge);
	void RemoveResultsFromList();

	void DeleteTooltipTools();


	void OnPresets();
	void OnPresetsSave();
	void OnPresetsSelection(int nPreset);
	static DWORD CheckExistenceOfPreset(LPCSTR szName,DWORD* pdwPresets); // Returns index to preset or FFFFFFFF

	

	static BOOL CALLBACK LocateProc(DWORD dwParam,CallingReason crReason,UpdateError ueCode,DWORD dwFoundFiles,const CLocater* pLocater);
	static BOOL CALLBACK LocateFoundProc(DWORD dwParam,BOOL bFolder,const CLocater* pLocater);

public:
	static void SetSystemImagelists(CListCtrl* pList,HICON* phIcon=NULL);
	
public:
	
	struct ImageHandlerDll {
		ImageHandlerDll();
		~ImageHandlerDll();
		
		BOOL IsLoaded();
		HMODULE hModule;
		IH_GETIMAGEDIMENSIONSA pGetImageDimensionsA;
		ULONG uToken;
	};
	ImageHandlerDll* m_pImageHandler;


	// 32 bit set for settings
	enum LocateDialogFlags {
		// Dialog
		fgLargeMode=0x00000001,
		fgDialogRememberFields=0x0000002,
		fgDialogMinimizeToST=0x00000004,
		fgDialogLeaveLocateBackground=0x00000008,
		fgDialogLargeModeOnly=0x40000000,
		fgDialogTopMost=0x80000000,
		fgDialogFlag=0xC000000F,
		fgDialogSave=0xC000000E, // mask to using when saving to registry
				
		// File list
		fgLVShowIcons=0x00000010,
		fgLVShowFileTypes=0x00000020,
		fgLVShowShellType=0x00000080,
		fgLVDontShowHiddenFiles=0x00000040,
		fgLVDontShowTooltips=0x00010000,
		fgLVNoDoubleItems=0x00020000,
		fgLVComputeMD5Sums=0x00040000,
		fgLVShowFlag=0x000700F0,
		
		fgLVUseGetFileTitle=0x00000000,
		fgLVUseOwnMethod=0x00000100,
		fgLVMethodFlag=0x00000100,

		fgLVAlwaysShowExtensions=0x00000000,
		fgLVHideKnownExtensions=0x00000200,
		fgLVNeverShowExtensions=0x00000400,
		fgLV1stCharUpper=0x00000800,
		fgLVExtensionFlag=fgLVAlwaysShowExtensions|fgLVHideKnownExtensions|fgLVNeverShowExtensions,
		
		fgLVStyleSystemDefine=0x00001000,
		fgLVStyleSingleClick=0x00002000,
		fgLVStyleDoubleClick=0x00000000,
		fgLVStyleClickFlag=0x00003000,
		fgLVStyleNeverUnderline=0x00000000,
		fgLVStyleUnderLine=0x00004000,
		fgLVStyleAlwaysUnderline=0x00008000|fgLVStyleUnderLine,
		fgLVStyleUnderlineFlag=0x00008000|fgLVStyleUnderLine,
		fgLVStyleFlag=0x0000F000,
		fgLVFlag=0x0007FFF0,
		fgLVSave=0x0007FFF0,
			
		// Name tab
		fgNameMultibleDirectories=0x04000000,
		fgNameDontAddRoots=0x00000000,
		fgNameAddEnabledRoots=0x01000000,
		fgNameAddAllRoots=0x02000000,
		fgNameRootFlag=0x03000000,
		fgNameFlag=0x0F000000,
		fgNameSave=0x0F000000,

		// Time/date/size tab
		fgTimeDateSizeFlag=0x00000000,
		fgTimeDateSizeSave=0x00000000,

		// Advanced tab
		fgLoadRegistryTypes=0x10000000,
		fgAdvancedFlag=0x10000000,
		fgAdvancedSave=0x10000000,

		// other
		fgOtherFlag=0x00000000,
		fgOtherSave=0x00000000,

		fgDefault=fgLVStyleSystemDefine|fgLVUseOwnMethod|fgLVHideKnownExtensions|fgLV1stCharUpper|
			fgLVShowIcons|fgLVShowFileTypes|fgLoadRegistryTypes,
		fgSave=fgDialogSave|fgLVSave|fgNameSave|fgTimeDateSizeSave|fgAdvancedSave|fgOtherSave		// mask to using when saving to registry
	};

	// Another 32 set for settings
	enum LocateDialogExtraFlags {
		// Background operations
		efDisableItemUpdating = 0x00,
        efEnableItemUpdating = 0x01,
		efItemUpdatingMask = 0x01,
		efItemUpdatingSave = 0x01,
		
		efDisableFSTracking = 0x00,
		efEnableFSTracking  = 0x02,
		efEnableFSTrackingOld = 0x04|efEnableFSTracking,
		efTrackingMask = efEnableFSTracking|efEnableFSTrackingOld,
		efTrackingSave = efEnableFSTracking|efEnableFSTrackingOld,
		
		efBackgroundDefault = efEnableItemUpdating|efEnableFSTracking,
		efBackgroundSave = efItemUpdatingSave|efTrackingSave,

		efDefault = efBackgroundDefault,
		efSave = efBackgroundSave
	};


public:
	CTabCtrl* m_pTabCtrl;
	CListCtrlEx* m_pListCtrl;
	CStatusBarCtrl* m_pStatusCtrl;
	CNameDlg m_NameDlg;
	CSizeDateDlg m_SizeDateDlg;
	CAdvancedDlg m_AdvancedDlg;

	CToolTipCtrl* m_pListTooltips;
	int m_iTooltipItem;
	int m_iTooltipSubItem;
	BOOL m_bTooltipActive;
	
protected:
	DWORD m_dwFlags;
	DWORD m_dwExtraFlags;

	
	ContextMenuStuff* m_pActiveContextMenu;
	
	CMenu m_Menu;
	HFONT m_hSendToListFont;

	CBitmap m_CircleBitmap;
	HICON* m_pLocateAnimBitmaps;
	HICON* m_pUpdateAnimBitmaps;
	WORD m_nCurLocateAnimBitmap;
	WORD m_nCurUpdateAnimBitmap;
	WORD m_nMaxYMinimized;
	WORD m_nMaxYMaximized;

	DWORD m_nLargeY;
	BYTE m_nButtonWidth;
	BYTE m_nPresetsButtonWidth;
	BYTE m_nPresetsButtonHeight;

	BYTE m_nButtonSpacing;

	
	
	WORD m_nTabbedDialogHeight;
	BYTE m_nTabHeaderHeight;
	
	HWND m_hNextClipboardViewer;
	HMENU m_hActivePopupMenu;
	
	HWND m_hLastFocus;

	DWORD m_dwMaxFoundFiles;
	CLocater* m_pLocater;
	CCheckFileNotificationsThread* m_pFileNotificationsThread;
	CBackgroundUpdater* m_pBackgroundUpdater;

	BYTE m_nSorting;	// used for sorting list item, 0-6 bits: detail type, 7 bit: if 1 ascend sorting
	BYTE m_ClickWait;

	WORD m_WaitEvery30;
	WORD m_WaitEvery60;

	// For volume serial and label information 
	struct VolumeInformation {
		WORD wDB;
		WORD wRootID;
		LPSTR szVolumeSerial;
		LPSTR szVolumeLabel;
		LPSTR szFileSystem;
		BYTE bType;

		VolumeInformation(WORD wDB,WORD wRootID,BYTE bType,DWORD dwVolumeSerial,LPCSTR szVolumeLabel,LPCSTR szFileSystem);
		~VolumeInformation();
	};
	CArrayFP<VolumeInformation*> m_aVolumeInformation;


	// Accessors
public:
	DWORD GetFlags() const { return m_dwFlags; }
	DWORD GetExtraFlags() const { return m_dwExtraFlags; }
	DWORD GetMaxFoundFiles() const { return m_dwMaxFoundFiles; }
	void SetMaxFoundFiles(DWORD dwValue) { m_dwMaxFoundFiles=dwValue; }

	static LPCSTR GetVolumeLabel(WORD wDB,WORD wRootID);
	static LPCSTR GetVolumeSerial(WORD wDB,WORD wRootID);
	static LPCSTR GetVolumeFileSystem(WORD wDB,WORD wRootID);


	friend class CLocateDlgThread;
	friend class CCheckFileNotificationsThread;
	friend class CBackgroundUpdater;

#ifdef _DEBUG
public:
	inline void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	inline void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	inline void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }

	static LRESULT CALLBACK DebugWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
#endif


	
};

inline CLocateDlg* GetLocateDlg()
{
	extern CLocateApp theApp;
	CLocateAppWnd* pLocateAppWnd=(CLocateAppWnd*)theApp.GetMainWnd();
	if (pLocateAppWnd==NULL)
		return NULL;
	if (pLocateAppWnd->m_pLocateDlgThread==NULL)
		return NULL;
	return pLocateAppWnd->m_pLocateDlgThread->m_pLocate;
}


#endif
