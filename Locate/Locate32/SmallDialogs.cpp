#include <HFCLib.h>
#include "Locate32.h"

///////////////////////////////////////////////////////////
// CSelectColumndDlg

BOOL CSelectColumndDlg::OnInitDialog(HWND hwndFocus)
{
	ASSERT(m_aIDs.GetSize()==m_aWidths.GetSize());

	m_pList=new CListCtrl(GetDlgItem(IDC_COLUMNS));
	m_pList->SetExtendedListViewStyle(LVS_EX_CHECKBOXES,LVS_EX_CHECKBOXES);
	m_pList->InsertColumn(0,"",LVCFMT_LEFT,250);

    int nItem;
	for (nItem=0;nItem<m_aSelectedCols.GetSize();nItem++)
	{
		m_pList->InsertItem(LVIF_TEXT|LVIF_PARAM,nItem,LPSTR_TEXTCALLBACK,0,0,0,
			LPARAM(new ColumnItem(m_aSelectedCols[nItem],
			(CLocateDlg::DetailType)m_aIDs[m_aSelectedCols[nItem]],
			m_aWidths[m_aSelectedCols[nItem]],
			m_aAligns[m_aSelectedCols[nItem]])));
		m_pList->SetCheckState(nItem,TRUE);
	}

	for (int i=0;i<m_aIDs.GetSize();i++)
	{
		if (m_aSelectedCols.Find(i)==-1)
		{
			m_pList->InsertItem(LVIF_TEXT|LVIF_PARAM,nItem++,
				LPSTR_TEXTCALLBACK,0,0,0,LPARAM(new ColumnItem(i,
				(CLocateDlg::DetailType)m_aIDs[i],m_aWidths[i],m_aAligns[i])));
			m_pList->SetCheckState(nItem,FALSE);
		}
	}
	
	CSpinButtonCtrl spin(GetDlgItem(IDC_SPIN));
	spin.SetRange(10,10000);
	spin.SetBuddy(GetDlgItem(IDC_WIDTH));
	
	DisableItems();
	
	return CDialog::OnInitDialog(hwndFocus);
}

BOOL CSelectColumndDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch (wID)
	{
	case IDC_OK:
		OnOK();
		break;
	case IDCANCEL:
	case IDC_CANCEL:
		EndDialog(0);
		break;
	case IDC_SHOW:
	case IDC_HIDE:
		ASSERT(m_pList->GetNextItem(-1,LVNI_SELECTED)!=-1);

		m_pList->SetCheckState(m_pList->GetNextItem(-1,LVNI_SELECTED),wID==IDC_SHOW);
		m_pList->SetFocus();
		break;
	case IDC_LEFT:
		{
			int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
			ASSERT(nItem!=-1);

			ColumnItem* pItem=(ColumnItem*)m_pList->GetItemData(nItem);
			if (pItem==NULL)
				break;

			pItem->m_nAlign=ColumnItem::Left;
			break;
		}
	case IDC_RIGHT:
		{
			int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
			ASSERT(nItem!=-1);

			ColumnItem* pItem=(ColumnItem*)m_pList->GetItemData(nItem);
			if (pItem==NULL)
				break;

			pItem->m_nAlign=ColumnItem::Right;
			break;
		}
	case IDC_CENTER:
		{
			int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
			ASSERT(nItem!=-1);

			ColumnItem* pItem=(ColumnItem*)m_pList->GetItemData(nItem);
			if (pItem==NULL)
				break;

			pItem->m_nAlign=ColumnItem::Center;
			break;
		}
	case IDC_UP:
		ItemUpOrDown(TRUE);
		break;
	case IDC_DOWN:
		ItemUpOrDown(FALSE);
		break;
	case IDC_WIDTH:
		if (wNotifyCode==EN_CHANGE)
		{
			int nItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
			ASSERT(nItem!=-1);

			ColumnItem* pItem=(ColumnItem*)m_pList->GetItemData(nItem);
			if (pItem==NULL)
				break;

			pItem->m_nWidth=GetDlgItemInt(IDC_WIDTH,NULL,FALSE);

		}
		break;
	}
	return CDialog::OnCommand(wID,wNotifyCode,hControl);
}

BOOL CSelectColumndDlg::ItemUpOrDown(BOOL bUp)
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

BOOL CSelectColumndDlg::OnClose()
{
	CDialog::OnClose();
	EndDialog(0);
	return 0;
}

void CSelectColumndDlg::OnDestroy()
{
	if (m_pList!=NULL)
	{
		delete m_pList;
		m_pList=NULL;
	}
	
	return CDialog::OnDestroy();
}

BOOL CSelectColumndDlg::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	switch (idCtrl)
	{
	case IDC_COLUMNS:
		return ListNotifyHandler((LV_DISPINFO*)pnmh,(NMLISTVIEW*)pnmh);
	}
	return CDialog::OnNotify(idCtrl,pnmh);
}

BOOL CSelectColumndDlg::ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm)
{
	switch(pLvdi->hdr.code)
	{
	case LVN_GETDISPINFO:
		{
			ColumnItem* pItem=(ColumnItem*)pLvdi->item.lParam;
			if (pItem==NULL)
				break;
			pLvdi->item.pszText=pItem->m_strName.GetBuffer();
			break;
		}
	case LVN_DELETEITEM:
		if (pNm->lParam!=NULL)
			delete (ColumnItem*) pNm->lParam;
		break;
	case LVN_ITEMCHANGED:
		if (pNm->uNewState&(1<<12))
		{
			if (m_pList->GetItemState(pNm->iItem,LVNI_SELECTED))
			{
				EnableDlgItem(IDC_SHOW,TRUE);
				EnableDlgItem(IDC_HIDE,FALSE);
			}
		}
		else if (pNm->uOldState&(1<<12))		
		{
			if (m_pList->GetItemState(pNm->iItem,LVNI_SELECTED))
			{
				EnableDlgItem(IDC_SHOW,FALSE);
				EnableDlgItem(IDC_HIDE,TRUE);
			}
		}
		if (pNm->uNewState&LVIS_SELECTED)
		{
			ColumnItem* pItem=(ColumnItem*)pNm->lParam;
			if (pItem==NULL)
				break;
				
			BOOL bChecked=m_pList->GetCheckState(pNm->iItem);
			EnableDlgItem(IDC_SHOW,!bChecked);
			EnableDlgItem(IDC_HIDE,bChecked);

            EnableDlgItem(IDC_UP,m_pList->GetNextItem(pNm->iItem,LVNI_ABOVE)!=-1);
			EnableDlgItem(IDC_DOWN,m_pList->GetNextItem(pNm->iItem,LVNI_BELOW)!=-1);

			EnableDlgItem(IDC_TEXT,TRUE);
			EnableDlgItem(IDC_WIDTH,TRUE);
			::InvalidateRect(GetDlgItem(IDC_SPIN),NULL,TRUE);
			SendDlgItemMessage(IDC_SPIN,UDM_SETPOS,0,pItem->m_nWidth);

			EnableDlgItem(IDC_ALIGN,TRUE);
			EnableDlgItem(IDC_LEFT,TRUE);
			EnableDlgItem(IDC_RIGHT,TRUE);
			EnableDlgItem(IDC_CENTER,TRUE);

			CheckDlgButton(IDC_LEFT,pItem->m_nAlign==ColumnItem::Left);
			CheckDlgButton(IDC_RIGHT,pItem->m_nAlign==ColumnItem::Right);
			CheckDlgButton(IDC_CENTER,pItem->m_nAlign==ColumnItem::Center);
		}
		else if (pNm->uOldState&LVNI_SELECTED)
			DisableItems();
		break;
	}
	return FALSE;
}

