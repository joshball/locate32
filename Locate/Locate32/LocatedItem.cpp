#include <HFCLib.h>
#include "Locate32.h"

#include <md5.h>
#ifdef _DEBUG
#pragma comment(lib, "libmd5d.lib")
#else
#pragma comment(lib, "libmd5.lib")
#endif




void CLocatedItem::SetFolder(const CLocater* pLocater)
{
	ItemDebugMessage("CLocatedItem::SetFolder BEGIN");

	wDatabaseID=pLocater->GetCurrentDatabaseID();
	wRootID=pLocater->GetCurrentDatabaseRootID();
	
	// Setting path, name and extension
	DWORD nPathLen=pLocater->GetCurrentPathLen();
	bNameLength=BYTE(pLocater->GetFolderNameLen());
	szPath=new WCHAR[nPathLen+bNameLength+2];
	
	MemCopyAtoW(szPath,pLocater->GetCurrentPath(),nPathLen);
	szName=szPath+(++nPathLen);
	MemCopyAtoW(szName,pLocater->GetFolderName(),DWORD(bNameLength)+1);
	
	for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!='.' && bExtensionPos>0 ;bExtensionPos--);
	if (bExtensionPos==0)
		bExtensionPos=bNameLength;
	
	// Settig title (maybe)
	if ((GetLocateDlg()->GetFlags()&(CLocateDlg::fgLVAlwaysShowExtensions|CLocateDlg::fgLV1stCharUpper))==CLocateDlg::fgLVAlwaysShowExtensions)
	{
		szTitle=szName;
		dwFlags|=LITEM_TITLEOK;
	}

	// Setting file size;
	dwFileSize=DWORD(-1);
	wFileSizeHi=0;

	// Setting file time
	wModifiedDate=pLocater->GetFolderModifiedDate();
    wModifiedTime=pLocater->GetFolderModifiedTime();
	wCreatedDate=pLocater->GetFolderCreatedDate();
	wCreatedTime=pLocater->GetFolderCreatedTime();
	wAccessedDate=pLocater->GetFolderAccessedDate();
    wAccessedTime=pLocater->GetFolderAccessedTime();
	
	// Setting attributes
	bAttribs=pLocater->GetFolderAttributes()&LITEMATTRIB_DBATTRIBFLAG;
	
	iIcon=DIR_IMAGE;
	if (nPathLen<=3)
		iParentIcon=DRIVE_IMAGE;
	else
		iParentIcon=DIR_IMAGE;


	ItemDebugMessage("CLocatedItem::SetFolder END");
}

void CLocatedItem::SetFolderW(const CLocater* pLocater)
{
	ItemDebugMessage("CLocatedItem::SetFolderW BEGIN");

	wDatabaseID=pLocater->GetCurrentDatabaseID();
	wRootID=pLocater->GetCurrentDatabaseRootID();
	
	// Setting path, name and extension
	DWORD nPathLen=pLocater->GetCurrentPathLen();
	bNameLength=BYTE(pLocater->GetFolderNameLen());
	szPath=new WCHAR[nPathLen+bNameLength+2];
	
	MemCopyW(szPath,pLocater->GetCurrentPathW(),nPathLen);
	szName=szPath+(++nPathLen);
	MemCopyW(szName,pLocater->GetFolderNameW(),DWORD(bNameLength)+1);
	
	for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!='.' && bExtensionPos>0 ;bExtensionPos--);
	if (bExtensionPos==0)
		bExtensionPos=bNameLength;
	
	// Settig title (maybe)
	if ((GetLocateDlg()->GetFlags()&(CLocateDlg::fgLVAlwaysShowExtensions|CLocateDlg::fgLV1stCharUpper))==CLocateDlg::fgLVAlwaysShowExtensions)
	{
		szTitle=szName;
		dwFlags|=LITEM_TITLEOK;
	}

	// Setting file size;
	dwFileSize=DWORD(-1);
	wFileSizeHi=0;

	// Setting file time
	wModifiedDate=pLocater->GetFolderModifiedDateW();
    wModifiedTime=pLocater->GetFolderModifiedTimeW();
	wCreatedDate=pLocater->GetFolderCreatedDateW();
	wCreatedTime=pLocater->GetFolderCreatedTimeW();
	wAccessedDate=pLocater->GetFolderAccessedDateW();
    wAccessedTime=pLocater->GetFolderAccessedTimeW();
	
	// Setting attributes
	bAttribs=pLocater->GetFolderAttributes()&LITEMATTRIB_DBATTRIBFLAG;
	
	iIcon=DIR_IMAGE;
	if (nPathLen<=3)
		iParentIcon=DRIVE_IMAGE;
	else
		iParentIcon=DIR_IMAGE;


	ItemDebugMessage("CLocatedItem::SetFolder END");
}

void CLocatedItem::SetFile(const CLocater* pLocater)
{
	ItemDebugMessage("CLocatedItem::SetFile BEGIN");

	wDatabaseID=pLocater->GetCurrentDatabaseID();
	wRootID=pLocater->GetCurrentDatabaseRootID();
	
	// Setting path, name and extension
	DWORD nPathLen=pLocater->GetCurrentPathLen();
	bNameLength=BYTE(pLocater->GetFileNameLen());
	szPath=new WCHAR[nPathLen+bNameLength+2];
	MemCopyAtoW(szPath,pLocater->GetCurrentPath(),nPathLen);
	szName=szPath+(++nPathLen);
	MemCopyAtoW(szName,pLocater->GetFileName(),DWORD(bNameLength)+1);
	bExtensionPos=pLocater->GetFileExtensionPos();
	if (bExtensionPos==0)
		bExtensionPos=bNameLength;
	else
		bExtensionPos++;
	
	// Settig title (maybe)
	if ((GetLocateDlg()->GetFlags()&(CLocateDlg::fgLVAlwaysShowExtensions|CLocateDlg::fgLV1stCharUpper))==CLocateDlg::fgLVAlwaysShowExtensions)
	{
		szTitle=szName;
		dwFlags|=LITEM_TITLEOK;
	}

	// Setting file size;
	dwFileSize=pLocater->GetFileSizeLo();
	wFileSizeHi=pLocater->GetFileSizeHi();

	// Setting file time
	wModifiedDate=pLocater->GetFileModifiedDate();
	wModifiedTime=pLocater->GetFileModifiedTime();
	wCreatedDate=pLocater->GetFileCreatedDate();
	wCreatedTime=pLocater->GetFileCreatedTime();
	wAccessedDate=pLocater->GetFileAccessedDate();
	wAccessedTime=pLocater->GetFileAccessedTime();

	// Setting attributes
	bAttribs=pLocater->GetFileAttributes()&LITEMATTRIB_DBATTRIBFLAG;

	iIcon=DEF_IMAGE;
	if (nPathLen<=3)
		iParentIcon=DRIVE_IMAGE;
	else
		iParentIcon=DIR_IMAGE;

	ItemDebugMessage("CLocatedItem::SetFile END");

}

