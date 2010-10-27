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

#include <cstdint>
#include <memory>

#include "DatabaseManager/declspec.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif


namespace sql {
    class ResultSet;
    class Statement;
}


class DatabaseImplementation;
class DataBinding;
class DatabaseWorkerThread;


class DBMANAGER_API DatabaseResult {
public:
    DatabaseResult(const DatabaseImplementation& impl, 
                   sql::Statement* statement, 
                   sql::ResultSet* result_set, 
                   bool multi_result);
    ~DatabaseResult();
    
    std::unique_ptr<sql::Statement>& getStatement();
    std::unique_ptr<sql::ResultSet>& getResultSet();

    void getNextRow(DataBinding* dataBinding, void* object);
    void resetRowIndex(int index = 0);
    
    void setWorkerReference(DatabaseWorkerThread* worker);
    DatabaseWorkerThread* getWorkerReference();

    bool isMultiResult();
    uint64_t getRowCount();

private:
    std::unique_ptr<sql::ResultSet> result_set_;
    std::unique_ptr<sql::Statement> statement_;

    const DatabaseImplementation& impl_;

    DatabaseWorkerThread* worker_;
    bool multi_result_;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif //MMOSERVER_DATABASEMANAGER_DATABASERESULT_H
