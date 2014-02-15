// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#ifndef WIN32
#include <Python.h>
#endif

#include "swganh_kernel.h"

#include "anh/scripting/python_shared_ptr.h"

#include "anh/event_dispatcher/event_dispatcher.h"
#include "anh/app/kernel_interface.h"
#include "anh/service/service_manager.h"

#include "ZoneServer/Services/terrain/terrain_service_interface.h"
#include "ZoneServer/Services/ham/ham_service_interface.h"
//#include "anh_core/combat/combat_service_interface.h"
//#include "swganh_core/chat/chat_service_interface.h"
//#include "swganh_core/gamesystems/gamesystems_service_interface.h"
/*#include "swganh_core/command/command_service_interface.h"
#include "swganh_core/simulation/simulation_service_interface.h"
#include "swganh_core/static/static_service_interface.h"
#include "swganh_core/sui/sui_service_interface.h"
#include "swganh_core/terrain/terrain_service_interface.h"
#include "swganh_core/equipment/equipment_service_interface.h"
#include "swganh_core/weather/weather_service_interface.h"
#include "swganh_core/simulation/spatial_provider_interface.h"
#include "swganh_core/player/player_service_interface.h"
#include "swganh_core/badge/badge_service_interface.h"
#include "swganh_core/map/map_service_interface.h"
#include "swganh_core/travel/travel_service_interface.h"
*/

#include <boost/python.hpp>
#include <anh\logger.h>

using namespace swganh::app;
using namespace boost::python;
using namespace std;

