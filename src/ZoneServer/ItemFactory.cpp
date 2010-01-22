/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ItemFactory.h"

#include "CraftingTool.h"
#include "CraftingStation.h"
#include "Deed.h"
#include "Firework.h"
#include "Food.h"
#include "Furniture.h"
#include "Instrument.h"
#include "Item.h"
#include "ManufacturingSchematic.h"
#include "Medicine.h"
#include "ObjectFactoryCallback.h"
#include "Scout.h"
#include "SurveyTool.h"
#include "TravelTicket.h"
#include "Weapon.h"
#include "Wearable.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/utils.h"

#include <assert.h>

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
				asyncContainer->mOfCallback->handleObjectReady(item,asyncContainer->mClient);

			else if(item->getLoadState() == LoadState_Attributes)
			{
				QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,ItemFactoryQuery_Attributes,asyncContainer->mClient);
				asContainer->mObject = item;

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT attributes.name,item_attributes.value,attributes.internal"
															 " FROM item_attributes"
															 " INNER JOIN attributes ON (item_attributes.attribute_id = attributes.id)"
															 " WHERE item_attributes.item_id = %"PRIu64" ORDER BY item_attributes.order",item->getId());
			}
		}
		break;

		case ItemFactoryQuery_Attributes:
		{
			_buildAttributeMap(asyncContainer->mObject,result);
			_postProcessAttributes(asyncContainer->mObject);

			Item* item = dynamic_cast<Item*>(asyncContainer->mObject);
			
			// now check whether we are a container
			// if so check whether we contain items
			if(item->getCapacity())
			{
				item->setLoadState(LoadState_ContainerContent);
				// query contents				
				QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,ItemFactoryQuery_Items,asyncContainer->mClient);
				asContainer->mObject = item;

				//containers are normal items like furniture, lightsabers and stuff
				mDatabase->ExecuteSqlAsync(this,asContainer,
						"(SELECT \'items\',items.id FROM items WHERE (parent_id=%"PRIu64"))"
						" UNION (SELECT \'resource_containers\',resource_containers.id FROM resource_containers WHERE (parent_id=%"PRIu64"))",
						item->getId(),item->getId());
				
				return;
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

			DataBinding* mBinding = mDatabase->CreateDataBinding(1);
			mBinding->addField(DFT_uint64,0,8);

			// nothing to load ? handle our callback immediately
			if(!count)
			{
				item->setLoadState(LoadState_Loaded);
				asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject,asyncContainer->mClient);
				return;
			}

			//enter us on the loadmap for future reference
			mObjectLoadMap.insert(std::make_pair(item->getId(),new(mILCPool.ordered_malloc()) InLoadingContainer(item,asyncContainer->mOfCallback,asyncContainer->mClient,static_cast<uint32>(count))));

			uint64 itemId;

			// request all children
			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(mBinding,&itemId);
				requestObject(this,itemId, 0, 0, asyncContainer->mClient);
			}
		}
		break;
		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void ItemFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,ItemFactoryQuery_MainData,client,id),
													"SELECT items.id,items.parent_id,items.item_family,items.item_type,items.privateowner_id,items.oX,items.oY,"
													"items.oZ,items.oW,items.x,items.y,items.z,items.planet_id,items.customName,"
													"item_types.object_string,item_types.stf_name,item_types.stf_file,item_types.stf_detail_name,"
													"item_types.stf_detail_file,items.maxCondition,items.damage,items.dynamicint32,"
													"item_types.equipSlots,item_types.equipRestrictions, item_customization.1, item_customization.2, item_types.container "
													"FROM items "
													"INNER JOIN item_types ON (items.item_type = item_types.id) "
													"LEFT JOIN item_customization ON (items.id = item_customization.id)"
													"WHERE items.id = %"PRIu64"",id);
}

//=============================================================================

Item* ItemFactory::_createItem(DatabaseResult* result)
{
	Item*			item;
	ItemIdentifier	itemIdentifier;

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mItemIdentifierBinding,(void*)&itemIdentifier);
	result->ResetRowIndex();

	switch(itemIdentifier.mFamilyId)
	{
		case ItemFamily_TravelTickets:			item	= new TravelTicket();				break;
		case ItemFamily_SurveyTools:			item	= new SurveyTool();					break;
		case ItemFamily_CraftingTools:			item	= new CraftingTool();				break;
		case ItemFamily_CraftingStations:		item	= new CraftingStation();			break;
		case ItemFamily_Foods:					item	= new Food();						break;
		case ItemFamily_Furniture:				item	= new Furniture();					break;
		case ItemFamily_Wearable:				item	= new Wearable();					break;
		case ItemFamily_ManufacturingSchematic:	item	= new ManufacturingSchematic();		break;
		case ItemFamily_Instrument:				item	= new Instrument();					break;
		case ItemFamily_Generic:				item	= new Item();						break;
		case ItemFamily_Weapon:					item	= new Weapon();						break;
		case ItemFamily_Deed:					item	= new Deed();						break;
		case ItemFamily_Medicine:				item	= new Medicine();					break;
		case ItemFamily_Scout:					item	= new Scout();						break;
		case ItemFamily_FireWork:				item	= new Item();						break;
		case ItemFamily_Hopper:					item	= new Item();						break;
			{
				switch(itemIdentifier.mTypeId)
				{
					case ItemType_Firework_Show:	item	= new FireworkShow();				break;
					default:						item	= new Firework();					break;
				}
			}
			break;

		default:
		{
			item = new Item();
			gLogger->logMsgF("ItemFactory::createItem unknown Family %u",MSG_HIGH,itemIdentifier.mFamilyId);
		}
		break;
	}

	result->GetNextRow(mItemBinding,item);

	item->setItemFamily(itemIdentifier.mFamilyId);
	item->setItemType(itemIdentifier.mTypeId);
	item->setLoadState(LoadState_Attributes);
	item->buildTanoCustomization(3); //x + 1

	return item;
}

