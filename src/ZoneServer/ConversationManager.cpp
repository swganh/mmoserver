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

#include "ConversationManager.h"


#include "Utils/logger.h"

#include "ActiveConversation.h"
#include "Conversation.h"
#include "NPCObject.h"
#include "PlayerObject.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

//=========================================================================================

bool					ConversationManager::mInsFlag = false;
ConversationManager*	ConversationManager::mSingleton = NULL;

//=========================================================================================

ConversationManager::ConversationManager(Database* database) :
    mDatabase(database),
    mActiveConversationPool(sizeof(ActiveConversation)),
    mDBAsyncPool(sizeof(CVAsyncContainer))
{
    mDatabase->executeSqlAsync(this,new(mDBAsyncPool.malloc()) CVAsyncContainer(ConvQuery_Conversations),"SELECT id FROM %s.conversations ORDER BY id",mDatabase->galaxy());
    
}

//=========================================================================================

ConversationManager* ConversationManager::Init(Database* database)
{
    if(mInsFlag == false)
    {
        mSingleton = new ConversationManager(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=========================================================================================

ConversationManager::~ConversationManager()
{
    mInsFlag = false;
    delete(mSingleton);
}

//=========================================================================================

void ConversationManager::handleDatabaseJobComplete(void* ref, DatabaseResult* result)
{
    CVAsyncContainer* asyncContainer = reinterpret_cast<CVAsyncContainer*>(ref);

    switch(asyncContainer->mQuery)
    {
    case ConvQuery_Conversations:
    {
        Conversation*		conv;
        CVAsyncContainer*	asCont;
        uint32				insertId;

        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,offsetof(Conversation,mId),4,0);

        uint64 count = result->getRowCount();

        for(uint32 i = 0; i < count; i++)
        {
            conv = new Conversation();
            result->getNextRow(binding,conv);

            insertId = conv->getId();
            mConversations.insert(insertId,conv);

            asCont = new(mDBAsyncPool.malloc()) CVAsyncContainer(ConvQuery_Pages);
            asCont->mConversation = conv;

            mDatabase->executeSqlAsync(this,asCont,"SELECT * FROM %s.conversation_pages WHERE conversation_id=%u ORDER BY page",mDatabase->galaxy(), insertId);
            
        }

		if (count)
		{
			LOG(INFO) << "Loaded " << count << " conversations";
		}

        mDatabase->destroyDataBinding(binding);
    }
    break;

    case ConvQuery_Pages:
    {
        ConversationPage*	page;
        CVAsyncContainer*	asCont;
        uint32				batchId;

        DataBinding*		pageBinding = mDatabase->createDataBinding(5);
        pageBinding->addField(DFT_uint32,offsetof(ConversationPage,mId),4,1);
        pageBinding->addField(DFT_bstring,offsetof(ConversationPage,mCustomText),512,2);
        pageBinding->addField(DFT_bstring,offsetof(ConversationPage,mStfFile),255,3);
        pageBinding->addField(DFT_bstring,offsetof(ConversationPage,mStfVariable),255,4);
        pageBinding->addField(DFT_uint32,offsetof(ConversationPage,mAnimation),4,6);

        DataBinding*	batchBinding = mDatabase->createDataBinding(1);
        batchBinding->addField(DFT_uint32,0,4,5);

        uint32 count = static_cast<uint32>(result->getRowCount());

        for(uint64 i = 0; i< count; i++)
        {
            page = new ConversationPage();
            result->getNextRow(pageBinding,page);

            page->mCustomText.convert(BSTRType_Unicode16);

            result->resetRowIndex(static_cast<int>(i));

            result->getNextRow(batchBinding,&batchId);

            asyncContainer->mConversation->mPages.push_back(page);


            // query options
            asCont = new(mDBAsyncPool.malloc()) CVAsyncContainer(ConvQuery_Page_OptionBatch);
            asCont->mConversationPage = page;

            mDatabase->executeSqlAsync(this,asCont,"SELECT conversation_options.id,conversation_options.customText,conversation_options.stf_file,"
                                       "conversation_options.stf_variable,conversation_options.event,conversation_options.pageLink "
                                       "FROM "
                                       "conversation_option_batches "
                                       "INNER JOIN conversation_options ON (conversation_option_batches.option_id = conversation_options.id) "
                                       "WHERE "
                                       "(conversation_option_batches.id = %u) ORDER BY conversation_option_batches.option_id", batchId);
           
        }

        mDatabase->destroyDataBinding(pageBinding);
        mDatabase->destroyDataBinding(batchBinding);
    }
    break;

    case ConvQuery_Page_OptionBatch:
    {
        ConversationOption*	option;
        DataBinding*		binding = mDatabase->createDataBinding(6);

        binding->addField(DFT_uint32,offsetof(ConversationOption,mId),4,0);
        binding->addField(DFT_bstring,offsetof(ConversationOption,mCustomText),512,1);
        binding->addField(DFT_bstring,offsetof(ConversationOption,mStfFile),255,2);
        binding->addField(DFT_bstring,offsetof(ConversationOption,mStfVariable),255,3);
        binding->addField(DFT_uint32,offsetof(ConversationOption,mEvent),4,4);
        binding->addField(DFT_uint32,offsetof(ConversationOption,mPageLinkId),4,5);

        uint64 count = result->getRowCount();

        for(uint32 i = 0; i < count; i++)
        {
            option = new ConversationOption();

            result->getNextRow(binding,option);

            option->mCustomText.convert(BSTRType_Unicode16);

            asyncContainer->mConversationPage->mOptions.push_back(option);
        }


        mDatabase->destroyDataBinding(binding);
    }
    break;

    default:
        break;
    }

    mDBAsyncPool.free(asyncContainer);
}

//=========================================================================================

Conversation* ConversationManager::getConversation(uint32 id)
{
    Conversations::iterator it = mConversations.find(id);

    if(it != mConversations.end())
        return((*it).second);

    return(NULL);
}

//=========================================================================================

ActiveConversation* ConversationManager::getActiveConversation(uint64 id)
{
    ActiveConversations::iterator it = mActiveConversations.find(id);

    if(it != mActiveConversations.end())
        return((*it).second);

    return(NULL);
}

//=========================================================================================

void ConversationManager::startConversation(NPCObject* npc,PlayerObject* player)
{
	//we can't converse whilst in combat
	/* commented out for preview as borked. client ctd ftl
	if(player->states.checkState(CreatureState_Combat))
	{
		stopConversation(player,true);
		gMessageLib->sendSystemMessage(player,"You may not start a Conversation whilst in Combat!");
	}*/

	// make sure theres no conversation running yet
	if(getActiveConversation(player->getId()) != NULL)
	{
		stopConversation(player);
	}

	// initialize a new one
	Conversation*		conv			= getConversation(npc->getInternalAttribute<uint32>("base_conversation"));
	ActiveConversation*	av				= new(mActiveConversationPool.malloc()) ActiveConversation(conv,player,npc);
	// ConversationPage*	currentPage		= av->getCurrentPage();

	mActiveConversations.insert(std::make_pair(player->getId(),av));

	// In case of npc trainers, they may not always open a dialog, they just chat in spatial. (like traniers that you can not train from yet)
	// We need a way to abort the dialog.
	// Pre process npc conversation.
	if (av->preProcessConversation())
	{
		ConversationPage* currentPage = av->getCurrentPage();

		// Get the options dialog data.
		av->prepareFilteredOptions();

		gMessageLib->sendStartNPCConversation(npc,player);

		if(currentPage->mAnimation)
		{
			if (gWorldConfig->isInstance())
			{
				// We are running in an instance.
				gMessageLib->sendCreatureAnimation(npc,gWorldManager->getNpcConverseAnimation(currentPage->mAnimation), player);
			}
			else
			{
				gMessageLib->sendCreatureAnimation(npc,gWorldManager->getNpcConverseAnimation(currentPage->mAnimation));
			}
		}

		gMessageLib->sendNPCDialogMessage(av,player);
		
		gMessageLib->sendNPCDialogOptions(av->getFilteredOptions(),player);
	}
	else
	{
		// We terminate (do not start) this conversation.
		stopConversation(player);
	}
}

//=========================================================================================

void ConversationManager::stopConversation(PlayerObject* player,bool sendStop)
{
    ActiveConversations::iterator it = mActiveConversations.find(player->getId());

    if (it != mActiveConversations.end())
    {
        if(sendStop)
        {
            gMessageLib->sendStopNPCConversation((*it).second->getNpc(),player);
        }

        // Notify NPC before destruction.
        (*it).second->getNpc()->stopConversation(player);

        mActiveConversationPool.free((*it).second);
        mActiveConversations.erase(it);

    }
}

//=========================================================================================

void ConversationManager::updateConversation(uint32 selectId,PlayerObject* player)
{
    ActiveConversation* av	= getActiveConversation(player->getId());

    if(!av)
    {
    	LOG(ERR) << "Could not find conversation intended for player [" << player->getId() << "]";
        return;
    }

    av->updateCurrentPage(selectId);

    ConversationPage*	currentPage = av->getCurrentPage();

    if(!currentPage)
    {
        stopConversation(player,true);
        return;
    }

    if(currentPage->mAnimation)
    {
        if (gWorldConfig->isInstance())
        {
            // We are running in an instance.
            gMessageLib->sendCreatureAnimation(av->getNpc(),gWorldManager->getNpcConverseAnimation(currentPage->mAnimation), player);
        }
        else
        {
            gMessageLib->sendCreatureAnimation(av->getNpc(),gWorldManager->getNpcConverseAnimation(currentPage->mAnimation));
        }
        // gMessageLib->sendCreatureAnimation(av->getNpc(),gWorldManager->getNpcConverseAnimation(currentPage->mAnimation));
    }

    gMessageLib->sendNPCDialogMessage(av,player);

    gMessageLib->sendNPCDialogOptions(av->getFilteredOptions(),player);

    // Post process npc conversation.
    av->postProcessCurrentPage();

}

//=========================================================================================

