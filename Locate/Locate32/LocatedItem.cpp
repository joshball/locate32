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
		szFileTitle=szName;
		dwFlags|=LITEM_FILETITLEOK;
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
		szFileTitle=szName;
		dwFlags|=LITEM_FILETITLEOK;
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
	if (bExtensionPos==0 && *szName!='.')
		bExtensionPos=bNameLength;
	else
		bExtensionPos++;
	
	// Settig title (maybe)
	if ((GetLocateDlg()->GetFlags()&(CLocateDlg::fgLVAlwaysShowExtensions|CLocateDlg::fgLV1stCharUpper))==CLocateDlg::fgLVAlwaysShowExtensions)
	{
		szFileTitle=szName;
		dwFlags|=LITEM_FILETITLEOK;
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
	if (bExtensionPos==0 && *szName!='.')
		bExtensionPos=bNameLength;
	else
		bExtensionPos++;
	
	ASSERT(!FileSystem::IsDirectory(GetPath()));

	// Settig title (maybe)
	if ((GetLocateDlg()->GetFlags()&(CLocateDlg::fgLVAlwaysShowExtensions|CLocateDlg::fgLV1stCharUpper))==CLocateDlg::fgLVAlwaysShowExtensions)
	{
		szFileTitle=szName;
		dwFlags|=LITEM_FILETITLEOK;
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

	if (szFileTitle!=NULL && szFileTitle!=szName)
		delete[] szFileTitle;
		
	szFileTitle=NULL;
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
	case CLocateDlg::Name:
		UpdateFileTitle();
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
	case CLocateDlg::Author:
	case CLocateDlg::Title:
	case CLocateDlg::Subject:
	case CLocateDlg::Pages:
	case CLocateDlg::Comments:
		UpdateSummaryProperties();
		break;
	case CLocateDlg::Category:
		UpdateDocSummaryProperties();
		break;
	case CLocateDlg::Description:
	case CLocateDlg::FileVersion:
	case CLocateDlg::ProductName:
	case CLocateDlg::ProductVersion:
		UpdateVersionInformation();
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
	case CLocateDlg::Name:
		return ShouldUpdateFileTitle() || ShouldUpdateIcon();
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


NDEBUGINLINE void CLocatedItem::UpdateFilename()
{
	/*
#ifdef _DEBUG
	// Convert short file name to long file name
	WCHAR szFullPath[MAX_PATH];
	ItemDebugMessage("CLocatedItem::UpdateFilename() BEGIN");
	
	DWORD dwLength=GetLocateApp()->m_pGetLongPathName(GetPath(),szFullPath,MAX_PATH);

	if (wcscmp(GetPath(),szFullPath)!=0)
	{
		CAppData::stdfunc();
	}

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
		WCHAR* pTmp=szPath;
		InterlockedExchangePointer((PVOID*)&szPath,alloccopy(szFullPath,dwLength));
		delete[] pTmp;

		InterlockedExchangePointer((PVOID*)&szName,szPath+LastCharIndex(szPath,L'\\')+1);
		bNameLength=BYTE(dwLength-DWORD(szName-szPath));

		for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!=L'.' && bExtensionPos>0 ;bExtensionPos--);
		if (bExtensionPos==0)
			bExtensionPos=bNameLength;
	}
    
#endif
	*/

	dwFlags|=LITEM_FILENAMEOK;

	ItemDebugMessage("CLocatedItem::UpdateFilename() END");
}
	

void CLocatedItem::UpdateFileTitle()
{
	ItemDebugMessage("CLocatedItem::UpdateFileTitle BEGIN");
	
	
	if (!(dwFlags&LITEM_FILENAMEOK))
		UpdateFilename();

	WCHAR* pNewFileTitle=NULL;
	BOOL bCheckCase=TRUE;

	if (IsFolder())
		pNewFileTitle=szName; // Show full name for folders
	else if ((GetLocateDlg()->GetFlags()&CLocateDlg::fgLVMethodFlag)==CLocateDlg::fgLVUseGetFileTitle)
	{
		WORD nLen=FileSystem::GetFileTitle(GetPath(),NULL,0);
		if (nLen!=0)
		{
			pNewFileTitle=new WCHAR[nLen];
			FileSystem::GetFileTitle(GetPath(),pNewFileTitle,nLen);
			bCheckCase=FALSE;
		}
		else 
			pNewFileTitle=szName;
	}
	else
	{
		switch (GetLocateDlg()->GetFlags()&CLocateDlg::fgLVExtensionFlag)
		{
		case CLocateDlg::fgLVAlwaysShowExtensions:
			pNewFileTitle=szName;
			break;
		case CLocateDlg::fgLVHideKnownExtensions:
			{
				if (bExtensionPos==bNameLength)
				{
					// No extension
					pNewFileTitle=szName;
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
									if (nLength>0)
									{
										WCHAR* pNewType=new WCHAR[nLength+1];
										if (RegKey.QueryValue(L"",pNewType,nLength))
										{
											WCHAR* pTmp=szType;
											InterlockedExchangePointer((VOID**)&szType,pNewType);
											dwFlags|=LITEM_TYPEOK;
											if (pTmp!=NULL)
												delete[] pTmp;
										}
										else
											delete[] pNewType;
									}
								}
								RegKey.CloseKey();
							}
						}
					}
				}
				if (bShowExtension)
				{
					pNewFileTitle=szName;
					break;
				}		
				// Continuing
			}
		case CLocateDlg::fgLVNeverShowExtensions:
			if (bExtensionPos==bNameLength)
				pNewFileTitle=szName;
			else
				pNewFileTitle=alloccopy(GetName(),DWORD(bExtensionPos)-1);
			break;
		}
	}

	if (!bCheckCase && GetLocateDlg()->GetFlags()&CLocateDlg::fgLV1stCharUpper)
	{
		BOOL bAllUpper=TRUE;
		DWORD i;
		for (i=0;pNewFileTitle[i]!='\0';i++)
		{
			if (!IsCharUpper(pNewFileTitle[i]))
			{
				bAllUpper=FALSE;
				break;
			}
		}
		if (bAllUpper)
		{
			if (pNewFileTitle==szName)
			{
				pNewFileTitle=new WCHAR[i+1];
				MemCopyW(pNewFileTitle,szName,i+1);
			}
			MakeLower(pNewFileTitle+1,i-1);
		}
	}

	ASSERT(pNewFileTitle!=NULL);

	WCHAR* pTmp=szFileTitle;
	InterlockedExchangePointer((PVOID*)&szFileTitle,pNewFileTitle);
	if (pTmp!=szName && pTmp!=NULL)
		delete[] pTmp;
	
	dwFlags|=LITEM_FILETITLEOK;

	ItemDebugMessage("CLocatedItem::UpdateFileTitle END");
}

