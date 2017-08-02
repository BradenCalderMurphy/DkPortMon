#include "DkPortMon.h"

extern PDEVICE_OBJECT		g_pThisDevObj;

NTSTATUS DkTgtCreateAndAttachDevice(PDEVICE_EXTENSION pDevEx, PIRP pIrp)
{
	NTSTATUS			ntStat = STATUS_SUCCESS;
	PFILE_OBJECT		pFlObj = NULL;
	UNICODE_STRING		usTgtDevName;
	PDEVICE_OBJECT		pTgtDevObj = NULL;

	DkDbgStr("@");

	RtlInitUnicodeString(&usTgtDevName, (PCWSTR) pIrp->AssociatedIrp.SystemBuffer);

	ntStat = IoGetDeviceObjectPointer(&usTgtDevName, GENERIC_ALL, &pFlObj, &pTgtDevObj);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error get device object pointer!", ntStat);
		return ntStat;
	}

	ObDereferenceObject(pFlObj);

	ntStat = IoCreateDevice(pDevEx->pDrvObj, 0, NULL, 
		pTgtDevObj->Characteristics, FILE_DEVICE_SECURE_OPEN, FALSE, &pDevEx->pTgtDevObj);
	if (!NT_SUCCESS(ntStat)){
		DkDbgVal("Error create target device object!", ntStat);
		goto EndFunc;
	}

	pDevEx->pTgtDevObj->Flags |= (pTgtDevObj->Flags & 
									(DO_BUFFERED_IO | DO_POWER_PAGABLE | DO_DIRECT_IO));

	pDevEx->pTgtNextDevObj = NULL;
	pDevEx->pTgtNextDevObj = IoAttachDeviceToDeviceStack(pDevEx->pTgtDevObj, pTgtDevObj);
	if (pDevEx->pTgtNextDevObj == NULL){
		DkDbgVal("Error attach device to device stack!", ntStat);
		ntStat = STATUS_UNSUCCESSFUL;
		goto EndFunc;
	}

	pDevEx->pTgtDevObj->Flags &= ~DO_DEVICE_INITIALIZING;

EndFunc:
	if (!NT_SUCCESS(ntStat)){
		if (pDevEx->pTgtNextDevObj){
			IoDetachDevice(pDevEx->pTgtNextDevObj);
			pDevEx->pTgtNextDevObj = NULL;
		}
		if (pDevEx->pTgtDevObj){
			IoDeleteDevice(pDevEx->pTgtDevObj);
			pDevEx->pTgtDevObj = NULL;
		}
	}

	return ntStat;
}

VOID DkTgtDetachAndDeleteDevice(PDEVICE_EXTENSION pDevEx)
{
	DkDbgStr("@");

	if (pDevEx->pTgtNextDevObj){
		IoDetachDevice(pDevEx->pTgtNextDevObj);
		pDevEx->pTgtNextDevObj = NULL;
	}
	if (pDevEx->pTgtDevObj){
		IoDeleteDevice(pDevEx->pTgtDevObj);
		pDevEx->pTgtDevObj = NULL;
	}
}

NTSTATUS DkTgtCompletePendedIrp(PWCH szFuncName, ULONG ulFuncNameByteLen, PUCHAR pDat, ULONG ulDatByteLen)
{
	PDKPORT_DAT			pNewDat = NULL;
	PIRP				pIrp = NULL;
	PDEVICE_EXTENSION	pDevExt = NULL;

	if (ulFuncNameByteLen > (DKPORT_STR_LEN * 2)){
		DkDbgStr("Error string data too big!");
		return STATUS_BUFFER_OVERFLOW;
	}
	if (ulDatByteLen > DKPORT_DAT_LEN){
		DkDbgStr("Error data too big!");
		return STATUS_BUFFER_OVERFLOW;
	}

	pDevExt = (PDEVICE_EXTENSION) g_pThisDevObj->DeviceExtension;
	
	pIrp = IoCsqRemoveNextIrp(&pDevExt->ioCsq, NULL);
	if (pIrp == NULL){
	
		DkQueAdd(szFuncName, ulFuncNameByteLen, pDat, ulDatByteLen);
	
	} else {

		pNewDat = (PDKPORT_DAT) pIrp->AssociatedIrp.SystemBuffer;
		RtlFillMemory(pNewDat, sizeof(DKPORT_DAT), '\0');
		pNewDat->FuncNameLen = ulFuncNameByteLen;
		pNewDat->DataLen = ulDatByteLen;
		if (szFuncName != NULL){
			RtlCopyMemory(pNewDat->StrFuncName, szFuncName, ulFuncNameByteLen);
		}
		if (pDat != NULL){
			RtlCopyMemory(pNewDat->Data, pDat, ulDatByteLen);
		}

		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = sizeof(DKPORT_DAT);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	}

	return STATUS_SUCCESS;
}