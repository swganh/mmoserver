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

#include "CombatManager.h"

#include "AttackableCreature.h"
#include "CMWeaponGroup.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "SkillManager.h"
#include "SpatialIndexManager.h"
#include "StateManager.h"
#include "Weapon.h"
#include "VehicleController.h"
#include "WorldManager.h"
#include "WorldConfig.h"

#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/clock.h"
#include "Utils/rand.h"

#include <algorithm>
#include <cstdlib>
#include <vector>
//=========================================================================================

bool			CombatManager::mInsFlag		= false;
CombatManager*	CombatManager::mSingleton	= NULL;

//=========================================================================================

CombatManager::CombatManager(Database* database) :
    mDatabase(database)
{
    //gLogger->log(LogManager::INFORMATION,"Start loading weapon groups.");
    // load default attack animations
    mDatabase->executeSqlAsync(this, 0, "SELECT id,defaultAttackAnimationCrc,defaultCombatSpam FROM %s.weapon_groups ORDER BY id",mDatabase->galaxy());

    // Without the script, attacks fall back to the old fixed rules.
    mRules.load("script/combat/rules.lua");

    // Stuns, dizziness and the like wear off (their durations: the special
    // moves table, migration 010).
    gWorldManager->getPlayerScheduler()->addTask(fastdelegate::MakeDelegate(this, &CombatManager::_expireStates), 7, 1000, NULL);
}

//=========================================================================================

CombatManager* CombatManager::Init(Database* database)
{
    if(mInsFlag == false)
    {
        mSingleton = new CombatManager(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//======================================================================================================================

CombatManager::~CombatManager()
{
    WeaponGroups::iterator it = mWeaponGroups.begin();

    while(it != mWeaponGroups.end())
    {
        delete(*it);

        it = mWeaponGroups.erase(it);
    }

    mInsFlag = false;
    delete(mSingleton);
}

//======================================================================================================================
//
// database callback
//

void CombatManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    CMWeaponGroup*	weaponGroup;
    DataBinding*	binding = mDatabase->createDataBinding(3);

    binding->addField(DFT_uint32,offsetof(CMWeaponGroup,mId),4,0);
    binding->addField(DFT_uint32,offsetof(CMWeaponGroup,mDefaultAttackAnimationCrc),4,1);
    binding->addField(DFT_bstring,offsetof(CMWeaponGroup,mDefaultCombatSpam),64,2);

    uint64 count = result->getRowCount();
    mWeaponGroups.reserve((uint32)count);

    for(uint64 i = 0; i < count; i++)
    {
        weaponGroup = new CMWeaponGroup();

        result->getNextRow(binding,weaponGroup);

        mWeaponGroups.push_back(weaponGroup);
    }

    mDatabase->destroyDataBinding(binding);
    //gLogger->log(LogManager::NOTICE,"Finished Loading weapon groups.");
}

//=============================================================================================================================
//
// get the animation crc for the default attack belonging to a weapon group
//

uint32 CombatManager::getDefaultAttackAnimation(uint32 weaponGroup)
{
    WeaponGroups::iterator it = mWeaponGroups.begin();

    while(it != mWeaponGroups.end())
    {
        if((*it)->mId == weaponGroup)
        {
            return((*it)->mDefaultAttackAnimationCrc);
        }

        ++it;
    }

    // default unarmed attack to be safe
    return(1136984016);
}

//=============================================================================================================================
//
// get the combat spam for the default attack belonging to a weapongroup
//

BString CombatManager::getDefaultSpam(uint32 weaponGroup)
{
    WeaponGroups::iterator it = mWeaponGroups.begin();

    while(it != mWeaponGroups.end())
    {
        if((*it)->mId == weaponGroup)
        {
            return((*it)->mDefaultCombatSpam.getAnsi());
        }

        ++it;
    }

    // default unarmed spam to be safe
    return("melee");
}

//=============================================================================================================================
//
// verify combat state
//

bool CombatManager::_verifyCombatState(CreatureObject* attacker, uint64 defenderId)
{
	PlayerObject* playerAttacker = dynamic_cast<PlayerObject*>(attacker);
	CreatureObject* defender = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(defenderId));

	if (!defender)
	{
		// No such object.
		return(false);
	}

	// Do not try to attack already incapped or dead objects.
	if (defender->isIncapacitated() || defender->isDead())
	{
		return false;
	}

	//Do not attack if we are incapped or already dead or mounted.
	if (attacker->isIncapacitated() || attacker->isDead() || playerAttacker->checkIfMounted())
	{
		return false;
	}

	// make sure we got both objects
	if (playerAttacker && defender)
	{
		//Do not attack if we are mounted
		if(playerAttacker->checkIfMounted())
		{
			return false;
		}

		// if our target is a player, he must be dueling us or both need to be overt(TODO)
		if (PlayerObject* defenderPlayer = dynamic_cast<PlayerObject*>(defender))
		{
			// also return, if our target is incapacitated or dead
			if(!playerAttacker->checkDuelList(defenderPlayer) || !defenderPlayer->checkDuelList(playerAttacker)
			|| defenderPlayer == playerAttacker)
			{
				return(false);
			}

			if(defenderPlayer->isIncapacitated())
			{
				// gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_incap");
				return(false);
			}
			else if(defenderPlayer->isDead())
			{
				// gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_dead");
				return(false);
			}

			// put us in combat state	
			gStateManager.setCurrentActionState(attacker, CreatureState_Combat);
            gStateManager.setCurrentActionState(attacker, CreatureState_CombatAttitudeNormal);
			// put our target in combat state
			if(!defenderPlayer->states.checkState(CreatureState_Combat))
			{

				gStateManager.setCurrentActionState(defender, CreatureState_Combat);
                gStateManager.setCurrentActionState(defender, CreatureState_CombatAttitudeNormal);
			}

			// update our defender list
			if (!playerAttacker->checkDefenderList(defenderPlayer->getId()))
			{
				playerAttacker->addDefender(defenderPlayer->getId());
				gMessageLib->sendDefenderUpdate(playerAttacker,1,playerAttacker->getDefenders()->size() - 1,defenderPlayer->getId());
			}

			// update our targets defender list
			if (!defenderPlayer->checkDefenderList(playerAttacker->getId()))
			{
				defenderPlayer->addDefender(playerAttacker->getId());
				gMessageLib->sendDefenderUpdate(defenderPlayer,1,defenderPlayer->getDefenders()->size() - 1,playerAttacker->getId());
			}

			if (!defenderPlayer->autoAttackEnabled())
			{
				// Player can/may start auto-attack if idle.
				defenderPlayer->getController()->enqueueAutoAttack(playerAttacker->getId());
			}

		}
		else
		{
			// our target is a creature
			if (defender->isIncapacitated())
			{
				// gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_incap");
				return(false);
			}
			else if (defender->isDead())
			{
				// gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_dead");
				return(false);
			}

			// Vefify that out target is attackable. It's not nice to spam attacks at innocent npc's.
			if (!(defender->getPvPStatus() & CreaturePvPStatus_Attackable))
			{
				return(false);
			}

			// put us in combat state
			// if (!playerAttacker->states.checkState((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal)))
			{
				// playerAttacker->togglePvPStateOn((CreaturePvPStatus)(CreaturePvPStatus_Attackable + CreaturePvPStatus_Aggressive + CreaturePvPStatus_Enemy));
				gMessageLib->sendUpdatePvpStatus(playerAttacker,playerAttacker, playerAttacker->getPvPStatus() | CreaturePvPStatus_Attackable);

				// TEST STATE MANAGER!
				gStateManager.setCurrentActionState(attacker, CreatureState_Combat);
                gStateManager.setCurrentActionState(attacker, CreatureState_CombatAttitudeNormal);
				
			}

			if (!defender->states.checkState((CreatureState_Combat)))
			{
				// Creature was NOT in combat before, and may very well be dormant.
				// Wake him up.
				gWorldManager->forceHandlingOfDormantNpc(defender->getId());
				gWorldManager->forceHandlingOfReadyNpc(defender->getId());

				// Creature may need some aggro built up before going into combat state??
			
				gStateManager.setCurrentActionState(defender, CreatureState_Combat);
                gStateManager.setCurrentActionState(defender, CreatureState_CombatAttitudeNormal);
			}

			gMessageLib->sendUpdatePvpStatus(defender, playerAttacker, defender->getPvPStatus() | CreaturePvPStatus_Attackable | CreaturePvPStatus_Enemy);

			// update our defender list
			if (!playerAttacker->checkDefenderList(defender->getId()))
			{
				playerAttacker->addDefender(defender->getId());
				gMessageLib->sendDefenderUpdate(playerAttacker,1,playerAttacker->getDefenders()->size() - 1,defender->getId());
			}

			// update our targets defender list
			if (!defender->checkDefenderList(playerAttacker->getId()))
			{
				defender->addDefender(playerAttacker->getId());
				gMessageLib->sendDefenderUpdate(defender,1, defender->getDefenders()->size() - 1,playerAttacker->getId());
			}
		}
	}
	else
	{
		return(false);
	}
	 
	return(true);
}

//======================================================================================================================

bool CombatManager::handleAttack(CreatureObject *attacker, uint64 targetId, ObjectControllerCmdProperties *cmdProperties, uint64* delayMs)
{
    CreatureObject* defender = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetId));

    // get the current weapon
    Weapon* weapon = dynamic_cast<Weapon*>(attacker->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hold_Left));
    if (!weapon)
    {
        return(false);
    }

    if (!defender)
    {
        return(false);
    }

    // if we are out of range, skip this attack.
    if (!mRules.inRange(buildWeapon(weapon), glm::distance(attacker->mPosition, defender->mPosition)))
    {
        // Target out of range.
        PlayerObject* playerAttacker = dynamic_cast<PlayerObject*>(attacker);
        if (playerAttacker && playerAttacker->isConnected())
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "target_out_of_range"), playerAttacker);
        }
        // It's like you shoot but missed, maintain cooldown.
        // return true;
        return false;
    }

    if(_verifyCombatState(attacker,defender->getId()))
    {

        // Execute the attack
        // An unpaid special does nothing: no other targets, no posture.
        if (_executeAttack(attacker,defender,cmdProperties,weapon,delayMs) != kNotPaid)
        {
            _attackArea(attacker, defender, cmdProperties, weapon);
            _setAttackerEndPosture(attacker, cmdProperties);
        }

        return(true);
    }

    return(false);
}

