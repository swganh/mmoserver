// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "default_permission.h"

#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\container_interface.h"

using namespace swganh::object;

bool DefaultPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
    return false;
}

bool DefaultPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
    return false;
}

bool DefaultPermission::canView(ContainerInterface* container, Object* requester)
{
    return false;
}