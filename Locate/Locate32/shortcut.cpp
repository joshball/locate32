
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
#include "../locate32/messages.h"

#endif

static BOOL _ContainString(LPCSTR s1,LPCSTR s2,size_t s2len) // Is s2 in the s1
{


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

#ifndef KEYHOOK_EXPORTS


BOOL __ContainString(LPCSTR s1,LPCSTR s2) // Is s2 in the s1
{

	BOOL bBreakIfNotMatch;
	if (s2[0]=='*')
	{
		if (s2[1]=='\0')
			return TRUE;
		s2++;
		bBreakIfNotMatch=FALSE;
	}
	else
		bBreakIfNotMatch=TRUE;

	while (*s1!='\0')
	{
		for (int i=0;;i++)
		{
			// is s1 too short?
			if (s1[i]=='\0')
			{
				if (s2[i]=='\0')
					return TRUE;
				return s2[i]=='*' && s2[i+1]=='\0';
			}
			// string differ
			if (s1[i]!=s2[i])
			{
				if (s2[i]=='?')
					continue;
				
				if (s2[i]=='*')
				{
					if (s2[i+1]=='\0')
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



/////////////////////////////////////////////
// CShortcut


CShortcut::CShortcut()
:	m_dwFlags(sfDefault),m_nDelay(0),
	m_bModifiers(0),m_bVirtualKey(0),
	m_pClass(NULL),m_pTitle(NULL) // This initializes union
{
	// At least one action is needed
	m_apActions.Add(new CAction);

	if ((m_dwFlags&sfKeyTypeMask)==sfLocal)
		m_wWhenPressed=wpDefault;
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
	

void CShortcut::ClearExtraInfo()
{
	if ((m_dwFlags&sfKeyTypeMask)!=sfLocal)
	{
		if (m_pClass!=NULL && m_pClass!=LPSTR(-1))
			delete[] m_pClass;
		if (m_pTitle!=NULL)
			delete[] m_pTitle;
	}
	
	m_pClass=NULL;m_pTitle=NULL;
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

CSubAction* CSubAction::FromData(DWORD nAction,const BYTE* pData,DWORD dwDataLen,DWORD& dwUsed)
{
	if (dwDataLen<sizeof(WORD))
		return NULL;

	if (*((WORD*)pData)!=0xFFEA)
		return NULL;
	
	CSubAction* pSubAction=new CSubAction((void*)NULL);
	dwUsed=pSubAction->FillFromData(nAction,pData+sizeof(WORD),dwDataLen-sizeof(WORD));
	if (dwUsed==0)
	{
		delete pSubAction;
		return NULL;
	}


	dwUsed+=sizeof(WORD);
	return pSubAction;
}
	
DWORD CSubAction::FillFromData(DWORD nAction,const BYTE* pData,DWORD dwDataLen)
{
	if (dwDataLen<2*sizeof(DWORD))
		return 0;
	
	DWORD dwUsed=0;
		
	
	m_nSubAction=*((DWORD*)(pData));
	m_pExtraInfo=*((void**)(pData+sizeof(DWORD)));

	pData+=2*sizeof(DWORD);
	dwUsed=2*sizeof(DWORD);
	dwDataLen-=2*sizeof(DWORD);

	switch (nAction)
	{
	case CAction::ResultListItems:
		if (m_nResultList==Execute && m_szVerb!=NULL)
		{
			DWORD dwLen;

			for (dwLen=0;dwLen<dwDataLen && ((LPCWSTR)pData)[dwLen]!='\0';dwLen++);

			if (dwLen==dwDataLen)
				return 0;

			dwLen++;
			m_szVerb=new WCHAR[dwLen];
            MemCopyW(m_szVerb,pData,dwLen);
			dwUsed+=dwLen*2;
			pData+=dwLen*2;
			dwDataLen-=dwLen;
		}
		else if (m_nResultList==ExecuteCommand && m_szCommand!=NULL)
		{
			DWORD dwLen;

			for (dwLen=0;dwLen<dwDataLen && ((LPCWSTR)pData)[dwLen]!='\0';dwLen++);

			if (dwLen==dwDataLen)
				return 0;
			
			dwLen++;
			m_szCommand=new WCHAR[dwLen];
            MemCopyW(m_szCommand,pData,dwLen);
			dwUsed+=dwLen*2;
			pData+=dwLen*2;
			dwDataLen-=dwLen;
		}
		break;
	case CAction::Advanced:
		if ((m_nAdvanced==SendMessage || m_nAdvanced==PostMessage) && 			
			m_pSendMessage!=NULL)
		{
			DWORD dwLen;

			m_pSendMessage=SendMessageInfo::FromData(pData,dwDataLen,dwLen);

			if (m_pSendMessage==NULL)
				return 0;
			
			pData+=dwLen;
			dwUsed+=dwLen;
			dwDataLen-=dwLen;
		}
		break;
	}
	return dwUsed;
}
	

CAction* CAction::FromData(const BYTE* pData,DWORD dwDataLen,DWORD& dwUsed)
{
	if (dwDataLen<sizeof(DWORD)+sizeof(WORD))
		return NULL;
	
	if (*((WORD*)pData)!=0xFFEC)
		return NULL;
	
	CAction* pAction=new CAction((void*)NULL);
	pAction->m_dwAction=*((DWORD*)(pData+sizeof(WORD)));
		
	dwUsed=pAction->FillFromData(pAction->m_dwAction,
		pData+sizeof(WORD)+sizeof(DWORD),dwDataLen-sizeof(WORD)-sizeof(DWORD));
	if (dwUsed==0)
	{
		delete pAction;
		return NULL;
	}

	dwUsed+=sizeof(WORD)+sizeof(DWORD);

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



DWORD CSubAction::GetData(DWORD nAction,BYTE* pData_,BOOL bHeader) const
{
	BYTE* pData=pData_;
	
	if (bHeader)
	{
		*((WORD*)pData)=0xFFEA;
		pData+=sizeof(WORD);
	}

	*((DWORD*)pData)=m_nSubAction;
	*((void**)(pData+sizeof(DWORD)))=m_pExtraInfo;
	pData+=2*sizeof(DWORD);
	

	switch (nAction)
	{
	case CAction::ResultListItems:
		if (m_nResultList==Execute && m_szVerb!=NULL)
		{
			DWORD dwUsed=istrlenw(m_szVerb)+1;
			MemCopyW(pData,m_szVerb,dwUsed);
			pData+=dwUsed*2;
		}
		else if (m_nResultList==ExecuteCommand && m_szCommand!=NULL)
		{
			DWORD dwUsed=istrlenw(m_szCommand)+1;
			MemCopyW(pData,m_szCommand,dwUsed);
			pData+=dwUsed*2;
		}
		break;
	case CAction::Advanced:
		if ((m_nAdvanced==SendMessage || m_nAdvanced==PostMessage) && 			
			m_pSendMessage!=NULL)
		{
			DWORD dwUsed=m_pSendMessage->GetData(pData);
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
	{
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
	}
	// END
	*/
	

	// Check file
	int nIndex=LastCharIndex(GetLocateApp()->GetExeName(),'\\')+1;
	char szPath[MAX_PATH];
	CopyMemory(szPath,GetLocateApp()->GetExeName(),nIndex);
	CopyMemory(szPath+nIndex,"defshrtc.dat",13);
    
	CFile File(TRUE);
	BYTE* pData=NULL;
	DWORD dwLength;
	try {
		File.Open(szPath,CFile::defRead);
		dwLength=File.GetLength();
		pData=new BYTE[dwLength];
		File.Read(pData,dwLength);
		File.Close();
		BOOL bRet=LoadShortcuts(pData,dwLength,aShortcuts,bLoadFlag);
		delete[] pData;
		
		if (bRet)
            return TRUE;
	}
	catch (...)
	{
		if (pData!=NULL)
			delete[] pData;
	}



	HRSRC hRsrc=FindResource(GetInstanceHandle(),MAKEINTRESOURCE(IDR_DEFAULTSHORTCUTS),"DATA");
    if (hRsrc==NULL)
		return FALSE;
    	
	dwLength=SizeofResource(GetInstanceHandle(),hRsrc);
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

void CSubAction::GetCopyFrom(DWORD nAction,CSubAction& rCopyFrom)
{
	CopyMemory(this,&rCopyFrom,sizeof(CSubAction));

	switch (nAction)
	{
	case CAction::ResultListItems:
		if (m_nResultList==Execute && m_szVerb!=NULL)
			m_szVerb=alloccopy(m_szVerb);
		if (m_nResultList==ExecuteCommand && m_szCommand!=NULL)
			m_szCommand=alloccopy(m_szCommand);
		break;
	case CAction::Advanced:
		if ((m_nAdvanced==SendMessage || m_nAdvanced==PostMessage ) &&
			m_pSendMessage!=NULL)
			m_pSendMessage=new SendMessageInfo(*m_pSendMessage);
		break;
	}
	
}

void CSubAction::ClearExtraInfo(DWORD nAction)
{
	switch (nAction)
	{
	case CAction::ResultListItems:
		if (m_nResultList==Execute && m_pExtraInfo!=NULL)
			delete[] m_szVerb;
		else if (m_nResultList==ExecuteCommand && m_szCommand!=NULL)
			delete[] m_szCommand;			
		break;
	case CAction::Advanced:
		if ((m_nAdvanced==SendMessage || m_nAdvanced==PostMessage ) &&
			m_pSendMessage!=NULL)
			delete m_pSendMessage;
		break;
	}
	m_pExtraInfo=NULL;
}

void CAction::ExecuteAction()
{
	switch (m_nAction)
	{
	case None:
		break;
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

void CSubAction::DoActivateControl()
{
	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg==NULL)
		return;
	
	if (GetCurrentThreadId()==GetLocateAppWnd()->m_pLocateDlgThread->m_nThreadID)
		pLocateDlg->OnCommand(LOWORD(m_nActivateControl),1,NULL);
	else
		pLocateDlg->SendMessage(WM_COMMAND,MAKEWPARAM(LOWORD(m_nActivateControl),1),0);
}


void CSubAction::DoMenuCommand()
{
	CLocateDlg* pLocateDlg=GetLocateDlg();
	if (pLocateDlg==NULL)
		return;

	if (GetCurrentThreadId()==GetLocateAppWnd()->m_pLocateDlgThread->m_nThreadID)
		pLocateDlg->OnCommand(LOWORD(m_nMenuCommand),0,NULL);
	else
		pLocateDlg->SendMessage(WM_COMMAND,MAKEWPARAM(LOWORD(m_nMenuCommand),0),0);
}

void CSubAction::DoShowHideDialog()
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
			if (wp.showCmd!=SW_SHOWMINIMIZED && wp.showCmd!=SW_HIDE)
				pLocateDlg->PostMessage(WM_CLOSE);
			else
				GetLocateAppWnd()->OnLocate();
		}
		else
			GetLocateAppWnd()->OnLocate();
		break;
	case RestoreDialog:
		if (pLocateDlg!=NULL)
			pLocateDlg->ShowWindow(CWnd::swRestore);
		break;
	case MaximizeDialog:
		if (pLocateDlg!=NULL)
			pLocateDlg->ShowWindow(CWnd::swMaximize);
		break;
	case MaximizeOrRestoreDialog:
		if (pLocateDlg!=NULL)
		{
			WINDOWPLACEMENT wp;
			wp.length=sizeof(WINDOWPLACEMENT);
			pLocateDlg->GetWindowPlacement(&wp);
			if (wp.showCmd!=SW_SHOWMAXIMIZED)
				pLocateDlg->ShowWindow(CWnd::swMaximize);
			else
				pLocateDlg->ShowWindow(CWnd::swRestore);
		}
		break;
	}
}



void CSubAction::DoResultListItems()
{
	CLocateDlg* pLocateDlg=GetLocateDlg();
	
	if (m_nResultList==ExecuteCommand)
	{
		CLocateDlg::ExecuteCommand(m_szCommand);
		return;
	}
		
	if (pLocateDlg==NULL)
		return;

	/*if (GetCurrentThreadId()==GetLocateAppWnd()->m_pLocateDlgThread->m_nThreadID)
		pLocateDlg->OnExecuteResultAction(m_nResultList,m_pExtraInfo);
	else*/
		pLocateDlg->SendMessage(WM_RESULTLISTACTION,m_nSubAction,(LPARAM)m_pExtraInfo);
}

void * __cdecl gmalloc(size_t size) { return GlobalAlloc(GPTR,size+1); }
BOOL CALLBACK WindowEnumProc(HWND hwnd,LPARAM lParam)
{
	if  (((LPSTR*)lParam)[0]!=NULL)
	{
		// Class is specified
		char szClass[200];
		if (GetClassName(hwnd,szClass,200)>0)
		{
			if (!__ContainString(szClass,((LPSTR*)lParam)[0]))
				return TRUE;
		}
		else if (((LPSTR*)lParam)[0][0]!='*' || ((LPSTR*)lParam)[0][1]!='\0')
			return TRUE;
	}

	if  (((LPSTR*)lParam)[1]!=NULL)
	{
		// Title is specified
		char szTitle[400];
		if (GetWindowText(hwnd,szTitle,400)>0)
		{
			if (!__ContainString(szTitle,((LPSTR*)lParam)[1]))
				return TRUE;
		}
		else if (((LPSTR*)lParam)[1][0]!='*' || ((LPSTR*)lParam)[1][1]!='\0')
			return TRUE;
	}


	((HWND*)lParam)[2]=hwnd;
	return FALSE;
}

void CSubAction::DoAdvanced()
{


	// Send/Post Message

	BOOL bFreeWParam=FALSE,bFreeLParam=FALSE;
	
	HWND hWnd=NULL;
	WPARAM wParam=NULL,lParam=NULL;

	if (m_pSendMessage->szWindow[0]=='0')
	{
		if (m_pSendMessage->szWindow[1]=='x' || 
			m_pSendMessage->szWindow[1]=='X')
		{
			// Hex value
			LPSTR szTemp;
			hWnd=(HWND)strtoul(m_pSendMessage->szWindow+2,&szTemp,16);
		}
	}
	else if (strcasecmp(m_pSendMessage->szWindow,"HWND_BROADCAST")==0)
		hWnd=HWND_BROADCAST;
	else if (GetLocateDlg()!=NULL && strcasecmp(m_pSendMessage->szWindow,"LOCATEDLG")==0)
		hWnd=*GetLocateDlg();
	else if (strcasecmp(m_pSendMessage->szWindow,"LOCATEST")==0)
		hWnd=*GetLocateAppWnd();
	else if (strncmp(m_pSendMessage->szWindow,"Find",4)==0)
	{
		int nIndex=FirstCharIndex(m_pSendMessage->szWindow,'(');
		if (nIndex!=-1)
		{
			LPCSTR pText=m_pSendMessage->szWindow+nIndex+1;
			LPSTR pClassAndWindow[3]={NULL,NULL,NULL};
			
			nIndex=FirstCharIndex(pText,',');
			if (nIndex==-1)
			{
				nIndex=FirstCharIndex(pText,')');
				if (nIndex==-1)
					pClassAndWindow[0]=alloccopy(pText);
				else
					pClassAndWindow[0]=alloccopy(pText,nIndex);
			}
			else
			{
				pClassAndWindow[0]=alloccopy(pText,nIndex);
				pText+=nIndex+1;

				nIndex=FirstCharIndex(pText,')');
				pClassAndWindow[1]=alloccopy(pText,nIndex);
			}

			EnumWindows(WindowEnumProc,LPARAM(pClassAndWindow));

			// Third cell is handle to window
			hWnd=(HWND)pClassAndWindow[2];

			delete[] pClassAndWindow[0];
			if (pClassAndWindow[1])
				delete[] pClassAndWindow[1];
		}
	}
	


	// Parse wParam
	if (m_pSendMessage->szWParam!=NULL)
	{
		if (m_pSendMessage->szWParam[0]=='0')
		{
			if (m_pSendMessage->szWParam[1]=='x' || 
				m_pSendMessage->szWParam[1]=='X')
			{
				// Hex value
				LPSTR szTemp;
				wParam=(WPARAM)strtoul(m_pSendMessage->szWParam+2,&szTemp,16);
			}
			else if (m_pSendMessage->szWParam[1]!='\0')
			{
				DWORD dwLength;
				wParam=(WPARAM)dataparser(m_pSendMessage->szWParam,istrlen(m_pSendMessage->szWParam),gmalloc,&dwLength);
				*((BYTE*)wParam+dwLength)=0;
				bFreeWParam=TRUE;
			}
		}
		else if ((wParam=atoi(m_pSendMessage->szWParam))==0)
		{
			DWORD dwLength;
			wParam=(WPARAM)dataparser(m_pSendMessage->szWParam,istrlen(m_pSendMessage->szWParam),gmalloc,&dwLength);
			*((BYTE*)wParam+dwLength)=0;
			bFreeWParam=TRUE;
		}
	}

	// Parse lParam
	if (m_pSendMessage->szLParam!=NULL)
	{
		if (m_pSendMessage->szLParam[0]=='0')
		{
			if (m_pSendMessage->szLParam[1]=='x' || 
				m_pSendMessage->szLParam[1]=='X')
			{
				// Hex value
				LPSTR szTemp;
				lParam=(WPARAM)strtoul(m_pSendMessage->szLParam+2,&szTemp,16);
			}
			else if (m_pSendMessage->szLParam[1]!='\0')
			{
				DWORD dwLength;
				lParam=(WPARAM)dataparser(m_pSendMessage->szLParam,istrlen(m_pSendMessage->szLParam),gmalloc,&dwLength);
				*((BYTE*)lParam+dwLength)=0;
				bFreeLParam=TRUE;
			}
		}
		else if ((lParam=atoi(m_pSendMessage->szLParam))==0)
		{
			DWORD dwLength;
			lParam=(WPARAM)dataparser(m_pSendMessage->szLParam,istrlen(m_pSendMessage->szLParam),gmalloc,&dwLength);
			*((BYTE*)lParam+dwLength)=0;
            bFreeLParam=TRUE;
		}
	}

	if (hWnd!=NULL)
	{
		if (m_nAdvanced==PostMessage)
			::PostMessage(hWnd,m_pSendMessage->nMessage,wParam,lParam);
		else
			::SendMessage(hWnd,m_pSendMessage->nMessage,wParam,lParam);
	}

	if (bFreeWParam)
		GlobalFree((HANDLE)wParam);
	if (bFreeLParam)
		GlobalFree((HANDLE)lParam);

}


CSubAction::SendMessageInfo::SendMessageInfo(SendMessageInfo& rCopyFrom)
{
	CopyMemory(this,&rCopyFrom,sizeof(SendMessageInfo));

	if (szWindow!=NULL)
		szWindow=alloccopy(szWindow);
	if (szWParam!=NULL)
		szWParam=alloccopy(szWParam);
	if (szLParam!=NULL)
		szLParam=alloccopy(szLParam);
}
		
DWORD CSubAction::SendMessageInfo::GetData(BYTE* pData_) const
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

void CShortcut::ModifyMenus(CArrayFP<CShortcut*>& aShortcuts,HMENU hMainMenu,HMENU hSubMenu)
{
	VirtualKeyName* pVirtKeyNames=GetVirtualKeyNames();

	for (int i=0;i<aShortcuts.GetSize();i++)
	{
		// Check whether it is menu item
		for (int a=0;a<aShortcuts[i]->m_apActions.GetSize();a++)
		{
			CAction* pAction=aShortcuts[i]->m_apActions[a];

			if (pAction->m_nAction==CAction::MenuCommand)
			{
				BYTE bSubMenu=CAction::GetMenuAndSubMenu(pAction->m_nMenuCommand);
				HMENU hMenu=GetSubMenu((bSubMenu&128)?hMainMenu:hSubMenu,bSubMenu&~128);

				MENUITEMINFO mii;
				mii.cbSize=sizeof(MENUITEMINFO);
				mii.fMask=MIIM_FTYPE|MIIM_STRING;
				mii.cch=0;
				if (!GetMenuItemInfo(hMenu,LOWORD(pAction->m_nMenuCommand),FALSE,&mii))
					continue;

				if (mii.fType!=MFT_STRING)
					continue;

				CString Title,Key;
				mii.dwTypeData=Title.GetBuffer(mii.cch++);
				if (!GetMenuItemInfo(hMenu,LOWORD(pAction->m_nMenuCommand),FALSE,&mii))
					continue;

				if (Title.FindFirst('\t')!=-1)
					continue;

				aShortcuts[i]->FormatKeyLabel(pVirtKeyNames,Key);
								
				Title << '\t' << Key;
				mii.dwTypeData=Title.GetBuffer();
				
				SetMenuItemInfo(hMenu,LOWORD(pAction->m_nMenuCommand),FALSE,&mii);


			}

		}
	}

	delete[] pVirtKeyNames;
}



BYTE CSubAction::GetMenuAndSubMenu(CAction::ActionMenuCommands nMenuCommand)
{
	switch (HIWORD(nMenuCommand))
	{
	case IDS_SHORTCUTMENUFILENOITEM:
		return SUBMENU_FILEMENUNOITEMS;
	case IDS_SHORTCUTMENUFILEITEM:
		if (nMenuCommand==CAction::FileOpenContainingFolder ||
			nMenuCommand==CAction::FileRemoveFromThisList)
			return SUBMENU_EXTRACONTEXTMENUITEMS;
		else
			return SUBMENU_FILEMENU;
	case IDS_SHORTCUTMENUEDIT:
		return 128|1;
		break;
	case IDS_SHORTCUTMENUVIEW:
	case IDS_SHORTCUTMENUVIEWARRANGEICONS:
		return 128|2;
	case IDS_SHORTCUTMENUOPTIONS:
		return 128|3;		
	case IDS_SHORTCUTMENUHELP:
		return 128|4;
	case IDS_SHORTCUTMENUSPECIAL:
		return SUBMENU_EXTRACONTEXTMENUITEMS;
	case IDS_SHORTCUTMENUPRESETS:
		return SUBMENU_PRESETSELECTION;
	case IDS_SHORTCUTMENUDIRECTORIES:
		return SUBMENU_MULTIDIRSELECTION;
	}

	return 0;
}

void CShortcut::FormatKeyLabel(VirtualKeyName* pVirtualKeyNames,BYTE bKey,BYTE bModifiers,BOOL bScancode,CString& str)
{
	if (bKey==0)
	{
		str.LoadString(IDS_SHORTCUTNONE);
		return;
	}


	// Formatting modifiers
	if (bModifiers&MOD_WIN)
		str.LoadString(IDS_SHORTCUTMODEXT);
	else
		str.Empty();
	if (bModifiers&MOD_CONTROL)
		str.AddString(IDS_SHORTCUTMODCTRL);
	if (bModifiers&MOD_ALT)
		str.AddString(IDS_SHORTCUTMODALT);
	if (bModifiers&MOD_SHIFT)
		str.AddString(IDS_SHORTCUTMODSHIFT);
	
	if (bScancode)
	{
		CString str2;
		str2.Format(IDS_SHORTCUTSCANCODE,(int)bKey);
		str << str2;
		return;
	}

	int i;
	for (i=0;pVirtualKeyNames[i].bKey!=0 && pVirtualKeyNames[i].bKey!=bKey;i++);
	if (pVirtualKeyNames[i].iFriendlyNameId!=0)
	{
		str.AddString(pVirtualKeyNames[i].iFriendlyNameId);
		return;
	}

	BYTE pKeyState[256];
	ZeroMemory(pKeyState,256);

	WORD wChar;
	int nRet=ToAscii(bKey,0,pKeyState,&wChar,0);
	if (nRet==1)
	{
		CharUpperBuff((LPSTR)&wChar,1);
		str << char(wChar);
	}
	else if (nRet==2)
	{
		CharUpperBuff((LPSTR)&wChar,2);
		str << (LPSTR(&wChar))[0] << (LPSTR(&wChar))[0];
	}
	else if (pVirtualKeyNames[i].pName!=NULL)
		str << pVirtualKeyNames[i].pName;
	else
		str << (int) bKey;
}


CShortcut::VirtualKeyName* CShortcut::GetVirtualKeyNames()
{
	VirtualKeyName aVirtualKeys[]={
		{VK_BACK,"VK_BACK",IDS_KEYBACKSPACE},
		{VK_ESCAPE,"VK_ESCAPE",IDS_KEYESC},
		{VK_TAB,"VK_TAB",IDS_KEYTAB},
		{VK_CAPITAL,"VK_CAPITAL",IDS_KEYCAPSLOCK},
		{VK_RETURN,"VK_RETURN",IDS_KEYENTER},
		{VK_SPACE,"VK_SPACE",IDS_KEYSPACE},
		{VK_PRIOR,"VK_PRIOR",IDS_KEYPAGEUP},
		{VK_NEXT,"VK_NEXT",IDS_KEYPAGEDOWN},
		{VK_END,"VK_END",IDS_KEYEND},
		{VK_HOME,"VK_HOME",IDS_KEYHOME},
		{VK_LEFT,"VK_LEFT",IDS_KEYLEFT},
		{VK_UP,"VK_UP",IDS_KEYUP},
		{VK_RIGHT,"VK_RIGHT",IDS_KEYRIGHT},
		{VK_DOWN,"VK_DOWN",IDS_KEYDOWN},
		{VK_SNAPSHOT,"VK_SNAPSHOT",IDS_KEYPRINTSCREEN},
		{VK_SCROLL,"VK_SCROLL",IDS_KEYSCROLLLOCK},
		{VK_PAUSE,"VK_PAUSE",IDS_KEYPAUSE},
		{VK_INSERT,"VK_INSERT",IDS_KEYINS},
		{VK_DELETE,"VK_DELETE",IDS_KEYDEL},
		{VK_NUMLOCK,"VK_NUMLOCK",IDS_KEYNUMLOCK},
		{VK_NUMPAD0,"VK_NUMPAD0",IDS_KEYNUM0},
		{VK_NUMPAD1,"VK_NUMPAD1",IDS_KEYNUM1},
		{VK_NUMPAD2,"VK_NUMPAD2",IDS_KEYNUM2},
		{VK_NUMPAD3,"VK_NUMPAD3",IDS_KEYNUM3},
		{VK_NUMPAD4,"VK_NUMPAD4",IDS_KEYNUM4},
		{VK_NUMPAD5,"VK_NUMPAD5",IDS_KEYNUM5},
		{VK_NUMPAD6,"VK_NUMPAD6",IDS_KEYNUM6},
		{VK_NUMPAD7,"VK_NUMPAD7",IDS_KEYNUM7},
		{VK_NUMPAD8,"VK_NUMPAD8",IDS_KEYNUM8},
		{VK_NUMPAD9,"VK_NUMPAD9",IDS_KEYNUM9},
		{VK_MULTIPLY,"VK_MULTIPLY",IDS_KEYNUMMUL},
		{VK_ADD,"VK_ADD",IDS_KEYNUMADD},
		{VK_SEPARATOR,"VK_SEPARATOR",0},
		{VK_SUBTRACT,"VK_SUBTRACT",IDS_KEYNUMSUB},
		{VK_DECIMAL,"VK_DECIMAL",IDS_KEYNUMDECIMAL},
		{VK_DIVIDE,"VK_DIVIDE",IDS_KEYNUMDIV},
		{VK_F1,"VK_F1",IDS_KEYF1},
		{VK_F2,"VK_F2",IDS_KEYF2},
		{VK_F3,"VK_F3",IDS_KEYF3},
		{VK_F4,"VK_F4",IDS_KEYF4},
		{VK_F5,"VK_F5",IDS_KEYF5},
		{VK_F6,"VK_F6",IDS_KEYF6},
		{VK_F7,"VK_F7",IDS_KEYF7},
		{VK_F8,"VK_F8",IDS_KEYF8},
		{VK_F9,"VK_F9",IDS_KEYF9},
		{VK_F10,"VK_F10",IDS_KEYF10},
		{VK_F11,"VK_F11",IDS_KEYF11},
		{VK_F12,"VK_F12",IDS_KEYF12},
		{VK_F13,"VK_F13",IDS_KEYF13},
		{VK_F14,"VK_F14",IDS_KEYF14},
		{VK_F15,"VK_F15",0},
		{VK_F16,"VK_F16",0},
		{VK_F17,"VK_F17",0},
		{VK_F18,"VK_F18",0},
		{VK_F19,"VK_F19",0},
		{VK_F20,"VK_F20",0},
		{VK_F21,"VK_F21",0},
		{VK_F22,"VK_F22",0},
		{VK_F23,"VK_F23",0},
		{VK_F24,"VK_F24",0}
	};

	VirtualKeyName* pRet=new VirtualKeyName[sizeof(aVirtualKeys)/sizeof(VirtualKeyName)+1];

	int i;
	for (i=0;i<sizeof(aVirtualKeys)/sizeof(VirtualKeyName);i++)
	{
		pRet[i].bKey=aVirtualKeys[i].bKey;
		pRet[i].pName=alloccopy(aVirtualKeys[i].pName);
		pRet[i].iFriendlyNameId=aVirtualKeys[i].iFriendlyNameId;
	}
	pRet[i].bKey=0;
	pRet[i].pName=NULL;
	pRet[i].iFriendlyNameId=0;
    return pRet;
}

int CSubAction::GetActivateTabsActionLabelStringId(CAction::ActionActivateTabs uSubAction)
{
	switch (uSubAction)
	{
	case NameAndLocationTab:
		return IDS_NAME;
	case SizeAndDataTab:
		return IDS_SIZEDATE;
	case AdvancedTab:
		return IDS_ADVANCED;
	case NextTab:
		return IDS_KEYNEXTTAB;
	case PrevTab:
		return IDS_KEYPREVTAB;
	default:
		return 0;
	}
}
	
int CSubAction::GetShowHideDialogActionLabelStringId(CAction::ActionShowHideDialog uSubAction)
{
	switch (uSubAction)
	{
	case ShowDialog:
		return IDS_ACTIONSWDIALOGSHOW;
	case MinimizeDialog:
		return IDS_ACTIONSWDIALOGMINIMINZE;
	case CloseDialog:
		return IDS_ACTIONSWDIALOGCLOSE;
	case ShowOrHideDialog:
		return IDS_ACTIONSWDIALOGSHOWORHIDE;
	case OpenOrCloseDialog:
		return IDS_ACTIONSWDIALOGOPENORCLOSE;
	case RestoreDialog:
		return IDS_ACTIONSWDIALOGRESTORE;
	case MaximizeDialog:
		return IDS_ACTIONSWDIALOGMAXIMIZE;
	case MaximizeOrRestoreDialog:
		return IDS_ACTIONSWDIALOMAXIMIZEORRESTORE;
	default:
		return 0;
	}
}

int CSubAction::GetResultItemActionLabelStringId(CAction::ActionResultList uSubAction)
{
	switch (uSubAction)
	{
	case Execute:
		return IDS_ACTIONRESITEMEXECUTE;
	case Copy:
		return IDS_ACTIONRESITEMCOPY;
	case Cut:
		return IDS_ACTIONRESITEMCUT;
	case MoveToRecybleBin:
		return IDS_ACTIONRESITEMRECYCLE;
	case DeleteFile:
		return IDS_ACTIONRESITEMDELETE;
	case OpenContextMenu:
		return IDS_ACTIONRESITEMOPENCONTEXTMENU;
	case OpenContextMenuSimple:
		return IDS_ACTIONRESITEMOPENCONTEXTMENUSIMPLE;
	case OpenFolder:
		return IDS_ACTIONRESITEMOPENFOLDER;
	case OpenContainingFolder:
		return IDS_ACTIONRESITEMOPENCONTFOLDER;
	case Properties:
		return IDS_ACTIONRESITEMPROPERTIES;
	case ShowSpecialMenu:
		return IDS_ACTIONRESITEMSPECIALMENU;
	case ExecuteCommand:
		return IDS_ACTIONRESITEMEXECUTECOMMAND;
	case SelectFile:
		return IDS_ACTIONRESITEMSELECTFILE;
	default:
		return 0;
	}
}

int CSubAction::GetAdvancedActionStringLabelId(CAction::ActionAdvanced uSubAction)
{
	switch (uSubAction)
	{
	case CAction::SendMessage:
		return IDS_ACTIONADVSENDMESSAGE;
	case CAction::PostMessage:
		return IDS_ACTIONADVPOSTMESSAGE;
	default:
		return 0;
	}
}

#endif

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

BOOL CShortcut::IsWhenAndWhereSatisfied(HWND hSystemTrayWnd)  const
{
	if ((m_dwFlags&sfKeyTypeMask)==sfLocal)
	{
#ifndef KEYHOOK_EXPORTS
		CLocateDlg* pLocateDlg=GetLocateDlg();
		if (pLocateDlg!=NULL)
		{
			switch (pLocateDlg->m_pTabCtrl->GetCurSel())
			{
			case 0: // Name tab
				if (m_wWhenPressed&wpNameTabShown)
					return TRUE;
				break;
			case 1: // Size and Date tab
				if (m_wWhenPressed&wpSizeDateTabShown)
					return TRUE;
				break;
			case 2: // Advanced tab
				if (m_wWhenPressed&wpAdvancedTabShown)
					return TRUE;
				break;
			}
		}
#endif
		return FALSE;
	}

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
