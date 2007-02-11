#include <HFCLib.h>
#include "Locate32.h"

////////////////////////////////////////////
// CCheckFileNotificationsThread

#define CHANGE_BUFFER_LEN		10000



BOOL CCheckFileNotificationsThread::Start()
{
#ifdef THREADDISABLE_CHANGENOTIFIER
	return TRUE;
#else

	BkgDebugMessage("CCheckFileNotificationsThread::Start BEGIN");
	

	if (m_hThread!=NULL)
	{
		BkgDebugMessage("CCheckFileNotificationsThread::Start() ALREADY STARTED");
		return FALSE;
	}

	DWORD dwThreadID;
	m_hThread=CreateThread(NULL,0,NotificationThreadProc,this,CREATE_SUSPENDED,&dwThreadID);
	DebugOpenHandle(dhtThread,m_hThread,"CheckFileNotifications");

	if (m_hThread==NULL)
		return FALSE;
	SetThreadPriority(m_hThread,THREAD_PRIORITY_BELOW_NORMAL);
	ResumeThread(m_hThread);
	
	
	BkgDebugMessage("CCheckFileNotificationsThread::Start END");
	return TRUE;
#endif
}

BOOL CCheckFileNotificationsThread::Stop()
{
#ifdef THREADDISABLE_CHANGENOTIFIER
	return TRUE;
#else

	BkgDebugMessage("CCheckFileNotificationsThread::Stop BEGIN");
	
	HANDLE hThread=m_hThread;
	DWORD status=0;
	if (hThread==NULL)
	{
		BkgDebugMessage("CCheckFileNotificationsThread::Stop END_1");
		if (GetLocateDlg()->m_pFileNotificationsThread!=NULL)
			delete this;
		return FALSE;
	}
	
	BOOL bRet=::GetExitCodeThread(m_hThread,&status);
	if (bRet && status==STILL_ACTIVE)
	{
		if (m_pHandles!=NULL)
		{
			BkgDebugFormatMessage("CCheckFileNotificationsThread::Stop set event %X",DWORD(m_pHandles[0]));

			SetEvent(m_pHandles[0]);
			for (int i=0;i<30 && GetLocateDlg()->m_pFileNotificationsThread!=NULL;i++)
				Sleep(10);
		}
		if (GetLocateDlg()->m_pFileNotificationsThread!=NULL)
		{
			status=0;
			bRet=::GetExitCodeThread(hThread,&status);
			BOOL bTerminated=FALSE;
	
			while (bRet && status==STILL_ACTIVE)
			{
				if (::TerminateThread(hThread,1))
					bTerminated=TRUE;
	
				Sleep(100);
				bRet=::GetExitCodeThread(hThread,&status);
			}
	
			if (bTerminated && m_hThread!=NULL)
				delete this;
		}
	}
	
	BkgDebugMessage("CCheckFileNotificationsThread::Stop END");
	return TRUE;

#endif
}

inline void CCheckFileNotificationsThread::FileCreated(LPCWSTR szFile,DWORD dwLength,CLocateDlg* pLocateDlg)
{
	BkgDebugNumMessage("File created: %S",DWORD(szFile));

	if (pLocateDlg->m_pListCtrl==NULL)
		return;
	if (pLocateDlg->m_pListCtrl->GetItemCount()==0)
		return;



	WCHAR szPath[MAX_PATH];
	int nItem=pLocateDlg->m_pListCtrl->GetNextItem(-1,LVNI_ALL);
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)pLocateDlg->m_pListCtrl->GetItemData(nItem);
		if (pItem!=NULL)
		{
			if (pItem->GetPathLen()==dwLength)
			{
				if (pItem->IsDeleted())
				{
					MemCopyW(szPath,pItem->GetPath(),pItem->GetPathLen()+1);
					MakeLower(szPath);
					if (wcsncmp(szPath,szFile,dwLength)==0)
					{
						if (pItem->RemoveFlagsForChanged())
							pLocateDlg->PostMessage(WM_UPDATENEEDEDDETAILTS,WPARAM(nItem),LPARAM(pItem));
					}
				}
			}
		}
		nItem=pLocateDlg->m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
	}

	BkgDebugMessage("File created ENF");
}

inline void CCheckFileNotificationsThread::FileModified(LPCWSTR szFile,DWORD dwLength,CLocateDlg* pLocateDlg)
{
	BkgDebugNumMessage("File modified: %S",DWORD(szFile));

	if (pLocateDlg->m_pListCtrl==NULL)
		return;
	if (pLocateDlg->m_pListCtrl->GetItemCount()==0)
		return;

	WCHAR szPath[MAX_PATH];
	int nItem=pLocateDlg->m_pListCtrl->GetNextItem(-1,LVNI_ALL);
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)pLocateDlg->m_pListCtrl->GetItemData(nItem);
		if (pItem!=NULL)
		{
			if (pItem->GetPathLen()==dwLength)
			{
				MemCopyW(szPath,pItem->GetPath(),pItem->GetPathLen()+1);
				MakeLower(szPath);
			    if (wcsncmp(szPath,szFile,dwLength)==0)
				{
					if (pItem->RemoveFlagsForChanged())
						pLocateDlg->PostMessage(WM_UPDATENEEDEDDETAILTS,WPARAM(nItem),LPARAM(pItem));
				}
			}
		}
		nItem=pLocateDlg->m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
	}
	BkgDebugMessage("File modified END");
}

