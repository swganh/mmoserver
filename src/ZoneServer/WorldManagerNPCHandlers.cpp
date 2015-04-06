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

#include "WorldManager.h"
#include "WorldConfig.h"
#include "Utils/rand.h"
#include "PlayerObject.h"
#include "ConversationManager.h"
#include "NpcManager.h"
#include "NPCObject.h"
#include "Inventory.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"
#include "Utils/utils.h"
#include "Utils/clock.h"


//======================================================================================================================
//
//	Add a npc to the Dormant queue.
//

void WorldManager::addDormantNpc(uint64 creature, uint64 when)
{
    // gLogger->log(LogManager::DEBUG,"Adding dormant NPC handler... %" PRIu64 "",  creature);

    uint64 expireTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
    mNpcDormantHandlers.insert(std::make_pair(creature, expireTime + when));
}

//======================================================================================================================
//
//	Remove a npc from the Dormant queue.
//

void WorldManager::removeDormantNpc(uint64 creature)
{
    NpcDormantHandlers::iterator it = mNpcDormantHandlers.find(creature);
    if (it != mNpcDormantHandlers.end())
    {
        // Remove creature.
        mNpcDormantHandlers.erase(it);
    }
}

//======================================================================================================================
//
//	Force a npc from the Dormant queue to be handled at next tick.
//

void WorldManager::forceHandlingOfDormantNpc(uint64 creature)
{
    NpcDormantHandlers::iterator it = mNpcDormantHandlers.find(creature);
    if (it != mNpcDormantHandlers.end())
    {
        // Change the event time to NOW.
        uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();
        (*it).second = now;
    }
}
//======================================================================================================================
//
// Handle the queue of Dormant npc's.
//

bool WorldManager::_handleDormantNpcs(uint64 callTime, void* ref)
{
    NpcDormantHandlers::iterator it = mNpcDormantHandlers.begin();
    while (it != mNpcDormantHandlers.end())
    {
        //  The timer has expired?
        if (callTime >= ((*it).second))
        {
            // Yes, handle it.
            NPCObject* npc = dynamic_cast<NPCObject*>(this->getObjectById((*it).first));
            if (npc)
            {
                // uint64 waitTime = NpcManager::Instance()->handleDormantNpc(creature, callTime - (*it).second);
                // gLogger->log(LogManager::DEBUG,"Dormant... ID = %" PRIu64 "",  (*it).first);
                uint64 waitTime = NpcManager::Instance()->handleNpc(npc, callTime - (*it).second);

                if (waitTime)
                {
                    // Set next execution time.
                    (*it).second = callTime + waitTime;
                }
                else
                {
                    // gLogger->log(LogManager::DEBUG,"Removed dormant NPC handler... %" PRIu64 "",  (*it).first);

                    // Requested to remove the handler.
                    mNpcDormantHandlers.erase(it++);
                }
            }
            else
            {
                // Remove the expired object...
                mNpcDormantHandlers.erase(it++);
            }
        }
        else
        {
            ++it;
        }
    }
    return true;
}

//======================================================================================================================
//
//	Add a npc to the Ready queue.
//

void WorldManager::addReadyNpc(uint64 creature, uint64 when)
{
    uint64 expireTime = Anh_Utils::Clock::getSingleton()->getLocalTime();

    mNpcReadyHandlers.insert(std::make_pair(creature, expireTime + when));
}

//======================================================================================================================
//
//	Remove a npc from the Ready queue.
//

void WorldManager::removeReadyNpc(uint64 creature)
{
    NpcReadyHandlers::iterator it = mNpcReadyHandlers.find(creature);
    if (it != mNpcReadyHandlers.end())
    {
        // Remove creature.
        mNpcReadyHandlers.erase(it);
    }
}

//======================================================================================================================
//
//	Force a npc from the Ready queue to be handled at next tick.
//

void WorldManager::forceHandlingOfReadyNpc(uint64 creature)
{
    NpcReadyHandlers::iterator it = mNpcReadyHandlers.find(creature);
    if (it != mNpcReadyHandlers.end())
    {
        // Change the event time to NOW.
        uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();
        (*it).second = now;
    }
}

//======================================================================================================================
//
// Handle the queue of Ready npc's.
//

