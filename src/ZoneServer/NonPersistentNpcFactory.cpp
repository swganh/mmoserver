/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "NonPersistentNpcFactory.h"
#include "ObjectFactoryCallback.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "LogManager/LogManager.h"
#include "Utils/utils.h"
#include "Utils/rand.h"
#include "Trainer.h"
#include "FillerNPC.h"
#include "NPCObject.h"
#include "QuestGiver.h"
#include "AttackableStaticNpc.h"
#include "LairObject.h"
#include "AttackableCreature.h"
#include "Inventory.h"
#include "WorldConfig.h"
#include "WorldManager.h"

#include <assert.h>

//=============================================================================

class QueryNonPersistentNpcFactory
{
	public:

		QueryNonPersistentNpcFactory(ObjectFactoryCallback* ofCallback,uint32 queryType, uint64 templateId = 0, uint64 id = 0) :
									mOfCallback(ofCallback),mQueryType(queryType), mTemplateId(templateId), mId(id) {}

		QueryNonPersistentNpcFactory(ObjectFactoryCallback* ofCallback,
									 uint32 queryType,
									 uint64 templateId,
									 uint64 id,
									 uint64 spawnCellId,
									 Anh_Math::Vector3 spawnPosition,
									 Anh_Math::Quaternion spawnDirection,
									 uint64	respawnDelay,
									 uint64 parentLairId) :
									mOfCallback(ofCallback),
									mQueryType(queryType), 
									mTemplateId(templateId), 
									mId(id),
									mSpawnCellId(spawnCellId),
									mSpawnPosition(spawnPosition),
									mSpawnDirection(spawnDirection),
									mRespawnDelay(respawnDelay),
									mParentObjectId(parentLairId) {}

		ObjectFactoryCallback*	mOfCallback;
		uint32					mQueryType;
		uint64					mTemplateId;
		uint64					mId;
		uint64					mSpawnCellId;
		Anh_Math::Vector3		mSpawnPosition; 
		Anh_Math::Quaternion	mSpawnDirection;
		uint64					mRespawnDelay;
		uint64					mParentObjectId;
};

//=============================================================================

// Basic lair data when loading from DB.

class NpcLairEntityEx
{
	public:
		NpcLairEntityEx(){}
		
		uint64	mCreatureSpwanRegion;
		uint64	mTemplateId;
		uint32	mCreatureGroup;
		uint32	mNumberOfLairs;
		float	mSpawnPosX;
		float	mSpawnPosZ;
		float	mSpawnDirY;
		float	mSpawnDirW;
		uint32	mFamily;
		string	mFaction;
		string	mObjectString;
		string	mStfName;
		string	mStfFile;
};

//=============================================================================

NonPersistentNpcFactory*	NonPersistentNpcFactory::mSingleton  = NULL;

//======================================================================================================================

NonPersistentNpcFactory* NonPersistentNpcFactory::Instance(void)
{
	if (!mSingleton)
	{
		mSingleton = new NonPersistentNpcFactory(WorldManager::getSingletonPtr()->getDatabase());
	}
	return mSingleton;
}



// This constructor prevents the default constructor to be used, as long as the constructor is keept private.

NonPersistentNpcFactory::NonPersistentNpcFactory() : FactoryBase(NULL)
{
}

//=============================================================================
NonPersistentNpcFactory::NonPersistentNpcFactory(Database* database)  : FactoryBase(database)
{
	_setupDatabindings();
}

//=============================================================================

NonPersistentNpcFactory::~NonPersistentNpcFactory()
{
	_destroyDatabindings();
	mSingleton = NULL;
}

//=============================================================================

void NonPersistentNpcFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	
	QueryNonPersistentNpcFactory* asyncContainer = reinterpret_cast<QueryNonPersistentNpcFactory*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case NonPersistentNpcQuery_Attributes:
		{
			// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() AT NonPersistentNpcQuery_Attributes:", MSG_NORMAL);
			Object* object = gWorldManager->getObjectById(asyncContainer->mId);
			if (object)
			{
				_buildAttributeMap(object,result);
				if (object->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
				{
					// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Invoking mOfCallback->handleObjectReady(npc)", MSG_NORMAL);
					asyncContainer->mOfCallback->handleObjectReady(object);
				}
				else
				{
					// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Invoking NOTHING", MSG_NORMAL);
				}
			}
			else
			{
				gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Object is GONE", MSG_NORMAL);
			}
		}
		break;
//=============================================================================
//
//	Upgrade version for use of the correct DB.
//
//=============================================================================

		case NonPersistentNpcQuery_LairTemplate:
		{
			// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() AT NonPersistentNpcQuery_LairTemplate:", MSG_NORMAL);

			NpcLairEntityEx lair;

			DataBinding* lairSpawnBinding = mDatabase->CreateDataBinding(9);
			lairSpawnBinding->addField(DFT_uint64,offsetof(NpcLairEntityEx,mCreatureSpwanRegion),8,0);
			lairSpawnBinding->addField(DFT_uint64,offsetof(NpcLairEntityEx,mTemplateId),8,1);
			lairSpawnBinding->addField(DFT_uint32,offsetof(NpcLairEntityEx,mCreatureGroup),4,2);
			lairSpawnBinding->addField(DFT_uint32,offsetof(NpcLairEntityEx,mNumberOfLairs),4,3);
			lairSpawnBinding->addField(DFT_float,offsetof(NpcLairEntityEx,mSpawnPosX),4,4);
			lairSpawnBinding->addField(DFT_float,offsetof(NpcLairEntityEx,mSpawnPosZ),4,5);
			lairSpawnBinding->addField(DFT_float,offsetof(NpcLairEntityEx,mSpawnDirY),4,6);
			lairSpawnBinding->addField(DFT_float,offsetof(NpcLairEntityEx,mSpawnDirW),4,7);
			lairSpawnBinding->addField(DFT_uint32,offsetof(NpcLairEntityEx,mFamily),4,8);
			
			DataBinding* lairSpawnNpcBinding = mDatabase->CreateDataBinding(4);
			lairSpawnNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mModel),255,9);
			lairSpawnNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mSpecies),255,10);
			lairSpawnNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mSpeciesGroup),255,11);
			lairSpawnNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mFaction),32,12);
					
			uint64 count = result->getRowCount();
			assert(count == 1);
			result->GetNextRow(lairSpawnBinding,&lair);

			// Let's create the lair.
		
			// We save the lairs-type... that's kinda a template for the complete lair.
			LairObject* npc	= new LairObject(asyncContainer->mTemplateId);
			assert(npc);

			// Set the new if of this temp object.
			npc->setId(asyncContainer->mId);

			// Register object with WorldManager.
			gWorldManager->addObject(npc, true);

			// May need the height also, in case of pre set (fixed) spawn position.
			npc->mPosition.mX = lair.mSpawnPosX;
			npc->mPosition.mZ = lair.mSpawnPosZ;

			if (npc->getParentId() == 0)
			{
				// Heightmap only works outside.
				npc->mPosition.mY = npc->getHeightAt2DPosition(lair.mSpawnPosX, lair.mSpawnPosZ, true);
			}
			else
			{
				// We do not have support for handling creatures inside.
				assert(false);
				npc->mPosition.mY = 0;
			}
			
			npc->mDirection.mY = lair.mSpawnDirY;
			npc->mDirection.mW = lair.mSpawnDirW;

			// Let's get the spawn area.
			const Anh_Math::Rectangle spawnArea = gWorldManager->getSpawnArea(lair.mCreatureSpwanRegion);

			// lair.mCreatureSpwanRegion
			npc->setSpawnArea(spawnArea); 

			result->ResetRowIndex();
			result->GetNextRow(lairSpawnNpcBinding,(void*)npc );
			
			mDatabase->DestroyDataBinding(lairSpawnBinding);
			mDatabase->DestroyDataBinding(lairSpawnNpcBinding);

			Inventory*	npcInventory = new Inventory();
			npcInventory->setParent(npc);

			npc->mHam.mHealth.setCurrentHitPoints(500);
			npc->mHam.mAction.setCurrentHitPoints(500);
			npc->mHam.mMind.setCurrentHitPoints(500);
			npc->mHam.calcAllModifiedHitPoints();

			// inventory
			npcInventory->setId(npc->mId + 1);
			npcInventory->setParentId(npc->mId);
			npcInventory->setModelString("object/tangible/inventory/shared_creature_inventory.iff");
			
			npcInventory->setName("inventory");
			npcInventory->setNameFile("item_n");
			npcInventory->setTangibleGroup(TanGroup_Inventory);
			npcInventory->setTangibleType(TanType_CreatureInventory);
			npc->mEquipManager.addEquippedObject(CreatureEquipSlot_Inventory,npcInventory);

			npc->setType(ObjType_Creature);

			// This will ensure the use of the single H(am) bar.
			npc->setCreoGroup(CreoGroup_AttackableObject);	
			npc->mTypeOptions = 0x0;
			npc->togglePvPStateOn((CreaturePvPStatus)(CreaturePvPStatus_Attackable));

			// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Attempting to get the creature templates for group %u used by this lair.", MSG_NORMAL, lair.mCreatureGroup);

			QueryNonPersistentNpcFactory* asContainer = new QueryNonPersistentNpcFactory(asyncContainer->mOfCallback, NonPersistentNpcQuery_LairCreatureTemplates, asyncContainer->mTemplateId, asyncContainer->mId);
			// Do not transfer object refs, use the handle, i.e. asyncContainer->mId
			// asContainer->mObject = npc;

			mDatabase->ExecuteSqlAsync(this, asContainer,
							"SELECT creature_groups.creature_id "
							"FROM creature_groups "
							"WHERE creature_groups.creature_group_id=%u;",lair.mCreatureGroup);

		}
		break;

		case NonPersistentNpcQuery_LairCreatureTemplates:
		{
			// Get the lair object.
			LairObject* npc = dynamic_cast<LairObject*>(gWorldManager->getObjectById(asyncContainer->mId));
			assert(npc);

			uint64	creatureTemplateId;
			DataBinding* creatureTemplateBinding = mDatabase->CreateDataBinding(1);
			creatureTemplateBinding->addField(DFT_uint64,0,8,0);

			uint64 count = result->getRowCount();

			int32	spawnRateInc = 100/static_cast<uint32>(count);
			int32	spawnRate = -1;

			for (uint64 i = 0; i < count; i++)
			{
				result->GetNextRow(creatureTemplateBinding, &creatureTemplateId);
				// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Creature template %llu used by lair ", MSG_NORMAL, creatureTemplateId);
				npc->setCreatureTemplate((uint32)i, creatureTemplateId);

				spawnRate += spawnRateInc;
				npc->setCreatureSpawnRate((uint32)i, (uint32)spawnRate);
			}
			if (count > 0 && spawnRate < 99)
			{
				npc->setCreatureSpawnRate(static_cast<uint32>(count) - 1, 99);
			}
			mDatabase->DestroyDataBinding(creatureTemplateBinding);


			// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Attempting to get the Attributes", MSG_NORMAL);

			QueryNonPersistentNpcFactory* asContainer = new QueryNonPersistentNpcFactory(asyncContainer->mOfCallback,NonPersistentNpcQuery_Attributes, 0, npc->getId());

			mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT attributes.name,lair_attributes.value,attributes.internal"
				" FROM lair_attributes"
				" INNER JOIN attributes ON (lair_attributes.attribute_id = attributes.id)"
				" WHERE lair_attributes.lair_id = %lld ORDER BY lair_attributes.order", asyncContainer->mTemplateId);
		}
		break;

		case NonPersistentNpcQuery_NpcTemplate:
		{
			// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() AT NonPersistentNpcQuery_NpcTemplate:", MSG_NORMAL);
			NPCObject* npc = _createNonPersistentNpc(result, asyncContainer->mTemplateId, asyncContainer->mId, asyncContainer->mParentObjectId);
			assert(npc);

			// Spawn related data.
			npc->setCellIdForSpawn(asyncContainer->mSpawnCellId);
			npc->setSpawnPosition(asyncContainer->mSpawnPosition);
			npc->setSpawnDirection(asyncContainer->mSpawnDirection);
			npc->setRespawnDelay(asyncContainer->mRespawnDelay);

			if( npc->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
			{
				// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Invoking mOfCallback->handleObjectReady(npc)", MSG_NORMAL);
				asyncContainer->mOfCallback->handleObjectReady(npc);
			}
			else if (npc->getLoadState() == LoadState_Attributes)
			{
				QueryNonPersistentNpcFactory* asContainer = new QueryNonPersistentNpcFactory(asyncContainer->mOfCallback,NonPersistentNpcQuery_Attributes, 0, npc->getId());
				
				// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Invoking NonPersistentNpcQuery_Attributes NPC ID = %llu", MSG_NORMAL, npc->getId());

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT attributes.name,non_persistent_npc_attributes.value,attributes.internal"
					" FROM non_persistent_npc_attributes"
					" INNER JOIN attributes ON (non_persistent_npc_attributes.attribute_id = attributes.id)"
					" WHERE non_persistent_npc_attributes.npc_id = %lld ORDER BY non_persistent_npc_attributes.order", asyncContainer->mTemplateId);
			}
		}
		break;

		default:
		{
			gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() UNKNOWN query = %u\n", MSG_NORMAL, asyncContainer->mQueryType);
		}
		break;
	}

	delete asyncContainer;
}

