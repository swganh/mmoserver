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

#include "Utils/MathFunctions.h"

bool IsPointInRectangle(const glm::vec2& check_point, const glm::vec2& rectangle_center, float width, float height) {
    // Get nobuild lower right and upper left corners.
    glm::vec2 lower_left(rectangle_center.x - (0.5*width), rectangle_center.y - (0.5*height));
    glm::vec2 upper_right(rectangle_center.x + (0.5*width), rectangle_center.y + (0.5*height));

    // Check and see if the player is within this no build region.
    glm::vec2::bool_type greater_than = glm::greaterThanEqual(check_point, lower_left);
    glm::vec2::bool_type less_than = glm::lessThanEqual(check_point, upper_right);

    if (greater_than.x && greater_than.y && less_than.x && less_than.y) {
        return true;
    }

    return false;
}
