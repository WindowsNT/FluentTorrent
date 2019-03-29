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


# To Build

1. Undef PRE_BUILD in stdafx.h, this will exclude my auto-update library which is not open source
2. Compiler - Additional Include Directories -> Put include paths of libtorrent and boost 
3. Linker - Additional Libraries -> Include boost static libraries

Boost has different setups depending on x86/x64. Use the appropriate installers.
You need VS 2017+, Windows 10 Build 1903+ and SDK 17763+



