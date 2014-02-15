// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "swganh_core/simulation/spatial_provider_interface.h"
#include "swganh_core/object/permissions/container_permissions_interface.h"
#include "node.h"

namespace swganh
{
namespace simulation
{

class QuadtreeSpatialProvider
    : public swganh::simulation::SpatialProviderInterface
    , public std::enable_shared_from_this<QuadtreeSpatialProvider>
{
public:
    QuadtreeSpatialProvider();
    virtual ~QuadtreeSpatialProvider(void);

    uint64_t GetObjectId()
    {
        return 0;
    }

    void SvgToFile()
    {
        std::stringstream fname;
        fname << "./logs/scene_graph_" <<  scene_name_ << ".svg";
        root_node_.SvgDump(fname.str());
    }

    void SetSceneName(std::string name)
    {
        scene_name_ = name;
    }

    void SetSceneId(uint32_t id)
    {
        scene_id_ = id;
    }

    //Object Management
    virtual void AddObject(std::shared_ptr<swganh::object::Object> requester, std::shared_ptr<swganh::object::Object> newObject, int32_t arrangement_id=-2);
    virtual void RemoveObject(std::shared_ptr<swganh::object::Object> requester, std::shared_ptr<swganh::object::Object> oldObject);
    virtual void TransferObject(std::shared_ptr<swganh::object::Object> requester, std::shared_ptr<swganh::object::Object> object, std::shared_ptr<ContainerInterface> newContainer, glm::vec3 new_position, int32_t arrangement_id=-2);
    virtual void UpdateObject(std::shared_ptr<swganh::object::Object> obj, const swganh::object::AABB& old_bounding_volume, const swganh::object::AABB& new_bounding_volume);
    virtual std::set<std::shared_ptr<swganh::object::Object>> Query(boost::geometry::model::polygon<swganh::object::Point> query_box);

    virtual std::set<std::pair<float, std::shared_ptr<swganh::object::Object>>> FindObjectsInRangeByTag(const std::shared_ptr<swganh::object::Object> requester, const std::string& tag, float range=-1);

    virtual void ViewObjectsInRange(glm::vec3 position, float radius, uint32_t max_depth, bool topDown, std::function<void(std::shared_ptr<swganh::object::Object>)> func);

    // FOR USE BY TRANSFER OBJECT DO NOT CALL IN OUTSIDE CODE
    virtual int32_t __InternalInsert(std::shared_ptr<swganh::object::Object> object, glm::vec3 new_position, int32_t arrangement_id=-2);
    virtual void __InternalViewObjects(std::shared_ptr<swganh::object::Object> requester, uint32_t max_depth, bool topDown, std::function<void(std::shared_ptr<swganh::object::Object>)> func);
    virtual void __InternalGetObjects(std::shared_ptr<swganh::object::Object> requester, uint32_t max_depth, bool topDown, std::list<std::shared_ptr<swganh::object::Object>>& out);

    virtual void __InternalViewAwareObjects(std::function<void(std::shared_ptr<swganh::object::Object>)> func, std::shared_ptr<swganh::object::Object> hint=nullptr);

    virtual std::shared_ptr<ContainerInterface> GetContainer()
    {
        return nullptr;
    }
    virtual void SetContainer(const std::shared_ptr<ContainerInterface>& container) {}

    virtual void __InternalGetAbsolutes(glm::vec3& pos, glm::quat& rot);

private:
    quadtree::Node root_node_;
    std::string scene_name_;
    uint32_t scene_id_;
    quadtree::QueryBox GetQueryBoxViewRange(std::shared_ptr<swganh::object::Object> object);
    quadtree::QueryBox GetQueryBoxViewRange(const swganh::object::AABB& box);

    void CheckCollisions(std::shared_ptr<swganh::object::Object> object);
};

}
} // swganh::simulation
