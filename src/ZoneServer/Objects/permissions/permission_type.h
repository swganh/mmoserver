// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

namespace swganh
{
namespace object
{

enum PermissionType
{
    DEFAULT_PERMISSION = 1,
    WORLD_PERMISSION,
    STATIC_CONTAINER_PERMISSION,
    WORLD_CELL_PERMISSION,

    CREATURE_PERMISSION,
    CREATURE_CONTAINER_PERMISSION,
    RIDEABLE_PERMISSION,
    NO_VIEW_PERMISSION
};

}
}
