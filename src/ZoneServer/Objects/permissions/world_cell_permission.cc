// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "world_cell_permission.h"

#include "ZoneServer\Objects\Object\container_interface.h"

using namespace swganh::object;

bool WorldCellPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
    return (requester == object);
}

bool WorldCellPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
    return (requester == object);
}

bool WorldCellPermission::canView(ContainerInterface* container, Object* requester)
{
    return true;
}