/* Locate32 - Copyright (c) 1997-2009 Janne Huttunen */

#if !defined(RESULTDIALOGS_H)
#define RESULTDIALOGS_H

#if _MSC_VER >= 1000
#pragma once
#endif 

class CResults : public CExceptionObject
{
public:
	CResults(BOOL bThrowExceptions=FALSE);
	CResults(DWORD dwFlags,LPCWSTR szDescription,BOOL bThrowExceptions);
	~CResults();

	BOOL Initialize(DWORD dwFlags,LPCWSTR szDescription);
	void Close();


	// Initialize structures. bDataToTmpFile has to be FALSE if SaveToHtmlFile
	// with template file going to be used, otherwise bDataTmpFile has to be TRUE
	BOOL Create(CListCtrl* pList,int* pSelectedDetails,int nSelectedDetails,BOOL bDataToTmpFile);


	BOOL SaveToFile(LPCWSTR szFile) const;
	BOOL SaveToHtmlFile(LPCWSTR szFile) const;
	BOOL SaveToHtmlFile(LPCWSTR szFile,LPCWSTR szTemplateFile);
	
private:
	DWORD m_dwFlags;
	CStringW m_strDescription;

	int m_nSelectedDetails;
	int* m_pSelectedDetails;
	DWORD* m_pLengths;
	int m_nResults;
	int m_nFiles;
	int m_nDirectories;
	
	CWordArray m_aFromDatabases;

	CStringW m_sTempFile;

	CAutoPtrA<CLocateDlg::ViewDetails> m_AllDetails;
	CArray<CLocatedItem*> m_Items;	

private:
	struct Variable {
		enum Type {
			Integer,
			String
		} nType;
		union {
			int nInteger;
			LPCWSTR pString;
		};

		Variable() { nType=Integer; nInteger=0; }
		~Variable() { if (nType==String && pString!=NULL) delete[] pString; }
	};

	CStringMapFP<CHAR,Variable*> m_Variables;

	
	const Variable* GetVariable(LPCSTR szName) const;
	BOOL SetVariable(LPCSTR szName,int newInteger);
	BOOL SetVariable(LPCSTR szName,LPCWSTR pString);
	
	BOOL ParseBuffer(CStream& outFile,LPCWSTR pBuffer,int iBufferLen);
	BOOL ParseBlockLength(LPCWSTR pBuffer,int iBufferLen,int& riBlockLen) const;
	
	int EvalCondition(LPCWSTR pBuffer,int iConditionLength);
	void EvalCondition(LPCWSTR pBuffer,int iConditionLength,CStringW& output);


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
	void AddTemplates();
	
public:
	DWORD m_nFlags;
	CIntArray m_aDetails;
	CStringW m_strDescription;
	CStringW m_strTemplate;

private:
	CListCtrl* m_pList;
	CImageList m_ToolbarIL,m_ToolbarILHover,m_ToolbarILDisabled;

	CArrayFAP<LPCWSTR> m_TemplateFiles;


};

inline CResults::CResults(BOOL bThrowExceptions)
:	m_nSelectedDetails(0),m_pSelectedDetails(NULL),m_pLengths(NULL),m_dwFlags(0),CExceptionObject(bThrowExceptions)
{
}

inline CResults::CResults(DWORD dwFlags,LPCWSTR szDescription,BOOL bThrowExceptions)
:	m_nSelectedDetails(0),m_pSelectedDetails(NULL),m_pLengths(NULL),CExceptionObject(bThrowExceptions)
{
	Initialize(dwFlags,szDescription);
}

inline CResults::~CResults()
{
	Close();
}

inline const CResults::Variable* CResults::GetVariable(LPCSTR szName) const
{
	POSITION pPos=m_Variables.Find(szName);
	if (pPos==NULL)
		return FALSE;
	return m_Variables.GetAt(pPos);
}


	
#endif
