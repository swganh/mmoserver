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

#include <cstdio>
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
    EXPECT_DOUBLE_EQ(1, eval("rules.inRange(knife, 6.5) and 1 or 0"));
    EXPECT_DOUBLE_EQ(0, eval("rules.inRange(knife, 7.5) and 1 or 0"));
}

TEST_F(CombatRulesTest, AccuracyAddsSkillModsAndPostures) {
    run(kCdefLua);
    // Novice Marksman: +10 pistol accuracy (p. 145).
    // 17 m apart: 15 m once the bodies are allowed for, the ideal range.
    run("a = { weapon = cdef, distance = 17,"
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

// The target window with the noob CDEF, a marksman standing against a womp
// rat (docs/COMBAT.md): -15 at 2 m, +3 at 8 m, +23 at 16 m, -31 at 40 m,
// the client adding its own -5. The rules' modifier at the same distances.
TEST_F(CombatRulesTest, RangeModMatchesTheTargetWindow) {
    run(kCdefLua);
    run("function at(d) return rules.accuracy({ weapon = cdef, distance = d, attacker = {}, defender = {} }) - 5 end");
    EXPECT_NEAR(-15, eval("at(2)"), 2);
    EXPECT_NEAR(3, eval("at(8)"), 2);
    EXPECT_NEAR(23, eval("at(16)"), 2);
    EXPECT_NEAR(-31, eval("at(40)"), 2);
}

TEST_F(CombatRulesTest, DefenceModsLowerTheChance) {
    run(kCdefLua);
    run("a = { weapon = cdef, distance = 17, attacker = {},"
        " defender = { skill_mods = { ranged_defense = 25, melee_defense = 100 } } }");
    EXPECT_DOUBLE_EQ(5, eval("rules.accuracy(a)"));
}

TEST_F(CombatRulesTest, HitChanceIsBounded) {
    run(kCdefLua);
    run("far = { weapon = cdef, distance = 64, attacker = {}, defender = {} }");
    EXPECT_DOUBLE_EQ(5, eval("rules.hitChance(far)"));
    run("near = { weapon = cdef, distance = 17, attacker = { posture = 'prone',"
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
    attack.distance = 17;

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

TEST_F(CombatRulesTest, MaxRangeFromCpp) {
    // Area specials reach the weapon's max range: the CDEF's 64 m, bare
    // hands the group default.
    float range = 0;
    ASSERT_TRUE(rules_.maxRange(noobCdef(), range));
    EXPECT_FLOAT_EQ(64, range);
    CombatWeapon hands;
    hands.group = "unarmed";
    ASSERT_TRUE(rules_.maxRange(hands, range));
    EXPECT_GT(range, 0);
}

TEST_F(CombatRulesTest, InRangeFromCpp) {
    CombatWeapon cdef = noobCdef();
    EXPECT_TRUE(rules_.inRange(cdef, 64));
    EXPECT_TRUE(rules_.inRange(cdef, 66));
    EXPECT_FALSE(rules_.inRange(cdef, 67));
}

TEST_F(CombatRulesTest, ClientRangesFromCpp) {
    std::string layout;
    std::vector<double> values;
    ASSERT_TRUE(rules_.clientRanges(noobCdef(), layout, values));
    EXPECT_EQ("ffiii", layout);
    ASSERT_EQ(5u, values.size());
    EXPECT_DOUBLE_EQ(15, values[0]);
    EXPECT_DOUBLE_EQ(64, values[1]);
    EXPECT_DOUBLE_EQ(-10, values[2]);
    EXPECT_DOUBLE_EQ(30, values[3]);
    EXPECT_DOUBLE_EQ(-90, values[4]);

    // A weapon without ranges in metres gets its group's.
    CombatWeapon old = noobCdef();
    old.midRange = 0;
    old.maxRange = 0;
    ASSERT_TRUE(rules_.clientRanges(old, layout, values));
    EXPECT_DOUBLE_EQ(15, values[0]);
    EXPECT_DOUBLE_EQ(64, values[1]);

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

TEST_F(CombatRulesTest, SpecialsCostTheWeaponsCostsTimesTheirModifiers) {
    // Overcharge Shot 1 with the noob CDEF (10/20/10 costs, p. 392):
    // modifiers 0.5, 0.5, 0.5 (p. 80).
    CombatAttack attack;
    attack.weapon = noobCdef();
    attack.weapon.healthCost = 10;
    attack.weapon.actionCost = 20;
    attack.weapon.mindCost = 10;
    attack.healthCostMultiplier = 0.5f;
    attack.actionCostMultiplier = 0.5f;
    attack.mindCostMultiplier = 0.5f;
    int health = -1, action = -1, mind = -1;

    // The default attack costs nothing (p. 389).
    ASSERT_TRUE(rules_.costs(attack, health, action, mind));
    EXPECT_EQ(0, health + action + mind);

    attack.special = true;
    ASSERT_TRUE(rules_.costs(attack, health, action, mind));
    EXPECT_EQ(5, health);
    EXPECT_EQ(10, action);
    EXPECT_EQ(5, mind);
}

TEST_F(CombatRulesTest, BareHandsCostTheDefaultUnarmedCosts) {
    // Unarmed Lunge 1 (1, 1, 1; p. 82) with bare hands, which have no
    // attack costs: the vibroknuckler's 15/25/15 (p. 390) stand in.
    CombatAttack attack;
    attack.weapon.group = "unarmed";
    attack.special = true;
    attack.healthCostMultiplier = 1;
    attack.actionCostMultiplier = 1;
    attack.mindCostMultiplier = 1;
    int health = -1, action = -1, mind = -1;
    ASSERT_TRUE(rules_.costs(attack, health, action, mind));
    EXPECT_EQ(15, health);
    EXPECT_EQ(25, action);
    EXPECT_EQ(15, mind);
}

TEST_F(CombatRulesTest, PostureChangesLastFiveSeconds) {
    run("a = { defender = {}, effects = { posture_down = { chance = 100, duration = 0 } } }");
    run("landed = rules.effects(a, function() return 0.5 end)");
    EXPECT_DOUBLE_EQ(1, eval("#landed"));
    EXPECT_DOUBLE_EQ(5, eval("landed[1].duration"));
}

TEST_F(CombatRulesTest, HitsWoundWithTheWeaponsWoundChance) {
    // A dagger's 9% (p. 390): a roll under it wounds a tenth of the damage.
    run("a = { weapon = { group = 'onehandmelee', wound_chance = 9 } }");
    EXPECT_DOUBLE_EQ(5, eval("rules.wound(a, 50, 0.05)"));
    EXPECT_DOUBLE_EQ(0, eval("rules.wound(a, 50, 0.5)"));
    EXPECT_DOUBLE_EQ(1, eval("rules.wound(a, 4, 0.05)"));
    EXPECT_DOUBLE_EQ(0, eval("rules.wound(a, 0, 0.05)"));
    // Bare hands have no wound chance: the vibroknuckler's 8% (p. 390).
    run("b = { weapon = { group = 'unarmed' } }");
    EXPECT_DOUBLE_EQ(3, eval("rules.wound(b, 30, 0.07)"));
    EXPECT_DOUBLE_EQ(0, eval("rules.wound(b, 30, 0.09)"));
}

TEST_F(CombatRulesTest, WoundsAndFatigueReachCpp) {
    CombatAttack attack;
    attack.weapon.group = "rifle";
    attack.weapon.minDamage = 100;
    attack.weapon.maxDamage = 100;
    attack.weapon.woundChance = 100;
    attack.accuracyBonus = 1000;
    run("math.random = function(a) if a then return a end return 0 end");
    CombatOutcome outcome = rules_.attack(attack);
    ASSERT_TRUE(outcome.ok);
    ASSERT_TRUE(outcome.hit);
    EXPECT_EQ(outcome.damage / 10, outcome.wound);
    EXPECT_GT(outcome.wound, 0);
    EXPECT_FLOAT_EQ(0.25f, outcome.fatigue);
    // The wound makes the pool bleed: half the wound every 5 s for 30 s.
    EXPECT_EQ(outcome.wound / 2, outcome.bleed);
    EXPECT_FLOAT_EQ(5, outcome.bleedInterval);
    EXPECT_FLOAT_EQ(30, outcome.bleedDuration);
}

TEST_F(CombatRulesTest, OnlyWoundsBleed) {
    run("a = {}");
    run("bleed, interval, duration = rules.bleed(a, 'health', 0)");
    EXPECT_DOUBLE_EQ(0, eval("bleed"));
    run("bleed, interval, duration = rules.bleed(a, 'mind', 1)");
    EXPECT_DOUBLE_EQ(1, eval("bleed"));
    EXPECT_DOUBLE_EQ(5, eval("interval"));
    EXPECT_DOUBLE_EQ(30, eval("duration"));
}

TEST_F(CombatRulesTest, IntimidatedDefendersAreEasierToHit) {
    run("a = { attacker = {}, defender = {} }");
    double plain = eval("rules.accuracy(a)");
    run("a.defender.states = { intimidated = true }");
    EXPECT_DOUBLE_EQ(plain + 10, eval("rules.accuracy(a)"));
}

TEST_F(CombatRulesTest, BrawlerMovesRollTheirChance) {
    // Intimidate 1: the command table's 25%, + the brawler's intimidate mod,
    // - the defender's intimidate_defense.
    run("a = { base_chance = 25, attacker = { skill_mods = { intimidate = 20 } },"
        " defender = { skill_mods = { intimidate_defense = 5 } } }");
    run("landed, chance, duration = rules.move('intimidate1', a, 0.39)");
    EXPECT_DOUBLE_EQ(40, eval("chance"));
    EXPECT_DOUBLE_EQ(1, eval("landed and 1 or 0"));
    EXPECT_DOUBLE_EQ(30, eval("duration"));
    run("landed = rules.move('intimidate1', a, 0.41)");
    EXPECT_DOUBLE_EQ(0, eval("landed and 1 or 0"));
    // Taunt has no state; no move is certain.
    run("b = { attacker = { skill_mods = { taunt = 100 } }, defender = {} }");
    run("landed, chance, duration = rules.move('taunt', b, 0)");
    EXPECT_DOUBLE_EQ(95, eval("chance"));
    EXPECT_DOUBLE_EQ(0, eval("duration"));

    std::string kind;
    float range = 0, radius = 0;
    ASSERT_TRUE(rules_.moveReach("warcry1", kind, range, radius));
    EXPECT_EQ("intimidate", kind);
    EXPECT_FLOAT_EQ(15, radius);
    EXPECT_FALSE(rules_.moveReach("berserk1", kind, range, radius));

    CombatAttack attack;
    bool landed = false;
    float chance = 0, duration = 0;
    ASSERT_TRUE(rules_.move("intimidate2", attack, 40, landed, chance, duration));
    EXPECT_FLOAT_EQ(40, chance);
}

TEST_F(CombatRulesTest, BerserkHitsHarderFasterAndIsEasierToHit) {
    // Berserk 1: strength 10, + the berserk mod (Master Brawler 20).
    run("a = { attacker = { skill_mods = { berserk = 20 } } }");
    run("buff, strength, duration = rules.selfBuff('berserk1', a)");
    EXPECT_EQ(1, eval("buff == 'berserk' and 1 or 0"));
    EXPECT_DOUBLE_EQ(30, eval("strength"));
    EXPECT_DOUBLE_EQ(20, eval("duration"));

    run(kCdefLua);
    run("b = { weapon = cdef, attacker = {}, defender = {} }");
    double accuracy = eval("rules.accuracy(b)");
    double delay = eval("rules.delay(b)");
    double damage = eval("rules.damage(b, 1)");
    run("b.attacker.buffs = { berserk = 30 }");
    EXPECT_DOUBLE_EQ(accuracy + 30, eval("rules.accuracy(b)"));
    EXPECT_NEAR(delay * 0.7, eval("rules.delay(b)"), 1e-9);
    EXPECT_GT(eval("rules.damage(b, 1)"), damage);
    // A berserk defender is easier to hit.
    run("b.attacker.buffs = nil; b.defender.buffs = { berserk = 10 }");
    EXPECT_DOUBLE_EQ(accuracy + 10, eval("rules.accuracy(b)"));
}

TEST_F(CombatRulesTest, CenterOfBeingNeedsMeleeAndRaisesDefence) {
    // Novice Brawler: unarmed efficacy 10, duration 5.
    run("a = { weapon = { group = 'unarmed' }, attacker = { skill_mods ="
        " { unarmed_center_of_being_efficacy = 10, center_of_being_duration_unarmed = 5 } } }");
    run("buff, strength, duration = rules.selfBuff('centerofbeing', a)");
    EXPECT_EQ(1, eval("buff == 'center_of_being' and 1 or 0"));
    EXPECT_DOUBLE_EQ(10, eval("strength"));
    EXPECT_DOUBLE_EQ(15, eval("duration"));
    run("a.weapon.group = 'pistol'");
    run("buff, reason = rules.selfBuff('centerofbeing', a)");
    EXPECT_EQ(1, eval("buff == nil and reason == 'melee' and 1 or 0"));

    run("b = { attacker = {}, defender = { buffs = { center_of_being = 10 } } }");
    double centered = eval("rules.accuracy(b)");
    run("b.defender.buffs = nil");
    EXPECT_DOUBLE_EQ(centered + 10, eval("rules.accuracy(b)"));

    CombatAttack attack;
    attack.weapon.group = "unarmed";
    std::string buff, reason;
    float strength = 0, duration = 0;
    EXPECT_FALSE(rules_.selfBuff("centerofbeing", attack, buff, strength, duration, reason));
    EXPECT_EQ("skill", reason);
    attack.attacker.skillMods["berserk"] = 5;
    ASSERT_TRUE(rules_.selfBuff("berserk2", attack, buff, strength, duration, reason));
    EXPECT_EQ("berserk", buff);
    EXPECT_FLOAT_EQ(25, strength);
    // Buffs reach the rules from C++.
    attack.weapon = noobCdef();
    attack.distance = 15;
    CombatOutcome plain = rules_.attack(attack);
    attack.defender.buffs["center_of_being"] = 10;
    CombatOutcome withBuff = rules_.attack(attack);
    ASSERT_TRUE(plain.ok && withBuff.ok);
    EXPECT_FLOAT_EQ(plain.chance - 10, withBuff.chance);
}

TEST_F(CombatRulesTest, HarvestGivesTheTableQuantityPlusSkill) {
    int amount = 0, xp = 0;
    // Womp rat meat (4) by a Novice Scout (creature_harvesting 15).
    ASSERT_TRUE(rules_.harvest(4, 15, amount, xp));
    EXPECT_EQ(4, amount);
    EXPECT_EQ(4, xp);
    // Bantha meat (450) by a master (+80 or so).
    ASSERT_TRUE(rules_.harvest(450, 80, amount, xp));
    EXPECT_EQ(810, amount);
    // Nothing in the table, nothing harvested.
    ASSERT_TRUE(rules_.harvest(0, 15, amount, xp));
    EXPECT_EQ(0, amount);
    EXPECT_EQ(0, xp);
}

TEST_F(CombatRulesTest, IncapsGrowLongerAndTheThirdKills) {
    bool dies = true;
    int seconds = 0;
    ASSERT_TRUE(rules_.incap(1, dies, seconds));
    EXPECT_FALSE(dies);
    EXPECT_EQ(30, seconds);
    ASSERT_TRUE(rules_.incap(2, dies, seconds));
    EXPECT_FALSE(dies);
    EXPECT_EQ(60, seconds);
    ASSERT_TRUE(rules_.incap(3, dies, seconds));
    EXPECT_TRUE(dies);
    // Three incaps within any 10-minute span kill.
    ASSERT_TRUE(rules_.incapWindow(seconds));
    EXPECT_EQ(600, seconds);
}

TEST_F(CombatRulesTest, AccuracyBonusAddsToTheModifier) {
    run(kCdefLua);
    run("a = { weapon = cdef, distance = 17, attacker = {}, defender = {}, accuracy_bonus = 15 }");
    EXPECT_DOUBLE_EQ(45, eval("rules.accuracy(a)"));
}

TEST_F(CombatRulesTest, EffectsRollTheirChanceLessDefence) {
    run("a = { defender = { skill_mods = { stun_defense = 30 } },"
        " effects = { stun = { chance = 85, duration = 45 }, dizzy = { chance = 100, duration = 0 } } }");
    EXPECT_DOUBLE_EQ(55, eval("rules.effectChance(a, 'stun', 85)"));
    EXPECT_DOUBLE_EQ(100, eval("rules.effectChance(a, 'dizzy', 100)"));
    // A roll of 0.5 lands both (50 < 55, 50 < 100); no duration means 30 s.
    run("landed = rules.effects(a, function() return 0.5 end)");
    EXPECT_DOUBLE_EQ(2, eval("#landed"));
    EXPECT_EQ("stun", evalString("landed[1].name"));
    EXPECT_DOUBLE_EQ(45, eval("landed[1].duration"));
    EXPECT_DOUBLE_EQ(30, eval("landed[2].duration"));
    // A roll of 0.6 misses the stun (60 >= 55).
    run("landed = rules.effects(a, function() return 0.6 end)");
    EXPECT_DOUBLE_EQ(1, eval("#landed"));
    EXPECT_EQ("dizzy", evalString("landed[1].name"));
}

TEST_F(CombatRulesTest, EffectsReachCpp) {
    CombatAttack attack;
    attack.weapon = noobCdef();
    attack.distance = 17;
    attack.special = true;
    attack.accuracyBonus = 200;     // always hits (95%)...
    attack.effects["knockdown"] = CombatEffect(100, 0);
    int knockdowns = 0, hits = 0;
    for (int i = 0; i < 100; ++i) {
        CombatOutcome outcome = rules_.attack(attack);
        ASSERT_TRUE(outcome.ok);
        if (outcome.hit) {
            ++hits;
            ASSERT_EQ(1u, outcome.effects.size());
            EXPECT_EQ("knockdown", outcome.effects[0].first);
            ++knockdowns;
        } else {
            EXPECT_TRUE(outcome.effects.empty());
        }
    }
    EXPECT_EQ(hits, knockdowns);
    EXPECT_GT(hits, 80);
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
