# FluentTorrent
A quick torrent client using Fluent Design and XAML islands.

# Binary distributions
https://www.turboirc.com/ft, prebuild x64/x86 static binaries with auto update

# Libraries used

* LibTorrent - https://libtorrent.org/
* UWPLib - https://github.com/WindowsNT/uwplib
* MT - https://github.com/WindowsNT/mt
* XML - https://github.com/WindowsNT/xml
* OpenSSL
* SQLite3
* Boost

# Windows Technologies Used
* XAML Islands and C++/WinRT: https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/get-started
* Antimalware Scan Interface: https://docs.microsoft.com/en-us/windows/desktop/AMSI/antimalware-scan-interface-portal

# To Build

You need:
* Boost
* LibTorrent
* VS 2017+
* Windows 10 Build 1903+
* SDK 17763+

1. Undef PRE_BUILD in stdafx.h, this will exclude my auto-update library which is not part of this project
2. Compiler - Additional Include Directories -> Put include paths of libtorrent and boost 
3. Linker - Additional Libraries -> Include boost static libraries

Boost has different setups depending on x86/x64. Use the appropriate installers.



