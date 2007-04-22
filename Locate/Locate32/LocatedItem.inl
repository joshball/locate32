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
		StringCbPrintfW(g_szwBuffer,30*sizeof(WCHAR),L"%dx%d",dim.cx,dim.cy);
		return g_szwBuffer;
	}
    return const_cast<LPWSTR>(szwEmpty);	
}


inline LPWSTR CLocatedItem::FormatPages() const
{
	ISDLGTHREADOK

	ExtraInfo* pInfo=GetFieldForType(CLocateDlg::Pages);
	if (pInfo!=NULL)
	{
		if (pInfo->nPages==0)
			return const_cast<LPWSTR>(szwEmpty);	

		if (g_szwBuffer!=NULL)
			delete[] g_szwBuffer;
		g_szwBuffer=new WCHAR[11];
		_ultow_s(pInfo->nPages,g_szwBuffer,11,10);
		return g_szwBuffer;
	}
	return const_cast<LPWSTR>(szwEmpty);	
}

inline LPWSTR CLocatedItem::GetDetailText(CLocateDlg::DetailType nDetailType) const
{
	
	switch (nDetailType)
	{
	case CLocateDlg::Name:
		if (GetFileTitle()!=NULL)
			return GetFileTitle();
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
	case CLocateDlg::Author:
	case CLocateDlg::Title:
	case CLocateDlg::Subject:
	case CLocateDlg::Category:
	case CLocateDlg::Comments:
	case CLocateDlg::Description:
	case CLocateDlg::FileVersion:
	case CLocateDlg::ProductName:
	case CLocateDlg::ProductVersion:
		{
			LPWSTR ret=GetExtraText(nDetailType);
			if (ret==NULL)
				return const_cast<LPWSTR>(szwEmpty);
			return ret;
		}
	case CLocateDlg::Pages:
		return FormatPages();
	case CLocateDlg::ImageDimensions:
		return FormatImageInformation();				
	case CLocateDlg::Database:
		return const_cast<LPWSTR>(GetLocateApp()->GetDatabase(GetDatabaseID())->GetName());
	case CLocateDlg::DatabaseDescription:
		return const_cast<LPWSTR>(GetLocateApp()->GetDatabase(GetDatabaseID())->GetDescription());
	case CLocateDlg::DatabaseArchive:
		return const_cast<LPWSTR>(GetLocateApp()->GetDatabase(GetDatabaseID())->GetArchiveName());
	case CLocateDlg::VolumeLabel:
		return const_cast<LPWSTR>(CLocateDlg::GetDBVolumeLabel(GetDatabaseID(),GetRootID()));
	case CLocateDlg::VolumeSerial:
		return const_cast<LPWSTR>(CLocateDlg::GetDBVolumeSerial(GetDatabaseID(),GetRootID()));
	case CLocateDlg::VolumeFileSystem:
		return const_cast<LPWSTR>(CLocateDlg::GetDBVolumeFileSystem(GetDatabaseID(),GetRootID()));
	}
	return const_cast<LPWSTR>(szwEmpty);
}


inline BOOL CLocatedItem::ShouldUpdateFileTitle() const 
{ 
	// do not depend on efEnableUpdating
	return !(dwFlags&LITEM_FILETITLEOK && dwFlags&LITEM_FILENAMEOK); 
}

inline BOOL CLocatedItem::ShouldUpdateFilename() const 
{ 
#ifdef _DEBUG
	// do not depend on efEnableUpdating
	return !(dwFlags&LITEM_FILENAMEOK); 
#else
	return FALSE;
#endif
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
		if (!FileSystem::IsDirectory(GetPath()))
			SetToDeleted();
	}
	else
	{
		if (!FileSystem::IsFile(GetPath()))
			SetToDeleted();
	}
}

inline CLocatedItem::CLocatedItem()
:	szFileTitle(NULL),szType(NULL),dwFlags(0),pFirstExtraInfo(NULL)
{
}



inline CLocatedItem::~CLocatedItem()
{
	ClearData();
}



inline void CLocatedItem::UpdateIcon()
{
	SHFILEINFOW fi;
	ItemDebugMessage("CLocatedItem::UpdateIcon BEGIN");
	if (GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->GetFlags()&CLocateDlg::fgLVShowIcons)
	{
        if (GetFileInfo(GetPath(),0,&fi,/*SHGFI_ICON|*/SHGFI_SYSICONINDEX))
			iIcon=fi.iIcon;
		else
			iIcon=GetLocateApp()->m_nDelImage;
	}
	else if (IsDeleted())
		iIcon=GetLocateApp()->m_nDelImage;
	else
		iIcon=GetLocateApp()->m_nDefImage;

	dwFlags|=LITEM_ICONOK;

	ItemDebugMessage("CLocatedItem::UpdateIcon END");
	
}

inline void CLocatedItem::UpdateParentIcon()
{
	ItemDebugMessage("CLocatedItem::UpdateParentIcon BEGIN");
	SHFILEINFOW fi;
	LPWSTR szParent=GetParent();
	if (GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->GetFlags()&CLocateDlg::fgLVShowIcons)
	{
		if (szParent[1]==L':' && szParent[2]==L'\0')
		{
			WCHAR szDrive[]=L"X:\\";
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

	ItemDebugMessage("CLocatedItem::UpdateParentIcon END");
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

inline int CLocatedItem::GetPages() const
{
	ExtraInfo* pInfo=GetFieldForType(CLocateDlg::Pages);
	if (pInfo!=NULL)
		return pInfo->nPages;
	return 0;
}


inline LPWSTR CLocatedItem::GetExtraText(CLocateDlg::DetailType nDetail) const
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
		case CLocateDlg::Author:
		case CLocateDlg::Title:
		case CLocateDlg::Subject:
		case CLocateDlg::Category:
		case CLocateDlg::Comments:
		case CLocateDlg::Pages:
		case CLocateDlg::Description:
		case CLocateDlg::FileVersion:
		case CLocateDlg::ProductName:
		case CLocateDlg::ProductVersion:
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
	case CLocateDlg::Author:
	case CLocateDlg::Title:
	case CLocateDlg::Subject:
	case CLocateDlg::Category:
	case CLocateDlg::Comments:
	case CLocateDlg::Description:
	case CLocateDlg::FileVersion:
	case CLocateDlg::ProductName:
	case CLocateDlg::ProductVersion:
		if (szText!=NULL && szText!=szwEmpty)
			delete[] szText;
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