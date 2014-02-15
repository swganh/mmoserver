// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <list>
#include <set>
#include <memory>
#include <fstream>

#include <boost/array.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <glm/glm.hpp>

#include <swganh_core/object/object.h>
namespace quadtree
{

typedef boost::geometry::model::box< boost::geometry::model::d2::point_xy<double> > Region;
typedef boost::geometry::model::box< boost::geometry::model::d2::point_xy<double> > QueryBox;
typedef boost::geometry::model::d2::point_xy<double> Point;
typedef boost::geometry::model::polygon<Point> QueryRegion;

enum NodeQuadrant
{
    NW_QUADRANT = 0,
    NE_QUADRANT,
    SW_QUADRANT,
    SE_QUADRANT,
    ROOT
}; // enum LeafNodeQuadrant

/**
 * \brief A Quadtree node.
 */
class Node : public std::enable_shared_from_this<Node>
{
public:
    enum NodeState
    {
        BRANCH,
        LEAF
    };

    Node(
        NodeQuadrant quadrant,
        Region region,
        uint32_t level,
        uint32_t max_level,
        Node* parent = NULL
    );

    ~Node(void);

    void InsertObject(std::shared_ptr<swganh::object::Object> obj);
    bool RemoveObject(std::shared_ptr<swganh::object::Object> obj);
    void UpdateObject(std::shared_ptr<swganh::object::Object> obj, const swganh::object::AABB& old_bounding_volume, const swganh::object::AABB& new_bounding_volume);
    void Split();

    void SvgDump(std::string fname);
    void SvgDumpRegions(std::ofstream& file);
    void SvgDumpObjects(std::ofstream& file);

    std::set<std::shared_ptr<swganh::object::Object>> Query(QueryBox query_box);

    const NodeQuadrant& GetQuadrant(void)
    {
        return quadrant_;
    }
    const uint32_t& GetLevel(void)
    {
        return level_;
    }
    const NodeState& GetState(void)
    {
        return state_;
    }
    const Region& GetRegion(void)
    {
        return region_;
    }
    const boost::array<std::shared_ptr<Node>, 4>& GetLeafNodes(void)
    {
        return leaf_nodes_;
    }
    const std::set<std::shared_ptr<swganh::object::Object>>& GetObjects(void)
    {
        return objects_;
    }
    std::set<std::shared_ptr<swganh::object::Object>> GetContainedObjects(void);

protected:
    void InsertObject_(std::shared_ptr<swganh::object::Object> obj);
    void RemoveObject_(std::shared_ptr<swganh::object::Object> obj);
    std::shared_ptr<Node> GetNodeContainingVolume_(swganh::object::AABB volumn);

    Node* GetRootNode_(void)
    {
        // Go to the root.
        if(parent_ != NULL)
            return parent_->GetRootNode_();
        else
            return this;
    }

private:
    NodeQuadrant quadrant_;
    Region region_;
    uint32_t level_;
    uint32_t max_level_;
    NodeState state_;
    std::set<std::shared_ptr<swganh::object::Object>> objects_;
    boost::array<std::shared_ptr<Node>, 4> leaf_nodes_;
    Node* parent_;
};

} // namespace quadtree
