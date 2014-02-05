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
	static void BuildInventoryCreditsDelta(CreatureObject* const  creature);
	static void BuildBankCreditsDelta(CreatureObject* const  creature);
	static void BuildStatDefenderDelta(CreatureObject* const  creature);
	static void BuildBattleFatigueDelta(CreatureObject* const  creature);
    
	
	static void BuildStatBaseDelta(CreatureObject* const  creature);
    static void BuildSkillDelta(CreatureObject* const  creature);
    static void BuildPostureDelta(CreatureObject* const  creature);
    static void BuildPostureUpdate(CreatureObject* const  creature);
    static void BuildFactionRankDelta(CreatureObject* const  creature);
    static void BuildOwnerIdDelta(CreatureObject* const  creature);
    static void BuildScaleDelta(CreatureObject* const  creature);
    
    static void BuildStateBitmaskDelta(CreatureObject* const  creature);
    static void BuildStatWoundDelta(CreatureObject* const  creature);
    static void BuildAccelerationMultiplierBaseDelta(CreatureObject* const  creature);
    static void BuildAccelerationMultiplierModifierDelta(CreatureObject* const  creature);
    static void BuildStatEncumberanceDelta( CreatureObject* const creature);
    static void BuildSkillModDelta(CreatureObject* const  creature);
    static void BuildSpeedMultiplierBaseDelta(CreatureObject* const  creature);
    static void BuildSpeedMultiplierModifierDelta(CreatureObject* const  creature);
    static void BuildListenToIdDelta(CreatureObject* const  creature);
    static void BuildRunSpeedDelta(CreatureObject* const  creature);
    static void BuildSlopeModifierAngleDelta(CreatureObject* const  creature);
    static void BuildSlopeModifierPercentDelta(CreatureObject* const  creature);
    static void BuildTurnRadiusDelta(CreatureObject* const  creature);
    static void BuildWalkingSpeedDelta(CreatureObject* const  creature);
    static void BuildWaterModifierPrecentDelta(CreatureObject* const  creature);
    static void BuildMissionCriticalObjectDelta(CreatureObject* const  creature);
    static void BuildCombatLevelDelta(CreatureObject* const  creature);
    static void BuildAnimationDelta(CreatureObject* const  creature);
    static void BuildMoodAnimationDelta(CreatureObject* const  creature);
    static void BuildWeaponIdDelta(CreatureObject* const  creature);
    static void BuildGroupIdDelta(CreatureObject* const  creature);
    static void BuildInviteSenderIdDelta(CreatureObject* const  creature);
    static void BuildGuildIdDelta(CreatureObject* const  creature);
    static void BuildTargetIdDelta(CreatureObject* const  creature);
    static void BuildMoodIdDelta(CreatureObject* const  creature);
    static void BuildPerformanceIdDelta(CreatureObject* const  creature);
    static void BuildStatCurrentDelta(CreatureObject* const  creature);
    static void BuildStatMaxDelta(CreatureObject* const  creature);
    static void BuildEquipmentDelta(CreatureObject* const  creature);
    static void BuildDisguiseDelta(CreatureObject* const  creature);
    static void BuildStationaryDelta(CreatureObject* const  creature);
    static void BuildUpdatePvpStatusMessage(const std::shared_ptr<CreatureObject>& object);

    // baselines
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline1(const std::shared_ptr<CreatureObject>& CreatureObject, boost::unique_lock<boost::mutex>& lock);
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline3(const std::shared_ptr<CreatureObject>& CreatureObject, boost::unique_lock<boost::mutex>& lock);
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline4(const std::shared_ptr<CreatureObject>& CreatureObject, boost::unique_lock<boost::mutex>& lock);
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline6(const std::shared_ptr<CreatureObject>& creature, boost::unique_lock<boost::mutex>& lock);

private:
    typedef swganh::event_dispatcher::ValueEvent<CreatureObject*> CreatureObjectEvent;
};
