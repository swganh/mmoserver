/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_DATABASECALLBACK_H
#define ANH_DATABASEMANAGER_DATABASECALLBACK_H

//======================================================================================================================
class DatabaseResult;


//======================================================================================================================
class DatabaseCallback
{
public:
  virtual void                    handleDatabaseJobComplete(void* ref, DatabaseResult* result) {};
};





#endif //MMOSERVER_DATABASEMANAGER_DATABASECALLBACK_H


