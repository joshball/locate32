#if !defined(LOCATEDLG_INL)
#define LOCATEDLG_INL

#if _MSC_VER >= 1000
#pragma once
#endif

inline CLocateDlg::CNameDlg::CNameDlg()
:	CDialog(IDD_NAME),m_nMaxBrowse(DEFAULT_NUMBEROFDIRECTORIES),
	m_pBrowse(NULL),m_pMultiDirs(NULL)
{
}

inline WORD CLocateDlg::CNameDlg::ComputeChecksumFromDir(LPCSTR szDir)
{				
	WORD wCheksum=0;
	for (int j=0;szDir[j]!='\0';j++)
		wCheksum+=WORD(szDir[j]*(j+1));
	return wCheksum;
}

inline CLocateDlg::CNameDlg::DirSelection::DirSelection(BYTE bSelected_)
:	nType(CLocateDlg::CNameDlg::NotSelected),pTitleOrDirectory(NULL),bSelected(bSelected_)
{
}

inline CLocateDlg::CNameDlg::DirSelection::~DirSelection()
{
	FreeData();
}

inline void CLocateDlg::CNameDlg::DirSelection::FreeData()
{
	if (pTitleOrDirectory!=NULL)
	{
		delete[] pTitleOrDirectory;
		pTitleOrDirectory=NULL;
	}
}

inline void CLocateDlg::CNameDlg::AddDirectoryToList(CArray<LPSTR>& aDirectories,LPCSTR szDirectory)
{
	for (int i=0;i<aDirectories.GetSize();i++)
	{
		if (strcmp(aDirectories[i],szDirectory)==0)
			return;
	}
	aDirectories.Add(alloccopy(szDirectory));
}

inline void CLocateDlg::CNameDlg::AddDirectoryToList(CArray<LPSTR>& aDirectories,LPCSTR szDirectory,DWORD dwLength)
{
	for (int i=0;i<aDirectories.GetSize();i++)
	{
		if (strncmp(aDirectories[i],szDirectory,dwLength)==0)
		{
			if (aDirectories[i][dwLength]=='\0')
				return;
		}
	}
	aDirectories.Add(alloccopy(szDirectory,dwLength));
}

inline void CLocateDlg::CNameDlg::AddDirectoryToList(CArray<LPSTR>& aDirectories,FREEDATA szDirectory)
{
	for (int i=0;i<aDirectories.GetSize();i++)
	{
		if (strcmp(aDirectories[i],LPCSTR(szDirectory))==0)
		{
			delete[] (LPSTR)szDirectory;
			return;
		}
	}
	aDirectories.Add(LPSTR(szDirectory));
}



inline CLocateDlg::CSizeDateDlg::CSizeDateDlg()
:	CDialog(IDD_SIZEDATE)
{
}

inline CLocateDlg::CAdvancedDlg::CAdvancedDlg()
:	CDialog(IDD_ADVANCED),m_hTypeUpdaterThread(NULL),m_hDefaultTypeIcon(NULL),m_dwFlags(0)
{
}

inline void CLocateDlg::ClearMenuVariables()
{
	if (m_pActiveContextMenu!=NULL)
	{
		m_pActiveContextMenu->Release();
		m_pActiveContextMenu=NULL;
	}
	if (m_hActivePopupMenu!=NULL)
	{
		FreeSendToMenuItems(m_hActivePopupMenu);
		DestroyMenu(m_hActivePopupMenu);
		m_hActivePopupMenu=NULL;
	}
}
	
inline BOOL CLocateDlg::IsSendToMenu(HMENU hMenu)
{
	UINT nID=GetMenuItemID(hMenu,0);
	return nID>=IDM_DEFSENDTOITEM && nID<IDM_DEFSENDTOITEM+1000;
}
	
inline BOOL CLocateDlg::CSizeDateDlg::LookOnlyFiles() const
{
	return IsDlgButtonChecked(IDC_CHECKMINIMUMSIZE) ||
		IsDlgButtonChecked(IDC_CHECKMAXIMUMSIZE);
}

inline CLocateDlgThread::CLocateDlgThread()
:	m_pLocate(NULL)
{
	m_bAutoDelete=TRUE;
}

inline CLocateDlg::ImageHandlerDll::ImageHandlerDll()
:	pGetImageDimensionsA(NULL),uToken(0)
{
	DWORD bLoadDll=TRUE;
	{
		CRegKey RegKey;
		if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource),CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
			RegKey.QueryValue("Load imagehandler",bLoadDll);
	}
	if (!bLoadDll)
	{
		hModule=NULL;
		return;
	}

	hModule=LoadLibrary("imghnd.dll");
	if (hModule!=NULL)
	{
		IH_INITLIBRARY pInitLibrary=(IH_INITLIBRARY)GetProcAddress(hModule,"InitLibrary");
		if (pInitLibrary==NULL)
		{
			FreeLibrary(hModule);
			hModule=NULL;
			return;
		}
		if (!pInitLibrary(&uToken))
		{
			FreeLibrary(hModule);
			hModule=NULL;
			return;
		}
		pGetImageDimensionsA=(IH_GETIMAGEDIMENSIONSA)GetProcAddress(hModule,"GetImageDimensionsA");
		if (pGetImageDimensionsA==NULL)
		{
			FreeLibrary(hModule);
			hModule=NULL;
			return;
		}
	}
}

inline CLocateDlg::ImageHandlerDll::~ImageHandlerDll()
{
	if (hModule!=NULL)
	{
		IH_UNINITLIBRARY pUnIninitLibrary=(IH_UNINITLIBRARY)GetProcAddress(hModule,"UninitLibrary");
		if (pUnIninitLibrary!=NULL)
			pUnIninitLibrary(uToken);
		FreeLibrary(hModule);
	}
}
		
inline BOOL CLocateDlg::ImageHandlerDll::IsLoaded()
{
	return hModule!=NULL;
}

inline CLocateDlg::CAdvancedDlg::FileType::FileType()
:	szExtensions(NULL),szTitle(NULL),szType(NULL),hIcon(NULL),szIconPath(NULL)
{
}
			
inline CLocateDlg::CAdvancedDlg::FileType::FileType(FREEDATA frType,FREEDATA frTitle)
:	szExtensions(NULL),szTitle(LPSTR(frTitle)),szType(LPSTR(frType)),szIconPath(NULL),hIcon(NULL)
{
}



#endif