void CLocatedItem::SetFileW(const CLocater* pLocater)
{
	ItemDebugMessage("CLocatedItem::SetFileW BEGIN");

	wDatabaseID=pLocater->GetCurrentDatabaseID();
	wRootID=pLocater->GetCurrentDatabaseRootID();
	
	// Setting path, name and extension
	DWORD nPathLen=pLocater->GetCurrentPathLen();
	bNameLength=BYTE(pLocater->GetFileNameLen());
	szPath=new WCHAR[nPathLen+bNameLength+2];
	MemCopyW(szPath,pLocater->GetCurrentPathW(),nPathLen);
	szName=szPath+(++nPathLen);
	MemCopyW(szName,pLocater->GetFileNameW(),DWORD(bNameLength)+1);
	bExtensionPos=pLocater->GetFileExtensionPos();
	if (bExtensionPos==0)
		bExtensionPos=bNameLength;
	else
		bExtensionPos++;
	
	// Settig title (maybe)
	if ((GetLocateDlg()->GetFlags()&(CLocateDlg::fgLVAlwaysShowExtensions|CLocateDlg::fgLV1stCharUpper))==CLocateDlg::fgLVAlwaysShowExtensions)
	{
		szTitle=szName;
		dwFlags|=LITEM_TITLEOK;
	}

	// Setting file size;
	dwFileSize=pLocater->GetFileSizeLoW();
	wFileSizeHi=pLocater->GetFileSizeHiW();

	// Setting file time
	wModifiedDate=pLocater->GetFileModifiedDateW();
	wModifiedTime=pLocater->GetFileModifiedTimeW();
	wCreatedDate=pLocater->GetFileCreatedDateW();
	wCreatedTime=pLocater->GetFileCreatedTimeW();
	wAccessedDate=pLocater->GetFileAccessedDateW();
	wAccessedTime=pLocater->GetFileAccessedTimeW();

	// Setting attributes
	bAttribs=pLocater->GetFileAttributes()&LITEMATTRIB_DBATTRIBFLAG;

	iIcon=DEF_IMAGE;
	if (nPathLen<=3)
		iParentIcon=DRIVE_IMAGE;
	else
		iParentIcon=DIR_IMAGE;

	ItemDebugMessage("CLocatedItem::SetFileW END");

}

void CLocatedItem::ClearData()
{
	//ItemDebugMessage("CLocatedItem::ClearData BEGIN");

	if (szTitle!=NULL && szTitle!=szName)
		delete[] szTitle;
		
	szTitle=NULL;
	if (szPath!=NULL)
	{
		delete[] szPath;
		szPath=NULL;
	}
	if (szType!=NULL)
	{
		delete[] szType;
		szType=NULL;
	}
	
	DeleteAllExtraFields();

	//ItemDebugMessage("CLocatedItem::ClearData END");
}

void CLocatedItem::UpdateByDetail(CLocateDlg::DetailType nDetail)
{
	switch(nDetail)
	{
	case CLocateDlg::FullPath:
		UpdateFilename();
		break;
	case CLocateDlg::Title:
		UpdateTitle();
		UpdateIcon();
		break;
	case CLocateDlg::InFolder:
		UpdateParentIcon();
		break;
	case CLocateDlg::FileSize:
	case CLocateDlg::DateModified:
	case CLocateDlg::DateCreated:
	case CLocateDlg::DateAccessed:
		UpdateFileSizeAndTime();
		break;
	case CLocateDlg::FileType:
		UpdateType();
		break;
	case CLocateDlg::Attributes:
		UpdateAttributes();
		break;
	case CLocateDlg::ImageDimensions:
		UpdateDimensions();
		break;
	case CLocateDlg::Owner:
		UpdateOwner();
		break;
	case CLocateDlg::ShortFileName:
		UpdateShortFileName();
		break;
	case CLocateDlg::ShortFilePath:
		UpdateShortFilePath();
		break;
	case CLocateDlg::MD5sum:
		ComputeMD5sum();
		break;
	}	
}


	
BOOL CLocatedItem::ShouldUpdateByDetail(CLocateDlg::DetailType nDetail) const
{
	switch(nDetail)
	{
	case CLocateDlg::FullPath:
		return ShouldUpdateFilename();
	case CLocateDlg::Database:
	case CLocateDlg::DatabaseDescription:
	case CLocateDlg::DatabaseArchive:
	case CLocateDlg::VolumeLabel:
	case CLocateDlg::VolumeSerial:
	case CLocateDlg::VOlumeFileSystem:
		return FALSE;
	case CLocateDlg::Title:
		return ShouldUpdateTitle() || ShouldUpdateIcon();
	case CLocateDlg::InFolder:
		return ShouldUpdateParentIcon();
	case CLocateDlg::FileSize:
		return ShouldUpdateFileSize();
	case CLocateDlg::DateModified:
	case CLocateDlg::DateCreated:
	case CLocateDlg::DateAccessed:
		return ShouldUpdateTimeAndDate();
	case CLocateDlg::FileType:
		return ShouldUpdateType();
	case CLocateDlg::Attributes:
		return ShouldUpdateAttributes();
	default:
		return ShouldUpdateExtra(nDetail);
	}	
}

void CLocatedItem::UpdateFilename()
{
	WCHAR szFullPath[MAX_PATH];
	
	DWORD dwLength=GetLocateApp()->m_pGetLongPathName(GetPath(),szFullPath,MAX_PATH);
	if (dwLength==GetPathLen())
	{
		// Checking assumptions, i.e. length of file name and extension does not change 
		// and extension is same
		ASSERT(szFullPath[DWORD(szName-szPath)-1]=='\\' && 
			(szFullPath[DWORD(szName-szPath)+bExtensionPos-1]=='.' || 
			szFullPath[DWORD(szName-szPath)+bExtensionPos]=='\0'));

        //This fixes case
		MemCopyW(szPath,szFullPath,dwLength);
	}
	else if (dwLength>0)
	{
		delete[] szPath;
		szPath=new WCHAR[dwLength+2];
		MemCopyW(szPath,szFullPath,dwLength+1);
        
		szName=szPath+LastCharIndex(szPath,L'\\')+1;
		bNameLength=BYTE(dwLength-DWORD(szName-szPath));

		for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!=L'.' && bExtensionPos>0 ;bExtensionPos--);
		if (bExtensionPos==0)
			bExtensionPos=bNameLength;

	}
    
	dwFlags|=LITEM_FILENAMEOK;
}
	

