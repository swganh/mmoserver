/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_ZONESERVER_H
#define ANH_ZONESERVER_ZONESERVER_H

#include "Utils/typedefs.h"

//======================================================================================================================

class NetworkManager;
class Service;
class DatabaseManager;
class Database;

class MessageDispatch;
class CharacterLoginHandler;
class ObjectControllerDispatch;

//======================================================================================================================

class ProcessAddress
{
	public:

	  uint32	mType;
	  int8		mAddress[16];
	  uint16	mPort;
	  uint32	mStatus;
	  uint32	mActive;
};

//======================================================================================================================

class ZoneServer
{
	public:

		ZoneServer(int8* mapName);
		~ZoneServer(void);

		void	Process(void);

		void	handleWMReady();

		string  getZoneName()  { return mZoneName; }

	private:

		void	_updateDBServerList(uint32 status);
		void	_connectToConnectionServer(void);

		string                        mZoneName;

		NetworkManager*               mNetworkManager;
		DatabaseManager*              mDatabaseManager;
	
		Service*                      mRouterService;
		Database*                     mDatabase;

		MessageDispatch*              mMessageDispatch;
		CharacterLoginHandler*        mCharacterLoginHandler;
		ObjectControllerDispatch*     mObjectControllerDispatch;
};

//======================================================================================================================

#endif  // ANH_ZONESERVER_ZONESERVER_H






