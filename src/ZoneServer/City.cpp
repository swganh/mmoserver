/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "City.h"
#include "PlayerObject.h"
#include "QTRegion.h"
#include "QuadTree.h"
#include "WorldManager.h"
#include "ZoneTree.h"

//=============================================================================

City::City() : RegionObject(),
mSI(gWorldManager->getSI()),
mQTRegion(NULL)
{
	mActive		= false;
	mRegionType = Region_City;
}

//=============================================================================

City::~City()
{
}

//=============================================================================

void City::update()
{
	if(!mSubZoneId)
	{
		mQTRegion	= mSI->getQTRegion(mPosition.x,mPosition.z);
		mSubZoneId	= (uint32)mQTRegion->getId();
		mQueryRect	= Anh_Math::Rectangle(mPosition.x - mWidth,mPosition.z - mHeight,mWidth * 2,mHeight * 2);
	}

	Object*		object;
	ObjectSet	objList;
	
	mSI->getObjectsInRange(this,&objList,ObjType_Player,mWidth);

	if(mQTRegion)
	{
		mQTRegion->mTree->getObjectsInRange(this,&objList,ObjType_Player,&mQueryRect);
	}

	ObjectSet::iterator objIt = objList.begin();

	while(objIt != objList.end())
	{
		object = (*objIt);

		if(!(checkKnownObjects(object)))
		{
			onObjectEnter(object);
		}

		++objIt;
	}

	ObjectSet oldKnownObjects = mKnownObjects;
	ObjectSet::iterator objSetIt = oldKnownObjects.begin();

	while(objSetIt != oldKnownObjects.end())
	{
		object = (*objSetIt);

		if(objList.find(object) == objList.end())
		{
			onObjectLeave(object);
		}

		++objSetIt;
	}
}

//=============================================================================

void City::onObjectEnter(Object* object)
{
	PlayerObject* player = (PlayerObject*)object;
	//player->setCityRegionId(this->getId());

	addKnownObjectSafe(object);

	gLogger->logMsgF("%s entered %s (%u players in city)",MSG_HIGH,player->getFirstName().getAnsi(),mCityName.getAnsi(),mKnownPlayers.size());
}

//=============================================================================

void City::onObjectLeave(Object* object)
{

	PlayerObject* player = (PlayerObject*)object;

	//if(player->getCityRegionId() == this->getId())
		//player->setCityRegionId(0);

	removeKnownObject(object);

	gLogger->logMsgF("%s left %s (%u players in city)",MSG_HIGH,player->getFirstName().getAnsi(),mCityName.getAnsi(),mKnownPlayers.size());
}

//=============================================================================

