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

#include "DatabaseResult.h"

#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "DatabaseImplementation.h"

DatabaseResult::DatabaseResult(const DatabaseImplementation& impl, sql::Statement* statement, sql::ResultSet* result_set, bool multi_result)
    : result_set_(result_set)
	, statement_(statement)
    , impl_(impl)
    , worker_(nullptr)
    , multi_result_(multi_result) {}


DatabaseResult::~DatabaseResult() {}


std::unique_ptr<sql::Statement>& DatabaseResult::getStatement() {
    return statement_;
}


std::unique_ptr<sql::ResultSet>& DatabaseResult::getResultSet() {
    return result_set_;
}


void DatabaseResult::getNextRow(DataBinding* dataBinding, void* object) {
    // Just shunt this method to the actual implementation method.  This might have thread problems right now.
    impl_.getNextRow(this, dataBinding, object);
}


void DatabaseResult::resetRowIndex(int index) {
    impl_.resetRowIndex(this,index);
}


void DatabaseResult::setWorkerReference(DatabaseWorkerThread* worker) {
    worker_ = worker;
}


DatabaseWorkerThread* DatabaseResult::getWorkerReference() {
    return worker_;
}


bool DatabaseResult::isMultiResult() {
    return multi_result_;
}


uint64_t DatabaseResult::getRowCount() { 
    return result_set_ ? result_set_->rowsCount() : 0; 
}
