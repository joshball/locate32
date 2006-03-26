#if !defined(LOCATEDITEM_INL)
#define LOCATEDITEM_INL

#if _MSC_VER >= 1000
#pragma once
#endif


inline LPWSTR CLocatedItem::FormatAttributes() const
{
	if (!IsDeleted())
	{
		ISDLGTHREADOK
		if (g_szwBuffer!=NULL)
			delete[] g_szwBuffer;
		g_szwBuffer=new WCHAR[10];

		LPWSTR pPtr=g_szwBuffer;

		if (IsFolder())
			*(pPtr++)=L'D';
		if (GetAttributes()&LITEMATTRIB_READONLY)
			*(pPtr++)=L'R';
		if (GetAttributes()&LITEMATTRIB_HIDDEN)
			*(pPtr++)=L'H';
		if (GetAttributes()&LITEMATTRIB_SYSTEM)
			*(pPtr++)=L'S';
		if (GetAttributes()&LITEMATTRIB_ARCHIVE)
			*(pPtr++)=L'A';
		*pPtr=L'\0';
		return g_szwBuffer;
	}
	return const_cast<LPWSTR>(szwEmpty);
}


inline LPWSTR CLocatedItem::FormatImageInformation() const
{
	ISDLGTHREADOK

	SIZE dim;
	if (GetImageDimensions(dim))
	{
		if (dim.cx==0 || dim.cy==0)
			return const_cast<LPWSTR>(szwEmpty);	
		
		if (g_szwBuffer!=NULL)
			delete[] g_szwBuffer;
		g_szwBuffer=new WCHAR[30];
		StringCbPrintfW(g_szwBuffer,30,L"%dx%d",dim.cx,dim.cy);
		return g_szwBuffer;
	}
    return const_cast<LPWSTR>(szwEmpty);	
}


inline LPWSTR CLocatedItem::GetDetailText(CLocateDlg::DetailType nDetailType) const
{
	
	switch (nDetailType)
	{
	case CLocateDlg::Title:
		if (GetTitle()!=NULL)
			return GetTitle();
		else
			return GetName();		
	case CLocateDlg::InFolder:
		return GetParent();
	case CLocateDlg::FullPath:
		return GetPath();
	case CLocateDlg::FileSize:
		ISDLGTHREADOK
		if (GetFileSizeLo()==DWORD(-1))
			return const_cast<LPWSTR>(szwEmpty);
		if (g_szwBuffer!=NULL)
			delete[] g_szwBuffer;
		return (g_szwBuffer=((CLocateApp*)GetApp())->FormatFileSizeString(
			GetFileSizeLo(),GetFileSizeHi()));
	case CLocateDlg::FileType:
		if (GetType()==NULL)
			return const_cast<LPWSTR>(szwEmpty);
		return GetType();
	case CLocateDlg::DateModified:
		ISDLGTHREADOK
		if (g_szwBuffer!=NULL)
			delete[] g_szwBuffer;
		return (g_szwBuffer=((CLocateApp*)GetApp())->FormatDateAndTimeString(
			GetModifiedDate(),GetModifiedTime()));
	case CLocateDlg::DateCreated:
		ISDLGTHREADOK
		if (g_szwBuffer!=NULL)
			delete[] g_szwBuffer;
		return (g_szwBuffer=((CLocateApp*)GetApp())->FormatDateAndTimeString(
			GetCreatedDate(),GetCreatedTime()));
	case CLocateDlg::DateAccessed:
		ISDLGTHREADOK
		if (g_szwBuffer!=NULL)
			delete[] g_szwBuffer;
		return (g_szwBuffer=((CLocateApp*)GetApp())->FormatDateAndTimeString(
			GetAccessedDate(),GetAccessedTime()));
	case CLocateDlg::Attributes:
		return FormatAttributes();				
	case CLocateDlg::Owner:
	case CLocateDlg::ShortFileName:
	case CLocateDlg::ShortFilePath:
	case CLocateDlg::MD5sum:
		{
			LPWSTR ret=GetExtraText(nDetailType);
			if (ret==NULL)
				return const_cast<LPWSTR>(szwEmpty);
			return ret;
		}
	case CLocateDlg::ImageDimensions:
		return FormatImageInformation();				
	case CLocateDlg::Database:
		return const_cast<LPWSTR>(GetLocateApp()->GetDatabase(GetDatabaseID())->GetName());
	case CLocateDlg::DatabaseDescription:
		return const_cast<LPWSTR>(GetLocateApp()->GetDatabase(GetDatabaseID())->GetDescription());
	case CLocateDlg::DatabaseArchive:
		return const_cast<LPWSTR>(GetLocateApp()->GetDatabase(GetDatabaseID())->GetArchiveName());
	case CLocateDlg::VolumeLabel:
		return const_cast<LPWSTR>(CLocateDlg::GetVolumeLabel(GetDatabaseID(),GetRootID()));
	case CLocateDlg::VolumeSerial:
		return const_cast<LPWSTR>(CLocateDlg::GetVolumeSerial(GetDatabaseID(),GetRootID()));
	case CLocateDlg::VOlumeFileSystem:
		return const_cast<LPWSTR>(CLocateDlg::GetVolumeFileSystem(GetDatabaseID(),GetRootID()));
	}
	return const_cast<LPWSTR>(szwEmpty);
}


