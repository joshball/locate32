#include <HFCLib.h>
#include "Locate32.h"

BOOL CResults::Initialize(DWORD dwFlags,LPCSTR szDescription)
{
	m_dwFlags=dwFlags;
	m_strDescription=szDescription;
	m_nResults=0;
	m_nFiles=0;
	m_nDirectories=0;
	
	// Initializing temp file
	char szTempPath[MAX_PATH];
	if (!GetTempPath(MAX_PATH,szTempPath))
	{
		if (m_bThrow)
			throw CFileException(CFileException::badPath,GetLastError(),"TEMP");
		else
			return FALSE;
	}
	
	if (!GetTempFileName(szTempPath,"lsr",0,m_sTempFile.GetBuffer(MAX_PATH)))
	{
		if (m_bThrow)
			throw CFileException(CFileException::fileCreate,GetLastError(),"temp file");
		else
			return FALSE;
	}

	return TRUE;
}

void CResults::Close()
{
	if (m_pDetails!=NULL)
	{
		delete[] m_pDetails;
		m_pDetails=NULL;
	}
	if (m_pLengths!=NULL)
	{
		delete[] m_pLengths;
		m_pLengths=NULL;
	}
	if (!m_sTempFile.IsEmpty())
	{
		DeleteFile(m_sTempFile);
		m_sTempFile.Empty();
	}
}


BOOL CResults::Create(CListCtrl* pList,int* pDetails,int nDetails)
{
	CFile tmpFile(m_sTempFile,CFile::defWrite,TRUE);

	m_nDetails=nDetails;
	if (nDetails>0)
	{
		m_pDetails=new int[nDetails];
		m_pLengths=new DWORD[nDetails];
		
		sMemCopy(m_pDetails,pDetails,nDetails*sizeof(int));
		sMemSet(m_pLengths,0,nDetails*sizeof(int));
	}

	int mask=(m_dwFlags&RESULT_INCLUDESELECTEDITEMS)?LVNI_SELECTED:LVNI_ALL;


	int nItem=pList->GetNextItem(-1,mask);
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)pList->GetItemData(nItem);
        if (pItem!=NULL)
		{
			for (int i=0;i<nDetails;i++)
			{
				// Updating if necessary
				if (pItem->ShouldUpdateByDetail((CLocateDlg::DetailType)m_pDetails[i]))
					pItem->UpdateByDetail((CLocateDlg::DetailType)m_pDetails[i]);
				
				// Retrieving detail text
				LPSTR szDetail=pItem->GetDetailText((CLocateDlg::DetailType)m_pDetails[i]);
				DWORD dwLength;
				dstrlen(szDetail,dwLength);

				// Checking length
				if (dwLength>m_pLengths[i])
					m_pLengths[i]=dwLength;
				
				//Writing detail ot temp file
				tmpFile.Write(dwLength);
				tmpFile.Write(szDetail,dwLength);
			}
			m_nResults++;
			if (!pItem->IsDeleted())
			{
				if (pItem->IsFolder())
					m_nDirectories++;
				else
					m_nFiles++;
			}

			// Checking database IDs
			WORD wID=pItem->GetDatabaseID();
			for (int i=m_aFromDatabases.GetSize()-1;i>=0;i--)
			{
				if (m_aFromDatabases[i]==wID)
					break;
			}
			if (i<0)
				m_aFromDatabases.Add(wID);
		}
		nItem=pList->GetNextItem(nItem,mask);
	}
	return TRUE;
}

