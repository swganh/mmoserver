// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct PlayClientEffectObjectMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0x8855434A;
    }

    std::string client_effect_file; // e.g. "clienteffect/frs_dark_envy.cef"
    std::string auxiliary_string; // Place from where to start the animation. See wiki for examples.
    uint64_t object_id;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(client_effect_file);
        buffer.write(auxiliary_string);
        buffer.write(object_id);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        client_effect_file = buffer.read<std::string>();
        auxiliary_string = buffer.read<std::string>();
        object_id = buffer.read<uint64_t>();
    }
};

}
} // namespace swganh::messages
