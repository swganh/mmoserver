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
#include "Trainer.h"
#include "FillerNPC.h"
#include "NPCObject.h"
#include "QuestGiver.h"
#include "AttackableStaticNpc.h"
#include "LairObject.h"
#include "AttackableCreature.h"
#include "Inventory.h"
#include "WorldConfig.h"

#include <assert.h>

//=============================================================================

class QueryNonPersistentNpcFactory
{
	public:

		QueryNonPersistentNpcFactory(ObjectFactoryCallback* ofCallback,uint32 queryType, uint64 templateId = 0, uint64 id = 0) :
									mOfCallback(ofCallback),mQueryType(queryType), mTemplateId(templateId), mId(id) {}

		QueryNonPersistentNpcFactory(ObjectFactoryCallback* ofCallback,uint32 queryType, uint64 templateId, uint64 id, const SpawnData &spawn) :
		  mOfCallback(ofCallback),mQueryType(queryType), mTemplateId(templateId), mId(id), mSpawn(spawn) {}

		 QueryNonPersistentNpcFactory(ObjectFactoryCallback* ofCallback,uint32 queryType, uint64 templateId, uint64 id, const LairData &lairData) :
		  mOfCallback(ofCallback),mQueryType(queryType), mTemplateId(templateId), mId(id), mLairData(lairData) {}

		ObjectFactoryCallback*	mOfCallback;
		NPCObject*				mObject;
		uint32					mQueryType;
		uint64					mTemplateId;
		uint64					mId;
		SpawnData				mSpawn;
		LairData				mLairData;
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
		case NonPersistentNpcQuery_MainData:
		{
			// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() AT NonPersistentNpcQuery_MainData:", MSG_NORMAL);
			NPCObject* npc = _createNonPersistentNpc(result, asyncContainer->mId);

			if( npc->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
			{
				// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Invoking mOfCallback->handleObjectReady(npc)", MSG_NORMAL);
				asyncContainer->mOfCallback->handleObjectReady(npc);
			}

			else if (npc->getLoadState() == LoadState_Attributes)
			{
				QueryNonPersistentNpcFactory* asContainer = new QueryNonPersistentNpcFactory(asyncContainer->mOfCallback,NonPersistentNpcQuery_Attributes);
				asContainer->mObject = npc;
				
				// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Invoking NonPersistentNpcQuery_Attributes NPC ID = %llu", MSG_NORMAL, npc->getId());

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT attributes.name,persistent_npc_attributes.value,attributes.internal"
					" FROM persistent_npc_attributes"
					" INNER JOIN attributes ON (persistent_npc_attributes.attribute_id = attributes.id)"
					" WHERE persistent_npc_attributes.npc_id = %lld ORDER BY persistent_npc_attributes.order", asyncContainer->mTemplateId);
			}
		}
		break;

		case NonPersistentNpcQuery_Attributes:
		{
			// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() AT NonPersistentNpcQuery_Attributes:", MSG_NORMAL);
			_buildAttributeMap(asyncContainer->mObject,result);
			if(asyncContainer->mObject->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
			{
				// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Invoking mOfCallback->handleObjectReady(npc)", MSG_NORMAL);
				asyncContainer->mOfCallback->handleObjectReady(asyncContainer->mObject);
			}
			else
			{
				// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Invoking NOTHING", MSG_NORMAL);
			}
		}
		break;


		case NonPersistentNpcQuery_Spawn:
		{
			// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() AT NonPersistentNpcQuery_Spawn:", MSG_NORMAL);

			NPCObject* npc = _createNonPersistentNpc(result, asyncContainer->mId);

			// if(npc->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
			///	asyncContainer->mOfCallback->handleObjectReady(npc);

			// else
			if (npc->getLoadState() == LoadState_Attributes)
			{
				// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Attempting to get the Attributes", MSG_NORMAL);
				AttackableCreature* attackableNpc = dynamic_cast<AttackableCreature*>(npc);
				assert(attackableNpc);

				// Give object spawn info.
				attackableNpc->setSpawnData(asyncContainer->mSpawn);

				QueryNonPersistentNpcFactory* asContainer = new QueryNonPersistentNpcFactory(asyncContainer->mOfCallback,NonPersistentNpcQuery_Attributes);
				asContainer->mObject = npc;

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT attributes.name,persistent_npc_attributes.value,attributes.internal"
					" FROM persistent_npc_attributes"
					" INNER JOIN attributes ON (persistent_npc_attributes.attribute_id = attributes.id)"
					" WHERE persistent_npc_attributes.npc_id = %lld ORDER BY persistent_npc_attributes.order", asyncContainer->mTemplateId);
			}
			else
			{
				gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Attempting NOTHING\n", MSG_NORMAL);
			}
		}
		break;

