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
#include <oprghdlr.h>
#include <acpiioct.h>
#include <acpitabl.h>
// #include "PciCommon.h"
/*
** these two APIs HalGetBusData & HalSetBusData are depricated.
*/

#if 0
VOID EnumeratePCI()
{
  ULONG bus=0;
  ULONG dev;
  ULONG func;
  PCI_COMMON_CONFIG PciConfig;
  PCI_SLOT_NUMBER   SlotNumber;

  KdPrint(("Bus \tDevice\tFunc\tVendor\tDevice\tBaseCls\tSubCls\tIRQ\tPIN\n"));
  for (dev = 0; dev < PDI_DEVICE_MAX; dev++)
  {
    for (func = 0; func < PDI_FUNCTION_MAX; func++)
    {
      SlotNumber.u.AsULONG = 0;
      SlotNumber.u.bits.DeviceNumber = dev;
      SlotNumber.u.bits.FunctionNumber = func;
      RtlZeroMemory(&PciConfig, sizeof(PCI_COMMON_CONFIG));
//      ULONG size = HalGetBusData()
    }
  }
}

#endif

#define CM_STA_NAME (ULONG)('ATS_')
#define CM_PSR_NAME (ULONG)('RSP_')
// FOR COM Device 
#define CM_PRS_NAME (ULONG)('SRP_')
#define CM_DIS_NAME (ULONG)('SID_')

