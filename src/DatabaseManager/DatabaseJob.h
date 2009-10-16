/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_DATABASEJOB_H
#define ANH_DATABASEMANAGER_DATABASEJOB_H

#include <stdlib.h>
#include <cstring>

//======================================================================================================================
class DatabaseCallback;
class DatabaseResult;
class DataBinding;


//======================================================================================================================
class DatabaseJob
{
public:
	DatabaseJob() : mDatabaseCallback(NULL),mDatabaseResult(NULL),mClientReference(NULL),mMultiJob(false){}
  DatabaseCallback*           getCallback(void)                               { return mDatabaseCallback; }
  DatabaseResult*             getDatabaseResult(void)                         { return mDatabaseResult; };
  void*                       getClientReference(void)                        { return mClientReference; }
  int8*                       getSql(void)                                    { return mSql; }

  void                        setCallback(DatabaseCallback* callback)         { mDatabaseCallback = callback; }
  void                        setDatabaseResult(DatabaseResult* result)       { mDatabaseResult = result; }
  void                        setClientReference(void* ref)                   { mClientReference = ref; }
  void                        setSql(int8* sql)                               { strcpy(mSql, sql); }
  void						  setMultiJob(bool job){ mMultiJob = job; }
  bool						  isMultiJob(){ return mMultiJob; }

private:
  DatabaseCallback*           mDatabaseCallback;
  DatabaseResult*             mDatabaseResult;
  void*                       mClientReference;
  int8                        mSql[8192];
  bool						  mMultiJob;
};




#endif // ANH_DATABASEMANAGER_DATABASEJOB_H



