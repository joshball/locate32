#if !defined(LOCATEDITEM_H)
#define LOCATEDITEM_H

#if _MSC_VER >= 1000
#pragma once
#endif

class CLocater;

// For dwFlags
#define LITEM_TITLEOK				0x1
#define LITEM_TYPEOK				0x2
#define LITEM_ICONOK				0x4
#define LITEM_FILESIZEOK			0x8
#define LITEM_TIMEDATEOK			0x10
#define LITEM_ATTRIBOK				0x20
#define LITEM_PARENTICONOK			0x40
#define LITEM_FILENAMEOK			0x80 // Only case is checked
#define LITEM_EXTRASOK				0x100 // Only for refreshing
#define LITEM_COULDCHANGE			LITEM_TYPEOK|LITEM_ICONOK|LITEM_FILESIZEOK|LITEM_TIMEDATEOK|LITEM_ATTRIBOK|LITEM_EXTRASOK|LITEM_FILENAMEOK

#define LITEMATTRIB_HIDDEN			0x1
#define LITEMATTRIB_READONLY		0x2
#define LITEMATTRIB_ARCHIVE			0x4
#define LITEMATTRIB_SYSTEM			0x8
#define LITEMATTRIB_CUTTED			0x10
#define LITEMATTRIB_DIRECTORY		0x80

#define LITEMATTRIB_DBATTRIBFLAG	0x8F

class CLocatedItem 
{
public:
	CLocatedItem(BOOL bFolder,const CLocater* pLocater);
	~CLocatedItem();

private:
	void SetFolder(const CLocater* pLocater);
	void SetFile(const CLocater* pLocater);
	void ClearData();

public:
	void ReFresh(CLocateDlg::DetailType nDetail,BOOL& bReDraw);
	void ReFresh(CArray<CLocateDlg::DetailType>& aDetails,BOOL& bReDraw);
	void ReFresh(int iDetails,CLocateDlg::DetailType* pDetails,BOOL& bReDraw);

	void UpdateByDetail(CLocateDlg::DetailType nDetail);
	void UpdateTitle();
	void UpdateIcon();
	void UpdateParentIcon();
	void UpdateType();
	void UpdateFileSizeAndTime();
	void UpdateAttributes();
	void UpdateFilename();
	void UpdateDimensions();
	void UpdateOwner();
	void UpdateShortFileName();
	void UpdateShortFilePath();
	void ComputeMD5sum(BOOL bForce=FALSE);

	void SetToDeleted();
	void OemtoAnsi();

	void ChangeName(LPCSTR szNewName,int iLength=-1);

private:
	LPSTR szTitle;
	LPSTR szName;
	mutable LPSTR szPath;
	LPSTR szType;
	int iIcon;
	int iParentIcon;

	DWORD dwFlags;
	WORD wModifiedDate;
	WORD wModifiedTime;
	WORD wCreatedDate;
	WORD wCreatedTime;
	WORD wAccessedDate;
	WORD wAccessedTime;
	DWORD dwFileSize;
	BYTE bFileSizeHi;
	BYTE bExtensionPos;
	BYTE bAttribs;
	BYTE bNameLength;

	WORD wDatabaseID;
	WORD wRootID;

public:
	//static LPSTR m_szBuffer; 

public:
	DWORD GetFlags() const { return dwFlags; }
	void AddFlags(DWORD dwAdd) { dwFlags|=dwAdd; }
	void RemoveFlags(DWORD dwRemove) { dwFlags&=~dwRemove; }
	BOOL RemoveFlagsForChanged(); // Returing ok if changed
	void CheckIfDeleted();

	BOOL ShouldUpdateByDetail(CLocateDlg::DetailType nDetail) const;
	BOOL ShouldUpdateTitle() const;
	BOOL ShouldUpdateFilename() const;
	BOOL ShouldUpdateType() const;
	BOOL ShouldUpdateIcon() const;
	BOOL ShouldUpdateFileSize() const;
	BOOL ShouldUpdateTimeAndDate() const;
	BOOL ShouldUpdateFileSizeOrDate() const;
	BOOL ShouldUpdateAttributes() const;
	BOOL ShouldUpdateParentIcon() const;
	BOOL ShouldUpdateParentIcon2() const;
	
