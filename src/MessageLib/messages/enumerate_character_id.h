// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct EnumerateCharacterId : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 2;
    }
    uint32_t Opcode() const
    {
        return 0x65EA4574;
    }

	struct CharacterData
	{
		std::wstring name;
		uint64_t character_id;
		uint32_t race_crc;
		uint32_t galaxy_id;
		uint32_t status;
	};

    std::vector<CharacterData> characters;

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write<uint32_t>(characters.size());
        std::for_each(
            characters.begin(),
            characters.end(),
            [&buffer] (const CharacterData& character)
        {
            buffer.write(character.name);
            buffer.write(character.race_crc);
            buffer.write(character.character_id);
            buffer.write(character.galaxy_id);
            buffer.write(character.status);
        });
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        uint32_t character_count = buffer.read<uint32_t>();
        for (uint32_t i = 0; i < character_count; i++)
        {
            CharacterData character;
            character.name = buffer.read<std::wstring>();
            character.race_crc = buffer.read<int32_t>();
            character.character_id = buffer.read<uint64_t>();
            character.galaxy_id = buffer.read<uint32_t>();
            character.status = buffer.read<int32_t>();
            characters.push_back(character);
        }
    }
};

EnumerateCharacterId BuildEnumerateCharacterId(std::vector<EnumerateCharacterId::CharacterData> characters);

}
} // namespace swganh::messages
