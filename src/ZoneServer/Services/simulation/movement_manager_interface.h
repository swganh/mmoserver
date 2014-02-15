// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <memory>

#ifdef WIN32
#include <concurrent_unordered_map.h>
#else
#include <tbb/concurrent_unordered_map.h>

namespace Concurrency
{
using ::tbb::concurrent_unordered_map;
}

#endif

#include "swganh_core/messages/controllers/data_transform.h"
#include "swganh_core/messages/controllers/data_transform_with_parent.h"

namespace swganh
{
class EventDispatcher;
namespace event_dispatcher
{
class EventDispatcherInterface;
}
}  // namespace swganh::event_dispatcher

namespace swganh
{
namespace object
{
class Object;
}
}  // namespace swganh::object

namespace swganh
{
namespace simulation
{
class SpatialProviderInterface;

class MovementManagerInterface
{
public:
    virtual ~MovementManagerInterface() {}

    virtual void HandleDataTransform(
        const std::shared_ptr<swganh::object::Object>& controller,
        swganh::messages::controllers::DataTransform message) = 0;

    virtual void HandleDataTransformWithParent(
        const std::shared_ptr<swganh::object::Object>& controller,
        swganh::messages::controllers::DataTransformWithParent message) = 0;

    virtual void SendDataTransformMessage(const std::shared_ptr<swganh::object::Object>& object, uint32_t unknown = 0x0000000B) = 0;
    virtual void SendUpdateDataTransformMessage(const std::shared_ptr<swganh::object::Object>& object) = 0;

    virtual void HandleDataTransformServer(
        const std::shared_ptr<swganh::object::Object>& object,
        const glm::vec3& new_position) = 0;

    /**
    * Used internally for server movements (ie: NPCS)
    */
    virtual void HandleDataTransformWithParentServer(
        const std::shared_ptr<swganh::object::Object>& parent,
        const std::shared_ptr<swganh::object::Object>& object,
        const glm::vec3& new_position) = 0;

    virtual void SendDataTransformWithParentMessage(const std::shared_ptr<swganh::object::Object>& object, uint32_t unknown = 0x0000000B) = 0;
    virtual void SendUpdateDataTransformWithParentMessage(const std::shared_ptr<swganh::object::Object>& object) = 0;
    virtual void SetSpatialProvider(std::shared_ptr<swganh::simulation::SpatialProviderInterface> spatial_provider) = 0;
    virtual void ResetMovementCounter(std::shared_ptr<swganh::object::Object> object)=0;
};

}
}  // namespace swganh::simulation
