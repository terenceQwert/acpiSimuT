#pragma once


#define NUMBER_HASH_BUCKETS 37
#define OBJECT_TO_OBJECT_HEADER( o ) \
    CONTAINING_RECORD( (o), OBJECT_HEADER, Body )
#define OBJECT_HEADER_TO_NAME_INFO( oh ) ((POBJECT_HEADER_NAME_INFO) \
    ((oh)->NameInfoOffset == 0 ? NULL : ((PCHAR)(oh) - (oh)->NameInfoOffset)))

#define ACPI_PATH L"\\Driver\\ACPI"

typedef struct _OBJECT_CREATE_INFORMATION
{
  ULONG Attributes;
  PVOID RootDirectory;
  PVOID ParseContext;
  CHAR ProbeMode;
  ULONG PagedPoolCharge;
  ULONG NonPagedPoolCharge;
  ULONG SecurityDescriptorCharge;
  PVOID SecurityDescriptor;
  PSECURITY_QUALITY_OF_SERVICE SecurityQos;
  SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
} OBJECT_CREATE_INFORMATION, *POBJECT_CREATE_INFORMATION;
typedef struct _OBJECT_HEADER
{
  LONG PointerCount;
  union
  {
    LONG HandleCount;
    PVOID NextToFree;
  };
  POBJECT_TYPE Type;
  UCHAR NameInfoOffset;
  UCHAR HandleInfoOffset;
  UCHAR QuotaInfoOffset;
  UCHAR Flags;
  union
  {
    POBJECT_CREATE_INFORMATION ObjectCreateInfo;
    PVOID QuotaBlockCharged;
  };
  PVOID SecurityDescriptor;
  QUAD Body;
} OBJECT_HEADER, *POBJECT_HEADER;
typedef struct _OBJECT_DIRECTORY
{
  struct _OBJECT_DIRECTORY_ENTRY* HashBuckets[NUMBER_HASH_BUCKETS];
  struct _OBJECT_DIRECTORY_ENTRY** LookupBucket;
  BOOLEAN LookupFound;
  USHORT SymbolicLinkUsageCount;
  struct _DEVICE_MAP* DeviceMap;
} OBJECT_DIRECTORY, *POBJECT_DIRECTORY;
typedef struct _OBJECT_HEADER_NAME_INFO
{
  POBJECT_DIRECTORY Directory;
  UNICODE_STRING Name;
  ULONG Reserved;
#if DBG
  ULONG Reserved2;
  LONG DbgDereferenceCount;
#endif
} OBJECT_HEADER_NAME_INFO, *POBJECT_HEADER_NAME_INFO;



extern "C"
{
  NTKERNELAPI
    NTSTATUS
    ObReferenceObjectByName(
      IN PUNICODE_STRING ObjectName,
      IN ULONG Attributes,
      IN PACCESS_STATE PassedAccessState OPTIONAL,
      IN ACCESS_MASK DesiredAccess OPTIONAL,
      IN POBJECT_TYPE ObjectType,
      IN KPROCESSOR_MODE AccessMode,
      IN OUT PVOID ParseContext OPTIONAL,
      OUT PVOID *Object
    );
  NTKERNELAPI
    PDEVICE_OBJECT
    NTAPI
    IoGetBaseFileSystemDeviceObject(
      IN PFILE_OBJECT FileObject
    );
  extern POBJECT_TYPE IoDeviceObjectType;
  extern POBJECT_TYPE *IoDriverObjectType;
}

PDRIVER_OBJECT
EnumDeviceStack(PWSTR pwsDeviceName);
