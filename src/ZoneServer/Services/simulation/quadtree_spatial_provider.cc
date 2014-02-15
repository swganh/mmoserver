// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "quadtree_spatial_provider.h"

#include "swganh/logger.h"

#include "swganh_core/object/object.h"
#include "swganh_core/object/permissions/world_permission.h"

using std::shared_ptr;

using namespace swganh::observer;
using namespace swganh::object;
using namespace swganh::simulation;
using namespace quadtree;

static int VIEWING_RANGE = 128;

QuadtreeSpatialProvider::QuadtreeSpatialProvider()
    : root_node_(ROOT, Region(quadtree::Point(-8300.0f, -8300.0f),
                              quadtree::Point(8300.0f, 8300.0f)), 0, 9, nullptr)
{
    SetPermissions(std::shared_ptr<ContainerPermissionsInterface>(new WorldPermission()));
}

QuadtreeSpatialProvider::~QuadtreeSpatialProvider(void)
{}

void QuadtreeSpatialProvider::AddObject(std::shared_ptr<swganh::object::Object> requester, shared_ptr<Object> object, int32_t arrangement_id)
{
    boost::upgrade_lock<boost::shared_mutex> uplock(global_container_lock_);
    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> unique(uplock);
        root_node_.InsertObject(object);
        object->SetContainer(shared_from_this());
        object->SetArrangementId(arrangement_id);
        object->SetSceneId(scene_id_);
    }

    CheckCollisions(object);

    // Make objects aware
    __InternalViewObjects(object, 1, true, [&](shared_ptr<Object> found_object)
    {
        found_object->__InternalAddAwareObject(object, true);
        object->__InternalAddAwareObject(found_object, true);
    });
}

void QuadtreeSpatialProvider::RemoveObject(std::shared_ptr<swganh::object::Object> requester,shared_ptr<Object> object)
{
    boost::upgrade_lock<boost::shared_mutex> uplock(global_container_lock_);

    __InternalViewObjects(object, 1, false, [&](shared_ptr<Object> found_object)
    {
        found_object->__InternalRemoveAwareObject(object, true);
        object->__InternalRemoveAwareObject(found_object, true);
    });

    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> unique(uplock);
        root_node_.RemoveObject(object);
        object->SetContainer(nullptr);
    }
}

void QuadtreeSpatialProvider::UpdateObject(shared_ptr<Object> obj, const swganh::object::AABB& old_bounding_volume, const swganh::object::AABB& new_bounding_volume)
{
    std::vector<std::shared_ptr<Object>> deleted_objects;

    boost::upgrade_lock<boost::shared_mutex> uplock(global_container_lock_);

    auto old_objects = root_node_.Query(GetQueryBoxViewRange(old_bounding_volume));
    {
        boost::upgrade_to_unique_lock<boost::shared_mutex> unique(uplock);
        root_node_.UpdateObject(obj, old_bounding_volume, new_bounding_volume);
    }

    CheckCollisions(obj);

    for(auto object : root_node_.Query(GetQueryBoxViewRange(new_bounding_volume)))
    {
        auto itr = old_objects.find(object);
        if(itr != old_objects.end())
        {
            //It's not old, that's all we can do for now.
            old_objects.erase(itr);
        }
        else
        {
            //It's new! Update!
            object->__InternalAddAwareObject(obj, true);
            obj->__InternalAddAwareObject(object, true);
        }
    }

    for(auto object : old_objects)
    {
        //It's old! Toss it!
        obj->__InternalRemoveAwareObject(object, true);
        object->__InternalRemoveAwareObject(obj, true);
    }
}

void QuadtreeSpatialProvider::TransferObject(std::shared_ptr<swganh::object::Object> requester,std::shared_ptr<Object> object, std::shared_ptr<ContainerInterface> newContainer, glm::vec3 new_position, int32_t arrangement_id)
{
    //Perform the transfer
    if (object != newContainer)
    {
        boost::upgrade_lock<boost::shared_mutex> uplock(global_container_lock_);
        {
            boost::upgrade_to_unique_lock<boost::shared_mutex> unique(uplock);
            root_node_.RemoveObject(object);
            arrangement_id = newContainer->__InternalInsert(object, new_position, arrangement_id);
        }

        //Split into 3 groups -- only ours, only new, and both ours and new
        std::set<std::shared_ptr<Object>> oldObservers, newObservers, bothObservers;

        object->__InternalViewAwareObjects([&] (std::shared_ptr<Object> observer)
        {
            oldObservers.insert(observer);
        });

        newContainer->__InternalViewAwareObjects([&] (std::shared_ptr<Object> observer)
        {
            if(newContainer->GetPermissions()->canView(newContainer, observer))
            {
                auto itr = oldObservers.find(observer);
                if(itr != oldObservers.end())
                {
                    oldObservers.erase(itr);
                    bothObservers.insert(observer);
                }
                else
                {
                    newObservers.insert(observer);
                }
            }
        });

        //Send Creates to only new
        for_each(newObservers.begin(), newObservers.end(), [&object](shared_ptr<Object> observer)
        {
            object->__InternalAddAwareObject(observer, true);
        });

        //Send updates to both
        for_each(bothObservers.begin(), bothObservers.end(), [&object](shared_ptr<Object> observer)
        {
            object->SendUpdateContainmentMessage(observer->GetController());
        });

        //Send destroys to only ours
        for_each(oldObservers.begin(), oldObservers.end(), [&object](shared_ptr<Object> observer)
        {
            object->__InternalRemoveAwareObject(observer, true);
        });
    }
}

