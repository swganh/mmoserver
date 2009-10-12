/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H
#define ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H

#include "Utils/mutex.h"
#include "Utils/typedefs.h"
#include "zthread/Mutex.h"
#include "zthread/Thread.h"

//======================================================================================================================
class Database;
class DatabaseJob;
class DatabaseImplementation;
enum  DBType;


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

  void                        Lock(void)              { mWorkerThreadMutex.acquire(); }
  void                        Unlock(void)            { mWorkerThreadMutex.release(); }
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

  ZThread::Mutex              mWorkerThreadMutex;
  ZThread::Thread*			  mThread;
  bool						  mExit;
};



//======================================================================================================================

inline void DatabaseWorkerThread::ExecuteJob(DatabaseJob* job)
{
  mCurrentJob = job;
}

//======================================================================================================================

class WorkerThreadRunnable : public ZThread::Runnable
{
	public:

		WorkerThreadRunnable(DatabaseWorkerThread* r){ mWorkerThread = r; }
		~WorkerThreadRunnable(){}

		virtual void run(){ mWorkerThread->run(); }

		DatabaseWorkerThread* mWorkerThread;
};

//======================================================================================================================

#endif // ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H




