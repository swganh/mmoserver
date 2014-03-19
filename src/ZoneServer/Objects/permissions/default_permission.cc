// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "default_permission.h"

#include "ZoneServer\Objects\Object\Object.h"
#include "ZoneServer\Objects\Object\container_interface.h"
#include "ZoneServer\WorldManager.h"

using namespace swganh::object;

bool DefaultPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{
	Object* container_object = dynamic_cast<Object*>(container);

	//if we are a creature and a container in our inventory gets accessed
	//we need to make sure only we can access it (or an admin)
	if(container_object->getPermissionParent()->getObjectType() == SWG_CREATURE)	{
		if(container_object->getPermissionParent() == requester->getPermissionParent())	{
			return true;
		}
	}

	//in case the root parent is a building we need to check the admin list
	//check admin list
	if(object->getPermissionParent()->getObjectType() == SWG_BUILDING)	{

		return true;
	}

    return false;
}

bool DefaultPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
	Object* container_object = dynamic_cast<Object*>(container);
	if(container_object->getPermissionParent()->getObjectType() == SWG_CREATURE)	{
		if(container_object->getPermissionParent() == requester->getPermissionParent())	{
			return true;
		}
	}


	//in case the root parent is a building we need to check the admin list
	//check admin list
	if(object->getPermissionParent()->getObjectType() == SWG_BUILDING)	{

		return true;
	}

    return false;
}

bool DefaultPermission::canView(ContainerInterface* container, Object* requester)
{
	Object* container_object = dynamic_cast<Object*>(container);

	if(container_object->getPermissionParent()->getObjectType() == SWG_CREATURE)	{
		if(container_object->getPermissionParent() == requester->getPermissionParent())	{
			return true;
		}
		else	{
			return false;
		}
	}

	if(container_object->getPermissionParent()->getObjectType() == SWG_BUILDING)	{

		return true;
	}
	//in case the root parent is a building we need to check the admin list

    return false;
}