BOOL CResults::SaveToFile(LPCSTR szFile) const
{
	// Opening files
	CFile outFile(szFile,CFile::defWrite,TRUE);
	CFile tmpFile(m_sTempFile,CFile::defRead,TRUE);

	// Writing header
	{
		CString str;
		str.Format(IDS_SAVERESULTSHEADER,m_nResults,m_nFiles,m_nDirectories);
		outFile.Write(str);
	}


	
	// Checking width of labels if necessary
	CAllocArrayTmpl<CString> pLabels(max(m_nDetails,1));

	if (m_dwFlags&RESULT_INCLUDELABELS)
	{
	    for (int i=0;i<m_nDetails;i++)
		{
			pLabels[i].LoadString(IDS_LISTNAME+m_pDetails[i],LanguageSpecificResource);
			
			if (pLabels[i].GetLength()>m_pLengths[i])
				m_pLengths[i]=pLabels[i].GetLength();
		}
	}
	
	// Checking the fields tallest length
	DWORD dwMaxLength=0;
	for (int i=0;i<m_nDetails;i++)
	{
		if (m_pLengths[i]>dwMaxLength)
			dwMaxLength=m_pLengths[i];
	}
	
	// Initializing buffers
	CAllocArrayTmpl<BYTE> szBuffer(dwMaxLength+3,TRUE);
	CAllocArrayTmpl<BYTE> szSpaces(dwMaxLength+2,TRUE);
	dMemSet(szSpaces,' ',dwMaxLength+2);
	
	if (m_dwFlags&RESULT_INCLUDEDATE)
	{
		char szDate[200];
		outFile.Write(CString(IDS_SAVERESULTSDATE));
		DWORD dwLength=GetDateFormat(NULL,DATE_SHORTDATE,NULL,NULL,szDate,200);
        szDate[dwLength]=' ';
		outFile.Write(szDate,dwLength);
		dwLength=GetTimeFormat(NULL,0,NULL,NULL,szDate,200);
		szDate[dwLength++]='\r';
		szDate[dwLength++]='\n';
		outFile.Write(szDate,dwLength);
	}
	
	if (m_dwFlags&RESULT_INCLUDEDESCRIPTION)
	{
		outFile.Write(m_strDescription);
		outFile.Write("\r\n",2);
	}
	
	if (m_dwFlags&RESULT_INCLUDEDBINFO && m_aFromDatabases.GetSize()>0)
	{
		CString str(IDS_SAVERESULTSDBCAPTION);
		outFile.Write(str);
		outFile.Write("\r\n",2);

		for (int i=0;i<m_aFromDatabases.GetSize();i++)
		{
			const CDatabase* pDatabase=GetLocateApp()->GetDatabase(m_aFromDatabases[i]);
			str.Format(IDS_SAVERESULTSDB,pDatabase->GetName(),
				pDatabase->GetCreator(),pDatabase->GetDescription(),
				pDatabase->GetArchiveName());
			outFile.Write(str);
			outFile.Write("\r\n",2);
		}
	}

	outFile.Write("\r\n",2);

	if (m_dwFlags&RESULT_INCLUDELABELS && m_nDetails>0)
	{
		for (i=0;i<m_nDetails-1;i++)
		{
			outFile.Write((LPCSTR)pLabels[i],pLabels[i].GetLength());
			outFile.Write(szSpaces,m_pLengths[i]-pLabels[i].GetLength()+2);			
		}

		outFile.Write(pLabels[i]);
		outFile.Write("\r\n",2);
	}
	
    if (m_nDetails==0)
		return TRUE;

	// Saving data to files
	for (int nRes=0;nRes<m_nResults;nRes++)
	{
		DWORD dwLength;
			
		for (i=0;i<m_nDetails-1;i++)
		{
			// Reading length and data
			tmpFile.Read(dwLength);
            tmpFile.Read(szBuffer,dwLength);

			outFile.Write(szBuffer,dwLength);
			outFile.Write(szSpaces,m_pLengths[i]-dwLength+2);
		}

		tmpFile.Read(dwLength);
		tmpFile.Read(szBuffer,dwLength);

		szBuffer[dwLength++]='\r';
		szBuffer[dwLength++]='\n';
		outFile.Write(szBuffer,dwLength);
	}
    
	return TRUE;
}


CSaveResultsDlg::CSaveResultsDlg()
:	CFileDialog(FALSE,"*",szEmpty,OFN_EXPLORER|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_NOREADONLYRETURN|OFN_ENABLESIZING,NULL),
	m_nFlags(IDC_TITLE|RESULT_INCLUDEDATE|RESULT_INCLUDELABELS),m_pList(NULL)
{
	DWORD nFlags=GetSystemFeaturesFlag();
	EnableFeatures(nFlags);
	if (nFlags&(efWin2000|efWinME))
		SetTemplate(IDD_RESULTSAVEDIALOG2000);
	else
		SetTemplate(IDD_RESULTSAVEDIALOG);
	CString str;
	str.LoadString(IDS_TXTFILTERS);
	m_strFilter=new char[str.GetLength()+1];
	sMemCopy(m_strFilter,(LPCSTR)str,str.GetLength()+1);
	str.LoadString(IDS_SAVERESULTS);
	m_pofn->lpstrTitle=new char[str.GetLength()+1];
	if (m_pofn->lpstrTitle!=NULL)
		sMemCopy((void*)m_pofn->lpstrTitle,(LPCSTR)str,str.GetLength()+1);

	// Setting default details
	m_aDetails.Add(CLocateDlg::DetailType::FullPath);
	m_aDetails.Add(CLocateDlg::DetailType::FileSize);
	m_aDetails.Add(CLocateDlg::DetailType::DateModified);
}