void CSelectColumndDlg::DisableItems()
{
	EnableDlgItem(IDC_HIDE,FALSE);
	EnableDlgItem(IDC_SHOW,FALSE);
	EnableDlgItem(IDC_UP,FALSE);
	EnableDlgItem(IDC_DOWN,FALSE);
	EnableDlgItem(IDC_TEXT,FALSE);
	EnableDlgItem(IDC_WIDTH,FALSE);
	::InvalidateRect(GetDlgItem(IDC_SPIN),NULL,TRUE);

	EnableDlgItem(IDC_ALIGN,FALSE);
	EnableDlgItem(IDC_LEFT,FALSE);
	EnableDlgItem(IDC_RIGHT,FALSE);
	EnableDlgItem(IDC_CENTER,FALSE);
	
}

void CSelectColumndDlg::OnOK()
{
	m_aSelectedCols.RemoveAll();
	
    int nIndex=m_pList->GetNextItem(-1,LVNI_ALL);
	if (nIndex==-1)
	{
		ShowErrorMessage(IDS_ERRORNOCOLUMNSSELECTED,IDS_ERROR,MB_OK|MB_ICONINFORMATION);
		return;
	}

	while (nIndex!=-1)
	{
		ColumnItem* pItem=(ColumnItem*)m_pList->GetItemData(nIndex);
		m_aWidths[pItem->m_nCol]=pItem->m_nWidth; // Setting width
		m_aAligns[pItem->m_nCol]=pItem->m_nAlign; // Setting align
		
		if (m_pList->GetCheckState(nIndex))
			m_aSelectedCols.Add(pItem->m_nCol);
		nIndex=m_pList->GetNextItem(nIndex,LVNI_ALL);
	}

	EndDialog(1);
}

///////////////////////////////////////////////////////////
// CSelectDatabasesDlg


CSelectDatabasesDlg::~CSelectDatabasesDlg()
{
	if (m_pRegKey!=NULL)
		delete[] m_pRegKey;
	if (m_pSelectDatabases!=NULL)
	{
		delete[] m_pSelectDatabases;
		m_pSelectDatabases=NULL;
	}
}

BOOL CSelectDatabasesDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	
	CenterWindow();

	// Creating list control
	m_pList=new CListCtrl(GetDlgItem(IDC_DATABASES));
	m_pList->SetExtendedListViewStyle(LVS_EX_CHECKBOXES|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT,
		LVS_EX_CHECKBOXES|LVS_EX_HEADERDRAGDROP|LVS_EX_FULLROWSELECT);
	
	m_pList->InsertColumn(0,CString(IDS_DATABASENAME),LVCFMT_LEFT,100,0);
	m_pList->InsertColumn(1,CString(IDS_DATABASEFILE),LVCFMT_LEFT,130,0);
	m_pList->InsertColumn(2,CString(IDS_DATABASECREATOR),LVCFMT_LEFT,70,0);
	m_pList->InsertColumn(3,CString(IDS_DATABASEDESCRIPTION),LVCFMT_LEFT,100,0);
	
	if (!(m_bFlags&flagShowThreads))
	{
		// Hiding controls related to threads
		ShowDlgItem(IDC_THREADSLABEL,swHide);
		ShowDlgItem(IDC_THREADS,swHide);
		ShowDlgItem(IDC_THREADSPIN,swHide);
		ShowDlgItem(IDC_UP,swHide);
		ShowDlgItem(IDC_DOWN,swHide);

		CRect rect;
		::GetWindowRect(GetDlgItem(IDC_DATABASES),&rect);
		::SetWindowPos(GetDlgItem(IDC_DATABASES),NULL,0,0,
			rect.Width(),rect.Height()+28,SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
		
	}
	else 
	{
		if (((CLocateApp*)GetApp())->m_wComCtrlVersion<0x0600)
		{
			m_pList->InsertColumn(4,CString(IDS_THREADID),LVCFMT_LEFT,40,0);
			int oa[]={3,0,1,2};
			m_pList->SetColumnOrderArray(4,oa);
		}

		// Setting threads counter
		CSpinButtonCtrl Spin(GetDlgItem(IDC_THREADSPIN));
		Spin.SetBuddy(GetDlgItem(IDC_THREADS));
		Spin.SetRange(1,99);
	}

	if (m_bFlags&flagDisablePresets)
	{
		EnableDlgItem(IDC_PRESETSLABEL,FALSE);
		EnableDlgItem(IDC_PRESETS,FALSE);
		EnableDlgItem(IDC_PRESETSLABEL,FALSE);
		EnableDlgItem(IDC_SAVE,FALSE);
		EnableDlgItem(IDC_DELETE,FALSE);
	}

	m_pList->LoadColumnsState(HKCU,m_pRegKey,"Database List Widths");
	
	LoadPresets();

	EnableButtons();    
	return FALSE;
}

BOOL CSelectDatabasesDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);
	switch (wID)
	{
	case IDC_OK:
		OnOK();
		break;
	case IDCANCEL:
	case IDC_CANCEL:
		EndDialog(0);
		break;
	case IDC_DOWN:
	case IDC_UP:
		if (ItemUpOrDown(wID==IDC_UP))
			SendDlgItemMessage(IDC_PRESETS,CB_SETCURSEL,CUSTOM_PRESET);
		break;
	case IDC_SAVE:
		{
			CSavePresetDlg pd(this);
			pd.DoModal(*this);
			break;
		}
	case IDC_PRESETS:
		if (wNotifyCode==CBN_SELCHANGE)
			OnPresetCombo();
		break;
	case IDC_DELETE:
		OnDeletePreset();
		break;
	case IDC_THREADS:
		if (wNotifyCode==EN_CHANGE)
			OnThreads();
		else if (wNotifyCode==EN_SETFOCUS)
			SendDlgItemMessage(IDC_THREADS,EM_SETSEL,0,-1);
		break;
	}
	return FALSE;
}

