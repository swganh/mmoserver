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

#include <memory>

#include "ZoneServer\Objects\Object\Object_Enums.h"
#include "ZoneServer\Objects\Creature Object\creature_message_builder.h"
#include "ZoneServer\Objects\Creature Object\CreatureObject.h"
#include "ZoneServer\Objects\Player Object\PlayerObject.h"

#include "ZoneServer/Objects/Bank.h"
#include "ZoneServer/Objects/Inventory.h"

#include "anh\event_dispatcher\event_dispatcher.h"
#include "ZoneServer\Services\equipment\equipment_service.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include "ZoneServer/WorldManager.h"

#include "MessageLib/MessageLib.h"

using namespace swganh::event_dispatcher;
using namespace swganh::messages;

void CreatureMessageBuilder::RegisterEventHandlers()
{
	event_dispatcher_->Subscribe("CreatureObject::InventoryCredits", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildInventoryCreditsDelta(value_event->Get());

    });

	event_dispatcher_->Subscribe("CreatureObject::BankCredits", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildInventoryCreditsDelta(value_event->Get());

    });

	event_dispatcher_->Subscribe("CreatureObject::BattleFatigue", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildBattleFatigueDelta(value_event->Get());

    });

	event_dispatcher_->Subscribe("CreatureObject::StatCurrent", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatCurrentDelta(value_event->Get());

    });
    event_dispatcher_->Subscribe("CreatureObject::StatMax", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatMaxDelta(value_event->Get());

    });
	event_dispatcher_->Subscribe("CreatureObject::StatEncumberance", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatEncumberanceDelta(value_event->Get());

    });
	event_dispatcher_->Subscribe("CreatureObject::StatWound", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatWoundDelta(value_event->Get());

    });
	event_dispatcher_->Subscribe("CreatureObject::StatBase", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildStatBaseDelta(value_event->Get());

    });

	event_dispatcher_->Subscribe("CreatureObject::EquipmentItem", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildEquipmentDelta(value_event->Get());
    });

	event_dispatcher_->Subscribe("CreatureObject::WeaponId", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildWeaponIdDelta(value_event->Get());
    });

	event_dispatcher_->Subscribe("CreatureObject::Skill", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildSkillDelta(value_event->Get());
    });

	event_dispatcher_->Subscribe("CreatureObject::SkillMod", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildSkillModDelta(value_event->Get());
    });

	event_dispatcher_->Subscribe("CreatureObject::Posture", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
        auto value_event = std::static_pointer_cast<CreatureObjectEvent>(incoming_event);
        BuildPostureDelta(value_event->Get());
        //BuildPostureUpdate(value_event->Get());
    });
}

void CreatureMessageBuilder::BuildWeaponIdDelta(CreatureObject* const creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildWeaponIdDelta: " << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_6, 5, SWG_CREATURE);
    message.data.write<uint64_t>(creature->GetWeaponId());
    gMessageLib->broadcastDelta(message,creature);
}

void CreatureMessageBuilder::BuildEquipmentDelta(CreatureObject* const creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildEquipmentDelta: " << creature->getId();
	DeltasMessage message = CreateDeltasMessage(creature, VIEW_6, 15, SWG_CREATURE);
	if(creature->SerializeEquipment(&message))
		gMessageLib->broadcastDelta(message,creature);
}


void CreatureMessageBuilder::BuildStatEncumberanceDelta(CreatureObject* const creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildStatEncumberanceDelta: " << creature->getId();
	DeltasMessage message = CreateDeltasMessage(creature, VIEW_4, 2, SWG_CREATURE);
	if(creature->SerializeStatEncumberances(&message))
		gMessageLib->broadcastDelta(message,creature);
}


void CreatureMessageBuilder::BuildStatCurrentDelta(CreatureObject* const  creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildStatCurrentDelta : " << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_6, 13, SWG_CREATURE);
    if(creature->SerializeCurrentStats(&message))
		gMessageLib->broadcastDelta(message,creature);
}

void CreatureMessageBuilder::BuildStatMaxDelta(CreatureObject* const  creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildStatMaxDelta : " << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_6, 14, SWG_CREATURE);
    if(creature->SerializeMaxStats(&message))
		gMessageLib->broadcastDelta(message,creature);
}

void CreatureMessageBuilder::BuildStatBaseDelta(CreatureObject* const  creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildStatBaseDelta : " << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_1, 2, SWG_CREATURE);
    if(creature->SerializeBaseStats(&message))
		gMessageLib->broadcastDelta(message,creature);
}



