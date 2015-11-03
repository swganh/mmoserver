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

#include "DatabaseManager/DatabaseImplementationMySql.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>


#ifdef ERROR
#undef ERROR
#endif

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "Utils/logger.h"

#include <mysql_connection.h>
#include <mysql_driver.h>

#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

#include "Utils/bstring.h"

#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"


DatabaseImplementationMySql::DatabaseImplementationMySql(
    const std::string& host, 
    uint16_t port, 
    const std::string& user, 
    const std::string& pass, 
    const std::string& schema)
{
    sql::Driver* driver = sql::mysql::get_driver_instance();
    
    sql::ConnectOptionsMap connection_options;
    connection_options["hostName"] = host;
    connection_options["userName"] = user;
    connection_options["password"] = pass;
    connection_options["schema"] = schema;

    connection_.reset(driver->connect(connection_options));

    connection_->getDriver()->threadInit();
}


DatabaseImplementationMySql::~DatabaseImplementationMySql() {
    connection_->getDriver()->threadEnd();
}


DatabaseResult* DatabaseImplementationMySql::executeSql(const std::string& sql, bool procedure) {
    DatabaseResult* result = nullptr;

    try {
        //DLOG(INFO) << sql;

        sql::Statement* statement = connection_->createStatement();    
        statement->execute(sql);
        
        sql::ResultSet* result_set = statement->getResultSet();
        result_set = result_set ? result_set : nullptr;

        result = new(ResultPool::ordered_malloc()) DatabaseResult(*this, statement, result_set, procedure);
    } catch(const sql::SQLException& e) {
        LOG(FATAL) << e.what();
    }

    return result;
}


void DatabaseImplementationMySql::destroyResult(DatabaseResult* result) {
    if (!result)
    {
        LOG(WARNING) << "DatabaseResult is NULL";
        return;
    }
    // For a multi-result statement to be destroyed properly all results must
    // be processed, failure to do so results in out-of-sync errors.
    if(result->isMultiResult()) {
        std::unique_ptr<sql::ResultSet> res;
        while (result->getStatement()->getMoreResults()) {
            res.reset(result->getStatement()->getResultSet());

            while(res->next()) {}
        }
    }

	//
	sql::Statement* statement = result->getStatement().get();
	result->getStatement().release();
	delete(statement);
	
	sql::ResultSet* res = result->getResultSet().get();
	result->getResultSet().release();
	delete(res);
	//

    ResultPool::ordered_free(result);
}


void DatabaseImplementationMySql::getNextRow(DatabaseResult* result, DataBinding* binding, void* object) const {
    std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

    if (! result_set) {
        return;
    }

    // Advance to the next row, if this fails check to see if this is a 
    // multi-result statement. If so attempt to retrieve more results.
    if (! result_set->next()) {
        if (result->isMultiResult()) {
            if (! result->getStatement()->getMoreResults()) {
                return;
            }

            result_set.reset(result->getStatement()->getResultSet());

            if (! result_set->next()) {
                return;
            }
        } else {
            return;
        }
    }
    
    for (uint32_t i = 0, field_count = binding->getFieldCount(); i < field_count; ++i) {
        processFieldBinding_(result_set, binding, i, object);
    }
}


void DatabaseImplementationMySql::resetRowIndex(DatabaseResult* result, uint64_t index) const {
    if(!result) {
        LOG(ERR) << "Bad Ptr 'DatabaseResult* result' at DatabaseImplementationMySql::ResetRowIndex.";
        return;
    }

    std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

    if (!result_set) {
        LOG(ERR) <<"Bad Ptr '(MYSQL_RES*)result->getResultSetReference()' at DatabaseImplementationMySql::ResetRowIndex.";
        return;
    }

    result_set->absolute(index);
}


