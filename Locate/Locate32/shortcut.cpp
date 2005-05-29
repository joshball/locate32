
#ifndef KEYHOOK_EXPORTS

// Included to Locate
#include <HFCLib.h>
#include "Locate32.h"

#else

// Included to keyhook
#include <windows.h>

#include <hfcdef.h>
#include <hfcarray.h>

#include "../lan_resources.h"
#include "../locate32/shortcut.h"
#include "../keyhook/keyhelper.h"

#endif

/////////////////////////////////////////////
// CShortcut

#ifndef KEYHOOK_EXPORTS
CShortcut::CShortcut()
:	m_dwFlags(sfDefault),m_nDelay(0),
	m_pClass(NULL),m_pTitle(NULL),
	m_bModifiers(0),m_bVirtualKey(0)
{
	// At least one action is needed
	m_apActions.Add(new CKeyboardAction);

	if ((m_dwFlags&sfKeyTypeMask)==sfLocal)
		m_wWherePressed=wpDefault;
}

CShortcut::CShortcut(CShortcut& rCopyFrom)
{
	CopyMemory(this,&rCopyFrom,sizeof(CShortcut));

	if ((m_dwFlags&sfKeyTypeMask)!=sfLocal)
	{
		if (m_pClass!=NULL && m_pClass!=LPSTR(-1))
			m_pClass=alloccopy(m_pClass);
		if (m_pTitle!=NULL)
			m_pTitle=alloccopy(m_pTitle);
	}

	for (int i=0;i<m_apActions.GetSize();i++)
		m_apActions[i]=new CKeyboardAction(*m_apActions[i]);
}
	

CShortcut::~CShortcut()
{
	if ((m_dwFlags&sfKeyTypeMask)!=sfLocal)
	{
		if (m_pClass!=NULL && m_pClass!=LPSTR(-1))
			delete[] m_pClass;
		if (m_pTitle!=NULL)
			delete[] m_pTitle;
	}
	m_apActions.RemoveAll();
}

BYTE CShortcut::HotkeyModifiersToModifiers(BYTE bHotkeyModifier)
{
	BYTE bModifiers=0;
	if (bHotkeyModifier&HOTKEYF_ALT)
		bModifiers|=ModifierAlt;
	if (bHotkeyModifier&HOTKEYF_CONTROL)
		bModifiers|=ModifierControl;
	if (bHotkeyModifier&HOTKEYF_SHIFT)
		bModifiers|=ModifierShift;
	return bModifiers;
}

BYTE CShortcut::ModifiersToHotkeyModifiers(BYTE bModifier)
{
	BYTE bRet=0;
	if (bModifier&ModifierAlt)
		bRet|=HOTKEYF_ALT;
	if (bModifier&ModifierControl)
		bRet|=HOTKEYF_CONTROL;
	if (bModifier&ModifierShift)
		bRet|=HOTKEYF_SHIFT;
	return bRet;
}

BOOL CShortcut::LoadShortcuts(CArrayFP<CShortcut*>& aShortcuts,BYTE bLoadFlags)
{
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::defRead)!=ERROR_SUCCESS)
		return FALSE;

	DWORD dwDataLength=RegKey.QueryValueLength("Shortcuts");
	if (dwDataLength<4)
		return FALSE;

	BYTE* pData=new BYTE[dwDataLength];
	RegKey.QueryValue("Shortcuts",(LPSTR)pData,dwDataLength,NULL);
	RegKey.CloseKey();

	BOOL bRet=LoadShortcuts(pData,dwDataLength,aShortcuts,bLoadFlags);
	delete[] pData;
	return bRet;
}

BOOL CShortcut::LoadShortcuts(const BYTE* pData,DWORD dwDataLength,CArrayFP<CShortcut*>& aShortcuts,BYTE bLoadFlag)
{
	const BYTE* pPtr=pData;
	while (dwDataLength>=4 && *((DWORD*)pPtr)!=NULL)
	{
		DWORD dwLength;
		CShortcut* pShortcut=CShortcut::FromData(pPtr,dwDataLength,dwLength);

		if (pShortcut==NULL)
			return FALSE;
		
		BOOL bLoad=FALSE;

		switch (pShortcut->m_dwFlags&sfKeyTypeMask)
		{
		case sfLocal:
			if (bLoadFlag&loadLocal)
				bLoad=TRUE;
			break;
		case sfGlobalHotkey:
			if (bLoadFlag&loadGlobalHotkey)
				bLoad=TRUE;
			break;
		case sfGlobalHook:
			if (bLoadFlag&loadGlobalHook)
				bLoad=TRUE;
			break;
		}

		if (bLoad)
			aShortcuts.Add(pShortcut);
		else
			delete pShortcut;

		pPtr+=dwLength;
		dwDataLength-=dwLength;
	}

	return TRUE;
}

