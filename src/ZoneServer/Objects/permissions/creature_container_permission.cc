// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "creature_container_permission.h"

#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\container_interface.h"

using namespace swganh::object;

bool CreatureContainerPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
    return (container->GetContainer() == requester);
}

bool CreatureContainerPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
    return (container->GetContainer() == requester);
}

bool CreatureContainerPermission::canView(ContainerInterface* container, Object* requester)
{
    return (container->GetContainer() == requester);
}