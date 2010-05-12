/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Database.h"

#include "DataBindingFactory.h"
#include "DatabaseCallback.h"
#include "DatabaseImplementation.h"
#include "DatabaseImplementationMySql.h"
#include "DatabaseJob.h"
#include "DatabaseType.h"
#include "DatabaseWorkerThread.h"
#include "Transaction.h"

#include "LogManager/LogManager.h"

#include "ConfigManager/ConfigManager.h"

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

		default:break;
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
		if(job && job->getCallback())
		{
			job->getCallback()->handleDatabaseJobComplete(job->getClientReference(), job->getDatabaseResult());
		}

		// Free the result and the job
		this->DestroyResult(job->getDatabaseResult());

		mJobPool.ordered_free(job);
	}
}

//======================================================================================================================
DatabaseResult* Database::ExecuteSynchSql(const int8* sql, ...)
{
	// format our sql string
	va_list args;
	va_start(args, sql);
	int8    localSql[8192];
	/*int32 len = */vsnprintf(localSql, sizeof(localSql), sql, args);
	#if !defined(_DEBUG)
	#endif

	#if defined(_DEBUG)
		int8 message[8192];
		sprintf(message, "WARNING: SYNCHRONOUS SQL STATEMENT: %s",localSql);
		gLogger->logMsg(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_RED);
	#endif

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
	/*int32 len = */vsnprintf(localSql, sizeof(localSql), sql, args);

	//gLogger->logMsgF("SqlDump: len:%u - %s", MSG_LOW, len, localSql);

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
	/*int32 len = */vsnprintf(localSql, sizeof(localSql), sql, args);

	//gLogger->logMsgF("SqlDump: len:%u - %s", MSG_LOW, len, localSql);

	// Setup our job.
	DatabaseJob* job = new(mJobPool.ordered_malloc()) DatabaseJob();
	job->setCallback(callback);
	job->setClientReference(ref);
	job->setSql(localSql);
	job->setMultiJob(false);

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

	// Setup our job.
	DatabaseJob* job = new(mJobPool.ordered_malloc()) DatabaseJob();
	job->setCallback(callback);
	job->setClientReference(ref);
	job->setSql(localSql);
	job->setMultiJob(false);

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
	//int32 len = vsnprintf(localSql, sizeof(localSql), sql, args);

	//gLogger->logMsgF("SqlDump: len:%u - %s", MSG_LOW, len, localSql);

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
	/*int32 len = */vsnprintf(localSql, sizeof(localSql), sql, args);

	//gLogger->logMsgF("SqlDump: len:%u - %s", MSG_LOW, len, localSql);

	// Setup our job.
	DatabaseJob* job = new(mJobPool.ordered_malloc()) DatabaseJob();
	job->setCallback(callback);
	job->setClientReference(ref);
	job->setSql(localSql);
	job->setMultiJob(true);

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




