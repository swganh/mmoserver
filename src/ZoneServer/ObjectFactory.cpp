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

#include "ObjectFactory.h"

#include <assert.h>

#include <cppconn/resultset.h>


#include "Utils/logger.h"

#include "Utils/utils.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include "MessageLib/MessageLib.h"

#include "BuildingFactory.h"
#include "CreatureFactory.h"
#include "Deed.h"
#include "Datapad.h"
#include "ContainerManager.h"
#include "CellObject.h"
#include "DraftSchematic.h"
#include "HarvesterFactory.h"
#include "HouseObject.h"
#include "HouseFactory.h"
#include "FactoryFactory.h"
#include "IntangibleObject.h"
#include "IntangibleFactory.h"
#include "ManufacturingSchematic.h"
#include "ObjectFactoryCallback.h"
#include "PlayerObject.h"
#include "Inventory.h"
#include "PlayerObjectFactory.h"
#include "RegionFactory.h"
#include "ResourceManager.h"
#include "StructureManager.h"
#include "TangibleFactory.h"
#include "TravelMapHandler.h"
#include "WaypointFactory.h"
#include "WorldManager.h"

using std::stringstream;

//=============================================================================

bool				ObjectFactory::mInsFlag    = false;
ObjectFactory*		ObjectFactory::mSingleton  = NULL;

//======================================================================================================================

ObjectFactory*	ObjectFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new ObjectFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

ObjectFactory::ObjectFactory(Database* database) :
    mDatabase(database),
    mDbAsyncPool(sizeof(OFAsyncContainer))
{
    mPlayerObjectFactory	= PlayerObjectFactory::Init(mDatabase);
    mTangibleFactory		= TangibleFactory::Init(mDatabase);
    mIntangibleFactory		= IntangibleFactory::Init(mDatabase);
    mCreatureFactory		= CreatureFactory::Init(mDatabase);
    mBuildingFactory		= BuildingFactory::Init(mDatabase);
    mRegionFactory			= new RegionFactory(mDatabase);
    mWaypointFactory		= WaypointFactory::Init(mDatabase);
    mHarvesterFactory		= HarvesterFactory::Init(mDatabase);
    mFactoryFactory			= FactoryFactory::Init(mDatabase);
    mHouseFactory			= HouseFactory::Init(mDatabase);
}

//=============================================================================

ObjectFactory::~ObjectFactory()
{
    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void ObjectFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{}

//=============================================================================
//
// create a new manufacture schematic with default values
//
void ObjectFactory::requestNewDefaultManufactureSchematic(ObjectFactoryCallback* ofCallback,uint32 schemCrc,uint64 parentId)
{
    stringstream query_stream;
    query_stream << "SELECT "<<mDatabase->galaxy() << ".sf_DefaultManufactureSchematicCreate(" 
                 << schemCrc << "," << parentId << ")";
    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Create item failed";
            return;
        }

        mTangibleFactory->requestObject(ofCallback,result_set->getUInt64(1),TanGroup_Item,0, 0);
    });
}

//=============================================================================
//
// create a new item based on a tangible template (clone it)
//
void ObjectFactory::requestNewClonedItem(ObjectFactoryCallback* ofCallback,uint64 templateId,uint64 parentId)
{
    stringstream query_stream;
    query_stream << "SELECT "<<mDatabase->galaxy() << ".sf_DefaultItemCreateByTangibleTemplate(" 
                 << parentId << "," << templateId << ")";
    
    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Create item failed";
            return;
        }

        mTangibleFactory->requestObject(ofCallback,result_set->getUInt64(1),TanGroup_Item,0, 0);
    });
}

//=============================================================================
//
// create a new item by schematic crc with default attributes
//
void ObjectFactory::requestNewDefaultItem(ObjectFactoryCallback* ofCallback, uint32 schemCrc, uint64 parentId, uint16 planetId, const glm::vec3& position, const BString& customName)
{
    BString newBStr(customName);
    newBStr.convert(BSTRType_ANSI);
    std::string name(mDatabase->escapeString(newBStr.getAnsi()));
    stringstream query_stream;
    query_stream << "SELECT "<<mDatabase->galaxy() << ".sf_DefaultItemCreateBySchematic(" 
                 << schemCrc << "," << parentId << "," << planetId << "," 
                 << position.x << "," << position.y << "," << position.z << ",'"
                 << name << "')";
    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Create item failed";
            return;
        }

        mTangibleFactory->requestObject(ofCallback,result_set->getUInt64(1),TanGroup_Item,0, 0);
    });
}

