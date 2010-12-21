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

#include "NpcManager.h"
#include "AttackableCreature.h"
#include "CombatManager.h"
#include "CreatureObject.h"
#include "PlayerObject.h"
#include "Weapon.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "StateManager.h"
#include "ZoneServer/NonPersistentNpcFactory.h"
#include "MessageLib/MessageLib.h"


#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/utils.h"
#include "Utils/rand.h"

#include <cassert>

//=============================================================================

// Basic lair data when loading from DB.

class NpcLairEntity
{
public:
    NpcLairEntity() {}

    uint64	mLairsId;
    uint64	mLairTemplateId;
    uint32	mNumberOfLairs;
};


//=============================================================================

NpcManager* NpcManager::mInstance  = NULL;


//======================================================================================================================

NpcManager* NpcManager::Instance(void)
{
    if (!mInstance)
    {
        mInstance = new NpcManager(WorldManager::getSingletonPtr()->getDatabase());
    }
    return mInstance;
}


// This constructor prevents the default constructor to be used, as long as the constructor is keept private.

NpcManager::NpcManager()
{
}

//=============================================================================


NpcManager::NpcManager(Database* database) :mDatabase(database)
{
    // _setupDatabindings();
    this->loadLairs();
}


//=============================================================================

NpcManager::~NpcManager()
{
    // _destroyDatabindings();
    mInstance = NULL;
}

//=============================================================================


//=============================================================================
//
//	Here we start the real stuff.
//

/*
void NpcManager::addCreature(uint64 creatureId, const SpawnData *spawn)
{

	AttackableCreature* npc = dynamic_cast<AttackableCreature*>(gWorldManager->getObjectById(creatureId));
	if (!npc)
	{
		return;
	}

	// Update npc with spawn data. The Npc will be the owner of this data.
	npc->setSpawnData(spawn);
	// npc->setAiState(NpcIsDormant);

	// Activate npc.
	// gWorldManager->addReadyNpc(creatureId, spawn->mBasic.timeToFirstSpawn);

	gWorldManager->addDormantNpc(creatureId, spawn->mBasic.timeToFirstSpawn);
}
*/

//=============================================================================
//
//	Handle npc.
//
//	Main purposes are to get the npc in suitable timer queue and handling generall issues

uint64 NpcManager::handleNpc(NPCObject* npc, uint64 timeOverdue)
{
    uint64 waitTime;
    uint64 newWaitTime;

    // Handle events.
    NPCObject::Npc_AI_State oldState = npc->getAiState();
    npc->handleEvents();

    newWaitTime = npc->handleState(timeOverdue);
    waitTime = newWaitTime;

    NPCObject::Npc_AI_State newState = npc->getAiState();
    if (newState != oldState)
    {
        waitTime = 0;
        if (newState == AttackableCreature::NpcIsDormant)
        {
            gWorldManager->addDormantNpc(npc->getId(), newWaitTime);
        }
        else if (newState == AttackableCreature::NpcIsReady)
        {
            gWorldManager->addReadyNpc(npc->getId(), newWaitTime);
        }
        else if (newState == AttackableCreature::NpcIsActive)
        {
            gWorldManager->addActiveNpc(npc->getId(), newWaitTime);
        }
        else
        {
            assert(false && "NpcManager::handleNpc invalid AI state");
        }
    }
    return waitTime;
}


void NpcManager::handleExpiredCreature(uint64 creatureId)
{
    CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(creatureId));
    if (creature)
    {
        creature->killEvent();
    }
}

void NpcManager::handleObjectReady(Object* object)
{
    CreatureObject* creature = dynamic_cast<CreatureObject*>(object);
    if (creature)
    {
        creature->respawn();
    }
}


//=============================================================================================================================
//
// This part is where the natural lairs are loaded from DB.
//
//=============================================================================================================================
void NpcManager::loadLairs(void)
{
    //load lair and creature spawn, and optionally heightmaps cache.
    // NpcFamily_NaturalLairs

    mDatabase->executeSqlAsync(this,new NpcAsyncContainer(NpcQuery_Lairs), "SELECT lairs.id, lairs.lair_template, lairs.count FROM %s.lairs "
                "INNER JOIN %s.spawns ON (lairs.creature_spawn_region = spawns.id) WHERE spawns.spawn_planet=%u AND lairs.family=%u ORDER BY lairs.id;",
                mDatabase->galaxy(),mDatabase->galaxy(),gWorldManager->getZoneId(), NpcFamily_NaturalLairs);
   
}

