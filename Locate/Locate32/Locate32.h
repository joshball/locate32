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
#include "shortcut.h"
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
//#define FASTALLOCATORTYPE					CBufferAllocatorThreadSafe<BYTE*,65535,BUFFERALLOC_EXTRALEN>
#define FASTALLOCATORTYPE					CAllocator

//#define DEBUGALLOCATORTYPE					CDebugAllocator<DEBUGALLOC_EXTRALEN>
#define DEBUGALLOCATORTYPE					CAllocator

//#define THREADDISABLE_BACKROUNDUPDATER
//#define THREADDISABLE_CHANGENOTIFIER



// Defaults:
#define DEFAULT_NUMBEROFDIRECTORIES		4
#define DEFAULT_NUMBEROFNAMES			10
#define DEFAULT_NUMBEROFTYPES			10


// Timer IDs
#define ID_REDRAWITEMS				1
#define ID_CLEARMENUVARS			3
#define ID_CLICKWAIT				4
#define ID_LOCATEANIM				5
#define ID_UPDATEANIM				6
#define ID_SYNCSCHEDULES			7
#define ID_CHECKSCHEDULES			8
#define ID_ENSUREVISIBLEICON		9


// Actual id is 30 <= x < 30+Nshortcuts
#define ID_SHORTCUTACTIONTIMER		30

#define ID_UPDATESTATUS				1
#define ID_IDLEEXIT					2
#define ID_CHECKFOREGROUNDWND		3




// Include other files
#include "Debugging.h"

#include "messages.h"
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

#include "shortcuts.inl"
#include "LocatedItem.inl"
#include "LocateDlg.inl"
#include "SettingsDlg.inl"


#define COLUMNSNUM					5
#define DEL_IMAGE					((CLocateApp*)GetApp())->m_nDelImage
#define DEF_IMAGE					((CLocateApp*)GetApp())->m_nDefImage
#define DIR_IMAGE					((CLocateApp*)GetApp())->m_nDirImage
#define DRIVE_IMAGE					((CLocateApp*)GetApp())->m_nDriveImage



#define RESULT_INCLUDEDATE			0x1
#define RESULT_INCLUDELABELS		0x2
#define RESULT_INCLUDEDBINFO		0x4
#define RESULT_INCLUDEDESCRIPTION	0x8
#define RESULT_INCLUDESELECTEDITEMS	0x10

#define RESULT_SAVESTATE			0x07

#define SUBMENU_FILEMENU					0
#define SUBMENU_FILEMENUNOITEMS				1
#define SUBMENU_CONTEXTMENUPLAIN			2
#define SUBMENU_EXTRACONTEXTMENUITEMS		3
#define SUBMENU_CONTEXTMENUNOITEMS			4
#define SUBMENU_MULTIDIRSELECTION			5
#define SUBMENU_PRESETSELECTION				6
#define SUBMENU_OPENITEMFORFILEMENU			7

// Special menu in SUBMENU_EXTRACONTEXTMENUITEMS
#define	SUBMENU_SPECIALMENU					2

#define RESULT_ACTIVATESELECTEDITEMS	0x1000



// String copyers
#define sMemCopy(dst,src,len)	CopyMemory(dst,src,len)
#define sMemZero(dst,len)		ZeroMemory(dst,len)
#define sMemSet(dst,val,len)	iMemSet(dst,val,len)
#define sstrlen(str,len)		dstrlen(str,len)

#define sMemCopyW				iMemCopyW
#define sstrlenW				dwstrlen


#endif