#pragma once
NTSTATUS
Acpi_Wmi_Registration(
  IN PDEVICE_EXTENSION pDevExt
);

NTSTATUS
QueryRegInfo(
  IN PDEVICE_OBJECT     DeviceObject,
  OUT ULONG             *RegFlags,
  OUT PUNICODE_STRING   Instancename,
  OUT PUNICODE_STRING   *RegistryPath,
  OUT PUNICODE_STRING   MofResoureName,
  OUT PDEVICE_OBJECT    *Pdo
);

NTSTATUS
QueryDataBlock(
  IN PDEVICE_OBJECT       DeviceObject,
  IN PIRP                 pIrp,
  IN ULONG                Guidindex,
  IN ULONG                InstanceIndex,
  IN ULONG                InstanceCount,
  IN OUT PULONG           InstanceLengthArray,
  IN ULONG                OutBufferSize,
  OUT PUCHAR              Buffer
);

NTSTATUS
SetDataBlock(
  IN PDEVICE_OBJECT fdo,
  IN PIRP irp,
  IN ULONG guidIndex,
  IN ULONG instIndex,
  IN ULONG bufSize,
  IN PUCHAR buffer
);

NTSTATUS
SetDataItem(
  IN PDEVICE_OBJECT fdo,
  IN PIRP           irp,
  IN ULONG          guidIndex,
  IN ULONG          instIndex,
  IN ULONG          id,
  IN ULONG          bufSize,
  IN PUCHAR         buffer
);


NTSTATUS 
FunctionControl(
  PDEVICE_OBJECT fdo, 
  PIRP irp,
  ULONG guidindex, 
  WMIENABLEDISABLECONTROL fcn, 
  BOOLEAN enable
);

NTSTATUS
ExecuteMehtod(
  IN PDEVICE_OBJECT fdo,
  IN PIRP           irp,
  IN ULONG          guidIndex,
  IN ULONG          instIndex,
  IN ULONG          id,
  IN ULONG          cbInbuf,
  OUT ULONG         cbOutbuf,
  IN OUT PUCHAR         buffer
);