CSaveResultsDlg::~CSaveResultsDlg()
{
	if (m_pofn->lpstrTitle!=NULL)
	{
		delete[] (LPSTR)m_pofn->lpstrTitle;
		m_pofn->lpstrTitle=NULL;
	}

	if (m_pList!=NULL)
		delete m_pList;
}

BOOL CSaveResultsDlg::OnInitDialog(HWND hwndFocus)
{
	// Initializing detail list view
	m_pList=new CListCtrl(GetDlgItem(IDC_DETAILS));
	m_pList->SetExtendedListViewStyle(LVS_EX_CHECKBOXES,LVS_EX_CHECKBOXES);
	m_pList->InsertColumn(0,"",LVCFMT_LEFT,330,0);
		
	// Initialing toolbar
	m_ToolbarIL.Create(IDB_SAVERESTOOLBARBITMAPS,14,2,RGB(255,255,255));
	m_ToolbarILHover.Create(IDB_SAVERESTOOLBARBITMAPSH,14,2,RGB(255,255,255));
	m_ToolbarILDisabled.Create(IDB_SAVERESTOOLBARBITMAPSD,14,2,RGB(255,255,255));
	SendDlgItemMessage(IDC_TOOLBAR,TB_SETIMAGELIST,0,LPARAM(HIMAGELIST(m_ToolbarIL)));
	SendDlgItemMessage(IDC_TOOLBAR,TB_SETHOTIMAGELIST,0,LPARAM(HIMAGELIST(m_ToolbarILHover)));
	SendDlgItemMessage(IDC_TOOLBAR,TB_SETDISABLEDIMAGELIST,0,LPARAM(HIMAGELIST(m_ToolbarILDisabled)));
	
	TBBUTTON tb;
	memset(&tb,0,sizeof(TBBUTTON));
	tb.idCommand=IDC_UP;
	tb.fsStyle=TBSTYLE_BUTTON;
	tb.fsState=TBSTATE_ENABLED;
	tb.iBitmap=0;
	SendDlgItemMessage(IDC_TOOLBAR,TB_INSERTBUTTON,0,LPARAM(&tb));
	tb.idCommand=IDC_DOWN;
	tb.iBitmap=1;
	SendDlgItemMessage(IDC_TOOLBAR,TB_INSERTBUTTON,1,LPARAM(&tb));
	EnableDlgItem(IDC_TOOLBAR,m_pList->GetNextItem(-1,LVNI_SELECTED)!=-1);

	
	// Setting dialog items to correspond with m_nFlags
	if (m_nFlags&RESULT_INCLUDEDATE)
		CheckDlgButton(IDC_DATE,1);
	if (m_nFlags&RESULT_INCLUDELABELS)
		CheckDlgButton(IDC_LABELS,1);
	if (m_nFlags&RESULT_INCLUDEDBINFO)
		CheckDlgButton(IDC_DBINFO,1);
	if (m_nFlags&RESULT_INCLUDEDESCRIPTION)
	{
		CheckDlgButton(IDC_DESCRIPTIONTOGGLE,1);
		EnableDlgItem(IDC_DESCRIPTION,1);
		SetDlgItemText(IDC_DESCRIPTION,m_strDescription);
	}
	else
		EnableDlgItem(IDC_DESCRIPTION,0);

	if (m_nFlags&RESULT_ACTIVATESELECTEDITEMS)
	{
		if (m_nFlags&RESULT_INCLUDESELECTEDITEMS)
			CheckDlgButton(IDC_SELECTEDITEMS,1);
		else
			CheckDlgButton(IDC_ALLITEMS,1);
	}
	else
	{
		CheckDlgButton(IDC_ALLITEMS,1);
		EnableDlgItem(IDC_SELECTEDITEMS,FALSE);
	}

	// Inserting details to list view and checking selected
	int nItem;
	for (nItem=0;nItem<m_aDetails.GetSize();nItem++)
	{
		m_pList->InsertItem(LVIF_TEXT|LVIF_PARAM,nItem,
			LPSTR_TEXTCALLBACK,0,0,0,LPARAM(m_aDetails[nItem]));
		m_pList->SetCheckState(nItem,TRUE);
	}

	for (int nDetail=0;nDetail<=CLocateDlg::DetailType::LastType;nDetail++)
	{
		if (m_aDetails.Find(nDetail)==-1)
		{
			m_pList->InsertItem(LVIF_TEXT|LVIF_PARAM,nItem++,
				LPSTR_TEXTCALLBACK,0,0,0,LPARAM(nDetail));
			m_pList->SetCheckState(nItem,FALSE);
		}
	}
	
	return CFileDialog::OnInitDialog(hwndFocus);
}


