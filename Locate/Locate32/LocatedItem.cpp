#include <HFCLib.h>
#include "Locate32.h"

#include <md5.h>
#ifdef HFC_MTLIBS
#pragma comment(lib, "libmd5mt.lib")
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
	szPath=(char*)Allocation.AllocateFast(nPathLen+bNameLength+2);
	sMemCopy(szPath,pLocater->GetCurrentPath(),nPathLen);
	szName=szPath+(++nPathLen);
	sMemCopy(szName,pLocater->GetFolderName(),DWORD(bNameLength)+1);
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
	bFileSizeHi=0;

	// Setting file time
	wModifiedDate=pLocater->GetFolderModifiedDate();
    wModifiedTime=pLocater->GetFolderModifiedTime();
	wCreatedDate=pLocater->GetFolderCreatedDate();
	wCreatedTime=WORD(-1);
	wAccessedDate=pLocater->GetFolderAccessedDate();
    wAccessedTime=WORD(-1);
	
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
	szPath=(char*)Allocation.AllocateFast(nPathLen+bNameLength+2);
	sMemCopy(szPath,pLocater->GetCurrentPath(),nPathLen);
	szName=szPath+(++nPathLen);
	sMemCopy(szName,pLocater->GetFileName(),DWORD(bNameLength)+1);
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
	bFileSizeHi=pLocater->GetFileSizeHi();

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

void CLocatedItem::ClearData()
{
	//ItemDebugMessage("CLocatedItem::ClearData BEGIN");

	if (szTitle!=NULL && szTitle!=szName)
		Allocation.Free(szTitle);
	szTitle=NULL;
	if (szPath!=NULL)
	{
		Allocation.Free(szPath);
		szPath=NULL;
	}
	if (szType!=NULL)
	{
		Allocation.Free(szType);
		szType=NULL;
	}
	
	DeleteAllExtraFields();

	//ItemDebugMessage("CLocatedItem::ClearData END");
}

void CLocatedItem::UpdateByDetail(CLocateDlg::DetailType nDetail)
{
	switch(nDetail)
	{
	case CLocateDlg::DetailType::FullPath:
		UpdateFilename();
		break;
	case CLocateDlg::DetailType::Title:
		UpdateTitle();
		UpdateIcon();
		break;
	case CLocateDlg::DetailType::InFolder:
		UpdateParentIcon();
		break;
	case CLocateDlg::DetailType::FileSize:
	case CLocateDlg::DetailType::DateModified:
	case CLocateDlg::DetailType::DateCreated:
	case CLocateDlg::DetailType::DateAccessed:
		UpdateFileSizeAndTime();
		break;
	case CLocateDlg::DetailType::FileType:
		UpdateType();
		break;
	case CLocateDlg::DetailType::Attributes:
		UpdateAttributes();
		break;
	case CLocateDlg::DetailType::ImageDimensions:
		UpdateDimensions();
		break;
	case CLocateDlg::DetailType::Owner:
		UpdateOwner();
		break;
	case CLocateDlg::DetailType::ShortFileName:
		UpdateShortFileName();
		break;
	case CLocateDlg::DetailType::ShortFilePath:
		UpdateShortFilePath();
		break;
	case CLocateDlg::DetailType::MD5sum:
		ComputeMD5sum();
		break;
	}	
}


	
BOOL CLocatedItem::ShouldUpdateByDetail(CLocateDlg::DetailType nDetail) const
{
	switch(nDetail)
	{
	case CLocateDlg::DetailType::FullPath:
		return ShouldUpdateFilename();
	case CLocateDlg::DetailType::Database:
	case CLocateDlg::DetailType::DatabaseDescription:
	case CLocateDlg::DetailType::DatabaseArchive:
	case CLocateDlg::DetailType::VolumeLabel:
	case CLocateDlg::DetailType::VolumeSerial:
	case CLocateDlg::DetailType::VOlumeFileSystem:
		return FALSE;
	case CLocateDlg::DetailType::Title:
		return ShouldUpdateTitle() || ShouldUpdateIcon();
	case CLocateDlg::DetailType::InFolder:
		return ShouldUpdateParentIcon();
	case CLocateDlg::DetailType::FileSize:
		return ShouldUpdateFileSize();
	case CLocateDlg::DetailType::DateModified:
	case CLocateDlg::DetailType::DateCreated:
	case CLocateDlg::DetailType::DateAccessed:
		return ShouldUpdateTimeAndDate();
	case CLocateDlg::DetailType::FileType:
		return ShouldUpdateType();
	case CLocateDlg::DetailType::Attributes:
		return ShouldUpdateAttributes();
	default:
		return ShouldUpdateExtra(nDetail);
	}	
}