inline void CCheckFileNotificationsThread::FileDeleted(LPCWSTR szFile,DWORD dwLength,CLocateDlg* pLocateDlg)
{
	BkgDebugNumMessage("File deleted: %S",DWORD(szFile));

	if (pLocateDlg->m_pListCtrl==NULL)
		return;
	if (pLocateDlg->m_pListCtrl->GetItemCount()==0)
		return;

	WCHAR szPath[MAX_PATH];
	int nItem=pLocateDlg->m_pListCtrl->GetNextItem(-1,LVNI_ALL);
	while (nItem!=-1)
	{
		CLocatedItem* pItem=(CLocatedItem*)pLocateDlg->m_pListCtrl->GetItemData(nItem);
		if (pItem!=NULL)
		{
			if (pItem->GetPathLen()==dwLength)
			{
				if (!pItem->IsDeleted())
				{
					MemCopyW(szPath,pItem->GetPath(),pItem->GetPathLen()+1);
					MakeLower(szPath);
					if (wcsncmp(szPath,szFile,dwLength)==0)
					{
						pItem->SetToDeleted();
						pLocateDlg->PostMessage(WM_UPDATENEEDEDDETAILTS,WPARAM(nItem),LPARAM(pItem));
					}
				}
			}
		}
		nItem=pLocateDlg->m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
	}

	BkgDebugMessage("File deleted END");
}


BOOL CCheckFileNotificationsThread::RunningProcNew()
{
	BkgDebugMessage("CCheckFileNotificationsThread::RunningProcNew() BEGIN");
	CreateHandlesNew();
	DWORD dwOut;

	for (;;)
	{
		BkgDebugFormatMessage("CCheckFileNotificationsThread::RunningProc(), GOING TO SLEEP, m_pHandles[0]=%X",DWORD(m_pHandles[0]));
		DWORD nRet=WaitForMultipleObjects(m_nHandles,m_pHandles,FALSE,INFINITE);
		BkgDebugNumMessage("CCheckFileNotificationsThread::RunningProc(), WAKED nRet=%X",nRet);

		if (nRet==WAIT_OBJECT_0) // The first is end event
		{
			CAppData::stdfunc();
			break;
		}
		else if (nRet>WAIT_OBJECT_0 && nRet<WAIT_OBJECT_0+m_nHandles)
		{
			CLocateDlg* pLocateDlg=GetLocateDlg();
			
            // Locate dialog is also closed, stopping this process
			if (pLocateDlg==NULL)
				break;

			DIRCHANGEDATA* pChangeData=m_pChangeDatas[nRet-WAIT_OBJECT_0];
			
			// Asking changes
			if (pLocateDlg->m_pLocater==NULL) // if locating in process, do nothing
			{
				if (GetOverlappedResult(pChangeData->hDir,&pChangeData->ol,&dwOut,FALSE))
				{
					while (pLocateDlg->m_pBackgroundUpdater!=NULL &&
						!pLocateDlg->m_pBackgroundUpdater->m_lIsWaiting)
						Sleep(200);
					
					if (dwOut==0)
						UpdateItemsInRoot(pChangeData->szRoot,pLocateDlg);
					else
					{
						FILE_NOTIFY_INFORMATION* pStruct=(FILE_NOTIFY_INFORMATION*)pChangeData->pBuffer;
						while (1)
						{
							DWORD dwLength=pStruct->FileNameLength/2;
							
							m_pFile=new WCHAR[pChangeData->dwRootLength+dwLength+2];
							MemCopyW(m_pFile,pChangeData->szRoot,pChangeData->dwRootLength);
							MemCopyW(m_pFile+pChangeData->dwRootLength,pStruct->FileName,dwLength);
							dwLength+=pChangeData->dwRootLength;
							m_pFile[dwLength]='\0';
							MakeLower(m_pFile);

							switch(pStruct->Action)
							{
							case FILE_ACTION_ADDED:
							case FILE_ACTION_RENAMED_NEW_NAME:
								FileCreated(m_pFile,dwLength,pLocateDlg);
								break;
							case FILE_ACTION_REMOVED:
							case FILE_ACTION_RENAMED_OLD_NAME:
								FileDeleted(m_pFile,dwLength,pLocateDlg);
								break;
							case FILE_ACTION_MODIFIED:
								FileModified(m_pFile,dwLength,pLocateDlg);
								break;
							}
							delete[] m_pFile;
							m_pFile=NULL;
							
							BkgDebugMessage("CCheckFileNotificationsThread::RunningProc() 1");
							
							if (pStruct->NextEntryOffset==0)
								break;
							*((char**)&pStruct)+=pStruct->NextEntryOffset;

							BkgDebugMessage("CCheckFileNotificationsThread::RunningProc() 2");
						}		
					}
				}
			}
			
			BkgDebugMessage("CCheckFileNotificationsThread::RunningProc() Coing to listen changes");

			// Coing to listen changes
			BOOL bRet=m_pReadDirectoryChangesW(pChangeData->hDir,pChangeData->pBuffer,CHANGE_BUFFER_LEN,TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|
				FILE_NOTIFY_CHANGE_ATTRIBUTES|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE|
				FILE_NOTIFY_CHANGE_CREATION|FILE_NOTIFY_CHANGE_SECURITY,
				&dwOut,&pChangeData->ol,NULL);

			if (!bRet)
				ResetEvent(pChangeData->ol.hEvent);
		}
	}
	delete this;
	
	
	BkgDebugMessage("CCheckFileNotificationsThread::RunningProcNew() END");
	return FALSE;
}



