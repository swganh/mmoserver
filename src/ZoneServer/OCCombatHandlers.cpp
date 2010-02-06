/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "AttackableCreature.h"
#include "Datapad.h"
#include "Inventory.h"
#include "MissionManager.h"
#include "MissionObject.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Common/Message.h"
#include "Common/MessageFactory.h"


//=============================================================================================================================
//
// duel
//

void ObjectController::_handleDuel(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		player	= dynamic_cast<PlayerObject*>(mObject);
	Object*				target	= gWorldManager->getObjectById(targetId);

	// make sure we got a target and that its a player
	if(target && target->getType() == ObjType_Player)
	{
		PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(target);

		// don't duel ourself
		if(player == targetPlayer)
		{
			return;
		}

		// check if he's already in our duel list
		if(player->checkDuelList(targetPlayer))
		{
			// TODO: add id
			gMessageLib->sendSystemMessage(player,L"","duel","already_challenged","","",L"",0,"","",L"",targetId);
			return;
		}
		// add him to our list
		else
		{
			// check if he already challenged us
			if(targetPlayer->checkDuelList(player))
			{
				player->addToDuelList(targetPlayer);

				// start the duel
				gMessageLib->sendUpdatePvpStatus(player,targetPlayer,player->getPvPStatus() | CreaturePvPStatus_Attackable | CreaturePvPStatus_Aggressive);
				gMessageLib->sendUpdatePvpStatus(targetPlayer,player,targetPlayer->getPvPStatus() | CreaturePvPStatus_Attackable | CreaturePvPStatus_Aggressive);

				gMessageLib->sendSystemMessage(player,L"","duel","accept_self","","",L"",0,"","",L"",targetId);
				gMessageLib->sendSystemMessage(targetPlayer,L"","duel","accept_target","","",L"",0,"","",L"",player->getId());
			}
			// challenge him
			else
			{
				// If target have me ignored, auto decline the invitation.
				string ignoreName = player->getFirstName();
				ignoreName.toLower();

				// check our ignorelist
				if(targetPlayer->checkIgnoreList(ignoreName.getCrc()))
				{
					gMessageLib->sendSystemMessage(player,L"","duel","reject_target","","",L"",0,"","",L"",targetId);
				}
				else
				{
					player->addToDuelList(targetPlayer);

					gMessageLib->sendSystemMessage(player,L"","duel","challenge_self","","",L"",0,"","",L"",targetId);
					gMessageLib->sendSystemMessage(targetPlayer,L"","duel","challenge_target","","",L"",0,"","",L"",player->getId());
				}
			}
		}
	}
}

//=============================================================================================================================
//
// endduel
//