void CLocatedItem::UpdateFilename()
{
	char szFullPath[MAX_PATH];
	
	DWORD dwLength=GetLocateApp()->m_pGetLongPathName(GetPath(),szFullPath,MAX_PATH);
	if (dwLength==GetPathLen())
	{
		// Checking assumptions, i.e. length of file name and extension does not change 
		// and extension is same
		ASSERT(szFullPath[DWORD(szName-szPath)-1]=='\\' && 
			(szFullPath[DWORD(szName-szPath)+bExtensionPos-1]=='.' || 
			szFullPath[DWORD(szName-szPath)+bExtensionPos]=='\0'));

        //This fixes case
		CopyMemory(szPath,szFullPath,dwLength);
	}
	else if (dwLength>0)
	{
        Allocation.Free(szPath);
		szPath=(char*)Allocation.AllocateFast(dwLength+2);
		CopyMemory(szPath,szFullPath,dwLength+1);
        
		szName=szPath+LastCharIndex(szPath,'\\')+1;
		bNameLength=BYTE(dwLength-DWORD(szName-szPath));

		for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!='.' && bExtensionPos>0 ;bExtensionPos--);
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
		Allocation.Free(szTitle);
	
	if (!(dwFlags&LITEM_FILENAMEOK))
		UpdateFilename();

	if ((GetLocateDlg()->GetFlags()&CLocateDlg::fgLVMethodFlag)==CLocateDlg::fgLVUseGetFileTitle)
	{
		if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
			return;
		
		szTitle=(char*)Allocation.Allocate(bNameLength+1);
		WORD nLen=GetFileTitle(GetPath(),szTitle,bNameLength+1);
		if (nLen!=0)
		{
			Allocation.Free(szTitle);
			szTitle=(char*)Allocation.Allocate(nLen);
			GetFileTitle(GetPath(),szTitle,nLen);
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
											Allocation.Free(szType);
										szType=(char*)Allocation.Allocate(nLength);
										if (RegKey.QueryValue("",szType,nLength))
											dwFlags|=LITEM_TYPEOK;
										else
											Allocation.Free(szType);
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
			szTitle=(char*)Allocation.Allocate(bExtensionPos);
			sMemCopy(szTitle,GetName(),DWORD(bExtensionPos)-1);
			szTitle[bExtensionPos-1]='\0';
			break;
		}
		if (GetLocateDlg()->GetFlags()&CLocateDlg::fgLV1stCharUpper)
		{
			BOOL bAllUpper=TRUE;
			for (DWORD i=0;szTitle[i]!='\0';i++)
			{
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
					szTitle=(char*)Allocation.Allocate(i+1);
					sMemCopy(szTitle,szName,i+1);
				}
				CharLowerBuff(szTitle+1,i-1);
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
		Allocation.Free(szType);


	if (!(GetLocateDlg()->GetFlags()&CLocateDlg::fgLVShowFileTypes))
	{
		char buf[300];
		DWORD dwLength;

		// File/folder does not exist
		if (IsFolder())
		{
			dwLength=LoadString(IDS_DIRECTORYTYPE,buf,300);
			szType=(LPSTR)Allocation.Allocate(dwLength+1);
			sMemCopy(szType,buf,dwLength+1);
		}
		else
		{
			dwLength=LoadString(IDS_UNKNOWNTYPE,buf,300)+1;		
			szType=(LPSTR)Allocation.Allocate(GetExtensionLength()+dwLength+1);
			sMemCopy(szType,GetExtension(),GetExtensionLength());
			szType[GetExtensionLength()]=' ';
			sMemCopy(szType+GetExtensionLength()+1,buf,dwLength);
		}
		dwFlags|=LITEM_TYPEOK;
		return;
	}

	if (GetLocateDlg()->GetFlags()&CLocateDlg::fgLVShowShellType)
	{
		// Using shell functions
		SHFILEINFO fi;
		if (ShouldUpdateIcon() && GetLocateDlg()->GetFlags()&CLocateDlg::fgLVShowIcons)
		{
			// Taking icon now too
			if (!SHGetFileInfo(GetPath(),0,&fi,sizeof(SHFILEINFO),SHGFI_TYPENAME|SHGFI_ICON|SHGFI_SYSICONINDEX))
			{
				SetToDeleted();
				return;
			}
			iIcon=fi.iIcon;
			dwFlags|=LITEM_ICONOK;
		}
		else if (!SHGetFileInfo(GetPath(),0,&fi,sizeof(SHFILEINFO),SHGFI_TYPENAME))
		{
			SetToDeleted();
			return;
		}
		
		DWORD dwTypeLen;
		dstrlen(fi.szTypeName,dwTypeLen);
		
		szType=(char*)Allocation.Allocate(++dwTypeLen);
		sMemCopy(szType,fi.szTypeName,dwTypeLen);
		dwFlags|=LITEM_TYPEOK;
		return;
	}

	// File/folder does not exist
	if (IsFolder())
	{
		if (!CFile::IsDirectory(GetPath()))
		{
			SetToDeleted();
			return;
		}
		char szBuffer[80];
		DWORD dwTextLen=LoadString(IDS_DIRECTORYTYPE,szBuffer,80)+1;
		szType=(char*)Allocation.Allocate(dwTextLen);
		sMemCopy(szType,szBuffer,dwTextLen);
		dwFlags|=LITEM_TYPEOK;
		return;
	}
	else
	{
		if (!CFile::IsFile(GetPath()))
		{
			SetToDeleted();
			return;
		}
	}

	CRegKey RegKey;
	CString Type;
	if (RegKey.OpenKey(HKCR,szName+bExtensionPos-1,CRegKey::openExist|CRegKey::samRead|CRegKey::samQueryValue)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("",Type);
		RegKey.CloseKey();
		
		if (!Type.IsEmpty())
		{
			if (RegKey.OpenKey(HKCR,Type,CRegKey::openExist|CRegKey::samRead|CRegKey::samQueryValue)==ERROR_SUCCESS)
			{
				// Taking type now
				DWORD nLength=RegKey.QueryValueLength("");
				if (nLength)
				{
					szType=(char*)Allocation.Allocate(nLength);
					if (RegKey.QueryValue("",szType,nLength))
					{
						dwFlags|=LITEM_TYPEOK;
						return;
					}
					else
						Allocation.Free(szType);

				}
				RegKey.CloseKey();
			}
		}
	}

	char szBuffer[80];
	DWORD dwTextLen=LoadString(IDS_UNKNOWNTYPE,szBuffer,80)+1;

	if (bExtensionPos!=bNameLength)
	{
		szType=(char*)Allocation.Allocate(dwTextLen+GetExtensionLength()+1);
		sMemCopy(szType,GetExtension(),GetExtensionLength());
		CharUpperBuff(szType,GetExtensionLength());
		szType[GetExtensionLength()]=' ';
		sMemCopy(szType+GetExtensionLength()+1,szBuffer,dwTextLen);
	}
	else // No extension
	{
		szType=(char*)Allocation.Allocate(dwTextLen);
		sMemCopy(szType,szBuffer,dwTextLen);
	}
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


	DWORD dwAttributes=GetFileAttributes(GetPath());
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
		WIN32_FIND_DATA fd;
		HANDLE hFind=FindFirstFile(GetPath(),&fd);
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

	HANDLE hFile=CreateFile(GetPath(),0,FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,OPEN_EXISTING,0,NULL);

	if (hFile==INVALID_HANDLE_VALUE)
	{
		if (GetLastError()==ERROR_SHARING_VIOLATION)
		{
			// Another method to query information
			WIN32_FIND_DATA fd;
			HANDLE hFind=FindFirstFile(GetPath(),&fd);
			if (hFind!=INVALID_HANDLE_VALUE)
			{
				dwFileSize=fd.nFileSizeLow;
				bFileSizeHi=(BYTE)fd.nFileSizeHigh;


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
	bFileSizeHi=static_cast<BYTE>(dwFileSizeHiTemp);

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

	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::DetailType::ImageDimensions);
	pField->bShouldUpdate=FALSE;

	SIZE dim;
	if (!GetLocateDlg()->m_pImageHandler->pGetImageDimensionsA(GetPath(),&dim))
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

	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::DetailType::MD5sum);
	pField->bShouldUpdate=FALSE;

	if (!bForce && !(GetLocateDlg()->GetFlags()&CLocateDlg::fgLVComputeMD5Sums))
		return; 
		
	if (pField->szText!=NULL)
	{
		Allocation.Free(pField->szText);
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

		pField->szText=(char*)Allocation.AllocateFast(2*16+1);

		for (int i=0;i<16;i++)
		{
			BYTE bHi=BYTE(digest[i]>>4)&0xF;
			BYTE bLo=BYTE(digest[i]&0xF);

			pField->szText[i*2]=bHi>=10?bHi-10+'a':bHi+'0';
			pField->szText[i*2+1]=bLo>=10?bLo-10+'a':bLo+'0';
		}
		pField->szText[16*2]='\0';
	}

	ItemDebugMessage("CLocatedItem::ComputeMD5sum END");
}

void CLocatedItem::UpdateOwner()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;
	
	ItemDebugMessage("CLocatedItem::UpdateOwner BEGIN");
	
	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::DetailType::Owner);
	pField->bShouldUpdate=FALSE;

	if (IsDeleted())
		return;

	if (pField->szText!=NULL)
	{
		Allocation.Free(pField->szText);
		pField->szText=NULL;
	}
			
    	
	DWORD dwNeeded=0;
	if (!GetFileSecurity(GetPath(),OWNER_SECURITY_INFORMATION,NULL,0,&dwNeeded))
	{
		if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
			return;
	}

    PSECURITY_DESCRIPTOR pDesc=(PSECURITY_DESCRIPTOR)new BYTE[dwNeeded+2];

    if (!GetFileSecurity(szPath,OWNER_SECURITY_INFORMATION,pDesc,dwNeeded+2,&dwNeeded))
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
	

	char szOwner[200],szDomain[200];
	DWORD dwOwnerLen=199,dwDomainLen=199;
	SID_NAME_USE sUse;
	
	BOOL bRet;
	if (szPath[0]=='\\' && szPath[1]=='\\')
	{
		DWORD dwLength=2+FirstCharIndex(szPath+2,'\\');
		char* szServer=new char[dwLength+1];
		sMemCopy(szServer,szPath,dwLength);
		szServer[dwLength]='\0';
		bRet=LookupAccountSid(szServer,psid,szOwner,&dwOwnerLen,szDomain,&dwDomainLen,&sUse);
		delete[] szServer;
	}
	else
		bRet=LookupAccountSid(NULL,psid,szOwner,&dwOwnerLen,szDomain,&dwDomainLen,&sUse);

	if (bRet)
	{
		pField->szText=(char*)Allocation.AllocateFast(dwOwnerLen+dwDomainLen+2);
		if (dwDomainLen>0)
		{
			sMemCopy(pField->szText,szDomain,dwDomainLen);
			pField->szText[dwDomainLen]='\\';
			sMemCopy(pField->szText+dwDomainLen+1,szOwner,dwOwnerLen+1);                
		}
		else
			sMemCopy(pField->szText,szOwner,dwOwnerLen+1);
	}
	delete[] (BYTE*) pDesc;

	ItemDebugMessage("CLocatedItem::UpdateOwner END");
}

void CLocatedItem::UpdateShortFileName()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;

	ItemDebugFormatMessage4("CLocatedItem::UpdateShortFileName BEGIN %s",GetPath(),0,0,0);
	
	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::DetailType::ShortFileName);
	pField->bShouldUpdate=FALSE;

	if (IsDeleted())
		return;

	char szShortPath[MAX_PATH];
	DWORD nLength=GetShortPathName(GetPath(),szShortPath,MAX_PATH);
	if (nLength==0)
	{
		if (pField->szText!=NULL)
			Allocation.Free(pField->szText); 
		pField->szText=(char*)Allocation.Allocate(2);
		pField->szText[0]='\0';
		ItemDebugMessage("CLocatedItem::UpdateShortFileName END2");
		return;
	}

	for (int nStart=nLength-1;nStart>=0 && szShortPath[nStart]!='\\';nStart--);
	if (nStart>0)
		nStart++;
    nLength-=nStart;

	if (pField->szText!=NULL)
		Allocation.Free(pField->szText); 

	pField->szText=(char*)Allocation.AllocateFast(nLength+1);
	sMemCopy(pField->szText,szShortPath+nStart,nLength+1);

	ItemDebugMessage("CLocatedItem::UpdateShortFileName END");
}

