#if !defined(BACKGROUND_H)
#define BACKGROUND_H

#if _MSC_VER >= 1000
#pragma once
#endif

class CCheckFileNotificationsThread
{
public:
	CCheckFileNotificationsThread();
	~CCheckFileNotificationsThread();

public:
	BOOL Start();
	BOOL Stop();

	void CouldStop();

	static DWORD WINAPI NotificationThreadProc(LPVOID lpParameter);
	BOOL RunningProcNew();
	BOOL RunningProcOld();

	void FileCreated(LPCSTR szFile,DWORD dwLength,CLocateDlg* pLocateDlg);
	void FileModified(LPCSTR szFile,DWORD dwLength,CLocateDlg* pLocateDlg);
	void FileDeleted(LPCSTR szFile,DWORD dwLength,CLocateDlg* pLocateDlg);
	void UpdateItemsInRoot(LPCSTR szRoot,CLocateDlg* pLocateDlg);

protected:
	BOOL CreateHandlesOld();
	BOOL CreateHandlesNew();
	BOOL DestroyHandles();
	

	

public:
	HANDLE m_hThread;

	UINT m_nHandles;
	HANDLE* m_pHandles;
	struct DIRCHANGEDATA{
		DIRCHANGEDATA();
		~DIRCHANGEDATA();

		OVERLAPPED ol;
		HANDLE hDir;
		BYTE* pBuffer;
		char* szRoot;
		DWORD dwRootLength;

	};
	union {
		LPSTR* m_pRoots;
		DIRCHANGEDATA** m_pChangeDatas;
	};
	
	BOOL (WINAPI* m_pReadDirectoryChangesW)(HANDLE,LPVOID,DWORD,BOOL,DWORD,LPDWORD,LPOVERLAPPED,LPOVERLAPPED_COMPLETION_ROUTINE);
#ifdef _DEBUG
public:
	void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif

};

class CBackgroundUpdater
{
public:
	CBackgroundUpdater(CListCtrl* pList);
	~CBackgroundUpdater();

public:
	BOOL Start();
	BOOL Stop();
	void GoToSleep();

	void CouldStop();

	static DWORD WINAPI UpdaterThreadProc(LPVOID lpParameter);
	BOOL RunningProc();

	void StopWaiting();
	BOOL IsWaiting() const;

	void AddToUpdateList(CLocatedItem* pItem, int iItem,CLocateDlg::DetailType nDetail);

public:
	HANDLE m_hThread;
	HANDLE m_phEvents[2]; // First is end event, secont is don't wait event

	struct Item{
		CLocatedItem* m_pItem;
		int m_iItem;
		CArray<CLocateDlg::DetailType> m_aDetails;
		
		Item() {};
		Item(CLocatedItem* pItem, int iItem);
		Item(CLocatedItem* pItem, int iItem,CLocateDlg::DetailType nDetail);
		

	public:
		// Overloaded operators, for fast allocating
		void* operator new(size_t size) { return Allocation.Allocate(size); }
		void operator delete(void* pObject) { Allocation.Free(pObject); }
		void operator delete(void* pObject,size_t size) { Allocation.Free(pObject); }
	};
	
	CListCtrl* m_pList;

	volatile LONG m_lIsWaiting;
	volatile LONG m_lGoToSleep;


	// Creating m_aUpdateList as threadsafe
protected:
	CArrayFP<Item*> m_aUpdateList;
	HANDLE m_hUpdateListPtrInUse;

public:
	CArrayFP<Item*>* GetUpdaterListPtr();
	void ReleaseUpdaterListPtr();
	
	

#ifdef _DEBUG
public:
	void* operator new(size_t size) { return DebugAlloc.Allocate(size,__LINE__,__FILE__); }
	void operator delete(void* pObject) { DebugAlloc.Free(pObject); }
	void operator delete(void* pObject,size_t size) { DebugAlloc.Free(pObject); }
#endif
};

inline CCheckFileNotificationsThread::DIRCHANGEDATA::DIRCHANGEDATA()
:	hDir(NULL),pBuffer(NULL),szRoot(NULL)
{
	ol.hEvent=NULL;
}


inline CCheckFileNotificationsThread::DIRCHANGEDATA::~DIRCHANGEDATA()
{
	if (hDir!=NULL)
	{
		CancelIo(hDir);
		CloseHandle(hDir);
	}
	if (ol.hEvent!=NULL)
	{
		CloseHandle(ol.hEvent);
	}
	if (pBuffer!=NULL)
		delete[] pBuffer;
	if (szRoot!=NULL)
		delete[] szRoot;
}


