#include <HFCLib.h>
#include "common.h"

#include <parsers.h>


LPSTR ReadIniFile(LPSTR* pFile,LPCSTR szSection,BYTE& bFileIsReg)
{
	char szPath[MAX_PATH];
	GetModuleFileName(NULL,szPath,MAX_PATH);
	int nStart=LastCharIndex(szPath,'\\')+1;
	strcpy_s(szPath+nStart,MAX_PATH-nStart,"locate.ini");

	bFileIsReg=TRUE;

	char* pFileContent=NULL;
	try
	{
		CFile Ini(szPath,CFile::defRead|CFile::otherErrorWhenEOF,TRUE);
		DWORD dwSize=Ini.GetLength();
		pFileContent=new char[dwSize+1];
		Ini.Read(pFileContent,dwSize);
		pFileContent[dwSize]='\0';
		Ini.Close();
	}
	catch (...)
	{
		if (pFileContent!=NULL)
			delete[] pFileContent;
		return NULL;
	}

	LPCSTR pPtr=NULL;
	LPSTR pKeyName=NULL;

	CString Key,Value;

	if (szSection!=NULL)
		pPtr=FindSectionStart(pFileContent,szSection);
	if (pPtr==NULL)
		pPtr=FindSectionStart(pFileContent,"DEFAULT");

	while (pPtr!=NULL)
	{
		pPtr=FindNextValue(pPtr,Key,Value);
		if (Key.IsEmpty())
			break;

		while (Value.LastChar()==' ')
			Value.DelLastChar();

		if (Key.CompareNoCase("KEY")==0)
			pKeyName=Value.GiveBuffer();
		else if (Key.CompareNoCase("FILE")==0)
		{
			if (pFile!=NULL)
				*pFile=Value.GiveBuffer();
		}
		else if (Key.CompareNoCase("FILETYPE")==0)
		{
			if (Value.CompareNoCase("BIN")==0)
				bFileIsReg=FALSE;
			else if (Value.CompareNoCase("REG")==0)
				bFileIsReg=TRUE;
		}
	}
	
	delete[] pFileContent;
	return pKeyName;		
}

BOOL LoadSettingsFromFile(LPCSTR szKey,LPCSTR szFile,BYTE bFileIsReg)
{
	HKEY hKey;
	LONG lRet=RegOpenKeyEx(HKEY_CURRENT_USER,szKey,
		0,KEY_READ,&hKey);
	
	// Check wheter key exists
	if (lRet!=ERROR_FILE_NOT_FOUND)
	{
		// Key exists, using it
		RegCloseKey(hKey);
		return TRUE;
	}

	if (bFileIsReg)
	{
				
		// Restore key
		char szCommand[2000];
		sprintf_s(szCommand,2000,"regedit /s \"%s\"",szFile);

		PROCESS_INFORMATION pi;
		STARTUPINFO si; // Ansi and Unicode versions are same
		ZeroMemory(&si,sizeof(STARTUPINFO));
		si.cb=sizeof(STARTUPINFO);
		
		if (CreateProcess(NULL,szCommand,NULL,
			NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS,
			NULL,NULL,&si,&pi))
		{
			WaitForSingleObject(pi.hProcess,2000);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);	
		}
		else
			return FALSE;

		return TRUE;
	}

	// Acquiring required privileges	
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
	{
		PTOKEN_PRIVILEGES ns=(PTOKEN_PRIVILEGES)new BYTE[sizeof(DWORD)+sizeof(LUID_AND_ATTRIBUTES)+2];
		if (LookupPrivilegeValue(NULL,SE_BACKUP_NAME,&(ns->Privileges[0].Luid)))
		{
			ns->PrivilegeCount=1;
			ns->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
			if (!AdjustTokenPrivileges(hToken,FALSE,ns,0,NULL,NULL))
			{
				//ShowError(NULL,IDS_ERRORCANNOTENABLEPRIVILEGE,GetLastError());
			}
		}
		if (LookupPrivilegeValue(NULL,SE_RESTORE_NAME,&(ns->Privileges[0].Luid)))
		{
			ns->PrivilegeCount=1;
			ns->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
			if (!AdjustTokenPrivileges(hToken,FALSE,ns,0,NULL,NULL))
			{
				//ShowError(NULL,IDS_ERRORCANNOTENABLEPRIVILEGE,GetLastError());
			}

		}
		delete[] (BYTE*)ns;
	}

	// First, check that we can restore key
	
	lRet=RegCreateKeyEx(HKEY_CURRENT_USER,szKey,
		0,NULL,REG_OPTION_BACKUP_RESTORE,KEY_ALL_ACCESS,NULL,&hKey,NULL);
	if (lRet!=ERROR_SUCCESS)
	{
		//ShowError(hWnd,IDS_ERRORCANNOTCREATEKEY,lRet);
		return FALSE;
	}
	lRet=RegRestoreKey(hKey,szFile,0);
	RegCloseKey(hKey);		
	
	return lRet==ERROR_SUCCESS;
}