//=============================================================================
//
// create a new item with default attributes
//
void ObjectFactory::requestNewDefaultItem(ObjectFactoryCallback* ofCallback,uint32 familyId,uint32 typeId,uint64 parentId,uint16 planetId, const glm::vec3& position, const BString& customName)
{
    BString newBStr(customName);
    newBStr.convert(BSTRType_ANSI);
    std::string name(mDatabase->escapeString(newBStr.getAnsi()));
    stringstream query_stream;
    query_stream << "SELECT "<<mDatabase->galaxy() << ".sf_DefaultItemCreate(" 
                 << familyId << "," << typeId << "," << parentId << "," 
                 << (uint64) 0 << "," << planetId << "," << position.x << ","
                 << position.y << "," << position.z << ",'" << name << "')";

    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Create item failed";
            return;
        }

        mTangibleFactory->requestObject(ofCallback,result_set->getUInt64(1),TanGroup_Item,0, 0);
    });
}

//=============================================================================
//
// create a new item with default attributes
//
void ObjectFactory::requestNewDefaultItemWithUses(ObjectFactoryCallback* ofCallback,uint32 familyId,uint32 typeId,uint64 parentId,uint16 planetId, const glm::vec3& position, const BString& customName, int useCount)
{
    BString newBStr(customName);
    newBStr.convert(BSTRType_ANSI);
    std::string name(mDatabase->escapeString(newBStr.getAnsi()));
    stringstream query_stream;
    query_stream << "CALL "<<mDatabase->galaxy()<<".sp_CreateForagedItem(" 
                 << familyId << "," << typeId << "," << parentId << "," 
                 << (uint64) 0 << "," << planetId << "," << position.x << ","
                 << position.y << "," << position.z << ",'" << name 
                 << "'," << useCount <<")";




    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Create item failed";
            return;
        }
        mTangibleFactory->requestObject(ofCallback,result_set->getUInt64(1),TanGroup_Item,0, 0);
    });

}


//=============================================================================
//
// create a new travel ticket
//
void ObjectFactory::requestNewTravelTicket(ObjectFactoryCallback* ofCallback,TicketProperties ticketProperties,uint64 parentId,uint16 planetId)
{
    // make sure to escape strings to prevent
    std::string srcPlanet(gWorldManager->getPlanetNameById(static_cast<uint8>(ticketProperties.srcPlanetId)));
    std::string dstPlanet(gWorldManager->getPlanetNameById(static_cast<uint8>(ticketProperties.dstPlanetId)));
    srcPlanet = mDatabase->escapeString(srcPlanet);
    dstPlanet = mDatabase->escapeString(dstPlanet);

    stringstream query_stream;
    query_stream << "SELECT "<<mDatabase->galaxy() << ".sf_TravelTicketCreate("
                 << "'" << srcPlanet << "', '" << ticketProperties.srcPoint->descriptor << "', "
                 << "'" << dstPlanet << "', '" << ticketProperties.dstPoint->descriptor << "', "
                 << parentId << "," << 0.0f << "," << 0.0f << ","
                 << 0.0f << "," << planetId << ")";



    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Create item failed";
            return;
        }

        mTangibleFactory->requestObject(ofCallback,result_set->getUInt64(1),TanGroup_Item,0, 0);
    });
}

//=============================================================================
//
// create a new resource container
//
void ObjectFactory::requestNewResourceContainer(ObjectFactoryCallback* ofCallback,uint64 resourceId,uint64 parentId,uint16 planetId,uint32 amount)
{
    stringstream query_stream;
    query_stream << "SELECT "<<mDatabase->galaxy() << ".sf_ResourceContainerCreate("
                 << resourceId << "," << parentId << ","
                 << 0.0f << "," << 0.0f << "," << 0.0f << ","
                 << planetId << "," << amount << ")";
    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Create item failed";
            return;
        }

        mTangibleFactory->requestObject(ofCallback,result_set->getUInt64(1), TanGroup_ResourceContainer , 0, 0);
    });
}