//=============================================================================

// Just a placeholder o make compiler happy.
void NonPersistentNpcFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	assert(false);
	// requestObject(ofCallback,id, 0);
}

//=============================================================================

NPCObject* NonPersistentNpcFactory::_createNonPersistentNpc(DatabaseResult* result, uint64 templateId, uint64 npcNewId, uint64 controllingObject)
{
	NpcIdentifier	npcIdentifier;

	// gLogger->logMsgF("NonPersistentNpcFactory::_createNonPersistentNpc() ObjectId = %llu", MSG_NORMAL, npcNewId);
	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mNpcIdentifierBinding,(void*)&npcIdentifier);
	result->ResetRowIndex();

	// gLogger->logMsgF("NonPersistentNpcFactory::_createNonPersistentNpc() TemplateId = %llu ObjectId = %llu, familyId = %u controllingObject = %llu", MSG_NORMAL, templateId, npcNewId, npcIdentifier.mFamilyId, controllingObject);

	return this->createNonPersistentNpc(result, templateId, npcNewId, npcIdentifier.mFamilyId, controllingObject);
}


NPCObject* NonPersistentNpcFactory::createNonPersistentNpc(DatabaseResult* result, uint64 templateId, uint64 npcNewId, uint32 familyId, uint64 controllingObject)
{
	NPCObject*		npc;

	switch(familyId)
	{
		case NpcFamily_Trainer:
		{
			npc	= new Trainer();
		}
		break;		

		case NpcFamily_Filler:
		{
			npc	= new FillerNPC();
		}
		break;

		case NpcFamily_QuestGiver:
		{
			npc	= new QuestGiver();
		}
		break;

		case NpcFamily_AttackableObject:
		{
			// Stuff like Debris.
			npc	= new AttackableStaticNpc();
		}
		break;

		case NpcFamily_AttackableCreatures:
		{
			// gLogger->logMsgF("NonPersistentNpcFactory::_createNonPersistentNpc() Created a NpcFamily_AttackableCreatures", MSG_NORMAL);
			// Stuff like npc's and womp rats :).
			npc	= new AttackableCreature(templateId);
		}
		break;

		case NpcFamily_NaturalLairs:
		{
			// First time lairs.
			// gLogger->logMsgF("NonPersistentNpcFactory::createNonPersistentNpc() Created a NpcFamily_NaturalLairs", MSG_NORMAL);

			//Lairs are not supported here, at least not yet.
			assert(false);
			npc	= new LairObject(templateId);
		}
		break;

		default:
		{
			gLogger->logMsgF("NonPersistentNpcFactory::createNonPersistent unknown Family %u",MSG_HIGH,familyId);
			assert(false);
			npc = new NPCObject();
		}
		break;
	}

	assert(npc);

	// Set the new temporarily id.
	npc->setId(npcNewId);

	// Register object with WorldManager.
	gWorldManager->addObject(npc, true);

	Inventory*	npcInventory = new Inventory();
	npcInventory->setParent(npc);

	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mNonPersistentNpcBinding,(void*)npc);

	// The template for this creature, in case of a respawn.
	npc->mNpcTemplateId = templateId;

	// Should bet fetched from attributes, these will do as defaults.
	npc->mHam.mHealth.setCurrentHitPoints(500);
	npc->mHam.mAction.setCurrentHitPoints(500);
	npc->mHam.mMind.setCurrentHitPoints(500);
	npc->mHam.calcAllModifiedHitPoints();

	// inventory
	npcInventory->setId(npc->mId + 1);
	npcInventory->setParentId(npc->mId);
	npcInventory->setModelString("object/tangible/inventory/shared_creature_inventory.iff");
	
	npcInventory->setName("inventory");
	npcInventory->setNameFile("item_n");
	npcInventory->setTangibleGroup(TanGroup_Inventory);
	npcInventory->setTangibleType(TanType_CreatureInventory);
	npc->mEquipManager.addEquippedObject(CreatureEquipSlot_Inventory,npcInventory);

	if (npc->getNpcFamily() == NpcFamily_AttackableObject)
	{
		// Dynamic spawned pve-enabled "static" creatures like debris.
		npc->setType(ObjType_Creature);
		npc->setCreoGroup(CreoGroup_AttackableObject);

		npc->mTypeOptions = 0x0;
		
		// Let's start as non-attackable. 
		// npc->togglePvPStateOn((CreaturePvPStatus)(CreaturePvPStatus_Attackable));
	}
	else if (npc->getNpcFamily() == NpcFamily_NaturalLairs)
	{
		//Lairs are not supported here, at least not yet.
		assert(false);

		// Dynamic spawned pve-enabled "static" creatures like lairs.
		npc->setType(ObjType_Creature);

		// This will ensure the use of the single H(am) bar.
		npc->setCreoGroup(CreoGroup_AttackableObject);	
		npc->mTypeOptions = 0x0;
		npc->togglePvPStateOn((CreaturePvPStatus)(CreaturePvPStatus_Attackable));

		// npc->mHam.mHealth.setCurrentHitPoints(5000);
		// npc->mHam.mHealth.setMaxHitPoints(5000);
		// npc->mHam.mHealth.setBaseHitPoints(5000);
		// npc->mHam.calcAllModifiedHitPoints();

		// Let's put some credits in the inventory.
		// npcInventory->setCredits((gRandom->getRand()%25) + 10);
		// gLogger->logMsgF("NonPersistentNpcFactory::createNonPersistentNpc() WOW, I'm a lair", MSG_NORMAL);
	}
	else if (npc->getNpcFamily() == NpcFamily_AttackableCreatures)
	{
		// Dynamic spawned pve-enabled creatures.
		npc->setType(ObjType_Creature);
		npc->setCreoGroup(CreoGroup_Creature);

		npc->mTypeOptions = 0x0;

		if (gWorldConfig->isTutorial())
		{
			npc->togglePvPStateOn((CreaturePvPStatus)(CreaturePvPStatus_Attackable + CreaturePvPStatus_Aggressive + CreaturePvPStatus_Enemy ));
		}
		else
		{
			npc->togglePvPStateOn((CreaturePvPStatus)(CreaturePvPStatus_Attackable));
		}

		AttackableCreature* attackableNpc = dynamic_cast<AttackableCreature*>(npc);
		assert(attackableNpc);

		// Fix this later
		// Also set the owner (lair) who's controlling this creature.
		attackableNpc->setLairId(controllingObject);

		Weapon* defaultWeapon = new Weapon();
		defaultWeapon->setId(gWorldManager->getRandomNpId());

		defaultWeapon->setParentId(npc->mId);
		defaultWeapon->setModelString("object/weapon/melee/unarmed/shared_unarmed_default_player.iff");
		defaultWeapon->setGroup(WeaponGroup_Unarmed);
		defaultWeapon->setEquipSlotMask(CreatureEquipSlot_Weapon);
		defaultWeapon->addInternalAttribute("weapon_group","1");
 
		npc->mEquipManager.setDefaultWeapon(defaultWeapon);
		npc->mEquipManager.equipDefaultWeapon();

// Weapon to use should be gotten from attibutes or whereever we find that kind of info.
		// This little fellow may need a gun.
		Weapon* pistol = new Weapon();
		pistol->setId(gWorldManager->getRandomNpId());

		pistol->setParentId(npc->mId);
		pistol->setModelString("object/weapon/ranged/pistol/shared_pistol_cdef.iff");
		pistol->setGroup(WeaponGroup_Pistol);
		pistol->setEquipSlotMask(CreatureEquipSlot_Weapon);
		pistol->addInternalAttribute("weapon_group","32");
		attackableNpc->setPrimaryWeapon(pistol);

		
		// A saber can be handy, too.
		Weapon* saber = new Weapon();
		saber->setId(gWorldManager->getRandomNpId());

		saber->setParentId(npc->mId);
		saber->setModelString("object/weapon/melee/sword/shared_sword_lightsaber_vader.iff");
		saber->setGroup(WeaponGroup_2h);
		saber->setEquipSlotMask(CreatureEquipSlot_Weapon);
		saber->addInternalAttribute("weapon_group","4");
		attackableNpc->setSecondaryWeapon(saber);

		if (gWorldConfig->isTutorial())
		{
			attackableNpc->equipPrimaryWeapon();
		}
		else
		{
			// attackableNpc->equipSecondaryWeapon();
		}

// Should be handle by "loot manager"
		// Let's put some credits in the inventory.
		npcInventory->setCredits((gRandom->getRand()%25) + 10);
	}
	else
	{
		npc->mTypeOptions = 0x108;
	}
	npc->setLoadState(LoadState_Attributes);

	// Save default direction, since player can make the npc change heading.
	// Can't apply this to a dynamically created npc.
	// npc->storeDefaultDirection();
	return npc;
}