BOOL CCheckFileNotificationsThread::RunningProcOld()
{
	BkgDebugNumMessage("CCheckFileNotificationsThread::RunningProcOld() BEGIN, thread is 0x%X",GetCurrentThreadId());
	CreateHandlesOld();
	for (;;)
	{
		BkgDebugMessage("CCheckFileNotificationsThread::RunningProcOld(), GOING TO SLEEP");
		DWORD nRet=WaitForMultipleObjects(m_nHandles,m_pHandles,FALSE,INFINITE);
		BkgDebugNumMessage("CCheckFileNotificationsThread::RunningProcOld(), WAKED nRet=%X",nRet);
		if (nRet==WAIT_OBJECT_0) // The first is end event
			break;
		else if (nRet>WAIT_OBJECT_0 && nRet<WAIT_OBJECT_0+m_nHandles)
		{
			BkgDebugFormatMessage4("Something is changed in %S",m_pRoots[nRet-WAIT_OBJECT_0],0,0,0);

			CLocateDlg* pLocateDlg=GetLocateDlg();
			
			// Locate dialog is also closed, stopping this process
			if (pLocateDlg==NULL)
				break;

			if (pLocateDlg->m_pLocater==NULL) // if locating in process, do nothing
			{
				while (pLocateDlg->m_pBackgroundUpdater!=NULL &&
					!pLocateDlg->m_pBackgroundUpdater->m_lIsWaiting)
					Sleep(200);
				
				// Updating changed items by checking all items
				UpdateItemsInRoot(m_pRoots[nRet-WAIT_OBJECT_0],pLocateDlg);		
			}

#ifdef _DEBUG_LOGGING
			BOOL bRet=FindNextChangeNotification(m_pHandles[nRet-WAIT_OBJECT_0]);
			BkgDebugFormatMessage4("CCheckFileNotificationsThread::RunningProcOld(): FindNextChangeNotification returns %X, nret=%X, nRet-WAIT_OBJECT_0=%X",bRet,nRet,nRet-WAIT_OBJECT_0,0);
#else	
			FindNextChangeNotification(m_pHandles[nRet-WAIT_OBJECT_0]);
#endif		
		}
		else 
			BkgDebugFormatMessage4("CCheckFileNotificationsThread::RunningProcOld(): nRet not handled, nRet=0x%X, handles=%d, GetLastError()=0x%X",nRet,m_nHandles,GetLastError(),0);


	}
	delete this;
	
	
	BkgDebugMessage("CCheckFileNotificationsThread::RunningProcOld() END");
	return FALSE;
}

void CCheckFileNotificationsThread::UpdateItemsInRoot(LPCWSTR szRoot,CLocateDlg* pLocateDlg)
{
	BkgDebugFormatMessage("CCheckFileNotificationsThread::UpdateItemsInRoot BEGIN szRoot=%s",szRoot);
	
	if (pLocateDlg->m_pListCtrl==NULL)
		return;
	if (pLocateDlg->m_pListCtrl->GetItemCount()==0)
		return;

	
	// Updating changed items by checking all items
	if (szRoot[1]=='\0')
	{
		BkgDebugMessage("CCheckFileNotificationsThread::UpdateItemsInRoot 1a");

		WCHAR szDriveLower=szRoot[0];
		WCHAR szDriveUpper=szRoot[0];
		MakeUpper(&szDriveUpper,1);

		BkgDebugMessage("CCheckFileNotificationsThread::UpdateItemsInRoot 2a");

		int nItem=pLocateDlg->m_pListCtrl->GetNextItem(-1,LVNI_ALL);
		while (nItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)pLocateDlg->m_pListCtrl->GetItemData(nItem);

			if (pItem!=NULL)
			{
				// Checking whether path is in changed volume
				LPCWSTR szPath=pItem->GetPath();
				if ((szPath[0]==szDriveLower || szPath[0]==szDriveUpper) &&
					szPath[1]==L':')
				{
					// Just disabling flags, let background thread do the rest
					if (pItem->RemoveFlagsForChanged())
						pLocateDlg->PostMessage(WM_UPDATENEEDEDDETAILTS,WPARAM(nItem),LPARAM(pItem));
				}
			}

			nItem=pLocateDlg->m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
		}
	}
	else
	{
		DWORD dwLength=(DWORD)istrlenw(szRoot);

		BkgDebugMessage("CCheckFileNotificationsThread::UpdateItemsInRoot 1b");

		int nItem=pLocateDlg->m_pListCtrl->GetNextItem(-1,LVNI_ALL);
		while (nItem!=-1)
		{
			CLocatedItem* pItem=(CLocatedItem*)pLocateDlg->m_pListCtrl->GetItemData(nItem);

			if (pItem!=NULL)
			{
				if (pItem->GetPathLen()>=dwLength)
				{
					LPWSTR szPath=alloccopy(pItem->GetPath(),pItem->GetPathLen());
					MakeLower(szPath);
		                
					if (wcsncmp(szPath,szRoot,dwLength)==0)
					{
						// Just disabling flags, let background thread do the rest
						if (pItem->RemoveFlagsForChanged())
							pLocateDlg->PostMessage(WM_UPDATENEEDEDDETAILTS,WPARAM(nItem),LPARAM(pItem));
					}
					delete[] szPath;
				}
			}
			nItem=pLocateDlg->m_pListCtrl->GetNextItem(nItem,LVNI_ALL);
		}
	}

	DebugMessage("CCheckFileNotificationsThread::UpdateItemsInRoot END");
}

