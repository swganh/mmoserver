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

#ifndef ANH_MATHLIB_CIRCLE_H
#define ANH_MATHLIB_CIRCLE_H

#include "Shape.h"


//=============================================================================

namespace Anh_Math
{
class Circle : public Shape
{
public:

    Circle() : Shape(),mRadius(0.0f) {}
    Circle(const glm::vec3& center, float radius) : Shape(center),mRadius(radius) {}
    Circle(float centerX, float centerZ, float radius) : Shape(centerX,0.0f,centerZ),mRadius(radius) {}
    virtual ~Circle() {}

    float	getRadius() {
        return mRadius;
    }
    void	setRadius(float radius) {
        mRadius = radius;
    }

protected:

    float mRadius;
};
}

//=============================================================================

#endif


