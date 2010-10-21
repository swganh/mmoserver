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

#include <cstdint>

#include <functional>
#include <list>
#include <queue>
#include <string>

#include <boost/any.hpp>
#include <boost/noncopyable.hpp>
#include <boost/pool/pool.hpp>

#include "Utils/typedefs.h"
#include "Utils/concurrent_queue.h"

#include "DatabaseManager/DatabaseCallback.h"
#include "DatabaseManager/DatabaseType.h"
#include "DatabaseManager/DataBindingFactory.h"
#include "DatabaseManager/declspec.h"


//======================================================================================================================

class DataBinding;
class DatabaseWorkerThread;
class DatabaseImplementation;
class DatabaseResult;
class DatabaseJob;
class Transaction;

typedef Anh_Utils::concurrent_queue<DatabaseJob*>				DatabaseJobQueue;
typedef Anh_Utils::concurrent_queue<DatabaseWorkerThread*>		DatabaseWorkerThreadQueue;

typedef std::list<boost::any> QueryParameters;

 // Win32 complains about stl during linkage, disable the warning.
#ifdef _WIN32
#pragma warning (push)
#pragma warning (disable : 4275 4251)
#endif

class DBMANAGER_API Database : private boost::noncopyable {
public:
    Database(DBType type, const std::string& host, uint16_t port, const std::string& user, const std::string& pass, const std::string& schema);
    ~Database(void);
    
    void executeAsyncSql(const std::string& sql, AsyncDatabaseCallback callback);
    void executeAsyncSql(const std::string& sql, const QueryParameters& parameters, AsyncDatabaseCallback callback);
    
    void executeAsyncProcedure(const std::string& sql, AsyncDatabaseCallback callback);

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
    bool									  releaseJobPoolMemory() {
        return(mJobPool.release_memory());
    }
    bool									  releaseTransactionPoolMemory() {
        return(mTransactionPool.release_memory());
    }
    bool									  releaseBindingPoolMemory() {
        return(mDataBindingFactory->releasePoolMemory());
    }
    int									  GetCount(const int8* tablename);
    int									  GetSingleValueSync(const int8* sql);
private:
    // Disable the default constructor, construction always occurs through the
    // single overloaded constructor.
    Database();

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

// Re-enable the warning.
#ifdef _WIN32
#pragma warning (pop)
#endif

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






