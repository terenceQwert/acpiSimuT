
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

#pragma PAGEDCODE

NTSTATUS DeviceClose(IN PDEVICE_OBJECT , IN PIRP pIrp)
{
  PAGED_CODE();
  KdPrint(("AcpiSim: Enter DeviceClose\n"));
  NTSTATUS status = STATUS_SUCCESS;
  pIrp->IoStatus.Status = status;
  pIrp->IoStatus.Information = 0;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  KdPrint(("AcpiSim: Leave DeviceClose\n"));
  return status;
}
