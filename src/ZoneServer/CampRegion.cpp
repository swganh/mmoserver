				  /*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/
#include <list>
#include "CampRegion.h"
#include "Camp.h"
#include "PlayerObject.h"
#include "QTRegion.h"
#include "QuadTree.h"
#include "WorldManager.h"
#include "ZoneTree.h"
#include "MessageLib/MessageLib.h"

//=============================================================================
struct CampRegion::campLink
{
	uint64 objectID;
	uint32 tickCount;
	uint64 lastSeenTime;
};

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

	
	mSetUpTime = gWorldManager->GetCurrentGlobalTick();
}

//=============================================================================

CampRegion::~CampRegion()
{
}

//=============================================================================

void CampRegion::update()
{
	//Camps have a max timer of 55 minutes
	if(gWorldManager->GetCurrentGlobalTick() - mSetUpTime > 3300000)
	{
		despawnCamp();
		return;
	}

	if(mAbandoned)
	{
		if((gWorldManager->GetCurrentGlobalTick() >= mExpiresTime) && (!mDestroyed))
		{
			despawnCamp();
		}
	}

	PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mOwnerId));

	if(!owner)
	{
		despawnCamp();
		return;
	}

	if(owner->checkState(CreatureState_Combat))
	{
		//abandon
		mAbandoned	= true;
		mExpiresTime	= gWorldManager->GetCurrentGlobalTick(); //There is no grace period for combat.
		return;
	}

	if(!mSubZoneId)
	{
		mQTRegion	= mSI->getQTRegion(mPosition.x,mPosition.z);
		mSubZoneId	= (uint32)mQTRegion->getId();
		mQueryRect	= Anh_Math::Rectangle(mPosition.x - mWidth,mPosition.z - mHeight,mWidth*2,mHeight*2);
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
		{
			applyHAMHealing(object);
			mXp++;
		}

		if(!(checkKnownObjects(object)))
		{
			onObjectEnter(object);

			std::list<campLink*>::iterator i;
			bool alreadyExists = false;

			for(i = links.begin(); i != links.end(); i++)
			{
				if((*i)->objectID == object->getId())
				{
					alreadyExists = true;
				}
			}

			if(!alreadyExists)
			{
				campLink* temp = new campLink;
				temp->objectID = object->getId();
				temp->lastSeenTime = gWorldManager->GetCurrentGlobalTick();
				temp->tickCount = 0;

				links.push_back(temp);
			}
		}
		else
		{
			//Find the right link
			std::list<campLink*>::iterator i;

			for(i = links.begin(); i != links.end(); i++)
			{
				if((*i)->objectID == object->getId())
				{

					(*i)->lastSeenTime = gWorldManager->GetCurrentGlobalTick();

					if((*i)->tickCount == 15)
					{
						applyWoundHealing(object);
						(*i)->tickCount = 0;
					}
					else
						(*i)->tickCount++;

					break;
				}
			}


			/*
			//This code causes the Zone Server to print relational position and rotation info
			//to allow the adding of items without much effort.
			int8 text[256];
			sprintf(text,"Position: mX=%f mY=%f mZ=%f\nDirection: mX=%f mY=%f mZ=%f mW=%f", (object->mPosition.x - this->mPosition.x), (object->mPosition.y - this->mPosition.y), (object->mPosition.z - this->mPosition.z), object->mDirection.x,object->mDirection.y,object->mDirection.z,object->mDirection.w);
			*/
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

	//prune the list
	std::list<campLink*>::iterator i = links.begin();

	while(i != links.end())
	{
		if(gWorldManager->GetCurrentGlobalTick() - (*i)->lastSeenTime >= 30000)
		{
			delete (*i);
			i = links.erase(i);
		}
		else
		{
			i++;
		}
	}
}

//=============================================================================

void CampRegion::onObjectEnter(Object* object)
{

	if(object->getParentId() == mParentId)
	{
		//PlayerObject* player = (PlayerObject*)object;
		this->addKnownObjectSafe(object);
		object->addKnownObjectSafe(this);

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
      gMessageLib->sendSystemMessage(player, uT.getUnicode16());
		}
		else
		{
			//ensure it's not time to destroy the camp
			mAbandoned = false;
		}

	}

}

//=============================================================================

void CampRegion::onObjectLeave(Object* object)
{
	PlayerObject* player = (PlayerObject*)object;
	this->removeKnownObject(object);
	object->removeKnownObject(this);

	if(object->getId() == mOwnerId)
	{
		mAbandoned	= true;

		//We want to have this camp die after the owner has been gone longer 
		//than he stayed in the camp, with a max of two minutes.
		uint64 mTempCurrentTime = gWorldManager->GetCurrentGlobalTick();

		if((mTempCurrentTime - mSetUpTime) > 120000)
			mExpiresTime = mTempCurrentTime + 120000;
		else
			mExpiresTime = mTempCurrentTime + (mTempCurrentTime - mSetUpTime);
	}
	else
	{
		int8 text[64];
		sprintf(text,"You have left %s's camp", this->getCampOwnerName().getAnsi());
		string uT = text;
		uT.convert(BSTRType_Unicode16);
    gMessageLib->sendSystemMessage(player, uT.getUnicode16());
	}
		//check whether we are the owner and if yes set our abandoning timer
}

