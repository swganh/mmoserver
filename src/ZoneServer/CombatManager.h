/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_COMBATMANAGER_H
#define ANH_ZONESERVER_COMBATMANAGER_H

#define gCombatManager	CombatManager::getSingletonPtr()

#include "Utils/typedefs.h"
#include "CMWeaponGroup.h"
#include "DatabaseManager/DatabaseCallback.h"


//=========================================================================================

class Database;
class DatabaseCallback;
class DatabaseResult;
class Weapon;
class CreatureObject;
class ObjectControllerCmdProperties;

typedef	std::vector<CMWeaponGroup*>	WeaponGroups;

//=========================================================================================

class CombatManager : public DatabaseCallback
{
	public:

		static CombatManager*	Init(Database* database);
		static CombatManager*	getSingletonPtr() { return mSingleton; }

		void					handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		uint32					getDefaultAttackAnimation(uint32 weaponGroup);
		string					getDefaultSpam(uint32 weaponGroup);

		bool					handleAttack(CreatureObject*  attacker,uint64 targetId,ObjectControllerCmdProperties* cmdProperties);

		~CombatManager();

	private:

		CombatManager(Database* database);

		// bool					_verifyCombatState(CreatureObject* attacker,CreatureObject* defender);
		bool					_verifyCombatState(CreatureObject* attacker, uint64 defenderId);
		
		uint8					_executeAttack(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon);
		uint8					_hitCheck(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon);
		int32					_mitigateDamage(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,int32 oldDamage,Weapon* weapon);
		uint8					_tryStateEffects(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon);

		static bool				mInsFlag;
		static CombatManager*	mSingleton;
		Database*				mDatabase;
		WeaponGroups			mWeaponGroups;
};

//=========================================================================================

#endif