DWORD WINAPI CCheckFileNotificationsThread::NotificationThreadProc(LPVOID lpParameter)
{
	if (((CCheckFileNotificationsThread*)lpParameter)->m_pReadDirectoryChangesW!=NULL)
		return ((CCheckFileNotificationsThread*)lpParameter)->RunningProcNew();
	return ((CCheckFileNotificationsThread*)lpParameter)->RunningProcOld();
}

BOOL CCheckFileNotificationsThread::CreateHandlesNew()
{
	BkgDebugMessage("CCheckFileNotificationsThread::CreateHandlesNew() BEGIN");
	
	ASSERT(m_pHandles==NULL);

	CArrayFAP<LPWSTR> aRoots;
	CDatabaseInfo::GetRootsFromDatabases(aRoots,GetLocateApp()->GetDatabases(),TRUE);
	
    m_pHandles=new HANDLE[aRoots.GetSize()+2];
	m_pChangeDatas=new DIRCHANGEDATA*[aRoots.GetSize()+2];
	DIRCHANGEDATA* pChangeData=NULL;
	
	ASSERT(m_pHandles!=NULL);

	m_pHandles[0]=CreateEvent(NULL,FALSE,FALSE,NULL);
	DebugOpenEvent(m_pHandles[0]);
	m_pChangeDatas[0]=NULL;

	m_nHandles=1;
	DWORD dwOut;


	for (int i=0;i<aRoots.GetSize();i++)
	{
		LPWSTR szRoot=aRoots.GetAt(i);
		if (szRoot[1]==':' && szRoot[2]=='\0')
		{
			szRoot=alloccopy(szRoot,3);
			szRoot[2]=L'\\';
		}
		
#ifdef _DEBUG_LOGGING
		// If logging is on, do not use change notifications for root containing log file
		LPCSTR pLogFile=GetDebugLoggingFile();
		if (pLogFile!=NULL)
		{
			if (szRoot[1]==':' && szRoot[2]=='\0')
			{
				char szDrive[]="X:\\";
				szDrive[0]=W2Ac(szRoot[0]);
				MakeLower(szDrive,1);
				if (szDrive[0]==pLogFile[0] && pLogFile[1]==':')
					continue;
			}
			else
			{
				char* szPath=alloccopyWtoA(szRoot);
				MakeLower(szPath);
				BOOL bSame=strcmp(szPath,pLogFile)==0;
				delete[] szPath;
                if (bSame)
					continue;
			}		
		}
#endif
		
		// Allocating new CHANGEDATA struct
		if (pChangeData==NULL)
		{
			pChangeData=new DIRCHANGEDATA;
			pChangeData->ol.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
			DebugOpenEvent(pChangeData->ol.hEvent);
			pChangeData->pBuffer=new BYTE[CHANGE_BUFFER_LEN];
		}

		if (IsUnicodeSystem())
			pChangeData->hDir=CreateFileW(szRoot,GENERIC_READ /*FILE_LIST_DIRECTORY*/,FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,
				NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,NULL);
		else
			pChangeData->hDir=CreateFile(W2A(szRoot),GENERIC_READ /*FILE_LIST_DIRECTORY*/,FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,
				NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,NULL);
		DebugOpenHandle(dhtFile,pChangeData->hDir,szRoot);


		if (pChangeData->hDir==INVALID_HANDLE_VALUE)
			continue;


		BOOL bRet=m_pReadDirectoryChangesW(pChangeData->hDir,pChangeData->pBuffer,CHANGE_BUFFER_LEN,TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|
            FILE_NOTIFY_CHANGE_ATTRIBUTES|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE|
            FILE_NOTIFY_CHANGE_CREATION|FILE_NOTIFY_CHANGE_SECURITY,
			&dwOut,&pChangeData->ol,NULL);
		
				
		if (bRet)
		{
			pChangeData->dwRootLength=(DWORD)istrlenw(szRoot);
			if (szRoot[pChangeData->dwRootLength-1]=='\\')
			{
				if (szRoot!=aRoots.GetAt(i))
					pChangeData->szRoot=szRoot;
				else
				{
					pChangeData->szRoot=new WCHAR[pChangeData->dwRootLength+4];
					MemCopyW(pChangeData->szRoot,szRoot,pChangeData->dwRootLength+1);
				}
				MakeLower(pChangeData->szRoot);
			}
			else
			{
				pChangeData->szRoot=new WCHAR[pChangeData->dwRootLength+4];
				MemCopyW(pChangeData->szRoot,szRoot,pChangeData->dwRootLength);
				pChangeData->szRoot[pChangeData->dwRootLength++]='\\';
				pChangeData->szRoot[pChangeData->dwRootLength]='\0';
				MakeLower(pChangeData->szRoot);
			}
			m_pHandles[m_nHandles]=pChangeData->ol.hEvent;
			m_pChangeDatas[m_nHandles]=pChangeData;


			pChangeData=NULL;
			m_nHandles++;

		}
		else
		{
			CloseHandle(pChangeData->hDir);
			DebugCloseHandle(dhtFile,pChangeData->hDir,szRoot);
		}

	}


	if (pChangeData!=NULL)
		delete pChangeData;
	
	BkgDebugMessage("CCheckFileNotificationsThread::CreateHandlesNew() END");
	return TRUE;
}

