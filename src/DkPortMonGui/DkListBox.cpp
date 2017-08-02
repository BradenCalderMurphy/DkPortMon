#include "DkListBox.h"

CDkListBox::CDkListBox(): m_hWnd(NULL), m_hPar(NULL), m_hInst(NULL)
{
}

CDkListBox::CDkListBox(HWND hPar, HINSTANCE hInst): m_hWnd(NULL)
{
	m_hPar = hPar;
	m_hInst = hInst;
}

CDkListBox::~CDkListBox(void)
{
	Destroy();
}

bool CDkListBox::Create(int iLeft, int iTop, int iWidth, int iHeight)
{
	if ((m_hPar == NULL) || (m_hInst == NULL))
		return false;

	m_hWnd = ::CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL, 
		iLeft, iTop, iWidth, iHeight,
		m_hPar, NULL, m_hInst, NULL);
	if (!m_hWnd)
		return false;

	HFONT hDefFont = (HFONT) ::GetStockObject(DEFAULT_GUI_FONT);
	if (hDefFont){
		::SendMessage(m_hWnd, WM_SETFONT, (WPARAM)hDefFont, (LPARAM)TRUE);
	}
	return true;
}

bool CDkListBox::Create(HWND hPar, HINSTANCE hInst, int iLeft, int iTop, int iWidth, int iHeight)
{
	m_hPar = hPar;
	m_hInst = hInst;
	return Create(iLeft, iTop, iWidth, iHeight);
}

void CDkListBox::Destroy()
{
	if (m_hWnd){
		::DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
	m_hInst = NULL;
}

bool CDkListBox::AddStringItem(const wchar_t *szStrItem)
{
	LRESULT lRes = 0;
	ULONG curSel = 0;
	lRes = ::SendMessage(m_hWnd, LB_ADDSTRING, NULL, (LPARAM)szStrItem);
	curSel = (ULONG) lRes;
	if (lRes < 0)
		return false;
	lRes = ::SendMessage(m_hWnd, LB_SETCURSEL, (WPARAM)curSel, NULL);
	return (lRes >= 0) ? true : false;
}

bool CDkListBox::DeleteStringItem()
{
	LRESULT lRes = 0;
	lRes = ::SendMessage(m_hWnd, LB_GETCURSEL, NULL, NULL);
	if (lRes < 0)
		return false;
	lRes = ::SendMessage(m_hWnd, LB_DELETESTRING, (WPARAM)lRes, NULL);
	return (lRes >= 0) ? true : false;
}

void CDkListBox::ClearItems()
{
	int i = 0;
	LRESULT lRes = 0;
	
	lRes = ::SendMessage(m_hWnd, LB_GETCOUNT, (WPARAM)NULL, (LPARAM)NULL);
	if (lRes > 0){
		for (i = (int) lRes; i >= 0; i--){
			::SendMessage(m_hWnd, LB_DELETESTRING, (WPARAM)i, NULL);
		}
	}
}

void CDkListBox::Resize(RECT rc)
{
	::MoveWindow(m_hWnd, rc.left, rc.top, rc.right, rc.bottom, TRUE);
}