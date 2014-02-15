// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "swganh/service/service_interface.h"

#include "swganh/app/swganh_kernel.h"
#include "swganh_core/object/object_controller_interface.h"
#include "swganh_core/object/permissions/permission_type.h"
#include "swganh_core/object/waypoint/waypoint.h"

namespace swganh
{
class ByteBuffer;
}

namespace swganh
{
namespace connection
{
class ConnectionClient;
}

namespace messages
{
struct BaseSwgMessage;
}

namespace equipment
{
class EquipmentServiceInterface;
}

namespace object
{

typedef std::function<
void (std::shared_ptr<swganh::object::Object>, swganh::messages::ObjControllerMessage*)
> ObjControllerHandler;

typedef Concurrency::concurrent_unordered_map<
uint32_t,
ObjControllerHandler
> ObjControllerHandlerMap;

class Object;
class ObjectManager;
}

namespace simulation
{

class SimulationServiceInterface : public swganh::service::BaseService
{
public:
    virtual ~SimulationServiceInterface() {}

    virtual void StartScene(const std::string& scene_label) = 0;
    virtual void StopScene(const std::string& scene_label) = 0;

    virtual uint32_t SceneIdByName(const std::string& scene_label) = 0;
    virtual std::string SceneNameById(uint32_t scene_id) = 0;
    virtual bool SceneExists(const std::string& scene_label) = 0;
    virtual bool SceneExists(uint32_t scene_id) = 0;

    virtual void AddObjectToScene(std::shared_ptr<swganh::object::Object> object, const std::string& scene_label) = 0;

    virtual void RegisterObjectFactories() = 0;

    virtual void PersistObject(uint64_t object_id, bool persist_inherited = false) = 0;
    /*
    *	\brief this persists the given object and all related objects (ie: everything contained inside this object)
    */
    virtual void PersistRelatedObjects(const std::shared_ptr<swganh::object::Object>& object) = 0;
    virtual void PersistRelatedObjects(uint64_t parent_object_id, bool persist_inherited = false) = 0;

    virtual std::shared_ptr<swganh::object::Object> LoadObjectById(uint64_t object_id) = 0;
    virtual std::shared_ptr<swganh::object::Object> LoadObjectById(uint64_t object_id, uint32_t type) = 0;

    virtual std::set<std::pair<float, std::shared_ptr<swganh::object::Object>>> FindObjectsInRangeByTag(const std::shared_ptr<swganh::object::Object> requester, const std::string& tag, float range=-1) = 0;

    virtual std::shared_ptr<swganh::object::Object> GetObjectByCustomName(const std::wstring& custom_name) = 0;
    virtual std::shared_ptr<swganh::object::Object> GetObjectByCustomName(const std::string& custom_name) = 0;

    virtual void TransferObjectToScene(uint64_t object_id, const std::string& scene) = 0;
    virtual void TransferObjectToScene(uint64_t object_id, const std::string& scene, float x, float y, float z) = 0;
    virtual void TransferObjectToScene(std::shared_ptr<swganh::object::Object> object, const std::string& scene) = 0;
    virtual void TransferObjectToScene(std::shared_ptr<swganh::object::Object> object, const std::string& scene, float x, float y, float z) = 0;

    template<typename T>
    std::shared_ptr<T> LoadObjectById(uint64_t object_id)
    {
        std::shared_ptr<swganh::object::Object> object = LoadObjectById(object_id, T::type);
#ifdef _DEBUG
        return std::dynamic_pointer_cast<T>(object);
#else
        return std::static_pointer_cast<T>(object);
#endif
    }

    virtual std::shared_ptr<swganh::object::Object> GetObjectById(uint64_t object_id) = 0;

    template<typename T>
    std::shared_ptr<T> GetObjectById(uint64_t object_id)
    {
        std::shared_ptr<swganh::object::Object> object = GetObjectById(object_id);

#ifdef _DEBUG
        return std::dynamic_pointer_cast<T>(move(object));
#else
        return std::static_pointer_cast<T>(move(object));
#endif
    }

    /**
     * Removes the requested object from the simulation.
     */
    virtual void RemoveObjectById(uint64_t object_id) = 0;
    virtual void RemoveObject(const std::shared_ptr<swganh::object::Object>& object) = 0;

    virtual void DestroyObject(const std::shared_ptr<swganh::object::Object>& object) = 0;

    virtual std::shared_ptr<swganh::observer::ObserverInterface> StartControllingObject(
        const std::shared_ptr<swganh::object::Object>& object,
        std::shared_ptr<swganh::connection::ConnectionClientInterface> client) = 0;