void ObjectController::_handleEndDuel(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*		player	= dynamic_cast<PlayerObject*>(mObject);
	Object*				target	= gWorldManager->getObjectById(targetId);

	// make sure we got a target and that its a player
	if(target && target->getType() == ObjType_Player)
	{
		PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(target);

		// are we dueling him
		if(!player->checkDuelList(targetPlayer))
		{
			// nop
			gMessageLib->sendSystemMessage(player,L"","duel","not_dueling","","",L"",0,"","",L"",targetId);
			return;
		}
		// remove him from our list
		else
		{
			player->removeFromDuelList(targetPlayer);

			// see if he was dueling us
			if(targetPlayer->checkDuelList(player))
			{
				// remove us from the other guys list
				targetPlayer->removeFromDuelList(player);

				// end the duel
				gMessageLib->sendUpdatePvpStatus(player,targetPlayer);
				gMessageLib->sendUpdatePvpStatus(targetPlayer,player);

				gMessageLib->sendSystemMessage(player,L"","duel","end_self","","",L"",0,"","",L"",targetId);
				gMessageLib->sendSystemMessage(targetPlayer,L"","duel","end_target","","",L"",0,"","",L"",player->getId());

				// also clear the defender list and combat states
				if (player->checkDefenderList(targetPlayer->getId()))
				{
					// player->removeDefender(targetPlayer);
					// gMessageLib->sendDefenderUpdate(player,0,0,targetPlayer->getId());
					player->removeDefenderAndUpdateList(targetPlayer->getId());

					// no more defenders, end combat
					if(player->getDefenders()->empty())
					{
						player->toggleStateOff((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal));
						gMessageLib->sendStateUpdate(player);
						//WARNING WHAT FOLLOWS IS A DIRTY HACK TO GET STATES CLEARING ON COMBAT END
							//At some point negative states should be handled either by the buff manager as short duration buffs or via a new manager for debuffs
		
							//not in combat clear all temp combat states from player
							// player->toggleStateOff(CreatureState_Dizzy);
							// gMessageLib->sendStateUpdate(player);

							// player->toggleStateOff(CreatureState_Blinded);
							// gMessageLib->sendStateUpdate(player);

							// player->toggleStateOff(CreatureState_Stunned);
							// gMessageLib->sendStateUpdate(player);

							// player->toggleStateOff(CreatureState_Intimidated);
							// gMessageLib->sendStateUpdate(player);

							// player->setPosture(CreaturePosture_Upright);
							// gMessageLib->sendPostureUpdate(player);
							// gMessageLib->sendSelfPostureUpdate(player);

							// gMessageLib->sendSystemMessage(player,L"All states cleared - dirty hack - will fix later");

							//END OF DIRTY COMBAT STATE HACK
					}
				}

				if (targetPlayer->checkDefenderList(player->getId()))
				{
					// targetPlayer->removeDefender(player);
					// gMessageLib->sendDefenderUpdate(targetPlayer,0,0,player->getId());
					targetPlayer->removeDefenderAndUpdateList(player->getId());

					// no more defenders, end combat
					if(targetPlayer->getDefenders()->empty())
					{
						targetPlayer->toggleStateOff((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal));
						gMessageLib->sendStateUpdate(targetPlayer);
						//WARNING WHAT FOLLOWS IS A DIRTY HACK TO GET STATES CLEARING ON COMBAT END
							//At some point negative states should be handled either by the buff manager as short duration buffs or via a new manager for debuffs

							//not in combat clear all temp combat states from target player
							// targetPlayer->toggleStateOff(CreatureState_Dizzy);
							// gMessageLib->sendStateUpdate(targetPlayer);

							// targetPlayer->toggleStateOff(CreatureState_Blinded);
							// gMessageLib->sendStateUpdate(targetPlayer);

							// targetPlayer->toggleStateOff(CreatureState_Stunned);
							// gMessageLib->sendStateUpdate(targetPlayer);

							// targetPlayer->toggleStateOff(CreatureState_Intimidated);
							// gMessageLib->sendStateUpdate(targetPlayer);

							// targetPlayer->setPosture(CreaturePosture_Upright);
							// gMessageLib->sendPostureUpdate(targetPlayer);
							// gMessageLib->sendSelfPostureUpdate(targetPlayer);

							// gMessageLib->sendSystemMessage(targetPlayer,L"All states cleared - dirty hack - will fix later");
							//END OF DIRTY COMBAT STATE HACK

					}
				}
			}
		}
	}
}

//=============================================================================================================================
//
//	Make peace with everything, stop auto-attack.
//	Remove all defenders from player defender list.
// 
//	Current enemies can very well start attcking this player again.
// 
// 

void ObjectController::_handlePeace(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
	if (player)
	{
		// player->removeAllDefender();
		player->mDefenders.clear();

		gMessageLib->sendBaselinesCREO_6(player,player);
		gMessageLib->sendEndBaselines(player->getPlayerObjId(),player);
		
		// gMessageLib->sendDefenderUpdate(player,4,0,0);

		player->setCombatTargetId(0);

		player->toggleStateOff((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal));
		player->toggleStateOn(CreatureState_Peace);
		gMessageLib->sendStateUpdate(player);
		player->disableAutoAttack();
	}
}

//=============================================================================================================================
//
// set target
//

void ObjectController::setTarget(Message* message)
{
	CreatureObject*  creatureObject = dynamic_cast<CreatureObject*>(mObject);

	//creatureObject->setTarget(gWorldManager->getObjectById(message->getUint64()));
	creatureObject->setTarget(message->getUint64());	
	// There is a reason we get data like targets from the client, as handlers (id's) instead of references (pointers).

	// gLogger->logMsgF("ObjectController::setTarget: Object %"PRIu64" targets = %"PRIu64"", MSG_NORMAL, creatureObject->getId(), creatureObject->getTargetId());

	gMessageLib->sendTargetUpdateDeltasCreo6(creatureObject);
} 

//=============================================================================================================================
//
// death blow
//
//	Right now, we will only DeathBlow players that are incapacitated and active duelling.
// 

