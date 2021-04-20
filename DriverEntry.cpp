#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#ifdef __cplusplus
}
#endif

#include "AcpiSmiWdmCommon.h"
#include "Enum.h"
#include "Feature_Flag.h"
#include "AcpiWmiCallback.h"

extern "C" 
NTSTATUS DriverEntry(
  IN PDRIVER_OBJECT DriverObject,
  IN PUNICODE_STRING RegistryPath
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
// #pragma alloc_text (INIT, EchoPrintDriverVersion)
// #pragma alloc_text (PAGE, EchoEvtDeviceAdd)
#endif
NTSTATUS AddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject);
NTSTATUS WdmPnp(IN PDEVICE_OBJECT fdo, IN PIRP pIrp);

NTSTATUS WmiSysIoDispatch(IN PDEVICE_OBJECT fdo, IN PIRP pIrp);
NTSTATUS SystemControl(IN PDEVICE_OBJECT fdo, IN PIRP pIrp);

VOID DumpDeviceStack(IN PDEVICE_OBJECT pdo);
VOID DisplayProcessName();
VOID LinkListTest();
VOID New_Test();
void CreateFileFromWDM();

UNICODE_STRING GlobalRegistryPath;

void Unload(IN PDRIVER_OBJECT DriverObject);

#pragma INITCODE
extern "C"
NTSTATUS DriverEntry(
  IN PDRIVER_OBJECT DriverObject,
  IN PUNICODE_STRING RegistryPath
)
{
  KdPrint(("Enter AcpiSim DriverEntry\n"));
  KdPrint(("Obj (%0x8) Registry = \"%ws\"\n",DriverObject, RegistryPath->Buffer));
  GlobalRegistryPath.MaximumLength = RegistryPath->Length + sizeof(UNICODE_NULL);
  GlobalRegistryPath.Buffer = (PWCH)ExAllocatePoolWithTag(PagedPool, GlobalRegistryPath.MaximumLength, 'StaB');
  if (!GlobalRegistryPath.Buffer)
  {
    KdPrint(("Couldn't allocate pool for registry path.\n"));
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  else
  {
    RtlCopyUnicodeString(&GlobalRegistryPath, RegistryPath);
  }

  DriverObject->DriverExtension->AddDevice = AddDevice;
  DriverObject->DriverStartIo = StartIo;
  DriverObject->MajorFunction[IRP_MJ_PNP] = WdmPnp;
  DriverObject->MajorFunction[IRP_MJ_CREATE] = AcpiSmiWdmDispatchRoutine;
  DriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceClose;
#if BUFFER_IO
  DriverObject->MajorFunction[IRP_MJ_READ] = Reader;
#else
  DriverObject->MajorFunction[IRP_MJ_READ] = HelloWDMDirectIoRead;
#endif
  DriverObject->MajorFunction[IRP_MJ_WRITE] = Writer;
  DriverObject->MajorFunction[IRP_MJ_CLEANUP] = AcpiSmiWdmDispatchRoutine;
  DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HelloWDMDeviceIoControl;
  DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = WmiSysIoDispatch;
  DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = AcpiSmiWdmDispatchRoutine;
  DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = SystemControl;
  DriverObject->DriverUnload = Unload;
  KdPrint(("Leave AcpiSim DriverEntry\n"));
  return STATUS_SUCCESS;
}

PDEVICE_OBJECT mykdbDevice;

#define NOTEBOOK_KB_DEV_NAME L"\\Device\\KeyboardClass0"
#define MY_FILTER_DEV_NAME L"\\Device\\kbdfilter_0"
NTSTATUS 
MyAttachDevice(PDRIVER_OBJECT DriverObject)
{
  KdPrint(("Enter MyAttachDevice \n"));
  NTSTATUS status;
  UNICODE_STRING TargetDevice;
  UNICODE_STRING myfilter;
  RtlInitUnicodeString(&TargetDevice, NOTEBOOK_KB_DEV_NAME);
  RtlInitUnicodeString(&myfilter, MY_FILTER_DEV_NAME);
  status = IoCreateDevice(
    DriverObject, sizeof(DEVICE_EXTENSION), 
    &myfilter, FILE_DEVICE_KEYBOARD,
    0, FALSE, &mykdbDevice);
  if (!NT_SUCCESS(status))
  {
    KdPrint(("Enter MyAttachDevice Error Status=0x%08x \n", status));
    return status;
  }
  mykdbDevice->Flags |= DO_BUFFERED_IO;
  mykdbDevice->Flags &= ~DO_DEVICE_INITIALIZING;
  RtlZeroMemory(mykdbDevice->DeviceExtension, sizeof(DEVICE_EXTENSION));
  status = IoAttachDevice(mykdbDevice, &TargetDevice, &((PDEVICE_EXTENSION)mykdbDevice->DeviceExtension)->LowerDevice);
  if (!NT_SUCCESS(status))
  {
    KdPrint(("Fail to do device attach \n"));
    IoDeleteDevice(mykdbDevice);
    return status;
  }
  KdPrint(("attach to \\Device\\KeyboardClass0 success \n"));
  KdPrint(("Exit MyAttachDevice \n"));
  return status;
}
void Dump(IN PDRIVER_OBJECT	pDriverObject)
{
  KdPrint(("----------------------------------------------\n"));
  KdPrint(("Begin Dump...\n"));
  KdPrint(("Driver Address:0x%08X\n", pDriverObject));
  KdPrint(("Driver name:%wZ\n", &pDriverObject->DriverName));
  KdPrint(("Driver HardwareDatabase:%wZ\n", pDriverObject->HardwareDatabase));
  KdPrint(("Driver first device:0X%08X\n", pDriverObject->DeviceObject));

  PDEVICE_OBJECT pDevice = pDriverObject->DeviceObject;
  int i = 1;
  for (; pDevice != NULL; pDevice = pDevice->NextDevice)
  {
    KdPrint(("The %d device\n", i++));
    KdPrint(("Device AttachedDevice:0X%08X\n", pDevice->AttachedDevice));
    KdPrint(("Device NextDevice:0X%08X\n", pDevice->NextDevice));
    KdPrint(("Device StackSize:%d\n", pDevice->StackSize));
    KdPrint(("Device's DriverObject:0X%08X\n", pDevice->DriverObject));
  }

  KdPrint(("Dump over!\n"));
  KdPrint(("----------------------------------------------\n"));
}
#pragma PAGED_CODE
NTSTATUS AddDevice(
  IN PDRIVER_OBJECT DriverObject, 
  IN PDEVICE_OBJECT PhysicalDeviceObject
)
{
  PAGED_CODE();
  KdPrint(("AcpiSim: Enter Add Device\n"));

  NTSTATUS ntStatus = STATUS_SUCCESS;
  PDEVICE_OBJECT fdo;
  UNICODE_STRING  devName;
  KdPrint(("PDO Devicetype = %x\n", PhysicalDeviceObject->DeviceType));
  KdPrint(("PhysicalDeviceObject = %x\n", PhysicalDeviceObject));
//  MyAttachDevice(DriverObject);
  RtlInitUnicodeString(&devName, MYWDM_NAME);
  ntStatus = IoCreateDevice(
    DriverObject,
    sizeof(DEVICE_EXTENSION),
#if USE_NAME
    &devName,
#else
    NULL,
#endif
//    FILE_DEVICE_UNKNOWN,
    FILE_DEVICE_BATTERY,
    0,
    FALSE,
    &fdo
  );
  if (!NT_SUCCESS(ntStatus))
  {
    KdPrint(("AcpiSim: IoCreateDevice fail with status %x\n", ntStatus));
    return STATUS_SUCCESS;
//    return ntStatus; 
  }
  PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;

  ///
  /// store this function device object.
  ///
  pdx->fdo = fdo;
  pdx->NextStackDevice = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);

  ///
  /// saved it!
  ///
  pdx->PDO = PhysicalDeviceObject;
  KdPrint(("AcpiSim: lower device = %x\n", pdx->NextStackDevice));
  pdx->NextPeerDevice = DriverObject->DeviceObject->NextDevice;
  memset(pdx->buffer, 0, sizeof(pdx->buffer));
//  Dump(DriverObject);
  UNICODE_STRING  symLinkName;
  RtlInitUnicodeString(&symLinkName, SMBOL_LNK_NAME);
  pdx->ustrDeviceName = devName;
  pdx->ustrSymLinkName = symLinkName;

  ntStatus = IoCreateSymbolicLink(&symLinkName, &devName);
  if (!NT_SUCCESS(ntStatus))
  {
    KdPrint(("AcpiSim: - create symbolic link fail with status = %d\n", ntStatus));
    IoDeleteSymbolicLink(&pdx->ustrDeviceName);
    ntStatus = IoCreateSymbolicLink(&symLinkName, &devName);

    if (!NT_SUCCESS(ntStatus))
    {
      return ntStatus;
    }
  }

#if BUFFER_IO
  fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
#else
  fdo->Flags |= DO_DIRECT_IO;
#endif
  fdo->Flags &= ~DO_DEVICE_INITIALIZING;
#if 0
  IoInitializeTimer(fdo, OnTimer, NULL);
#else
  KeInitializeTimer(&pdx->pollingTimer);
  KeInitializeDpc(&pdx->pollingDPC, PollingTimerDpc, (PVOID)fdo);
#endif

  ///
  /// Register WMI related function
  ///
#if 1
  ntStatus = Acpi_Wmi_Registration(pdx);
  if (NT_ERROR(ntStatus))
  {
    KdPrint(("Register WMI fail with status code = 0x%x", ntStatus));
  }
  
#endif
//  Dump(DriverObject);
//  DumpDeviceStack(PhysicalDeviceObject);
  KdPrint(("AcpiSim: Leave Add Device\n"));
  return STATUS_SUCCESS;
}

