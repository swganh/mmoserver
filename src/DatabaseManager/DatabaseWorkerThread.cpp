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


DatabaseWorkerThread::DatabaseWorkerThread(DBType type, Database* database, const std::string& host, uint16 port, const std::string& user, const std::string& pass, const std::string& schema) 
    : database_(database)
    , database_impl_(nullptr)
{
    switch (type) {
        case DBTYPE_MYSQL:
            database_impl_.reset(new DatabaseImplementationMySql(host, port, user, pass, schema));
            break;

        default:
            break;
    }
}


DatabaseWorkerThread::~DatabaseWorkerThread() {}


void DatabaseWorkerThread::executeJob(DatabaseJob* job) { 
    active_.Send([=] {
        job->result = database_impl_->ExecuteSql(job->query.c_str(), job->multi_job);

        database_->pushDatabaseJobComplete(job);

        if (! job->result->isMultiResult()) {
            database_->pushIdleWorker(this);
        } else {
            job->result->setWorkerReference(this);
        }
    }); 
}
