#include "appFunction.h"
#include <iostream>
using namespace std;
UINT WINAPI Thread(LPVOID context)
{
  cout << "Enter Thread " << endl;
  OVERLAPPED overlap = { 0 };
  overlap.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  UCHAR buffer[10];
  ULONG ulRead = 0;
  BOOL bRead = ReadFile(*(PHANDLE)context, buffer, 10, &ulRead, &overlap);
  WaitForSingleObject(overlap.hEvent, INFINITE);
  printf("Thread %s\n", buffer);
  return 0;
}