#pragma
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp)
{
  PAGED_CODE();
  KdPrint(("Enter DefaultHandler\n"));
  NTSTATUS status = STATUS_SUCCESS;
  Irp->IoStatus.Status = status;
  Irp->IoStatus.Information = 0;
  IoCompleteRequest(Irp, IO_NO_INCREMENT);
//  IoSkipCurrentIrpStackLocation(Irp);
  KdPrint(("AcpiSim: NextStackDevice:%08x\n", pdx->NextStackDevice));
  KdPrint(("Leave DefaultHandler\n"));
  return status;
//
// an IRP cannot be handled twice, otherwise, BSOD code happen
//
//  return IoCallDriver(pdx->NextStackDevice, Irp);
}



#if USE_NAME
#pragma
NTSTATUS RemoveDevice(PDEVICE_EXTENSION pdx, PIRP pIrp)
{
  PAGED_CODE();
  KdPrint(("Enter HanldeRemoveDevice\n"));
  pIrp->IoStatus.Status = DefaultPnpHandler(pdx, pIrp);
  IoDeleteSymbolicLink(&pdx->ustrSymLinkName);

  if (pdx->NextStackDevice)
    IoDetachDevice(pdx->NextStackDevice);
  IoDeleteDevice(pdx->fdo);
  KdPrint(("Leave RemoveDevice\n"));
  return STATUS_SUCCESS;
}
#endif



