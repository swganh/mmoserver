// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "creature_container_permission.h"

#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\container_interface.h"

using namespace swganh::object;

bool CreatureContainerPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
	Object* container_object = dynamic_cast<Object*>(container);
	//LOG(info) << "CreatureContainerPermission::canInsert container->GetContainer() " << container_object->getRootParent()->getId() << "requester : " << requester->getRootParent()->getId();
	return (container_object->getPermissionParent() == requester->getPermissionParent());
}

bool CreatureContainerPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
	Object* container_object = dynamic_cast<Object*>(container);
	//LOG(info) << "CreatureContainerPermission::canRemove container->GetContainer() " << container_object->getRootParent()<< "requester : " << requester->getRootParent()->getId();
    return (container_object->getPermissionParent() == requester->getPermissionParent());
}

bool CreatureContainerPermission::canView(ContainerInterface* container, Object* requester)
{
	Object* container_object = dynamic_cast<Object*>(container);
	//LOG(info) << "CreatureContainerPermission::canView container->GetContainer() " << container_object->getRootParent()->getId() << "requester : " << requester->getRootParent()->getId();
    return (container_object->getPermissionParent() == requester->getPermissionParent());
}