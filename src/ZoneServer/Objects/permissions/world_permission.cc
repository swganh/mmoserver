// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "world_permission.h"

#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\container_interface.h"

using namespace swganh::object;

bool WorldPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
    return (requester == object);
}

bool WorldPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
    return (requester == object);
}

bool WorldPermission::canView(ContainerInterface* container, Object* requester)
{
    return true;
}