void exportSWGANHKernel()
{
	LOG(error) << "";
    class_<AppConfig, boost::noncopyable>("AppConfig")
		.add_property("script_directory", &AppConfig::script_directory, &AppConfig::script_directory)
		.add_property("galaxy_name", &AppConfig::galaxy_name, &AppConfig::galaxy_name)
        ;

    class_<KernelInterface, boost::noncopyable>("Kernel", no_init)
        ;

    class_<SwganhKernel, bases<KernelInterface>, boost::noncopyable>("SWGKernel", "Provides an interface to access the Service Manager and App Configuration", no_init)
        .def("appConfig", &swganh::app::SwganhKernel::GetAppConfig, return_internal_reference<>(), "gets the app configuration")
        .def("serviceManager", &swganh::app::SwganhKernel::GetServiceManager,return_internal_reference<>(), "Gets the application's :class:`.ServiceManager`")
        .def("eventDispatcher", &swganh::app::SwganhKernel::GetEventDispatcher, return_value_policy<reference_existing_object>(), "gets the applications :class:`.EventDispatcher`")
        ;

    class_<swganh::service::ServiceManager, boost::noncopyable>("ServiceManager", "provides an interface to common services", no_init)
	/*	.def("gamesystemsService", make_function(
               std::bind(&swganh::service::ServiceManager::GetService<swganh::gamesystems::GameSystemsServiceInterface>, std::placeholders::_1, "GameSystemsService"),
               return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::gamesystems::GameSystemsServiceInterface*, swganh::service::ServiceManager*>()),
                "returns an internal refrence of the :class:`.GameSystems`")
		.def("chatService", make_function(
               std::bind(&swganh::service::ServiceManager::GetService<swganh::chat::ChatServiceInterface>, std::placeholders::_1, "ChatService"),
               return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::chat::ChatServiceInterface*, swganh::service::ServiceManager*>()),
                "returns an internal refrence of the :class:`.ChatService`")
       .def("combatService", make_function(
               std::bind(&swganh::service::ServiceManager::GetService<swganh::combat::CombatServiceInterface>, std::placeholders::_1, "CombatService"),
               return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::combat::CombatServiceInterface*, swganh::service::ServiceManager*>()),
                "returns an internal refrence of the :class:`.CombatService`")
       .def("commandService", make_function(
               std::bind(&swganh::service::ServiceManager::GetService<swganh::command::CommandServiceInterface>, std::placeholders::_1, "CommandService"),
               return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::command::CommandServiceInterface*, swganh::service::ServiceManager*>()),
                "returns an internal refrence of the :class:`.CommandService`")
        .def("simulationService", make_function(
               std::bind(&swganh::service::ServiceManager::GetService<swganh::simulation::SimulationServiceInterface>, std::placeholders::_1, "SimulationService"),
                return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::simulation::SimulationServiceInterface*, swganh::service::ServiceManager*>()),
                "returns an internal refrence of the :class:`.SimulationService`")
		.def("suiService", make_function(
			  std::bind(&swganh::service::ServiceManager::GetService<swganh::sui::SUIServiceInterface>, std::placeholders::_1, "SuiService"),
                return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::sui::SUIServiceInterface*, swganh::service::ServiceManager*>()),
				"returns an internal refrence of the :class:`.SUIService`")
		.def("staticService", make_function(
               std::bind(&swganh::service::ServiceManager::GetService<swganh::statics::StaticServiceInterface>, std::placeholders::_1, "StaticService"),
                return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::statics::StaticServiceInterface*, swganh::service::ServiceManager*>()),
                "returns an internal refrence of the :class:`.StaticService`")
		.def("playerService", make_function(
               std::bind(&swganh::service::ServiceManager::GetService<swganh::player::PlayerServiceInterface>, std::placeholders::_1, "PlayerService"),
                return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::player::PlayerServiceInterface*, swganh::service::ServiceManager*>()),
                "returns an internal refrence of the :class:`.PlayerService`")
				*/
		.def("terrainService", make_function(
				std::bind(&swganh::service::ServiceManager::GetService<swganh::terrain::TerrainServiceInterface>, std::placeholders::_1, "TerrainService"),
                return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::terrain::TerrainServiceInterface*, swganh::service::ServiceManager*>()),
				"returns an internal refrence of the :class:`.TerrainService`")
		.def("hamService", make_function(
				std::bind(&swganh::service::ServiceManager::GetService<swganh::ham::HamServiceInterface>, std::placeholders::_1, "HamService"),
                return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::ham::HamServiceInterface*, swganh::service::ServiceManager*>()),
				"returns an internal refrence of the :class:`.HamService`")
				/*
		.def("equipmentService", make_function(
				std::bind(&swganh::service::ServiceManager::GetService<swganh::equipment::EquipmentServiceInterface>, std::placeholders::_1, "EquipmentService"),
                return_value_policy<reference_existing_object>(),
                boost::mpl::vector<swganh::equipment::EquipmentServiceInterface*, swganh::service::ServiceManager*>()),
				"returns an internal refrence of the :class:`.EquipmentService`")
		.def("weatherService", make_function(
				std::bind(&swganh::service::ServiceManager::GetService<swganh::weather::WeatherServiceInterface>, std::placeholders::_1, "WeatherService"),
				return_value_policy<reference_existing_object>(),
				boost::mpl::vector<swganh::weather::WeatherServiceInterface*, swganh::service::ServiceManager*>()),
				"returns an internal refrence of the :class:`.WeatherService`")
		.def("badgeService", make_function(
				std::bind(&swganh::service::ServiceManager::GetService<swganh::badge::BadgeServiceInterface>, std::placeholders::_1, "BadgeService"),
				return_value_policy<reference_existing_object>(),
				boost::mpl::vector<swganh::badge::BadgeServiceInterface*, swganh::service::ServiceManager*>()),
				"returns an internal reference of the :class:`.BadgeService`")
		.def("mapService", make_function(
				std::bind(&swganh::service::ServiceManager::GetService<swganh::map::MapServiceInterface>, std::placeholders::_1, "MapService"),
				return_value_policy<reference_existing_object>(),
				boost::mpl::vector<swganh::map::MapServiceInterface*, swganh::service::ServiceManager*>()),
				"returns an internal reference of the :class:`.MapService`")
		.def("travelService", make_function(
				std::bind(&swganh::service::ServiceManager::GetService<swganh::travel::TravelServiceInterface>, std::placeholders::_1, "TravelService"),
				return_value_policy<reference_existing_object>(),
				boost::mpl::vector<swganh::travel::TravelServiceInterface*, swganh::service::ServiceManager*>()),
				"returns an internal reference of the :class:.`TravelService`")
				*/
       ;
       
}
void exportEventDispatcher()
{
    class_<swganh::event_dispatcher::EventDispatcher, boost::noncopyable>("EventDispatcher", no_init)
        .def("dispatch",
            make_function(
                std::bind(&swganh::event_dispatcher::EventDispatcher::Dispatch, std::placeholders::_1, std::placeholders::_2),
                default_call_policies(),
                boost::mpl::vector<void, swganh::event_dispatcher::EventDispatcher*, std::shared_ptr<swganh::event_dispatcher::EventInterface>>()), 
            "dispatches an event to be processed later")
        .def("subscribe", &swganh::event_dispatcher::EventDispatcher::Subscribe, "subscribes to an event and attaches a callback") 
        ;
}
