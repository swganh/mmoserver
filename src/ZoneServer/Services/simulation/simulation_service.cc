// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#ifndef WIN32
#include <Python.h>
#endif

#include "simulation_service.h"

#include <boost/algorithm/string.hpp>

#include "swganh/byte_buffer.h"
#include "swganh/crc.h"
#include "swganh/event_dispatcher.h"
#include "swganh/service/service_manager.h"
#include "swganh/database/database_manager.h"
#include "swganh/network/server_interface.h"
#include "swganh/plugin/plugin_manager.h"
#include "swganh_core/object/object_controller.h"
#include "swganh/scripting/python_instance_creator.h"
#include "swganh/app/swganh_kernel.h"

#include "swganh_core/command/command_interface.h"
#include "swganh_core/command/command_service_interface.h"

#include "swganh_core/connection/connection_client_interface.h"
#include "swganh_core/connection/connection_service_interface.h"

#include "swganh_core/messages/select_character.h"
#include "swganh_core/messages/scene_create_object_by_crc.h"

#include "swganh_core/player/player_service_interface.h"

#include "swganh_core/object/object.h"
#include "swganh_core/object/object_manager.h"
#include "swganh_core/object/creature/creature.h"
#include "swganh_core/object/player/player.h"

#include "swganh_core/simulation/scene_manager_interface.h"
#include "swganh_core/simulation/scene_interface.h"
#include "swganh_core/messages/cmd_start_scene.h"
#include "swganh_core/messages/cmd_scene_ready.h"
#include "swganh_core/messages/obj_controller_message.h"
#include "swganh_core/messages/update_containment_message.h"
#include "swganh_core/messages/update_transform_message.h"
#include "swganh_core/messages/update_transform_with_parent_message.h"

#include "swganh/tre/resource_manager.h"
#include "swganh/tre/visitors/objects/object_visitor.h"

#include "swganh_core/object/object_events.h"

#include "swganh_core/equipment/equipment_service.h"
#include "movement_manager.h"
#include "scene_manager.h"
#include "swganh_core/simulation/movement_manager_interface.h"

using namespace swganh;
using namespace std;
using namespace swganh::connection;
using namespace swganh::messages;
using namespace swganh::messages::controllers;
using namespace swganh::network;
using namespace swganh::object;
using namespace swganh::simulation;
using namespace swganh::equipment;
using namespace swganh::player;

using namespace swganh::tre;

using swganh::observer::ObserverInterface;
using swganh::network::ServerInterface;
using swganh::network::Session;
using swganh::service::ServiceDescription;
using swganh::app::SwganhKernel;
using swganh::command::CommandInterface;
using swganh::scripting::PythonInstanceCreator;

namespace swganh
{
namespace simulation
{

class SimulationServiceImpl
{
public:
    SimulationServiceImpl(SwganhKernel* kernel)
        : kernel_(kernel)
    {
    }

    const shared_ptr<ObjectManager>& GetObjectManager()
    {
        if (!object_manager_)
        {
            object_manager_ = make_shared<ObjectManager>(kernel_);
        }

        return object_manager_;
    }

    const shared_ptr<SceneManagerInterface>& GetSceneManager()
    {
        if (!scene_manager_)
        {
            scene_manager_ = kernel_->GetPluginManager()->CreateObject<SceneManager>("Simulation::SceneManager");
        }

        return scene_manager_;
    }

    const shared_ptr<swganh::equipment::EquipmentServiceInterface>& GetEquipmentService()
    {
        if (!equipment_service_)
        {
            equipment_service_ = kernel_->GetPluginManager()->CreateObject<EquipmentService>("Equipment::EquipmentService");
        }

        return equipment_service_;
    }

    void HandleDataTransform(
        const shared_ptr<Object>& object,
        DataTransform* message)
    {
        auto find_iter = controlled_objects_.find(object->GetObjectId());
        if (find_iter != controlled_objects_.end())
        {
            // get the scene the object is in
            auto scene = GetSceneManager()->GetScene(find_iter->second->GetObject()->GetSceneId());
            if (scene)
            {
                scene->HandleDataTransform(object, *message);
            }
        }
    }

