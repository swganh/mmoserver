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

#ifndef SRC_UTILS_MATHFUNCTIONS_H_
#define SRC_UTILS_MATHFUNCTIONS_H_

#include <glm/glm.hpp>

/**
 * Checks to see if a given point is within the bounds of a rectangle given its center and width/height.
 *
 * \param check_point A point to check whether or not is inside a rectangle's bounds.
 * \param rectangle_center The center point of a rectangle.
 * \param width The width of the rectangle.
 * \param height The height of the rectangle.
 * \returns True if the check_point is within the rectangle bounds, false if not.
 */
bool IsPointInRectangle(const glm::vec2& check_point, const glm::vec2& rectangle_center, float width, float height);

#endif  // SRC_UTILS_MATHFUNCTIONS_H_
