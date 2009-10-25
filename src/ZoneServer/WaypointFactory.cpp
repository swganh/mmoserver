/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "WaypointFactory.h"
#include "ObjectFactoryCallback.h"
#include "WaypointObject.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"

#include <assert.h>

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
			asyncContainer->mOfCallback->handleObjectReady(waypoint,asyncContainer->mClient);
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void WaypointFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,WaypointFQuery_MainData,client),
								"SELECT waypoints.waypoint_id,waypoints.owner_id,waypoints.x,waypoints.y,waypoints.z,"
								"waypoints.name,planet.name,waypoints.active,waypoints.type"
								" FROM waypoints INNER JOIN planet ON (waypoints.planet_id = planet.planet_id)"
								" WHERE (waypoints.waypoint_id = %"PRId64")",id);
}

//=============================================================================

WaypointObject* WaypointFactory::_createWaypoint(DatabaseResult* result)
{
	WaypointObject*	waypoint = new WaypointObject();

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mWaypointBinding,(void*)waypoint);

	waypoint->mName.convert(BSTRType_Unicode16);
	waypoint->setPlanetCRC(waypoint->getModelString().getCrc());

	return waypoint;
}

//=============================================================================

void WaypointFactory::_setupDatabindings()
{
	mWaypointBinding = mDatabase->CreateDataBinding(9);
	mWaypointBinding->addField(DFT_uint64,offsetof(WaypointObject,mId),8,0);
	mWaypointBinding->addField(DFT_uint64,offsetof(WaypointObject,mParentId),8,1);
	mWaypointBinding->addField(DFT_float,offsetof(WaypointObject,mCoords.mX),4,2);
	mWaypointBinding->addField(DFT_float,offsetof(WaypointObject,mCoords.mY),4,3);
	mWaypointBinding->addField(DFT_float,offsetof(WaypointObject,mCoords.mZ),4,4);
	mWaypointBinding->addField(DFT_bstring,offsetof(WaypointObject,mName),255,5);
	mWaypointBinding->addField(DFT_bstring,offsetof(WaypointObject,mModel),255,6);
	mWaypointBinding->addField(DFT_uint8,offsetof(WaypointObject,mActive),1,7);
	mWaypointBinding->addField(DFT_uint8,offsetof(WaypointObject,mWPType),1,8);
}

//=============================================================================

void WaypointFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mWaypointBinding);
}

//=============================================================================

