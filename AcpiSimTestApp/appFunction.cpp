#include <iostream>
#include <Windows.h>
#include <SetupAPI.h>
#include <winioctl.h>

HANDLE GetDeviceViaInterfaces(GUID * pGuid, DWORD instances)
{
  HDEVINFO info = SetupDiGetClassDevs(pGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
  if (info == INVALID_HANDLE_VALUE)
  {
    printf("No HDEVINFO available for this GUID\n");
    return NULL;
  }

  SP_INTERFACE_DEVICE_DATA ifData;
  ifData.cbSize = sizeof(ifData);
  if (!SetupDiEnumDeviceInterfaces(info, NULL, pGuid, instances, &ifData))
  {
    printf("No SP_INTERFACE_DEVICE_DATA available for this GUID interface\n");
    SetupDiDestroyDeviceInfoList(info);
    return NULL;
  }
  DWORD RegLen;
  SetupDiGetDeviceInterfaceDetail(info, &ifData, NULL, 0, &RegLen, NULL);
  PSP_INTERFACE_DEVICE_DETAIL_DATA ifDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA) new TCHAR[RegLen];
  if (ifDetail == NULL)
  {
    SetupDiDestroyDeviceInfoList(info);
    return NULL;
  }
  // Get two symoblic link
  ifDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
  if (!SetupDiGetDeviceInterfaceDetail(info, &ifData, ifDetail, RegLen, NULL, NULL))
  {
    SetupDiDestroyDeviceInfoList(info);
    delete ifDetail;
    return NULL;
  }
  printf("Symbolic link is %ws\n", ifDetail->DevicePath);
#if USE_IRP_PENDING
  HANDLE rv = CreateFile(ifDetail->DevicePath,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
    NULL);
#else
  HANDLE rv = CreateFile(ifDetail->DevicePath,
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL);
#endif
  if (rv == INVALID_HANDLE_VALUE)
    rv = NULL;
  delete ifDetail;
  SetupDiDestroyDeviceInfoList(info);
  return rv;
}