	BOOL ShouldUpdateExtra(CLocateDlg::DetailType nDetail) const;

	
	// Accessors
	WORD GetDatabaseID() const { return wDatabaseID; }
	WORD GetRootID() const { return wRootID; }

	BYTE GetAttributes() const { return bAttribs; }
	void AddAttribute(BYTE bAttribute) { bAttribs|=bAttribute; }
	void RemoveAttribute(BYTE bAttribute) { bAttribs&=~bAttribute; }

	BOOL IsDeleted() const { return iIcon==((CLocateApp*)GetApp())->m_nDelImage && dwFileSize==DWORD(-1) && wModifiedTime==WORD(-1) && wModifiedDate==WORD(-1); }
	BOOL IsFolder() const { return bAttribs&LITEMATTRIB_DIRECTORY; }
	BOOL IsHidden() const { return bAttribs&LITEMATTRIB_HIDDEN; }
	BOOL IsReadOnly() const { return bAttribs&LITEMATTRIB_READONLY; }
	BOOL IsSystem() const { return bAttribs&LITEMATTRIB_SYSTEM; }
	BOOL IsArchive() const { return bAttribs&LITEMATTRIB_ARCHIVE; }
	BOOL IsCutted() const { return bAttribs&LITEMATTRIB_CUTTED; }

	LPSTR GetName() const { return szName; }
	DWORD GetNameLen() const { return bNameLength; }
	LPSTR GetPath() const { szName[-1]='\\'; return szPath; }
	DWORD GetPathLen() const { return bNameLength+DWORD(szName-szPath); }
	LPSTR GetParent() const { szName[-1]='\0'; return szPath; }
	LPSTR GetTitle() const { return szTitle; }
	LPSTR GetType() const { return szType; }
	
	LPSTR GetDetailText(CLocateDlg::DetailType nDetailType) const;
	LPSTR GetToolTipText() const;


	int GetIcon() const { return iIcon; }
	int GetParentIcon() const { return iParentIcon; }
	WORD GetModifiedDate() const { return wModifiedDate; }
	WORD GetModifiedTime() const { return wModifiedTime; }
	WORD GetCreatedDate() const { return wCreatedDate; }
	WORD GetCreatedTime() const { return wCreatedTime; }
	WORD GetAccessedDate() const { return wAccessedDate; }
	WORD GetAccessedTime() const { return wAccessedTime; }
	DWORD GetFileSizeLo() const { return dwFileSize; }
	DWORD GetFileSizeHi() const { return bFileSizeHi; }
	LONGLONG GetFileSize() const { return ((LONGLONG)bFileSizeHi)<<32|(LONGLONG)dwFileSize; }
	LPSTR GetExtension() const { return szName+bExtensionPos; }
	DWORD GetExtensionLength() const { return bNameLength-bExtensionPos; }

	BOOL GetImageDimensions(SIZE& dim) const;
	int GetImageDimensionsProduct() const;
	LPSTR GetExtraText(CLocateDlg::DetailType nDetailType) const; 
	void ExtraSetUpdateWhenFileSizeChanged();
	void DeleteAllExtraFields();
    BOOL IsItemShortcut() const;

	LPSTR FormatFileSize() const;
	LPSTR FormatAttributes() const;
	LPSTR FormatOwner() const;
	LPSTR FormatImageInformation() const;

private:
	
	/* For extra field i.e. fields not obtained from db */
	struct ExtraInfo {
		ExtraInfo(CLocateDlg::DetailType nType);
		~ExtraInfo();
		
		BOOL ShouldUpdate() const;

		CLocateDlg::DetailType nType;
		union {
			SIZE szImageDimension;
			char* szText;
		};
		BYTE bShouldUpdate;

		ExtraInfo* pNext;
	public:
		void* operator new(size_t size) { return Allocation.AllocateFast(size); }
		void operator delete(void* pObject) { Allocation.Free(pObject); }
		void operator delete(void* pObject,size_t size) { Allocation.Free(pObject); }
	};
	ExtraInfo* pFirstExtraInfo;

