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


#pragma once

#include <cstdint>
#include <string>
#include <boost/optional.hpp>

#include "ZoneServer\Objects\Tangible Object\tangible_message_builder.h"

namespace	swganh	{
namespace	event_dispatcher	{
	class EventDispatcher;
}
}

class CreatureObject;

class CreatureMessageBuilder : public BaseMessageBuilder
{
public:
    CreatureMessageBuilder(swganh::event_dispatcher::EventDispatcher* event_dispatcher_)
        : BaseMessageBuilder(event_dispatcher_)
    {
        RegisterEventHandlers();
    }

    virtual void RegisterEventHandlers();

    // deltas
	static void BuildInventoryCreditsDelta(const std::shared_ptr<CreatureObject>& creature);
	static void BuildBankCreditsDelta(const std::shared_ptr<CreatureObject>& creature);
	static void BuildStatDefenderDelta(const std::shared_ptr<CreatureObject>& creature);

    static void BuildStatBaseDelta(const std::shared_ptr<CreatureObject>& creature);
    static void BuildSkillDelta(const std::shared_ptr<CreatureObject>& creature);
    static void BuildPostureDelta(const std::shared_ptr<CreatureObject>& creature);
    static void BuildPostureUpdate(const std::shared_ptr<CreatureObject>& creature);
    static void BuildFactionRankDelta(const std::shared_ptr<CreatureObject>& creature);
    static void BuildOwnerIdDelta(const std::shared_ptr<CreatureObject>& CreatureObject);
    static void BuildScaleDelta(const std::shared_ptr<CreatureObject>& CreatureObject);
    static void BuildBattleFatigueDelta(const std::shared_ptr<CreatureObject>& CreatureObject);
    static void BuildStateBitmaskDelta(const std::shared_ptr<CreatureObject>& CreatureObject);
    static void BuildStatWoundDelta(const std::shared_ptr<CreatureObject>& CreatureObject);
    static void BuildAccelerationMultiplierBaseDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildAccelerationMultiplierModifierDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildStatEncumberanceDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildSkillModDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildSpeedMultiplierBaseDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildSpeedMultiplierModifierDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildListenToIdDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildRunSpeedDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildSlopeModifierAngleDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildSlopeModifierPercentDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildTurnRadiusDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildWalkingSpeedDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildWaterModifierPrecentDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildMissionCriticalObjectDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildCombatLevelDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildAnimationDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildMoodAnimationDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildWeaponIdDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildGroupIdDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildInviteSenderIdDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildGuildIdDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildTargetIdDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildMoodIdDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildPerformanceIdDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildStatCurrentDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildStatMaxDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildEquipmentDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildDisguiseDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildStationaryDelta(const std::shared_ptr<CreatureObject>& Creature);
    static void BuildUpdatePvpStatusMessage(const std::shared_ptr<CreatureObject>& object);

    // baselines
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline1(const std::shared_ptr<CreatureObject>& CreatureObject, boost::unique_lock<boost::mutex>& lock);
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline3(const std::shared_ptr<CreatureObject>& CreatureObject, boost::unique_lock<boost::mutex>& lock);
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline4(const std::shared_ptr<CreatureObject>& CreatureObject, boost::unique_lock<boost::mutex>& lock);
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline6(const std::shared_ptr<CreatureObject>& creature, boost::unique_lock<boost::mutex>& lock);

private:
    typedef swganh::event_dispatcher::ValueEvent<std::shared_ptr<CreatureObject>> CreatureObjectEvent;
};