		case NonPersistentNpcQuery_Lair:
		{
			// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() AT NonPersistentNpcQuery_Lair:", MSG_NORMAL);
			NPCObject* npc = _createNonPersistentNpc(result, asyncContainer->mId);

			if (npc->getLoadState() == LoadState_Attributes)
			{
				// gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Attempting to get the Attributes", MSG_NORMAL);

				LairObject* lair = dynamic_cast<LairObject*>(npc);
				assert(lair);

				// Give object spawn info.
				lair->setLairData(asyncContainer->mLairData);

				QueryNonPersistentNpcFactory* asContainer = new QueryNonPersistentNpcFactory(asyncContainer->mOfCallback,NonPersistentNpcQuery_Attributes);
				asContainer->mObject = npc;

				mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT attributes.name,persistent_npc_attributes.value,attributes.internal"
					" FROM persistent_npc_attributes"
					" INNER JOIN attributes ON (persistent_npc_attributes.attribute_id = attributes.id)"
					" WHERE persistent_npc_attributes.npc_id = %lld ORDER BY persistent_npc_attributes.order", asyncContainer->mTemplateId);
			}
			else
			{
				gLogger->logMsgF("NonPersistentNpcFactory::handleDatabaseJobComplete() Attempting NOTHING\n", MSG_NORMAL);
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
	requestObject(ofCallback,id, 0);
}

//=============================================================================

void NonPersistentNpcFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 templateId, uint64 npcNewId)
{
	// gLogger->logMsgF("NonPersistentNpcFactory::requestObject TemplateId = %llu, ObjectId will be = %llu", MSG_NORMAL, templateId, npcNewId);

	mDatabase->ExecuteSqlAsync(this,new QueryNonPersistentNpcFactory(ofCallback,NonPersistentNpcQuery_MainData, templateId, npcNewId),
		"SELECT persistent_npcs.posture,persistent_npcs.state,persistent_npcs.cl,"
		"persistent_npcs.type,persistent_npcs.stf_variable_id,persistent_npcs.stf_file_id,faction.name,"
		"persistent_npcs.moodId,persistent_npcs.family,persistent_npcs.scale "
		"FROM persistent_npcs "
		"INNER JOIN faction ON (persistent_npcs.faction = faction.id) "
		"WHERE (persistent_npcs.id = %lld)",templateId);
}

//=============================================================================

void NonPersistentNpcFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 templateId, uint64 npcNewId, const SpawnData spawn)
{
	// gLogger->logMsgF("NonPersistentNpcFactory::requestSpawnObject() TemplateId = %llu, ObjectId will be = %llu", MSG_NORMAL, templateId, npcNewId);

	mDatabase->ExecuteSqlAsync(this,new QueryNonPersistentNpcFactory(ofCallback,NonPersistentNpcQuery_Spawn, templateId, npcNewId, spawn),
		"SELECT persistent_npcs.posture,persistent_npcs.state,persistent_npcs.cl,"
		"persistent_npcs.type,persistent_npcs.stf_variable_id,persistent_npcs.stf_file_id,faction.name,"
		"persistent_npcs.moodId,persistent_npcs.family,persistent_npcs.scale "
		"FROM persistent_npcs "
		"INNER JOIN faction ON (persistent_npcs.faction = faction.id) "
		"WHERE (persistent_npcs.id = %lld)",templateId);
}

//=============================================================================

void NonPersistentNpcFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 templateId, uint64 npcNewId, const LairData lairData)
{
	// gLogger->logMsgF("NonPersistentNpcFactory::requestSpawnObject() TemplateId = %llu, ObjectId will be = %llu", MSG_NORMAL, templateId, npcNewId);

	mDatabase->ExecuteSqlAsync(this,new QueryNonPersistentNpcFactory(ofCallback,NonPersistentNpcQuery_Lair, templateId, npcNewId, lairData),
		"SELECT persistent_npcs.posture,persistent_npcs.state,persistent_npcs.cl,"
		"persistent_npcs.type,persistent_npcs.stf_variable_id,persistent_npcs.stf_file_id,faction.name,"
		"persistent_npcs.moodId,persistent_npcs.family,persistent_npcs.scale "
		"FROM persistent_npcs "
		"INNER JOIN faction ON (persistent_npcs.faction = faction.id) "
		"WHERE (persistent_npcs.id = %lld)",templateId);
}

//=============================================================================

