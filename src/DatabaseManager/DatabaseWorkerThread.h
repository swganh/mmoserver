/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H
#define ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H

#include "DatabaseType.h"
#include "Utils/typedefs.h"
#include <boost/thread/thread.hpp>

//======================================================================================================================

class Database;
class DatabaseJob;
class DatabaseImplementation;


//======================================================================================================================
class DatabaseWorkerThread
{
public:
                              DatabaseWorkerThread(DBType type, Database* datbase);
                              ~DatabaseWorkerThread(void);

  void                        Startup(int8* host, uint16 port, int8* user, int8* pass, int8* schema);
  void                        Shutdown(void);
  virtual void				  run(); 

  void                        ExecuteJob(DatabaseJob* job);

  void						  requestExit(){ mExit = true; }

protected:
  int8                        mHostname[256];
  int16                       mPort;
  int8                        mUsername[64];
  int8                        mPassword[64];
  int8                        mSchema[64];
  
private:
  void                        _startup(void);
  void                        _shutdown(void);

  bool						  mIsDone;
  Database*                   mDatabase;
  DatabaseImplementation*     mDatabaseImplementation;

  DatabaseJob*                mCurrentJob;
  DBType                      mDatabaseImplementationType;

  boost::mutex              mWorkerThreadMutex;
  boost::thread			    mThread;
  bool						  mExit;
};



//======================================================================================================================

inline void DatabaseWorkerThread::ExecuteJob(DatabaseJob* job)
{
    boost::mutex::scoped_lock lk(mWorkerThreadMutex);
    mCurrentJob = job;
}

//======================================================================================================================

#endif // ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H
