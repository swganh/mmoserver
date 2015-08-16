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

#include "ItemFactory.h"


#include "Utils/logger.h"

#include "CraftingTool.h"
#include "CraftingStation.h"
#include "Deed.h"
#include "Firework.h"
#include "Food.h"
#include "Furniture.h"
#include "FactoryCrate.h"
#include "Instrument.h"
#include "Item.h"
#include "ItemTerminal.h"
#include "ManufacturingSchematic.h"
#include "Medicine.h"
#include "ObjectFactoryCallback.h"
#include "TangibleFactory.h"
#include "Scout.h"
#include "BugJar.h"
#include "SurveyTool.h"
#include "TravelTicket.h"
#include "Weapon.h"
#include "Wearable.h"
#include "ChanceCube.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "WorldConfig.h"

#include "Utils/utils.h"

#include <cassert>

//=============================================================================

bool			ItemFactory::mInsFlag    = false;
ItemFactory*	ItemFactory::mSingleton  = NULL;

//======================================================================================================================

ItemFactory*	ItemFactory::Init(Database* database)
{
    if(!mInsFlag)
    {
        mSingleton = new ItemFactory(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

ItemFactory::ItemFactory(Database* database) : FactoryBase(database)
{
    _setupDatabindings();
}

//=============================================================================

ItemFactory::~ItemFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void ItemFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case ItemFactoryQuery_MainData:
    {
        Item* item = _createItem(result);

        if(item->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
        {
            asyncContainer->mOfCallback->handleObjectReady(item,asyncContainer->mClient);
        }
        else if(item->getLoadState() == LoadState_Attributes)
        {
            QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,ItemFactoryQuery_Attributes,asyncContainer->mClient);
            asContainer->mObject = item;
            asContainer->mDepth = asyncContainer->mDepth;

            mDatabase->executeSqlAsync(this,asContainer,"SELECT attributes.name,item_attributes.value,attributes.internal"
                                       " FROM %s.item_attributes"
                                       " INNER JOIN %s.attributes ON (item_attributes.attribute_id = attributes.id)"
                                       " WHERE item_attributes.item_id = %" PRIu64 " ORDER BY item_attributes.order",
                                       mDatabase->galaxy(),mDatabase->galaxy(),item->getId());
               }
    }
    break;

    case ItemFactoryQuery_Attributes:
    {
        _buildAttributeMap(asyncContainer->mObject,result);
        _postProcessAttributes(asyncContainer->mObject);

        Item* item = dynamic_cast<Item*>(asyncContainer->mObject);

        // now check whether we are a container - if so check whether we contain items
        // make sure we dont iterate in loops! max iteration depth should probably not exceed 5
        uint16 ContainerDepth = gWorldConfig->getPlayerContainerDepth();

        if(item->getCapacity() && (asyncContainer->mDepth <= ContainerDepth))
        {
            item->setLoadState(LoadState_ContainerContent);
            // query contents
            QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,ItemFactoryQuery_Items,asyncContainer->mClient);
            asContainer->mObject = item;
            asContainer->mDepth = asyncContainer->mDepth;

            //containers are normal items like furniture, lightsabers and stuff
            mDatabase->executeSqlAsync(this,asContainer,
                                       "(SELECT \'items\',items.id FROM %s.items WHERE (parent_id=%" PRIu64 "))"
                                       " UNION (SELECT \'resource_containers\',resource_containers.id FROM %s.resource_containers WHERE (parent_id=%" PRIu64 "))",
                                       mDatabase->galaxy(),item->getId(),
                                       mDatabase->galaxy(),item->getId());
            

        }

        //

        if(asyncContainer->mObject->getLoadState() == LoadState_Loaded)
        {
            asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject,asyncContainer->mClient);
        }
    }
    break;

    case ItemFactoryQuery_Items:
    {
        uint64		count = result->getRowCount();

        Item* item = dynamic_cast<Item*>(asyncContainer->mObject);

        item->setLoadCount((uint32)count);

        Type1_QueryContainer queryContainer;

        DataBinding*	binding = mDatabase->createDataBinding(2);
        binding->addField(DFT_bstring,offsetof(Type1_QueryContainer,mString),64,0);
        binding->addField(DFT_uint64,offsetof(Type1_QueryContainer,mId),8,1);


        // nothing to load ? handle our callback immediately
        if(!count)
        {
            item->setLoadState(LoadState_Loaded);
            asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject,asyncContainer->mClient);
            return;
        }

        //otherwise enter us on the loadmap for future reference
        mObjectLoadMap.insert(std::make_pair(item->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(item,asyncContainer->mOfCallback,asyncContainer->mClient,static_cast<uint32>(count))));

        // request all children
        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&queryContainer);

            if(strcmp(queryContainer.mString.getAnsi(),"items") == 0)
            {
                // increase our iteration depth
                requestContainerContent(this,queryContainer.mId, 0, 0, asyncContainer->mClient, asyncContainer->mDepth+1);
            }
            else if(strcmp(queryContainer.mString.getAnsi(),"resource_containers") == 0)
            {
                // no need to worry about iteration depth with resourceContainers
                gTangibleFactory->requestObject(this,queryContainer.mId,TanGroup_ResourceContainer, 0, asyncContainer->mClient);
            }
        }
    }
    break;
    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void ItemFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,ItemFactoryQuery_MainData,client,id);
    asContainer->mDepth = 0;

    mDatabase->executeSqlAsync(this,asContainer,
                               "SELECT items.id,items.parent_id,items.item_family,items.item_type,items.privateowner_id,items.oX,items.oY,"
                               "items.oZ,items.oW,items.x,items.y,items.z,items.planet_id,items.customName,"
                               "item_types.object_string,item_types.stf_name,item_types.stf_file,item_types.stf_detail_name,"
                               "item_types.stf_detail_file,items.maxCondition,items.damage,items.dynamicint32,"
                               "item_types.equipSlots,item_types.equipRestrictions, item_customization.1, item_customization.2, item_types.container "
                               "FROM %s.items "
                               "INNER JOIN %s.item_types ON (items.item_type = item_types.id) "
                               "LEFT JOIN %s.item_customization ON (items.id = item_customization.id)"
                               "WHERE items.id = %" PRIu64 "",mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),id);
   
}

