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

#include "LocateShellExtension.h"

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

//
//  FUNCTION: CShellExt::QueryContextMenu(HMENU, UINT, UINT, UINT, UINT)
//
//  PURPOSE: Called by the shell just before the context menu is displayed.
//           This is where you add your specific menu items.
//
//  PARAMETERS:
//    hMenu      - Handle to the context menu
//    indexMenu  - Index of where to begin inserting menu items
//    idCmdFirst - Lowest value for new menu ID's
//    idCmtLast  - Highest value for new menu ID's
//    uFlags     - Specifies the context of the menu event
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CLocateShellExtension::QueryContextMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags)
{
    ODS("CShellExt::QueryContextMenu()\r\n");

    UINT idCmd = idCmdFirst;
    char szMenuText[64];
    char szMenuText2[64];
    char szMenuText3[64];
    char szMenuText4[64];
    BOOL bAppendItems=TRUE;

    if ((uFlags & 0x000F) == CMF_NORMAL)  //Check == here, since CMF_NORMAL=0
    {
        ODS("CMF_NORMAL...\r\n");
        lstrcpyn(szMenuText, "&New .GAK menu 1, Normal File", ARRAYSIZE(szMenuText));
        lstrcpyn(szMenuText2, "&New .GAK menu 2, Normal File", ARRAYSIZE(szMenuText2));
        lstrcpyn(szMenuText3, "&New .GAK menu 3, Normal File", ARRAYSIZE(szMenuText3));
        lstrcpyn(szMenuText4, "&New .GAK menu 4, Normal File", ARRAYSIZE(szMenuText4));
    }
    else
        if (uFlags & CMF_VERBSONLY)
        {
            ODS("CMF_VERBSONLY...\r\n");
            lstrcpyn(szMenuText, "&New .GAK menu 1, Shortcut File", ARRAYSIZE(szMenuText));
            lstrcpyn(szMenuText2, "N&ew .GAK menu 2, Shortcut File", ARRAYSIZE(szMenuText2));
            lstrcpyn(szMenuText3, "&New .GAK menu 3, Shortcut File", ARRAYSIZE(szMenuText3));
            lstrcpyn(szMenuText4, "&New .GAK menu 4, Shortcut File", ARRAYSIZE(szMenuText4));
        }
    else
        if (uFlags & CMF_EXPLORE)
        {
            ODS("CMF_EXPLORE...\r\n");
            lstrcpyn(szMenuText, "&New .GAK menu 1, Normal File right click in Explorer", ARRAYSIZE(szMenuText));
            lstrcpyn(szMenuText2, "N&ew .GAK menu 2, Normal File right click in Explorer", ARRAYSIZE(szMenuText2));
            lstrcpyn(szMenuText3, "&New .GAK menu 3, Normal File right click in Explorer", ARRAYSIZE(szMenuText3));
            lstrcpyn(szMenuText4, "&New .GAK menu 4, Normal File right click in Explorer", ARRAYSIZE(szMenuText4));
        }
    else
        if (uFlags & CMF_DEFAULTONLY)
        {
            ODS("CMF_DEFAULTONLY...\r\n");
            bAppendItems = FALSE;
        }
    else
        {
            char szTemp[32];

            wsprintf(szTemp, "uFlags==>%d\r\n", uFlags);
            ODS("CMF_default...\r\n");
            ODS(szTemp);
            bAppendItems = FALSE;
        }

    if (bAppendItems)
    {
		InsertMenu(hMenu,indexMenu++,MF_STRING|MF_BYPOSITION,idCmd++,szMenuText);
		return ResultFromShort(idCmd-idCmdFirst); //Must return number of menu
                                                  //items we added.
	}

	return NOERROR;
}

