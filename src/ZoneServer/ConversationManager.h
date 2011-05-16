/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_CONVERSATIONMANAGER_H
#define ANH_ZONESERVER_CONVERSATIONMANAGER_H

#define gConversationManager	ConversationManager::getSingletonPtr()

#include "Utils/typedefs.h"
#include "DatabaseManager/DatabaseCallback.h"
#include <boost/pool/pool.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <map>


//=========================================================================================#

class Database;
class DatabaseResult;
class CVAsyncContainer;
class Conversation;
class ActiveConversation;
class ConversationPage;
class ConversationOption;
class PlayerObject;
class NPCObject;

//=========================================================================================

typedef boost::ptr_map<uint32,Conversation>	Conversations;
typedef std::map<uint64,ActiveConversation*>	ActiveConversations;

//=========================================================================================

enum ConversationQuery
{
    ConvQuery_Conversations		= 1,
    ConvQuery_Pages				= 2,
    ConvQuery_Page_OptionBatch	= 3
};

//=========================================================================================

class CVAsyncContainer
{
public:

    CVAsyncContainer(ConversationQuery query) {
        mQuery = query;
    }
    ~CVAsyncContainer() {}

    ConversationQuery	mQuery;
    Conversation*		mConversation;
    ConversationPage*	mConversationPage;
    ConversationOption*	mConversationOption;
};

//=========================================================================================

class ConversationManager : public DatabaseCallback
{
public:

    static ConversationManager*		Init(Database* database);
    static ConversationManager*		getSingletonPtr() {
        return mSingleton;
    }

    ~ConversationManager();

    virtual void					handleDatabaseJobComplete(void* ref, DatabaseResult* result);

    void							startConversation(NPCObject* npc,PlayerObject* player);
    void							stopConversation(PlayerObject* player,bool sendStop = false);
    void							updateConversation(uint32 selectId,PlayerObject* player);

    Conversation*					getConversation(uint32 id);
    ActiveConversation*				getActiveConversation(uint64 id);

private:

    ConversationManager(Database* database);

    static bool					mInsFlag;
    static ConversationManager*		mSingleton;
    Database*						mDatabase;

    Conversations					mConversations;
    ActiveConversations				mActiveConversations;
    boost::pool<>					mActiveConversationPool;
    boost::pool<>					mDBAsyncPool;
};

//=========================================================================================

#endif