void CLocatedItem::UpdateTitle()
{
	ItemDebugMessage("CLocatedItem::UpdateTitle BEGIN");
	//ItemDebugFormatMessage4("CLocatedItem::UpdateTitle1: %d",DWORD(GetLocateDlg()->GetFlags()&CLocateDlg::fgLVExtensionFlag),0,0,0);
	ItemDebugFormatMessage4("CLocatedItem::UpdateTitle1: ShouldUpdate=%d",ShouldUpdateTitle(),0,0,0);

	if (szTitle!=szName && szTitle!=NULL)
		delete[] szTitle;
	
	if (!(dwFlags&LITEM_FILENAMEOK))
		UpdateFilename();

	if ((GetLocateDlg()->GetFlags()&CLocateDlg::fgLVMethodFlag)==CLocateDlg::fgLVUseGetFileTitle)
	{
		if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
			return;
		
		szTitle=new WCHAR[bNameLength+1];
		WORD nLen=FileSystem::GetFileTitle(GetPath(),szTitle,bNameLength+1);
		if (nLen!=0)
		{
			delete[] szTitle;
			szTitle=new WCHAR[nLen];
			FileSystem::GetFileTitle(GetPath(),szTitle,nLen);
		}
	}
	else
	{
		switch (GetLocateDlg()->GetFlags()&CLocateDlg::fgLVExtensionFlag)
		{
		case CLocateDlg::fgLVAlwaysShowExtensions:
			szTitle=szName;
			break;
		case CLocateDlg::fgLVHideKnownExtensions:
			{
				if (bExtensionPos==bNameLength)
				{
					// No extension
					szTitle=szName;
					break;
				}
				BOOL bShowExtension=TRUE;
				CRegKey RegKey;
				CString Type;
				
				if (RegKey.OpenKey(HKCR,szName+bExtensionPos-1,CRegKey::openExist|CRegKey::samRead|CRegKey::samQueryValue)==ERROR_SUCCESS)
				{
					DWORD nTemp=0;
					// Trying first HKCR/.ext
					if (RegQueryValueEx(RegKey,"AlwaysShowExt",NULL,NULL,NULL,&nTemp)!=ERROR_SUCCESS)
					{
						// not found, trying HKCR/type
						RegKey.QueryValue("",Type);
						RegKey.CloseKey();
					
						if (!Type.IsEmpty())
						{
							if (RegKey.OpenKey(HKCR,Type,CRegKey::openExist|CRegKey::samRead|CRegKey::samQueryValue)==ERROR_SUCCESS)
							{
								if (RegQueryValueEx(RegKey,"AlwaysShowExt",NULL,NULL,NULL,&nTemp)!=ERROR_SUCCESS)
									bShowExtension=FALSE;
								else if (RegQueryValueEx(RegKey,"NeverShowExt",NULL,NULL,NULL,&nTemp)==ERROR_SUCCESS)
									bShowExtension=FALSE;

								if (ShouldUpdateType() && (GetLocateDlg()->GetFlags()&(CLocateDlg::fgLVShowFileTypes|CLocateDlg::fgLVShowShellType))==CLocateDlg::fgLVShowFileTypes)
								{
									// Taking type now
									DWORD nLength=RegKey.QueryValueLength("");
									if (nLength)
									{
										if (szType!=NULL)
											delete[] szType;
											
										szType=new WCHAR[nLength];
										if (RegKey.QueryValue(L"",szType,nLength))
											dwFlags|=LITEM_TYPEOK;
										else
											delete[] szType;
									}
								}
								RegKey.CloseKey();
							}
						}
					}
				}
				if (bShowExtension)
				{
					szTitle=szName;
					break;
				}			
			}
		case CLocateDlg::fgLVNeverShowExtensions:
			szTitle=new WCHAR[bExtensionPos];
			MemCopyW(szTitle,GetName(),DWORD(bExtensionPos)-1);
			szTitle[bExtensionPos-1]=L'\0';
			break;
		}
		if (GetLocateDlg()->GetFlags()&CLocateDlg::fgLV1stCharUpper)
		{
			BOOL bAllUpper=TRUE;
			DWORD i;
			for (i=0;szTitle[i]!='\0';i++)
			{
				// Todo: test this in Win9X
				if (!IsCharUpper(szTitle[i]))
				{
					bAllUpper=FALSE;
					break;
				}
			}
			if (bAllUpper)
			{
				if (szTitle==szName)
				{
					szTitle=new WCHAR[i+1];
					MemCopyW(szTitle,szName,i+1);
				}
				MakeLower(szTitle+1,i-1);
			}
		}
	}
	dwFlags|=LITEM_TITLEOK;

	ItemDebugMessage("CLocatedItem::UpdateTitle END");
}

