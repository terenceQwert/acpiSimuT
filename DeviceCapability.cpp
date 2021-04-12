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
CompleteIrp(IN PIRP Irp, IN NTSTATUS status, IN ULONG info)
{
  Irp->IoStatus.Status = status;
  Irp->IoStatus.Information = info;
  IoCompleteRequest(Irp, IO_NO_INCREMENT);
  return status;
}

NTSTATUS 
PnpQueryCapabilitiesHandler(IN PDEVICE_EXTENSION pdx, IN PIRP irp)
{
  NTSTATUS status = ForwardAndWait(pdx, irp);
  if (NT_SUCCESS(status))
  {
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(irp);
    PDEVICE_CAPABILITIES deviceCapabilities = IrpStack->Parameters.DeviceCapabilities.Capabilities;
    for (int ds = PowerSystemWorking; ds < PowerSystemMaximum; ds++)
      KdPrint(("Capabilitiesfrom bus: DeviceState[%d]=%d\n", ds, deviceCapabilities->DeviceState[ds]));

    DEVICE_POWER_STATE dps;
    SetMostPoweredState(PowerSystemWorking, PowerDeviceD0);
    SetMostPoweredState(PowerSystemSleeping1, PowerDeviceD3);
    SetMostPoweredState(PowerSystemSleeping2, PowerDeviceD3);
    SetMostPoweredState(PowerSystemSleeping3, PowerDeviceD3);
    SetMostPoweredState(PowerSystemHibernate, PowerDeviceD3);
    SetMostPoweredState(PowerSystemShutdown, PowerDeviceD3);

    deviceCapabilities->Removable = TRUE;

    for (int ds = PowerSystemWorking; ds < PowerSystemMaximum; ds++)
      KdPrint(("Capabilities now: DeviceState[%d]=%d\n", ds, deviceCapabilities->DeviceState[ds]));
  }
  irp->IoStatus.Status = status;
  irp->IoStatus.Information = 0;
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  return status;
}

