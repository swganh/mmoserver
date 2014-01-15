// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "terrain_service.h"

#include <map>

#include <ZoneServer\Services\scene_events.h>
//#include "swganh_core/simulation/scene_events.h"

#include <anh/event_dispatcher/event_dispatcher.h>
#include <anh/logger.h>

#include "anh/tre/resource_manager.h"
//#include "anh/tre/visitors/terrain/layer_visitor.h"
#include "anh/tre/visitors/terrain/terrain_visitor.h"

#include "anh/tre/visitors/terrain/detail/container_layer.h"
#include "anh/tre/visitors/terrain/detail/boundary_layer.h"
#include "anh/tre/visitors/terrain/detail/height_layer.h"
#include "anh/tre/visitors/terrain/detail/filter_layer.h"
#include "anh/tre/visitors/terrain/detail/boundary_polygon.h"
#include "anh/tre/visitors/terrain/detail/header.h"

using namespace swganh::terrain;
using namespace swganh::tre;

using namespace swganh::event_dispatcher;

TerrainService::TerrainService(swganh::app::SwganhKernel* kernel) : kernel_(kernel)
{
	LOG(error) << "TerrainService::TerrainService register events";
	kernel_->GetEventDispatcher()->Subscribe("SceneManager:NewScene", [&] (const std::shared_ptr<swganh::event_dispatcher::EventInterface>& newEvent)
    {
        auto real_event = std::static_pointer_cast<swganh::simulation::NewSceneEvent>(newEvent);
        try
        {
			LOG(error) << "TerrainService::TerrainService event fired";
            SceneEntry entry;
            entry.terrain_visitor_ = kernel_->GetResourceManager()->GetResourceByName<TerrainVisitor>(real_event->terrain_filename, false);

            boost::lock_guard<boost::mutex> lock(terrain_mutex_);
            scenes_.insert(SceneMap::value_type(real_event->scene_id, std::move(entry)));
        }
        catch(...)
        {
            LOG(error) << "Failed to load trn file: " << real_event->terrain_filename;
        }
		
    });

    kernel_->GetEventDispatcher()->Subscribe("SceneManager:DestroyScene", [&] (const std::shared_ptr<swganh::event_dispatcher::EventInterface>& newEvent)
    {
		
        auto real_event = std::static_pointer_cast<swganh::simulation::DestroySceneEvent>(newEvent);

        boost::lock_guard<boost::mutex> lock(terrain_mutex_);
        this->scenes_.erase(real_event->scene_id);
		
    });

    SetServiceDescription(swganh::service::ServiceDescription(
                              "Terrain Service",
                              "terrain",
                              "0.1",
                              "127.0.0.1",
                              0,
                              0,
                              0));
}

TerrainService::~TerrainService()
{}

void TerrainService::Initialize()
{}

bool TerrainService::waterHeightHelper(swganh::tre::ContainerLayer* layer, float x, float z, float& result)
{
    //Check our boundaries
	std::vector<BoundaryLayer*>::iterator it;
	for(it = layer->boundaries.begin();it != layer->boundaries.end();it++)
    {
        if((*it)->GetType() == LAYER_TYPE_BOUNDARY_POLYGON)
        {
            BoundaryPolygon* bpol = (BoundaryPolygon*)(*it);
            if(bpol->use_water_height && bpol->IsContained(x, z))
            {
                result = bpol->water_height;
                return true;
            }
        }
    }

    //Check our children recursively
	std::vector<ContainerLayer*>::iterator itCL;
    for(itCL = layer->children.begin(); itCL != layer->children.end(); itCL++)
    {
        if(waterHeightHelper((*itCL), x, z, result))
        {
            return true;
        }
    }

    return false;
}

float TerrainService::GetWaterHeight(uint32_t scene_id, float x, float z, float raw)
{
    boost::lock_guard<boost::mutex> lock(terrain_mutex_);
    auto itr = scenes_.find(scene_id);
    if(itr != scenes_.end())
    {
		std::vector<ContainerLayer*>::iterator itCL;
		for(itCL = itr->second.terrain_visitor_->GetLayers().begin();itCL != itr->second.terrain_visitor_->GetLayers().end(); itCL ++)
        //for(auto& child : itr->second.terrain_visitor_->GetLayers())
        {
            float result;
            if(waterHeightHelper((*itCL), x, z, result))
            {
                return result;
            }
        }

        if(!raw)
        {
            //Todo:Apply any necessary layer modifications
        }

        auto header = itr->second.terrain_visitor_->GetHeader();
        if(header->use_global_water_height)
        {
            return header->global_water_height;
        }
    }
    return FLT_MIN;
}