BOOL SaveSettingsToFile(LPCSTR szKey,LPCSTR szFile,BYTE bFileIsReg)
{
	if (bFileIsReg)
	{
		// Registry script
		char szCommand[2000];
		sprintf_s(szCommand,2000,"regedit /ea \"%s\" HKEY_CURRENT_USER\\%s",szFile,szKey);

		if (FileSystem::IsFile(szFile))
			DeleteFile(szFile);

		PROCESS_INFORMATION pi;
		STARTUPINFO si; // Ansi and Unicode versions are same
		ZeroMemory(&si,sizeof(STARTUPINFO));
		si.cb=sizeof(STARTUPINFO);
		
		
		if (CreateProcess(NULL,szCommand,NULL,
			NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|NORMAL_PRIORITY_CLASS,
			NULL,NULL,&si,&pi))
		{
			WaitForSingleObject(pi.hProcess,2000);		
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);	
		}
		else
			return FALSE;

		if (FileSystem::IsFile(szFile))
		{
			CRegKey::DeleteKey(HKCU,szKey);
		}

		return TRUE;		
	}

	// Acquiring required privileges	
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
	{
		PTOKEN_PRIVILEGES ns=(PTOKEN_PRIVILEGES)new BYTE[sizeof(DWORD)+sizeof(LUID_AND_ATTRIBUTES)+2];
		if (LookupPrivilegeValue(NULL,SE_BACKUP_NAME,&(ns->Privileges[0].Luid)))
		{
			ns->PrivilegeCount=1;
			ns->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
			if (!AdjustTokenPrivileges(hToken,FALSE,ns,0,NULL,NULL))
			{
				//ShowError(NULL,IDS_ERRORCANNOTENABLEPRIVILEGE,GetLastError());
			}
		}
		if (LookupPrivilegeValue(NULL,SE_RESTORE_NAME,&(ns->Privileges[0].Luid)))
		{
			ns->PrivilegeCount=1;
			ns->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
			if (!AdjustTokenPrivileges(hToken,FALSE,ns,0,NULL,NULL))
			{
				//ShowError(NULL,IDS_ERRORCANNOTENABLEPRIVILEGE,GetLastError());
			}

		}
		delete[] (BYTE*)ns;
	}

	// Data format
	HKEY hRegKey;
	LONG lRet=RegOpenKeyEx(HKEY_CURRENT_USER,szKey,
		0,KEY_READ,&hRegKey);

	if (lRet!=ERROR_SUCCESS)
	{
		//ShowError(hWnd,IDS_ERRORCANNOTOPENKEY,lRet);
		return FALSE;
	}

	DeleteFile(szFile);
	lRet=RegSaveKey(hRegKey,szFile,NULL);
	RegCloseKey(hRegKey);		
	
	if (lRet==ERROR_SUCCESS)
		CRegKey::DeleteKey(HKCU,szKey);
	
	return TRUE;
}
