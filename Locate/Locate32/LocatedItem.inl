#if !defined(LOCATEDITEM_INL)
#define LOCATEDITEM_INL

#if _MSC_VER >= 1000
#pragma once
#endif


inline LPSTR CLocatedItem::FormatAttributes() const
{
	if (!IsDeleted())
	{
		ISDLGTHREADOK
		if (g_szBuffer!=NULL)
			delete[] g_szBuffer;
		g_szBuffer=new char[10];

		LPSTR pPtr=g_szBuffer;

		if (IsFolder())
			*(pPtr++)='D';
		if (GetAttributes()&LITEMATTRIB_READONLY)
			*(pPtr++)='R';
		if (GetAttributes()&LITEMATTRIB_HIDDEN)
			*(pPtr++)='H';
		if (GetAttributes()&LITEMATTRIB_SYSTEM)
			*(pPtr++)='S';
		if (GetAttributes()&LITEMATTRIB_ARCHIVE)
			*(pPtr++)='A';
		*pPtr='\0';
		return g_szBuffer;
	}
	return const_cast<LPSTR>(szEmpty);
}


inline LPSTR CLocatedItem::FormatImageInformation() const
{
	ISDLGTHREADOK

	SIZE dim;
	if (GetImageDimensions(dim))
	{
		if (dim.cx==0 || dim.cy==0)
			return const_cast<LPSTR>(szEmpty);	
		
		if (g_szBuffer!=NULL)
			delete[] g_szBuffer;
		g_szBuffer=new char[30];
		wsprintf(g_szBuffer,"%dx%d",dim.cx,dim.cy);
		return g_szBuffer;
	}
    return const_cast<LPSTR>(szEmpty);	
}


inline LPSTR CLocatedItem::GetDetailText(CLocateDlg::DetailType nDetailType) const
{
	
	switch (nDetailType)
	{
	case CLocateDlg::DetailType::Title:
		if (GetTitle()!=NULL)
			return GetTitle();
		else
			return GetName();		
	case CLocateDlg::DetailType::InFolder:
		return GetParent();
	case CLocateDlg::DetailType::FullPath:
		return GetPath();
	case CLocateDlg::DetailType::FileSize:
		ISDLGTHREADOK
		if (GetFileSizeLo()==DWORD(-1))
			return const_cast<LPSTR>(szEmpty);
		if (g_szBuffer!=NULL)
			delete[] g_szBuffer;
		return (g_szBuffer=((CLocateApp*)GetApp())->FormatFileSizeString(
			GetFileSizeLo(),GetFileSizeHi()));
	case CLocateDlg::DetailType::FileType:
		if (GetType()==NULL)
			return const_cast<LPSTR>(szEmpty);
		return GetType();
	case CLocateDlg::DetailType::DateModified:
		ISDLGTHREADOK
		if (g_szBuffer!=NULL)
			delete[] g_szBuffer;
		return (g_szBuffer=((CLocateApp*)GetApp())->FormatDateAndTimeString(
			GetModifiedDate(),GetModifiedTime()));
	case CLocateDlg::DetailType::DateCreated:
		ISDLGTHREADOK
		if (g_szBuffer!=NULL)
			delete[] g_szBuffer;
		return (g_szBuffer=((CLocateApp*)GetApp())->FormatDateAndTimeString(
			GetCreatedDate(),GetCreatedTime()));
	case CLocateDlg::DetailType::DateAccessed:
		ISDLGTHREADOK
		if (g_szBuffer!=NULL)
			delete[] g_szBuffer;
		return (g_szBuffer=((CLocateApp*)GetApp())->FormatDateAndTimeString(
			GetAccessedDate(),GetAccessedTime()));
	case CLocateDlg::DetailType::Attributes:
		return FormatAttributes();				
	case CLocateDlg::DetailType::Owner:
	case CLocateDlg::DetailType::ShortFileName:
	case CLocateDlg::DetailType::ShortFilePath:
	case CLocateDlg::DetailType::MD5sum:
		{
			LPSTR ret=GetExtraText(nDetailType);
			if (ret==NULL)
				return const_cast<LPSTR>(szEmpty);
			return ret;
		}
	case CLocateDlg::DetailType::ImageDimensions:
		return FormatImageInformation();				
	case CLocateDlg::DetailType::Database:
		return const_cast<LPSTR>(GetLocateApp()->GetDatabase(GetDatabaseID())->GetName());
	case CLocateDlg::DetailType::DatabaseDescription:
		return const_cast<LPSTR>(GetLocateApp()->GetDatabase(GetDatabaseID())->GetDescription());
	case CLocateDlg::DetailType::DatabaseArchive:
		return const_cast<LPSTR>(GetLocateApp()->GetDatabase(GetDatabaseID())->GetArchiveName());
	case CLocateDlg::DetailType::VolumeLabel:
		return const_cast<LPSTR>(CLocateDlg::GetVolumeLabel(GetDatabaseID(),GetRootID()));
	case CLocateDlg::DetailType::VolumeSerial:
		return const_cast<LPSTR>(CLocateDlg::GetVolumeSerial(GetDatabaseID(),GetRootID()));
	case CLocateDlg::DetailType::VOlumeFileSystem:
		return const_cast<LPSTR>(CLocateDlg::GetVolumeFileSystem(GetDatabaseID(),GetRootID()));
	}
	return const_cast<LPSTR>(szEmpty);
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



#endif