#include "appFunction.h"
#include <iostream>
#include "../AcpiSim/AcpiSmiWdmIoControl.h"

using namespace std;


VOID ExecuteTimerStartControl( HANDLE pDevHandle, ULONG timeInterval)
{
  ULONG cbin = sizeof(int);
  UCHAR *InputBuffer = new UCHAR[sizeof(GUID)];
  ULONG cbout = sizeof(GUID);
  UCHAR *OutputBuffer = new UCHAR[sizeof(GUID)];
  DWORD bBytesReturn = 0;

  memcpy(InputBuffer, &timeInterval, sizeof(int));
  memset(OutputBuffer, 0, sizeof(GUID));

  BOOL bResult = DeviceIoControl(
    pDevHandle,
    START_TIMER,
    InputBuffer,
    cbin,
    OutputBuffer,
    cbout,
    &bBytesReturn,
    NULL
  );


}

VOID ExecuteTimerStopControl(HANDLE pDevHandle)
{
  ULONG cbin = sizeof(GUID);
  UCHAR *InputBuffer = new UCHAR[sizeof(GUID)];
  ULONG cbout = sizeof(GUID);
  UCHAR *OutputBuffer = new UCHAR[sizeof(GUID)];
  DWORD bBytesReturn = 0;

  memset(InputBuffer, 0xEE, sizeof(GUID));
  memset(OutputBuffer, 0, sizeof(GUID));

  BOOL bResult = DeviceIoControl(
    pDevHandle,
    STOP_TIMER,
    InputBuffer,
    cbin,
    OutputBuffer,
    cbout,
    &bBytesReturn,
    NULL
  );

}