inline CCheckFileNotificationsThread::CCheckFileNotificationsThread()
:	m_hThread(NULL),m_nHandles(0),m_pHandles(NULL),m_pRoots(NULL)
{
	DebugMessage("CCheckFileNotificationsThread::CCheckFileNotificationsThread()");

	if (GetLocateDlg()->GetFlags()&CLocateDlg::fgOtherUseOldMethodToNotifyChanges)
		m_pReadDirectoryChangesW=NULL;
	else
		m_pReadDirectoryChangesW=(BOOL (WINAPI*)(HANDLE,LPVOID,DWORD,BOOL,DWORD,LPDWORD,LPOVERLAPPED,LPOVERLAPPED_COMPLETION_ROUTINE))
			GetProcAddress(GetModuleHandle("kernel32.dll"),"ReadDirectoryChangesW");
	
}

inline CCheckFileNotificationsThread::~CCheckFileNotificationsThread()
{	
	DestroyHandles();
		
	if (m_hThread!=NULL)
	{
		
		GetLocateDlg()->m_pFileNotificationsThread=NULL;
		HANDLE hThread=m_hThread;
		m_hThread=NULL;

		CloseHandle(hThread);
	}
	else
		GetLocateDlg()->m_pFileNotificationsThread=NULL;

}

inline void CCheckFileNotificationsThread::CouldStop()
{
	if (m_pHandles==NULL)
		return;
	SetEvent(m_pHandles[0]);
}


inline CBackgroundUpdater::CBackgroundUpdater(CListCtrl* pList)
:	m_pList(pList),m_hThread(NULL)
{
	BkgDebugMessage("CBackgroundUpdater::CBackgroundUpdater()");
	m_phEvents[0]=CreateEvent(NULL,TRUE,FALSE,NULL);
	m_phEvents[1]=CreateEvent(NULL,TRUE,FALSE,NULL);

	m_hUpdateListPtrInUse=CreateMutex(NULL,FALSE,NULL);
}

inline CBackgroundUpdater::~CBackgroundUpdater()
{	
	if (m_hThread!=NULL)
	{
		GetLocateDlg()->m_pBackgroundUpdater=NULL;
		HANDLE hThread=m_hThread;
		m_hThread=NULL;

		BkgDebugMessage("CBackgroundUpdater::~CBackgroundUpdater()");
		CloseHandle(hThread);
	}
	else
		GetLocateDlg()->m_pBackgroundUpdater=NULL;
	
	
	CloseHandle(m_phEvents[0]);
	CloseHandle(m_phEvents[1]);

	if (m_hUpdateListPtrInUse!=NULL)
	{
		CloseHandle(m_hUpdateListPtrInUse);
		m_hUpdateListPtrInUse=NULL;
	}
}


inline CArrayFP<CBackgroundUpdater::Item*>* CBackgroundUpdater::GetUpdaterListPtr()
{
	// Waiting for other thread to complete
	BkgDebugMessage("CBackgroundUpdater::GetUpdaterListPtr() waiting for mutex");

	if (WaitForMutex(m_hUpdateListPtrInUse,BACKGROUNDUPDATERMUTEXTIMEOUT))
	{
		BkgDebugMessage("CBackgroundUpdater::GetUpdaterListPtr() WaitForMutex returns error");
		return NULL;
	}
	BkgDebugMessage("CBackgroundUpdater::GetUpdaterListPtr() got mutex");
	return &m_aUpdateList;
}

inline void CBackgroundUpdater::ReleaseUpdaterListPtr()
{
	BkgDebugMessage("CBackgroundUpdater::ReleaseUpdaterListPtr() releasing mutex");
	ReleaseMutex(m_hUpdateListPtrInUse);
	BkgDebugMessage("CBackgroundUpdater::ReleaseUpdaterListPtr() releasing mutex done");
}


inline void CBackgroundUpdater::StopWaiting()
{
	BkgDebugMessage("CBackgroundUpdater::StopWaiting() BEGIN");
	if (m_aUpdateList.GetSize()==0)
	{
		BkgDebugMessage("CBackgroundUpdater::StopWaiting() END (no wake)");
	
		return; // No reason to wake
	}
    if (m_hThread!=NULL)
		Start();
	if (m_lIsWaiting)
		SetEvent(m_phEvents[1]);

	InterlockedExchange(&m_lIsWaiting,FALSE);
	BkgDebugMessage("CBackgroundUpdater::StopWaiting() END");
	
}

inline void CBackgroundUpdater::CouldStop()
{
	if (m_hThread==NULL)
		return;
	
	InterlockedExchange(&m_lGoToSleep,TRUE);
	SetEvent(m_phEvents[0]); // 0 = end envent
}

inline BOOL CBackgroundUpdater::IsWaiting() const
{
	return m_lIsWaiting;
}

inline void CBackgroundUpdater::GoToSleep()
{
	InterlockedExchange(&m_lGoToSleep,TRUE);
}

	
inline CBackgroundUpdater::Item::Item(CLocatedItem* pItem, int iItem)
:	m_pItem(pItem),m_iItem(iItem)
{
}

inline CBackgroundUpdater::Item::Item(CLocatedItem* pItem, int iItem,CLocateDlg::DetailType nDetail)
:	m_pItem(pItem),m_iItem(iItem)
{
	m_aDetails.Add(nDetail);
}
#endif