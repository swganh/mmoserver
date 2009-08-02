/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CHATSERVER_H
#define ANH_CHATSERVER_H

#include "Utils/typedefs.h"
#include "Common/DispatchClient.h"

//======================================================================================================================

class NetworkManager;
class Service;
class DatabaseManager;
class Database;
class MessageDispatch;
class ChatManager;
class TradeManagerChatHandler;
class GroupManager;
class CSRManager;

//======================================================================================================================

class ProcessAddress
{
	public:

		uint32		mType;
		int8        mAddress[16];
		uint16      mPort;
		uint32      mStatus;
		uint32      mActive;
};

//======================================================================================================================
class ChatServer
{
	public:

		ChatServer();
		~ChatServer();

		void	Startup();
		void    Shutdown();
		void    Process();

	private:

		void    _updateDBServerList(uint32 status);
		void    _connectToConnectionServer();

		NetworkManager*				  mNetworkManager;
		DatabaseManager*              mDatabaseManager;

		Service*                      mRouterService;
		Database*                     mDatabase;

		MessageDispatch*              mMessageDispatch;

		TradeManagerChatHandler*	  mTradeManagerChatHandler;
		ChatManager*				  mChatManager;
		GroupManager*				  mGroupManager;
		CSRManager*					  mCSRManager;

		DispatchClient*				  mClient;

};

#endif 



