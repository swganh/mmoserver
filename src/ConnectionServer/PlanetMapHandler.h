/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ADMINSERVER_PLANETMAPHANDLER_H
#define ANH_ADMINSERVER_PLANETMAPHANDLER_H

#include "ConnectionDispatchCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/typedefs.h"


//======================================================================================================================
class Message;
class Database;
class DataBinding;
class ConnectionDispatch;
class ConnectionClient;

class MapLocation
{
public:
  uint64		mId;
  int8  		mName[64];
  uint8		  mCategory;
  uint8		  mSubCategory;
  float		  mX;
  float		  mZ;
  uint8		  mListIcon;
};

class PlanetMapHandlerAsyncContainer
{
public:
  ConnectionClient*     mClient;
  string              mPlanetName;
};


//======================================================================================================================
class PlanetMapHandler : public ConnectionDispatchCallback, public DatabaseCallback
{
public:

	                              PlanetMapHandler();
	                              ~PlanetMapHandler();

	void			                    Startup(Database* database, ConnectionDispatch* dispatch);
	void			                    Shutdown();
	void			                    Process();

  // Inherited from ConnectionDispatchCallback
	virtual void	                handleDispatchMessage(uint32 opcode, Message* message, ConnectionClient* client);

  // Inherited from DatabaseCallback  
  virtual void                   handleDatabaseJobComplete(void* ref, DatabaseResult* result);

private:

	void                          _processMapLocationsRequest(Message* message, ConnectionClient* client);


	Database*                     mDatabase;
  DataBinding*                  mDataBinding;
	ConnectionDispatch*              mConnectionDispatch;
};


#endif 


