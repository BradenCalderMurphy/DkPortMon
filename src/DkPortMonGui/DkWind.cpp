#include "DkWind.h"

CDkWind::CDkWind(void): m_hWnd(NULL), m_hInst(NULL), 
m_szClasName(L"DkWindClass")
{
	m_ListBox = CDkListBox();
	::RtlZeroMemory(m_szCaption, DKWIND_MAX_CAP_SZ*2);
	::StringCchCopyW((STRSAFE_LPWSTR) m_szCaption, DKWIND_MAX_CAP_SZ, L"DkPortMon");

	OnStartMenu			= NULL;
	OnStopMenu			= NULL;
	OnClose				= NULL;
	OnClearMenu			= NULL;
	OnSelectPortMenu	= NULL;
}

CDkWind::~CDkWind(void)
{
	Destroy();
}

bool CDkWind::Register()
{
	WNDCLASSEX wcx		= {0};
	wcx.cbClsExtra		= 0;
	wcx.cbSize			= sizeof(WNDCLASSEX);
	wcx.cbWndExtra		= 0;
	wcx.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcx.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
	wcx.hIcon			= ::LoadIcon(NULL, IDI_APPLICATION);
	wcx.hIconSm			= ::LoadIcon(NULL, IDI_APPLICATION);
	wcx.hInstance		= m_hInst;
	wcx.lpfnWndProc		= (WNDPROC) CDkWind::DkWindProc;
	wcx.lpszClassName	= m_szClasName;
	wcx.lpszMenuName	= MAKEINTRESOURCE(IDC_MAIN_MENU);
	wcx.style			= CS_HREDRAW | CS_VREDRAW;
	
	ATOM res = ::RegisterClassEx(&wcx);
	return (res > 0) ? true : false;
}

void CDkWind::Unregister()
{
	::UnregisterClass(m_szClasName, m_hInst);
}

bool CDkWind::Create(HINSTANCE hInst, int iLeft, int iTop, int iWidht, int iHeight)
{
	m_hInst = hInst;
	if (!Register())
		return false;

	m_hWnd = ::CreateWindowEx(0, m_szClasName, m_szCaption,
		WS_OVERLAPPEDWINDOW, iLeft, iTop, iWidht, iHeight, NULL, NULL, m_hInst,
		reinterpret_cast<LPVOID>(this));
	if (m_hWnd == NULL){
		Unregister();
		return false;
	}
	
	InitializeChilds();

	return true;
}

bool CDkWind::Show(int nShowCmd)
{
	::ShowWindow(m_hWnd, nShowCmd);
	::UpdateWindow(m_hWnd);
	return true;
}

bool CDkWind::Hide()
{
	if (!::ShowWindow(m_hWnd, SW_HIDE))
		return false;
	return true;
}

void CDkWind::Destroy()
{
	if (m_hWnd){
		::DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
	Unregister();
	m_hInst = NULL;

	OnStartMenu = NULL;
	OnStopMenu = NULL;
	OnClose = NULL;
}

bool CDkWind::ClientRect(LPRECT pRect)
{
	RECT rc;
	if (!::GetClientRect(m_hWnd, &rc))
		return false;
	*pRect = rc;
	return true;
}

HWND CDkWind::GetHandle() { return m_hWnd; }

void CDkWind::InitializeChilds()
{
	RECT rc;
	
	ClientRect(&rc);
	m_ListBox.Create(m_hWnd, m_hInst, 0, 0, rc.right, rc.bottom);

}

void CDkWind::AddListBoxItem(TCHAR *szText)
{
	m_ListBox.AddStringItem(szText);
}

void CDkWind::TriggerStartMenu()
{
	if (OnStartMenu != NULL){
		(* OnStartMenu)();
	}
}

void CDkWind::TriggerStopMenu()
{
	if (OnStopMenu != NULL){
		(* OnStopMenu)();
	}
}

bool CDkWind::TriggerClose()
{
	bool		bRes = false;
	if (OnClose != NULL){
		(* OnClose)(bRes);
	}

	return bRes;
}

void CDkWind::ListBoxClear()
{
	m_ListBox.ClearItems();
}

void CDkWind::TriggerClearMenu()
{
	if (OnClearMenu != NULL){
		(* OnClearMenu)();
	}
}

void CDkWind::TriggerSelectPortMenu()
{
	if (OnSelectPortMenu != NULL){
		(* OnSelectPortMenu)();
	}
}


LRESULT CALLBACK CDkWind::DkWindProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm)
{
	LPCREATESTRUCT		pCreateStruct = (LPCREATESTRUCT)lParm;
	static CDkWind		*pThis = NULL;
	RECT				clRc;
	
	switch (uMsg)
	{
	case WM_CREATE:
		pThis = (CDkWind *)pCreateStruct->lpCreateParams;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParm))
		{
		case IDM_EXIT:
			if (pThis->TriggerClose()){
				::DestroyWindow(hWnd);
			}
			break;

		case IDM_TOOL_START:
			pThis->TriggerStartMenu();
			break;

		case IDM_TOOL_STOP:
			pThis->TriggerStopMenu();
			break;

		case IDM_TOOL_CLEAR:
			pThis->TriggerClearMenu();
			break;

		case IDM_ABOUT:
			::MessageBox(hWnd, TEXT("This is supposed to be about box."), TEXT("About"),
				MB_OK | MB_ICONINFORMATION);
			break;

		case IDM_TOOL_SELECT:
			pThis->TriggerSelectPortMenu();
			break;
		}
		break;

	case WM_SIZE:
	case WM_SIZING:
		::GetClientRect(hWnd, &clRc);
		pThis->m_ListBox.Resize(clRc);
		break;

	case WM_TIMER:
		break;

	case WM_CLOSE:
		if (pThis->TriggerClose()){
			::DestroyWindow(hWnd);
		}
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	default:
		return ::DefWindowProc(hWnd, uMsg, wParm, lParm);
	}

	return FALSE;
}
