/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CSRMANAGER_H
#define ANH_ZONESERVER_CSRMANAGER_H

#include <map>

#include "DatabaseManager/DatabaseCallback.h"
#include "Common/MessageDispatchCallback.h"
#include "ChatMessageLib.h"

#define gCSRManager CSRManager::GetSingletonPtr()

class Database;
class DataBinding;
class Message;
class MessageDispatch;
class Category;
class ChatManager;
class CSRManager;

typedef void (CSRManager::*CSRFuncPointer)(Message*,DispatchClient*);
typedef std::map<uint32, CSRFuncPointer> CSRCommandMap;

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

		CSRAsyncContainer(CSRQuery cq): mCategory(NULL) { mQueryType = cq; }
		~CSRAsyncContainer(){}

		CSRQuery		mQueryType;
		Category*		mCategory;
		DispatchClient*	mClient;
};

//======================================================================================================================

class CSRManager : public MessageDispatchCallback, public DatabaseCallback
{
	public:

		static CSRManager* GetSingletonPtr() { return mSingleton; }
		static CSRManager* Init(Database* database, MessageDispatch* messagedispatch, ChatManager* chatManager);

		~CSRManager();

		//Database callback
		virtual void		handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client);
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
		CSRCommandMap				mCommandMap;

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