void CreatureMessageBuilder::BuildStatWoundDelta(CreatureObject* const  creature)
{
	LOG(info) << "CreatureMessageBuilder::BuildStatWoundDelta : " << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_3, 17, SWG_CREATURE);
    if(creature->SerializeStatWounds(&message))
		gMessageLib->broadcastDelta(message,creature);
}


void CreatureMessageBuilder::BuildBattleFatigueDelta(CreatureObject* const  creature)
{
	PlayerObject* const  player = dynamic_cast<PlayerObject*>(creature);
	if(!player)	{
		return;
	}
	LOG(info) << "CreatureMessageBuilder::BuildBattleFatigueDelta" << creature->getId();
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_3, 15, SWG_CREATURE);
    message.data.write<uint32_t>(creature->GetBattleFatigue());
    gMessageLib->sendDelta(message,player);
}


void CreatureMessageBuilder::BuildInventoryCreditsDelta(CreatureObject* const  creature)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(creature);
	if(!player)	{
		return;
	}
	auto equipment_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory = dynamic_cast<Inventory*>(equipment_service->GetEquippedObject(creature, "inventory"));

	if(!inventory)	{
		return;
	}

    DeltasMessage message = CreateDeltasMessage(creature, VIEW_1, 1, SWG_CREATURE);
    message.data.write(inventory->getCredits());
	gMessageLib->sendDelta(message,player);
	 
}

void CreatureMessageBuilder::BuildBankCreditsDelta(CreatureObject* const  creature)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(creature);
	if(!player)	{
		return;
	}

	auto equipment_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto bank = dynamic_cast<Bank*>(equipment_service->GetEquippedObject(creature, "bank"));

	if(!bank)	{
		return;
	}

    DeltasMessage message = CreateDeltasMessage(creature, VIEW_1, 0, SWG_CREATURE);
    message.data.write(bank->getCredits());
	gMessageLib->sendDelta(message,player);
	 
}

void CreatureMessageBuilder::BuildSkillModDelta(CreatureObject* const  creature)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(creature);

	//we do not want to send empty deltas!
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_4, 3, SWG_CREATURE);
    if (creature->SerializeSkillMods(&message))	{
		gMessageLib->sendDelta(message,player);
	}
    
}

void CreatureMessageBuilder::BuildSkillDelta(CreatureObject* const  creature)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(creature);

    DeltasMessage message = CreateDeltasMessage(creature, VIEW_1, 3, SWG_CREATURE);
    if (creature->SerializeSkills(&message))	{
		gMessageLib->sendDelta(message,player);
	}

}

void CreatureMessageBuilder::BuildPostureDelta(CreatureObject* const creature)
{
    DeltasMessage message = CreateDeltasMessage(creature, VIEW_3, 11, SWG_CREATURE);
    message.data.write<uint8_t>(creature->GetPosture());
    gMessageLib->broadcastDelta(message,creature);
    
}

//is only send for the player to the player
boost::optional<BaselinesMessage> CreatureMessageBuilder::BuildBaseline1(CreatureObject* const creature, boost::unique_lock<boost::mutex>& lock)
{
	auto message = CreateBaselinesMessage(creature, lock, VIEW_1, 4);

	PlayerObject* player = dynamic_cast<PlayerObject*>(creature);

	auto bank_object = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService")->GetEquippedObject(player, "bank");
    Bank* bank = dynamic_cast<Bank*>(bank_object);
	if(!bank)    {
        LOG(error) << "CreatureMessageBuilder::BuildBaseline1 bank panik";
		return BaselinesMessage(std::move(message));
    }

	auto inventory_object = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService")->GetEquippedObject(player, "inventory");
    Inventory* inventory = dynamic_cast<Inventory*>(inventory_object);
	if(!inventory)    {
        LOG(error) << "CreatureMessageBuilder::BuildBaseline1 inventory panik";
		return BaselinesMessage(std::move(message));
    }

    
	message.data.write<uint32_t>(bank->getCredits());                  // Bank Credits
	message.data.write<uint32_t>(inventory->getCredits());                  // Cash Credits
    creature->SerializeBaseStats(&message, lock);                           // Stats Negative
    creature->SerializeSkills(&message, lock);                                   // Skills
    return BaselinesMessage(std::move(message));
}