extern ULONG pendingkey;
#pragma
void Unload(IN PDRIVER_OBJECT pDriverObject)
{
  PAGED_CODE();
  KdPrint(("Enter Unload\n"));
  PDEVICE_OBJECT pNextDevObj;
  pNextDevObj = pDriverObject->DeviceObject;
  while (NULL != pNextDevObj)
  {
    PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pNextDevObj->DeviceExtension;
    UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
    IoDeleteSymbolicLink(&pLinkName);
    pNextDevObj = pNextDevObj->NextDevice;
    IoDeleteDevice( pDevExt->fdo);
  }

  ///
  /// free buffer allocation
  ///
  ExFreePool(GlobalRegistryPath.Buffer);
  KdPrint(("Leave Unload\n"));
}

#pragma PAGEDCODE
NTSTATUS WdmPnp(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{

  PAGED_CODE();
#if DBG
  static char * fcnName[] = {
    "IRP_MN_START_DEVICE",
    "IRP_MN_QUERY_REMOVE_DEVICE",
    "IRP_MN_REMOVE_DEVICE",
    "IRP_MN_CANCEL_REMOVE_DEVICE",
    "IRP_MN_STOP_DEVICE",
    "IRP_MN_QUERY_STOP_DEVICE",
    "IRP_MN_CANCEL_STOP_DEVICE",
    "IRP_MN_QUERY_DEVICE_RELATIONS",
    "IRP_MN_QUERY_INTERFACE",
    "IRP_MN_QUERY_CAPABILITIES",
    "IRP_MN_QUERY_RESOURCES",
    "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
    "IRP_MN_QUERY_DEVICE_TEXT",
    "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
    "",
    "IRP_MN_READ_CONFIG",
    "IRP_MN_WRITE_CONFIG",
    "IRP_MN_EJECT",
    "IRP_MN_SET_LOCK",
    "IRP_MN_QUERY_ID",
    "IRP_MN_QUERY_PNP_DEVICE_STATE",
    "IRP_MN_QUERY_BUS_INFORMATION",
    "IRP_MN_DEVICE_USAGE_NOTIFICATION",
    "IRP_MN_SURPRISE_REMOVAL"
  };  
#endif // DBGS
  KdPrint(("Enter WdmPnp\n"));
  NTSTATUS status = STATUS_SUCCESS;
  PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
  static NTSTATUS(*fcntab[]) (PDEVICE_EXTENSION pdx, PIRP Irp) = {
#if 1
   StartDevice,                 // IRP_MN_START_DEVICE
#else
    DefaultPnpHandler,          // Start Device
#endif
    DefaultPnpHandler,          // IRP_MN_QUERY_REMOVE_DEVICE
    RemoveDevice,               // IRP_MN_REMOVE_DEVICE
    DefaultPnpHandler,          // IRP_MN_CANCEL_REMOVE_DEVICE
    DefaultPnpHandler,          // IRP_MN_STOP_DEVICE
    DefaultPnpHandler,          // IRP_MN_QUERY_STOP_DEVICE
    DefaultPnpHandler,          // IRP_MN_CANCEL_STOP_DEVICE
    DeviceQueryDeviceRelation,  // IRP_MN_QUERY_DEVICE_RELATIONS
    DefaultPnpHandler,          // IRP_MN_QUERY_INTERFACE
    PnpQueryCapabilitiesHandler,// IRP_MN_QUERY_CAPABILITIES
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler,
    DefaultPnpHandler
  };
  ULONG fcn = stack->MinorFunction;
  if (fcn >= arraysize(fcntab))
  {
    //
    // for over flow dispatch minor, use DefaultPnpHandler.
    //
    status = DefaultPnpHandler(pdx, Irp);
    return status;
  }
  KdPrint(("PNP  Request (%s)\n", fcnName[fcn]));
  status = (*fcntab[fcn])(pdx, Irp);
  KdPrint(("Leave WdmPnp"));
  return status;
}