void CSelectDatabasesDlg::SelectDatabases(char* pNames)
{
	if (m_pSelectDatabases!=NULL)
		delete[] m_pSelectDatabases;
    if (pNames==NULL)
	{
		m_pSelectDatabases=NULL;
		m_bFlags&=~flagSelectedMask;
		m_bFlags|=flagGlobalIsSelected;
		return;
	}
	
	DWORD dwLength=1;
	LPSTR pPtr=pNames;
	while (*pPtr!='\0')
	{
		int iStrLen=istrlen(pPtr)+1;
		dwLength+=iStrLen;
		pPtr+=iStrLen;
	}
	m_pSelectDatabases=new char[dwLength];
	CopyMemory(m_pSelectDatabases,pNames,dwLength);
}

BOOL CSelectDatabasesDlg::OnClose()
{
	CDialog::OnClose();
	EndDialog(0);
	return FALSE;
}

void CSelectDatabasesDlg::OnDestroy()
{
	if (m_pList!=NULL)
	{
		m_pList->SaveColumnsState(HKCU,m_pRegKey,"Database List Widths");
		delete m_pList;
		m_pList=NULL;
	}

	CDialog::OnDestroy();
}

BOOL CSelectDatabasesDlg::OnNotify(int idCtrl,LPNMHDR pnmh)
{
	switch (idCtrl)
	{
	case IDC_DATABASES:
		return ListNotifyHandler((LV_DISPINFO*)pnmh,(NMLISTVIEW*)pnmh);
	}
	return CDialog::OnNotify(idCtrl,pnmh);
}

void CSelectDatabasesDlg::OnOK()
{
	DebugFormatMessage("CSelectDatabasesDlg::OnOK() BEGIN, Update: %d, bReturnNotSelected: %d",
		m_bFlags&flagSetUpdateState?TRUE:FALSE,m_bFlags&flagReturnNotSelected?TRUE:FALSE);

	// Saves last set
	if (!(m_bFlags&flagDisablePresets))
		SavePreset(NULL);

	// Get the first item
	int nNext;
	int nItem=m_pList->GetNextItem(-1,LVNI_ALL);

	while ((nNext=m_pList->GetNextItem(nItem,LVNI_ABOVE))!=-1)
	{
		if (nNext==nItem)
			break; // This should not be like that, why is it?
		nItem=nNext;
	}
	
	while (nItem!=-1)
	{
		PDATABASE pDatabase=(PDATABASE)m_pList->GetItemData(nItem);
		ASSERT(pDatabase!=NULL);

		if (m_bFlags&flagReturnNotSelected || IsItemEnabled(pDatabase))
		{
			DebugFormatMessage("Database %s is selected",pDatabase->GetName());

			m_rSelectedDatabases.Add(pDatabase);
			m_pList->SetItemData(nItem,NULL);
		}

		nNext=m_pList->GetNextItem(nItem,LVNI_BELOW);
		if (nNext==nItem)
			break;
		nItem=nNext;
	}

	int nCurSel=SendDlgItemMessage(IDC_PRESETS,CB_GETCURSEL);
	DebugFormatMessage("Selection: %d",nCurSel);
	m_bFlags&=~flagSelectedMask;
	switch (nCurSel)
	{
	case GLOBAL_PRESET:
		m_bFlags|=flagGlobalIsSelected;
		DebugMessage("global selected");
		break;
	case CUSTOM_PRESET:
        m_bFlags|=flagCustomIsSelected;
		DebugMessage("preset");
		break;
	case LATEST_PRESET:
		m_bFlags|=flagLasestIsSelected;
		DebugMessage("latest");
		break;
	}
	EndDialog(1);

	DebugMessage("CSelectDatabasesDlg::OnOK() END");
}

void CSelectDatabasesDlg::OnDeletePreset()
{
	int nCurSel=SendDlgItemMessage(IDC_PRESETS,CB_GETCURSEL);
	if (nCurSel==CB_ERR)
		return;
	
	
	int nTextLen=SendDlgItemMessage(IDC_PRESETS,CB_GETLBTEXTLEN,nCurSel);
	if (nTextLen==CB_ERR)
		return;
	char* pText=new char[nTextLen+1];
    if (SendDlgItemMessage(IDC_PRESETS,CB_GETLBTEXT,nCurSel,LPARAM(pText))==CB_ERR)
	{
		delete[] pText;
		return;
	}
	
	SendDlgItemMessage(IDC_PRESETS,CB_DELETESTRING,nCurSel);
	

	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,m_pRegKey,CRegKey::openExist|CRegKey::samAll)==ERROR_SUCCESS)
	{
		CString sName;
		BOOL bBreaked=FALSE;
		for (int i=0;RegKey.EnumValue(i,sName)>0;i++)
		{
			if (strncmp(sName,"Preset ",7)==0)
			{
				DWORD nIndex=sName.FindFirst(':')+1;

                if (nIndex>0 &&  nIndex<sName.GetLength()) 
				{
					if (strcasecmp(LPCSTR(sName)+nIndex,pText)==0)
					{
						RegKey.DeleteValue(sName);
						break;
					}
				}
			}
		}

		while (CheckRegistryIntegrity(RegKey));
		
	}
	delete[] pText;

	SendDlgItemMessage(IDC_PRESETS,CB_SETCURSEL,CUSTOM_PRESET);

}

BOOL CSelectDatabasesDlg::CheckRegistryIntegrity(CRegKey& RegKey)
{
	int nShouldBe=0;
	CString sName;
	for (int i=0;RegKey.EnumValue(i,sName)>0;i++)
	{
		if (strncmp(sName,"Preset ",7)==0)
		{
			DWORD nIndex=sName.FindFirst(':')+1;
			if (nIndex>0 &&  nIndex<sName.GetLength()) 
			{
				int nIs=atoi(LPCSTR(sName)+7);
				if (nIs!=nShouldBe)
				{
					// Renaming
					DWORD dwLength=RegKey.QueryValueLength(sName);
					char* pData=new char[dwLength+2];
					DWORD dwType;
					RegKey.QueryValue(sName,pData,dwLength,&dwType);
					RegKey.DeleteValue(sName);
						CString name(LPCSTR(sName)+nIndex);
					sName.Format("Preset %03d:%s",nShouldBe,LPCSTR(name));
					RegKey.SetValue(sName,pData,dwLength,dwType);

					return TRUE;
				}
				nShouldBe++;
			}
		}
	}
	return FALSE;
}

void CSelectDatabasesDlg::OnThreads()
{
	int nThreads=GetDlgItemInt(IDC_THREADS);

	if (nThreads<1)
		SetDlgItemInt(IDC_THREADS,nThreads=1,FALSE);

	ChangeNumberOfThreads(nThreads);
	EnableButtons();
}

