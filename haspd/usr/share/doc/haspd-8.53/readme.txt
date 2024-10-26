HASP Support for WINE on Linux/x86 (July 2005)
==============================================

This initial release includes a Win32 HASP HL API (version 1.12) enabling 
protected applications to run under WINE on Linux. WINE detection is 
handled automatically - the application can run on native Windows 
platforms as well as under WINE, without any adaptation.

Installation Notes
------------------

For HASP HL keys accessed locally, you need to start the "winehasp" Linux 
daemon before launching the protected program:
 
  ./winehasp

The aksusbd daemon and the optional aksparlnx driver must be installed and 
running before launching the winehasp daemon.

The HASP API and the winehasp daemon communicate over IP port 2970 by default. 
Should the port 2970 already be used by another service, the port
number can be changed by starting the winehasp daemon with the -p parameter:

  ./winehasp -p 1234

Note: If the port number is changed, you must also set the new port number in
WINE:

  wine setwinehaspport.exe 1234

Aladdin Knowledge Systems Ltd. (c) 1985 - 2005. All rights reserved.