BOOL CCheckFileNotificationsThread::CreateHandlesOld()
{
	ASSERT(m_pHandles==NULL);

	BkgDebugMessage("CCheckFileNotificationsThread::CreateHandlesOld() BEGIN");
	
	CArrayFAP<LPWSTR> aRoots;
	CDatabaseInfo::GetRootsFromDatabases(aRoots,GetLocateApp()->GetDatabases());
	
	m_pHandles=new HANDLE[aRoots.GetSize()+1];
	m_pRoots=new LPWSTR[aRoots.GetSize()+1];
	
	ASSERT(m_pHandles!=NULL);

	m_pHandles[0]=CreateEvent(NULL,FALSE,FALSE,STRNULL);
	DebugOpenEvent(m_pHandles[0]);
	m_pRoots[0]=NULL;

	m_nHandles=1;
	for (int i=0;i<aRoots.GetSize();i++)
	{
		const LPWSTR szRoot=aRoots.GetAt(i);
		if (szRoot[1]==':' && szRoot[2]=='\0')
		{
			char szDrive[]="X:\\";
			szDrive[0]=W2Ac(szRoot[0]);
			
#ifdef _DEBUG_LOGGING
			// If logging is on, do not use change notifications for root containing log file
			LPCSTR pLogFile=GetDebugLoggingFile();
			if (pLogFile!=NULL)
			{
				MakeLower(szDrive,1);
				if (szDrive[0]==pLogFile[0] && pLogFile[1]==':')
					continue;
			}			
#endif
			

			BkgDebugFormatMessage4("Type of %s is 0x%X",szDrive,FileSystem::GetDriveType(szRoot),0,0);
			m_pHandles[m_nHandles]=FindFirstChangeNotification(szDrive,TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE);
			DebugOpenHandle(dhtMisc,m_pHandles[m_nHandles],STRNULL);

			BkgDebugFormatMessage4("FindFirstChangeNotification1,%d returned: 0x%X, drive is %s, GetLastError()=0x%X",i,m_pHandles[m_nHandles],szDrive,GetLastError());
			if (m_pHandles[m_nHandles]!=INVALID_HANDLE_VALUE)
			{
				m_pRoots[m_nHandles]=new WCHAR[2];
				m_pRoots[m_nHandles][0]=szRoot[0];
				m_pRoots[m_nHandles][1]='\0';
				MakeLower(m_pRoots[m_nHandles],1);
				m_nHandles++;
			}
		}
		else
		{
			
#ifdef _DEBUG_LOGGING
			// If logging is on, do not use change notifications for root containing log file
			LPCSTR pLogFile=GetDebugLoggingFile();
			if (pLogFile!=NULL)
			{
				char* szPath=alloccopyWtoA(szRoot);
                MakeLower(szPath);

				BOOL bSame=strcmp(szPath,pLogFile)==0;
				delete[] szPath;
                if (bSame)
					continue;
			}			
#endif
			if (IsUnicodeSystem())
				m_pHandles[m_nHandles]=FindFirstChangeNotificationW(szRoot,TRUE,
					FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE);
			else
				m_pHandles[m_nHandles]=FindFirstChangeNotification(W2A(szRoot),TRUE,
					FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE);
			DebugOpenEvent(m_pHandles[m_nHandles]);

			BkgDebugFormatMessage4("FindFirstChangeNotification2,%d returned: 0x%X, drive is %S, GetLastError()=0x%X",i,m_pHandles[m_nHandles],szRoot,GetLastError());
			if (m_pHandles[m_nHandles]!=INVALID_HANDLE_VALUE)
			{
				m_pRoots[m_nHandles]=alloccopy(szRoot);
				MakeLower(m_pRoots[m_nHandles]);
				m_nHandles++;
			}
		}
	}
	

	BkgDebugMessage("CCheckFileNotificationsThread::CreateHandlesOld() END");
	return TRUE;
}

