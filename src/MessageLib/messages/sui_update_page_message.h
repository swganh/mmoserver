// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

#include <swganh_core/sui/sui_window_interface.h>

namespace swganh
{
namespace messages
{

struct SUIUpdatePageMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x5F3342F6;
    }

    int32_t window_id;
    std::string script_name;
    std::vector<swganh::sui::SUIWindowInterface::SUI_WINDOW_COMPONENT> components;
    uint64_t ranged_object;
    float range;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(window_id);
        buffer.write(script_name);
        buffer.write(components.size());

        for(auto& component : components)
        {
            buffer.write(component.type);
            buffer.write(component.wide_params.size());
            for(auto& wide : component.wide_params)
            {
                buffer.write(wide);
            }

            buffer.write(component.narrow_params.size());
            for(auto& narrow : component.narrow_params)
            {
                buffer.write(narrow);
            }
        }

        buffer.write(ranged_object);
        buffer.write(range);
        buffer.write<uint64_t>(0); //Unknown
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        window_id = buffer.read<int32_t>();
        script_name = buffer.read<std::string>();

        int component_count = buffer.read<int32_t>();
        for(int i=0; i < component_count; ++i)
        {
            swganh::sui::SUIWindowInterface::SUI_WINDOW_COMPONENT component;

            int narrows = buffer.read<int32_t>();
            for(int n=0; n < narrows; ++n)
            {
                component.narrow_params.push_back(buffer.read<std::string>());
            }

            int wides = buffer.read<int32_t>();
            for(int w=0; w < wides; ++w)
            {
                component.wide_params.push_back(buffer.read<std::wstring>());
            }

            components.push_back(component);
        }
        ranged_object = buffer.read<uint64_t>();
        range = buffer.read<float>();
        buffer.read<uint64_t>();//Unknown Long
    }
};

}
} // namespace swganh::messages