	void DeleteExtraInfoField(CLocateDlg::DetailType nType);
	ExtraInfo* CreateExtraInfoField(CLocateDlg::DetailType nType);
	ExtraInfo* GetFieldForType(CLocateDlg::DetailType nType) const;
	

public:
	// Overloaded operators, for fast allocating
	void* operator new(size_t size) { return Allocation.AllocateFast(size); }
	void operator delete(void* pObject) { Allocation.Free(pObject); }
	void operator delete(void* pObject,size_t size) { Allocation.Free(pObject); }
};

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
			iIcon=fi.iIcon;
		else
			iIcon=GetLocateApp()->m_nDelImage;
	}
	else if (IsDeleted())
		iIcon=GetLocateApp()->m_nDelImage;
	else
    	iIcon=GetLocateApp()->m_nDefImage;

	dwFlags|=LITEM_ICONOK;
}

inline void CLocatedItem::UpdateParentIcon()
{
	SHFILEINFO fi;
	LPSTR szParent=GetParent();
	if (GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->GetFlags()&CLocateDlg::fgLVShowIcons)
	{
		if (szParent[1]==':' && szParent[2]=='\0')
		{
			char szDrive[]="X:\\";
			szDrive[0]=szParent[0];
			if (SHGetFileInfo(szDrive,0,&fi,sizeof(SHFILEINFO),/*SHGFI_ICON|*/SHGFI_SYSICONINDEX))
				iParentIcon=fi.iIcon;
			else
				iParentIcon=((CLocateApp*)GetApp())->m_nDelImage;
		}
		else
		{
			if (SHGetFileInfo(GetParent(),0,&fi,sizeof(SHFILEINFO),/*SHGFI_ICON|*/SHGFI_SYSICONINDEX))
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

inline void CLocatedItem::ReFresh(CLocateDlg::DetailType nDetail,BOOL& bReDraw)
{
	ItemDebugFormatMessage4("CLocatedItem::ReFresh BEGIN, item=%s flags=%X nDetail=%X su=%d",
		GetName(),GetFlags(),DWORD(nDetail),ShouldUpdateByDetail(nDetail));
	
	//TODO: More optimal code may exists
	if (ShouldUpdateByDetail(nDetail))
	{
		UpdateByDetail(nDetail);
		bReDraw=TRUE;
	}
	
	ItemDebugFormatMessage4("CLocatedItem::ReFresh END, item=%s flags=%X nDetail=%X su=%d",
		GetName(),GetFlags(),DWORD(nDetail),ShouldUpdateByDetail(nDetail));
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

inline void CLocatedItem::ReFresh(int iDetails,CLocateDlg::DetailType* pDetails,BOOL& bReDraw)
{
	//TODO: More optimal code may exists
	for (int i=0;i<iDetails;i++)
	{
		if (ShouldUpdateByDetail(pDetails[i]))
		{
			UpdateByDetail(pDetails[i]);
			bReDraw=TRUE;
		}
	}
}

inline BOOL CLocatedItem::GetImageDimensions(SIZE& dim) const
{
	ExtraInfo* pInfo=GetFieldForType(CLocateDlg::DetailType::ImageDimensions);
	if (pInfo!=NULL)
	{
		dim=pInfo->szImageDimension;
		return TRUE;
	}
	return FALSE;
}

inline int CLocatedItem::GetImageDimensionsProduct() const
{
	ExtraInfo* pInfo=GetFieldForType(CLocateDlg::DetailType::ImageDimensions);
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
		case CLocateDlg::DetailType::Owner:
		case CLocateDlg::DetailType::ImageDimensions:
            pTmp->bShouldUpdate=TRUE;
			break;
		case CLocateDlg::DetailType::MD5sum:
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
	case CLocateDlg::DetailType::ShortFileName:
	case CLocateDlg::DetailType::ShortFilePath:
	case CLocateDlg::DetailType::Owner:
	case CLocateDlg::DetailType::MD5sum:
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
