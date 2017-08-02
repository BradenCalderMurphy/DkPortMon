#include "DkPortMon.h"

extern PDEVICE_OBJECT		g_pThisDevObj;

NTSTATUS DkPower(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS			ntStat = STATUS_SUCCESS;
	PDEVICE_EXTENSION	pDevExt = NULL;
	PIO_STACK_LOCATION	pStack = NULL;

	KdPrint(("DkPortMon2, %s(): Pointer to device object = %p", __FUNCTION__, pDevObj));

	pDevExt = (PDEVICE_EXTENSION) g_pThisDevObj->DeviceExtension;

	ntStat = IoAcquireRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error acquire remove lock!", ntStat);
		DkCompleteRequest(pIrp, ntStat, 0);
		return ntStat;
	}

	pStack = IoGetCurrentIrpStackLocation(pIrp);
	switch (pStack->MinorFunction){
		case IRP_MN_POWER_SEQUENCE:
			DkDbgStr("IRP_MN_POWER_SEQUENCE");
			break;


		case IRP_MN_QUERY_POWER:
			DkDbgStr("IRP_MN_QUERY_POWER");
			break;


		case IRP_MN_SET_POWER:
			DkDbgStr("IRP_MN_SET_POWER");
			break;


		case IRP_MN_WAIT_WAKE:
			DkDbgStr("IRP_MN_WAIT_WAKE");
			break;


		default:
			DkDbgStr("Unknown power request!");
			break;
	}

	IoReleaseRemoveLock(&pDevExt->ioRemLock, (PVOID) pIrp);

	PoStartNextPowerIrp(pIrp);
	IoSkipCurrentIrpStackLocation(pIrp);

#if (NTDDI_VERSION < NTDDI_VISTA)
	if (pDevObj == g_pThisDevObj){
		return PoCallDriver(pDevExt->pNextDevObj, pIrp);
	} else {
		return PoCallDriver(pDevExt->pTgtNextDevObj, pIrp);
	}
#else
	if (pDevObj == g_pThisDevObj) {
		return IoCallDriver(pDevExt->pNextDevObj, pIrp);
	} else {
		return IoCallDriver(pDevExt->pTgtNextDevObj, pIrp);
	}
#endif
}
