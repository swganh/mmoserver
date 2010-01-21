/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_MATHLIB_SHAPE_H
#define ANH_MATHLIB_SHAPE_H

#include "Vector3.h"


//=============================================================================

namespace Anh_Math
{
	class Shape
	{
		public:

			Shape() : mPosition(0.0f,0.0f,0.0f){}
			Shape(Vector3 position){ mPosition = position; }
			Shape(float x,float y,float z) : mPosition(x,y,z){}
			virtual ~Shape(){}

			Vector3*		getPosition(){ return &mPosition; }
			void			setPosition(Vector3 position){ mPosition = position; }
			void			setPosition(float x,float y,float z){ mPosition.mX = x;mPosition.mY = y;mPosition.mZ = z; }

		protected:

			Vector3	mPosition;
	};
}

//=============================================================================

#endif


