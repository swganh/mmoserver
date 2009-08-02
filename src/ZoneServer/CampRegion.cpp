				  /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "CampRegion.h"
#include "Camp.h"
#include "QTRegion.h"
#include "WorldManager.h"
#include "PlayerObject.h"

//=============================================================================

CampRegion::CampRegion() : RegionObject(),
mSI(gWorldManager->getSI()),
mQTRegion(NULL)
{
	mActive			= true;
	mDestroyed		= false;

	mRegionType		= Region_Camp;

	mAbandoned		= false;
	mXp				= 0;

	mLeftTime = mSetUpTime = gWorldManager->GetCurrentGlobalTick();
}

//=============================================================================

CampRegion::~CampRegion()
{
}

//=============================================================================

void CampRegion::update()
{
	if(mAbandoned)
	{
		if((gWorldManager->GetCurrentGlobalTick() - mLeftTime > 5000) && (!mDestroyed))
		{
			despawnCamp();
		}

		return;
	}
	
	PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mOwnerId));
	
	if(owner->checkState(CreatureState_Combat))
	{
		//abandon
		mAbandoned	= true;
		mLeftTime	= gWorldManager->GetCurrentGlobalTick();
	}

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
		mQTRegion->mTree->getObjectsInRangeContains(this,&objList,ObjType_Player,&mQueryRect);
	}

	ObjectSet::iterator objIt = objList.begin();

	while(objIt != objList.end())
	{
		object = (*objIt);

		//one xp per player in camp every 2 seconds
		if(!mAbandoned)
			mXp ++;

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

void CampRegion::onObjectEnter(Object* object)
{

	if(object->getParentId() == mParentId)
	{
		//PlayerObject* player = (PlayerObject*)object;
		addKnownObject(object);
		
		VisitorSet::iterator it = mVisitorSet.find(object->getId());
		
		if(it == mVisitorSet.end())
			mVisitorSet.insert(object->getId());
		
		PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mOwnerId));
		
		if(owner && (owner->getId() != object->getId()))
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(object);
			int8 text[64];
			sprintf(text,"You have entered %s's camp",this->getCampOwnerName().getAnsi());
			string uT = text;
			uT.convert(BSTRType_Unicode16);
			gMessageLib->sendSystemMessage(player, uT);
		}

	}
	
}

//=============================================================================

void CampRegion::onObjectLeave(Object* object)
{
	PlayerObject* player = (PlayerObject*)object;
	removeKnownObject(object);

	if(object->getId() == mOwnerId)
	{
		mAbandoned	= true;
		mLeftTime	= gWorldManager->GetCurrentGlobalTick();
	}
	else
	{
		int8 text[64];
		sprintf(text,"You have left %s's camp",this->getCampOwnerName().getAnsi());
		string uT = text;
		uT.convert(BSTRType_Unicode16);
		gMessageLib->sendSystemMessage(player, uT);
	}
		//check whether we are the owner and if yes set our abandoning timer	
}

//=============================================================================


void	CampRegion::despawnCamp()
{
	mDestroyed	= true;
	mActive		= false;

	gLogger->logMsg("destroy the camp");

	PlayerObject* owner = NULL;//dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mOwnerId));
	//we need to destroy our camp!!
	Camp* camp = dynamic_cast<Camp*>(gWorldManager->getObjectById(mCampId));
	ItemList* iL = camp->getItemList();
	
	ItemList::iterator iLiT = iL->begin();
	while(iLiT != iL->end())
	{
		TangibleObject* tangible = (*iLiT);
		gMessageLib->sendDestroyObject_InRangeofObject(tangible);
		gWorldManager->destroyObject(tangible);
		iLiT++;
	}

	gMessageLib->sendDestroyObject_InRangeofObject(camp);
	gWorldManager->destroyObject(camp);
	
	gWorldManager->addRemoveRegion(this);

	//now grant xp
	if(mXp)
	{
		if(mXp > mXpMax)
			mXp = mXpMax;

		PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mOwnerId));
		if(player)
			gSkillManager->addExperience(XpType_camp,mXp,player);
		//still get db side in
	}


}

void	CampRegion::applyXp()
{

}