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

struct NewbieTutorialResponse : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0xCA88FBAD;
    }

    std::string client_ready; // arbitrary string: "clientReady" (lowercase c, uppercase R)

    NewbieTutorialResponse()
        : client_ready("clientReady")
    {}

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(client_ready);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        client_ready = buffer.read<std::string>();
    }
};

}
} // namespace swganh::messages