void ObjectController::_handleDeathBlow(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// gLogger->logMsgF("ObjectController::_handleDeathBlow: targetId = %"PRIu64"", MSG_NORMAL, targetId);
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);

	if (player && targetId)	// Any object targeted?
	{
		// Do we have a valid target?
		if (PlayerObject* target = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId)))
		{
			if (target->isIncapacitated())
			{
				// Are we able to perform the DB?
				if (!player->isIncapacitated() && !player->isDead())
				{
					// Do we have the executioner in targets duel list?
					if (target->checkDuelList(player))
					{
						// here we go... KILL HIM!
						target->die();
					}
				}
			}
		}
	}
}

//=============================================================================================================================
//
// loot
//

void ObjectController::_handleLoot(uint64 targetId, Message *message, ObjectControllerCmdProperties *cmdProperties)
{
	// gLogger->logMsgF("ObjectController::_handleLoot: targetId = %"PRIu64"",MSG_NORMAL, targetId);
	
	PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
	Datapad* datapad = dynamic_cast<Datapad*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));

	// Loot creatures
	this->lootAll(targetId, player);

	MissionList::iterator it = datapad->getMissions()->begin();
	while(it != datapad->getMissions()->end())
	{
	   MissionObject* mission = dynamic_cast<MissionObject*>(*it);
	   if(mission->getMissionType() != destroy) { ++it; continue; }
	   if(player->mPosition.inRange2D(mission->getDestination().Coordinates,20))
	   {
			gMessageLib->sendPlayClientEffectLocMessage("clienteffect/combat_explosion_lair_large.cef",mission->getDestination().Coordinates,player);
			gMissionManager->missionComplete(player,mission);

			it = datapad->removeMission(it);
			delete mission;
	   }
	   else
	   {
			++it;
	   }
	   
	}

return;
}
//=============================================================================================================================
//
// Selected the pre-designated coining facility.
//

void ObjectController::cloneAtPreDesignatedFacility(PlayerObject* player, SpawnPoint* spawnPoint)
{
	if (player)
	{
		// Copy wounds data from clone to character-table.

		// There is noo need to do it, we will save the correct in DB when we store the player data.
		// And... pick a better name for the sp_.. below... like updateWoundsWithCloneData-something....
		// int8 sql_sp[128];
		// sprintf(sql_sp,"call swganh.sp_CharacterActivateClone(%I64u)", player->getId());
		// (gWorldManager->getDatabase())->ExecuteSqlAsync(NULL,NULL,sql_sp);

		// Update player objct with new data for wounds.
		ObjControllerAsyncContainer* asyncContainer;
		
		asyncContainer = new ObjControllerAsyncContainer(OCQuery_CloneAtPreDes);
		asyncContainer->playerObject = player;
		asyncContainer->anyPtr = (void*)spawnPoint;

		int8 sql[256];
		sprintf(sql,"SELECT health_wounds,strength_wounds,constitution_wounds,action_wounds,quickness_wounds,"
			"stamina_wounds,mind_wounds,focus_wounds,willpower_wounds"
			" FROM character_clone"
			" WHERE"
			" (character_id = %"PRIu64");",player->getId());

		mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	}
}

//=============================================================================================================================
//
//	Loot a creature of all items and credits, if possible.
// 
//	Used both from _handleLoot() and radil's "loot all".
// 

