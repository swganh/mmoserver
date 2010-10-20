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

#include "Database.h"

// Fix for issues with glog redefining this constant
#ifdef ERROR
#undef ERROR
#endif

#include <glog/logging.h>

#include "DataBinding.h"
#include "DataBindingFactory.h"
#include "DatabaseCallback.h"
#include "DatabaseImplementation.h"
#include "DatabaseImplementationMySql.h"
#include "DatabaseJob.h"
#include "DatabaseType.h"
#include "DatabaseWorkerThread.h"
#include "Transaction.h"

#include "Common/ConfigManager.h"

#include <cstdarg>
#include <cstdlib>
#include <cstdio>

//======================================================================================================================
Database::Database(DBType type, char* host, uint16 port, char* user, char* pass, char* schema) :
    mDatabaseType(type),
    mDataBindingFactory(0),
    mDatabaseImplementation(0),
    mJobPool(sizeof(DatabaseJob)),
    mTransactionPool(sizeof(Transaction))
{
    // Create and startup our factorys
    mDataBindingFactory = new DataBindingFactory();

    // Create our own DatabaseImplementation for synchronous queries
    // Create our DBImplementation object
    switch (mDatabaseType)
    {
    case DBTYPE_MYSQL:
    {
        mDatabaseImplementation = reinterpret_cast<DatabaseImplementation*>(new DatabaseImplementationMySql(host, port, user, pass, schema));
    }
    break;

    default:
        break;
    }

    // Create our worker threads and put them in the idle queue
    mMinThreads = gConfig->read<uint32>("DBMinThreads");
    mMaxThreads = gConfig->read<uint32>("DBMaxThreads");
    DatabaseWorkerThread* newWorker = 0;
    for (uint32 i = 0; i < mMinThreads; i++)
    {
        newWorker = new DatabaseWorkerThread(mDatabaseType, this, host, port, user, pass, schema);

        pushIdleWorker(newWorker);
    }
}


//======================================================================================================================
Database::~Database(void)
{
    DatabaseWorkerThread* worker = 0;

    while(mWorkerIdleQueue.size())
    {
        worker = mWorkerIdleQueue.pop();
        delete(worker);
    }

    //shutdown local implementation
    delete(mDatabaseImplementation);

    // Shutdown our factories and destroy them.
    delete(mDataBindingFactory);
}

void Database::executeAsyncSql(const std::string& sql, AsyncDatabaseCallback callback) {    
    // Setup our job.
    DatabaseJob* job = new(mJobPool.ordered_malloc()) DatabaseJob();
    job->callback = callback;
    job->query = sql;
    job->multi_job = false;

    // Add the job to our processList;
    mJobPendingQueue.push(job);
}

void Database::executeAsyncSql(const std::string& sql, const QueryParameters& parameters, AsyncDatabaseCallback calback) {

}

void Database::executeAsyncProcedure(const std::string& sql, AsyncDatabaseCallback callback) {    
    // Setup our job.
    DatabaseJob* job = new(mJobPool.ordered_malloc()) DatabaseJob();
    job->callback = callback;
    job->query = sql;
    job->multi_job = true;

    // Add the job to our processList;
    mJobPendingQueue.push(job);
}

//======================================================================================================================

void Database::Process(void)
{
    DatabaseWorkerThread* worker = 0;
    DatabaseJob* job = 0;

    // Check to see if we have an idle worker, and a job to give it.
    if(mWorkerIdleQueue.size() && mJobPendingQueue.size())
    {
        // Pop the worker and job off thier queues.
        worker	= mWorkerIdleQueue.pop();
        job		= mJobPendingQueue.pop();

        // Hand The job to the worker.
        worker->ExecuteJob(job);
    }

    // Now process any completed jobs.
    uint32 completedCount = mJobCompleteQueue.size();

    for (uint32 i = 0; i < completedCount; i++)
    {
        // pop a job
        job = mJobCompleteQueue.pop();

        // let our client handle the result, if theres a callback
        if(job) {
            if (job->old_callback) {
                job->old_callback->handleDatabaseJobComplete(job->client_reference, job->result);
            } 
            
            if (boost::optional<AsyncDatabaseCallback> c = job->callback) {
                (*c)(job->result);
            }
        }

        // Free the result and the job
        this->DestroyResult(job->result);

        mJobPool.ordered_free(job);
    }
}
//======================================================================================================================
int Database::GetCount(const int8* tablename)
{
    int8    sql[100];
    sprintf(sql, "SELECT COUNT(*) FROM %s;",tablename);
    return GetSingleValueSync(sql);
}
//======================================================================================================================
int Database::GetSingleValueSync(const int8* sql)
{
    uint32 value = 0;
    DatabaseResult* result = ExecuteSql(sql);

    DataBinding* bind = CreateDataBinding(1);
    bind->addField(DFT_uint32,0,4,0);
    result->GetNextRow(bind,&value);
    DestroyResult(result);
    if(bind) SAFE_DELETE(bind);
    return value;
}
//======================================================================================================================
DatabaseResult* Database::ExecuteSynchSql(const int8* sql, ...)
{
    // format our sql string
    va_list args;
    va_start(args, sql);
    int8    localSql[8192];
    /*int32 len = */
    vsnprintf(localSql, sizeof(localSql), sql, args);
#if !defined(_DEBUG)
#endif

    int8 message[8192];
    sprintf(message, "SYNCHRONOUS SQL STATEMENT: %s",localSql);
    DLOG(INFO) << "SYNCHRONOUS SQL: " << localSql;
    //gLogger->logS(LogManager::DEBUG,(LOG_CHANNEL_FILE | LOG_CHANNEL_SYSLOG), message);
    //gLogger->log(LogManager::SQL,"sql :: %s",localSql); // SQL Debug Log
    va_end(args);
    return ExecuteSql(localSql);
}
DatabaseResult* Database::ExecuteSql(const int8* sql, ...)
{

    DatabaseResult* newResult = 0;

    // format our sql string
    va_list args;
    va_start(args, sql);
    int8    localSql[8192];
    /*int32 len = */
    vsnprintf(localSql, sizeof(localSql), sql, args);

    // Run our query and return our result set.
    newResult = mDatabaseImplementation->ExecuteSql(localSql);

    va_end(args);
    return newResult;
}


