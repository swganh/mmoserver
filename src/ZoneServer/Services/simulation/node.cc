// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "node.h"

#include "swganh_core/object/object.h"

std::stringstream current_collision_points;

template <typename Point>
void GetCollisionBoxPoints(Point const& p)
{
    current_collision_points << " " << p.x() << "," << p.y();
}

namespace quadtree
{

Node::Node(NodeQuadrant quadrant, Region region, uint32_t level, uint32_t max_level, Node* parent)
    : quadrant_(quadrant)
    , region_(region)
    , level_(level)
    , max_level_(max_level)
    , state_(LEAF)
    , leaf_nodes_()
    , parent_(parent)
{
    // If this is the root node, we need to do an initial split.
    if(quadrant_ == ROOT)
        Split();
}

Node::~Node(void)
{
}

void Node::InsertObject(std::shared_ptr<swganh::object::Object> obj)
{
    obj->BuildSpatialProfile();

    // If the amount of objects contained is equal to or exceeds (in the case of objects not fitting
    // completely into one node), and we havn't reached the "maximum level" count, and we are a LEAF
    // node, Split().
    if(objects_.size() >= 1 && level_ < max_level_ && state_ == LEAF)
    {
        Split();
    }

    // Flipped to true if a child node of proper size was found to
    // contain the object. Otherwise, the object will be added to
    // this node.
    bool success = false;

    if(state_ == BRANCH)
    {
        std::for_each(leaf_nodes_.begin(), leaf_nodes_.end(), [=, &obj, &success](std::shared_ptr<Node> node)
        {
            // If we can fit within the node, traverse.
            if(boost::geometry::within( obj->GetAABB(), node->GetRegion() ))
            {
                node->InsertObject(obj);
                success = true;
                return;
            }
        });
    }

    if(success)
        return;


    objects_.insert(obj);
}

bool Node::RemoveObject(std::shared_ptr<swganh::object::Object> obj)
{
    // Search this node for the object by id, if it it found
    // we can return;
    for(auto i = objects_.begin(); i != objects_.end(); )
    {
        if(obj->GetObjectId() == (*i)->GetObjectId())
        {
            i = objects_.erase(i);
            return true;
        }
        i++;
    }

    // We didn't find the object in this branch, traverse through
    // each leaf node if we are a BRANCH.
    if(state_ == BRANCH)
    {
        auto bounding_volume = obj->GetAABB();
        std::set<std::shared_ptr<Node>> checked_nodes_;

        for(std::shared_ptr<Node> node : leaf_nodes_)
        {
            // If we can actually fit inside the node, traverse farther.
            if(boost::geometry::within(bounding_volume, node->GetRegion()))
            {
                checked_nodes_.insert(node);
                if(node->RemoveObject(obj))
                {
                    return true;
                }
            }
        }

        //Clearly we're still in there somewhere...we just don't know where anymore.
        //Position must've been mucked with before removal.
        bool output = false;
        for(auto node : leaf_nodes_)
        {
            if(checked_nodes_.find(node) != checked_nodes_.end())
            {
                if(node->RemoveObject(obj))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void Node::Split()
{
    if(state_ == BRANCH)
        return;

    state_ = BRANCH;

    Point center((region_.min_corner().x() + region_.max_corner().x()) / 2, (region_.min_corner().y() + region_.max_corner().y()) / 2);
    Point upper_center((region_.min_corner().x() + region_.max_corner().x()) / 2, region_.max_corner().y());
    Point left_center(region_.min_corner().x(), (region_.min_corner().y() + region_.max_corner().y()) / 2);
    Point right_center(region_.max_corner().x(), (region_.min_corner().y() + region_.max_corner().y()) / 2);
    Point bottom_center((region_.min_corner().x() + region_.max_corner().x()) / 2, region_.min_corner().y());

    leaf_nodes_[NW_QUADRANT] = std::make_shared<Node>(NW_QUADRANT, Region(left_center, upper_center), level_ + 1, max_level_, this);
    leaf_nodes_[NE_QUADRANT] = std::make_shared<Node>(NE_QUADRANT, Region(center, region_.max_corner()), level_ + 1, max_level_, this);
    leaf_nodes_[SW_QUADRANT] = std::make_shared<Node>(SW_QUADRANT, Region(region_.min_corner(), center), level_ + 1, max_level_, this);
    leaf_nodes_[SE_QUADRANT] = std::make_shared<Node>(SE_QUADRANT, Region(bottom_center, right_center), level_ + 1, max_level_, this);

    for(auto i = objects_.begin(); i != objects_.end();)
    {
        auto obj = (*i);
        auto bounding_volume = obj->GetAABB();
        bool success = false;
        for(std::shared_ptr<Node> node : leaf_nodes_)
        {
            if(boost::geometry::within(bounding_volume , node->GetRegion()))
            {
                i = objects_.erase(i);
                node->InsertObject(std::move(obj));
                success = true;
                break;
            }
        }

        if(!success)
            i++;
    }
}

std::set<std::shared_ptr<swganh::object::Object>> Node::Query(QueryBox query_box)
{
    std::set<std::shared_ptr<swganh::object::Object>> return_list;

    std::for_each(objects_.begin(), objects_.end(), [=,& return_list](std::shared_ptr<swganh::object::Object> obj)
    {
        if(boost::geometry::intersects(obj->GetAABB(), query_box))
            return_list.insert(obj);
    });

    if(state_ == BRANCH)
    {
        for(std::shared_ptr<Node> node : leaf_nodes_)
        {
            // Node is within Query Box.
            if(boost::geometry::within(node->GetRegion(), query_box))
            {
                auto sub_objects = node->GetContainedObjects();
                return_list.insert(sub_objects.begin(), sub_objects.end());
                continue;
            }

            // Query Box is within node.
            if(boost::geometry::within(query_box, node->GetRegion()))
            {
                auto sub_objects = node->Query(query_box);
                return_list.insert(sub_objects.begin(), sub_objects.end());
                break;
            }

            // Query Box intersects with node.
            if(boost::geometry::intersects(query_box, node->GetRegion()))
            {
                auto sub_objects = node->Query(query_box);
                return_list.insert( sub_objects.begin(), sub_objects.end() );
            }
        }
    }

    return return_list;
}

std::set<std::shared_ptr<swganh::object::Object>> Node::GetContainedObjects(void)
{
    std::set<std::shared_ptr<swganh::object::Object>> objs(objects_.begin(), objects_.end());
    if(state_ == BRANCH)
    {
        for(const std::shared_ptr<Node> node : leaf_nodes_)
        {
            auto sub_objects = node->GetContainedObjects();
            objs.insert(sub_objects.begin(), sub_objects.end());
        }
    }
    return objs;
}

void Node::UpdateObject(std::shared_ptr<swganh::object::Object> obj, const swganh::object::AABB& old_bounding_volume, const swganh::object::AABB& new_bounding_volume)
{
    // Check the objects of this node.
    for(auto i = objects_.begin(); i != objects_.end(); i++)
    {
        auto node_obj = (*i);
        if(node_obj->GetObjectId() == obj->GetObjectId())
        {
            // If we are in the same node, we don't need to do anything.
            if(boost::geometry::within(new_bounding_volume, region_))
            {
                return;
            }

            // Move our object from this node to a new node.
            std::shared_ptr<Node> node = GetRootNode_()->GetNodeContainingVolume_(new_bounding_volume);
            objects_.erase(i);
            node->InsertObject(obj);
            return;
        }
    };

    if(state_ == BRANCH)
    {
        for(std::shared_ptr<Node> node : leaf_nodes_)
        {
            // Go further into the tree if our point is within our child node.
            if(boost::geometry::within(old_bounding_volume, node->GetRegion()))
            {
                node->UpdateObject(obj, old_bounding_volume, new_bounding_volume);
                return;
            }
        }
    }
}

std::shared_ptr<Node> Node::GetNodeContainingVolume_(swganh::object::AABB volumn)
{
    // If we don't within the actual Spatial Indexing area, bail.
    if(!boost::geometry::within(volumn, region_))
        throw new std::runtime_error("Quadtree: Object position out of bounds.");

    if(state_ == BRANCH)
    {
        // See if we can fit inside leaf_nodes_
        for(std::shared_ptr<Node> node : leaf_nodes_)
        {
            if(boost::geometry::within(volumn, node->GetRegion()))
            {
                return node->GetNodeContainingVolume_(volumn);
            }
        }
    }

    // If not, we are between or in this node.
    return std::shared_ptr<Node>(shared_from_this());
}

void Node::SvgDump(std::string fname)
{
    std::ofstream file;
    file.open(fname);
    file << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1440\" height=\"900\" version=\"1.1\" viewBox=\"-8300 -8300 16600 16600\" overflow=\"visible\">\n";

    file << "<g>\n";
    SvgDumpRegions(file);
    file << "<" << '/' << "g>\n";

    file << "<g>\n";
    SvgDumpObjects(file);
    file << "<" << '/' << "g>\n";

    file << "<" << '/' << "svg>";
    file.close();
}

void Node::SvgDumpRegions(std::ofstream& file)
{
    std::stringstream region_points;
    boost::geometry::box_view<Region> box_view(region_);
    for(boost::range_iterator<boost::geometry::box_view<Region> const>::type it = boost::begin(box_view); it != boost::end(box_view); ++it)
    {
        region_points << " " << (float)(*it).x() << "," << (float)(*it).y() * -1.0f;
    }

    file << "<polygon points=\"" << region_points.str() << "\" style=\"fill-opacity:0;fill:none;stroke:green;stroke-width:5px\"" << '/' << "> \n";

    if(state_ == BRANCH)
    {
        for(std::shared_ptr<Node> node : leaf_nodes_)
        {
            if(node != nullptr)
                node->SvgDumpRegions(file);
        }
    }
}

void Node::SvgDumpObjects(std::ofstream& file)
{

    for(std::shared_ptr<swganh::object::Object> obj : objects_)
    {
        std::stringstream bounding_volume_points;

        auto bounding_volume = obj->GetAABB();
        auto collision_box = obj->GetWorldCollisionBox();

        current_collision_points.str("");
        current_collision_points.clear();

        boost::geometry::for_each_point(collision_box, GetCollisionBoxPoints<Point>);

        boost::geometry::box_view<swganh::object::AABB> bounding_volume_view(bounding_volume);
        for(boost::range_iterator<boost::geometry::box_view<swganh::object::AABB>>::type it = boost::begin(bounding_volume_view); it != boost::end(bounding_volume_view); ++it)
        {
            bounding_volume_points << " " << (*it).x() << "," << (*it).y() * -1.0f;
        }

        auto name = obj->GetCustomName();
        file << "<text x=\"" << obj->GetPosition().x << "\" y=\"" << obj->GetPosition().z * -1.0f << "\" fill=\"black\" style=\"text-anchor: middle;\" font-size=\"8px\">" << std::string(name.begin(), name.end()) << "<" << '/' << "text>\n";
        file << "<polygon points=\"" << bounding_volume_points.str() << "\" style=\"fill-opacity:0;fill:none;stroke:red;stroke-width:0.4px\"" << '/' << "> \n";
        file << "<polygon points=\"" << current_collision_points.str() << "\" style=\"fill-opacity:0;fill:none;stroke:blue;stroke-width:0.4px\"" << '/' << "> \n";

        obj->ViewObjects(obj, 0, true, [=, &file](std::shared_ptr<swganh::object::Object> object)
        {
            if(object->GetCustomName().size() > 0)
            {
                std::cout << "Printing internal object of ";
                std::wcout << obj->GetCustomName() << " : ";
                std::wcout << object->GetCustomName() << std::endl;
            }
            std::stringstream bounding_volume_points;

            auto bounding_volume = object->GetAABB();
            auto collision_box = object->GetWorldCollisionBox();

            current_collision_points.str("");
            current_collision_points.clear();

            boost::geometry::for_each_point(collision_box, GetCollisionBoxPoints<Point>);

            boost::geometry::box_view<swganh::object::AABB> bounding_volume_view(bounding_volume);
            for(boost::range_iterator<boost::geometry::box_view<swganh::object::AABB>>::type it = boost::begin(bounding_volume_view); it != boost::end(bounding_volume_view); ++it)
            {
                bounding_volume_points << " " << (*it).x() << "," << (*it).y() * -1.0f;
            }

            auto name = object->GetCustomName();
            auto abs_position = glm::vec3();
            auto abs_orientation = glm::quat();
            object->GetAbsolutes(abs_position, abs_orientation);
            file << "<text x=\"" << abs_position.x << "\" y=\"" << abs_position.z * -1.0f << "\" fill=\"black\" style=\"text-anchor: middle;\" font-size=\"8px\">" << std::string(name.begin(), name.end()) << " * <" << '/' << "text>\n";
            file << "<polygon points=\"" << bounding_volume_points.str() << "\" style=\"fill-opacity:0;fill:none;stroke:red;stroke-width:0.4px\"" << '/' << "> \n";
            file << "<polygon points=\"" << current_collision_points.str() << "\" style=\"fill-opacity:0;fill:none;stroke:blue;stroke-width:0.4px\"" << '/' << "> \n";
        });
    }

    if(state_ == BRANCH)
    {
        for(std::shared_ptr<Node> node : leaf_nodes_)
        {
            if(node != nullptr)
                node->SvgDumpObjects(file);
        }
    }
}

} // namespace quadtree
