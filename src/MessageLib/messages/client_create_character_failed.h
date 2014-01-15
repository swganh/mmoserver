// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ClientCreateCharacterFailed : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xDF333C6E;
    }

    std::wstring unk1;
    std::string stf_file;
    std::string error_string;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(unk1);
        buffer.write(stf_file);
        buffer.write(0);
        buffer.write(error_string);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        unk1 = buffer.read<std::wstring>();
        stf_file = buffer.read<std::string>();
        buffer.read<uint32_t>();
        error_string = buffer.read<std::string>();
    }
};

}
}  // namespace swganh::messages