void CLocatedItem::UpdateType() 
{
	ItemDebugMessage("CLocatedItem::UpdateType BEGIN");
	
	if (szType!=NULL)
		delete[] szType;
		


	if (!(GetLocateDlg()->GetFlags()&CLocateDlg::fgLVShowFileTypes))
	{
		WCHAR buf[300];
		DWORD dwLength;

		// File/folder does not exist
		if (IsFolder())
		{
			dwLength=LoadString(IDS_DIRECTORYTYPE,buf,300);
			szType=new WCHAR[dwLength+1];
			MemCopyW(szType,buf,dwLength+1);
		}
		else
		{
			dwLength=LoadString(IDS_UNKNOWNTYPE,buf,300)+1;		
			szType=new WCHAR[GetExtensionLength()+dwLength+1];
			MemCopyW(szType,GetExtension(),GetExtensionLength());
			szType[GetExtensionLength()]=L' ';
			MemCopyW(szType+GetExtensionLength()+1,buf,dwLength);
		}
		dwFlags|=LITEM_TYPEOK;
		return;
	}

	if (GetLocateDlg()->GetFlags()&CLocateDlg::fgLVShowShellType)
	{
		// Using shell functions
		SHFILEINFOW fi;
		if (ShouldUpdateIcon() && GetLocateDlg()->GetFlags()&CLocateDlg::fgLVShowIcons)
		{
			// Taking icon now too
			if (!GetFileInfo(GetPath(),0,&fi,SHGFI_TYPENAME|SHGFI_ICON|SHGFI_SYSICONINDEX))
			{
				SetToDeleted();
				return;
			}
			iIcon=fi.iIcon;
			DebugFormatMessage("dwFlags|=LITEM_ICONOK by UpdateType for %s",GetPath());
			dwFlags|=LITEM_ICONOK;
		}
		else if (!GetFileInfo(GetPath(),0,&fi,SHGFI_TYPENAME))
		{
			SetToDeleted();
			return;
		}
		
		DWORD dwTypeLen=istrlenw(fi.szTypeName);
		
		szType=new WCHAR[++dwTypeLen];
		MemCopyW(szType,fi.szTypeName,dwTypeLen);
		dwFlags|=LITEM_TYPEOK;
		return;
	}

	// File/folder does not exist
	if (IsFolder())
	{
		if (!FileSystem::IsDirectory(GetPath()))
		{
			SetToDeleted();
			return;
		}
		WCHAR szBuffer[80];
		DWORD dwTextLen=LoadString(IDS_DIRECTORYTYPE,szBuffer,80)+1;
		szType=new WCHAR[dwTextLen];
		MemCopyW(szType,szBuffer,dwTextLen);
		dwFlags|=LITEM_TYPEOK;
		return;
	}
	else
	{
		if (!FileSystem::IsFile(GetPath()))
		{
			SetToDeleted();
			return;
		}
	}

	CRegKey RegKey;
	CStringW Type;
	if (RegKey.OpenKey(HKCR,szName+bExtensionPos-1,CRegKey::openExist|CRegKey::samRead|CRegKey::samQueryValue)==ERROR_SUCCESS)
	{
		RegKey.QueryValue(L"",Type);
		RegKey.CloseKey();
		
		if (!Type.IsEmpty())
		{
			if (RegKey.OpenKey(HKCR,Type,CRegKey::openExist|CRegKey::samRead|CRegKey::samQueryValue)==ERROR_SUCCESS)
			{
				// Taking type now
				DWORD nLength=RegKey.QueryValueLength("");
				if (nLength)
				{
					szType=new WCHAR[nLength];
					if (RegKey.QueryValue(L"",szType,nLength))
					{
						dwFlags|=LITEM_TYPEOK;
						return;
					}
					else
						delete[] szType;

				}
				RegKey.CloseKey();
			}
		}
	}

	WCHAR szBuffer[80];
	DWORD dwTextLen=LoadString(IDS_UNKNOWNTYPE,szBuffer,80)+1;

	if (bExtensionPos!=bNameLength)
	{
		szType=new WCHAR[dwTextLen+GetExtensionLength()+1];
		MemCopyW(szType,GetExtension(),GetExtensionLength());
		MakeUpper(szType,GetExtensionLength());
		szType[GetExtensionLength()]=L' ';
		MemCopyW(szType+GetExtensionLength()+1,szBuffer,dwTextLen);
	}
	else // No extension
		szType=alloccopy(szBuffer,dwTextLen);
	
	dwFlags|=LITEM_TYPEOK;

	ItemDebugMessage("CLocatedItem::UpdateType END");
}

void CLocatedItem::UpdateAttributes()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;

	ItemDebugMessage("CLocatedItem::UpdateAttributes BEGIN");


	dwFlags|=LITEM_ATTRIBOK;
	if (IsDeleted())
		return;


	DWORD dwAttributes=FileSystem::GetFileAttributes(GetPath());
	if (dwAttributes==DWORD(-1))
	{
		SetToDeleted();
		return;
	}
	bAttribs=CDatabaseUpdater::GetAttribFlag(dwAttributes)|(bAttribs&LITEMATTRIB_DIRECTORY);
	
	ItemDebugMessage("CLocatedItem::UpdateAttributes END");
}

void CLocatedItem::UpdateFileSizeAndTime()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;

	ItemDebugMessage("CLocatedItem::UpdateFileSizeAndTime BEGIN");
	
	if (IsFolder())
	{
		union {
			WIN32_FIND_DATA fd;
			WIN32_FIND_DATAW fdw; // The beginning of the structures are equal
		};
		HANDLE hFind;
		if (IsUnicodeSystem())	
			hFind=FindFirstFileW(GetPath(),&fdw);
		else
			hFind=FindFirstFileA(W2A(GetPath()),&fd);

		if (hFind==INVALID_HANDLE_VALUE)
			SetToDeleted();
		else
		{
			FindClose(hFind);
			FILETIME ft2;
			
			FileTimeToLocalFileTime(&fd.ftLastWriteTime,&ft2);
			FileTimeToDosDateTime(&ft2,&wModifiedDate,&wModifiedTime);
			FileTimeToLocalFileTime(&fd.ftCreationTime,&ft2);
			FileTimeToDosDateTime(&ft2,&wCreatedDate,&wCreatedTime);
			FileTimeToLocalFileTime(&fd.ftLastAccessTime,&ft2);
			FileTimeToDosDateTime(&ft2,&wAccessedDate,&wAccessedTime);

			if (wAccessedTime==0)
				wAccessedTime=WORD(-1);

			dwFlags|=LITEM_TIMEDATEOK|LITEM_FILESIZEOK;
		}
		return;
	}

	HANDLE hFile;
	if (IsUnicodeSystem())
		hFile=CreateFileW(GetPath(),0,FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,OPEN_EXISTING,0,NULL);
	else
		hFile=CreateFile(W2A(GetPath()),0,FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,OPEN_EXISTING,0,NULL);

	if (hFile==INVALID_HANDLE_VALUE)
	{
		if (GetLastError()==ERROR_SHARING_VIOLATION)
		{
			// Another method to query information
			union {
				WIN32_FIND_DATA fd;
				WIN32_FIND_DATAW fdw; // The beginning of the structures are equal
			};
			HANDLE hFind;
			if (IsUnicodeSystem())	
				hFind=FindFirstFileW(GetPath(),&fdw);
			else
				hFind=FindFirstFileA(W2A(GetPath()),&fd);

			if (hFind!=INVALID_HANDLE_VALUE)
			{
				dwFileSize=fd.nFileSizeLow;
				wFileSizeHi=(BYTE)fd.nFileSizeHigh;


				FILETIME ft2;
				FileTimeToLocalFileTime(&fd.ftLastWriteTime,&ft2);
				FileTimeToDosDateTime(&ft2,&wModifiedDate,&wModifiedTime);
				FileTimeToLocalFileTime(&fd.ftCreationTime,&ft2);
				FileTimeToDosDateTime(&ft2,&wCreatedDate,&wCreatedTime);
				FileTimeToLocalFileTime(&fd.ftLastAccessTime,&ft2);
				FileTimeToDosDateTime(&ft2,&wAccessedDate,&wAccessedTime);
	
				dwFlags|=LITEM_TIMEDATEOK|LITEM_FILESIZEOK;

				FindClose(hFind);
				return;
			}
		}


		SetToDeleted();
		return;
	}

	DWORD dwFileSizeHiTemp;
	dwFileSize=::GetFileSize(hFile,&dwFileSizeHiTemp);
	wFileSizeHi=static_cast<BYTE>(dwFileSizeHiTemp);

	FILETIME ftCreated,ftModified,ftAccessed,ft2;
	GetFileTime(hFile,&ftCreated,&ftAccessed,&ftModified);
	FileTimeToLocalFileTime(&ftModified,&ft2);
	FileTimeToDosDateTime(&ft2,&wModifiedDate,&wModifiedTime);
	FileTimeToLocalFileTime(&ftCreated,&ft2);
	FileTimeToDosDateTime(&ft2,&wCreatedDate,&wCreatedTime);
	FileTimeToLocalFileTime(&ftAccessed,&ft2);
	FileTimeToDosDateTime(&ft2,&wAccessedDate,&wAccessedTime);
	
	if (wAccessedTime==0) // 0 wAccessedTime probably indicates that system does not save time
		wAccessedTime=WORD(-1);
	
	dwFlags|=LITEM_TIMEDATEOK|LITEM_FILESIZEOK;
	CloseHandle(hFile);

	ItemDebugMessage("CLocatedItem::UpdateFileSizeAndTime END");
}

