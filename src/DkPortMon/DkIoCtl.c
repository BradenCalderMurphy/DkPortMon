#include "DkPortMon.h"

extern PDEVICE_OBJECT		g_pThisDevObj;

NTSTATUS DkDevCtl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS				ntStat = STATUS_SUCCESS;
	PDEVICE_EXTENSION		pDevExt = NULL;
	PIO_STACK_LOCATION		pStack = NULL;
	ULONG					bufWrLen = 0;


	pDevExt = (PDEVICE_EXTENSION) g_pThisDevObj->DeviceExtension;

	ntStat = IoAcquireRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error acquire remove lock!", ntStat);
		DkCompleteRequest(pIrp, ntStat, 0);
		return ntStat;
	}

	if (pDevObj != g_pThisDevObj)
		return DkTgtDevCtl(pDevExt, pIrp);

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	switch(pStack->Parameters.DeviceIoControl.IoControlCode){
		case IOCTL_DKPORTMON_ATTACH_DEVICE:
			bufWrLen = pStack->Parameters.DeviceIoControl.InputBufferLength;
			if (bufWrLen <= 0){
				ntStat = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			ntStat = DkTgtCreateAndAttachDevice(pDevExt, pIrp);
			break;


		case IOCTL_DKPORTMON_DETACH_DEVICE:
			DkTgtDetachAndDeleteDevice(pDevExt);
			break;


		default:
			ntStat = STATUS_INVALID_DEVICE_REQUEST;
			break;
	}

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);

	DkCompleteRequest(pIrp, ntStat, 0);
	return ntStat;
}

