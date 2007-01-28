////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2007 Janne Huttunen
////////////////////////////////////////////////////////////////////


#ifndef HFCDEBUG_H
#define HFCDEBUG_H



#ifdef _DEBUG
void MsgToText(DWORD msg,LPSTR text,DWORD maxlen);

void AddDebugMenuItems(HWND hWnd);
void AddDebugNote(HWND hWndParent);
void ViewAllocators(HWND hWndParent);
void ViewDebugLog(HWND hWndParent);
void ViewOpenHandles(HWND hWndParent);
void Assert(BOOL bIsOK,int line,char* file);

#define ASSERT(_a)			Assert(_a,__LINE__,__FILE__);
#define ASSERT_VALID(_a)	Assert(_a!=NULL && _a!=(void*)-1,__LINE__,__FILE__);

#define _DEBUG_LOGGING

#define DEBUGVIRTUAL	virtual
#define DEBUGINLINE		inline
#define NDEBUGINLINE

#else

#define MsgToText(_a,_b,_c)
#define AddDebugMenuItems(_a)
#define AddDebugNote(_a)
#define ViewAllocators(_a)
#define ViewDebugLog(_a)
#define ViewOpenHandles(_a)

#define ASSERT(_a)
#define ASSERT_VALID(_a)

#define DEBUGVIRTUAL
#define DEBUGINLINE
#define NDEBUGINLINE	inline

#endif

enum DebugFlags { 
	DebugLogHandleOperations = 0x1
};

enum DebugHandleType {
	dhtMemoryBlock=0,
	dhtFile=1,
	dhtFileFind=2,
	dhtThread=3,
	dhtProcess=4,
	dhtEvent=5,
	dhtGdiObject=6,
	dhtWindow=7,
	dhtRegKey=8,
	dhtMenu=9,
	dhtMutex=10,
	dhtMisc=255
};


#ifdef _DEBUG_LOGGING

void DebugSetFlags(DebugFlags bDebugFlag,BOOL bRemove=FALSE);

void StartDebugLogging();
void DebugMessage(LPCSTR msg);
void DebugHexDump(LPCSTR desc,BYTE* pData,SIZE_T datalen);
void DebugWndMessage(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
void DebugNumMessage(LPCSTR text,DWORD num);
void DebugFormatMessage(LPCSTR text,...);
void EndDebugLogging();
LPCSTR GetDebugLoggingFile();
#ifdef DEF_WCHAR
void DebugMessage(LPCWSTR msg);
void DebugHexDump(LPCWSTR desc,BYTE* pData,SIZE_T datalen);
void DebugNumMessage(LPCWSTR text,DWORD num);
void DebugFormatMessage(LPCWSTR text,...);
#endif




void DebugLogOpenHandles();
void DebugOpenHandle2(DebugHandleType bType,void* pValue,LPCSTR szInfo,int iLine,LPCSTR szFile);
void DebugCloseHandle2(DebugHandleType bType,void* pValue,LPCSTR szInfo,int iLine,LPCSTR szFile);
#ifdef DEF_WCHAR
void DebugOpenHandle2(DebugHandleType bType,void* pValue,LPCWSTR szInfo,int iLine,LPCSTR szFile);
void DebugCloseHandle2(DebugHandleType bType,void* pValue,LPCWSTR szInfo,int iLine,LPCSTR szFile);
#endif

					 
#else
inline void StartDebugLogging() {}
inline void DebugMessage(LPCSTR msg) {}
inline void DebugHexDump(LPCSTR desc,BYTE* pData,SIZE_T datalen) {}
inline void DebugWndMessage(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) {}
inline void DebugNumMessage(LPCSTR text,DWORD num) {}
inline void DebugFormatMessage(LPCSTR text,...)  {}
inline void EndDebugLogging()  {}
#ifdef DEF_WCHAR
inline void DebugMessage(LPCWSTR msg) {}
inline void DebugHexDump(LPCWSTR desc,BYTE* pData,SIZE_T datalen) {}
inline void DebugNumMessage(LPCWSTR text,DWORD num) {}
inline void DebugFormatMessage(LPCWSTR text,...)  {}
#endif

inline void DebugSetFlags(DebugFlags bDebugFlag,BOOL bRemove=FALSE) {}
inline void DebugLogOpenHandles() {}
inline void DebugOpenHandle2(DebugHandleType bType,void* pValue,LPCSTR szInfo,int iLine,LPCSTR szFile) {}
inline void DebugCloseHandle2(DebugHandleType bType,void* pValue,LPCSTR szInfo,int iLine,LPCSTR szFile) {}
#ifdef DEF_WCHAR
inline void DebugOpenHandle2(DebugHandleType bType,void* pValue,LPCWSTR szInfo,int iLine,LPCSTR szFile) {}
inline void DebugCloseHandle2(DebugHandleType bType,void* pValue,LPCWSTR szInfo,int iLine,LPCSTR szFile) {}
#endif

#endif

#define DebugOpenHandle(bType,pValue,szInfo) DebugOpenHandle2(bType,pValue,szInfo,__LINE__,__FILE__);
#define DebugCloseHandle(bType,pValue,szInfo) DebugCloseHandle2(bType,pValue,szInfo,__LINE__,__FILE__);


#endif
