/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "PersistentNpcFactory.h"
#include "ZoneServer/PlayerEnums.h"
#include "FillerNPC.h"
#include "Zoneserver/Objects/Inventory.h"
#include "ZoneServer\Objects\Object\ObjectManager.h"
#include "ZoneServer/GameSystemManagers/NPC Manager/NPCObject.h"
#include "ZoneServer/Objects/Object/ObjectFactoryCallback.h"
#include "QuestGiver.h"
#include "Trainer.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"

#include "ZoneServer\Services\ham\ham_service.h"

#include <anh\app\swganh_kernel.h>
#include <anh\service/service_manager.h>


#include "ZoneServer\WorldManager.h"
//=============================================================================

bool					PersistentNpcFactory::mInsFlag    = false;
PersistentNpcFactory*	PersistentNpcFactory::mSingleton  = NULL;

//======================================================================================================================

PersistentNpcFactory*	PersistentNpcFactory::Init(swganh::app::SwganhKernel*	kernel)
{
    if(!mInsFlag)
    {
        mSingleton = new PersistentNpcFactory(kernel);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

PersistentNpcFactory::PersistentNpcFactory(swganh::app::SwganhKernel*	kernel) : FactoryBase(kernel)
{
    _setupDatabindings();
}

//=============================================================================

PersistentNpcFactory::~PersistentNpcFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    //delete(mSingleton);
}

//=============================================================================

void PersistentNpcFactory::handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case PersistentNpcQuery_MainData:
    {
		try	{
			NPCObject* npc = _createPersistentNpc(result);
			if(npc->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
				asyncContainer->mOfCallback->handleObjectReady(npc,asyncContainer->mClient);

			else if(npc->getLoadState() == LoadState_Attributes)
			{
				QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,PersistentNpcQuery_Attributes,asyncContainer->mClient);
				asContainer->mObject = npc;

				mDatabase->executeSqlAsync(this,asContainer,"SELECT attributes.name,persistent_npc_attributes.value,attributes.internal"
										   " FROM %s.persistent_npc_attributes"
										   " INNER JOIN %s.attributes ON (persistent_npc_attributes.attribute_id = attributes.id)"
										   " WHERE persistent_npc_attributes.npc_id = %"PRIu64" ORDER BY persistent_npc_attributes.order",
										   mDatabase->galaxy(),mDatabase->galaxy(),npc->getId());
			}
		}
		
		catch(std::exception const& e)	{		
			std::cout << "Exception: " << e.what() << "\n";
		}

        
    }
    break;

    case PersistentNpcQuery_Attributes:
    {
        _buildAttributeMap(asyncContainer->mObject,result);

        if(asyncContainer->mObject->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
            asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject,asyncContainer->mClient);
    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void PersistentNpcFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,PersistentNpcQuery_MainData,client),
                               "SELECT persistent_npcs.id,persistent_npcs.parentId,persistent_npcs.firstName,persistent_npcs.lastName,persistent_npcs.posture,persistent_npcs.state,persistent_npcs.cl,"
                               "persistent_npcs.oX,persistent_npcs.oY,persistent_npcs.oZ,persistent_npcs.oW,persistent_npcs.x,persistent_npcs.y,persistent_npcs.z,"
                               "persistent_npcs.type,persistent_npcs.stf_variable_id,persistent_npcs.stf_file_id,faction.name,"
                               "persistent_npcs.moodId,persistent_npcs.family,persistent_npcs.scale "
                               "FROM %s.persistent_npcs "
                               "INNER JOIN %s.faction ON (persistent_npcs.faction = faction.id) "
                               "WHERE (persistent_npcs.id = %"PRIu64")",
                               mDatabase->galaxy(),mDatabase->galaxy(),id);
    
}

//=============================================================================

