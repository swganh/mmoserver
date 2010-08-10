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

#ifndef ANH_DATABASEMANAGER_DATABASERESULT_H
#define ANH_DATABASEMANAGER_DATABASERESULT_H

#include "Utils/typedefs.h"
#include "DatabaseManager/declspec.h"


//======================================================================================================================
class DatabaseImplementation;
class DataBinding;
class DatabaseWorkerThread;


//======================================================================================================================
class DBMANAGER_API DatabaseResult
{
public:
                              DatabaseResult(bool multiResult = false) 
                                  :mWorkerReference(0), mConnectionReference(0),mResultSetReference(0),mRowCount(0),mDatabaseImplementation(0),mMultiResult(multiResult) {};
                              ~DatabaseResult(void) {};

  virtual void               GetNextRow(DataBinding* dataBinding, void* object);
  void                        ResetRowIndex(int index = 0);

  void*						  getConnectionReference(void){ return mConnectionReference; }
  void						  setConnectionReference(void* ref)	{ mConnectionReference =  ref; }

  void						  setWorkerReference(DatabaseWorkerThread* worker){ mWorkerReference = worker; }
  DatabaseWorkerThread*		  getWorkerReference(){ return mWorkerReference; }

  bool						  isMultiResult(){ return mMultiResult; }
  void						  setMultiResult(bool b){ mMultiResult = b; }

  DatabaseImplementation*     getDatabaseImplementation(void)                 { return mDatabaseImplementation; }
  void*                       getResultSetReference(void)                     { return mResultSetReference; }
  uint64                      getRowCount(void)                               { return mRowCount; }

  void                        setDatabaseImplementation(DatabaseImplementation* impl)   { mDatabaseImplementation = impl; }
  void                        setResultSetReference(void* ref)                { mResultSetReference = ref; }
  void                        setRowCount(uint64 count)                       { mRowCount = count; }

private:
  DatabaseWorkerThread*			mWorkerReference;
  void*							mConnectionReference;
  void*							mResultSetReference;
  uint64						mRowCount;
  DatabaseImplementation*		mDatabaseImplementation;
  bool							mMultiResult;
};


#endif //MMOSERVER_DATABASEMANAGER_DATABASERESULT_H



