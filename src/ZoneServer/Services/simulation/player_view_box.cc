// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include <swganh_core/simulation/player_view_box.h>

using namespace swganh::simulation;
using namespace swganh::object;

static double VIEWING_RANGE = 128.0f;

PlayerViewBox::PlayerViewBox(std::shared_ptr<Object> player)
    : Object()
    , player_(player)
{
}

PlayerViewBox::~PlayerViewBox()
{
}

void PlayerViewBox::OnCollisionEnter(std::shared_ptr<Object> collider)
{
    std::cout << "PlayerViewBox::OnCollisionEnter" << std::endl;
    // Add aware.
}

void PlayerViewBox::OnCollisionLeave(std::shared_ptr<Object> collider)
{
    std::cout << "PlayerViewBox::OnCollisionLeave" << std::endl;
    // Remove aware.
}

void PlayerViewBox::__BuildCollisionBox()
{
    local_collision_box_.clear();
    boost::geometry::append(local_collision_box_, boost::geometry::make<Point>(-1.0f * VIEWING_RANGE, VIEWING_RANGE));
    boost::geometry::append(local_collision_box_, boost::geometry::make<Point>(VIEWING_RANGE, VIEWING_RANGE));
    boost::geometry::append(local_collision_box_, boost::geometry::make<Point>(VIEWING_RANGE, -1.0f * VIEWING_RANGE));
    boost::geometry::append(local_collision_box_, boost::geometry::make<Point>(-1.0f * VIEWING_RANGE, -1.0f * VIEWING_RANGE));
}