//=============================================================================
//
// creates a harvester based on the supplied deed
//
void ObjectFactory::requestnewHarvesterbyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, BString customName, PlayerObject* player)
{
    //create a new Harvester Object with the attributes as specified by the deed

    StructureDeedLink* deedLink;
    deedLink = gStructureManager->getDeedData(deed->getItemType());

    float oX, oY, oZ, oW;
    if(dir== 0.0)
    {
        oX = 0;
        oY = 0;
        oZ = 0;
        oW = 1;
    }
    if(dir== 1.0)	 //ok
    {
        oX = 0.0;
        oY = static_cast<float>(0.71);
        oZ = 0.0;
        oW = static_cast<float>(0.71);
    }

    if(dir== 2.0)
    {
        oX = 0;
        oY = 1;
        oZ = 0;
        oW = 0;
    }

    if(dir== 3.0)
    {
        oX = 0;
        oY = static_cast<float>(-0.71);
        oZ = 0;
        oW = static_cast<float>(0.71);
    }
    BString newBStr(customName);
    newBStr.convert(BSTRType_ANSI);
    std::string name(mDatabase->escapeString(newBStr.getAnsi()));
    stringstream query_stream;
    query_stream << "SELECT "<<mDatabase->galaxy() << ".sf_DefaultHarvesterCreate("
                 << deedLink->structure_type << "," << 0 << ","
                 << player->getId() << "," << gWorldManager->getZoneId() << "," 
                 << oX << "," << oY << "," << oZ << "," << oW << "," << x 
                 << "," << y << "," << z << ",'" << name << "'," << deed->getId() << ")";
    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Unable to create new default harvester, no result" ;
            return;
        }

        uint64 requestId = result_set->getUInt64(1);
        mHarvesterFactory->requestObject(ofCallback,requestId,0,0,client);

        //now we need to update the Owners Lots

        //case he might have logged out already - even if thats *very* unlikely (heck of a query that would have been)
        if(player)
        {
            gStructureManager->UpdateCharacterLots(player->getId());
				
			Inventory* inventory = player->getInventory();				
			gContainerManager->removeObject(deed, inventory);
			Datapad* datapad	= player->getDataPad();

            glm::vec3 coords;
            coords.x = x;
            coords.y = y;
            coords.z = z;
            datapad->requestNewWaypoint("Harvester",coords, gWorldManager->getPlanetIdByName(gWorldManager->getPlanetNameThis()),1);
        }

        // now we need to link the deed to the harvester in the db and remove it out of the inventory in the db
        stringstream query_sql;
        query_sql << "UPDATE "<<mDatabase->galaxy()<<".items SET parent_id = " << requestId 
                  << " WHERE id = " << deed->getId();
        mDatabase->executeAsyncSql(query_sql);
    });
}

