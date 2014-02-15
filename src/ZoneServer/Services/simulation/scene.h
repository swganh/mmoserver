// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "swganh_core/simulation/scene_interface.h"
#include "swganh_core/simulation/spatial_provider_interface.h"
#include <cstdint>
#include <string>

namespace swganh
{
namespace app
{
class SwganhKernel;
}
}

namespace swganh
{
namespace simulation
{
class SpatialProviderInterface;

struct SceneDescription
{
    uint32_t id;
    std::string name;
    std::string label;
    std::string description;
    std::string terrain;
};

class Scene : public swganh::simulation::SceneInterface
{
public:
    Scene(SceneDescription description, swganh::app::SwganhKernel* kernel);
    Scene(
        uint32_t id,
        std::string name,
        std::string label,
        std::string description,
        std::string terrain,
        swganh::app::SwganhKernel* kernel);

    virtual ~Scene() {}

    uint32_t GetSceneId() const;
    const std::string& GetName() const;
    const std::string& GetLabel() const;
    const std::string& GetDescription() const;
    const std::string& GetTerrainMap() const;

    void AddObject(std::shared_ptr<swganh::object::Object> object);
    void RemoveObject(std::shared_ptr<swganh::object::Object> object);
    virtual void ViewObjects(std::shared_ptr<swganh::object::Object> requester, uint32_t max_depth,
                             bool topDown, std::function<void(std::shared_ptr<swganh::object::Object>)> func);
    virtual void ViewObjects(glm::vec3 position, float radius, uint32_t max_depth, bool topDown,
                             std::function<void(std::shared_ptr<swganh::object::Object>)> func);

    void HandleDataTransform(const std::shared_ptr<swganh::object::Object>& controller, swganh::messages::controllers::DataTransform message);
    void HandleDataTransformWithParent(const std::shared_ptr<swganh::object::Object>& controller, swganh::messages::controllers::DataTransformWithParent message);

    void HandleDataTransformServer(
        const std::shared_ptr<swganh::object::Object>& object,
        const glm::vec3& new_position);

    /**
    * Used internally for server movements (ie: NPCS)
    */
    void HandleDataTransformWithParentServer(
        const std::shared_ptr<swganh::object::Object>& parent,
        const std::shared_ptr<swganh::object::Object>& object,
        const glm::vec3& new_position);

    std::set<std::pair<float, std::shared_ptr<swganh::object::Object>>> FindObjectsInRangeByTag(const std::shared_ptr<swganh::object::Object> requester, const std::string& tag, float range=-1);

private:
    Scene();

    class SceneImpl;
    std::shared_ptr<SceneImpl> impl_;
};

}
}  // namespace swganh::simulation
