/* Image property handler front-end for GDI+ 
Copyright (C) 2003-2004 Janne Huttunen				*/

#if !defined(IMAGEHANDLER_H)
#define IMAGEHANDLER_H

#if _MSC_VER >= 1000
#pragma once
#endif 


#ifdef IMAGEHANDLER_EXPORTS
#define IMAGEHANDLER_API __declspec(dllexport)
#else
#define IMAGEHANDLER_API DECLSPEC_IMPORT
#endif

extern "C" {

IMAGEHANDLER_API BOOL InitLibrary(ULONG* pToken);
IMAGEHANDLER_API void UninitLibrary(ULONG uToken);

IMAGEHANDLER_API BOOL GetImageDimensionsA(LPCSTR szFile,SIZE* dim);
IMAGEHANDLER_API BOOL GetImageDimensionsW(LPCWSTR szFile,SIZE* dim);
}

// Function pointer types
typedef BOOL(*IH_INITLIBRARY)(ULONG*);
typedef void(*IH_UNINITLIBRARY)(ULONG);
typedef BOOL(*IH_GETIMAGEDIMENSIONSA)(LPCSTR,SIZE*);
typedef BOOL(*IH_GETIMAGEDIMENSIONSW)(LPCWSTR,SIZE*);
#endif