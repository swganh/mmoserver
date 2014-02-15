// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once
/*
#include "swganh_core/simulation/movement_manager_interface.h"

#ifdef WIN32
#include <concurrent_unordered_map.h>
#else
#include <tbb/concurrent_unordered_map.h>

namespace Concurrency
{
using ::tbb::concurrent_unordered_map;
}

#endif

namespace swganh
{
namespace app
{
class SwganhKernel;
} // app
namespace simulation
{
class SpatialProviderInterface;
class SimulationServiceInterface;
}
} // swganh::simulation

namespace swganh
{
namespace simulation
{

/**
* Handles movement related messages
*/
/*
class MovementManager : public swganh::simulation::MovementManagerInterface
{
public:
    /*
    * Creates a new instance
    */
  /*  explicit MovementManager(swganh::app::SwganhKernel* kernel, std::string scene_name);

    virtual ~MovementManager() {}

    /*
    * Handles the normal data transform (used while outside).
    */
    /*void HandleDataTransform(
        const std::shared_ptr<swganh::object::Object>& object,
        swganh::messages::controllers::DataTransform message);

    /**
    * Handles the with parent data transform (used while inside)
    */
    /*void HandleDataTransformWithParent(
        const std::shared_ptr<swganh::object::Object>& object,
        swganh::messages::controllers::DataTransformWithParent message);

    /**
    * Used internally for server movements (ie: NPCS)
    */
    /*void HandleDataTransformServer(
        const std::shared_ptr<swganh::object::Object>& object,
        const glm::vec3& new_position);

    void HandleDataTransformWithParentServer(
        const std::shared_ptr<swganh::object::Object>& parent,
        const std::shared_ptr<swganh::object::Object>& object,
        const glm::vec3& new_position);

    /**
    * Sends the data transform message
    */
/*
    void SendDataTransformMessage(const std::shared_ptr<swganh::object::Object>& object, uint32_t unknown = 0x0000000B);
    void SendUpdateDataTransformMessage(const std::shared_ptr<swganh::object::Object>& object);
    void SendDataTransformWithParentMessage(const std::shared_ptr<swganh::object::Object>& object, uint32_t unknown = 0x0000000B);
    void SendUpdateDataTransformWithParentMessage(const std::shared_ptr<swganh::object::Object>& object);

    void SetSpatialProvider(std::shared_ptr<swganh::simulation::SpatialProviderInterface> spatial_provider);

    void ResetMovementCounter(std::shared_ptr<swganh::object::Object> object);
private:
    void RegisterEvents(swganh::EventDispatcher* event_dispatcher);

    bool ValidateCounter_(uint64_t object_id, uint32_t counter);

    typedef Concurrency::concurrent_unordered_map<
    uint64_t, uint32_t
    > UpdateCounterMap;

    std::string scene_name_;
    UpdateCounterMap counter_map_;
    std::shared_ptr<swganh::simulation::SpatialProviderInterface> spatial_provider_;
    swganh::simulation::SimulationServiceInterface* simulation_service_;
    swganh::app::SwganhKernel* kernel_;
};

}
}  // namespace swganh::simulation
