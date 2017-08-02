#include "DkPortMon.h"

PDEVICE_OBJECT			g_pThisDevObj;
UNICODE_STRING			g_usDevName;
UNICODE_STRING			g_usLnkName;

NTSTATUS DriverEntry(PDRIVER_OBJECT pDrvObj, PUNICODE_STRING pUsRegPath)
{
	//NTSTATUS				ntStat = STATUS_SUCCESS;
	UCHAR					ucCnt = 0;

	DkDbgStr("@");

	g_pThisDevObj = NULL;
	RtlInitUnicodeString(&g_usDevName, DKPORT_DEV_NAME);
	RtlInitUnicodeString(&g_usLnkName, DKPORT_LNK_NAME);

	pDrvObj->DriverExtension->AddDevice = DkAddDevice;
	pDrvObj->DriverUnload				= DkUnload;

	for (ucCnt = 0; ucCnt < IRP_MJ_MAXIMUM_FUNCTION; ucCnt++){
		pDrvObj->MajorFunction[ucCnt] = DkForward;
	}

	pDrvObj->MajorFunction[IRP_MJ_CREATE]					=
		pDrvObj->MajorFunction[IRP_MJ_CLOSE]				=
		pDrvObj->MajorFunction[IRP_MJ_CLEANUP]				= DkCreateClose;

	pDrvObj->MajorFunction[IRP_MJ_READ]						=
		pDrvObj->MajorFunction[IRP_MJ_WRITE]				= DkReadWrite;

	pDrvObj->MajorFunction[IRP_MJ_PNP]						= DkPnp;

	pDrvObj->MajorFunction[IRP_MJ_POWER]					= DkPower;

	pDrvObj->MajorFunction[IRP_MJ_DEVICE_CONTROL]			= DkDevCtl;

	pDrvObj->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]	= DkIntDevCtl;

	return STATUS_SUCCESS;
}

VOID DkUnload(PDRIVER_OBJECT pDrvObj)
{
	DkDbgStr("@");
}

VOID DkCompleteRequest(PIRP pIrp, NTSTATUS completeStatus, ULONG_PTR ulInfo)
{
	pIrp->IoStatus.Status = completeStatus;
	pIrp->IoStatus.Information = ulInfo;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
}

NTSTATUS DkForward(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS			ntStat = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt = NULL;
	PIO_STACK_LOCATION	pStack = NULL;

	pDevExt = (PDEVICE_EXTENSION) g_pThisDevObj->DeviceExtension;
	ntStat = IoAcquireRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error acquire remove lock!", ntStat);
		DkCompleteRequest(pIrp, ntStat, 0);
		return ntStat;
	}

	if (g_pThisDevObj != pDevObj)
		return DkTgtForward(pDevExt, pIrp);

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	KdPrint(("DkPotrMon2, %s(): IRP Major Function for this device: 0x%X", 
		__FUNCTION__, pStack->MajorFunction));

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);

	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pDevExt->pNextDevObj, pIrp);
}

NTSTATUS DkTgtForward(PDEVICE_EXTENSION pDevExt, PIRP pIrp)
{
	PIO_STACK_LOCATION		pStack = NULL;
	UNICODE_STRING			usDat;

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	switch (pStack->MajorFunction){
		case IRP_MJ_FLUSH_BUFFERS:
			RtlInitUnicodeString(&usDat, L"IRP_MJ_FLUSH_BUFFERS");
			break;

		case IRP_MJ_QUERY_INFORMATION:
			RtlInitUnicodeString(&usDat, L"IRP_MJ_QUERY_INFORMATION");
			break;

		case IRP_MJ_SET_INFORMATION:
			RtlInitUnicodeString(&usDat, L"IRP_MJ_SET_INFORMATION");
			break;

		case IRP_MJ_SYSTEM_CONTROL:
			RtlInitUnicodeString(&usDat, L"IRP_MJ_SYSTEM_CONTROL");
			break;

		default:
			RtlInitUnicodeString(&usDat, L"IRP_MJ_UNKNOWN");
			break;
	}

	DkTgtCompletePendedIrp(usDat.Buffer, usDat.Length, NULL, 0);

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);
	
	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);

}
