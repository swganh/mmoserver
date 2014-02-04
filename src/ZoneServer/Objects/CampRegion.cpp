/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "ZoneServer\Services\ham\ham_service.h"

#include <algorithm>
#include <list>

#include "MessageLib/MessageLib.h"

#include "ZoneServer/Objects/Camp.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/WorldManager.h"
#include "CampRegion.h"

#include "ZoneServer\Services\ham\ham_service.h"
#include "anh/app/swganh_kernel.h"
#include "anh\service\service_manager.h"


//=============================================================================
struct CampRegion::campLink
{
    uint64 objectID;
    uint32 tickCount;
    uint64 lastSeenTime;
};

//=============================================================================

CampRegion::CampRegion() : RegionObject()
{
    mActive			= true;
    mDestroyed		= false;

    mRegionType		= Region_Camp;

    mAbandoned		= false;
    mXp				= 0;


    mSetUpTime = gWorldManager->GetCurrentGlobalTick();
}


CampRegion::~CampRegion() {}


void CampRegion::update() {
    //Camps have a max timer of 55 minutes
    if (gWorldManager->GetCurrentGlobalTick() - mSetUpTime > 3300000) {
        despawnCamp();
        return;
    }

    if(mAbandoned && (gWorldManager->GetCurrentGlobalTick() >= mExpiresTime) && (!mDestroyed)) {
        despawnCamp();
        return;
    }

    PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mOwnerId));
    if (!owner)	{
        despawnCamp();
        return;
    }

    if(owner->states.checkState(CreatureState_Combat)) {
        //abandon
        mAbandoned = true;
        mExpiresTime = gWorldManager->GetCurrentGlobalTick(); //There is no grace period for combat.
        return;
    }

    std::for_each(mVisitingPlayers.begin(), mVisitingPlayers.end(), [=] (uint64_t player_id) {
        Object* visitor = gWorldManager->getObjectById(player_id);
        if (!visitor) {
            assert(false && "The camp is holding a reference to a player that doesn't exist");
            return;
        }

        if (!mAbandoned) {
            applyHAMHealing(visitor);
            mXp++;
        }

        auto it = std::find_if(links.begin(), links.end(), [=] (campLink* link) {
            return link->objectID == visitor->getId();
        });

        if (it != links.end()) {
            (*it)->lastSeenTime = gWorldManager->GetCurrentGlobalTick();

            if ((*it)->tickCount == 15) {
                applyWoundHealing(visitor);
                (*it)->tickCount = 0;
            } else {
                (*it)->tickCount++;
            }
        }
    });
}

//=============================================================================

void CampRegion::onObjectEnter(Object* object) {
    if (object->getType() != ObjType_Player) {
        return;
    }

    PlayerObject* visitor = dynamic_cast<PlayerObject*>(object);
    if (!visitor) {
        return;
    }

    if (!addVisitor(visitor)) {
        return;
    }

    auto it = std::find_if(links.begin(), links.end(), [=] (campLink* link) {
        return link->objectID == visitor->getId();
    });

    if (it != links.end()) {
        campLink* temp = new campLink;
        temp->objectID = visitor->getId();
        temp->lastSeenTime = gWorldManager->GetCurrentGlobalTick();
        temp->tickCount = 0;

        links.push_back(temp);
    }

    PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(mOwnerId));

    if (owner && (owner->getId() != visitor->getId())) {
        std::string text = "You have entered " + this->getCampOwnerName() + "'s camp.";
        gMessageLib->SendSystemMessage(std::wstring(text.begin(), text.end()).c_str(), visitor);
    } else {
        //ensure it's not time to destroy the camp
        mAbandoned = false;
    }
}

//=============================================================================

void CampRegion::onObjectLeave(Object* object) {
    if (object->getType() != ObjType_Player) {
        return;
    }

    if(object->getId() == mOwnerId)	{
        mAbandoned	= true;

        //We want to have this camp die after the owner has been gone longer
        //than he stayed in the camp, with a max of two minutes.
        uint64 mTempCurrentTime = gWorldManager->GetCurrentGlobalTick();

        if((mTempCurrentTime - mSetUpTime) > 120000) {
            mExpiresTime = mTempCurrentTime + 120000;
        } else {
            mExpiresTime = mTempCurrentTime + (mTempCurrentTime - mSetUpTime);
        }
    } else {
        PlayerObject* player = dynamic_cast<PlayerObject*>(object);
        std::string text = "You have left " + this->getCampOwnerName() + "'s camp.";
        gMessageLib->SendSystemMessage(std::wstring(text.begin(), text.end()).c_str(), player);
    }

    removeVisitor(object);
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
	gSpatialIndexManager->RemoveRegion(getSharedFromThis());

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

	gWorldManager->destroyObject(camp);
}

void	CampRegion::applyWoundHealing(Object* object)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(object);

    //Make sure it's a player.
    if(player == NULL)
        return;

	auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

	for(uint8 i = HamBar_Health; i < HamBar_Willpower;i++)	{
			ham->RemoveWound(player, i, 1);
	}
}

void	CampRegion::applyHAMHealing(Object* object)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(object);

    //Make sure it's a player.
    if(player == NULL)
        return;

    auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

    //Heal the Ham
	int32 HealthRegenRate = ham->regenerationModifier(player, HamBar_Health);
    int32 ActionRegenRate = ham->regenerationModifier(player, HamBar_Action);
    int32 MindRegenRate = ham->regenerationModifier(player, HamBar_Mind);

    //Because we tick every 2 seconds, we need to double this.
    HealthRegenRate += (int32)(HealthRegenRate * mHealingModifier) * 2;
    ActionRegenRate += (int32)(ActionRegenRate * mHealingModifier) * 2;
    MindRegenRate	+= (int32)(MindRegenRate * mHealingModifier) * 2;
	
	if(ham->getModifiedHitPoints(player, HamBar_Health) - player->GetStatCurrent(HamBar_Health) > 0)
    {
        //Regen Health
		mHealingDone += ham->UpdateCurrentHitpoints(player, HamBar_Health, HealthRegenRate);
    }

    if(ham->getModifiedHitPoints(player, HamBar_Action) - player->GetStatCurrent(HamBar_Action) > 0)
    {
        //Regen Action
        mHealingDone += ham->UpdateCurrentHitpoints(player, HamBar_Action, HealthRegenRate);
    }

    if(ham->getModifiedHitPoints(player, HamBar_Mind) - player->GetStatCurrent(HamBar_Mind) > 0)
    {
        //Regen Mind
        mHealingDone += ham->UpdateCurrentHitpoints(player, HamBar_Mind, HealthRegenRate);
    }

}

void	CampRegion::applyXp()
{
    //mXP = The amount of XP accumulated via vistors in the camp
    mXp += mHealingDone; //The Amount of Healing Done
}
