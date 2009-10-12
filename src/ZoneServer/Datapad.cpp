/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Datapad.h"
#include "IntangibleObject.h"
#include "MissionObject.h"
#include "MathLib/Quaternion.h"
#include "ManufacturingSchematic.h"
#include "MessageLib/MessageLib.h"
#include "WaypointObject.h"
#include "WorldManager.h"
#include "WorldConfig.h"
#include "PlayerObject.h"
#include "ObjectFactory.h"

//=============================================================================

Datapad::Datapad() : TangibleObject()
{
	mTanGroup					= TanGroup_Datapad;
	mTanType					= TanType_CharacterDatapad;
	mSchematicUpdateCounter		= 0;
	mWayPointCapacity			= gWorldConfig->getConfiguration("Player_DataPad_WayPointCapacity",(uint8)100);
	mCapacity					= gWorldConfig->getConfiguration("Player_Datapad_Capacity",(uint8)100);
	mMissionCapacity			= gWorldConfig->getConfiguration("Player_Datapad_MissionCapacity",(uint8)2);

	if(mMissionCapacity	>6)
	{
		gLogger->logErrorF("Configuration","Datapad::Datapad() : mMissionCapacity	was bigger than 6 - bigger entries are not supported due to the way Mission IDs are handled",MSG_NORMAL);
		//gLogger->logMsgF("Datapad::Datapad() : mMissionCapacity	was bigger than 6 - bigger entries are not supported due to the way Mission IDs are handled",MSG_NORMAL);
		mMissionCapacity = 6;
	}
	
	/*
		//Absolute max of 6! To raise this change the 
		//mission ID system from uint16 to uint32/uint64
	*/

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

	mData.clear();
	DataList::iterator iter = mData.begin(); 	
	while(iter != mData.end())
	{
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
			mWayPointCapacity++;
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
			mWayPointCapacity++;
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

bool Datapad::addWaypoint(WaypointObject* waypoint)
{ 
	if(mWayPointCapacity)
	{
		mWaypoints.push_back(waypoint); 
		mWayPointCapacity--; 
		return true;
	}
	else
	{
		PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
		gMessageLib->sendSystemMessage(player,L"","base_player","too_many_waypoints");
		return false;
	}
}

bool Datapad::addData(IntangibleObject* Data) 
{ 
	if(mCapacity)
	{
		mData.push_back(Data); 
		//PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
		mCapacity--; 

		// the datapads (or inventories) client volume counters are clientside updated
		// items being contained by the inventory (or datapad) do count as made out
		// by the objects volume count (in the tano 3)
		// please make sure that the containment isnt send by the factory before the item is even created

		return true;
	}
	else
	{
		PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
		gMessageLib->sendSystemMessage(player,L"","base_player","too_many_waypoints");
		return false;

	}
}

//=============================================================================

void Datapad::handleObjectReady(Object* object,DispatchClient* client)
{
	if(WaypointObject* waypoint = dynamic_cast<WaypointObject*>(object))
	{
		if(addWaypoint(waypoint))
			gMessageLib->sendWaypointsUpdate(mOwner);
		else
		{
			//remove it from db ...
		}
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


void Datapad::requestNewWaypoint(string name,const Anh_Math::Vector3 coords,uint16 planetId,uint8 wpType)
{
	
	if(!mCapacity)
	{
		PlayerObject*	player			= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
		
		if(player)
			gMessageLib->sendSystemMessage(player,L"","base_player","too_many_waypoints");
		
		return;
	}

	gObjectFactory->requestNewWaypoint(this,name,coords,planetId, mOwner->getId(),wpType);
}

//=============================================================================

bool Datapad::addManufacturingSchematic(ManufacturingSchematic* ms)
{ 
	if(mCapacity)
	{
		mManufacturingSchematics.push_back(ms); 
		mCapacity--; 
		return true;
	}
	else
	{
		PlayerObject*	player			= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
		if(player)
			gMessageLib->sendSystemMessage(player,L"","ui_craft","err_datapad_full_prose");
		return false;
	}
}

//=============================================================================

bool Datapad::addMission(MissionObject* mission) 
{ 
	PlayerObject*	player			= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getParentId()));					
	if(!mCapacity)
	{
		gMessageLib->sendSystemMessage(player,L"Error datapad at max capacity. Couldn't create the mission.");
		return false;
	}
	if(!mMissionCapacity)
	{
		gMessageLib->sendSystemMessage(player,L"Error datapad mission system at max capacity. Couldn't create the mission.");
		return false;
	}
	mMissions.push_back(mission); 
	mCapacity--; 
	mMissionCapacity--; 
	return true;
}