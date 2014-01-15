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

struct MapLocation
{
    uint64_t id;
    std::wstring name;
    float x;
    float y;
    // use only one of the following; choose the type to use and assign it to the appropriate display mode
    uint8_t type_displayAsCategory;
    uint8_t type_displayAsSubcategory;
    uint8_t type_displayAsActive;
};

struct GetMapLocationsResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 28;
    }
    uint32_t Opcode() const
    {
        return 0x9F80464C;
    }

    std::string planet_name;
    std::list<MapLocation> locations;
    std::list<MapLocation> blank_list1;
    std::list<MapLocation> blank_list2;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(planet_name);
        buffer.write(locations.size());
        std::for_each(locations.begin(), locations.end(), [&buffer] (MapLocation location)
        {
            buffer.write(location.id);
            buffer.write(location.name);
            buffer.write(location.x);
            buffer.write(location.y);
            buffer.write(location.type_displayAsCategory);
            buffer.write(location.type_displayAsSubcategory);
            buffer.write(location.type_displayAsActive);
        });
        // unclear why the following lists are needed; locations will be displayed no matter the list
        buffer.write(blank_list1.size());
        std::for_each(blank_list1.begin(), blank_list1.end(), [&buffer] (MapLocation location)
        {
            buffer.write(location.id);
            buffer.write(location.name);
            buffer.write(location.x);
            buffer.write(location.y);
            buffer.write(location.type_displayAsCategory);
            buffer.write(location.type_displayAsSubcategory);
            buffer.write(location.type_displayAsActive);
        });
        buffer.write(blank_list2.size());
        std::for_each(blank_list2.begin(), blank_list2.end(), [&buffer] (MapLocation location)
        {
            buffer.write(location.id);
            buffer.write(location.name);
            buffer.write(location.x);
            buffer.write(location.y);
            buffer.write(location.type_displayAsCategory);
            buffer.write(location.type_displayAsSubcategory);
            buffer.write(location.type_displayAsActive);
        });
        buffer.write<uint32_t>(0);
        buffer.write<uint32_t>(0);
        buffer.write<uint32_t>(0);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        planet_name = buffer.read<std::string>();
        uint32_t locations_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < locations_count; i++)
        {
            MapLocation location;
            location.id = buffer.read<uint64_t>();
            location.name = buffer.read<std::wstring>();
            location.x = buffer.read<float>();
            location.y = buffer.read<float>();
            location.type_displayAsCategory = buffer.read<uint8_t>();
            location.type_displayAsSubcategory = buffer.read<uint8_t>();
            location.type_displayAsActive = buffer.read<uint8_t>();
            locations.push_back(location);
        }
        // unclear why the following lists are needed; locations will be displayed no matter the list
        uint32_t blank_list1_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < blank_list1_count; i++)
        {
            MapLocation location;
            location.id = buffer.read<uint64_t>();
            location.name = buffer.read<std::wstring>();
            location.x = buffer.read<float>();
            location.y = buffer.read<float>();
            location.type_displayAsCategory = buffer.read<uint8_t>();
            location.type_displayAsSubcategory = buffer.read<uint8_t>();
            location.type_displayAsActive = buffer.read<uint8_t>();
            blank_list1.push_back(location);
        }
        uint32_t blank_list2_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < blank_list2_count; i++)
        {
            MapLocation location;
            location.id = buffer.read<uint64_t>();
            location.name = buffer.read<std::wstring>();
            location.x = buffer.read<float>();
            location.y = buffer.read<float>();
            location.type_displayAsCategory = buffer.read<uint8_t>();
            location.type_displayAsSubcategory = buffer.read<uint8_t>();
            location.type_displayAsActive = buffer.read<uint8_t>();
            blank_list2.push_back(location);
        }
    }
};

}
} // namespace swganh::messages
