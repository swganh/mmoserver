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

#include "FillerNPC.h"
#include "Tutorial.h"
#include "Conversation.h"
#include "PlayerObject.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "Utils/rand.h"
#include "MessageLib/MessageLib.h"


static const uint64	tutorialPlayersPeriodUpdateTime = 5000;		// 5000ms

//=============================================================================

FillerNPC::FillerNPC() : NPCObject()
{
    mNpcFamily	= NpcFamily_Filler;

    mRadialMenu = RadialMenuPtr(new RadialMenu());

    mRadialMenu->addItem(1,0,radId_converseStart,radAction_Default);
    mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}

//=============================================================================

FillerNPC::~FillerNPC()
{
    mTutorialPlayers.clear();
}

//=============================================================================

void FillerNPC::filterConversationOptions(ConversationPage* page,std::vector<ConversationOption*>* filteredOptions,PlayerObject* player)
{
    // no filtering for now
    std::vector<ConversationOption*>::iterator optionIt = page->mOptions.begin();

    while(optionIt != page->mOptions.end())
    {
        filteredOptions->push_back(*optionIt);
        ++optionIt;
    }
}

//=============================================================================

void FillerNPC::postProcessfilterConversation(ActiveConversation* av,ConversationPage* page,PlayerObject* player)
{

}

//=============================================================================

bool FillerNPC::preProcessfilterConversation(ActiveConversation* av, Conversation* conversation, PlayerObject* player)
{
    return false;
}

//=============================================================================

void FillerNPC::handleEvents(void)
{
}

//=============================================================================

uint64 FillerNPC::handleState(uint64 timeOverdue)
{
    uint64 waitTime = tutorialPlayersPeriodUpdateTime;
    if (tutorialPlayersPeriodUpdateTime >= timeOverdue)
    {
        waitTime -= timeOverdue;
    }

    // Handle player that has registred as running the tutorial.
    TutorialPlayers::iterator it = mTutorialPlayers.begin();
    while (it != mTutorialPlayers.end())
    {
        //  The timer has expired?
        ((*it).second)->when -= tutorialPlayersPeriodUpdateTime;
        if (((*it).second)->when <= 0)
        {
            PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById((*it).first));
            if (player)
            {
                // Handle the player
                this->randomChatWithPlayer(player, (*it).second);
                ((*it).second)->when += (((*it).second)->tauntBasePeriodTime) + (gRandom->getRand() % (int32) ((*it).second)->tauntBasePeriodTime+1);
            }
            else
            {
                // Remove the expired player, and his data, please...
                delete ((*it).second);
                mTutorialPlayers.erase(it++);
            }
        }
        else
        {
            it++;
        }
    }
    if (mTutorialPlayers.empty())
    {
        // We have to un-register this npc for service.
        waitTime = 0;

        // No-no... do not fiddle with other objects iterators, when we are used by them.
        // gWorldManager->removeDormantNpc(this->getId());
    }

    return waitTime;
}

//=============================================================================


//======================================================================================================================
//
//	Add a player to the queue with players from the tutorial.
//

void FillerNPC::addTutorialPlayer(uint64 playerId, TutorialTauntConfigData* configData)
{
    // uint64 expireTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
    if (mTutorialPlayers.empty())
    {
        // We have to register this npc for service.
        gWorldManager->addDormantNpc(this->getId(), (uint64)tutorialPlayersPeriodUpdateTime);
    }
    mTutorialPlayers.insert(std::make_pair(playerId, configData));
}

//======================================================================================================================
//
//	Remove a player from the queue with players from the tutorial.
//

void FillerNPC::removeTutorialPlayer(uint64 playerId)
{
    TutorialPlayers::iterator it = mTutorialPlayers.find(playerId);
    if (it != mTutorialPlayers.end())
    {
        // Remove him, and his data, please...
        delete ((*it).second);
        mTutorialPlayers.erase(it);
    }

    if (mTutorialPlayers.empty())
    {
        // We have to un-register this npc for service.
        gWorldManager->removeDormantNpc(this->getId());
    }
}

//======================================================================================================================
//
// Send random spatial chat to a player in the tutorial.
//

void FillerNPC::randomChatWithPlayer(PlayerObject* player, TutorialTauntConfigData* configData)
{
    if (player && player->isConnected())
    {
        uint8 index = gRandom->getRand() % 5;
        std::string chatMsg = configData->taunts[index].getAnsi();
        player->getTutorial()->spatialChat(this->getId(), chatMsg);
    }
}

//======================================================================================================================
//
// Setup taunts for semi-random execution in the tutorial.
//

void FillerNPC::setupTutorialTaunts(uint64 playerId, uint64 maxTauntPeriod, BString taunt1, BString taunt2, BString taunt3, BString taunt4, BString taunt5)
{
    // lol
    TutorialTauntConfigData* configData = new TutorialTauntConfigData;
    configData->taunts[0] = taunt1;
    configData->taunts[1] = taunt2;
    configData->taunts[2] = taunt3;
    configData->taunts[3] = taunt4;
    configData->taunts[4] = taunt5;

    // Let's setup the taunt time.
    configData->tauntBasePeriodTime = maxTauntPeriod;

    configData->when = (maxTauntPeriod) + (gRandom->getRand() % (int32) (maxTauntPeriod+1));
    this->addTutorialPlayer(playerId, configData);
}