//======================================================================================================================

void Database::ExecuteSqlAsync(DatabaseCallback* callback, void* ref, const int8* sql, ...)
{
    // format our sql string
    va_list args;
    va_start(args, sql);
    int8    localSql[20192];
    /*int32 len = */
    vsnprintf(localSql, sizeof(localSql), sql, args);
    
    DLOG(INFO) << "sql: " << localSql;
    //just put it here centrally so we can save tons of time editing ???
    //gLogger->log(LogManager::SQL,"sql :: %s",localSql); // SQL Debug Log

    // Setup our job.
    DatabaseJob* job = new(mJobPool.ordered_malloc()) DatabaseJob();
    job->old_callback = callback;
    job->client_reference = ref;
    job->query = localSql;
    job->multi_job = false;

    // Add the job to our processList;
    mJobPendingQueue.push(job);

    va_end(args);
}

//the reasoning behind this is the following
//attachments in mails are pseudostrings
//that means it is in reality binary data packaged in an unicode string
//when we escape it for mysql all the binary output gets escaped as it should
//only the % (25hex) is not escaped
//this gets interpreted as a formatting sign by vsnprintf() and subsequently is removed
//which invalidates our binary data!!!!!!!!!!!
//sch
void Database::ExecuteSqlAsyncNoArguments(DatabaseCallback* callback, void* ref, const int8* sql)
{
    int8    localSql[20192];

    sprintf(localSql,"%s", sql);
    
    DLOG(INFO) << "sql: " << localSql;
    //gLogger->log(LogManager::SQL,"sql :: %s",localSql); // SQL Debug Log

    // Setup our job.
    DatabaseJob* job = new(mJobPool.ordered_malloc()) DatabaseJob();
    job->old_callback = callback;
    job->client_reference = ref;
    job->query = localSql;
    job->multi_job = false;

    // Add the job to our processList;
    mJobPendingQueue.push(job);
}
//======================================================================================================================

DatabaseResult* Database::ExecuteProcedure(const int8* sql, ...)
{
    DatabaseResult* newResult = 0;

    // format our sql string
    va_list args;
    va_start(args, sql);
    int8    localSql[20192];
    /*int32 len = */
    vsnprintf(localSql, sizeof(localSql), sql, args);
    //int32 len = vsnprintf(localSql, sizeof(localSql), sql, args);

    // Run our query and return our result set.
    newResult = mDatabaseImplementation->ExecuteSql(localSql,true);

    va_end(args);

    return newResult;
}


//======================================================================================================================

void Database::ExecuteProcedureAsync(DatabaseCallback* callback, void* ref, const int8* sql, ...)
{
    // format our sql string
    va_list args;
    va_start(args, sql);
    int8    localSql[20192];
    /*int32 len = */
    vsnprintf(localSql, sizeof(localSql), sql, args);
    
    DLOG(INFO) << "sql: " << localSql;
    //gLogger->log(LogManager::SQL,"sql :: %s",localSql); // SQL Debug Log
    // Setup our job.
    DatabaseJob* job = new(mJobPool.ordered_malloc()) DatabaseJob();
    job->old_callback = callback;
    job->client_reference = ref;
    job->query = localSql;
    job->multi_job = true;

    // Add the job to our processList
    mJobPendingQueue.push(job);

    va_end(args);
}

//======================================================================================================================

void Database::DestroyResult(DatabaseResult* result)
{
    DatabaseWorkerThread* worker = mDatabaseImplementation->DestroyResult(result);

    if(worker)
    {
        pushIdleWorker(worker);
    }
}


//======================================================================================================================
DataBinding* Database::CreateDataBinding(uint16 fieldCount)
{
    return mDataBindingFactory->CreateDataBinding(fieldCount);
}


//======================================================================================================================
void  Database::DestroyDataBinding(DataBinding* binding)
{
    mDataBindingFactory->DestroyDataBinding(binding);
}

//======================================================================================================================

uint32 Database::Escape_String(int8* target,const int8* source,uint32 length)
{
    return(mDatabaseImplementation->Escape_String(target,source,length));
}

//======================================================================================================================

Transaction* Database::startTransaction(DatabaseCallback* callback, void* ref)
{
    return(new(mTransactionPool.ordered_malloc()) Transaction(this,callback,ref));
}

//======================================================================================================================

void Database::destroyTransaction(Transaction* t)
{
    mTransactionPool.ordered_free(t);
}


//======================================================================================================================

bool Database::releaseResultPoolMemory()
{
    return(mDatabaseImplementation->releaseResultPoolMemory());
}

//======================================================================================================================




