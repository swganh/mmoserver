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
    // Ends the creature's self buffs at once, without messages (cloning).
    void					clearSelfBuffs(CreatureObject* creature);

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
    // Whether the creature is bleeding; stopBleeding cures it (First Aid)
    // and returns false if it was not bleeding.
    bool					isBleeding(CreatureObject* creature) const;
    bool					stopBleeding(CreatureObject* creature);

    ~CombatManager();

private:

    CombatManager(Database* database);

    // bool					_verifyCombatState(CreatureObject* attacker,CreatureObject* defender);
    bool					_verifyCombatState(CreatureObject* attacker, uint64 defenderId);

    // secondary: another target of an area or cone special, which costs and
    // delays nothing more. Returns 0 hit, 1 miss, kNotPaid if the attacker
    // could not pay for the special.
    static const uint8		kNotPaid = 2;
    uint8					_executeAttack(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon, uint64* delayMs, bool secondary = false);
    // An area or cone special also hits every other target it reaches.
    void					_attackArea(CreatureObject* attacker, CreatureObject* defender, ObjectControllerCmdProperties* cmdProperties, Weapon* weapon);
    // The posture a special leaves the attacker in (Dive Shot: prone...).
    void					_setAttackerEndPosture(CreatureObject* attacker, ObjectControllerCmdProperties* cmdProperties);
    // Whether the attacker may fight this creature: alive, and an attackable
    // creature or a player duelling the attacker.
    bool					_canFight(CreatureObject* attacker, CreatureObject* target);
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
    // Bleeding: one per creature and pool.
    struct Bleed
    {
        uint64		creatureId;
        uint8		bar;		// HamBar_Health, _Action, _Mind
        int32		perTick;
        uint64		interval;	// ms
        uint64		nextTick;	// ms, Clock local time
        uint64		expires;	// ms, Clock local time
    };
    std::vector<Bleed>		mBleeds;
    // Each player's battle fatigue not yet whole (the rules give fractions).
    std::map<uint64, float>	mFatigueCarry;
    void					_startBleeding(CreatureObject* creature, uint8 bar, int32 perTick, float interval, float duration);
    void					_tickBleeding(uint64 now);
    // Removes the Bleeding state and tells a player, once no pool bleeds.
    void					_bleedingEnded(uint64 creatureId);
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


