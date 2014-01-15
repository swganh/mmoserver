// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "pob_visitor.h"
#include "../../iff/iff.h"

using namespace swganh::tre;
using namespace std::placeholders;
using namespace std;

PobVisitor::NodeNameIndex PobVisitor::nameLookup_;

PobVisitor::PobVisitor()
    : VisitorInterface()
{
    if(nameLookup_.empty())
    {
        nameLookup_.insert(make_pair<string, NodeFunctor>(string("INDX"), bind(&PobVisitor::_handleINDX, _1, _2)));
        nameLookup_.insert(make_pair<string, NodeFunctor>(string("0003DATA"), bind(&PobVisitor::_handle0003DATA, _1, _2)));
        nameLookup_.insert(make_pair<string, NodeFunctor>(string("0005DATA"), bind(&PobVisitor::_handle0005DATA, _1, _2)));
        nameLookup_.insert(make_pair<string, NodeFunctor>(string("PRTL"), bind(&PobVisitor::_handlePRTL, _1, _2)));
        nameLookup_.insert(make_pair<string, NodeFunctor>(string("PRTSPRTL"), bind(&PobVisitor::_handlePRTL, _1, _2)));
        nameLookup_.insert(make_pair<string, NodeFunctor>(string("PRTL0004"), bind(&PobVisitor::_handlePRTL0004, _1, _2)));
        nameLookup_.insert(make_pair<string, NodeFunctor>(string("0000VERT"), bind(&PobVisitor::_handle0000VERT, _1, _2)));
    }
}

void PobVisitor::visit_folder(uint32_t depth, std::string name, uint32_t size)
{
}

void PobVisitor::visit_data(uint32_t depth, std::string name, uint32_t size, swganh::ByteBuffer& data)
{
    NodeNameIndexIterator it = nameLookup_.find(name);
    if( it != nameLookup_.cend())
    {
        it->second(this, data);
    }
}

void PobVisitor::_handle0003DATA(PobVisitor* pob, swganh::ByteBuffer& data)
{
    int test = data.read<uint32_t>();

    pob->portals_.reserve(test);
    pob->cells_.reserve(data.read<uint32_t>());
}

void PobVisitor::_handlePRTL(PobVisitor* pob, swganh::ByteBuffer& data)
{
    Portal newP;

    uint32_t size = data.read<uint32_t>();
    newP.vertices.reserve(size);
    for(uint32_t i = 0; i < size; ++i)
    {
        glm::vec3 vector;
        vector.x = data.read<float>();
        vector.y = data.read<float>();
        vector.z = data.read<float>();
        newP.vertices.push_back(std::move(vector));
    }

    pob->portals_.push_back(newP);
}

void PobVisitor::_handle0005DATA(PobVisitor* pob, swganh::ByteBuffer& data)
{
    Cell cell;
    cell.links.reserve(data.read<uint32_t>());
    cell.unkFlag2 = data.read<char>();
    cell.name = data.read<std::string>(false,true);
    cell.mesh = data.read<std::string>(false,true);
    cell.unkFlag3 = data.read<char>();
    cell.collision = data.read<std::string>(false,true);
    pob->cells_.push_back(std::move(cell));
}

void PobVisitor::_handle0000VERT(PobVisitor* pob, swganh::ByteBuffer& data)
{
    if(!(pob->cells_.size() > 0))
        return;

    Cell& cell = pob->cells_[pob->cells_.size()-1];

    uint32_t count = data.size()/12;
    cell.vertices.reserve(count);

    for(uint32_t i=0; i < count; ++i)
    {
        glm::vec3 vector;
        vector.x = data.read<float>();
        vector.y = data.read<float>();
        vector.z = data.read<float>();
        cell.vertices.push_back(std::move(vector));
    }
}

void PobVisitor::_handleINDX(PobVisitor* pob, swganh::ByteBuffer& data)
{
    if(!(pob->cells_.size() > 0))
        return;

    Cell& cell = pob->cells_[pob->cells_.size()-1];

    uint32_t count = data.size()/12;
    cell.triangles.reserve(count);

    for(uint32_t i=0; i < count; ++i)
    {
        triangle vector;
        vector.a = data.read<uint32_t>();
        vector.b = data.read<uint32_t>();
        vector.c = data.read<uint32_t>();
        cell.triangles.push_back(std::move(vector));
    }
}

void PobVisitor::_handlePRTL0004(PobVisitor* pob, swganh::ByteBuffer& data)
{
    if(!(pob->cells_.size() > 0))
        return;

    Cell& cell = pob->cells_[pob->cells_.size()-1];

    Link link;

    //I think this is always 1.
    //assert( == 1);
    data.read<char>();


    link.portal_id = data.read<uint32_t>();

    //I think this is a normal of some kind.
    link.unkFlag2 = data.read<char>();
    link.dst_cellid = data.read<uint32_t>();

    cell.links.push_back(std::move(link));
}

void PobVisitor::debug()
{
    /*std::for_each(portals_.begin(), portals_.end(), [] (Portal& p) {
    	if(p.vertices.size() != 4)
    	{
    		printf("%d\n", static_cast<int>(p.vertices.size()));
    	}
    });*/
}