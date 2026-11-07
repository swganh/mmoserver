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

#ifndef ANH_ZONESERVER_COMBATMANAGER_H
#define ANH_ZONESERVER_COMBATMANAGER_H

#define gCombatManager	CombatManager::getSingletonPtr()

#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "ScriptEngine/CombatRules.h"

#include <string>


//=========================================================================================

class Database;
class DatabaseCallback;
class DatabaseResult;
class Weapon;
class CreatureObject;
class CMWeaponGroup;
class ObjectControllerCmdProperties;

typedef	std::vector<CMWeaponGroup*>	WeaponGroups;

//=========================================================================================

class CombatManager : public DatabaseCallback
{
public:

    static CombatManager*	Init(Database* database);
    static CombatManager*	getSingletonPtr() {
        return mSingleton;
    }

    void					handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    uint32					getDefaultAttackAnimation(uint32 weaponGroup);
    BString					getDefaultSpam(uint32 weaponGroup);

    // delayMs, if given, gets the time to the attacker's next attack, from
    // the combat rules (data/script/combat/rules.lua).
    bool					handleAttack(CreatureObject*  attacker,uint64 targetId,ObjectControllerCmdProperties* cmdProperties, uint64* delayMs = 0);

    // The combat rules script, shared with NpcManager.
    CombatRules&			rules() { return mRules; }

    // An attack as the rules see it, from the zone's objects.
    CombatSide				buildSide(CreatureObject* creature);
    CombatWeapon			buildWeapon(Weapon* weapon);

    // Applies a hit's damage to the pool the rules chose.
    void					applyDamage(CreatureObject* defender, const std::string& pool, int32 damage);

    ~CombatManager();

private:

    CombatManager(Database* database);

    // bool					_verifyCombatState(CreatureObject* attacker,CreatureObject* defender);
    bool					_verifyCombatState(CreatureObject* attacker, uint64 defenderId);

    uint8					_executeAttack(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon, uint64* delayMs);
    uint8					_tryStateEffects(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon);

    static bool				mInsFlag;
    static CombatManager*	mSingleton;
    Database*				mDatabase;
    WeaponGroups			mWeaponGroups;
    CombatRules				mRules;
};

//=========================================================================================

#endif