NTSTATUS DkTgtDevCtl(PDEVICE_EXTENSION pDevExt, PIRP pIrp)
{
	//NTSTATUS			ntStat = STATUS_SUCCESS;
	PIO_STACK_LOCATION	pStack = NULL;
	UNICODE_STRING		usStrDat;

	DkDbgStr("@");

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	
	switch (pStack->Parameters.DeviceIoControl.IoControlCode){
		case IOCTL_SERIAL_GET_BAUD_RATE:
			DkDbgStr("Ctl code: IOCTL_SERIAL_GET_BAUD_RATE");
			return DkTgtHandleIoCtlGetReq(pDevExt, pIrp);


		case IOCTL_SERIAL_GET_HANDFLOW:
			DkDbgStr("Ctl code: IOCTL_SERIAL_GET_HANDFLOW");
			return DkTgtHandleIoCtlGetReq(pDevExt, pIrp);

		
		case IOCTL_SERIAL_GET_LINE_CONTROL:
			DkDbgStr("Ctl code: IOCTL_SERIAL_GET_LINE_CONTROL");
			return DkTgtHandleIoCtlGetReq(pDevExt, pIrp);


		case IOCTL_SERIAL_GET_TIMEOUTS:
			DkDbgStr("Ctl code: IOCTL_SERIAL_GET_TIMEOUTS");
			return DkTgtHandleIoCtlGetReq(pDevExt, pIrp);


		case IOCTL_SERIAL_SET_BAUD_RATE:
			DkDbgStr("Ctl code: IOCTL_SERIAL_SET_BAUD_RATE");
			DkTgtHandleIoCtlSetReq(L"IOCTL_SERIAL_SET_BAUD_RATE", pDevExt, pIrp, pStack);
			break;


		case IOCTL_SERIAL_SET_HANDFLOW:
			DkDbgStr("Ctl code: IOCTL_SERIAL_SET_HANDFLOW");
			DkTgtHandleIoCtlSetReq(L"IOCTL_SERIAL_SET_HANDFLOW", pDevExt, pIrp, pStack);
			break;


		case IOCTL_SERIAL_SET_LINE_CONTROL:
			DkDbgStr("Ctl code: IOCTL_SERIAL_SET_LINE_CONTROL");
			DkTgtHandleIoCtlSetReq(L"IOCTL_SERIAL_SET_LINE_CONTROL", pDevExt, pIrp, pStack);
			break;


		case IOCTL_SERIAL_SET_TIMEOUTS:
			DkDbgStr("Ctl code: IOCTL_SERIAL_SET_TIMEOUTS");
			DkTgtHandleIoCtlSetReq(L"IOCTL_SERIAL_SET_TIMEOUTS", pDevExt, pIrp, pStack);
			break;


		case IOCTL_SERIAL_SET_QUEUE_SIZE:
			DkDbgStr("Ctl code: IOCTL_SERIAL_SET_QUEUE_SIZE");
			DkTgtHandleIoCtlSetReq(L"IOCTL_SERIAL_SET_QUEUE_SIZE", pDevExt, pIrp, pStack);
			break;


		case IOCTL_SERIAL_CONFIG_SIZE:
			DkDbgStr("Ctl code: IOCTL_SERIAL_SERIAL_CONFIG_SIZE");
			RtlInitUnicodeString(&usStrDat, L"IOCTL_SERIAL_CONFIG_SIZE");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;


		case IOCTL_SERIAL_GET_CHARS:
			DkDbgStr("Ctl code: IOCTL_SERIAL_GET_CHARS");
			return DkTgtHandleIoCtlGetReq(pDevExt, pIrp);


		case IOCTL_SERIAL_SET_CHARS:
			DkDbgStr("Ctl code: IOCTL_SERIAL_SET_CHARS");
			DkTgtHandleIoCtlSetReq(L"IOCTL_SERIAL_SET_CHARS", pDevExt, pIrp, pStack);
			break;


		case IOCTL_SERIAL_SET_DTR:
			DkDbgStr("Ctl code: IOCTL_SERIAL_SET_DTR");
			DkTgtHandleIoCtlSetReq(L"IOCTL_SERIAL_SET_DTR", pDevExt, pIrp, pStack);
			break;


		case IOCTL_SERIAL_SET_RTS:
			DkDbgStr("Ctl code: IOCTL_SERIAL_SET_RTS");
			DkTgtHandleIoCtlSetReq(L"IOCTL_SERIAL_SET_RTS", pDevExt, pIrp, pStack);
			break;


		case IOCTL_SERIAL_CLR_DTR:
			DkDbgStr("Ctl code: IOCTL_SERIAL_CLR_DTR");
			RtlInitUnicodeString(&usStrDat, L"IOCTL_SERIAL_CLR_DTR");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;


		case IOCTL_SERIAL_CLR_RTS:
			DkDbgStr("Ctl code: IOCTL_SERIAL_CLR_RTS");
			RtlInitUnicodeString(&usStrDat, L"IOCTL_SERIAL_CLR_RTS");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;


		case IOCTL_SERIAL_SET_WAIT_MASK:
			DkDbgStr("Ctl code: IOCTL_SERIAL_SET_WAIT_MASK");
			DkTgtHandleIoCtlSetReq(L"IOCTL_SERIAL_SET_WAIT_MASK", pDevExt, pIrp, pStack);
			break;


		case IOCTL_SERIAL_GET_WAIT_MASK:
			DkDbgStr("Ctl code: IOCTL_SERIAL_GET_WAIT_MASK");
			return DkTgtHandleIoCtlGetReq(pDevExt, pIrp);
			

		case IOCTL_SERIAL_WAIT_ON_MASK:
			DkDbgStr("Ctl code: IOCTL_SERIAL_WAIT_ON_MASK");
			RtlInitUnicodeString(&usStrDat, L"IOCTL_SERIAL_WAIT_ON_MASK");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;

		case IOCTL_SERIAL_PURGE:
			DkDbgStr("Ctl code: IOCTL_SERIAL_PURGE");
			RtlInitUnicodeString(&usStrDat, L"IOCTL_SERIAL_PURGE");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;

		case IOCTL_SERIAL_GET_COMMSTATUS:
			DkDbgStr("Ctl code: IOCTL_SERIAL_GET_COMMSTATUS");
			return DkTgtHandleIoCtlGetReq(pDevExt, pIrp);


		default:
			DkDbgVal("Unknown Ctl code!", pStack->Parameters.DeviceIoControl.IoControlCode);
			RtlInitUnicodeString(&usStrDat, L"IOCTL_SERIAL_UNKNOWN");
			if (pStack->Parameters.DeviceIoControl.InputBufferLength > 0){
				DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length,
					pIrp->AssociatedIrp.SystemBuffer,
					pStack->Parameters.DeviceIoControl.InputBufferLength);
			} else if (pStack->Parameters.DeviceIoControl.OutputBufferLength > 0){
				DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length,
					pIrp->AssociatedIrp.SystemBuffer,
					pStack->Parameters.DeviceIoControl.OutputBufferLength);
			} else {
				DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			}
			break;
	}

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PIRP) pIrp);

	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);
}

NTSTATUS DkIntDevCtl(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS				ntStat = STATUS_SUCCESS;
	PDEVICE_EXTENSION		pDevExt = NULL;

	pDevExt = (PDEVICE_EXTENSION) g_pThisDevObj->DeviceExtension;

	ntStat = IoAcquireRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error acquire remove lock!", ntStat);
		DkCompleteRequest(pIrp, ntStat, 0);
		return ntStat;
	}

	if (pDevObj != g_pThisDevObj){
		return DkTgtIntDevCtl(pDevExt, pIrp);
	}

	DkDbgStr("###");

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);

	DkCompleteRequest(pIrp, ntStat, 0);
	return ntStat;
}

