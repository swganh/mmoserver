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

#ifndef ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H
#define ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include <boost/noncopyable.hpp>

#include "Utils/ActiveObject.h"

#include "DatabaseManager/DatabaseType.h"

struct DatabaseJob;
class DatabaseImplementation;


/*! \brief An encapsulation of a thread dedicated to executing an sql query 
* asynchronusly. 
*/
class DatabaseWorkerThread : private boost::noncopyable {
public:
    typedef std::function<void (DatabaseWorkerThread*, DatabaseJob*)> Callback;

public:
    /*! Overloaded constructor takes in the managing Database instance and 
    * connection details for starting a new connection.
    * 
    * \param type The type of database to connect to.
    * \param host The hostname to connect to the database on.
    * \param post The port to connect to the database on.
    * \param user The username to connect to the database with.
    * \param pass The password to connect to the database with.
    * \param schema The schema to connect to to perform queries on.
    */
    DatabaseWorkerThread(DBType type, 
                         const std::string& host, 
                         uint16_t port, 
                         const std::string& user, 
                         const std::string& pass, 
                         const std::string& schema);

    /*! Executes a DatabaseJob asynchronusly on the worker's private thread.
    *
    * \param job The database job to execute.
    * \param callback The callback to invoke once execution of the job 
    *   has completed.
    */
    void executeJob(DatabaseJob* job, Callback callback);

private:
    // Disable default construction.
    DatabaseWorkerThread();

    utils::ActiveObject active_;
    
    std::unique_ptr<DatabaseImplementation> database_impl_;
};

#endif // ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H
