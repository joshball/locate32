/* Keyhook handler for Locate
Copyright (C) 2004-2005 Janne Huttunen				*/

#if !defined(KEYHOOK_H)
#define KEYHOOK_H

#if _MSC_VER >= 1000
#pragma once
#endif 

#ifdef KEYHOOK_EXPORTS
#define KEYHOOK_API __declspec(dllexport)
#else
#define KEYHOOK_API DECLSPEC_IMPORT
#endif

extern "C" {
	KEYHOOK_API HHOOK SetHook(HWND hTargetWndHHOOK,PSHORTCUT* pShortcuts,DWORD nShortcuts);
	KEYHOOK_API BOOL UnsetHook(HHOOK hHook);


	KEYHOOK_API LRESULT CALLBACK HookKeyboardProc(int code,WPARAM wParam,LPARAM lParam);
}


// Messages
#define WM_EXECUTESHORTCUT			WM_APP+107 // wParam is index to shortcut 
#define WM_GETLOCATEDLG				WM_APP+105 // wParam==0: return HWND, wParam==1: return PTR, wParam==2: return PTR to ST



#endif