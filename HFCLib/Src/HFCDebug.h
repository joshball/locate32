////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
////////////////////////////////////////////////////////////////////


#ifndef HFCDEBUG_H
#define HFCDEBUG_H



#ifdef _DEBUG
void MsgToText(DWORD msg,LPSTR text);

void AddDebugMenuItems(HWND hWnd);
void AddDebugNote(HWND hWndParent);
void ViewAllocators(HWND hWndParent);
void ViewDebugLog(HWND hWndParent);
void Assert(BOOL bIsOK,int line,char* file);

#define ASSERT(_a)			Assert(_a,__LINE__,__FILE__);
#define ASSERT_VALID(_a)	Assert(_a!=NULL && _a!=(void*)-1,__LINE__,__FILE__);

#define _DEBUG_LOGGING

#define DEBUGVIRTUAL	virtual
#define DEBUGINLINE		inline
#define NDEBUGINLINE

#else

#define MsgToText(_a,_b)
#define AddDebugMenuItems(_a)
#define AddDebugNote(_a)
#define ViewAllocators(_a)
#define ViewDebugLog(_a)

#define ASSERT(_a)
#define ASSERT_VALID(_a)

#define DEBUGVIRTUAL
#define DEBUGINLINE
#define NDEBUGINLINE	inline

#endif


#ifdef _DEBUG_LOGGING
void StartDebugLogging();
void DebugMessage(LPCSTR msg);
void DebugWndMessage(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
void DebugNumMessage(LPCSTR text,DWORD num);
void DebugFormatMessage(LPCSTR text,...);
void EndDebugLogging();
LPCSTR GetDebugLoggingFile();
#else
#define StartDebugLogging()
#define DebugMessage(_a)
#define DebugWndMessage(_a,_b,_c,_d)
#define DebugNumMessage(_a,_b)
#define DebugFormatMessage
#define EndDebugLogging()
#endif

#endif