//======================================================================================================================

namespace {

std::string lowerCase(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    return text;
}

// A weapon's damage type ("Energy", "Heat", "Electricity"...) as the armour
// effectiveness key buildSide uses ("energy", "elemental_heat"...).
std::string damageTypeName(const std::string& type)
{
    std::string name = lowerCase(type);
    if (name == "heat" || name == "cold" || name == "acid" || name == "electrical")
    {
        return "elemental_" + name;
    }
    if (name == "electricity")
    {
        return "elemental_electrical";
    }
    return name;
}

// "None", "Light", "Medium", "Heavy" (or a number) as 0-3.
int armorLevel(const std::string& rating)
{
    std::string level = lowerCase(rating);
    if (level == "light") return 1;
    if (level == "medium") return 2;
    if (level == "heavy") return 3;
    return atoi(level.c_str());
}

float numberAttribute(Object* object, const char* key, float fallback)
{
    if (!object->hasAttribute(key))
    {
        return fallback;
    }
    std::string value = object->getAttribute<std::string>(key);
    if (value.empty())
    {
        return fallback;
    }
    return static_cast<float>(atof(value.c_str()));
}

float internalNumberAttribute(Object* object, const char* key, float fallback)
{
    if (!object->hasInternalAttribute(key))
    {
        return fallback;
    }
    std::string value = object->getInternalAttribute<std::string>(key);
    if (value.empty())
    {
        return fallback;
    }
    return static_cast<float>(atof(value.c_str()));
}

const char* weaponGroupName(uint32 group)
{
    switch (group)
    {
    case WeaponGroup_Unarmed:		return "unarmed";
    case WeaponGroup_1h:			return "onehandmelee";
    case WeaponGroup_2h:			return "twohandmelee";
    case WeaponGroup_Polearm:		return "polearm";
    case WeaponGroup_Rifle:			return "rifle";
    case WeaponGroup_Pistol:		return "pistol";
    case WeaponGroup_Carbine:		return "carbine";
    case WeaponGroup_Flamethrower:	return "flamethrower";
    case WeaponGroup_Saber:			return "saber";
    case WeaponGroup_Launcher:		return "launcher";
    default:						return "unarmed";
    }
}

const char* postureName(uint64 posture)
{
    switch (posture)
    {
    case CreaturePosture_Upright:	return "standing";
    case CreaturePosture_Crouched:	return "kneeling";
    case CreaturePosture_Prone:		return "prone";
    default:						return "other";
    }
}

} // namespace

