// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include "anh/byte_buffer.h"

#include "MessageLib/messages/obj_controller_message.h"

namespace swganh
{
namespace messages
{
namespace controllers
{

enum Trails
{
    NOTRAIL = 0x00,
    LEFT_FOOT_TRAIL = 0x01,
    RIGHT_FOOT_TRAIL = 0x02,
    LEFT_HAND_FOOT_TRAIL = 0x04,
    RIGHT_HAND_TRAIL = 0x08,
    WEAPON_TRAIL = 0x10,
    DEFAULT_TRAIL = 0xFF
};

struct CombatDefender
{
    CombatDefender()
        : defender_id(0)
        , defender_end_posture(0)
        , hit_type(0)
        , defender_special_move_effect(0) {}
    uint64_t defender_id;
    uint8_t defender_end_posture;
    uint8_t hit_type;
    uint8_t defender_special_move_effect;
};

class CombatActionMessage : public ObjControllerMessage
{
public:
    explicit CombatActionMessage(uint32_t controller_type = 0x0000001B)
        : ObjControllerMessage(controller_type, message_type())
        , action_crc(0)
        , attacker_id(0)
        , attacker_end_posture(0)
        , trails_bit_flag(DEFAULT_TRAIL)
        , combat_special_move_effect(0)
        , defender_list(std::vector<CombatDefender>())
    {}

    CombatActionMessage(const ObjControllerMessage& base)
        : ObjControllerMessage(base)
    {
    }

    static uint32_t message_type()
    {
        return 0x000000CC;
    }

    uint32_t action_crc;
    uint64_t attacker_id;
    uint64_t weapon_id;
    uint8_t attacker_end_posture;
    uint8_t trails_bit_flag;
    uint8_t combat_special_move_effect;

    std::vector<CombatDefender> defender_list;

    void OnControllerSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(action_crc);
        buffer.write(attacker_id);
        buffer.write(weapon_id);
        buffer.write(attacker_end_posture);
        buffer.write(trails_bit_flag);
        buffer.write(combat_special_move_effect);
        buffer.write<uint16_t>(defender_list.size());

        std::for_each(begin(defender_list), end(defender_list),[&buffer](CombatDefender defender)
        {
            buffer.write<uint32_t>(0);
            buffer.write(defender.defender_id);
            buffer.write(defender.defender_end_posture);
            buffer.write(defender.hit_type);
            buffer.write(defender.defender_special_move_effect);
        });
    }

    void OnControllerDeserialize(swganh::ByteBuffer& buffer)
    {
        action_crc = buffer.read<uint32_t>();
        attacker_id = buffer.read<uint64_t>();
        weapon_id = buffer.read<uint64_t>();
        attacker_end_posture = buffer.read<uint8_t>();
        trails_bit_flag = buffer.read<uint8_t>();
        combat_special_move_effect = buffer.read<uint8_t>();
        int defender_size = buffer.read<uint16_t>();
        for(int i = 0; i< defender_size; i++)
        {
            CombatDefender list;
            list.defender_id = buffer.read<uint64_t>();
            list.defender_end_posture = buffer.read<uint8_t>();
            list.hit_type = buffer.read<uint8_t>();
            list.defender_special_move_effect = buffer.read<uint8_t>();
            defender_list.push_back(list);
        }
    }
};

}
}
}  // namespace swganh::messages::controllers
