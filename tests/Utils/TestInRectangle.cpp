/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include <gtest/gtest.h>

#include "Utils/MathFunctions.h"

TEST(MathFunctionsTests, CanCheckIfPointIsInRectangleBoundries) {
    // Load up some initial data
    glm::vec2 rec_center(-5160.0f, 4380.0f);
    glm::vec2 player_pos1(-4000.0f, 4000.0f);
    glm::vec2 player_pos2(-6500.0f, 3000.0f);

    float width = 2500.0f;
    float height = 1500.0f;

    // This point should be inside the rectangle.
    EXPECT_EQ(true, IsPointInRectangle(player_pos1, rec_center, width, height));

    // This point should not be inside the rectangle.
    EXPECT_EQ(false, IsPointInRectangle(player_pos2, rec_center, width, height));
}