CombatSide CombatManager::buildSide(CreatureObject* creature)
{
    CombatSide side;

    if (dynamic_cast<PlayerObject*>(creature))
    {
        side.kind = "player";
    }
    else if (creature->getCreoGroup() == CreoGroup_AttackableObject)
    {
        side.kind = "object";
    }
    else
    {
        side.kind = "creature";
    }
    side.posture = postureName(creature->states.getPosture());
    struct { CreatureState state; const char* name; } kStates[] = {
        { CreatureState_Intimidated, "intimidated" }, { CreatureState_Stunned, "stunned" },
        { CreatureState_Blinded, "blinded" }, { CreatureState_Dizzy, "dizzy" },
        { CreatureState_Berserk, "berserk" },
    };
    for (size_t i = 0; i < sizeof(kStates) / sizeof(kStates[0]); ++i)
    {
        if (creature->states.checkState(kStates[i].state))
        {
            side.states.push_back(kStates[i].name);
        }
    }
    for (size_t i = 0; i < mBuffs.size(); ++i)
    {
        if (mBuffs[i].creatureId == creature->getId())
        {
            side.buffs[mBuffs[i].name] = mBuffs[i].strength;
        }
    }

    SkillModsList* mods = creature->getSkillMods();
    for (SkillModsList::iterator it = mods->begin(); it != mods->end(); ++it)
    {
        side.skillMods[gSkillManager->getSkillModById(it->first).getAnsi()] = it->second;
    }

    // A creature's base chance to hit, from the guides' creature tables
    // (migration 011); it replaces the rules' base to-hit.
    if (side.kind == "creature" && creature->hasAttribute("creature_tohit"))
    {
        side.toHit = static_cast<float>(atof(creature->getAttribute<std::string>("creature_tohit").c_str()));
    }

    // Creatures carry their armour as attributes; worn armour is not read yet.
    if (creature->hasAttribute("armor_rating"))
    {
        side.armorRating = armorLevel(creature->getAttribute<std::string>("armor_rating"));
    }
    static const char* kDamageTypes[] = {
        "kinetic", "energy", "blast", "stun", "elemental_heat", "elemental_cold",
        "elemental_acid", "elemental_electrical", "restraint"
    };
    for (size_t i = 0; i < sizeof(kDamageTypes) / sizeof(kDamageTypes[0]); ++i)
    {
        std::string key = std::string("cat_armor_effectiveness.armor_eff_") + kDamageTypes[i];
        float effectiveness = numberAttribute(creature, key.c_str(), 0);
        if (effectiveness != 0)
        {
            side.armor[kDamageTypes[i]] = effectiveness;
        }
    }

    return side;
}

CombatWeapon CombatManager::buildWeapon(Weapon* weapon)
{
    CombatWeapon w;
    w.group = weaponGroupName(weapon->getGroup());
    w.minDamage = numberAttribute(weapon, "cat_wpn_damage.wpn_damage_min", 50);
    w.maxDamage = numberAttribute(weapon, "cat_wpn_damage.wpn_damage_max", 100);
    if (weapon->hasAttribute("cat_wpn_damage.wpn_damage_type"))
    {
        w.damageType = damageTypeName(weapon->getAttribute<std::string>("cat_wpn_damage.wpn_damage_type"));
    }
    w.speed = numberAttribute(weapon, "wpn_attack_speed", 0);
    w.zeroMod = numberAttribute(weapon, "cat_wpn_rangemods.wpn_range_zero", 0);
    w.midMod = numberAttribute(weapon, "cat_wpn_rangemods.wpn_range_mid", 0);
    w.maxMod = numberAttribute(weapon, "cat_wpn_rangemods.wpn_range_max", 0);
    // Ranges in metres: internal attributes, migration 007.
    w.midRange = internalNumberAttribute(weapon, "combat_range_mid", 0);
    w.maxRange = internalNumberAttribute(weapon, "combat_range_max", 0);
    if (weapon->hasAttribute("wpn_armor_pierce_rating"))
    {
        w.armorPiercing = armorLevel(weapon->getAttribute<std::string>("wpn_armor_pierce_rating"));
    }
    // What a special costs is a multiple of these (p. 389).
    w.healthCost = numberAttribute(weapon, "cat_wpn_attack_cost.wpn_attack_cost_health", 0);
    w.actionCost = numberAttribute(weapon, "cat_wpn_attack_cost.wpn_attack_cost_action", 0);
    w.mindCost = numberAttribute(weapon, "cat_wpn_attack_cost.wpn_attack_cost_mind", 0);
    w.woundChance = numberAttribute(weapon, "cat_wpn_damage.wpn_wound_chance", 0);
    return w;
}

void CombatManager::sendWeaponRanges(PlayerObject* player)
{
    Weapon* weapon = dynamic_cast<Weapon*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hold_Left));
    if (!weapon)
    {
        return;
    }
    std::string layout;
    std::vector<double> values;
    if (!mRules.clientRanges(buildWeapon(weapon), layout, values))
    {
        return;
    }
    gMessageLib->sendWeaponRanges(player, weapon->getId(), layout, values);
}

void CombatManager::applyDamage(CreatureObject* defender, const std::string& pool, int32 damage)
{
    if (defender->getCreoGroup() == CreoGroup_AttackableObject)
    {
        defender->getHam()->updateSingleHam(-damage, true);
        return;
    }

    uint8 bar = HamBar_Health;
    if (pool == "action")
    {
        bar = HamBar_Action;
    }
    else if (pool == "mind")
    {
        bar = HamBar_Mind;
    }
    defender->getHam()->updatePropertyValue(bar, HamProperty_CurrentHitpoints, -damage, true);
}

