// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "slot_definition_visitor.h"

#include "../../iff/iff.h"

using namespace std;
using namespace swganh::tre;

void SlotDefinitionVisitor::visit_folder(uint32_t depth, std::string name, uint32_t size)
{
}

void SlotDefinitionVisitor::visit_data(uint32_t depth, std::string name, uint32_t size, swganh::ByteBuffer& data)
{
    if(name == "0006DATA")
    {
        uint32_t final_loc = data.read_position() + size;
        while(data.read_position() < final_loc)
        {
            slot_entry entry;

            entry.name = data.read<std::string>(false, true);

            entry.global = data.read<char>() != 0;
            entry.canMod = data.read<char>() != 0;
            entry.exclusive = data.read<char>() != 0;

            entry.hardpoint_name = data.read<std::string>(false, true);
            entry.unkValue = data.read<uint32_t>();

            slots_.push_back(std::move(entry));
        }
    }
}

size_t SlotDefinitionVisitor::findSlotByName(const std::string& name)
{
    for(size_t i = 0; i < slots_.size(); ++i)
    {
        if(slots_[i].name == name)
            return i;
    }
    return -1;
}