bool WorldManager::_handleReadyNpcs(uint64 callTime, void* ref)
{
    NpcReadyHandlers::iterator it = mNpcReadyHandlers.begin();
    while (it != mNpcReadyHandlers.end())
    {
        //  The timer has expired?
        if (callTime >= ((*it).second))
        {
            // Yes, handle it.
            NPCObject* npc = dynamic_cast<NPCObject*>(this->getObjectById((*it).first));
            if (npc)
            {
                // uint64 waitTime = NpcManager::Instance()->handleReadyNpc(creature, callTime - (*it).second);
                // gLogger->log(LogManager::DEBUG,"Ready...");
                // gLogger->log(LogManager::DEBUG,"Ready... ID = %" PRIu64 "",  (*it).first);
                uint64 waitTime = NpcManager::Instance()->handleNpc(npc, callTime - (*it).second);
                if (waitTime)
                {
                    // Set next execution time.
                    (*it).second = callTime + waitTime;
                }
                else
                {
                    // Requested to remove the handler.
                    mNpcReadyHandlers.erase(it++);
                    // gLogger->log(LogManager::DEBUG,"Removed ready NPC handler...");
                }
            }
            else
            {
                // Remove the expired object...
                mNpcReadyHandlers.erase(it++);
                // gLogger->log(LogManager::DEBUG,"Removed ready NPC handler...");
            }
        }
        else
        {
            ++it;
        }
    }
    return true;
}

//======================================================================================================================
//
//	Add a npc to the Active queue.
//

void WorldManager::addActiveNpc(uint64 creature, uint64 when)
{
    uint64 expireTime = Anh_Utils::Clock::getSingleton()->getLocalTime();

    mNpcActiveHandlers.insert(std::make_pair(creature, expireTime + when));
}

//======================================================================================================================
//
//	Remove a npc from the Active queue.
//

void WorldManager::removeActiveNpc(uint64 creature)
{
    NpcActiveHandlers::iterator it = mNpcActiveHandlers.find(creature);
    if (it != mNpcActiveHandlers.end())
    {
        // Remove creature.
        mNpcActiveHandlers.erase(it);
    }
}

//======================================================================================================================
//
// Handle the queue of Active npc's.
//
bool WorldManager::_handleActiveNpcs(uint64 callTime, void* ref)
{
    NpcActiveHandlers::iterator it = mNpcActiveHandlers.begin();
    while (it != mNpcActiveHandlers.end())
    {
        //  The timer has expired?
        if (callTime >= ((*it).second))
        {
            // Yes, handle it.
            NPCObject* npc = dynamic_cast<NPCObject*>(this->getObjectById((*it).first));
            if (npc)
            {
                // uint64 waitTime = NpcManager::Instance()->handleActiveNpc(creature, callTime - (*it).second);
                // gLogger->log(LogManager::DEBUG,"Active...");
                // gLogger->log(LogManager::DEBUG,"Active... ID = %" PRIu64 "",  (*it).first);
                uint64 waitTime = NpcManager::Instance()->handleNpc(npc, callTime - (*it).second);
                if (waitTime)
                {
                    // Set next execution time.
                    (*it).second = callTime + waitTime;
                }
                else
                {
                    // Requested to remove the handler.
                    mNpcActiveHandlers.erase(it++);
                    // gLogger->log(LogManager::DEBUG,"Removed active NPC handler...");
                }
            }
            else
            {
                // Remove the expired object...
                mNpcActiveHandlers.erase(it++);
                // gLogger->log(LogManager::DEBUG,"Removed active NPC handler...");
            }
        }
        else
        {
            ++it;
        }
    }
    return true;
}

//======================================================================================================================

uint64 WorldManager::getRandomNpNpcIdSequence()
{
    // We need two free consequetives (sp?) id's for a NPC since NPC's have an Inventory. :)
    bool done = false;
    uint64 randomNpIdPair = 0;
    uint64 counter = 0;

    while (!done)
    {
        randomNpIdPair = getRandomNpId();
        // gLogger->log(LogManager::DEBUG,"Got %" PRIu64 "",  randomNpIdPair);

        // if (checkdNpId(randomNpIdPair) && checkdNpId(randomNpIdPair + 1))
        // Check if next id is free to use
        if (checkdNpId(randomNpIdPair + 1))
        {
            // Yes, reserve it!
            if (addNpId(randomNpIdPair + 1))
            {
                done = true;
            }
        }
        else
        {
            // Release the current id.
            removeNpId(randomNpIdPair);
        }

        if (counter++ > 1000)
        {
            // TODO: Better handling of this...
            randomNpIdPair = 0;
            break;
        }
    }
    return randomNpIdPair;
}

uint64 WorldManager::getRandomNpId()
{

    int32 watchDogCounter = 10000;
    bool found = false;
    uint64 id;
    while ((found == false) && (watchDogCounter > 0))
    {
#if defined(_MSC_VER)
        id = (gRandom->getRand()%1000000) + 422212465065984;
#else
        id = (gRandom->getRand()%1000000) + 422212465065984LLU;
#endif
        if (checkdNpId(id))
        {
            // reserve the id
            addNpId(id);
            found = true;
        }
        else
        {
            // We need to some sort of indication of system failure.
            watchDogCounter--;

        }
    }

    if (found == false)
    {
        id = 0;
    }
    return id;

    // return (gRandom->getRand()%1000000 + 422212465065984);
}

