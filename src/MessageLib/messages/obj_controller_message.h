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

class ObjControllerMessage : public BaseSwgMessage
{
public:
    virtual uint16_t Opcount() const
    {
        return 5;
    }
    virtual uint32_t Opcode() const
    {
        return 0x80CE5E46;
    }

    ObjControllerMessage()
    {}

    ObjControllerMessage(uint32_t controller_type_, uint32_t message_type_)
    {
        controller_type = controller_type_;
        message_type = message_type_;
        observable_id = 0;
        tick_count = 0;
    }

    ObjControllerMessage(const ObjControllerMessage& other)
    {
        controller_type = other.controller_type;
        message_type = other.message_type;
        observable_id = other.observable_id;
        tick_count = other.tick_count;
    }

    uint32_t controller_type;
    uint32_t message_type;
    uint64_t observable_id;
    uint32_t tick_count;
    swganh::ByteBuffer data;

    virtual void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(data.data(), data.size());
    }

    virtual void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        data = std::move(buffer);
    }

    virtual void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(controller_type);
        buffer.write(message_type);
        buffer.write(observable_id);
        buffer.write(tick_count);

        OnControllerSerialize(buffer);
    }

    virtual void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        controller_type = buffer.read<uint32_t>();
        message_type = buffer.read<uint32_t>();
        observable_id = buffer.read<uint64_t>();
        tick_count = buffer.read<uint32_t>();
        OnControllerDeserialize(buffer);
    }
    virtual void SetObserverId(uint64_t observer_id)
    {
        observable_id = observer_id;
    }
};

}
}  // namespace swganh::messages