BOOL CShortcut::SaveShortcuts(const CArrayFP<CShortcut*>& aShortcuts)
{
	DWORD dwLength=sizeof(DWORD);
	int i;

	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::defWrite)!=ERROR_SUCCESS)
		return FALSE;

	for (i=0;i<aShortcuts.GetSize();i++)
		dwLength+=aShortcuts[i]->GetDataLength();

    BYTE* pData=new BYTE[dwLength];
	DWORD dwUsed=0;

	for (i=0;i<aShortcuts.GetSize();i++)
		dwUsed+=aShortcuts[i]->GetData(pData+dwUsed);
	
	*((DWORD*)(pData+dwUsed))=NULL;

	ASSERT(dwUsed+sizeof(DWORD)==dwLength);

	BOOL bRet=RegKey.SetValue("Shortcuts",(LPSTR)pData,dwLength,REG_BINARY)==ERROR_SUCCESS;
    delete[] pData;

	return bRet;
}

DWORD CShortcut::GetData(BYTE* _pData) const
{
	BYTE* pData=_pData;
	DWORD dwUsed;

	*((WORD*)pData)=0xFFED; // Start mark
	pData+=sizeof(WORD);

	CopyMemory(pData,this,sizeof(CShortcut));
	pData+=sizeof(CShortcut);


	if ((m_dwFlags&sfKeyTypeMask)!=sfLocal)
	{
		if (m_pClass!=NULL && m_pClass!=LPSTR(-1))
		{
			dwUsed=istrlen(m_pClass)+1;
			CopyMemory(pData,m_pClass,dwUsed);
			pData+=dwUsed;
		}

		if (m_pTitle!=NULL)
		{
            dwUsed=istrlen(m_pTitle)+1;
			CopyMemory(pData,m_pTitle,dwUsed);
			pData+=dwUsed;
		}
	}

	for (int i=0;i<m_apActions.GetSize();i++)
	{
		dwUsed=m_apActions[i]->GetData(pData);
		pData+=dwUsed;
	}

	return DWORD(pData-_pData);
}

DWORD CShortcut::GetDataLength() const
{
	DWORD dwLen=sizeof(CShortcut)+sizeof(WORD);
	if ((m_dwFlags&sfKeyTypeMask)!=sfLocal)
	{
		if (m_pClass!=NULL && m_pClass!=LPSTR(-1))
			dwLen+=istrlen(m_pClass)+1;

		if (m_pTitle!=NULL)
			dwLen+=istrlen(m_pTitle)+1;
	}

	for (int i=0;i<m_apActions.GetSize();i++)
		dwLen+=m_apActions[i]->GetDataLength();

	return dwLen;
}

CShortcut* CShortcut::FromData(const BYTE* pData,DWORD dwDataLen,DWORD& dwUsed)
{
	DWORD dwLen;

	if (dwDataLen<sizeof(CShortcut)+sizeof(WORD))
		return NULL;

    if (*((WORD*)pData)!=0xFFED)
		return NULL;

	CShortcut* pShortcut=new CShortcut((void*)NULL);
	CopyMemory(pShortcut,pData+sizeof(WORD),sizeof(CShortcut));	
	dwUsed=sizeof(WORD)+sizeof(CShortcut);
	pData+=sizeof(WORD)+sizeof(CShortcut);
	dwDataLen-=dwUsed;

	if ((pShortcut->m_dwFlags&sfKeyTypeMask)!=sfLocal) // Load class and title 
	{
		if (pShortcut->m_pClass!=NULL && pShortcut->m_pClass!=LPSTR(-1))
		{
			for (dwLen=0;dwLen<dwDataLen && pData[dwLen]!='\0';dwLen++);

			if (dwLen==dwDataLen)
			{
				delete pShortcut;
				return NULL;
			}

			dwLen++;
			pShortcut->m_pClass=new char[dwLen];
			CopyMemory(pShortcut->m_pClass,pData,dwLen);
			dwUsed+=dwLen;
			pData+=dwLen;
			dwDataLen-=dwLen;
		}
		
		if (pShortcut->m_pTitle!=NULL)
		{
			for (dwLen=0;dwLen<dwDataLen && pData[dwLen]!='\0';dwLen++);

			if (dwLen==dwDataLen)
			{
				delete pShortcut;
				return NULL;
			}

			dwLen++;
			pShortcut->m_pTitle=new char[dwLen];
			CopyMemory(pShortcut->m_pTitle,pData,dwLen);
			dwUsed+=dwLen;
			pData+=dwLen;
			dwDataLen-=dwLen;
		}
	}

	DWORD dwActions=pShortcut->m_apActions.GetSize();
	pShortcut->m_apActions.GiveBuffer(); // There is no allocated data
	
    for (DWORD i=0;i<dwActions;i++)
	{
		CKeyboardAction* pAction=CKeyboardAction::FromData(pData,dwDataLen,dwLen);

		if (pAction==NULL)
		{
			delete pShortcut;
			return NULL;
		}

		pShortcut->m_apActions.Add(pAction);
		pData+=dwLen;
		dwUsed+=dwLen;
		dwDataLen-=dwLen;

	}
	return pShortcut;
}