void CSelectDatabasesDlg::OnPresetCombo()
{
	int nCurSel=SendDlgItemMessage(IDC_PRESETS,CB_GETCURSEL);
	if (nCurSel==CB_ERR)
		return;

	if (nCurSel==GLOBAL_PRESET) // Global
		InsertDatabases();
	else if (nCurSel==LATEST_PRESET)
		LoadPreset(NULL);
	else if (nCurSel==SELECTED_PRESET && m_pSelectDatabases!=NULL )
		InsertSelected();
	else
	{
		int nTextLen=SendDlgItemMessage(IDC_PRESETS,CB_GETLBTEXTLEN,nCurSel);
		if (nTextLen==CB_ERR)
			return;
		char* pText=new char[nTextLen+1];
        if (SendDlgItemMessage(IDC_PRESETS,CB_GETLBTEXT,nCurSel,LPARAM(pText))!=CB_ERR)
			LoadPreset(pText);
		delete[] pText;
	}
}

BOOL CSelectDatabasesDlg::ListNotifyHandler(LV_DISPINFO *pLvdi,NMLISTVIEW *pNm)
{
	switch(pLvdi->hdr.code)
	{
	case LVN_ITEMCHANGED:
		if (pNm->lParam!=NULL && (pNm->uNewState&0x00002000)!=(pNm->uOldState&0x00002000))
		{
			if (EnableItem((CDatabase*)pNm->lParam,m_pList->GetCheckState(pNm->iItem)))
				SendDlgItemMessage(IDC_PRESETS,CB_SETCURSEL,CUSTOM_PRESET);
		}
		break;
	case NM_CLICK:
		EnableButtons();
		break;
	case LVN_DELETEITEM:
		if (pNm->lParam!=NULL)
			delete (CDatabase*)pNm->lParam;
		break;
	case LVN_GETDISPINFO:
		{
			CDatabase* pDatabase=(CDatabase*)pLvdi->item.lParam;
			if (pDatabase==NULL)
				break;
				
			switch (pLvdi->item.iSubItem)
			{
			case 0:
				pLvdi->item.pszText=const_cast<LPSTR>(pDatabase->GetName());
				break;
			case 1:
				pLvdi->item.pszText=const_cast<LPSTR>(pDatabase->GetArchiveName());
				break;
			case 2:
				pLvdi->item.pszText=const_cast<LPSTR>(pDatabase->GetCreator());
				break;
			case 3:
				pLvdi->item.pszText=const_cast<LPSTR>(pDatabase->GetDescription());
				break;
			case 4:
				ISDLGTHREADOK
				if (g_szBuffer!=NULL)
					delete[] g_szBuffer;
				g_szBuffer=new char[20];
				itoa(pDatabase->GetThreadId()+1,g_szBuffer,10);
				pLvdi->item.pszText=g_szBuffer;
				break;
			}
			break;
		}
	}
	return 0;
}


BOOL CSelectDatabasesDlg::ItemUpOrDown(BOOL bUp)
{
	int nSelected=m_pList->GetNextItem(-1,LVNI_SELECTED);
	ASSERT(nSelected!=-1);
	CDatabase* pSelected=(CDatabase*)m_pList->GetItemData(nSelected);
	
	int nOther=m_pList->GetNextItem(nSelected,bUp?LVNI_ABOVE:LVNI_BELOW);
	if (nOther==-1 || nOther==nSelected)
	{
		if (!(m_bFlags&flagShowThreads))
			return FALSE;
		if (bUp && pSelected->GetThreadId()>0)
			return IncreaseThread(nSelected,pSelected,TRUE);
		else if (!bUp && pSelected->GetThreadId()<m_nThreadsCurrently-1)
			return IncreaseThread(nSelected,pSelected,FALSE);
		return FALSE;
	}

	CDatabase* pOther=(CDatabase*)m_pList->GetItemData(nOther);
	if (m_bFlags&flagShowThreads && pOther->GetThreadId()!=pSelected->GetThreadId())
	{
		ASSERT(bUp?pSelected->GetThreadId()>0:pSelected->GetThreadId()<m_nThreadsCurrently-1);
		return IncreaseThread(nSelected,pSelected,bUp);
	}

	// This is working in this dialog! Wou
	LPARAM pParam=m_pList->GetItemData(nSelected);
	m_pList->SetItemData(nSelected,m_pList->GetItemData(nOther));
	m_pList->SetItemData(nOther,pParam);
	UINT nState=m_pList->GetItemState(nSelected,0xFFFFFFFF);
	m_pList->SetItemState(nSelected,m_pList->GetItemState(nOther,0xFFFFFFFF),0xFFFFFFFF);
	m_pList->SetItemState(nOther,nState,0xFFFFFFFF);

	m_pList->EnsureVisible(nOther,FALSE);
	m_pList->RedrawItems(min(nSelected,nOther),max(nSelected,nOther));

	m_pList->UpdateWindow();
	
	EnableButtons();
	m_pList->SetFocus();
	return TRUE;
}	

BOOL CSelectDatabasesDlg::InsertDatabases()
{
	m_pList->DeleteAllItems();

	// Counting highest thread
	if (m_bFlags&flagShowThreads)
	{
		WORD wHighestThread=0;
		for (int i=0;i<m_rOrigDatabases.GetSize();i++)
		{
			if (wHighestThread<m_rOrigDatabases[i]->GetThreadId())
				wHighestThread=m_rOrigDatabases[i]->GetThreadId();
		}
		ChangeNumberOfThreads(wHighestThread+1);
		SendDlgItemMessage(IDC_THREADSPIN,UDM_SETPOS,0,MAKELONG(m_nThreadsCurrently,0));
	}
	
	

	// Checking whether groups should be taken care of
	LVITEM li;
	li.pszText=LPSTR_TEXTCALLBACK;
	if (m_bFlags&flagShowThreads && m_nThreadsCurrently>1 && ((CLocateApp*)GetApp())->m_wComCtrlVersion>=0x0600)
        li.mask=LVIF_TEXT|LVIF_PARAM|LVIF_GROUPID;
	else
		li.mask=LVIF_TEXT|LVIF_PARAM;
	li.iSubItem=0;
	
	for (li.iItem=0;li.iItem<m_rOrigDatabases.GetSize();li.iItem++)
	{
		CDatabase* pDatabase=new CDatabase(*m_rOrigDatabases[li.iItem]);
		li.lParam=LPARAM(pDatabase);
		if (m_bFlags&flagShowThreads)
			li.iGroupId=pDatabase->GetThreadId();
		else
		{
			li.iGroupId=0;
			pDatabase->SetThreadId(0);
		}

		m_pList->SetCheckState(m_pList->InsertItem(&li),IsItemEnabled(pDatabase));
	}

	return TRUE;
}

