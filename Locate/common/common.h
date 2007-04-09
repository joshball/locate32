//
// Common routines for locate
//
// Copyright 2006-2007 Janne Huttunen

#ifndef COMMON_H
#define COMMON_H

#pragma comment(lib,"common.lib")

// Common routines 


// Returns string containing default location for file (allocated with new)
LPWSTR GetDefaultFileLocation(LPCWSTR szFileName,BOOL bMustExists=NULL,DWORD* lpdwSize=NULL);

// Settings
LPSTR ReadIniFile(LPSTR* pFile,LPCSTR szSection,BYTE& bFileIsReg); // will return registry key name
BOOL LoadSettingsFromFile(LPCSTR szKey,LPCSTR szFile,BYTE bFileIsReg);
BOOL SaveSettingsToFile(LPCSTR szKey,LPCSTR szFile,BYTE bFileIsReg);

#endif
