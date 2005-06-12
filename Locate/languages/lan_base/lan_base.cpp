#include <windows.h>

/* 
An example of source code for locate32 language file 
Copyright (C) 2003-2004 Janne Huttunen <jmhuttun@venda.uku.fi>
*/


/* Function for retrieving text from text resource, defined below */
BOOL GetVersionText(LPCSTR,LPSTR, DWORD);

/* GetVersionText needs version.lib, this works at least with Visual C++ */
#pragma comment(lib,"version.lib")


// Instance handle of this module, required by GetVersionText
HINSTANCE hInstance;

/*
Locate32 uses function GetLocateLanguageFileInfo to check whether dll is provides language information
So this function have to be implemented!

Parameters:

szLanguage:				Pointer to a string buffer that receives the null-terminated string 
						specifying language

dwMaxLanguageLength:	Maximum characters in szLanguage ('\0' is included)

szDescription:			Pointer to a string buffer that receives the description of this file

dwMaxDescriptionLength:	Maximum characters in szDescription ('\0' is included)


*/
extern "C" void __declspec(dllexport) __cdecl GetLocateLanguageFileInfo(
	LPSTR /* OUT */ szLanguage,
	DWORD /* IN  */ dwMaxLanguageLength,
	LPSTR /* OUT */ szDescription,
	DWORD /* IN  */ dwMaxDescriptionLength)
{
	// Retrieving description from version resource
	if (!GetVersionText("FileDescription",szDescription,dwMaxDescriptionLength))
	{
		if (dwMaxDescriptionLength>=22)
			dwMaxDescriptionLength=22;
		strncpy(szDescription,"English language file",dwMaxDescriptionLength-1);
		szDescription[dwMaxDescriptionLength-1]='\0';
	}

	// Retrieving language from version resource
	if (!GetVersionText("ProvidesLanguage",szLanguage,dwMaxDescriptionLength))
	{
		if (dwMaxLanguageLength>=8)
			dwMaxLanguageLength=8;
		strncpy(szLanguage,"English",dwMaxLanguageLength-1);
		szLanguage[dwMaxLanguageLength-1]='\0';
	}
}

/* Standard DllMain function, this does nothing but sets hInstance */

extern "C"
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInstance=hinstDLL;
	return TRUE;
}


/* 

Function for retrieving data from version resource 

Parameters:

szBlock:				Name of version block

szText:					Pointer to a string buffer that receives the data

dwMaxTextLen:			Maximum characters in szText ('\0' is included)
 
*/

BOOL GetVersionText(
	LPCSTR /* IN */ szBlock,
	LPSTR /* OUT */ szText, 
	DWORD /* IN  */ dwMaxTextLen
	)
{
    // Creating copyright and version strings
	// Retrieving module filename
	char szModulePath[MAX_PATH];
	if (!GetModuleFileName(hInstance,szModulePath,MAX_PATH))
		return FALSE;
    
	// Copying version information to buffer
	UINT iDataLength=GetFileVersionInfoSize(szModulePath,NULL);
	if (iDataLength<2)
		return FALSE;
	BYTE* pData=new BYTE[iDataLength];
	if (pData==NULL)
		return FALSE;

	if (!GetFileVersionInfo(szModulePath,NULL,iDataLength,pData))
	{
		delete[] pData;
		return FALSE;
	}
	
	VOID* pTranslations,* pProductVersion=NULL;
	char szTranslation[100];
	
	// Checking first translation block
	if (!VerQueryValue(pData,"VarFileInfo\\Translation",&pTranslations,&iDataLength))
	{
		delete[] pData;
		return FALSE;
	}
	wsprintf(szTranslation,"\\StringFileInfo\\%04X%04X\\%s",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1],szBlock);
	
	
	if (!VerQueryValue(pData,szTranslation,&pProductVersion,&iDataLength))
	{
		// Checking english if nothing else does not found
		wsprintf(szTranslation,"\\StringFileInfo\\040904b0\\%s",szBlock);
		
		if (!VerQueryValue(pData,szTranslation,&pProductVersion,&iDataLength))
		{
			delete[] pData;
			return FALSE;
		}
	}
	
	// Copying information from pProductVersion to szText
	if (dwMaxTextLen>=iDataLength)
		dwMaxTextLen=iDataLength;
	strncpy(szText,(LPCSTR)pProductVersion,dwMaxTextLen-1);
    szText[dwMaxTextLen-1]='\0';
	
	delete[] pData;
	return TRUE;
}
	