bool CombatManager::handleBrawlerMove(PlayerObject* attacker, uint64 targetId, ObjectControllerCmdProperties* cmdProperties)
{
    std::string name = lowerCase(cmdProperties->mCommandStr.getAnsi());
    std::string kind;
    float range = 0, radius = 0;
    if (!mRules.moveReach(name, kind, range, radius))
    {
        LOG(WARNING) << "Brawler move " << name << ": not in the combat rules (rules.MOVES)";
        return false;
    }
    if (attacker->isIncapacitated() || attacker->isDead())
    {
        return false;
    }

    // The targets: the one targeted, or every enemy around for Warcry.
    std::vector<CreatureObject*> targets;
    if (radius > 0)
    {
        ObjectSet around;
        gSpatialIndexManager->getObjectsInRange(attacker, &around, ObjType_Creature | ObjType_NPC, radius, true);
        for (ObjectSet::iterator it = around.begin(); it != around.end(); ++it)
        {
            AttackableCreature* creature = dynamic_cast<AttackableCreature*>(*it);
            if (creature && !creature->isDead() && !creature->isIncapacitated()
                    && (creature->getPvPStatus() & CreaturePvPStatus_Attackable))
            {
                targets.push_back(creature);
            }
        }
        if (targets.empty())
        {
            gMessageLib->SendSystemMessage(L"There is no one around to hear your warcry.", attacker);
            return false;
        }
    }
    else
    {
        CreatureObject* target = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetId));
        // Only someone the brawler may fight, as for an attack
        // (_verifyCombatState): a live, attackable creature or a duel opponent.
        PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(target);
        bool eligible = target && target != attacker && !target->isDead() && !target->isIncapacitated()
            && (targetPlayer ? (attacker->checkDuelList(targetPlayer) && targetPlayer->checkDuelList(attacker))
                             : (target->getPvPStatus() & CreaturePvPStatus_Attackable) != 0);
        if (!eligible)
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "target_type_prohibited"), attacker);
            return false;
        }
        if (glm::distance(attacker->mPosition, target->mPosition) > range)
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "target_out_of_range"), attacker);
            return false;
        }
        targets.push_back(target);
    }

    // The move's flat HAM cost (command_table).
    if (!_payFlatCosts(attacker, cmdProperties))
    {
        return false;
    }

    int landedCount = 0;
    for (size_t i = 0; i < targets.size(); ++i)
    {
        CreatureObject* defender = targets[i];
        // Using the move on someone starts the fight, as an attack does.
        if (!_verifyCombatState(attacker, defender->getId()))
        {
            continue;
        }
        if (cmdProperties->mAnimationCrc)
        {
            gMessageLib->sendCombatAction(attacker, defender, cmdProperties->mAnimationCrc);
        }

        CombatAttack attack;
        attack.attacker = buildSide(attacker);
        attack.defender = buildSide(defender);
        attack.distance = glm::distance(attacker->mPosition, defender->mPosition);
        bool landed = false;
        float chance = 0, duration = 0;
        if (!mRules.move(name, attack, cmdProperties->mIntimidateChance, landed, chance, duration))
        {
            continue;
        }
        LOG(INFO) << "Brawler move " << name << " by " << attacker->getId() << " on " << defender->getId()
                  << ": " << chance << "% " << (landed ? "landed" : "failed");
        if (!landed)
        {
            continue;
        }
        ++landedCount;

        if (kind == "intimidate")
        {
            CombatOutcome outcome;
            outcome.effects.push_back(std::make_pair(std::string("intimidate"), duration));
            _applyEffects(defender, outcome);
        }
        else if (kind == "taunt")
        {
            if (AttackableCreature* creature = dynamic_cast<AttackableCreature*>(defender))
            {
                creature->taunt(attacker->getId());
            }
        }
    }

    // What happened, for the brawler.
    std::wstring move(name.begin(), name.end());
    if (landedCount)
    {
        gMessageLib->SendSystemMessage(L"Your " + move + L" succeeds" +
            (targets.size() > 1 ? L" on " + std::to_wstring(landedCount) + L" of " + std::to_wstring(targets.size()) : L"") + L".", attacker);
    }
    else
    {
        gMessageLib->SendSystemMessage(L"Your " + move + L" fails.", attacker);
    }
    return true;
}

void CombatManager::clearSelfBuffs(CreatureObject* creature)
{
    for (size_t i = 0; i < mBuffs.size(); )
    {
        if (mBuffs[i].creatureId == creature->getId())
        {
            mBuffs[i] = mBuffs.back();
            mBuffs.pop_back();
            continue;
        }
        ++i;
    }
}

bool CombatManager::_payFlatCosts(CreatureObject* creature, ObjectControllerCmdProperties* cmdProperties)
{
    Ham* ham = creature->getHam();
    if (!ham->checkMainPools(cmdProperties->mHealthCost, cmdProperties->mActionCost, cmdProperties->mMindCost))
    {
        if (PlayerObject* player = dynamic_cast<PlayerObject*>(creature))
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), player);
        }
        return false;
    }
    if (cmdProperties->mHealthCost) ham->updatePropertyValue(HamBar_Health, HamProperty_CurrentHitpoints, -cmdProperties->mHealthCost);
    if (cmdProperties->mActionCost) ham->updatePropertyValue(HamBar_Action, HamProperty_CurrentHitpoints, -cmdProperties->mActionCost);
    if (cmdProperties->mMindCost) ham->updatePropertyValue(HamBar_Mind, HamProperty_CurrentHitpoints, -cmdProperties->mMindCost);
    return true;
}

