#pragma once

#define DKWIND_MAX_CAP_SZ	128

#include "Windows.h"
#include "Strsafe.h"
#include "Res.h"

#include "DkListBox.h"

class CDkWind
{
public:
	CDkWind(void);
	~CDkWind(void);
	bool Create(HINSTANCE hInst, int iLeft, int iTop, int iWidht, int iHeight);
	void Destroy(void);
	bool Show(int nShowCmd);
	bool Hide(void);
	bool ClientRect(LPRECT pRect);
	HWND GetHandle(void);
	void AddListBoxItem(TCHAR *szText);
	void ListBoxClear(void);

	void (* OnStartMenu)(void);
	void (* OnStopMenu)(void);
	void (* OnClose)(bool &bCanClose);
	void (* OnClearMenu)(void);
	void (* OnSelectPortMenu)(void);

private:
	HWND		m_hWnd;
	HINSTANCE	m_hInst;
	LPCWSTR		m_szClasName;
	WCHAR		m_szCaption[DKWIND_MAX_CAP_SZ];

	static LRESULT CALLBACK DkWindProc(HWND hWnd, UINT uMsg, WPARAM wParm, LPARAM lParm);

	bool Register();
	void Unregister();
	void InitializeChilds(void);

	void TriggerStartMenu(void);
	void TriggerStopMenu(void);
	bool TriggerClose(void);
	void TriggerClearMenu(void);
	void TriggerSelectPortMenu(void);

	CDkListBox		m_ListBox;
};
