# HelloWDM
- when using WinDBG
1. load symbol & source file, ex 
File-->Syumbol File Path => srv*;F:\R5800.1\WP\prebuilt\7180\driver\desktop\QCDXDriver;C:\tmp\HelloWDM\ARM64\Debug
File--> Source File Path => C:\tmp\HelloWDM
2. bp HelloWDM!DriverEntry
3. install driver from target Windows
