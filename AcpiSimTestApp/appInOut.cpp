#include <iostream>
#include <Windows.h>
#include <SetupAPI.h>
#include <winioctl.h>
#include "../AcpiSmiWdmIoControl.h"

DWORD In_32(HANDLE hDevice, USHORT port)
{
  DWORD dwOutput;
  DWORD inputBuffer[2] = {
    port,
    4
  };
  DWORD dwResult;
  DeviceIoControl(hDevice, READ_PORT, inputBuffer, sizeof(inputBuffer), &dwResult, sizeof(DWORD), &dwOutput, NULL);
  return dwResult;
}

void Out_32(HANDLE hDevice, USHORT port, DWORD value)
{
  DWORD  dwOutput;
  DWORD inputBuffer[3] = { port, 4, value };
  DeviceIoControl(hDevice, WRITE_PORT, inputBuffer, sizeof(inputBuffer), NULL, 0, &dwOutput, NULL);
}

