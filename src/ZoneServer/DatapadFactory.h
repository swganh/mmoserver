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

#ifndef ANH_ZONESERVER_DATAPAD_FACTORY_H
#define ANH_ZONESERVER_DATAPAD_FACTORY_H

#include "ObjectFactoryCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"

#define	 gDatapadFactory	DatapadFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class Datapad;
class DispatchClient;
class WaypointFactory;

//=============================================================================

enum DPFQuery
{
    DPFQuery_MainDatapadData		= 1,
    DPFQuery_Objects				= 2,
    DPFQuery_Data					= 3,
    DPFQuery_ManufactureSchematic	= 4,
    DPFQuery_ObjectCount			= 5,
    DPFQuery_ItemId					= 6,
    DPFQuery_Item					= 7,
    DPFQuery_MSParent				= 8
};

//=============================================================================

class DatapadFactory : public FactoryBase, public ObjectFactoryCallback
{
public:

    static DatapadFactory*	getSingletonPtr() {
        return mSingleton;
    }
    static DatapadFactory*	Init(Database* database);

    ~DatapadFactory();

    virtual void	handleObjectReady(Object* object,DispatchClient* client);
    void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
    void			requestManufacturingSchematic(ObjectFactoryCallback* ofCallback, uint64 id);

private:

    DatapadFactory(Database* database);

    void					_setupDatabindings();
    void					_destroyDatabindings();

    Datapad*				_createDatapad(DatabaseResult* result);

    static DatapadFactory*	mSingleton;
    static bool				mInsFlag;

    WaypointFactory*		mWaypointFactory;

    DataBinding*			mDatapadBinding;
    InLoadingContainer*		mIlc;
};

//=============================================================================


#endif

