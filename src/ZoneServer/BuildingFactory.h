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

#ifndef ANH_ZONESERVER_BUILDING_OBJECT_FACTORY_H
#define ANH_ZONESERVER_BUILDING_OBJECT_FACTORY_H

#include "ObjectFactoryCallback.h"
#include "FactoryBase.h"

#define 	gBuildingFactory	BuildingFactory::getSingletonPtr()


//=============================================================================

class BuildingObject;
class CellFactory;
class Database;
class DataBinding;
class DispatchClient;

//=============================================================================

enum BFQuery
{
    BFQuery_MainData	= 1,
    BFQuery_Cells		= 2,
    BFQuery_CloneData	= 3
};

//=============================================================================

class BuildingFactory : public FactoryBase, public ObjectFactoryCallback
{
public:

    static BuildingFactory*	getSingletonPtr() {
        return mSingleton;
    }
    static BuildingFactory*	Init(Database* database);

    ~BuildingFactory();

    virtual void	handleObjectReady(Object* object,DispatchClient* client);
    void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

    void			releaseAllPoolsMemory();

private:

    BuildingFactory(Database* database);

    void			_setupDatabindings();
    void			_destroyDatabindings();

    BuildingObject*	_createBuilding(DatabaseResult* result);

    static BuildingFactory*		mSingleton;
    static bool					mInsFlag;

    CellFactory*				mCellFactory;

    DataBinding*				mBuildingBinding;
    DataBinding*				mSpawnBinding;
};

//=============================================================================


#endif


