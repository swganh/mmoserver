// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "ridable_permission.h"

#include "ZoneServer\Objects\permissions/container_permissions_interface.h"
#include "ZoneServer\Objects\Object\container_interface.h"
#include "ZoneServer\Objects\MountObject.h"

using namespace swganh::object;

bool RideablePermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
    MountObject* mount = dynamic_cast<MountObject*>(container);
	return requester == object && mount && mount->owner() == requester->getId();
}

bool RideablePermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
    MountObject* mount = dynamic_cast<MountObject*>(container);
    return requester == object && mount && mount->owner() == requester->getId();
}

bool RideablePermission::canView(ContainerInterface* container, Object* requester)
{
    return true;
}