void CLocatedItem::UpdateType() 
{
	ItemDebugMessage("CLocatedItem::UpdateType BEGIN");
	
	WCHAR* pNewType=NULL;
		
	if (!(GetLocateDlg()->GetFlags()&CLocateDlg::fgLVShowFileTypes))
	{
		WCHAR buf[300];
		DWORD dwLength;

		// No extension
		if (IsFolder())
			pNewType=allocstringW(IDS_DIRECTORYTYPE);
		else
		{
			dwLength=LoadString(IDS_UNKNOWNTYPE,buf,300)+1;		
			pNewType=new WCHAR[GetExtensionLength()+dwLength+1];
			MemCopyW(pNewType,GetExtension(),GetExtensionLength());
			pNewType[GetExtensionLength()]=L' ';
			MemCopyW(pNewType+GetExtensionLength()+1,buf,dwLength);
		}
	}
	else if (GetLocateDlg()->GetFlags()&CLocateDlg::fgLVShowShellType)
	{
		// Using shell functions
		SHFILEINFOW fi;
		if (ShouldUpdateIcon() && GetLocateDlg()->GetFlags()&CLocateDlg::fgLVShowIcons)
		{
			// Taking icon now too
			if (!GetFileInfo(GetPath(),0,&fi,SHGFI_TYPENAME|SHGFI_ICON|SHGFI_SYSICONINDEX))
			{
				// File does not exist
				SetToDeleted();
				ItemDebugMessage("CLocatedItem::UpdateType END1");
			}
			iIcon=fi.iIcon;
			DebugFormatMessage("dwFlags|=LITEM_ICONOK by UpdateType for %s",GetPath());
			dwFlags|=LITEM_ICONOK;
		}
		else if (!GetFileInfo(GetPath(),0,&fi,SHGFI_TYPENAME))
		{
			// File does not exist
			SetToDeleted();
			ItemDebugMessage("CLocatedItem::UpdateType END2");
			return;
		}
		
		pNewType=alloccopy(fi.szTypeName);
	}
	else if (IsFolder())
	{
		if (!FileSystem::IsDirectory(GetPath()))
		{
			// Folder does not exist
			SetToDeleted();
			ItemDebugMessage("CLocatedItem::UpdateType END3");
			return;
		}
		pNewType=allocstringW(IDS_DIRECTORYTYPE);
	}
	else
	{
		if (!FileSystem::IsFile(GetPath()))
		{
			SetToDeleted();
			ItemDebugMessage("CLocatedItem::UpdateType END4");
			return;
		}
	
		CRegKey RegKey;
		CStringW Type;
		BOOL bOK=FALSE;

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
					ItemDebugFormatMessage1("CLocatedItem::UpdateType Type from registry, %d",nLength);
					if (nLength>0)
					{
						pNewType=new WCHAR[nLength+1];
						if (RegKey.QueryValue(L"",pNewType,nLength))
						{
							ItemDebugMessage("CLocatedItem::UpdateType Type from registry2");
							ItemDebugMessage(pNewType);
							
							bOK=TRUE;
						}
						else
							delete[] pNewType;
					}
					RegKey.CloseKey();
				}
			}
		}

		if (!bOK)
		{
			WCHAR szBuffer[300];
			DWORD dwTextLen=LoadString(IDS_UNKNOWNTYPE,szBuffer,300)+1;

			if (bExtensionPos!=bNameLength)
			{
				pNewType=new WCHAR[dwTextLen+GetExtensionLength()+1];
				MemCopyW(pNewType,GetExtension(),GetExtensionLength());
				MakeUpper(pNewType,GetExtensionLength());
				pNewType[GetExtensionLength()]=L' ';
				MemCopyW(pNewType+GetExtensionLength()+1,szBuffer,dwTextLen);
			}
			else // No extension
				pNewType=alloccopy(szBuffer,dwTextLen);
		}	
	}
	
	WCHAR* pTmp=szType;
	InterlockedExchangePointer((PVOID*)&szType,pNewType);
	if (pTmp!=NULL)
		delete[] pTmp;

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
		DebugOpenHandle(dhtFileFind,hFind,GetPath());

		if (hFind==INVALID_HANDLE_VALUE)
			SetToDeleted();
		else
		{
			FindClose(hFind);
			DebugCloseHandle(dhtFileFind,hFind,GetPath());
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

		ItemDebugMessage("CLocatedItem::UpdateFileSizeAndTime END1");
		return;
	}

	HANDLE hFile;
	if (IsUnicodeSystem())
		hFile=CreateFileW(GetPath(),0,FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,OPEN_EXISTING,0,NULL);
	else
		hFile=CreateFile(W2A(GetPath()),0,FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,OPEN_EXISTING,0,NULL);
	DebugOpenHandle(dhtFile,hFile,GetPath());

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
			DebugOpenHandle(dhtFileFind,hFind,GetPath());
			
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
				DebugCloseHandle(dhtFileFind,hFind,GetPath());
			
				ItemDebugMessage("CLocatedItem::UpdateFileSizeAndTime END2");
				return;
			}
		}


		SetToDeleted();

		ItemDebugMessage("CLocatedItem::UpdateFileSizeAndTime END3");
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
	DebugCloseHandle(dhtFile,hFile,GetPath());

	ItemDebugMessage("CLocatedItem::UpdateFileSizeAndTime END");
}