void NpcManager::handleDatabaseJobComplete(void* ref, DatabaseResult* result)
{
    NpcAsyncContainer* asyncContainer = reinterpret_cast<NpcAsyncContainer*>(ref);
    switch(asyncContainer->mQuery)
    {
    case NpcQuery_Lairs:
    {
        // warning... this assumes that someone has called the Init()-function before we execute the line below.
        // For now, it's done at ZoneServer::Startup().
        NonPersistentNpcFactory* nonPersistentNpcFactory = NonPersistentNpcFactory::Instance();

        NpcLairEntity lair;

        // Here we will get the lair type.
        DataBinding* lairSpawnBinding = mDatabase->createDataBinding(3);
        lairSpawnBinding->addField(DFT_uint64,offsetof(NpcLairEntity,mLairsId),8,0);
        lairSpawnBinding->addField(DFT_uint64,offsetof(NpcLairEntity,mLairTemplateId),8,1);
        lairSpawnBinding->addField(DFT_uint32,offsetof(NpcLairEntity,mNumberOfLairs),4,2);

        uint64 count = result->getRowCount();

        for (uint64 i = 0; i < count; i++)
        {
            result->getNextRow(lairSpawnBinding,&lair);

            for (uint64 lairs = 0; lairs < lair.mNumberOfLairs; lairs++)
            {
                // We need two id's in sequence, since nps'c have an inventory.
                uint64 npcNewId = gWorldManager->getRandomNpNpcIdSequence();

                if (npcNewId != 0)
                {
                    nonPersistentNpcFactory->requestLairObject(this, lair.mLairsId, npcNewId);
                }
            }
        }
        mDatabase->destroyDataBinding(lairSpawnBinding);
    }
    break;

    default:
    {
    }
    break;
    }

}


//=============================================================================================================================
//
// This is temp code that shall be integrated with the CombatManager.
//
//=============================================================================================================================







//=============================================================================================================================
//
// verify combat state
//