//=============================================================================
//
// creates a fatory based on the supplied deed
//
void ObjectFactory::requestnewFactorybyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, BString customName, PlayerObject* player)
{
    StructureDeedLink* deedLink;
    deedLink = gStructureManager->getDeedData(deed->getItemType());

    float oX, oY, oZ, oW;
    if(dir== 0.0)
    {
        oX = 0;
        oY = 0;
        oZ = 0;
        oW = 1;
    }
    if(dir== 1.0)	 //ok
    {
        oX = 0.0;
        oY = static_cast<float>(0.71);
        oZ = 0.0;
        oW = static_cast<float>(0.71);
    }

    if(dir== 2.0)
    {
        oX = 0;
        oY = 1;
        oZ = 0;
        oW = 0;
    }

    if(dir== 3.0)
    {
        oX = 0;
        oY = static_cast<float>(-0.71);
        oZ = 0;
        oW = static_cast<float>(0.71);
    }


    DLOG(INFO) << "New Factory dir is "<<dir<<","<<oX<<","<<oY<<","<<oZ<<","<<oW;

    BString newBStr(customName);
    newBStr.convert(BSTRType_ANSI);
    std::string name(mDatabase->escapeString(newBStr.getAnsi()));
    stringstream query_stream;
    query_stream << "SELECT "<<mDatabase->galaxy() << ".sf_DefaultFactoryCreate("
                 << deedLink->structure_type << "," << 0 << ","
                 << player->getId() << "," << gWorldManager->getZoneId() << "," 
                 << oX << "," << oY << "," << oZ << "," << oW << "," << x 
                 << "," << y << "," << z << ",'" << name << "'," << deed->getId() << ")";
    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
    if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Unable to create new default factory, no result" ;
            return;
        }

        uint64 requestId = result_set->getUInt64(1);
        mFactoryFactory->requestObject(ofCallback,requestId,0,0,client);

        //now we need to update the Owners Lots
        //case he might have logged out already - even if thats *very* unlikely (heck of a query that would have been)
        if(player)
        {            
            gStructureManager->UpdateCharacterLots(player->getId());
				
			Inventory* inventory = player->getInventory();				
			gContainerManager->removeObject(deed, inventory);
			Datapad* datapad	= player->getDataPad();

            glm::vec3 coords;
            coords.x = x;
            coords.y = y;
            coords.z = z;
            datapad->requestNewWaypoint("Player Factory",coords,gWorldManager->getPlanetIdByName(gWorldManager->getPlanetNameThis()),1);
        }

        // now we need to link the deed to the factory in the db and remove it out of the inventory in the db
        stringstream query_sql;
        query_sql << "UPDATE "<<mDatabase->galaxy()<<".items SET parent_id = " << requestId 
                  << " WHERE id = " << deed->getId();
        mDatabase->executeAsyncSql(query_sql);
    });
}

void ObjectFactory::requestnewHousebyDeed(ObjectFactoryCallback* ofCallback,Deed* deed,DispatchClient* client, float x, float y, float z, float dir, BString customName, PlayerObject* player)
{
    //create a new Harvester Object with the attributes as specified by the deed

    StructureDeedLink* deedLink;
    deedLink = gStructureManager->getDeedData(deed->getItemType());

    float oX, oY, oZ, oW;
    if(dir== 0.0)
    {
        oX = 0;
        oY = 0;
        oZ = 0;
        oW = 1;
    }
    if(dir== 1.0)	 //ok
    {
        oX = 0.0;
        oY = 0.707107f;
        oZ = 0.0;
        oW = 0.707107f;
    }

    if(dir== 2.0)
    {
        oX = 0;
        oY = 1;
        oZ = 0;
        oW = 0;
    }

    if(dir== 3.0)
    {
        oX = 0;
        oY = -0.707107f;
        oZ = 0;
        oW = 0.707107f;
    }


    DLOG(INFO) << "New House dir is "<<dir<<","<<oX<<","<<oY<<","<<oZ<<","<<oW;
    
    BString newBStr(customName);
    newBStr.convert(BSTRType_ANSI);
    std::string name(mDatabase->escapeString(newBStr.getAnsi()));
    stringstream query_stream;
    query_stream << "SELECT "<<mDatabase->galaxy() << ".sf_DefaultHouseCreate("
                 << deedLink->structure_type << "," << 0 << ","
                 << player->getId() << "," << gWorldManager->getZoneId() << "," 
                 << oX << "," << oY << "," << oZ << "," << oW << "," << x 
                 << "," << y << "," << z << ",'" << name << "'," << deed->getId() << ")";
    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (! client || !result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Unable to create new default house, no result" ;
            return;
        }

        uint64 requestId = result_set->getUInt64(1);

        if(!requestId)
        {
            LOG(ERR) << "Create house failed : result is 0";
        }

        mHouseFactory->requestObject(ofCallback,requestId,0,0,client);

        //now we need to update the Owners Lots
        //cave he might have logged out already - even if thats *very* unlikely (heck of a query that would have been)
        if(player)
        {
            gStructureManager->UpdateCharacterLots(player->getId());
				
			Inventory* inventory = player->getInventory();				
			gContainerManager->removeObject(deed, inventory);
			Datapad* datapad	= player->getDataPad();

            glm::vec3 coords;
            coords.x = x;
            coords.y = y;
            coords.z = z;

            datapad->requestNewWaypoint("Player House",coords,gWorldManager->getPlanetIdByName(gWorldManager->getPlanetNameThis()),1);
        }

        // now we need to link the deed to the factory in the db and remove it out of the inventory in the db
        stringstream query_sql;
        query_sql << "UPDATE "<<mDatabase->galaxy()<<".items SET parent_id = " << requestId 
                  << " WHERE id = " << deed->getId();
        mDatabase->executeAsyncSql(query_sql);
    });
}