BOOL CCheckFileNotificationsThread::DestroyHandles()
{
	BkgDebugMessage("CCheckFileNotificationsThread::DestroyHandles() BEGIN");
	
	if (m_pHandles==NULL)
		return TRUE;

	HANDLE* pHandles=m_pHandles;
	m_pHandles=NULL;
	DWORD nHandles=m_nHandles;

	if (m_pReadDirectoryChangesW!=NULL)
	{
		DIRCHANGEDATA** pChangeDatas=m_pChangeDatas;
		m_pChangeDatas=NULL;
	
		// Pointers used with ReadDirectoryChangesW		
		CloseHandle(pHandles[0]);
		DebugCloseEvent(pHandles[0]);
		for (UINT n=1;n<nHandles;n++)
		{
			//DebugFormatMessage("Deleting CHANGEDIR for %s",pChangeDatas[n]->szRoot);
			
			ASSERT(pHandles[n]==pChangeDatas[n]->ol.hEvent);

			// Destructor also closes this handle
			// CloseHandle(pHandles[n]);

			delete pChangeDatas[n];
		}
		delete[] pHandles;
		delete[] pChangeDatas;
	}
	else
	{
		// Pointers used with traditional method
		CloseHandle(pHandles[0]);
		DebugCloseEvent(pHandles[0]);
		for (UINT n=1;n<nHandles;n++)
		{
			FindCloseChangeNotification(pHandles[n]);
			DebugCloseHandle(dhtMisc,pHandles[n],STRNULL);
			delete[] m_pRoots[n];
		}
		delete[] pHandles;
		delete[] m_pRoots;
	}
	m_pRoots=NULL;
	BkgDebugMessage("CCheckFileNotificationsThread::DestroyHandles() END");
	return TRUE;
}



////////////////////////////////////////////
// CBackgroundUpdater

BOOL CBackgroundUpdater::Start()
{
#ifdef THREADDISABLE_BACKROUNDUPDATER
	return TRUE;
#else
    BkgDebugMessage("CBackgroundUpdater::Start() BEGIN");

	if (m_hThread!=NULL)
	{
		BkgDebugMessage("CBackgroundUpdater::Start() ALREADY STARTED");
		return FALSE;
	}

	DWORD dwThreadID;
	HANDLE hThread=CreateThread(NULL,0,UpdaterThreadProc,this,CREATE_SUSPENDED,&dwThreadID);
	DebugOpenHandle(dhtThread,hThread,"BackgroundUpdater");
	if (hThread==NULL)
		return FALSE;
	
	InterlockedExchangePointer(&m_hThread,hThread);
	SetThreadPriority(m_hThread,THREAD_PRIORITY_BELOW_NORMAL);
	
	InterlockedExchange(&m_lIsWaiting,FALSE);
	ResumeThread(m_hThread);

	BkgDebugMessage("CBackgroundUpdater::Start() END");
	return TRUE;
#endif
}


void CBackgroundUpdater::CreateEventsAndMutex()
{
	m_phEvents[0]=CreateEvent(NULL,TRUE,FALSE,NULL);
	DebugOpenEvent(m_phEvents[0]);
	m_phEvents[1]=CreateEvent(NULL,TRUE,FALSE,NULL);
	DebugOpenEvent(m_phEvents[1]);

	m_hUpdateListPtrInUse=CreateMutex(NULL,FALSE,NULL);
	DebugOpenMutex(m_hUpdateListPtrInUse);
}

CBackgroundUpdater::~CBackgroundUpdater()
{	
	if (m_hThread!=NULL)
	{
		InterlockedExchangePointer((PVOID*)&GetLocateDlg()->m_pBackgroundUpdater,NULL);
		HANDLE hThread=m_hThread;
		InterlockedExchangePointer(&m_hThread,NULL);
		m_hThread=NULL;

		BkgDebugMessage("CBackgroundUpdater::~CBackgroundUpdater()");
		CloseHandle(hThread);
		DebugCloseThread(hThread);
	}
	else
		InterlockedExchangePointer((PVOID*)&GetLocateDlg()->m_pBackgroundUpdater,NULL);
	
	
	CloseHandle(m_phEvents[0]);
	DebugCloseEvent(m_phEvents[0]);
	CloseHandle(m_phEvents[1]);
	DebugCloseEvent(m_phEvents[1]);

	if (m_hUpdateListPtrInUse!=NULL)
	{
		CloseHandle(m_hUpdateListPtrInUse);
		DebugCloseMutex(m_hUpdateListPtrInUse);
		m_hUpdateListPtrInUse=NULL;
	}
}


