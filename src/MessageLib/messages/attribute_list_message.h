// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct Attribute
{
    Attribute(std::string name_, std::wstring val)
        : name(name_)
        , value(val) { }
    std::string name;
    std::wstring value;
    bool operator==(const Attribute& new_attr)
    {
        return new_attr.name == name;
    }
};

struct AttributeListMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xF3F12F2A;
    }

    uint64_t object_id;
    std::vector<Attribute> attributes;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(object_id);
        buffer.write<uint32_t>(attributes.size());
        std::for_each(attributes.begin(), attributes.end(), [&buffer] (Attribute attribute)
        {
            buffer.write<std::string>(attribute.name);
            buffer.write<std::wstring>(attribute.value);
        });
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        object_id = buffer.read<uint64_t>();
        uint32_t attribute_count = buffer.read<uint32_t>();
        for(uint32_t i = 0; i < attribute_count; i++)
        {

            auto name = buffer.read<std::string>();
            auto value = buffer.read<std::wstring>();
            attributes.push_back(std::move(Attribute(name, value)));
        }
    }
};

}
} // namespace swganh::messages
