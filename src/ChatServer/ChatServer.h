/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CHATSERVER_H
#define ANH_CHATSERVER_H

#include "Utils/typedefs.h"

//======================================================================================================================

class CharacterAdminHandler;
class ChatManager;
class CSRManager;
class Database;
class DatabaseManager;
class DispatchClient;
class GroupManager;
class MessageDispatch;
class NetworkManager;
class PlanetMapHandler;
class Service;
class TradeManagerChatHandler;
class StructureManagerChatHandler;

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

		CharacterAdminHandler*        mCharacterAdminHandler;
		PlanetMapHandler*			  mPlanetMapHandler;
		TradeManagerChatHandler*	  mTradeManagerChatHandler;
		StructureManagerChatHandler*  mStructureManagerChatHandler;
		ChatManager*				  mChatManager;
		GroupManager*				  mGroupManager;
		CSRManager*					  mCSRManager;

		DispatchClient*				  mClient;

};

#endif 