    void HandleDataTransformWithParent(
        const shared_ptr<Object>& object,
        DataTransformWithParent* message)
    {
        auto find_iter = controlled_objects_.find(object->GetObjectId());
        if (find_iter != controlled_objects_.end())
        {
            // get the scene the object is in
            auto scene = GetSceneManager()->GetScene(find_iter->second->GetObject()->GetSceneId());
            if (scene)
            {
                scene->HandleDataTransformWithParent(object, *message);
            }
        }
    }

    void AddObjectToScene(std::shared_ptr<swganh::object::Object> object, const std::string& scene_label)
    {
        auto scene = scene_manager_->GetScene(scene_label);
        if (scene)
        {
            scene->AddObject(object);
        }
    }

    void PersistObject(uint64_t object_id, bool persist_inherited)
    {
        object_manager_->PersistObject(object_id, persist_inherited);
    }

    void PersistRelatedObjects(const std::shared_ptr<swganh::object::Object>& object)
    {
        object_manager_->PersistRelatedObjects(object, true);
    }

    void PersistRelatedObjects(uint64_t parent_object_id, bool persist_inherited)
    {
        object_manager_->PersistRelatedObjects(parent_object_id, persist_inherited);
    }

    shared_ptr<Object> LoadObjectById(uint64_t object_id)
    {
        auto object = object_manager_->LoadObjectById(object_id);

        return object;
    }

    shared_ptr<Object> LoadObjectById(uint64_t object_id, uint32_t type)
    {
        auto object = object_manager_->LoadObjectById(object_id, type);

        return object;
    }

    shared_ptr<Object> GetObjectById(uint64_t object_id)
    {
        return object_manager_->GetObjectById(object_id);
    }

    void RemoveObjectById(uint64_t object_id)
    {
        auto object = object_manager_->GetObjectById(object_id);
        if (object)
        {
            RemoveObject(object);
        }
    }

    void RemoveObject(const shared_ptr<Object>& object)
    {
        auto scene = scene_manager_->GetScene(object->GetSceneId());
        if (scene)
        {
            scene->RemoveObject(object);
        }
        StopControllingObject(object);

        // We're ok with the object existing in the object_manager until server shutdown
		//actually ... no were not ???
        //if the object remains and the core saves async to db when the parent is gone
		//we get faulty parent_ids
		//thus best iterate from bottom to top!
		object->ViewObjects(nullptr, 0, false, [&](shared_ptr<Object> viewObject){
        	object_manager_->RemoveObject(viewObject);
        });
        object_manager_->RemoveObject(object);

    }

    void DestroyObject(const std::shared_ptr<swganh::object::Object>& object)
    {
        RemoveObject(object);
        object_manager_->DeleteObjectFromStorage(object);
    }

    std::set<std::pair<float, std::shared_ptr<swganh::object::Object>>> FindObjectsInRangeByTag(const std::shared_ptr<swganh::object::Object> requester, const std::string& tag, float range=-1)
    {
        return scene_manager_->GetScene(requester->GetSceneId())->FindObjectsInRangeByTag(requester, tag, range);
    }

    shared_ptr<Object> GetObjectByCustomName(const wstring& custom_name)
    {
        return object_manager_->GetObjectByCustomName(custom_name);
    }

    void TransferObjectToSceneWithPosition(uint64_t object_id, const string& scene, float x, float y, float z)
    {
        auto obj = GetObjectById(object_id);
        if(obj != nullptr)
            TransferObjectToScene(obj, scene, glm::vec3(x, y, z));
    }
    void TransferObjectToSceneWithPosition(shared_ptr<Object> obj, const string& scene, float x, float y, float z)
    {
        TransferObjectToScene(obj, scene, glm::vec3(x, y, z));
    }
    void TransferObjectToScene(shared_ptr<Object> obj, const string& scene, glm::vec3 position)
    {
        // Get Next Scene
        auto scene_obj = scene_manager_->GetScene(scene);

        if (!scene_obj)
        {
            throw std::runtime_error("Requested transfer to an invalid scene: " + scene);
        }

        // Clear Controller
        auto controller = obj->GetController();
        obj->ClearController();


        // Remove from existing scene
        auto old_scene = scene_manager_->GetScene(obj->GetSceneId());
        if(old_scene)
        {
            old_scene->RemoveObject(obj);
        }

        //Update the object's scene_id
        obj->SetSceneId(scene_obj->GetSceneId());

        //Update the object's position.
        obj->SetPosition(position);

        // CmdStartScene
        if(controller != nullptr)
        {
            CmdStartScene start_scene;
            start_scene.ignore_layout = 0;
            start_scene.character_id = obj->GetObjectId();

            start_scene.terrain_map = scene_obj->GetTerrainMap();
            start_scene.position = position;
            start_scene.shared_race_template = obj->GetTemplate();
            start_scene.galaxy_time = 0;

            controller->Notify(&start_scene, [=](uint16_t sequence)
            {
                std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!Attaching controller...." << std::endl;
                // Reset Controller
                obj->SetController(controller);

                // Add object to scene and send baselines
                scene_obj->AddObject(obj);
            });
        }
    }

