// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "static_container_permission.h"

#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\container_interface.h"

using namespace swganh::object;

bool StaticContainerPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
    return false;
}

bool StaticContainerPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
    return true;
}

bool StaticContainerPermission::canView(ContainerInterface* container, Object* requester)
{
    return true;
}