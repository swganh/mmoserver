// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <algorithm>
#include <glm/glm.hpp>
#include <list>
#include <string>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct PlanetTravelPointListResponse : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 9;
    }
    uint32_t Opcode() const
    {
        return 0x4D32541F;
    }

    std::string planet_name;
    std::list<std::string> location_names; // contains names of the starports and/or shuttleports
    std::list<glm::vec3> location_coordinates;
    std::list<uint32_t> location_tax_rates; // uncertain, possibly ticket prices minus a tax amount instead of just a tax amount
    std::list<uint8_t> location_starport_flags; // 0 = shuttleport, 1 = starport

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(planet_name);
        buffer.write<uint32_t>(location_names.size());
        std::for_each(location_names.begin(), location_names.end(), [&buffer] (std::string name)
        {
            buffer.write(name);
        });
        buffer.write<uint32_t>(location_coordinates.size());
        std::for_each(location_coordinates.begin(), location_coordinates.end(), [&buffer] (glm::vec3 coordinate)
        {
            buffer.write(coordinate.x);
            buffer.write(coordinate.z);
            buffer.write(coordinate.y);
        });
        buffer.write<uint32_t>(location_tax_rates.size());
        std::for_each(location_tax_rates.begin(), location_tax_rates.end(), [&buffer] (uint32_t tax_rate)
        {
            buffer.write(tax_rate);
        });
        buffer.write<uint32_t>(location_starport_flags.size());
        std::for_each(location_starport_flags.begin(), location_starport_flags.end(), [&buffer] (uint8_t starport_flag)
        {
            buffer.write(starport_flag);
        });
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        planet_name = buffer.read<std::string>();
        uint32_t location_names_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < location_names_count; i++)
        {
            std::string name;
            name = buffer.read<std::string>();
            location_names.push_back(name);
        }
        uint32_t location_coordinates_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < location_coordinates_count; i++)
        {
            glm::vec3 coordinate;
            coordinate.x = buffer.read<float>();
            coordinate.z = buffer.read<float>();
            coordinate.y = buffer.read<float>();
            location_coordinates.push_back(coordinate);
        }
        uint32_t location_tax_rates_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < location_tax_rates_count; i++)
        {
            uint32_t tax_rate;
            tax_rate = buffer.read<uint32_t>();
            location_tax_rates.push_back(tax_rate);
        }
        uint32_t location_starport_flags_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < location_starport_flags_count; i++)
        {
            uint8_t starport_flag;
            starport_flag = buffer.read<uint8_t>();
            location_starport_flags.push_back(starport_flag);
        }
    }
};

}
} // namespace swganh::messages