//=============================================================================
//
// create a new waypoint
// never call this directly - always go over the datapad!!!!!  we need to check the capacity
//
void ObjectFactory::requestNewWaypoint(ObjectFactoryCallback* ofCallback,BString name, const glm::vec3& coords,uint16 planetId,uint64 ownerId,uint8 wpType)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(ownerId));
    BString newBStr(name);
    newBStr.convert(BSTRType_ANSI);
    std::string strName(mDatabase->escapeString(newBStr.getAnsi()));
    stringstream query_stream;
    query_stream << "SELECT "<<mDatabase->galaxy() << ".sf_WaypointCreate('" << strName << "',"
                 << ownerId << "," << coords.x << "," << coords.y << ","
                 << coords.z << "," << planetId << "," << (int)wpType << ")";
    mDatabase->executeAsyncProcedure(query_stream, [=](DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Unable to create waypoint ";
            return;
        }

        mWaypointFactory->requestObject(ofCallback, result_set->getUInt64(1) ,0,0, player->getClient());
    });
}
//=============================================================================
//
// update existing waypoint
// never call this directly - always go over the datapad!!!!!
//
void ObjectFactory::requestUpdatedWaypoint(ObjectFactoryCallback* ofCallback,uint64_t wpId,BString name, 
    const glm::vec3& coords,uint16_t planetId,uint64_t ownerId,uint16_t activeStatus)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(ownerId));
    name.convert(BSTRType_ANSI);
    std::string strName(mDatabase->escapeString(name.getAnsi()));
    stringstream query_stream;
    query_stream << "CALL "<<mDatabase->galaxy()<<".sp_WaypointUpdate('" << strName << "',"
                 << wpId << "," << coords.x << "," << coords.y << ","
                 << coords.z << "," << planetId << "," << activeStatus << ")";
    mDatabase->executeAsyncProcedure(query_stream, [=](DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Unable to update waypoint ";
            return;
        }
        if (result_set->getInt(1) != 0)
        {
            LOG(WARNING) << "Unable to update waypoint, sql failed with error: " << result_set->getInt(1);
        }
        else
        {
            mWaypointFactory->requestObject(ofCallback,wpId,0,0,player->getClient());
        }
    });
}
//=============================================================================

void ObjectFactory::requestTanoNewParent(ObjectFactoryCallback* ofCallback,uint64 ObjectId,uint64 parentId, TangibleGroup Group)
{
    //this is used to create an item after an auction and to load a Manufacturing schematic into the DataPad after it was removed from the factory
    //After the owner ID was changed the item is requested through the OFQuery_Default request
    stringstream query_stream;
    
    switch(Group)
    {
    case TanGroup_ManufacturingSchematic:
    case TanGroup_Item:
    {
        query_stream.str(std::string());
        query_stream << "UPDATE "<<mDatabase->galaxy()<<".items SET parent_id = " << parentId 
                     << " WHERE id = " << ObjectId;
    }
    break;

    case TanGroup_ResourceContainer:
    {
        query_stream.str(std::string());
        query_stream << "UPDATE "<<mDatabase->galaxy()<<".resource_containers SET parent_id = " << parentId 
                     << " WHERE id = " << ObjectId;
    }
    break;
    case TanGroup_Terminal:
    {
        query_stream.str(std::string());
        query_stream << "UPDATE "<<mDatabase->galaxy()<<".terminals SET parent_id = " << parentId 
                     << " WHERE id = " << ObjectId;
    }
    break;
    default:
    {
        LOG(WARNING) << "No Object could be created of type: " << Group;
        return;
        break;
    }
    }

    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (!result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next()) {
            LOG(WARNING) << "Create item failed";
            return;
        }
        mTangibleFactory->requestObject(ofCallback,result_set->getUInt64(1), Group, 0, 0);
    });
}