#define MY_TAG 'gTyM' // Poll tag for memory allocation
NTSTATUS
SendDownStreamIrp(
  IN PDEVICE_OBJECT Pdo,
  IN ULONG          Ioctl,
  IN PVOID          InputBuffer,
  IN ULONG          InputSize,
  IN PVOID          OutputBuffer,
  IN ULONG          OutputSize
)
{
  IO_STATUS_BLOCK ioBlock;
  KEVENT          myIoCtlEvent;
  NTSTATUS        status;
  PIRP            irp;

  KeInitializeEvent(&myIoCtlEvent, SynchronizationEvent, FALSE);

  // Build hte request
  irp = IoBuildDeviceIoControlRequest(
    Ioctl,
    Pdo,
    InputBuffer,
    InputSize,
    OutputBuffer,
    OutputSize,
    FALSE,
    &myIoCtlEvent,
    &ioBlock
  );
  if (!irp)
  {
    KdPrint(("SendDownStreamIrp fail @check point1\n"));
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  // Pass request to Pdo, always wait for complete routine
  status = IoCallDriver(Pdo, irp);
  if (status == STATUS_PENDING)
  {
    // wait for the IRP to be completed, and then return the status code
    KeWaitForSingleObject(
      &myIoCtlEvent,
      Executive,
      KernelMode,
      FALSE,
      NULL
    );
    status = ioBlock.Status;
  }
  return status;
}

VOID 
AcpiEvaluate( PDEVICE_OBJECT pDevObj, int opCode)
{
  ACPI_EVAL_INPUT_BUFFER inputBuffer;
  ACPI_EVAL_OUTPUT_BUFFER outputBuffer;
  
//  IO_STATUS_BLOCK         ioStatus;
  NTSTATUS                status;
  PACPI_METHOD_ARGUMENT argument;
//  PIRP                  irp;

  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  KdPrint(("AcpiEvaluate -Start option = %d\n", opCode));

  PAGED_CODE();
  
  RtlZeroMemory(&inputBuffer, sizeof(ACPI_EVAL_INPUT_BUFFER));
  inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
  switch (opCode)
  {
  case 0:
    KdPrint(("_PSR run\n"));
    inputBuffer.MethodNameAsUlong = CM_PSR_NAME;
    break;
  case 0x21:
    KdPrint(("_PRS run\n"));
    inputBuffer.MethodNameAsUlong = CM_PRS_NAME;
    break;
  case 0x22:
    KdPrint(("_DIS run\n"));
    inputBuffer.MethodNameAsUlong = CM_DIS_NAME;
    break;
  default:
    KdPrint(("_STA run\n"));
    inputBuffer.MethodNameAsUlong = CM_STA_NAME;
    break;
  }
//  inputBuffer.MethodNameAsUlong = CM_SWS_NAME;

  //
  // Intilaize the outputbuffer
  //
  RtlZeroMemory(&outputBuffer, sizeof(ACPI_EVAL_OUTPUT_BUFFER));

#if 1
  SendDownStreamIrp(pDevExt->NextStackDevice,
    IOCTL_ACPI_EVAL_METHOD,
    &inputBuffer,
    sizeof(ACPI_EVAL_INPUT_BUFFER),
    &outputBuffer,
    sizeof(ACPI_EVAL_OUTPUT_BUFFER)
  );
#else
  //
  // Initialize an IRP
  //
  irp = IoBuildDeviceIoControlRequest(
    IOCTL_ACPI_EVAL_METHOD,
    pDevExt->NextStackDevice,
    &inputBuffer,
    sizeof(ACPI_EVAL_INPUT_BUFFER),
    &outputBuffer,
    sizeof(ACPI_EVAL_OUTPUT_BUFFER),
    FALSE,
    NULL,
    &ioStatus
  );

  //
  // irp initialize failed?
  //
  if (!irp)
  {
    status = STATUS_INSUFFICIENT_RESOURCES;
    return;
  }

  //
  // Send to ACPI driver
  //
  status = IoCallDriver(pDevExt->NextStackDevice, irp);
  if (!NT_SUCCESS(status))
  {
    status = STATUS_UNSUCCESSFUL;
    KdPrint(("ACPI evaluatte fail check point 1 \n"));
    return;
  }
#endif  // 
  //
  // Crack the result
  // 
  argument = &(outputBuffer.Argument[0]);

  ///
  /// Check output buffer's signature
  ///
  if (outputBuffer.Signature != ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE)
  {
    KdPrint(("cannot find outputbuffer's signature\n"));
  }

  //
  // We are expecting an integer
  //
  if (argument->Type != ACPI_METHOD_ARGUMENT_INTEGER)
  {
    status = STATUS_ACPI_INVALID_DATA;
    KdPrint(("ACPI evaluatte fail check point 2 \n"));
    return;
  }
  KdPrint(("argument->Type = %d\n", argument->Type));
  KdPrint(("value = %d\n", argument->Argument));
  KdPrint(("AcpiEvaluate -End \n"));

}

NTSTATUS AcpiChildEnum(
  PDEVICE_OBJECT pDevObj, 
  int opCode
)
{
  ACPI_ENUM_CHILDREN_INPUT_BUFFER inputBuffer = { 0 };
  ACPI_ENUM_CHILDREN_OUTPUT_BUFFER  outputSizeBuffer = { 0 };
  PACPI_ENUM_CHILDREN_OUTPUT_BUFFER  outputBuffer;
  ULONG bufferSize;
  PACPI_ENUM_CHILD  childObject = NULL;
  ULONG index;
  NTSTATUS  status;
  PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
  KdPrint(("AcpiChildEnum Entry opCode = 0x%x\n", opCode));
  // Fill in the input data
  inputBuffer.Signature = ACPI_ENUM_CHILDREN_INPUT_BUFFER_SIGNATURE;
  inputBuffer.Flags = ENUM_CHILDREN_MULTILEVEL;

#if 1
  // Send the request along
  status = SendDownStreamIrp(
    pDevExt->NextStackDevice,
    IOCTL_ACPI_ENUM_CHILDREN,
    &inputBuffer,
    sizeof(inputBuffer),
    &outputSizeBuffer,
    sizeof(outputSizeBuffer)
  );


  if (STATUS_BUFFER_OVERFLOW != status)
  {
    // There should be at least one child device (that is the deivce itself)
    // Return error return status
  }

  // verify the data
  // Note: The NumberOfChildren returned by ACPI actually contains the required size 
  // when hte status returned is STATUS_BUFFER_OVERFLOW
  if ((outputSizeBuffer.Signature != ACPI_ENUM_CHILDREN_OUTPUT_BUFFER_SIGNATURE) ||
    (outputSizeBuffer.NumberOfChildren < sizeof(ACPI_ENUM_CHILDREN_OUTPUT_BUFFER)))
  {
    KdPrint(("AcpiChildEnum Exit with STATUS_ACPI_INVALID_DATA\n"));
    return STATUS_ACPI_INVALID_DATA;
  }

  //
  // Allocate a buffer to hold all the child devices
  //
  bufferSize = outputSizeBuffer.NumberOfChildren;
  outputBuffer = (PACPI_ENUM_CHILDREN_OUTPUT_BUFFER)ExAllocatePoolWithTag(PagedPool, bufferSize, MY_TAG);
  if (NULL == outputBuffer)
  {
    KdPrint(("AcpiChildEnum Exit with STATUS_INSUFFICIENT_RESOURCES\n"));
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  RtlZeroMemory(outputBuffer, bufferSize);

  // Allocate a new IRP with the new output buffer
  // Send another request together with the new output buffer
  status = SendDownStreamIrp(
    pDevExt->NextStackDevice,
    IOCTL_ACPI_ENUM_CHILDREN,
    &inputBuffer,
    sizeof(inputBuffer),
    outputBuffer,
    bufferSize
  );
  
  // verify the data
  if ((outputBuffer->Signature != ACPI_ENUM_CHILDREN_OUTPUT_BUFFER_SIGNATURE) ||
    (outputBuffer->NumberOfChildren == 0))
  {
    KdPrint(("AcpiChildEnum Exit(2) with STATUS_ACPI_INVALID_DATA\n"));
    return STATUS_ACPI_INVALID_DATA;
  }
  //
  // Skip the first child device because ACPI returns the devie itself
  // as the first child device
  //
  childObject = (PACPI_ENUM_CHILD)&(outputBuffer->Children[0]);
  for (index = 1; index < outputBuffer->NumberOfChildren; index++)
  {
    // Proceed to the next ACPI child device
    childObject = ACPI_ENUM_CHILD_NEXT(childObject);
    // Process each child device.
    KdPrint(("Print Name = %s\n", childObject->Name));
  }
#endif
  return STATUS_SUCCESS;
}



#pragma PAGEDCODE
NTSTATUS HelloWDMDeviceIoControl(
  IN PDEVICE_OBJECT pDevObj,
  IN PIRP pIrp
)
{
  NTSTATUS status = STATUS_SUCCESS;
  KdPrint(("HelloWDMDeviceIoControl Entry\n"));

  PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
//  ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
//  ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
  ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
//  UCHAR * inputBuffer = (UCHAR*)pIrp->AssociatedIrp.SystemBuffer;
//  UCHAR * OutputBuffer = (UCHAR*)pIrp->AssociatedIrp.SystemBuffer;
  PULONG dwIntputBuffer = (ULONG*)pIrp->AssociatedIrp.SystemBuffer;
  PULONG dwOutputBuffer = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

  ULONG info = 0;
//  UCHAR PORT = 0x70;
//  UCHAR Data = 0x71;
//  UCHAR Port_Offset = 0x50;
//  UCHAR buf = 0;
//  UCHAR index = 0;
  KSPIN_LOCK my_SpinLock;
//  KIRQL       irql;

  // IO Action
  ULONG port = (ULONG)(*dwIntputBuffer);
  dwIntputBuffer++;
  UCHAR method = (UCHAR)(*dwIntputBuffer);
  dwIntputBuffer++;
  ULONG value = (ULONG)(*dwIntputBuffer);
  KeInitializeSpinLock(&my_SpinLock);
  switch (code)
  {
  case IOCTL_TEST1:
#if 0
    KdPrint(("IOCTL_TEST1\n"));
    for (ULONG i = 0; i < cbin; i++)
    {
      KdPrint(("%X\n", inputBuffer[i]));
    }
    KeAcquireSpinLock(&my_SpinLock, &irql);
    for( index=0;index < 0x20;index++){
      WRITE_PORT_UCHAR((PUCHAR)PORT, Port_Offset+index);
      buf = READ_PORT_UCHAR((PUCHAR)Data);
      KdPrint(("Index_%0x =0x%0x \n", index, buf));
    }
    KeReleaseSpinLock(&my_SpinLock, irql);
    memset(OutputBuffer, 0xAA, cbout);
    info = cbout;
    EnumDeviceStack(ACPI_PATH);
#else
    {
      int *InputBuffer = (int*)pIrp->AssociatedIrp.SystemBuffer;
//      KdPrint(("%d\n", *InputBuffer));
//      PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
//      KdPrint(("IOCTL stack device(LowerDevice) = %x\n", pdx->LowerDevice));
//      KdPrint(("IOCTL stack device(NextStackDevice) = %x\n", pdx->NextStackDevice));
//      KdPrint(("IOCTL stack device(NextPeerDevice) = %x\n", pdx->NextPeerDevice));
      if (*InputBuffer == 0x100)
      {
//        NTSTATUS ntStatus = STATUS_SUCCESS;
        AcpiChildEnum(pDevObj, *InputBuffer);
      }
      else
      {
        AcpiEvaluate(pDevObj, *InputBuffer);
      }
//      EnumDeviceStack(InputBuffer);
    }
#endif
    break;
  case READ_PORT:
    KdPrint(("READ_PORT\n"));
    KdPrint(("port:%x\n", port));
    KdPrint(("method:%x\n", method));

    if (method == 1)
    {
      *dwOutputBuffer = READ_PORT_UCHAR((PUCHAR)port);
    }
    else if (method == 2)
    {
      *dwOutputBuffer = READ_PORT_USHORT((PUSHORT)port);
    }
    else if (method == 4)
    {
      *dwOutputBuffer = READ_PORT_ULONG((PULONG)port);
    }
    info = 4;
    break;
  case WRITE_PORT:
    KdPrint(("WRITE_PORT\n"));
    KdPrint(("port:%x\n", port));
    KdPrint(("method:%x\n", method));
    KdPrint(("value:%x\n", value));
    if (1 == method)
    {
      WRITE_PORT_UCHAR((PUCHAR)port, (UCHAR)value);
    } else if(2 == method)
    {
      WRITE_PORT_USHORT((PUSHORT)port, (USHORT)value);
    }
    else if (4 == method)
    {
      WRITE_PORT_ULONG((PULONG)port, (ULONG)value);
    }
    info = 0;
    break;
  case PCI_CONFIG:
    PCI_COMMON_CONFIG pci_config;
    status = ReadWriteConfigSpace(pDevObj, 0, &pci_config, 0, sizeof(PCI_COMMON_CONFIG));
    if (NT_SUCCESS(status))
    {
      KdPrint(("VendorID:%x\n", pci_config.VendorID));
      KdPrint(("DeviceID:%x\n", pci_config.DeviceID));
      KdPrint(("Command:%x\n", pci_config.Command));
      KdPrint(("Status:%x\n", pci_config.Status));
      KdPrint(("RevisionID:%x\n", pci_config.RevisionID));
      KdPrint(("ProgIf:%x\n", pci_config.ProgIf));
      KdPrint(("SubClass:%x\n", pci_config.SubClass));
      KdPrint(("BaseClass:%x\n", pci_config.BaseClass));
      KdPrint(("CacheLineSize:%x\n", pci_config.CacheLineSize));
      KdPrint(("LatencyTimer:%x\n", pci_config.LatencyTimer));
      KdPrint(("HeaderType:%x\n", pci_config.HeaderType));
      KdPrint(("BIST:%x\n", pci_config.BIST));
      for (int i = 0; i < 6; i++)
      {
        KdPrint(("BaseAddresses[%d]:0X%08X\n", i, pci_config.u.type0.BaseAddresses[i]));
      }
      KdPrint(("InterruptLine:%d\n", pci_config.u.type0.InterruptLine));
      KdPrint(("InterruptPin:%d\n", pci_config.u.type0.InterruptPin));
    }
    break;
  case START_TIMER:
    Start_Timer_Function( pDevObj, pIrp);
    break;
  case STOP_TIMER:
    Stop_Timer_Function( pDevObj, pIrp);
    break;
  default:
    status = STATUS_INVALID_VARIANT;
    break;
  }
  pIrp->IoStatus.Status = status;
  pIrp->IoStatus.Information = info;
  IoCompleteRequest(pIrp, IO_NO_INCREMENT);

  KdPrint(("HelloWDMDeviceIoControl Exit\n"));
  return status;
}