	/**
	*	this will remove all subscribers from the Objects subscription list and drop the 
	*	Controller's (subscription) refcount by one
	*/
    virtual void StopControllingObject(uint64_t object_id) = 0;
    virtual void StopControllingObject(const std::shared_ptr<swganh::object::Object>& object) = 0;

	/**
	*	Returns the Object Controller
	*/
	virtual std::shared_ptr<swganh::object::ObjectController> GetObjectController(uint64_t object_id) = 0;

    template<typename MessageType>
    struct GenericControllerHandler
    {
        typedef std::function<void (
            std::shared_ptr<swganh::object::Object>, MessageType*)
        > HandlerType;
    };

    /**
     * Register's a message handler for processing ObjControllerMessage payloads.
     *
     * This overload accepts a member function and a pointer (either naked or smart)
     * and converts the request to the proper message type.
     *
     * \code{.cpp}
     *
     *  RegisterControllerHandler(&MyClass::HandleSomeControllerMessage, this);
     *
     * \param memfunc A member function that can process a concrete ObjControllerMessage type.
     * \param instance An instance of a class that implements memfunc.
     */
    template<typename T, typename U, typename MessageType>
    void RegisterControllerHandler(void (T::*memfunc)(const std::shared_ptr<swganh::object::Object>&, MessageType*), U instance)
    {
        RegisterControllerHandler<MessageType>(std::bind(memfunc, instance, std::placeholders::_1, std::placeholders::_2));
    }

    /**
     * Register's a message handler for processing ObjControllerMessage payloads.
     *
     * This handler automatically converts the request to the proper message type.
     *
     * \param handler A std::function object representing the handler.
     */
    template<typename MessageType>
    void RegisterControllerHandler(
        typename GenericControllerHandler<MessageType>::HandlerType&& handler)
    {
        auto shared_handler = std::make_shared<typename GenericControllerHandler<MessageType>::HandlerType>(std::move(handler));

        auto wrapped_handler = [this, shared_handler] (
                                   std::shared_ptr<swganh::object::Object> object,
                                   swganh::messages::ObjControllerMessage* message)
        {
            MessageType tmp(*message);
            tmp.OnControllerDeserialize(message->data);

            (*shared_handler)(object, &tmp);
        };

        RegisterControllerHandler(MessageType::message_type(), std::move(wrapped_handler));
    }

    /**
     * Register's a message handler for processing ObjControllerMessage payloads.
     *
     * This is the low level registration and should be used when wanting to bypass
     * automatic message conversion.
     *
     * \param handler_id The object controller message type.
     * \param handler The object controller handler.
     */
    virtual void RegisterControllerHandler(uint32_t handler_id, swganh::object::ObjControllerHandler&& handler) = 0;
    virtual void UnregisterControllerHandler(uint32_t handler_id) = 0;

    virtual void SendToAll(swganh::messages::BaseSwgMessage* message) = 0;
    virtual void SendToScene(swganh::messages::BaseSwgMessage* message, uint32_t scene_id) = 0;
    virtual void SendToScene(swganh::messages::BaseSwgMessage* message, std::string scene_name) = 0;
    virtual void SendToSceneInRange(swganh::messages::BaseSwgMessage* message, uint32_t scene_id, glm::vec3 position, float radius) = 0;
    virtual void SendToSceneInRange(swganh::messages::BaseSwgMessage* message, std::string scene_name, glm::vec3 position, float radius) = 0;

    template<typename T>
    std::shared_ptr<T> CreateObjectFromTemplateAs(const std::string& template_name,
            swganh::object::PermissionType type=swganh::object::DEFAULT_PERMISSION, bool is_persisted=true, uint64_t object_id=0)
    {
        std::shared_ptr<swganh::object::Object> object = CreateObjectFromTemplate(template_name, type, is_persisted, object_id);
#ifdef _DEBUG
        return std::dynamic_pointer_cast<T>(object);
#else
        return std::static_pointer_cast<T>(object);
#endif
    }
    virtual std::shared_ptr<swganh::object::Object> CreateObjectFromTemplate(const std::string& template_name,
            swganh::object::PermissionType type=swganh::object::DEFAULT_PERMISSION, bool is_persisted=true, uint64_t object_id=0) = 0;

    virtual void PrepareToAccomodate(uint32_t delta) = 0;

    virtual const std::shared_ptr<swganh::equipment::EquipmentServiceInterface>& GetEquipmentService() = 0;

};

}
}  // namespace swganh::simulation