inline BOOL CLocatedItem::ShouldUpdateTitle() const 
{ 
	// do not depend on efEnableUpdating
	return !(dwFlags&LITEM_TITLEOK && dwFlags&LITEM_FILENAMEOK); 
}

inline BOOL CLocatedItem::ShouldUpdateFilename() const 
{ 
	// do not depend on efEnableUpdating
	return !(dwFlags&LITEM_FILENAMEOK); 
}

inline BOOL CLocatedItem::ShouldUpdateType() const 
{
	// do not depend on efEnableUpdating
	return !(dwFlags&LITEM_TYPEOK);  
}

inline BOOL CLocatedItem::ShouldUpdateIcon() const 
{
	// do not depend on efEnableUpdating
	return !(dwFlags&LITEM_ICONOK) && GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->GetFlags()&CLocateDlg::fgLVShowIcons;
}


inline BOOL CLocatedItem::ShouldUpdateFileSize() const 
{
	return dwFlags&LITEM_FILESIZEOK?FALSE:(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating?TRUE:FALSE); 
}

inline BOOL CLocatedItem::ShouldUpdateTimeAndDate() const 
{
	return dwFlags&LITEM_TIMEDATEOK?FALSE:(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating?TRUE:FALSE);  
}

inline BOOL CLocatedItem::ShouldUpdateFileSizeOrDate() const 
{
	if (ShouldUpdateFileSize())
		return TRUE;
	return ShouldUpdateTimeAndDate(); 
}

inline BOOL CLocatedItem::ShouldUpdateAttributes() const 
{
	return dwFlags&LITEM_ATTRIBOK?FALSE:(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating?TRUE:FALSE);
}

inline BOOL CLocatedItem::ShouldUpdateParentIcon() const 
{
	return !(dwFlags&LITEM_PARENTICONOK) && GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->GetFlags()&CLocateDlg::fgLVShowIcons;
}

inline BOOL CLocatedItem::ShouldUpdateParentIcon2() const 
{
	return !(dwFlags&LITEM_PARENTICONOK); 
}

inline BOOL CLocatedItem::ShouldUpdateExtra(CLocateDlg::DetailType nDetail) const
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return FALSE;

	ExtraInfo* pTmp=pFirstExtraInfo;
	while (pTmp!=NULL)
	{
		if (pTmp->nType==nDetail)
			return pTmp->bShouldUpdate;
		pTmp=pTmp->pNext;
	}
	return TRUE;
}

inline void CLocatedItem::CheckIfDeleted()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;
	

	if (IsDeleted())
		return;
	if (IsFolder())
	{
		if (!CFile::IsDirectory(GetPath()))
			SetToDeleted();
	}
	else
	{
		if (!CFile::IsFile(GetPath()))
			SetToDeleted();
	}
}


inline CLocatedItem::CLocatedItem(BOOL bFolder,const CLocater* pLocater)
:	szTitle(NULL),szType(NULL),dwFlags(0),pFirstExtraInfo(NULL)
{
	if (bFolder)
		SetFolder(pLocater);
	else
		SetFile(pLocater);
}

