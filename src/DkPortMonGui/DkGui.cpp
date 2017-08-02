#include "DkGui.h"

CDkWind				g_MainWind;
CDkPortClient		g_DkPortClient;
TCHAR				g_SzSelPort[256];
HINSTANCE			g_hInst;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	DkInitializeComponents();

	g_hInst = hInstance;

	if (g_MainWind.Create(hInstance, 100, 200, 500, 300)){

		g_MainWind.Show(nCmdShow);

		MSG msg;
		while (::GetMessage(&msg, NULL, 0, 0) > 0){
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	} else {
		::MessageBox(NULL, L"Error create window!", L"Error", MB_OK | MB_ICONERROR);
	}

	return 0;
}

void DkInitializeComponents()
{
	g_MainWind						= CDkWind();
	g_MainWind.OnStartMenu			= DkWindOnStartMenu;
	g_MainWind.OnStopMenu			= DkWindOnStopMenu;
	g_MainWind.OnClose				= DkWindOnClose;
	g_MainWind.OnClearMenu			= DkWindOnClearMenu;
	g_MainWind.OnSelectPortMenu		= DkWindOnSelectPortMenu;

	g_DkPortClient					= CDkPortClient();
	g_DkPortClient.OnDataReceived	= DkPortOnDataReceived;

	RtlFillMemory(&g_SzSelPort[0], sizeof(g_SzSelPort), '\0');
}

