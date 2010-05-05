/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "CombatManager.h"

#include "AttackableCreature.h"
#include "CMWeaponGroup.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "Weapon.h"
#include "VehicleController.h"
#include "WorldManager.h"
#include "WorldConfig.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/rand.h"
//=========================================================================================

bool			CombatManager::mInsFlag		= false;
CombatManager*	CombatManager::mSingleton	= NULL;

//=========================================================================================

CombatManager::CombatManager(Database* database) :
mDatabase(database)
{
	// load default attack animations
	mWeaponGroups.reserve(10);

	mDatabase->ExecuteSqlAsync(this,0,"SELECT id,defaultAttackAnimationCrc,defaultCombatSpam FROM weapon_groups ORDER BY id");
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
	DataBinding*	binding = mDatabase->CreateDataBinding(3);

	binding->addField(DFT_uint32,offsetof(CMWeaponGroup,mId),4,0);
	binding->addField(DFT_uint32,offsetof(CMWeaponGroup,mDefaultAttackAnimationCrc),4,1);
	binding->addField(DFT_bstring,offsetof(CMWeaponGroup,mDefaultCombatSpam),64,2);

	uint64 count = result->getRowCount();

	for(uint64 i = 0;i < count;i++)
	{
		weaponGroup = new CMWeaponGroup();

		result->GetNextRow(binding,weaponGroup);

		mWeaponGroups.push_back(weaponGroup);
	}
	if(result->getRowCount())
	{
		gLogger->logMsgLoadSuccess("CombatManager::Loading %"PRIu64" weapon groups...",MSG_NORMAL, count);
	}
	else
	{
		gLogger->logMsgLoadFailure("CombatManager::Loading weapon groups...",MSG_NORMAL);
	}
	mDatabase->DestroyDataBinding(binding);
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

string CombatManager::getDefaultSpam(uint32 weaponGroup)
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
			if (!playerAttacker->checkState(CreatureState_Combat))
			{
				playerAttacker->toggleStateOn((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal));
				gMessageLib->sendStateUpdate(playerAttacker);
			}

			// put our target in combat state
			if(!defenderPlayer->checkState(CreatureState_Combat))
			{
				defenderPlayer->toggleStateOn((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal));
				gMessageLib->sendStateUpdate(defenderPlayer);
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
			// if (!playerAttacker->checkState((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal)))
			{
				// playerAttacker->togglePvPStateOn((CreaturePvPStatus)(CreaturePvPStatus_Attackable + CreaturePvPStatus_Aggressive + CreaturePvPStatus_Enemy));
				gMessageLib->sendUpdatePvpStatus(playerAttacker,playerAttacker, playerAttacker->getPvPStatus() | CreaturePvPStatus_Attackable);

				playerAttacker->toggleStateOn((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal));
				gMessageLib->sendStateUpdate(playerAttacker);

				// playerAttacker->toggleStateOn(CreatureState_Combat);
				// gMessageLib->sendStateUpdate(playerAttacker);
			}

			if (!defender->checkState((CreatureState_Combat)))
			{
				// Creature was NOT in combat before, and may very well be dormant.
				// Wake him up.
				gWorldManager->forceHandlingOfDormantNpc(defender->getId());
				gWorldManager->forceHandlingOfReadyNpc(defender->getId());

				// Creature may need some aggro built up before going into combat state??
				defender->toggleStateOn((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal));
				gMessageLib->sendStateUpdate(defender);
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

bool CombatManager::handleAttack(CreatureObject *attacker, uint64 targetId, ObjectControllerCmdProperties *cmdProperties)
{
	CreatureObject* defender = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetId));

	// get the current weapon
	Weapon* weapon = dynamic_cast<Weapon*>(attacker->getEquipManager()->getEquippedObject(CreatureEquipSlot_Weapon));
	if (!weapon)
	{
		return(false);
	}

	if (!defender)
	{
		return(false);
	}

	// NOTE: Some weapon data just for tesing and to give the npc a fair chance...
	uint32 weaponGroup = weapon->getGroup();
	float weaponRange = 6.0;

	//if (weaponGroup <= WeaponGroup_Polearm)
	//{
	//	weaponRange = 6.0;
	//}
	// else
	if (weaponGroup == WeaponGroup_Rifle)
	{
		weaponRange = 64.0;
	}
	else if (weaponGroup == WeaponGroup_Pistol)
	{
		weaponRange = 35.0;
	}
	else if (weaponGroup == WeaponGroup_Carbine)
	{
		weaponRange = 50.0;
	}
	//else
	//{
	//	weaponRange = 6.0;
	//}

	// if we are out of range, skip this attack.
    if (glm::distance(attacker->mPosition, defender->mPosition) > weaponRange)
	{
		// Target out of range.
		PlayerObject* playerAttacker = dynamic_cast<PlayerObject*>(attacker);
		if (playerAttacker && playerAttacker->isConnected())
		{
			gMessageLib->sendSystemMessage(playerAttacker,L"","error_message","target_out_of_range", "", "", L"", 0, "", "");
		}
		// It's like you shoot but missed, maintain cooldown.
		// return true;
		return false;
	}

	if(_verifyCombatState(attacker,defender->getId()))
	{

		// Execute the attack
		/*uint8 attackResult = */_executeAttack(attacker,defender,cmdProperties,weapon);

		return(true);
	}

	return(false);
}

//======================================================================================================================

uint8 CombatManager::_executeAttack(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon)
{
	uint8	randomHitPool			= 100;
	//uint8	randomPoolHitChance		= 100;
	uint8	stateApplied			= 0;
	int32	multipliedDamage		= 0;
	string	combatSpam				= "melee";

	// first see if we actually hit our target
	uint8 attackResult = _hitCheck(attacker,defender,cmdProperties,weapon);

	// only proceed, if so
	if(!attackResult)
	{
		// TODO: retrieve from weapon
		int32 baseMinDamage	= 50;
		int32 baseMaxDamage	= 100;

		// NOTE: Some weapon data just for tesing and to give the npc a fair chance...

		if (weapon->hasAttribute("cat_wpn_damage.wpn_damage_min"))
		{
			baseMinDamage = weapon->getAttribute<int32>("cat_wpn_damage.wpn_damage_min");
			// gLogger->logMsgF("CombatManager::_executeAttack Weapon baseMinDamage = %d", MSG_NORMAL, baseMinDamage);
		}
		if (weapon->hasAttribute("cat_wpn_damage.wpn_damage_max"))
		{
			baseMaxDamage = weapon->getAttribute<int32>("cat_wpn_damage.wpn_damage_max");
			// gLogger->logMsgF("CombatManager::_executeAttack Weapon baseMaxDamage = %d", MSG_NORMAL, baseMaxDamage);
		}

		int32 baseDamage	= -((gRandom->getRand()%(baseMaxDamage - baseMinDamage)) + baseMinDamage);

		// apply damage multiplier
		if(cmdProperties->mDamageMultiplier)
		{
			multipliedDamage = static_cast<uint32>(static_cast<float>(baseDamage) * cmdProperties->mDamageMultiplier);
		}
		else
		{
			multipliedDamage = baseDamage;
		}

		// mitigation
		multipliedDamage = _mitigateDamage(attacker,defender,cmdProperties,multipliedDamage,weapon);

		// state effects
		stateApplied = _tryStateEffects(attacker,defender,cmdProperties,weapon);


		// Here is the deal. When a player makes damage to a npc, we have to register the player, its group, damage done and what (kind of) weapon used.
		NPCObject* npc = dynamic_cast<NPCObject*>(defender);
		if (!defender->isDead() && npc)
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(attacker);
			if (player)
			{
                npc->updateDamage(player->getId(), player->getGroupId(), weapon->getGroup(), -multipliedDamage, player->getPosture(), glm::distance(defender->mPosition, player->mPosition));
			}
		}

		// ham damage
		// if no target pool set, pick a random one
		if(!cmdProperties->mHealthHitChance && !cmdProperties->mActionHitChance && !cmdProperties->mMindHitChance)
		{
			switch(gRandom->getRand()%3)
			{
				case 0: randomHitPool = HamBar_Health;	break;
				case 1: randomHitPool = HamBar_Action;	break;
				case 2: randomHitPool = HamBar_Mind;	break;

				default: randomHitPool = 0;				break;
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
		}
		else
		{
			defender->getHam()->updateSingleHam(multipliedDamage, true);
		}
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
					string playerName(str);
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
					string playerName(str);
					playerName.convert(BSTRType_Unicode16);
					gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_dead", "", "", L"", 0, "", "", playerName);
				}
				else
				{
					// Disabled Spam when killing creatures.
					// gMessageLib->sendSystemMessage(playerAttacker,L"","base_player","prose_target_dead", "", "", L"", 0, defender->getSpeciesGroup(), defender->getSpeciesString());
				}
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
			case 0:case 2:case 3:case 4:
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
		case 0:	combatSpam << "_hit";		break;
		case 1:	combatSpam << "_miss";		break;
		case 2:	combatSpam << "_block";		break;
		case 3:	combatSpam << "_evade";		break;
		case 4: combatSpam << "_counter";	break;

		default:break;
	}
	gMessageLib->sendCombatSpam(attacker,defender,-multipliedDamage,"cbt_spam",combatSpam);


	return(0);
}