void QuadtreeSpatialProvider::ViewObjectsInRange(glm::vec3 position, float radius, uint32_t max_depth, bool topDown, std::function<void(std::shared_ptr<swganh::object::Object>)> func)
{
    std::set<std::shared_ptr<Object>> contained_objects;

    boost::shared_lock<boost::shared_mutex> lock(global_container_lock_);
    contained_objects = root_node_.Query(QueryBox(swganh::object::Point(position.x - radius, position.z - radius),
                                         swganh::object::Point(position.x + radius, position.z + radius)));

for (auto& object : contained_objects)
    {
        if (topDown)
            func(object);

        if (max_depth != 1)
            object->__InternalViewObjects(nullptr, (max_depth == 0 ? 0 : max_depth - 1), topDown, func);

        if (!topDown)
            func(object);
    }

}

void QuadtreeSpatialProvider::__InternalViewObjects(std::shared_ptr<Object> requester, uint32_t max_depth, bool topDown, std::function<void(std::shared_ptr<Object>)> func)
{
    std::set<std::shared_ptr<Object>> contained_objects;
    uint32_t requester_instance = 0;
    if (requester)
    {
        requester_instance = requester->GetInstanceId();
        contained_objects = root_node_.Query(GetQueryBoxViewRange(requester->GetAABB()));
    }

for (auto& object : contained_objects)
    {
        uint32_t object_instance = object->GetInstanceId();

        if(object_instance == 0 || object_instance == requester_instance)
        {
            if (topDown)
                func(object);

            if (max_depth != 1)
                object->__InternalViewObjects(requester, (max_depth == 0 ? 0 : max_depth - 1), topDown, func);

            if (!topDown)
                func(object);
        }
    }
}

void QuadtreeSpatialProvider::__InternalGetObjects(std::shared_ptr<Object> requester, uint32_t max_depth, bool topDown, std::list<std::shared_ptr<Object>>& out)
{
    std::set<std::shared_ptr<Object>> contained_objects;
    uint32_t requester_instance = 0;
    if (requester)
    {
        requester_instance = requester->GetInstanceId();
        contained_objects = root_node_.Query(GetQueryBoxViewRange(requester->GetAABB()));
    }

for (auto object : contained_objects)
    {
        uint32_t object_instance = object->GetInstanceId();

        if(object_instance == 0 || object_instance == requester_instance)
        {
            if (topDown)
                out.push_back(object);

            if (max_depth != 1)
                object->__InternalGetObjects(requester, (max_depth == 0 ? 0 : max_depth - 1), topDown, out);

            if (!topDown)
                out.push_back(object);
        }
    }
}

void QuadtreeSpatialProvider::__InternalViewAwareObjects(std::function<void(std::shared_ptr<swganh::object::Object>)> func, std::shared_ptr<swganh::object::Object> hint)
{
    __InternalViewObjects(hint, 0, true, func);
}

int32_t QuadtreeSpatialProvider::__InternalInsert(std::shared_ptr<Object> object, glm::vec3 new_position, int32_t arrangement_id)
{
    //Update position now to make sure the object ends up where it needs to be.
    object->SetContainer(shared_from_this());
    object->SetPosition(new_position);
    root_node_.InsertObject(object);
    object->SetSceneId(scene_id_);
    return -1;
}

void QuadtreeSpatialProvider::__InternalGetAbsolutes(glm::vec3& pos, glm::quat& rot)
{
    pos = glm::vec3();
    rot = glm::quat();
}

QueryBox QuadtreeSpatialProvider::GetQueryBoxViewRange(std::shared_ptr<Object> object)
{
    return GetQueryBoxViewRange(object->GetAABB());
}

QueryBox QuadtreeSpatialProvider::GetQueryBoxViewRange(const swganh::object::AABB& box)
{
    auto &min = box.min_corner(), &max = box.max_corner();

    return QueryBox(quadtree::Point(min.x() - VIEWING_RANGE, min.y() - VIEWING_RANGE),
                    quadtree::Point(max.x() + VIEWING_RANGE, max.y() + VIEWING_RANGE));
}

