#include <windows.h>
#include <stdio.h>

/* 
An example of source code for locate32 language file 
Copyright (C) 2003-2006 Janne Huttunen <jmhuttun@venda.uku.fi>
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
		strcpy_s(szDescription,dwMaxDescriptionLength,"English language file");

	// Retrieving language from version resource
	if (!GetVersionText("ProvidesLanguage",szLanguage,dwMaxLanguageLength))
		strcpy_s(szLanguage,dwMaxLanguageLength,"English");
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
	BYTE* pData=new BYTE[iDataLength+2];
	if (pData==NULL)
		return FALSE;

	if (!GetFileVersionInfo(szModulePath,NULL,iDataLength,pData))
	{
		delete[] pData;
		return FALSE;
	}
	
	VOID* pTranslations,* pProductVersion=NULL;
	char szTranslation[200];
	
	
	// Checking first translation block
	if (!VerQueryValue(pData,"VarFileInfo\\Translation",&pTranslations,&iDataLength))
	{
		delete[] pData;
		return FALSE;
	}
	sprintf_s(szTranslation,200,"\\StringFileInfo\\%04X%04X\\%s",LPWORD(pTranslations)[0],LPWORD(pTranslations)[1],szBlock);
	
	
	
	if (!VerQueryValue(pData,szTranslation,&pProductVersion,&iDataLength))
	{
		// Checking english if nothing else does not found
		sprintf_s(szTranslation,200,"\\StringFileInfo\\040904b0\\%s",szBlock);
		
		if (!VerQueryValue(pData,szTranslation,&pProductVersion,&iDataLength))
		{
			delete[] pData;
			return FALSE;
		}
	}


	
	// Copying information from pProductVersion to szText
	strcpy_s(szText,dwMaxTextLen,(LPCSTR)pProductVersion);
	
	delete[] pData;
	return TRUE;
}
	



