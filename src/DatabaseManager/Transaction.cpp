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
}


Transaction::~Transaction() {
    mQueries.flush();
}


void Transaction::addQuery(const char* query, ...) {
    va_list	args;
    va_start(args,query);
    va_list copy;
    va_copy(copy, args);

    // Sized to the query: fixed buffers overflowed (escaping can double it).
    int32_t	len = vsnprintf(nullptr, 0, query, copy);
    va_end(copy);
    std::string localSql(len > 0 ? len : 0, '\0');
    if (len > 0) {
        vsnprintf(&localSql[0], localSql.size() + 1, query, args);
    }
    va_end(args);

    // The queries are one value bound to sp_MultiTransaction: no escaping.
    mQueries << localSql << "$$";
}


void Transaction::execute() {
    // Bound, not formatted: a % in player text was read as a format
    // directive, and the text was cut at 20 KB.
    mDatabase->executePreparedAsync(mCallback, mReference,
        std::string("CALL ") + mDatabase->galaxy() + ".sp_MultiTransaction(?)", {mQueries.str()}, true);
    mDatabase->destroyTransaction(this);
}
