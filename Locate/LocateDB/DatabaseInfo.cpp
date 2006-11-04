/* Copyright (c) 1997-2006 Janne Huttunen
   database updater v2.99.6.11040                  */

#include <HFCLib.h>
#include "Locatedb.h"


BOOL CDatabaseInfo::GetInfo(CDatabase::ArchiveType nArchiveType,LPCWSTR szArchivePath)
{
	sCreator.Empty();
	sDescription.Empty();
	aRootFolders.RemoveAll();

	BYTE* szBuffer=NULL;
	BOOL bRet=TRUE;

	CFile* dbFile=NULL;
	
	try
	{
		switch (nArchiveType)
		{
		case CDatabase::archiveFile:
			dbFile=new CFile(szArchivePath,CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
			break;
		default:
			throw CFileException(CFileException::notImplemented,-1,szArchivePath);
		}
			
		szBuffer=new BYTE[10];
		
		dwFileSize=dbFile->GetLength();
		
		dbFile->Read(szBuffer,9);

		if (szBuffer[0]!='L' || szBuffer[1]!='O' || 
			szBuffer[2]!='C' || szBuffer[3]!='A' || 
			szBuffer[4]!='T' || szBuffer[5]!='E' || 
			szBuffer[6]!='D' || szBuffer[7]!='B')
		{
			throw CFileException(CFileException::invalidFormat,-1,szArchivePath);
		}
	
		if (szBuffer[8]>='0') // New database format
		{
			DWORD dwTemp;
			bVersion=(szBuffer[8]-'0')*10;
			
			dbFile->Read(szBuffer,2);
			bVersion+=(szBuffer[0]-'0');
			bLongFilenames=szBuffer[1]&0x1;
			if (szBuffer[1]&0x20)
				cCharset=Unicode;
			else if (szBuffer[1]&0x10)
				cCharset=Ansi;
			else
				cCharset=OEM;
			delete[] szBuffer;
			szBuffer=NULL;


			// Reading header size
			dbFile->Read(dwTemp);

			if (cCharset==Unicode)
			{
				// Reading creator and description
				dbFile->Read(sCreator);
				dbFile->Read(sDescription);
				dbFile->Read(sExtra1);
				dbFile->Read(sExtra2);
			}
			else
			{
				CString strA;
				// Reading creator and description
				dbFile->Read(strA);
				sCreator=strA;

				dbFile->Read(strA);
				sDescription=strA;

				// Reading extra data, for future use
				dbFile->Read(strA);
				sExtra1=strA;
				dbFile->Read(strA);
				sExtra2=strA;
			}

			

			// Reading time
			dbFile->Read(dwTemp);
			tCreationTime=CTime(LOWORD(dwTemp),HIWORD(dwTemp));

			// Reading number of files and directories
			dbFile->Read(dwNumberOfFiles);
			dbFile->Read(dwNumberOfDirectories);

			// Reading drive/directory information
			DWORD dwBlockSize;
			dbFile->Read(dwBlockSize);
			while (dwBlockSize>0)
			{
				CRoot* pRoot=new CRoot;
				
				DWORD dwSeekLength=dwBlockSize-1-4-4-4;

				// Reading type and path
				dbFile->Read((BYTE*)&pRoot->rtType,1);
				
				if (cCharset==Unicode)
				{
					dbFile->Read(pRoot->sPath);
					dbFile->Read(pRoot->sVolumeName);

					dbFile->Read(pRoot->dwVolumeSerial);
				
					dbFile->Read(pRoot->sFileSystem);

					dwSeekLength-=((DWORD)pRoot->sPath.GetLength()+1+
						(DWORD)pRoot->sVolumeName.GetLength()+1+(DWORD)pRoot->sFileSystem.GetLength()+1)*2;

					
				}
				else
				{
					CString strA;
					dbFile->Read(strA);
					pRoot->sPath=strA;
				
					// Reading volume name and serial and filesystem
					dbFile->Read(strA);
					pRoot->sVolumeName=strA;

					dbFile->Read(pRoot->dwVolumeSerial);
				
					dbFile->Read(strA);
					pRoot->sFileSystem=strA;

					dwSeekLength-=(DWORD)pRoot->sPath.GetLength()+1+
						(DWORD)pRoot->sVolumeName.GetLength()+1+(DWORD)pRoot->sFileSystem.GetLength()+1;

				}
				
		
				// Reading number of files and directories
				dbFile->Read(pRoot->dwNumberOfFiles);
				dbFile->Read(pRoot->dwNumberOfDirectories);
					
				aRootFolders.Add(pRoot);

				
				dbFile->Seek(dwSeekLength,CFile::current);
				dbFile->Read(dwBlockSize);
			}

			dbFile->Close();
		}
		else if (szBuffer[8]>=1 && szBuffer[8]<=4)
		{
			bVersion=szBuffer[8];
			delete[] szBuffer;
			szBuffer=NULL;

			cCharset=OEM;
			switch (bVersion)
			{
			case 2:
			case 4:
				bLongFilenames=TRUE;
				break;
			default:
				bLongFilenames=FALSE;
				break;
			}				
			
			CString strA;
			// Reading creator and description
			dbFile->Read(strA);
			sCreator=strA;
			dbFile->Read(strA);
			sDescription=strA;
			
			// Resolving drives
			CString sDrives;
			dbFile->Read(sDrives);
			
			for (int i=0;i<sDrives.GetLength();i++)
				aRootFolders.Add(new CRoot(sDrives[i]));	
				
			DWORD nTime;
			dbFile->Read(&nTime,4);
			tCreationTime=(time_t)nTime;
			dbFile->Close();
		}
		else
			throw CFileException(CFileException::invalidFormat,-1,szArchivePath);
	}
	catch (...)
	{
		bRet=FALSE;
	}
	if (dbFile!=NULL)
		delete dbFile;
	if (szBuffer!=NULL)
		delete[] szBuffer;
	return bRet;
}




BOOL CDatabaseInfo::GetRootsFromDatabases(CArray<LPWSTR>& aRoots,const PDATABASE* pDatabases,int nDatabases,BOOL bOnlyEnabled)
{
	for (int nDB=0;nDB<nDatabases;nDB++)
	{
		if (bOnlyEnabled && !pDatabases[nDB]->IsEnabled())
			continue;

		CArray<LPWSTR> aDBRoots;
		if (!GetRootsFromDatabase(aDBRoots,pDatabases[nDB]))
			continue;
		
		// Checking that root does not already exists
		for (int i=0;i<aRoots.GetSize();i++)
		{
			int j=0;
			while (j<aDBRoots.GetSize())
			{
				if (strcasecmp(aDBRoots[j],aRoots[i])==0)
				{
					// Already exists, deleting
					delete[] aDBRoots[j];
					aDBRoots.RemoveAt(j);
				}
				else
					j++;
			}
		}

		// Inserting new roots
		for (int i=0;i<aDBRoots.GetSize();i++)
			aRoots.Add(aDBRoots[i]);
	}		
	return TRUE;
}


BOOL CDatabaseInfo::GetRootsFromDatabase(CArray<LPWSTR>& aRoots,const CDatabase* pDatabase)
{
	BYTE* szBuffer=NULL;
	
	CFile* dbFile=NULL;
	
	BOOL bRet=TRUE;
		
	try
	{
		switch (pDatabase->GetArchiveType())
		{
		case CDatabase::archiveFile:
			dbFile=new CFile(pDatabase->GetArchiveName(),CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
			break;
		default:
			throw CFileException(CFileException::notImplemented,
					-1,pDatabase->GetArchiveName());
		}
		
		szBuffer=new BYTE[11];
		
		dbFile->Read(szBuffer,11);

		if (szBuffer[0]!='L' || szBuffer[1]!='O' || 
			szBuffer[2]!='C' || szBuffer[3]!='A' || 
			szBuffer[4]!='T' || szBuffer[5]!='E' || 
			szBuffer[6]!='D' || szBuffer[7]!='B')
		{
			throw CFileException(CFileException::invalidFormat,-1,pDatabase->GetArchiveName());
		}
	
		if (szBuffer[8]!='2' && szBuffer[9]!='0') // Not supported file format
		{
			throw CFileException(CFileException::invalidFormat,-1,pDatabase->GetArchiveName());
		}

		BOOL bUnicode=szBuffer[10]&0x20;

		delete[] szBuffer;
		szBuffer=NULL;

		// Skipping other header fields
		DWORD dwBlockSize;
		dbFile->Read(dwBlockSize);
		dbFile->Seek(dwBlockSize,CFile::current);

		// Reading drive/directory information
		if (bUnicode)
		{
			CStringW Path;
			BYTE bPathLen;
				
			dbFile->Read(dwBlockSize);
			while (dwBlockSize>0)
			{
				// Reading type and path
				dbFile->Read(bPathLen);
				dbFile->Read(Path);
					
				
				aRoots.Add(alloccopy(Path,Path.GetLength()));
							
				dbFile->Seek(dwBlockSize-1-DWORD((Path.GetLength()+1)*2),
					CFile::current);
				
				dbFile->Read(dwBlockSize);
			}
		}
		else
		{
			CString Path;
			BYTE bPathLen;

			dbFile->Read(dwBlockSize);
			while (dwBlockSize>0)
			{
				// Reading type and path
				dbFile->Read(bPathLen);
				dbFile->Read(Path);

				aRoots.Add(alloccopyAtoW(Path,Path.GetLength()));
							
				dbFile->Seek(dwBlockSize-1-DWORD((Path.GetLength()+1)),
					CFile::current);
				
				dbFile->Read(dwBlockSize);
			}
		}
		dbFile->Close();
	}
	catch (CFileException ex)
	{
#ifdef _DEBUG
		switch (ex.m_cause)
		{
		case CFileException::fileOpen:
		case CFileException::badPath:
		case CFileException::fileNotFound:
			DebugFormatMessage(L"GetRootsFromDatabase:FILEOPEN/BADPATH/NOTFOUND: %s",
				pDatabase->GetArchiveName()!=NULL?pDatabase->GetArchiveName():L"");
			break;
		case CFileException::readFault:
		case CFileException::fileCorrupt:
		case CFileException::sharingViolation:
		case CFileException::lockViolation:
		case CFileException::accessDenied:
			DebugFormatMessage(L"GetRootsFromDatabase:READFAULT/CORRUPT/SHARING/LOCK: %s",
				pDatabase->GetArchiveName()!=NULL?pDatabase->GetArchiveName():L"");
			break;
		case CFileException::endOfFile:
		case CFileException::badSeek:
		case CFileException::invalidFile:
			DebugFormatMessage(L"GetRootsFromDatabase:EOF/BADSEEK/INVALID: %s",
				pDatabase->GetArchiveName()!=NULL?pDatabase->GetArchiveName():L"");
			break;
		default:
			DebugFormatMessage(L"GetRootsFromDatabase:UNKNOWN: %s",
				pDatabase->GetArchiveName()!=NULL?pDatabase->GetArchiveName():L"");
			break;
		}
#endif
		bRet=FALSE;

	}
	catch (...)
	{
		DebugFormatMessage(L"GetRootsFromDatabase: Unknown error, %s",
			pDatabase->GetArchiveName()!=NULL?pDatabase->GetArchiveName():L"");
		bRet=FALSE;
	}

	if (dbFile!=NULL)
		delete dbFile;
	if (szBuffer!=NULL)
		delete[] szBuffer;
	return bRet;
}


BOOL CDatabaseInfo::ReadFilesAndDirectoriesCount(CDatabase::ArchiveType nArchiveType,LPCWSTR szArchive,DWORD& dwFiles,DWORD& dwDirectories)
{
	CFile* dbFile=NULL;
	BOOL bRet=TRUE;
	
	dwFiles=DWORD(-1);
	dwDirectories=DWORD(-1);

	if (szArchive==NULL)
		return FALSE;
		
	try
	{
		switch (nArchiveType)
		{
		case CDatabase::archiveFile:
			dbFile=new CFile(szArchive,CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
			dbFile->CloseOnDelete();
			break;
		default:
			throw CFileException(CFileException::notImplemented,
					-1,szArchive);
		}
			
		BYTE szBuffer[11];
		
		dbFile->Read(szBuffer,11);
		if (szBuffer[0]!='L' || szBuffer[1]!='O' || 
			szBuffer[2]!='C' || szBuffer[3]!='A' || 
			szBuffer[4]!='T' || szBuffer[5]!='E' || 
			szBuffer[6]!='D' || szBuffer[7]!='B' ||
			szBuffer[8]<'0')
		{
			throw CFileException(CFileException::invalidFormat,-1,szArchive);
		}
	
		DWORD dwHeaderSize;
		dbFile->Read(dwHeaderSize);

		dbFile->Seek(dwHeaderSize-2*sizeof(DWORD),CFile::current);

		/*// Reading creator, description and reserved data
		CString Temp;
		dbFile->Read(Temp);
		dbFile->Read(Temp);
		dbFile->Read(Temp);
		dbFile->Read(Temp);

		dbFile->Seek(4,CFile::current);*/
		
		dbFile->Read(dwFiles);
		dbFile->Read(dwDirectories);

		dbFile->Close();
		
	}
	catch (...)
	{
		bRet=FALSE;
	}
	if (dbFile!=NULL)
		delete dbFile;
	return bRet;
}