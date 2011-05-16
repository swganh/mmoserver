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

#ifndef ANH_ZONESERVER_FACTORY_OBJECT_FACTORY_H
#define ANH_ZONESERVER_FACTORY_OBJECT_FACTORY_H

#include "ObjectFactoryCallback.h"
#include "FactoryBase.h"

#define gFactoryFactory FactoryFactory::getSingletonPtr()


//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class FactoryObject;

//=============================================================================

enum FFQuery
{
    FFQuery_MainData					= 1,
    FFQuery_ResourceData				= 2,
    FFQuery_byDeed						= 3,
    FFQuery_AdminData					= 4,
    FFQuery_AttributeData				= 5,
    FFQuery_Hopper						= 6,
    FFQuery_HopperUpdate				= 7,
    FFQuery_HopperItemAttributeUpdate	= 8

};

//=============================================================================

class FactoryFactory : public FactoryBase, public ObjectFactoryCallback
{
public:

    static FactoryFactory*	getSingletonPtr() {
        return mSingleton;
    }
    static FactoryFactory*	Init(Database* database);

    ~FactoryFactory();

    virtual void	handleObjectReady(Object* object,DispatchClient* client);
    void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
    void			upDateHopper(ObjectFactoryCallback* ofCallback,uint64 hopperId, DispatchClient* client, FactoryObject* factory);

    void			releaseAllPoolsMemory();

private:

    FactoryFactory(Database* database);

    void			_setupDatabindings();
    void			_destroyDatabindings();

    void			_createFactory(DatabaseResult* result, FactoryObject* factory);

    static FactoryFactory*		mSingleton;
    static bool					mInsFlag;


    DataBinding*				mFactoryBinding;

};

//=============================================================================


#endif