void CLocatedItem::UpdateDimensions()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;

	ItemDebugMessage("CLocatedItem::UpdateDimensions BEGIN");
	

	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::ImageDimensions);
	pField->bShouldUpdate=FALSE;

	if (GetLocateDlg()->m_pImageHandler==NULL)
		return;

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
		WCHAR* pTmp=pField->szText;
		InterlockedExchangePointer((PVOID*)&pField->szText,NULL);
		delete[] pTmp;
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
		pFile->CloseOnDelete();
	
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

		WCHAR* pNewText=new WCHAR[2*16+1];

		for (int i=0;i<16;i++)
		{
			BYTE bHi=BYTE(digest[i]>>4)&0xF;
			BYTE bLo=BYTE(digest[i]&0xF);

			pNewText[i*2]=bHi>=10?bHi-10+L'a':bHi+L'0';
			pNewText[i*2+1]=bLo>=10?bLo-10+L'a':bLo+L'0';
		}
		pNewText[16*2]=L'\0';

		InterlockedExchangePointer((PVOID*)&pField->szText,pNewText);
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

	
	if (pField->szText!=NULL)
	{
		WCHAR* pTmp=pField->szText;
		InterlockedExchangePointer((PVOID*)&pField->szText,NULL);
		delete[] pTmp;
	}
		
	if (IsDeleted())
	{
		ItemDebugMessage("CLocatedItem::UpdateOwner END1");
		return;
	}

    	
	DWORD dwNeeded=0;
	if (!FileSystem::GetFileSecurity(GetPath(),OWNER_SECURITY_INFORMATION,NULL,0,&dwNeeded))
	{
		if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
		{
			ItemDebugMessage("CLocatedItem::UpdateOwner END2");
			return;
		}
	}

    PSECURITY_DESCRIPTOR pDesc=(PSECURITY_DESCRIPTOR)new BYTE[dwNeeded+2];

    if (!FileSystem::GetFileSecurity(szPath,OWNER_SECURITY_INFORMATION,pDesc,dwNeeded+2,&dwNeeded))
	{
		delete[] (BYTE*) pDesc;
		ItemDebugMessage("CLocatedItem::UpdateOwner END3");
			
		return;
	}
	
	PSID psid;
	BOOL bDefaulted=TRUE;
	if (!GetSecurityDescriptorOwner(pDesc,&psid,&bDefaulted))
	{
		delete[] (BYTE*) pDesc;
		ItemDebugMessage("CLocatedItem::UpdateOwner END4");
			
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
		WCHAR* pNewText=new WCHAR[dwOwnerLen+dwDomainLen+2];
		if (dwDomainLen>0)
		{
			MemCopyW(pNewText,szDomain,dwDomainLen);
			pNewText[dwDomainLen]=L'\\';
			MemCopyW(pNewText+dwDomainLen+1,szOwner,dwOwnerLen+1);                
		}
		else
			MemCopyW(pNewText,szOwner,dwOwnerLen+1);
		
		InterlockedExchangePointer((PVOID*)&pField->szText,pNewText);
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

	if (pField->szText!=NULL)
	{
		WCHAR* pTmp=pField->szText;
		InterlockedExchangePointer((PVOID*)&pField->szText,NULL);
		delete[] pTmp;
	}
	
	if (IsDeleted())
	{
		ItemDebugMessage("CLocatedItem::UpdateShortFileName END2");
		return;
	}

	

	WCHAR szShortPath[MAX_PATH];
	DWORD nLength=FileSystem::GetShortPathName(GetPath(),szShortPath,MAX_PATH);
	if (nLength==0)
	{
		//pField->szText=allocemptyW();
		ItemDebugMessage("CLocatedItem::UpdateShortFileName END2");
		return;
	}

	int nStart;
	for (nStart=nLength-1;nStart>=0 && szShortPath[nStart]!='\\';nStart--);
	if (nStart>0)
		nStart++;
    nLength-=nStart;

	InterlockedExchangePointer((PVOID*)&pField->szText,alloccopy(szShortPath+nStart,nLength));

	ItemDebugMessage("CLocatedItem::UpdateShortFileName END");
}

