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

#include "DatabaseManager/Transaction.h"

#include <cstdarg>
#include <cstdint>

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "DatabaseManager/DatabaseImplementation.h"
#include "DatabaseManager/DatabaseImplementationMySql.h"


Transaction::Transaction(Database* database, DatabaseCallback* callback, void* ref)
    : mDatabase(database)
    , mCallback(callback)
    , mReference(ref)
{
    mQueries.flush();
    mQueries << "CALL "<< mDatabase->galaxy()<<".sp_MultiTransaction(\"";
}


Transaction::~Transaction() {
    mQueries.flush();
}


void Transaction::addQuery(const char* query, ...) {
    va_list	args;
    va_start(args,query);
    char localSql[2048], escapedSql[2500];
    int32_t	len = vsnprintf(localSql, sizeof(localSql), query, args);

    // need to escape
    mDatabase->escapeString(escapedSql, localSql, len);

    mQueries << escapedSql << "$$";

    va_end(args);
}


void Transaction::execute() {
    mQueries << "\")";

    mDatabase->executeProcedureAsync(mCallback, mReference, mQueries.str().c_str());
    mDatabase->destroyTransaction(this);
}
