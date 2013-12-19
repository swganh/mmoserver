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
#include "AttackableCreature.h"
#include "Datapad.h"
#include "Inventory.h"
//#include "MissionManager.h"
#include "MissionObject.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "StateManager.h"
#include "WorldManager.h"
#include "ContainerManager.h"

#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"


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

		// if our target is dead
		if (targetPlayer->isDead()){
			gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "target_already_dead", 0, targetId, 0), player);
			return;
		}

        // don't duel ourself
        if(player == targetPlayer || !targetPlayer->getHam()->checkMainPools(1, 1, 1))
        {
            return;
        }

		// if we are dead
		if (player->isDead()){
			gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), player); // need to find the correct message
			return;
		}
		

        // check if he's already in our duel list
        if(player->checkDuelList(targetPlayer))
        {
            // TODO: add id
            gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "already_challenged", 0, targetId, 0), player);
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
                gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "accept_self", 0, targetId, 0), player);
                gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "accept_target", 0, player->getId(), 0), targetPlayer);
            }
            // challenge him
            else
            {
                // If target have me ignored, auto decline the invitation.
                BString ignoreName = player->getFirstName();
                ignoreName.toLower();

                // check our ignorelist
                if(targetPlayer->checkIgnoreList(ignoreName.getCrc()))
                {
                    gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "reject_target", 0, targetId, 0), player);
					return;
                }
                else
                {
                    player->addToDuelList(targetPlayer);
                    gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "challenge_self", 0, targetId, 0), player);
                    gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "challenge_target", 0, player->getId(), 0), targetPlayer);
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
            gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "not_dueling", 0, targetId, 0), player);
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

                gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "end_self", 0, targetId, 0), player);
                gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "end_target", 0, player->getId(), 0), targetPlayer);

                // also clear the defender list and combat states
                if (player->checkDefenderList(targetPlayer->getId()))
                {
                    // player->removeDefender(targetPlayer);
                    // gMessageLib->sendDefenderUpdate(player,0,0,targetPlayer->getId());
                    player->removeDefenderAndUpdateList(targetPlayer->getId());

                    // no more defenders, end combat
                    if(player->getDefenders()->empty())
                    {
                        gStateManager.setCurrentActionState(player, CreatureState_Peace);
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
                        gStateManager.setCurrentActionState(player, CreatureState_Peace);
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

        // gMessageLib->sendDefenderUpdate(player,4,0,0);

        player->setCombatTargetId(0);

        //player->states.toggleActionOff((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal));
        // peace state automatically removes the combat states
        gStateManager.setCurrentActionState(player,CreatureState_Peace);

        gMessageLib->sendBaselinesCREO_6(player,player);
        gMessageLib->sendEndBaselines(player->getPlayerObjId(),player);
                
        player->disableAutoAttack();

        //End any duels if both players press peace
        
        PlayerList* pList = player->getDuelList();
        PlayerList::iterator it = pList->begin();

        while(it != pList->end())
        {
            // check the target's peace state
            if (!(*it)->states.checkState(CreatureState_Combat) )
            {
                _handleEndDuel((*it)->getId(), NULL, NULL);
                it = pList->begin();
            }
            else
            {
                ++it;
            }
        }

    }
}

//=============================================================================================================================
//
// set target
//