//=============================================================================

void ItemFactory::requestContainerContent(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client, uint32 depth)
{
    QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,ItemFactoryQuery_MainData,client,id);
    asContainer->mDepth = depth;

    mDatabase->executeSqlAsync(this,asContainer,
                               "SELECT items.id,items.parent_id,items.item_family,items.item_type,items.privateowner_id,items.oX,items.oY,"
                               "items.oZ,items.oW,items.x,items.y,items.z,items.planet_id,items.customName,"
                               "item_types.object_string,item_types.stf_name,item_types.stf_file,item_types.stf_detail_name,"
                               "item_types.stf_detail_file,items.maxCondition,items.damage,items.dynamicint32,"
                               "item_types.equipSlots,item_types.equipRestrictions, item_customization.1, item_customization.2, item_types.container "
                               "FROM %s.items "
                               "INNER JOIN %s.item_types ON (items.item_type = item_types.id) "
                               "LEFT JOIN %s.item_customization ON (items.id = item_customization.id)"
                               "WHERE items.id = %" PRIu64 "",mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),id);
  
}

//=============================================================================

Item* ItemFactory::_createItem(DatabaseResult* result)
{
    Item*			item;
    ItemIdentifier	itemIdentifier;

    result->getNextRow(mItemIdentifierBinding,(void*)&itemIdentifier);
    result->resetRowIndex();

    switch(itemIdentifier.mFamilyId)
    {
    case ItemFamily_TravelTickets:
        item	= new TravelTicket();
        break;
    case ItemFamily_SurveyTools:
        item	= new SurveyTool();
        break;
    case ItemFamily_CraftingTools:
        item	= new CraftingTool();
        break;
    case ItemFamily_CraftingStations:
        item	= new CraftingStation();
        break;
    case ItemFamily_Foods:
        item	= new Food();
        break;
    case ItemFamily_Furniture:
        item	= new Furniture();
        break;
    case ItemFamily_Wearable:
        item	= new Wearable();
        break;
    case ItemFamily_ManufacturingSchematic:
        item	= new ManufacturingSchematic();
        break;
    case ItemFamily_Instrument:
        item	= new Instrument();
        break;
    case ItemFamily_Generic:
        item	= new Item();
        break;
    case ItemFamily_Dice:
    {
        switch(itemIdentifier.mTypeId)
        {
        case ItemType_Generic_ChanceCube:
            item = new ChanceCube();
            break;
        default:
            item = new Item();
            break;
        }
    }
    break;

    case ItemFamily_Weapon:
        item	= new Weapon();
        break;
    case ItemFamily_Deed:
        item	= new Deed();
        break;
    case ItemFamily_Medicine:
        item	= new Medicine();
        break;
    case ItemFamily_Scout:
        item	= new Scout();
        break;
    case ItemFamily_FireWork:
    {
        switch(itemIdentifier.mTypeId)
        {
        case ItemType_Firework_Show:
            item	= new FireworkShow();
            break;
        default:
            item	= new Firework();
            break;
        }
    }
    break;
    case ItemFamily_FactoryCrate:
        item	= new FactoryCrate();
        break;
    case ItemFamily_Hopper:
        item	= new Item();
        break;
    case ItemFamily_BugJar:
        item	= new BugJar();
        break;
    case ItemFamily_Component:
        item	= new Item();
        break;
    case ItemFamily_Terminal:
        item  = new ItemTerminal();
        break;
    default:
    {
        item = new Item();
        LOG(INFO) << "ItemFactory::createItem (itemType:"<< itemIdentifier.mTypeId << ") with unknown Family " << itemIdentifier.mFamilyId;
    }
    break;
    }

    result->getNextRow(mItemBinding,item);

    item->setItemFamily(itemIdentifier.mFamilyId);
    item->setItemType(itemIdentifier.mTypeId);
    item->setLoadState(LoadState_Attributes);
    item->buildTanoCustomization(3); //x + 1

    return item;
}

