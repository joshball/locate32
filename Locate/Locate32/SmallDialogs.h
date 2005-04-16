#if !defined(SMALLDIALOGS_H)
#define SMALLDIALOGS_H

#if _MSC_VER >= 1000
#pragma once
#endif

class CSelectColumndDlg : public CDialog  
{
public:
	CSelectColumndDlg();

	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();
	virtual void OnDestroy();
	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);

	void OnOK();

	BOOL ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm);
	void DisableItems();
	BOOL ItemUpOrDown(BOOL bUp);

	struct ColumnItem 
	{
		
		CLocateDlg::DetailType m_nType;
		int m_nCol;
		int m_nWidth;
		CString m_strName;

		enum Align {
			Left=LVCFMT_LEFT,
			Right=LVCFMT_RIGHT,
			Center=LVCFMT_CENTER
		} m_nAlign;

		ColumnItem(int nCol,CLocateDlg::DetailType nType,int nWidth,Align nAlign);

	};


public:	
	CIntArray m_aSelectedCols; // In order
	CIntArray m_aIDs;
	CIntArray m_aWidths;
	CArray<ColumnItem::Align> m_aAligns;

private:
	CListCtrl* m_pList;
};



#define CUSTOM_PRESET		0
#define GLOBAL_PRESET		1
#define LATEST_PRESET		2
#define SELECTED_PRESET		3

class CSelectDatabasesDlg : public CDialog  
{
private:
	class CSavePresetDlg: public ::CSavePresetDlg  
	{
	public:
		CSavePresetDlg(CSelectDatabasesDlg* pParent);
		virtual void OnOK();

		CSelectDatabasesDlg* m_pParent;
	};

public:
	CSelectDatabasesDlg(const CArray<PDATABASE>& rOrigDatabases,CArray<PDATABASE>& rSelectedDatabases,BYTE bFlags,LPCSTR szRegKey);
	virtual ~CSelectDatabasesDlg();

	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();
	virtual void OnDestroy();
	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
	BOOL ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm);
    
	void OnOK();
	void OnThreads();
	void OnPresetCombo();
	void OnDeletePreset();

	
	BOOL InsertDatabases();
	BOOL InsertDatabases(WORD wCount,WORD wThreads,const WORD* pwDatabaseIDs,const WORD* pwThreads,
		WORD wSelectedCount,const WORD* pwSelectedIds);
	BOOL InsertSelected();
	
	void LoadPresets();
	BOOL LoadPreset(LPCSTR szName);
	BOOL SavePreset(LPCSTR szName);

	static BOOL CheckRegistryIntegrity(CRegKey& RegKey);


	void EnableThreadGroups(int nThreadGroups);
	void RemoveThreadGroups();
	void ChangeNumberOfThreads(int nThreads);
	BOOL ItemUpOrDown(BOOL bUp);
	BOOL IncreaseThread(int nItem,CDatabase* pDatabase,BOOL bDecrease=FALSE);

	void EnableButtons();
	BOOL IsItemEnabled(const CDatabase* pDatabase);
	BOOL EnableItem(CDatabase* pDatabase,BOOL bEnable);

	void SelectDatabases(char* pNames); // pNames is double zero terminated array of strins
		
public:
	enum Flags {
		flagDisablePresets=0x1,
		flagShowThreads=0x2,
		flagSetUpdateState=0x4,
		flagReturnNotSelected=0x8,
		flagGlobalIsSelected=0x10,
		flagCustomIsSelected=0x20,
		flagLasestIsSelected=0x30,
		flagSelectedMask=0x30
	};
	const CArray<PDATABASE>& m_rOrigDatabases;
	CArray<PDATABASE>& m_rSelectedDatabases;
	LPSTR m_pRegKey;
	BYTE m_bFlags;
	LPSTR m_pSelectDatabases;

	int m_nThreadsCurrently;
	CListCtrl* m_pList;

};

class CChangeCaseDlg: public CDialog  
{
public:
	CChangeCaseDlg();

	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();

	enum {
		Sentence,
		Lowercase,
		Uppercase,
		Title,
		Toggle
	} nSelectedCase;
	BOOL bForExtension;
};

class CChangeFilenameDlg: public CDialog  
{
public:
	CChangeFilenameDlg();

	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();

	CString m_sFileName;
};

class CRemovePresetDlg: public CDialog  
{
public:
	CRemovePresetDlg();

	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();
	
	void OnOK();
};


#include "SmallDialogs.inl"

#endif