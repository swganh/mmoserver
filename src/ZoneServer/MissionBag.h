/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_MISSIONBAG_H
#define ANH_ZONESERVER_TANGIBLE_MISSIONBAG_H

#include "TangibleObject.h"
#include "MissionObject.h"

typedef std::vector<MissionObject*>		MissionList;

//=============================================================================

class MissionBag : public TangibleObject
{
	friend class ObjectFactory;
	friend class PlayerObjectFactory;

	public:

		MissionBag();
		MissionBag(uint64 id,PlayerObject* parent,const string model,const string name = "",const string file = "");
		~MissionBag();

		void		  setParent(PlayerObject* player){ mParent = player; }
		PlayerObject* getParent() { return mParent; }

		MissionList*   getMissions(){ return &mMissions; }
		MissionObject* getMissionById(uint64 id);
		void		   addMission(MissionObject* mission) { mMissions.push_back(mission); mCapacity--; }
		void		   spawnNAdd();
		bool		   removeMission(uint64 id);
		bool		   removeMission(MissionObject* mission);

	private:

		PlayerObject*	mParent;
		MissionList		mMissions;
		uint8			mCapacity;

};

//=============================================================================

#endif