//=============================================================================

void NonPersistentNpcFactory::_setupDatabindings()
{
	mNonPersistentNpcBinding = mDatabase->CreateDataBinding(11);
	mNonPersistentNpcBinding->addField(DFT_uint64,offsetof(NPCObject,mSpeciesId),8,0);
	mNonPersistentNpcBinding->addField(DFT_uint64,offsetof(NPCObject,mLootGroupId),8,1);
	mNonPersistentNpcBinding->addField(DFT_uint8,offsetof(NPCObject,mPosture),1,2);
	mNonPersistentNpcBinding->addField(DFT_uint64,offsetof(NPCObject,mState),8,3);
	mNonPersistentNpcBinding->addField(DFT_uint16,offsetof(NPCObject,mCL),2,4);
	mNonPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mModel),255,5);
	mNonPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mSpecies),255,6);
	mNonPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mSpeciesGroup),255,7);
	mNonPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mFaction),32,8);
	mNonPersistentNpcBinding->addField(DFT_uint8,offsetof(NPCObject,mMoodId),1,9);
	mNonPersistentNpcBinding->addField(DFT_float,offsetof(NPCObject,mScale),4,10);

	mNpcIdentifierBinding = mDatabase->CreateDataBinding(1);
	mNpcIdentifierBinding->addField(DFT_uint32,offsetof(NpcIdentifier,mFamilyId),4,11);
}

