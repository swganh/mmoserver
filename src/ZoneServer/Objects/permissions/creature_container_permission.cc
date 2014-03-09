// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "creature_container_permission.h"

#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\container_interface.h"

using namespace swganh::object;

bool CreatureContainerPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
	LOG(info) << "CreatureContainerPermission::canInsert container->GetContainer() " << container->GetContainer()->getId() << "requester : " << requester->getId();
	return (container->GetContainer()->getId() == requester->getId());
}

bool CreatureContainerPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
	LOG(info) << "CreatureContainerPermission::canRemove container->GetContainer() " << container->GetContainer()->getId() << "requester : " << requester->getId();
    return (container->GetContainer()->getId() == requester->getId());
}

bool CreatureContainerPermission::canView(ContainerInterface* container, Object* requester)
{
	LOG(info) << "CreatureContainerPermission::canView container->GetContainer() " << container->GetContainer()->getId() << "requester : " << requester->getId();
    return (container->GetContainer()->getId() == requester->getId());
}