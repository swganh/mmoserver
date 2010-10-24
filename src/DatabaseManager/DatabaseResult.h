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

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <cstdint>
#include <memory>

#include "DatabaseManager/declspec.h"

namespace sql {
    class ResultSet;
    class Statement;
}

class DatabaseImplementation;
class DataBinding;
class DatabaseWorkerThread;


//======================================================================================================================
class DBMANAGER_API DatabaseResult
{
public:
    DatabaseResult(sql::Statement* statement, sql::ResultSet* result_set, bool multiResult);
    ~DatabaseResult();
    
    std::unique_ptr<sql::Statement>& getStatement();

    void setResultSet(std::unique_ptr<sql::ResultSet> result_set);

    std::unique_ptr<sql::ResultSet>& getResultSet();

    virtual void               GetNextRow(DataBinding* dataBinding, void* object);
    void                        ResetRowIndex(int index = 0);

    void*						  getConnectionReference(void) {
        return mConnectionReference;
    }
    void						  setConnectionReference(void* ref)	{
        mConnectionReference =  ref;
    }

    void						  setWorkerReference(DatabaseWorkerThread* worker) {
        mWorkerReference = worker;
    }
    DatabaseWorkerThread*		  getWorkerReference() {
        return mWorkerReference;
    }

    bool						  isMultiResult() {
        return mMultiResult;
    }
    void						  setMultiResult(bool b) {
        mMultiResult = b;
    }

    DatabaseImplementation*     getDatabaseImplementation()                 {
        return mDatabaseImplementation;
    }
    void*                       getResultSetReference()                     {
        return mResultSetReference;
    }
    uint64_t                      getRowCount();

    void                        setDatabaseImplementation(DatabaseImplementation* impl)   {
        mDatabaseImplementation = impl;
    }
    void                        setResultSetReference(void* ref)                {
        mResultSetReference = ref;
    }

private:
    std::unique_ptr<sql::ResultSet> result_set_;
    std::unique_ptr<sql::Statement> statement_;

    DatabaseWorkerThread*			mWorkerReference;
    void*							mConnectionReference;
    void*							mResultSetReference;
    DatabaseImplementation*		mDatabaseImplementation;
    bool							mMultiResult;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif //MMOSERVER_DATABASEMANAGER_DATABASERESULT_H
