// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "ZoneServer\Objects\object\container_interface.h"
#include "ZoneServer\Objects\permissions/container_permissions_interface.h"
#include "ZoneServer\Objects\Object\Object.h"
#include <glm/glm.hpp>
#include <boost/thread/locks.hpp>

using namespace swganh::object;

boost::shared_mutex ContainerInterface::global_container_lock_;

ContainerPermissionsInterface* ContainerInterface::GetPermissions()
{
    return container_permissions_;
}

void ContainerInterface::SetPermissions(ContainerPermissionsInterface* obj)
{
    container_permissions_ = obj;
}

bool ContainerInterface::HasContainedObjects()
{
    boost::shared_lock<boost::shared_mutex> shared(global_container_lock_);
    return __InternalHasContainedObjects();
}

bool ContainerInterface::__InternalHasContainedObjects()
{
    bool has_objects = false;
    std::list<Object*> out;
    __InternalGetObjects(nullptr, 1, true, out);
    return out.size() > 0;
}

std::list<Object*> ContainerInterface::GetObjects(Object* requester, uint32_t max_depth, bool topDown)
{
    boost::shared_lock<boost::shared_mutex> shared(global_container_lock_);
    std::list<Object*> out;
    __InternalGetObjects(requester, max_depth, topDown, out);
    return out;
}

void ContainerInterface::GetObjects(Object* requester, uint32_t max_depth, bool topDown, std::list<Object*>& out)
{
    boost::shared_lock<boost::shared_mutex> shared(global_container_lock_);
    __InternalGetObjects(requester, max_depth, topDown, out);
}

void ContainerInterface::ViewObjects(Object* requester, uint32_t max_depth, bool topDown, std::function<void(Object*)> func)
{
    boost::shared_lock<boost::shared_mutex> shared(global_container_lock_);
    __InternalViewObjects(requester, max_depth, topDown, func);
}

void ContainerInterface::AddAwareObject(Object* observer)
{
    boost::shared_lock<boost::shared_mutex> shared(global_container_lock_);
    __InternalAddAwareObject(observer, true);
}

void ContainerInterface::InternalReloadPlayer()
{
	   boost::shared_lock<boost::shared_mutex> shared(global_container_lock_);
	   __InternalReloadPlayer();
}

void ContainerInterface::ViewAwareObjects(std::function<void(Object*)> func)
{
    boost::shared_lock<boost::shared_mutex> shared(global_container_lock_);
    __InternalViewAwareObjects(func);
}

void ContainerInterface::RemoveAwareObject(Object* observer)
{
    boost::shared_lock<boost::shared_mutex> shared(global_container_lock_);
    __InternalRemoveAwareObject(observer, true);
}

void ContainerInterface::GetAbsolutes(glm::vec3& pos, glm::quat& rot)
{
    boost::shared_lock<boost::shared_mutex> shared(global_container_lock_);
    __InternalGetAbsolutes(pos, rot);
}

//void ContainerInterface::TransferObject(std::shared_ptr<Object> requester, std::shared_ptr<Object> object, std::shared_ptr<ContainerInterface> newContainer, int32_t arrangement_id=-2)
//{
//TransferObject(requester, object, newContainer, object->GetPosition(), arrangement_id);
//}