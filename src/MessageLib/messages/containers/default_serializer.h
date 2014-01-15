// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "anh/byte_buffer.h"

namespace swganh
{
namespace containers
{

template<typename T>
class DefaultSerializer
{
public:

    static void SerializeBaseline(swganh::ByteBuffer& data, const T& t)
    {
        data.write<T>(t);
    }

    static void SerializeDelta(swganh::ByteBuffer& data, const T& t)
    {
        data.write<T>(t);
    }

};

}
}