    shared_ptr<Object> TransferObjectToScene(uint64_t object_id, const string& scene)
    {
        // Get Object
        auto obj = GetObjectById(object_id);

        TransferObjectToScene(obj, scene, obj->GetPosition());

        return obj;
    }

    shared_ptr<ObserverInterface> StartControllingObject(const shared_ptr<Object>& object, shared_ptr<ConnectionClientInterface> client)
    {
        shared_ptr<ObjectController> controller = nullptr;

        // If a controller already exists update it, otherwise create a new controller record.
        auto find_iter = controlled_objects_.find(object->GetObjectId());
        if (find_iter != controlled_objects_.end())
        {
            controller = find_iter->second;
            controller->SetRemoteClient(client);
        }
        else
        {
            controller = make_shared<ObjectController>(object, client);
            object->SetController(controller);

            controlled_objects_.insert(make_pair(object->GetObjectId(), controller));
        }

        auto connection_client = std::static_pointer_cast<ConnectionClientInterface>(client);
        connection_client->SetController(controller);

        return controller;
    }

    void StopControllingObject(const shared_ptr<Object>& object)
    {
        auto find_iter = controlled_objects_.find(object->GetObjectId());

        if (find_iter == controlled_objects_.end())
        {
            return;
        }

		//remove our controller - think of it as the link to our client
        object->ClearController();

		// ok were removing the Object controller here - 
        LOG(warning) << "Removing controlled object";
        controlled_objects_.unsafe_erase(find_iter);
    }

	std::shared_ptr<swganh::object::ObjectController> GetObjectController(uint64_t object_id) 
	{
		auto find_iter = controlled_objects_.find(object_id);

        if (find_iter != controlled_objects_.end())
        {
			return(find_iter->second);
        }

		return nullptr;
	}

    void RegisterControllerHandler(uint32_t handler_id, swganh::object::ObjControllerHandler&& handler)
    {
        auto find_iter = controller_handlers_.find(handler_id);

        if (find_iter != controller_handlers_.end())
        {
            return;
        }

        controller_handlers_.insert(make_pair(handler_id, move(handler)));
    }

    void UnregisterControllerHandler(uint32_t handler_id)
    {
        auto find_iter = controller_handlers_.find(handler_id);

        if (find_iter == controller_handlers_.end())
        {
            throw std::runtime_error("ObjControllerHandler does not exist");
        }

        controller_handlers_.unsafe_erase(find_iter);
    }

    void HandleObjControllerMessage(
        const shared_ptr<ConnectionClientInterface>& client,
        ObjControllerMessage* message)
    {
        auto find_iter = controller_handlers_.find(message->message_type);

        if (find_iter == controller_handlers_.end())
        {
            DLOG(warning) << "No handler registered to process the given message. " << message->data << std::endl;
            return;
        }

        auto object = object_manager_->GetObjectById(client->GetController()->GetId());
        if(object != nullptr)
        {
            find_iter->second(object, message);
        }
    }

