/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "QuestGiver.h"
#include "ActiveConversation.h"
#include "Conversation.h"
#include "Inventory.h"
#include "PlayerObject.h"
#include "Tutorial.h"
#include "WorldManager.h"
#include "WorldConfig.h"
#include "MessageLib/MessageLib.h"
#include "MathLib/Quaternion.h"
//=============================================================================

QuestGiver::QuestGiver() : NPCObject()
{
	mNpcFamily	= NpcFamily_QuestGiver;

	mRadialMenu = RadialMenuPtr(new RadialMenu());

	mRadialMenu->addItem(1,0,radId_converseStart,radAction_Default); 
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default); 
}

//=============================================================================

QuestGiver::~QuestGiver()
{
}

//=============================================================================

void QuestGiver::filterConversationOptions(ConversationPage* page,std::vector<ConversationOption*>* filteredOptions,PlayerObject* player)
{
	std::vector<ConversationOption*>::iterator optionIt = page->mOptions.begin();

	// gLogger->logMsgF("QuestGiver::filterConversationOptions: page->mId = %u",MSG_NORMAL,page->mId);
	switch(page->mId)
	{

		// By default, don't filter.
		case 0:
		default:
		{
			// gLogger->logMsg("Trainer::filterConversationOptions: default");
			while(optionIt != page->mOptions.end())
			{
				// gLogger->logMsg("Trainer::filterConversationOptions: pushed something");
				filteredOptions->push_back(*optionIt);
				++optionIt;
			}
		}
		break;
	}
}

//=============================================================================


void QuestGiver::postProcessfilterConversation(ActiveConversation* av,ConversationPage* page,PlayerObject* player)
{

}

//=============================================================================

bool QuestGiver::preProcessfilterConversation(ActiveConversation* av, Conversation* conversation, PlayerObject* player)
{
	// TODO: Check if NPC ready for conversation, i.e. we are allowed to do the quest.
	bool status = true;
	if (gWorldConfig->isTutorial())
	{
		// Inform Tutorial about start of npc conversations.
		player->getTutorial()->npcConversationHasStarted(this->getId());

		// Have to do a dirty test of when the npc gives the player a weapon.
#if defined(_MSC_VER)
		if (this->mId == 47513085665)
#else
		if (this->mId == 47513085665LLU)
#endif
		{
			Tutorial * tutorial = player->getTutorial();
			uint32 subState = tutorial->getSubState();
			if (subState < 16)
			{
				// av->setCurrentPage(8);	// "Move along."
				// gLogger->logMsgF("QuestGiver::preProcessfilterConversation: Returned modified pagelink %u",MSG_NORMAL, 8);
				status = false;
				char elements[5][32];
				memset(elements, 0, sizeof(elements));
				string aMess("Move along.");	// @newbie_convo:trooper_move_along
				aMess.convert(BSTRType_Unicode16);
				if (!gWorldConfig->isInstance())
				{
					gMessageLib->sendSpatialChat(this,aMess,elements);
					// gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(46)); // none.
				}
				else
				{
					gMessageLib->sendSpatialChat(this, aMess, elements, player);
					// gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(46), player); // none.
				}
			}
			if ((subState == 17) || (subState == 18))
			{
				// av->setCurrentPage(7);	/// "What are you doing here?  Get a move on.  I'm, uh, still guarding here."
				// gLogger->logMsgF("QuestGiver::preProcessfilterConversation: Returned modified pagelink %u",MSG_NORMAL, 7);

				// If player has "lost" his gun, we let the npc-officer give him a new.
				Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
				if (inventory)
				{
					if (inventory->itemExist(player->getTutorial()->getQuestWeaponFamily(), player->getTutorial()->getQuestWeaponType()))
					{
						status = false;
						char elements[5][32];
						memset(elements, 0, sizeof(elements));
						string aMess("What are you doing here?  Get a move on.  I'm, uh, still guarding here."); // @newbie_convo:convo_4_repeat
						aMess.convert(BSTRType_Unicode16);
						if (!gWorldConfig->isInstance())
						{
							gMessageLib->sendSpatialChat(this,aMess,elements);
							gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(30)); // scared
						}
						else
						{
							gMessageLib->sendSpatialChat(this, aMess, elements, player);
							gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(30), player); // scared
						}
					}
					else
					{
						// Let's give him the damn gun (when he trigs that part of the conversation, see handleConversationEvent
					}
				}
			}
			else if (subState >= 19)	// We have killed the pirate.
			{
				status = false;
				char elements[5][32];
				memset(elements, 0, sizeof(elements));
				string aMess("Good job with that pirate, but what are you doing back here?  Go back through the big room where the pirate was."); // @newbie_convo:good_job
				aMess.convert(BSTRType_Unicode16);
				if (!gWorldConfig->isInstance())
				{
					gMessageLib->sendSpatialChat(this,aMess,elements);
					gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(41)); // tiphat
				}
				else
				{
					gMessageLib->sendSpatialChat(this, aMess, elements, player);
					gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(41), player); // tiphat
				}
			}
		}
