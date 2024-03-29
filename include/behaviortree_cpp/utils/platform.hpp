//
// Platform.h
//
// $Id: //poco/1.3/Foundation/include/Poco/Platform.h#5 $
//
// Library: Foundation
// Package: Core
// Module:  Platform
//
// Platform and architecture identification macros.
//
// NOTE: This file may be included from both C++ and C code, so it
//       must not contain any C++ specific things.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

#ifndef Foundation_Platform_INCLUDED
#define Foundation_Platform_INCLUDED

//
// Platform Identification
//
#define BT_OS_FREE_BSD 0x0001
#define BT_OS_AIX 0x0002
#define BT_OS_HPUX 0x0003
#define BT_OS_TRU64 0x0004
#define BT_OS_LINUX 0x0005
#define BT_OS_MAC_OS_X 0x0006
#define BT_OS_NET_BSD 0x0007
#define BT_OS_OPEN_BSD 0x0008
#define BT_OS_IRIX 0x0009
#define BT_OS_SOLARIS 0x000a
#define BT_OS_QNX 0x000b
#define BT_OS_VXWORKS 0x000c
#define BT_OS_CYGWIN 0x000d
#define BT_OS_UNKNOWN_UNIX 0x00ff
#define BT_OS_WINDOWS_NT 0x1001
#define BT_OS_WINDOWS_CE 0x1011
#define BT_OS_VMS 0x2001

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS_FAMILY_BSD 1
#define BT_OS BT_OS_FREE_BSD
#elif defined(_AIX) || defined(__TOS_AIX__)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS BT_OS_AIX
#elif defined(hpux) || defined(_hpux)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS BT_OS_HPUX
#elif defined(__digital__) || defined(__osf__)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS BT_OS_TRU64
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__TOS_LINUX__)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS BT_OS_LINUX
#elif defined(__APPLE__) || defined(__TOS_MACOS__)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS_FAMILY_BSD 1
#define BT_OS BT_OS_MAC_OS_X
#elif defined(__NetBSD__)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS_FAMILY_BSD 1
#define BT_OS BT_OS_NET_BSD
#elif defined(__OpenBSD__)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS_FAMILY_BSD 1
#define BT_OS BT_OS_OPEN_BSD
#elif defined(sgi) || defined(__sgi)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS BT_OS_IRIX
#elif defined(sun) || defined(__sun)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS BT_OS_SOLARIS
#elif defined(__QNX__)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS BT_OS_QNX
#elif defined(unix) || defined(__unix) || defined(__unix__)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS BT_OS_UNKNOWN_UNIX
#elif defined(_WIN32_WCE)
#define BT_OS_FAMILY_WINDOWS 1
#define BT_OS BT_OS_WINDOWS_CE
#elif defined(_WIN32) || defined(_WIN64)
#define BT_OS_FAMILY_WINDOWS 1
#define BT_OS BT_OS_WINDOWS_NT
#elif defined(__CYGWIN__)
#define BT_OS_FAMILY_UNIX 1
#define BT_OS BT_OS_CYGWIN
#elif defined(__VMS)
#define BT_OS_FAMILY_VMS 1
#define BT_OS BT_OS_VMS
#endif

#endif  // Foundation_Platform_INCLUDED
