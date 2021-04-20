#pragma once

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