bool CombatManager::handleSelfBuff(PlayerObject* player, ObjectControllerCmdProperties* cmdProperties)
{
    std::string name = lowerCase(cmdProperties->mCommandStr.getAnsi());
    if (player->isIncapacitated() || player->isDead())
    {
        return false;
    }

    CombatAttack attack;
    attack.attacker = buildSide(player);
    Weapon* weapon = dynamic_cast<Weapon*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hold_Left));
    if (weapon)
    {
        attack.weapon = buildWeapon(weapon);
    }
    else
    {
        attack.weapon.group = "unarmed";
    }

    std::string buff, reason;
    float strength = 0, duration = 0;
    if (!mRules.selfBuff(name, attack, buff, strength, duration, reason))
    {
        if (reason == "melee")
        {
            gMessageLib->SendSystemMessage(L"You must be unarmed or wielding a melee weapon to center your being.", player);
        }
        else if (reason == "skill")
        {
            gMessageLib->SendSystemMessage(L"You lack the skill to center your being with this weapon.", player);
        }
        else
        {
            LOG(WARNING) << "Self buff " << name << ": not in the combat rules (rules.selfBuff)";
        }
        return false;
    }

    // One at a time: it runs its course before it can be used again.
    for (size_t i = 0; i < mBuffs.size(); ++i)
    {
        if (mBuffs[i].creatureId == player->getId() && mBuffs[i].name == buff)
        {
            gMessageLib->SendSystemMessage(buff == "berserk" ? L"You are already berserk." : L"Your being is already centered.", player);
            return false;
        }
    }

    if (!_payFlatCosts(player, cmdProperties))
    {
        return false;
    }

    uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();
    TimedBuff timed = { player->getId(), buff, strength, now + static_cast<uint64>(duration * 1000.0f) };
    mBuffs.push_back(timed);
    if (buff == "berserk")
    {
        gStateManager.setCurrentActionState(player, CreatureState_Berserk);
        gMessageLib->SendSystemMessage(L"You go berserk!", player);
    }
    else
    {
        gMessageLib->SendSystemMessage(L"You center your being.", player);
    }
    LOG(INFO) << "Self buff " << name << " on " << player->getId() << ": " << buff << " " << strength
              << " for " << duration << " s";
    return true;
}

void CombatManager::applyWounds(CreatureObject* defender, const CombatOutcome& outcome)
{
    if (defender->getCreoGroup() == CreoGroup_AttackableObject || defender->isIncapacitated() || defender->isDead())
    {
        return;
    }

    if (outcome.wound > 0)
    {
        uint8 bar = HamBar_Health;
        if (outcome.pool == "action")
        {
            bar = HamBar_Action;
        }
        else if (outcome.pool == "mind")
        {
            bar = HamBar_Mind;
        }
        defender->getHam()->updatePropertyValue(bar, HamProperty_Wounds, outcome.wound, false, true);
        LOG(INFO) << "Combat wound " << outcome.wound << " " << outcome.pool << " on " << defender->getId();

        if (outcome.bleed > 0 && outcome.bleedInterval > 0 && outcome.bleedDuration > 0)
        {
            _startBleeding(defender, bar, outcome.bleed, outcome.bleedInterval, outcome.bleedDuration);
        }
    }

    // Battle fatigue only matters to players (it hampers wound healing).
    // The rules give fractions (a quarter a hit); they add up until whole.
    if (outcome.fatigue > 0 && defender->getType() == ObjType_Player)
    {
        float& carry = mFatigueCarry[defender->getId()];
        carry += outcome.fatigue;
        int32 whole = static_cast<int32>(carry);
        carry -= static_cast<float>(whole);
        if (whole > 0)
        {
            defender->getHam()->updateBattleFatigue(whole, true);
        }
    }
}

bool CombatManager::isBleeding(CreatureObject* creature) const
{
    for (size_t i = 0; i < mBleeds.size(); ++i)
    {
        if (mBleeds[i].creatureId == creature->getId())
        {
            return true;
        }
    }
    return false;
}

void CombatManager::_startBleeding(CreatureObject* creature, uint8 bar, int32 perTick, float interval, float duration)
{
    uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();
    uint64 expires = now + static_cast<uint64>(duration * 1000.0f);
    bool wasBleeding = isBleeding(creature);

    // A new wound to a pool that bleeds already: the stronger bleed, for the
    // longer time.
    for (size_t i = 0; i < mBleeds.size(); ++i)
    {
        if (mBleeds[i].creatureId == creature->getId() && mBleeds[i].bar == bar)
        {
            mBleeds[i].perTick = std::max(mBleeds[i].perTick, perTick);
            mBleeds[i].expires = std::max(mBleeds[i].expires, expires);
            return;
        }
    }
    uint64 intervalMs = static_cast<uint64>(interval * 1000.0f);
    Bleed bleed = { creature->getId(), bar, perTick, intervalMs, now + intervalMs, expires };
    mBleeds.push_back(bleed);
    LOG(INFO) << "Bleeding " << perTick << " every " << interval << " s for " << duration << " s, bar "
              << static_cast<uint32>(bar) << " on " << creature->getId();

    if (!wasBleeding)
    {
        gStateManager.setCurrentActionState(creature, CreatureState_Bleeding);
        if (PlayerObject* player = dynamic_cast<PlayerObject*>(creature))
        {
            gMessageLib->SendSystemMessage(L"You are bleeding!", player);
        }
    }
}

void CombatManager::_tickBleeding(uint64 now)
{
    for (size_t i = 0; i < mBleeds.size(); )
    {
        Bleed& bleed = mBleeds[i];
        CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(bleed.creatureId));
        // Incapacitation and death end it; so does its time running out.
        bool over = !creature || creature->isIncapacitated() || creature->isDead() || bleed.expires <= now;
        if (!over && bleed.nextTick <= now)
        {
            bleed.nextTick += bleed.interval;
            creature->getHam()->updatePropertyValue(bleed.bar, HamProperty_CurrentHitpoints, -bleed.perTick, true);
            // The drain may have incapacitated or killed it; look it up again.
            creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(mBleeds[i].creatureId));
            over = !creature || creature->isIncapacitated() || creature->isDead();
        }
        if (!over)
        {
            ++i;
            continue;
        }
        uint64 creatureId = bleed.creatureId;
        mBleeds[i] = mBleeds.back();
        mBleeds.pop_back();
        _bleedingEnded(creatureId);
    }
}

void CombatManager::_bleedingEnded(uint64 creatureId)
{
    for (size_t i = 0; i < mBleeds.size(); ++i)
    {
        if (mBleeds[i].creatureId == creatureId)
        {
            return;
        }
    }
    LOG(INFO) << "Bleeding on " << creatureId << " ended";
    CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(creatureId));
    if (!creature)
    {
        return;
    }
    if (creature->states.checkState(CreatureState_Bleeding))
    {
        gStateManager.removeActionState(creature, CreatureState_Bleeding);
    }
    PlayerObject* player = dynamic_cast<PlayerObject*>(creature);
    if (player && !player->isIncapacitated() && !player->isDead())
    {
        gMessageLib->SendSystemMessage(L"You have stopped bleeding.", player);
    }
}

