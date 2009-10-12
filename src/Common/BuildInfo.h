/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_COMMON_BUILDINFO_H
#define ANH_COMMON_BUILDINFO_H

#include <string>
#include "Utils/typedefs.h"

//======================================================================================================================

std::string GetBuildNumber();
std::string GetBuildTime();
std::string GetBuildString();

#endif // ANH_COMMON_BUILDINFO_H