void CLocatedItem::UpdateShortFilePath()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;

	ItemDebugMessage("CLocatedItem::UpdateShortFilePath BEGIN");
	
	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::ShortFilePath);
	pField->bShouldUpdate=FALSE;
	
	if (pField->szText!=NULL)
	{
		WCHAR* pTmp=pField->szText;
		InterlockedExchangePointer((PVOID*)&pField->szText,NULL);
		delete[] pTmp;
	}

	if (IsDeleted())
	{
		ItemDebugMessage("CLocatedItem::UpdateShortFilePath END1");
		return;
	}

	WCHAR szShortPath[MAX_PATH];
	DWORD nLength=FileSystem::GetShortPathName(GetPath(),szShortPath,MAX_PATH);
	if (nLength==0)
	{
		ItemDebugMessage("CLocatedItem::UpdateShortFilePath END2");
		return;
	}
	
	InterlockedExchangePointer((PVOID*)&pField->szText,alloccopy(szShortPath,nLength));

	ItemDebugMessage("CLocatedItem::UpdateShortFilePath END");
}

void CLocatedItem::UpdateSummaryProperties()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;
	
	ItemDebugMessage("CLocatedItem::UpdateSummaryProperties BEGIN");
	
	ExtraInfo* pFields[5];
	pFields[0]=CreateExtraInfoField(CLocateDlg::Author);
	pFields[1]=CreateExtraInfoField(CLocateDlg::Title);
	pFields[2]=CreateExtraInfoField(CLocateDlg::Subject);
	pFields[3]=CreateExtraInfoField(CLocateDlg::Comments);
	pFields[4]=CreateExtraInfoField(CLocateDlg::Pages);
	

	for (int i=0;i<4;i++)
	{
		pFields[i]->bShouldUpdate=FALSE;
		if (pFields[i]->szText!=NULL)
		{
			WCHAR* pTmp=pFields[i]->szText;
			InterlockedExchangePointer((PVOID*)&pFields[i]->szText,NULL);
			delete[] pTmp;
		}
	}	
	pFields[4]->bShouldUpdate=FALSE;
	pFields[4]->nPages=0;


	
	if (IsDeleted())
		return;

	IPropertySetStorage* ppss;
	IPropertyStorage* pps;
	
	STGOPTIONS stgo;
	stgo.usVersion=STGOPTIONS_VERSION;
	

	HRESULT(STDAPICALLTYPE* pStgOpenStorageEx)(const WCHAR*,DWORD,DWORD,DWORD,STGOPTIONS*,
		  PSECURITY_DESCRIPTOR,REFIID,void**);
	pStgOpenStorageEx=(HRESULT(STDAPICALLTYPE *)(const WCHAR*,DWORD,DWORD,DWORD,STGOPTIONS*,
		  PSECURITY_DESCRIPTOR,REFIID,void**))GetProcAddress(GetModuleHandle("Ole32.dll"),"StgOpenStorageEx");
	if (pStgOpenStorageEx==NULL)
		return;

	HRESULT hRes=pStgOpenStorageEx(GetPath(),STGM_READ|STGM_SHARE_EXCLUSIVE,STGFMT_ANY,0,NULL,0,
			IID_IPropertySetStorage,(void**)&ppss);

	if (!SUCCEEDED(hRes))
		return;
	
	hRes=ppss->Open(FMTID_SummaryInformation,STGM_READ|STGM_SHARE_EXCLUSIVE,&pps);
	if (!SUCCEEDED(hRes))
	{
		ppss->Release();
		return;
	}

	
	PROPSPEC rgpspec[5];
	PROPVARIANT rgpropvar[5];
	
	for (int i=0;i<5;i++)
	{
		rgpspec[i].ulKind=PRSPEC_PROPID;
		PropVariantInit(&rgpropvar[i]);
	}

	rgpspec[0].propid=PIDSI_AUTHOR;
	rgpspec[1].propid=PIDSI_TITLE;
	rgpspec[2].propid=PIDSI_SUBJECT;
	rgpspec[3].propid=PIDSI_COMMENT;
	rgpspec[4].propid=PIDSI_PAGECOUNT;

	hRes=pps->ReadMultiple(5,rgpspec,rgpropvar);
	if (SUCCEEDED(hRes))
	{
		for (int i=0;i<4;i++) // Last is pages
		{
			switch (rgpropvar[i].vt)
			{
			case VT_LPSTR:
				InterlockedExchangePointer((PVOID*)&pFields[i]->szText,
					alloccopyAtoW(rgpropvar[i].pszVal));
				break;
			case VT_LPWSTR:
				InterlockedExchangePointer((PVOID*)&pFields[i]->szText,
					alloccopy(rgpropvar[i].pwszVal));
				break;
			}
			PropVariantClear(&rgpropvar[i]);
		}

		switch (rgpropvar[4].vt)
		{
		case VT_I2:
			pFields[4]->nPages=rgpropvar[4].iVal;
			break;
		case VT_I4:
		case VT_INT:
			pFields[4]->nPages=rgpropvar[4].lVal;
			break;
		case VT_I8:
			pFields[4]->nPages=(INT)rgpropvar[4].hVal.LowPart;
			break;
		case VT_UI2:
			pFields[4]->nPages=rgpropvar[4].uiVal;
			break;
		case VT_UI4:
		case VT_UINT:
			pFields[4]->nPages=rgpropvar[4].ulVal;
			break;
		case VT_UI8:
			pFields[4]->nPages=(DWORD)rgpropvar[4].uhVal.LowPart;
			break;
		}
		PropVariantClear(&rgpropvar[4]);
		
	}


	pps->Release();
	ppss->Release();
				
}


