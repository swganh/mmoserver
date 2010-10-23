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

#include "DatabaseResult.h"
#include "Utils/typedefs.h"
#include <boost/pool/singleton_pool.hpp>
#include "DatabaseManager/declspec.h"


//======================================================================================================================

class DataBinding;
class DatabaseWorkerThread;

typedef boost::singleton_pool<DatabaseResult,sizeof(DatabaseResult),boost::default_user_allocator_malloc_free> ResultPool;

//======================================================================================================================
class DBMANAGER_API DatabaseImplementation
{
public:
    virtual ~DatabaseImplementation() {}

    virtual DatabaseResult*			ExecuteSql(const int8* sql, bool procedure = false) = 0;

    virtual DatabaseWorkerThread*	DestroyResult(DatabaseResult* result) = 0;

    virtual void						GetNextRow(DatabaseResult* result, DataBinding* binding, void* object) = 0;
    virtual void						ResetRowIndex(DatabaseResult* result, uint64 index = 0) = 0;

    virtual uint32					Escape_String(int8* target,const int8* source,uint32 length) = 0;

    bool								releaseResultPoolMemory() {
        return(ResultPool::release_memory());
    }
    
protected:
};



#endif //^ANH_DATABASEMANAGER_DATABASEIMPLEMENTATION_H





