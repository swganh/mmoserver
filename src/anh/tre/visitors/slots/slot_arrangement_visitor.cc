// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "slot_arrangement_visitor.h"

#include "../../iff/iff.h"

using namespace std;
using namespace swganh::tre;

void SlotArrangementVisitor::visit_folder(uint32_t depth, std::string name, uint32_t size)
{
}

void SlotArrangementVisitor::visit_data(uint32_t depth, std::string name, uint32_t size, swganh::ByteBuffer& data)
{
    if(name == "0000ARG ")
    {
        uint32_t end_pos = data.read_position() + size;
        std::vector<string> combination;
        while(data.read_position() < end_pos)
        {
            combination.push_back(data.read<string>(false, true));
        }
        combinations_.push_back(std::move(combination));
    }
}