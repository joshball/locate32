#include <HFCLib.h>
#include "Locate32.h"
#include <imapi.h>
#include <uxtheme.h>
#include <tmschema.h>

#define BUTTON_WIDTHORIG		18
#define BUTTON_WIDTHTHEME		13

#define IDC_EXPLICITIDATE		100
#define IDC_RELATIVEDATE		101
#define IDC_RELATIVEDATESPIN	102


BOOL GetIMAPIBurningDevices(CArray<LPWSTR>& aDevicePaths)
{
	CRegKey RegKey;
	if (RegKey.OpenKey(HKCR,"IMAPI.MSDiscMasterObj\\CLSID",CRegKey::defRead)!=ERROR_SUCCESS)
		return FALSE;

	WCHAR szCLSID[50];
	if (RegKey.QueryValue(L"",szCLSID,50)==0)
		return FALSE;

	CLSID clsid;
	if (CLSIDFromString(szCLSID,&clsid)!=NO_ERROR)
		return FALSE;

	
	HRESULT hRes;
	IDiscMaster* pdm;
	hRes=CoCreateInstance(clsid,NULL,CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER,IID_IDiscMaster,(void**)&pdm);
	if (FAILED(hRes))
		return FALSE;

	hRes=pdm->Open();
	if (FAILED(hRes))
	{
		pdm->Release();
		return FALSE;
	}

	IEnumDiscRecorders* pedr;
	hRes=pdm->EnumDiscRecorders(&pedr);
	if (SUCCEEDED(hRes))
	{
		IDiscRecorder* pdr;
		DWORD dwReturned;
		while ((hRes=pedr->Next(1,&pdr,&dwReturned))==S_OK)
		{
			BSTR bPath;
			hRes=pdr->GetPath(&bPath);
			if (SUCCEEDED(bPath))
			{
				if (bPath[0]=='\\')
				{
					WCHAR szName[MAX_PATH];
					WCHAR szTemp[MAX_PATH]=L"";
					WCHAR drive[]=L" :";
					GetLogicalDriveStringsW(MAX_PATH,szTemp);

					LPWSTR pPtr=szTemp;
					while (*pPtr!='\0')
					{
						*drive=*pPtr;
						if (QueryDosDeviceW(drive, szName,MAX_PATH))
						{
							if (wcscmp(szName,bPath)==0)
								aDevicePaths.Add(alloccopy(pPtr));
						}

						pPtr+=istrlenw(pPtr)+1;
					}
				}
				else
					aDevicePaths.Add(alloccopy(bPath));
			}

			pdr->Release();
		}
		pedr->Release();
	}


	pdm->Close();
	pdm->Release();

	return TRUE;
}




CDateTimeCtrlEx::CDateTimeCtrlEx(HWND hWnd)
:	CDateTimeCtrl(hWnd),m_hTimePickerWnd(NULL),m_hEditWnd(NULL),
	m_hSpinWnd(NULL),m_hTheme(NULL),m_pDrawThemeBackground(NULL),
	m_bDeleteOnDestroy(FALSE),m_dwFlags(ModeExplicit|Normal)
{
	m_hUxTheme=LoadLibrary("uxtheme.dll");
	if (m_hUxTheme!=NULL)
	{
		m_pDrawThemeBackground=(HRESULT(STDAPICALLTYPE *)(HTHEME,HDC,int,int,const RECT*,const RECT*))
			GetProcAddress(m_hUxTheme,"DrawThemeBackground");


		HTHEME(STDAPICALLTYPE * pOpenThemeData)(HWND,LPCWSTR)=(HTHEME(STDAPICALLTYPE*)(HWND,LPCWSTR))
			GetProcAddress(m_hUxTheme,"OpenThemeData");
		if (pOpenThemeData!=NULL)
		{
			m_hTheme=pOpenThemeData(hWnd,L"COMBOBOX");
		}
	}
}

