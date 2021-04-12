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
NTSTATUS StartDevice(
  PDEVICE_EXTENSION pdx, 
  PIRP pIrp
)
{
  PAGED_CODE();
  KdPrint(("Enter HandleStartDevcie\n"));
  NTSTATUS status = STATUS_SUCCESS;
  status = ForwardAndWait(pdx, pIrp);
  if (!NT_SUCCESS(status))
  {
    pIrp->IoStatus.Status = status;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    return status;
  }


  PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(pIrp);
  PCM_PARTIAL_RESOURCE_LIST raw;
  if (stack->Parameters.StartDevice.AllocatedResources)
  {
    KdPrint(("Show translated resources\n"));
    raw = &stack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList;
    ShowResources(raw);
  }
  else
  {
    KdPrint(("Cannot get translated resource\n"));
    raw = NULL;
  }

  ///
  /// ONLY FOR Device with 'ASIM' declaration - BEGIN
  /// basetbl.asl
#define BASE_TBL_OPREGION_SUPPORT 0
#if BASE_TBL_OPREGION_SUPPORT
  AcpiInstallOpRegionHandler(pdx->NextStackDevice);
#endif
  ///
  /// ONLY FOR Device with 'ASIM' declaration - END
  ///
  pIrp->IoStatus.Status = STATUS_SUCCESS;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  KdPrint(("Leave StartDevice\n"));
  return status;
}
