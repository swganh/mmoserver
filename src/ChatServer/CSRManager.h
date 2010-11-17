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

#ifndef ANH_ZONESERVER_CSRMANAGER_H
#define ANH_ZONESERVER_CSRMANAGER_H

#include <map>

#include "DatabaseManager/DatabaseCallback.h"
#include "ChatMessageLib.h"

#define gCSRManager CSRManager::GetSingletonPtr()

class Database;
class DataBinding;
class Message;
class MessageDispatch;
class Category;
class ChatManager;
class CSRManager;

//======================================================================================================================

enum CSRQuery
{
    CSRQuery_Tickets			= 0,
    CSRQuery_CommentsByTicket	= 1,
    CSRQuery_Categories			= 2,
    CSRQuery_SubCategories		= 3,
    CSRQuery_NewTicket			= 4,
    CSRQuery_SearchKB			= 5,
    CSRQuery_FullArticle		= 6,
    CSRQuery_TicketActivity		= 7
};

//======================================================================================================================

class CSRAsyncContainer
{
public:

    CSRAsyncContainer(CSRQuery cq): mCategory(nullptr) {
        mQueryType = cq;
    }
    ~CSRAsyncContainer() {}

    CSRQuery		mQueryType;
    Category*		mCategory;
    DispatchClient*	mClient;
};

//======================================================================================================================

class CSRManager : public DatabaseCallback
{
public:

    static CSRManager* GetSingletonPtr() {
        return mSingleton;
    }
    static CSRManager* Init(Database* database, MessageDispatch* messagedispatch, ChatManager* chatManager);

    ~CSRManager();

    //Database callback
    virtual void		handleDatabaseJobComplete(void* ref,DatabaseResult* result);

private:
    CSRManager(Database* database, MessageDispatch* messagedispatch, ChatManager* chatManager);

    static CSRManager*			mSingleton;
    static bool					mInsFlag;

    void						_loadDatabindings();
    void						_destroyDatabindings();
    void						_registerCallbacks();
    void						_unregisterCallbacks();
    void						_loadCommandMap();

    Database*					mDatabase;
    MessageDispatch*			mMessageDispatch;

    DataBinding*				mTicketBinding;
    DataBinding*				mCommentBinding;
    DataBinding*				mCategoryBinding;
    DataBinding*				mSubCategoryBinding;
    DataBinding*				mArticleSearchBinding;
    DataBinding*				mFullArticleBinding;

    CategoryList				mCategoryList;

    //Packet processors
    void						_processConnectPlayerMessage(Message* message, DispatchClient* client);
    void						_processAppendCommentMessage(Message* message, DispatchClient* client);
    void						_processCancelTicketMessage(Message* message, DispatchClient* client);
    void						_processCreateTicketMessage(Message* message, DispatchClient* client);
    void						_processGetArticleMessage(Message* message, DispatchClient* client);
    void						_processGetCommentsMessage(Message* message, DispatchClient* client);
    void						_processGetTicketsMessage(Message* message, DispatchClient* client);
    void						_processNewTicketActivityMessage(Message* message, DispatchClient* client);
    void						_processRequestCategoriesMessage(Message* message, DispatchClient* client);
    void						_processSearchKnowledgeBaseMessage(Message* message, DispatchClient* client);

    ChatManager*				mChatManager;
};

#endif
