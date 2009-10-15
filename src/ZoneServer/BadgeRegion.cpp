/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "BadgeRegion.h"
#include "PlayerObject.h"
#include "QTRegion.h"
#include "QuadTree.h"
#include "WorldManager.h"
#include "ZoneTree.h"

//=============================================================================

BadgeRegion::BadgeRegion() : RegionObject(),
mSI(gWorldManager->getSI()),
mQTRegion(NULL)
{
	mActive		= true;
	mRegionType = Region_Badge;
}

//=============================================================================

BadgeRegion::~BadgeRegion()
{
}

//=============================================================================

void BadgeRegion::update()
{
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
		object = (*objIt);

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

void BadgeRegion::onObjectEnter(Object* object)
{
	if(object->getParentId() == mParentId)
	{
		PlayerObject* player = (PlayerObject*)object;
		addKnownObject(object);

		if(!(player->checkBadges(mBadgeId)))
			player->addBadge(mBadgeId);
	}
}

//=============================================================================

void BadgeRegion::onObjectLeave(Object* object)
{
	PlayerObject* player = (PlayerObject*)object;
	removeKnownObject(object);
}

//=============================================================================

