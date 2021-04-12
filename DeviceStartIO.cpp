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

VOID 
onCancelIrp(
  IN PDEVICE_OBJECT DeviceObject,
  IN PIRP Irp
)
{
  KdPrint(("Enter CancelReadIRP\n"));
  KIRQL oldIrql;
  if (Irp == DeviceObject->CurrentIrp)
  {
    oldIrql = Irp->CancelIrql;
    // release cancel spink lock
    IoReleaseCancelSpinLock(oldIrql);
    // continue next irp
    IoStartNextPacket(DeviceObject, TRUE);
    // lower irql
    KeLowerIrql(oldIrql);

  }
  else
  {
    // remove this irp from queue
    KeRemoveEntryDeviceQueue(&DeviceObject->DeviceQueue, &Irp->Tail.Overlay.DeviceQueueEntry);
    // rlease cance spind lok
    IoReleaseCancelSpinLock(Irp->CancelIrql);
  }
  Irp->IoStatus.Status = STATUS_CANCELLED;
  Irp->IoStatus.Information = 0;
  IoCompleteRequest(Irp, IO_NO_INCREMENT);
  KdPrint(("Leave CancelReadIRP\n"));
}

#pragma LOCKEDCODE
VOID
StartIo(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP           pIrp
)
{
  KdPrint(("Enter StartIo\n"));
  KIRQL oldirql;
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
  IoAcquireCancelSpinLock(&oldirql);
  if (pIrp != pDevObj->CurrentIrp || pIrp->Cancel)
  {
    // put in queue
    IoReleaseCancelSpinLock(oldirql);
    KdPrint(("Leave StartIo\n"));
  }
  else
  {
    // this irp is on-going process, it cannot be cancelled. put it in queue, 
    // configure cancel routine as rull, due to it is not allow be cancelled
    IoSetCancelRoutine(pIrp, NULL);
    IoReleaseCancelSpinLock(oldirql);
  }
  KEVENT event;
  KeInitializeEvent(&event, NotificationEvent, FALSE);
  LARGE_INTEGER timeout;
  timeout.QuadPart = -3 * 1000 * 1000 * 10;
  // delay time 3 seconds to simulate IRP operation
  KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, &timeout);
  // Set IRP Status
  ULONG ulRead = stack->Parameters.Read.Length;

  pIrp->IoStatus.Status = STATUS_SUCCESS;
  pIrp->IoStatus.Information = ulRead;
  if( ulRead > strlen(DATA_ITEM))
    memcpy(pIrp->AssociatedIrp.SystemBuffer, DATA_ITEM, strlen(DATA_ITEM));
  else
  {
    strcpy((char*)pIrp->AssociatedIrp.SystemBuffer, "empty");
  }
  // end of IRP request
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  // read any irp from queue, and continue process by StartIO
  IoStartNextPacket(pDevObj, TRUE);
  KdPrint(("Leave StartIo\n"));
 
}

