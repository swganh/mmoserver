/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_DATABASEMANAGER_DATABASE_H
#define ANH_DATABASEMANAGER_DATABASE_H

#include "DatabaseType.h"
#include "Utils/typedefs.h"
#include "Utils/concurrent_queue.h"
#include <queue>
#include "DataBindingFactory.h"
#include <boost/pool/pool.hpp>


//======================================================================================================================

class DataBinding;
class DatabaseWorkerThread;
class DatabaseImplementation;
class DatabaseCallback;
class DatabaseResult;
class DatabaseJob;
class Transaction;

typedef Anh_Utils::concurrent_queue<DatabaseJob*>				DatabaseJobQueue;
typedef Anh_Utils::concurrent_queue<DatabaseWorkerThread*>		DatabaseWorkerThreadQueue;

//======================================================================================================================

class Database
{
  public:
                                          Database(DBType type);
                                          ~Database(void);
            
  void                                    Startup(int8* host, uint16 port, int8* user, int8* pass, int8* schema);
  void                                    Shutdown(void);
  void                                    Process(void);

  DatabaseResult*                         ExecuteSynchSql(const int8* sql, ...);
  //DatabaseResult*                         ExecuteSql(int8* sql, ...);
  void                                    ExecuteSqlAsync(DatabaseCallback* callback, void* ref, const int8* sql, ...);
  void									  ExecuteSqlAsyncNoArguments(DatabaseCallback* callback, void* ref, const int8* sql);

  DatabaseResult*                         ExecuteProcedure(const int8* sql, ...);
  void                                    ExecuteProcedureAsync(DatabaseCallback* callback, void* ref, const int8* sql, ...);

  uint32								  Escape_String(int8* target,const int8* source,uint32 length);

  void									  DestroyResult(DatabaseResult* result);

  DataBinding*                            CreateDataBinding(uint16 fieldCount);
  void									  DestroyDataBinding(DataBinding* binding);

  DatabaseWorkerThread*                   popIdleWorker();
  void									  pushIdleWorker(DatabaseWorkerThread* worker);

  void									  pushDatabaseJobComplete(DatabaseJob* job);

  Transaction*							  startTransaction(DatabaseCallback* callback, void* ref);
  void									  destroyTransaction(Transaction* t);

  bool									  releaseResultPoolMemory();	
  bool									  releaseJobPoolMemory(){ return(mJobPool.release_memory()); }
  bool									  releaseTransactionPoolMemory(){ return(mTransactionPool.release_memory()); }
  bool									  releaseBindingPoolMemory(){ return(mDataBindingFactory->releasePoolMemory()); }
  
private:

  DBType                                  mDatabaseType;      // This denotes which DB implementation we are connecting to. MySQL, Postgres, etc.

  DataBindingFactory*                     mDataBindingFactory;

  DatabaseJobQueue                        mJobPendingQueue;
  DatabaseJobQueue                        mJobCompleteQueue;
  DatabaseWorkerThreadQueue               mWorkerIdleQueue;

  DatabaseImplementation*                 mDatabaseImplementation;  // Use this implementation for any syncronous calls.

  uint32                                  mMinThreads;
  uint32                                  mMaxThreads;

  boost::pool<boost::default_user_allocator_malloc_free>							  mJobPool;
  boost::pool<boost::default_user_allocator_malloc_free>							  mTransactionPool;
protected:
	DatabaseResult*                         ExecuteSql(const int8* sql, ...);
};

//======================================================================================================================

inline DatabaseWorkerThread* Database::popIdleWorker(void)
{
  DatabaseWorkerThread* worker = 0;

  worker = mWorkerIdleQueue.pop();

  return worker;
}

//======================================================================================================================

inline void Database::pushIdleWorker(DatabaseWorkerThread* worker)
{
  mWorkerIdleQueue.push(worker);
}

//======================================================================================================================

inline void Database::pushDatabaseJobComplete(DatabaseJob* job)
{
  mJobCompleteQueue.push(job);
}

//======================================================================================================================

#endif // ANH_DATABASEMANAGER_DATABASE_H