void CLocatedItem::UpdateDimensions()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;

	ItemDebugMessage("CLocatedItem::UpdateDimensions BEGIN");
	
	if (GetLocateDlg()->m_pImageHandler==NULL)
		return;

	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::ImageDimensions);
	pField->bShouldUpdate=FALSE;

	SIZE dim;
	if (!GetLocateDlg()->m_pImageHandler->pGetImageDimensionsW(GetPath(),&dim))
	{
		pField->szImageDimension.cx=0;
		pField->szImageDimension.cy=0;
	}
	else
		pField->szImageDimension=dim;

	ItemDebugMessage("CLocatedItem::UpdateDimensions END");
}

void CLocatedItem::ComputeMD5sum(BOOL bForce)
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;

	ItemDebugMessage("CLocatedItem::ComputeMD5sum BEGIN");

	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::MD5sum);
	pField->bShouldUpdate=FALSE;

	if (!bForce && !(GetLocateDlg()->GetFlags()&CLocateDlg::fgLVComputeMD5Sums))
		return; 
		
	if (pField->szText!=NULL)
	{
		delete[] pField->szText;
		pField->szText=NULL;
	}

	if (IsFolder() || IsDeleted())
		return; // No need



	// Computing MD5 sum
	md5_state_t state;
	md5_byte_t digest[16];
	md5_init(&state);

	BOOL bOK=TRUE;
	CFile* pFile=NULL;
	try {
		pFile=new CFile(GetPath(),CFile::defRead,TRUE);
	
		md5_byte_t* pData=new md5_byte_t[1024];
	
		for (;;)
		{	
			DWORD dwRead=pFile->Read(pData,1024*sizeof(md5_byte_t));
			if (dwRead==0)
				break;
			md5_append(&state, pData, dwRead);
		}
        
		delete[] pData;		
		pFile->Close();
	}
	catch (...)
	{
		bOK=FALSE;
	}

	if (pFile!=NULL)
		delete pFile;

    if (bOK)
	{
		md5_finish(&state, digest);

		pField->szText=new WCHAR[2*16+1];

		for (int i=0;i<16;i++)
		{
			BYTE bHi=BYTE(digest[i]>>4)&0xF;
			BYTE bLo=BYTE(digest[i]&0xF);

			pField->szText[i*2]=bHi>=10?bHi-10+L'a':bHi+L'0';
			pField->szText[i*2+1]=bLo>=10?bLo-10+L'a':bLo+L'0';
		}
		pField->szText[16*2]=L'\0';
	}

	ItemDebugMessage("CLocatedItem::ComputeMD5sum END");
}

void CLocatedItem::UpdateOwner()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;
	
	ItemDebugMessage("CLocatedItem::UpdateOwner BEGIN");
	
	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::Owner);
	pField->bShouldUpdate=FALSE;

	if (IsDeleted())
		return;

	if (pField->szText!=NULL)
	{
		delete[] pField->szText;
		pField->szText=NULL;
	}
			
    	
	DWORD dwNeeded=0;
	if (!FileSystem::GetFileSecurity(GetPath(),OWNER_SECURITY_INFORMATION,NULL,0,&dwNeeded))
	{
		if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
			return;
	}

    PSECURITY_DESCRIPTOR pDesc=(PSECURITY_DESCRIPTOR)new BYTE[dwNeeded+2];

    if (!FileSystem::GetFileSecurity(szPath,OWNER_SECURITY_INFORMATION,pDesc,dwNeeded+2,&dwNeeded))
	{
		delete[] (BYTE*) pDesc;
		return;
	}
	
	PSID psid;
	BOOL bDefaulted=TRUE;
	if (!GetSecurityDescriptorOwner(pDesc,&psid,&bDefaulted))
	{
		delete[] (BYTE*) pDesc;
		return;
	}
	

	WCHAR szOwner[200],szDomain[200];
	DWORD dwOwnerLen=199,dwDomainLen=199;
	SID_NAME_USE sUse;
	
	BOOL bRet;
	if (szPath[0]==L'\\' && szPath[1]==L'\\')
	{
		DWORD dwLength=2+(DWORD)FirstCharIndex(szPath+2,L'\\');
		WCHAR* szServer=new WCHAR[dwLength+1];
		MemCopyW(szServer,szPath,dwLength);
		szServer[dwLength]='\0';
		bRet=FileSystem::LookupAccountSid(szServer,psid,szOwner,&dwOwnerLen,szDomain,&dwDomainLen,&sUse);
		delete[] szServer;
	}
	else
		bRet=FileSystem::LookupAccountSid(NULL,psid,szOwner,&dwOwnerLen,szDomain,&dwDomainLen,&sUse);

	if (bRet)
	{
		pField->szText=new WCHAR[dwOwnerLen+dwDomainLen+2];
		if (dwDomainLen>0)
		{
			MemCopyW(pField->szText,szDomain,dwDomainLen);
			pField->szText[dwDomainLen]=L'\\';
			MemCopyW(pField->szText+dwDomainLen+1,szOwner,dwOwnerLen+1);                
		}
		else
			MemCopyW(pField->szText,szOwner,dwOwnerLen+1);
	}
	delete[] (BYTE*) pDesc;

	ItemDebugMessage("CLocatedItem::UpdateOwner END");
}