#if defined(_MSC_VER)
		else if (this->mId == 47513085685)	// The Quatermaster
#else
		else if (this->mId == 47513085685LLU)	// The Quatermaster
#endif
		{
			Tutorial * tutorial = player->getTutorial();
			uint32 subState = tutorial->getSubState();
			if (subState < 24)
			{
				// The default negative reply when trying to start a conversation in wrong state.
				av->setCurrentPage(3);		
			}
		}
	}
	return status;
}

//=============================================================================

void QuestGiver::prepareConversation(PlayerObject* player)
{

	// Let's turn to the player asking for my attention.
	float x = player->mPosition.mX - this->mPosition.mX;
	float z = player->mPosition.mZ - this->mPosition.mZ;
	float h = sqrt(x*x + z*z);

	if ((z/h) < 0.0)
	{	
		if (x/h < 0.0)
		{
			this->mDirection.mW = static_cast<float>(cos((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
			this->mDirection.mY = static_cast<float>(sin((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
		}
		else
		{
			this->mDirection.mY = sin(0.5f*acos(z/h));
			this->mDirection.mW = cos(0.5f*acos(z/h));
		}
	}
	else
	{
		this->mDirection.mY = sin(0.5f*asin(x/h));	
		this->mDirection.mW = cos(0.5f*acos(z/h));
	}
	// gLogger->logMsgF("%f %f %f %f",MSG_NORMAL, this->mDirection.mX, this->mDirection.mY, this->mDirection.mZ, this->mDirection.mW);
	
	// send out position updates to known players
	this->setInMoveCount(this->getInMoveCount() + 1);

	if (!gWorldConfig->isInstance())
	{
		if (this->getParentId())
		{
			// We are inside a cell.
			gMessageLib->sendDataTransformWithParent(this);
			gMessageLib->sendUpdateTransformMessageWithParent(this);
		}
		else
		{
			gMessageLib->sendDataTransform(this);
			gMessageLib->sendUpdateTransformMessage(this);
		}
	}
	else
	{
		if (this->getParentId())
		{
			// We are inside a cell.
			gMessageLib->sendDataTransformWithParent(this, player);
			gMessageLib->sendUpdateTransformMessageWithParent(this, player);
		}
		else
		{
			gMessageLib->sendDataTransform(this, player);
			gMessageLib->sendUpdateTransformMessage(this, player);
		}
	}
	// gLogger->logMsgF("%f %f %f %f",MSG_NORMAL, this->mDirection.mX, this->mDirection.mY, this->mDirection.mZ, this->mDirection.mW);

	setLastConversationTarget(player->getId());

	// Start the conversation timeout in case we disconnect.
	gWorldManager->addNpcConversation(300000, this);	// Trainer will restore it's facing after 5 min.
}


//=============================================================================

void QuestGiver::stopConversation(PlayerObject* player)
{
	if (player)
	{
		if (gWorldConfig->isInstance())
		{
			// Inform Tutorial about stop of npc conversations.
			player->getTutorial()->npcConversationHasEnded(this->getId());
		}

		if (player->getId() == getLastConversationTarget())
		{
			gWorldManager->addNpcConversation(15000, this);	// NPC will restore it's facing 15 seconds after finished conversation.
		}
		else if (gWorldConfig->isInstance())
		{
			if (player->getGroupId())
			{
				if (PlayerObject* lastPlayer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(getLastConversationTarget())))
				{
					if (lastPlayer->getGroupId() != player->getGroupId())
					{
						// Last target was not in my group (so it's another instance).
						gWorldManager->addNpcConversation(15000, this);	// NPC will restore it's facing 15 seconds after finished conversation.
					}
				}
			}
			else
			{
				// We are alone in the instance.
				gWorldManager->addNpcConversation(15000, this);	// NPC will restore it's facing 15 seconds after finished conversation.
			}
		}
	}
}

//=============================================================================

void QuestGiver::restorePosition(PlayerObject* player)
{
	restoreDefaultDirection();

	// send out position updates to known players
	this->setInMoveCount(this->getInMoveCount() + 1);

	if (!gWorldConfig->isInstance())
	{
		if (this->getParentId())
		{
			// We are inside a cell.
			gMessageLib->sendDataTransformWithParent(this);
			gMessageLib->sendUpdateTransformMessageWithParent(this);
		}
		else
		{
			gMessageLib->sendDataTransform(this);
			gMessageLib->sendUpdateTransformMessage(this);
		}
	}
	else
	{
		// Le't see if player still around.
		if (player->isConnected())
		{
			if (this->getParentId())
			{
				// We are inside a cell.
				gMessageLib->sendDataTransformWithParent(this, player);
				gMessageLib->sendUpdateTransformMessageWithParent(this, player);
			}
			else
			{
				gMessageLib->sendDataTransform(this, player);
				gMessageLib->sendUpdateTransformMessage(this, player);
			}
		}
	}
}

// Return pageLink for next conversation.
uint32 QuestGiver::handleConversationEvent(ActiveConversation* av,ConversationPage* page,ConversationOption* option,PlayerObject* player)
{
	// gLogger->logMsgF("QuestGiver::conversationEvent: page->mId = %u",MSG_NORMAL,page->mId);
		
	std::vector<ConversationOption*>::iterator optionIt = page->mOptions.begin();
	uint32 pageLink = page->mId;

	if (gWorldConfig->isTutorial())
	{
#if defined(_MSC_VER)
		if (this->mId == 47513085665)	// The covard Officer
#else
		if (this->mId == 47513085665LLU)	// The covard Officer
#endif
		{
			// The npc gives the player a weapon, and we have to advance to next state.
			if (page->mId == 5)
			{
				// Have to do a dirty test of when the npc gives the player a weapon.
				Tutorial * tutorial = player->getTutorial();
				uint32 subState = tutorial->getSubState();
				if (subState == 16)
				{
					tutorial->setSubState(17);
				}

				// Give player his quest gun.
				player->getTutorial()->addQuestWeapon(0, 0); // Use default weapon.
				gMessageLib->sendSystemMessage(player, L"","newbie_tutorial/system_messages","receive_weapon");
			}
		}
#if defined(_MSC_VER)
		else if (this->mId == 47513085683)	// The officer in the mission terminal room
#else
		else if (this->mId == 47513085683LLU)	// The officer in the mission terminal room
#endif
		{
			Tutorial * tutorial = player->getTutorial();
			uint32 subState = tutorial->getSubState();
			if (subState == 22)
			{
				tutorial->setSubState(23);
			}
		}
#if defined(_MSC_VER)
		else if (this->mId == 47513085685)	// The Quatermaster
#else
		else if (this->mId == 47513085685LLU)	// The Quatermaster
#endif
		{
			Tutorial * tutorial = player->getTutorial();
			uint32 subState = tutorial->getSubState();
			if (subState == 25)
			{
				// We have done the conversation, and are ready to leave this place...
				tutorial->setSubState(26);
			}
		}

	}
	return pageLink;
}