BOOL CSelectDatabasesDlg::InsertSelected()
{
	if (m_pSelectDatabases==NULL)
		return FALSE;

	InsertDatabases();

	int nItem=m_pList->GetNextItem(-1,LVNI_ALL);
	
	while (nItem!=-1)
	{
		CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(nItem);
		if (pDatabase!=NULL)
		{
			BOOL bFound=FALSE;
			LPSTR pPtr=m_pSelectDatabases;
			while (*pPtr!='\0')
			{
				int iStrLen=istrlen(pPtr)+1;
				if (strncmp(pPtr,pDatabase->GetName(),iStrLen)==0)
				{
					bFound=TRUE;
					break;
				}
				pPtr+=iStrLen;
			}
			EnableItem(pDatabase,bFound);
			m_pList->SetCheckState(nItem,bFound);
		}

		nItem=m_pList->GetNextItem(nItem,LVNI_ALL);
	}
	return TRUE;
}
	

BOOL CSelectDatabasesDlg::InsertDatabases(WORD wCount,WORD wThreads,const WORD* pwDatabaseIDs,const WORD* pwThreads,
										  WORD wSelectedCount,const WORD* pwSelectedIds)
{
	m_pList->DeleteAllItems();

	ChangeNumberOfThreads(wThreads);
	SendDlgItemMessage(IDC_THREADSPIN,UDM_SETPOS,0,MAKELONG(m_nThreadsCurrently,0));

	// Handled dbs
	BYTE* pTaken=new BYTE[max(m_rOrigDatabases.GetSize(),2)];
	ZeroMemory(pTaken,m_rOrigDatabases.GetSize());
    
	LVITEM li;
	li.pszText=LPSTR_TEXTCALLBACK;
	if (m_bFlags&flagShowThreads && m_nThreadsCurrently>1 && ((CLocateApp*)GetApp())->m_wComCtrlVersion>=0x0600)
        li.mask=LVIF_TEXT|LVIF_PARAM|LVIF_GROUPID;
	else
		li.mask=LVIF_TEXT|LVIF_PARAM;
	li.iSubItem=0;
	li.iItem=0;

	for (int i=0;i<wCount;i++)
	{
		CDatabase* pDatabase=NULL;

		// Searching database
		for (int j=0;j<m_rOrigDatabases.GetSize();j++)
		{
			if (m_rOrigDatabases[j]->GetID()==pwDatabaseIDs[i])
			{
				pDatabase=new CDatabase(*m_rOrigDatabases[j]);
				pTaken[j]=TRUE;
				break;
			}
		}

		if (pDatabase==NULL)
			continue; // Deleted database

		// Inserting item
		li.lParam=LPARAM(pDatabase);
		if (m_bFlags&flagShowThreads)
		{
			li.iGroupId=pwThreads[i];
			pDatabase->SetThreadId(pwThreads[i]);
		}
		else
		{
			li.iGroupId=0;
			pDatabase->SetThreadId(0);
		}
		li.iItem=m_pList->InsertItem(&li);

		BOOL bSelected=FALSE;
		for (int j=0;j<wSelectedCount;j++)
		{
			if (pDatabase->GetID()==pwSelectedIds[j])
			{
				bSelected=TRUE;
				break;
			}
		}

		EnableItem(pDatabase,bSelected);
		m_pList->SetCheckState(li.iItem,bSelected);
		li.iItem++;
	}


	// Inserting databases which are not eat inserted
	for (int i=0;i<m_rOrigDatabases.GetSize();i++)
	{
		if (pTaken[i]==0)
		{
			CDatabase* pDatabase=new CDatabase(*m_rOrigDatabases[i]);
			
			// Inserting item
			li.lParam=LPARAM(pDatabase);
			if (m_bFlags&flagShowThreads)
				li.iGroupId=pDatabase->GetThreadId();
			else
			{
				li.iGroupId=0;
				pDatabase->SetThreadId(0);
			}
			li.iItem=m_pList->InsertItem(&li);

			EnableItem(pDatabase,FALSE);
			m_pList->SetCheckState(li.iItem,FALSE);
			li.iItem++;
		}
	}

	return TRUE;
}


void CSelectDatabasesDlg::EnableThreadGroups(int nThreadGroups)
{
	if (m_pList->IsGroupViewEnabled())
		return;

	m_pList->EnableGroupView(TRUE);
	m_nThreadsCurrently=nThreadGroups;
	
	// Creating groups
	CStringW str;
	LVGROUP lg;
	dMemSet(&lg,0,sizeof(LVGROUP));
	lg.cbSize=sizeof(LVGROUP);
	lg.mask=LVGF_HEADER|LVGF_GROUPID|LVGF_ALIGN;
	lg.state=LVGS_NORMAL;
	lg.uAlign=LVGA_HEADER_LEFT;

	for (lg.iGroupId=0;lg.iGroupId<nThreadGroups;lg.iGroupId++)
	{
		str.Format(IDS_THREADNAME,lg.iGroupId+1);
		lg.pszHeader=str.GetBuffer();

		m_pList->InsertGroup(lg.iGroupId,&lg);
	}

	// Setting groups IDs
	LVITEM li;
	li.mask=LVIF_GROUPID;
	li.iItem=m_pList->GetNextItem(-1,LVNI_ALL);
	li.iSubItem=0;
	while (li.iItem!=-1)
	{
		CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(li.iItem);
		ASSERT(pDatabase!=NULL);

		li.iGroupId=pDatabase->GetThreadId();
		
		m_pList->SetItem(&li);
		li.iItem=m_pList->GetNextItem(li.iItem,LVNI_ALL);
	}

}

void CSelectDatabasesDlg::RemoveThreadGroups()
{
	if (!m_pList->IsGroupViewEnabled())
		return;

	m_pList->RemoveAllGroups();
	m_pList->EnableGroupView(FALSE);
}

