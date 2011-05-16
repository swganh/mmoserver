/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "BuildInfo.h"
#include "Utils/typedefs.h"

#include <fstream>

std::string GetBuildNumber()
{
    // @note: this string is special, it is set by subversion each time a
    // commit is made. Before returning the value to the caller, remove the extra
    // information that subversion adds.
    static std::string build_num;

    if (build_num.length() == 0)
    {
        std::ifstream version_file("VERSION");

        std::string build_string = GetBuildString();
        build_num = ANH_VERSION_MAJOR"."ANH_VERSION_MINOR"."ANH_VERSION_PATCH"."+build_string.substr(0, build_string.find_first_of(" "));
    }

    return build_num;
}

std::string GetBuildTime()
{
    // @note: this string is special, it is set by subversion each time a
    // commit is made. Before returning the value to the caller, remove the extra
    // information that subversion adds.
    static std::string build_time;

    if (build_time.length() == 0)
    {
        std::string build_string = GetBuildString();
        build_time = build_string.substr(build_string.find_first_of(" ")+1);
    }

    return build_time;
}

std::string GetBuildString()
{
    static std::string build_revision;
    static std::string build_timestamp;

    if (build_revision.length() == 0)
    {
        std::ifstream version_file("VERSION");

        if (version_file.is_open())
        {
            std::getline(version_file, build_revision);
            std::getline(version_file, build_timestamp);

            if (! version_file.eof() && version_file.peek() != '\n') {
                std::getline(version_file, build_revision);
            }
        }

        version_file.close();
    }

    return build_revision+" "+build_timestamp;
}

