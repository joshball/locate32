/* Locate32 - Copyright (c) 1997-2008 Janne Huttunen */

#if !defined(RESULTDIALOGS_H)
#define RESULTDIALOGS_H

#if _MSC_VER >= 1000
#pragma once
#endif 

class CResults : public CExceptionObject
{
public:
	CResults(BOOL bThrowExceptions=FALSE);
	CResults(DWORD dwFlags,LPCSTR szDescription,BOOL bThrowExceptions);
	~CResults();

	BOOL Initialize(DWORD dwFlags,LPCSTR szDescription);
	void Close();

	BOOL Create(CListCtrl* pList,int* pDetails,int nDetails);
	BOOL SaveToFile(LPCSTR szFile) const;
	BOOL SaveToHtmlFile(LPCSTR szFile) const;

private:
	DWORD m_dwFlags;
	CString m_strDescription;

	int m_nDetails;
	int* m_pDetails;
	DWORD* m_pLengths;
	int m_nResults;
	int m_nFiles;
	int m_nDirectories;
	
	CWordArray m_aFromDatabases;

	CString m_sTempFile;
	


};

class CSaveResultsDlg : public CFileDialog  
{
public:
	CSaveResultsDlg();
	virtual ~CSaveResultsDlg();
	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnFileNameOK();
	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	
	BOOL ListNotifyHandler(NMLISTVIEW *pNm);
	BOOL ItemUpOrDown(BOOL bUp);
	
public:
	DWORD m_nFlags;
	CIntArray m_aDetails;
	CString m_strDescription;

private:
	CListCtrl* m_pList;
	CImageList m_ToolbarIL,m_ToolbarILHover,m_ToolbarILDisabled;


};

inline CResults::CResults(BOOL bThrowExceptions)
:	m_nDetails(0),m_pDetails(NULL),m_pLengths(NULL),m_dwFlags(0),CExceptionObject(bThrowExceptions)
{
}

inline CResults::CResults(DWORD dwFlags,LPCSTR szDescription,BOOL bThrowExceptions)
:	m_nDetails(0),m_pDetails(NULL),m_pLengths(NULL),CExceptionObject(bThrowExceptions)
{
	Initialize(dwFlags,szDescription);
}

inline CResults::~CResults()
{
	Close();
}

#endif
