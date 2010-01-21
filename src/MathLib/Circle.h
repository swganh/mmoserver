/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
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

			Circle() : Shape(),mRadius(0.0f){}
			Circle(Vector3 center,float radius) : Shape(center),mRadius(radius){}
			Circle(float centerX,float centerZ,float radius) : Shape(centerX,0.0f,centerZ),mRadius(radius){}
			virtual ~Circle(){}

			float	getRadius(){ return mRadius; }
			void	setRadius(float radius){ mRadius = radius; }

		protected:

			float mRadius;
	};
}

//=============================================================================

#endif


