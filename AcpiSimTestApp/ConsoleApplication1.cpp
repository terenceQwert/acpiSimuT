// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <SetupAPI.h>
#include <winioctl.h>
#include <process.h>
#include <string>
#include "appFunction.h"
//#include "../PciCommon.h"
// #include  "../../DriverB/HelloWdmIoControl.h"
// #include  "../../DriverB/Feature_Flag.h"
#include "../PciCommon.h"
#include "../AcpiSmiWdmIoControl.h"
#include  "../Feature_Flag.h"

using namespace std;
#if 0
void _cdecl operator delete (void *pointer)
{
  std::cout << "delete invoke";
  delete [] pointer;
  pointer = NULL;
}
#endif



using namespace std;
// {27056255-0FAC-42F8-BCED-E646141A5915}
static const GUID gGuid =
{ 0x27056255, 0xfac, 0x42f8, { 0xbc, 0xed, 0xe6, 0x46, 0x14, 0x1a, 0x59, 0x15 } };


#define WRITE_LENGTH sizeof(GUID)
void Read( HANDLE devHandler)
{
  UCHAR *buffer = new UCHAR[WRITE_LENGTH];
  memset(buffer, 0, WRITE_LENGTH);
  ULONG ulRead = 0;

  BOOL bResult = ReadFile(devHandler, buffer, WRITE_LENGTH, &ulRead, NULL);
  if (bResult)
  {
    printf("Read %d bytes", ulRead);
    for (ULONG i = 0; i < ulRead; i++)
    {
      printf("0x%02x ", buffer[i]);
    }
    printf("\n");
  }
  else
  {
    printf("GetLastError() = %d\n", GetLastError());
  }

}

VOID ReadEx(HANDLE devHandler)
{
  UCHAR *buffer = new UCHAR[WRITE_LENGTH];
  memset(buffer, 0, WRITE_LENGTH);
  ULONG ulRead = 0;
  OVERLAPPED ov1 = { 0 };
  OVERLAPPED ov2 = { 0 };

  BOOL bResult = ReadFile(devHandler, buffer, WRITE_LENGTH, &ulRead, &ov1);
  if (bResult)
  {
    printf("Read %d bytes", ulRead);
    for (ULONG i = 0; i < ulRead; i++)
    {
      printf("0x%02x ", buffer[i]);
    }
    printf("\n");
  }
  else if ( !bResult && GetLastError() == ERROR_IO_PENDING)
  {
    printf("The operation is pending \n");
  }
  else
  {
    printf("GetLastError() = %d\n", GetLastError());
  }

  Sleep(2000);
  CancelIo(devHandler);
  
  delete[] buffer;
}

void Write(HANDLE devHandler)
{
#if BUFFER_IO // current driver is with BUFFER_IO
  UCHAR *buffer = new UCHAR[WRITE_LENGTH];
  DWORD ulWrite = 0;
  memcpy_s(buffer, WRITE_LENGTH, &gGuid, WRITE_LENGTH);
  BOOL bResult = WriteFile(devHandler, buffer, WRITE_LENGTH, &ulWrite, NULL);
  if (bResult)
  {
    printf("write data success, length = %d\n", ulWrite);
  }
#endif
}

void IOCTL( HANDLE devHandle, int arg1)
{
#define DRIVER_NAME_BUFFER 128

  WCHAR *pwDeviceName = new WCHAR[128];
//  wcscpy_s(pwDeviceName, DRIVER_NAME_BUFFER, L"DRIVER\\ACPI");
  wcscpy_s(pwDeviceName, DRIVER_NAME_BUFFER, L"Device\\00000013");
  int InputBuffer = arg1;
  printf("InputBuffer = 0x%x\n", InputBuffer);
  ULONG cbin = sizeof(ULONG);
//  ULONG cbin = sizeof(GUID);
//  UCHAR *InputBuffer = new UCHAR[sizeof(GUID)];
  ULONG cbout = sizeof(GUID);
  UCHAR *OutputBuffer = new UCHAR[sizeof(GUID)];
  DWORD bBytesReturn = 0;

//  memset(InputBuffer, 0xEE, sizeof(GUID));
  memset(OutputBuffer, 0, sizeof(GUID));

  BOOL bResult = DeviceIoControl(
    devHandle,
    IOCTL_TEST1,
//    InputBuffer,
//    cbin,
//    pwDeviceName,
    &InputBuffer,
    cbin,
    OutputBuffer,
    cbout,
    &bBytesReturn,
    NULL
  );
}


