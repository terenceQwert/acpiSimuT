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
#include <acpiioct.h>
#include <acpitabl.h>
//#include  <wdmguid.h>
VOID 
GetDeviceObjectInfo(PDEVICE_OBJECT DevObj)
{
  POBJECT_HEADER ObjectHeader;
  POBJECT_HEADER_NAME_INFO ObjectNameInfo;

  if (DevObj == NULL)
  {
    KdPrint(("DevObject is null\n"));
  }
  ObjectHeader = OBJECT_TO_OBJECT_HEADER(DevObj);
  if (ObjectHeader)
  {
    // query dvice name and print
    ObjectNameInfo = OBJECT_HEADER_TO_NAME_INFO(ObjectHeader);
    if (ObjectNameInfo && ObjectNameInfo->Name.Buffer)
    {
        KdPrint(("Driver Name: %wZ = Device Name:%wZ - Driver Addresss 0x%x - Device Addresss:0x%x\n",
        &DevObj->DriverObject->DriverName,
        &ObjectNameInfo->Name,
        DevObj->DriverObject,
        DevObj));

    }

    // if unmae device 
    else if( DevObj->DriverObject)
    {
      KdPrint(("Driver Name: %wZ = Device Name:%wZ - Driver Addresss 0x%x - Device Addresss:0x%x\n",
        &DevObj->DriverObject->DriverName,
        L"NULL",
        DevObj->DriverObject,
        DevObj));
    }
  }
}

VOID GetAttachdDeviceInfo(PDEVICE_OBJECT DevObj)
{
  PDEVICE_OBJECT DeviceObject;
  if (DevObj == NULL)
  {
    KdPrint(("DevObj is NULL \n"));
    return;
  }
  DeviceObject = DevObj->AttachedDevice;
  // enumerate each device from stack
  while (DeviceObject)
  {
    KdPrint(("Attached Driver Name:%wZ, Attached Driver Address:0x%x, Attached DeviceAddress:0x%x\n",
      &DeviceObject->DriverObject->DriverName,
      DeviceObject->DriverObject,
      DeviceObject));
    // Get DeviceOject from Device's Stack
    DeviceObject = DeviceObject->AttachedDevice;
  }
}

#define APCI_DRIVER_NAME L"\\Driver\\ACPI"


#if 0
PDRIVER_OBJECT
EnumDeviceStack(PWSTR )
{
  UNICODE_STRING DriverName;
  PDRIVER_OBJECT DriverObject = NULL;
  PDEVICE_OBJECT DeviceObject = NULL;
  // initialize unicode string 
  RtlInitUnicodeString(&DriverName, APCI_DRIVER_NAME);

  KdPrint(("Start enumeration driver object \n"));
  KdPrint(("DriverName = %wZ\n", DriverName));
  ObReferenceObjectByName(&DriverName, OBJ_CASE_INSENSITIVE, NULL, 0,
    (POBJECT_TYPE)IoDriverObjectType,
    KernelMode,
    NULL,
    (PVOID*)&DriverObject);

  if (NULL == DriverObject)
  {
    KdPrint(("DriverObject = NULL, Cannnot enum this driver object\n"));
    return NULL;
  }
  DeviceObject = DriverObject->DeviceObject;
  while (DeviceObject)
  {
    GetDeviceObjectInfo(DeviceObject);
    if (DeviceObject->AttachedDevice)
    {
      GetAttachdDeviceInfo(DeviceObject);
    }
    if (DeviceObject->Vpb && DeviceObject->Vpb->DeviceObject)
    {
      // grabe device objct
      GetDeviceObjectInfo(DeviceObject->Vpb->DeviceObject);
      if (DeviceObject->Vpb->DeviceObject->AttachedDevice)
      {
        GetAttachdDeviceInfo(DeviceObject->Vpb->DeviceObject);
      }
    }
    // get next devices
    DeviceObject = DeviceObject->NextDevice;
  }
  return DriverObject;
}

#endif  // EnumDeviceStack
DEFINE_GUID(GUID_ACPI_INTERFACE_STANDARD2, 0xe8695f63L, 0x1831, 0x4870, 0xa8, 0xcf, 0x9c, 0x2f, 0x03, 0xf9, 0xdc, 0xb5);

#define SIMPLE_DRIVER_PATH L"\\Device\\0000004b"
PDRIVER_OBJECT EnumDeviceStack(PWSTR)
{
#if 0
  PWSTR pwSymbolicLinkList = NULL;
  NTSTATUS status = STATUS_SUCCESS;
  status = IoGetDeviceInterfaces(&GUID_ACPI_INTERFACE_STANDARD2,
    NULL, 
    DEVICE_INTERFACE_INCLUDE_NONACTIVE, 
    &pwSymbolicLinkList
  );
#else
  UNICODE_STRING Win32DeviceName = { 0 };
  PFILE_OBJECT  fileObject = NULL;
  PDEVICE_OBJECT LowerDeviceObject = NULL;
  RtlInitUnicodeString(&Win32DeviceName, SIMPLE_DRIVER_PATH);
  NTSTATUS status = IoGetDeviceObjectPointer(&Win32DeviceName, FILE_ALL_ACCESS, &fileObject, &LowerDeviceObject);
  if (!NT_SUCCESS(status))
  {
//    STATUS_SUCCESS
    KdPrint(("EnumStack: fail with status = %x\n", status));
    return NULL;
  }
  KdPrint(("EnumStack: fileObject=%x, LowerDeviceobjetc = %x\n", fileObject, LowerDeviceObject));
#endif;
  return NULL;
}
