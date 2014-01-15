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

struct ResourceInfo
{
    std::string name;
    uint64_t object_id;
    std::string type;
};

struct ResourceListForSurveyMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x8A64B1D5;
    }

    std::list<ResourceInfo> resources;
    std::string resources_type; // type of all resources in the list
    uint64_t surveyor_object_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<uint32_t>(resources.size());
        std::for_each(resources.begin(), resources.end(), [&buffer] (ResourceInfo resource)
        {
            buffer.write(resource.name);
            buffer.write(resource.object_id);
            buffer.write(resource.type);
        });
        buffer.write(resources_type);
        buffer.write(surveyor_object_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        uint32_t resources_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < resources_count; i++)
        {
            ResourceInfo resource;
            resource.name = buffer.read<std::string>();
            resource.object_id = buffer.read<uint64_t>();
            resource.type = buffer.read<std::string>();
            resources.push_back(resource);
        }
        resources_type = buffer.read<std::string>();
        surveyor_object_id = buffer.read<uint64_t>();
    }
};

}
} // namespace swganh::messages