void CDateTimeCtrlEx::CreateControls()
{
	RECT rcClientRect;
	GetClientRect(&rcClientRect);
	DWORD dwStyle=WS_GROUP | WS_TABSTOP | WS_CHILD;
	if (GetStyle()&WS_DISABLED)
		dwStyle|=WS_DISABLED;

	if (m_hTheme!=NULL && m_pDrawThemeBackground!=NULL)
		rcClientRect.right-=BUTTON_WIDTHTHEME;
	else
		rcClientRect.right-=BUTTON_WIDTHORIG;

	if (m_hTimePickerWnd==NULL)
	{
		m_hTimePickerWnd=CreateWindowEx(WS_EX_NOPARENTNOTIFY|WS_EX_CLIENTEDGE,
			"SysDateTimePick32","",dwStyle|WS_VISIBLE,
			rcClientRect.left,rcClientRect.top,rcClientRect.right,rcClientRect.bottom,
			*this,(HMENU)IDC_EXPLICITIDATE,GetInstanceHandle(),NULL);
	}
	if (m_hEditWnd==NULL)
	{
		m_hEditWnd=CreateWindowEx(WS_EX_NOPARENTNOTIFY|WS_EX_CLIENTEDGE|WS_EX_RIGHT,
			"EDIT","",dwStyle|ES_AUTOHSCROLL|ES_NUMBER,
			rcClientRect.left,rcClientRect.top,rcClientRect.right,rcClientRect.bottom,
			*this,(HMENU)IDC_RELATIVEDATE,GetInstanceHandle(),NULL);
	}
	if (m_hSpinWnd==NULL)
	{
		m_hSpinWnd=CreateWindowEx(WS_EX_NOPARENTNOTIFY|WS_EX_CLIENTEDGE|WS_EX_RIGHT,
			"msctls_updown32","",dwStyle|UDS_ALIGNRIGHT|UDS_ARROWKEYS,
			rcClientRect.left,rcClientRect.top,rcClientRect.right,rcClientRect.bottom,
			*this,(HMENU)IDC_RELATIVEDATESPIN,GetInstanceHandle(),NULL);
		::SendMessage(m_hSpinWnd,UDM_SETBUDDY,WPARAM(m_hEditWnd),0);
		::SendMessage(m_hSpinWnd,UDM_SETRANGE,0,1000);
	}

	
	SetRelativeDate(0,DTXF_NOMODECHANGE);
}


CDateTimeCtrlEx::~CDateTimeCtrlEx()
{
	if (m_hUxTheme!=NULL)
	{
		HRESULT(STDAPICALLTYPE *pCloseThemeData)(HTHEME)=(HRESULT(STDAPICALLTYPE *)(HTHEME))GetProcAddress(m_hUxTheme,"CloseThemeData");
		if (pCloseThemeData!=NULL)
		{
			if (m_hTheme!=NULL)     
				pCloseThemeData(m_hTheme);
		}
		FreeLibrary(m_hUxTheme);
	}
}


void CDateTimeCtrlEx::OnPaint()
{
	CPaintDC dc(this);

	CRect rcRect;
	GetClientRect(&rcRect);
	
	if (m_pDrawThemeBackground!=NULL && m_hTheme!=NULL)
	{
		rcRect.left=rcRect.right-BUTTON_WIDTHTHEME;

		int nState=CBXS_NORMAL;
		if (GetStyle()&WS_DISABLED)
			nState=CBXS_DISABLED;
		else if (m_dwFlags&Hot)
			nState=CBXS_HOT;
		else if (m_dwFlags&Pressed)
			nState=CBXS_PRESSED;
		m_pDrawThemeBackground(m_hTheme,dc,CP_DROPDOWNBUTTON,nState,&rcRect,NULL);
		return;
	}

	rcRect.left=rcRect.right-BUTTON_WIDTHORIG+1;
	rcRect.top++;
	rcRect.right--;
	rcRect.bottom-=2;

	UINT nState=DFCS_SCROLLDOWN;
	if (GetStyle()&WS_DISABLED)
		nState|=DFCS_INACTIVE;
	else if (m_dwFlags&Pressed)
		nState|=DFCS_PUSHED;

	DrawFrameControl(dc,&rcRect,DFC_SCROLL,nState);
}

