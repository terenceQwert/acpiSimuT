#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif
#include "AcpiSmiWdmCommon.h"


NTSTATUS
ReadWriteConfigSpace(
  IN PDEVICE_OBJECT DeviceObj,
  IN ULONG ReadOrWrite,
  IN PVOID Buffer,
  IN ULONG Offset,
  IN ULONG Length
)
{
  KEVENT event;
  NTSTATUS status;
  PIRP irp;
  IO_STATUS_BLOCK ioStatusBlock;
  PIO_STACK_LOCATION irpStack;
  PDEVICE_OBJECT targetObject;

  PAGED_CODE();
  KeInitializeEvent(&event, NotificationEvent, FALSE);
  targetObject = IoGetAttachedDeviceReference(DeviceObj);
  irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP, targetObject, NULL, 0, NULL, &event, &ioStatusBlock);

  if (NULL == irp)
  {
    status = STATUS_INSUFFICIENT_RESOURCES;
    goto end;
  }
  irpStack = IoGetNextIrpStackLocation(irp);
  if (ReadOrWrite == 0)
  {
    irpStack->MinorFunction = IRP_MN_READ_CONFIG;
  }
  else
  {
    irpStack->MinorFunction = IRP_MN_WRITE_CONFIG;
  }
  // set PCI configuration space
  irpStack->Parameters.ReadWriteConfig.WhichSpace = PCI_WHICHSPACE_CONFIG;
  // set buffer
  irpStack->Parameters.ReadWriteConfig.Buffer = Buffer;
  // set offset 
  irpStack->Parameters.ReadWriteConfig.Offset = Offset;
  // set operation length
  irpStack->Parameters.ReadWriteConfig.Length = Length;

  irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

  // start low-level drive
  status = IoCallDriver(targetObject, irp);
  if (STATUS_PENDING == status)
  {
    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    status = ioStatusBlock.Status;
  }
end:
  ObDereferenceObject(targetObject);
  return status;
}
