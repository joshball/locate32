////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2005 Janne Huttunen
////////////////////////////////////////////////////////////////////

#include "HFCLib.h"

BOOL SystemInfo(LPSYSTEMINFO info)
{
#ifdef WIN32
	OSVERSIONINFO ver;
	info->is32BIT=TRUE;
	ver.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&ver);
	switch(ver.dwPlatformId)
	{
		case VER_PLATFORM_WIN32s:
			info->System=osWin32S;
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			info->System=osWin95;
			break;
		case VER_PLATFORM_WIN32_NT:
			info->System=osWinNT;
			break;
	}
	info->hiWINVer=ver.dwMajorVersion;
	info->loWINVer=ver.dwMinorVersion;
    info->hiOSVer=0;
    info->loOSVer=0;
	info->hiIEVer=0;
	info->loIEVer=0;
	CRegKey Key;
	if (Key.OpenKey(HKLM,"SOFTWARE\\Microsoft\\Internet Explorer",CRegKey::openExist|CRegKey::samRead)==NOERROR)
	{
		CString Version;
		if (Key.QueryValue("Version",Version))
		{
			int hi=Version.FindFirst('.');
			int lo=Version.FindNext('.',hi);
			info->hiIEVer=atoi(Version.Left(hi));
			info->loIEVer=atoi(Version.Mid(hi,lo));
		}
		return FALSE;
	}	
	return TRUE;
#else
    WORD _ax=0;
    __asm__("
      movw $0x1600,%%ax\n
      int $0x2f\n
      movw %%ax,%0"
      :"=g" (_ax)
      :
      : "memory","ax","bx","cx","dx"
    );
    info->hiWINVer=LOBYTE(_ax);
    info->loWINVer=HIBYTE(_ax);
    __asm__("
      movw $0x3000,%%ax\n
      int $0x21\n
      movw %%ax,%0"
      :"=g" (_ax)
      :
      : "memory","ax","bx","cx","dx"
    );
    info->hiOSVer=LOBYTE(_ax);
    info->loOSVer=HIBYTE(_ax);
    info->hiIEVer=0;
    info->loIEVer=0;
    info->is32BIT=FALSE;
    info->System=osDOS;
    return TRUE;
#endif
}

DWORD GetSystemFeaturesFlag()
{
#ifdef WIN32
	SYSTEMINFO info;
	SystemInfo(&info);
	DWORD nFlags=0;
	switch (info.System)
	{
	case osWin95:
		if (info.hiWINVer>=5 || (info.hiWINVer==4 && info.loWINVer>=90))
			nFlags|=efWinME;
		if (info.hiWINVer>=5 || (info.hiWINVer==4 && info.loWINVer>=10))
			nFlags|=efWin98|efWin95;
		nFlags|=efWin95;
		break;
	case osWinNT:
		if (info.hiWINVer>=5)
		{
			if (info.loWINVer>=1)
				nFlags|=efWinXP;
			nFlags|=efWin2000;
		}
		nFlags|=efWinNT4;
		break;
	default:
		break;
	}
	if (info.hiIEVer>=4)
		nFlags|=efIE4;
	if (info.hiIEVer>=5)
		nFlags|=efIE5;
	if (info.hiIEVer>=6)
		nFlags|=efIE6;
	return nFlags;
#else
	return 0;
#endif
}
