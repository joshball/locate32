#include <HFCLib.h>
#include "Locate32.h"

#include <Winternl.h>

BOOL CAboutDlg::OnCommand(WORD wID, WORD wNotifyCode, HWND hControl)
{
	switch(wID)
	{
	case IDC_OK:
		{
			EndDialog(0);
			break;
		}
	case IDC_MAILME:
		{
			CWaitCursor wait;
			ShellExecute(*this,NULL,"http://venda.uku.fi/~jmhuttun/soft/bugreport",
				NULL,NULL,0);
			break;
		}
	case IDC_GOTOHOMEPAGE:
		{
			CWaitCursor wait;
			ShellExecute(*this,NULL,"http://locate32.webhop.org", //http://venda.uku.fi/~jmhuttun/soft",
				NULL,NULL,0);
			break;
		}
	}
	return FALSE;
}

BOOL CAboutDlg::OnInitDialog(HWND hwndFocus)
{
	CDialog::OnInitDialog(hwndFocus);
	CWaitCursor wait;
	OSVERSIONINFO ver;
	MEMORYSTATUS mem;
	CString text,text2;

	// Setting banner
	SendDlgItemMessage(IDC_ABOUTBANNER,STM_SETIMAGE,IMAGE_BITMAP,
		(LPARAM)LoadImage(IDB_ABOUTBANNER,IMAGE_BITMAP,0,0,LR_SHARED|LR_DEFAULTSIZE));
	
	// Creating copyright and version strings
	{
		CStringW str;
#ifdef _DEBUG
		str.Format(L"%s � 1997-2006 Janne Huttunen\nTHIS IS DEBUG VERSION, %s %s",
			(LPCWSTR)ID2W(IDS_COPYRIGHT),(LPCWSTR)A2W(__DATE__),(LPCWSTR)A2W(__TIME__));
#else
		str.Format(L"%s � 1997-2006 Janne Huttunen",(LPCWSTR)ID2W(IDS_COPYRIGHT));
#endif
		SetDlgItemText(IDC_COPYRIGHT,str);

		if (IsUnicodeSystem())
		{
			CStringW sExeName=GetApp()->GetExeNameW();
			UINT iDataLength=GetFileVersionInfoSizeW(sExeName,NULL);
			BYTE* pData=new BYTE[iDataLength];
			GetFileVersionInfoW(sExeName,NULL,iDataLength,pData);
			VOID* pTranslations,* pProductVersion=NULL;
			VerQueryValueW(pData,L"VarFileInfo\\Translation",&pTranslations,&iDataLength);
			WCHAR szTranslation[100];
			StringCbPrintfW(szTranslation,100*sizeof(WCHAR),L"\\StringFileInfo\\%4X%4X\\ProductVersion",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1]);
			for (int i=0;szTranslation[i]!='\0';i++)
			{
				if (szTranslation[i]==' ')
					szTranslation[i]='0';
			}
			if (!VerQueryValueW(pData,szTranslation,&pProductVersion,&iDataLength))
				VerQueryValueW(pData,L"\\StringFileInfo\\040904b0\\ProductVersion",&pProductVersion,&iDataLength);
			
			if (pProductVersion!=NULL)
				SetDlgItemText(IDC_VERSION,CStringW(IDS_VERSION)+LPCWSTR(pProductVersion));
		}
		else
		{
			UINT iDataLength=GetFileVersionInfoSize(GetApp()->GetExeName(),NULL);
			BYTE* pData=new BYTE[iDataLength];
			GetFileVersionInfo(GetApp()->GetExeName(),NULL,iDataLength,pData);
			VOID* pTranslations,* pProductVersion=NULL;
			VerQueryValue(pData,"VarFileInfo\\Translation",&pTranslations,&iDataLength);
			char szTranslation[100];
			StringCbPrintf(szTranslation,100,"\\StringFileInfo\\%4X%4X\\ProductVersion",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1]);
			for (int i=0;szTranslation[i]!='\0';i++)
			{
				if (szTranslation[i]==' ')
					szTranslation[i]='0';
			}
			if (!VerQueryValue(pData,szTranslation,&pProductVersion,&iDataLength))
				VerQueryValue(pData,"\\StringFileInfo\\040904b0\\ProductVersion",&pProductVersion,&iDataLength);
			
			if (pProductVersion!=NULL)
				SetDlgItemText(IDC_VERSION,CString(IDS_VERSION)+LPCSTR(pProductVersion));
		}
		

	}
	
	ver.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	if(GetVersionEx(&ver))
	{
		switch(ver.dwPlatformId)
		{
		case VER_PLATFORM_WIN32s:
			text2.LoadString(IDS_SYSWIN32S);
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			{
				if (ver.dwMajorVersion>=4 && ver.dwMinorVersion>=90)
					text.Format(IDS_SYSWINME,ver.dwMajorVersion,ver.dwMinorVersion,(WORD)ver.dwBuildNumber);
				else if (ver.dwMajorVersion==4 && ver.dwMinorVersion>=10)
					text.Format(IDS_SYSWIN98,ver.dwMajorVersion,ver.dwMinorVersion,(WORD)ver.dwBuildNumber);
				else
					text.Format(IDS_SYSWIN95,ver.dwMajorVersion,ver.dwMinorVersion,(WORD)ver.dwBuildNumber);
				text << ver.szCSDVersion;
				break;
			}
		case VER_PLATFORM_WIN32_NT:
			{
				if (ver.dwMajorVersion>=5 && ver.dwMinorVersion>=1)
				{
					OSVERSIONINFOEX osx;
					osx.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
					GetVersionEx((OSVERSIONINFO*)&osx);
					text.Format(IDS_SYSWINXP,osx.dwMajorVersion,osx.dwMinorVersion,osx.dwBuildNumber);
					text << ' ' << osx.szCSDVersion;
				}
				else if (ver.dwMajorVersion>=5)
				{
					OSVERSIONINFOEX osx;
					osx.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
					GetVersionEx((OSVERSIONINFO*)&osx);
					if (osx.wProductType==VER_NT_SERVER && osx.wSuiteMask&VER_SUITE_ENTERPRISE)
						text.Format(IDS_SYSWIN2000ENTEPRISE,osx.dwMajorVersion,osx.dwMinorVersion,osx.dwBuildNumber);
					else if (osx.wProductType==VER_NT_SERVER)
						text.Format(IDS_SYSWIN2000SERVER,osx.dwMajorVersion,osx.dwMinorVersion,osx.dwBuildNumber);
					else
						text.Format(IDS_SYSWIN2000WORKSTATION,osx.dwMajorVersion,osx.dwMinorVersion,osx.dwBuildNumber);
					text << ' ' << osx.szCSDVersion;
				}
				else
				{
					text.Format(IDS_SYSWINNT,ver.dwMajorVersion,ver.dwMinorVersion,ver.dwBuildNumber);
					text << ver.szCSDVersion;
				}
				break;
			}
		}
	}
	mem.dwLength=sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&mem);
	text2.Format(IDS_SYSPHYSMEM,mem.dwTotalPhys>>10,mem.dwAvailPhys>>10);
	text<<text2;
	text2.Format(IDS_SYSPAGEDMEM,mem.dwTotalPageFile>>10,mem.dwAvailPageFile>>10);
	text<<text2;
	SetDlgItemText(IDC_SYSINFO,text);
	SetIcon(NULL,TRUE);
	SetIcon(NULL,FALSE);
	return FALSE;
}

