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

#include "WaypointFactory.h"
#include "WorldConfig.h"
#include "ObjectFactoryCallback.h"
#include "WaypointObject.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"

//=============================================================================

bool				WaypointFactory::mInsFlag    = false;
WaypointFactory*	WaypointFactory::mSingleton  = NULL;

//======================================================================================================================

WaypointFactory*	WaypointFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new WaypointFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

WaypointFactory::WaypointFactory(Database* database) : FactoryBase(database)
{
    _setupDatabindings();
}

//=============================================================================

WaypointFactory::~WaypointFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void WaypointFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case WaypointFQuery_MainData:
    {
        WaypointObject* waypoint = _createWaypoint(result);
        // can't check waypoints on other planets in tutorial
        if (!gWorldConfig->isTutorial())
            asyncContainer->mOfCallback->handleObjectReady(waypoint,asyncContainer->mClient);
    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void WaypointFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,WaypointFQuery_MainData,client),
                               "SELECT waypoints.waypoint_id,waypoints.owner_id,waypoints.x,waypoints.y,waypoints.z,"
                               "waypoints.name,planet.name,waypoints.active,waypoints.type"
                               " FROM %s.waypoints INNER JOIN %s.planet ON (waypoints.planet_id = planet.planet_id)"
                               " WHERE (waypoints.waypoint_id = %" PRIu64 ")",
                               mDatabase->galaxy(),mDatabase->galaxy(),id);
  
}

//=============================================================================

WaypointObject* WaypointFactory::_createWaypoint(DatabaseResult* result)
{
    WaypointObject*	waypoint = new WaypointObject();

    result->getNextRow(mWaypointBinding,(void*)waypoint);

    waypoint->mName.convert(BSTRType_Unicode16);
    waypoint->setPlanetCRC(waypoint->getModelString().getCrc());

    return waypoint;
}

//=============================================================================

void WaypointFactory::_setupDatabindings()
{
    mWaypointBinding = mDatabase->createDataBinding(9);
    mWaypointBinding->addField(DFT_uint64,offsetof(WaypointObject,mId),8,0);
    mWaypointBinding->addField(DFT_uint64,offsetof(WaypointObject,mParentId),8,1);
    mWaypointBinding->addField(DFT_float,offsetof(WaypointObject,mCoords.x),4,2);
    mWaypointBinding->addField(DFT_float,offsetof(WaypointObject,mCoords.y),4,3);
    mWaypointBinding->addField(DFT_float,offsetof(WaypointObject,mCoords.z),4,4);
    mWaypointBinding->addField(DFT_bstring,offsetof(WaypointObject,mName),255,5);
    mWaypointBinding->addField(DFT_bstring,offsetof(WaypointObject,mModel),255,6);
    mWaypointBinding->addField(DFT_uint8,offsetof(WaypointObject,mActive),1,7);
    mWaypointBinding->addField(DFT_uint8,offsetof(WaypointObject,mWPType),1,8);
}

//=============================================================================

void WaypointFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mWaypointBinding);
}

//=============================================================================