bool NpcManager::_verifyCombatState(CreatureObject* attacker, uint64 defenderId)
{
	if (!attacker || !defenderId)
	{
		return false;
	}

	CreatureObject* defender = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(defenderId));
	if (!defender)
	{
		return false;
	}

	// If the target (defender) do have me in his defender list, we should not bother.
	/*
	if (defender->checkDefenderList(attacker->getId()))
	{
		return true;
	}
	*/

	PlayerObject* playerAttacker = dynamic_cast<PlayerObject*>(attacker);

	// make sure we got both objects
	if (playerAttacker && defender)
	{
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
			if(!playerAttacker->states.checkState(CreatureState_Combat))
			{
                gStateManager.setCurrentActionState(playerAttacker,CreatureState_Combat);
                gStateManager.setCurrentActionState(playerAttacker,CreatureState_CombatAttitudeNormal);
			}

			// put our target in combat state
			if(!defenderPlayer->states.checkState(CreatureState_Combat))
			{
				gStateManager.setCurrentActionState(defenderPlayer,CreatureState_Combat);
                gStateManager.setCurrentActionState(defenderPlayer,CreatureState_CombatAttitudeNormal);
			}

			// update our defender list
			if (!playerAttacker->checkDefenderList(defenderPlayer->getId())) // or if (!playerAttacker->checkDefenderList(defenderId)
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

			// Vefify that our target is attackable. It's not nice to spam attacks at innocent npc's.
			if (!(defender->getPvPStatus() & CreaturePvPStatus_Attackable))
			{
				return(false);
			}

			// put us in combat state
			if (!playerAttacker->states.checkState(CreatureState_Combat))
			{
				gStateManager.setCurrentActionState(playerAttacker,CreatureState_Combat);
                gStateManager.setCurrentActionState(playerAttacker,CreatureState_CombatAttitudeNormal);
			}

			// put our target in combat state
			if (!defender->states.checkState(CreatureState_Combat))
			{
				gStateManager.setCurrentActionState(defender,CreatureState_Combat);
                gStateManager.setCurrentActionState(defender,CreatureState_CombatAttitudeNormal);
			}

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
				gMessageLib->sendDefenderUpdate(defender,1,defender->getDefenders()->size() - 1,playerAttacker->getId());
			}
		}
		return(true);
	}
	else if (AttackableCreature* attackerNpc = dynamic_cast<AttackableCreature*>(attacker))
	{
		// make sure we got both objects
		if (attackerNpc && defender)
		{
			// Our target can be a player or another npc.
			if (PlayerObject* defenderPlayer = dynamic_cast<PlayerObject*>(defender))
			{
				// The target (defender) is a player. Kill him!

				if (defenderPlayer->isIncapacitated())
				{
					// gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_incap");
					return(false);
				}
				else if(defenderPlayer->isDead())
				{
					return(false);
				}

				/*
				if (!defenderPlayer->checkPvPState(CreaturePvPStatus_Attackable))
				{
					// Player is not attackable
					return(false);
				}
				*/

				// put us in combat state
				if (!attackerNpc->states.checkState(CreatureState_Combat))
				{
					gStateManager.setCurrentActionState(attackerNpc,CreatureState_Combat);
                    gStateManager.setCurrentActionState(attackerNpc,CreatureState_CombatAttitudeNormal);
				}

				// put our target in combat stance
				if (!defenderPlayer->states.checkState(CreatureState_Combat))
				{
					gMessageLib->sendUpdatePvpStatus(defenderPlayer,defenderPlayer, defenderPlayer->getPvPStatus() | CreaturePvPStatus_Attackable | CreaturePvPStatus_Aggressive); //  | CreaturePvPStatus_Enemy);

					gStateManager.setCurrentActionState(defenderPlayer,CreatureState_Combat);
                    gStateManager.setCurrentActionState(defenderPlayer,CreatureState_CombatAttitudeNormal);

				}

				// If the target (defender) is not on our list, update pvp-status.

				if (!attackerNpc->checkDefenderList(defenderPlayer->getId()))
				{

					// May not be neeeded, since the pvp-status is changed when getting enough aggro.
					// gMessageLib->sendUpdatePvpStatus(attackerNpc,defenderPlayer, attackerNpc->getPvPStatus() | CreaturePvPStatus_Attackable | CreaturePvPStatus_Aggressive | CreaturePvPStatus_Enemy);

					// update our defender list
					attackerNpc->addDefender(defenderPlayer->getId());
					gMessageLib->sendDefenderUpdate(attackerNpc,1,attackerNpc->getDefenders()->size() - 1,defenderPlayer->getId());

					// Update player and all his group mates currently in range.
					/*
					PlayerList inRangeMembers = defenderPlayer->getInRangeGroupMembers(true);
					PlayerList::iterator it	= inRangeMembers.begin();
					while (it != inRangeMembers.end())
					{
						PlayerObject* player = (*it);

						// If player online, send emote.
						if (player && player->isConnected())
						{
							MessageLib->sendUpdatePvpStatus(attackerNpc,player);
						}
					}
					*/
				}

				// update our targets defender list
				if (!defenderPlayer->checkDefenderList(attackerNpc->getId()))
				{
					defenderPlayer->addDefender(attackerNpc->getId());
					gMessageLib->sendDefenderUpdate(defenderPlayer,1,defenderPlayer->getDefenders()->size() - 1, attackerNpc->getId());
				}

				// Player can/may start auto-attack if idle.
				if (!defenderPlayer->autoAttackEnabled())
				{
					defenderPlayer->getController()->enqueueAutoAttack(attackerNpc->getId());
				}
			}
			else if (dynamic_cast<AttackableCreature*>(defender))
			{
				// The target (defender) is a npc. Kill him!
				// Here I think some validation would be great, don't wanna kill you piket-brother?

				if (defenderPlayer->isIncapacitated())
				{
					// gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_incap");
					return(false);
				}
				else if(defenderPlayer->isDead())
				{
					return(false);
				}

				// put us in combat state
				if (!attackerNpc->states.checkState(CreatureState_Combat))
				{
					gStateManager.setCurrentActionState(attackerNpc,CreatureState_Combat);
                    gStateManager.setCurrentActionState(attackerNpc,CreatureState_CombatAttitudeNormal);
				}

				// put our target in combat state
				if (!defenderPlayer->states.checkState(CreatureState_Combat))
				{
					gStateManager.setCurrentActionState(defenderPlayer,CreatureState_Combat);
                    gStateManager.setCurrentActionState(defenderPlayer,CreatureState_CombatAttitudeNormal);
				}

				// update our defender list
				if (!attackerNpc->checkDefenderList(defenderPlayer->getId()))
				{
					attackerNpc->addDefender(defenderPlayer->getId());
					gMessageLib->sendDefenderUpdate(attackerNpc,1,attackerNpc->getDefenders()->size() - 1,defenderPlayer->getId());
				}

				// update our targets defender list
				if (!defenderPlayer->checkDefenderList(attackerNpc->getId()))
				{
					defenderPlayer->addDefender(attackerNpc->getId());
					gMessageLib->sendDefenderUpdate(defenderPlayer,1,defenderPlayer->getDefenders()->size() - 1, attackerNpc->getId());
				}
			}
			return(true);
		}
		return(false);
	}
	return false;
}


