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

#include "RegionFactory.h"

#ifdef _WIN32
#undef ERROR
#endif
#include <glog/logging.h>

#include "BadgeRegionFactory.h"
#include "CityFactory.h"
#include "ObjectFactoryCallback.h"
#include "RegionObject.h"
#include "SpawnRegionFactory.h"

#include "Utils/utils.h"

#include <assert.h>

using namespace std;

RegionFactory::RegionFactory(Database* database) : FactoryBase(database)
{
    mCityFactory = make_shared<CityFactory>(mDatabase);
    mBadgeRegionFactory = make_shared<BadgeRegionFactory>(mDatabase);
    mSpawnRegionFactory = make_shared<SpawnRegionFactory>(mDatabase);
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
        LOG(ERROR) << "Unknown group [" << subGroup << "]";
        break;
    }
}