inline CLocatedItem::~CLocatedItem()
{
	ClearData();
}

inline void CLocatedItem::OemtoAnsi()
{
	GetPath();
	OemToChar(szPath,szPath);
}

inline void CLocatedItem::UpdateIcon()
{
	SHFILEINFO fi;
	if (GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->GetFlags()&CLocateDlg::fgLVShowIcons)
	{
        if (SHGetFileInfo(GetPath(),0,&fi,sizeof(SHFILEINFO),/*SHGFI_ICON|*/SHGFI_SYSICONINDEX))
		{
			iIcon=fi.iIcon;
			DebugFormatMessage("CLocatedItem::UpdateIcon(): iIcon for %s is %d",GetName(),iIcon);
			
		}
		else
		{
			iIcon=GetLocateApp()->m_nDelImage;
			DebugFormatMessage("CLocatedItem::UpdateIcon(): failed to get icon, iIcon for %s is %d",GetName(),iIcon);
		}
	}
	else if (IsDeleted())
	{
		iIcon=GetLocateApp()->m_nDelImage;
		DebugFormatMessage("CLocatedItem::UpdateIcon(): use deleted icon %d for %s",iIcon,GetName());
	}
	else
	{
		iIcon=GetLocateApp()->m_nDefImage;
		DebugFormatMessage("CLocatedItem::UpdateIcon(): use default icon %d for %s",iIcon,GetName());
	}

	dwFlags|=LITEM_ICONOK;
}

inline void CLocatedItem::UpdateParentIcon()
{
	SHFILEINFOW fi;
	LPWSTR szParent=GetParent();
	if (GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->GetFlags()&CLocateDlg::fgLVShowIcons)
	{
		if (szParent[1]==':' && szParent[2]=='\0')
		{
			WCHAR szDrive[]="X:\\";
			szDrive[0]=szParent[0];
			if (GetFileInfo(szDrive,0,&fi,/*SHGFI_ICON|*/SHGFI_SYSICONINDEX))
				iParentIcon=fi.iIcon;
			else
				iParentIcon=((CLocateApp*)GetApp())->m_nDelImage;
		}
		else
		{
			if (GetFileInfo(GetParent(),0,&fi,/*SHGFI_ICON|*/SHGFI_SYSICONINDEX))
				iParentIcon=fi.iIcon;
			else
				iParentIcon=((CLocateApp*)GetApp())->m_nDelImage;
		}
	}
	else if (IsDeleted())
		iIcon=GetLocateApp()->m_nDelImage;
	else
    	iIcon=GetLocateApp()->m_nDefImage;

	dwFlags|=LITEM_PARENTICONOK;
}


inline void CLocatedItem::ReFresh(CArray<CLocateDlg::DetailType>& aDetails,BOOL& bReDraw)
{
	ItemDebugFormatMessage4("CLocatedItem::ReFresh BEGIN, item=%s flags=%X",GetName(),GetFlags(),0,0);

	//TODO: More optimal code may exists
	for (int i=0;i<aDetails.GetSize();i++)
	{
		ItemDebugFormatMessage4("CLocatedItem::ReFresh 1, item=%s flags=%X nDetail=%X su=%d",
			GetName(),GetFlags(),DWORD(aDetails[i]),ShouldUpdateByDetail(aDetails[i]));
		if (ShouldUpdateByDetail(aDetails[i]))
		{
			UpdateByDetail(aDetails[i]);
			bReDraw=TRUE;
		}
		ItemDebugFormatMessage4("CLocatedItem::ReFresh 2, item=%s flags=%X nDetail=%X su=%d",
			GetName(),GetFlags(),DWORD(aDetails[i]),ShouldUpdateByDetail(aDetails[i]));
	}

	ItemDebugFormatMessage4("CLocatedItem::ReFresh END, item=%s flags=%X",GetName(),GetFlags(),0,0);
}

