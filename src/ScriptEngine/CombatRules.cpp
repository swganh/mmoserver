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

#include "CombatRules.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "Utils/logger.h"

#include <ctime>

namespace {

void setString(lua_State* l, const char* key, const std::string& value)
{
    lua_pushstring(l, value.c_str());
    lua_setfield(l, -2, key);
}

void setNumber(lua_State* l, const char* key, double value)
{
    lua_pushnumber(l, value);
    lua_setfield(l, -2, key);
}

void pushSide(lua_State* l, const CombatSide& side)
{
    lua_newtable(l);
    setString(l, "kind", side.kind);
    setString(l, "posture", side.posture);
    setNumber(l, "armor_rating", side.armorRating);
    if (side.toHit >= 0)
    {
        setNumber(l, "to_hit", side.toHit);
    }

    lua_newtable(l);
    for (std::map<std::string, int>::const_iterator it = side.skillMods.begin(); it != side.skillMods.end(); ++it)
    {
        setNumber(l, it->first.c_str(), it->second);
    }
    lua_setfield(l, -2, "skill_mods");

    lua_newtable(l);
    for (std::map<std::string, float>::const_iterator it = side.armor.begin(); it != side.armor.end(); ++it)
    {
        setNumber(l, it->first.c_str(), it->second);
    }
    lua_setfield(l, -2, "armor");

    lua_newtable(l);
    for (std::vector<std::string>::const_iterator it = side.states.begin(); it != side.states.end(); ++it)
    {
        lua_pushboolean(l, 1);
        lua_setfield(l, -2, it->c_str());
    }
    lua_setfield(l, -2, "states");

    lua_newtable(l);
    for (std::map<std::string, float>::const_iterator it = side.buffs.begin(); it != side.buffs.end(); ++it)
    {
        setNumber(l, it->first.c_str(), it->second);
    }
    lua_setfield(l, -2, "buffs");
}

void pushWeapon(lua_State* l, const CombatWeapon& weapon)
{
    lua_newtable(l);
    setString(l, "group", weapon.group);
    setString(l, "damage_type", weapon.damageType);
    setNumber(l, "min_damage", weapon.minDamage);
    setNumber(l, "max_damage", weapon.maxDamage);
    setNumber(l, "speed", weapon.speed);
    setNumber(l, "zero_mod", weapon.zeroMod);
    setNumber(l, "mid_mod", weapon.midMod);
    setNumber(l, "max_mod", weapon.maxMod);
    setNumber(l, "mid_range", weapon.midRange);
    setNumber(l, "max_range", weapon.maxRange);
    setNumber(l, "armor_piercing", weapon.armorPiercing);
    setNumber(l, "health_cost", weapon.healthCost);
    setNumber(l, "action_cost", weapon.actionCost);
    setNumber(l, "mind_cost", weapon.mindCost);
    setNumber(l, "wound_chance", weapon.woundChance);
}

void pushAttack(lua_State* l, const CombatAttack& attack)
{
    lua_newtable(l);
    pushSide(l, attack.attacker);
    lua_setfield(l, -2, "attacker");
    pushSide(l, attack.defender);
    lua_setfield(l, -2, "defender");
    pushWeapon(l, attack.weapon);
    lua_setfield(l, -2, "weapon");
    setNumber(l, "distance", attack.distance);
    setNumber(l, "damage_multiplier", attack.damageMultiplier);
    setNumber(l, "delay_multiplier", attack.delayMultiplier);
    if (!attack.pool.empty())
    {
        setString(l, "pool", attack.pool);
    }
    lua_pushboolean(l, attack.special ? 1 : 0);
    lua_setfield(l, -2, "special");
    setNumber(l, "accuracy_bonus", attack.accuracyBonus);

    lua_newtable(l);
    setNumber(l, "health", attack.healthCostMultiplier);
    setNumber(l, "action", attack.actionCostMultiplier);
    setNumber(l, "mind", attack.mindCostMultiplier);
    lua_setfield(l, -2, "cost_multipliers");

    lua_newtable(l);
    for (std::map<std::string, CombatEffect>::const_iterator it = attack.effects.begin(); it != attack.effects.end(); ++it)
    {
        lua_newtable(l);
        setNumber(l, "chance", it->second.chance);
        setNumber(l, "duration", it->second.duration);
        lua_setfield(l, -2, it->first.c_str());
    }
    lua_setfield(l, -2, "effects");
}

} // namespace

