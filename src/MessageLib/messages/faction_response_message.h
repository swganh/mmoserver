// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <algorithm>
#include <list>
#include <string>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct FactionResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 6;
    }
    uint32_t Opcode() const
    {
        return 0x5DD53957;
    }

    std::string faction_rank;
    uint32_t faction_points_rebel;
    uint32_t faction_points_imperial;
    uint32_t faction_points_hutt;
    std::list<std::string> faction_names;
    std::list<float> faction_points_list;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(faction_rank);
        buffer.write(faction_points_rebel);
        buffer.write(faction_points_imperial);
        buffer.write(faction_points_hutt);
        buffer.write<uint32_t>(faction_names.size());
        std::for_each(faction_names.begin(), faction_names.end(), [&buffer] (std::string faction_name)
        {
            buffer.write(faction_name);
        });
        buffer.write<uint32_t>(faction_points_list.size());
        std::for_each(faction_points_list.begin(), faction_points_list.end(), [&buffer] (float faction_points)
        {
            buffer.write(faction_points);
        });
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        faction_rank = buffer.read<std::string>();
        faction_points_rebel = buffer.read<uint32_t>();
        faction_points_imperial = buffer.read<uint32_t>();
        faction_points_hutt = buffer.read<uint32_t>();
        uint32_t faction_names_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < faction_names_count; i++)
        {
            std::string faction_name;
            faction_name = buffer.read<std::string>();
            faction_names.push_back(faction_name);
        }
        uint32_t faction_points_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < faction_points_count; i++)
        {
            float faction_points;
            faction_points = buffer.read<float>();
            faction_points_list.push_back(faction_points);
        }
    }
};

}
} // namespace swganh::messages