void CLocatedItem::UpdateDocSummaryProperties()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;
	
	ItemDebugMessage("CLocatedItem::UpdateDocSummaryProperties BEGIN");
	
	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::Category);
	pField->bShouldUpdate=FALSE;

	if (pField->szText!=NULL)
	{
		WCHAR* pTmp=pField->szText;
		InterlockedExchangePointer((PVOID*)&pField->szText,NULL);
		delete[] pTmp;
	}
			
	if (IsDeleted())
		return;

	IPropertySetStorage* ppss;
	IPropertyStorage* pps;
	
	STGOPTIONS stgo;
	stgo.usVersion=STGOPTIONS_VERSION;
	
	HRESULT(STDAPICALLTYPE* pStgOpenStorageEx)(const WCHAR*,DWORD,DWORD,DWORD,STGOPTIONS*,
		  PSECURITY_DESCRIPTOR,REFIID,void**);
	pStgOpenStorageEx=(HRESULT(STDAPICALLTYPE *)(const WCHAR*,DWORD,DWORD,DWORD,STGOPTIONS*,
		  PSECURITY_DESCRIPTOR,REFIID,void**))GetProcAddress(GetModuleHandle("Ole32.dll"),"StgOpenStorageEx");
	if (pStgOpenStorageEx==NULL)
		return;

	HRESULT hRes=pStgOpenStorageEx(GetPath(),STGM_READ|STGM_SHARE_EXCLUSIVE,STGFMT_ANY,0,NULL,0,
		IID_IPropertySetStorage,(void**)&ppss);
	if (!SUCCEEDED(hRes))
		return;
	
	hRes=ppss->Open(FMTID_DocSummaryInformation,STGM_READ|STGM_SHARE_EXCLUSIVE,&pps);
	if (!SUCCEEDED(hRes))
	{
		ppss->Release();
		return;
	}

	
	PROPSPEC rgpspec[1];
	PROPVARIANT rgpropvar[1];
	
	PropVariantInit(&rgpropvar[0]);
	rgpspec[0].ulKind=PRSPEC_PROPID;
	rgpspec[0].propid=PIDDSI_CATEGORY;

	hRes=pps->ReadMultiple(1,rgpspec,rgpropvar);
	if (SUCCEEDED(hRes))
	{
		switch (rgpropvar[0].vt)
		{
		case VT_LPSTR:
			InterlockedExchangePointer((PVOID*)&pField->szText,
				alloccopyAtoW(rgpropvar[0].pszVal));
			break;
		case VT_LPWSTR:
			InterlockedExchangePointer((PVOID*)&pField->szText,
				alloccopy(rgpropvar[0].pwszVal));
			break;
		}
		PropVariantClear(&rgpropvar[0]);
	}


	pps->Release();
	ppss->Release();
}