void ObjectController::lootAll(uint64 targetId, PlayerObject* playerObject)
{
	// gLogger->logMsgF("ObjectController::lootAll Handle for loot_all", MSG_NORMAL);

	// First, we have to have a connected player and a valid source to loot.
	AttackableCreature* creatureObject = dynamic_cast<AttackableCreature*>(gWorldManager->getObjectById(targetId));

	// AttackableCreature* npcObject = dynamic_cast<NPCObject*>(gWorldManager->getObjectById(targetId));
	if (creatureObject && playerObject->isConnected() && !playerObject->isDead() && !playerObject->isIncapacitated() && (creatureObject->getNpcFamily() == NpcFamily_AttackableCreatures) && creatureObject->isDead())
	{

		if (creatureObject->allowedToLoot(playerObject->getId(), playerObject->getGroupId()))
		{
			// Creature Inventory.
			Inventory* inventory = dynamic_cast<Inventory*>(creatureObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

			// Player Inventory.
			Inventory* playerInventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
			
			if (inventory && playerInventory)
			{
				// Looks like we have valid input, now handle it!

				int32 lootedCredits = inventory->getCredits();
				inventory->setCredits(0);

				// Get all items from creature inventory.
				ObjectIDList*			invObjList	= inventory->getObjects();
				ObjectIDList::iterator	invObjectIt = invObjList->begin();
				int32 lootedItems = 0;
				
				
				while (invObjectIt != invObjList->end())
				{
					Object* object = gWorldManager->getObjectById((*invObjectIt));

					// Move the object to player inventory.
					Item* item = dynamic_cast<Item*>(object);
					if (item)
					{
						// TODO: Check for player inventory full, and handle containers and resource containers etc...
						// TODO: add the destroy objects to the remove Object interface at one point
						gMessageLib->sendDestroyObject(object->getId(),playerObject);
						// creatureInventory->removeObject(itemObject);

						gObjectFactory->requestNewDefaultItem(playerInventory, item->getItemFamily(), item->getItemType(), playerInventory->getId(),99,Anh_Math::Vector3(),"");
						invObjectIt = inventory->removeObject(invObjectIt);
						lootedItems++;
					}
					else
					{
						invObjectIt++;
					}
				}

				// This is used as "have we lotted anything?"
				// TODO: More accurate loot messages when there are items left, inventory full etc...
				if (lootedCredits > 0 || lootedItems > 0)
				{
					if (lootedItems > 0)
					{
						// "You have completely looted the corpse of all items."
						gMessageLib->sendSystemMessage(playerObject, L"", "base_player", "corpse_looted");
					}

					if (lootedCredits > 0)
					{
						if (playerObject->getGroupId() != 0)
						{
							// We are grouped. Split the credits with the group members in range.
							PlayerList inRangeMembers = playerObject->getInRangeGroupMembers(false);

							// Number of additional members.
							int32 noOfMembers = inRangeMembers.size();
							int32 splittedCredits = lootedCredits/(noOfMembers + 1);
											
							int8 str[64];
							sprintf(str,"%u", lootedCredits);
							string lootCreditsString(str);
							lootCreditsString.convert(BSTRType_Unicode16);

							if (splittedCredits == 0)
							{
								// To little to split.
								// "GROUP] You split %TU credits and receive %TT credits as your share."
								gMessageLib->sendSystemMessage(playerObject, L"", "group", "prose_split_coins_self", "", "", L"", 0, "", "", lootCreditsString, 0, 0, 0, "", "", lootCreditsString);
								// "There are insufficient group funds to split"
								gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "nsf_to_split");
							}
							else
							{
								int32 totalProse = lootedCredits;
								PlayerList::iterator it	= inRangeMembers.begin();
								while (it != inRangeMembers.end())
								{
									// "[GROUP] You receive %DI credits as your share."
									gMessageLib->sendSystemMessage((*it), L"", "group", "prose_split", "", "", L"", splittedCredits);
									
									// Now we need to add the credits to player inventory.
									Inventory* playerInventory = dynamic_cast<Inventory*>((*it)->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
									if (playerInventory)
									{
										playerInventory->updateCredits(splittedCredits);
									}
									totalProse -= splittedCredits;
									++it;
								}

								int8 str[64];
								sprintf(str,"%u", totalProse);
								string splitedLootCreditsString(str);
								splitedLootCreditsString.convert(BSTRType_Unicode16);

								// "GROUP] You split %TU credits and receive %TT credits as your share."
								gMessageLib->sendSystemMessage(playerObject, L"", "group", "prose_split_coins_self", "", "", L"", 0, "", "", splitedLootCreditsString, 0, 0, 0, "", "", lootCreditsString);

								// Now we need to add the credits to our own inventory.
								Inventory* playerInventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
								if (playerInventory)
								{
									playerInventory->updateCredits(totalProse);
								}
							}
						}
						else
						{
							gMessageLib->sendSystemMessage(playerObject, L"", "base_player", "prose_coin_loot_no_target", "", "", L"", lootedCredits);

							// Now we need to add the credits to our own inventory.
							Inventory* playerInventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
							if (playerInventory)
							{
								playerInventory->updateCredits(lootedCredits);
							}
						}
					}
				}
				else
				{
					// TODO: Another message for "loot all"!!!

					// @error_message:corpse_empty "You find nothing else of value on the selected corpse."
					gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "corpse_empty");
				}
				// Put this creaure in the pool of delayed destruction and remove the corpse from scene.
				gWorldManager->addCreatureObjectForTimedDeletion(creatureObject->getId(), LootedCorpseTimeout);
			}
		}
		else
		{
			// Player do not have permission to loot this corpse.
			gMessageLib->sendSystemMessage(playerObject,L"","error_message","no_corpse_permission");
		}
	}
}
