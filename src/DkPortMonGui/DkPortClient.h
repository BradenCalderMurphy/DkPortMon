#pragma once

#include "windows.h"
#include "strsafe.h"
#include "devioctl.h"

#include "..\Inc\Shared.h"

#define DKPORT_NAME_STR		L"\\\\.\\DkPortMon2"

#define DKPORTCLIENT_MAX_STR_LEN	128

class CDkPortClient
{
public:
	DKPORT_DAT			m_Dat;

	CDkPortClient(void);
	~CDkPortClient(void);
	bool Start(TCHAR *pSzDevName);
	bool Stop(void);
	bool IsActive(void);

	void (* OnDataReceived)(void);

private:
	HANDLE				m_hPort;
	bool				m_bReqStop;
	OVERLAPPED			m_RdOvr;
	HANDLE				m_hThread;

	static DWORD GetData(PVOID pDat);

	void TriggerOnDataReceived(void);
};