CombatRules::CombatRules()
    : mState(luaL_newstate())
    , mRules(0)
{
    luaL_openlibs(mState);
}

CombatRules::~CombatRules()
{
    lua_close(mState);
}

bool CombatRules::load(const std::string& path)
{
    if (mRules)
    {
        luaL_unref(mState, LUA_REGISTRYINDEX, mRules);
        mRules = 0;
    }

    if (luaL_loadfile(mState, path.c_str()) != 0 || lua_pcall(mState, 0, 1, 0) != 0)
    {
        LOG(WARNING) << "Combat rules: could not load " << path << ": " << lua_tostring(mState, -1);
        lua_pop(mState, 1);
        return false;
    }
    if (!lua_istable(mState, -1))
    {
        LOG(WARNING) << "Combat rules: " << path << " does not return a table";
        lua_pop(mState, 1);
        return false;
    }

    lua_pushvalue(mState, -1);
    lua_setglobal(mState, "rules");
    mRules = luaL_ref(mState, LUA_REGISTRYINDEX);

    // Each zone rolls its own dice.
    lua_getglobal(mState, "math");
    lua_getfield(mState, -1, "randomseed");
    lua_pushnumber(mState, static_cast<lua_Number>(std::time(0)));
    lua_pcall(mState, 1, 0, 0);
    lua_pop(mState, 1);

    LOG(INFO) << "Combat rules loaded from " << path;
    return true;
}

// Calls rules[function] with the args on the stack; leaves the results.
bool CombatRules::_call(const char* function, int args, int results)
{
    if (!mRules)
    {
        lua_pop(mState, args);
        return false;
    }

    lua_rawgeti(mState, LUA_REGISTRYINDEX, mRules);
    lua_getfield(mState, -1, function);
    lua_remove(mState, -2);
    if (!lua_isfunction(mState, -1))
    {
        LOG(WARNING) << "Combat rules: no function rules." << function;
        lua_pop(mState, args + 1);
        return false;
    }
    lua_insert(mState, -(args + 1));

    if (lua_pcall(mState, args, results, 0) != 0)
    {
        LOG(WARNING) << "Combat rules: rules." << function << " failed: " << lua_tostring(mState, -1);
        lua_pop(mState, 1);
        return false;
    }
    return true;
}