void CSelectDatabasesDlg::ChangeNumberOfThreads(int nThreads)
{
	if (nThreads>1 && !(m_bFlags&flagShowThreads))
		return;
	
	if (nThreads>m_nThreadsCurrently)
	{
		// Number is increased
		if (((CLocateApp*)GetApp())->m_wComCtrlVersion<0x0600)
		{
			m_nThreadsCurrently=nThreads;
			return;
		}

		if (m_nThreadsCurrently==1)
			EnableThreadGroups(nThreads);
		else
		{
			// Insertig new thread groups
			CStringW str;
			LVGROUP lg;
			dMemSet(&lg,0,sizeof(LVGROUP));
			lg.cbSize=sizeof(LVGROUP);
			lg.mask=LVGF_HEADER|LVGF_GROUPID|LVGF_ALIGN;
			lg.state=LVGS_NORMAL;
			lg.uAlign=LVGA_HEADER_LEFT;

			for (lg.iGroupId=m_nThreadsCurrently;lg.iGroupId<nThreads;lg.iGroupId++)
			{
				str.Format(IDS_THREADNAME,lg.iGroupId+1);
				lg.pszHeader=str.GetBuffer();

				m_pList->InsertGroup(lg.iGroupId,&lg);
			}
		
		}
		m_nThreadsCurrently=nThreads;
		m_pList->RedrawItems(0,m_pList->GetItemCount());
	}
	else if (nThreads<m_nThreadsCurrently)
	{
		// Ensuring that there is no any items with higher thread ID than available
		int nItem=m_pList->GetNextItem(-1,LVNI_ALL);
		while (nItem!=-1)
		{
			CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(nItem);
			ASSERT(pDatabase!=NULL);

			if (pDatabase->GetThreadId()>=nThreads)
			{
				pDatabase->SetThreadId(nThreads-1);
				
				LVITEM li;
				li.iItem=nItem;
				li.iSubItem=0;
				li.mask=LVIF_GROUPID;
				li.iGroupId=nThreads-1;
				m_pList->SetItem(&li);
			}
			nItem=m_pList->GetNextItem(nItem,LVNI_ALL);
		}


		if (((CLocateApp*)GetApp())->m_wComCtrlVersion>=0x0600)
		{
			if (nThreads==1)
				RemoveThreadGroups();
			else
			{
				// Removing unused thread groups
				while (m_nThreadsCurrently>nThreads)
					m_pList->RemoveGroup(--m_nThreadsCurrently);
			}
		}
		m_nThreadsCurrently=nThreads;

		m_pList->RedrawItems(0,m_pList->GetItemCount());
	}
}


BOOL CSelectDatabasesDlg::IncreaseThread(int nItem,CDatabase* pDatabase,BOOL bDecrease)
{
	if ((bDecrease && pDatabase->GetThreadId()<1) || 
		(!bDecrease && pDatabase->GetThreadId()>=m_nThreadsCurrently-1))
		return FALSE;

	pDatabase->SetThreadId(pDatabase->GetThreadId()+(bDecrease?-1:1));

	if (GetLocateApp()->m_wComCtrlVersion>=0x0600)
	{
		LVITEM li;
		li.mask=LVIF_GROUPID;
		li.iItem=nItem;
		li.iSubItem=0;
		li.iGroupId=pDatabase->GetThreadId();
		m_pList->SetItem(&li);
	}
	else
		m_pList->RedrawItems(nItem,nItem);
	
	m_pList->EnsureVisible(nItem,FALSE);
	m_pList->SetFocus();
	EnableButtons();
	return TRUE;
}

void CSelectDatabasesDlg::EnableButtons()
{
	int nSelectedItem=m_pList->GetNextItem(-1,LVNI_SELECTED);
	
	if (nSelectedItem!=-1)
	{
		CDatabase* pDatabase=(CDatabase*)m_pList->GetItemData(nSelectedItem);
		ASSERT(pDatabase!=NULL);

		// Checking item above
		int nAnother=m_pList->GetNextItem(nSelectedItem,LVNI_ABOVE);
		if (nAnother==-1 || nAnother==nSelectedItem)
			EnableDlgItem(IDC_UP,pDatabase->GetThreadId()>0);
		else
			EnableDlgItem(IDC_UP,TRUE);
		
		// Checking item below
		nAnother=m_pList->GetNextItem(nSelectedItem,LVNI_BELOW);
		if (nAnother==-1 || nAnother==nSelectedItem)
			EnableDlgItem(IDC_DOWN,pDatabase->GetThreadId()<m_nThreadsCurrently-1); 
		else
			EnableDlgItem(IDC_DOWN,TRUE);
	}
	else
	{
		EnableDlgItem(IDC_UP,FALSE);
		EnableDlgItem(IDC_DOWN,FALSE);
	}
}


void CSelectDatabasesDlg::LoadPresets()
{
	SendDlgItemMessage(IDC_PRESETS,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_PRESETCUSTOM));
	SendDlgItemMessage(IDC_PRESETS,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_PRESETGLOBAL));
	SendDlgItemMessage(IDC_PRESETS,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_PRESETLATEST));
	if (m_pSelectDatabases!=NULL)
		SendDlgItemMessage(IDC_PRESETS,CB_ADDSTRING,0,(LPARAM)(LPCSTR)CString(IDS_PRESETSELECTED));
	
	if (m_bFlags&flagDisablePresets)
	{
		SendDlgItemMessage(IDC_PRESETS,CB_SETCURSEL,CUSTOM_PRESET);
		return;
	}

	
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,m_pRegKey,CRegKey::openExist|CRegKey::samRead|CRegKey::samQueryValue)==ERROR_SUCCESS)
	{
		CString sName;
		for (int i=0;RegKey.EnumValue(i,sName)>0;i++)
		{
			if (strncmp(sName,"Preset ",7)==0)
			{
				DWORD nIndex=sName.FindFirst(':')+1;
				
				if (nIndex>0 &&  nIndex<sName.GetLength()) 
					SendDlgItemMessage(IDC_PRESETS,CB_ADDSTRING,0,(LPARAM)(LPCSTR(sName)+nIndex));
			}
		}
	}

	// Leading previous preset
	if (InsertSelected())
		SendDlgItemMessage(IDC_PRESETS,CB_SETCURSEL,SELECTED_PRESET);
	else if (m_bFlags&flagGlobalIsSelected)
	{
		InsertDatabases();
		SendDlgItemMessage(IDC_PRESETS,CB_SETCURSEL,GLOBAL_PRESET);
	}
	else if (LoadPreset(NULL))
		SendDlgItemMessage(IDC_PRESETS,CB_SETCURSEL,LATEST_PRESET);
	else
	{
		InsertDatabases();
		SendDlgItemMessage(IDC_PRESETS,CB_SETCURSEL,GLOBAL_PRESET);
	}
}