BOOL CBackgroundUpdater::Stop()
{
#ifdef THREADDISABLE_BACKROUNDUPDATER
	return TRUE;
#else
	BkgDebugMessage("CBackgroundUpdater::Stop() BEGIN");

	HANDLE hThread=m_hThread;
	DWORD status=0;
	if (hThread==NULL)
	{
		BkgDebugMessage("CBackgroundUpdater::Stop() END_1");
		if (GetLocateDlg()->m_pBackgroundUpdater!=NULL)
			delete this;
		return FALSE;
	}
	
	BOOL bRet=::GetExitCodeThread(m_hThread,&status);
	if (bRet && status==STILL_ACTIVE)
	{
		InterlockedExchange(&m_lGoToSleep,TRUE);

		SetEvent(m_phEvents[0]); // 0 = end envent
		for (int i=0;i<30 && GetLocateDlg()->m_pBackgroundUpdater!=NULL;i++)
			Sleep(10);

		if (GetLocateDlg()->m_pBackgroundUpdater!=NULL)
		{
			BOOL bTerminated=FALSE;

			status=0;
			bRet=::GetExitCodeThread(hThread,&status);
			while (bRet && status==STILL_ACTIVE)
			{
				if (::TerminateThread(hThread,1))
					bTerminated=TRUE;
				Sleep(100);
				bRet=::GetExitCodeThread(hThread,&status);
			}
	
			if (bTerminated && m_hThread!=NULL)
				delete this;
		}
	}

	BkgDebugMessage("CBackgroundUpdater::Stop() END");
	return TRUE;
#endif
}

inline BOOL CBackgroundUpdater::RunningProc()
{
	BkgDebugNumMessage("CBackgroundUpdater::RunningProc() BEGIN, running thread 0x%X",GetCurrentThreadId());

	for (;;)
	{
		InterlockedExchange(&m_lGoToSleep,FALSE);
		InterlockedExchange(&m_lIsWaiting,TRUE);

		BkgDebugMessage("CBackgroundUpdater::RunningProc(): Going to sleep.");
		
		DWORD nRet=WaitForMultipleObjects(2,m_phEvents,FALSE,INFINITE);
		InterlockedExchange(&m_lIsWaiting,FALSE);

		BkgDebugFormatMessage4("CBackgroundUpdater::RunningProc(): Woked, nRet=%d.",nRet,0,0,0);


		if (nRet==WAIT_TIMEOUT)
			continue;
		if (nRet==WAIT_OBJECT_0) // 0 = end event
			break;
		else if (nRet==WAIT_OBJECT_0+1)
		{
			RECT rcViewRect;
			DWORD nListStyle=m_pList->GetStyle()&LVS_TYPEMASK;
			m_pList->GetClientRect(&rcViewRect);
			


			int nIconSizeX,nIconSizeY;
			ImageList_GetIconSize(
				m_pList->GetImageList(nListStyle==LVS_ICON?LVSIL_NORMAL:LVSIL_SMALL),
				&nIconSizeX,&nIconSizeY);

			
			BkgDebugNumMessage("CBackgroundUpdater::RunningProc(): Waked. %d item to be updated",m_aUpdateList.GetSize());

			for (;;)
			{
				// Swapping items about to be updated to local variable
				// to prevend data access error between threads
				
				CArrayFP<CBackgroundUpdater::Item*>* pList=GetUpdaterListPtr();
				if (pList==NULL)
					continue;

				CArrayFP<Item*> aUpdateList;
				aUpdateList.Swap(*pList);
				ReleaseUpdaterListPtr();
                
				if (aUpdateList.GetSize()==0)
					break;
			
				
				for (int i=0;i<aUpdateList.GetSize() && !m_lGoToSleep;i++)
				{
					Item* pItem=aUpdateList.GetAt(i);

					BkgDebugFormatMessage("Checking whether item %s needs to be updated ",pItem->m_pItem->GetName());

					POINT pt;
					if (m_pList->GetItemPosition(pItem->m_iItem,&pt))
					{
						BkgDebugFormatMessage4("step two for %s, pt.x=%d, pt.y=%d",
							pItem->m_pItem->GetName(),pt.x,pt.y,0);

						// X axes does not need to be checked when report mode is on
						POINT ptOrigin;
						if (!m_pList->GetOrigin(&ptOrigin))
						{
							ptOrigin.x=0;
							ptOrigin.y=0;
						}

						if (pt.y >= ptOrigin.y-nIconSizeY && pt.y<=rcViewRect.bottom+ptOrigin.y &&
							(nListStyle==LVS_REPORT || (pt.x >= ptOrigin.x-nIconSizeX && pt.x<=rcViewRect.right+ptOrigin.x)))
						{
							BOOL bReDraw=FALSE;
							
							BkgDebugFormatMessage("Refreshing %s",pItem->m_pItem->GetName());
							pItem->m_pItem->ReFresh(pItem->m_aDetails,bReDraw); // Item is visible

							if (bReDraw)
								m_pList->PostMessage(LVM_REDRAWITEMS,pItem->m_iItem,pItem->m_iItem);
						}
					}
					
				}
				aUpdateList.RemoveAll();
				
			}
			ResetEvent(m_phEvents[1]);
			BkgDebugMessage("CBackgroundUpdater::RunningProc(): I am tired");
			
			InterlockedExchange(&m_lGoToSleep,FALSE);
		}
		else 
			BkgDebugFormatMessage4("CBackgroundUpdater::RunningProc(): nRet not handled, nRet:0x%X, events=2, GetLastError()=0x%X",nRet,GetLastError(),0,0);
	}
	delete this;
	
	DebugMessage("CBackgroundUpdater::RunningProc() END");
	return TRUE;
}