//=============================================================================

void NonPersistentNpcFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mNonPersistentNpcBinding);
	mDatabase->DestroyDataBinding(mNpcIdentifierBinding);
}

//=============================================================================
//
//	Upgrade version for use of the correct DB.
//
//=============================================================================
void NonPersistentNpcFactory::requestLairObject(ObjectFactoryCallback* ofCallback, uint64 lairsId, uint64 npcNewId)
{
	// gLogger->logMsgF("NonPersistentNpcFactory::requestLairObject() LairsId = %llu, ObjectId will be = %llu", MSG_NORMAL, lairsId, npcNewId);

	mDatabase->ExecuteSqlAsync(this,new QueryNonPersistentNpcFactory(ofCallback, NonPersistentNpcQuery_LairTemplate, lairsId, npcNewId),
								"SELECT lairs.creature_spawn_region, lairs.lair_template, lairs.creature_group, lairs.count, lairs.spawn_x, lairs.spawn_z, "
								"lairs.spawn_dir_Y, lairs.spawn_dir_W, "
								"lairs.family, lair_templates.lair_object_string, lair_templates.stf_name, lair_templates.stf_file, "
								"faction.name "
								"FROM lairs "
								"INNER JOIN spawns ON (lairs.creature_spawn_region = spawns.id AND %u = spawns.spawn_planet) "
								"INNER JOIN lair_templates ON (lairs.lair_template = lair_templates.id) "
								"INNER JOIN faction ON (lairs.faction = faction.id) "
								"WHERE lairs.id=%u;",gWorldManager->getZoneId(), lairsId);
}