//======================================================================================================================
//
// see if the attack hits, misses, is blocked or evaded
//

uint8 CombatManager::_hitCheck(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon)
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

//======================================================================================================================

int32 CombatManager::_mitigateDamage(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,int32 oldDamage,Weapon* weapon)
{
	// TODO
	return(oldDamage);
}

//======================================================================================================================

uint8 CombatManager::_tryStateEffects(CreatureObject* attacker,CreatureObject* defender,ObjectControllerCmdProperties *cmdProperties,Weapon* weapon)
{
	if(cmdProperties->mKnockdownChance)
	{
		defender->toggleStateOff(CreatureState_SittingOnChair);
		defender->setPosture(CreaturePosture_KnockedDown);
		defender->updateMovementProperties();
		defender->getHam()->updateRegenRates();

		gMessageLib->sendPostureAndStateUpdate(defender);
		if(PlayerObject* player = dynamic_cast<PlayerObject*>(defender))
		{
			//See if our player is mounted -- if so dismount him 
			if(player->checkIfMounted())
			{
				//Get the player's mount
				if(VehicleController* controller = dynamic_cast<VehicleController*>(gWorldManager->getObjectById(player->getMount()->getPetController())))
				{
					//Now dismount
					controller->dismountPlayer();
				}
			}

			gMessageLib->sendUpdateMovementProperties(player);
			gMessageLib->sendSelfPostureUpdate(player);
		}
	}

	if(cmdProperties->mDizzyChance)
	{
		defender->toggleStateOn(CreatureState_Dizzy);

		gMessageLib->sendStateUpdate(defender);
	}

	if(cmdProperties->mBlindChance)
	{
		defender->toggleStateOn(CreatureState_Blinded);

		gMessageLib->sendStateUpdate(defender);
	}

	if(cmdProperties->mStunChance)
	{
		defender->toggleStateOn(CreatureState_Stunned);

		gMessageLib->sendStateUpdate(defender);
	}

	if(cmdProperties->mIntimidateChance)
	{
		defender->toggleStateOn(CreatureState_Intimidated);

		gMessageLib->sendStateUpdate(defender);
	}

	if(cmdProperties->mPostureDownChance)
	{
		defender->toggleStateOff(CreatureState_SittingOnChair);
		defender->setPosture(CreaturePosture_Crouched);
		defender->updateMovementProperties();
		defender->getHam()->updateRegenRates();

		gMessageLib->sendPostureAndStateUpdate(defender);
		if(PlayerObject* player = dynamic_cast<PlayerObject*>(defender))
		{
			gMessageLib->sendUpdateMovementProperties(player);
			gMessageLib->sendSelfPostureUpdate(player);
		}
	}

	return(0);
}

//======================================================================================================================


