// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <algorithm>
#include <list>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct SurveyResource
{
    float x;
    float y; // should be 0, because the z(y)-axis is irrelevant for resources
    float z; // standard thus is tu use xyz with y being height
    float percentage;

    SurveyResource()
        : y(0)
    {}
};

struct SurveyMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x877F79AC;
    }

    std::list<SurveyResource> resources;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(resources.size());
        std::for_each(resources.begin(), resources.end(), [&buffer] (SurveyResource resource)
        {
            buffer.write(resource.x);
            buffer.write(resource.z);
            buffer.write(resource.y);
            buffer.write(resource.percentage);
        });
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        uint32_t resources_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < resources_count; i++)
        {
            SurveyResource resource;
            resource.x = buffer.read<float>();
            resource.z = buffer.read<float>();
            resource.y = buffer.read<float>();
            resource.percentage = buffer.read<float>();
            resources.push_back(resource);
        }
    }
};

}
} // namespace swganh::messages