void CDateTimeCtrlEx::ChangeMode(BOOL bToRelativeMode)
{
	if (!bToRelativeMode)
	{
		if ((m_dwFlags&ModeMask)==ModeExplicit)
			return;

		::ShowWindow(m_hTimePickerWnd,SW_SHOW);
		::ShowWindow(m_hEditWnd,SW_HIDE);
		::ShowWindow(m_hSpinWnd,SW_HIDE);

		::SetFocus(m_hTimePickerWnd);

		m_dwFlags&=~ModeMask;
		m_dwFlags|=ModeExplicit;
	}
	else // mode==Relative
	{
		if ((m_dwFlags&ModeMask)==ModeRelative)
			return;

		::ShowWindow(m_hTimePickerWnd,SW_HIDE);
		::ShowWindow(m_hEditWnd,SW_SHOW);
		::ShowWindow(m_hSpinWnd,SW_SHOW);

		::SetFocus(m_hEditWnd);

		m_dwFlags&=~ModeMask;
		m_dwFlags|=ModeRelative;
	}
}

	
LRESULT CALLBACK CDateTimeCtrlEx::WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	CDateTimeCtrlEx* pData;
	if (msg==WM_CREATE)
	{
		CREATESTRUCT* pCreateStruct=(CREATESTRUCT*)lParam;
		if (pCreateStruct->lpCreateParams==NULL)
		{
			pData=new CDateTimeCtrlEx(hWnd);
			if (pData==NULL)
			{
				SetHFCError(HFC_CANNOTALLOC);
				return FALSE;
			}
		}
		else
		{
			pData=(CDateTimeCtrlEx*)pCreateStruct->lpCreateParams;
			pData->SetHandle(hWnd);
		}
		
		::SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)pData);

		pData->CreateControls();		
		return FALSE;
	}

	pData=(CDateTimeCtrlEx*)::GetWindowLongPtr(hWnd,GWLP_USERDATA);	
	if (pData==NULL)
		return DefWindowProc(hWnd,msg,wParam,lParam);


	switch (msg)
	{
	case WM_DESTROY:
		delete pData;
		::SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)NULL);
		return 0;
	case WM_ENABLE:
		::EnableWindow(pData->m_hTimePickerWnd,(BOOL)wParam);
		::EnableWindow(pData->m_hEditWnd,(BOOL)wParam);
		::EnableWindow(pData->m_hSpinWnd,(BOOL)wParam);
		pData->Invalidate();
		break;	
	case DTM_GETSYSTEMTIME:
		if ((pData->m_dwFlags&ModeMask)==ModeExplicit)
		{
			wParam&=~DTXF_FORSAVE;
			return ::SendMessage(pData->m_hTimePickerWnd,msg,wParam,lParam);
		}
		else if (wParam&DTXF_FORSAVE)
		{
			((SYSTEMTIME*)lParam)->wYear=0xFFFF;
			((SYSTEMTIME*)lParam)->wMonth=0xFFFF;
			((SYSTEMTIME*)lParam)->wDay=pData->GetRelativeDate();
			return GDT_VALID|DTXF_FORSAVE;
		}
		else
		{
			GetLocalTime((SYSTEMTIME*)lParam);
			CTime::DecreaseDaysInSystemTime((SYSTEMTIME*)lParam,pData->GetRelativeDate());
			return GDT_VALID;
		}
	case DTM_SETSYSTEMTIME:
		if (((SYSTEMTIME*)lParam)->wYear==0xFFFF && ((SYSTEMTIME*)lParam)->wMonth==0xFFFF)
		{
			// Relative
			pData->ChangeMode(TRUE);
			pData->SetRelativeDate(((SYSTEMTIME*)lParam)->wDay,0);
			return 1;
		}
		else
		{
			pData->ChangeMode(FALSE);
			return ::SendMessage(pData->m_hTimePickerWnd,msg,wParam,lParam);
		}
	case DTM_GETRANGE:
	case DTM_SETRANGE:
	case DTM_SETFORMATA:
	case DTM_SETFORMATW:
	case DTM_SETMCCOLOR:
	case DTM_GETMCCOLOR:
	case DTM_GETMONTHCAL:
	case DTM_SETMCFONT:
	case DTM_GETMCFONT:
		return ::SendMessage(pData->m_hTimePickerWnd,msg,wParam,lParam);
	case WM_PAINT:
		pData->OnPaint();
		break;
	case WM_MOUSEMOVE:
		if ((pData->m_dwFlags&ButtonStateMask)==Normal)
		{
			pData->m_dwFlags&=~ButtonStateMask;
			pData->m_dwFlags|=Hot;
			
			pData->Invalidate();
			pData->SetCapture();
		}
		else if ((pData->m_dwFlags&ButtonStateMask)==Hot)
		{
			CRect rcButtonArea;
			pData->GetClientRect(&rcButtonArea);
			if (pData->m_hTheme!=NULL && pData->m_pDrawThemeBackground!=NULL)
				rcButtonArea.left=rcButtonArea.right-BUTTON_WIDTHTHEME;
			else
				rcButtonArea.left=rcButtonArea.right-BUTTON_WIDTHORIG;


			if (!rcButtonArea.IsPtInRect(LOWORD(lParam),HIWORD(lParam)))
			{
				ReleaseCapture();
				pData->m_dwFlags&=~ButtonStateMask;
				pData->m_dwFlags|=Normal;
				pData->Invalidate();
			}
		}		
		return 0;
	case WM_LBUTTONDOWN:
		{
			if ((pData->m_dwFlags&ButtonStateMask)==Hot)
				ReleaseCapture();
			pData->m_dwFlags&=~ButtonStateMask;
			pData->m_dwFlags|=Pressed;
			pData->Invalidate();
	
			RECT rcWindowRect;
			::GetWindowRect(hWnd,&rcWindowRect);
			
			CMenu PopupMenus;
			PopupMenus.LoadMenu(IDR_POPUPMENU);
			
			if ((pData->m_dwFlags&ModeMask)==ModeExplicit)
				PopupMenus.CheckMenuItem(IDM_EXPLICITDATE,MF_CHECKED|MF_BYCOMMAND);
			else
				PopupMenus.CheckMenuItem(IDM_RELATIVEDATE,MF_CHECKED|MF_BYCOMMAND);


			int nCmd=TrackPopupMenu(PopupMenus.GetSubMenu(SUBMENU_DATETIMEPICKEXMENU),
				TPM_RIGHTALIGN|TPM_TOPALIGN|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD,
				rcWindowRect.right,rcWindowRect.bottom,0,hWnd,NULL);
			PopupMenus.DestroyMenu();

			switch (nCmd)
			{
			case IDM_EXPLICITDATE:
				pData->ChangeMode(FALSE);
				break;
			case IDM_RELATIVEDATE:
				pData->ChangeMode(TRUE);
				break;
			}			

			pData->m_dwFlags&=~ButtonStateMask;
			pData->m_dwFlags|=Normal;
			pData->Invalidate();
			
			return 0;
		}
		
	case WM_LBUTTONUP:
		if ((pData->m_dwFlags&ButtonStateMask)==Pressed)
		{
			ReleaseCapture();
			pData->m_dwFlags&=~ButtonStateMask;
			pData->m_dwFlags|=Normal;
			pData->Invalidate();
		}
		return 0;
	case WM_SETFONT:
		if (pData!=NULL)
		{
			::SendMessage(pData->m_hTimePickerWnd,WM_SETFONT,wParam,lParam);
			::SendMessage(pData->m_hEditWnd,WM_SETFONT,wParam,lParam);
			::SendMessage(pData->m_hSpinWnd,WM_SETFONT,wParam,lParam);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RELATIVEDATE:
			switch (HIWORD(wParam))
			{
			case EN_KILLFOCUS:
				pData->SetRelativeDate((int)::SendMessage(pData->m_hSpinWnd,UDM_GETPOS32,0,0),
					DTXF_NOSPINCHANGE|DTXF_NOMODECHANGE);
				break;
			case EN_CHANGE:
				if (!(pData->m_dwFlags&SpinBoxIsUpdating))
				{
					int nVal=0;
					int nLen=(int)::SendMessage(pData->m_hEditWnd,WM_GETTEXTLENGTH,0,0);
						
					if (nLen>0)
					{
						if (IsUnicodeSystem())
						{
							WCHAR* pText=new WCHAR[nLen+1];
							::SendMessageW(pData->m_hEditWnd,WM_GETTEXT,nLen+1,(LPARAM)pText);
							nVal=GetValueFromText(pText);
							delete[] pText;
						}
						else
						{
							char* pText=new char[nLen+1];
							::SendMessage(pData->m_hEditWnd,WM_GETTEXT,nLen+1,(LPARAM)pText);
							nVal=GetValueFromText(A2W(pText));
							delete[] pText;
						}
						

					}
					::SendMessage(pData->m_hSpinWnd,UDM_SETPOS32,0,nVal);
				}
				break;
			}
			break;
		}
		break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom==IDC_RELATIVEDATESPIN && 
			((LPNMHDR)lParam)->code==UDN_DELTAPOS)
		{
			LPNMUPDOWN pUD=(LPNMUPDOWN)lParam;
			int nNewPos=pUD->iPos+pUD->iDelta;
			
			if (nNewPos<0)
			{
				pUD->iDelta=-pUD->iPos;
				nNewPos=0;
			}

			pData->m_dwFlags|=SpinBoxIsUpdating;

			//::SendMessage(pData->m_hEditWnd,WM_SETTEXT,0,(LPARAM)szEmpty);
			pData->SetRelativeDate(nNewPos,DTXF_NOSPINCHANGE|DTXF_NOMODECHANGE);			

			pData->m_dwFlags&=~SpinBoxIsUpdating;
		}
		break;
	case DTMX_SETRELDATE:
		// wParam is new pos, lparam contains flags (DTXF_*)
		pData->SetRelativeDate((int)wParam,(DWORD)lParam);
		break;
	case DTMX_GETRELDATE:
		return pData->GetRelativeDate();
	case DTMX_GETCLASS:
		return (LRESULT)pData;
	case DTMX_CHANGEMODE:
		pData->ChangeMode((BOOL)wParam);
		break;
	case DTMX_GETMODE:
		return pData->GetMode();
	case WM_SETFOCUS:
		if ((pData->m_dwFlags&ModeMask)==ModeExplicit)
			::SetFocus(pData->m_hTimePickerWnd);
		else
		{
			::SetFocus(pData->m_hEditWnd);
			::SendMessage(pData->m_hEditWnd,EM_SETSEL,0,-1);
		}

		return 0;
	}
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

