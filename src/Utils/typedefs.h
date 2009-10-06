/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_TYPEDEFS_H
#define ANH_TYPEDEFS_H

//=====================================================================================
//
// Version settings
//

#define ANH_VERSION_MAJOR	"0"
#define ANH_VERSION_MINOR	"1"
#define ANH_VERSION_NAME	"Dev"

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

#elif defined(__GNUC__)
	#define ANH_COMPILER ANH_COMPILER_GNUC
	#define ANH_COMP_VER (((__GNUC__)*100) + (__GNUC_MINOR__*10) + \
		__GNUC_PATCHLEVEL__)
#else
	#pragma error "Compiler not supported!"
#endif

//=====================================================================================
//
// Disable unwanted warnings
//
#ifdef _MSC_VER
	#pragma warning(disable : 4244)	
	#pragma warning(disable : 4267) 
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
// Set Windows specific stuff
//
#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)
	
	#ifndef _WINSOCK2API_
#include <WINSOCK2.h>
#endif

#ifndef _WINDOWS_
#include <windows.h>
#endif /* _WINDOWS_ */
	
	#include <time.h>
	

	#define WS_VERSION MAKEWORD(2,0)

	//#define socklen_t int

	typedef unsigned long		ulong;
	typedef long long			int64;
	typedef unsigned long long  uint64;
	typedef signed long long	sint64;

	#undef CreateService   // needed for mutex.h
//
// Set Linux specific stuff
//
#elif(ANH_PLATFORM == ANH_PLATFORM_LINUX)
	#include <sys/timeb.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <stdint.h>
	#include <pthread.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <errno.h>

	#define INVALID_SOCKET	-1
	#define SOCKET_ERROR	-1
	#define closesocket		close

	typedef int64_t		int64;
	typedef uint64_t	uint64;
	typedef int64_t		sint64;
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

typedef char				int8;
typedef short				int16;
typedef long				int32;

typedef unsigned int        uint;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned long       uint32;

typedef signed char         sint8;
typedef signed short        sint16;
typedef signed long         sint32;

#include "bstring.h"  // Bad bad bad.  Don't include headers in headers if at all possible.  
// unfotunately, this is needed here for base type funtionality.
// This is also a circular dependency between bstring.h and typedefs.h
// do not move the order of this include.  Needs to be after all the standard
// type definitions.

typedef BString             string;

typedef unsigned int        SOCKET;

//=====================================================================================
//
// Platform specific Macros
//
#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)
	#define msleep(time) { Sleep(time); };
	#define ssleep(time) { Sleep(time*1000); };
#elif(ANH_PLATFORM == ANH_PLATFORM_LINUX)
	#define msleep(time) { usleep(1000 * time); };
	#define ssleep(time) { sleep(time); };
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

