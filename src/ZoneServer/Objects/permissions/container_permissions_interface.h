// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include "permission_type.h"
#include <memory>

class Object;

namespace swganh
{
namespace object
{

class ContainerInterface;

class ContainerPermissionsInterface
{
public:

    virtual ~ContainerPermissionsInterface() {}

    virtual PermissionType GetType() = 0;

    virtual bool canInsert(ContainerInterface* container, Object* requester, Object* object) = 0;

    virtual bool canRemove(ContainerInterface* container, Object* requester, Object* object) = 0;

    virtual bool canView(ContainerInterface* container, Object* requester) = 0;

};

}
}
