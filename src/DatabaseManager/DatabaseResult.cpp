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

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "DatabaseResult.h"
#include "DatabaseImplementation.h"

#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <mysql.h>
#include <stdlib.h>
#include <stdio.h>

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

#ifdef _WIN32
#pragma warning(pop)
#endif
