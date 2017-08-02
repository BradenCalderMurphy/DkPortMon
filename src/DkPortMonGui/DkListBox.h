#pragma once

#include "Windows.h"

class CDkListBox
{
public:
	CDkListBox(void);
	CDkListBox(HWND hPar, HINSTANCE hInst);
	~CDkListBox(void);
	bool Create(int iLeft, int iTop, int iWidth, int iHeight);
	bool Create(HWND hPar, HINSTANCE hInst, int iLeft, int iTop, int iWidth, int iHeight);
	void Destroy(void);
	bool AddStringItem(const wchar_t *szStrItem);
	bool DeleteStringItem(void);
	void ClearItems(void);
	void Resize(RECT rc);

private:
	HWND m_hWnd;
	HWND m_hPar;
	HINSTANCE m_hInst;
};