NPCObject* PersistentNpcFactory::_createPersistentNpc(swganh::database::DatabaseResult* result)
{
    if (!result->getRowCount()) {
    	return nullptr;
    }

    NPCObject*		npc	;
    NpcIdentifier	npcIdentifier;

    result->getNextRow(mNpcIdentifierBinding,(void*)&npcIdentifier);
    result->resetRowIndex();

    switch(npcIdentifier.mFamilyId)
    {
    case NpcFamily_Trainer:
        npc	= new Trainer();
        break;
    case NpcFamily_Filler:
        npc	= new FillerNPC();
        break;
    case NpcFamily_QuestGiver:
        npc	= new QuestGiver();
        break;

    default:
    {
        npc = new NPCObject();
        DLOG(INFO) << "PersistentNpcFactory::createPersistent unknown Family " << npcIdentifier.mFamilyId;
    }
    break;
    }

	auto permissions_objects_ = gObjectManager->GetPermissionsMap();
	npc->SetPermissions(permissions_objects_.find(swganh::object::CREATURE_PERMISSION)->second.get());//CREATURE_PERMISSION

    Inventory*		npcInventory	= new Inventory();
    npcInventory->setParent(npc);
	npcInventory->SetPermissions(permissions_objects_.find(swganh::object::CREATURE_CONTAINER_PERMISSION)->second.get());//CREATURE_PERMISSION

    result->getNextRow(mPersistentNpcBinding,(void*)npc);

	std::string name = npc->getFirstName() + " " + npc->getLastName();

	npc->setCustomName(std::u16string(name.begin(), name.end()));
	

	auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");
	
	for(int8 i = 0; i<9;i++)	{
		npc->InitStatBase(500);
		npc->InitStatCurrent(500);
		npc->InitStatMax(500);
		npc->InitStatWound(0);
	}

	gObjectManager->LoadSlotsForObject(npc);

    // inventory
    npcInventory->setId(npc->mId + INVENTORY_OFFSET);
    npcInventory->setParentId(npc->mId);
    npcInventory->SetTemplate("object/tangible/inventory/shared_creature_inventory.iff");
    npcInventory->setName("inventory");
    npcInventory->setNameFile("item_n");
    npcInventory->setTangibleGroup(TanGroup_Inventory);
    npcInventory->setTangibleType(TanType_CreatureInventory);
    
	gObjectManager->LoadSlotsForObject(npcInventory);
	
	npc->InitializeObject(npcInventory);
	
	gWorldManager->addObject(npcInventory,true);

    npc->mTypeOptions = 0x108;
    //npc->setPvPStatus(npc->getPvPStatus() + CreaturePvPStatus_Attackable + CreaturePvPStatus_Aggressive);

    npc->setLoadState(LoadState_Attributes);

    // Save default direction, since player can make the npc change heading.
    npc->storeDefaultDirection();

	npc->object_type_ = SWG_CREATURE;

    return npc;
}

//=============================================================================

void PersistentNpcFactory::_setupDatabindings()
{
    mPersistentNpcBinding = mDatabase->createDataBinding(20);
    mPersistentNpcBinding->addField(swganh::database::DFT_uint64,offsetof(NPCObject,mId),8,0);
    mPersistentNpcBinding->addField(swganh::database::DFT_uint64,offsetof(NPCObject,mParentId),8,1);
	mPersistentNpcBinding->addField(swganh::database::DFT_stdstring,offsetof(NPCObject,first_name),64,2);
    mPersistentNpcBinding->addField(swganh::database::DFT_stdstring,offsetof(NPCObject,last_name),64,3);
    mPersistentNpcBinding->addField(swganh::database::DFT_uint8,offsetof(NPCObject,mPosture),1,4);
    mPersistentNpcBinding->addField(swganh::database::DFT_uint64,offsetof(NPCObject,mState),8,5);
    mPersistentNpcBinding->addField(swganh::database::DFT_uint16,offsetof(NPCObject,mCL),2,6);
    mPersistentNpcBinding->addField(swganh::database::DFT_float,offsetof(NPCObject,mDirection.x),4,7);
    mPersistentNpcBinding->addField(swganh::database::DFT_float,offsetof(NPCObject,mDirection.y),4,8);
    mPersistentNpcBinding->addField(swganh::database::DFT_float,offsetof(NPCObject,mDirection.z),4,9);
    mPersistentNpcBinding->addField(swganh::database::DFT_float,offsetof(NPCObject,mDirection.w),4,10);
    mPersistentNpcBinding->addField(swganh::database::DFT_float,offsetof(NPCObject,mPosition.x),4,11);
    mPersistentNpcBinding->addField(swganh::database::DFT_float,offsetof(NPCObject,mPosition.y),4,12);
    mPersistentNpcBinding->addField(swganh::database::DFT_float,offsetof(NPCObject,mPosition.z),4,13);
	mPersistentNpcBinding->addField(swganh::database::DFT_stdstring,offsetof(NPCObject,template_string_),256,14);
    mPersistentNpcBinding->addField(swganh::database::DFT_bstring,offsetof(NPCObject,mSpecies),64,15);
    mPersistentNpcBinding->addField(swganh::database::DFT_bstring,offsetof(NPCObject,mSpeciesGroup),64,16);
    mPersistentNpcBinding->addField(swganh::database::DFT_bstring,offsetof(NPCObject,mFaction),64,17);
    mPersistentNpcBinding->addField(swganh::database::DFT_uint8,offsetof(NPCObject,mMoodId),1,18);
    mPersistentNpcBinding->addField(swganh::database::DFT_float,offsetof(NPCObject,mScale),4,20);

    mNpcIdentifierBinding = mDatabase->createDataBinding(1);
    mNpcIdentifierBinding->addField(swganh::database::DFT_uint32,offsetof(NpcIdentifier,mFamilyId),4,19);
    //mNpcIdentifierBinding->addField(swganh::database::DFT_uint32,offsetof(NPCIdentifier,mTypeId),4,3);
}

//=============================================================================

void PersistentNpcFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mPersistentNpcBinding);
    mDatabase->destroyDataBinding(mNpcIdentifierBinding);
}

//=============================================================================

