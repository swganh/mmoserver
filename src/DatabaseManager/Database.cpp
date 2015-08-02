/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

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

#define NOMINMAX

#include "Database.h"


#ifdef ERROR
#undef ERROR
#endif

#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

#include "Utils/logger.h"

#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DataBindingFactory.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "DatabaseManager/DatabaseImplementation.h"
#include "DatabaseManager/DatabaseImplementationMySql.h"
#include "DatabaseManager/DatabaseJob.h"
#include "DatabaseManager/DatabaseType.h"
#include "DatabaseManager/DatabaseWorkerThread.h"
#include "DatabaseManager/Transaction.h"


Database::Database(DBType type, const std::string& host, uint16_t port, const std::string& user, const std::string& pass, const std::string& schema, DatabaseConfig& config) 
    : database_impl_(nullptr)
    , job_pool_(sizeof(DatabaseJob))
    , transaction_pool_(sizeof(Transaction))
{
    // Create our own DatabaseImplementation for synchronous queries
    // Create our DBImplementation object
    switch (type) {
        case DBTYPE_MYSQL: 
            database_impl_.reset(new DatabaseImplementationMySql(host, port, user, pass, schema));
            break;
    }

	uint32_t min_threads = config.getDbMinThreads();
	uint32_t max_threads = config.getDbMaxThreads();
	global_ = config.getDbGlobalSchema();
	galaxy_ = config.getDbGalaxySchema();
	config_ = config.getDbConfigSchema();

    // Create our worker threads and put them in the idle queue
    uint32_t const hardware_threads = boost::thread::hardware_concurrency();
    uint32_t const num_threads = std::min(hardware_threads != 0 ? hardware_threads : min_threads, max_threads);

    DatabaseWorkerThread* worker = nullptr;
    for (uint32_t i = 0; i < num_threads; i++) {
        worker = new DatabaseWorkerThread(type, host, port, user, pass, schema);
        idle_worker_queue_.push(worker);
    }
}


Database::~Database() {
    DatabaseWorkerThread* worker = 0;

    while(idle_worker_queue_.unsafe_size()) {
        if (idle_worker_queue_.try_pop(worker)) {
            delete worker;
        }
    }
}

void Database::executeAsyncSql(const std::stringstream& sql) {    
    // just pass the stringstream string
    executeAsyncSql(sql.str());
}

void Database::executeAsyncSql(const std::string& sql) {    
    // Setup our job.
    DatabaseJob* job = new(job_pool_.ordered_malloc()) DatabaseJob();
    job->query = sql;
    job->multi_job = false;

    // Add the job to our processList;
    job_pending_queue_.push(job);
}
void Database::executeAsyncSql(const std::stringstream& sql, AsyncDatabaseCallback callback) {    
    // just pass the stringstream string
    executeAsyncSql(sql.str(), callback);
}

void Database::executeAsyncSql(const std::string& sql, AsyncDatabaseCallback callback) {    
    // Setup our job.
    DatabaseJob* job = new(job_pool_.ordered_malloc()) DatabaseJob();
    job->callback = callback;
    job->query = sql;
    job->multi_job = false;

    // Add the job to our processList;
    job_pending_queue_.push(job);
}

void Database::executeAsyncProcedure(const std::stringstream& sql) {    
    executeAsyncProcedure(sql.str());
}

void Database::executeAsyncProcedure(const std::string& sql) {    
    // Setup our job.
    DatabaseJob* job = new(job_pool_.ordered_malloc()) DatabaseJob();
    job->query = sql;
    job->multi_job = true;

    // Add the job to our processList;
    job_pending_queue_.push(job);
}

void Database::executeAsyncProcedure(const std::stringstream& sql, AsyncDatabaseCallback callback) {    
    executeAsyncProcedure(sql.str(), callback);
}

void Database::executeAsyncProcedure(const std::string& sql, AsyncDatabaseCallback callback) {    
    // Setup our job.
    DatabaseJob* job = new(job_pool_.ordered_malloc()) DatabaseJob();
    job->callback = callback;
    job->query = sql;
    job->multi_job = true;

    // Add the job to our processList;
    job_pending_queue_.push(job);
}


void Database::process() {
    DatabaseWorkerThread* worker = nullptr;
    DatabaseJob* job = nullptr;

    // Check to see if we have any idle workers/jobs and execute them.
    int process_count = std::min(idle_worker_queue_.unsafe_size(), job_pending_queue_.unsafe_size());
    for (int i = 0; i < process_count; ++i) {
        // Pop the worker and job off their queues.
        if (!idle_worker_queue_.try_pop(worker)) {
            continue;
        }

        if (!job_pending_queue_.try_pop(job)) {
            idle_worker_queue_.push(worker);
            continue;
        }

        // Hand The job to the worker.
        worker->executeJob(job, [this] (DatabaseWorkerThread* worker, DatabaseJob* job) {
            // If this is a multi result (meaning a stored procedure was executed
            // using CALL) then there can be more than one result. Performing
            // another query before the entire result has been processed will
            // result in out of sync queries, for this reason the worker thread
            // is stored with the result, otherwise it is added back to the 
            // idle pool.
            if (job->result->isMultiResult()) {
                job->result->setWorkerReference(worker);
            } else {
                idle_worker_queue_.push(worker);
            }

            pushDatabaseJobComplete(job);      
        });
    }

    // Now process any completed jobs.
    int completed = job_complete_queue_.unsafe_size();
    for (int i = 0; i < completed; ++i) {
        // let our client handle the result, if theres a callback
        if( job_complete_queue_.try_pop(job)) {
            if (job->old_callback) {
                job->old_callback->handleDatabaseJobComplete(job->client_reference, job->result);
            }
            
            if (boost::optional<AsyncDatabaseCallback> c = job->callback) {
                (*c)(job->result);
            }

            // Free the result and the job
            destroyResult(job->result);
            job_pool_.ordered_free(job);
        }
    }
}