bool CombatManager::stopBleeding(CreatureObject* creature)
{
    bool found = false;
    for (size_t i = 0; i < mBleeds.size(); )
    {
        if (mBleeds[i].creatureId == creature->getId())
        {
            mBleeds[i] = mBleeds.back();
            mBleeds.pop_back();
            found = true;
            continue;
        }
        ++i;
    }
    if (found)
    {
        _bleedingEnded(creature->getId());
    }
    return found;
}

//======================================================================================================================

CombatAttack CombatManager::_buildAttack(CreatureObject* attacker, CreatureObject* defender, ObjectControllerCmdProperties* cmdProperties, Weapon* weapon)
{
	CombatAttack attack;
	attack.attacker = buildSide(attacker);
	attack.defender = buildSide(defender);
	attack.weapon = buildWeapon(weapon);
	attack.distance = glm::distance(attacker->mPosition, defender->mPosition);
	if (cmdProperties->mDamageMultiplier)
	{
		attack.damageMultiplier = cmdProperties->mDamageMultiplier;
	}
	if (cmdProperties->mDelayMultiplier)
	{
		attack.delayMultiplier = cmdProperties->mDelayMultiplier;
	}
	if (cmdProperties->mHealthHitChance)
	{
		attack.pool = "health";
	}
	else if (cmdProperties->mActionHitChance)
	{
		attack.pool = "action";
	}
	else if (cmdProperties->mMindHitChance)
	{
		attack.pool = "mind";
	}

	// A special (anything but the default attack, 0xa8fef90a): its to-hit
	// modifier, costs and effects (command_table, migration 010).
	attack.special = cmdProperties->mCmdCrc != 0xa8fef90a;
	attack.accuracyBonus = static_cast<float>(cmdProperties->mAccuracyBonus);
	attack.healthCostMultiplier = cmdProperties->mHealthCostMultiplier;
	attack.actionCostMultiplier = cmdProperties->mActionCostMultiplier;
	attack.mindCostMultiplier = cmdProperties->mMindCostMultiplier;
	struct { const char* name; float chance; float duration; } effects[] = {
		{ "knockdown", cmdProperties->mKnockdownChance, 0 },
		{ "posture_down", cmdProperties->mPostureDownChance, 0 },
		{ "posture_up", cmdProperties->mPostureUpChance, 0 },
		{ "stun", cmdProperties->mStunChance, cmdProperties->mStunDuration },
		{ "dizzy", cmdProperties->mDizzyChance, cmdProperties->mDizzyDuration },
		{ "blind", cmdProperties->mBlindChance, cmdProperties->mBlindDuration },
		{ "intimidate", cmdProperties->mIntimidateChance, 0 },
	};
	for (size_t i = 0; i < sizeof(effects) / sizeof(effects[0]); ++i)
	{
		if (effects[i].chance > 0)
		{
			attack.effects[effects[i].name] = CombatEffect(effects[i].chance, effects[i].duration);
		}
	}
	return attack;
}

bool CombatManager::_payCosts(CreatureObject* attacker, const CombatAttack& attack)
{
	int health = 0, action = 0, mind = 0;
	if (!attack.special || !mRules.costs(attack, health, action, mind) || (health + action + mind) == 0)
	{
		return true;
	}
	Ham* ham = attacker->getHam();
	if (!ham || !ham->checkMainPools(health, action, mind))
	{
		return false;
	}
	if (health) ham->updatePropertyValue(HamBar_Health, HamProperty_CurrentHitpoints, -health);
	if (action) ham->updatePropertyValue(HamBar_Action, HamProperty_CurrentHitpoints, -action);
	if (mind) ham->updatePropertyValue(HamBar_Mind, HamProperty_CurrentHitpoints, -mind);
	return true;
}

bool CombatManager::_canFight(CreatureObject* attacker, CreatureObject* target)
{
    if (!target || target == attacker || target->isDead() || target->isIncapacitated())
    {
        return false;
    }
    if (PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(target))
    {
        PlayerObject* playerAttacker = dynamic_cast<PlayerObject*>(attacker);
        return playerAttacker && playerAttacker->checkDuelList(targetPlayer) && targetPlayer->checkDuelList(playerAttacker);
    }
    return (target->getPvPStatus() & CreaturePvPStatus_Attackable) != 0;
}

void CombatManager::_attackArea(CreatureObject* attacker, CreatureObject* defender, ObjectControllerCmdProperties* cmdProperties, Weapon* weapon)
{
    std::string shape = lowerCase(cmdProperties->mAreaShape.getAnsi());
    if (shape != "area" && shape != "cone")
    {
        return;
    }
    float reach = cmdProperties->mAreaLength;
    if (reach <= 0 && !mRules.maxRange(buildWeapon(weapon), reach))
    {
        return;
    }

    // A cone points from the attacker at the main target and widens to its
    // width at its far end.
    glm::vec3 forward = defender->mPosition - attacker->mPosition;
    forward.y = 0;
    if (glm::length(forward) > 0.001f)
    {
        forward = glm::normalize(forward);
    }

    ObjectSet around;
    gSpatialIndexManager->getObjectsInRange(attacker, &around, ObjType_Creature | ObjType_NPC | ObjType_Player, reach, true);
    int hits = 0;
    for (ObjectSet::iterator it = around.begin(); it != around.end(); ++it)
    {
        CreatureObject* other = dynamic_cast<CreatureObject*>(*it);
        if (!other || other == defender || !_canFight(attacker, other))
        {
            continue;
        }
        glm::vec3 offset = other->mPosition - attacker->mPosition;
        offset.y = 0;
        if (glm::length(offset) > reach)
        {
            continue;
        }
        if (shape == "cone")
        {
            float along = glm::dot(offset, forward);
            float halfWidth = cmdProperties->mAreaWidth * 0.5f * along / reach;
            if (along <= 0 || glm::length(offset - forward * along) > halfWidth)
            {
                continue;
            }
        }
        if (_verifyCombatState(attacker, other->getId()))
        {
            _executeAttack(attacker, other, cmdProperties, weapon, NULL, true);
            ++hits;
        }
    }
    LOG(INFO) << "Special " << cmdProperties->mCommandStr.getAnsi() << " (" << shape << ", " << reach
              << " m) by " << attacker->getId() << " also hit " << hits;
}

