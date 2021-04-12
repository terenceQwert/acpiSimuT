#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif
#include "AcpiSmiWdmCommon.h"

#if 0
#pragma PAGEDCODE
NTSTATUS HelloWDMDirectIoRead(
  IN PDEVICE_OBJECT /* pDevObj */, 
  IN PIRP pIrp
)
{

  KdPrint(("HelloWDMDirectIoRead Entry\n"));
  NTSTATUS Status = STATUS_SUCCESS;
//  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
  ULONG ulReadLength = stack->Parameters.Read.Length;
  KdPrint(("ulReadLength = %d\n", ulReadLength));
  ULONG mdl_length = MmGetMdlByteCount(pIrp->MdlAddress);
  PVOID mdl_address = MmGetMdlVirtualAddress(pIrp->MdlAddress);

  ULONG mdl_offset = MmGetMdlByteOffset(pIrp->MdlAddress);

  KdPrint(("mdl_address = 0x%08X", mdl_address));
  KdPrint(("mdl_length = %d", mdl_length));
  KdPrint(("mdl_offset = %d", mdl_offset));
  if (mdl_length != ulReadLength)
  {
    pIrp->IoStatus.Information = 0;
    Status = STATUS_UNSUCCESSFUL;
  }
  else {
    PVOID kernel_address = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
    KdPrint(("mdl_address = 0x%08X", kernel_address));
    memset(kernel_address, 0xAA, ulReadLength);
    pIrp->IoStatus.Information = ulReadLength;
  }
  pIrp->IoStatus.Status = Status;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);
  KdPrint(("HelloWDMDirectIoRead Exit\n"));
  return Status;
}
#endif

#pragma PAGEDCODE
NTSTATUS OnRequestComplete(
  PDEVICE_OBJECT /* junk */, 
  PIRP /*pIrp*/, 
  PKEVENT pev
)
{
  KeSetEvent(pev, 0, FALSE);
  return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma PAGEDCODE
NTSTATUS ForwardAndWait(PDEVICE_EXTENSION pdx, PIRP pIrp)
{
  KdPrint(("AcpiSim: ForwardAndWait - Start\n"));
  PAGED_CODE();
  KEVENT event;
  KeInitializeEvent(&event, NotificationEvent, FALSE);
  IoCopyCurrentIrpStackLocationToNext(pIrp);
  IoSetCompletionRoutine(pIrp, (PIO_COMPLETION_ROUTINE)OnRequestComplete, (PVOID)&event, TRUE, TRUE, TRUE);

  IoCallDriver(pdx->NextStackDevice, pIrp);
  // wait for PDO complete
  KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
  KdPrint(("AcpiSim: ForwardAndWait - End\n"));
  return pIrp->IoStatus.Status;
}


#pragma PAGEDCODE
VOID ShowResources(IN PCM_PARTIAL_RESOURCE_LIST list)
{
  PCM_PARTIAL_RESOURCE_DESCRIPTOR resource = list->PartialDescriptors;
  ULONG nres = list->Count;
  ULONG i;

  for (i = 0; i < nres; ++i, ++resource)
  {						// for each resource
    ULONG type = resource->Type;

    static char* name[] = {
      "CmResourceTypeNull",
      "CmResourceTypePort",
      "CmResourceTypeInterrupt",
      "CmResourceTypeMemory",
      "CmResourceTypeDma",
      "CmResourceTypeDeviceSpecific",
      "CmResourceTypeBusNumber",
      "CmResourceTypeDevicePrivate",
      "CmResourceTypeAssignedResource",
      "CmResourceTypeSubAllocateFrom",
    };

    KdPrint(("    type %s", type < arraysize(name) ? name[type] : "unknown"));

    switch (type)
    {					// select on resource type
    case CmResourceTypePort:
    case CmResourceTypeMemory:
      KdPrint((" start %8X%8.8lX length %X\n",
        resource->u.Port.Start.HighPart, resource->u.Port.Start.LowPart,
        resource->u.Port.Length));
      break;

    case CmResourceTypeInterrupt:
      KdPrint(("  level %X, vector %X, affinity %X\n",
        resource->u.Interrupt.Level, resource->u.Interrupt.Vector,
        resource->u.Interrupt.Affinity));
      break;

    case CmResourceTypeDma:
      KdPrint(("  channel %d, port %X\n",
        resource->u.Dma.Channel, resource->u.Dma.Port));
    }					// select on resource type
  }						// for each resource
}							// ShowResources


