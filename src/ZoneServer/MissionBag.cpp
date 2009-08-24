/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib/MessageLib.h"
#include "MathLib/Quaternion.h"
#include "MissionBag.h"

#include "PlayerObject.h"


//=============================================================================

MissionBag::MissionBag() : TangibleObject()
{
	mTanGroup = TanGroup_PlayerInternal;
	mTanType = TanType_MissionBag;
}

//=============================================================================

MissionBag::MissionBag(uint64 id,PlayerObject* parent,string model,string name,string file) : TangibleObject(id,parent->getId(),model,TanGroup_PlayerInternal,TanType_MissionBag,name,file),
mCapacity(14)
{
	mTanGroup = TanGroup_PlayerInternal;
	mTanType = TanType_MissionBag;
	mParent = parent; 

	//Load the mission bag with blank missions
	for(int i = 0; i < 10; i++)
	{
		MissionObject* mission = new MissionObject(parent, id);
		mMissions.push_back(mission);
	}

}

//=============================================================================

MissionBag::~MissionBag()
{
	MissionList::iterator it = mMissions.begin();	
	while(it != mMissions.end())
	{
		delete(*it);
		it = mMissions.erase(it);
	}
}

//=============================================================================

/*
 * This function creates a new mission, adds it to this bag
 * and then spawns it client side.
*/
void MissionBag::spawnNAdd()
{
	if(mCapacity <= 0)
	{
		gLogger->logMsgF("ERROR: Mission Bag full.\n", MSG_HIGH);
	}

	//create a new mission
	MissionObject* mission = new MissionObject(mParent,mId);

	//add it to this bag
	mMissions.push_back(mission);
	mCapacity--;

	//Spawn the mission client side
	gMessageLib->sendCreateObjectByCRC(mission, mParent, false);
	gMessageLib->sendContainmentMessage(mission->getId(), mId, 0xffffffff, mParent);
	gMessageLib->sendBaselinesMISO_3(mission, mParent);
	gMessageLib->sendBaselinesMISO_6(mission, mParent);
	gMessageLib->sendBaselinesMISO_8(mission, mParent);
	gMessageLib->sendBaselinesMISO_9(mission, mParent);
	gMessageLib->sendEndBaselines(mission->getId(), mParent);

return;
}

//=============================================================================

MissionObject* MissionBag::getMissionById(uint64 id)
{
	MissionList::iterator it = mMissions.begin();

	while(it != mMissions.end())
	{
		if((*it)->getId() == id) return(*it); ++it;
	}

return NULL;
}

//=============================================================================

bool MissionBag::removeMission(MissionObject* mission)
{
	MissionList::iterator it = mMissions.begin();

	while(it != mMissions.end())
	{
		if((*it) == mission)
		{
			mMissions.erase(it);
			mCapacity++;
			return true;
		}

		++it;
	}

return false;
}

//=============================================================================

bool MissionBag::removeMission(uint64 id)
{
	MissionList::iterator it = mMissions.begin();

	while(it != mMissions.end())
	{
		if((*it)->getId() == id)
		{
			mMissions.erase(it);
			mCapacity++;
			return true;
		}

		++it;
	}

return false;
}

//=============================================================================


