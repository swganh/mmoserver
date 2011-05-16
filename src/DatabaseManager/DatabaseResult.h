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

#ifndef ANH_DATABASEMANAGER_DATABASERESULT_H
#define ANH_DATABASEMANAGER_DATABASERESULT_H

#include <cstdint>
#include <memory>

#include <boost/noncopyable.hpp>

namespace sql {
    class ResultSet;
    class Statement;
}

class Database;
class DatabaseImplementation;
class DataBinding;
class DatabaseWorkerThread;

/*! A container class for database results. 
*/
class DatabaseResult : private boost::noncopyable {
public:
    /*! Sets up the the database result after a query has been run.
    *
    * \param impl The database implementation/connection that executed the query.
    * \param statement The sql query/statement that was just executed.
    * \param result_set The result set provided by the underlying database abstraction library
    * \param multi_result Indicates whether the query was a multi-result query.
    */
    DatabaseResult(const DatabaseImplementation& impl, 
                   sql::Statement* statement, 
                   sql::ResultSet* result_set, 
                   bool multi_result);
    ~DatabaseResult();
    
    /*! Returns the statement was executed.
    */
    std::unique_ptr<sql::Statement>& getStatement();

    /*! Returns the result set from the underlying database abstraction library.
    */
    std::unique_ptr<sql::ResultSet>& getResultSet();

    /*! Retrieves the next row and binds it to the specified object.
    *
    * \param data_binding The binding rules to be used when processing the row.
    * \param object The object to bind the next row too.
    */
    void getNextRow(DataBinding* dataBinding, void* object);

    /*! Resets the row index to the specified value (defaults to 0).
    *
    * \param index The index to reset the result set too.
    */
    void resetRowIndex(int index = 0);
    
    /*! Returns whether or not this is the result of a multi-result statement (
    * such as using a CALL query to invoke a stored procedure).
    */
    bool isMultiResult();

    /*! Returns the number of rows returned by the query.
    */
    uint64_t getRowCount();

private:
    friend class Database;

    DatabaseResult();

    void setWorkerReference(DatabaseWorkerThread* worker);
    DatabaseWorkerThread* getWorkerReference();

    std::unique_ptr<sql::ResultSet> result_set_;
    std::unique_ptr<sql::Statement> statement_;

    const DatabaseImplementation& impl_;

    DatabaseWorkerThread* worker_;
    bool multi_result_;
};

#endif //MMOSERVER_DATABASEMANAGER_DATABASERESULT_H
