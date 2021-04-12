#ifdef __cplusplus
extern "C"
{
#endif
#include <wdm.h>
#include <acpitabl.h>
#include <acpiioct.h>
#ifdef __cplusplus
}
#endif

#include "AcpiSmiWdmCommon.h"


void DumpDeviceStack(IN PDEVICE_OBJECT pdo)
{
  KdPrint(("------------------------------------\n"));
  KdPrint(("Begin Dump device stack....\n"));

  PDEVICE_OBJECT pDevice = pdo;
  int i = 0;
  for (; pDevice != NULL; pDevice = pDevice->NextDevice)
  {
    KdPrint(("The %d device in device stack \n", i++));
    KdPrint(("Device AttachdDevic:0x%08X\n", pDevice->AttachedDevice));
    KdPrint(("Device Next Device: 0x%08X\n", pDevice->NextDevice));
    KdPrint(("Device StackSzie:%d\n", pDevice->StackSize));
    KdPrint(("Driver Name = %wZ\n", pDevice->DriverObject->DriverName));
    KdPrint(("Device's DriverObject: 0x%08X", pDevice->DriverObject));
//    ACPI_ENUM_CHILDREN_INPUT_BUFFER_SIGNATURE
  }
  KdPrint(("************************************\n"));
}


void DisplayProcessName()
{
  KdPrint(("DisplayProcessName Start\n"));
  PEPROCESS pEProcess = PsGetCurrentProcess();
  PTSTR ProcessName = (PTSTR)(((ULONG64)pEProcess) + 0x174);
  KdPrint(("%s\n", ProcessName));
  KdPrint(("DisplayProcessName End\n"));
}



VOID LinkListTest()
{
  LIST_ENTRY linkListHead;
  InitializeListHead(&linkListHead);

  PMYDATASTRUCT pData;
  ULONG i = 0;
  KdPrint(("Begin insert to linking list\n"));
  for (i=0; i < 10; i++)
  {
    pData = (PMYDATASTRUCT)ExAllocatePoolWithTag(PagedPool, sizeof(PMYDATASTRUCT),'1111');
    pData->data_1 = i;
    InsertHeadList(&linkListHead, &pData->listEntry);
  }

  // dump from linking list
  KdPrint(("Begin remove from link list\n"));
  while (!IsListEmpty(&linkListHead))
  {
    PLIST_ENTRY pEntry = RemoveTailList(&linkListHead);
    pData = CONTAINING_RECORD(pEntry,
      MYDATASTRUCT,
      listEntry);
    KdPrint(("%d\n", pData->data_1));
    ExFreePool(pData);
  }
}

void CreateFileFromWDM()
{
  OBJECT_ATTRIBUTES objectAttributes;
  IO_STATUS_BLOCK ioStatus;
  HANDLE hFile;
  UNICODE_STRING logFileUnicodeString;

  RtlInitUnicodeString(&logFileUnicodeString,
    L"\\??\\C:\\1.log");
  InitializeObjectAttributes(&objectAttributes,
    &logFileUnicodeString,
    OBJ_CASE_INSENSITIVE,
    NULL,
    NULL);
  NTSTATUS ntStatus = ZwCreateFile(
    &hFile,
    GENERIC_WRITE,
    &objectAttributes,
    &ioStatus,
    NULL,
    FILE_ATTRIBUTE_NORMAL,
    FILE_SHARE_READ,
    FILE_OPEN_IF,
    FILE_SYNCHRONOUS_IO_NONALERT,
    NULL,
    0
  );
  if (NT_SUCCESS(ntStatus))
  {
    KdPrint(("Create file successfully\n"));
  }
  else
  {
    KdPrint(("Create file unsuccessfully\n"));
  }
  ZwClose(hFile);
}

