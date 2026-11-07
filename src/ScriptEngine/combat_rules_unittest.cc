/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html
---------------------------------------------------------------------------------------
*/

// data/script/combat/rules.lua: the maths, through its functions, and the
// C++ side that passes an attack in and reads the outcome.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "ScriptEngine/CombatRules.h"

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
}

namespace {

const char* kRules = MMOSERVER_SOURCE_DIR "/data/script/combat/rules.lua";

// The noob CDEF pistol new characters get (An Empire Divided p. 392).
CombatWeapon noobCdef() {
    CombatWeapon w;
    w.group = "pistol";
    w.damageType = "energy";
    w.minDamage = 50;
    w.maxDamage = 80;
    w.speed = 4;
    w.zeroMod = -10;
    w.midMod = 30;
    w.maxMod = -90;
    w.midRange = 15;
    w.maxRange = 64;
    return w;
}

class CombatRulesTest : public ::testing::Test {
protected:
    void SetUp() {
        ASSERT_TRUE(rules_.load(kRules));
    }

    // Evaluates a Lua expression against the loaded rules; returns a number.
    double eval(const std::string& expression) {
        lua_State* l = rules_.state();
        std::string code = "return " + expression;
        if (luaL_loadstring(l, code.c_str()) != 0 || lua_pcall(l, 0, 1, 0) != 0) {
            ADD_FAILURE() << expression << ": " << lua_tostring(l, -1);
            lua_pop(l, 1);
            return -1e9;
        }
        double value = lua_tonumber(l, -1);
        lua_pop(l, 1);
        return value;
    }

    std::string evalString(const std::string& expression) {
        lua_State* l = rules_.state();
        std::string code = "return " + expression;
        if (luaL_loadstring(l, code.c_str()) != 0 || lua_pcall(l, 0, 1, 0) != 0) {
            ADD_FAILURE() << expression << ": " << lua_tostring(l, -1);
            lua_pop(l, 1);
            return "";
        }
        std::string value = lua_tostring(l, -1) ? lua_tostring(l, -1) : "";
        lua_pop(l, 1);
        return value;
    }

    void run(const std::string& code) {
        lua_State* l = rules_.state();
        if (luaL_loadstring(l, code.c_str()) != 0 || lua_pcall(l, 0, 0, 0) != 0) {
            ADD_FAILURE() << code << ": " << lua_tostring(l, -1);
            lua_pop(l, 1);
        }
    }

