/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ADMINSERVER_ADMINSERVER_H
#define ANH_ADMINSERVER_ADMINSERVER_H

#include "Utils/typedefs.h"


//======================================================================================================================
class CharacterAdminHandler;
class PlanetMapHandler;

class NetworkManager;
class Service;
class DatabaseManager;
class Database;

class MessageDispatch;


//======================================================================================================================
class ProcessAddress
{
public:
  uint32                          mType;
  int8                            mAddress[16];
  uint16                          mPort;
  uint32                          mStatus;
  uint32                          mActive;
};


//======================================================================================================================
class AdminServer
{
public:
                                AdminServer(void);
                                ~AdminServer(void);

  void                          Startup(void);
  void                          Shutdown(void);
  void                          Process(void);

private:
  void                          _updateDBServerList(uint32 status);
  void                          _connectToConnectionServer(void);

  NetworkManager*               mNetworkManager;
  DatabaseManager*              mDatabaseManager;

  Service*                      mRouterService;
  Database*                     mDatabase;

  MessageDispatch*              mMessageDispatch;
  CharacterAdminHandler*        mCharacterAdminHandler;
  PlanetMapHandler*				mPlanetMapHandler;
 
};



#endif //MMOSERVER_ADMINSERVER_ADMINSERVER_H