int CDateTimeCtrlEx::GetRelativeDate() const
{
	if ((m_dwFlags&ModeMask)==ModeExplicit)
		return -1;

	return (int)::SendMessage(m_hSpinWnd,UDM_GETPOS32,0,0);
}
	

void CDateTimeCtrlEx::SetRelativeDate(int nNewPos,DWORD dwFlags)
{
	if (!(dwFlags&DTXF_NOMODECHANGE))
		ChangeMode(TRUE);

	WCHAR szStringBuffer[100];
	int nLen=-1;
	if (nNewPos==0)
		LoadString(IDS_DATETODAY,szStringBuffer,100);
	else if (nNewPos==1)
		LoadString(IDS_DATEDAY,szStringBuffer,100);
	else
	{
		if (_itow_s(nNewPos,szStringBuffer,100,10)!=0)
			szStringBuffer[0]='\0';
		nLen=istrlen(szStringBuffer);
		
		if (nLen+1<100)
		{
			szStringBuffer[nLen]=L' ';
			LoadString(IDS_DATENDAYS,szStringBuffer+nLen+1,100-nLen-1);
		}
	}

	if (IsUnicodeSystem())
		::SendMessageW(m_hEditWnd,WM_SETTEXT,0,(LPARAM)szStringBuffer);
	else
		::SendMessage(m_hEditWnd,WM_SETTEXT,0,(LPARAM)(LPCSTR)W2A(szStringBuffer));

	//::SendMessage(m_hEditWnd,EM_SETSEL,0,nLen);

	if (!(dwFlags&DTXF_NOSPINCHANGE))
		::SendMessage(m_hSpinWnd,UDM_SETPOS32,0,nNewPos);
}

BOOL RegisterDataTimeExCltr()
{
	WNDCLASSEX wc;
	ZeroMemory(&wc,sizeof(WNDCLASSEX));
	wc.cbSize=sizeof(WNDCLASSEX);
	wc.style=CS_DBLCLKS|CS_VREDRAW|CS_HREDRAW|CS_PARENTDC;
	wc.hInstance=GetInstanceHandle();
	wc.lpszClassName="DATETIMEPICKEX";
	wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
	wc.lpfnWndProc=CDateTimeCtrlEx::WndProc;
	if (RegisterClassEx(&wc)==NULL)
		return FALSE;

	ZeroMemory(&wc,sizeof(WNDCLASSEX));
	wc.cbSize=sizeof(WNDCLASSEX);
	wc.style=CS_DBLCLKS|CS_VREDRAW|CS_HREDRAW|CS_PARENTDC;
	wc.hInstance=GetResourceHandle(LanguageSpecificResource);
	wc.lpszClassName="DATETIMEPICKEX";
	wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
	wc.lpfnWndProc=CDateTimeCtrlEx::WndProc;
	return RegisterClassEx(&wc)!=NULL;
}