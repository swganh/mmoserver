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

#include "PersistentNpcFactory.h"
#include "PlayerEnums.h"
#include "FillerNPC.h"
#include "Inventory.h"
#include "NPCObject.h"
#include "ObjectFactoryCallback.h"
#include "QuestGiver.h"
#include "Trainer.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "LogManager/LogManager.h"
#include "Utils/utils.h"


//=============================================================================

bool					PersistentNpcFactory::mInsFlag    = false;
PersistentNpcFactory*	PersistentNpcFactory::mSingleton  = NULL;

//======================================================================================================================

PersistentNpcFactory*	PersistentNpcFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new PersistentNpcFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

PersistentNpcFactory::PersistentNpcFactory(Database* database) : FactoryBase(database)
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

void PersistentNpcFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case PersistentNpcQuery_MainData:
		{
			NPCObject* npc = _createPersistentNpc(result);

			if(npc->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
				asyncContainer->mOfCallback->handleObjectReady(npc,asyncContainer->mClient);

			else if(npc->getLoadState() == LoadState_Attributes)
			{
				QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,PersistentNpcQuery_Attributes,asyncContainer->mClient);
				asContainer->mObject = npc;

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT attributes.name,persistent_npc_attributes.value,attributes.internal"
					" FROM persistent_npc_attributes"
					" INNER JOIN attributes ON (persistent_npc_attributes.attribute_id = attributes.id)"
					" WHERE persistent_npc_attributes.npc_id = %"PRIu64" ORDER BY persistent_npc_attributes.order",npc->getId());
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

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void PersistentNpcFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,PersistentNpcQuery_MainData,client),
		"SELECT persistent_npcs.id,persistent_npcs.parentId,persistent_npcs.firstName,persistent_npcs.lastName,persistent_npcs.posture,persistent_npcs.state,persistent_npcs.cl,"
		"persistent_npcs.oX,persistent_npcs.oY,persistent_npcs.oZ,persistent_npcs.oW,persistent_npcs.x,persistent_npcs.y,persistent_npcs.z,"
		"persistent_npcs.type,persistent_npcs.stf_variable_id,persistent_npcs.stf_file_id,faction.name,"
		"persistent_npcs.moodId,persistent_npcs.family,persistent_npcs.scale "
		"FROM persistent_npcs "
		"INNER JOIN faction ON (persistent_npcs.faction = faction.id) "
		"WHERE (persistent_npcs.id = %"PRIu64")",id);
}

//=============================================================================

NPCObject* PersistentNpcFactory::_createPersistentNpc(DatabaseResult* result)
{
	NPCObject*		npc	;
	NpcIdentifier	npcIdentifier;
	

	uint64 count = result->getRowCount();

	result->GetNextRow(mNpcIdentifierBinding,(void*)&npcIdentifier);
	result->ResetRowIndex();

	switch(npcIdentifier.mFamilyId)
	{
		case NpcFamily_Trainer:		npc	= new Trainer();		break;
		case NpcFamily_Filler:		npc	= new FillerNPC();		break;
		case NpcFamily_QuestGiver:	npc	= new QuestGiver();		break;
		
		default:
		{
			npc = new NPCObject();
			gLogger->log(LogManager::DEBUG,"PersistentNpcFactory::createPersistent unknown Family %u",npcIdentifier.mFamilyId);
		}
		break;
	}

	Inventory*		npcInventory	= new Inventory();
	npcInventory->setParent(npc);

	result->GetNextRow(mPersistentNpcBinding,(void*)npc);

	npc->mHam.mHealth.setCurrentHitPoints(500);
	npc->mHam.mAction.setCurrentHitPoints(500);
	npc->mHam.mMind.setCurrentHitPoints(500);
	npc->mHam.calcAllModifiedHitPoints();

	// inventory
	npcInventory->setId(npc->mId + INVENTORY_OFFSET);
	npcInventory->setParentId(npc->mId);
	npcInventory->setModelString("object/tangible/inventory/shared_creature_inventory.iff");
	npcInventory->setName("inventory");
	npcInventory->setNameFile("item_n");
	npcInventory->setTangibleGroup(TanGroup_Inventory);
	npcInventory->setTangibleType(TanType_CreatureInventory);
	npc->mEquipManager.addEquippedObject(CreatureEquipSlot_Inventory,npcInventory);
	npc->mTypeOptions = 0x108;
	//npc->setPvPStatus(npc->getPvPStatus() + CreaturePvPStatus_Attackable + CreaturePvPStatus_Aggressive);

	npc->setLoadState(LoadState_Attributes);

	// Save default direction, since player can make the npc change heading.
	npc->storeDefaultDirection();

	return npc;
}

//=============================================================================

void PersistentNpcFactory::_setupDatabindings()
{
	mPersistentNpcBinding = mDatabase->CreateDataBinding(20);
	mPersistentNpcBinding->addField(DFT_uint64,offsetof(NPCObject,mId),8,0);
	mPersistentNpcBinding->addField(DFT_uint64,offsetof(NPCObject,mParentId),8,1);
	mPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mFirstName),64,2);
	mPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mLastName),64,3);
	mPersistentNpcBinding->addField(DFT_uint8,offsetof(NPCObject,mPosture),1,4);
	mPersistentNpcBinding->addField(DFT_uint64,offsetof(NPCObject,mState),8,5);
	mPersistentNpcBinding->addField(DFT_uint16,offsetof(NPCObject,mCL),2,6);
	mPersistentNpcBinding->addField(DFT_float,offsetof(NPCObject,mDirection.x),4,7);
	mPersistentNpcBinding->addField(DFT_float,offsetof(NPCObject,mDirection.y),4,8);
	mPersistentNpcBinding->addField(DFT_float,offsetof(NPCObject,mDirection.z),4,9);
	mPersistentNpcBinding->addField(DFT_float,offsetof(NPCObject,mDirection.w),4,10);
	mPersistentNpcBinding->addField(DFT_float,offsetof(NPCObject,mPosition.x),4,11);
	mPersistentNpcBinding->addField(DFT_float,offsetof(NPCObject,mPosition.y),4,12);
	mPersistentNpcBinding->addField(DFT_float,offsetof(NPCObject,mPosition.z),4,13);
	mPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mModel),256,14);
	mPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mSpecies),64,15);
	mPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mSpeciesGroup),64,16);
	mPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mFaction),64,17);
	mPersistentNpcBinding->addField(DFT_uint8,offsetof(NPCObject,mMoodId),1,18);
	mPersistentNpcBinding->addField(DFT_float,offsetof(NPCObject,mScale),4,20);

	mNpcIdentifierBinding = mDatabase->CreateDataBinding(1);
	mNpcIdentifierBinding->addField(DFT_uint32,offsetof(NpcIdentifier,mFamilyId),4,19);
	//mNpcIdentifierBinding->addField(DFT_uint32,offsetof(NPCIdentifier,mTypeId),4,3);
}

//=============================================================================

void PersistentNpcFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mPersistentNpcBinding);
	mDatabase->DestroyDataBinding(mNpcIdentifierBinding);
}

//=============================================================================