std::set<std::shared_ptr<swganh::object::Object>> QuadtreeSpatialProvider::Query(boost::geometry::model::polygon<swganh::object::Point> query_box)
{
    std::set<std::shared_ptr<swganh::object::Object>> return_vector;
    QueryBox aabb;

    boost::geometry::envelope(query_box, aabb);

    return_vector = root_node_.Query(aabb); // Find objects without our AABB
    for(auto i = return_vector.begin(); i != return_vector.end(); i++)
    {
        // Do more precise intersection detection, if we are not colliding, erase.
        if(boost::geometry::intersects((*i)->GetWorldCollisionBox(), query_box) == false)
            i = return_vector.erase(i);
    }

    return return_vector;
}

std::set<std::pair<float, std::shared_ptr<swganh::object::Object>>> QuadtreeSpatialProvider::FindObjectsInRangeByTag(std::shared_ptr<swganh::object::Object> requester, const std::string& tag, float range)
{
    std::set<std::pair<float, std::shared_ptr<swganh::object::Object>>> obj_map;

    if(requester->GetContainer() != shared_from_this())
    {
        auto root_obj = requester->GetContainer();
        while(root_obj->GetContainer() != shared_from_this() && root_obj->GetContainer() != nullptr)
            root_obj = root_obj->GetContainer();

        root_obj->ViewObjects(requester, 0, true, [=, &obj_map](std::shared_ptr<swganh::object::Object> object)
        {
            if(object->HasFlag(tag))
            {
                glm::vec3 pos1, pos2;
                glm::quat o1, o2;
                requester->GetAbsolutes(pos1, o1);
                object->GetAbsolutes(pos2, o2);
                obj_map.insert(std::make_pair(glm::distance(pos1, pos2), object));
            }
        });

        return obj_map;
    }

    QueryBox query_box;
    if(range < 0) // Query the whole map.
        query_box = QueryBox(quadtree::Point(-8300.0f, -8300.0f), quadtree::Point(8300.0f, 8300.0f));
    else // Generate query box.
        query_box = QueryBox(
                        quadtree::Point(requester->GetPosition().x - (range /  2.0f), requester->GetPosition().z - (range / 2.0f)),
                        quadtree::Point(requester->GetPosition().x + (range / 2.0f), requester->GetPosition().z + (range / 2.0f))
                    );

    auto objects = root_node_.Query(query_box);
    for(auto& object : objects)
    {
        if(object->HasFlag(tag))
        {
            glm::vec3 pos1, pos2;
            glm::quat o1, o2;
            requester->GetAbsolutes(pos1, o1);
            object->GetAbsolutes(pos2, o2);

            obj_map.insert(std::pair<float, std::shared_ptr<swganh::object::Object>>(glm::distance(pos1, pos2),object));
            object->ViewObjects(object, 0, true, [=, &obj_map](std::shared_ptr<Object> object)
            {
                glm::vec3 pos1, pos2;
                glm::quat o1, o2;
                if(object->HasFlag(tag))
                {
                    requester->GetAbsolutes(pos1, o1);
                    object->GetAbsolutes(pos2, o2);
                    obj_map.insert(std::pair<float, std::shared_ptr<swganh::object::Object>>(glm::distance(pos1, pos2), object));
                }
            });
        }
    }
    return obj_map;
}

void QuadtreeSpatialProvider::CheckCollisions(std::shared_ptr<swganh::object::Object> object)
{
    // Prep work
    auto new_objects = root_node_.Query(object->GetAABB());
    auto old_objects = object->GetCollidedObjects();

    auto new_itr = new_objects.begin();
    auto new_end = new_objects.end();
    bool new_done = new_itr == new_end;

    auto old_itr = old_objects.begin();
    auto old_end = old_objects.end();
    bool old_done = old_itr == old_end;

    while(!new_done || !old_done)
    {
        if(old_done || (!new_done && *new_itr < *old_itr))
        {
            //It's a new object!
            if (object != *new_itr)
            {
                object->AddCollidedObject(*new_itr);
                (*new_itr)->AddCollidedObject(object);
            }

            ++new_itr;
        }
        else if(new_done || (!old_done && *old_itr < *new_itr))
        {
            //It's an old object!
            if(object != *old_itr)
            {
                object->RemoveCollidedObject(*old_itr);
                (*old_itr)->RemoveCollidedObject(object);
            }

            ++old_itr;
        }
        else
        {
            //Otherwise both are equal
            if(object != *new_itr)
            {
                object->OnCollisionStay(*new_itr);
                (*new_itr)->OnCollisionStay(object);
            }

            ++new_itr;
            ++old_itr;
        }

        new_done = new_itr == new_end;
        old_done = old_itr == old_end;
    }
}
