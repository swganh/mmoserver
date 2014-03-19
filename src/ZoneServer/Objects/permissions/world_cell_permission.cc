// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "world_cell_permission.h"

#include "ZoneServer\Objects\Object\container_interface.h"
#include "ZoneServer\GameSystemManagers\Structure Manager\CellObject.h"
#include "ZoneServer\GameSystemManagers\Structure Manager\PlayerStructure.h"
#include "ZoneServer\WorldManager.h"

using namespace swganh::object;

bool WorldCellPermission::canInsert(ContainerInterface* container, Object* requester, Object* object)
{

	return true;

	CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(container->getId()));
	PlayerStructure*	structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(cell->getParentId()));

	auto admin_data = structure->getAdminData();

	if((object == requester ) && (admin_data.check_entry(requester->getId() || admin_data.check_admin(requester->getId() ))))	{
		return true;
	}

	
	if((object != requester ) && admin_data.check_admin(requester->getId()))	{
		return true;
	}

	return false;
}

bool WorldCellPermission::canRemove(ContainerInterface* container, Object* requester, Object* object)
{
    CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(container->getId()));
	PlayerStructure*	structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(cell->getParentId()));

	return true;

	if(container->getId() == object->getId())	{
		return true;
	}

	auto admin_data = structure->getAdminData();

	if((object == requester ) && (admin_data.check_entry(requester->getId() || admin_data.check_admin(requester->getId() ))))	{
		return true;
	}

	
	if((object != requester ) && admin_data.check_admin(requester->getId()))	{
		return true;
	}

	return false;
}

bool WorldCellPermission::canView(ContainerInterface* container, Object* requester)
{

	return true;

    CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(container->getId()));
	PlayerStructure*	structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(cell->getParentId()));

	auto admin_data = structure->getAdminData();

	if((admin_data.check_entry(requester->getId() || admin_data.check_admin(requester->getId() ))))	{
		return true;
	}

	return false;
}