float TerrainService::GetHeight(uint32_t scene_id, float x, float z, bool raw)
{
    boost::lock_guard<boost::mutex> lock(terrain_mutex_);
    auto itr = scenes_.find(scene_id);
    if(itr != scenes_.end())
    {
        //Read in height at this point
        auto& layers = itr->second.terrain_visitor_->GetLayers();
        auto& fractals = itr->second.terrain_visitor_->GetFractals();

        float affector_transform = 1.0f;
        float height_result = 0.0f;

		std::vector<ContainerLayer*>::iterator itCL;
        for(itCL = layers.begin(); itCL != layers.end(); itCL ++)
        {
            if((*itCL)->enabled)
            {
                processLayerHeight((*itCL), x, z, height_result, affector_transform, fractals);
            }
        }

        if(!raw)
        {
            //Todo:Apply any necessary layer modifications
        }

        return (float) height_result;
    }
    return FLT_MIN;
}

bool TerrainService::IsWater(uint32_t scene_id, float x, float z, bool raw)
{
    float water_height = GetWaterHeight(scene_id, x, z, raw);
    if (water_height != FLT_MIN)
    {
        float height = GetHeight(scene_id, x, z);
        if (height <= water_height)
            return true;
    }
    return false;
}

float TerrainService::processLayerHeight(ContainerLayer* layer, float x, float z, float& base_value, float affector_transform, std::map<uint32_t,Fractal*>& fractals)
{
    std::vector<BoundaryLayer*> boundaries = layer->boundaries;
    std::vector<HeightLayer*> heights = layer->heights;
    std::vector<FilterLayer*> filters = layer->filters;

    float transform_value = 0.0f;
    bool has_boundaries = false;

    for (unsigned int i = 0; i < boundaries.size(); i++)
    {
        BoundaryLayer* boundary = (BoundaryLayer*)boundaries.at(i);

        if (!boundary->enabled)
            continue;
        else
            has_boundaries = true;

        float result = (float) boundary->Process(x, z);

        result = calculateFeathering(result, boundary->feather_type);

        if (result > transform_value)
            transform_value = result;

        if (transform_value >= 1)
            break;
    }

    if (has_boundaries == false)
        transform_value = 1.0f;

    if (layer->invert_boundaries)
        transform_value = 1.0f - transform_value;

    if (transform_value != 0)
    {
        for (unsigned int i = 0; i < filters.size(); ++i)
        {
            FilterLayer* filter = (FilterLayer*)filters.at(i);

            if (!filter->enabled)
                continue;

            float result = (float) filter->Process(x, z, transform_value, base_value, fractals);

            result = calculateFeathering(result, filter->feather_type);

            if (transform_value > result)
                transform_value = result;

            if (transform_value == 0)
                break;
        }

        if (layer->invert_filters)
            transform_value = 1.0f - transform_value;

        if (transform_value != 0)
        {
            for (unsigned int i = 0; i < heights.size(); i++)
            {
                HeightLayer* affector = (HeightLayer*)heights.at(i);

                if (affector->enabled)
                {
                    affector->GetBaseHeight(x, z, transform_value, base_value, fractals);
                }
            }

            std::vector<ContainerLayer*> children = layer->children;

            for (unsigned int i = 0; i < children.size(); i++)
            {
                ContainerLayer* child = children.at(i);

                if (child->enabled)
                    processLayerHeight(child, x, z, base_value, affector_transform * transform_value, fractals);
            }
        }
    }

    return transform_value;
}

float TerrainService::calculateFeathering(float value, int featheringType)
{
    float result = value;

    switch (featheringType)
    {
    case 1:
        result = result * result;
        break;
    case 2:
        result = sqrt(result);
        break;
    case 3:
        result = result * result * (3 - 2 * result);
        break;
    case 0:
        break;
    default:
        result = 0;
        break;
    }

    return result;
}
