// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "enumerate_character_id.h"

using namespace swganh::messages;
using namespace std;

EnumerateCharacterId swganh::messages::BuildEnumerateCharacterId(std::vector<EnumerateCharacterId::CharacterData> characters)
{
    EnumerateCharacterId message;
    message.characters = move(characters);

    return message;
}
