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
#include <vector>


//=========================================================================================

class Database;
class DatabaseCallback;
class DatabaseResult;
class Weapon;
class CreatureObject;
class CMWeaponGroup;
class ObjectControllerCmdProperties;
class PlayerObject;

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
    // Intimidate, Warcry and Taunt (rules.MOVES): their HAM cost, then a roll
    // against each target. False if the move could not be used.
    bool					handleBrawlerMove(PlayerObject* attacker, uint64 targetId, ObjectControllerCmdProperties* cmdProperties);
    // Berserk and Center of Being: the rules decide (rules.selfBuff).
    bool					handleSelfBuff(PlayerObject* player, ObjectControllerCmdProperties* cmdProperties);

    // The combat rules script, shared with NpcManager.
    CombatRules&			rules() { return mRules; }

    // An attack as the rules see it, from the zone's objects.
    CombatSide				buildSide(CreatureObject* creature);
    CombatWeapon			buildWeapon(Weapon* weapon);

    // Tells the player's client the range data of the weapon in hand, for
    // the target window's accuracy display.
    void					sendWeaponRanges(PlayerObject* player);

    // Applies a hit's damage to the pool the rules chose.
    void					applyDamage(CreatureObject* defender, const std::string& pool, int32 damage);
    // The wound and battle fatigue a hit leaves (rules.wound, FATIGUE_PER_HIT).
    void					applyWounds(CreatureObject* defender, const CombatOutcome& outcome);

    ~CombatManager();

private:

    CombatManager(Database* database);

    // bool					_verifyCombatState(CreatureObject* attacker,CreatureObject* defender);
    bool					_verifyCombatState(CreatureObject* attacker, uint64 defenderId);

    uint8					_executeAttack(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon, uint64* delayMs);
    // The attack as the rules see it: the sides, the weapon and the command.
    CombatAttack			_buildAttack(CreatureObject* attacker, CreatureObject* defender, ObjectControllerCmdProperties* cmdProperties, Weapon* weapon);
    // Charges a special's HAM costs; false (nothing charged) if the attacker
    // cannot pay them.
    bool					_payCosts(CreatureObject* attacker, const CombatAttack& attack);
    // Applies the effects that landed; timed states are removed again later.
    void					_applyEffects(CreatureObject* defender, const CombatOutcome& outcome);
    // Scheduler callback: removes timed states whose time is up.
    bool					_expireStates(uint64 callTime, void* ref);

    struct TimedState
    {
        uint64	creatureId;
        uint64	state;
        uint64	expires;	// ms, Clock local time
    };
    std::vector<TimedState>	mTimedStates;
    // Self buffs (berserk, Center of Being) and when they end.
    struct TimedBuff
    {
        uint64		creatureId;
        std::string	name;
        float		strength;
        uint64		expires;	// ms, Clock local time
    };
    std::vector<TimedBuff>	mBuffs;
    // Charges a command's flat HAM costs; false (nothing charged) if the
    // creature cannot pay them.
    bool					_payFlatCosts(CreatureObject* creature, ObjectControllerCmdProperties* cmdProperties);

    // A TimedState that stands a creature back up instead of ending a state.
    static const uint64		kStandUp = 0;
    void					_timeStandUp(CreatureObject* creature, uint64 when);

    static bool				mInsFlag;
    static CombatManager*	mSingleton;
    Database*				mDatabase;
    WeaponGroups			mWeaponGroups;
    CombatRules				mRules;
};

//=========================================================================================

#endif


