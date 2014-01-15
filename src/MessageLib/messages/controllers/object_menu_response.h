// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"

#include "MessageLib/messages/obj_controller_message.h"
#include "radial_options.h"

namespace swganh
{
namespace messages
{
namespace controllers
{

// Radial Response
class ObjectMenuResponse : public ObjControllerMessage
{
public:
    explicit ObjectMenuResponse(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
    {}

    ObjectMenuResponse(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x00000147;
    }

    uint64_t target_id;
    uint64_t owner_id;
    std::vector<RadialOptions> radial_options;
    uint8_t response_count;

    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(target_id);
        buffer.write(owner_id);

        uint32_t size = radial_options.size();
        if (size > 0)
        {
            // write size
            buffer.write(size);
            uint8_t counter = 0;
            for(auto& radial : radial_options)
            {
                buffer.write(++counter);
                buffer.write(radial.parent_item);
                buffer.write(static_cast<uint8_t>(radial.identifier));
                buffer.write(radial.action);
                buffer.write(radial.custom_description);
            }
        }
        else
        {
            buffer.write(0);
        }
        buffer.write(response_count);
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        target_id = buffer.read<uint64_t>();
        owner_id = buffer.read<uint64_t>();

    }
};

}
}
}  // namespace swganh::messages::controllers
