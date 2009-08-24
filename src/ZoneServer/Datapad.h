/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_DATAPAD_H
#define ANH_ZONESERVER_TANGIBLE_DATAPAD_H

#include "TangibleObject.h"
#include "WaypointObject.h"
#include "MissionObject.h"
#include "IntangibleObject.h"
#include "ObjectFactoryCallback.h"
#include "ManufacturingSchematic.h"


//=============================================================================

class PlayerObject;

typedef std::vector<WaypointObject*>			WaypointList;
typedef std::vector<ManufacturingSchematic*>	ManufacturingSchematicList;
typedef std::vector<MissionObject*>				MissionList;
typedef std::vector<IntangibleObject*>			DataList;

//=============================================================================

class Datapad : public TangibleObject, public ObjectFactoryCallback
{
	friend class DatapadFactory;

	public:

		Datapad();
		~Datapad();

		// inherited callback
		virtual void	handleObjectReady(Object* object,DispatchClient* client);

		// player reference
		void			setOwner(PlayerObject* player){ mOwner = player; }

		// waypoints
		WaypointList*	getWaypoints(){ return &mWaypoints; }
		WaypointObject*	getWaypointById(uint64 id);
		WaypointObject*	getWaypointByName(string name);
		bool			addWaypoint(WaypointObject* waypoint);
		bool			removeWaypoint(uint64 id);
		bool			removeWaypoint(WaypointObject* waypoint);
		void			setObjectLoadCounter(uint32 count){ mObjectLoadCounter = count; }
		void			requestNewWaypoint(string name,const Anh_Math::Vector3 coords,uint16 planetId,uint8 wpType);

		//missions
		MissionList*   getMissions() { return &mMissions; }
		MissionObject* getMissionById(uint64 id);
		bool		   addMission(MissionObject* mission);
		bool		   removeMission(uint64 id);
		bool		   removeMission(MissionObject* mission);
		bool		   hasMission(){return( mMissions.size()>0);}
		
		MissionList::iterator removeMission(MissionList::iterator it);

		//data -- aka mounts,pets,vehicles, etc
		DataList*		   getData() { return &mData; }
		IntangibleObject*  getDataById(uint64 id);
		bool		       addData(IntangibleObject* Data);
		bool		       removeData(uint64 id);
		bool		       removeData(IntangibleObject* Data);
		DataList::iterator removeData(DataList::iterator it);

		// ManufacturingSchematics
		ManufacturingSchematicList*		getManufacturingSchematics(){ return &mManufacturingSchematics; }
		ManufacturingSchematic*			getManufacturingSchematicById(uint64 id);

		
		bool							addManufacturingSchematic(ManufacturingSchematic* ms);
		bool							removeManufacturingSchematic(uint64 id);
		bool							removeManufacturingSchematic(ManufacturingSchematic* ms);


		// capacity
		uint8			getCapacity(){ return mCapacity; }
		uint8			getMissionCapacity() { return mMissionCapacity; }
		
		uint32			mWaypointUpdateCounter;
		uint32			mManSUpdateCounter;
		uint32			mSchematicUpdateCounter;
		uint32			mMissionUpdateCounter;

	private:

		uint8						mCapacity;
		uint8						mWayPointCapacity;
		uint8						mMissionCapacity;
		WaypointList				mWaypoints;
		ManufacturingSchematicList	mManufacturingSchematics;
		MissionList					mMissions;
		DataList					mData;
		PlayerObject*				mOwner;
		uint32						mObjectLoadCounter;
};

//=============================================================================

#endif