NPCObject* NonPersistentNpcFactory::_createNonPersistentNpc(DatabaseResult* result, uint64 npcNewId)
{
	NPCObject*		npc;
	NpcIdentifier	npcIdentifier;

	// gLogger->logMsgF("NonPersistentNpcFactory::_createNonPersistentNpc() ObjectId = %llu", MSG_NORMAL, npcNewId);
	uint64 count = result->getRowCount();
	assert(count == 1);

	result->GetNextRow(mNpcIdentifierBinding,(void*)&npcIdentifier);
	result->ResetRowIndex();

	// gLogger->logMsgF("NonPersistentNpcFactory::_createNonPersistentNpc() ObjectId = %llu, familyId = %u", MSG_NORMAL, npcNewId, npcIdentifier.mFamilyId);

	switch(npcIdentifier.mFamilyId)
	{
		// TODO: Fix this when we get a real DB for the non persistents.
		// This is a test, since we are still using the persistent DB.
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
			npc	= new AttackableCreature();
		}
		break;

		case NpcFamily_NaturalLairs:
		{
			// First time lairs.
			npc	= new LairObject();
		}
		break;

		default:
		{
			npc = new NPCObject();
			gLogger->logMsgF("NonPersistentNpcFactory::createNonPersistent unknown Family %u\n",MSG_HIGH,npcIdentifier.mFamilyId);
		}
		break;
	}

	Inventory*	npcInventory = new Inventory();
	npcInventory->setParent(npc);

	result->GetNextRow(mNonPersistentNpcBinding,(void*)npc);

	// Override the persistent id, we are still fetching the modell from wrong database (the persistent_npcs one).
	npc->setId(npcNewId);

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
		// Dynamic spawned pve-enabled "static" creatures like lairs.
		npc->setType(ObjType_Creature);

		// This will ensure the use of the single H(am) bar.
		npc->setCreoGroup(CreoGroup_AttackableObject);	
		npc->mTypeOptions = 0x0;
		npc->togglePvPStateOn((CreaturePvPStatus)(CreaturePvPStatus_Attackable));

		npc->mHam.mHealth.setCurrentHitPoints(5000);
		npc->mHam.mHealth.setMaxHitPoints(5000);
		npc->mHam.mHealth.setBaseHitPoints(5000);
		npc->mHam.calcAllModifiedHitPoints();

		// Let's put some credits in the inventory.
		// npcInventory->setCredits((gRandom->getRand()%25) + 10);
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

		Weapon* defaultWeapon = new Weapon();
		defaultWeapon->setId(gWorldManager->getRandomNpId());

		defaultWeapon->setParentId(npc->mId);
		defaultWeapon->setModelString("object/weapon/melee/unarmed/shared_unarmed_default_player.iff");
		defaultWeapon->setGroup(WeaponGroup_Unarmed);
		defaultWeapon->setEquipSlotMask(CreatureEquipSlot_Weapon);
		defaultWeapon->addInternalAttribute("weapon_group","1");
 
		npc->mEquipManager.setDefaultWeapon(defaultWeapon);
		npc->mEquipManager.equipDefaultWeapon();

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
		// Let's put some credits in the inventory.
		// npcInventory->setCredits((gRandom->getRand()%25) + 10);
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
	mNonPersistentNpcBinding = mDatabase->CreateDataBinding(9);
	mNonPersistentNpcBinding->addField(DFT_uint8,offsetof(NPCObject,mPosture),1,0);
	mNonPersistentNpcBinding->addField(DFT_uint64,offsetof(NPCObject,mState),8,1);
	mNonPersistentNpcBinding->addField(DFT_uint16,offsetof(NPCObject,mCL),2,2);
	mNonPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mModel),256,3);
	mNonPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mSpecies),64,4);
	mNonPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mSpeciesGroup),64,5);
	mNonPersistentNpcBinding->addField(DFT_bstring,offsetof(NPCObject,mFaction),64,6);
	mNonPersistentNpcBinding->addField(DFT_uint8,offsetof(NPCObject,mMoodId),1,7);
	mNonPersistentNpcBinding->addField(DFT_float,offsetof(NPCObject,mScale),4,9);

	mNpcIdentifierBinding = mDatabase->CreateDataBinding(1);
	mNpcIdentifierBinding->addField(DFT_uint32,offsetof(NpcIdentifier,mFamilyId),4,8);
}

//=============================================================================

void NonPersistentNpcFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mNonPersistentNpcBinding);
	mDatabase->DestroyDataBinding(mNpcIdentifierBinding);
}

//=============================================================================
