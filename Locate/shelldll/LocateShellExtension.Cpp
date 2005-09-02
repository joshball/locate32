//
// Locate Shell Extension
// Copyright 2005 Janne Huttunen
//
// class id:  73ACE856-AD8F-4346-9A07-775E45221BC8
//
//           CShellExt::AddRef and CShellExt::Release code.

#ifndef STRICT
#define STRICT
#endif

#define INC_OLE2        // WIN32, get ole2 from windows.h

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>

#define ResultFromShort(i)  ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(i)))


//
// Initialize GUIDs (should be done only and at-least once per DLL/EXE)
//
#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#include "../lan_resources.h"
#include "LocateShellExtension.h"

#pragma data_seg()

//
// Global variables
//
UINT      g_cRefThisDll = 0;    // Reference count of this DLL.
HINSTANCE g_hmodThisDll = NULL; // Handle to this DLL itself.

#ifdef _LOGGING
HANDLE hOutput=NULL;

void DebugMessage(LPCSTR szMessage)
{
	if (hOutput==NULL)
	{
		hOutput=CreateFile("c:\\lshex-deb.log",GENERIC_WRITE,
			FILE_SHARE_READ,NULL,
			OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if (hOutput!=INVALID_HANDLE_VALUE)
		{
			SetFilePointer(hOutput,0,NULL,FILE_END);
			DWORD dwWrite;
			WriteFile(hOutput,"Logging started\r\n",17,&dwWrite,NULL);
		}
		else
		{
			hOutput=NULL;
			return;
		}
	}

	DWORD dwWritten;
	DWORD dwLen=(DWORD)strlen(szMessage);
	WriteFile(hOutput,szMessage,dwLen,&dwWritten,NULL);
	
	if (dwLen>0 && szMessage[dwLen-1]!='\n')
		WriteFile(hOutput,"\r\n",2,&dwWritten,NULL);

	FlushFileBuffers(hOutput);

}
#endif

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        // Extension DLL one-time initialization
        g_hmodThisDll = hInstance;


		ODS("In DLLMain, DLL_PROCESS_ATTACH\r\n");
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        ODS("In DLLMain, DLL_PROCESS_DETACH\r\n");

#ifdef _LOGGING		
		if (hOutput!=NULL)
		{
			DebugMessage("End of logging");
			CloseHandle(hOutput);
			hOutput=NULL;
		}
#endif
    }

    return 1;   // ok
}

//---------------------------------------------------------------------------
// DllCanUnloadNow
//---------------------------------------------------------------------------

