// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <list>
#include <memory>
#include <glm/glm.hpp>

#include "swganh_core/object/container_interface.h"
#include "swganh_core/object/object.h"


namespace swganh
{
namespace simulation
{

/**
 * Provides Spatial Indexing functionality.
 */
class SpatialProviderInterface : public swganh::object::ContainerInterface
{
public:
    virtual void SvgToFile()=0;
    virtual void UpdateObject(std::shared_ptr<swganh::object::Object> obj, const swganh::object::AABB& old_bounding_volume, const swganh::object::AABB& new_bounding_volume) = 0;
    virtual void ViewObjectsInRange(glm::vec3 position, float radius, uint32_t max_depth, bool topDown, std::function<void(std::shared_ptr<swganh::object::Object>)> func) = 0;
    virtual std::set<std::shared_ptr<swganh::object::Object>> Query(boost::geometry::model::polygon<swganh::object::Point> query_box) = 0;
    virtual std::set<std::pair<float, std::shared_ptr<swganh::object::Object>>> FindObjectsInRangeByTag(const std::shared_ptr<swganh::object::Object> requester, const std::string& tag, float range=-1)=0;
};

}
} // namespace swganh::simulation
