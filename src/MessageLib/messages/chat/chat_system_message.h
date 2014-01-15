// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include "anh/byte_buffer.h"
#include "MessageLib/messages/base_swg_message.h"

namespace swganh
{
namespace messages
{

struct ChatSystemMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 4;
    }
    uint32_t Opcode() const
    {
        return 0x6D2A6413;
    }

    uint8_t display_type;
    std::wstring message;
    std::wstring out_of_band;

    void AddProsePackage(const swganh::ByteBuffer* prose_package)
    {
        prose_package_ = prose_package;
    }

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(display_type);
        if (message.size())
        {
            buffer.write(message);
            buffer.write<uint32_t>(0);
        }
        if (prose_package_->size())
        {
            buffer.write<uint32_t>(0);
            buffer.append(*prose_package_);
        }
        else
        {
            buffer.write(out_of_band);
        }
    }
    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        display_type = buffer.read<uint8_t>();
        message = buffer.read<std::wstring>();
        buffer.read<uint32_t>();
        out_of_band = buffer.read<std::wstring>();
    }

private :
    const swganh::ByteBuffer* prose_package_;
};

}
}
