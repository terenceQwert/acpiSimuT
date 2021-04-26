#pragma once
#include <Windows.h>
HANDLE GetDeviceViaInterfaces(GUID * pGuid, DWORD instances);
DWORD In_32(HANDLE hDevice, USHORT port);
void Out_32(HANDLE hDevice, USHORT port, DWORD value);

//
typedef struct _PCI_SLOT_NUMBER {
  union {
    struct {
      ULONG   DeviceNumber : 5;
      ULONG   FunctionNumber : 3;
      ULONG   Reserved : 24;
    } bits;
    ULONG   AsULONG;
  } u;
} PCI_SLOT_NUMBER, *PPCI_SLOT_NUMBER;
//

UINT WINAPI Thread(LPVOID context);

VOID ExecuteTimerStartControl(HANDLE, ULONG timeInterval);
VOID ExecuteTimerStopControl(HANDLE);