void DisplayPCIConfiguration(HANDLE hDevice, int bus, int dev, int fun)
{
  DWORD dwAdddr;
  DWORD dwData;
  PCI_COMMON_CONFIG pci_configuration;
  PCI_SLOT_NUMBER SlotNumber;
  SlotNumber.u.AsULONG = 0;
  SlotNumber.u.bits.DeviceNumber = dev;
  SlotNumber.u.bits.FunctionNumber = fun;
  dwAdddr = 0x80000000 | (bus << 16) | (SlotNumber.u.AsULONG << 8);
  for (int i = 0; i < 0x80; i += 4)
  {
    Out_32(hDevice, PCI_CONFIG_ADDRESS, dwAdddr | i);
    dwData = In_32(hDevice, PCI_CONFIG_DATA);
    memcpy(((PUCHAR)&pci_configuration) + i, &dwData, 4);
  }
  printf("bus:%d\tdev:%d\tfunc:%d\n", bus, dev, fun);
  printf("VendorID:%x\n", pci_configuration.VendorID);
  printf("DeviceID:%x\n", pci_configuration.DeviceID);
  printf("Command:%x\n", pci_configuration.Command);
  printf("Status:%x\n", pci_configuration.Status);
  printf("RevisionID:%x\n", pci_configuration.Status);
  printf("ProgIf:%x\n", pci_configuration.ProgIf);
  printf("SubClass:%x\n", pci_configuration.SubClass);
  printf("BaseClass:%x\n", pci_configuration.BaseClass);
  printf("CacheLineSize:%x\n", pci_configuration.CacheLineSize);
  printf("LatencyTimer:%x\n", pci_configuration.LatencyTimer);
  printf("HeaderType:%x\n", pci_configuration.HeaderType);
  printf("BIST:%x\n", pci_configuration.BIST);
  for (int i = 0; i < 6; i++)
  {
    printf("BaseAddress[%d]:0X%08X\n", i, pci_configuration.u.type0.BaseAddresses[i]);
  }
  printf("InterruptLine:%d\n", pci_configuration.u.type0.InterruptLine);
  printf("InterruptPin:%d\n", pci_configuration.u.type0.InterruptPin);
}

void timer_function(int argc, char **argv, HANDLE hDevice)
{
  if (argc > 1)
  {
    string str = argv[1];
    if (str == "timer")
    {
      str = argv[2];
      if (str == "start") {
        ExecuteTimerStartControl(hDevice, 3*1000*1000);
        }
      else
        ExecuteTimerStopControl(hDevice);
    }
  }
}

int main( int argc, char **argv)
{
  HANDLE hDevice = NULL;

#if USE_NAME
   hDevice = CreateFile(DRV_NAME, GENERIC_READ | GENERIC_WRITE,
    0, NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    NULL);
  if (hDevice == INVALID_HANDLE_VALUE)
  {
    std::cout << "Fail TO OBTAIN FILE HANDLE TO DEVICE" << endl;
    return 1;
  }
#else
  hDevice = GetDeviceViaInterfaces(&MyWDMDevice, 0);
#endif

#if USE_IRP_PENDING
//  ReadEx(hDevice);
#else 
  Read(hDevice);
  Write(hDevice);
  Read(hDevice);
#endif

//  timer_function(argc, argv, hDevice);
//  DisplayPCIConfiguration(hDevice, 2, 0, 0);
  cout << "start doing ioctl  " << endl;
  if (argc < 2)
  {
    cout << "cp (1)" << endl;
    IOCTL(hDevice, 0);
  }
  else
  {
    cout << "cp (2)" << endl;
    IOCTL(hDevice, strtoul(argv[1],NULL,0));
  }
#if 0
//#if DRIVER_START_IO
  HANDLE hThread[2];
  hThread[0] = (HANDLE)_beginthreadex(NULL, 0, Thread, &hDevice, 0, NULL);
  hThread[1] = (HANDLE)_beginthreadex(NULL, 0, Thread, &hDevice, 0, NULL);

  WaitForMultipleObjects(2, hThread, TRUE, INFINITE);
  cout << " finish driver StartIO call" << endl;
#endif


  CloseHandle(hDevice);
  std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
