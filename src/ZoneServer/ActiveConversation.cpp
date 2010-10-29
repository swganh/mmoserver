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

#include "ActiveConversation.h"
#include "Conversation.h"
#include "NPCObject.h"
#include "PlayerObject.h"
#include "SkillManager.h"

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
    }

    if (mNpc->getFirstName().getLength() == 0)
    {
        // mNpc->setFirstName("MrRandom Name");
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

    uint32 pageLink = option->mPageLinkId;	// The state we are going to enter as default.

    PlayerObject*	player	= dynamic_cast<PlayerObject*>(mPlayer);
    NPCObject*		npc		= dynamic_cast<NPCObject*>(mNpc);
    if (glm::distance(player->mPosition, npc->mPosition) > 11.0f)
    {
        // We are out of range.
        mCurrentPage = NULL;
        gMessageLib->SendSystemMessage(common::OutOfBand("system_msg", "out_of_range"), player);
        return;
    }

    if(option->mEvent)
    {
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
    if (mCurrentPage)
    {
        mNpc->postProcessfilterConversation(this, mCurrentPage,mPlayer);
    }
}

//=============================================================================

bool ActiveConversation::preProcessConversation()
{
    bool status = false;
    if (mCurrentPage)
    {
        status = mNpc->preProcessfilterConversation(this, mBaseConversation ,mPlayer);
    }
    return status;
}






