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

#ifndef ANH_MATHLIB_SHAPE_H
#define ANH_MATHLIB_SHAPE_H

#include <glm/glm.hpp>

//=============================================================================

namespace Anh_Math
{
class Shape
{
public:

    Shape() : mPosition(0.0f,0.0f,0.0f) {}
    Shape(const glm::vec3& position) {
        mPosition = position;
    }
    Shape(float x,float y,float z) : mPosition(x,y,z) {}
    virtual ~Shape() {}

    const glm::vec3&		getPosition() {
        return mPosition;
    }
    void			setPosition(const glm::vec3& position) {
        mPosition = position;
    }
    void			setPosition(float x,float y,float z) {
        mPosition.x = x;
        mPosition.y = y;
        mPosition.z = z;
    }

protected:

    glm::vec3	mPosition;
};
}

//=============================================================================

#endif