    CombatRules rules_;
};

const char* kCdefLua =
    "cdef = { group = 'pistol', damage_type = 'energy', min_damage = 50, max_damage = 80,"
    " speed = 4, zero_mod = -10, mid_mod = 30, max_mod = -90, mid_range = 15, max_range = 64 }";

TEST_F(CombatRulesTest, RangeModFollowsTheWeaponsCurve) {
    run(kCdefLua);
    EXPECT_DOUBLE_EQ(-10, eval("rules.rangeMod(cdef, 0)"));
    EXPECT_DOUBLE_EQ(10, eval("rules.rangeMod(cdef, 7.5)"));
    EXPECT_DOUBLE_EQ(30, eval("rules.rangeMod(cdef, 15)"));
    EXPECT_DOUBLE_EQ(-30, eval("rules.rangeMod(cdef, 39.5)"));
    EXPECT_DOUBLE_EQ(-90, eval("rules.rangeMod(cdef, 64)"));
    EXPECT_DOUBLE_EQ(-90, eval("rules.rangeMod(cdef, 100)"));
}

TEST_F(CombatRulesTest, WeaponsWithoutRangeDataUseTheGroupDefaults) {
    run("old = { group = 'pistol', zero_mod = 0, mid_mod = 30, max_mod = -80 }");
    EXPECT_DOUBLE_EQ(30, eval("rules.rangeMod(old, 15)"));
    EXPECT_DOUBLE_EQ(1, eval("rules.inRange(old, 64) and 1 or 0"));
    EXPECT_DOUBLE_EQ(0, eval("rules.inRange(old, 70) and 1 or 0"));
    run("knife = { group = 'onehandmelee' }");
    EXPECT_DOUBLE_EQ(1, eval("rules.inRange(knife, 5.5) and 1 or 0"));
    EXPECT_DOUBLE_EQ(0, eval("rules.inRange(knife, 7) and 1 or 0"));
}

TEST_F(CombatRulesTest, AccuracyAddsSkillModsAndPostures) {
    run(kCdefLua);
    // Novice Marksman: +10 pistol accuracy (p. 145).
    run("a = { weapon = cdef, distance = 15,"
        " attacker = { posture = 'standing', skill_mods = { pistol_accuracy = 10 } },"
        " defender = { posture = 'standing' } }");
    EXPECT_DOUBLE_EQ(40, eval("rules.accuracy(a)"));
    EXPECT_DOUBLE_EQ(90, eval("rules.hitChance(a)"));

    run("a.attacker.posture = 'kneeling'");
    EXPECT_DOUBLE_EQ(50, eval("rules.accuracy(a)"));
    // Ranged attacks against a prone target are harder.
    run("a.defender.posture = 'prone'");
    EXPECT_DOUBLE_EQ(30, eval("rules.accuracy(a)"));
    // Melee attacks against a prone target are easier.
    run("a.weapon = { group = 'onehandmelee' }; a.distance = 1; a.attacker.posture = 'standing'");
    EXPECT_DOUBLE_EQ(20, eval("rules.accuracy(a)"));
}

TEST_F(CombatRulesTest, DefenceModsLowerTheChance) {
    run(kCdefLua);
    run("a = { weapon = cdef, distance = 15, attacker = {},"
        " defender = { skill_mods = { ranged_defense = 25, melee_defense = 100 } } }");
    EXPECT_DOUBLE_EQ(5, eval("rules.accuracy(a)"));
}

TEST_F(CombatRulesTest, HitChanceIsBounded) {
    run(kCdefLua);
    run("far = { weapon = cdef, distance = 64, attacker = {}, defender = {} }");
    EXPECT_DOUBLE_EQ(5, eval("rules.hitChance(far)"));
    run("near = { weapon = cdef, distance = 15, attacker = { posture = 'prone',"
        " skill_mods = { pistol_accuracy = 100 } }, defender = {} }");
    EXPECT_DOUBLE_EQ(95, eval("rules.hitChance(near)"));
    // A creature's own to-hit replaces the base chance.
    run("bite = { weapon = { group = 'unarmed' }, distance = 1,"
        " attacker = { kind = 'creature', to_hit = 28 }, defender = {} }");
    EXPECT_DOUBLE_EQ(28, eval("rules.hitChance(bite)"));
}

TEST_F(CombatRulesTest, ArmourRatingFollowsTheGuide) {
    // p. 388: 100 damage becomes 156 two levels above, 25 two levels below.
    EXPECT_NEAR(1.5625, eval("rules.armorRatingFactor(2, 0)"), 1e-9);
    EXPECT_DOUBLE_EQ(1.25, eval("rules.armorRatingFactor(3, 2)"));
    EXPECT_DOUBLE_EQ(1, eval("rules.armorRatingFactor(1, 1)"));
    EXPECT_DOUBLE_EQ(0.5, eval("rules.armorRatingFactor(0, 1)"));
    EXPECT_DOUBLE_EQ(0.25, eval("rules.armorRatingFactor(0, 2)"));
}

TEST_F(CombatRulesTest, DamageSpansTheWeaponAndAppliesArmour) {
    run(kCdefLua);
    run("a = { weapon = cdef, attacker = {}, defender = {} }");
    EXPECT_DOUBLE_EQ(50, eval("rules.damage(a, 0)"));
    EXPECT_DOUBLE_EQ(65, eval("rules.damage(a, 0.5)"));
    EXPECT_DOUBLE_EQ(80, eval("rules.damage(a, 1)"));
    // A special's damage multiplier.
    run("a.damage_multiplier = 2");
    EXPECT_DOUBLE_EQ(130, eval("rules.damage(a, 0.5)"));
    // Light armour against an unrated weapon: half; 40% energy effectiveness.
    run("a.damage_multiplier = 1; a.defender = { armor_rating = 1, armor = { energy = 40, kinetic = 90 } }");
    EXPECT_DOUBLE_EQ(20, eval("rules.damage(a, 0.5)"));
    // Never less than 1.
    run("a.defender = { armor = { energy = 100 } }");
    EXPECT_DOUBLE_EQ(1, eval("rules.damage(a, 0.5)"));
}

TEST_F(CombatRulesTest, PoolsFollowTheWeights) {
    run("a = {}");
    EXPECT_EQ("health", evalString("rules.pool(a, 0)"));
    EXPECT_EQ("health", evalString("rules.pool(a, 0.49)"));
    EXPECT_EQ("action", evalString("rules.pool(a, 0.5)"));
    EXPECT_EQ("mind", evalString("rules.pool(a, 0.8)"));
    EXPECT_EQ("mind", evalString("rules.pool(a, 0.9999)"));
    run("a.pool = 'mind'");
    EXPECT_EQ("mind", evalString("rules.pool(a, 0)"));
}

TEST_F(CombatRulesTest, DelayIsTheWeaponsSpeedLessSpeedMods) {
    run(kCdefLua);
    run("a = { weapon = cdef, attacker = {} }");
    EXPECT_DOUBLE_EQ(4, eval("rules.delay(a)"));
    // Novice Marksman: +5 pistol speed (p. 145).
    run("a.attacker.skill_mods = { pistol_speed = 5 }");
    EXPECT_NEAR(3.8, eval("rules.delay(a)"), 1e-9);
    run("a.delay_multiplier = 2");
    EXPECT_NEAR(7.6, eval("rules.delay(a)"), 1e-9);
    run("a.weapon = { group = 'unarmed' }; a.delay_multiplier = 0.1");
    EXPECT_DOUBLE_EQ(1, eval("rules.delay(a)"));
}

TEST_F(CombatRulesTest, AttackFromCpp) {
    CombatAttack attack;
    attack.attacker.kind = "player";
    attack.attacker.skillMods["pistol_accuracy"] = 10;
    attack.attacker.skillMods["pistol_speed"] = 5;
    attack.defender.kind = "creature";
    attack.weapon = noobCdef();
    attack.distance = 15;

    int hits = 0;
    for (int i = 0; i < 1000; ++i) {
        CombatOutcome outcome = rules_.attack(attack);
        ASSERT_TRUE(outcome.ok);
        EXPECT_FLOAT_EQ(90, outcome.chance);
        EXPECT_FLOAT_EQ(3.8f, outcome.delay);
        if (outcome.hit) {
            ++hits;
            EXPECT_GE(outcome.damage, 50);
            EXPECT_LE(outcome.damage, 80);
            EXPECT_TRUE(outcome.pool == "health" || outcome.pool == "action" || outcome.pool == "mind")
                << outcome.pool;
        } else {
            EXPECT_EQ(0, outcome.damage);
        }
    }
    EXPECT_GT(hits, 850);
    EXPECT_LT(hits, 950);

    // A special naming its pool.
    attack.pool = "action";
    attack.distance = 0;
    attack.attacker.posture = "prone";
    attack.attacker.skillMods["pistol_accuracy"] = 200;
    CombatOutcome outcome = rules_.attack(attack);
    ASSERT_TRUE(outcome.ok);
    if (outcome.hit) {
        EXPECT_EQ("action", outcome.pool);
    }
}

TEST_F(CombatRulesTest, InRangeFromCpp) {
    CombatWeapon cdef = noobCdef();
    EXPECT_TRUE(rules_.inRange(cdef, 64));
    EXPECT_FALSE(rules_.inRange(cdef, 66));
}

TEST_F(CombatRulesTest, ClientRangesFromCpp) {
    std::string layout;
    std::vector<double> values;
    // The weapon's own values, not the probe in use while the message is
    // worked out.
    ASSERT_EQ(0, luaL_dostring(rules_.state(), "rules.CLIENT_RANGES_PROBE = nil"));
    ASSERT_TRUE(rules_.clientRanges(noobCdef(), layout, values));
    ASSERT_EQ(5u, layout.size());
    ASSERT_EQ(5u, values.size());
    EXPECT_DOUBLE_EQ(-10, values[0]);
    EXPECT_DOUBLE_EQ(15, values[1]);
    EXPECT_DOUBLE_EQ(30, values[2]);
    EXPECT_DOUBLE_EQ(64, values[3]);
    EXPECT_DOUBLE_EQ(-90, values[4]);

    // A weapon without ranges in metres gets its group's.
    CombatWeapon old = noobCdef();
    old.midRange = 0;
    old.maxRange = 0;
    ASSERT_TRUE(rules_.clientRanges(old, layout, values));
    EXPECT_DOUBLE_EQ(15, values[1]);
    EXPECT_DOUBLE_EQ(64, values[3]);

    lua_State* l = rules_.state();
    // Any layout of i and f, as long as it matches the values.
    ASSERT_EQ(0, luaL_dostring(l, "rules.clientRanges = function() return 'ifi', 1, 2.5, 3 end"));
    ASSERT_TRUE(rules_.clientRanges(noobCdef(), layout, values));
    EXPECT_EQ("ifi", layout);
    EXPECT_DOUBLE_EQ(2.5, values[1]);
    // A layout that does not match, or other letters: nothing sent.
    ASSERT_EQ(0, luaL_dostring(l, "rules.clientRanges = function() return 'ii', 1 end"));
    EXPECT_FALSE(rules_.clientRanges(noobCdef(), layout, values));
    ASSERT_EQ(0, luaL_dostring(l, "rules.clientRanges = function() return 'iq', 1, 2 end"));
    EXPECT_FALSE(rules_.clientRanges(noobCdef(), layout, values));
    // A script that returns nothing turns the message off.
    ASSERT_EQ(0, luaL_dostring(l, "rules.clientRanges = function() end"));
    EXPECT_FALSE(rules_.clientRanges(noobCdef(), layout, values));
    EXPECT_EQ(0, lua_gettop(l));
}

TEST(CombatRulesLoadTest, MissingScriptFallsBack) {
    CombatRules rules;
    EXPECT_FALSE(rules.load("no/such/rules.lua"));
    EXPECT_FALSE(rules.isLoaded());
    CombatOutcome outcome = rules.attack(CombatAttack());
    EXPECT_FALSE(outcome.ok);
    CombatWeapon pistol;
    pistol.group = "pistol";
    EXPECT_TRUE(rules.inRange(pistol, 30));
    EXPECT_FALSE(rules.inRange(pistol, 40));
    std::string layout;
    std::vector<double> values;
    EXPECT_FALSE(rules.clientRanges(pistol, layout, values));
}

} // namespace
