#ifdef __cplusplus
extern "C"
{
#include <oprghdlr.h>
#include <acpiioct.h>
}
#endif



PVOID   g_OpRegionSharedMemory = 0;
PVOID   g_OperationRegionObject = 0;
#define ACPISIM_OPREGION_TYPE      0x81
#define OPREGION_SIZE               1024    // use a hardcoded value of 1024 for our operation region size
#define ACPISIM_POOL_TAG            (ULONG) 'misA'
#define ACPISIM_TAG                 (ULONG) 'misA'


NTSTATUS
AcpisimOpRegionHandler(
    ULONG AccessType,
    PVOID /*OperationRegionObject*/,
    ULONG Address,
    ULONG Size,
    PULONG Data,
    ULONG_PTR /*Context*/,
    PACPI_OP_REGION_CALLBACK /*CompletionHandler*/,
    PVOID /* CompletionContext */
  )
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  KdPrint(("AcpisimOpRegionHandler Entry\n"));
  UCHAR * pData = 0;
  //
  // Insert additional handler code here
  //
  switch (AccessType)
  {
  case ACPI_OPREGION_WRITE:
    RtlCopyMemory((PVOID)((ULONG_PTR)g_OpRegionSharedMemory + Address), Data, Size);
    pData = (UCHAR*)((ULONG_PTR)g_OpRegionSharedMemory + Address);
    KdPrint(("ACPI_OPREGION_WRITE data = 0x%x\n", *pData));
    status = STATUS_SUCCESS;
    break;
  case ACPI_OPREGION_READ:
    RtlCopyMemory(Data, (PVOID)((ULONG_PTR)g_OpRegionSharedMemory + Address), Size);
    KdPrint(("ACPI_OPREGION_READ data = 0x%x\n", *Data));
    status = STATUS_SUCCESS;
    break;
    break;
  default:
    KdPrint(("Unknown Opregion access type. Ignoreing\n"));
    status = STATUS_INVALID_DEVICE_REQUEST;
  }
  KdPrint(("AcpisimOpRegionHandler Exit\n"));

  return STATUS_SUCCESS;
}

VOID
AcpiInstallOpRegionHandler(
  IN PDEVICE_OBJECT pDevObj
)
{
  NTSTATUS status = STATUS_UNSUCCESSFUL;
  g_OpRegionSharedMemory = ExAllocatePoolWithTag(
    NonPagedPool,
    OPREGION_SIZE,
    ACPISIM_POOL_TAG
  );
  RtlZeroMemory(g_OpRegionSharedMemory, OPREGION_SIZE);
  status = RegisterOpRegionHandler(
    pDevObj,
    ACPI_OPREGION_ACCESS_AS_COOKED,
    ACPISIM_OPREGION_TYPE,
    (PACPI_OP_REGION_HANDLER)AcpisimOpRegionHandler,
    (PVOID)ACPISIM_TAG,
    0,
    &g_OperationRegionObject
  );

}