//=============================================================================

void ItemFactory::_setupDatabindings()
{
    mItemBinding = mDatabase->createDataBinding(23);
    mItemBinding->addField(DFT_uint64,offsetof(Item,mId),8,0);
    mItemBinding->addField(DFT_uint64,offsetof(Item,mParentId),8,1);
    mItemBinding->addField(DFT_uint64,offsetof(Item,mPrivateOwner),8,4);
    mItemBinding->addField(DFT_float,offsetof(Item,mDirection.x),4,5);
    mItemBinding->addField(DFT_float,offsetof(Item,mDirection.y),4,6);
    mItemBinding->addField(DFT_float,offsetof(Item,mDirection.z),4,7);
    mItemBinding->addField(DFT_float,offsetof(Item,mDirection.w),4,8);
    mItemBinding->addField(DFT_float,offsetof(Item,mPosition.x),4,9);
    mItemBinding->addField(DFT_float,offsetof(Item,mPosition.y),4,10);
    mItemBinding->addField(DFT_float,offsetof(Item,mPosition.z),4,11);
    mItemBinding->addField(DFT_bstring,offsetof(Item,mCustomName),256,13);
    mItemBinding->addField(DFT_bstring,offsetof(Item,mModel),256,14);
    mItemBinding->addField(DFT_bstring,offsetof(Item,mName),64,15);
    mItemBinding->addField(DFT_bstring,offsetof(Item,mNameFile),64,16);
    mItemBinding->addField(DFT_bstring,offsetof(Item,mDetailFile),64,18);
    mItemBinding->addField(DFT_uint32,offsetof(Item,mMaxCondition),4,19);
    mItemBinding->addField(DFT_uint32,offsetof(Item,mDamage),4,20);
    mItemBinding->addField(DFT_uint32,offsetof(Item,mDynamicInt32),4,21);
    mItemBinding->addField(DFT_uint64,offsetof(Item,mEquipSlots),4,22);
    mItemBinding->addField(DFT_uint32,offsetof(Item,mEquipRestrictions),4,23);
    mItemBinding->addField(DFT_uint16,offsetof(Item,mCustomization[1]),2,24);
    mItemBinding->addField(DFT_uint16,offsetof(Item,mCustomization[2]),2,25);
    mItemBinding->addField(DFT_uint16,offsetof(Item,mCapacity),2,26);



    mItemIdentifierBinding = mDatabase->createDataBinding(2);
    mItemIdentifierBinding->addField(DFT_uint32,offsetof(ItemIdentifier,mFamilyId),4,2);
    mItemIdentifierBinding->addField(DFT_uint32,offsetof(ItemIdentifier,mTypeId),4,3);
}