CombatOutcome CombatRules::attack(const CombatAttack& attack)
{
    CombatOutcome outcome;

    pushAttack(mState, attack);
    if (!_call("attack", 1, 1))
    {
        return outcome;
    }
    if (!lua_istable(mState, -1))
    {
        LOG(WARNING) << "Combat rules: rules.attack did not return a table";
        lua_pop(mState, 1);
        return outcome;
    }

    lua_getfield(mState, -1, "hit");
    outcome.hit = lua_toboolean(mState, -1) != 0;
    lua_pop(mState, 1);

    lua_getfield(mState, -1, "chance");
    outcome.chance = static_cast<float>(lua_tonumber(mState, -1));
    lua_pop(mState, 1);

    lua_getfield(mState, -1, "damage");
    outcome.damage = static_cast<int>(lua_tonumber(mState, -1));
    lua_pop(mState, 1);

    lua_getfield(mState, -1, "pool");
    if (lua_isstring(mState, -1))
    {
        outcome.pool = lua_tostring(mState, -1);
    }
    lua_pop(mState, 1);

    lua_getfield(mState, -1, "delay");
    if (lua_isnumber(mState, -1))
    {
        outcome.delay = static_cast<float>(lua_tonumber(mState, -1));
    }
    lua_pop(mState, 1);

    lua_getfield(mState, -1, "wound");
    outcome.wound = static_cast<int>(lua_tonumber(mState, -1));
    lua_pop(mState, 1);

    lua_getfield(mState, -1, "fatigue");
    outcome.fatigue = static_cast<float>(lua_tonumber(mState, -1));
    lua_pop(mState, 1);

    lua_getfield(mState, -1, "bleed");
    outcome.bleed = static_cast<int>(lua_tonumber(mState, -1));
    lua_pop(mState, 1);
    lua_getfield(mState, -1, "bleed_interval");
    outcome.bleedInterval = static_cast<float>(lua_tonumber(mState, -1));
    lua_pop(mState, 1);
    lua_getfield(mState, -1, "bleed_duration");
    outcome.bleedDuration = static_cast<float>(lua_tonumber(mState, -1));
    lua_pop(mState, 1);

    lua_getfield(mState, -1, "effects");
    if (lua_istable(mState, -1))
    {
        int count = static_cast<int>(lua_objlen(mState, -1));
        for (int i = 1; i <= count; ++i)
        {
            lua_rawgeti(mState, -1, i);
            if (lua_istable(mState, -1))
            {
                lua_getfield(mState, -1, "name");
                lua_getfield(mState, -2, "duration");
                if (lua_isstring(mState, -2))
                {
                    outcome.effects.push_back(std::make_pair(std::string(lua_tostring(mState, -2)),
                                              static_cast<float>(lua_tonumber(mState, -1))));
                }
                lua_pop(mState, 2);
            }
            lua_pop(mState, 1);
        }
    }
    lua_pop(mState, 1);

    lua_pop(mState, 1);
    outcome.ok = true;
    return outcome;
}

bool CombatRules::costs(const CombatAttack& attack, int& health, int& action, int& mind)
{
    health = action = mind = 0;
    pushAttack(mState, attack);
    if (!_call("costs", 1, 3))
    {
        return false;
    }
    health = static_cast<int>(lua_tonumber(mState, -3));
    action = static_cast<int>(lua_tonumber(mState, -2));
    mind = static_cast<int>(lua_tonumber(mState, -1));
    lua_pop(mState, 3);
    return true;
}

bool CombatRules::incap(int count, bool& dies, int& seconds)
{
    lua_pushnumber(mState, count);
    if (!_call("incap", 1, 2))
    {
        return false;
    }
    dies = lua_toboolean(mState, -2) != 0;
    seconds = static_cast<int>(lua_tonumber(mState, -1));
    lua_pop(mState, 2);
    return true;
}

bool CombatRules::moveReach(const std::string& name, std::string& kind, float& range, float& radius)
{
    lua_pushstring(mState, name.c_str());
    if (!_call("moveReach", 1, 3))
    {
        return false;
    }
    bool known = lua_isstring(mState, -3) != 0;
    if (known)
    {
        kind = lua_tostring(mState, -3);
        range = static_cast<float>(lua_tonumber(mState, -2));
        radius = static_cast<float>(lua_tonumber(mState, -1));
    }
    lua_pop(mState, 3);
    return known;
}

bool CombatRules::move(const std::string& name, const CombatAttack& attack, float baseChance,
                       bool& landed, float& chance, float& duration)
{
    lua_pushstring(mState, name.c_str());
    pushAttack(mState, attack);
    setNumber(mState, "base_chance", baseChance);
    lua_getglobal(mState, "math");
    lua_getfield(mState, -1, "random");
    lua_remove(mState, -2);
    if (lua_pcall(mState, 0, 1, 0) != 0)
    {
        lua_pop(mState, 3);
        return false;
    }
    if (!_call("move", 3, 3))
    {
        return false;
    }
    landed = lua_toboolean(mState, -3) != 0;
    chance = static_cast<float>(lua_tonumber(mState, -2));
    duration = static_cast<float>(lua_tonumber(mState, -1));
    lua_pop(mState, 3);
    return true;
}