inline void CLocatedItem::ReFresh(CArray<CLocateDlg::DetailType>& aDetails,int* pUpdated)
{
	ItemDebugFormatMessage4("CLocatedItem::ReFresh BEGIN, item=%s flags=%X",GetName(),GetFlags(),0,0);
	
	int iUpdatedCount=0;

	//TODO: More optimal code may exists
	for (int i=0;i<aDetails.GetSize();i++)
	{
		ItemDebugFormatMessage4("CLocatedItem::ReFresh 1, item=%s flags=%X nDetail=%X su=%d",
			GetName(),GetFlags(),DWORD(aDetails[i]),ShouldUpdateByDetail(aDetails[i]));
		if (ShouldUpdateByDetail(aDetails[i]))
		{
			UpdateByDetail(aDetails[i]);
			pUpdated[iUpdatedCount++]=i;
		}
		ItemDebugFormatMessage4("CLocatedItem::ReFresh 2, item=%s flags=%X nDetail=%X su=%d",
			GetName(),GetFlags(),DWORD(aDetails[i]),ShouldUpdateByDetail(aDetails[i]));
	}

	pUpdated[iUpdatedCount]=-1;
	ItemDebugFormatMessage4("CLocatedItem::ReFresh END, item=%s flags=%X",GetName(),GetFlags(),0,0);
}

inline BOOL CLocatedItem::GetImageDimensions(SIZE& dim) const
{
	ExtraInfo* pInfo=GetFieldForType(CLocateDlg::ImageDimensions);
	if (pInfo!=NULL)
	{
		dim=pInfo->szImageDimension;
		return TRUE;
	}
	return FALSE;
}

inline int CLocatedItem::GetImageDimensionsProduct() const
{
	ExtraInfo* pInfo=GetFieldForType(CLocateDlg::ImageDimensions);
	if (pInfo!=NULL)
		return pInfo->szImageDimension.cx*pInfo->szImageDimension.cy;
	return 0;
}

inline LPSTR CLocatedItem::GetExtraText(CLocateDlg::DetailType nDetail) const
{
	ExtraInfo* pInfo=GetFieldForType(nDetail);
	if (pInfo!=NULL)
		return pInfo->szText;
	return NULL;
}

inline void CLocatedItem::ExtraSetUpdateWhenFileSizeChanged()
{
	ExtraInfo* pTmp=pFirstExtraInfo;
	while (pTmp!=NULL)
	{
		switch (pTmp->nType)
		{
		case CLocateDlg::Owner:
		case CLocateDlg::ImageDimensions:
            pTmp->bShouldUpdate=TRUE;
			break;
		case CLocateDlg::MD5sum:
			if (GetLocateDlg()->GetFlags()&CLocateDlg::fgLVComputeMD5Sums)
				pTmp->bShouldUpdate=TRUE;
			break;
		}
		pTmp=pTmp->pNext;
	}
}

inline void CLocatedItem::DeleteAllExtraFields()
{
    while (pFirstExtraInfo!=NULL)
	{
		ExtraInfo*pTmp=pFirstExtraInfo;
		pFirstExtraInfo=pTmp->pNext;
		delete pTmp;
	}
} 


inline CLocatedItem::ExtraInfo::ExtraInfo(CLocateDlg::DetailType nType_)
:	nType(nType_),pNext(NULL),szText(NULL),bShouldUpdate(TRUE)
{
}

inline CLocatedItem::ExtraInfo::~ExtraInfo()
{
	switch (nType)
	{
	case CLocateDlg::ShortFileName:
	case CLocateDlg::ShortFilePath:
	case CLocateDlg::Owner:
	case CLocateDlg::MD5sum:
		if (szText!=NULL && szText!=szEmpty)
			Allocation.Free(szText);
		break;
	}
}


inline CLocatedItem::ExtraInfo* CLocatedItem::GetFieldForType(CLocateDlg::DetailType nType) const
{
	ExtraInfo* pTmp=pFirstExtraInfo;
	while (pTmp!=NULL)
	{
		if (pTmp->nType==nType)
			return pTmp;
		pTmp=pTmp->pNext;
	}
	return NULL;
}



inline CLocatedItem::ExtraInfo* CLocatedItem::CreateExtraInfoField(CLocateDlg::DetailType nType)
{
	/* First check if field already exists */
	ExtraInfo* pField=GetFieldForType(nType);
	if (pField==NULL)
	{
		pField=new ExtraInfo(nType);
		pField->pNext=pFirstExtraInfo;
		pFirstExtraInfo=pField;
	}
	return pField;
}

#endif