void CLocatedItem::UpdateVersionInformation()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;
	
	ItemDebugMessage("CLocatedItem::UpdateVersionInformation BEGIN");
	
	ExtraInfo* pFields[4];
	pFields[0]=CreateExtraInfoField(CLocateDlg::Description);
	pFields[1]=CreateExtraInfoField(CLocateDlg::FileVersion);
	pFields[2]=CreateExtraInfoField(CLocateDlg::ProductName);
	pFields[3]=CreateExtraInfoField(CLocateDlg::ProductVersion);
	
	for (int i=0;i<4;i++)
	{
		pFields[i]->bShouldUpdate=FALSE;
		if (pFields[i]->szText!=NULL)
		{
			WCHAR* pTmp=pFields[i]->szText;
			InterlockedExchangePointer((PVOID*)&pFields[i]->szText,NULL);
			delete[] pTmp;
		}
	}	

	if (IsDeleted())
		return;

	if (IsUnicodeSystem())
	{
		UINT iDataLength=GetFileVersionInfoSizeW(GetPath(),NULL);
		if (iDataLength>0)
		{
			VOID *pTranslations;
			DWORD DefTranslations[2]={0x0490,0x04b0};
			
			LPWSTR pProductVersion=NULL,pProductName=NULL;
			LPWSTR pFileVersion=NULL,pDescription=NULL;
			
			BYTE* pData=new BYTE[iDataLength];
			GetFileVersionInfoW(GetPath(),NULL,iDataLength,pData);
		
			if (!VerQueryValueW(pData,L"VarFileInfo\\Translation",&pTranslations,&iDataLength))
				pTranslations=DefTranslations;

			WCHAR szTranslation[100];
			
			StringCbPrintfW(szTranslation,100*sizeof(WCHAR),L"\\StringFileInfo\\%04X%04X\\ProductVersion",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1]);
			if (!VerQueryValueW(pData,szTranslation,(void**)&pProductVersion,&iDataLength))
				VerQueryValueW(pData,L"\\StringFileInfo\\040904b0\\ProductVersion",(void**)&pProductVersion,&iDataLength);

			StringCbPrintfW(szTranslation,100*sizeof(WCHAR),L"\\StringFileInfo\\%04X%04X\\FileVersion",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1]);
			if (!VerQueryValueW(pData,szTranslation,(void**)&pFileVersion,&iDataLength))
				VerQueryValueW(pData,L"\\StringFileInfo\\040904b0\\FileVersion",(void**)&pFileVersion,&iDataLength);

			StringCbPrintfW(szTranslation,100*sizeof(WCHAR),L"\\StringFileInfo\\%04X%04X\\FileDescription",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1]);
			if (!VerQueryValueW(pData,szTranslation,(void**)&pDescription,&iDataLength))
				VerQueryValueW(pData,L"\\StringFileInfo\\040904b0\\FileDescription",(void**)&pDescription,&iDataLength);

			StringCbPrintfW(szTranslation,100*sizeof(WCHAR),L"\\StringFileInfo\\%04X%04X\\ProductName",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1]);
			if (!VerQueryValueW(pData,szTranslation,(void**)&pProductName,&iDataLength))
				VerQueryValueW(pData,L"\\StringFileInfo\\040904b0\\ProductName",(void**)&pProductName,&iDataLength);

			if (pDescription!=NULL)
				pFields[0]->szText=alloccopy(pDescription);
			if (pFileVersion!=NULL)
				pFields[1]->szText=alloccopy(pFileVersion);
			if (pProductName!=NULL)
				pFields[2]->szText=alloccopy(pProductName);
			if (pProductVersion!=NULL)
				pFields[3]->szText=alloccopy(pProductVersion);
			
			delete[] pData;
		}
	}
	else
	{
		UINT iDataLength=GetFileVersionInfoSize(W2A(GetPath()),NULL);
		if (iDataLength>0)
		{
			VOID *pTranslations;
			DWORD DefTranslations[2]={0x0490,0x04b0};
			
			LPSTR pProductVersion=NULL,pProductName=NULL;
			LPSTR pFileVersion=NULL,pDescription=NULL;
			
			BYTE* pData=new BYTE[iDataLength];
			GetFileVersionInfo(W2A(GetPath()),NULL,iDataLength,pData);
		
			if (!VerQueryValue(pData,"VarFileInfo\\Translation",&pTranslations,&iDataLength))
				pTranslations=DefTranslations;

			CHAR szTranslation[100];
			
			StringCbPrintf(szTranslation,100,"\\StringFileInfo\\%04X%04X\\ProductVersion",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1]);
			if (!VerQueryValue(pData,szTranslation,(void**)&pProductVersion,&iDataLength))
				VerQueryValue(pData,"\\StringFileInfo\\040904b0\\ProductVersion",(void**)&pProductVersion,&iDataLength);

			StringCbPrintf(szTranslation,100,"\\StringFileInfo\\%04X%04X\\FileVersion",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1]);
			if (!VerQueryValue(pData,szTranslation,(void**)&pFileVersion,&iDataLength))
				VerQueryValue(pData,"\\StringFileInfo\\040904b0\\FileVersion",(void**)&pFileVersion,&iDataLength);

			StringCbPrintf(szTranslation,100,"\\StringFileInfo\\%04X%04X\\FileDescription",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1]);
			if (!VerQueryValue(pData,szTranslation,(void**)&pDescription,&iDataLength))
				VerQueryValue(pData,"\\StringFileInfo\\040904b0\\FileDescription",(void**)&pDescription,&iDataLength);

			StringCbPrintf(szTranslation,100,"\\StringFileInfo\\%04X%04X\\ProductName",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1]);
			if (!VerQueryValue(pData,szTranslation,(void**)&pProductName,&iDataLength))
				VerQueryValue(pData,"\\StringFileInfo\\040904b0\\ProductName",(void**)&pProductName,&iDataLength);

			if (pDescription!=NULL)
				InterlockedExchangePointer((PVOID*)&pFields[0]->szText,alloccopyAtoW(pDescription));
			if (pFileVersion!=NULL)
				InterlockedExchangePointer((PVOID*)&pFields[1]->szText,alloccopyAtoW(pFileVersion));
			if (pProductName!=NULL)
				InterlockedExchangePointer((PVOID*)&pFields[2]->szText,alloccopyAtoW(pProductName));
			if (pProductVersion!=NULL)
				InterlockedExchangePointer((PVOID*)&pFields[3]->szText,alloccopyAtoW(pProductVersion));
			
			delete[] pData;
		}
	}
}

