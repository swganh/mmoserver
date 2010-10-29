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

#include "QuestGiver.h"
#include "ActiveConversation.h"
#include "Conversation.h"
#include "Inventory.h"
#include "PlayerObject.h"
#include "Tutorial.h"
#include "WorldManager.h"
#include "WorldConfig.h"
#include "MessageLib/MessageLib.h"

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

    // gLogger->log(LogManager::DEBUG,"QuestGiver::filterConversationOptions: page->mId = %u",page->mId);
    switch(page->mId)
    {

        // By default, don't filter.
    case 0:
    default:
    {
        // gLogger->log(LogManager::DEBUG,"Trainer::filterConversationOptions: default");
        while(optionIt != page->mOptions.end())
        {
            // gLogger->log(LogManager::DEBUG,"Trainer::filterConversationOptions: pushed something");
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
                // gLogger->log(LogManager::DEBUG,"QuestGiver::preProcessfilterConversation: Returned modified pagelink %u", 8);
                status = false;
                std::wstring message(L"@newbie_tutorial/newbie_convo:trooper_move_along");
                if (!gWorldConfig->isInstance())
                {
                    gMessageLib->SendSpatialChat(this, message);
                    // gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(46)); // none.
                }
                else
                {
                    gMessageLib->SendSpatialChat(this, message, player);
                    // gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(46), player); // none.
                }
            }
            if ((subState == 17) || (subState == 18))
            {
                // av->setCurrentPage(7);	/// "What are you doing here?  Get a move on.  I'm, uh, still guarding here."
                // gLogger->log(LogManager::DEBUG,"QuestGiver::preProcessfilterConversation: Returned modified pagelink %u", 7);

                // If player has "lost" his gun, we let the npc-officer give him a new.
                Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
                if (inventory)
                {
                    if (inventory->itemExist(player->getTutorial()->getQuestWeaponFamily(), player->getTutorial()->getQuestWeaponType()))
                    {
                        status = false;
                        std::wstring message(L"@newbie_tutorial/newbie_convo:convo_4_repeat");
                        if (!gWorldConfig->isInstance())
                        {
                            gMessageLib->SendSpatialChat(this, message);
                            gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(30)); // scared
                        }
                        else
                        {
                            gMessageLib->SendSpatialChat(this, message, player);
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
                std::wstring message(L"@newbie_tutorial/newbie_convo:good_job");
                if (!gWorldConfig->isInstance())
                {
                    gMessageLib->SendSpatialChat(this, message);
                    gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(41)); // tiphat
                }
                else
                {
                    gMessageLib->SendSpatialChat(this, message, player);
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
    faceObject(player);

    // send out position updates to known players
    // @TODO This should not be a part of the Object's responsibility
    //  since in theory simulation objects shouldn't know about the methods
    //  used to transport data. Consider abstracting this into a layer similar
    //  to the one proposed for deltas.
    this->setInMoveCount(this->getInMoveCount() + 1);

    if (!gWorldConfig->isInstance())
    {
        if (this->getParentId())
        {
            // We are inside a cell.
            gMessageLib->sendDataTransformWithParent053(this);
            gMessageLib->sendUpdateTransformMessageWithParent(this);
        }
        else
        {
            gMessageLib->sendDataTransform053(this);
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
    // gLogger->log(LogManager::DEBUG,"%f %f %f %f", this->mDirection.x, this->mDirection.y, this->mDirection.z, this->mDirection.w);

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
            gMessageLib->sendDataTransformWithParent053(this);
            gMessageLib->sendUpdateTransformMessageWithParent(this);
        }
        else
        {
            gMessageLib->sendDataTransform053(this);
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
    // gLogger->log(LogManager::DEBUG,"QuestGiver::conversationEvent: page->mId = %u",page->mId);

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
                gMessageLib->SendSystemMessage(::common::OutOfBand("newbie_tutorial/system_messages", "receive_weapon"), player);
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


