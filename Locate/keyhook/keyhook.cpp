/* Keyhook handler for Locate
Copyright (C) 2004 Janne Huttunen				*/

#include <windows.h>
#include "keyhelper.h"

//#pragma data_seg(".SHRDATA")
// Shared variables here
//#pragma data_seg()
//#pragma comment(linker, "/section:.SHRDATA,rws")

HINSTANCE g_hDllInstance=NULL;
HHOOK g_hHook=NULL;
int g_nHookCounter=0;
HWND g_hTargetWindow=NULL;

BOOL isstrssame(const char* s1,const char* s2)
{
	for (int i=0;s1[i]==s2[i];i++)
	{
		if (s1[i]=='\0')
			return TRUE;
	}
	return FALSE;
}

HHOOK SetHook(HWND hTargetWindow)
{
	if (g_nHookCounter==0)
	{
		g_hHook=SetWindowsHookEx(WH_KEYBOARD_LL,HookKeyboardProc,g_hDllInstance,0);
		g_hTargetWindow=hTargetWindow;
	}


	if (g_hHook!=NULL)
		g_nHookCounter++;
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
				if ((p->vkCode == VK_F3) && ((p->flags & LLKHF_ALTDOWN) == 0))
				{
					// F3
					HWND hWnd=GetForegroundWindow();
	
					if (hWnd!=NULL)
					{
						char szClassName[40];
						GetClassName(hWnd,szClassName,40);
						if (isstrssame(szClassName,"ExploreWClass") ||
							isstrssame(szClassName,"CabinetWClass") ||
							isstrssame(szClassName,"Progman"))
						{
							if (wParam==WM_KEYUP || wParam==WM_SYSKEYUP)
							{
								bKeyDown=FALSE;
								return 1;
							}
							else
							{
								bKeyDown=TRUE;
                                PostMessage(g_hTargetWindow,WM_ANOTHERINSTANCE,0,0);
								return 1;
							}
						}
					}
				}
				else if ((p->vkCode == 'F') && ((GetKeyState(VK_CONTROL) & 0x8000) != 0) )
				{
					// Ctrl+F
					HWND hWnd=GetForegroundWindow();
	
					if (hWnd!=NULL)
					{
						char szClassName[40];
						GetClassName(hWnd,szClassName,40);
						if (isstrssame(szClassName,"ExploreWClass") ||
							isstrssame(szClassName,"CabinetWClass"))
						{
							if (wParam==WM_KEYUP || wParam==WM_SYSKEYUP)
							{
								bKeyDown=FALSE;
								return 1;
							}
							else
							{
								bKeyDown=TRUE;
                                PostMessage(g_hTargetWindow,WM_ANOTHERINSTANCE,0,0);
								return 1;
							}
						}
					}
				}
				else if (p->vkCode == 'F' && (GetKeyState(VK_LWIN)&0x8000 || GetKeyState(VK_RWIN)&0x8000))
				{
					if (wParam==WM_KEYUP || wParam==WM_SYSKEYUP)
					{
						bKeyDown=FALSE;
                        PostMessage(g_hTargetWindow,WM_ANOTHERINSTANCE,0,0);
						//return 1;
						break;
					}
					else
					{
						bKeyDown=TRUE;
						return 1;
					}
				}
				break;
			}
			if (bKeyDown)
			{	
				PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) lParam;
				
				
				//GetLocateAppWnd()->PostMessage(WM_SETFOCUSFORWINDOW);
				if (p->vkCode==VK_LWIN || p->vkCode==VK_RWIN)
				{
					bKeyDown=FALSE;
					//return 1;
				}
			}
			break;
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
