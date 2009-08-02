/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ADMINSERVER_PLANETMAPHANDLER_H
#define ANH_ADMINSERVER_PLANETMAPHANDLER_H

#include "DatabaseManager/DatabaseCallback.h"
#include "Common/MessageDispatchCallback.h"
#include "Utils/typedefs.h"


//======================================================================================================================
class Message;
class Database;
class DataBinding;
class MessageDispatch;
class DispatchClient;

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
  DispatchClient*     mClient;
  string              mPlanetName;
};


//======================================================================================================================
class PlanetMapHandler : public MessageDispatchCallback, public DatabaseCallback
{
public:

	                              PlanetMapHandler();
	                              ~PlanetMapHandler();

	void			                    Startup(Database* database, MessageDispatch* dispatch);
	void			                    Shutdown();
	void			                    Process();

  // Inherited from MessageDispatchCallback
	virtual void	                handleDispatchMessage(uint32 opcode, Message* message, DispatchClient* client);

  // Inherited from DatabaseCallback  
  virtual void                   handleDatabaseJobComplete(void* ref, DatabaseResult* result);

private:

	void                          _processMapLocationsRequest(Message* message, DispatchClient* client);


	Database*                     mDatabase;
  DataBinding*                  mDataBinding;
	MessageDispatch*              mMessageDispatch;
};


#endif 