void CLocatedItem::UpdateShortFileName()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;

	ItemDebugFormatMessage4("CLocatedItem::UpdateShortFileName BEGIN %s",GetPath(),0,0,0);
	
	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::ShortFileName);
	pField->bShouldUpdate=FALSE;

	if (IsDeleted())
		return;

	WCHAR szShortPath[MAX_PATH];
	DWORD nLength=FileSystem::GetShortPathName(GetPath(),szShortPath,MAX_PATH);
	if (nLength==0)
	{
		if (pField->szText!=NULL)
			delete[] pField->szText; 
		pField->szText=allocemptyW();
		ItemDebugMessage("CLocatedItem::UpdateShortFileName END2");
		return;
	}

	int nStart;
	for (nStart=nLength-1;nStart>=0 && szShortPath[nStart]!='\\';nStart--);
	if (nStart>0)
		nStart++;
    nLength-=nStart;

	if (pField->szText!=NULL)
		delete[] pField->szText; 

	pField->szText=alloccopy(szShortPath+nStart,nLength);

	ItemDebugMessage("CLocatedItem::UpdateShortFileName END");
}

void CLocatedItem::UpdateShortFilePath()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;

	ItemDebugMessage("CLocatedItem::UpdateShortFilePath BEGIN");
	
	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::ShortFilePath);
	pField->bShouldUpdate=FALSE;
	if (IsDeleted())
		return;

	WCHAR szShortPath[MAX_PATH];
	DWORD nLength=FileSystem::GetShortPathName(GetPath(),szShortPath,MAX_PATH);
	if (nLength==0)
	{
		if (pField->szText!=NULL)
			delete[] pField->szText; 

		pField->szText=allocemptyW();
		ItemDebugMessage("CLocatedItem::UpdateShortFilePath END2");
	}
	


	if (pField->szText!=NULL)
		delete[] pField->szText; 

	pField->szText=alloccopy(szShortPath,nLength);

	ItemDebugMessage("CLocatedItem::UpdateShortFilePath END");
}


void CLocatedItem::SetToDeleted()
{
	ItemDebugMessage("CLocatedItem::SetToDeleted BEGIN");
	
	WCHAR szBuffer[80];
	DWORD dwLength=LoadString(IDS_DELETEDFILE,szBuffer,80)+1;
	if (szType!=NULL)
		delete[] szType;
	szType=alloccopy(szBuffer,dwLength);
	iIcon=DEL_IMAGE;
	
	dwFileSize=DWORD(-1);
	wModifiedDate=WORD(-1);
	wModifiedTime=WORD(-1);
	wCreatedDate=WORD(-1);
	wCreatedTime=WORD(-1);
	wAccessedDate=WORD(-1);
	wAccessedTime=WORD(-1);

    DeleteAllExtraFields();

	dwFlags&=~LITEM_PARENTICONOK;
	dwFlags|=LITEM_ICONOK|LITEM_TYPEOK|LITEM_TIMEDATEOK|LITEM_FILESIZEOK;
	DebugFormatMessage(L"dwFlags|=LITEM_ICONOK by SetToDeleted for %s",GetPath());

	ItemDebugMessage("CLocatedItem::SetToDeleted END");
}


