// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "no_view_permission.h"

#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\container_interface.h"

using namespace swganh::object;

bool NoViewPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
    return true;
}

bool NoViewPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
    return true;
}

bool NoViewPermission::canView(ContainerInterface* container, Object* requester)
{
    return false;
}