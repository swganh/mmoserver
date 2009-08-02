/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "Datapad.h"
#include "MessageLib/MessageLib.h"
#include "WorldManager.h"


//=============================================================================

Datapad::Datapad() : TangibleObject(),mCapacity(100),mMissionCapacity(DATAPAD_MAX_MISSIONS)
{
	mTanGroup					= TanGroup_Datapad;
	mTanType					= TanType_CharacterDatapad;
	mSchematicUpdateCounter		= 0;
}

//=============================================================================

Datapad::~Datapad()
{
	//--------------------------------------------
	WaypointList::iterator it = mWaypoints.begin();
	while(it != mWaypoints.end())
	{
		it = mWaypoints.erase(it);
	}
	//--------------------------------------------
	MissionList::iterator ite = mMissions.begin(); 	
	while(ite != mMissions.end())
	{
		ite = mMissions.erase(ite);	
	}
	//--------------------------------------------
	DataList::iterator iter = mData.begin(); 	
	while(iter != mData.end())
	{
		gWorldManager->destroyObject(*iter);
		iter = mData.erase(iter);
	}
	//--------------------------------------------

}

//=============================================================================

WaypointObject* Datapad::getWaypointById(uint64 id)
{
	WaypointList::iterator it = mWaypoints.begin();

	while(it != mWaypoints.end())
	{
		if((*it)->getId() == id)
			return(*it);
		++it;
	}

	return(NULL);
}

//=============================================================================

bool Datapad::removeWaypoint(WaypointObject* waypoint)
{
	WaypointList::iterator it = mWaypoints.begin();

	while(it != mWaypoints.end())
	{
		if((*it) == waypoint)
		{
			mWaypoints.erase(it);
			mCapacity++;
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

bool Datapad::removeWaypoint(uint64 id)
{
	WaypointList::iterator it = mWaypoints.begin();

	while(it != mWaypoints.end())
	{
		if((*it)->getId() == id)
		{
			mWaypoints.erase(it);
			mCapacity++;
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

WaypointObject* Datapad::getWaypointByName(string name)
{
	WaypointList::iterator it = mWaypoints.begin();

	while(it != mWaypoints.end())
	{
		string wpName = (*it)->getName();
		wpName.convert(BSTRType_ANSI);

		if(strcmp(wpName.getAnsi(),name.getAnsi()) == 0)
			return(*it);

		++it;
	}

	return(NULL);
}

//=============================================================================

ManufacturingSchematic* Datapad::getManufacturingSchematicById(uint64 id)
{
	ManufacturingSchematicList::iterator it = mManufacturingSchematics.begin();

	while(it != mManufacturingSchematics.end())
	{
		if((*it)->getId() == id)
			return(*it);
		++it;
	}

	return(NULL);
}

//=============================================================================

bool Datapad::removeManufacturingSchematic(uint64 id)
{
	ManufacturingSchematicList::iterator it = mManufacturingSchematics.begin();

	while(it != mManufacturingSchematics.end())
	{
		if((*it)->getId() == id)
		{
			mManufacturingSchematics.erase(it);
			mCapacity++;
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

bool Datapad::removeManufacturingSchematic(ManufacturingSchematic* ms)
{
	ManufacturingSchematicList::iterator it = mManufacturingSchematics.begin();

	while(it != mManufacturingSchematics.end())
	{
		if((*it) == ms)
		{
			mManufacturingSchematics.erase(it);
			mCapacity++;
			return(true);
		}

		++it;
	}

	return(false);
}

//=============================================================================

void Datapad::handleObjectReady(Object* object,DispatchClient* client)
{
	if(WaypointObject* waypoint = dynamic_cast<WaypointObject*>(object))
	{
		addWaypoint(waypoint);

		gMessageLib->sendWaypointsUpdate(mOwner);
	}
}

//=============================================================================  

MissionObject* Datapad::getMissionById(uint64 id)
{
	MissionList::iterator it = mMissions.begin();

	while(it != mMissions.end())
	{
		if((*it)->getId() == id) return(*it); ++it;
	}

return NULL;
}

//=============================================================================

bool Datapad::removeMission(MissionObject* mission)
{
	MissionList::iterator it = mMissions.begin();
	while(it != mMissions.end())
	{
		if((*it) == mission)
		{
			removeMission(it);
			return true;
		}
		++it;
	}
return false;
}

//=============================================================================

bool Datapad::removeMission(uint64 id)
{
	MissionList::iterator it = mMissions.begin();
	while(it != mMissions.end())
	{
		if((*it)->getId() == id)
		{
			removeMission(it);
			return true;
		}
		++it;
	}
return false;
}

//=============================================================================

MissionList::iterator Datapad::removeMission(MissionList::iterator it)
{
	it = mMissions.erase(it);
	mCapacity++;
	mMissionCapacity++;
return it;
}

//=============================================================================

IntangibleObject* Datapad::getDataById(uint64 id)
{
	DataList::iterator it = mData.begin();

	while(it != mData.end())
	{
		if((*it)->getId() == id) return(*it); ++it;
	}

return NULL;
}

//=============================================================================

bool Datapad::removeData(IntangibleObject* Data)
{
	DataList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == Data)
		{
			removeData(it);
			return true;
		}
		++it;
	}
return false;
}

//=============================================================================

bool Datapad::removeData(uint64 id)
{
	DataList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it)->getId() == id)
		{
			removeData(it);
			return true;
		}
		++it;
	}
return false;
}

//=============================================================================

DataList::iterator Datapad::removeData(DataList::iterator it)
{
	it = mData.erase(it);
	mCapacity++;
return it;
}

//=============================================================================


