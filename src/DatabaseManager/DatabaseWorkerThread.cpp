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
#include "DatabaseWorkerThread.h"

#include "Database.h"
#include "DatabaseImplementation.h"
#include "DatabaseImplementationMySql.h"
#include "DatabaseJob.h"
#include "DatabaseType.h"

#include <boost/thread/thread.hpp>

#if defined(__GNUC__)
// GCC implements tr1 in the <tr1/*> headers. This does not conform to the TR1
// spec, which requires the header without the tr1/ prefix.
#include <tr1/functional>
#else
#include <functional>
#endif

//======================================================================================================================

DatabaseWorkerThread::DatabaseWorkerThread(DBType type, Database* database, char* host, uint16 port, char* user, char* pass, char* schema) :
    mDatabase(database),
    mDatabaseImplementation(0),
    mCurrentJob(0),
    mDatabaseImplementationType(type)
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

DatabaseWorkerThread::~DatabaseWorkerThread(void)
{
    mExit = true;

    mThread.interrupt();
    mThread.join();

    // Shutdown our DBImplementation
    delete mDatabaseImplementation;
}

//======================================================================================================================

void DatabaseWorkerThread::_startup(void)
{
    // Create our DBImplementation object
    switch (mDatabaseImplementationType)
    {
    case DBTYPE_MYSQL:
        mDatabaseImplementation = reinterpret_cast<DatabaseImplementation*>(new DatabaseImplementationMySql(mHostname, mPort, mUsername, mPassword, mSchema));
        break;

    default:
        break;
    }

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
            DatabaseResult* result = mDatabaseImplementation->ExecuteSql(mCurrentJob->query.c_str(),mCurrentJob->multi_job);

            // Attach the result to our job and send it back.
            mCurrentJob->result = result;

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







