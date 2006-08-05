#if !defined(DEBUGGING_H)
#define DEBUGGING_H

#if _MSC_VER >= 1000
#pragma once
#endif 


#ifdef _DEBUG
extern DEBUGALLOCATORTYPE DebugAlloc;
#define _DEBUG_LOGGING
#endif


// Enabling debug logging

#ifdef _DEBUG_LOGGING


//#define DEBUGMSG_DIALOG
//#define DEBUGMSG_ITEMS
//#define DEBUGMSG_DBCALLBACK
//#define DEBUGMSG_BACKGROUND
//#define DEBUGMSG_FILEOBJECT


#ifdef DEBUGMSG_DIALOGS
#define DlgDebugMessage(a)						DebugMessage(a)
#else
#define DlgDebugMessage(a) 
#endif

#ifdef DEBUGMSG_ITEMS
#define ItemDebugMessage(a)						DebugMessage(a)
#define ItemDebugFormatMessage1(a,b1)			DebugFormatMessage(a,b1)
#define ItemDebugFormatMessage4(a,b1,b2,b3,b4)	DebugFormatMessage(a,b1,b2,b3,b4)
#else
#define ItemDebugMessage(a) 
#define ItemDebugFormatMessage1(a,b1)
#define ItemDebugFormatMessage4(a,b1,b2,b3,b4)
#endif

#ifdef DEBUGMSG_BACKGROUND
#define BkgDebugMessage(a)						DebugMessage(a)
#define BkgDebugNumMessage(a,b)					DebugNumMessage(a,b)
#define BkgDebugFormatMessage(a,b)				DebugFormatMessage(a,b)
#define BkgDebugFormatMessage4(a,b1,b2,b3,b4)	DebugFormatMessage(a,b1,b2,b3,b4)
#else
#define BkgDebugMessage(a)
#define BkgDebugNumMessage(a,b)
#define BkgDebugFormatMessage(a,b) 
#define BkgDebugFormatMessage4(a,b1,b2,b3,b4)
#endif

#ifdef DEBUGMSG_DBCALLBACK
#define DbcDebugMessage(a)						DebugMessage(a)
#define DbcDebugFormatMessage2(a,b1,b2)			DebugFormatMessage(a,b1,b2)
#else
#define DbcDebugMessage(a) 
#define DbcDebugFormatMessage2(a,b1,b2)
#endif


#ifdef DEBUGMSG_FILEOBJECT
#define FoDebugMessage(a)						DebugMessage(a)
#define FoDebugFormatMessage2(a,b1,b2)			DebugFormatMessage(a,b1,b2)
#define FoDebugFormatMessage4(a,b1,b2,b3,b4)	DebugFormatMessage(a,b1,b2,b3,b4)
#else
#define FoDebugMessage(a) 
#define FoDebugFormatMessage2(a,b1,b2)
#define FoDebugFormatMessage4(a,b1,b2,b3,b4)
#endif

#else

// Release version
#define DlgDebugMessage(a)

#define ItemDebugMessage(a)
#define ItemDebugFormatMessage4

#define BkgDebugMessage(a)
#define BkgDebugFormatMessage(a,b)
#define BkgDebugFormatMessage4(a,b1,b2,b3,b4)
#define BkgDebugNumMessage(a,b)

#define DbcDebugMessage(a)
#define DbcDebugFormatMessage2(a,b1,b2)

#define FoDebugMessage(a) 
#define FoDebugFormatMessage2(a,b1,b2)
#define FoDebugFormatMessage4(a,b1,b2,b3,b4)
#endif

#endif