//======================================================================================================================

bool NpcManager::handleAttack(CreatureObject *attacker, uint64 targetId) // , ObjectControllerCmdProperties *cmdProperties)
{
    CreatureObject* defender = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetId));

    if (_verifyCombatState(attacker, targetId))
    {
        // get the current weapon
        Weapon* weapon = dynamic_cast<Weapon*>(attacker->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hold_Left));

        if (!weapon)
        {
            return(false);
        }

        // do the attack
        /*uint8 attackResult = */_executeAttack(attacker, defender, weapon);

        return(true);
    }

    return(false);
}

//======================================================================================================================

// uint8 CombatManager::_executeAttack(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon)
uint8 NpcManager::_executeAttack(CreatureObject* attacker,CreatureObject* defender,Weapon* weapon)
{
    uint8	randomHitPool			= 100;
    //uint8	randomPoolHitChance		= 100;
    //uint8	stateApplied			= 0;
    int32	multipliedDamage		= 0;
    BString	combatSpam				= "melee";

    // first see if we actually hit our target
    // uint8 attackResult = _hitCheck(attacker,defender,cmdProperties,weapon);
    uint8 attackResult = _hitCheck(attacker,defender,weapon);

    // only proceed, if so
    if (!attackResult)
    {
        // TODO: retrieve from weapon
        AttackableCreature* attackerNpc = dynamic_cast<AttackableCreature*>(attacker);
        if (!attackerNpc)
        {
            return 0;
        }

        int32 baseMinDamage	= attackerNpc->getMinDamage();
        int32 baseMaxDamage	= attackerNpc->getMaxDamage();

        int32 baseDamage	= -((gRandom->getRand()%(baseMaxDamage - baseMinDamage)) + baseMinDamage);

        // apply damage multiplier
        /*
        if(cmdProperties->mDamageMultiplier)
        {
        	multipliedDamage = baseDamage * cmdProperties->mDamageMultiplier;
        }
        else
        {
        	multipliedDamage = baseDamage;
        }
        */
        multipliedDamage = baseDamage;

        // mitigation
        // multipliedDamage = _mitigateDamage(attacker,defender,cmdProperties,multipliedDamage,weapon);

        // state effects
        // stateApplied = _tryStateEffects(attacker,defender,cmdProperties,weapon);

        // ham damage
        // if no target pool set, pick a random one
        // if (!cmdProperties->mHealthHitChance && !cmdProperties->mActionHitChance && !cmdProperties->mMindHitChance)
        {
            switch(gRandom->getRand()%3)
            {
            case 0:
                randomHitPool = HamBar_Health;
                break;
            case 1:
                randomHitPool = HamBar_Action;
                break;
            case 2:
                randomHitPool = HamBar_Mind;
                break;

            default:
                randomHitPool = 0;
                break;
            }
        }

        if (defender->getCreoGroup() != CreoGroup_AttackableObject)
        {
            // random pool attack
            if(randomHitPool != 100)
            {
                defender->getHam()->updatePropertyValue(randomHitPool,HamProperty_CurrentHitpoints,multipliedDamage,true);
            }
            // direct pool attack
            /*
            else
            {
            	// health hit
            	if(cmdProperties->mHealthHitChance)
            	{
            		defender->getHam()->updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints,multipliedDamage,true);
            	}
            	// action hit
            	else if(cmdProperties->mActionHitChance)
            	{
            		defender->getHam()->updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints,multipliedDamage,true);
            	}
            	// mind hit
            	else if(cmdProperties->mMindHitChance)
            	{
            		defender->getHam()->updatePropertyValue(HamBar_Mind,HamProperty_CurrentHitpoints,multipliedDamage,true);
            	}
            }
            */
        }
        else
        {
            defender->getHam()->updateSingleHam(multipliedDamage, true);
        }
        /*
        if (defender->isIncapacitated())
        {
        	PlayerObject* playerAttacker = dynamic_cast<PlayerObject*>(attacker);
        	if (playerAttacker && playerAttacker->isConnected())
        	{
        		if (defender->getType() == ObjType_Player)
        		{
        			int8 str[128];
        			if (defender->getLastName().getLength())
        			{
        				sprintf(str,"%s %s", defender->getFirstName().getAnsi(), defender->getLastName().getAnsi());
        			}
        			else
        			{
        				sprintf(str,"%s", defender->getFirstName().getAnsi());
        			}
        			BString playerName(str);
        			playerName.convert(BSTRType_Unicode16);

        			gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_incap", "", "", L"", 0, "", "", playerName);
        		}
        		else
        		{
        			gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_incap", "", "", L"", 0, defender->getSpeciesGroup(), defender->getSpeciesString());
        		}
        	}
        }

        if (defender->isDead())
        {
        	PlayerObject* playerAttacker = dynamic_cast<PlayerObject*>(attacker);
        	if (playerAttacker && playerAttacker->isConnected())
        	{
        		if (defender->getType() == ObjType_Player)
        		{
        			int8 str[128];
        			if (defender->getLastName().getLength())
        			{
        				sprintf(str,"%s %s", defender->getFirstName().getAnsi(), defender->getLastName().getAnsi());
        			}
        			else
        			{
        				sprintf(str,"%s", defender->getFirstName().getAnsi());
        			}
        			BString playerName(str);
        			playerName.convert(BSTRType_Unicode16);
        			gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_dead", "", "", L"", 0, "", "", playerName);
        		}
        		else
        		{
        			gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_dead", "", "", L"", 0, defender->getSpeciesGroup(), defender->getSpeciesString());
        		}
        	}
        }
        */
    }

    // fly text and animations
    // default attack(s)
    // Assume we only use the default attack for now.
    // if (cmdProperties->mCmdCrc == 0xa8fef90a)
    {
        // This is just a temp fix, we need to solve how to differ creatures from human npc's.
        uint32 animCrc;
        if (gWorldConfig->isTutorial())
        {
            animCrc = gCombatManager->getDefaultAttackAnimation(weapon->getGroup());
        }
        else
        {
            // creature_attack_light
            animCrc = 0x8BF5B8B6;
        }
        // creature_attack_light 0xB6B8F58B (reversed from WIKI 0x8BF5B8B6)
        // creature_attack_medium 0x48CBE352 ( 0x52E3CB48 )
        // creature_attack_special_1_medium 0x24AEEF7A
        // creature_attack_special_1_light 0x13E6B2DE
        // creature_attack_special_2_medium 0x7ACB0D96
        // creature_attack_special_2_light 0x205E4B0F
        // creature_attack_ranged_light 0x8D49FC90
        // creature_attack_ranged_medium 0x59E0483C


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
    /*
    else
    {
    	switch(attackResult)
    	{
    		// hit
    		case 0:case 2:case 3:case 4:
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
    */

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
        // gMessageLib->sendFlyText(defender,"combat_effects","block",0,255,0);
    }
    // break;

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
    // if(cmdProperties->mCmdCrc == 0xa8fef90a)
    {
        // combatSpam = gCombatManager->getDefaultSpam(weapon->getGroup());
        combatSpam = "creature";
    }
    // special attack
    /*
    else
    {
    	if(cmdProperties->mCbtSpam.getLength())
    	{
    		combatSpam = cmdProperties->mCbtSpam.getAnsi();
    	}
    }
    */

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
//
// see if the attack hits, misses, is blocked or evaded
//

// uint8 NpcManager::_hitCheck(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon)
uint8 NpcManager::_hitCheck(CreatureObject* attacker,CreatureObject* defender,Weapon* weapon)
{
    // TODO
    // If the defender is a static object, like lair or debis, it can't counterattack, block or evade.
    uint8 hit = 0;
    if (defender->getCreoGroup() == CreoGroup_AttackableObject)
    {
        // Note that a return of 0 is a hit, return of 1 is a miss.
        hit = gRandom->getRand()%4;	// 75% chance for a hit, 25% chance for a miss. That sounds fair when hittin a static object,
        if (hit > 1)
        {
            // This is a "hit".
            hit = 0;
        }
    }
    else
    {
        // It's a "normal" npc or player

        // return(gRandom->getRand()%5);
        // Adjusting the hit chance, when testing.
        // hit = gRandom->getRand()%9;	// 66% chance for a hit, 33% chance for the rest (miss, block, counterattck and evade)
        hit = gRandom->getRand()%12;	// 75% chance for a hit, 25% chance for the rest (miss, block, counterattck and evade)
        if (hit > 4)
        {
            // This is a "hit".
            hit = 0;
        }
    }
    return(hit);
}


