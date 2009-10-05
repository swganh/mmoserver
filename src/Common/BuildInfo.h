/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_COMMON_BUILDINFO_H
#define ANH_COMMON_BUILDINFO_H

#include "Utils/typedefs.h"

//======================================================================================================================
static const char* GetBuildString(void)
{
	return "swgANH "ANH_VERSION_NAME" - "ANH_VERSION_MAJOR"."ANH_VERSION_MINOR".1512 - 2009/10/04 21:41:13\n";
}

static const char* GetBuildNumber(void)
{
	return "1512";
}

static const char* GetBuildTime(void)
{
	return "2009/10/04 21:41:13";
}


#endif // ANH_COMMON_BUILDINFO_H


