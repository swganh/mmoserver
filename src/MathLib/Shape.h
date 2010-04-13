/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
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

			Shape() : mPosition(0.0f,0.0f,0.0f){}
            Shape(const glm::vec3& position){ mPosition = position; }
			Shape(float x,float y,float z) : mPosition(x,y,z){}
			virtual ~Shape(){}

			const glm::vec3&		getPosition(){ return mPosition; }
			void			setPosition(const glm::vec3& position){ mPosition = position; }
			void			setPosition(float x,float y,float z){ mPosition.x = x;mPosition.y = y;mPosition.z = z; }

		protected:

			glm::vec3	mPosition;
	};
}

//=============================================================================

#endif


