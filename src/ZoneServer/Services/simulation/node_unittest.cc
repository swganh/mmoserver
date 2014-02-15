// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE Spatial Index Test
#include <boost/test/unit_test.hpp>

#include <boost/random.hpp>

#include "node.h"
#include "swganh_core/object/object.h"

#include <boost/foreach.hpp>
#include <boost/geometry.hpp>


using namespace quadtree;

///
class NodeTest
{
public:
    NodeTest()
        : root_node_(ROOT, Region(Point(-8300.0f, -8300.0f), Point(8300.0f, 8300.0f)), 0, 9, nullptr)
        , event_dispatcher_(io_service_)
    {}

    ~NodeTest()
    {}

protected:
    virtual void SetUp() { }

    Node root_node_;
    swganh::EventDispatcher event_dispatcher_;
    boost::asio::io_service io_service_;
};

BOOST_FIXTURE_TEST_SUITE(SpatialIndexNode, NodeTest)
///
BOOST_AUTO_TEST_CASE(CanInsertRemoveObject)
{
    std::shared_ptr<swganh::object::Object> obj(new swganh::object::Object());
    obj->SetEventDispatcher(&event_dispatcher_);
    obj->SetPosition(glm::vec3(10.0f, 10.0f, 10.0f));

    root_node_.InsertObject(obj);
    BOOST_CHECK_EQUAL(size_t(1), root_node_.GetContainedObjects().size());

    root_node_.RemoveObject(obj);
    BOOST_CHECK_EQUAL(size_t(0), root_node_.GetContainedObjects().size());
}

///
BOOST_AUTO_TEST_CASE(VerifyQuadrantSplit)
{
    std::shared_ptr<swganh::object::Object> obj1(new swganh::object::Object()), obj2(new swganh::object::Object()), obj3(new swganh::object::Object()), obj4(new swganh::object::Object());

    obj1->SetEventDispatcher(&event_dispatcher_);
    obj2->SetEventDispatcher(&event_dispatcher_);
    obj3->SetEventDispatcher(&event_dispatcher_);
    obj4->SetEventDispatcher(&event_dispatcher_);

    obj1->SetObjectId(1);
    obj2->SetObjectId(2);
    obj3->SetObjectId(3);
    obj4->SetObjectId(4);

    obj1->SetPosition(glm::vec3(-10.0f, 0.0f, 10.0f));
    obj2->SetPosition(glm::vec3(10.0f, 0.0f, 10.0f));
    obj3->SetPosition(glm::vec3(-10.0f, 0.0f, -10.0f));
    obj4->SetPosition(glm::vec3(10.0f, 0.0f, -10.0f));

    root_node_.InsertObject(obj1);
    root_node_.InsertObject(obj2);
    root_node_.InsertObject(obj3);
    root_node_.InsertObject(obj4);

    BOOST_CHECK_EQUAL(size_t(1), root_node_.GetLeafNodes()[NW_QUADRANT]->GetObjects().size());
    BOOST_CHECK_EQUAL(size_t(1), root_node_.GetLeafNodes()[NE_QUADRANT]->GetObjects().size());
    BOOST_CHECK_EQUAL(size_t(1), root_node_.GetLeafNodes()[SW_QUADRANT]->GetObjects().size());
    BOOST_CHECK_EQUAL(size_t(1), root_node_.GetLeafNodes()[SE_QUADRANT]->GetObjects().size());

    BOOST_CHECK_EQUAL(uint64_t(1), (*root_node_.GetLeafNodes()[NW_QUADRANT]->GetObjects().begin())->GetObjectId());
    BOOST_CHECK_EQUAL(uint64_t(2), (*root_node_.GetLeafNodes()[NE_QUADRANT]->GetObjects().begin())->GetObjectId());
    BOOST_CHECK_EQUAL(uint64_t(3), (*root_node_.GetLeafNodes()[SW_QUADRANT]->GetObjects().begin())->GetObjectId());
    BOOST_CHECK_EQUAL(uint64_t(4), (*root_node_.GetLeafNodes()[SE_QUADRANT]->GetObjects().begin())->GetObjectId());

    root_node_.RemoveObject(obj1);
    root_node_.RemoveObject(obj2);
    root_node_.RemoveObject(obj3);
    root_node_.RemoveObject(obj4);

    // TODO: Check that all LeafNodes have been disposed of.
}

