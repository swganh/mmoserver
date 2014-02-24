// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once
#include "anh/byte_buffer.h"

class SkillModNotFound : public std::runtime_error
{
public:
    SkillModNotFound()
        : runtime_error("Skill mod not found")
    {}
};

struct SkillModStruct
{
    SkillModStruct()
    {}

    SkillModStruct(std::string identifier_, uint32_t base_, uint32_t modifier_)
        : identifier(identifier_)
        , base(base_)
        , modifier(modifier_)
    {}

    ~SkillModStruct(void)
    {}

    void SerializeBaseline(swganh::ByteBuffer& buffer)
    {
        buffer.write<uint8_t>(0);                 // Unused (delta left over so please keep otherwise we'll crash :) )
        buffer.write<std::string>(identifier);    // Identifier
        buffer.write<uint32_t>(base);             // Base
        buffer.write<uint32_t>(modifier);         // Modifier
    }

    void SerializeDelta(swganh::ByteBuffer& buffer)
    {
		buffer.write<std::string>(identifier);    // Identifier
        buffer.write<uint32_t>(base);             // Base
        buffer.write<uint32_t>(modifier);         // Modifier
    }
	
	
	//
	static void SerializeDelta(swganh::ByteBuffer& buffer, SkillModStruct& sm)
    {
        sm.SerializeDelta(buffer);
    }
    
	//without this skillmods is broken as the struct cant be serialized to the messagestream
    static void SerializeBaseline(swganh::ByteBuffer& buffer, SkillModStruct& sm)
    {
        sm.SerializeBaseline(buffer);
    }


    bool operator==(const SkillModStruct& other)
    {
        return (identifier == other.identifier);
    }

    std::string identifier;
    uint32_t base;
    uint32_t modifier;
};