void CLocatedItem::UpdateShortFilePath()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return;

	ItemDebugMessage("CLocatedItem::UpdateShortFilePath BEGIN");
	
	ExtraInfo* pField=CreateExtraInfoField(CLocateDlg::DetailType::ShortFilePath);
	pField->bShouldUpdate=FALSE;
	if (IsDeleted())
		return;

	char szShortPath[MAX_PATH];
	DWORD nLength=GetShortPathName(GetPath(),szShortPath,MAX_PATH);
	if (nLength==0)
	{
		if (pField->szText!=NULL)
			Allocation.Free(pField->szText); 

		pField->szText=(char*)Allocation.AllocateFast(2);
		pField->szText[0]='\0';
		ItemDebugMessage("CLocatedItem::UpdateShortFilePath END2");
	}
	


	if (pField->szText!=NULL)
		Allocation.Free(pField->szText); 

	pField->szText=(char*)Allocation.AllocateFast(nLength+1);
	sMemCopy(pField->szText,szShortPath,nLength+1);

	ItemDebugMessage("CLocatedItem::UpdateShortFilePath END");
}


void CLocatedItem::SetToDeleted()
{
	ItemDebugMessage("CLocatedItem::SetToDeleted BEGIN");
	
	char szBuffer[80];
	DWORD dwLength=LoadString(IDS_DELETEDFILE,szBuffer,80)+1;
	if (szType!=NULL)
		Allocation.Free(szType);
	szType=(char*)Allocation.Allocate(dwLength);
	sMemCopy(szType,szBuffer,dwLength);

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

	ItemDebugMessage("CLocatedItem::SetToDeleted END");
}