NTSTATUS DkTgtIntDevCtl(PDEVICE_EXTENSION pDevExt, PIRP pIrp)
{
	//NTSTATUS				ntStat = STATUS_SUCCESS;
	PIO_STACK_LOCATION		pStack = NULL;
	UNICODE_STRING			usStrDat;

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	switch (pStack->Parameters.DeviceIoControl.IoControlCode){
		case IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS:
			DkDbgStr("IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS");
			RtlInitUnicodeString(&usStrDat, L"IOCTL_SERIAL_INTERNAL_BASIC_SETTINGS");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;

		case IOCTL_SERIAL_INTERNAL_CANCEL_WAIT_WAKE:
			DkDbgStr("IOCTL_SERIAL_INTERNAL_CANCEL_WAIT_WAKE");
			RtlInitUnicodeString(&usStrDat, L"IOCTL_SERIAL_INTERNAL_CANCEL_WAIT_WAKE");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;

		case IOCTL_SERIAL_INTERNAL_DO_WAIT_WAKE:
			DkDbgStr("IOCTL_SERIAL_INTERNAL_DO_WAIT_WAKE");
			RtlInitUnicodeString(&usStrDat, L"IOCTL_SERIAL_INTERNAL_DO_WAIT_WAKE");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;

		case IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS:
			DkDbgStr("IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS");
			RtlInitUnicodeString(&usStrDat, L"IOCTL_SERIAL_INTERNAL_RESTORE_SETTINGS");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;

		default:
			DkDbgVal("Unknown internal device control request!", 
				pStack->Parameters.DeviceIoControl.IoControlCode);
			break;
	}

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);

	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);
}

NTSTATUS DkTgtCompletion(PDEVICE_OBJECT pDevObj, PIRP pIrp, PVOID pCtx)
{
	PDEVICE_EXTENSION		pDevExt = NULL;
	PIO_STACK_LOCATION		pStack = NULL;
	UNICODE_STRING			usStr;

	pDevExt = (PDEVICE_EXTENSION) pCtx;

	// Just to satisfy prefast
	if (pDevExt == NULL)
		return STATUS_UNSUCCESSFUL;

	if (pIrp->PendingReturned)
		IoMarkIrpPending(pIrp);

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	switch (pStack->Parameters.DeviceIoControl.IoControlCode){
		case IOCTL_SERIAL_GET_BAUD_RATE:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_BAUD");
			break;
		case IOCTL_SERIAL_GET_CHARS:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_CHARS");
			break;
		case IOCTL_SERIAL_GET_COMMCONFIG:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_COMMCONFIG");
			break;
		case IOCTL_SERIAL_GET_COMMSTATUS:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_COMMSTATUS");
			break;
		case IOCTL_SERIAL_GET_DTRRTS:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_DTRRTS");
			break;
		case IOCTL_SERIAL_GET_HANDFLOW:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_HANDFLOW");
			break;
		case IOCTL_SERIAL_GET_LINE_CONTROL:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_LINE_CONTROL");
			break;
		case IOCTL_SERIAL_GET_MODEM_CONTROL:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_MODEM_CONTROL");
			break;
		case IOCTL_SERIAL_GET_MODEMSTATUS:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_MODEM_CONTROL");
			break;
		case IOCTL_SERIAL_GET_PROPERTIES:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_PROPERTIES");
			break;
		case IOCTL_SERIAL_GET_STATS:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_STATS");
			break;
		case IOCTL_SERIAL_GET_TIMEOUTS:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_TIMEOUTS");
			break;
		case IOCTL_SERIAL_GET_WAIT_MASK:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_GET_WAIT_MASK");
			break;
		default:
			RtlInitUnicodeString(&usStr, L"IOCTL_SERIAL_UNKNOWN");
			break;
	}

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);

	DkTgtCompletePendedIrp(usStr.Buffer, usStr.Length, 
		pIrp->AssociatedIrp.SystemBuffer,
		pStack->Parameters.DeviceIoControl.OutputBufferLength);

	return STATUS_SUCCESS;
}

NTSTATUS DkTgtHandleIoCtlGetReq(PDEVICE_EXTENSION pDevExt, PIRP pIrp)
{
	IoCopyCurrentIrpStackLocationToNext(pIrp);
	IoSetCompletionRoutine(pIrp, 
		(PIO_COMPLETION_ROUTINE) DkTgtCompletion,
		(PVOID) pDevExt, TRUE, TRUE, TRUE);

	return IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);
}

VOID DkTgtHandleIoCtlSetReq(PWCH szStr, PDEVICE_EXTENSION pDevExt, PIRP pIrp, PIO_STACK_LOCATION pStack)
{
	UNICODE_STRING		usStrDat;

	RtlInitUnicodeString(&usStrDat, szStr);
	DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length,
		pIrp->AssociatedIrp.SystemBuffer,
		pStack->Parameters.DeviceIoControl.InputBufferLength);
}