void CombatManager::_setAttackerEndPosture(CreatureObject* attacker, ObjectControllerCmdProperties* cmdProperties)
{
    std::string posture = lowerCase(cmdProperties->mAttackerEndPosture.getAnsi());
    if (posture.empty() || attacker->isDead() || attacker->isIncapacitated())
    {
        return;
    }
    if (posture == "prone")
    {
        gStateManager.setCurrentPostureState(attacker, CreaturePosture_Prone);
        gStateManager.setCurrentLocomotionState(attacker, CreatureLocomotion_Prone);
    }
    else if (posture == "kneeling")
    {
        gStateManager.setCurrentPostureState(attacker, CreaturePosture_Crouched);
        gStateManager.setCurrentLocomotionState(attacker, CreatureLocomotion_Kneeling);
    }
    else if (posture == "standing")
    {
        gStateManager.setCurrentPostureState(attacker, CreaturePosture_Upright);
        gStateManager.setCurrentLocomotionState(attacker, CreatureLocomotion_Standing);
    }
}

uint8 CombatManager::_executeAttack(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon, uint64* delayMs, bool secondary)
{
	int32	multipliedDamage		= 0;
	BString	combatSpam				= "melee";

	// The combat rules (data/script/combat/rules.lua) decide the outcome.
	CombatAttack attack = _buildAttack(attacker, defender, cmdProperties, weapon);

	// A special the attacker cannot pay for does not happen. Its other
	// targets (an area or cone) were paid for with the first.
	if (!secondary && !_payCosts(attacker, attack))
	{
		if (PlayerObject* player = dynamic_cast<PlayerObject*>(attacker))
		{
			gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), player);
		}
		if (delayMs)
		{
			*delayMs = 1000;
		}
		return kNotPaid;
	}

	CombatOutcome outcome = mRules.attack(attack);
	if (!outcome.ok)
	{
		// No rules: the old ones, two hits in three for the weapon's damage.
		outcome.hit = (gRandom->getRand() % 3) != 0;
		int32 range = static_cast<int32>(attack.weapon.maxDamage - attack.weapon.minDamage) + 1;
		outcome.damage = static_cast<int32>(attack.weapon.minDamage) + gRandom->getRand() % (range > 0 ? range : 1);
		static const char* kPools[] = { "health", "action", "mind" };
		outcome.pool = attack.pool.empty() ? kPools[gRandom->getRand() % 3] : attack.pool;
		// The client's attack has no default time (0): the old 2 s then.
		outcome.delay = cmdProperties->mDefaultTime ? cmdProperties->mDefaultTime / 1000.0f : 2.0f;
	}
	if (delayMs)
	{
		*delayMs = static_cast<uint64>(outcome.delay * 1000.0f);
	}

	// 0 hit, 1 miss: what the fly text and the combat spam below expect.
	uint8 attackResult = outcome.hit ? 0 : 1;

	// only proceed, if so
	if(!attackResult)
	{
		multipliedDamage = -outcome.damage;

		// states: the effects that landed (rules.effects)
		_applyEffects(defender, outcome);


		// Here is the deal. When a player makes damage to a npc, we have to register the player, its group, damage done and what (kind of) weapon used.
		NPCObject* npc = dynamic_cast<NPCObject*>(defender);
		if (!defender->isDead() && npc)
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(attacker);
			if (player)
			{
                npc->updateDamage(player->getId(), player->getGroupId(), weapon->getGroup(), -multipliedDamage, player->states.getPosture(), glm::distance(defender->mPosition, player->mPosition));
			}
		}

		applyDamage(defender, outcome.pool, outcome.damage);
		applyWounds(defender, outcome);
		if (defender->isIncapacitated())
		{
			PlayerObject* playerAttacker = dynamic_cast<PlayerObject*>(attacker);
			if (playerAttacker && playerAttacker->isConnected())
			{
                gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_target_incap", 0, defender->getId(), 0), playerAttacker);
            }
        }
        if (defender->isDead())
        {
            PlayerObject* playerAttacker = dynamic_cast<PlayerObject*>(attacker);
            if (playerAttacker && playerAttacker->isConnected())
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "killer_target_dead"), playerAttacker, true);
            }
        }
    }


    // fly text and animations
    // default attack(s)
    if(cmdProperties->mCmdCrc == 0xa8fef90a)
    {
        uint32 animCrc = getDefaultAttackAnimation(weapon->getGroup());

        switch(attackResult)
        {
            // hit
        case 0:
        case 2:
        case 3:
        case 4:
        {
            gMessageLib->sendCombatAction(attacker,defender,animCrc,0,0,1);
        }
        break;

        // miss
        case 1:
        {
            gMessageLib->sendCombatAction(attacker,defender,animCrc);
        }
        break;
        }
    }
    // special attack
    else
    {
        switch(attackResult)
        {
            // hit
        case 0:
        case 2:
        case 3:
        case 4:
        {
            gMessageLib->sendCombatAction(attacker,defender,cmdProperties->mAnimationCrc,cmdProperties->mTrail1,cmdProperties->mTrail2,1);
        }
        break;

        //miss
        case 1:
        {
            gMessageLib->sendCombatAction(attacker,defender,cmdProperties->mAnimationCrc,cmdProperties->mTrail1,cmdProperties->mTrail2);
        }
        break;
        }
    }

    switch(attackResult)
    {
    case 0:
    {
        // Defender got hit.
    }
    break;

    case 1:
    {
        gMessageLib->sendFlyText(defender,"combat_effects","miss",255,255,255);
    }
    break;

    case 2:
        // We cant block yet, can we?
    {
        gMessageLib->sendFlyText(defender,"combat_effects","block",0,255,0);
        gMessageLib->sendCombatAction(defender,attacker,0xe430ff04);
    }
    break;

    case 3:
    {
        gMessageLib->sendFlyText(defender,"combat_effects","dodge",0,255,0);
        gMessageLib->sendCombatAction(defender,attacker,0xe430ff04);	// Dodge
    }
    break;

    case 4:
    {
        gMessageLib->sendFlyText(defender,"combat_effects","counterattack",0,255,0);	// I can's see this effect working?
    }
    break;
    }

    // send combat spam
    // default attack
    if(cmdProperties->mCmdCrc == 0xa8fef90a)
    {
        combatSpam = getDefaultSpam(weapon->getGroup());
    }
    // special attack
    else
    {
        if(cmdProperties->mCbtSpam.getLength())
        {
            combatSpam = cmdProperties->mCbtSpam.getAnsi();
        }
    }

    switch(attackResult)
    {
    case 0:
        combatSpam << "_hit";
        break;
    case 1:
        combatSpam << "_miss";
        break;
    case 2:
        combatSpam << "_block";
        break;
    case 3:
        combatSpam << "_evade";
        break;
    case 4:
        combatSpam << "_counter";
        break;

    default:
        break;
    }
    gMessageLib->sendCombatSpam(attacker,defender,-multipliedDamage,"cbt_spam",combatSpam);


    return(0);
}

