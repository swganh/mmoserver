// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include <anh/event_dispatcher/event_dispatcher.h>

namespace swganh
{
namespace simulation
{
struct NewSceneEvent : swganh::event_dispatcher::BaseEvent
{
    NewSceneEvent(swganh::event_dispatcher::EventType type, uint32_t scene_id_, std::string scene_label_, std::string terrain_filename_)
        : BaseEvent(type)
        , scene_id(scene_id_)
        , scene_label(scene_label_)
        , terrain_filename(terrain_filename_)
    {}
    uint32_t scene_id;
    std::string scene_label;
    std::string terrain_filename;
};

struct DestroySceneEvent : swganh::event_dispatcher::BaseEvent
{
    DestroySceneEvent(swganh::event_dispatcher::EventType type, uint32_t scene_id_)
        : BaseEvent(type)
        , scene_id(scene_id_)
    {}
    uint32_t scene_id;
};

}
}