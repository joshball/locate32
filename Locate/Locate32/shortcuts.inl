#if !defined(SHORTCUTS_INL)
#define SHORTCUTS_INL

#if _MSC_VER >= 1000
#pragma once
#endif

inline CAction::CAction()
:	m_nAction(ActivateControl),	m_nActivateControl(FindNow),m_pExtraInfo(NULL)
{
}

inline CAction::~CAction()
{
	ClearExtraInfo();
}


inline CAction::SendMessageInfo::SendMessageInfo()
:	nMessage(0),szWindow(NULL),szWParam(NULL),szLParam(NULL)
{
}

inline CAction::SendMessageInfo::~SendMessageInfo()
{
	if (szWindow!=NULL)
		delete[] szWindow;
	if (szWParam!=NULL)
		delete[] szWParam;
	if (szLParam!=NULL)
		delete[] szLParam;
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
:	m_dwFlags(sfDefault),m_pClass(NULL),m_pTitle(NULL) // This initializes union
{
}


inline CShortcut::~CShortcut()
{
	ClearExtraInfo();
	m_apActions.RemoveAll();
}

inline CAction::CAction(void* pVoid)
:	m_pExtraInfo(NULL)
{
}

inline DWORD CAction::GetDataLength() const
{
	DWORD dwLength=sizeof(CAction)+2;
	
	switch (m_nAction)
	{
	case ResultListItems:
		if (m_nResultList==Execute && m_szVerb!=NULL)
			dwLength+=(DWORD)strlen(m_szVerb)+1;
		break;
	case Advanced:
		if ((m_nAdvanced==SendMessage || m_nAdvanced==PostMessage ) &&
			m_pSendMessage!=NULL)
			dwLength+=m_pSendMessage->GetDataLength();
		break;
	}

	return dwLength;
}


inline DWORD CAction::SendMessageInfo::GetDataLength() const
{
	DWORD dwLength=sizeof(WORD)+sizeof(DWORD)+3; // 3*'\0'

	if (szWindow!=NULL)
		dwLength+=(DWORD)strlen(szWindow); // '\0' already included
	

	if (szWParam!=NULL)
		dwLength+=(DWORD)strlen(szWParam); // '\0' already included

	if (szLParam!=NULL)
		dwLength+=(DWORD)strlen(szLParam); // '\0' already included

	return dwLength;
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


inline void CAction::DoActivateTab()
{
	if (GetLocateAppWnd()->m_pLocateDlgThread==NULL)
		return;
	
	GetLocateAppWnd()->m_pLocateDlgThread->m_pLocate->OnActivateTab((int)m_nActivateTab);
}


inline void CAction::DoResultListItems()
{
	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg==NULL)
		return;

	if (GetCurrentThreadId()==GetLocateAppWnd()->m_pLocateDlgThread->m_nThreadID)
		pLocateDlg->OnExecuteResultAction(m_nResultList,m_pExtraInfo);
	else
		pLocateDlg->SendMessage(WM_RESULTLISTACTION,m_nSubAction,(LPARAM)m_pExtraInfo);
}


inline void CShortcut::ResolveMnemonics(CArrayFP<CShortcut*>& aShortcuts,HWND* hDialogs)
{
	for (int i=0;i<aShortcuts.GetSize();i++)
	{
		if (aShortcuts[i]->m_dwFlags&CShortcut::sfUseMemonic)
			aShortcuts[i]->m_bVirtualKey=aShortcuts[i]->GetMnemonicForAction(hDialogs);
	}

}

inline void CShortcut::FormatKeyLabel(VirtualKeyName* pVirtualKeyNames,CString& str) const
{
    FormatKeyLabel(pVirtualKeyNames,m_bVirtualKey,m_bModifiers,(m_dwFlags&CShortcut::sfVirtualKeyIsScancode)?TRUE:FALSE,str);
}

#endif

#endif