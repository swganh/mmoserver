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
#include "DatabaseManager/DatabaseConfig.h"

struct DatabaseJob;
class DataBinding;
class DatabaseWorkerThread;
class DatabaseImplementation;
class DatabaseResult;
class Transaction;

typedef tbb::concurrent_queue<DatabaseJob*> DatabaseJobQueue;
typedef tbb::concurrent_queue<DatabaseWorkerThread*> DatabaseWorkerThreadQueue;

/*! An encapsulation of a connection to a database.
*/
class Database : private boost::noncopyable 
{
public:
    /*! Connects to a specified database.
    *
    * \param type The type of database to connect to (such as MySQL).
    * \param host The database host to connect to.
    * \param port The port of the database host to connect to.
    * \param user The username for accessing the requested schema.
    * \param pass The password for accessing the requested schema.
    * \param schema The database to connect to.
	* \param db_min_threads The minimum number of threads used to process database work.
	* \param db_max_threads The maximum number of threads used to process database work.
    */
    Database(DBType type, const std::string& host, uint16_t port, const std::string& user, const std::string& pass, const std::string& schema, DatabaseConfig& config);
    ~Database();

    /*! Executes an asynchronus sql query.
    *   with a stringstream
    * \param sql The sql query to run.
    * 
    */
    void executeAsyncSql(const std::stringstream& sql);
    
    /*! Executes an asynchronus sql query.
    *
    * \param sql The sql query to run.
    */
    void executeAsyncSql(const std::string& sql);

    /*! Executes an asynchronus sql query and invokes the specified callback
    *   on completion with a stringstream
    * \param sql The sql query to run.
    * 
    */
    void executeAsyncSql(const std::stringstream& sql, AsyncDatabaseCallback callback);

    /*! Executes an asynchronus sql query and invokes the specified callback on
    * completion.
    *
    * \param sql The sql query to run.
    * \param callback The callback to invoke once the sql query has been executed.
    */
    void executeAsyncSql(const std::string& sql, AsyncDatabaseCallback callback);

    /*! Executes an asynchronus stored procedure.
    *
    * \param sql The sql query to run.
    */
    void executeAsyncProcedure(const std::stringstream& sql);
        
    /*! Executes an asynchronus stored procedure.
    *
    * \param sql The sql query to run.
    */
    void executeAsyncProcedure(const std::string& sql);

        /*! Executes an asynchronus stored procedure and invokes the specified 
    * callback on completion.
    *
    * \param sql The sql query to run.
    * \param callback The callback to invoke once the sql query has been executed.
    */
    void executeAsyncProcedure(const std::stringstream& sql, AsyncDatabaseCallback callback);

    /*! Executes an asynchronus stored procedure and invokes the specified 
    * callback on completion.
    *
    * \param sql The sql query to run.
    * \param callback The callback to invoke once the sql query has been executed.
    */
    void executeAsyncProcedure(const std::string& sql, AsyncDatabaseCallback callback);
    
    /*! Processes async queries.
    */
    void process();
    
    /*! Executes an sql query with an unspecified number of parameters.
    *
    * \depricated This method is being phased out for a more type-safe solution.
    */
    DatabaseResult* executeSynchSql(const char* sql, ...);

    /*! Executes an sql query asynchronusly with an unspecified number of parameters.
    *
    * \depricated This method is being phased out for a more type-safe solution.
    */
    void executeSqlAsync(DatabaseCallback* callback, void* ref, const char* sql, ...);
    
    /*! Executes an sql query asynchronusly.
    *
    * \depricated This method is being phased out for a more type-safe solution.
    */
    void executeSqlAsyncNoArguments(DatabaseCallback* callback, void* ref, const char* sql);

    /*! Executes an sql procedure with an unspecified number of parameters.
    *
    * \depricated This method is being phased out for a more type-safe solution.
    */
    DatabaseResult* executeProcedure(const char* sql, ...);

    /*! Executes an sql procedure asynchronusly with an unspecified number of parameters.
    *
    * \depricated This method is being phased out for a more type-safe solution.
    */
    void executeProcedureAsync(DatabaseCallback* callback, void* ref, const char* sql, ...);

    /*! Escapes a string to prepare for storage in a database.
    *
    * \param target The container to hold the escaped string.
    * \param source The original string that needs escaping.
    * \param length The length of the original string.
    * 
    * \return Returns the length of the escaped string.
    */
    uint32_t escapeString(char* target, const char* source, uint32_t length);

    
    /*! Escapes a string to prepare for storage in a database.
    *
    * \param source The original string that needs escaping.
    * 
    * \return Returns the escaped string.
    */
    std::string escapeString(const std::string& source);

    /*! Destroys the requested database result.
    *
    * \param result The database result to destroy.
    */
    void destroyResult(DatabaseResult* result);

    /*! Creates a databinding for retrieving sql data.
    *
    * \depricated This method is being phased out for a more type-safe solution.
    */
    DataBinding* createDataBinding(uint16_t fieldCount);

    /*! Destroys a databinding for retrieving sql data.
    *
    * \depricated This method is being phased out for a more type-safe solution.
    */
    void destroyDataBinding(DataBinding* binding);

    /*! Begins a transaction intended to execute multiple statements.
    *
    * \param callback The database callback to invoke at the end of the query.
    * \param ref State data needed for the callback to process the results of the transaction.
    *
    * \return A transaction object to execute multiple queries with.
    */
    Transaction* startTransaction(DatabaseCallback* callback, void* ref);

    /*! Destroys the requested database transaction.
    *
    * \param t The database transaction to destroy.
    */
    void destroyTransaction(Transaction* t);

    /*! Releases the memory allocated for result sets.
    *
    * \return Returns true if the pool was released, false if not.
    */
    bool releaseResultPoolMemory();

    /*! Releases the memory allocated for asynchronus query jobs.
    *
    * \return Returns true if the pool was released, false if not.
    */
    bool releaseJobPoolMemory();

    /*! Releases the memory allocated for transactions.
    *
    * \return Returns true if the pool was released, false if not.
    */
    bool releaseTransactionPoolMemory();

    /*! Releases the memory allocated for row bindings.
    *
    * \return Returns true if the pool was released, false if not.
    */
    bool releaseBindingPoolMemory();

    const char* global() { return global_.c_str(); }
    const char* galaxy() { return galaxy_.c_str(); }
    const char* config() { return config_.c_str(); }

private:
    // Disable the default constructor, construction always occurs through the
    // single overloaded constructor.
    Database();

    DatabaseResult* executeSql(const char* sql, ...);
    
    void pushDatabaseJobComplete(DatabaseJob* job);

    DataBindingFactory binding_factory_;

    DatabaseJobQueue job_pending_queue_;
    DatabaseJobQueue job_complete_queue_;
    DatabaseWorkerThreadQueue idle_worker_queue_;

    std::unique_ptr<DatabaseImplementation> database_impl_;  // Use this implementation for any syncronous calls.
    
    boost::pool<boost::default_user_allocator_malloc_free> job_pool_;
    boost::pool<boost::default_user_allocator_malloc_free> transaction_pool_;

    std::string global_;
    std::string galaxy_;
    std::string config_;
};

#endif // ANH_DATABASEMANAGER_DATABASE_H