//======================================================================================================================

void CombatManager::_applyEffects(CreatureObject* defender, const CombatOutcome& outcome)
{
	uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();

	for (size_t i = 0; i < outcome.effects.size(); ++i)
	{
		const std::string& name = outcome.effects[i].first;
		uint64 durationMs = static_cast<uint64>(outcome.effects[i].second * 1000.0f);

		LOG(INFO) << "Combat effect " << name << " on " << defender->getId() << " for " << outcome.effects[i].second
		          << " s, posture " << static_cast<uint32>(defender->states.getPosture());

		if (name == "knockdown")
		{
			gStateManager.setCurrentPostureState(defender, CreaturePosture_KnockedDown);
			gStateManager.setCurrentLocomotionState(defender, CreatureLocomotion_KnockedDown);
			_timeStandUp(defender, now + durationMs);

			if(PlayerObject* player = dynamic_cast<PlayerObject*>(defender))
			{
				//See if our player is mounted -- if so dismount him
				if(player->checkIfMounted())
				{
					//Get the player's mount
					if(VehicleController* vehicle = dynamic_cast<VehicleController*>(gWorldManager->getObjectById(player->getMount()->controller())))
					{
						//Now dismount
						vehicle->DismountPlayer();
					}
				}
			}
			continue;
		}
		// Down: standing to kneeling, kneeling to prone. Up: the other way.
		if (name == "posture_down" || name == "posture_up")
		{
			uint64 posture = defender->states.getPosture();
			bool down = (name == "posture_down");
			if (down && posture == CreaturePosture_Upright)
				gStateManager.setCurrentPostureState(defender, CreaturePosture_Crouched);
			else if (down && posture == CreaturePosture_Crouched)
				gStateManager.setCurrentPostureState(defender, CreaturePosture_Prone);
			else if (!down && posture == CreaturePosture_Prone)
				gStateManager.setCurrentPostureState(defender, CreaturePosture_Crouched);
			else if (!down && posture == CreaturePosture_Crouched)
				gStateManager.setCurrentPostureState(defender, CreaturePosture_Upright);
			if (down)
				_timeStandUp(defender, now + durationMs);
			LOG(INFO) << "Combat effect " << name << " on " << defender->getId() << ": posture now "
			          << static_cast<uint32>(defender->states.getPosture());
			continue;
		}

		CreatureState state;
		if (name == "stun") state = CreatureState_Stunned;
		else if (name == "dizzy") state = CreatureState_Dizzy;
		else if (name == "blind") state = CreatureState_Blinded;
		else if (name == "intimidate") state = CreatureState_Intimidated;
		else continue;

		gStateManager.setCurrentActionState(defender, state);

		// The state ends after its duration; a new one restarts the time.
		bool found = false;
		for (size_t j = 0; j < mTimedStates.size(); ++j)
		{
			if (mTimedStates[j].creatureId == defender->getId() && mTimedStates[j].state == static_cast<uint64>(state))
			{
				mTimedStates[j].expires = now + durationMs;
				found = true;
				break;
			}
		}
		if (!found)
		{
			TimedState timed = { defender->getId(), static_cast<uint64>(state), now + durationMs };
			mTimedStates.push_back(timed);
		}
	}
}

void CombatManager::_timeStandUp(CreatureObject* creature, uint64 when)
{
	// Players get up themselves.
	if (creature->getType() == ObjType_Player)
	{
		return;
	}
	for (size_t j = 0; j < mTimedStates.size(); ++j)
	{
		if (mTimedStates[j].creatureId == creature->getId() && mTimedStates[j].state == kStandUp)
		{
			mTimedStates[j].expires = when;
			return;
		}
	}
	TimedState timed = { creature->getId(), kStandUp, when };
	mTimedStates.push_back(timed);
}

bool CombatManager::_expireStates(uint64 callTime, void* ref)
{
	uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();
	for (size_t i = 0; i < mTimedStates.size(); )
	{
		if (mTimedStates[i].expires > now)
		{
			++i;
			continue;
		}
		CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(mTimedStates[i].creatureId));
		if (creature && mTimedStates[i].state == kStandUp)
		{
			// A creature knocked down or kneeling gets up again.
			uint64 posture = creature->states.getPosture();
			if (!creature->isDead() && !creature->isIncapacitated()
			    && (posture == CreaturePosture_KnockedDown || posture == CreaturePosture_Crouched || posture == CreaturePosture_Prone))
			{
				gStateManager.setCurrentPostureState(creature, CreaturePosture_Upright);
				gStateManager.setCurrentLocomotionState(creature, CreatureLocomotion_Standing);
			}
		}
		else if (creature && creature->states.checkState(static_cast<CreatureState>(mTimedStates[i].state)))
		{
			gStateManager.removeActionState(creature, static_cast<CreatureState>(mTimedStates[i].state));
		}
		mTimedStates[i] = mTimedStates.back();
		mTimedStates.pop_back();
	}

	_tickBleeding(now);

	for (size_t i = 0; i < mBuffs.size(); )
	{
		if (mBuffs[i].expires > now)
		{
			++i;
			continue;
		}
		CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(mBuffs[i].creatureId));
		if (creature)
		{
			bool berserk = (mBuffs[i].name == "berserk");
			if (berserk && creature->states.checkState(CreatureState_Berserk))
			{
				gStateManager.removeActionState(creature, CreatureState_Berserk);
			}
			if (PlayerObject* player = dynamic_cast<PlayerObject*>(creature))
			{
				gMessageLib->SendSystemMessage(berserk ? L"You are no longer berserk." : L"Your being is no longer centered.", player);
			}
			LOG(INFO) << "Self buff " << mBuffs[i].name << " on " << mBuffs[i].creatureId << " ended";
		}
		mBuffs[i] = mBuffs.back();
		mBuffs.pop_back();
	}
	return true;
}

//======================================================================================================================


