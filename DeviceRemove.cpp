#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif
#include "AcpiSmiWdmCommon.h"
#pragma PAGEDCODE
NTSTATUS 
RemoveDevice(
  PDEVICE_EXTENSION pdx,
  PIRP Irp
)
{
  PAGED_CODE();
  KdPrint(("Enter RemoveDevice\n"));

  Irp->IoStatus.Status = STATUS_SUCCESS;
  NTSTATUS status = DefaultPnpHandler(pdx, Irp);
  // Set Device interface
  IoSetDeviceInterfaceState(&pdx->interfaceName, FALSE);
  // Relase UNICODE string
  RtlFreeUnicodeString(&pdx->interfaceName);
  // call IoDetachDevice() to pop device out stack
  if (pdx->NextStackDevice)
  {
    IoDetachDevice(pdx->NextStackDevice);
  }

  // free fdo
  IoDeleteDevice(pdx->fdo);

  KdPrint(("Leave RemoveDevice\n"));
  return status;
}
