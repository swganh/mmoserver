// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#ifndef WIN32
#include <Python.h>
#endif

#include "swganh/scripting/python_shared_ptr.h"
#include "simulation_service_interface.h"

#include "swganh_core/object/objects.h"

#include <boost/python.hpp>
#include <boost/python/overloads.hpp>

using namespace swganh::simulation;
using namespace swganh::object;
using namespace boost::python;
using namespace std;

boost::python::tuple CreateObjectFromTemplate(const std::string& template_name,
        PermissionType permission_type = DEFAULT_PERMISSION, bool is_persisted=true, uint64_t object_id=0)
{
    return boost::python::make_tuple(template_name, permission_type, is_persisted, object_id);
}
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(CreateOverload, CreateObjectFromTemplate, 1, 4)

void exportSimulationService()
{
    typedef void (SimulationServiceInterface::*PersistRelatedObjectsBinding)(const std::shared_ptr<swganh::object::Object>&);
    typedef std::shared_ptr<swganh::object::Object> (SimulationServiceInterface::*GetObjectByIdBinding)(uint64_t);
    typedef std::shared_ptr<swganh::object::Object> (SimulationServiceInterface::*GetObjectByCustomNameBinding)(const std::string&);
    typedef void (SimulationServiceInterface::*TransferObjectToSceneBinding)(uint64_t, const std::string&);
    typedef void (SimulationServiceInterface::*TransferObjectToSceneObjectBinding)(shared_ptr<swganh::object::Object>, const std::string&);
    typedef void (SimulationServiceInterface::*TransferObjectToSceneAndPositionBinding)(uint64_t, const std::string&, float, float, float);
    typedef void (SimulationServiceInterface::*TransferObjectToSceneObjectAndPositionBinding)(shared_ptr<swganh::object::Object>, const std::string&, float, float, float);
    //typedef std::shared_ptr<swganh::object::Object> (SimulationServiceInterface::*LoadObjectByIdBinding)(uint64_t object_id);

    //auto create_obj_overload = CreateOverload(args("template_name", "permission_type", "is_persisted", "object_id"));

    enum_<PermissionType>("ContainerPermission")
    .value("DEFAULT", DEFAULT_PERMISSION)
    .value("WORLD", WORLD_PERMISSION)
    .value("STATIC_CONTAINER", STATIC_CONTAINER_PERMISSION)
    .value("WORLD_CELL", WORLD_CELL_PERMISSION)
    .value("CREATURE", CREATURE_PERMISSION)
    .value("CREATURE_CONTAINER", CREATURE_CONTAINER_PERMISSION)
    .value("RIDEABLE", RIDEABLE_PERMISSION)
    .value("NO_VIEW", NO_VIEW_PERMISSION)
    ;
    // @TODO update transfer def to have unique names
    class_<SimulationServiceInterface, std::shared_ptr<SimulationServiceInterface>, boost::noncopyable>("SimulationService", "The simulation service handles the current scenes aka planets", no_init)
    .def("persist", PersistRelatedObjectsBinding(&SimulationServiceInterface::PersistRelatedObjects), "persists the specified object and it's containing objects")
    .def("findObjectById", GetObjectByIdBinding(&SimulationServiceInterface::LoadObjectById), "Finds an object by its id")
    .def("findBuildingById", &SimulationServiceInterface::LoadObjectById<Building>, "Finds a building object by its id")
    .def("findCellById", &SimulationServiceInterface::LoadObjectById<Cell>, "Finds a cell object by its id")
    .def("findCreatureById", &SimulationServiceInterface::LoadObjectById<Creature>, "Finds a creature object by its id")
    .def("findFactoryCrateById", &SimulationServiceInterface::LoadObjectById<FactoryCrate>, "Finds a factory crate object by its id")
    .def("findGroupById", &SimulationServiceInterface::LoadObjectById<Group>, "Finds a group object by its id")
    .def("findGuildById", &SimulationServiceInterface::LoadObjectById<Guild>, "Finds a guild object by its id")
    .def("findHarvesterInstallationById", &SimulationServiceInterface::LoadObjectById<HarvesterInstallation>, "Finds a harvester installation object by its id")
    .def("findIntangibleById", &SimulationServiceInterface::LoadObjectById<Intangible>, "Finds an intangible object by its id")
    .def("findManufactureSchematicById", &SimulationServiceInterface::LoadObjectById<ManufactureSchematic>, "Finds a manufacture schematic object by its id")
    .def("findMissionById", &SimulationServiceInterface::LoadObjectById<Mission>, "Finds a mission object by its id")
    .def("findPlayerById", &SimulationServiceInterface::LoadObjectById<Player>, "Finds a player object by its id")
    .def("findResourceContainerById", &SimulationServiceInterface::LoadObjectById<ResourceContainer>, "Finds a resource container object by its id")
    .def("findShipById", &SimulationServiceInterface::LoadObjectById<Ship>, "Finds a ship object by its id")
    .def("findStaticById", &SimulationServiceInterface::LoadObjectById<Static>, "Finds a static object by its id")
    .def("findTangibleById", &SimulationServiceInterface::LoadObjectById<Tangible>, "Finds a tangible object by its id")
    .def("findWeaponById", &SimulationServiceInterface::LoadObjectById<Weapon>, "Finds a weapon object by its id")
    .def("transfer", TransferObjectToSceneBinding(&SimulationServiceInterface::TransferObjectToScene), "transfers the object to a new scene")
    .def("transfer", TransferObjectToSceneObjectBinding(&SimulationServiceInterface::TransferObjectToScene), "transfers the object to a new scene")
    .def("transfer", TransferObjectToSceneAndPositionBinding(&SimulationServiceInterface::TransferObjectToScene), "transfers the object to a new scene and changes the position")
    .def("transfer", TransferObjectToSceneObjectAndPositionBinding(&SimulationServiceInterface::TransferObjectToScene), "transfers the object to a new scene and changes the position")
    .def("findObject", GetObjectByCustomNameBinding(&SimulationServiceInterface::GetObjectByCustomName), "***DEPRECATED USE CHAT SERVICE*** finds the object by their custom name")
    .def("addObjectToScene", &SimulationServiceInterface::AddObjectToScene, "Adds the Object to the specified scene")
    .def("startScene", &SimulationServiceInterface::StartScene, "starts a scene by its label")
    .def("stopScene", &SimulationServiceInterface::StopScene, "stops a scene by the given label")
    .def("createObject", &SimulationServiceInterface::CreateObjectFromTemplate, CreateOverload(args("template_name", "permission_type", "is_persisted", "object_id"), "Creates an object of the given template"))
    .def("removeObject", &SimulationServiceInterface::RemoveObject, "Removes an object from the simulation (delete).")
    .def("removeObjectById", &SimulationServiceInterface::RemoveObjectById, "Removes an object from the simulation by id (delete).")
    .def("destroyObject", &SimulationServiceInterface::DestroyObject, "Destroy's an object from the simulation and persistent storage.")
    .def("getSceneNameById", &SimulationServiceInterface::SceneNameById, "Returns a scenes name given its id")
    .def("getSceneIdByName", &SimulationServiceInterface::SceneIdByName, "Returns a scenes id given its name")
    ;
}
