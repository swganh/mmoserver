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

#ifndef ANH_ZONESERVER_OBJECT_FACTORY_H
#define ANH_ZONESERVER_OBJECT_FACTORY_H

#include "Object_Enums.h"
#include "TangibleEnums.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/bstring.h"
#include "Utils/typedefs.h"

#include <glm/glm.hpp>
#include <boost/pool/pool.hpp>

#define 	gObjectFactory	ObjectFactory::getSingletonPtr()

class BuildingFactory;
class CreatureFactory;
class Database;
class Deed;
class DispatchClient;
class DraftSchematic;
class HarvesterFactory;
class HouseFactory;
class FactoryFactory;
class IntangibleFactory;
class OFAsyncContainer;
class Object;
class ObjectFactoryCallback;
class PlayerObject;
class PlayerObjectFactory;
class RegionFactory;
class TangibleFactory;
class TangibleObject;
class TicketProperties;
class WaypointFactory;

//=============================================================================

enum OFQuery
{
    OFQuery_Default						= 0,
    OFQuery_ResourceContainerCreate		= 1,
    OFQuery_WaypointCreate				= 2,
    OFQuery_Item						= 3,
    OFQuery_Harvester					= 4,
    OFQuery_Factory						= 5,
    OFQuery_House						= 6,
    QFQuery_WaypointUpdate				= 7
};

//=============================================================================

class ObjectFactory : public DatabaseCallback
{
public:

    static ObjectFactory*	getSingletonPtr() {
        return mSingleton;
    }
    static ObjectFactory*	Init(Database* database);

    ~ObjectFactory();

    virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    void					requestObject(ObjectType objType,uint16 subGroup,uint16 subType,ObjectFactoryCallback* ofCallback,uint64 id,DispatchClient* client = 0);

    // create new objects in the database
    void					requestNewClonedItem(ObjectFactoryCallback* ofCallback,uint64 templateId,uint64 parentId);//creates a clone item after a tangible template - out of a crate for exampl
    void					requestNewDefaultItem(ObjectFactoryCallback* ofCallback,uint32 schemCrc,uint64 parentId,uint16 planetId, const glm::vec3& position, const BString& customName = "");
    void					requestNewDefaultItem(ObjectFactoryCallback* ofCallback,uint32 familyId,uint32 typeId,uint64 parentId,uint16 planetId, const glm::vec3& position, const BString& customName = "");
    void					requestNewDefaultManufactureSchematic(ObjectFactoryCallback* ofCallback,uint32 schemCrc,uint64 parentId);
    void					requestNewWaypoint(ObjectFactoryCallback* ofCallback,BString name, const glm::vec3& coords,uint16 planetId,uint64 ownerId,uint8 wpType);
    void					requestUpdatedWaypoint(ObjectFactoryCallback* ofCallback,uint64_t wpId,BString name, const glm::vec3& coords,uint16_t planetId,uint64_t ownerId,uint16_t activeStatus);
    void					requestNewTravelTicket(ObjectFactoryCallback* ofCallback,TicketProperties ticketProperties,uint64 parentId,uint16 planetId);
    void					requestNewResourceContainer(ObjectFactoryCallback* ofCallback,uint64 resourceId,uint64 parentId,uint16 planetId,uint32 amount);
    void					requestnewHarvesterbyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, BString customName, PlayerObject* player);
    void					requestnewFactorybyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, BString customName, PlayerObject* player);
    void					requestnewHousebyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, BString customName, PlayerObject* player);
    void					requestNewDefaultItemWithUses(ObjectFactoryCallback* ofCallback,uint32 familyId,uint32 typeId,uint64 parentId,uint16 planetId, const glm::vec3& position, const BString& customName, int useCount);

    void					requestTanoNewParent(ObjectFactoryCallback* ofCallback,uint64 ObjectId,uint64 parentId, TangibleGroup  Group);


    void					deleteObjectFromDB(Object* object);
    void					deleteObjectFromDB(uint64 id);
    void					GiveNewOwnerInDB(Object* object, uint64 ID);
    void					createIteminInventory(ObjectFactoryCallback* ofCallback,uint64 ObjectId, TangibleGroup  Group);

    void					releaseAllPoolsMemory();

private:

    ObjectFactory(Database* database);

    static ObjectFactory*	mSingleton;
    static bool			mInsFlag;

    Database*				mDatabase;

    PlayerObjectFactory*	mPlayerObjectFactory;
    TangibleFactory*		mTangibleFactory;
    IntangibleFactory*		mIntangibleFactory;
    CreatureFactory*		mCreatureFactory;
    BuildingFactory*		mBuildingFactory;
    RegionFactory*			mRegionFactory;
    WaypointFactory*		mWaypointFactory;
    HarvesterFactory*		mHarvesterFactory;
    FactoryFactory*			mFactoryFactory;
    HouseFactory*			mHouseFactory;

    boost::pool<boost::default_user_allocator_malloc_free>	mDbAsyncPool;
};

class OFAsyncContainer
{
public:

    OFAsyncContainer(ObjectFactoryCallback* of,OFQuery oq,DispatchClient* cl) {
        ofCallback = of;
        query = oq;
        client = cl;
    }

    DispatchClient*			client;
    ObjectFactoryCallback*	ofCallback;
    OFQuery					query;
    uint64					Id;
    TangibleGroup			Group;
    uint64					DeedId;
    uint64					OwnerId;
    uint64					PlayerId;

    glm::vec3		coords;
};


#endif
