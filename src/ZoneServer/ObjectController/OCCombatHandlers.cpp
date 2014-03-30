/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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
#include "Zoneserver/Objects/AttackableCreature.h"
#include "Zoneserver/Objects/Datapad.h"
#include <Zoneserver/Objects/Inventory.h>
#include <ZoneServer\Objects\Item.h>
//#include "ZoneServer/GameSystemManagers/Mission Manager/MissionManager.h"
#include "ZoneServer/GameSystemManagers/Mission Manager/MissionObject.h"
#include "Zoneserver/ObjectController/ObjectController.h"
#include "ZoneServer/ObjectController/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/Objects/Object/ObjectFactory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/GameSystemManagers/State Manager/StateManager.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"
#include "ZoneServer\GameSystemManagers\Group Manager\GroupManager.h"

#include "ZoneServer\Services\equipment\equipment_service.h"

#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include "ZoneServer\Services\ham\ham_service.h"
#include "anh/app/swganh_kernel.h"
#include "anh\service\service_manager.h"

//=============================================================================================================================
//
// duel
//

void ObjectController::_handleDuel(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); 
	PlayerObject* player = creature->GetGhost();
    Object*				target	= gWorldManager->getObjectById(targetId);

	auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

    // make sure we got a target and that its a player
    if(target && target->getType() == ObjType_Player)
    {
        PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(target);

		// if our target is dead
		if (targetPlayer->GetCreature()->isDead()){
			gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "target_already_dead", 0, targetId, 0), player);
			return;
		}

        // don't duel ourself
        if(player == targetPlayer || (!ham->checkMainPools(targetPlayer->GetCreature(), 1, 1, 1)))
        {
            return;
        }

		// if we are dead
		if (player->GetCreature()->isDead()){
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
                gMessageLib->sendUpdatePvpStatus(player->GetCreature(),targetPlayer,player->GetCreature()->getPvPStatus() | CreaturePvPStatus_Attackable | CreaturePvPStatus_Aggressive);
                gMessageLib->sendUpdatePvpStatus(targetPlayer->GetCreature(),player,targetPlayer->GetCreature()->getPvPStatus() | CreaturePvPStatus_Attackable | CreaturePvPStatus_Aggressive);
                gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "accept_self", 0, targetId, 0), player);
                gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "accept_target", 0, player->getId(), 0), targetPlayer);
            }
            // challenge him
            else
            {
                // If target have me ignored, auto decline the invitation.
                BString ignoreName = player->GetCreature()->getFirstName().c_str();
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
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* player = creature->GetGhost();
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
                gMessageLib->sendUpdatePvpStatus(player->GetCreature(),targetPlayer);
                gMessageLib->sendUpdatePvpStatus(targetPlayer->GetCreature(),player);

                gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "end_self", 0, targetId, 0), player);
                gMessageLib->SendSystemMessage(::common::OutOfBand("duel", "end_target", 0, player->getId(), 0), targetPlayer);

                // also clear the defender list and combat states
                if (player->GetCreature()->checkDefenderList(targetPlayer->getId()))
                {
                    // player->removeDefender(targetPlayer);
                    // gMessageLib->sendDefenderUpdate(player,0,0,targetPlayer->getId());
                    player->GetCreature()->RemoveDefender(targetPlayer->getId());

                    // no more defenders, end combat
                    if(player->GetCreature()->GetDefender().empty())
                    {
                        gStateManager.setCurrentActionState(player->GetCreature(), CreatureState_Peace);
                    }
                }

                if (targetPlayer->GetCreature()->checkDefenderList(player->getId()))
                {
                    // targetPlayer->removeDefender(player);
                    // gMessageLib->sendDefenderUpdate(targetPlayer,0,0,player->getId());
                    targetPlayer->GetCreature()->RemoveDefender(player->getId());

                    // no more defenders, end combat
                    if(targetPlayer->GetCreature()->GetDefender().empty())
                    {
                        gStateManager.setCurrentActionState(player->GetCreature(), CreatureState_Peace);
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
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* player = creature->GetGhost();
    if (player)
    {
        // player->removeAllDefender();
		auto defenderList = player->GetCreature()->GetDefender();
        auto defenderIt = defenderList.begin();
		while (defenderIt != defenderList.end())
		{
			if (CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*defenderIt))))
			{
				defenderCreature->RemoveDefender(player->getId());

				if (PlayerObject* defenderPlayer = dynamic_cast<PlayerObject*>(defenderCreature))
				{
					gMessageLib->sendUpdatePvpStatus(player->GetCreature(),defenderPlayer);
				}

				// if no more defenders, clear combat state
				if (!defenderCreature->GetDefender().size())
				{
					gStateManager.setCurrentActionState(defenderCreature, CreatureState_Peace);
				}
			}
			// If we remove self from all defenders, then we should remove all defenders from self. Remember, we are dead.
			player->GetCreature()->RemoveDefender(*defenderIt);//were using a copy
			defenderIt++;
		}

        player->setCombatTargetId(0);

        //player->toggleStateOff((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal));
        // peace state automatically removes the combat states
        gStateManager.setCurrentActionState(player->GetCreature(),CreatureState_Peace);

        //gMessageLib->sendBaselinesCREO_6(player,player);
        //gMessageLib->sendEndBaselines(player->getId(),player);
                
        player->disableAutoAttack();

        //End any duels if both players press peace
        
        PlayerList* pList = player->getDuelList();
        PlayerList::iterator it = pList->begin();

        while(it != pList->end())
        {
            // check the target's peace state
            if (!(*it)->GetCreature()->states.checkState(CreatureState_Combat) )
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
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* player = creature->GetGhost();

    if (player && targetId)	// Any object targeted?
    {
        // Do we have a valid target?
        if (PlayerObject* target = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId)))
        {
            if (target->GetCreature()->isIncapacitated())
            {
                // Are we able to perform the DB?
                if (!player->GetCreature()->isIncapacitated() && !player->GetCreature()->isDead())
                {
                    // Do we have the executioner in targets duel list?
                    if (target->checkDuelList(player))
                    {
                        // here we go... KILL HIM!
						gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "death_blow"), player);
                        target->GetCreature()->die();
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
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* player = creature->GetGhost();
    Datapad* datapad			= player->getDataPad();

    // Loot creatures
    this->lootAll(targetId, player);

    MissionList::iterator it = datapad->getMissions()->begin();
    while(it != datapad->getMissions()->end())
    {
       MissionObject* mission = dynamic_cast<MissionObject*>(*it);
       if(mission->getMissionType() != destroy) { ++it; continue; }

       if (glm::distance(player->GetCreature()->mPosition, mission->getDestination().Coordinates) < 20)
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
//	Loot a creature of all items and credits, if possible.
//
//	Used both from _handleLoot() and radil's "loot all".
//

void ObjectController::lootAll(uint64 targetId, PlayerObject* playerObject)
{
    // First, we have to have a connected player and a valid source to loot.
    AttackableCreature* creatureObject = dynamic_cast<AttackableCreature*>(gWorldManager->getObjectById(targetId));

	auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	// Creature Inventory.
	auto inventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(creatureObject, "inventory"));

	// Player Inventory.
	auto playerInventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(playerObject->GetCreature(), "inventory"));

    // AttackableCreature* npcObject = dynamic_cast<NPCObject*>(gWorldManager->getObjectById(targetId));
    if (creatureObject && playerObject->isConnected() && !playerObject->GetCreature()->isDead() && !playerObject->GetCreature()->isIncapacitated() && (creatureObject->getNpcFamily() == NpcFamily_AttackableCreatures) && creatureObject->isDead())
    {

        if (creatureObject->allowedToLoot(playerObject->getId(), playerObject->GetCreature()->getGroupId()))
        {            


			if (inventory && playerInventory)            {
                // Looks like we have valid input, now handle it!

                int32 lootedCredits = inventory->getCredits();
                inventory->setCredits(0);

                // Get all items from creature inventory.
                int32 lootedItems = 0;
				
				inventory->ViewObjects(creatureObject, 0, true, [&] (Object* object) {

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
							gContainerManager->destroyObjectToRegisteredPlayers(inventory, object->getId(), true);
				
                        }
                        lootedItems++;
                    }
                    
                });

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
                        if (playerObject->GetCreature()->getGroupId() != 0)
                        {
                            // We are grouped. Split the credits with the group members in range.
                            ObjectList inRangeMembers = gGroupManager->getInRangeGroupMembers(false);

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
                                auto it	= inRangeMembers.begin();
                                while (it != inRangeMembers.end())	{
                                    // "[GROUP] You receive %DI credits as your share."
									PlayerObject* member = dynamic_cast<PlayerObject*>(*it);
									if(member)	{
										gMessageLib->SendSystemMessage(::common::OutOfBand("group", "prose_split", 0, 0, 0, splittedCredits), member);

										// Now we need to add the credits to player inventory.
										auto groupmember_Inventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(member->GetCreature(), "inventory"));
										if (groupmember_Inventory)                                    {
											groupmember_Inventory->updateCredits(splittedCredits);
										}
										totalProse -= splittedCredits;
									}
                                    ++it;
                                }

                                int8 str[64];
                                sprintf(str,"%u", totalProse);
                                BString splitedLootCreditsString(str);
                                splitedLootCreditsString.convert(BSTRType_Unicode16);

                                // "GROUP] You split %TU credits and receive %TT credits as your share."
                                gMessageLib->SendSystemMessage(::common::OutOfBand("group", "prose_split_coins_self", splitedLootCreditsString.getUnicode16(), lootCreditsString.getUnicode16(), L""), playerObject);

                                // Now we need to add the credits to our own inventory.                                
                                if (playerInventory)                                {
                                    playerInventory->updateCredits(totalProse);
                                }
                            }
                        }
                        else
                        {
                            gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_coin_loot_no_target", 0, 0, 0, lootedCredits), playerObject);

                            // Now we need to add the credits to our own inventory.                 
                            if (playerInventory)                            {
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