BOOL CSaveResultsDlg::ItemUpOrDown(BOOL bUp)
{
	int nSelected=m_pList->GetNextItem(-1,LVNI_SELECTED);
	ASSERT(nSelected!=-1);

	int nOther=m_pList->GetNextItem(nSelected,bUp?LVNI_ABOVE:LVNI_BELOW);
	if (nOther==-1 || nOther==nSelected)
		return FALSE;

	// This is found to be the best way to do this
	LVITEM li;
	BOOL bSelected=m_pList->GetCheckState(nSelected);
	li.mask=LVIF_STATE|LVIF_PARAM;
	li.stateMask=0xFFFFFFFF;
	li.iItem=nSelected;
	li.iSubItem=0;
	m_pList->GetItem(&li);
	m_pList->SetItemData(nSelected,NULL);
	m_pList->DeleteItem(nSelected);
	li.iItem=nOther;
	li.mask=LVIF_PARAM|LVIF_STATE|LVIF_TEXT;
	li.pszText=LPSTR_TEXTCALLBACK;
	nOther=m_pList->InsertItem(&li);
	m_pList->SetCheckState(nOther,bSelected);
	m_pList->EnsureVisible(nOther,FALSE);
	m_pList->SetFocus();
	return TRUE;
}

BOOL CSaveResultsDlg::OnFileNameOK()
{
	m_nFlags=0;
	if (IsDlgButtonChecked(IDC_DATE))
		m_nFlags=RESULT_INCLUDEDATE;
	if (IsDlgButtonChecked(IDC_LABELS))
		m_nFlags|=RESULT_INCLUDELABELS;
	if (IsDlgButtonChecked(IDC_DBINFO))
		m_nFlags|=RESULT_INCLUDEDBINFO;
	if (IsDlgButtonChecked(IDC_DESCRIPTIONTOGGLE))
	{
		m_nFlags|=RESULT_INCLUDEDESCRIPTION;
		GetDlgItemText(IDC_DESCRIPTION,m_strDescription);
	}
	else
		m_strDescription.Empty();
	if (IsDlgButtonChecked(IDC_SELECTEDITEMS))
		m_nFlags|=RESULT_INCLUDESELECTEDITEMS;


	int nItem=m_pList->GetNextItem(-1,LVNI_ALL);
	m_aDetails.RemoveAll();
	while (nItem!=-1)
	{
		if (m_pList->GetCheckState(nItem))
			m_aDetails.Add(m_pList->GetItemData(nItem));	
		nItem=m_pList->GetNextItem(nItem,LVNI_ALL);
	}
	return TRUE;
}


BOOL CSaveResultsDlg::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	switch (idCtrl)
	{
	case IDC_DETAILS:
		return ListNotifyHandler((LV_DISPINFO*)pnmh,(NMLISTVIEW*)pnmh);
	}
	return CFileDialog::OnNotify(idCtrl,pnmh);
}

BOOL CSaveResultsDlg::ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm)
{
	switch(pLvdi->hdr.code)
	{
	case LVN_GETDISPINFO:
		{
			if (pLvdi->item.lParam<=CLocateDlg::DetailType::LastType)
			{
				m_strBuffer.LoadString(IDS_LISTNAME+pLvdi->item.lParam,LanguageSpecificResource);
				pLvdi->item.pszText=m_strBuffer.GetBuffer();
			}
			break;
		}
	case LVN_ITEMCHANGED:
		if (pNm->uNewState&LVIS_SELECTED)
		{
			EnableDlgItem(IDC_TOOLBAR,TRUE);

			SendDlgItemMessage(IDC_TOOLBAR,TB_SETSTATE,IDC_UP,m_pList->GetNextItem(pNm->iItem,LVNI_ABOVE)!=-1?TBSTATE_ENABLED:0);
			SendDlgItemMessage(IDC_TOOLBAR,TB_SETSTATE,IDC_DOWN,m_pList->GetNextItem(pNm->iItem,LVNI_BELOW)!=-1?TBSTATE_ENABLED:0);
			
		}
		else if (pNm->uOldState&LVNI_SELECTED)
			EnableDlgItem(IDC_TOOLBAR,FALSE);

		break;
	}
	return FALSE;
}


BOOL CSaveResultsDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch (wID)
	{
	case IDC_DESCRIPTIONTOGGLE:
		EnableDlgItem(IDC_DESCRIPTION,IsDlgButtonChecked(IDC_DESCRIPTIONTOGGLE));
		break;
	case IDC_UP:
		ItemUpOrDown(TRUE);
		break;
	case IDC_DOWN:
		ItemUpOrDown(FALSE);
		break;
	}
	return CFileDialog::OnCommand(wID,wNotifyCode,hControl);
}