STDAPI DllCanUnloadNow(void)
{
    ODS("In DLLCanUnloadNow\r\n");

    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
    ODS("In DllGetClassObject\r\n");

    *ppvOut = NULL;

    if (IsEqualIID(rclsid, CLSID_ShellExtension))
    {
        CShellExtClassFactory *pcf = new CShellExtClassFactory;

        return pcf->QueryInterface(riid, ppvOut);
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

CShellExtClassFactory::CShellExtClassFactory()
{
    ODS("CShellExtClassFactory::CShellExtClassFactory()\r\n");

    m_cRef = 0L;

    g_cRefThisDll++;
}

CShellExtClassFactory::~CShellExtClassFactory()
{
    g_cRefThisDll--;
}

STDMETHODIMP CShellExtClassFactory::QueryInterface(REFIID riid,
                                                   LPVOID FAR *ppv)
{
    ODS("CShellExtClassFactory::QueryInterface()\r\n");

    *ppv = NULL;

    // Any interface on this object is the object pointer

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;

        AddRef();

        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::AddRef()
{
    ODS("CShellExtClassFactory::AddRef()\r\n");

	char szNum[100];
	wsprintf(szNum,"m_cRef is after this call %d",m_cRef+1);
	DebugMessage(szNum);

	return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::Release()
{
	ODS("CShellExtClassFactory::Release()\r\n");

	char szNum[100];
	wsprintf(szNum,"m_cRef is after this call %d",m_cRef-1);
	DebugMessage(szNum);

    if (--m_cRef)
        return m_cRef;

	DebugMessage("Removing class"); 
	delete this;

    return 0L;
}

STDMETHODIMP CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                                      REFIID riid,
                                                      LPVOID *ppvObj)
{
    ODS("CShellExtClassFactory::CreateInstance()\r\n");

    *ppvObj = NULL;

    // Shell extensions typically don't support aggregation (inheritance)

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    // Create the main shell extension object.  The shell will then call
    // QueryInterface with IID_IShellExtInit--this is how shell extensions are
    // initialized.

    LPCSHELLEXT pShellExt = new CLocateShellExtension();  //Create the CShellExt object

    if (NULL == pShellExt)
        return E_OUTOFMEMORY;

    return pShellExt->QueryInterface(riid, ppvObj);
}


STDMETHODIMP CShellExtClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}

// *********************** CShellExt *************************
CLocateShellExtension::CLocateShellExtension()
{
    ODS("CShellExt::CShellExt()\r\n");

    m_cRef = 0L;
    m_pDataObj = NULL;

    g_cRefThisDll++;
}

CLocateShellExtension::~CLocateShellExtension()
{
    if (m_pDataObj)
        m_pDataObj->Release();

    g_cRefThisDll--;
}

STDMETHODIMP CLocateShellExtension::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
    {
        ODS("CShellExt::QueryInterface()==>IID_IShellExtInit\r\n");

        *ppv = (LPSHELLEXTINIT)this;
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        ODS("CShellExt::QueryInterface()==>IID_IContextMenu\r\n");

        *ppv = (LPCONTEXTMENU)this;
    }
    /*else if (IsEqualIID(riid, IID_IPersistFile))
    {
        ODS("CShellExt::QueryInterface()==>IPersistFile\r\n");

        *ppv = (LPPERSISTFILE)this;
    }*/
    /*else if (IsEqualIID(riid, IID_IShellPropSheetExt))
    {
        ODS("CShellExt::QueryInterface()==>IShellPropSheetExt\r\n");

        *ppv = (LPSHELLPROPSHEETEXT)this;
    }*/
    
    if (*ppv)
    {
        AddRef();

        return NOERROR;
    }

    ODS("CShellExt::QueryInterface()==>Unknown Interface!\r\n");

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CLocateShellExtension::AddRef()
{
	
    ODS("CShellExt::AddRef()\r\n");

	char szNum[100];
	wsprintf(szNum,"m_cRef is after this call %d",m_cRef+1);
	DebugMessage(szNum);
    
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) CLocateShellExtension::Release()
{
    ODS("CShellExt::Release()\r\n");

	char szNum[100];
	wsprintf(szNum,"m_cRef is after this call %d",m_cRef-1);
	DebugMessage(szNum);

    if (--m_cRef)
        return m_cRef;

    DebugMessage("delete this");
	delete this;

    return 0L;
}

STDMETHODIMP CLocateShellExtension::Initialize(LPCITEMIDLIST pIDFolder,
                                   LPDATAOBJECT pDataObj,
                                   HKEY hRegKey)
{
	char text[2000],szPath[MAX_PATH];

	wsprintf(text,"CLocateShellExtension::Initialize(%X,%X,%X)",pIDFolder,pDataObj,hRegKey);
	DebugMessage(text);


	if (SHGetPathFromIDList(pIDFolder,szPath))
	{
		wsprintf(text,"szIDFolder=%s",szPath);
		DebugMessage(text);
	}
	else
	{
		DebugMessage("Cannot retrieve path from id list");
	}


	FORMATETC fe;
	fe.cfFormat=CF_HDROP;
	fe.ptd=NULL;
	fe.dwAspect=DVASPECT_CONTENT;
	fe.lindex=-1;
	fe.tymed=TYMED_HGLOBAL;

	if (pDataObj->QueryGetData(&fe)==S_OK)
	{
		STGMEDIUM med;
		if (pDataObj->GetData(&fe,&med)==S_OK)
		{
			int nCount=DragQueryFile((HDROP)med.hGlobal,0xFFFFFFFF,szPath,MAX_PATH);
			for (int i=0;i<nCount;i++)
			{
				if (DragQueryFile((HDROP)med.hGlobal,i,szPath,MAX_PATH))
					DebugMessage(szPath);
				else
					ODS("DragQueryFile failed");
			}

			GlobalFree(med.hGlobal);
		}
		else
		{
			ODS("GetData failed");
		}
	}
	else
	{
		ODS("QueryGetData failed");
	}


	if (hRegKey!=NULL)
	{
		LONG cb=MAX_PATH;
		if (RegQueryValue(hRegKey,"",szPath,&cb)==ERROR_SUCCESS)
		{
			ODS("RegKey:");
			ODS(szPath);
		}
		else
		{
			ODS("RegQueryValue failed");
		}
	}



    // Initialize can be called more than once

    if (m_pDataObj)
        m_pDataObj->Release();

    // duplicate the object pointer and registry handle

    if (pDataObj)
    {
        m_pDataObj = pDataObj;
        pDataObj->AddRef();
    }

    return NOERROR;
}