void ObjectController::handleSetTarget(Message* message)
{
    CreatureObject*  creatureObject = dynamic_cast<CreatureObject*>(mObject);

    //creatureObject->setTarget(gWorldManager->getObjectById(message->getUint64()));
    creatureObject->setTarget(message->getUint64());	
    // There is a reason we get data like targets from the client, as handlers (id's) instead of references (pointers).

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
						gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "death_blow"), player);
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
    PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
    Datapad* datapad			= player->getDataPad();

    // Loot creatures
    this->lootAll(targetId, player);

    MissionList::iterator it = datapad->getMissions()->begin();
    while(it != datapad->getMissions()->end())
    {
       MissionObject* mission = dynamic_cast<MissionObject*>(*it);
       if(mission->getMissionType() != destroy) { ++it; continue; }

       if (glm::distance(player->mPosition, mission->getDestination().Coordinates) < 20)
       {
		/*MissionList::iterator it = datapad->getMissions()->begin();
		while(it != datapad->getMissions()->end())
		{
			MissionObject* mission = dynamic_cast<MissionObject*>(*it);
			if(mission->getMissionType() != destroy) {
				++it;
				continue;
			}

			if (glm::distance(player->mPosition, mission->getDestination().Coordinates) < 20)
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

		}*/
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
        // (gWorldManager->getDatabase())->ExecuteProcedureAsync(NULL,NULL,sql_sp);

        // Update player objct with new data for wounds.
        ObjControllerAsyncContainer* asyncContainer;
        asyncContainer = new ObjControllerAsyncContainer(OCQuery_CloneAtPreDes);
        asyncContainer->playerObject = player;
        asyncContainer->anyPtr = (void*)spawnPoint;

        int8 sql[256];
        sprintf(sql,"SELECT health_wounds,strength_wounds,constitution_wounds,action_wounds,quickness_wounds,"
                "stamina_wounds,mind_wounds,focus_wounds,willpower_wounds"
                " FROM %s.character_clone"
                " WHERE"
                " (character_id = %" PRIu64 ");",mDatabase->galaxy(),player->getId());

        mDatabase->executeSqlAsync(this,asyncContainer,sql);
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
                        //assume size is 1 slot
                        if(playerInventory->checkSlots(1))
                        {
                            gObjectFactory->requestNewDefaultItem(playerInventory, item->getItemFamily(), item->getItemType(), playerInventory->getId(), 99, glm::vec3(), "");

                            //remove from container - destroy for watching players
							gContainerManager->destroyObjectToRegisteredPlayers(inventory,(*invObjectIt), true);
				
                        }
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
                        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "corpse_looted"), playerObject);
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
                            BString lootCreditsString(str);
                            lootCreditsString.convert(BSTRType_Unicode16);

                            if (splittedCredits == 0)
                            {
                                // To little to split.
                                // "GROUP] You split %TU credits and receive %TT credits as your share."
                                gMessageLib->SendSystemMessage(::common::OutOfBand("group", "prose_split_coins_self", lootCreditsString.getUnicode16(), lootCreditsString.getUnicode16(), L""), playerObject);
                                // "There are insufficient group funds to split"
                                gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "nsf_to_split", 0, 0, 0, splittedCredits), playerObject);
                            }
                            else
                            {
                                int32 totalProse = lootedCredits;
                                PlayerList::iterator it	= inRangeMembers.begin();
                                while (it != inRangeMembers.end())
                                {
                                    // "[GROUP] You receive %DI credits as your share."
                                    gMessageLib->SendSystemMessage(::common::OutOfBand("group", "prose_split", 0, 0, 0, splittedCredits), playerObject);

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
                                BString splitedLootCreditsString(str);
                                splitedLootCreditsString.convert(BSTRType_Unicode16);

                                // "GROUP] You split %TU credits and receive %TT credits as your share."
                                gMessageLib->SendSystemMessage(::common::OutOfBand("group", "prose_split_coins_self", splitedLootCreditsString.getUnicode16(), lootCreditsString.getUnicode16(), L""), playerObject);

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
                            gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_coin_loot_no_target", 0, 0, 0, lootedCredits), playerObject);

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
                    gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "corpse_empty"), playerObject);
                }
                // Put this creaure in the pool of delayed destruction and remove the corpse from scene.
                gWorldManager->addCreatureObjectForTimedDeletion(creatureObject->getId(), LootedCorpseTimeout);
            }
        }
        else
        {
            // Player do not have permission to loot this corpse.
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "no_corpse_permission"), playerObject);
        }
    }
}
