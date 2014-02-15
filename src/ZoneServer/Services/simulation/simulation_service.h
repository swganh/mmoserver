// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "swganh_core/simulation/simulation_service_interface.h"

namespace swganh
{
namespace network
{
class ServerInterface;
}
namespace object
{
class ObjectManager;
}

namespace simulation
{

class SimulationServiceImpl;

class SimulationService : public swganh::simulation::SimulationServiceInterface
{
public:
    explicit SimulationService(swganh::app::SwganhKernel* kernel);

    ~SimulationService();

    void StartScene(const std::string& scene_label);
    void StopScene(const std::string& scene_label);
    virtual uint32_t SceneIdByName(const std::string& scene_label);
    virtual std::string SceneNameById(uint32_t scene_id);

    std::set<std::pair<float, std::shared_ptr<swganh::object::Object>>> FindObjectsInRangeByTag(const std::shared_ptr<swganh::object::Object> requester, const std::string& tag, float range=-1);

    bool SceneExists(const std::string& scene_label);
    bool SceneExists(uint32_t scene_id);

    void RegisterObjectFactories();

    void PersistObject(uint64_t object_id, bool persist_inherited = false);
    /*
    *	\brief this persists the given object and all related objects (ie: everything contained inside this object)
    */
    void PersistRelatedObjects(const std::shared_ptr<swganh::object::Object>& object);
    void PersistRelatedObjects(uint64_t parent_object_id, bool persist_inherited = false);

    void AddObjectToScene(std::shared_ptr<swganh::object::Object> object, const std::string& scene_label);

    std::shared_ptr<swganh::object::Object> LoadObjectById(uint64_t object_id);
    std::shared_ptr<swganh::object::Object> LoadObjectById(uint64_t object_id, uint32_t type);

    std::shared_ptr<swganh::object::Object> GetObjectById(uint64_t object_id);

    std::shared_ptr<swganh::object::Object> GetObjectByCustomName(const std::wstring& custom_name);
    std::shared_ptr<swganh::object::Object> GetObjectByCustomName(const std::string& custom_name);

    void TransferObjectToScene(uint64_t object_id, const std::string& scene);
    void TransferObjectToScene(uint64_t object_id, const std::string& scene, float x, float y, float z);
    void TransferObjectToScene(std::shared_ptr<swganh::object::Object> object, const std::string& scene);
    void TransferObjectToScene(std::shared_ptr<swganh::object::Object> object, const std::string& scene, float x, float y, float z);
    /**
     * Removes the requested object from the simulation.
     */
    void RemoveObjectById(uint64_t object_id);
    void RemoveObject(const std::shared_ptr<swganh::object::Object>& object);

    void DestroyObject(const std::shared_ptr<swganh::object::Object>& object);

    std::shared_ptr<swganh::observer::ObserverInterface> StartControllingObject(
        const std::shared_ptr<swganh::object::Object>& object,
        std::shared_ptr<swganh::connection::ConnectionClientInterface> client);

    void StopControllingObject(uint64_t object_id);
    void StopControllingObject(const std::shared_ptr<swganh::object::Object>& object);

	/**
	*	Returns the ObjectController of the Object with the given ID
	*/
	std::shared_ptr<swganh::object::ObjectController> GetObjectController(uint64_t object_id);

    void RegisterControllerHandler(uint32_t handler_id, swganh::object::ObjControllerHandler&& handler);

    void UnregisterControllerHandler(uint32_t handler_id);

    virtual void SendToAll(swganh::messages::BaseSwgMessage* message);
    virtual void SendToScene(swganh::messages::BaseSwgMessage* message, uint32_t scene_id);
    virtual void SendToScene(swganh::messages::BaseSwgMessage* message, std::string scene_name);
    virtual void SendToSceneInRange(swganh::messages::BaseSwgMessage* message, uint32_t scene_id, glm::vec3 position, float radius);
    virtual void SendToSceneInRange(swganh::messages::BaseSwgMessage* message, std::string scene_name, glm::vec3 position, float radius);

    virtual std::shared_ptr<swganh::object::Object> CreateObjectFromTemplate(const std::string& template_name,
            swganh::object::PermissionType type=swganh::object::DEFAULT_PERMISSION, bool is_persisted=true,
            uint64_t object_id=0);

    virtual const std::shared_ptr<swganh::equipment::EquipmentServiceInterface>& GetEquipmentService();

    virtual void PrepareToAccomodate(uint32_t delta);
    std::shared_ptr<swganh::object::ObjectManager> GetObjectManager();

    virtual void Initialize();
    virtual void Startup();

private:

    std::unique_ptr<SimulationServiceImpl> impl_;
    std::shared_ptr<swganh::network::ServerInterface> server_;
    swganh::app::SwganhKernel* kernel_;
};

}
}  // namespace swganh::simulation
