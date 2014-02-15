// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "swganh_core/simulation/scene_interface.h"
#include "swganh/app/swganh_kernel.h"

namespace sql
{
class Connection;
}  // namespace sql

namespace swganh
{
namespace simulation
{

class Scene;

class SceneManagerInterface
{
public:
    virtual ~SceneManagerInterface() {}

    virtual void LoadSceneDescriptionsFromDatabase(const std::shared_ptr<sql::Connection>& connection) = 0;

    virtual std::shared_ptr<SceneInterface> GetScene(const std::string& scene_label) const = 0;
    virtual std::shared_ptr<SceneInterface> GetScene(uint32_t scene_id) const = 0;
    virtual void ViewScenes(std::function<void(const std::string&, std::shared_ptr<Scene>)> func) = 0;

    virtual void StartScene(const std::string& scene_label, swganh::app::SwganhKernel* kernel) = 0;
    virtual void StopScene(const std::string& scene_label, swganh::app::SwganhKernel* kernel) = 0;
};

}
}  // namespace swganh::simulation
