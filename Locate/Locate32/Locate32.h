#ifndef LOCATE32_H
#define LOCATE32_H

class CLocateDlg;
class CFileProperties;
class CLocater;
class CLocatedItem;
class CLocateApp;
class CCheckFileNotificationsThread;
class CBackgroundUpdater;

extern LPSTR g_szBuffer; 


#include "commonres.h"
#include "lan_resources.h"

#include "locatedb.h"
#include "locater.h"


#include "ImageHandler.h"
#include "keyhelper.h"


// This macro is defined to ensure that g_szBuffer is only used in 
// LocateDlg thread, not simultaneously by several threaad. 
#define ISDLGTHREADOK		{ASSERT(GetCurrentThreadId()==GetLocateAppWnd()->m_pLocateDlgThread->m_nThreadID); }


// Wait times for thread safe system
#define BACKGROUNDUPDATERMUTEXTIMEOUT	5000
#define LOCATEAPPUPDATERSMUTEXTIMEOUT	5000


#define BUFFERALLOC_EXTRALEN				0
#define DEBUGALLOC_EXTRALEN					10

//DONT USE #define FASTALLOCATORTYPE					CBufferAllocator<BYTE*,65535,BUFFERALLOC_EXTRALEN>
#define FASTALLOCATORTYPE					CBufferAllocatorThreadSafe<BYTE*,65535,BUFFERALLOC_EXTRALEN>
//#define FASTALLOCATORTYPE					CAllocator

//#define DEBUGALLOCATORTYPE					CDebugAllocator<DEBUGALLOC_EXTRALEN>
#define DEBUGALLOCATORTYPE					CAllocator

//#define THREADDISABLE_BACKROUNDUPDATER
//#define THREADDISABLE_CHANGENOTIFIER



// Defaults:
#define DEFAULT_NUMBEROFDIRECTORIES		4


#include "Debugging.h"

#include "Data.h"
#include "FileObject.h"
#include "LocateApp.h"
#include "LocateDlg.h"
#include "LocatedItem.h"
#include "Background.h"
#include "ResultsDialogs.h"
#include "SettingsDlg.h"
#include "DatabaseInfos.h"
#include "SmallDialogs.h"
#include "AboutDlg.h"

#include "LocatedItem.inl"
#include "LocateDlg.inl"
#include "SettingsDlg.inl"


#define COLUMNSNUM					5
#define DEL_IMAGE					((CLocateApp*)GetApp())->m_nDelImage
#define DEF_IMAGE					((CLocateApp*)GetApp())->m_nDefImage
#define DIR_IMAGE					((CLocateApp*)GetApp())->m_nDirImage
#define DRIVE_IMAGE					((CLocateApp*)GetApp())->m_nDriveImage

#define ID_REDRAWITEMS				1
#define ID_CLICKWAIT				4
#define ID_LOCATEANIM				5
#define ID_UPDATEANIM				6
#define ID_SYNCSCHEDULES			7
#define ID_CHECKSCHEDULES			8

#define ID_UPDATESTATUS				1
#define ID_IDLEEXIT					2

#define RESULT_INCLUDEDATE			0x1
#define RESULT_INCLUDELABELS		0x2
#define RESULT_INCLUDEDBINFO		0x4
#define RESULT_INCLUDEDESCRIPTION	0x8
#define RESULT_INCLUDESELECTEDITEMS	0x10

#define RESULT_SAVESTATE			0x07

#define SUBMENU_FILEMENU					0
#define SUBMENU_FILEMENUNOITEMS				1
#define SUBMENU_CONTEXTMENUPLAIN			2
#define SUBMENU_CONTEXTMENUFORQUERIED		3
#define SUBMENU_CONTEXTMENUNOITEMS			4
#define SUBMENU_MULTIDIRSELECTION			5
#define SUBMENU_PRESETSELECTION				6


#define RESULT_ACTIVATESELECTEDITEMS	0x1000

#define WM_SYSTEMTRAY				WM_APP+100
#define WM_ANOTHERINSTANCE			WM_APP+101
#define WM_UPDATENEEDEDDETAILTS		WM_APP+102 //wParam is item, lParam is pointer to item
#define WM_REFRESHNOTIFIERHANDLERS	WM_APP+103
#define WM_GETLOCATEDLG				WM_APP+105
#define WM_FREEUPDATESTATUSPOINTERS	WM_APP+106


// String copyers
#define sMemCopy(dst,src,len)	CopyMemory(dst,src,len)
#define sMemZero(dst,len)		ZeroMemory(dst,len)
#define sMemSet(dst,val,len)	iMemSet(dst,val,len)
#define sstrlen(str,len)		dstrlen(str,len)

#define sMemCopyW				iMemCopyW
#define sstrlenW				dwstrlen


#endif