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

#ifndef ANH_SCRIPTENGINE_COMBATRULES_H
#define ANH_SCRIPTENGINE_COMBATRULES_H

#include <map>
#include <string>
#include <vector>

struct lua_State;

// The combat rules script (data/script/combat/rules.lua): what an attack
// does. The zone fills a CombatAttack from its objects, the script works
// out the outcome. The script runs in its own Lua state, apart from the
// zone's scripts, and is called directly (no coroutines, no waiting).

struct CombatSide
{
    CombatSide() : kind("creature"), posture("standing"), armorRating(0), toHit(-1.0f) {}

    std::string                     kind;       // "player", "creature", "object"
    std::string                     posture;    // "standing", "kneeling", "prone", "other"
    std::map<std::string, int>      skillMods;
    int                             armorRating; // 0-3
    std::map<std::string, float>    armor;      // damage type -> effectiveness %
    float                           toHit;      // creature to-hit %, < 0 if unknown
    std::vector<std::string>        states;     // "intimidated", "stunned", ...
    std::map<std::string, float>    buffs;      // "berserk", "center_of_being" -> strength
};

struct CombatWeapon
{
    CombatWeapon()
        : group("unarmed"), damageType("kinetic"), minDamage(1), maxDamage(1), speed(0)
        , zeroMod(0), midMod(0), maxMod(0), midRange(0), maxRange(0), armorPiercing(0)
        , healthCost(0), actionCost(0), mindCost(0), woundChance(0) {}

    std::string group;          // "pistol", "carbine", "rifle", "unarmed", ...
    std::string damageType;     // "energy", "kinetic", ...
    float       minDamage;
    float       maxDamage;
    float       speed;          // seconds per attack, 0 if unknown
    float       zeroMod;
    float       midMod;
    float       maxMod;
    float       midRange;       // metres, 0 if unknown
    float       maxRange;       // metres, 0 if unknown
    int         armorPiercing;  // 0-3
    float       healthCost;     // the weapon's attack costs, for specials
    float       actionCost;
    float       mindCost;
    float       woundChance;    // % chance a hit wounds, 0 if unknown
};

// An effect a special can apply: its chance (%) and duration (seconds).
struct CombatEffect
{
    CombatEffect() : chance(0), duration(0) {}
    CombatEffect(float c, float d) : chance(c), duration(d) {}

    float chance;
    float duration;
};

struct CombatAttack
{
    CombatAttack()
        : distance(0), damageMultiplier(1), delayMultiplier(1), special(false), accuracyBonus(0)
        , healthCostMultiplier(0), actionCostMultiplier(0), mindCostMultiplier(0) {}

    CombatSide      attacker;
    CombatSide      defender;
    CombatWeapon    weapon;
    float           distance;
    float           damageMultiplier;
    float           delayMultiplier;
    std::string     pool;       // "" for the default attack
    bool            special;    // a special move, not the default attack
    float           accuracyBonus;
    float           healthCostMultiplier;
    float           actionCostMultiplier;
    float           mindCostMultiplier;
    std::map<std::string, CombatEffect> effects;    // "knockdown", "stun"...
};

struct CombatOutcome
{
    CombatOutcome() : ok(false), hit(false), chance(0), damage(0), pool("health"), delay(2.0f), wound(0), fatigue(0)
        , bleed(0), bleedInterval(0), bleedDuration(0) {}

    bool        ok;         // false if the script failed; the rest are defaults then
    bool        hit;
    float       chance;     // %
    int         damage;
    std::string pool;       // "health", "action", "mind"
    float       delay;      // seconds to the next attack
    int         wound;      // wound to the pool hit
    float       fatigue;    // battle fatigue the defender gains (fractions add up)
    int         bleed;          // the wounded pool loses this per tick...
    float       bleedInterval;  // ...every this many seconds...
    float       bleedDuration;  // ...for this many seconds
    // The effects that landed: name ("knockdown", "stun"...), seconds.
    std::vector<std::pair<std::string, float> > effects;
};

class CombatRules
{
public:
    CombatRules();
    ~CombatRules();

    // Loads the rules script; false (and logged) if it fails.
    bool            load(const std::string& path);
    bool            isLoaded() const { return mRules != 0; }

    CombatOutcome   attack(const CombatAttack& attack);
    bool            inRange(const CombatWeapon& weapon, float distance);
    // The weapon's max range in metres (rules.ranges), for area specials.
    bool            maxRange(const CombatWeapon& weapon, float& range);
    // What a special costs (rules.costs); false (and 0s) if no rules.
    bool            costs(const CombatAttack& attack, int& health, int& action, int& mind);
    // The count-th incap within the window (1 for the first): whether it
    // kills, else its length in seconds; and the window's length in seconds.
    bool            incap(int count, bool& dies, int& seconds);
    bool            incapWindow(int& seconds);
    // A brawler move (Intimidate, Warcry, Taunt): its kind ("intimidate",
    // "taunt") and reach in metres; and whether it lands on attack.defender,
    // its chance in % and its state's duration in seconds.
    bool            moveReach(const std::string& name, std::string& kind, float& range, float& radius);
    bool            move(const std::string& name, const CombatAttack& attack, float baseChance,
                         bool& landed, float& chance, float& duration);
    // A self buff (Berserk, Center of Being) for attack.attacker holding
    // attack.weapon: the buff, its strength and duration in seconds. False
    // if it cannot be used; reason says why ("melee", "skill", "unknown").
    bool            selfBuff(const std::string& name, const CombatAttack& attack, std::string& buff,
                             float& strength, float& duration, std::string& reason);
    // A scout's harvest of a creature table quantity with the given
    // creature_harvesting skill mod: units and scout XP.
    bool            harvest(int quantity, int skill, int& amount, int& xp);
    // The fields of the WeaponRanges message: layout ("i" int32, "f" float
    // per field) and values. False if no rules, the script returns nothing
    // (send nothing then) or the layout is not valid.
    bool            clientRanges(const CombatWeapon& weapon, std::string& layout, std::vector<double>& values);

    // For the tests: the Lua state, with the rules table in global "rules".
    lua_State*      state() { return mState; }

private:
    CombatRules(const CombatRules&);
    CombatRules& operator=(const CombatRules&);

    bool            _call(const char* function, int args, int results);

    lua_State*      mState;
    int             mRules;     // registry reference to the rules table, 0 if none
};

#endif
