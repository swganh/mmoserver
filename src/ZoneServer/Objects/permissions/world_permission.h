// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include "ZoneServer\Objects\permissions/container_permissions_interface.h"

namespace swganh
{
namespace object
{

class WorldPermission : public ContainerPermissionsInterface
{
    virtual PermissionType GetType()
    {
        return WORLD_PERMISSION;
    }

    bool canInsert(ContainerInterface* container, Object* requester, Object* object);

    bool canRemove(ContainerInterface* container, Object* requester, Object* object);

    bool canView(ContainerInterface* container, Object* requester);
};

}
}