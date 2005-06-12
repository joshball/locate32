#ifndef MESSAGES_H
#define MESSAGES_H


#define WM_SYSTEMTRAY				WM_APP+100
#define WM_ANOTHERINSTANCE			WM_APP+101
#define WM_UPDATENEEDEDDETAILTS		WM_APP+102 //wParam is item, lParam is pointer to item
#define WM_REFRESHNOTIFIERHANDLERS	WM_APP+103 
#define WM_GETLOCATEDLG				WM_APP+105 // wParam==0: return HWND, wParam==1: return PTR, wParam==2: return PTR to ST
#define WM_FREEUPDATESTATUSPOINTERS	WM_APP+106
#define WM_EXECUTESHORTCUT			WM_APP+107 // wParam is index to shortcut 
#define WM_RESETSHORTCUTS			WM_APP+108
#define WM_RESULTLISTACTION			WM_APP+109
#define WM_GETSELECTEDITEMPATH		WM_APP+110

#endif