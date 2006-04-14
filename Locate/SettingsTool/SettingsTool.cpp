
#include <windows.h>
#include "resource.h"

#define MAX_LOADSTRING 100


HINSTANCE hInst;

INT_PTR CALLBACK	DialogProc(HWND, UINT, WPARAM, LPARAM);


int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	hInst=hInstance;
	DialogBox(hInstance,MAKEINTRESOURCE(IDD_MAIN),NULL,DialogProc);

	return (int) 0;
}

BOOL SaveSettings(HWND hWnd)
{
	char szPath[MAX_PATH]="";
	OPENFILENAME ofn;
	ZeroMemory(&ofn,sizeof(OPENFILENAME));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=hWnd;
	ofn.hInstance=hInst;
	ofn.lpstrFilter="Registry files\0*.reg\0\0";
	ofn.lpstrFile=szPath;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrTitle="Save settings";
	ofn.Flags=OFN_ENABLESIZING|OFN_EXPLORER|OFN_LONGNAMES|OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt="*.reg";

	if (!GetSaveFileName(&ofn))
		return FALSE;
	
	HKEY hRegKey=RegOpenKey(
	return TRUE;
}

BOOL RestoreSettings(HWND hWnd)
{
	
	char szPath[MAX_PATH]="";
	OPENFILENAME ofn;
	ZeroMemory(&ofn,sizeof(OPENFILENAME));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=hWnd;
	ofn.hInstance=hInst;
	ofn.lpstrFilter="Registry files\0*.reg\0All Files\0*\0\0";
	ofn.lpstrFile=szPath;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrTitle="Restore settings";
	ofn.Flags=OFN_ENABLESIZING|OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_LONGNAMES;
	ofn.lpstrDefExt="*.reg";

	if (!GetOpenFileName(&ofn))
		return FALSE;
	



	return TRUE;
}

void RemoveSettings(HWND hWnd)
{
}

INT_PTR CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
			HICON hIcon=(HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_SETTINGSTOOL),IMAGE_ICON,32,32,LR_SHARED);
			//SetIcon(hWnd,hIcon,TRUE);
			SetClassLong(hWnd,GCL_HICON,(LONG)hIcon);
			hIcon=(HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_SETTINGSTOOL),IMAGE_ICON,16,16,LR_SHARED);
			//SetIcon(hWnd,hIcon,FALSE);
			SetClassLong(hWnd,GCLP_HICONSM,(LONG)hIcon);
			break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SAVESETTINGS:
			SaveSettings(hWnd);
			break;
		case IDC_RESTORESETTINGS:
			RestoreSettings(hWnd);
			break;
		case IDC_REMOVESETTINGS:
			RemoveSettings(hWnd);
			break;
		default:
			return DefDlgProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CLOSE:
		EndDialog(hWnd,0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	//default:
		//return DefDlgProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
