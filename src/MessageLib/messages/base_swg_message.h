// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cassert>
#include <cstdint>

#include "anh/byte_buffer.h"
//#include "swganh/utilities.h"

namespace swganh
{
namespace messages
{

struct BaseSwgMessage
{
    virtual ~BaseSwgMessage() {}

	/*	in this core the opcount is set and written in the message layer
	*	the higher byte is in use as *priority* (well sorta
	*	the lower is used for routing purpses 
	*/
    virtual uint16_t Opcount() const = 0;
    virtual uint32_t Opcode() const = 0;
    virtual void OnSerialize(swganh::ByteBuffer& buffer) const = 0;
    virtual void OnDeserialize(swganh::ByteBuffer& buffer) = 0;

    virtual void Serialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(Opcode());

        OnSerialize(buffer);
    }

    virtual void SetObserverId(uint64_t observer_id)
    {
    }

    virtual void Deserialize(swganh::ByteBuffer buffer)
    {
        uint32_t opcode = buffer.read<uint32_t>();

        if (opcode != Opcode())
        {
            assert(true && "Opcodes don't match");
            return;
        }

        OnDeserialize(buffer);
    }
};

}
}  // namespace swganh::messages
