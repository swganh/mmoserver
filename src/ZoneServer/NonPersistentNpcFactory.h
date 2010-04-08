/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_NON_PERSISTENT_NPC_FACTORY_H
#define ANH_ZONESERVER_NON_PERSISTENT_NPC_FACTORY_H

#include "DatabaseManager/DatabaseCallback.h"
#include "FactoryBase.h"
#include "NpcIdentifier.h"
#include "MathLib/Vector3.h"
#include "MathLib/Quaternion.h"

class NPCObject;
class DatabaseResult;

#define	 gNonPersistentNpcFactory	NonPersistentNpcFactory::Instance()

//=============================================================================
// class WorldManager;

// class Database;
// class DataBinding;
// class DispatchClient;
// class ObjectFactoryCallback;
// class NPCObject;

// class SpawnData;
// class LairData;

//=============================================================================

enum NonPersistentNpcQuery
{
	// NonPersistentNpcQuery_MainData		= 1,
	// NonPersistentNpcQuery_Spawn			= 2,
	// NonPersistentNpcQuery_Lair			= 3,
	NonPersistentNpcQuery_Attributes	= 4,

	// Upgraded version
	NonPersistentNpcQuery_LairTemplate	= 5,			// Create the basic lair
	NonPersistentNpcQuery_LairCreatureTemplates	= 6,	// Get templates for creatures that can be spawned by lair.
	// NonPersistentNpcQuery_LairCreature = 7,				// Create creature from lair creature templates.
	NonPersistentNpcQuery_NpcTemplate = 8				// Create Npc from npc-template.

};

//=============================================================================

class NonPersistentNpcFactory : public FactoryBase
{
	public:
		static NonPersistentNpcFactory* Instance(void);
		/*
		static inline NonPersistentNpcFactory* Instance(void)
		{
			if (!mSingleton)
			{
				// mSingleton = new NonPersistentNpcFactory(WorldManager::getSingletonPtr()->getDatabase());
			}
			return mSingleton;
		}
		*/
		static inline void deleteFactory(void)
		{
			if (mSingleton)
			{
				delete mSingleton;
				mSingleton = 0;
			}
		}

		void	handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void	requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);
		// void	requestObject(ObjectFactoryCallback* ofCallback,uint64 id, uint64 npcNewId);
		// void	requestObject(ObjectFactoryCallback* ofCallback,uint64 templateId, uint64 npcNewId, const SpawnData spawn);
		// void	requestObject(ObjectFactoryCallback* ofCallback,uint64 templateId, uint64 npcNewId, const LairData lairData);

		// Upgraded versions
		void			requestLairObject(ObjectFactoryCallback* ofCallback, uint64 templateId, uint64 npcNewId);
		// void			requestNpcObject(ObjectFactoryCallback* ofCallback, uint64 creatureTemplateId, uint64 npcNewId, uint64 parentLairId);
		void			requestNpcObject(ObjectFactoryCallback* ofCallback,
										   uint64 creatureTemplateId,
										   uint64 npcNewId,
										   uint64 spawnCellId,
										   const glm::vec3& spawnPosition,
										   const glm::quat&	spawnDirection,
										   uint64 respawnDelay,
										   uint64 parentLairId = 0);


	protected:
		NonPersistentNpcFactory(Database* database);
		~NonPersistentNpcFactory();

	private:
		NonPersistentNpcFactory();

		void				_setupDatabindings();
		void				_destroyDatabindings();

		NPCObject*			_createNonPersistentNpc(DatabaseResult* result, uint64 templateId, uint64 npcNewId, uint64 controllingObject);
		NPCObject*			createNonPersistentNpc(DatabaseResult* result, uint64 templateId, uint64 npcNewId, uint32 familyId, uint64 controllingObject);

		static NonPersistentNpcFactory*	mSingleton;
		static bool						mInsFlag;

		DataBinding*					mNonPersistentNpcBinding;
		DataBinding*					mNpcIdentifierBinding;
};

//=============================================================================

#endif