BOOL CLocatedItem::RemoveFlagsForChanged()
{
	if (!(GetLocateDlg()->GetExtraFlags()&CLocateDlg::efEnableItemUpdating))
		return FALSE;

	ItemDebugFormatMessage4("CLocatedItem::RemoveFlagsForChanged BEGIN, item:%s, flags already=%X",GetPath(),dwFlags,0,0);
	

	WIN32_FIND_DATA fd;
	
	// Checking whether file is available
	HANDLE hFind=FindFirstFile(GetPath(),&fd);
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
	char szFullPath[MAX_PATH];
	DWORD dwLength=GetLocateApp()->m_pGetLongPathName(GetPath(),szFullPath,MAX_PATH);
	if (dwLength==GetPathLen())
	{
		// Checking assumptions, i.e. length of file name and extension does not change 
		// and extension is same
		ASSERT(szFullPath[DWORD(szName-szPath)-1]=='\\' && 
			(szFullPath[DWORD(szName-szPath)+bExtensionPos-1]=='.' || 
			szFullPath[DWORD(szName-szPath)+bExtensionPos]=='\0'));

        //This fixes case
		if (strncmp(szPath,szFullPath,dwLength)!=0)
		{
			RemoveFlags(LITEM_TITLEOK);   
			CopyMemory(szPath,szFullPath,dwLength);
		}
		AddFlags(LITEM_FILENAMEOK);
	}
	else if (dwLength>0)
	{
        Allocation.Free(szPath);
		szPath=(char*)Allocation.AllocateFast(dwLength+2);
		CopyMemory(szPath,szFullPath,dwLength+1);
        
		szName=szPath+LastCharIndex(szPath,'\\')+1;
		bNameLength=BYTE(dwLength-DWORD(szName-szPath));

		for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!='.' && bExtensionPos>0 ;bExtensionPos--);
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
			bFileSizeHi=static_cast<BYTE>(fd.nFileSizeHigh);
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
		bFileSizeHi=static_cast<BYTE>(fd.nFileSizeHigh);

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

void CLocatedItem::ChangeName(LPCSTR szNewName,int iLength)
{
	char* szOldPath=GetPath();
	if (iLength==-1)
		iLength=istrlen(szNewName);

	DWORD dwDirectoryLen=DWORD(szName-szPath);
	szPath=(char*)Allocation.AllocateFast(dwDirectoryLen+iLength+1);
	
	// Copying directory
	CopyMemory(szPath,szOldPath,dwDirectoryLen);
	szName=szPath+dwDirectoryLen;
	
	// Copying filename
	CopyMemory(szName,szNewName,iLength);
	szName[iLength]='\0';
	bNameLength=iLength;
	
	// Finding extension
	for (bExtensionPos=bNameLength-1; szName[bExtensionPos-1]!='.' && bExtensionPos>0 ;bExtensionPos--);
	if (bExtensionPos==0)
		bExtensionPos=bNameLength;

	MoveFile(szOldPath,szPath);
	Allocation.Free(szOldPath);
	

	AddFlags(LITEM_FILENAMEOK);
	RemoveFlags(LITEM_TITLEOK);
}

LPSTR CLocatedItem::GetToolTipText() const
{
	ISDLGTHREADOK

	if (g_szBuffer!=NULL)
		delete[] g_szBuffer;

	if (IsDeleted())
	{
		
		if (IsFolder())
		{
			CString str(IDS_TOOLTIPFORDIRECTORYDELETED);
			int nLen=str.GetLength()+GetPathLen()+istrlen(GetType())+2;
			g_szBuffer=new char[nLen];
			sprintfex(g_szBuffer,nLen,str,GetName(),GetParent(),GetType());
		}
		else
		{
			CString str(IDS_TOOLTIPFORFILEDELETED);
			int nLen=str.GetLength()+GetPathLen()+istrlen(GetType())+2;
			g_szBuffer=new char[nLen];
			sprintfex(g_szBuffer,nLen,str,GetName(),GetParent(),GetType());
		}
		return g_szBuffer;
	}

	
	char* szDate=GetLocateApp()->FormatDateAndTimeString(GetModifiedDate(),GetModifiedTime());
		
	if (ShouldUpdateFileSizeOrDate())
		((CLocatedItem*)this)->UpdateFileSizeAndTime();
	if (ShouldUpdateType())
		((CLocatedItem*)this)->UpdateType();

	if (IsFolder())
	{
		CString str(IDS_TOOLTIPFORDIRECTORY);
		
		int nLen=str.GetLength()+GetPathLen()+istrlen(GetType())+istrlen(szDate)+2;
		g_szBuffer=new char[nLen];
		sprintfex(g_szBuffer,nLen,str,GetName(),GetParent(),GetType(),szDate);
	}
	else
	{
		CString str(IDS_TOOLTIPFORFILE);
		
		int nLen=str.GetLength()+GetPathLen()+istrlen(GetType())+istrlen(szDate)+25;
		g_szBuffer=new char[nLen];
		
		char szSize[25];

		if (GetFileSize()>LONGLONG(1024*1024*1024)) // Over 1 Gb
		{
			int nLength=StringCbPrintf(szSize,25,"%1.2f",double(GetFileSize())/(1024*1024*1024));
			while (szSize[nLength-1]=='0')
				nLength--;
			if (szSize[nLength-1]=='.')
				nLength--;
			::LoadString(IDS_GB,szSize+nLength,25-nLength);
		}	
		else if (GetFileSize()>LONGLONG(1024*1024)) // Over 1 Mb
		{
			int nLength=StringCbPrintf(szSize,25,"%1.2f",double(GetFileSize())/(1024*1024));
			while (szSize[nLength-1]=='0')
				nLength--;
			if (szSize[nLength-1]=='.')
				nLength--;
			::LoadString(IDS_MB,szSize+nLength,25-nLength);
		}	
		else if (GetFileSize()>LONGLONG(1024)) // Over 1 Gb
		{
			int nLength=StringCbPrintf(szSize,25,"%1.2f",double(GetFileSize())/(1024));
			while (szSize[nLength-1]=='0')
				nLength--;
			if (szSize[nLength-1]=='.')
				nLength--;
			::LoadString(IDS_KB,szSize+nLength,25-nLength);
		}	
		else
		{
			int nLength=StringCbPrintf(szSize,25,"%d",GetFileSizeLo());
			::LoadString(IDS_BYTES,szSize+nLength,25-nLength);
		}
		

		sprintfex(g_szBuffer,nLen,str,GetName(),GetParent(),GetType(),szDate,szSize);
	}

	delete[] szDate;
	return g_szBuffer;
}