BOOL CSelectDatabasesDlg::SavePreset(LPCSTR szName)
{
	
	// First, check whether name already exists
	int iOverwriteItem=-1;

	if (szName!=NULL)
	{
		if (szName[0]=='\0' ||  
			strcmp(szName,"Database List Widths")==0 || 
			strcmp(szName,"LastPreset")==0)
		{
			CString str;
			str.Format(IDS_PRESETNAMENOTVALID,szName);
			MessageBox(str,CString(IDS_ERROR),MB_OK|MB_ICONERROR);
			return FALSE;
		}

	
		// First item is global, second is last
		for (int i=SendDlgItemMessage(IDC_PRESETS,CB_GETCOUNT)-1;i>=2;i--)
		{
			int nItemTextLength=SendDlgItemMessage(IDC_PRESETS,CB_GETLBTEXTLEN,i);
			if (nItemTextLength==CB_ERR)
				continue;

            LPCSTR pItemText=new char[nItemTextLength+1];
            if (SendDlgItemMessage(IDC_PRESETS,CB_GETLBTEXT,i,LPARAM(pItemText))!=CB_ERR)
			{
				if (strcasecmp(pItemText,szName)==0)
				{
					CString str;
					str.Format(IDS_OVERWRITEPRESET,szName);
					if (MessageBox(str,CString(IDS_PRESETSAVETITLE),MB_YESNO)==IDNO)
						return FALSE;
					
					iOverwriteItem=i-(m_pSelectDatabases!=NULL?4:3);
					break;
				}
			}
			delete[] pItemText;
		}
	}


	CRegKey RegKey;
	if(RegKey.OpenKey(HKCU,m_pRegKey,CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
	{
		// Registry value constains
		// wNumberOfDatabases (WORD)
		// wNumberOfThreads (WORD)
		// wNumberOfSelectedDbs (WORD)
		// database ID (WORD)
		// ..
		// database ID (WORD)
		// thread ID (WORD)
		// ..
		// thread ID (WORD)
		// selected DB (WORD)
		// ...
		// selected DB (WORD)
		
		// Get enabled databases
		WORD* pThreads=new WORD[m_pList->GetItemCount()];
		WORD* pIDs=new WORD[m_pList->GetItemCount()];
        CWordArray aSelected;

		int nDatabases=0;
		// Get the first item
		int nNext;
		int nItem=m_pList->GetNextItem(-1,LVNI_ALL);
		while ((nNext=m_pList->GetNextItem(nItem,LVNI_ABOVE))!=-1)
		{
			if (nNext==nItem)
				break; // This should not be like that, why is it?
			nItem=nNext;
		}
		// Now we have top index
		while (nItem!=-1)
		{
			PDATABASE pDatabase=(PDATABASE)m_pList->GetItemData(nItem);
			if (pDatabase!=NULL)
			{
				pIDs[nDatabases]=pDatabase->GetID();
				pThreads[nDatabases]=pDatabase->GetThreadId();
				nDatabases++;

				if (IsItemEnabled(pDatabase))
					aSelected.Add(pDatabase->GetID());
			}

			nNext=m_pList->GetNextItem(nItem,LVNI_BELOW);
			if (nNext==nItem)
				break;
			nItem=nNext;
		}

		DWORD dwLength=sizeof(WORD)*(3+2*nDatabases+aSelected.GetSize());
		char* pData=new char [dwLength];
		WORD* pPtr=(WORD*)pData;
		*(pPtr++)=nDatabases;
		*(pPtr++)=m_nThreadsCurrently;
		*(pPtr++)=aSelected.GetSize();

		CopyMemory(pPtr,pIDs,nDatabases*sizeof(WORD));
		pPtr+=nDatabases;
		CopyMemory(pPtr,pThreads,nDatabases*sizeof(WORD));
		pPtr+=nDatabases;
		CopyMemory(pPtr,aSelected.GetData(),aSelected.GetSize()*sizeof(WORD));
		
		if (szName!=NULL)
		{
			char* szText=new char[15+istrlen(szName)];
			if (iOverwriteItem>=0)
				wsprintf(szText,"Preset %03d:%s",iOverwriteItem,szName);
			else
				wsprintf(szText,"Preset %03d:%s",SendDlgItemMessage(IDC_PRESETS,CB_GETCOUNT)-2,szName);

			RegKey.SetValue(szText,pData,dwLength,REG_BINARY);
		}
		else
			RegKey.SetValue("LastPreset",pData,dwLength,REG_BINARY);
        delete[] pData;
		delete[] pIDs;
		delete[] pThreads;
	}

	if (szName!=NULL)
	{
        if (iOverwriteItem==-1)
			iOverwriteItem=SendDlgItemMessage(IDC_PRESETS,CB_ADDSTRING,0,LPARAM(szName));
		else
		{
			iOverwriteItem+=m_pSelectDatabases!=NULL?4:3;
			SendDlgItemMessage(IDC_PRESETS,CB_SETITEMDATA,iOverwriteItem,LPARAM(szName));
		}

		if (iOverwriteItem!=-1)
			SendDlgItemMessage(IDC_PRESETS,CB_SETCURSEL,iOverwriteItem);
			
	}

	return TRUE;
}

BOOL CSelectDatabasesDlg::LoadPreset(LPCSTR szName)
{
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,m_pRegKey,CRegKey::openExist|CRegKey::samRead|CRegKey::samQueryValue)!=ERROR_SUCCESS)
		return FALSE;

	CString sName;
	if (szName!=NULL)
	{
		BOOL bFound=FALSE;
		for (int i=0;RegKey.EnumValue(i,sName)>0;i++)
		{
			if (strncmp(sName,"Preset ",7)==0)
			{
				DWORD nIndex=sName.FindFirst(':')+1;
				
				if (nIndex>0 &&  nIndex<sName.GetLength()) 
				{
					if (strcasecmp(LPCSTR(sName)+nIndex,szName)==0)
					{
						bFound=TRUE;
						break;
					}
				}
			}
		}
		if (!bFound)
			return FALSE;
	}
	else
		sName="LastPreset";

	DWORD dwLength=RegKey.QueryValueLength(sName);
    if (dwLength<2 || dwLength%2==1)
		return FALSE;

	WORD* pData=new WORD[dwLength>>1];
	if (RegKey.QueryValue(sName,(LPSTR)pData,dwLength))
	{	
		WORD wDatabases=pData[0];
		if (sizeof(WORD)*(3+2*wDatabases+pData[2])!=dwLength)
			return FALSE;

		WORD* pDatabaseIDs=pData+3;
		WORD* pThreadsIDs=pDatabaseIDs+wDatabases;
		WORD* pSelectedIDs=pThreadsIDs+wDatabases;

		InsertDatabases(wDatabases,pData[1],pDatabaseIDs,pThreadsIDs,pData[2],pSelectedIDs);
	}

	delete[] pData;
	return TRUE;
}	

///////////////////////////////////////////////////////////
// CSelectDatabasesDlg::CPresetNameDlg

BOOL CSavePresetDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	CenterWindow();
	return FALSE;
}

BOOL CSavePresetDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);
	switch (wID)
	{
	case IDC_OK:
		OnOK();
		break;
	case IDCANCEL:
	case IDC_CANCEL:
		OnCancel();
		break;
	case IDC_EDIT:
		if (wNotifyCode==EN_SETFOCUS)
			::SendMessage(hControl,EM_SETSEL,0,MAKELPARAM(0,-1));
		break;
	}
	return FALSE;
}

