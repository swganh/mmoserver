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

DatabaseWorkerThread::DatabaseWorkerThread(DBType type, Database* database, const std::string& host, uint16 port, const std::string& user, const std::string& pass, const std::string& schema) 
    : hostname_(host)
    , username_(user)
    , password_(pass)
    , schema_(schema)
    , database_(database)
    , database_impl_(0)
    , current_job_(nullptr)
    , db_type_(type)
    , port_(port)
    , exit_(false)
{
    // start our thread
    boost::thread t(std::bind(&DatabaseWorkerThread::run, this));
    thread_ = boost::move(t);
}

//======================================================================================================================

DatabaseWorkerThread::~DatabaseWorkerThread(void)
{
    exit_ = true;

    thread_.interrupt();
    thread_.join();

    // Shutdown our DBImplementation
    delete database_impl_;
}


void DatabaseWorkerThread::run()
{
    // Call our internal _startup method
    startup_();

    // Main loop
    while(! exit_)
    {
        // Is there a job waiting?
        if(current_job_)
        {
            boost::mutex::scoped_lock lk(mutex_);
            // Execute our query
            DatabaseResult* result = database_impl_->ExecuteSql(current_job_->query.c_str(), current_job_->multi_job);

            // Attach the result to our job and send it back.
            current_job_->result = result;

            // put it on the complete list
            database_->pushDatabaseJobComplete(current_job_);

            // Put ourselves back on the idle list.
            if(!result->isMultiResult())
            {
                database_->pushIdleWorker(this);
            }
            else
                result->setWorkerReference(this);

            current_job_ = nullptr;
        }

        // and always sleep a little.
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }

    // internal shutdown method
    shutdown_();
}


void DatabaseWorkerThread::requestExit() {
    exit_ = true;
}


void DatabaseWorkerThread::startup_()
{
    // Create our DBImplementation object
    switch (db_type_)
    {
    case DBTYPE_MYSQL:
        database_impl_ = reinterpret_cast<DatabaseImplementation*>(new DatabaseImplementationMySql(hostname_, port_, username_, password_, schema_));
        break;

    default:
        break;
    }

    is_done_ = false;
}


void DatabaseWorkerThread::shutdown_()
{
    is_done_ = true;
}
