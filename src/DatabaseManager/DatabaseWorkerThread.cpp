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

#include <boost/thread/thread.hpp>


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
  boost::thread t(std::tr1::bind(&DatabaseWorkerThread::run, this));
  mThread = boost::move(t);
}

//======================================================================================================================

void DatabaseWorkerThread::Shutdown(void)
{
	mExit = true;

    mThread.interrupt();
    mThread.join();

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
		if(mCurrentJob)
		{
            boost::mutex::scoped_lock lk(mWorkerThreadMutex);
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

		// and always sleep a little.
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	  }
	 
	 // internal shutdown method
	 _shutdown();
}

//======================================================================================================================