BOOL CLocatedItem::RemoveFlagsForChanged()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return FALSE;

	ItemDebugFormatMessage4("CLocatedItem::RemoveFlagsForChanged BEGIN, item:%S, flags already=%X",GetPath(),dwFlags,0,0);
	

	union {
		WIN32_FIND_DATA fd;
		WIN32_FIND_DATAW fdw;
	};
	
	// Checking whether file is available
	HANDLE hFind;
	if (IsUnicodeSystem())
		hFind=FindFirstFileW(GetPath(),&fdw);
	else
		hFind=FindFirstFile(W2A(GetPath()),&fd);

	if (hFind==INVALID_HANDLE_VALUE)
	{
		if (!IsDeleted())
		{
			SetToDeleted();
			ItemDebugMessage("CLocatedItem::RemoveFlagsForChanged ENDUD");
			return TRUE;
		}
		ItemDebugMessage("CLocatedItem::RemoveFlagsForChanged ENDDD");
		return FALSE;
	}
	FindClose(hFind);
    
	// Checking whether filename and title is correct
	WCHAR szFullPath[MAX_PATH];
	DWORD dwLength=GetLocateApp()->m_pGetLongPathName(GetPath(),szFullPath,MAX_PATH);
	if (dwLength==GetPathLen())
	{
		// Checking assumptions, i.e. length of file name and extension does not change 
		// and extension is same
		ASSERT(szFullPath[DWORD(szName-szPath)-1]==L'\\' && 
			(szFullPath[DWORD(szName-szPath)+bExtensionPos-1]==L'.' || 
			szFullPath[DWORD(szName-szPath)+bExtensionPos]==L'\0'));

        //This fixes case
		if (wcsncmp(szPath,szFullPath,dwLength)!=0)
		{
			RemoveFlags(LITEM_TITLEOK);   
			MemCopyW(szPath,szFullPath,dwLength);
		}
		AddFlags(LITEM_FILENAMEOK);
	}
	else if (dwLength>0)
	{
        delete[] szPath;
		szPath=new WCHAR[dwLength+2];
		MemCopyW(szPath,szFullPath,dwLength+1);
        
		szName=szPath+LastCharIndex(szPath,L'\\')+1;
		bNameLength=BYTE(dwLength-DWORD(szName-szPath));

		for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!=L'.' && bExtensionPos>0 ;bExtensionPos--);
		if (bExtensionPos==0)
			bExtensionPos=bNameLength;
	
		AddFlags(LITEM_FILENAMEOK);
		RemoveFlags(LITEM_TITLEOK);
	}

	if (IsDeleted())
	{
		// File has come back
		dwFlags&=~(LITEM_COULDCHANGE|LITEM_TITLEOK);

		// Settings information obtained from WIN32_FIND_DATA structure
		if (!IsFolder())
		{
			dwFileSize=fd.nFileSizeLow;
			wFileSizeHi=static_cast<BYTE>(fd.nFileSizeHigh);
		}

				
		FILETIME ftLocal;
		FileTimeToLocalFileTime(&fd.ftLastWriteTime,&ftLocal);
		FileTimeToDosDateTime(&ftLocal,&wModifiedDate,&wModifiedTime);
	
		FileTimeToLocalFileTime(&fd.ftCreationTime,&ftLocal);
		FileTimeToDosDateTime(&ftLocal,&wCreatedDate,&wCreatedTime);
	
		FileTimeToLocalFileTime(&fd.ftLastAccessTime,&ftLocal);
		FileTimeToDosDateTime(&ftLocal,&wAccessedDate,&wAccessedTime);
		
		bAttribs=CDatabaseUpdater::GetAttribFlag(fd.dwFileAttributes)|(bAttribs&LITEMATTRIB_DIRECTORY);
				
		dwFlags|=LITEM_TIMEDATEOK|LITEM_FILESIZEOK|LITEM_ATTRIBOK;
		return TRUE;
	}

	

	BOOL bRet=FALSE;

	BYTE bAttribFlag=CDatabaseUpdater::GetAttribFlag(fd.dwFileAttributes)|(bAttribs&LITEMATTRIB_DIRECTORY);
	if (bAttribFlag!=bAttribs)
	{
		bRet=TRUE;
		bAttribs=bAttribFlag|(bAttribs&LITEMATTRIB_CUTTED);
		dwFlags|=LITEM_ATTRIBOK;
	}

	if (!IsFolder() && fd.nFileSizeLow!=dwFileSize)
	{
		dwFileSize=fd.nFileSizeLow;
		wFileSizeHi=static_cast<BYTE>(fd.nFileSizeHigh);

		dwFlags|=LITEM_FILESIZEOK;
		ExtraSetUpdateWhenFileSizeChanged();
		bRet=TRUE;
	}
		

	WORD wTempCreationTime,wTempCreationDate;
	WORD wTempModificationTime,wTempModificationDate;
	WORD wTempAccessedTime,wTempAccessedDate;

	FILETIME ftLocal;
	FileTimeToLocalFileTime(&fd.ftLastWriteTime,&ftLocal);
	FileTimeToDosDateTime(&ftLocal,&wTempModificationDate,&wTempModificationTime);
	
	FileTimeToLocalFileTime(&fd.ftCreationTime,&ftLocal);
	FileTimeToDosDateTime(&ftLocal,&wTempCreationDate,&wTempCreationTime);
	
	FileTimeToLocalFileTime(&fd.ftLastAccessTime,&ftLocal);
	FileTimeToDosDateTime(&ftLocal,&wTempAccessedDate,&wTempAccessedTime);

	if (wTempModificationDate!=wModifiedDate || wTempModificationTime!=wModifiedTime ||
		wTempCreationDate!=wCreatedDate || wTempCreationTime!=wCreatedTime)
	{
		bRet=TRUE;
		wModifiedDate=wTempModificationDate;
		wModifiedTime=wTempModificationTime;
		wCreatedTime=wTempCreationTime;
		wCreatedDate=wTempCreationDate;
		wAccessedTime=wTempAccessedTime;
		wAccessedDate=wTempAccessedDate;
		ExtraSetUpdateWhenFileSizeChanged();
		dwFlags|=LITEM_TIMEDATEOK;
	}
	else if (wTempAccessedDate!=wAccessedDate || wTempAccessedTime!=wAccessedDate)
	{
		wAccessedTime=wTempAccessedTime;
		wAccessedDate=wTempAccessedDate;
		dwFlags|=LITEM_TIMEDATEOK;
	}

	ItemDebugFormatMessage4("CLocatedItem::RemoveFlagsForChanged END, flagsnow=%X",dwFlags,0,0,0);
	
	return bRet;
}
	
BOOL CLocatedItem::IsItemShortcut() const
{
	CRegKey RegKey;
	CString Type;
	if (RegKey.OpenKey(HKCR,szName+bExtensionPos-1,CRegKey::openExist|CRegKey::samRead|CRegKey::samQueryValue)==ERROR_SUCCESS)
	{
		if (RegKey.QueryValueLength("IsShortcut")>0)
			return TRUE;

		RegKey.QueryValue("",Type);
		RegKey.CloseKey();
		
		if (!Type.IsEmpty())
		{
			if (RegKey.OpenKey(HKCR,Type,CRegKey::openExist|CRegKey::samRead|CRegKey::samQueryValue)==ERROR_SUCCESS)
				return RegKey.QueryValueLength("IsShortcut")>0;
		}
	}
	return FALSE;
}

void CLocatedItem::DeleteExtraInfoField(CLocateDlg::DetailType nType)
{
	if (pFirstExtraInfo==NULL)
		return;
	if (pFirstExtraInfo->nType==nType)
	{
		ExtraInfo* pTmp=pFirstExtraInfo;
		pFirstExtraInfo=pTmp->pNext;
		delete pTmp;
	}
	else
	{
		ExtraInfo* pTmp=pFirstExtraInfo;
		while (pTmp->pNext!=NULL)
		{
			if (pTmp->pNext->nType==nType)
			{
				ExtraInfo* pTmp2=pTmp->pNext;
				pTmp->pNext=pTmp2->pNext;
				delete pTmp2;
				return;
			}
			pTmp=pTmp->pNext;
		}
	}
}

