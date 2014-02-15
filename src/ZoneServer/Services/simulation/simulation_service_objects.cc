
#ifndef WIN32
#include <Python.h>
#endif

#include "simulation_service.h"

#include "swganh_core/object/building/building.h"
#include "swganh_core/object/building/building_factory.h"
#include "swganh_core/object/building/building_message_builder.h"

#include "swganh_core/object/cell/cell.h"
#include "swganh_core/object/cell/cell_factory.h"
#include "swganh_core/object/cell/cell_message_builder.h"

#include "swganh_core/object/creature/creature.h"
#include "swganh_core/object/creature/creature_factory.h"
#include "swganh_core/object/creature/creature_message_builder.h"

#include "swganh_core/object/factory_crate/factory_crate.h"
#include "swganh_core/object/factory_crate/factory_crate_factory.h"
#include "swganh_core/object/factory_crate/factory_crate_message_builder.h"

#include "swganh_core/object/group/group.h"
#include "swganh_core/object/group/group_message_builder.h"

#include "swganh_core/object/guild/guild.h"
#include "swganh_core/object/guild/guild_factory.h"
#include "swganh_core/object/guild/guild_message_builder.h"

#include "swganh_core/object/installation/installation.h"
#include "swganh_core/object/installation/installation_factory.h"
#include "swganh_core/object/installation/installation_message_builder.h"

#include "swganh_core/object/harvester_installation/harvester_installation.h"
#include "swganh_core/object/harvester_installation/harvester_installation_factory.h"
#include "swganh_core/object/harvester_installation/harvester_installation_message_builder.h"

#include "swganh_core/object/installation/installation.h"
#include "swganh_core/object/installation/installation_factory.h"
#include "swganh_core/object/installation/installation_message_builder.h"

#include "swganh_core/object/intangible/intangible.h"
#include "swganh_core/object/intangible/intangible_factory.h"
#include "swganh_core/object/intangible/intangible_message_builder.h"

#include "swganh_core/object/mission/mission.h"
#include "swganh_core/object/mission/mission_factory.h"
#include "swganh_core/object/mission/mission_message_builder.h"

#include "swganh_core/object/player/player.h"
#include "swganh_core/object/player/player_factory.h"
#include "swganh_core/object/player/player_message_builder.h"

#include "swganh_core/object/resource_container/resource_container.h"
#include "swganh_core/object/resource_container/resource_container_factory.h"
#include "swganh_core/object/resource_container/resource_container_message_builder.h"

#include "swganh_core/object/static/static.h"
#include "swganh_core/object/static/static_factory.h"
#include "swganh_core/object/static/static_message_builder.h"

#include "swganh_core/object/tangible/tangible.h"
#include "swganh_core/object/tangible/tangible_factory.h"
#include "swganh_core/object/tangible/tangible_message_builder.h"

#include "swganh_core/object/waypoint/waypoint.h"
#include "swganh_core/object/waypoint/waypoint_factory.h"
#include "swganh_core/object/waypoint/waypoint_message_builder.h"

#include "swganh_core/object/weapon/weapon.h"
#include "swganh_core/object/weapon/weapon_factory.h"
#include "swganh_core/object/weapon/weapon_message_builder.h"

#include "swganh_core/object/object_manager.h"

#include "swganh/logger.h"

using namespace swganh::simulation;
using namespace swganh::object;

void SimulationService::RegisterObjectFactories()
{
    auto object_manager = GetObjectManager();
    object_manager->RegisterObjectType<Object>();
    object_manager->RegisterObjectType<Static>();
    object_manager->RegisterObjectType<Intangible>();
    object_manager->RegisterObjectType<Installation>();
    object_manager->RegisterObjectType<HarvesterInstallation>();
    object_manager->RegisterObjectType<Mission>();
    object_manager->RegisterObjectType<Guild>();
    object_manager->RegisterObjectType<Waypoint>();
    object_manager->RegisterObjectType<Cell>();
    object_manager->RegisterObjectType<Player>();
    object_manager->RegisterObjectType<ResourceContainer>();
    object_manager->RegisterObjectType<FactoryCrate>();
    object_manager->RegisterObjectType<Weapon>();
    object_manager->RegisterObjectType<Building>();
    object_manager->RegisterObjectType<Tangible>();
    object_manager->RegisterObjectType<Creature>();
}