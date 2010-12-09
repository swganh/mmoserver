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

#include "ZoneServer/ForageManager.h"

#include <list>

#include "MathLib/Rectangle.h"

#include "MessageLib/MessageLib.h"

#include "ZoneServer/MedicManager.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ScoutManager.h"
#include "ZoneServer/WorldManager.h"

ForageManager*	ForageManager::mSingleton = NULL;

ForageManager::ForageManager()
{
    pHead = NULL;
}

//================================================================================
//FORAGING!
//================================================================================

class ForageAttempt
{
public:
    ForageAttempt(PlayerObject* player, uint64 time, forageClasses forageClass)
    {
        startTime = time;
        playerID = player->getId();
        completed = false;

        mForageClass = forageClass;

        orig_x = player->mPosition.x;
        orig_y = player->mPosition.y;
        orig_z = player->mPosition.z;
    }

    uint64 startTime;
    uint64 playerID;
    forageClasses mForageClass;
    bool completed;

    float orig_x;
    float orig_y;
    float orig_z;
};

class ForagePocket
{
public:
    ForagePocket(PlayerObject* player)
    {

        innerRect = Anh_Math::Rectangle(player->mPosition.x - 10,player->mPosition.z - 10,20,20);
        outterRect = Anh_Math::Rectangle(player->mPosition.x - 30,player->mPosition.z - 30,60,60);

        pNext = NULL;
    }

    ~ForagePocket()
    {
        //This shouldn't be a problem, but it's here just in case.

        for(std::list<ForageAttempt*>::iterator it=attempts.begin(); it != attempts.end();)
        {
            delete (*it);
            it=attempts.erase(it);
        }
    }

    bool containsPlayer(PlayerObject* player)
    {
        if((outterRect.getPosition().x > player->mPosition.x) && ((outterRect.getPosition().x + outterRect.getWidth()) < player->mPosition.x))
        {
            if((outterRect.getPosition().z > player->mPosition.z) && ((outterRect.getPosition().z + outterRect.getHeight()) < player->mPosition.z))
                return true;
            else
                return false;
        }
        else
            return false;
    }

    void addAttempt(ForageAttempt* attempt)
    {
        attempts.push_back(attempt);
    }

    bool updateAttempts(uint64 currentTime); //if True Delete this Pocket, if False don't

    ForagePocket* pNext;
    Anh_Math::Rectangle outterRect;

private:
    std::list<ForageAttempt*> attempts;

    Anh_Math::Rectangle innerRect;
};


void ForageManager::startForage(PlayerObject* player, forageClasses forageClass)
{
    //Check for Inside building
    if(player->getParentId() != 0)
    {
        gForageManager->failForage(player, NOT_OUTSIDE);
        return;
    }

    //Check for combat
    if(player->states.checkState(CreatureState_Combat))
    {
        gForageManager->failForage(player, IN_COMBAT);
        return;
    }

    //Check for action being too low
    if(player->getHam()->mAction.getCurrentHitPoints() < 101)
    {
        gForageManager->failForage(player, ACTION_LOW);
        return;
    }

    //Check for skill being too low
    if(forageClass == ForageClass_Scout && !player->checkSkill(45)) //Scout -> Survival 1
    {
        gForageManager->failForage(player, NO_SKILL);
        return;
    }
    else if(forageClass == ForageClass_Medic && !player->checkSkill(51))
    {
        gForageManager->failForage(player, NO_SKILL);
        return;
    }

    //Already foraging
    if(player->isForaging())
    {
        gForageManager->failForage(player, ALREADY_FORAGING);
        return;
    }

    player->setForaging(true);

    //Starts the Foraging Animation
    gMessageLib->sendCreatureAnimation(player, std::string("forage"));

    //Use up some action!
    player->getHam()->updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints, -100);

    //Creates a ForageAttempt object for tracking the forage operation
    ForageAttempt* attempt = new ForageAttempt(player, gWorldManager->GetCurrentGlobalTick(), forageClass);

    //FIND THE APPROPRIATE FORAGEPocket
    ForagePocket* it = pHead;
    while(it != NULL)
    {
        if(it->containsPlayer(player))
        {
            it->addAttempt(attempt);
            return;
        }
        it = it->pNext;
    }

    //None of them contained the player. We need to make new one.

    ForagePocket* new_pocket = new ForagePocket(player);
    it = pHead;
    ForagePocket* previousHead = NULL;
    while(it != NULL)
    {
        previousHead = it;
        it = it->pNext;
    }

    if(previousHead == NULL)
        pHead = new_pocket;
    else
        previousHead->pNext = new_pocket;

    new_pocket->addAttempt(attempt);
}

