#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#include <oprghdlr.h>
#include <acpiioct.h>
#include <acpitabl.h>
#include <dsm.h>
#include "AcpiSmiWdmCommon.h"
#include "Enum.h"
#include "wmi42.h"
#ifdef __cplusplus
}
#endif

#pragma
NTSTATUS WmiSysIoDispatch(IN PDEVICE_OBJECT, IN PIRP irp)
{
  PAGED_CODE();
  KdPrint(("Enter AcpiSmiWdmDispatchRoutine\n"));
  irp->IoStatus.Status = STATUS_SUCCESS;
  irp->IoStatus.Information = 0;  // no bytes transferred.
  IoCompleteRequest(irp, IO_NO_INCREMENT);
  KdPrint(("Leave AcpiSmiWdmDispatchRoutine\n"));
  return STATUS_SUCCESS;
}

PCHAR
WMIMinorFunctionString(
  UCHAR MinorFunction
)
{
  switch (MinorFunction)
  {
  case IRP_MN_CHANGE_SINGLE_INSTANCE:
    return "IRP_MN_CHANGE_SINGLE_INSTANCE";
  case IRP_MN_CHANGE_SINGLE_ITEM:
    return "IRP_MN_CHANGE_SINGLE_ITEM";
  case IRP_MN_DISABLE_COLLECTION:
    return "IRP_MN_DISABLE_COLLECTION";
  case IRP_MN_DISABLE_EVENTS:
    return "IRP_MN_DISABLE_EVENTS";
  case IRP_MN_ENABLE_COLLECTION:
    return "IRP_MN_ENABLE_COLLECTION";
  case IRP_MN_ENABLE_EVENTS:
    return "IRP_MN_ENABLE_EVENTS";
  case IRP_MN_EXECUTE_METHOD:
    return "IRP_MN_EXECUTE_METHOD";
  case IRP_MN_QUERY_ALL_DATA:
    return "IRP_MN_QUERY_ALL_DATA";
  case IRP_MN_QUERY_SINGLE_INSTANCE:
    return "IRP_MN_QUERY_SINGLE_INSTANCE";
  case IRP_MN_REGINFO:
    return "IRP_MN_REGINFO";
  default:
    return "IRP_MN_?????";
  }
}

//
// to generate this Wmi42Guid declaration, 
// due to VS2017 cannot default binding wmimofck to generate properly header file, 
// add following line: 
/// <Wmimofck Include=".\$(IntDir)\wmi42.bmf">
/// <HeaderOutputFile>.\$(IntDir)\wmi42.h</HeaderOutputFile>
/// <VBScriptTestOutputFile>.\$(IntDir)\wmi42.vbs</VBScriptTestOutputFile>
/// </Wmimofck>
// detail, can refer to 'WDKSample\storage\msdsm\src'
//

GUID AcpiSimWMIGUID = Wmi42Guid;
WMIGUIDREGINFO guidlist[] = {
  {&AcpiSimWMIGUID, 1, WMIREG_FLAG_INSTANCE_PDO}
};

WMILIB_CONTEXT libinfo = {
  arraysize(guidlist),
  guidlist,
  QueryRegInfo,
  QueryDataBlock,
  NULL,               // SetDataBlock
  NULL,               // SetDataItem
  NULL,               // Executemethod
  NULL                // FunctionControl
};

NTSTATUS 
SystemControl(
  IN PDEVICE_OBJECT fdo,
  IN PIRP irp
)
{
  NTSTATUS status = STATUS_SUCCESS;
  PDEVICE_EXTENSION pDevExt = NULL;
  PIO_STACK_LOCATION  stack;
  PDEVICE_OBJECT lower_device;
  SYSCTL_IRP_DISPOSITION disposition = IrpForward;

  PAGED_CODE();
  KdPrint(("Enter SystemControl\n"));
  stack = IoGetCurrentIrpStackLocation(irp);
  KdPrint(("SystemControl %s\n", WMIMinorFunctionString(stack->MinorFunction)));

  //
  // MajorFunction: IRP_MJ_SYSTEM_CONTROL & MinorFunction between IRP_MN_QUERY_ALL_DATA ~ IRP_MN_EXECUTE_METHOD
  // for WMI
  //
  status = WmiSystemControl(&libinfo, fdo, irp, &disposition);
  pDevExt = (PDEVICE_EXTENSION)fdo->DeviceExtension;
  // it is ok, but, not trig WMI callback
  lower_device = pDevExt->NextStackDevice;
  // below cause BSOD
  // lower_device = pDevExt->fdo;
  switch (disposition)
  {
    case IrpProcessed:
      break;
    case IrpNotCompleted:
      //
      // If it is WMI, completing the IRP is our responsiblity. This casuse would 
      // be the normal one for IRP_MN_REGINFO. WMILIB has already filled in the IoStatus
      // block of hte IRP, so we need only call IoCompleteRequest.
      //
      KdPrint(("SystemControl [IoCompleteRequest]\n"));
      IoCompleteRequest(irp, IO_NO_INCREMENT);
      break;
    case IrpForward:
    case IrpNotWmi:
    default:
    {
      IoSkipCurrentIrpStackLocation(irp);
      status = IoCallDriver(lower_device, irp);
    }
  }
  KdPrint(("Leave SystemControl\n"));
  return status;
}
