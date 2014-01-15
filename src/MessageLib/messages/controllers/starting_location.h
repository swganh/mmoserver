// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"

#include "MessageLib/messages/obj_controller_message.h"

namespace swganh
{
namespace messages
{
namespace controllers
{

struct StartPlanet
{
    StartPlanet()
        : starting_name("")
        , planet_name("")
        , x(0.0f), y(0.0f)
        , image_style("")
        , route_open(0)
    {}
    std::string starting_name;
    std::string planet_name;
    float x,y;
    std::string image_style;
    uint8_t route_open;

};

class StartingLocation : public ObjControllerMessage
{
public:
    explicit StartingLocation(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
        , starting_locations(std::vector<StartPlanet>())
    {}

    StartingLocation(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x000001FC;
    }

    std::vector<StartPlanet> starting_locations;


    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(starting_locations.size());
        for(auto& location : starting_locations)
        {
            buffer.write(location.starting_name);
            buffer.write(location.planet_name);
            buffer.write<float>(0);
            buffer.write<float>(0);
            buffer.write(0);
            buffer.write(location.image_style);
            buffer.write(0);
            buffer.write(location.route_open);
        }
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        int size = buffer.read<uint32_t>();
        for (int i = 0; i < size; i++)
        {
            StartPlanet p;
            p.starting_name = buffer.read<std::string>();
            p.planet_name = buffer.read<std::string>();
            p.x = buffer.read<float>();
            p.y = buffer.read<float>();
            buffer.read<std::string>();
            p.image_style = buffer.read<std::string>();
            buffer.read<std::string>();
            p.route_open = buffer.read<uint8_t>();

            starting_locations.push_back(std::move(p));
        }
    }
};

}
}
}  // namespace swganh::messages::controllers