uint32_t DatabaseImplementationMySql::escapeString(char* target, const char* source, uint32_t length) {
    if (!target) {
        LOG(ERR) << "Bad Ptr 'int8* target' at DatabaseImplementationMySql::Escape_String.";
        return 0;
    }

    if (!source) {
        LOG(ERR) << "Bad Ptr 'const int8* source' at DatabaseImplementationMySql::Escape_String.";
        return 0;
    }

    sql::mysql::MySQL_Connection* mysql_conn = dynamic_cast<sql::mysql::MySQL_Connection*>(connection_.get()); 
    std::string tmp = mysql_conn->escapeString(source);
        
    strncpy(target, tmp.c_str(), tmp.length());
    target[tmp.length()] = 0;

    return tmp.length();
}


std::string DatabaseImplementationMySql::escapeString(const std::string& source) {    
    sql::mysql::MySQL_Connection* mysql_conn = dynamic_cast<sql::mysql::MySQL_Connection*>(connection_.get()); 
    return mysql_conn->escapeString(source);
}


void DatabaseImplementationMySql::processFieldBinding_(
    std::unique_ptr<sql::ResultSet>& result, 
    DataBinding* binding, 
    uint32_t field_id,
    void* object) const
{
    // Mysql Connector/c++ starts it's field id's with 1 instead of 0 so create
    // a temporary variable that compensates for the offset.
    uint32_t result_field_id = binding->getField(field_id).column + 1;

    switch (binding->getField(field_id).type) {
        case DFT_int8: {
            *((char*)&((char*)object)[binding->getField(field_id).offset]) = result->getInt(result_field_id);
            break;
        }

        case DFT_uint8: {
            *((unsigned char*)&((char*)object)[binding->getField(field_id).offset]) = result->getUInt(result_field_id);
            break;
        }

        case DFT_int16: {
            *((short*)&((char*)object)[binding->getField(field_id).offset]) = result->getInt(result_field_id);
            break;
        }

        case DFT_uint16: {
            *((unsigned short*)&((char*)object)[binding->getField(field_id).offset]) = result->getUInt(result_field_id);
            break;
        }

        case DFT_int32: {
            *((int*)&((char*)object)[binding->getField(field_id).offset]) = result->getInt(result_field_id);
            break;
        }

        case DFT_uint32: {
            *((uint32_t*)&((char*)object)[binding->getField(field_id).offset]) = result->getUInt(result_field_id);
            break;
        }

        case DFT_int64: {
            *((long long*)&((char*)object)[binding->getField(field_id).offset]) = result->getInt64(result_field_id);
            break;
        }

        case DFT_uint64: {
            *((unsigned long long*)&((char*)object)[binding->getField(field_id).offset]) = result->getUInt64(result_field_id);
            break;
        }

        case DFT_float: {
            *((float*)&((char*)object)[binding->getField(field_id).offset]) = result->getDouble(result_field_id);
            break;
        }

        case DFT_double: {
            *((double*)&((char*)object)[binding->getField(field_id).offset]) = result->getDouble(result_field_id);;
            break;
        }

        case DFT_datetime: {
            break;
        }

        case DFT_string: {
            std::string tmp = result->getString(result_field_id);
            strncpy(&((char*)object)[binding->getField(field_id).offset], tmp.c_str(), tmp.length());
            ((char*)object)[binding->getField(field_id).offset + tmp.length()] = 0;
        
            break;
        }

        case DFT_bstring: {
            // get our string object
            BString* bindingString = reinterpret_cast<BString*>(((char*)object) + binding->getField(field_id).offset);
            // Now assign the string to the object
            std::string tmp = result->getString(result_field_id);
            *bindingString = tmp.c_str();
            break;
        }
                                  
        case DFT_raw: {
            std::string tmp = result->getString(result_field_id);
            strncpy(&((char*)object)[binding->getField(field_id).offset], tmp.c_str(), tmp.length());
            break;
        }

        default: { break; }
    }    
}

#ifdef _WIN32
#pragma warning(pop)
#endif
