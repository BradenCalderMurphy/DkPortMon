#ifndef __DKPORTMON_H__
#define __DKPORTMON_H__

#include "Wdm.h"
#include "Ntddser.h"

#include "DkQue.h"

#define DKPORT_DEV_NAME		L"\\Device\\DkPortMon2"
#define DKPORT_LNK_NAME		L"\\DosDevices\\Global\\DkPortMon2"
#define DKMTAG				(ULONG)'dkaE'

#include "..\Inc\Shared.h"

typedef struct DEVICE_EXTENSION_Tag {
	PDEVICE_OBJECT			pNextDevObj;
	PDEVICE_OBJECT			pTgtDevObj;
	PDEVICE_OBJECT			pTgtNextDevObj;
	IO_REMOVE_LOCK			ioRemLock;
	PDRIVER_OBJECT			pDrvObj;
	LIST_ENTRY				lePendIrp;
	IO_CSQ					ioCsq;
	KSPIN_LOCK				csqSpinLock;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

DRIVER_INITIALIZE	DriverEntry;
DRIVER_UNLOAD		DkUnload;
DRIVER_ADD_DEVICE	DkAddDevice;

__drv_dispatchType(IRP_MJ_CREATE)
__drv_dispatchType(IRP_MJ_CLOSE)
__drv_dispatchType(IRP_MJ_CLEANUP)						DRIVER_DISPATCH			DkCreateClose;

__drv_dispatchType(IRP_MJ_READ)
__drv_dispatchType(IRP_MJ_WRITE)						DRIVER_DISPATCH			DkReadWrite;

__drv_dispatchType(IRP_MJ_PNP)							DRIVER_DISPATCH			DkPnp;

__drv_dispatchType(IRP_MJ_POWER)						DRIVER_DISPATCH			DkPower;

__drv_dispatchType(IRP_MJ_DEVICE_CONTROL)				DRIVER_DISPATCH			DkDevCtl;

__drv_dispatchType(IRP_MJ_INTERNAL_DEVICE_CONTROL)		DRIVER_DISPATCH			DkIntDevCtl;

__drv_dispatchType_other								DRIVER_DISPATCH			DkForward;

VOID	DkCompleteRequest(PIRP pIrp, NTSTATUS completeStatus, ULONG_PTR ulInfo);

IO_COMPLETION_ROUTINE DkPnpStartCompletion;

NTSTATUS DkTgtCreateClose(PDEVICE_EXTENSION pDevExt, PIRP pIrp);
NTSTATUS DkTgtReadWrite(PDEVICE_EXTENSION pDevExt, PIRP pIrp);
NTSTATUS DkTgtDevCtl(PDEVICE_EXTENSION pDevExt, PIRP pIrp);
NTSTATUS DkTgtIntDevCtl(PDEVICE_EXTENSION pDevExt, PIRP pIrp);
NTSTATUS DkTgtForward(PDEVICE_EXTENSION pDevExt, PIRP pIrp);

IO_COMPLETION_ROUTINE DkTgtReadCompletion;

NTSTATUS DkTgtCreateAndAttachDevice(PDEVICE_EXTENSION pDevEx, PIRP pIrp);
VOID	 DkTgtDetachAndDeleteDevice(PDEVICE_EXTENSION pDevEx);

NTSTATUS DkTgtCompletePendedIrp(PWCH szFuncName, ULONG ulFuncNameByteLen, PUCHAR pDat, ULONG ulDatByteLen);

IO_COMPLETION_ROUTINE DkTgtCompletion;

NTSTATUS DkTgtHandleIoCtlGetReq(PDEVICE_EXTENSION pDevExt, PIRP pIrp);
VOID	 DkTgtHandleIoCtlSetReq(PWCH szStr, PDEVICE_EXTENSION pDevExt, PIRP pIrp, PIO_STACK_LOCATION pStack);


#define DkDbgStr(a) KdPrint(("DkPortMon2, %s(): %s", __FUNCTION__, a))
#define DkDbgVal(a, b) KdPrint(("DkPortMon2, %s(): %s "#b" = 0x%X", __FUNCTION__, a, b))


#endif  // End of __DKPORTMON_H__
