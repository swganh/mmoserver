/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "SpawnRegion.h"
#include "QTRegion.h"
#include "WorldManager.h"
#include "PlayerObject.h"
#include "ZoneTree.h"

//=============================================================================

SpawnRegion::SpawnRegion() : RegionObject(),
mSI(gWorldManager->getSI()),
mMission(0),
mQTRegion(NULL)
{
	mActive		= true;
	mRegionType = Region_Spawn;
}

//=============================================================================

SpawnRegion::~SpawnRegion()
{
}

//=============================================================================

void SpawnRegion::update()
{
	//run about every 4.5 seconds
	if(!mSubZoneId)
	{
		mQTRegion	= mSI->getQTRegion(mPosition.mX,mPosition.mZ);
		mSubZoneId	= (uint32)mQTRegion->getId();
		mQueryRect	= Anh_Math::Rectangle(mPosition.mX - mWidth,mPosition.mZ - mHeight,mWidth * 2,mHeight * 2);
	}

	Object*		object;
	ObjectSet	objList;
	
	if(mParentId)
	{
		mSI->getObjectsInRange(this,&objList,ObjType_Player,mWidth);
	}

	if(mQTRegion)
	{
		mQTRegion->mTree->getObjectsInRange(this,&objList,ObjType_Player,&mQueryRect);
	}

	ObjectSet::iterator objIt = objList.begin();

	while(objIt != objList.end())
	{
		object = dynamic_cast<Object*>(*objIt);

		if(!(checkKnownObjects(object)))
		{
			onObjectEnter(object);
		}

		++objIt;
	}

	PlayerObjectSet oldKnownObjects = mKnownPlayers;
	PlayerObjectSet::iterator objSetIt = oldKnownObjects.begin();

	while(objSetIt != oldKnownObjects.end())
	{
		object = dynamic_cast<Object*>(*objSetIt);

		if(objList.find(object) == objList.end())
		{
			onObjectLeave(object);
		}

		++objSetIt;
	}
}

//=============================================================================

void SpawnRegion::onObjectEnter(Object* object)
{
	if(object->getParentId() == mParentId)
	{
		PlayerObject* player = (PlayerObject*)object;
		addKnownObject(object);
	}
}

//=============================================================================

void SpawnRegion::onObjectLeave(Object* object)
{
	PlayerObject* player = (PlayerObject*)object;
	removeKnownObject(object);
}

//=============================================================================

