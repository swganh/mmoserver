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

#ifndef ANH_DATABASEMANAGER_DATABASEIMPLEMENTATION_H
#define ANH_DATABASEMANAGER_DATABASEIMPLEMENTATION_H

#include <cstdint>

#include <boost/pool/singleton_pool.hpp>

#include "DatabaseManager/DatabaseResult.h"

class DataBinding;

typedef boost::singleton_pool<DatabaseResult, 
    sizeof(DatabaseResult),
    boost::default_user_allocator_malloc_free> ResultPool;


/*! An implementation class that provides needed functionality for a specific
* rdbm type such as MySQL.
*/
class DatabaseImplementation {
public:
    virtual ~DatabaseImplementation() {}

    /*! Executes a query and returns a result set.
    * 
    * \param sql The sql query to execute.
    * \param procedure Indicates whether or not the query is a procedure and thus
    *   a multi-result statement.
    */
    virtual DatabaseResult* executeSql(const std::string& sql, bool procedure = false) = 0;

    /*! Destroys the requested database result.
    *
    * \param result The database result to destroy.
    */
    virtual void destroyResult(DatabaseResult* result) = 0;

    /*! Retrieves the next row and binds it to the specified object.
    *
    * \param result The result to retrieve the next row for.
    * \param data_binding The binding rules to be used when processing the row.
    * \param object The object to bind the next row too.
    */
    virtual void getNextRow(DatabaseResult* result, DataBinding* binding, void* object) const = 0;

    /*! Resets the row index to the specified value (defaults to 0).
    *
    * \param result The result to reset the index for.
    * \param index The index to reset the result set too.
    */
    virtual void resetRowIndex(DatabaseResult* result, uint64_t index = 0) const = 0;

    /*! Escapes a string to prepare for storage in a database.
    *
    * \param target The container to hold the escaped string.
    * \param source The original string that needs escaping.
    * \param length The length of the original string.
    * 
    * \return Returns the length of the escaped string.
    */
    virtual uint32_t escapeString(char* target, const char* source, uint32_t length) = 0;
    
    /*! Escapes a string to prepare for storage in a database.
    *
    * \param source The original string that needs escaping.
    * 
    * \return Returns the escaped string.
    */
    virtual std::string escapeString(const std::string& source) = 0;

    /*! Releases the memory allocated for result sets.
    *
    * \return Returns true if the pool was released, false if not.
    */
    bool releaseResultPoolMemory() {
        return(ResultPool::release_memory());
    }
};

#endif //^ANH_DATABASEMANAGER_DATABASEIMPLEMENTATION_H
