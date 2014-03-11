// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include <list>
#include <memory>
#include <functional>

//#include <swganh/observer/observer_interface.h>
#include <boost/thread/shared_mutex.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Utils/typedefs.h"

class Object;

namespace swganh
{
namespace object
{
class ContainerPermissionsInterface;

class ContainerInterface
{
public:

    virtual ~ContainerInterface() {}

    virtual uint64 getId() const = 0;

    //Object Management
    virtual bool AddObject(Object* requester, Object* newObject, int32_t arrangement_id=-2) = 0;
    virtual bool RemoveObject(Object* requester, Object* oldObject) = 0;
    //virtual void TransferObject(std::shared_ptr<Object> requester, std::shared_ptr<Object> object, std::shared_ptr<ContainerInterface> newContainer, int32_t arrangement_id=-2);
    virtual void TransferObject(Object* requester, Object* object, ContainerInterface* newContainer, glm::vec3 position, int32_t arrangement_id=-2) = 0;
    virtual void SwapSlots(Object* requester, Object* object, int32_t new_arrangement_id) {};

    bool HasContainedObjects();
    bool __InternalHasContainedObjects();

    std::list<Object*> GetObjects(Object* requester, uint32_t max_depth, bool topDown);
    void GetObjects(Object* requester, uint32_t max_depth, bool topDown, std::list<Object*>& out);
    virtual void __InternalGetObjects(Object* requester, uint32_t max_depth, bool topDown, std::list<Object*>& out) = 0;

    void ViewObjects(Object* requester, uint32_t max_depth, bool topDown, std::function<void(Object*)> func);
    virtual void __InternalViewObjects(Object* requester, uint32_t max_depth, bool topDown, std::function<void(Object*)> func) = 0;

    virtual ContainerPermissionsInterface* GetPermissions();
    virtual void SetPermissions(ContainerPermissionsInterface* obj);

    //FOR USE BY TRANSFER OBJECT ONLY. DO NOT CALL IN OUTSIDE CODE
    virtual void __InternalTransfer(Object* requester, Object* object, ContainerInterface* newContainer, int32_t arrangement_id=-2) {};
    virtual int32_t __InternalInsert(Object* object, glm::vec3 new_position, int32_t arrangement_id=-2) = 0;

    //Call to Create
    void AddAwareObject(Object* observer);
    virtual void __InternalAddAwareObject(Object* observer, bool reverse_still_valid) {};

    //Call to View
    void ViewAwareObjects(std::function<void(Object*)> func);
    virtual void __InternalViewAwareObjects(std::function<void(Object*)> func) = 0;

    //Call to Destroy
    void RemoveAwareObject(Object* observer);
    virtual void __InternalRemoveAwareObject(Object* observer, bool reverse_still_valid) {};

    virtual ContainerInterface* GetContainer() = 0;
    virtual void SetContainer( ContainerInterface* container) = 0;

    virtual void GetAbsolutes(glm::vec3& pos, glm::quat& rot);
    virtual void __InternalGetAbsolutes(glm::vec3& pos, glm::quat& rot) = 0;

protected:
    swganh::object::ContainerPermissionsInterface* container_permissions_;

    static boost::shared_mutex global_container_lock_;
};
}
}