// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "anh/service/service_interface.h"

namespace swganh
{
namespace terrain
{
class TerrainServiceInterface : public swganh::service::BaseService
{
public:

    virtual ~TerrainServiceInterface() {}

    virtual float GetHeight(uint32_t scene_id, float x, float z, bool raw=false) = 0;

    virtual float GetWaterHeight(uint32_t scene_id, float x, float z, float raw=false) = 0;

    virtual bool IsWater(uint32_t scene_id, float x, float z, bool raw=false) = 0;

};
}
}
