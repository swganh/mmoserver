/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_NPC_MANAGER_H
#define ANH_ZONESERVER_NPC_MANAGER_H


#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/typedefs.h"
#include "ObjectFactoryCallback.h"

//=============================================================================

class AttackableCreature;
class CreatureObject;
class Database;
class NPCObject;
class SpawnData;
class Weapon;

//======================================================================================================================
//
// Container for asyncronous database queries
//
enum NpcQuery
{
	NpcQuery_Lairs = 0,
};

class NpcAsyncContainer
{
	public:
		NpcAsyncContainer(NpcQuery query){ mQuery = query;}
		NpcQuery mQuery;
};


//=============================================================================


class NpcManager  : public ObjectFactoryCallback, public DatabaseCallback
{
	public:

		static NpcManager* NpcManager::Instance(void);

		static inline void deleteManager(void)    
		{ 
			if (mInstance)
			{
				delete mInstance;
				mInstance = 0;
			}
		}

		// Inherited  interface that we have to provide.
		virtual void	handleObjectReady(Object* object);
		virtual void    handleDatabaseJobComplete(void* ref, DatabaseResult* result);

		// void	addCreature(uint64 creatureId, const SpawnData *spawn);
		void	handleExpiredCreature(uint64 creatureId);
		// void	removeNpc(uint64 npcId);
		bool	handleAttack(CreatureObject *attacker, uint64 targetId);

		uint64	handleNpc(NPCObject* npc, uint64 timeOverdue);

		void	loadLairs(void);
		

	protected:
		NpcManager(Database* database);
		~NpcManager();

	private:
		// This constructor prevents the default constructor to be used, since it is private.
		NpcManager();
		
		// Simulated Combat Manager
		uint8	_hitCheck(CreatureObject* attacker,CreatureObject* defender,Weapon* weapon);
		bool	_verifyCombatState(CreatureObject* attacker, uint64 defenderId);
		
		uint8	_executeAttack(CreatureObject* attacker,CreatureObject* defender,Weapon* weapon);
		void	setTargetDirection(AttackableCreature* npc);
		// void			_setupDatabindings();
		// void			_destroyDatabindings();

		static NpcManager* mInstance;
		Database* mDatabase;
		// DataBinding*	mItemIdentifierBinding;
		// DataBinding*	mItemBinding;
};

//=============================================================================



#endif