void CSavePresetDlg::OnOK()
{
    GetDlgItemText(IDC_EDIT,m_sReturnedPreset);

	if (m_sReturnedPreset.IsEmpty())
	{
		CString msg;
		msg.Format(IDS_PRESETNAMENOTVALID,LPCSTR(m_sReturnedPreset));

		MessageBox(msg,CString(IDS_PRESETSAVETITLE),MB_OK|MB_ICONEXCLAMATION);
		SetFocus(IDC_EDIT);
	}
	else
		EndDialog(1);
}

void CSavePresetDlg::OnCancel()
{
	EndDialog(0);
}

///////////////////////////////////////////////////////////
// CSelectDatabasesDlg::CSavePresetDlg

void CSelectDatabasesDlg::CSavePresetDlg::OnOK()
{
	CString sName;
	
    GetDlgItemText(IDC_EDIT,sName);
	
	if (m_pParent->SavePreset(sName))
		EndDialog(1);
}

///////////////////////////////////////////////////////////
// CChangeCaseDlg

BOOL CChangeCaseDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	CenterWindow();
	switch (nSelectedCase)
	{
	case Sentence:
		CheckDlgButton(IDC_SENTENCECASE,TRUE);
		break;
	case Uppercase:
		CheckDlgButton(IDC_UPPERCASE,TRUE);
		break;
	case Lowercase:
		CheckDlgButton(IDC_LOWERCASE,TRUE);
		break;
	case Title:
		CheckDlgButton(IDC_TITLECASE,TRUE);
		break;
	case Toggle:
		CheckDlgButton(IDC_TOGGLECASE,TRUE);
		break;
	}
	CheckDlgButton(IDC_EXTENSIONS,bForExtension);
	return FALSE;
}

BOOL CChangeCaseDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch (wID)
	{
	case IDC_OK:
		if (IsDlgButtonChecked(IDC_SENTENCECASE))
			nSelectedCase=Sentence;
		else if (IsDlgButtonChecked(IDC_LOWERCASE))
			nSelectedCase=Lowercase;
		else if (IsDlgButtonChecked(IDC_UPPERCASE))
			nSelectedCase=Uppercase;
		else if (IsDlgButtonChecked(IDC_TITLECASE))
			nSelectedCase=Title;
		else if (IsDlgButtonChecked(IDC_TOGGLECASE))
			nSelectedCase=Toggle;
		
		bForExtension=IsDlgButtonChecked(IDC_EXTENSIONS);

		EndDialog(1);
		break;
	case IDCANCEL:
	case IDC_CANCEL:
		EndDialog(0);
		break;
	}
	return CDialog::OnCommand(wID,wNotifyCode,hControl);
}

BOOL CChangeCaseDlg::OnClose()
{
	CDialog::OnClose();
	EndDialog(0);
	return FALSE;
}

///////////////////////////////////////////////////////////
// CChangeFilenameDlg

BOOL CChangeFilenameDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	SetDlgItemText(IDC_EDIT,m_sFileName);
	CenterWindow();
	return FALSE;
}

BOOL CChangeFilenameDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch (wID)
	{
	case IDC_OK:
		if (GetDlgItemText(IDC_EDIT,m_sFileName)>0)
		{
			if (!CFile::IsValidFileName(m_sFileName))
			{
				CString msg;
				msg.Format(IDS_INVALIDFILENAME,(LPCSTR)m_sFileName);
				MessageBox(msg,CString(IDS_DATABASESETTINGS),MB_OK|MB_ICONINFORMATION);
                break;
			}
			EndDialog(1);
		}
		break;
	case IDCANCEL:
	case IDC_CANCEL:
		EndDialog(0);
		break;
	case IDC_EDIT:
		if (wNotifyCode==EN_SETFOCUS)
			::SendMessage(hControl,EM_SETSEL,0,-1);
		break;
	}
	return CDialog::OnCommand(wID,wNotifyCode,hControl);
}

BOOL CChangeFilenameDlg::OnClose()
{
	CDialog::OnClose();
	EndDialog(0);
	return FALSE;
}

///////////////////////////////////////////////////////////
// CRemovePresetDlg


BOOL CRemovePresetDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	CenterWindow();


	// First, find indentifiers
	CRegKey RegKey;
	CString Path;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Dialogs\\SearchPresets";
	if (RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
		return FALSE;
	
	
	CRegKey RegKey2;
	char szBuffer[30];

	for (int nPreset=0;nPreset<1000;nPreset++)
	{
		wsprintf(szBuffer,"Preset %03d",nPreset);

		if (RegKey2.OpenKey(RegKey,szBuffer,CRegKey::openExist|CRegKey::samRead)!=ERROR_SUCCESS)
			break;

		CString sCurrentName;
		RegKey2.QueryValue("",sCurrentName);

		SendDlgItemMessage(IDC_PRESETS,CB_ADDSTRING,0,(LPARAM)(LPCSTR)sCurrentName);		

		RegKey2.CloseKey();
	}		

	// Choosing first
	SendDlgItemMessage(IDC_PRESETS,CB_SETCURSEL,0,0);		
	return FALSE;
}

BOOL CRemovePresetDlg::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	CDialog::OnCommand(wID,wNotifyCode,hControl);
	switch (wID)
	{
	case IDCANCEL:
	case IDC_OK:
		OnOK();
		break;
	case IDC_CANCEL:
		EndDialog(0);
		break;
	case IDC_EDIT:
		if (wNotifyCode==EN_SETFOCUS)
			::SendMessage(hControl,EM_SETSEL,0,MAKELPARAM(0,-1));
		break;
	}
	return FALSE;
}


BOOL CRemovePresetDlg::OnClose()
{
	CDialog::OnClose();
	EndDialog(0);
	return FALSE;
}




void CRemovePresetDlg::OnOK()
{
	int nSelection=SendDlgItemMessage(IDC_PRESETS,CB_GETCURSEL);
	if (nSelection==CB_ERR)
		return;

	CRegKey RegKey;
	CString Path;
	Path.LoadString(IDS_REGPLACE,CommonResource);
	Path<<"\\Dialogs\\SearchPresets";
	
	LONG lErr=RegKey.OpenKey(HKCU,Path,CRegKey::openExist|CRegKey::samAll);
	if (lErr!=ERROR_SUCCESS)
	{
		ReportSystemError(NULL,lErr,0);
		return;
	}
	
	char szKeyName[30];
	wsprintf(szKeyName,"Preset %03d",nSelection);

	if ((lErr=RegKey.DeleteKey(szKeyName))!=ERROR_SUCCESS)
	{
		ReportSystemError(NULL,lErr,0);
		return;
	}

    for (nSelection++;;nSelection++)
	{
		char szRenameKey[30];
		wsprintf(szRenameKey,"Preset %03d",nSelection);
		
		if (RegKey.RenameSubKey(szRenameKey,szKeyName)!=ERROR_SUCCESS)
			break;
	
		strcpy(szKeyName,szRenameKey);
	}

	EndDialog(1);
}
