// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <algorithm>
#include <list>
#include <memory>
#include <string>
#include <vector>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"
//#include "swganh_core/login/galaxy_status.h"

namespace swganh
{
namespace login
{
class LoginClientInterface;
}
} // namespace swganh::login

namespace swganh
{
namespace messages
{

struct Cluster
{
    uint32_t server_id;
    std::string server_name;
    // D = 3600 * GMTOffset
    uint32_t distance;
};

struct LoginEnumCluster : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xC11C63B9;
    }

    std::list<Cluster> servers;
    uint32_t max_account_chars;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<uint32_t>(servers.size());
        std::for_each(servers.begin(), servers.end(), [&buffer] (Cluster cluster)
        {
            buffer.write<int32_t>(cluster.server_id);
            buffer.write<std::string>(cluster.server_name);
            buffer.write<uint32_t>(cluster.distance);
        });
        buffer.write<uint32_t>(max_account_chars);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        uint32_t server_count = buffer.read<uint32_t>();
        for(uint32_t i = 0; i < server_count; i++)
        {
            Cluster cluster;
            cluster.server_id = buffer.read<int32_t>();
            cluster.server_name = buffer.read<std::string>();
            cluster.distance = buffer.read<int32_t>();
            servers.push_back(cluster);
        }
        max_account_chars = buffer.read<uint32_t>();
    }
};

//LoginEnumCluster BuildLoginEnumCluster(std::shared_ptr<swganh::login::LoginClientInterface> login_client, const std::vector<login::GalaxyStatus>& galaxy_status);

}
} // namespace swganh::messages
