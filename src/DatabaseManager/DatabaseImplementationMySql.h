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

#ifndef ANH_DATABASEMANAGER_DATABASEIMPLEMENTATIONMYSQL_H
#define ANH_DATABASEMANAGER_DATABASEIMPLEMENTATIONMYSQL_H

#include <cstdint>
#include <memory>
#include <string>

#include <boost/noncopyable.hpp>

#include "DatabaseManager/DatabaseImplementation.h"

namespace sql {
    class Connection;
    class ResultSet;
    class Statement;
}

class DataBinding;
class DatabaseResult;

class DatabaseImplementationMySql : public DatabaseImplementation , private boost::noncopyable {
public:
    DatabaseImplementationMySql(const std::string& host, uint16_t port, const std::string& user, const std::string& pass, const std::string& schema);
    ~DatabaseImplementationMySql();

    DatabaseResult* executeSql(const std::string& sql, bool procedure = false);
    void destroyResult(DatabaseResult* result);

    void getNextRow(DatabaseResult* result, DataBinding* binding, void* object) const;
    void resetRowIndex(DatabaseResult* result, uint64_t index = 0) const;

    uint32_t escapeString(char* target, const char* source, uint32_t length);
    
    std::string escapeString(const std::string& source);

private:
    void processFieldBinding_(std::unique_ptr<sql::ResultSet>& result, DataBinding* binding, uint32_t field_id, void* object) const;

    std::unique_ptr<sql::Connection> connection_;
    std::unique_ptr<sql::Statement> statement_;
};

#endif // ANH_DATABASEMANAGER_DATABASEIMPLEMENTATIONMYSQL_H
