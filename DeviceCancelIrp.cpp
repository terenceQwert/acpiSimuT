#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif
#include "AcpiSmiWdmCommon.h"


VOID 
CancelReadIrp(
  IN PDEVICE_OBJECT /* DeviceObject */,
  IN PIRP irp
)
{
  KdPrint(("Enter CancelReadIrp\n"));
//  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
  irp->IoStatus.Status = STATUS_CANCELLED;
  irp->IoStatus.Information = 0;
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  IoReleaseCancelSpinLock(irp->CancelIrql);

  KdPrint(("Leave CancelReadIrp\n"));
}
