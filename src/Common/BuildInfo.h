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

std::string GetBuildNumber(void)
{
    // @note: this string is special, it is set by subversion each time a
    // commit is made. Before returning the value to the caller, remove the extra
    // information that subversion adds.
    static std::string build_num("$Revision$");
	return build_num.substr(11, build_num.length()-13);
}

std::string GetBuildTime(void)
{
    // @note: this string is special, it is set by subversion each time a
    // commit is made. Before returning the value to the caller, remove the extra
    // information that subversion adds.
    static std::string build_time("$Date$");
	return build_time.substr(7, build_time.length()-9);
}

std::string GetBuildString(void)
{
    static std::string build_string;

    if (build_string.length() == 0) 
    {
        build_string += "swgANH "ANH_VERSION_NAME" - "ANH_VERSION_MAJOR"."ANH_VERSION_MINOR".";
        build_string += GetBuildNumber();
        build_string += " - ";
        build_string += GetBuildTime();
    }

	return build_string;
}

#endif // ANH_COMMON_BUILDINFO_H


