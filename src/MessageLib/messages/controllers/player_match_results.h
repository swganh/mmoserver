// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"

#include "MessageLib/messages/obj_controller_message.h"

namespace swganh
{
namespace messages
{
namespace controllers
{

struct PlayerMatch
{
    PlayerMatch()
        : player_bitmask(0)
        , player_name(L"")
        , race_id(0)
        , region_name("")
        , planet_name("")
        , guild_name("")
        , profession_title("")
    {}
    uint32_t player_bitmask;
    std::wstring player_name;
    uint32_t race_id;
    std::string region_name;
    std::string planet_name;
    std::string guild_name;
    std::string profession_title;
};

class PlayerMatchResults : public ObjControllerMessage
{
public:
    explicit PlayerMatchResults(uint32_t controller_type = 0x0000000B)
        : ObjControllerMessage(controller_type, message_type())
        , player_matches(std::vector<PlayerMatch>())
    {}

    PlayerMatchResults(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x000001E7;
    }

    std::vector<PlayerMatch> player_matches;


    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(player_matches.size());
        for(auto& match : player_matches)
        {
            buffer.write(4); // bitmask size / 4
            buffer.write(match.player_bitmask);
            // @TODO: Not sure what these are...
            buffer.write<uint32_t>(0);
            buffer.write<uint32_t>(0);
            buffer.write<uint32_t>(0);
            buffer.write(match.player_name);
            buffer.write(match.race_id);
            buffer.write(match.region_name);
            buffer.write(match.planet_name);
            buffer.write(match.guild_name);
            buffer.write(match.profession_title);

        }

    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        uint32_t count = buffer.read<uint32_t>();
        for (int i = 0; i < count; i++)
        {
            PlayerMatch m;
            // bitmask / 4
            buffer.read<uint32_t>();
            m.player_bitmask = buffer.read<uint32_t>();
            // ukn
            buffer.read<uint32_t>();
            buffer.read<uint32_t>();
            buffer.read<uint32_t>();
            m.player_name = buffer.read<std::wstring>();
            m.race_id = buffer.read<uint32_t>();
            m.region_name = buffer.read<std::string>();
            m.planet_name = buffer.read<std::string>();
            m.guild_name = buffer.read<std::string>();
            m.profession_title = buffer.read<std::string>();
            player_matches.push_back(std::move(m));
        }
    }
};

}
}
}  // namespace swganh::messages::controllers
