#include <HFCLib.h>
#include "Locate32.h"

CDatabaseInfos::CDatabaseInfos(HWND hParent)
:	CPropertySheet(IDS_DATABASEINFOCAPTION,hParent,0)
{
	const CArray<PDATABASE>& rDatabases=GetLocateApp()->GetDatabases();

	for (int i=0;i<rDatabases.GetSize();i++)
	{
		CDatabaseInfoPage* pPage=new CDatabaseInfoPage(rDatabases[i]);
		
		m_aInfoPages.Add(pPage);
		AddPage((CPropertyPage*)pPage);
	}

	m_psh.dwFlags|=PSH_NOAPPLYNOW|PSH_NOCONTEXTHELP;
}

BOOL CDatabaseInfos::CDatabaseInfoPage::OnCommand(WORD wID,WORD wNotifyCode,HWND hControl)
{
	switch(wID)
	{
	case IDC_OK:
		EndDialog(0);
		break;
	}
	return CDialog::OnCommand(wID,wNotifyCode,hControl);
}

BOOL CDatabaseInfos::CDatabaseInfoPage::OnInitDialog(HWND hwndFocus)
{
	CString Drives,temp;
	
	SetDlgItemText(IDC_FILE,m_pDatabase->GetArchiveName());

	
	if (m_pDatabase->DoDatabaseFileExist())
	{
		CDatabaseInfo* di=CDatabaseInfo::GetFromDatabase(m_pDatabase);
		
		//Resolving information
		if (di==NULL)
		{
			SendDlgItemMessage(IDC_DATABASEFILEICON,STM_SETICON,(WPARAM)LoadIcon(IDI_INVALIDDBFILE),0);
		

			temp.LoadString(IDS_DBNOTVALID);
			SetDlgItemText(IDC_CREATED,temp);
			temp.LoadString(IDS_UNKNOWN);
			SetDlgItemText(IDC_CREATOR,temp);
			SetDlgItemText(IDC_DESCRIPTION,temp);
			SetDlgItemText(IDC_FILESIZE,temp);
			SetDlgItemText(IDC_VERSION,temp);
			SetDlgItemText(IDC_FILES,temp);
			SetDlgItemText(IDC_DIRECTORIES,temp);
			return CDialog::OnInitDialog(hwndFocus);
		}
	
		// Setting file icon
		SendDlgItemMessage(IDC_DATABASEFILEICON,STM_SETICON,(WPARAM)LoadIcon(IDI_DBFILE),0);
		
		// Initializing list control
		m_pList=new CListCtrl(GetDlgItem(IDC_FOLDERS));
		CLocateDlg::SetSystemImagelists(m_pList);
		m_pList->SetExtendedListViewStyle(LVS_EX_HEADERDRAGDROP,LVS_EX_HEADERDRAGDROP);
	
		if (di->bVersion>=20)
		{
			m_pList->InsertColumn(0,CString(IDS_VOLUMEPATH),LVCFMT_LEFT,100,0);
			m_pList->InsertColumn(1,CString(IDS_VOLUMETYPE),LVCFMT_LEFT,50,1);
			m_pList->InsertColumn(2,CString(IDS_VOLUMELABEL),LVCFMT_LEFT,80,2);
			m_pList->InsertColumn(3,CString(IDS_VOLUMESERIAL),LVCFMT_LEFT,70,3);
			m_pList->InsertColumn(4,CString(IDS_VOLUMEFILESYSTEM),LVCFMT_LEFT,55,4);
			m_pList->InsertColumn(5,CString(IDS_VOLUMEFILES),LVCFMT_RIGHT,50,5);
			m_pList->InsertColumn(6,CString(IDS_VOLUMEDIRECTORIES),LVCFMT_RIGHT,50,6);
		}
		else
		{
			m_pList->InsertColumn(0,CString(IDS_VOLUMEPATH),LVCFMT_LEFT,300,0);
			m_bOldDB=TRUE;
		}

		CString str("Database Info List Widths for ");
		str << m_pDatabase->GetName();
		m_pList->LoadColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs",str);
		
		//Settting creator and description
		SetDlgItemText(IDC_CREATOR,di->sCreator);
		SetDlgItemText(IDC_DESCRIPTION,di->sDescription);
		
		//Setting created date
		if (di->tCreationTime.m_time>0)
		{
			SYSTEMTIME st=di->tCreationTime;
			int nTemp=GetDateFormat(LOCALE_USER_DEFAULT,DATE_SHORTDATE,&st,NULL,temp.GetBuffer(200),200);
			if (nTemp>=0)
			{
				GetTimeFormat(LOCALE_USER_DEFAULT,0,&st,NULL,temp.GetBuffer()+nTemp,200-nTemp);
				temp[nTemp-1]=' ';
				SetDlgItemText(IDC_CREATED,temp);
			}		
			else
			{
				temp.LoadString(IDS_UNKNOWN);
				SetDlgItemText(IDC_CREATED,temp);
			}
		}
		else
		{
			temp.LoadString(IDS_UNKNOWN);
			SetDlgItemText(IDC_CREATED,temp);
		}
		
		// Setting filesize
		if (di->dwFileSize>50000)
		{
			temp=di->dwFileSize/1024;
			temp.AddString(IDS_KB);
		}
		else
		{
			temp=di->dwFileSize;
			temp.AddString(IDS_BYTES);
		}
		SetDlgItemText(IDC_FILESIZE,temp);
		
		// Setting number of files and directories
		if (di->dwNumberOfDirectories!=(DWORD)-1)
			temp=di->dwNumberOfFiles;
		else
			temp.LoadString(IDS_UNKNOWN);
		SetDlgItemText(IDC_FILES,temp);
		if (di->dwNumberOfDirectories!=(DWORD)-1)
			temp=di->dwNumberOfDirectories;
		else
			temp.LoadString(IDS_UNKNOWN);
		SetDlgItemText(IDC_DIRECTORIES,temp);
		
		// Setting version
		switch (di->bVersion)
		{
		case 1:
			temp.Format(IDS_DBVERSION1,di->bVersion);
			break;
		case 2:
			temp.Format(IDS_DBVERSION2,di->bVersion);
			break;
		case 3:
			temp.Format(IDS_DBVERSION3,di->bVersion);
			break;
		case 4:
			temp.Format(IDS_DBVERSION4,di->bVersion);
			break;
		default:
			temp.Format(IDS_DBVERSIONNEW,di->bVersion,
				di->bLongFilenames?(LPCSTR)CString(IDS_YES):(LPCSTR)CString(IDS_NO),
				di->bAnsi?(LPCSTR)CString(IDS_NO):(LPCSTR)CString(IDS_YES));
			break;
		}
		SetDlgItemText(IDC_VERSION,temp);
		
		// Setting drive/folder information
		for (int i=0;i<di->aRootFolders.GetSize();i++)
		{
			CString Temp;

			switch (di->aRootFolders.GetAt(i)->rtType)
			{
			case CDatabaseInfo::Unknown:
				Temp.LoadString(IDS_VOLUMETYPEUNKNOWN);
				break;
			case CDatabaseInfo::Fixed:
				Temp.LoadString(IDS_VOLUMETYPEFIXED);
				break;
			case CDatabaseInfo::Removable:
				Temp.LoadString(IDS_VOLUMETYPEREMOVABLE);
				break;
			case CDatabaseInfo::CDRom:
				Temp.LoadString(IDS_VOLUMETYPECDROM);
				break;
			case CDatabaseInfo::Remote:
				Temp.LoadString(IDS_VOLUMETYPEREMOTE);
				break;
			case CDatabaseInfo::Ramdisk:
				Temp.LoadString(IDS_VOLUMETYPERAMDISK);
				break;
			case CDatabaseInfo::Directory:
				Temp.LoadString(IDS_VOLUMETYPEDIRECTORY);
				break;
			}

			LVITEM li;
			li.mask=LVIF_TEXT|LVIF_IMAGE;
			li.iItem=i;
			SHFILEINFO fi;
			if (SHGetFileInfo(di->aRootFolders.GetAt(i)->sPath+'\\',0,&fi,sizeof(SHFILEINFO),SHGFI_SMALLICON|SHGFI_SYSICONINDEX))
				li.iImage=fi.iIcon;
			else
				li.iImage=DEL_IMAGE;
			li.iSubItem=0;
			li.pszText=di->aRootFolders.GetAt(i)->sPath.GetBuffer();
			m_pList->InsertItem(&li);
			
			if (!m_bOldDB)
			{
				li.mask=LVIF_TEXT;
				li.iSubItem=1;
				li.pszText=Temp.GetBuffer();
				m_pList->SetItem(&li);

				li.iSubItem=2;
				li.pszText=di->aRootFolders.GetAt(i)->sVolumeName.GetBuffer();
				m_pList->SetItem(&li);

				li.iSubItem=3;
				Temp.Format("%0X-%0X",HIWORD(di->aRootFolders.GetAt(i)->dwVolumeSerial),
					LOWORD(di->aRootFolders.GetAt(i)->dwVolumeSerial));
				li.pszText=Temp.GetBuffer();
				m_pList->SetItem(&li);

				li.iSubItem=4;
				li.pszText=di->aRootFolders.GetAt(i)->sFileSystem.GetBuffer();
				m_pList->SetItem(&li);

				li.iSubItem=5;
				Temp=di->aRootFolders.GetAt(i)->dwNumberOfFiles;
				li.pszText=Temp.GetBuffer();
				m_pList->SetItem(&li);

				li.iSubItem=6;
				Temp=di->aRootFolders.GetAt(i)->dwNumberOfDirectories;
				li.pszText=Temp.GetBuffer();
				m_pList->SetItem(&li);
			}			
		}
	
		delete di;
	}
	else
	{
		// Setting file icon
		SendDlgItemMessage(IDC_DATABASEFILEICON,STM_SETICON,(WPARAM)LoadIcon(IDI_DELETEDFILE),0);
		
		temp.LoadString(IDS_NOTEXIST);
		SetDlgItemText(IDC_CREATED,temp);
		temp.LoadString(IDS_UNKNOWN);
		SetDlgItemText(IDC_CREATOR,temp);
		SetDlgItemText(IDC_DESCRIPTION,temp);
		SetDlgItemText(IDC_FILESIZE,temp);
		SetDlgItemText(IDC_VERSION,temp);
		SetDlgItemText(IDC_FILES,temp);
		SetDlgItemText(IDC_DIRECTORIES,temp);
	}
	
	return CPropertyPage::OnInitDialog(hwndFocus);
}

void CDatabaseInfos::CDatabaseInfoPage::OnDestroy()
{
	if (m_pList!=NULL)
	{
		CString str("Database Info List Widths for ");
		str << m_pDatabase->GetName();

		m_pList->SaveColumnsState(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\Dialogs",str);

		delete m_pList;
		m_pList=NULL;
	}

	CDialog::OnDestroy();
}