CShortcut::CKeyboardAction* CShortcut::CKeyboardAction::FromData(const BYTE* pData,DWORD dwDataLen,DWORD& dwUsed)
{
	if (dwDataLen<sizeof(CKeyboardAction)+sizeof(WORD))
		return NULL;
	
	if (*((WORD*)pData)!=0xFFEC)
		return NULL;

	
	CKeyboardAction* pAction=new CKeyboardAction((void*)NULL);
	CopyMemory(pAction,pData+sizeof(WORD),sizeof(CKeyboardAction));	
	dwUsed=sizeof(WORD)+sizeof(CKeyboardAction);
	
	return pAction;
}
	
DWORD CShortcut::CKeyboardAction::GetData(BYTE* pData) const
{
	*((WORD*)pData)=0xFFEC;
	CopyMemory(pData+sizeof(WORD),this,sizeof(CKeyboardAction));
	
	return sizeof(CKeyboardAction)+2;
}




BOOL CShortcut::GetDefaultShortcuts(CArrayFP<CShortcut*>& aShortcuts,BYTE bLoadFlag)
{
	// This code saves currently saved shortcuts to correct file
	// Uncommend and run once

	/*
	// BEGIN 
	LPCSTR szFile="C:\\My Documents\\Programming\\C\\Locate\\Locate32\\commonres\\defaultshortcuts.dat";
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCU,CString(IDS_REGPLACE,CommonResource)+"\\General",CRegKey::defRead)!=ERROR_SUCCESS)
		return FALSE;
	DWORD dwDataLength=RegKey.QueryValueLength("Shortcuts");
	if (dwDataLength<4)
		return FALSE;
	BYTE* pData=new BYTE[dwDataLength];
	RegKey.QueryValue("Shortcuts",(LPSTR)pData,dwDataLength,NULL);
	RegKey.CloseKey();
	CFile File(TRUE);

	try {
		File.Open(szFile,CFile::defWrite);
		File.Write(pData,dwDataLength);
		File.Close();
	}
	catch (...)
	{
	}
	// END
	*/

	HRSRC hRsrc=FindResource(GetInstanceHandle(),MAKEINTRESOURCE(IDR_DEFAULTSHORTCUTS),"DATA");
    if (hRsrc==NULL)
		return FALSE;
    	
	DWORD dwLength=SizeofResource(GetInstanceHandle(),hRsrc);
	if (dwLength<4)
		return FALSE;

	HGLOBAL hGlobal=LoadResource(GetInstanceHandle(),hRsrc);
	if (hGlobal==NULL)
		return FALSE;

	return LoadShortcuts((const BYTE*)LockResource(hGlobal),dwLength,aShortcuts,bLoadFlag);
}

char CShortcut::GetMnemonicForAction(HWND* hDialogs) const
{

	for (int i=0;i<m_apActions.GetSize();i++)
	{
		CShortcut::CKeyboardAction* pAction=m_apActions[i];

		if (pAction->m_nAction==CShortcut::CKeyboardAction::ActivateControl)
		{
			for (int j=0;hDialogs[j]!=NULL;j++)
			{	
				if (HIWORD(pAction->m_nActivateControl)& (1<<15))
					continue;


				HWND hControl=::GetDlgItem(hDialogs[j],HIWORD(pAction->m_nActivateControl));
				if (hControl!=NULL)
				{
					DWORD dwTextLen=::SendMessage(hControl,WM_GETTEXTLENGTH,0,0);
					char* pText=new char[dwTextLen+2];
					::SendMessage(hControl,WM_GETTEXT,dwTextLen+2,LPARAM(pText));

					for (DWORD k=0;k<dwTextLen-1;k++)
					{
						if (pText[k]=='&')
						{
							char cRet=pText[k+1];
							delete[] pText;
							return cRet;
						}
					}

					delete[] pText;
					break;
				}
			}
		}
	}

	return 0;
}