void NonPersistentNpcFactory::requestNpcObject(ObjectFactoryCallback* ofCallback, 
											   uint64 creatureTemplateId, 
											   uint64 npcNewId,
											   uint64 spawnCellId,
											   Anh_Math::Vector3 spawnPosition, 
											   Anh_Math::Quaternion	spawnDirection,
											   uint64 respawnDelay,
											   uint64 parentLairId)
{

	mDatabase->ExecuteSqlAsync(this,new QueryNonPersistentNpcFactory(ofCallback, NonPersistentNpcQuery_NpcTemplate, creatureTemplateId, npcNewId, spawnCellId, spawnPosition, spawnDirection, respawnDelay, parentLairId),
								"SELECT non_persistent_npcs.species_id, non_persistent_npcs.loot_group_id, "
								"non_persistent_npcs.posture, non_persistent_npcs.state, non_persistent_npcs.level, "
								"non_persistent_npcs.type, non_persistent_npcs.stf_variable_id, non_persistent_npcs.stf_file_id, "
								"faction.name, "
								"non_persistent_npcs.moodID, non_persistent_npcs.scale, non_persistent_npcs.family "
								"FROM non_persistent_npcs "
								"INNER JOIN faction ON (non_persistent_npcs.faction = faction.id) "
								"WHERE non_persistent_npcs.id=%lld;", creatureTemplateId);
}
