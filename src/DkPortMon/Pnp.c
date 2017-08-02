#include "DkPortMon.h"

extern PDEVICE_OBJECT			g_pThisDevObj;
extern UNICODE_STRING			g_usDevName;
extern UNICODE_STRING			g_usLnkName;

NTSTATUS DkAddDevice(PDRIVER_OBJECT pDrvObj, PDEVICE_OBJECT pPhysDevObj)
{
	NTSTATUS				ntStat = STATUS_SUCCESS;
	PDEVICE_EXTENSION		pDevExt = NULL;
	
	DkDbgStr("@");
	
	ntStat = IoCreateDevice(pDrvObj, sizeof(DEVICE_EXTENSION), &g_usDevName,
		FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &g_pThisDevObj);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error create device!", ntStat);
		return ntStat;
	}
	pDevExt = (PDEVICE_EXTENSION) g_pThisDevObj->DeviceExtension;
	pDevExt->pDrvObj = pDrvObj;
	
	ntStat = IoCreateSymbolicLink(&g_usLnkName, &g_usDevName);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error createsymbolic link!", ntStat);
		goto EndFunc;
	}

	IoInitializeRemoveLock(&pDevExt->ioRemLock, 0, 0, 0);
	KeInitializeSpinLock(&pDevExt->csqSpinLock);
	InitializeListHead(&pDevExt->lePendIrp);
	ntStat = IoCsqInitialize(&pDevExt->ioCsq,
		DkCsqInsertIrp, DkCsqRemoveIrp, DkCsqPeekNextIrp, DkCsqAcquireLock,
		DkCsqReleaseLock, DkCsqCompleteCanceledIrp);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error initialize Cancel-safe queue!", ntStat);
		goto EndFunc;
	}

	pDevExt->pNextDevObj = IoAttachDeviceToDeviceStack(g_pThisDevObj, pPhysDevObj);
	if (pDevExt->pNextDevObj == NULL){
		DkDbgStr("Error attach device to device stack!");
		ntStat = STATUS_UNSUCCESSFUL;
		goto EndFunc;
	}

	g_pThisDevObj->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
	g_pThisDevObj->Flags &= ~DO_DEVICE_INITIALIZING;

EndFunc:
	if (!NT_SUCCESS(ntStat)){
		if (g_pThisDevObj != NULL){
			IoDeleteSymbolicLink(&g_usLnkName);
			IoDeleteDevice(g_pThisDevObj);
			g_pThisDevObj = NULL;
		}
	}

	return ntStat;
}

NTSTATUS DkPnp(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS			ntStat = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt = NULL;
	PIO_STACK_LOCATION	pStack = NULL;
	KEVENT				kEvt;

	KdPrint(("DkPortMon2, %s(): Pointer to device object = %p", __FUNCTION__, pDevObj));

	pDevExt = (PDEVICE_EXTENSION) g_pThisDevObj->DeviceExtension;
	
	ntStat = IoAcquireRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error acquire remove lock!", ntStat);
		DkCompleteRequest(pIrp, ntStat, 0);
		return ntStat;
	}

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	switch(pStack->MinorFunction){
		case IRP_MN_START_DEVICE:
			DkDbgStr("IRP_MJ_START_DEVICE");
			
			KeInitializeEvent(&kEvt, NotificationEvent, FALSE);
			IoCopyCurrentIrpStackLocationToNext(pIrp);
			IoSetCompletionRoutine(pIrp, 
				(PIO_COMPLETION_ROUTINE) &DkPnpStartCompletion, (PVOID) &kEvt,
				TRUE, TRUE, TRUE);
			if (pDevObj == g_pThisDevObj){
				ntStat = IoCallDriver(pDevExt->pNextDevObj, pIrp);
			} else {
				ntStat = IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);
			}

			if (ntStat == STATUS_PENDING){
				KeWaitForSingleObject((PVOID) &kEvt, Executive, KernelMode, TRUE, NULL);
				ntStat = pIrp->IoStatus.Status;
			}
			
			IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);
			DkCompleteRequest(pIrp, ntStat, 0);
			
			return ntStat;


		case IRP_MN_REMOVE_DEVICE:
			DkDbgStr("IRP_MJ_REMOVE_DEVICE");

			IoReleaseRemoveLockAndWait(&pDevExt->ioRemLock, (PVOID) pIrp);
			
			IoSkipCurrentIrpStackLocation(pIrp);
			if (pDevObj == g_pThisDevObj){
				ntStat = IoCallDriver(pDevExt->pNextDevObj, pIrp);
			
				DkTgtDetachAndDeleteDevice(pDevExt);
				
				IoDetachDevice(pDevExt->pNextDevObj);
				IoDeleteSymbolicLink(&g_usLnkName);
				IoDeleteDevice(g_pThisDevObj);
			} else {
				ntStat = IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);
				DkTgtDetachAndDeleteDevice(pDevExt);
			}

			return ntStat;


		default:
			KdPrint(("DkPortMon2, %s(): IRP PNP Minor function: 0x%X", 
				__FUNCTION__, pStack->MinorFunction));
			break;

	}

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID)pIrp);

	IoSkipCurrentIrpStackLocation(pIrp);
	if (pDevObj == g_pThisDevObj){
		return IoCallDriver(pDevExt->pNextDevObj, pIrp);
	} else {
		return IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);
	}
}

NTSTATUS DkPnpStartCompletion(PDEVICE_OBJECT pDevObj, PIRP pIrp, PVOID pCtx)
{
	PKEVENT			pkEvt = NULL;

	pkEvt = (PKEVENT) pCtx;
	if (pkEvt == NULL){
		return STATUS_UNSUCCESSFUL;
	}
	if (pIrp->PendingReturned)
		IoMarkIrpPending(pIrp);

	KeSetEvent(pkEvt, IO_NO_INCREMENT, FALSE);
	
	return STATUS_MORE_PROCESSING_REQUIRED;
}