void CAboutDlg::OnDrawItem(UINT idCtl,LPDRAWITEMSTRUCT lpdis)
{
	CDialog::OnDrawItem(idCtl,lpdis);
	CDC dc(lpdis->hDC);
	CFont font;
	TEXTMETRIC tm;
	char szFace[100];
	dc.GetTextMetrics(&tm);
	dc.GetTextFace(100,szFace);
	font.CreateFont(tm.tmHeight,0,0,0,
		tm.tmWeight,tm.tmItalic,1,tm.tmStruckOut,
		tm.tmCharSet,OUT_CHARACTER_PRECIS,
		CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
		tm.tmPitchAndFamily,szFace);
	HFONT hOldFont=(HFONT)dc.SelectObject(font);
	CString text;
	GetDlgItemText(idCtl,text);
	dc.SetTextColor(RGB(0,0,255));
	dc.DrawText(text,&(lpdis->rcItem),DT_LEFT|DT_SINGLELINE|DT_VCENTER);
	dc.SelectObject(hOldFont);
	font.DeleteObject();
}

BOOL CAboutDlg::WindowProc(UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		{
			CRect rect;
			CBrush br;
			GetClientRect(&rect);

			br.CreateSolidBrush(RGB(252,248,240));
			
			FillRect((HDC)wParam,&CRect(0,68,rect.right,rect.bottom),GetSysColorBrush(COLOR_3DFACE));
			FillRect((HDC)wParam,&CRect(0,0,68,rect.bottom),br);
			return 1;
		}
	}
	return CDialog::WindowProc(msg,wParam,lParam);
}

