
#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif
#include "AcpiSmiWdmCommon.h"
#include  "SimulateData.h"
#include "Feature_Flag.h"

#if 0
VOID DriverCallDriver(IN PDEVICE_OBJECT pDevObj)
{
  NTSTATUS Status = STATUS_SUCCESS;
  OBJECT_ATTRIBUTES objAttributes;
  UNICODE_STRING  DeviceName;
  PDEVICE_EXTENSION  pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  RtlInitUnicodeString(&DeviceName, L"\\Device\\00000020");
  //  RtlInitUnicodeString(&DeviceName, L"\\Device\\0000003E");
    // initialize objAttributes
  InitializeObjectAttributes(&objAttributes, &DeviceName, OBJ_CASE_INSENSITIVE, NULL, NULL);
  PDEVICE_OBJECT DeviceObject = NULL;
  PFILE_OBJECT FileObject = NULL;
  Status = IoGetDeviceObjectPointer(&DeviceName, FILE_ALL_ACCESS, &FileObject, &DeviceObject);
  if (!NT_SUCCESS(Status))
  {
    KdPrint(("IoGetDeviceObjectPoint () 0x%x\n", Status));
    //    return;
  }
  KdPrint(("IoGetDeviceObjectPoint Success\n"));
  KdPrint(("FileObject = 0x%x\n", FileObject));
  KdPrint(("DeviceObject = 0x%x\n", DeviceObject));
  PDEVICE_OBJECT TargetDevice = IoAttachDeviceToDeviceStack(pDevObj, DeviceObject);
  if (!TargetDevice)
  {
    ObDereferenceObject(FileObject);
    KdPrint(("IoattachDeviceToDeviceStack() 0x%x\n", Status));
    return;
  }
  pdx->pAcpiDevice = DeviceObject;

}

#endif

VOID DriverCallDriver(IN PDEVICE_OBJECT pDevObj)
{
  NTSTATUS Status = STATUS_SUCCESS;
  OBJECT_ATTRIBUTES objAttributes;
  IO_STATUS_BLOCK   statusBlock;
  UNICODE_STRING  DeviceName;
  PDEVICE_EXTENSION  pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  LARGE_INTEGER offset;
  PFILE_OBJECT file_object;
  offset.QuadPart = 0;
  RtlInitUnicodeString(&DeviceName, L"\\Device\\00000020");
  // initialize objAttributes
  InitializeObjectAttributes(&objAttributes, &DeviceName, OBJ_CASE_INSENSITIVE, NULL, NULL);
  Status = ZwCreateFile(
    &pdx->hDevice,
    FILE_READ_ATTRIBUTES | SYNCHRONIZE,
    &objAttributes,
    &statusBlock,
    NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ,
    FILE_OPEN_IF, 0, NULL, 0);
  // acquire device extension
  if (NT_SUCCESS(Status))
  {

    KdPrint(("ReadEntry -- Open another device success\n"));
    Status = ZwReadFile(pdx->hDevice, NULL, NULL, NULL, &statusBlock, NULL, 0,
      &offset, NULL);
    if( Status == STATUS_PENDING) {
      KdPrint(("ReadEntry -- another return pending"));
      Status = ObReferenceObjectByHandle(
        pdx->hDevice, 
        EVENT_MODIFY_STATE, 
        *ExEventObjectType, 
        KernelMode, 
        (PVOID*)&file_object, 
        NULL);
      if (NT_SUCCESS(Status))
      {
        KdPrint(("Driver::Waiting\n"));
        KeWaitForSingleObject(&file_object->Event, Executive, KernelMode, FALSE, NULL);
        KdPrint(("antoher driver IRP ready \n"));
        ObDereferenceObject(file_object);
      }
    } 
    if (NT_SUCCESS(Status))
    {
      KdPrint(("ReadEntry -- read return success\n"));
    }
    else
    {
      KdPrint(("ReadEntry ErrorCode =%x", Status));
    }
    ZwClose(pdx->hDevice);
  }
}

#if !KEYBAORD_FILTER
#pragma PAGEDCODE
NTSTATUS Reader(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP pIrp)
{

  KdPrint(("Reader Entry\n"));
  NTSTATUS Status = STATUS_SUCCESS;
  DriverCallDriver(pDevObj);
#if USE_IRP_PENDING 
  IoSetCancelRoutine(pIrp, CancelReadIrp);
  // pending this irp 
  IoMarkIrpPending(pIrp);
#if DRIVER_START_IO
  // put it to queue for StartIo Serial handling 
  IoStartPacket(pDevObj, pIrp, 0, onCancelIrp);
#endif
  KdPrint(("Reader Exit\n"));
  return (Status = STATUS_PENDING);
#else
  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
  ULONG ulReaLength = stack->Parameters.Read.Length;
  pIrp->IoStatus.Status = Status;
  pIrp->IoStatus.Information = ulReaLength;
  //  memset(pIrp->AssociatedIrp.SystemBuffer, 0xaa, ulReaLength);
  memcpy(pIrp->AssociatedIrp.SystemBuffer, pDevExt->buffer, ulReaLength);
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  KdPrint(("Reader Exit\n"));
  return Status;
#endif
}

#endif