///
BOOST_AUTO_TEST_CASE(CanQuery)
{
    std::shared_ptr<swganh::object::Object> obj(new swganh::object::Object());
    obj->SetObjectId(0xDEADBABE);
    obj->SetEventDispatcher(&event_dispatcher_);
    obj->SetPosition(glm::vec3(10.0f, 0.0f, 10.0f));

    root_node_.InsertObject(obj);

    BOOST_CHECK_EQUAL(size_t(1), root_node_.Query(QueryBox( Point(0.0f, 0.0f), Point(15.0f, 15.0f) )).size());

    root_node_.RemoveObject(obj);

    BOOST_CHECK_EQUAL(size_t(0), root_node_.Query(QueryBox( Point(0.0f, 0.0f), Point(15.0f, 15.0f) )).size());
}

///
BOOST_AUTO_TEST_CASE(CanUpdateObject)
{
    std::shared_ptr<swganh::object::Object> obj(new swganh::object::Object());
    obj->SetObjectId(0xDEADBABE);
    obj->SetEventDispatcher(&event_dispatcher_);
    obj->SetPosition(glm::vec3(10.0f, 0.0f, 10.0f));

    root_node_.InsertObject(obj);

    BOOST_CHECK_EQUAL(size_t(1), root_node_.GetLeafNodes()[NE_QUADRANT]->GetObjects().size());
    BOOST_CHECK_EQUAL(size_t(0), root_node_.GetLeafNodes()[SE_QUADRANT]->GetObjects().size());

    glm::vec3 new_position(10.0f, 0.0f, -10.0f);
    auto old_bounding_volume = obj->GetAABB();
    obj->SetPosition(new_position);
    root_node_.UpdateObject(obj, old_bounding_volume, obj->GetAABB());

    BOOST_CHECK_EQUAL(size_t(0), root_node_.GetLeafNodes()[NE_QUADRANT]->GetObjects().size());
    BOOST_CHECK_EQUAL(size_t(1), root_node_.GetLeafNodes()[SE_QUADRANT]->GetObjects().size());

    root_node_.RemoveObject(obj);
}

///
BOOST_AUTO_TEST_CASE(CanInsertRemoveQueryOneThousand)
{
    std::vector<std::shared_ptr<swganh::object::Object>> objects;
    boost::random::mt19937 gen;
    boost::random::uniform_real_distribution<> random_generator(-3000.0f, 3000.0f);
    boost::random::uniform_int_distribution<> id_generator(1000, 1000000000);

    for(int i = 0; i < 1000; i++)
    {
        objects.push_back(std::make_shared<swganh::object::Object>());
        objects[i]->SetObjectId(id_generator(gen));
        objects[i]->SetEventDispatcher(&event_dispatcher_);
        objects[i]->SetPosition(glm::vec3(random_generator(gen), 0.0f, random_generator(gen)));
        root_node_.InsertObject(objects[i]);
    }

    BOOST_CHECK_EQUAL(size_t(1000), root_node_.GetContainedObjects().size());
    BOOST_CHECK_EQUAL(size_t(1000), root_node_.Query(QueryBox(Point(-3000, -3000), Point(3000, 3000))).size());

    for(int i = 0; i < 1000; i++)
    {
        root_node_.RemoveObject(objects[i]);
    }

    BOOST_CHECK_EQUAL(size_t(0), root_node_.GetContainedObjects().size());
    BOOST_CHECK_EQUAL(size_t(0), root_node_.Query(QueryBox(Point(-3000, -3000), Point(3000, 3000))).size());
}

///
BOOST_AUTO_TEST_CASE(CanInsertRemoveQueryTenThousand)
{
    std::vector<std::shared_ptr<swganh::object::Object>> objects;
    boost::random::mt19937 gen;
    boost::random::uniform_real_distribution<> random_generator(-3000.0f, 3000.0f);
    boost::random::uniform_int_distribution<> id_generator(1000, 1000000000);

    for(int i = 0; i < 10000; i++)
    {
        objects.push_back(std::make_shared<swganh::object::Object>());
        objects[i]->SetObjectId(id_generator(gen));
        objects[i]->SetEventDispatcher(&event_dispatcher_);
        objects[i]->SetPosition(glm::vec3(random_generator(gen), 0.0f, random_generator(gen)));
        root_node_.InsertObject(objects[i]);
    }

    BOOST_CHECK_EQUAL(size_t(10000), root_node_.GetContainedObjects().size());
    BOOST_CHECK_EQUAL(size_t(10000), root_node_.Query(QueryBox(Point(-3000, -3000), Point(3000, 3000))).size());

    for(int i = 0; i < 10000; i++)
    {
        root_node_.RemoveObject(objects[i]);
    }

    BOOST_CHECK_EQUAL(size_t(0), root_node_.GetContainedObjects().size());
    BOOST_CHECK_EQUAL(size_t(0), root_node_.Query(QueryBox(Point(-3000, -3000), Point(3000, 3000))).size());
}

BOOST_AUTO_TEST_SUITE_END()
/*****************************************************************************/