DatabaseResult* Database::executeSynchSql(const char* sql, ...) {
    // format our sql string
    va_list args;
    va_start(args, sql);
    char localSql[8192];
    vsnprintf(localSql, sizeof(localSql), sql, args);
    va_end(args);

    return executeSql(localSql);
}


DatabaseResult* Database::executeSql(const char* sql, ...) {
    // format our sql string
    va_list args;
    va_start(args, sql);
    char localSql[8192];

    vsnprintf(localSql, sizeof(localSql), sql, args);    
    va_end(args);

    // Run our query and return our result set.
    return database_impl_->executeSql(localSql);;
}


void Database::executeSqlAsync(DatabaseCallback* callback, 
                               void* ref, const char* sql, ...)
{
    // format our sql string
    va_list args;
    va_start(args, sql);
    char localSql[20192];
    vsnprintf(localSql, sizeof(localSql), sql, args);
    va_end(args);

    // Setup our job.
    DatabaseJob* job = new(job_pool_.ordered_malloc()) DatabaseJob();
    job->old_callback = callback;
    job->client_reference = ref;
    job->query = localSql;
    job->multi_job = false;

    // Add the job to our processList;
    job_pending_queue_.push(job);
}

//the reasoning behind this is the following
//attachments in mails are pseudostrings
//that means it is in reality binary data packaged in an unicode string
//when we escape it for mysql all the binary output gets escaped as it should
//only the % (25hex) is not escaped
//this gets interpreted as a formatting sign by vsnprintf() and subsequently is removed
//which invalidates our binary data!!!!!!!!!!!
//sch
void Database::executeSqlAsyncNoArguments(DatabaseCallback* callback, 
                                          void* ref, const char* sql) 
{
    char localSql[20192];
    sprintf(localSql, "%s", sql);

    // Setup our job.
    DatabaseJob* job = new(job_pool_.ordered_malloc()) DatabaseJob();
    job->old_callback = callback;
    job->client_reference = ref;
    job->query = localSql;
    job->multi_job = false;

    // Add the job to our processList;
    job_pending_queue_.push(job);
}


DatabaseResult* Database::executeProcedure(const char* sql, ...) {
    // format our sql string
    va_list args;
    va_start(args, sql);
    char localSql[20192];

    vsnprintf(localSql, sizeof(localSql), sql, args);
    va_end(args);

    return database_impl_->executeSql(localSql,true);
}


void Database::executeProcedureAsync(DatabaseCallback* callback, 
                                     void* ref, const char* sql, ...)
{
    // format our sql string
    va_list args;
    va_start(args, sql);
    char localSql[20192];

    vsnprintf(localSql, sizeof(localSql), sql, args);
    va_end(args);

    // Setup our job.
    DatabaseJob* job = new(job_pool_.ordered_malloc()) DatabaseJob();
    job->old_callback = callback;
    job->client_reference = ref;
    job->query = localSql;
    job->multi_job = true;

    // Add the job to our processList
    job_pending_queue_.push(job);
}


void Database::destroyResult(DatabaseResult* result) {
    DatabaseWorkerThread* worker = result->getWorkerReference();
    
    database_impl_->destroyResult(result);

    if(worker) {        
        idle_worker_queue_.push(worker);
    }
}


DataBinding* Database::createDataBinding(uint16_t fieldCount) {
    return binding_factory_.createDataBinding(fieldCount);
}


void  Database::destroyDataBinding(DataBinding* binding) {
    binding_factory_.destroyDataBinding(binding);
}


uint32_t Database::escapeString(char* target, const char* source, uint32_t length) {
    return database_impl_->escapeString(target,source,length);
}

std::string Database::escapeString(const std::string& source) {    
    return database_impl_->escapeString(source);
}


Transaction* Database::startTransaction(DatabaseCallback* callback, void* ref) {
    return(new(transaction_pool_.ordered_malloc()) Transaction(this,callback,ref));
}


void Database::destroyTransaction(Transaction* t) {
    transaction_pool_.ordered_free(t);
}


bool Database::releaseResultPoolMemory() {
    return(database_impl_->releaseResultPoolMemory());
}


bool Database::releaseJobPoolMemory() {
    return(job_pool_.release_memory());
}


bool Database::releaseTransactionPoolMemory() {
    return(transaction_pool_.release_memory());
}


bool Database::releaseBindingPoolMemory() {
    return(binding_factory_.releasePoolMemory());
}

void Database::pushDatabaseJobComplete(DatabaseJob* job) {
    job_complete_queue_.push(job);
}
