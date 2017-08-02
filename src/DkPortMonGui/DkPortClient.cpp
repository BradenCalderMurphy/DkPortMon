#include "DkPortClient.h"

CDkPortClient::CDkPortClient(void): m_hPort(NULL), m_bReqStop(true)
{
	OnDataReceived = NULL;
}

CDkPortClient::~CDkPortClient(void)
{
	if (m_hPort != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hPort);

}

bool CDkPortClient::Start(TCHAR *pSzDevName)
{
	bool		bRes = false;

	if (!m_bReqStop)
		return bRes;

	m_hPort = ::CreateFile(DKPORT_NAME_STR, GENERIC_ALL, 0, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	if (m_hPort == INVALID_HANDLE_VALUE)
		return false;

	
	m_RdOvr.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_RdOvr.OffsetHigh = 0;
	m_RdOvr.Offset = 0;
	if (!m_RdOvr.hEvent)
	{
		goto EndFunc;
	}

	m_bReqStop = false;

	TCHAR	szDevName[128];
	RtlFillMemory(szDevName, sizeof(szDevName), '\0');
	::StringCbCopy(szDevName, sizeof(szDevName), pSzDevName);
	DWORD	dwRes = 0;
	if (!::DeviceIoControl(m_hPort, IOCTL_DKPORTMON_ATTACH_DEVICE,
				(LPVOID) szDevName, sizeof(szDevName),
				NULL, 0, &dwRes, &m_RdOvr)){

					goto EndFunc;
	}

	m_hThread = ::CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE) &GetData, (LPVOID) this, 0, NULL);
	if (m_hThread == NULL){
		goto EndFunc;
	}

	bRes = true;


EndFunc:
	if (!bRes){
		m_bReqStop = true;
		::CloseHandle(m_RdOvr.hEvent);
		::CloseHandle(m_hPort);
	}

	return bRes;
}

bool CDkPortClient::Stop()
{
	DWORD		dwRet = 0;
	BYTE		dummy[32];
	
	if (!::DeviceIoControl(m_hPort, IOCTL_DKPORTMON_DETACH_DEVICE, 
							(LPVOID)dummy, sizeof(dummy),
							NULL, 0, &dwRet, NULL)){

			return false;
	}

	m_bReqStop = true;
	if (!::CloseHandle(m_hPort)){
		m_bReqStop = false;
		return false;
	}
	m_hPort = NULL;

	::WaitForSingleObject(m_hThread, INFINITE);

	::CloseHandle(m_RdOvr.hEvent);
	::CloseHandle(m_hThread);

	return true;
}

DWORD CDkPortClient::GetData(PVOID pDat)
{
	CDkPortClient * pThis = NULL;
	DWORD			dwRes = 0;
	DWORD			dwErr = 0;
	DKPORT_DAT		dkPortDat;

	if (!pDat)
		return 1;

	pThis = (CDkPortClient *) pDat;

	for(;;){
		RtlFillMemory(&dkPortDat, sizeof(DKPORT_DAT), '\0');

		::ReadFile(pThis->m_hPort, (LPVOID) &dkPortDat, sizeof(DKPORT_DAT), &dwRes, &pThis->m_RdOvr);
		dwErr = ::GetLastError();

		if (pThis->m_bReqStop)
			break;
		
		if (dwErr == ERROR_IO_PENDING){
			if (::GetOverlappedResult(pThis->m_hPort, &pThis->m_RdOvr, &dwRes, TRUE)){
				RtlCopyMemory(&pThis->m_Dat, &dkPortDat, sizeof(DKPORT_DAT));
				pThis->TriggerOnDataReceived();
			}
		}
	}

	::ExitThread(0);
}

void CDkPortClient::TriggerOnDataReceived()
{
	if (OnDataReceived != NULL){
		(* OnDataReceived)();
	}
}

bool CDkPortClient::IsActive()
{
	return !m_bReqStop;
}