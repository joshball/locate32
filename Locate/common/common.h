#ifndef COMMON_H
#define COMMON_H

#pragma comment(lib,"common.lib")

// Common routines 



// Settings
LPSTR ReadIniFile(LPSTR* pFile,LPCSTR szSection,BYTE& bFileIsReg); // will return registry key name
BOOL LoadSettingsFromFile(LPCSTR szKey,LPCSTR szFile,BYTE bFileIsReg);
BOOL SaveSettingsToFile(LPCSTR szKey,LPCSTR szFile,BYTE bFileIsReg);

#endif