//
//  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)
//
//  PURPOSE: Called by the shell after the user has selected on of the
//           menu items that was added in QueryContextMenu().
//
//  PARAMETERS:
//    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CLocateShellExtension::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    ODS("CShellExt::InvokeCommand()\r\n");

    HRESULT hr = E_INVALIDARG;

    //If HIWORD(lpcmi->lpVerb) then we have been called programmatically
    //and lpVerb is a command that should be invoked.  Otherwise, the shell
    //has called us, and LOWORD(lpcmi->lpVerb) is the menu ID the user has
    //selected.  Actually, it's (menu ID - idCmdFirst) from QueryContextMenu().
    if (!HIWORD(lpcmi->lpVerb))
    {
        UINT idCmd = LOWORD(lpcmi->lpVerb);

        switch (idCmd)
        {
            case 0:
                hr = DoGAKMenu1(lpcmi->hwnd,
                                lpcmi->lpDirectory,
                                lpcmi->lpVerb,
                                lpcmi->lpParameters,
                                lpcmi->nShow);
                break;

            case 1:
                hr = DoGAKMenu2(lpcmi->hwnd,
                                lpcmi->lpDirectory,
                                lpcmi->lpVerb,
                                lpcmi->lpParameters,
                                lpcmi->nShow);
                break;

            case 2:
                hr = DoGAKMenu3(lpcmi->hwnd,
                                lpcmi->lpDirectory,
                                lpcmi->lpVerb,
                                lpcmi->lpParameters,
                                lpcmi->nShow);
                break;

            case 3:
                hr = DoGAKMenu4(lpcmi->hwnd,
                                lpcmi->lpDirectory,
                                lpcmi->lpVerb,
                                lpcmi->lpParameters,
                                lpcmi->nShow);
                break;
        }
    }
    return hr;
}


//
//  FUNCTION: CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO)
//
//  PURPOSE: Called by the shell after the user has selected on of the
//           menu items that was added in QueryContextMenu().
//
//  PARAMETERS:
//    lpcmi - Pointer to an CMINVOKECOMMANDINFO structure
//
//  RETURN VALUE:
//
//
//  COMMENTS:
//

STDMETHODIMP CLocateShellExtension::GetCommandString(UINT_PTR idCmd,
                                         UINT uFlags,
                                         UINT FAR *reserved,
                                         LPSTR pszName,
                                         UINT cchMax)
{
    ODS("CShellExt::GetCommandString()\r\n");

    switch (idCmd)
    {
        case 0:
            lstrcpyn(pszName, "New menu item number 1", cchMax);
            break;

        case 1:
            lstrcpyn(pszName, "New menu item number 2", cchMax);
            break;

        case 2:
            lstrcpyn(pszName, "New menu item number 3", cchMax);
            break;

        case 3:
            lstrcpyn(pszName, "New menu item number 4", cchMax);
            break;
    }

    return NOERROR;
}

STDMETHODIMP CLocateShellExtension::DoGAKMenu1(HWND hParent,
                                   LPCSTR pszWorkingDir,
                                   LPCSTR pszCmd,
                                   LPCSTR pszParam,
                                   int iShowCmd)
{
    ODS("CShellExt::DoGAKMenu1()\r\n");

    MessageBox(hParent, "Menu item 1!", "Shell Extension Sample", MB_OK);

    return NOERROR;
}

STDMETHODIMP CLocateShellExtension::DoGAKMenu2(HWND hParent,
                                   LPCSTR pszWorkingDir,
                                   LPCSTR pszCmd,
                                   LPCSTR pszParam,
                                   int iShowCmd)
{
    ODS("CShellExt::DoGAKMenu2()\r\n");

    MessageBox(hParent, "Menu item 2!", "Shell Extension Sample", MB_OK);

    return NOERROR;
}

STDMETHODIMP CLocateShellExtension::DoGAKMenu3(HWND hParent,
                                   LPCSTR pszWorkingDir,
                                   LPCSTR pszCmd,
                                   LPCSTR pszParam,
                                   int iShowCmd)
{
    ODS("CShellExt::DoGAKMenu3()\r\n");

    MessageBox(hParent, "Menu item 3!", "Shell Extension Sample", MB_OK);

    return NOERROR;
}

STDMETHODIMP CLocateShellExtension::DoGAKMenu4(HWND hParent,
                                   LPCSTR pszWorkingDir,
                                   LPCSTR pszCmd,
                                   LPCSTR pszParam,
                                   int iShowCmd)
{
    ODS("CShellExt::DoGAKMenu4()\r\n");

    MessageBox(hParent, "Menu item 4!", "Shell Extension Sample", MB_OK);

    return NOERROR;
}
