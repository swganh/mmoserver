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
	#define snprintf _snprintf
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
#define MAX_PACKET_SIZE  2048  // TODO:  This needs to be changed to a configuration variable for the network module

#define MAX_CLIENT_PACKET_SIZE  496  //
#define MAX_SERVER_PACKET_SIZE  2048  //
#define MAX_UNRELIABLE_PACKET_SIZE  1024  //

#if defined(__WIN32__) || defined(_WIN32)

typedef __int8            int8;
typedef __int16           int16;
typedef __int32           int32;
typedef __int64           int64;
typedef unsigned __int32  uint;
typedef unsigned __int8   uint8;
typedef unsigned __int16  uint16;
typedef unsigned __int32  uint32;
typedef unsigned __int64  uint64;

#else

#include <cstdint>

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
#endif

#include "bstring.h"  // Bad bad bad.  Don't include headers in headers if at all possible.
// unfotunately, this is needed here for base type funtionality.
// This is also a circular dependency between bstring.h and typedefs.h
// do not move the order of this include.  Needs to be after all the standard
// type definitions.

typedef BString             string;

typedef unsigned int        SOCKET;

// Windows and unix handle their long long specifiers differently since windows doesn't
// fully support C99. To make everything play nicely across platforms we define our own
// constants.
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)

#if !defined(PRId32)
#define PRId32 "I32d"
#endif

#if !defined(PRIu32)
#define PRIu32 "I32u"
#endif

#if !defined(PRIx32)
#define PRIx32 "I32x"
#endif

#if !defined(PRId64)
#define PRId64 "I64d"
#endif

#if !defined(PRIu64)
#define PRIu64 "I64u"
#endif

#if !defined(PRIx64)
#define PRIx64 "I64x"
#endif

#define WidePRId32 L"I32d"
#define WidePRIu32 L"I32u"
#define WidePRIx32 L"I32x"
#define WidePRId64 L"I64d"
#define WidePRIu64 L"I64u"
#define WidePRIx64 L"I64x"

// Else we're on a platform that handles this stuff correctly.
#else

#if (defined(_INTTYPES_H) || defined(_INTTYPES_H_)) && !defined(PRId64)
#error "inttypes.h has already been included before this header file, but "
#error "without __STDC_FORMAT_MACROS defined."
#endif

#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

// GCC will concatenate wide and narrow strings correctly, so nothing needs to
// be done here.
#define WidePRId32 PRId32
#define WidePRIu32 PRIu32
#define WidePRIx32 PRIx32
#define WidePRId64 PRId64
#define WidePRIu64 PRIu64
#define WidePRIx64 PRIx64

#endif

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

