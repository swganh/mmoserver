// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "creature_permission.h"

#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\container_interface.h"

using namespace swganh::object;

bool CreaturePermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
	return (container->getId() == requester->getId());
}

bool CreaturePermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
    return (container->getId() == requester->getId());
}

bool CreaturePermission::canView(ContainerInterface* container, Object* requester)
{
    return true;
}