void DkWindOnStartMenu()
{
	TCHAR		buf[512];

	if (g_DkPortClient.Start(g_SzSelPort)){
		::MessageBox(g_MainWind.GetHandle(), TEXT("Monitor started."), TEXT("Info"),
			MB_OK | MB_ICONINFORMATION);

		RtlFillMemory(&buf[0], sizeof(buf), '\0');
		::swprintf_s(buf, sizeof(buf)/sizeof(TCHAR), 
			TEXT("Start monitor on port %s"), g_SzSelPort);
		g_MainWind.AddListBoxItem(buf);

	} else {
		::MessageBox(g_MainWind.GetHandle(), TEXT("Error start monitor!"),
			TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	
}

void DkWindOnStopMenu()
{
	TCHAR		buf[512];

	if (g_DkPortClient.Stop()){
		::MessageBox(g_MainWind.GetHandle(), TEXT("Monitor stopped."), TEXT("Info"),
			MB_OK | MB_ICONINFORMATION);

		RtlZeroMemory(&buf[0], sizeof(buf));
		::swprintf_s(buf, sizeof(buf)/sizeof(TCHAR), 
			TEXT("Stop monitor on port %s"), g_SzSelPort);
		g_MainWind.AddListBoxItem(buf);

	} else {
		::MessageBox(g_MainWind.GetHandle(), TEXT("Error stop monitor!"),
			TEXT("Error"), MB_OK | MB_ICONERROR);
	}	
}

void DkWindOnClose(bool &bCanClose)
{
	if (g_DkPortClient.IsActive()){
		::MessageBox(g_MainWind.GetHandle(),
			TEXT("Monitor still active!"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return;
	}

	if (::MessageBox(g_MainWind.GetHandle(), TEXT("Do you want to quit?"),
		TEXT("Confirmation"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES){
			bCanClose = true;
	}
}

void DkPortOnDataReceived()
{
	TCHAR	szFuncName[256];
	TCHAR	szDatLen[128];
	TCHAR	szData[4096];
	TCHAR	*pTmp = NULL;

	RtlFillMemory(szFuncName, sizeof(szFuncName), '\0');
	::swprintf_s(szFuncName, sizeof(szFuncName)/sizeof(TCHAR), TEXT("Request: %s"),
		g_DkPortClient.m_Dat.StrFuncName);

	RtlFillMemory(szDatLen, sizeof(szDatLen), '\0');
	::swprintf_s(szDatLen, sizeof(szDatLen)/sizeof(TCHAR), TEXT("Data length: %d byte(s)"), 
		g_DkPortClient.m_Dat.DataLen);

	RtlFillMemory(szData, sizeof(szData), '\0');
	pTmp = &szData[0];
	*pTmp++ = L'D'; *pTmp++ = L'a'; *pTmp++ = L't'; 
	*pTmp++ = L'a'; *pTmp++ = L':'; *pTmp++ = L' ';
	for (ULONG ul = 0; ul < g_DkPortClient.m_Dat.DataLen; ul++){
		
		if (ul >= sizeof(szData)-8)
			break;

		*pTmp++ = HexWChar[g_DkPortClient.m_Dat.Data[ul] >> 4 ];
		*pTmp++ = HexWChar[g_DkPortClient.m_Dat.Data[ul] & 0xF ];
		*pTmp++ = L' ';
	}

	g_MainWind.AddListBoxItem(TEXT("-----------------------------------------------------------------------------------------"));
	g_MainWind.AddListBoxItem(szFuncName);
	if (g_DkPortClient.m_Dat.DataLen > 0){
		g_MainWind.AddListBoxItem(szDatLen);
		g_MainWind.AddListBoxItem(szData);
	}
	g_MainWind.AddListBoxItem(TEXT("-----------------------------------------------------------------------------------------"));
}

void DkWindOnClearMenu()
{
	g_MainWind.ListBoxClear();
}

void DkWindOnSelectPortMenu()
{
	::DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SELECT), g_MainWind.GetHandle(), SelectBoxProc);
}

DWORD DkEnumPortAndAddToListBox(HWND hPar)
{
	const TCHAR * pRegEntry = TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM");

	HKEY		hKey;
	LSTATUS		lStat = 0;
	DWORD		dwIdx = 0, dwValNameLen = 0, dwDatLen = 0, dwType = REG_SZ;
	TCHAR		szValName[128];
	UCHAR		btDat[256];
	LRESULT		lRes;
	
	HWND hList = ::GetDlgItem(hPar, IDC_PORTLIST);
	
	if (hList){
		lStat = ::RegOpenKey(HKEY_LOCAL_MACHINE, pRegEntry, &hKey);
		if (lStat != ERROR_SUCCESS){
			::MessageBox(g_MainWind.GetHandle(), 
				TEXT("Error opening registry!"), TEXT("Error"),
				MB_OK | MB_ICONERROR);
			return 0;
		}

		for (;;){
			RtlFillMemory(szValName, sizeof(szValName), '\0');
			RtlFillMemory(btDat, sizeof(btDat), '\0');
			dwValNameLen = sizeof(szValName)/sizeof(WCHAR);
			dwDatLen = sizeof(btDat);
			dwType = REG_SZ;
			lStat = ::RegEnumValue(hKey, dwIdx, szValName, &dwValNameLen, NULL,
				&dwType, btDat, &dwDatLen);
			if (lStat == ERROR_NO_MORE_ITEMS){
				break;
			} else if (lStat != ERROR_SUCCESS) {
				::MessageBox(g_MainWind.GetHandle(), TEXT("Error enumerate registry!"),
					TEXT("Error"), MB_OK | MB_ICONERROR);
				break;
			} else {
				dwIdx++;
				lRes = ::SendMessage(hList, LB_ADDSTRING, 0, (LPARAM) &szValName[0]);
			}
		}

		::RegCloseKey(hKey);	
	}

	return dwIdx;
}

INT_PTR CALLBACK SelectBoxProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND		hList;
	LRESULT		lRes;
	static		DWORD dwPortNumb = 0;
	
	UNREFERENCED_PARAMETER(lParam);
	
	switch (message)
	{
	case WM_INITDIALOG:
		dwPortNumb = DkEnumPortAndAddToListBox(hDlg);
		if (dwPortNumb == 0){
			::MessageBox(hDlg, 
				TEXT("There no serial port detected in this computer!"),
				TEXT("Warning!"), MB_OK | MB_ICONWARNING);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			hList = ::GetDlgItem(hDlg, IDC_PORTLIST);
			if (hList){
				lRes = ::SendMessage(hList, LB_GETCURSEL, 0, 0);
				if (lRes >= 0){
					RtlFillMemory(&g_SzSelPort[0], sizeof(g_SzSelPort), '\0');
					::SendMessage(hList, LB_GETTEXT, (WPARAM) lRes, (LPARAM) &g_SzSelPort[0]);
					::EndDialog(hDlg, LOWORD(wParam));
				} else {
					if (dwPortNumb > 0){
						::MessageBox(hDlg, 
							TEXT("Please select a port name!"),
							TEXT("Error"), MB_OK | MB_ICONERROR);
					} else {
						::EndDialog(hDlg, LOWORD(wParam));
					}
				}
			}
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