void CLocatedItem::SetToDeleted()
{
	ItemDebugMessage("CLocatedItem::SetToDeleted BEGIN");
	
	WCHAR* pTmp=szType;
	InterlockedExchangePointer((PVOID*)&szType,allocstringW(IDS_DELETEDFILE));
	if (pTmp!=NULL)
		delete[] pTmp;
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
	DebugOpenHandle(dhtFileFind,hFind,GetPath());
			
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
	DebugCloseHandle(dhtFileFind,hFind,GetPath());
			
    
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
			RemoveFlags(LITEM_FILETITLEOK);   
			MemCopyW(szPath,szFullPath,dwLength);
		}
		AddFlags(LITEM_FILENAMEOK);
	}
	else if (dwLength>0)
	{
		WCHAR* pTmp=szPath;      
		InterlockedExchangePointer((PVOID*)&szPath,alloccopy(szFullPath,dwLength));       
		if (pTmp!=NULL)
			delete[] pTmp;
		
		InterlockedExchangePointer((PVOID*)&szName,szPath+LastCharIndex(szPath,L'\\')+1);
		bNameLength=BYTE(dwLength-DWORD(szName-szPath));

		for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!=L'.' && bExtensionPos>0 ;bExtensionPos--);
		if (bExtensionPos==0)
			bExtensionPos=bNameLength;
	
		AddFlags(LITEM_FILENAMEOK);
		RemoveFlags(LITEM_FILETITLEOK);
	}

	if (IsDeleted())
	{
		// File has come back
		dwFlags&=~(LITEM_COULDCHANGE|LITEM_FILETITLEOK);

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
		
		ItemDebugFormatMessage4("CLocatedItem::RemoveFlagsForChanged END2, flagsnow=%X",dwFlags,0,0,0);
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
	if (szFileTitle!=szName && szFileTitle!=NULL)
	{
		WCHAR* pTemp=szFileTitle;
		InterlockedExchangePointer((PVOID*)&szFileTitle,NULL);
		delete[] pTemp;
	}
	else
		InterlockedExchangePointer((PVOID*)&szFileTitle,szNewPath+dwDirectoryLen);


	InterlockedExchangePointer((PVOID*)&szName,szNewPath+dwDirectoryLen);
	bNameLength=iLength;
	InterlockedExchangePointer((PVOID*)&szPath,szNewPath);
	
	// Finding extension
	for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!=L'.' && bExtensionPos>0 ;bExtensionPos--);
	if (bExtensionPos==0)
		bExtensionPos=bNameLength;

	delete[] szOldPath;

	AddFlags(LITEM_FILENAMEOK);
	
	UpdateFileTitle();
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