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
const char* GetBuildString(void)
{
	return "swgANH "ANH_VERSION_NAME" - "ANH_VERSION_MAJOR"."ANH_VERSION_MINOR".$WCREV$ - $WCNOW$";
}


#endif // ANH_COMMON_BUILDINFO_H