//=============================================================================


void	CampRegion::despawnCamp()
{
	mDestroyed	= true;
	mActive		= false;

	PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mOwnerId));

	if(owner)
		owner->setHasCamp(false);

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
	applyXp();
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

void	CampRegion::applyWoundHealing(Object* object)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(object);

	//Make sure it's a player.
	if(player == NULL)
		return;

	Ham* hamz = player->getHam();

	if(hamz->mHealth.getWounds() > 0)
	{
		hamz->updatePropertyValue(HamBar_Health ,HamProperty_Wounds, -1);
		mHealingDone++;
	}

	if(hamz->mStrength.getWounds() > 0)
	{
		hamz->updatePropertyValue(HamBar_Strength ,HamProperty_Wounds, -1);
		mHealingDone++;
	}

	if(hamz->mConstitution.getWounds() > 0)
	{
		hamz->updatePropertyValue(HamBar_Constitution ,HamProperty_Wounds, -1);
		mHealingDone++;
	}

	if(hamz->mAction.getWounds() > 0)
	{
		hamz->updatePropertyValue(HamBar_Action ,HamProperty_Wounds, -1);
		mHealingDone++;
	}

	if(hamz->mQuickness.getWounds() > 0)
	{
		hamz->updatePropertyValue(HamBar_Quickness ,HamProperty_Wounds, -1);
		mHealingDone++;
	}

	if(hamz->mStamina.getWounds() > 0)
	{
		hamz->updatePropertyValue(HamBar_Stamina ,HamProperty_Wounds, -1);
		mHealingDone++;
	}

	if(hamz->mMind.getWounds() > 0)
	{
		hamz->updatePropertyValue(HamBar_Mind ,HamProperty_Wounds, -1);
		mHealingDone++;
	}

	if(hamz->mFocus.getWounds() > 0)
	{
		hamz->updatePropertyValue(HamBar_Focus ,HamProperty_Wounds, -1);
		mHealingDone++;
	}

	if(hamz->mWillpower.getWounds() > 0)
	{
		hamz->updatePropertyValue(HamBar_Willpower ,HamProperty_Wounds, -1);
		mHealingDone++;
	}

}

void	CampRegion::applyHAMHealing(Object* object)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(object);

	//Make sure it's a player.
	if(player == NULL)
		return;

	Ham* hamz = player->getHam();

	//Heal the Ham
	int32 HealthRegenRate = hamz->getHealthRegenRate();
	int32 ActionRegenRate = hamz->getActionRegenRate();
	int32 MindRegenRate = hamz->getMindRegenRate();

	//Because we tick every 2 seconds, we need to double this.
	HealthRegenRate += (int32)(HealthRegenRate * mHealingModifier) * 2;
	ActionRegenRate += (int32)(ActionRegenRate * mHealingModifier) * 2;
	MindRegenRate	+= (int32)(MindRegenRate * mHealingModifier) * 2;

	if(hamz->mHealth.getModifiedHitPoints() - hamz->mHealth.getCurrentHitPoints() > 0)
	{
		//Regen Health
		int32 oldVal = hamz->mHealth.getCurrentHitPoints();
		hamz->updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints, HealthRegenRate);
		mHealingDone += hamz->mHealth.getCurrentHitPoints() - oldVal;
	}
	
	if(hamz->mAction.getModifiedHitPoints() - hamz->mAction.getCurrentHitPoints() > 0)
	{
		//Regen Action
		int32 oldVal = hamz->mAction.getCurrentHitPoints();
		hamz->updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints, ActionRegenRate);
		mHealingDone += hamz->mAction.getCurrentHitPoints() - oldVal;
	}

	if(hamz->mMind.getModifiedHitPoints() - hamz->mMind.getCurrentHitPoints() > 0)
	{
		//Regen Mind
		int32 oldVal = hamz->mMind.getCurrentHitPoints();
		hamz->updatePropertyValue(HamBar_Mind, HamProperty_CurrentHitpoints, MindRegenRate);
		mHealingDone += hamz->mMind.getCurrentHitPoints() - oldVal;
	}

}

void	CampRegion::applyXp()
{
	//mXP = The amount of XP accumulated via vistors in the camp
	mXp += mHealingDone; //The Amount of Healing Done
}
