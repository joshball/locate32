#if !defined(SHORTCUTS_INL)
#define SHORTCUTS_INL

#if _MSC_VER >= 1000
#pragma once
#endif

inline CShortcut::CKeyboardAction::CKeyboardAction()
:	m_nAction(ActivateControl),	m_nActivateControl(FindNow)
{
}

inline CShortcut::CKeyboardAction::~CKeyboardAction()
{
}


inline BYTE CShortcut::GetHotkeyModifiers() const
{
    return ModifiersToHotkeyModifiers(m_bModifiers);	
}

inline void CShortcut::SetHotkeyModifiers(BYTE bHotkeyModifier)
{
	m_bModifiers=HotkeyModifiersToModifiers(bHotkeyModifier);
}

inline CShortcut::CShortcut(void* pVoid)
{
}

inline CShortcut::CKeyboardAction::CKeyboardAction(void* pVoid)
{
}

inline DWORD CShortcut::CKeyboardAction::GetDataLength() const
{
	return sizeof(CKeyboardAction)+2;
}

inline BOOL CShortcut::IsModifiersOk(BOOL bAltDown,BOOL bControlDown,BOOL bShiftDown,BOOL bWinDown) const
{
	if (m_bModifiers&ModifierAlt)
	{
		if (!bAltDown) return FALSE;
	}
	else
		if (bAltDown) return FALSE;

	if (m_bModifiers&ModifierControl)
	{
		if (!bControlDown) return FALSE;
	}
	else
		if (bControlDown) return FALSE;

	if (m_bModifiers&ModifierShift)
	{
		if (!bShiftDown) return FALSE;
	}
	else
		if (bShiftDown) return FALSE;

	if (m_bModifiers&ModifierWin)
	{
		if (!bWinDown) return FALSE;
	}
	else
		if (bWinDown) return FALSE;

	return TRUE;
}

inline BOOL CShortcut::IsForegroundWindowOk(HWND hSystemTrayWnd)  const
{
	if ((m_dwFlags&sfKeyTypeMask)==sfLocal)
		return FALSE;

	if (m_pClass==NULL && m_pTitle==NULL)
		return TRUE;

	HWND hWnd=GetForegroundWindow();
	if (hWnd==NULL)
		return FALSE;

	// Checking class 
	if (m_pClass!=NULL)
	{
        if (m_pClass==LPCSTR(-1))
			return HWND(SendMessage(hSystemTrayWnd,WM_GETLOCATEDLG,0,0))==hWnd;
		

		char szClassName[200];
		GetClassName(hWnd,szClassName,200);

		if (!DoClassOrTitleMatch(szClassName,m_pClass))
			return FALSE;
	}

	// Checking class name
	if (m_pTitle!=NULL)
	{
		char szTitleName[200];
		GetWindowText(hWnd,szTitleName,200);
		
		if (!DoClassOrTitleMatch(szTitleName,m_pTitle))
			return FALSE;
	}
	return TRUE;
}

#ifndef KEYHOOK_EXPORTS

inline void CShortcut::ExecuteAction()
{
	for (int i=0;i<m_apActions.GetSize();i++)
		m_apActions[i]->ExecuteAction();
}


inline void CShortcut::CKeyboardAction::DoActivateTab()
{
	if (GetLocateAppWnd()->m_pLocateDlgThread==NULL)
		return;
	
	GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->OnActivateTab((int)m_nActivateTab);
}

#endif

#endif