
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

NTSTATUS HellowWDMRead_TIMER(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP           pIrp
)
{
  KdPrint(("DrverA: Enter A Reader\n"));
  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  LARGE_INTEGER timeout;
  IoMarkIrpPending(pIrp);
  pDevExt->currentPendingIrp = pIrp;
//  ULONG ulMicroSecond = 3 * 1000 * 1000;
//  LARGE_INTEGER timeout = RtlConvertLongToLargeInteger(-10 * ulMicroSecond);
  timeout.QuadPart = -3 * 1000 * 1000 * 10;
  KeSetTimer(&pDevExt->pollingTimer, timeout, &pDevExt->pollingDPC);
  KdPrint(("DrverA: Leave A Reader\n"));
  return STATUS_PENDING;
}