    void HandleSelectCharacter(
        const shared_ptr<ConnectionClientInterface>& client,
        SelectCharacter* message)
	{

		auto event_dispatcher = kernel_->GetEventDispatcher();

		//this loads the creature Object!!!!
		//the player Object will automatically be loaded as equipped Object through the factory

		//as the Object is already loaded the Simulation server will not create a new Object via 
		//factory but return a pointer to the existing Object
		//this causes issues as the client (of the reconnecting player) wont get new Baselines

		//check if we are already loaded - this is the case if we relog for example after a client crash
		bool reload = false;
		
		auto object = this->GetObjectById(message->character_id);
		if(object)	{
			reload = true;

		}	else
		{
			object = LoadObjectById(message->character_id, Creature::type);
		}

        
        auto player = GetEquipmentService()->GetEquippedObject<Player>(object, "ghost");

        //Should be done on this thread to avoid issues with interleaving
        auto player_service = kernel_->GetServiceManager()->GetService<PlayerServiceInterface>("PlayerService");
        player_service->OnPlayerEnter(player);

        auto scene = scene_manager_->GetScene(object->GetSceneId());
        if (!scene)
        {
            throw std::runtime_error("Invalid scene selected for object");
        }

        // CmdStartScene
        CmdStartScene start_scene;
        start_scene.ignore_layout = 0;
        start_scene.character_id = object->GetObjectId();

        start_scene.terrain_map = scene->GetTerrainMap();
        start_scene.position = object->GetPosition();
        start_scene.shared_race_template = object->GetTemplate();
        start_scene.galaxy_time = 0;

        client->SendTo(start_scene, boost::optional<Session::SequencedCallback>(
                           [=](uint16_t sequence)
        {
            StartControllingObject(object, client);

			if(reload)	{
				object->InternalReloadPlayer();
			}

            if(object->GetContainer() == nullptr)
            {
                scene->AddObject(object);
            }
        }));
    }

    void SendToAll(swganh::messages::BaseSwgMessage* message)
    {
        scene_manager_->ViewScenes([&] (std::string name, std::shared_ptr<Scene> scene)
        {
            scene->ViewObjects(nullptr, 0, true,[&] (std::shared_ptr<Object> object)
            {
                auto controller = object->GetController();
                if(controller)
                    controller->Notify(message);
            });
        });
    }

    void SendToScene(swganh::messages::BaseSwgMessage* message, uint32_t scene_id)
    {
        scene_manager_->GetScene(scene_id)->ViewObjects(nullptr, 0, true, [&] (std::shared_ptr<Object> object)
        {
            auto controller = object->GetController();
            if(controller)
                controller->Notify(message);
        });
    }

    void SendToScene(swganh::messages::BaseSwgMessage* message, std::string scene_name)
    {
        scene_manager_->GetScene(scene_name)->ViewObjects(nullptr, 0, true, [&] (std::shared_ptr<Object> object)
        {
            auto controller = object->GetController();
            if(controller)
                controller->Notify(message);
        });
    }

    void SendToSceneInRange(swganh::messages::BaseSwgMessage* message, uint32_t scene_id, glm::vec3 position, float radius)
    {
        scene_manager_->GetScene(scene_id)->ViewObjects(position, radius, 0, true, [&] (std::shared_ptr<Object> object)
        {
            auto controller = object->GetController();
            if(controller)
                controller->Notify(message);
        });
    }

    void SendToSceneInRange(swganh::messages::BaseSwgMessage* message, std::string scene_name, glm::vec3 position, float radius)
    {
        scene_manager_->GetScene(scene_name)->ViewObjects(position, radius, 0, true, [&] (std::shared_ptr<Object> object)
        {
            auto controller = object->GetController();
            if(controller)
                controller->Notify(message);
        });
    }

private:
    shared_ptr<ObjectManager> object_manager_;
    shared_ptr<SceneManagerInterface> scene_manager_;
    shared_ptr<MovementManagerInterface> movement_manager_;
    shared_ptr<swganh::equipment::EquipmentServiceInterface> equipment_service_;
    SwganhKernel* kernel_;
    //ServerInterface* server_;

    ObjControllerHandlerMap controller_handlers_;

    Concurrency::concurrent_unordered_map<uint64_t, shared_ptr<ObjectController>> controlled_objects_;
    Concurrency::concurrent_unordered_map<uint64_t, shared_ptr<boost::asio::deadline_timer>> delayed_update_;
};

}
}  // namespace swganh::simulation

SimulationService::SimulationService(SwganhKernel* kernel)
    : impl_(new SimulationServiceImpl(kernel))
    , kernel_(kernel)
{
    impl_->GetSceneManager()->LoadSceneDescriptionsFromDatabase(kernel_->GetDatabaseManager()->getConnection("galaxy"));

    SetServiceDescription(ServiceDescription(
                              "SimulationService",
                              "simulation",
                              "0.1",
                              "127.0.0.1",
                              0,
                              0,
                              0));
}

