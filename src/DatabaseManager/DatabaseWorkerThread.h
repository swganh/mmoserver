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

#ifndef ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H
#define ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H

#include <cstdint>

#include <string>

#include "DatabaseType.h"
#include "Utils/typedefs.h"
#include "DatabaseManager/declspec.h"
#include <boost/thread/thread.hpp>

//======================================================================================================================

class Database;
class DatabaseJob;
class DatabaseImplementation;

// Win32 complains about stl during linkage, disable the warning.
#ifdef _WIN32
#pragma warning (push)
#pragma warning (disable : 4251)
#endif

//======================================================================================================================
class DBMANAGER_API DatabaseWorkerThread
{
public:
    DatabaseWorkerThread(DBType type, 
                         Database* database, 
                         const std::string& host, 
                         uint16 port, const 
                         std::string& user, 
                         const std::string& pass, 
                         const std::string& schema);

    ~DatabaseWorkerThread();

    void run();

    void ExecuteJob(DatabaseJob* job);

    void requestExit();

private:
    void                        startup_();
    void                        shutdown_();
    
    boost::mutex                mutex_;
    boost::thread			    thread_;

    std::string                 hostname_;
    std::string                 username_;
    std::string                 password_;
    std::string                 schema_;

    Database*                   database_;
    DatabaseImplementation*     database_impl_;

    DatabaseJob*                current_job_;
    
    DBType                      db_type_;
    
    uint16_t                    port_;

    bool						is_done_;
    bool						exit_;
};

// Re-enable the warning.
#ifdef _WIN32
#pragma warning (pop)
#endif

//======================================================================================================================

inline void DatabaseWorkerThread::ExecuteJob(DatabaseJob* job)
{
    boost::mutex::scoped_lock lk(mutex_);
    current_job_ = job;
}

//======================================================================================================================

#endif // ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H
