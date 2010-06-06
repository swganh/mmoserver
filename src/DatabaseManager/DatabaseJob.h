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

#ifndef ANH_DATABASEMANAGER_DATABASEJOB_H
#define ANH_DATABASEMANAGER_DATABASEJOB_H

#include <stdlib.h>
#include <cstring>

//======================================================================================================================
class DatabaseCallback;
class DatabaseResult;
class DataBinding;


//======================================================================================================================
class DatabaseJob
{
public:
	DatabaseJob() : mDatabaseCallback(NULL),mDatabaseResult(NULL),mClientReference(NULL),mMultiJob(false){}
  DatabaseCallback*           getCallback(void)                               { return mDatabaseCallback; }
  DatabaseResult*             getDatabaseResult(void)                         { return mDatabaseResult; };
  void*                       getClientReference(void)                        { return mClientReference; }
  int8*                       getSql(void)                                    { return mSql; }

  void                        setCallback(DatabaseCallback* callback)         { mDatabaseCallback = callback; }
  void                        setDatabaseResult(DatabaseResult* result)       { mDatabaseResult = result; }
  void                        setClientReference(void* ref)                   { mClientReference = ref; }
  void                        setSql(int8* sql)                               { strcpy(mSql, sql); }
  void						  setMultiJob(bool job){ mMultiJob = job; }
  bool						  isMultiJob(){ return mMultiJob; }

private:
  DatabaseCallback*           mDatabaseCallback;
  DatabaseResult*             mDatabaseResult;
  void*                       mClientReference;
  int8                        mSql[8192];
  bool						  mMultiJob;
};




#endif // ANH_DATABASEMANAGER_DATABASEJOB_H



