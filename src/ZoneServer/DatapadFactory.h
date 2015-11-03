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

