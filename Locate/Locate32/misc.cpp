#include <HFCLib.h>
#include "Locate32.h"
#include <imapi.h>


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

