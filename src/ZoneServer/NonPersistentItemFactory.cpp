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

#include "NonPersistentItemFactory.h"


#include "Utils/logger.h"

#include "Food.h"
#include "ItemFactory.h"
#include "ObjectFactoryCallback.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"

//=============================================================================

NonPersistentItemFactory* NonPersistentItemFactory::mSingleton  = NULL;

//======================================================================================================================

NonPersistentItemFactory* NonPersistentItemFactory::Instance(void)
{
    if (!mSingleton)
    {
        mSingleton = new NonPersistentItemFactory(WorldManager::getSingletonPtr()->getDatabase());
    }
    return mSingleton;
}


// This constructor prevents the default constructor to be used, as long as the constructor is kept private.

NonPersistentItemFactory::NonPersistentItemFactory() : FactoryBase(NULL)
{
}

//=============================================================================


NonPersistentItemFactory::NonPersistentItemFactory(Database* database) : FactoryBase(database)
{
    _setupDatabindings();
}


//=============================================================================

NonPersistentItemFactory::~NonPersistentItemFactory()
{
    _destroyDatabindings();
    mSingleton = NULL;
}

//=============================================================================

void NonPersistentItemFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    QueryNonPersistentItemFactory* asyncContainer = reinterpret_cast<QueryNonPersistentItemFactory*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case NPQuery_MainData:
    {
        Item* item = _createItem(result, asyncContainer->mId);

        // To be used when doing item factories.
        if (item->getLoadState() == LoadState_Attributes)
        {
            QueryNonPersistentItemFactory* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryNonPersistentItemFactory(asyncContainer->mOfCallback,NPQuery_Attributes,asyncContainer->mId);
            asContainer->mObject = item;

            mDatabase->executeSqlAsync(this,asContainer,"SELECT attributes.name,item_family_attribute_defaults.attribute_value,attributes.internal"
                                       " FROM %s.item_family_attribute_defaults"
                                       " INNER JOIN %s.attributes ON (item_family_attribute_defaults.attribute_id = attributes.id)"
                                       " WHERE item_family_attribute_defaults.item_type_id = %u ORDER BY item_family_attribute_defaults.attribute_order",
                                       mDatabase->galaxy(),mDatabase->galaxy(),item->getItemType());
           
        }
        else if (item->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
        {
            asyncContainer->mOfCallback->handleObjectReady(item);
        }
    }
    break;

    case NPQuery_Attributes:
    {
        _buildAttributeMap(asyncContainer->mObject,result);
        if ((asyncContainer->mObject->getLoadState() == LoadState_Loaded) && (asyncContainer->mOfCallback))
        {
            asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject);
        }
    }
    break;

    default:
        break;
    }
    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================
void NonPersistentItemFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    requestObject(ofCallback,id, 0);
}

//=============================================================================

void NonPersistentItemFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id, uint64 newId)
{
    mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryNonPersistentItemFactory(ofCallback,NPQuery_MainData,newId),
                               "SELECT item_family_attribute_defaults.family_id, item_family_attribute_defaults.item_type_id, item_types.object_string, item_types.stf_name, item_types.stf_file, item_types.stf_detail_file"
                               " FROM %.sitem_types"
                               " INNER JOIN %s.item_family_attribute_defaults ON (item_types.id = item_family_attribute_defaults.item_type_id AND item_family_attribute_defaults.attribute_id = 1)"
                               " WHERE (item_types.id = %" PRIu64 ")",mDatabase->galaxy(),mDatabase->galaxy(),id);
   
}

//=============================================================================

Item* NonPersistentItemFactory::_createItem(DatabaseResult* result, uint64 newId)
{
    if (!result->getRowCount()) {
    	return nullptr;
    }

    Item*			item;
    ItemIdentifier	itemIdentifier;

    result->getNextRow(mItemIdentifierBinding,(void*)&itemIdentifier);
    result->resetRowIndex();

    switch(itemIdentifier.mFamilyId)
    {
        // case ItemFamily_TravelTickets:			item	= new TravelTicket();				break;
        // case ItemFamily_SurveyTools:				item	= new SurveyTool();					break;
        // case ItemFamily_CraftingTools:			item	= new CraftingTool();				break;
        // case ItemFamily_CraftingStations:		item	= new CraftingStation();			break;
    case ItemFamily_Foods:
        item	= new Food();
        break;
        // case ItemFamily_Furniture:				item	= new Furniture();					break;
        // case ItemFamily_Wearable:				item	= new Wearable();					break;
        // case ItemFamily_ManufacturingSchematic:	item	= new ManufacturingSchematic();		break;
        // case ItemFamily_Instrument:
    case ItemFamily_Generic:
        item	= new Item();
        break;
        // case ItemFamily_Weapon:					item	= new Weapon();						break;

    default:
    {
        item = new Item();
    	LOG(ERR) << "Created item for unknown family [" << itemIdentifier.mFamilyId << "]";
    }
    break;
    }

    result->getNextRow(mItemBinding,item);

    item->setId(newId);
    item->setItemFamily(itemIdentifier.mFamilyId);
    item->setItemType(itemIdentifier.mTypeId);
    item->setLoadState(LoadState_Attributes);
    return item;
}

//=============================================================================

void NonPersistentItemFactory::_setupDatabindings()
{
    mItemBinding = mDatabase->createDataBinding(4);
    mItemBinding->addField(DFT_bstring,offsetof(Item,mModel),256,2);
    mItemBinding->addField(DFT_bstring,offsetof(Item,mName),64,3);
    mItemBinding->addField(DFT_bstring,offsetof(Item,mNameFile),64,4);
    mItemBinding->addField(DFT_bstring,offsetof(Item,mDetailFile),64,5);

    mItemIdentifierBinding = mDatabase->createDataBinding(2);
    mItemIdentifierBinding->addField(DFT_uint32,offsetof(ItemIdentifier,mFamilyId),4,0);
    mItemIdentifierBinding->addField(DFT_uint32,offsetof(ItemIdentifier,mTypeId),4,1);

}

//=============================================================================

void NonPersistentItemFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mItemBinding);
    mDatabase->destroyDataBinding(mItemIdentifierBinding);
}

//=============================================================================

