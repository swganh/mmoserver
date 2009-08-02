/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "FillerNPC.h"
#include "MathLib/Quaternion.h"

#include "MessageLib/MessageLib.h"
#include "WorldManager.h"
#include "WorldConfig.h"

#include "Conversation.h"
#include "PlayerObject.h"

static const int64	tutorialPlayersPeriodUpdateTime = 5000;		// 5000ms

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
	// gLogger->logMsgF("FillerNPC::handleState() Entering",MSG_NORMAL);

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
				((*it).second)->when += (((*it).second)->tauntBasePeriodTime) + (gRandom->getRand() % (int32) ((*it).second)->tauntBasePeriodTime);
			}
			else
			{
				// Remove the expired player, and his data, please... 
				// gLogger->logMsgF("FillerNPC::handleState: Remove the expired player with id %llu",MSG_NORMAL, (*it).first);
				delete ((*it).second);
				it = mTutorialPlayers.erase(it);
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

		// gLogger->logMsgF("FillerNPC::handleState: Unregister this service, since we have no players.",MSG_NORMAL);
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
		// gLogger->logMsgF("FillerNPC::handleState: Register this service, since we now have a player.",MSG_NORMAL);
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
		string chatMsg = configData->taunts[index];
		chatMsg.convert(BSTRType_Unicode16);
		char quack[5][32];
		memset(quack, 0, sizeof(quack));
		gMessageLib->sendSpatialChat(this, chatMsg, quack, player);
	}
}

//======================================================================================================================
//
// Setup taunts for semi-random execution in the tutorial.
//

void FillerNPC::setupTutorialTaunts(uint64 playerId, uint64 maxTauntPeriod, string taunt1, string taunt2, string taunt3, string taunt4, string taunt5)
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

	configData->when = (maxTauntPeriod) + (gRandom->getRand() % (int32) (maxTauntPeriod));
	this->addTutorialPlayer(playerId, configData);
}





