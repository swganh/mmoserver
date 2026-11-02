/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef ANH_TYPEDEFS_H
#define ANH_TYPEDEFS_H

//=====================================================================================
//
// Version settings
//

#define ANH_VERSION_MAJOR	"0"
#define ANH_VERSION_MINOR	"2"
#define ANH_VERSION_PATCH	"0"
#define ANH_VERSION_NAME	"dev"

//=====================================================================================
//
// Initial Platform/Compiler related settings
//
#define ANH_PLATFORM_WIN32	1
#define ANH_PLATFORM_LINUX	2

#define ANH_COMPILER_MSVC	1
#define ANH_COMPILER_GNUC	2

#define ANH_ARCHITECTURE_32	1
#define ANH_ARCHITECTURE_64	2

//=====================================================================================
//
// Set the initial compiler
//
#if defined(_MSC_VER)
#define ANH_COMPILER ANH_COMPILER_MSVC
#define ANH_COMP_VER _MSC_VER
#define localtime_r(a, b) localtime_s(b, a)

#elif defined(__GNUC__)
#define ANH_COMPILER ANH_COMPILER_GNUC
#define ANH_COMP_VER (((__GNUC__)*100) + (__GNUC_MINOR__*10) + \
		__GNUC_PATCHLEVEL__)
#else
#pragma error "Compiler not supported!"
#endif

//=====================================================================================
//
// Set the platform we are on
//
#if defined(__WIN32__) || defined(_WIN32)
#define ANH_PLATFORM ANH_PLATFORM_WIN32
#else
#define ANH_PLATFORM ANH_PLATFORM_LINUX
#endif

//=====================================================================================
//
// Common definitions
//

//495 client 1024 interserver
#define MAX_PACKET_SIZE  496  // TODO:  This needs to be changed to a configuration variable for the network module

#define MAX_CLIENT_PACKET_SIZE  496  //
#define MAX_SERVER_PACKET_SIZE  1400  //
#define MAX_UNRELIABLE_PACKET_SIZE  300  //

// Fixed-width types and their printf formats, the same on every platform:
// MSVC has had <cstdint>, <cinttypes> and a standard printf since 2015.
#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif

#include <cstdint>
#include <cinttypes>

typedef char		int8;
typedef int16_t		int16;
typedef int32_t		int32;

typedef uint32_t    uint;
typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;

typedef int8_t		sint8;
typedef int16_t		sint16;
typedef int32_t		sint32;

typedef int64_t		int64;
typedef uint64_t  	uint64;
typedef int64_t		sint64;

// Winsock's SOCKET is pointer-sized; elsewhere a socket is an int.
#if defined(_WIN32)
typedef uintptr_t SOCKET;
#else
typedef unsigned SOCKET;
#endif

// A wide string literal followed by a narrow one is wide (C++11), so the
// standard formats work in wide strings too.
#define WidePRId32 PRId32
#define WidePRIu32 PRIu32
#define WidePRIx32 PRIx32
#define WidePRId64 PRId64
#define WidePRIu64 PRIu64
#define WidePRIx64 PRIx64

//
// Common macros
//
#define SAFE_DELETE(a) {if(a) {delete a;a=NULL;}}


//=====================================================================================
//
// Compiler specific settings
//


//=====================================================================================
//
// Architecture specific settings
//
//#define SEND_BUFFER_SIZE = 8192

//=====================================================================================


#endif

