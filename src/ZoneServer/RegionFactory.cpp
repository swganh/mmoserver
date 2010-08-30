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

#include "RegionFactory.h"
#include "BadgeRegionFactory.h"
#include "CityFactory.h"
#include "ObjectFactoryCallback.h"
#include "QTRegionFactory.h"
#include "RegionObject.h"
#include "SpawnRegionFactory.h"

#include "Common/LogManager.h"
#include "Utils/utils.h"

#include <assert.h>

//=============================================================================

bool				RegionFactory::mInsFlag    = false;
RegionFactory*		RegionFactory::mSingleton  = NULL;

//======================================================================================================================

RegionFactory*	RegionFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new RegionFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

RegionFactory::RegionFactory(Database* database) : FactoryBase(database)
{
    mCityFactory			= CityFactory::Init(mDatabase);
    mBadgeRegionFactory		= BadgeRegionFactory::Init(mDatabase);
    mSpawnRegionFactory		= SpawnRegionFactory::Init(mDatabase);
    mQTRegionFactory		= QTRegionFactory::Init(mDatabase);
}

//=============================================================================

RegionFactory::~RegionFactory()
{
    mInsFlag = false;
    delete(mSingleton);
}

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
    case Region_Zone:
        mQTRegionFactory->requestObject(ofCallback,id,subGroup,subType,client);
        break;

    default:
        gLogger->log(LogManager::DEBUG,"RegionFactory::requestObject Unknown Group\n");
        break;
    }
}

//=============================================================================

void RegionFactory::releaseAllPoolsMemory()
{
    mCityFactory->releaseQueryContainerPoolMemory();
    mBadgeRegionFactory->releaseQueryContainerPoolMemory();
    mSpawnRegionFactory->releaseQueryContainerPoolMemory();
    mQTRegionFactory->releaseQueryContainerPoolMemory();
}

//=============================================================================

