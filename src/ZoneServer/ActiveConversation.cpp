/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ActiveConversation.h"
#include "NPCObject.h"
#include "SkillManager.h"
#include "PlayerObject.h"
#include "MessageLib/MessageLib.h"

//=============================================================================

ActiveConversation::ActiveConversation(Conversation* baseConv,PlayerObject* player,NPCObject* npc) :
mBaseConversation(baseConv),mPlayer(player),mNpc(npc),mDI(0),mTTId(0),mTOId(0)
{
	mSelectOptionMap.reserve(16);

	mTTCustom		= L"";
	mTTStfFile		= "";
	mTTStfVariable	= "";
	mTOCustom		= L"";
	mTOStfFile		= "";
	mTOStfVariable	= "";

	mCurrentPage	= mBaseConversation->getFirstPage();

	// Update NPC with title, for now we take that from DB, option page 0.
	// It's dirty and it will remain dirty until we add names and titles to npc's the proper way.
	if (mNpc->getTitle().getLength() == 0)
	{
		mNpc->setTitle(mCurrentPage->mStfVariable);
		// gLogger->logMsgF("ActiveConversation::ActiveConversation: NPC got new Title = %s", MSG_NORMAL, mNpc->getTitle().getAnsi());
	}
	else
	{
		// gLogger->logMsgF("ActiveConversation::ActiveConversation: NPC already have a Title = %s", MSG_NORMAL, mNpc->getTitle().getAnsi());
	}
	
	if (mNpc->getFirstName().getLength() == 0)
	{
		// mNpc->setFirstName("MrRandom Name");
		// gLogger->logMsgF("ActiveConversation::ActiveConversation: NPC got new Name = %s", MSG_NORMAL, mNpc->getFirstName().getAnsi());
	}
	else
	{
		// gLogger->logMsgF("ActiveConversation::ActiveConversation: NPC already have a Name = %s", MSG_NORMAL, mNpc->getFirstName().getAnsi());
	}

	// Moved to the owner of this conversation.
	// _prepareFilteredOptions();
	
}

//=============================================================================

ActiveConversation::~ActiveConversation()
{
}

//=============================================================================

void ActiveConversation::updateCurrentPage(uint32 selectId)
{
	ConversationOption* option = mSelectOptionMap[selectId];
	
	// gLogger->logMsgF("ActiveConversation::updateCurrentPage: selectId = %u\n",MSG_NORMAL,selectId);
	uint32 pageLink = option->mPageLinkId;	// The state we are going to enter as default.

	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mPlayer);
	NPCObject*		npc		= dynamic_cast<NPCObject*>(mNpc);
	if (!player->mPosition.inRange2D(npc->mPosition,11.0f))
	{
		// We are out of range.
		mCurrentPage = NULL;
		gMessageLib->sendSystemMessage(player,L"You are out of range.");
		return;
	}

	if(option->mEvent)
	{
		// gLogger->logMsgF("ActiveConversation::updateCurrentPage: option->mEvent = %u\n",MSG_NORMAL,option->mEvent);

		int32 DI = 0;

		// buffer data for the next page
		switch(option->mEvent)
		{
			// Select skill to train event, get %DI,%TO for the next page
			case 2:
			case 3:
			{
				Skill* skill	= gSkillManager->getSkillByName(option->mStfVariable);
				// We need them both...
				DI				= skill->mSkillPointsRequired;
				mDI				= skill->mMoneyRequired;
				mTOStfFile		= option->mStfFile.getAnsi();
				mTOStfVariable	= option->mStfVariable.getAnsi();
				// string playerName(dynamic_cast<PlayerObject*>(mPlayer)->getFirstName());
				// string playerName("Eruptor");	// For test
				// playerName.convert(BSTRType_Unicode16);
				// mTOCustom = playerName;
			}
			break;
		}

		// Handle npc events BEFORE we try to enter any dialogs...
		if (pageLink)
		{
			mCurrentPage = mBaseConversation->getPage(pageLink);
		}
		
		// Prepare data and select what dialog to use.
		pageLink = mNpc->handleConversationEvent(this,mCurrentPage,option,mPlayer);

		// And now it's time for the Dirty part....
		// Since the %DI is used in several cases (credits or skillpoints missing),
		// we need to sort that out.
		if ((option->mEvent == 2) && (pageLink == 10))
		{
			// We lack the skill points to train this skill...
			mDI	= DI;
		}
	}

	// Use default pageLinkId if not overrided by event.
	if (pageLink)
	{
		// Prepare next dialog, but depending of player status, the default "next page" may have changed.
		mCurrentPage = mBaseConversation->getPage(pageLink);
		prepareFilteredOptions();
	}
	else
	{
		mCurrentPage = NULL;
	}
}

//=============================================================================
void ActiveConversation::setCurrentPage(uint32 pageLink)
{
	if (pageLink)
	{
		mCurrentPage = mBaseConversation->getPage(pageLink);
	}
}

//=============================================================================

void ActiveConversation::prepareFilteredOptions()
{
	mSelectOptionMap.clear();

	(mNpc->filterConversationOptions(mCurrentPage,&mSelectOptionMap,mPlayer));
}

//=============================================================================

void ActiveConversation::postProcessCurrentPage()
{
	// gLogger->logMsg("ActiveConversation::postProcessCurrentPage");

	if (mCurrentPage)
	{
		mNpc->postProcessfilterConversation(this, mCurrentPage,mPlayer);
	}
}

//=============================================================================

bool ActiveConversation::preProcessConversation()
{
	// gLogger->logMsg("ActiveConversation::preProcessConversation");
	bool status = false;
	if (mCurrentPage)
	{
		status = mNpc->preProcessfilterConversation(this, mBaseConversation ,mPlayer);
	}
	return status;
}






