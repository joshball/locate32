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
	void ReFresh(CArray<CLocateDlg::DetailType>& aDetails,BOOL& bReDraw);
	void ReFresh(CArray<CLocateDlg::DetailType>& aDetails,int* pUpdated);
	

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
	
	void ChangeName(LPCWSTR szNewName,int iLength=-1);

private:
	LPWSTR szTitle;
	LPWSTR szName;
	mutable LPWSTR szPath;
	LPWSTR szType;
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
	WORD wFileSizeHi;
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

	LPWSTR GetName() const { return szName; }
	DWORD GetNameLen() const { return bNameLength; }
	LPWSTR GetPath() const { szName[-1]=L'\\'; return szPath; }
	DWORD GetPathLen() const { return bNameLength+DWORD(szName-szPath); }
	LPWSTR GetParent() const { szName[-1]=L'\0'; return szPath; }
	LPWSTR GetTitle() const { return szTitle; }
	LPWSTR GetType() const { return szType; }
	
	LPWSTR GetDetailText(CLocateDlg::DetailType nDetailType) const;
	LPWSTR GetToolTipText() const;


	int GetIcon() const { return iIcon; }
	int GetParentIcon() const { return iParentIcon; }
	WORD GetModifiedDate() const { return wModifiedDate; }
	WORD GetModifiedTime() const { return wModifiedTime; }
	WORD GetCreatedDate() const { return wCreatedDate; }
	WORD GetCreatedTime() const { return wCreatedTime; }
	WORD GetAccessedDate() const { return wAccessedDate; }
	WORD GetAccessedTime() const { return wAccessedTime; }
	DWORD GetFileSizeLo() const { return dwFileSize; }
	DWORD GetFileSizeHi() const { return wFileSizeHi; }
	LONGLONG GetFileSize() const { return ((LONGLONG)wFileSizeHi)<<32|(LONGLONG)dwFileSize; }
	LPWSTR GetExtension() const { return szName+bExtensionPos; }
	DWORD GetExtensionLength() const { return bNameLength-bExtensionPos; }

	BOOL GetImageDimensions(SIZE& dim) const;
	int GetImageDimensionsProduct() const;
	LPWSTR GetExtraText(CLocateDlg::DetailType nDetailType) const; 
	void ExtraSetUpdateWhenFileSizeChanged();
	void DeleteAllExtraFields();
    BOOL IsItemShortcut() const;

	LPWSTR FormatAttributes() const;
	LPWSTR FormatOwner() const;
	LPWSTR FormatImageInformation() const;

private:
	
	/* For extra field i.e. fields not obtained from db */
	struct ExtraInfo {
		ExtraInfo(CLocateDlg::DetailType nType);
		~ExtraInfo();
		
		BOOL ShouldUpdate() const;

		CLocateDlg::DetailType nType;
		union {
			SIZE szImageDimension;
			WCHAR* szText;
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



#endif
