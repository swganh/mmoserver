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
#include "ChatManager.h"
#include "ChatOpcodes.h"
#include "CSRManager.h"
#include "CSRObjects.h"
#include "CSROpcodes.h"
#include "Player.h"

#include "Utils/logger.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"

#include "Utils/utils.h"

#include <cassert>


//======================================================================================================================

bool			CSRManager::mInsFlag = false;
CSRManager*		CSRManager::mSingleton = NULL;

//======================================================================================================================

CSRManager::CSRManager(Database* database, MessageDispatch* dispatch, ChatManager* chatManager)
{

    mMessageDispatch = dispatch;
    mDatabase = database;
    mChatManager = chatManager;

    _registerCallbacks();
    _loadDatabindings();

    CSRAsyncContainer* asyncContainer = new CSRAsyncContainer(CSRQuery_Categories);
    mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_CSRCategoriesGet();",mDatabase->galaxy());
 
}

//======================================================================================================================

CSRManager* CSRManager::Init(Database* database, MessageDispatch* messagedispatch, ChatManager* chatManager)
{
    if (!mInsFlag)
    {
        mSingleton = new CSRManager(database, messagedispatch, chatManager);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//======================================================================================================================

CSRManager::~CSRManager()
{
    CategoryList::iterator iter = mCategoryList.begin();

    while (!mCategoryList.empty())
    {
        SubCategoryList* subCatList = (*iter)->GetSubCategories();
        SubCategoryList::iterator subIter = subCatList->begin();

        while (!subCatList->empty())
        {
            delete (*subIter);
            subCatList->erase(subIter);
            subIter = subCatList->begin();
        }

        delete (*iter);
        mCategoryList.erase(iter);
        iter = mCategoryList.begin();
    }

    _unregisterCallbacks();
    _destroyDatabindings();

    mInsFlag = false;
    mSingleton = NULL;
    // delete(mSingleton);
}

//======================================================================================================================

void CSRManager::_loadDatabindings()
{
    //Spety likes it when things line up!
    mTicketBinding = mDatabase->createDataBinding(12);
    mTicketBinding->addField(DFT_uint32,	offsetof(Ticket, mId),				4,		0);
    mTicketBinding->addField(DFT_bstring,	offsetof(Ticket, mPlayer),			33,		1);
    mTicketBinding->addField(DFT_uint32,	offsetof(Ticket, mCategoryId),		4,		2);
    mTicketBinding->addField(DFT_uint32,	offsetof(Ticket, mSubCategoryId),	4,		3);
    mTicketBinding->addField(DFT_bstring,	offsetof(Ticket, mComment),			8192,	4);
    mTicketBinding->addField(DFT_bstring,	offsetof(Ticket, mInfo),			8192,	5);
    mTicketBinding->addField(DFT_bstring,	offsetof(Ticket, mHarrasingUser),	8192,	6);
    mTicketBinding->addField(DFT_bstring,	offsetof(Ticket, mLanguage),		3,		7);
    mTicketBinding->addField(DFT_uint8,		offsetof(Ticket, mBugReport),		1,		8);
    mTicketBinding->addField(DFT_uint8,		offsetof(Ticket, mActivity),		1,		9);
    mTicketBinding->addField(DFT_uint8,		offsetof(Ticket, mClosed),			1,		10);
    mTicketBinding->addField(DFT_uint64,	offsetof(Ticket, mLastModified),	8,		11);

    mCommentBinding = mDatabase->createDataBinding(4);
    mCommentBinding->addField(DFT_uint32,	offsetof(Comment, mId),			4,		0);
    mCommentBinding->addField(DFT_uint32,	offsetof(Comment, mTicketId),	4,		1);
    mCommentBinding->addField(DFT_bstring,	offsetof(Comment, mText),		8192,	2);
    mCommentBinding->addField(DFT_bstring,	offsetof(Comment, mAuthor),		33,		3);

    mCategoryBinding = mDatabase->createDataBinding(2);
    mCategoryBinding->addField(DFT_uint32,	offsetof(Category, mId),	4,	0);
    mCategoryBinding->addField(DFT_bstring, offsetof(Category, mName),	46, 1);

    mSubCategoryBinding = mDatabase->createDataBinding(2);
    mSubCategoryBinding->addField(DFT_uint32,	offsetof(SubCategory, mId),		4,	0);
    mSubCategoryBinding->addField(DFT_bstring,	offsetof(SubCategory, mName),	46, 1);

    mArticleSearchBinding = mDatabase->createDataBinding(2);
    mArticleSearchBinding->addField(DFT_uint32,		offsetof(Article, mId),		4,	0);
    mArticleSearchBinding->addField(DFT_bstring,	offsetof(Article, mTitle),	46, 1);

    mFullArticleBinding = mDatabase->createDataBinding(3);
    mFullArticleBinding->addField(DFT_uint32,	offsetof(Article, mId),		4,		0);
    mFullArticleBinding->addField(DFT_bstring,	offsetof(Article, mTitle),	46,		1);
    mFullArticleBinding->addField(DFT_bstring,	offsetof(Article, mBody),	8192,	2);
}

//======================================================================================================================

void CSRManager::_registerCallbacks()
{
    mMessageDispatch->RegisterMessageCallback(opConnectPlayerMessage, std::bind(&CSRManager::_processConnectPlayerMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opSearchKnowledgeBaseMessage, std::bind(&CSRManager::_processSearchKnowledgeBaseMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opRequestCategoriesMessage, std::bind(&CSRManager::_processRequestCategoriesMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opNewTicketActivityMessage, std::bind(&CSRManager::_processNewTicketActivityMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opGetTicketsMessage, std::bind(&CSRManager::_processGetTicketsMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opGetCommentsMessage, std::bind(&CSRManager::_processGetCommentsMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opGetArticleMessage, std::bind(&CSRManager::_processGetArticleMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opCreateTicketMessage, std::bind(&CSRManager::_processCreateTicketMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opCancelTicketMessage, std::bind(&CSRManager::_processCancelTicketMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opAppendCommentMessage, std::bind(&CSRManager::_processAppendCommentMessage, this, std::placeholders::_1, std::placeholders::_2));
}

//======================================================================================================================

void CSRManager::_unregisterCallbacks()
{
    mMessageDispatch->UnregisterMessageCallback(opConnectPlayerMessage);
    mMessageDispatch->UnregisterMessageCallback(opSearchKnowledgeBaseMessage);
    mMessageDispatch->UnregisterMessageCallback(opRequestCategoriesMessage);
    mMessageDispatch->UnregisterMessageCallback(opNewTicketActivityMessage);
    mMessageDispatch->UnregisterMessageCallback(opGetTicketsMessage);
    mMessageDispatch->UnregisterMessageCallback(opGetCommentsMessage);
    mMessageDispatch->UnregisterMessageCallback(opGetArticleMessage);
    mMessageDispatch->UnregisterMessageCallback(opCreateTicketMessage);
    mMessageDispatch->UnregisterMessageCallback(opCancelTicketMessage);
    mMessageDispatch->UnregisterMessageCallback(opAppendCommentMessage);
}

//======================================================================================================================

void CSRManager::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mTicketBinding);
    mDatabase->destroyDataBinding(mCommentBinding);
    mDatabase->destroyDataBinding(mCategoryBinding);
    mDatabase->destroyDataBinding(mSubCategoryBinding);
    mDatabase->destroyDataBinding(mArticleSearchBinding);
    mDatabase->destroyDataBinding(mFullArticleBinding);
}

//======================================================================================================================

void CSRManager::_processConnectPlayerMessage(Message* message, DispatchClient* client)
{
    /*uint32 errorcode = */message->getUint32();

    gChatMessageLib->sendConnectPlayerResponseMessage(client);
}

//======================================================================================================================

void CSRManager::_processAppendCommentMessage( Message* message, DispatchClient* client )
{
	/*
    BString poster;
    BString comment;

    uint32 ticketid = message->getUint32();
    message->getStringAnsi(poster);
    message->getStringUnicode16(comment);
    comment.convert(BSTRType_ANSI);

    int8 cleanComment[4000], cleanPoster[4000];
    mDatabase->escapeString(cleanComment,comment.getAnsi(),comment.getLength());
    mDatabase->escapeString(cleanPoster,poster.getAnsi(), poster.getLength());
    mDatabase->executeProcedureAsync(NULL, NULL, "CALL %s.sp_CSRTicketCommentAdd(%u, '%s', '%s');", mDatabase->galaxy(),  ticketid, cleanComment, cleanPoster);
    
    mDatabase->executeProcedureAsync(NULL, NULL, "CALL %s.sp_CSRTicketActivityUpdate(%u);", mDatabase->galaxy(),  ticketid);
    

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opAppendCommentResponseMessage);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(ticketid);AAAAAAAAAAAAAAAAAAAAAARGH
	*/
}

//======================================================================================================================

void CSRManager::_processCancelTicketMessage( Message* message, DispatchClient* client )
{
    uint32 ticketid = message->getUint32();
    mDatabase->executeProcedureAsync(NULL, NULL, "CALL %s.sp_CSRTicketCancel(%u);", mDatabase->galaxy(),  ticketid);
    

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opCancelTicketResponseMessage);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(ticketid);
    Message* newMessage = gMessageFactory->EndMessage();

    client->SendChannelA(newMessage, client->getAccountId(), CR_Client, 3);
}

//======================================================================================================================

void CSRManager::_processCreateTicketMessage( Message* message, DispatchClient* client )
{
    BString playername;
    BString comment;
    BString info;
    BString harrassinguser;
    BString language;

    message->getStringAnsi(playername);
    uint32 category = message->getUint32();
    uint32 subcategory = message->getUint32();
    message->getStringUnicode16(comment);
    message->getStringUnicode16(info);
    message->getStringUnicode16(harrassinguser);
    message->getStringAnsi(language);
    /*uint32 errorcode = */
    message->getUint32();
    uint8 bugreport = message->getUint8();

    comment.convert(BSTRType_ANSI);
    info.convert(BSTRType_ANSI);;
    harrassinguser.convert(BSTRType_ANSI);

    CSRAsyncContainer* asyncContainer = new CSRAsyncContainer(CSRQuery_NewTicket);
    asyncContainer->mClient = client;

    int8 cleanPlayer[4000], cleanComment[4000], cleanInfo[4000], cleanHarrasser[4000], cleanLanguage[4000];
    mDatabase->escapeString(cleanPlayer, playername.getAnsi(), playername.getLength());
    mDatabase->escapeString(cleanInfo, info.getAnsi(), info.getLength());
    mDatabase->escapeString(cleanComment, comment.getAnsi(), comment.getLength());
    mDatabase->escapeString(cleanHarrasser, harrassinguser.getAnsi(), harrassinguser.getLength());
    mDatabase->escapeString(cleanLanguage, language.getAnsi(), language.getLength());

    mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_CSRTicketAdd('%s', %u, %u, '%s', '%s', '%s', '%s', %d);", mDatabase->galaxy(),  cleanPlayer, category, subcategory, cleanComment, cleanInfo, cleanHarrasser, cleanLanguage, bugreport);
    
}

//======================================================================================================================

void CSRManager::_processGetArticleMessage(Message *message, DispatchClient* client)
{
    BString id;
    message->getStringAnsi(id);
    id.convert(BSTRType_ANSI);

    CSRAsyncContainer* asynccontainer = new CSRAsyncContainer(CSRQuery_FullArticle);
    asynccontainer->mClient = client;
    mDatabase->executeProcedureAsync(this, asynccontainer, "CALL %s.sp_CSRKnowledgeBaseArticleGet(%s);", mDatabase->galaxy(),  id.getAnsi());
    
}

//======================================================================================================================

void CSRManager::_processGetCommentsMessage(Message *message, DispatchClient* client)
{
    uint32 ticketid = message->getUint32();

    CSRAsyncContainer* asyncContainer = new CSRAsyncContainer(CSRQuery_CommentsByTicket);
    asyncContainer->mClient = client;
    mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_CSRTicketCommentGet(%u);", mDatabase->galaxy(),  ticketid);
    
}

//======================================================================================================================

void CSRManager::_processGetTicketsMessage(Message *message, DispatchClient* client)
{
    CSRAsyncContainer* asyncContainer = new CSRAsyncContainer(CSRQuery_Tickets);
    asyncContainer->mClient = client;
    mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_CSRTicketGet (%" PRIu64 ");", mDatabase->galaxy(),  mChatManager->getPlayerByAccId(client->getAccountId())->getCharId());
    
}

//======================================================================================================================

void CSRManager::_processNewTicketActivityMessage(Message *message, DispatchClient* client)
{
	CSRAsyncContainer* asyncContainer = new CSRAsyncContainer(CSRQuery_TicketActivity);
    asyncContainer->mClient = client;
    mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_CSRTicketActivityGet (%" PRIu64 ");", mDatabase->galaxy(),  mChatManager->getPlayerByAccId(client->getAccountId())->getCharId());
    
}

//======================================================================================================================

void CSRManager::_processRequestCategoriesMessage(Message *message, DispatchClient* client)
{
    BString language;
    message->getStringAnsi(language);
    gChatMessageLib->sendRequestCategoriesResponseMessage(client, &mCategoryList);

}

//======================================================================================================================

void CSRManager::_processSearchKnowledgeBaseMessage(Message *message, DispatchClient* client)
{
    BString search;
    message->getStringUnicode16(search);
    search.convert(BSTRType_ANSI);
    BStringVector splitstring;
    search.split(splitstring, ' ');
    BStringVector::iterator iter = splitstring.begin();

    BString sql = "%";
    while (iter != splitstring.end())
    {
        int8 cleanSearchString[4000];
        mDatabase->escapeString(cleanSearchString, (*iter).getAnsi(), (*iter).getLength());
        sql << cleanSearchString;
        sql << "%";
        ++iter;
    }

    CSRAsyncContainer* asynccontainer = new CSRAsyncContainer(CSRQuery_SearchKB);
    asynccontainer->mClient = client;
    mDatabase->executeProcedureAsync(this, asynccontainer, "CALL %s.sp_CSRKnowledgeBaseArticleFind ('%s');", mDatabase->galaxy(),  sql.getAnsi());
    
}

//======================================================================================================================

void CSRManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    CSRAsyncContainer* asyncContainer = (CSRAsyncContainer*)ref;

    switch (asyncContainer->mQueryType)
    {
    case CSRQuery_Tickets:
    {
        uint64 count = result->getRowCount();

        if (count == 1) //There should only be one result
        {
            Ticket* ticket = new Ticket();
            result->getNextRow(mTicketBinding, ticket);
            ticket->mComment.convert(BSTRType_Unicode16);

            gChatMessageLib->sendGetTicketsResponseMessage(asyncContainer->mClient, ticket);
        }
    }
    break;

    case CSRQuery_CommentsByTicket:
    {
        uint64 count = result->getRowCount();

        CommentList* list = new CommentList();
        for (uint i = 0; i < count; i++)
        {
            Comment* comment = new Comment();
            result->getNextRow(mCommentBinding, comment);
            comment->mText.convert(BSTRType_Unicode16);

            list->push_back(comment);
        }
        gChatMessageLib->sendGetCommentsResponseMessage(asyncContainer->mClient, list);
    }
    break;

    case CSRQuery_TicketActivity:
    {
        uint64 count = result->getRowCount();

        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opNewTicketActivityResponseMessage);
        gMessageFactory->addUint32((uint32)count);
        gMessageFactory->addUint32((uint32)count);
        Message* message = gMessageFactory->EndMessage();

        asyncContainer->mClient->SendChannelA(message, asyncContainer->mClient->getAccountId(), CR_Client, 3);
    }
    break;

    case CSRQuery_NewTicket:
    {
        uint32 id;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32, 0, 4, 0);

        result->getNextRow(binding, &id);

        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opCreateTicketResponseMessage);
        gMessageFactory->addUint32(0);
        gMessageFactory->addUint32(id);
        Message* message = gMessageFactory->EndMessage();

        asyncContainer->mClient->SendChannelA(message, asyncContainer->mClient->getAccountId(), CR_Client, 3);
    }
    break;

    case CSRQuery_Categories:
    {
        uint64 count = result->getRowCount();

        for (uint i = 0; i < count; i++)
        {
            Category* category = new Category();

            result->getNextRow(mCategoryBinding, category);
            category->mName.convert(BSTRType_Unicode16);
            mCategoryList.push_back(category);

            CSRAsyncContainer* asContainer = new CSRAsyncContainer(CSRQuery_SubCategories);
            asContainer->mCategory = category;
            mDatabase->executeProcedureAsync(this, asContainer, "CALL %s.sp_CSRSubCategoriesGet (%u);", mDatabase->galaxy(),  category->mId);
            
        }
    }
    break;

    case CSRQuery_SubCategories:
    {
        uint64 count = result->getRowCount();

        for (uint i = 0; i < count; i++)
        {
            SubCategory* subcategory = new SubCategory();

            result->getNextRow(mSubCategoryBinding, subcategory);
            subcategory->mName.convert(BSTRType_Unicode16);
            asyncContainer->mCategory->GetSubCategories()->push_back(subcategory);
        }
    }
    break;

    case CSRQuery_SearchKB:
    {
        uint64 count = result->getRowCount();

        ArticleList* list = new ArticleList();
        for (uint i = 0; i < count; i++)
        {
            Article* article = new Article();
            result->getNextRow(mArticleSearchBinding, article);
            article->mTitle.convert(BSTRType_Unicode16);
            list->push_back(article);
        }
        gChatMessageLib->sendSearchKnowledgeBaseResponseMessage(asyncContainer->mClient, list);
    }
    break;

    case CSRQuery_FullArticle:
    {
        uint64 count = result->getRowCount();

        if (count == 1)
        {
            Article* article = new Article();
            result->getNextRow(mFullArticleBinding, article);
            article->mBody.convert(BSTRType_Unicode16);
            gChatMessageLib->sendGetArticleResponseMessage(asyncContainer->mClient, article);
        }
        else
            gChatMessageLib->sendGetArticleResponseMessage(asyncContainer->mClient, NULL);

    }
    break;

    default:
        break;
    }

    SAFE_DELETE(asyncContainer);
}

//======================================================================================================================