SimulationService::~SimulationService()
{}

void SimulationService::StartScene(const std::string& scene_label)
{
    impl_->GetSceneManager()->StartScene(scene_label, kernel_);
}

void SimulationService::StopScene(const std::string& scene_label)
{
    impl_->GetSceneManager()->StopScene(scene_label, kernel_);
}

uint32_t SimulationService::SceneIdByName(const std::string& scene_label)
{
    return impl_->GetSceneManager()->GetScene(scene_label)->GetSceneId();
}

std::string SimulationService::SceneNameById(uint32_t scene_id)
{
    return impl_->GetSceneManager()->GetScene(scene_id)->GetLabel();
}

void SimulationService::PersistObject(uint64_t object_id, bool persist_inherited)
{
    impl_->PersistObject(object_id, persist_inherited);
}
void SimulationService::PersistRelatedObjects(const std::shared_ptr<swganh::object::Object>& object)
{
    impl_->PersistRelatedObjects(object);
}
void SimulationService::PersistRelatedObjects(uint64_t parent_object_id, bool persist_inherited)
{
    impl_->PersistRelatedObjects(parent_object_id, persist_inherited);
}
shared_ptr<Object> SimulationService::LoadObjectById(uint64_t object_id)
{
    return impl_->LoadObjectById(object_id);
}
shared_ptr<Object> SimulationService::LoadObjectById(uint64_t object_id, uint32_t type)
{
    return impl_->LoadObjectById(object_id, type);
}

shared_ptr<Object> SimulationService::GetObjectById(uint64_t object_id)
{
    return impl_->GetObjectById(object_id);
}

void SimulationService::RemoveObjectById(uint64_t object_id)
{
    impl_->RemoveObjectById(object_id);
}

void SimulationService::RemoveObject(const shared_ptr<Object>& object)
{
    impl_->RemoveObject(object);
}

void SimulationService::DestroyObject(const std::shared_ptr<swganh::object::Object>& object)
{
    impl_->DestroyObject(object);
}

shared_ptr<Object> SimulationService::GetObjectByCustomName(const string& custom_name)
{
    return GetObjectByCustomName(wstring(begin(custom_name), end(custom_name)));
}

shared_ptr<Object> SimulationService::GetObjectByCustomName(const wstring& custom_name)
{
    return impl_->GetObjectByCustomName(custom_name);
}

void SimulationService::TransferObjectToScene(uint64_t object_id, const string& scene)
{
    impl_->TransferObjectToScene(object_id, scene);
}
void SimulationService::TransferObjectToScene(uint64_t object_id, const std::string& scene, float x, float y, float z)
{
    impl_->TransferObjectToSceneWithPosition(object_id, scene, x, y ,z);
}
void SimulationService::TransferObjectToScene(std::shared_ptr<swganh::object::Object> object, const std::string& scene)
{
    impl_->TransferObjectToScene(object, scene, object->GetPosition());
}
void SimulationService::TransferObjectToScene(std::shared_ptr<swganh::object::Object> object, const std::string& scene, float x, float y, float z)
{
    impl_->TransferObjectToSceneWithPosition(object, scene, x, y, z);
}
shared_ptr<ObserverInterface> SimulationService::StartControllingObject(
    const shared_ptr<Object>& object,
    shared_ptr<ConnectionClientInterface> client)
{
    return impl_->StartControllingObject(object, client);
}

void SimulationService::StopControllingObject(const shared_ptr<Object>& object)
{
    impl_->StopControllingObject(object);
}

std::shared_ptr<swganh::object::ObjectController> SimulationService::GetObjectController(uint64_t object_id) 
{
	return impl_->GetObjectController(object_id);
}

void SimulationService::StopControllingObject(uint64_t object_id)
{
    auto object = GetObjectById(object_id);

    if (object) impl_->StopControllingObject(object);
}

void SimulationService::RegisterControllerHandler(
    uint32_t handler_id,
    swganh::object::ObjControllerHandler&& handler)
{
    impl_->RegisterControllerHandler(handler_id, move(handler));
}

void SimulationService::UnregisterControllerHandler(uint32_t handler_id)
{
    impl_->UnregisterControllerHandler(handler_id);
}

