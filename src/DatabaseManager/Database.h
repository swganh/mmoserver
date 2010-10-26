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
#include <memory>
#include <queue>
#include <string>

#include <boost/noncopyable.hpp>
#include <boost/pool/pool.hpp>

#include <tbb/concurrent_queue.h>

#include "DatabaseManager/DatabaseCallback.h"
#include "DatabaseManager/DatabaseType.h"
#include "DatabaseManager/DataBindingFactory.h"
#include "DatabaseManager/declspec.h"

struct DatabaseJob;
class DataBinding;
class DatabaseWorkerThread;
class DatabaseImplementation;
class DatabaseResult;
class Transaction;

typedef tbb::concurrent_queue<DatabaseJob*> DatabaseJobQueue;
typedef tbb::concurrent_queue<DatabaseWorkerThread*> DatabaseWorkerThreadQueue;

 // Win32 complains about stl during linkage, disable the warning.
#ifdef _WIN32
#pragma warning (push)
#pragma warning (disable : 4275 4251)
#endif

class DBMANAGER_API Database : private boost::noncopyable {
public:
    Database(DBType type, const std::string& host, uint16_t port, const std::string& user, const std::string& pass, const std::string& schema);
    ~Database();
    
    /*! Executes an asynchronus sql query and invokes the specified callback on
    * completion.
    *
    * \param sql The sql query to run.
    * \param callback The callback to invoke once the sql query has been executed.
    */
    void executeAsyncSql(const std::string& sql, AsyncDatabaseCallback callback);

    /*! Executes an asynchronus stored procedure and invokes the specified 
    * callback on completion.
    *
    * \param sql The sql query to run.
    * \param callback The callback to invoke once the sql query has been executed.
    */
    void executeAsyncProcedure(const std::string& sql, AsyncDatabaseCallback callback);

    void Process();
    
    DatabaseResult* ExecuteSynchSql(const char* sql, ...);

    void ExecuteSqlAsync(DatabaseCallback* callback, void* ref, const char* sql, ...);
    void ExecuteSqlAsyncNoArguments(DatabaseCallback* callback, void* ref, const char* sql);

    DatabaseResult* ExecuteProcedure(const char* sql, ...);
    void ExecuteProcedureAsync(DatabaseCallback* callback, void* ref, const char* sql, ...);

    uint32_t Escape_String(char* target, const char* source, uint32_t length);

    void DestroyResult(DatabaseResult* result);

    DataBinding* CreateDataBinding(uint16_t fieldCount);
    void DestroyDataBinding(DataBinding* binding);

    Transaction* startTransaction(DatabaseCallback* callback, void* ref);
    void destroyTransaction(Transaction* t);

    bool releaseResultPoolMemory();
    bool releaseJobPoolMemory();
    bool releaseTransactionPoolMemory();
    bool releaseBindingPoolMemory();

private:
    // Disable the default constructor, construction always occurs through the
    // single overloaded constructor.
    Database();

    DatabaseResult* ExecuteSql(const char* sql, ...);
    
    void pushDatabaseJobComplete(DatabaseJob* job);

    DataBindingFactory binding_factory_;

    DatabaseJobQueue job_pending_queue_;
    DatabaseJobQueue job_complete_queue_;
    DatabaseWorkerThreadQueue idle_worker_queue_;

    std::unique_ptr<DatabaseImplementation> database_impl_;  // Use this implementation for any syncronous calls.
    
    boost::pool<boost::default_user_allocator_malloc_free> job_pool_;
    boost::pool<boost::default_user_allocator_malloc_free> transaction_pool_;
};

// Re-enable the warning.
#ifdef _WIN32
#pragma warning (pop)
#endif

#endif // ANH_DATABASEMANAGER_DATABASE_H