bool CombatRules::selfBuff(const std::string& name, const CombatAttack& attack, std::string& buff,
                           float& strength, float& duration, std::string& reason)
{
    lua_pushstring(mState, name.c_str());
    pushAttack(mState, attack);
    if (!_call("selfBuff", 2, 3))
    {
        reason = "unknown";
        return false;
    }
    bool ok = lua_isstring(mState, -3) != 0;
    if (ok)
    {
        buff = lua_tostring(mState, -3);
        strength = static_cast<float>(lua_tonumber(mState, -2));
        duration = static_cast<float>(lua_tonumber(mState, -1));
    }
    else
    {
        reason = lua_isstring(mState, -2) ? lua_tostring(mState, -2) : "unknown";
    }
    lua_pop(mState, 3);
    return ok;
}

bool CombatRules::harvest(int quantity, int skill, int& amount, int& xp)
{
    lua_pushnumber(mState, quantity);
    lua_pushnumber(mState, skill);
    if (!_call("harvest", 2, 2))
    {
        return false;
    }
    amount = static_cast<int>(lua_tonumber(mState, -2));
    xp = static_cast<int>(lua_tonumber(mState, -1));
    lua_pop(mState, 2);
    return true;
}

bool CombatRules::incapWindow(int& seconds)
{
    if (!_call("incapWindow", 0, 1))
    {
        return false;
    }
    seconds = static_cast<int>(lua_tonumber(mState, -1));
    lua_pop(mState, 1);
    return true;
}

bool CombatRules::maxRange(const CombatWeapon& weapon, float& range)
{
    pushWeapon(mState, weapon);
    if (!_call("ranges", 1, 2))
    {
        return false;
    }
    range = static_cast<float>(lua_tonumber(mState, -1));
    lua_pop(mState, 2);
    return true;
}

bool CombatRules::inRange(const CombatWeapon& weapon, float distance)
{
    pushWeapon(mState, weapon);
    lua_pushnumber(mState, distance);
    if (!_call("inRange", 2, 1))
    {
        // No rules: the weapon's own range, else the old fixed reach.
        float reach = weapon.maxRange;
        if (reach <= 0)
        {
            reach = 6.0f;
            if (weapon.group == "pistol") reach = 35.0f;
            else if (weapon.group == "carbine") reach = 50.0f;
            else if (weapon.group == "rifle") reach = 64.0f;
        }
        return distance <= reach;
    }
    bool result = lua_toboolean(mState, -1) != 0;
    lua_pop(mState, 1);
    return result;
}

bool CombatRules::clientRanges(const CombatWeapon& weapon, std::string& layout, std::vector<double>& values)
{
    static const int kMaxFields = 8;

    layout.clear();
    values.clear();

    int top = lua_gettop(mState);
    pushWeapon(mState, weapon);
    if (!_call("clientRanges", 1, LUA_MULTRET))
    {
        lua_settop(mState, top);
        return false;
    }
    int results = lua_gettop(mState) - top;
    bool ok = false;
    if (results >= 1 && lua_type(mState, top + 1) == LUA_TSTRING)
    {
        layout = lua_tostring(mState, top + 1);
        ok = !layout.empty() && static_cast<int>(layout.size()) <= kMaxFields
            && static_cast<int>(layout.size()) == results - 1
            && layout.find_first_not_of("if") == std::string::npos;
        for (int i = 2; ok && i <= results; ++i)
        {
            ok = lua_isnumber(mState, top + i) != 0;
            values.push_back(lua_tonumber(mState, top + i));
        }
        if (!ok)
        {
            LOG(WARNING) << "Combat rules: rules.clientRanges returned a bad layout \"" << layout
                         << "\" or values (" << results - 1 << " values)";
        }
    }
    lua_settop(mState, top);
    if (!ok)
    {
        layout.clear();
        values.clear();
    }
    return ok;
}