//=============================================================================

void ItemFactory::_setupDatabindings()
{
	mItemBinding = mDatabase->CreateDataBinding(23);
	mItemBinding->addField(DFT_uint64,offsetof(Item,mId),8,0);
	mItemBinding->addField(DFT_uint64,offsetof(Item,mParentId),8,1);
	mItemBinding->addField(DFT_uint64,offsetof(Item,mPrivateOwner),8,4);
	mItemBinding->addField(DFT_float,offsetof(Item,mDirection.mX),4,5);
	mItemBinding->addField(DFT_float,offsetof(Item,mDirection.mY),4,6);
	mItemBinding->addField(DFT_float,offsetof(Item,mDirection.mZ),4,7);
	mItemBinding->addField(DFT_float,offsetof(Item,mDirection.mW),4,8);
	mItemBinding->addField(DFT_float,offsetof(Item,mPosition.mX),4,9);
	mItemBinding->addField(DFT_float,offsetof(Item,mPosition.mY),4,10);
	mItemBinding->addField(DFT_float,offsetof(Item,mPosition.mZ),4,11);
	mItemBinding->addField(DFT_bstring,offsetof(Item,mCustomName),256,13);
	mItemBinding->addField(DFT_bstring,offsetof(Item,mModel),256,14);
	mItemBinding->addField(DFT_bstring,offsetof(Item,mName),64,15);
	mItemBinding->addField(DFT_bstring,offsetof(Item,mNameFile),64,16);
	mItemBinding->addField(DFT_bstring,offsetof(Item,mDetailFile),64,18);
	mItemBinding->addField(DFT_uint32,offsetof(Item,mMaxCondition),4,19);
	mItemBinding->addField(DFT_uint32,offsetof(Item,mDamage),4,20);
	mItemBinding->addField(DFT_uint32,offsetof(Item,mDynamicInt32),4,21);
	mItemBinding->addField(DFT_uint32,offsetof(Item,mEquipSlots),4,22);
	mItemBinding->addField(DFT_uint32,offsetof(Item,mEquipRestrictions),4,23);
	mItemBinding->addField(DFT_uint16,offsetof(Item,mCustomization[1]),2,24);
	mItemBinding->addField(DFT_uint16,offsetof(Item,mCustomization[2]),2,25);
	mItemBinding->addField(DFT_uint16,offsetof(Item,mCapacity),2,26);



	mItemIdentifierBinding = mDatabase->CreateDataBinding(2);
	mItemIdentifierBinding->addField(DFT_uint32,offsetof(ItemIdentifier,mFamilyId),4,2);
	mItemIdentifierBinding->addField(DFT_uint32,offsetof(ItemIdentifier,mTypeId),4,3);
}

//=============================================================================

void ItemFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mItemBinding);
	mDatabase->DestroyDataBinding(mItemIdentifierBinding);
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
					mDatabase->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='@crafting:tool_status_ready' WHERE item_id=%"PRIu64" AND attribute_id=%u",item->getId(),AttrType_CraftToolStatus);

					int8 sql[250];
					item->addAttribute("craft_tool_time","0");
					sprintf(sql,"INSERT INTO item_attributes VALUES(%"PRIu64",%u,'0',0,0)",item->getId(),AttrType_CraftToolTime);
					mDatabase->ExecuteSqlAsync(0,0,sql);
				}
			}
			break;




			default:break;
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
	Item*		item	= dynamic_cast<Item*>(ilc->mObject);

	assert(ilc);
	ilc->mLoadCounter --;

	gWorldManager->addObject(object,true);
	item->addData(object);

	if(!ilc->mLoadCounter)
	{
		item->setLoadState(LoadState_Loaded);
		ilc->mOfCallback->handleObjectReady(item,ilc->mClient);
		
		if(!(_removeFromObjectLoadMap(item->getId())))
			gLogger->logMsg("ItemFactory: Failed removing object from loadmap");

		mILCPool.free(ilc);
		return;
	}

}