void ObjectFactory::createIteminInventory(ObjectFactoryCallback* ofCallback,uint64 ObjectId, TangibleGroup Group)
{
    mTangibleFactory->requestObject(ofCallback,ObjectId,Group,0,0);
}


//=============================================================================

void ObjectFactory::GiveNewOwnerInDB(Object* object, uint64 ID)
{
    stringstream query_stream;

    switch(object->getType())
    {
    case ObjType_Tangible:
    {
        TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);

        switch(tangibleObject->getTangibleGroup())
        {
        case TanGroup_Item:
        {
            query_stream << "UPDATE "<<mDatabase->galaxy()<<".items SET parent_id = " << ID 
                         << " WHERE id = " << object ->getId();
        }
        break;

        case TanGroup_ResourceContainer:
        {
            query_stream << "UPDATE "<<mDatabase->galaxy()<<".resource_containers SET parent_id = " << ID 
                         << " WHERE id = " << object ->getId();
        }
        break;

        default:
            break;
        }
    }
    break;

    case ObjType_Waypoint:
    {
        query_stream << "UPDATE "<<mDatabase->galaxy()<<".waypoints SET parent_id = " << ID
                     << " WHERE id = " << object->getId();
    }
    break;

    default:
        break;
    }
    mDatabase->executeAsyncSql(query_stream);

}

//=============================================================================

void ObjectFactory::deleteObjectFromDB(uint64 id)
{
    Object* object = dynamic_cast<Object*>(gWorldManager->getObjectById(id));
    deleteObjectFromDB(object );
}

void ObjectFactory::deleteObjectFromDB(Object* object)
{
    stringstream query_stream;

    switch(object->getType())
    {
    case ObjType_Tangible:
    {
        TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);

        switch(tangibleObject->getTangibleGroup())
        {
        case TanGroup_Item:
        {
            Item* item = dynamic_cast<Item*>(object);
            if(item->getItemType() == ItemFamily_ManufacturingSchematic)
            {
                ManufacturingSchematic* schem = dynamic_cast<ManufacturingSchematic*> (object);
                if (schem)
                {
                    //first associated item
                    query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".items WHERE id = " << schem->getItem()->getId();
                    mDatabase->executeAsyncSql(query_stream);

                    //query_stream.str(std::string());
                    //query_stream << "DELETE FROM item_attributes WHERE item_id =" <<  schem->getItem()->getId();
                    //mDatabase->executeAsyncSql(query_stream);

                }

            }

            ObjectIDList* objectList		= item->getObjects();
            ObjectIDList::iterator objIt	= objectList->begin();

            while(objIt != objectList->end())
            {
                Object* object = gWorldManager->getObjectById((*objIt));
                deleteObjectFromDB(object);

                ++objIt;
            }
            query_stream.str(std::string());
            query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".items WHERE id = " << object->getId();
            mDatabase->executeAsyncSql(query_stream);

          //  query_stream.str(std::string());
            //query_stream << "DELETE FROM item_attributes WHERE item_id =" <<  object->getId();
            //mDatabase->executeAsyncSql(query_stream);

        }
        break;

        case TanGroup_ResourceContainer:
        {
            query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".resource_containers WHERE id = " <<  object->getId();
            mDatabase->executeAsyncSql(query_stream);

        }
        break;

        case TanGroup_Terminal:
        {
            query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".terminals WHERE id =" <<  object->getId();
            mDatabase->executeAsyncSql(query_stream);
        }
        break;

        default:
            break;
        }
    }
    break;

    case ObjType_Intangible:
    {
        IntangibleObject* itno = dynamic_cast<IntangibleObject*>(object);

        switch(itno->getItnoGroup())
        {
        case ItnoGroup_Vehicle:
        {
            query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".vehicle_cutomization WHERE vehicles_id =" <<  object->getId();
            mDatabase->executeAsyncSql(query_stream);

            query_stream.str(std::string());
            query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".vehicle_attributes WHERE vehicles_id = " <<  object->getId();
            mDatabase->executeAsyncSql(query_stream);

            query_stream.str(std::string());
            query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".vehicles WHERE id = " <<  object->getId();
            mDatabase->executeAsyncSql(query_stream);

        }
        break;

        default:
            break;
        }
    }
    break;

    case ObjType_Cell:
    {
        CellObject* cell = dynamic_cast<CellObject*>(object);
        ObjectIDList* cellObjects		= cell->getObjects();
        ObjectIDList::iterator objIt	= cellObjects->begin();

        while(objIt != cellObjects->end())
        {
            Object* childObject = gWorldManager->getObjectById((*objIt));

            deleteObjectFromDB(childObject);

            ++objIt;

            query_stream.str(std::string());
            query_stream << "UPDATE "<<mDatabase->galaxy()<<".characters SET parent_id = 0 WHERE parent_id = " <<  object->getId();
            mDatabase->executeAsyncSql(query_stream);
        }
        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".cells WHERE id = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);
        
        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".structure_cells WHERE id = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);
    }
    break;

    case ObjType_Building:
    {
        //only delete when a playerbuilding
        HouseObject* house = dynamic_cast<HouseObject*>(object);
        if(!house)
        {
            //no player building
            return;
        }

        CellObjectList*				cellList	= house->getCellList();
        CellObjectList::iterator	cellIt		= cellList->begin();

        while(cellIt != cellList->end())
        {
            CellObject* cell = (*cellIt);
            deleteObjectFromDB(cell);
            //remove items in the building from world and db

            ++cellIt;
        }
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".houses WHERE ID = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);

        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".structures WHERE ID = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);

        //Admin / Hopper Lists
        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".structure_admin_data WHERE StructureID = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);

        //update attributes cave redeed vs destroy
        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".structure_attributes WHERE Structure_id = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);
    }
    break;
    case ObjType_Structure:
    {

        //Harvester
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".structures WHERE ID = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);
        
        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".harvesters WHERE ID = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);

        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".factories WHERE ID = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);

        //Admin / Hopper Lists
        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".structure_admin_data WHERE StructureID = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);

        //update attributes redeed vs destroy
        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".structure_attributes WHERE Structure_id = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);

        //update hopper contents
        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".harvester_resources WHERE ID = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);
    }
    break;

    case ObjType_Waypoint:
    {
        query_stream.str(std::string());
        query_stream << "DELETE FROM "<<mDatabase->galaxy()<<".waypoints WHERE waypoint_id = " <<  object->getId();
        mDatabase->executeAsyncSql(query_stream);
    }
    break;


    default:
        break;
    }
}

