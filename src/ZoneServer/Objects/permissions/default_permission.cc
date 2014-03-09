// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "default_permission.h"

#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\container_interface.h"
#include "ZoneServer\WorldManager.h"

using namespace swganh::object;

bool DefaultPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
	if(object->getRootParent() == requester)	{
		return true;
	}

	//in case the root parent is a building we need to check the admin list
	//check admin list
	if(object->getRootParent()->getObjectType() == SWG_BUILDING)	{

		return true;
	}

    return false;
}

bool DefaultPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
	if(object->getRootParent() == requester)	{
		return true;
	}

	//in case the root parent is a building we need to check the admin list
	//check admin list
	if(object->getRootParent()->getObjectType() == SWG_BUILDING)	{

		return true;
	}

    return false;
}

bool DefaultPermission::canView(ContainerInterface* container, Object* requester)
{

	Object* object = gWorldManager->getObjectById(container->getId());
	if(object->getRootParent() == requester)	{
		return true;
	}

	if(object->getRootParent()->getObjectType() == SWG_BUILDING)	{

		return true;
	}
	//in case the root parent is a building we need to check the admin list

    return false;
}