DWORD WINAPI CBackgroundUpdater::UpdaterThreadProc(LPVOID lpParameter)
{
	return ((CBackgroundUpdater*)lpParameter)->RunningProc();
}

void CBackgroundUpdater::AddToUpdateList(CLocatedItem* pItem, int iItem,CLocateDlg::DetailType nDetail)
{
	BkgDebugFormatMessage("CBackgroundUpdater::AddToUpdateList BEGIN this is %X",this);
	
	CArrayFP<Item*>* pUpdateList=GetUpdaterListPtr();
	if (pUpdateList==NULL)
		return;

	for (int i=pUpdateList->GetSize()-1;i>=0;i--)
	{
		Item* pListItem=pUpdateList->GetAt(i);

		BkgDebugFormatMessage4("CBackgroundUpdater::AddToUpdateList Checking whether pItem=%X is pListItem=%X, i=%d, listsize=%d",
			(DWORD_PTR)pItem,(DWORD_PTR)pListItem,i,pUpdateList->GetSize());

		if (pListItem==NULL)
			continue;
		
		

		if (pUpdateList->GetAt(i)->m_pItem==pItem)
		{
			if (nDetail==CLocateDlg::Needed)
			{
				BkgDebugMessage("CBackgroundUpdater::AddToUpdateList checking whether all needed details");
				
				for (int type=0;type<=CLocateDlg::LastType;type++)
				{
					if (pItem->ShouldUpdateByDetail((CLocateDlg::DetailType)type))
					{
						int j;
                        for (j=pListItem->m_aDetails.GetSize()-1;j>=0;j--)
						{
							if (pListItem->m_aDetails[j]==(CLocateDlg::DetailType)type)
								break;
						}
						if (j<0) // Not found
						{
							BkgDebugFormatMessage4("CBackgroundUpdater::AddToUpdateList Adding new(1) detail %d to item %X",nDetail,(DWORD_PTR)pListItem,0,0);
							pListItem->m_aDetails.Add((CLocateDlg::DetailType)type);
						}
					}
				}
			}
            else
			{
				BkgDebugFormatMessage("CBackgroundUpdater::AddToUpdateList checking wheter detail %d exists",nDetail);
					
				int j;
				for (j=pListItem->m_aDetails.GetSize()-1;j>=0;j--)
				{
					if (pListItem->m_aDetails[j]==nDetail)
						break;
				}
				if (j<0)
				{
					// Not found
					BkgDebugFormatMessage4("CBackgroundUpdater::AddToUpdateList Adding new(2) detail %d to item %X",nDetail,(DWORD_PTR)pListItem,0,0);
					pListItem->m_aDetails.Add(nDetail);
				}
			}
			ReleaseUpdaterListPtr();
			
			BkgDebugMessage("CBackgroundUpdater::AddToUpdateList END (loop)");
			return;
		}
	}
	

	BkgDebugMessage("CBackgroundUpdater::AddToUpdateList END (loop)");
			
	if (nDetail!=CLocateDlg::Needed)
	{
		Item* pNewItem=new Item(pItem,iItem,nDetail);
		BkgDebugFormatMessage4("CBackgroundUpdater::AddToUpdateList Adding new item (%X) %X list size=%d",(DWORD_PTR)pItem,(DWORD_PTR)pNewItem,pUpdateList->GetSize(),0);
		pUpdateList->Add(pNewItem);
		ReleaseUpdaterListPtr();

		BkgDebugMessage("CBackgroundUpdater::AddToUpdateList END (newitem)");
		return;
	}

	// Adding needed
	ReleaseUpdaterListPtr();
	
	BkgDebugFormatMessage4("CBackgroundUpdater::AddToUpdateList Adding new item with all necessary details (%X,%d)",pItem,iItem,0,0);
	Item* pUpdateItem=new Item(pItem,iItem);

	for (int type=0;type<=CLocateDlg::LastType;type++)
	{
		if (pItem->ShouldUpdateByDetail((CLocateDlg::DetailType)type))
			pUpdateItem->m_aDetails.Add((CLocateDlg::DetailType)type);
	}
	if (pUpdateItem->m_aDetails.GetSize()>0)
	{
		pUpdateList=GetUpdaterListPtr();
		BkgDebugFormatMessage4("CBackgroundUpdater::AddToUpdateList Adding new item (%X), list size=%d",
			(DWORD_PTR)pUpdateItem,pUpdateList->GetSize(),0,0);
		pUpdateList->Add(pUpdateItem); 
		ReleaseUpdaterListPtr();
	}
	else
		delete pUpdateItem;
	

	BkgDebugMessage("CBackgroundUpdater::AddToUpdateList END (normal)");
}
