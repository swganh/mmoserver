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

#include "DatabaseType.h"
#include "Utils/typedefs.h"
#include "DatabaseManager/declspec.h"
#include <boost/thread/thread.hpp>

//======================================================================================================================

class Database;
class DatabaseJob;
class DatabaseImplementation;


//======================================================================================================================
class DBMANAGER_API DatabaseWorkerThread
{
public:
                              DatabaseWorkerThread(DBType type, Database* datbase, int8* host, uint16 port, int8* user, int8* pass, int8* schema);
                              ~DatabaseWorkerThread(void);

  virtual void				  run(); 

  void                        ExecuteJob(DatabaseJob* job);

  void						  requestExit(){ mExit = true; }

protected:
  int8                        mHostname[256];
  int16                       mPort;
  int8                        mUsername[64];
  int8                        mPassword[64];
  int8                        mSchema[64];
  
private:
  void                        _startup(void);
  void                        _shutdown(void);

  bool						  mIsDone;
  Database*                   mDatabase;
  DatabaseImplementation*     mDatabaseImplementation;

  DatabaseJob*                mCurrentJob;
  DBType                      mDatabaseImplementationType;  

    // Win32 complains about stl during linkage, disable the warning.
#ifdef _WIN32
#pragma warning (disable : 4251)
#endif
  boost::mutex              mWorkerThreadMutex;
  boost::thread			    mThread;
    // Re-enable the warning.
#ifdef _WIN32
#pragma warning (default : 4251)
#endif

  bool						  mExit;
};



//======================================================================================================================

inline void DatabaseWorkerThread::ExecuteJob(DatabaseJob* job)
{
    boost::mutex::scoped_lock lk(mWorkerThreadMutex);
    mCurrentJob = job;
}

//======================================================================================================================

#endif // ANH_DATABASEMANAGER_DATABASEWORKERTHREAD_H
