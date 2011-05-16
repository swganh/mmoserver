/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "DatabaseManager/DatabaseWorkerThread.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseImplementationMySql.h"
#include "DatabaseManager/DatabaseJob.h"


DatabaseWorkerThread::DatabaseWorkerThread(DBType type, const std::string& host, uint16_t port, const std::string& user, const std::string& pass, const std::string& schema)
    : database_impl_(nullptr)
{
    switch (type) {
        case DBTYPE_MYSQL:
            database_impl_.reset(new DatabaseImplementationMySql(host, port, user, pass, schema));
            break;
    }
}


void DatabaseWorkerThread::executeJob(DatabaseJob* job, Callback callback) { 
    active_.Send([=] {
        job->result = database_impl_->executeSql(job->query.c_str(), job->multi_job);
        callback(this, job);
    }); 
}
