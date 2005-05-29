/* Keyhook handler for Locate
Copyright (C) 2004-2005 Janne Huttunen				*/

#include <windows.h>

#include <hfcdef.h>
#include <hfcarray.h>

#include "../lan_resources.h"
#include "../locate32/shortcut.h"
#include "keyhelper.h"

#include "../locate32/shortcuts.inl"


//#pragma data_seg(".SHRDATA")
// Shared variables here
//#pragma data_seg()
//#pragma comment(linker, "/section:.SHRDATA,rws")

HINSTANCE g_hDllInstance=NULL;
HHOOK g_hHook=NULL;
int g_nHookCounter=0;
HWND g_hTargetWindow=NULL;

PSHORTCUT* g_pShortcuts=NULL;
DWORD g_nShortcuts=0;

BOOL isstrssame(const char* s1,const char* s2)
{
	for (int i=0;s1[i]==s2[i];i++)
	{
		if (s1[i]=='\0')
			return TRUE;
	}
	return FALSE;
}


HHOOK SetHook(HWND hTargetWindow,PSHORTCUT* pShortcuts,DWORD nShortcuts)
{
	g_pShortcuts=pShortcuts;
	g_nShortcuts=nShortcuts;
    
	if (g_nHookCounter==0)
	{
		g_hHook=SetWindowsHookEx(WH_KEYBOARD_LL,HookKeyboardProc,g_hDllInstance,0);
		g_hTargetWindow=hTargetWindow;
	}


	if (g_hHook!=NULL)
		g_nHookCounter++;

	// Reset sfKeyCurrentlyDown flags
	for (DWORD i=0;i<g_nShortcuts;i++)
		g_pShortcuts[i]->m_dwFlags&=~CShortcut::sfKeyCurrentlyDown;

	return g_hHook;
}

BOOL UnsetHook(HHOOK hHook)
{
	if (hHook!=g_hHook && hHook!=NULL)
		return UnhookWindowsHookEx(hHook);

	g_nHookCounter--;

	if (g_nHookCounter<=0)
	{
		if (!UnhookWindowsHookEx(g_hHook))
			return FALSE;

		g_hHook=NULL;
		g_hTargetWindow=NULL;
	}

	g_pShortcuts=NULL;
	g_nShortcuts=NULL;
	return TRUE;
}

	

LRESULT CALLBACK HookKeyboardProc(int code,WPARAM wParam,LPARAM lParam)
{
	static BOOL bKeyDown=FALSE;
	BOOL bKeyUp=FALSE;
	if (code == HC_ACTION) 
	{
		switch (wParam) 
		{
		case WM_KEYUP:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:  
		case WM_SYSKEYDOWN:
			{
				PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) lParam;

				for (DWORD i=0;i<g_nShortcuts;i++)
				{
					if ((g_pShortcuts[i]->m_dwFlags&CShortcut::sfKeyTypeMask)!=CShortcut::sfGlobalHook)
						continue;
					
					// Checking code
					if (g_pShortcuts[i]->m_dwFlags&CShortcut::sfVirtualKeyIsScancode)
					{
						if (g_pShortcuts[i]->m_bVirtualKey!=p->scanCode)
							continue;
					}
					else
					{
						if (g_pShortcuts[i]->m_bVirtualKey!=p->vkCode)
							continue;
					}
					
					// Checking modifiers
					if (!g_pShortcuts[i]->IsModifiersOk(p->flags&LLKHF_ALTDOWN,
						GetKeyState(VK_CONTROL) & 0x8000,
						GetKeyState(VK_SHIFT) & 0x8000,
						(GetKeyState(VK_LWIN)|GetKeyState(VK_RWIN)) & 0x8000))
						continue;
					
					if (!g_pShortcuts[i]->IsForegroundWindowOk(g_hTargetWindow))
						continue;

                    // Event is confirmed, should do something
					if (wParam==WM_KEYUP || wParam==WM_SYSKEYUP)
					{
						// Key up
						if (!(g_pShortcuts[i]->m_dwFlags&CShortcut::sfKeyCurrentlyDown))
                            continue;

						g_pShortcuts[i]->m_dwFlags&=~CShortcut::sfKeyCurrentlyDown;

						
						if ((g_pShortcuts[i]->m_dwFlags&CShortcut::sfExecuteMask)==CShortcut::sfExecuteWhenUp)
							PostMessage(g_hTargetWindow,WM_EXECUTESHORTCUT,i,0);

						if (g_pShortcuts[i]->m_dwFlags&CShortcut::sfRemoveKeyDownMessage)
							return 1;
					}
					else
					{
						// Key down
						g_pShortcuts[i]->m_dwFlags|=CShortcut::sfKeyCurrentlyDown;
						
						if ((g_pShortcuts[i]->m_dwFlags&CShortcut::sfExecuteMask)==CShortcut::sfExecuteWhenDown)
							PostMessage(g_hTargetWindow,WM_EXECUTESHORTCUT,i,0);

						if (g_pShortcuts[i]->m_dwFlags&CShortcut::sfRemoveKeyUpMessage)
							return 1;
					}
				}

				break;
			}
			
		}
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		g_hDllInstance=hinstDLL;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (g_hHook!=NULL)
			UnsetHook(g_hHook);
		break;

	};
	return TRUE;
}