boost::optional<BaselinesMessage> CreatureMessageBuilder::BuildBaseline3( CreatureObject* const creature, boost::unique_lock<boost::mutex>& lock)
{
    auto message = CreateBaselinesMessage(creature, lock, VIEW_3, 18);
    message.data.append((*TangibleMessageBuilder::BuildBaseline3(creature, lock)).data);
	message.data.write<uint8_t>(creature->GetPosture(lock));                        // Posture
    message.data.write<uint8_t>(creature->getFactionRank());                   // Faction Rank
    message.data.write<uint64_t>(creature->owner());                      // Owner Id
    message.data.write<float>(creature->getScale());                            // Scale
    message.data.write<uint32_t>(creature->GetBattleFatigue(lock));                // Battle Fatigue
    message.data.write<uint64_t>(creature->states.getAction());                 // States Bitmask
    creature->SerializeStatWounds(&message, lock);                          // Stat Wounds
    return BaselinesMessage(std::move(message));
}

boost::optional<BaselinesMessage> CreatureMessageBuilder::BuildBaseline4(CreatureObject* const creature, boost::unique_lock<boost::mutex>& lock)
{
    auto message = CreateBaselinesMessage(creature, lock, VIEW_4, 20);
    /*
	message.data.write<float>(creature->GetAccelerationMultiplierBase(lock));         // Acceleration Multiplier Base
    message.data.write<float>(creature->GetAccelerationMultiplierModifier(lock));     // Acceleration Multiplier Modifier
    creature->SerializeStatEncumberances(&message, lock);                       // Stat Encumberances
    creature->SerializeSkillMods(&message, lock, true);                               // Skill Mods
    message.data.write<float>(creature->GetSpeedMultiplierBase(lock));                // Speed Multiplier Base
    message.data.write<float>(creature->GetSpeedMultiplierModifier(lock));            // Speed Multiplier Modifier
    message.data.write<uint64_t>(creature->GetListenToId(lock));                      // Listen To Id
    message.data.write<float>(creature->GetRunSpeed(lock));                            // Run Speed
    message.data.write<float>(creature->GetSlopeModifierAngle(lock));                 // Slope Modifier Angle
    message.data.write<float>(creature->GetSlopeModifierPercent(lock));               // Slope Modifier Percent
    message.data.write<float>(creature->GetTurnRadius(lock));                          // Turn Radius
    message.data.write<float>(creature->GetWalkingSpeed(lock));                        // Walking Speed
    message.data.write<float>(creature->GetWaterModifierPercent(lock));               // Water Modifier Percent
    creature->SerializeMissionCriticalObjects(&message, lock);                 // Mission Critical Object
	*/
	PlayerObject* player = dynamic_cast<PlayerObject*>(creature);
	gMessageLib->sendBaselinesCREO_4(player);
    return BaselinesMessage(std::move(message));
}

boost::optional<BaselinesMessage> CreatureMessageBuilder::BuildBaseline6(CreatureObject* const creature, boost::unique_lock<boost::mutex>& lock)
{
    auto message = CreateBaselinesMessage(creature, lock, VIEW_6, 23);
    
	message.data.append((*TangibleMessageBuilder::BuildBaseline6(creature, lock)).data);
	//you guys actually managed to miss the defender list ???
    
	auto	defenders		= creature->GetDefender();

	message.data.write<uint16_t>(creature->getCL());                      // Combat Level
	message.data.write<std::string>(creature->getCurrentAnimation().getAnsi());                      // Current Animation
    message.data.write<std::string>(gWorldManager->getMood(creature->getMoodId()));                 // Mood Animation
    message.data.write<uint64_t>(creature->GetWeaponId(lock));                         // Weapon Id
    message.data.write<uint64_t>(creature->getGroupId());                          // Group Id
    message.data.write<uint64_t>(0);//creature->GetInviteSenderId(lock));                  // Invite Sender Id
    message.data.write<uint64_t>(0);//creature->IncrementInviteCounter(lock));                  // Invite Sender Counter
    message.data.write<uint32_t>(0);//creature->GetGuildId(lock));                          // Guild Id
    message.data.write<uint64_t>(creature->getTargetId());                         // Target Id
    message.data.write<uint8_t>(creature->getMoodId());                            // Mood Id
    message.data.write<uint32_t>(creature->UpdatePerformanceCounter());             // Performance Update Counter
    message.data.write<uint32_t>(creature->getPerformanceId());                    // Performance Id
    creature->SerializeCurrentStats(&message, lock);										// Current Stats
    creature->SerializeMaxStats(&message, lock);											// Max Stats
    creature->SerializeEquipment(&message, lock);											// Equipment
    message.data.write<std::string>("");//creature->GetDisguise(lock));                       // Disguise Template
    message.data.write<uint8_t>(0);//creature->IsStationary(lock));                         // Stationary
	
    return BaselinesMessage(std::move(message));
}