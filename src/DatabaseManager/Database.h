/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "DatabaseManager/declspec.h"


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
  DBMANAGER_API                                         Database(DBType type,int8* host, uint16 port, int8* user, int8* pass, int8* schema);
  DBMANAGER_API                                         ~Database(void);

  DBMANAGER_API void                                    Process(void);

  DBMANAGER_API DatabaseResult*                         ExecuteSynchSql(const int8* sql, ...);
  //DatabaseResult*                         ExecuteSql(int8* sql, ...);
  DBMANAGER_API void                                    ExecuteSqlAsync(DatabaseCallback* callback, void* ref, const int8* sql, ...);
  DBMANAGER_API void									  ExecuteSqlAsyncNoArguments(DatabaseCallback* callback, void* ref, const int8* sql);

  DBMANAGER_API DatabaseResult*                         ExecuteProcedure(const int8* sql, ...);
  DBMANAGER_API void                                    ExecuteProcedureAsync(DatabaseCallback* callback, void* ref, const int8* sql, ...);

  DBMANAGER_API uint32								  Escape_String(int8* target,const int8* source,uint32 length);

  DBMANAGER_API void									  DestroyResult(DatabaseResult* result);

  DBMANAGER_API DataBinding*                            CreateDataBinding(uint16 fieldCount);
  DBMANAGER_API void									  DestroyDataBinding(DataBinding* binding);

  DBMANAGER_API DatabaseWorkerThread*                   popIdleWorker();
  DBMANAGER_API void									  pushIdleWorker(DatabaseWorkerThread* worker);

  DBMANAGER_API void									  pushDatabaseJobComplete(DatabaseJob* job);

  DBMANAGER_API Transaction*							  startTransaction(DatabaseCallback* callback, void* ref);
  DBMANAGER_API void									  destroyTransaction(Transaction* t);

  DBMANAGER_API bool									  releaseResultPoolMemory();	
  DBMANAGER_API bool									  releaseJobPoolMemory(){ return(mJobPool.release_memory()); }
  DBMANAGER_API bool									  releaseTransactionPoolMemory(){ return(mTransactionPool.release_memory()); }
  DBMANAGER_API bool									  releaseBindingPoolMemory(){ return(mDataBindingFactory->releasePoolMemory()); }
  DBMANAGER_API int									  GetCount(const int8* tablename);
  DBMANAGER_API int									  GetSingleValueSync(const int8* sql);
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






