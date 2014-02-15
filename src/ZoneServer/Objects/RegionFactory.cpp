/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "RegionFactory.h"

#include "Utils/utils.h"
#include "anh/logger.h"

#include "BadgeRegionFactory.h"
#include "CityFactory.h"
#include "ZoneServer/Objects/Object/ObjectFactoryCallback.h"
#include "Zoneserver/Objects/RegionObject.h"
#include "ZoneServer/GameSystemManagers/Spawn Manager/SpawnRegionFactory.h"

#include <assert.h>

using namespace std;

RegionFactory::RegionFactory(swganh::app::SwganhKernel*	kernel) : FactoryBase(kernel)
{
    mCityFactory = make_shared<CityFactory>(kernel);
    mBadgeRegionFactory = make_shared<BadgeRegionFactory>(kernel);
    mSpawnRegionFactory = make_shared<SpawnRegionFactory>(kernel);
}

//=============================================================================

RegionFactory::~RegionFactory()
{}

//=============================================================================

void RegionFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    switch(subGroup)
    {
    case Region_City:
        mCityFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;
    case Region_Badge:
        mBadgeRegionFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;
    case Region_Spawn:
        mSpawnRegionFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;

    default:
        LOG(error) << "Unknown group [" << subGroup << "]";
        break;
    }
}
