////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2004 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

#if defined(_DEBUG)
#include <crtdbg.h>
#endif


#ifdef _DEBUG_LOGGING

#define LOGGING_NONE			1
#define LOGGING_FILE			2

static HANDLE hLogFile=NULL;
#ifdef WIN32
static char* pLogFile=NULL;
#endif
static DWORD nLoggingType=LOGGING_FILE;

#ifdef WIN32
LPCSTR GetDebugLoggingFile()
{
	return pLogFile;
}
#endif

void StartDebugLogging()
{
#ifdef WIN32
	CRegKey RegKey;
	CString File("HFCDebug.log");
	if (RegKey.OpenKey(HKCU,"Software\\HFCLib\\Debuging",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
	{
		RegKey.QueryValue("Type",(LPSTR)&nLoggingType,4);
		RegKey.QueryValue("File",File);
		RegKey.CloseKey();
	}
	if (nLoggingType==LOGGING_FILE)
	{
		if (hLogFile!=NULL)
			DebugMessage("HFCLIB: StartDebugLogging(): Logging is already running");
		else
		{
			char szFullPath[MAX_PATH],*temp;
			GetFullPathName(File,MAX_PATH,szFullPath,&temp);
			pLogFile=alloccopy(szFullPath);
			CharLower(pLogFile);
			
			hLogFile=CreateFile(File,GENERIC_WRITE|GENERIC_READ,
				FILE_SHARE_READ|FILE_SHARE_WRITE,
				NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			if (hLogFile==INVALID_HANDLE_VALUE)
			{
				MessageBox(NULL,"Cannot create HFCDEBUG.LOG file. Logging disabled","LOGGING FAILED",MB_OK|MB_ICONSTOP);
				nLoggingType=LOGGING_NONE;
				return;
			}
			SetFilePointer(hLogFile,0,NULL,FILE_END);
			char Text[400];
			DWORD verMS=GetHFCLibVerMS();
			DWORD verLS=GetHFCLibVerLS();
			DWORD verFlags=GetHFCLibFlags();
			sprintf(Text,"HFCLIB: LOGGING STARTED. HFCLIB Ver: %d.%d.%d.%d  %s %s %s",
				HIWORD(verMS),LOWORD(verMS),HIWORD(verLS),LOWORD(verLS),(verFlags&HFCFLAG_DEBUG?"DEBUG":"RELEASE"),(verFlags&HFCFLAG_DLL?"DLL":"STATIC"),(verFlags&HFCFLAG_WIN32?"WIN32":"DOS")); 
			DebugMessage(Text);
			
			CAppData* pAppData=GetAppData();
#ifdef DEF_APP
			if (pAppData!=NULL)
			{
				if (pAppData->pAppClass!=NULL)
				{
					sprintf(Text,"APP: %s EXE: %s",
						LPCSTR(pAppData->pAppClass->GetAppName()),
						LPCSTR(pAppData->pAppClass->GetExeName())); 
					DebugMessage(Text);
				}
			}
#endif	
		}
	}
#else
	if (nLoggingType!=LOGGING_NONE)
	{
		if (hLogFile!=NULL)
			DebugMessage("HFCLIB: TRY TO START DEGUG LOGGIN SECOND TIME.");
		else
		{
			hLogFile=CreateFile("HFCDEBUG.LOG",GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,
				NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			if (hLogFile==INVALID_HANDLE_VALUE)
			{
				MessageBox(NULL,"Cannot create HFCDEBUG.LOG file. Logging disabled","LOGGING FAILED",MB_OK|MB_ICONSTOP);
				nLoggingType=LOGGING_NONE;
				return;
			}
			SetFilePointer(hLogFile,0,NULL,FILE_END);
			char Text[400];
			DWORD verMS=GetHFCLibVerMS();
			DWORD verLS=GetHFCLibVerLS();
			DWORD verFlags=GetHFCLibFlags();
			sprintf(Text,"HFCLIB: LOGGING STARTED. HFCLIB Ver: %d.%d.%d.%d  %s %s %s",
				HIWORD(verMS),LOWORD(verMS),HIWORD(verLS),HIWORD(verLS),(verFlags&HFCFLAG_DEBUG)?"DEBUG":"RELEASE"),(verFlags&HFCFLAG_DLL?"DLL":"STATIC"),(verFlags&HFCFLAG_WIN32?"WIN32":"DOS")); 
			DebugMessage(Text);
		}
	}
#endif
}

void DebugMessage(LPCSTR msg)
{
#ifdef WIN32
	if (nLoggingType==LOGGING_FILE)
	{
		if (hLogFile==NULL)
		{
			StartDebugLogging();
			if (hLogFile==NULL)
				return;
		}
		char szBufr[2000];
		SYSTEMTIME st;
		DWORD writed;
		GetLocalTime(&st);
		int len=wsprintf(szBufr,"%02d%02d%02d %02d:%02d:%02d.%03d TID=%4X: %s\r\n",
			st.wYear%100,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,
			st.wMilliseconds,GetCurrentThreadId(),msg);
		WriteFile(hLogFile,szBufr,len,&writed,NULL);
		FlushFileBuffers(hLogFile);
	}
#else
	if (nLoggingType==LOGGING_FILE)
	{
		if (hLogFile==NULL)
		{
			StartDebugLogging();
			if (hLogFile==NULL)
				return;
		}
		char szBufr[2000];
		time_t ct=time(NULL);
		struct tm* Time=localtime(&ct);
		int len=sprintf(szBufr,"DATE %4d.%2d.%2d TIME: %2d.%2d.%2d.%3d MSG: %s\r\n",
			Time->tm_year+1900,Time->tm_mon+1,Time->tm_mday,Time->tm_hour,Time->tm_min,Time->tm_sec,0,msg);
		fwrite(szBufr,1,len,(FILE*)hLogFile);
		fflush((FILE*)hLogFile);
	}
#endif
}

#ifdef WIN32
void DebugWndMessage(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if (nLoggingType==LOGGING_FILE)
	{
		char buf[2000];
		char buf2[100];
		MsgToText(msg,buf2);
		sprintf(buf,"HWND: %lX MSG: %s WPARAM: %lX LPARAM: %lX",(DWORD)hWnd,buf2,(DWORD)wParam,(DWORD)lParam);
		DebugMessage(buf);
	}
}
#endif


void DebugNumMessage(LPCSTR text,DWORD num)
{
	if (nLoggingType==LOGGING_FILE)
	{
		char buf[2000];
		sprintf(buf,text,num);
		DebugMessage(buf);
	}
}

void DebugFormatMessage(LPCSTR text,...)
{
	if (nLoggingType==LOGGING_FILE)
	{
		va_list argList;
		va_start(argList,text);
		
		char buf[2000];
		vsprintf(buf,text,argList);
		DebugMessage(buf);
	}
}

void EndDebugLogging()
{
#ifdef WIN32
	if (nLoggingType==LOGGING_FILE)
	{
		DebugMessage("HFCLIB: LOGGING STOPPED");
		CloseHandle(hLogFile);
		if (pLogFile!=NULL)
		{
			delete[] pLogFile;
			pLogFile=NULL;
		}
		hLogFile=NULL;

		nLoggingType=LOGGING_NONE;
	}
#else
	if (nLoggingType!=LOGGING_NONE)
	{
		DebugMessage("HFCLIB: LOGGING STOPPED");
		fclose((FILE*)hLogFile);
		hLogFile=NULL;
	}
#endif
}

#endif

#ifdef _DEBUG

#ifdef DEF_WINDOWS
void AddDebugMenuItems(HWND hWnd)
{
	HMENU hMenu=GetSystemMenu(hWnd,FALSE);
	MENUITEMINFO mii;
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_TYPE|MIIM_SUBMENU;
	mii.fType=MFT_STRING;
	mii.hSubMenu=CreatePopupMenu();
	mii.dwTypeData="Debug";
	InsertMenuItem(hMenu,0,TRUE,&mii);

	hMenu=mii.hSubMenu;
	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_ID|MIIM_TYPE;
	mii.fType=MFT_STRING;
	
	mii.wID=0x62;
	mii.dwTypeData="Show HFCLib Version";
	InsertMenuItem(hMenu,0,TRUE,&mii);

	mii.fMask=MIIM_TYPE;
	mii.fType=MFT_SEPARATOR;
	mii.dwTypeData="""";
	InsertMenuItem(hMenu,1,TRUE,&mii);
	
	mii.fMask=MIIM_ID|MIIM_TYPE;
	mii.fType=MFT_STRING;
	mii.wID=0x61;
	mii.dwTypeData="View Allocators";
	InsertMenuItem(hMenu,3,TRUE,&mii);
	mii.wID=0x60;
	mii.dwTypeData="Add Debug Note";
	InsertMenuItem(hMenu,2,TRUE,&mii);
	
	
	mii.fMask=MIIM_TYPE;
	mii.fType=MFT_SEPARATOR;
	mii.dwTypeData="""";
	InsertMenuItem(hMenu,4,TRUE,&mii);
	
	mii.fMask=MIIM_ID|MIIM_TYPE;
	mii.fType=MFT_STRING;
	mii.dwTypeData="View Debug Log";
	mii.wID=0x63;
	InsertMenuItem(hMenu,5,TRUE,&mii);
	mii.dwTypeData="Start Debug Logging";
	mii.wID=0x64;
	InsertMenuItem(hMenu,6,TRUE,&mii);
	mii.dwTypeData="Stop Debug Logging";
	mii.wID=0x65;
	InsertMenuItem(hMenu,7,TRUE,&mii);
	
	mii.fMask=MIIM_TYPE;
	mii.fType=MFT_SEPARATOR;
	mii.dwTypeData="""";
	InsertMenuItem(hMenu,8,TRUE,&mii);
	
	mii.fMask=MIIM_TYPE|MIIM_SUBMENU;
	mii.fType=MFT_STRING;
	mii.hSubMenu=CreatePopupMenu();
	mii.dwTypeData="Post message";
	InsertMenuItem(hMenu,9,TRUE,&mii);
	
	mii.fMask=MIIM_ID|MIIM_TYPE;
	mii.fType=MFT_STRING;
	mii.wID=0x70;
	mii.dwTypeData="WM_QUIT";
	InsertMenuItem(mii.hSubMenu,0,TRUE,&mii);
	mii.wID=0x71;
	mii.dwTypeData="WM_DESTROY";
	InsertMenuItem(mii.hSubMenu,1,TRUE,&mii);
	mii.wID=0x72;
	mii.dwTypeData="WM_CLOSE";
	InsertMenuItem(mii.hSubMenu,2,TRUE,&mii);
	mii.wID=0x73;
	mii.dwTypeData="WM_USERCHANGE";
	InsertMenuItem(mii.hSubMenu,3,TRUE,&mii);
	mii.wID=0x74;
	mii.dwTypeData="WM_WININICHANGE";
	InsertMenuItem(mii.hSubMenu,4,TRUE,&mii);

}

void DebugCommandsProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if (msg==WM_SYSCOMMAND)
	{
		switch (wParam)
		{
		case 0x60:
			AddDebugNote(hWnd);
			break;
		case 0x61:
			ViewAllocators(hWnd);
			break;
		case 0x62:
			MessageBox(hWnd,GetHFCLibStr(),"HFC Library version.",MB_ICONINFORMATION|MB_OK);
			break;
		case 0x63:
			ViewDebugLog(hWnd);
			break;
		case 0x64:
			StartDebugLogging();
			break;
		case 0x65:
			EndDebugLogging();
			break;
		case 0x70:
			PostQuitMessage(1);
			break;
		case 0x71:
			PostMessage(hWnd,WM_DESTROY,0,0);
			break;
		case 0x72:
			PostMessage(hWnd,WM_CLOSE,0,0);
			break;
		case 0x73:
			PostMessage(hWnd,WM_USERCHANGED,0,0);
			break;
		case 0x74:
			PostMessage(hWnd,WM_WININICHANGE,0,0);
			break;
		}
	}
}

LRESULT CALLBACK AddDebugNoteWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		{
			RECT rect;
			GetClientRect(hWnd,&rect);
			HWND hCtrl=CreateWindow("EDIT",szEmpty,WS_VISIBLE|WS_CHILD|
				ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_MULTILINE|WS_VSCROLL|WS_HSCROLL,
				0,0,rect.right,rect.bottom-20,hWnd,(HMENU)100,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			hCtrl=CreateWindow("BUTTON","Add To &Note Log",WS_VISIBLE|WS_CHILD|
				BS_DEFPUSHBUTTON|BS_TEXT,0,rect.bottom-20,rect.right/3,20,
				hWnd,(HMENU)101,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			hCtrl=CreateWindow("BUTTON","Add To &Debug Log",WS_VISIBLE|WS_CHILD|
				BS_PUSHBUTTON|BS_TEXT,rect.right/3,rect.bottom-20,rect.right/3,20,
				hWnd,(HMENU)102,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			hCtrl=CreateWindow("BUTTON","&Cancel",WS_VISIBLE|WS_CHILD|
				BS_PUSHBUTTON|BS_TEXT,(rect.right/3)*2,rect.bottom-20,rect.right/3,20,
				hWnd,(HMENU)103,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			SetFocus(GetDlgItem(hWnd,100));
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 100:
			break;
		case 101:
			{
				CString File;
				CString str;
				CEdit edit(GetDlgItem(hWnd,100));
				{
					CRegKey RegKey;
					if (RegKey.OpenKey(HKLM,"Software\\HFC\\Notify files",CRegKey::openExist|CRegKey::samRead)==ERROR_SUCCESS)
						RegKey.QueryValue(GetApp()->GetAppName(),File);
				}
				if (!CFile::IsValidFileName(File))
				{
					CFileDialog fd(FALSE,"log",NULL,OFN_EXPLORER|OFN_HIDEREADONLY|OFN_NOREADONLYRETURN|OFN_ENABLESIZING,"Log files|*.log|Text files|*.txt||");
					fd.m_pofn->lpstrTitle="Set notify log file";
					fd.EnableFeatures();

					if (!fd.DoModal(hWnd))
						break;
					File=fd.GetPathName();
					CRegKey RegKey;
					if(RegKey.OpenKey(HKLM,"Software\\HFC\\Notify files",CRegKey::createNew|CRegKey::samAll)==ERROR_SUCCESS)
					{
						CRect rect;
						RegKey.SetValue(GetApp()->GetAppName(),File);
					}
				}
				HANDLE hFile=CreateFile(File,GENERIC_READ|GENERIC_WRITE,
					FILE_SHARE_READ,NULL,OPEN_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,NULL);
				if (hFile==NULL)
					break;
				
				SetFilePointer(hFile,0,NULL,FILE_END);
				DWORD nWrited=0;
				CTime now(CTime::GetCurrentTime());
				str.Format("DEBUG NOTIFY (%s,%s)\r\nBEGIN\r\n",(LPCSTR)GetApp()->GetAppName(),(LPCSTR)now.Format("%x %X"));
				WriteFile(hFile,str,str.GetLength(),&nWrited,NULL);
				if (nWrited<str.GetLength())
				{
					CloseHandle(hFile);
					break;
				}
				edit.GetText(str);
				WriteFile(hFile,str,str.GetLength(),&nWrited,NULL);
				if (nWrited<str.GetLength())
				{
					CloseHandle(hFile);
					break;
				}
				WriteFile(hFile,"\r\nEND\r\n",7,&nWrited,NULL);
				if (nWrited<7)
				{
					CloseHandle(hFile);
					break;
				}
				CloseHandle(hFile);
				DestroyWindow(hWnd);
				break;
			}
		case 102:
			{
				CString str,str2("USER NOTIFY: ");
				CEdit edit(GetDlgItem(hWnd,100));
				edit.GetText(str);
				str2 << str;
				DebugMessage(str2);
				DestroyWindow(hWnd);
				break;
			}
		case 103:
			DestroyWindow(hWnd);
			break;
		}
		break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
 
LRESULT CALLBACK ViewAllocatorsWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	void AddAllocatorsIDtoCombo(HWND hCombo);
	void GetAllocatorString(int iIndex,CString& str);
	
	switch (uMsg)
	{
	case WM_CREATE:
		{
			RECT rect;
			GetClientRect(hWnd,&rect);
			HWND hCtrl=CreateWindow("COMBOBOX",szEmpty,WS_VISIBLE|WS_CHILD|CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP,
				0,0,rect.right-100,100,
				hWnd,(HMENU)100,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);

			AddAllocatorsIDtoCombo(hCtrl);
			SendMessage(hCtrl,CB_SETCURSEL,0,0);
			
			hCtrl=CreateWindow("BUTTON","&Refresh",WS_VISIBLE|WS_CHILD|
				BS_PUSHBUTTON|BS_TEXT,rect.right-100,0,50,19,
				hWnd,(HMENU)101,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			hCtrl=CreateWindow("BUTTON","&Close",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|BS_TEXT,
				rect.right-50,0,50,19,hWnd,(HMENU)102,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			hCtrl=CreateWindow("EDIT",szEmpty,WS_VISIBLE|WS_CHILD|
				ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_MULTILINE|ES_READONLY|WS_VSCROLL|WS_HSCROLL,
				0,20,rect.right,rect.bottom-20,
				hWnd,(HMENU)103,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			CString str;
			GetAllocatorString(SendDlgItemMessage(hWnd,100,CB_GETCURSEL,0,0),str);
			int index=-1;
			while ((index=str.FindNext('\n',index+1))!=-1)
				str.InsChar(index,'\r');
			
			SetDlgItemText(hWnd,103,str);
			SetTimer(hWnd,0,500,NULL);
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		KillTimer(hWnd,0);
		break;
	case WM_SIZE:
		if (wParam!=SIZE_MINIMIZED)
		{
			::SetWindowPos(GetDlgItem(hWnd,100),HWND_TOP,0,0,LOWORD(lParam)-100,100,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
			::SetWindowPos(GetDlgItem(hWnd,101),HWND_TOP,LOWORD(lParam)-100,0,0,0,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
			::SetWindowPos(GetDlgItem(hWnd,102),HWND_TOP,LOWORD(lParam)-50,0,0,0,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
			::SetWindowPos(GetDlgItem(hWnd,103),HWND_TOP,0,0,LOWORD(lParam),HIWORD(lParam)-20,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
		}
		break;
	case WM_TIMER:
		{
			// Updating if necassary
			CString str;
			GetAllocatorString(SendDlgItemMessage(hWnd,100,CB_GETCURSEL,0,0),str);
			int index=-1;
			while ((index=str.FindNext('\n',index+1))!=-1)
				str.InsChar(index,'\r');
			
			DWORD dwLength=SendDlgItemMessage(hWnd,103,WM_GETTEXTLENGTH,0,0);
			if (str.GetLength()!=dwLength)
				SetDlgItemText(hWnd,103,str);
			else
			{
				char* szText=new char[dwLength+1];
				GetDlgItemText(hWnd,103,szText,dwLength+1);
				if (strcmp(szText,str)!=0)
					SetDlgItemText(hWnd,103,str);
				delete[] szText;
			}
			break;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 100:
			if (HIWORD(wParam)!=CBN_SELCHANGE)
				break;
		case 101:
			{
				CString str;
				GetAllocatorString(SendDlgItemMessage(hWnd,100,CB_GETCURSEL,0,0),str);
				int index=-1;
				while ((index=str.FindNext('\n',index+1))!=-1)
					str.InsChar(index,'\r');
				SetDlgItemText(hWnd,103,str);
				break;
			}
		case 102:
			DestroyWindow(hWnd);
			break;
		}
		break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

static void RefreshDebugLogViewer(HWND hWnd)
{
	if (hLogFile==NULL)
	{
		SetDlgItemText(hWnd,106,"Logging: OFF");
		SetDlgItemText(hWnd,107,"Cannot show log file. Logging is off.");
		return;
	}
	else
		SetDlgItemText(hWnd,106,"Logging: ON");

	SetFilePointer(hLogFile,0,NULL,FILE_BEGIN);
	DWORD dwLength=GetFileSize(hLogFile,NULL);
	char* pData=new char[dwLength];
	DWORD dwReaded;
	BOOL bRet=ReadFile(hLogFile,pData,dwLength,&dwReaded,NULL);
	if (!bRet || dwReaded<dwLength)
	{
		delete[] pData;
		SetDlgItemText(hWnd,107,"Cannot read log file");
		return;
	}
	CString str;
	for (DWORD i=0;i<dwLength;i++)
	{
		if (pData[i]=='\n')
			str << '\r';
		str << pData[i];
	}
	delete[] pData;
	SetDlgItemText(hWnd,107,str);

	SendDlgItemMessage(hWnd,107,EM_SETSEL,dwLength-1,-1);
	SendDlgItemMessage(hWnd,107,EM_SCROLLCARET,0,0);
	SendDlgItemMessage(hWnd,107,EM_SETSEL,-1,-1);
}
			
LRESULT CALLBACK ViewDebugLogWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		{
			RECT rect;
			GetClientRect(hWnd,&rect);
			
			HWND hCtrl=CreateWindow("BUTTON","&Start logging",WS_VISIBLE|WS_CHILD|
				BS_PUSHBUTTON|BS_TEXT,0,0,80,19,
				hWnd,(HMENU)101,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			
			hCtrl=CreateWindow("BUTTON","&End logging",WS_VISIBLE|WS_CHILD|
				BS_PUSHBUTTON|BS_TEXT,80,0,80,19,
				hWnd,(HMENU)102,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			
			hCtrl=CreateWindow("BUTTON","&Clean",WS_VISIBLE|WS_CHILD|
				BS_PUSHBUTTON|BS_TEXT,160,0,50,19,
				hWnd,(HMENU)103,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			
			hCtrl=CreateWindow("BUTTON","&Refresh",WS_VISIBLE|WS_CHILD|
				BS_PUSHBUTTON|BS_TEXT,210,0,50,19,
				hWnd,(HMENU)104,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			
			hCtrl=CreateWindow("BUTTON","&Close",WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON|
				BS_TEXT,260,0,50,19,hWnd,(HMENU)105,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			
			hCtrl=CreateWindow("STATIC","",WS_VISIBLE|WS_CHILD|SS_LEFT,
				312,3,1,1,hWnd,(HMENU)106,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			
			hCtrl=CreateWindow("EDIT",szEmpty,WS_VISIBLE|WS_CHILD|
				ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_MULTILINE|ES_READONLY|WS_VSCROLL|WS_HSCROLL,
				0,20,1,1,hWnd,(HMENU)107,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			
			hCtrl=CreateWindow("EDIT",szEmpty,WS_VISIBLE|WS_CHILD|WS_BORDER,
				0,rect.bottom-20,rect.right-40,19,hWnd,(HMENU)108,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);
			
			hCtrl=CreateWindow("BUTTON","&Add",WS_VISIBLE|WS_CHILD|BS_DEFPUSHBUTTON|BS_TEXT,
				0,rect.bottom-20,40,19,hWnd,(HMENU)109,GetInstanceHandle(),0);
			SendMessage(hCtrl,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),1);

			SetFocus(GetDlgItem(hWnd,108));

			
			RefreshDebugLogViewer(hWnd);
			
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		if (wParam!=SIZE_MINIMIZED)
		{
			::SetWindowPos(GetDlgItem(hWnd,106),HWND_TOP,0,0,LOWORD(lParam)-320,15,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
			
			::SetWindowPos(GetDlgItem(hWnd,107),HWND_TOP,0,0,LOWORD(lParam),HIWORD(lParam)-40,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOMOVE);
			
			::SetWindowPos(GetDlgItem(hWnd,108),HWND_TOP,0,HIWORD(lParam)-20,LOWORD(lParam)-42,20,SWP_NOACTIVATE|SWP_NOZORDER);
			::SetWindowPos(GetDlgItem(hWnd,109),HWND_TOP,LOWORD(lParam)-40,HIWORD(lParam)-19,0,0,SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 101:
			StartDebugLogging();
			RefreshDebugLogViewer(hWnd);
			SetFocus(GetDlgItem(hWnd,108));
			break;
		case 102:
			EndDebugLogging();
			RefreshDebugLogViewer(hWnd);
			SetFocus(GetDlgItem(hWnd,108));
			break;
		case 103:
			SetFilePointer(hLogFile,0,NULL,FILE_BEGIN);
			SetEndOfFile(hLogFile);
			RefreshDebugLogViewer(hWnd);
			SetFocus(GetDlgItem(hWnd,108));
			break;
		case 104:
			RefreshDebugLogViewer(hWnd);
			SetFocus(GetDlgItem(hWnd,108));
			break;
		case 105:
			DestroyWindow(hWnd);
			break;
		case 108:
			if (HIWORD(wParam)==EN_SETFOCUS)
				SendDlgItemMessage(hWnd,108,EM_SETSEL,0,-1);
			if (HIWORD(wParam)==EN_CHANGE)
				EnableWindow(GetDlgItem(hWnd,109),SendDlgItemMessage(hWnd,108,WM_GETTEXTLENGTH,0,0)>0);
			break;
		case 109:
			{
				char szMessage[1000];
				GetDlgItemText(hWnd,108,szMessage,1000);
				DebugMessage(szMessage);
				RefreshDebugLogViewer(hWnd);
				SetDlgItemText(hWnd,108,"");
				SetFocus(GetDlgItem(hWnd,108));
				break;
			}
		}
		break;
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

void AddDebugNote(HWND hWndParent)
{
	WNDCLASSEX wc;

    wc.cbSize=sizeof(WNDCLASSEX); 
    wc.style=CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc=AddDebugNoteWndProc; 
    wc.cbClsExtra=0;
    wc.cbWndExtra=0; 
    wc.hInstance=GetInstanceHandle();
    wc.hIcon=NULL;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW); 
    wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName=0;
    wc.lpszClassName="ADDDEBUGNOTEWND"; 
    wc.hIconSm=NULL;
	RegisterClassEx(&wc);

	HWND hWnd=CreateWindow("ADDDEBUGNOTEWND","Add Debug Note",
		WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_DLGFRAME|WS_SYSMENU,
		CW_USEDEFAULT,CW_USEDEFAULT,400,300,
		hWndParent,NULL,GetInstanceHandle(),NULL);
	ShowWindow(hWnd,SW_SHOW);
	UpdateWindow(hWnd);
	
	if (GetCurrentWinThread()==NULL)
	{
		MSG msg;
		while (GetMessage(&msg,NULL,0,0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	else
		GetCurrentWinThread()->ModalLoop();
}

void ViewAllocators(HWND hWndParent)
{
	WNDCLASSEX wc;

    wc.cbSize=sizeof(WNDCLASSEX); 
    wc.style=CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc=ViewAllocatorsWndProc; 
    wc.cbClsExtra=0;
    wc.cbWndExtra=0; 
    wc.hInstance=GetInstanceHandle();
    wc.hIcon=NULL;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW); 
    wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName=0;
    wc.lpszClassName="VIEWALLOCATORSWND"; 
    wc.hIconSm=NULL;
	RegisterClassEx(&wc);

	HWND hWnd=CreateWindow("VIEWALLOCATORSWND","View Allocators",
		WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_THICKFRAME|WS_SYSMENU,
		CW_USEDEFAULT,CW_USEDEFAULT,600,500,
		hWndParent,NULL,GetInstanceHandle(),NULL);
	ShowWindow(hWnd,SW_SHOW);
	UpdateWindow(hWnd);

	if (GetCurrentWinThread()==NULL)
	{
		MSG msg;
		while (GetMessage(&msg,NULL,0,0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	else
		GetCurrentWinThread()->ModalLoop();
}

void ViewDebugLog(HWND hWndParent)
{
	WNDCLASSEX wc;

    wc.cbSize=sizeof(WNDCLASSEX); 
    wc.style=CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc=ViewDebugLogWndProc; 
    wc.cbClsExtra=0;
    wc.cbWndExtra=0; 
    wc.hInstance=GetInstanceHandle();
    wc.hIcon=NULL;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW); 
    wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    wc.lpszMenuName=0;
    wc.lpszClassName="VIEWDEBUGLOGWND"; 
    wc.hIconSm=NULL;
	RegisterClassEx(&wc);

	HWND hWnd=CreateWindow("VIEWDEBUGLOGWND","View Debug Log",
		WS_VISIBLE|WS_OVERLAPPED|WS_CAPTION|WS_THICKFRAME|WS_SYSMENU,
		CW_USEDEFAULT,CW_USEDEFAULT,600,550,
		hWndParent,NULL,GetInstanceHandle(),NULL);
	ShowWindow(hWnd,SW_SHOW);
	UpdateWindow(hWnd);

	if (GetCurrentWinThread()==NULL)
	{
		MSG msg;
		while (GetMessage(&msg,NULL,0,0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	else
		GetCurrentWinThread()->ModalLoop();
}

#endif

void Assert(BOOL bIsOK,int line,char* file)
{
	if (bIsOK)
		return;
#ifdef DEF_WINDOWS
	MSG msg;
	BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
	_CrtDbgReport(_CRT_ASSERT, file, line, NULL, NULL);
	if (bQuit)
		PostQuitMessage(msg.wParam);
#else
	fprintf(stderr,"Assertation failed at %s line %d.",file,line);
#endif
}

#endif
