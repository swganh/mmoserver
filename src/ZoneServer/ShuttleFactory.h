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

#ifndef ANH_ZONESERVER_SHUTTLE_OBJECT_FACTORY_H
#define ANH_ZONESERVER_SHUTTLE_OBJECT_FACTORY_H

#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"

#define 	gShuttleFactory	ShuttleFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class Shuttle;

//=============================================================================

enum SHFQuery
{
    SHFQuery_MainData	= 1
};

//=============================================================================

class ShuttleFactory : public FactoryBase
{
public:

    static ShuttleFactory*	getSingletonPtr() {
        return mSingleton;
    }
    static ShuttleFactory*	Init(Database* database);

    ~ShuttleFactory();

    void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

private:

    ShuttleFactory(Database* database);

    void				_setupDatabindings();
    void				_destroyDatabindings();

    Shuttle*			_createShuttle(DatabaseResult* result);

    static ShuttleFactory*	mSingleton;
    static bool				mInsFlag;

    DataBinding*			mShuttleBinding;
};

//=============================================================================


#endif