//=============================================================================

void ItemFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mItemBinding);
    mDatabase->destroyDataBinding(mItemIdentifierBinding);
}

//=============================================================================

void ItemFactory::_postProcessAttributes(Object* object)
{
    if(Item* item = dynamic_cast<Item*>(object))
    {
        switch(item->getItemFamily())
        {
        case ItemFamily_Weapon:
        {
            if(Weapon* weapon = dynamic_cast<Weapon*>(item))
            {
                if(weapon->hasInternalAttribute("weapon_group"))
                {
                    weapon->setGroup(weapon->getInternalAttribute<uint32>("weapon_group"));
                }
                else
                {
                    weapon->addInternalAttribute("weapon_group","1");
                    weapon->setGroup(WeaponGroup_Unarmed);
                }
            }
        }
        break;

        case ItemFamily_CraftingTools:
        {
            if(item->hasAttribute("craft_tool_time"))
            {
                //read out the remaining time and start a new timer
                //int32 time = item->getInternalAttribute<int32>("craft_tool_time");
                //set 1 sec as the item is already in the inventory anyway after relog
                item->setTimer(1);

                //not without a player
                //we will do that in the inventory :)
                //gMessageLib->sendUpdateTimer(item,item->getOwner());

                gWorldManager->addBusyCraftTool((CraftingTool*)item);
            }
            else
            {
                item->setAttribute("craft_tool_status","@crafting:tool_status_ready");
                mDatabase->executeSqlAsync(0,0,"UPDATE %s.item_attributes SET value='@crafting:tool_status_ready' WHERE item_id=%" PRIu64 " AND attribute_id=%u",mDatabase->galaxy(),item->getId(),AttrType_CraftToolStatus);

                int8 sql[250];
                item->addAttribute("craft_tool_time","0");
                sprintf(sql,"INSERT INTO %s.item_attributes VALUES(%" PRIu64 ",%u,'0',0,0)",mDatabase->galaxy(),item->getId(),AttrType_CraftToolTime);
                mDatabase->executeAsyncSql(sql);
               
            }
        }
        break;
        default:
            break;
        }

        // Update status of insurance, if item support it.
        if (item->hasInternalAttribute("insured"))
        {
            uint32 insured = item->getInternalAttribute<uint32>("insured");
            if (insured)
            {
                item->setTypeOptions(item->getTypeOptions() | 4);
            }
        }
    }
}

//=============================================================================
//handles the ObjectReady callback of items children!

void ItemFactory::handleObjectReady(Object* object,DispatchClient* client)
{

    InLoadingContainer* ilc	= _getObject(object->getParentId());

    if (! ilc) {
        LOG(WARNING) << "Failed to locate InLoadingContainer for parent id [" << object->getParentId() << "]";
        LOG(WARNING) << "ItemFactory::handleObjectReady could not locate ILC for objectParentId: " << object->getParentId();
        return;
    }
    assert(ilc && "ItemFactory::handleObjectReady unable to find InLoadingContainer");

    Item*		item	= dynamic_cast<Item*>(ilc->mObject);
    // we can get factory crates, resource containers and other items at this point
    // when they are children of our containeritem

    ilc->mLoadCounter --;

    gWorldManager->addObject(object,true);
    item->addObjectSecure(object);

    if(!ilc->mLoadCounter)
    {
        item->setLoadState(LoadState_Loaded);
        ilc->mOfCallback->handleObjectReady(item,ilc->mClient);

        if(!(_removeFromObjectLoadMap(item->getId())))
            LOG(WARNING) << "Failed removing object from loadmap";

        mILCPool.free(ilc);
        return;
    }

}