void CShortcut::CKeyboardAction::ExecuteAction()
{
	switch (m_nAction)
	{
	case ActivateControl:
		DoActivateControl();
		break;
	case ActivateTab:
		DoActivateTab();
		break;
	case MenuCommand:
		DoMenuCommand();
		break;
	case ShowHideDialog:
		DoShowHideDialog();
		break;
	}
}

void CShortcut::CKeyboardAction::DoActivateControl()
{
	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg==NULL)
		return;

	HWND hControl=pLocateDlg->GetDlgItem(LOWORD(m_nActivateControl));
	if (hControl==0)
		hControl=pLocateDlg->m_NameDlg.GetDlgItem(LOWORD(m_nActivateControl));
	if (hControl==0)
		hControl=pLocateDlg->m_SizeDateDlg.GetDlgItem(LOWORD(m_nActivateControl));
	if (hControl==0)
		hControl=pLocateDlg->m_AdvancedDlg.GetDlgItem(LOWORD(m_nActivateControl));
	
	if (hControl==0)
		return;
	
	pLocateDlg->SendMessage(WM_COMMAND,MAKEWPARAM(LOWORD(m_nActivateControl),1),0);
}


void CShortcut::CKeyboardAction::DoMenuCommand()
{
	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg==NULL)
		return;

	pLocateDlg->SendMessage(WM_COMMAND,MAKEWPARAM(LOWORD(m_nMenuCommand),0),0);
}

void CShortcut::CKeyboardAction::DoShowHideDialog()
{
	CLocateDlg* pLocateDlg=GetLocateDlg();
	
	switch(m_nDialogCommand)
	{
	case ShowDialog:
		GetLocateAppWnd()->OnLocate();
		break;
	case MinimizeDialog:
		if (pLocateDlg!=NULL)
			pLocateDlg->ShowWindow(CWnd::swMinimize);
		break;
	case CloseDialog:
		if (pLocateDlg!=NULL)
			pLocateDlg->PostMessage(WM_CLOSE);
		break;
	case ShowOrHideDialog:
		if (pLocateDlg!=NULL)
		{
			WINDOWPLACEMENT wp;
			wp.length=sizeof(WINDOWPLACEMENT);
			pLocateDlg->GetWindowPlacement(&wp);
			if (wp.showCmd!=SW_SHOWMINIMIZED &&wp.showCmd!=SW_HIDE)
				pLocateDlg->ShowWindow(CWnd::swMinimize);
			else 
				GetLocateAppWnd()->OnLocate();
		}
		break;
	case OpenOrCloseDialog:
		if (pLocateDlg!=NULL)
		{
			WINDOWPLACEMENT wp;
			wp.length=sizeof(WINDOWPLACEMENT);
			pLocateDlg->GetWindowPlacement(&wp);
			if (wp.showCmd!=SW_SHOWMINIMIZED &&wp.showCmd!=SW_HIDE)
				pLocateDlg->PostMessage(WM_CLOSE);
			else
				GetLocateAppWnd()->OnLocate();
		}
		else
			GetLocateAppWnd()->OnLocate();
		break;
	}
}

#endif

static BOOL _ContainString(LPCSTR s1,LPCSTR s2,size_t s2len) // Is s2 in the s1
{
#ifdef _DEBUG
	LPCSTR orig1=s1,orig2=s2;
#endif 

    BOOL bBreakIfNotMatch;
	if (s2[0]=='*')
	{
		if (s2len==1)
			return TRUE;
		s2++;
		bBreakIfNotMatch=FALSE;
	}
	else
		bBreakIfNotMatch=TRUE;

	while (*s1!='\0')
	{
		for (size_t i=0;;i++)
		{
			// is s1 too short?
			if (s1[i]=='\0')
			{
				if (s2len<=i)
					return TRUE;
				return s2[i]=='*' && s2len<=i+1;
			}
			// string differ
			if (s1[i]!=s2[i])
			{
				if (s2[i]=='?')
					continue;
				
				if (s2[i]=='*')
				{
					if (s2len<=i+1)
						return TRUE;
					s2+=i+1;
					s1+=i-1;
					bBreakIfNotMatch=FALSE;
					break;
				}
				break;
			}
		}
		if (bBreakIfNotMatch)
			return FALSE;
		s1++;
	}
	return FALSE;
}

BOOL CShortcut::DoClassOrTitleMatch(LPCSTR pClass,LPCSTR pCondition)
{
	int nIndex;
    
	for (;;)
	{
		for (nIndex=0;pCondition[nIndex]!='\0' && pCondition[nIndex]!='|';nIndex++);
		
		if (_ContainString(pClass,pCondition,nIndex))
			return TRUE;

        if (pCondition[nIndex]=='\0')
			return FALSE;

        pCondition+=nIndex+1;
	}
}