void ForageManager::forageUpdate()
{
    ForagePocket* it = pHead;
    ForagePocket* previousHead = NULL;
    while(it != NULL)
    {
        if(it->updateAttempts(gWorldManager->GetCurrentGlobalTick())) //If true we delete this Pocket
        {
            if(previousHead == NULL)
            {
                pHead = it->pNext;
            }
            else
            {
                previousHead->pNext = it->pNext;
            }
            delete it;
            it = previousHead;
        }
        if(it != NULL)
            it = it->pNext;
    }
}


void ForageManager::failForage(PlayerObject* player, forageFails fail)
{
    if(!player || !player->isConnected())
        return;

    switch(fail)
    {
    case NOT_OUTSIDE:
        gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_inside"), player);
        break;
    case PLAYER_MOVED:
        gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_movefail"), player);
        break;
    case ACTION_LOW:
        gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_attrib"), player);
        break;
    case IN_COMBAT:
        gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_cant"), player);
        break;
    case AREA_EMPTY:
        gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_empty"), player);
        break;
    case ENTERED_COMBAT:
        gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_combatfail"), player);
        break;
    case NO_SKILL:
        gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_noskill"), player);
        break;
    case ALREADY_FORAGING:
        gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_already"), player);
        return;
    }

    player->setForaging(false);
}

bool ForagePocket::updateAttempts(uint64 currentTime)
{
    if(attempts.empty())
        return true;

    std::list<ForageAttempt*>::iterator it = attempts.begin();
    unsigned int AttemptCount = 0;
    while(it != attempts.end())
    {
        if((currentTime - (*it)->startTime) >= 300000) //5minutes until we reopen the pocket
        {
            delete (*it);
            it = attempts.erase(it);
            AttemptCount--;
        }
        else if((currentTime - (*it)->startTime) >= 8000 && !(*it)->completed)
        {
            PlayerObject* player = (PlayerObject*)gWorldManager->getObjectById((*it)->playerID);
            if(player != NULL)
            {
                if(player->states.checkState(CreatureState_Combat))
                {
                    ForageManager::failForage(player, ENTERED_COMBAT);
                    (*it)->completed = true;
                    it++;
                    AttemptCount++;
                    continue;
                }

                bool inIt = false;
                if((innerRect.getPosition().x > player->mPosition.x) && ((innerRect.getPosition().x + innerRect.getWidth()) < player->mPosition.x))
                {
                    if((innerRect.getPosition().z > player->mPosition.z) && ((innerRect.getPosition().z + innerRect.getHeight()) < player->mPosition.z))
                    {
                        inIt = true;
                    }
                }

                if(inIt && AttemptCount < 4)
                {
                    //The player has a chance to get something
                    ForageManager::successForage(player, (*it)->mForageClass);
                    (*it)->completed = true;
                }
                else
                {
                    ForageManager::failForage(player, AREA_EMPTY);
                    (*it)->completed = true;
                }
            }
            AttemptCount++;
            it++;
        }
        else
        {
            PlayerObject* player = (PlayerObject*)gWorldManager->getObjectById((*it)->playerID);
            if(!(*it)->completed && player)
            {
                if(player->states.checkState(CreatureState_Combat))
                {
                    ForageManager::failForage(player, ENTERED_COMBAT);
                    (*it)->completed = true;
                }

                if(!(*it)->completed)
                {
                    if((*it)->orig_x != player->mPosition.x || (*it)->orig_y != player->mPosition.y ||
                            (*it)->orig_z != player->mPosition.z)
                    {
                        ForageManager::failForage(player, PLAYER_MOVED);
                        (*it)->completed = true;
                    }
                }
            }
            AttemptCount++;
            it++;
        }
    }

    return false;
}


void ForageManager::successForage(PlayerObject* player, forageClasses forageClass)
{
    if(!player || player->isForaging() == false)
        return;

    switch(forageClass)
    {
    case ForageClass_Scout:
        gScoutManager->successForage(player);
        break;
    case ForageClass_Medic:
        gMedicManager->successForage(player);
        break;
    }
}
