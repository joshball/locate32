#if !defined(SMALLDIALOGS_INL)
#define SMALLDIALOGS_INL

#if _MSC_VER >= 1000
#pragma once
#endif

////////////////////////////////////////////////////////////
// CSelectColumndDlg

inline CSelectColumndDlg::CSelectColumndDlg()
:	CDialog(IDD_SELECTDETAILS),m_pList(NULL)
{
}

inline CSelectColumndDlg::ColumnItem::ColumnItem(int nCol,CLocateDlg::DetailType nType,int nWidth)
:	m_nType(nType),m_nWidth(nWidth),m_nCol(nCol)
{
	m_strName.LoadString(IDS_LISTNAME+int(nType),LanguageSpecificResource);
}

inline CSelectDatabasesDlg::CSelectDatabasesDlg(const CArray<PDATABASE>& rOrigDatabases,CArray<PDATABASE>& rSelectedDatabases,BYTE bFlags,LPCSTR szRegKey)
:	CDialog(IDD_SELECTDATABASES),
	m_rOrigDatabases(rOrigDatabases),m_rSelectedDatabases(rSelectedDatabases),
	m_pList(NULL),m_bFlags(bFlags),m_nThreadsCurrently(1),m_pSelectDatabases(NULL)
{
	if (szRegKey!=NULL)
		m_pRegKey=alloccopy(szRegKey);
	else
		m_pRegKey=NULL;

}

inline BOOL CSelectDatabasesDlg::IsItemEnabled(const CDatabase* pDatabase)
{
	if (m_bFlags&flagSetUpdateState)
		return pDatabase->IsGloballyUpdated();
	else
		return pDatabase->IsEnabled();
}

inline BOOL CSelectDatabasesDlg::EnableItem(CDatabase* pDatabase,BOOL bEnable)
{
	if (m_bFlags&flagSetUpdateState)
	{
		if (pDatabase->IsGloballyUpdated()==bEnable?1:0)
			return FALSE;
		pDatabase->UpdateGlobally(bEnable);
		DebugFormatMessage("db %s is %s (update)",pDatabase->GetName(),bEnable?"enabled":"disabled");
	}
	else
	{
		if (pDatabase->IsEnabled()==bEnable?1:0)
			return FALSE;
		pDatabase->Enable(bEnable);
		DebugFormatMessage("db %s is %s (locate)",pDatabase->GetName(),bEnable?"enabled":"disabled");
	}
	return TRUE;	
}

///////////////////////////////////////////////////////////
// CSavePresetDlg

inline CSavePresetDlg::CSavePresetDlg()
:	CDialog(IDD_PRESETNAME)
{
}

///////////////////////////////////////////////////////////
// CSelectDatabasesDlg::CSavePresetDlg

inline CSelectDatabasesDlg::CSavePresetDlg::CSavePresetDlg(CSelectDatabasesDlg* pParent)
:	::CSavePresetDlg(),m_pParent(pParent)
{
}

////////////////////////////////////////////////////////////
// CChangeCaseDlg

inline CChangeCaseDlg::CChangeCaseDlg()
:	CDialog(IDD_CHANGECASE), nSelectedCase(Sentence),bForExtension(FALSE)
{
}

////////////////////////////////////////////////////////////
// CChangeFilenameDlg

inline CChangeFilenameDlg::CChangeFilenameDlg()
:	CDialog(IDD_CHANGEFILENAME)
{
}

////////////////////////////////////////////////////////////
// CRemovePresetDlg

inline CRemovePresetDlg::CRemovePresetDlg()
: CDialog(IDD_PRESETREMOVE)
{
}


#endif