//======================================================================================================================
// Returns true if id not in use.
bool WorldManager::checkdNpId(uint64 id)
{
    NpIdSet::iterator it = mUsedTmpIds.find(id);
    return (it == mUsedTmpIds.end());
}

//======================================================================================================================


bool WorldManager::removeNpId(uint64 id)
{
    NpIdSet::iterator it = mUsedTmpIds.find(id);

    if(it != mUsedTmpIds.end())
    {
        mUsedTmpIds.erase(it);

        return(true);
    }

    return(false);
}

//======================================================================================================================
//
// get a random npc phrase
//

std::pair<std::wstring,uint32> WorldManager::getRandNpcChatter()
{

    if(mvNpcChatter.size())
        return(mvNpcChatter[gRandom->getRand()%mvNpcChatter.size()]);
    else
        return(std::make_pair(std::wstring(L"quack"),2));
}

//======================================================================================================================

void WorldManager::addNpcConversation(uint64 interval, NPCObject* npc)
{
    // Remove npc if already in list, we use the interval from last player that invoked conversation.

    NpcConversionTime* target = new NpcConversionTime;

    target->mNpc = npc;
    target->mTargetId = npc->getLastConversationTarget();
    target->mGroupId = 0;
    if (PlayerObject* player = dynamic_cast<PlayerObject*>(getObjectById(npc->getLastConversationTarget())))
    {
        target->mGroupId = player->getGroupId();
    }

    if (gWorldConfig->isInstance())
    {
        // We are running in an instance.
        NpcConversionTimers::iterator it = mNpcConversionTimers.begin();
        while (it != mNpcConversionTimers.end())
        {
            // Only erase if it's same player/group that updates AND when running zone as an instance.
            // gLogger->log(LogManager::DEBUG,"Comp NPC %" PRIu64 " and %" PRIu64 "", ((*it).first), npc);
            if (((*it).first) == npc->getId())
            {
                if (target->mGroupId)
                {
                    // We belong to a group.
                    if (target->mGroupId == ((*it).second)->mGroupId)
                    {
                        // gLogger->log(LogManager::DEBUG,"Delete (group) %" PRIu64 "", ((*it).second)->mTargetId);
                        delete ((*it).second);
                        it = mNpcConversionTimers.erase(it);
                        continue;
                    }
                }
                else
                {

                    if (target->mTargetId == ((*it).second)->mTargetId)
                    {
                        delete ((*it).second);
                        it = mNpcConversionTimers.erase(it);
                        continue;
                    }
                    else
                    {
                        // It may be a new instance using this object.
                        // gLogger->log(LogManager::DEBUG,"Unknown target.");
                    }
                }
            }
            ++it;
        }
    }
    else
    {
        // gLogger->log(LogManager::DEBUG,"Not instanced.");
        NpcConversionTimers::iterator it = mNpcConversionTimers.begin();
        while (it != mNpcConversionTimers.end())
        {
            if (((*it).first) == npc->getId())
            {
                // gLogger->log(LogManager::DEBUG,"Delete %" PRIu64 "", ((*it).second)->mTargetId);
                delete ((*it).second);
                mNpcConversionTimers.erase(it);
                break;
            }
            ++it;
        }
    }
    uint64 expireTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
    target->mInterval = expireTime + interval;
    mNpcConversionTimers.push_back(std::make_pair(npc->getId(), target));
}

//======================================================================================================================
//
// Handle npc conversations that has expired.
//
bool WorldManager::_handleNpcConversionTimers(uint64 callTime,void* ref)
{
    NpcConversionTimers::iterator it = mNpcConversionTimers.begin();
    while (it != mNpcConversionTimers.end())
    {
        // gLogger->log(LogManager::DEBUG,"WorldManager::_handleNpcConversionTimers: Checking callTime %" PRIu64 " againts %" PRIu64 "", callTime, (*it).second);
        // Npc timer has expired?
        if (callTime >= ((*it).second)->mInterval)
        {
            // gLogger->log(LogManager::DEBUG,"Calling restorePosition()");
            // Yes, rotate npc back to original position.
            if (PlayerObject* player = dynamic_cast<PlayerObject*>(getObjectById(((*it).second)->mTargetId)))
            {
                if (NPCObject* npc = dynamic_cast<NPCObject*>(getObjectById((*it).first)))
                {
                    npc->restorePosition(player);
                }
            }

            // Remove npc from list.
            // gLogger->log(LogManager::DEBUG,"\nActivated and deleted %" PRIu64 "", ((*it).second)->mTargetId);
            delete ((*it).second);
            it = mNpcConversionTimers.erase(it);
            continue;
        }
        ++it;
    }
    return (true);
}