//=============================================================================

void ObjectFactory::requestObject(ObjectType objType,uint16 subGroup,uint16 subType,ObjectFactoryCallback* ofCallback,uint64 id,DispatchClient* client)
{
    switch(objType)
    {
    case ObjType_Player:
    {
        mPlayerObjectFactory->requestObject(ofCallback,id,subGroup,subType,client);
    }
    break;

    case ObjType_Tangible:
    {
        mTangibleFactory->requestObject(ofCallback,id,subGroup,subType,client);
    }
    break;
    case ObjType_NPC:
    case ObjType_Creature:
    {
        mCreatureFactory->requestObject(ofCallback,id,subGroup,subType,client);
    }
    break;

    case ObjType_Building:
    {
        mBuildingFactory->requestObject(ofCallback,id,subGroup,subType,client);
    }
    break;

    case ObjType_Region:
    {
        mRegionFactory->requestObject(ofCallback,id,subGroup,subType,client);
    }
    break;

    case ObjType_Waypoint:
    {
        mWaypointFactory->requestObject(ofCallback,id,subGroup,subType,client);
    }
    break;

    default:
        DLOG(INFO) << "ObjectFactory::requestObject Unknown Object type";
        break;
    }
}
void ObjectFactory::releaseAllPoolsMemory()
{
    mDbAsyncPool.release_memory();

    mPlayerObjectFactory->releaseAllPoolsMemory();
    mTangibleFactory->releaseAllPoolsMemory();
    mIntangibleFactory->releaseAllPoolsMemory();
    mCreatureFactory->releaseAllPoolsMemory();
    mBuildingFactory->releaseAllPoolsMemory();
    //mRegionFactory->releaseAllPoolsMemory();
    mWaypointFactory->releaseQueryContainerPoolMemory();
    mHarvesterFactory->releaseAllPoolsMemory();
    mFactoryFactory->releaseAllPoolsMemory();

}