void CLocatedItem::ChangeName(CWnd* pWnd,LPCWSTR szNewName,int iLength)
{
	if (iLength==-1)
		iLength=(int)istrlenw(szNewName);

	DWORD dwDirectoryLen=DWORD(szName-szPath);
	ASSERT(dwDirectoryLen>0);

	WCHAR* szOldPath=GetPath();;
	WCHAR* szNewPath=new WCHAR[dwDirectoryLen+iLength+2];
	
	// Copying directory
	MemCopyW(szNewPath,szOldPath,dwDirectoryLen);
	MemCopyW(szNewPath+dwDirectoryLen,szNewName,iLength);
	szNewPath[dwDirectoryLen+iLength]=L'\0';
	

	if (!FileSystem::MoveFile(szOldPath,szNewPath))
	{
		
		CHAR* pError;
		CStringW str;
			
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,NULL,
			GetLastError(),LANG_USER_DEFAULT,(LPSTR)&pError,0,NULL))
		{
			str.Format(IDS_ERRORCANNOTRENAME,(LPCWSTR)A2W(pError));
			LocalFree(pError);
		}
		else
			str.Format(IDS_ERRORCANNOTRENAME,L"");

		pWnd->MessageBox(str,ID2W(IDS_ERROR),MB_OK|MB_ICONERROR);

		delete[] szNewPath;
		return;
	}





	
	// Do fileoperation first
	if (szTitle!=szName && szTitle!=NULL)
	{
		WCHAR* pTemp=szTitle;
		InterlockedExchangePointer((PVOID*)&szTitle,NULL);
		delete[] pTemp;
	}

	InterlockedExchangePointer((PVOID*)&szName,szNewPath+dwDirectoryLen);
	bNameLength=iLength;
	InterlockedExchangePointer((PVOID*)&szPath,szNewPath);
	
	// Finding extension
	for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!=L'.' && bExtensionPos>0 ;bExtensionPos--);
	if (bExtensionPos==0)
		bExtensionPos=bNameLength;

	delete[] szOldPath;

	AddFlags(LITEM_FILENAMEOK);
	
	UpdateTitle();
}

LPWSTR CLocatedItem::GetToolTipText() const
{
	ISDLGTHREADOK

	
	if (IsDeleted())
	{
		if (g_szwBuffer!=NULL)
			delete[] g_szwBuffer;
		CStringW str(IsFolder()?IDS_TOOLTIPFORDIRECTORYDELETED:IDS_TOOLTIPFORFILEDELETED);
		int nLen=(int)str.GetLength()+GetPathLen()+(int)istrlenw(GetType())+2;
		g_szwBuffer=new WCHAR[nLen];
		swprintfex(g_szwBuffer,nLen,str,GetName(),GetParent(),GetType());
		return g_szwBuffer;
	}

	
		
	if (ShouldUpdateFileSizeOrDate())
		((CLocatedItem*)this)->UpdateFileSizeAndTime();
	if (ShouldUpdateType())
		((CLocatedItem*)this)->UpdateType();

	WCHAR* szDate=GetLocateApp()->FormatDateAndTimeString(GetModifiedDate(),GetModifiedTime());
	

	if (IsFolder())
	{
		CStringW str(IDS_TOOLTIPFORDIRECTORY);
		int nLen=(int)str.GetLength()+GetPathLen()+(int)istrlenw(GetType())+(int)istrlenw(szDate)+2;
		if (g_szwBuffer!=NULL)
			delete[] g_szwBuffer;
		g_szwBuffer=new WCHAR[nLen];
		swprintfex(g_szwBuffer,nLen,str,GetName(),GetParent(),GetType(),szDate);
	}
	else 
	{
		CStringW text;
		
		WCHAR szSize[25];

		if (GetFileSize()>LONGLONG(1024*1024*1024)) // Over 1 Gb
		{
			StringCbPrintfW(szSize,25*sizeof(WCHAR),L"%1.2f",double(GetFileSize())/(1024*1024*1024));
			int nLength=(int)istrlenw(szSize);
			while (szSize[nLength-1]=='0')
				nLength--;
			if (szSize[nLength-1]=='.')
				nLength--;
			::LoadString(IDS_GB,szSize+nLength,25-nLength);
		}	
		else if (GetFileSize()>LONGLONG(1024*1024)) // Over 1 Mb
		{
			StringCbPrintfW(szSize,25*sizeof(WCHAR),L"%1.2f",double(GetFileSize())/(1024*1024));
			int nLength=(int)wcslen(szSize);
			while (szSize[nLength-1]=='0')
				nLength--;
			if (szSize[nLength-1]=='.')
				nLength--;
			::LoadString(IDS_MB,szSize+nLength,25-nLength);
		}	
		else if (GetFileSize()>LONGLONG(1024)) // Over 1 Gb
		{
			StringCbPrintfW(szSize,25*sizeof(WCHAR),L"%1.2f",double(GetFileSize())/(1024));
			int nLength=(int)wcslen(szSize);
			while (szSize[nLength-1]=='0')
				nLength--;
			if (szSize[nLength-1]=='.')
				nLength--;
			::LoadString(IDS_KB,szSize+nLength,25-nLength);
		}	
		else
		{
			HRESULT hRes=StringCbPrintfW(szSize,25*sizeof(WCHAR),L"%d",GetFileSizeLo());
			int nLength=(int)wcslen(szSize);
			if (SUCCEEDED(hRes))
				::LoadString(IDS_BYTES,szSize+nLength,25-nLength);
			else
				szSize[0]='\0';
		}

		text.FormatEx(IDS_TOOLTIPFORFILE,GetName(),GetParent(),GetType(),szDate,szSize);
			
		if (IsItemShortcut())
		{
			// Get command and target
			
			union {
				IShellLinkW* pslw;
				IShellLink* psl;
			};
			IPersistFile* ppf=NULL;
			psl=NULL;
			
			if (IsUnicodeSystem())
			{
				if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLinkW,(void**)&pslw)))
				{
					if (FAILED(pslw->QueryInterface(IID_IPersistFile,(void**)&ppf)))
						ppf=NULL;
				}
			}
			else
			{
				if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&psl)))
				{
					if (FAILED(psl->QueryInterface(IID_IPersistFile,(void**)&ppf)))
						ppf=NULL;
				}
			}

			if (ppf!=NULL)
			{
				if (SUCCEEDED(ppf->Load(GetPath(),STGM_READ)))
				{
					WCHAR szBuffer[INFOTIPSIZE+2];
					HRESULT hRes=pslw->GetPath(szBuffer,INFOTIPSIZE+2,NULL,0);
					if (SUCCEEDED(hRes))
					{
						text << L"\r\n";
						text.AddString(IDS_TOOLTIPTARGET);
						text << L' ' << szBuffer;
					}

					szBuffer[0]='\0';
					hRes=pslw->GetDescription(szBuffer,INFOTIPSIZE+2);
					if (SUCCEEDED(hRes) && szBuffer[0]!='\0')
					{
						text << L"\r\n";
						text.AddString(IDS_TOOLTIPDESCRIPTION);
						text << L' ' << szBuffer;
					}

	
				}
				ppf->Release();
			}

			if (psl!=NULL)
			{
				if (IsUnicodeSystem())
					pslw->Release();
				else
					psl->Release();
			}
		}

		if (g_szwBuffer!=NULL)
			delete[] g_szwBuffer;
		g_szwBuffer=text.GiveBuffer();
	}

	delete[] szDate;
	return g_szwBuffer;
}