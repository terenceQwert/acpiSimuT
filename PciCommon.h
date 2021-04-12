#pragma once

#define PCI_CONFIG_ADDRESS      0xCF8
#define PCI_CONFIG_DATA         0xCFC

#define PDI_BUS_MAX 0xff
#define PDI_DEVICE_MAX  0x1f
#define PDI_FUNCTION_MAX  0x7

#define PCI_TYPE0_ADDRESSES             6
#define PCI_TYPE1_ADDRESSES             2
#define PCI_TYPE2_ADDRESSES             5

typedef struct _PCI_COMMON_CONFIG {
  USHORT  VendorID;                   // (ro)
  USHORT  DeviceID;                   // (ro)
  USHORT  Command;                    // Device control
  USHORT  Status;
  UCHAR   RevisionID;                 // (ro)
  UCHAR   ProgIf;                     // (ro)
  UCHAR   SubClass;                   // (ro)
  UCHAR   BaseClass;                  // (ro)
  UCHAR   CacheLineSize;              // (ro+)
  UCHAR   LatencyTimer;               // (ro+)
  UCHAR   HeaderType;                 // (ro)
  UCHAR   BIST;                       // Built in self test

  union {
    struct _PCI_HEADER_TYPE_0 {
      ULONG   BaseAddresses[PCI_TYPE0_ADDRESSES];
      ULONG   CIS;
      USHORT  SubVendorID;
      USHORT  SubSystemID;
      ULONG   ROMBaseAddress;
      UCHAR   CapabilitiesPtr;
      UCHAR   Reserved1[3];
      ULONG   Reserved2;
      UCHAR   InterruptLine;      //
      UCHAR   InterruptPin;       // (ro)
      UCHAR   MinimumGrant;       // (ro)
      UCHAR   MaximumLatency;     // (ro)
    } type0;

    // end_wdm end_ntminiport end_ntndis

            //
            // PCI to PCI Bridge
            //

    struct _PCI_HEADER_TYPE_1 {
      ULONG   BaseAddresses[PCI_TYPE1_ADDRESSES];
      UCHAR   PrimaryBus;
      UCHAR   SecondaryBus;
      UCHAR   SubordinateBus;
      UCHAR   SecondaryLatency;
      UCHAR   IOBase;
      UCHAR   IOLimit;
      USHORT  SecondaryStatus;
      USHORT  MemoryBase;
      USHORT  MemoryLimit;
      USHORT  PrefetchBase;
      USHORT  PrefetchLimit;
      ULONG   PrefetchBaseUpper32;
      ULONG   PrefetchLimitUpper32;
      USHORT  IOBaseUpper16;
      USHORT  IOLimitUpper16;
      UCHAR   CapabilitiesPtr;
      UCHAR   Reserved1[3];
      ULONG   ROMBaseAddress;
      UCHAR   InterruptLine;
      UCHAR   InterruptPin;
      USHORT  BridgeControl;
    } type1;

    //
    // PCI to CARDBUS Bridge
    //

    struct _PCI_HEADER_TYPE_2 {
      ULONG   SocketRegistersBaseAddress;
      UCHAR   CapabilitiesPtr;
      UCHAR   Reserved;
      USHORT  SecondaryStatus;
      UCHAR   PrimaryBus;
      UCHAR   SecondaryBus;
      UCHAR   SubordinateBus;
      UCHAR   SecondaryLatency;
      struct {
        ULONG   Base;
        ULONG   Limit;
      }       Range[PCI_TYPE2_ADDRESSES - 1];
      UCHAR   InterruptLine;
      UCHAR   InterruptPin;
      USHORT  BridgeControl;
    } type2;

    // begin_wdm begin_ntminiport begin_ntndis

  } u;

  UCHAR   DeviceSpecific[192];

} PCI_COMMON_CONFIG, *PPCI_COMMON_CONFIG;

