#include "DkPortMon.h"

extern PDEVICE_OBJECT		g_pThisDevObj;

NTSTATUS DkCreateClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
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

	if (pDevObj != g_pThisDevObj)
		return DkTgtCreateClose(pDevExt, pIrp);

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	switch (pStack->MajorFunction){
		case IRP_MJ_CREATE:
			DkDbgStr("IRP_MJ_CREATE");
			DkQueInitialize();
			break;
			
		case IRP_MJ_CLOSE:
			DkDbgStr("IRP_MJ_CLOSE");
			break;

		case IRP_MJ_CLEANUP:
			DkDbgStr("IRP_MJ_CLEANUP");
			DkTgtDetachAndDeleteDevice(pDevExt);
			DkCleanUpQueue(g_pThisDevObj, pIrp);
			DkQueCleanUpData();
			break;

		default:
			DkDbgStr("Error unknown request!");
			ntStat = STATUS_INVALID_PARAMETER;
			break;
	}

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PIRP) pIrp);

	DkCompleteRequest(pIrp, ntStat, 0);

	return ntStat;
}

NTSTATUS DkTgtCreateClose(PDEVICE_EXTENSION pDevExt, PIRP pIrp)
{
	//NTSTATUS			ntStat = STATUS_SUCCESS;
	PIO_STACK_LOCATION	pStack = NULL;
	UNICODE_STRING		usStrDat;

	DkDbgStr("@");

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	switch (pStack->MajorFunction){
		case IRP_MJ_CREATE:
			DkDbgStr("IRP_MJ_CREATE");
			RtlInitUnicodeString(&usStrDat, L"IRP_MJ_CREATE");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;
			
		case IRP_MJ_CLOSE:
			DkDbgStr("IRP_MJ_CLOSE");
			RtlInitUnicodeString(&usStrDat, L"IRP_MJ_CLOSE");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;

		case IRP_MJ_CLEANUP:
			DkDbgStr("IRP_MJ_CLEANUP");
			RtlInitUnicodeString(&usStrDat, L"IRP_MJ_CLEANUP");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;

		default:
			DkDbgStr("Error unknown request!");
			RtlInitUnicodeString(&usStrDat, L"IRP_MJ_UNKNOWN");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;
	}
	
	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PIRP) pIrp);

	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);
}

NTSTATUS DkReadWrite(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS			ntStat = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt = NULL;
	PIO_STACK_LOCATION	pStack = NULL;
	PDKQUE_DAT			pQueDat = NULL;
	PDKPORT_DAT			pDat = NULL;

	DkDbgStr("@");
	pDevExt = (PDEVICE_EXTENSION) g_pThisDevObj->DeviceExtension;
	
	ntStat = IoAcquireRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error acquire remove lock!", ntStat);
		DkCompleteRequest(pIrp, ntStat, 0);
		return ntStat;
	}

	if (pDevObj != g_pThisDevObj)
		return DkTgtReadWrite(pDevExt, pIrp);

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	switch (pStack->MajorFunction){
		case IRP_MJ_READ:
			DkDbgStr("IRP_MJ_READ");
			if (pStack->Parameters.Read.Length != sizeof(DKPORT_DAT)){
				ntStat = STATUS_INVALID_PARAMETER;
				break;
			} else {
				pQueDat = DkQueGet();
				if (pQueDat == NULL){

					IoCsqInsertIrp(&pDevExt->ioCsq, pIrp, NULL);
					IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);
					return STATUS_PENDING;

				} else {
					pDat = (PDKPORT_DAT) pIrp->AssociatedIrp.SystemBuffer;
					
					RtlCopyMemory(pDat, &pQueDat->Dat, sizeof(DKPORT_DAT));
					
					DkQueDel(pQueDat);

					IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);
					
					DkCompleteRequest(pIrp, ntStat, (ULONG_PTR) sizeof(DKPORT_DAT));
					
					return ntStat;
				}
			}
			
		case IRP_MJ_WRITE:
			DkDbgStr("IRP_MJ_WRITE");
			ntStat = STATUS_NOT_IMPLEMENTED;
			break;

		default:
			DkDbgVal("Error unknown request!", pStack->MajorFunction);
			ntStat = STATUS_INVALID_PARAMETER;
			break;
	}

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);

	DkCompleteRequest(pIrp, ntStat, 0);

	return ntStat;
}

NTSTATUS DkTgtReadWrite(PDEVICE_EXTENSION pDevExt, PIRP pIrp)
{
	//NTSTATUS			ntStat = STATUS_SUCCESS;
	PIO_STACK_LOCATION	pStack = NULL;
	UNICODE_STRING		usStrDat;

	DkDbgStr("@");

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	switch (pStack->MajorFunction){
		case IRP_MJ_READ:
			DkDbgStr("IRP_MJ_READ");
			
			IoCopyCurrentIrpStackLocationToNext(pIrp);
			IoSetCompletionRoutine(pIrp, 
				(PIO_COMPLETION_ROUTINE) DkTgtReadCompletion, NULL, TRUE, TRUE, TRUE);

			return IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);


		case IRP_MJ_WRITE:
			DkDbgStr("IRP_MJ_WRITE");
			RtlInitUnicodeString(&usStrDat, L"IRP_MJ_WRITE");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length,
				(PUCHAR) pIrp->AssociatedIrp.SystemBuffer, pStack->Parameters.Write.Length);
			break;


		default:
			DkDbgVal("Error unknown request!", pStack->MajorFunction);
			RtlInitUnicodeString(&usStrDat, L"IRP_MJ_UNKNOWN");
			DkTgtCompletePendedIrp(usStrDat.Buffer, usStrDat.Length, NULL, 0);
			break;
	}

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);

	IoSkipCurrentIrpStackLocation(pIrp);
	return IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);
}

NTSTATUS DkTgtReadCompletion(PDEVICE_OBJECT pDevObj, PIRP pIrp, PVOID pCtx)
{
	PDEVICE_EXTENSION		pDevExt = NULL;
	UNICODE_STRING			usDat;
	PIO_STACK_LOCATION		pStack;

	if (pIrp->PendingReturned)
		IoMarkIrpPending(pIrp);

	RtlInitUnicodeString(&usDat, L"IRP_MJ_READ");

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	DkTgtCompletePendedIrp(usDat.Buffer, usDat.Length,
		(PUCHAR) pIrp->AssociatedIrp.SystemBuffer,
		pStack->Parameters.Read.Length);

	pDevExt = (PDEVICE_EXTENSION) g_pThisDevObj->DeviceExtension;
	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);

	return STATUS_SUCCESS;
}