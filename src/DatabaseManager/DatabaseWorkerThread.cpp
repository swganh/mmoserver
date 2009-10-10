/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "DatabaseWorkerThread.h"

#include "Database.h"
#include "DatabaseImplementation.h"
#include "DatabaseImplementationMySql.h"
#include "DatabaseJob.h"
#include "DatabaseType.h"
#include "LogManager/LogManager.h"


//======================================================================================================================

DatabaseWorkerThread::DatabaseWorkerThread(DBType type, Database* database) :
mDatabase(database),
mDatabaseImplementation(0),
mCurrentJob(0),
mDatabaseImplementationType(type)
{

}

//======================================================================================================================

DatabaseWorkerThread::~DatabaseWorkerThread(void)
{

}

//======================================================================================================================

void DatabaseWorkerThread::Startup(char* host, uint16 port, char* user, char* pass, char* schema) 
{
  mPort = port;
  strcpy(mHostname, host);
  strcpy(mUsername, user);
  strcpy(mPassword, pass);
  strcpy(mSchema, schema);

  mExit = false;

  // start our thread
  mThread = new ZThread::Thread(new WorkerThreadRunnable(this));
  mThread->setPriority(ZThread::Medium);
}

//======================================================================================================================

void DatabaseWorkerThread::Shutdown(void)
{
	mExit = true;

	try
	{
		mThread->wait();
	}
	catch(ZThread::Synchronization_Exception& e)
	{
		std::cerr << e.what() << std::endl; 
	}

	// Shutdown our DBImplementation
	mDatabaseImplementation->Shutdown();
	delete(mDatabaseImplementation);
}

//======================================================================================================================

void DatabaseWorkerThread::_startup(void)
{
  // Create our DBImplementation object
  switch (mDatabaseImplementationType)
  {
	case DBTYPE_MYSQL:
		mDatabaseImplementation = reinterpret_cast<DatabaseImplementation*>(new DatabaseImplementationMySql());
    break;

	default:
		break;
  }

  // Init the new DBImplementation class.
  mDatabaseImplementation->Startup(mHostname, mPort, mUsername, mPassword, mSchema);

  mIsDone = false;
}

//======================================================================================================================

void DatabaseWorkerThread::_shutdown(void)
{
	mIsDone = true;
}

//======================================================================================================================

void DatabaseWorkerThread::run()
{
  // Call our internal _startup method
  _startup();

	  // Main loop
	  while(!mExit)
	  {
		// Is there a job waiting?
		 mWorkerThreadMutex.acquire();

		if(mCurrentJob)
		{
		  // Execute our query
		  DatabaseResult* result = mDatabaseImplementation->ExecuteSql(mCurrentJob->getSql(),mCurrentJob->isMultiJob());

		  // Attach the result to our job and send it back.
		  mCurrentJob->setDatabaseResult(result);

		  // put it on the complete list
		  mDatabase->pushDatabaseJobComplete(mCurrentJob);

		  // Put ourselves back on the idle list.
		  if(!result->isMultiResult())
		  {
			mDatabase->pushIdleWorker(this);
		  }
		  else
			result->setWorkerReference(this);

		  mCurrentJob = 0;
		} 

		mWorkerThreadMutex.release();

		// and always sleep a little.
		msleep(1);
	  }
	 
	 // internal shutdown method
	 _shutdown();
}

//======================================================================================================================







