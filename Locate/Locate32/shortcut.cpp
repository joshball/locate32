
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
	m_bModifiers(0),m_bVirtualKey(0),
	m_pClass(NULL),m_pTitle(NULL) // This initializes union
{
	// At least one action is needed
	m_apActions.Add(new CAction);

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
		m_apActions[i]=new CAction(*m_apActions[i]);
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
	dwDataLen-=sizeof(WORD)+sizeof(CShortcut);

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
		CAction* pAction=CAction::FromData(pData,dwDataLen,dwLen);

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

CAction* CAction::FromData(const BYTE* pData,DWORD dwDataLen,DWORD& dwUsed)
{
	if (dwDataLen<sizeof(CAction)+sizeof(WORD))
		return NULL;
	
	if (*((WORD*)pData)!=0xFFEC)
		return NULL;

	CAction* pAction=new CAction((void*)NULL);
	CopyMemory(pAction,pData+sizeof(WORD),sizeof(CAction));	
	pData+=sizeof(WORD)+sizeof(CAction);
	dwUsed=sizeof(WORD)+sizeof(CAction);
	dwDataLen-=sizeof(WORD)+sizeof(CAction);

	switch (pAction->m_nAction)
	{
	case ResultListItems:
		if (pAction->m_nResultList==Execute && pAction->m_szVerb!=NULL)
		{
			DWORD dwLen;

			for (dwLen=0;dwLen<dwDataLen && pData[dwLen]!='\0';dwLen++);

			if (dwLen==dwDataLen)
			{
				delete pAction;
				return NULL;
			}

			dwLen++;
			pAction->m_szVerb=new char[dwLen];
            CopyMemory(pAction->m_szVerb,pData,dwLen);
			dwUsed+=dwLen;
			pData+=dwLen;
			dwDataLen-=dwLen;
		}
		break;
	case Advanced:
		if ((pAction->m_nAdvanced==SendMessage || pAction->m_nAdvanced==PostMessage) && 			
			pAction->m_pSendMessage!=NULL)
		{
			DWORD dwLen;

			pAction->m_pSendMessage=SendMessageInfo::FromData(pData,dwDataLen,dwLen);

			if (pAction->m_pSendMessage==NULL)
			{
				delete pAction;
				return NULL;
			}
	
			pData+=dwLen;
			dwUsed+=dwLen;
			dwDataLen-=dwLen;
		}
		break;
	}
	
	return pAction;
}

CAction::SendMessageInfo* CAction::SendMessageInfo::FromData(const BYTE* pData,DWORD dwDataLen,DWORD& dwUsed)
{
	if (dwDataLen<sizeof(WORD)+sizeof(DWORD)+sizeof(3))
		return NULL;
	
	if (*((WORD*)pData)!=0xFFEB)
		return NULL;

	SendMessageInfo* pSendMessage=new SendMessageInfo;
	
	// Set nMessage
	pSendMessage->nMessage=*((DWORD*)(pData+2));
	
	pData+=sizeof(DWORD)+sizeof(WORD);
	dwUsed+=sizeof(DWORD)+sizeof(WORD);
	dwDataLen-=sizeof(DWORD)+sizeof(WORD);

	
	// Set szWindow
	DWORD dwLen;
	for (dwLen=0;dwLen<dwDataLen && pData[dwLen]!='\0';dwLen++);
	if (dwLen==dwDataLen)
	{
		delete pSendMessage;
		return NULL;
	}
	dwLen++;
	if (dwLen>1)
	{
		pSendMessage->szWindow=new char[dwLen];
		CopyMemory(pSendMessage->szWindow,pData,dwLen);
	}
	else
		pSendMessage->szWindow=NULL;

	dwUsed+=dwLen;
	pData+=dwLen;
	dwDataLen-=dwLen;

	// Set szParam
	for (dwLen=0;dwLen<dwDataLen && pData[dwLen]!='\0';dwLen++);
	if (dwLen==dwDataLen)
	{
		delete pSendMessage;
		return NULL;
	}
	dwLen++;
	if (dwLen>1)
	{
		pSendMessage->szWParam=new char[dwLen];
		CopyMemory(pSendMessage->szWParam,pData,dwLen);
	}
	else
		pSendMessage->szWParam=NULL;

	dwUsed+=dwLen;
	pData+=dwLen;
	dwDataLen-=dwLen;


	// Set szLParam
	for (dwLen=0;dwLen<dwDataLen && pData[dwLen]!='\0';dwLen++);
	if (dwLen==dwDataLen)
	{
		delete pSendMessage;
		return NULL;
	}
	dwLen++;
	if (dwLen>1)
	{
		pSendMessage->szLParam=new char[dwLen];
		CopyMemory(pSendMessage->szLParam,pData,dwLen);
	}
	else
		pSendMessage->szLParam=NULL;

	dwUsed+=dwLen;
	pData+=dwLen;
	dwDataLen-=dwLen;


	return pSendMessage;
}

DWORD CAction::GetData(BYTE* pData_) const
{
	BYTE* pData=pData_;
	DWORD dwUsed;
	*((WORD*)pData)=0xFFEC;
	CopyMemory(pData+sizeof(WORD),this,sizeof(CAction));
	pData+=sizeof(CAction)+sizeof(WORD);
	

	switch (m_nAction)
	{
	case ResultListItems:
		if (m_nResultList==Execute && m_szVerb!=NULL)
		{
			dwUsed=istrlen(m_szVerb)+1;
			CopyMemory(pData,m_szVerb,dwUsed);
			pData+=dwUsed;
		}
		break;
	case Advanced:
		if ((m_nAdvanced==SendMessage || m_nAdvanced==PostMessage) && 			
			m_pSendMessage!=NULL)
		{
			dwUsed=m_pSendMessage->GetData(pData);
			pData+=dwUsed;
		}
		break;
	}

	return DWORD(pData-pData_);
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
		CAction* pAction=m_apActions[i];

		if (pAction->m_nAction==CAction::ActivateControl)
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
							CharUpperBuff(&cRet,1);
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

void CAction::ExecuteAction()
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
	case ResultListItems:
		DoResultListItems();
		break;
	case Advanced:
		DoAdvanced();
		break;
	default:
		ASSERT(0);
		break;
	}
}

void CAction::DoActivateControl()
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


void CAction::DoMenuCommand()
{
	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg==NULL)
		return;

	pLocateDlg->SendMessage(WM_COMMAND,MAKEWPARAM(LOWORD(m_nMenuCommand),0),0);
}

void CAction::DoShowHideDialog()
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


void CAction::DoAdvanced()
{
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

DWORD CAction::SendMessageInfo::GetData(BYTE* pData_) const
{
	BYTE* pData=pData_;
	
	*((WORD*)pData)=0xFFEB;
	pData+=sizeof(WORD);
    *((DWORD*)pData)=nMessage;
	pData+=sizeof(DWORD);

	if (szWindow!=NULL)
	{
        size_t dwUsed=strlen(szWindow)+1;
		CopyMemory(pData,szWindow,dwUsed);
		pData+=dwUsed;
	}
	else
		*(pData++)='\0';
		
	
	if (szWParam!=NULL)
	{
        size_t dwUsed=strlen(szWParam)+1;
		CopyMemory(pData,szWParam,dwUsed);
		pData+=dwUsed;
	}
	else
		*(pData++)='\0';
	
	if (szLParam!=NULL)
	{
        size_t dwUsed=strlen(szLParam)+1;
		CopyMemory(pData,szLParam,dwUsed);
		pData+=dwUsed;
	}
	else
		*(pData++)='\0';
	

	return DWORD(pData-pData_);
}