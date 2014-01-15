// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include "terrain_service_interface.h"
#include "anh/app/swganh_kernel.h"

#include <boost/thread/mutex.hpp>
#include <map>
#include <list>
#include <cstdint>
#include <memory>

namespace swganh
{
namespace tre
{
class TerrainVisitor;
//class LayerVisitor;
class Fractal;

class ContainerLayer;
}
}


namespace swganh
{


namespace terrain
{
struct LayerEntry
{
    //std::shared_ptr<swganh::tre::LayerVisitor> layer_visitor_;
    float offset_x, offset_z;
};

struct SceneEntry
{
    std::shared_ptr<swganh::tre::TerrainVisitor> terrain_visitor_;
    //std::list<LayerEntry> layers_;
};

typedef std::map<uint32_t, SceneEntry> SceneMap;

class TerrainService : public swganh::terrain::TerrainServiceInterface
{
public:

    TerrainService(swganh::app::SwganhKernel* kernel);
    ~TerrainService();

    virtual void Initialize();

    virtual float GetWaterHeight(uint32_t scene_id, float x, float z, float raw=false);
	
	/*	@brief gets the height at a defined (float x, float y ) spot
	*	scene_id is the id of the scene (instance)
	*	needs properly registered tre files in order to work
	*	example : "tre_config = e:/StarWarsGalaxies/swganh/swg2uu_live.cfg" in general.cfg
	*/
    virtual float GetHeight(uint32_t scene_id, float x, float z, bool raw=false);

	/*	@brief tests for the water height at a defined (float x, float y ) spot
	*	and then determines if the spot is low enough to have water
	*	scene_id is the id of the scene (instance)
	*	needs properly registered tre files in order to work
	*	example : "tre_config = e:/StarWarsGalaxies/swganh/swg2uu_live.cfg" in general.cfg
	*/
    virtual bool IsWater(uint32_t scene_id, float x, float z, bool raw=false);

private:

    bool waterHeightHelper(swganh::tre::ContainerLayer* layer, float x, float z, float& result);

    float processLayerHeight(swganh::tre::ContainerLayer* layer, float x, float z, float& base_value, float affector_transform, std::map<uint32_t, swganh::tre::Fractal*>& fractals);
    float calculateFeathering(float value, int featheringType);

    boost::mutex terrain_mutex_;
    SceneMap scenes_;
    swganh::app::SwganhKernel* kernel_;
};
}
}