void SimulationService::SendToAll(swganh::messages::BaseSwgMessage* message)
{
    impl_->SendToAll(message);
}

void SimulationService::SendToScene(swganh::messages::BaseSwgMessage* message, uint32_t scene_id)
{
    impl_->SendToScene(message, scene_id);
}

void SimulationService::SendToScene(swganh::messages::BaseSwgMessage* message, std::string scene_name)
{
    impl_->SendToScene(message, scene_name);
}

void SimulationService::SendToSceneInRange(swganh::messages::BaseSwgMessage* message, uint32_t scene_id, glm::vec3 position, float radius)
{
    impl_->SendToSceneInRange(message, scene_id, position, radius);
}

void SimulationService::SendToSceneInRange(swganh::messages::BaseSwgMessage* message, std::string scene_name, glm::vec3 position, float radius)
{
    impl_->SendToSceneInRange(message, scene_name, position, radius);
}

void SimulationService::AddObjectToScene(std::shared_ptr<swganh::object::Object> object, const std::string& scene_label)
{
    impl_->AddObjectToScene(object, scene_label);
}

std::set<std::pair<float, std::shared_ptr<swganh::object::Object>>> SimulationService::FindObjectsInRangeByTag(const std::shared_ptr<swganh::object::Object> requester, const std::string& tag, float range)
{
    return impl_->FindObjectsInRangeByTag(requester, tag, range);
}

void SimulationService::Initialize()
{}

void SimulationService::Startup()
{
    RegisterObjectFactories();

    auto connection_service = kernel_->GetServiceManager()->GetService<ConnectionServiceInterface>("ConnectionService");

    connection_service->RegisterMessageHandler(
        &SimulationServiceImpl::HandleSelectCharacter, impl_.get());

    connection_service->RegisterMessageHandler(
        &SimulationServiceImpl::HandleObjControllerMessage, impl_.get());

    SimulationServiceInterface::RegisterControllerHandler(
        &SimulationServiceImpl::HandleDataTransform, impl_.get());

    SimulationServiceInterface::RegisterControllerHandler(
        &SimulationServiceImpl::HandleDataTransformWithParent, impl_.get());

    kernel_->GetEventDispatcher()->Subscribe("Object::UpdatePosition", [this] (shared_ptr<swganh::EventInterface> incoming_event)
    {
        const auto& update_event = static_pointer_cast<swganh::object::UpdatePositionEvent>(incoming_event);
        auto scene = impl_->GetSceneManager()->GetScene(update_event->object->GetSceneId());

        if (update_event->parent && update_event->parent->GetObjectId() != 0)
        {
            scene->HandleDataTransformWithParentServer(update_event->parent, update_event->object, update_event->position);
        }
        else
        {
            scene->HandleDataTransformServer(update_event->object, update_event->position);
        }

    });

    kernel_->GetEventDispatcher()->Subscribe("Core::ApplicationInitComplete", [this] (shared_ptr<swganh::EventInterface> incoming_event)
    {
        //Now that services are started, start the scenes.
        auto& scenes = kernel_->GetAppConfig().scenes;

for (auto scene : scenes)
        {
            StartScene(scene);
        }
    });
}

shared_ptr<Object> SimulationService::CreateObjectFromTemplate(const string& template_name, PermissionType type,
        bool is_persisted, uint64_t object_id)
{
    return impl_->GetObjectManager()->CreateObjectFromTemplate(template_name, type, is_persisted, object_id);
}

void SimulationService::PrepareToAccomodate(uint32_t delta)
{
    impl_->GetObjectManager()->PrepareToAccomodate(delta);
}

const shared_ptr<swganh::equipment::EquipmentServiceInterface>& SimulationService::GetEquipmentService()
{
    return impl_->GetEquipmentService();
}

std::shared_ptr<swganh::object::ObjectManager> SimulationService::GetObjectManager()
{
    return impl_->GetObjectManager();
}

bool SimulationService::SceneExists(const std::string& scene_label)
{
    return impl_->GetSceneManager()->GetScene(SceneIdByName(scene_label)) ? true : false;
}

bool SimulationService::SceneExists(uint32_t scene_id)
{
    return impl_->GetSceneManager()->GetScene(scene_id) ? true : false;
}