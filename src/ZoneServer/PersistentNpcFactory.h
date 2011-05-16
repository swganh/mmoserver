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

#ifndef ANH_ZONESERVER_PERSISTENTNPC_FACTORY_H
#define ANH_ZONESERVER_PERSISTENTNPC_FACTORY_H

#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"
#include "NpcIdentifier.h"


#define	 gPersistentNPCFactory	PersistentNPCFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class NPCObject;

//=============================================================================

enum PersistentNpcQuery
{
    PersistentNpcQuery_MainData		= 1,
    PersistentNpcQuery_Attributes	= 2
};

//=============================================================================

class PersistentNpcFactory : public FactoryBase
{
public:

    static PersistentNpcFactory*	getSingletonPtr() {
        return mSingleton;
    }
    static PersistentNpcFactory*	Init(Database* database);

    ~PersistentNpcFactory();

    void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

private:

    PersistentNpcFactory(Database* database);

    void				_setupDatabindings();
    void				_destroyDatabindings();

    NPCObject*			_createPersistentNpc(DatabaseResult* result);

    static PersistentNpcFactory*	mSingleton;
    static bool						mInsFlag;

    DataBinding*					mPersistentNpcBinding;
    DataBinding*					mNpcIdentifierBinding;
};

//=============================================================================


#endif

