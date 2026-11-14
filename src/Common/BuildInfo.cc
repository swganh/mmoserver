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

#include "BuildInfo.h"
#include "Utils/typedefs.h"

// ANH_BUILD_REVISION ("branch@commit", "-dirty" with local changes) and
// ANH_BUILD_COMMIT_TIME, written from git at build time.
#include "build_info_generated.h"

// "0.2.0 creatures@5b2bac2": the version and the git revision built.
std::string GetBuildNumber()
{
    return ANH_VERSION_MAJOR "." ANH_VERSION_MINOR "." ANH_VERSION_PATCH " " ANH_BUILD_REVISION;
}

// When the commit built was made; empty without git.
std::string GetBuildTime()
{
    return ANH_BUILD_COMMIT_TIME;
}

std::string GetBuildString()
{
    return GetBuildNumber() + " " + GetBuildTime();
}
