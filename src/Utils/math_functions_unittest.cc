/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "Utils/MathFunctions.h"

#include <gtest/gtest.h>

// Wrapping tests in an anonymous namespace prevents potential name conflicts
namespace {
    
TEST(MathFunctionsTests, CanCheckIfPointIsInRectangleBoundries) {
    // Load up some initial data
    glm::vec2 rec_center(-5160.0f, 4380.0f);
    glm::vec2 player_pos1(-4000.0f, 4000.0f);
    glm::vec2 player_pos2(-6500.0f, 3000.0f);

    float width = 2500.0f;
    float height = 1500.0f;

    // This point should be inside the rectangle.
    EXPECT_TRUE(IsPointInRectangle(player_pos1, rec_center, width, height));

    // This point should not be inside the rectangle.
    EXPECT_FALSE(IsPointInRectangle(player_pos2, rec_center, width, height));
}

}  // namespace
