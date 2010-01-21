/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_MATHLIB_RECTANGLE_H
#define ANH_MATHLIB_RECTANGLE_H

#include "Shape.h"


//=============================================================================

namespace Anh_Math
{
	class Rectangle : public Shape
	{
		public:

			Rectangle() : Shape(),mWidth(0.0f),mHeight(0.0f){}
			Rectangle(Vector3 lowPosition,float width,float height) : Shape(lowPosition),mWidth(width),mHeight(height){}
			Rectangle(float lowX,float lowZ,float width,float height) : Shape(lowX,0.0f,lowZ),mWidth(width),mHeight(height){}
			virtual ~Rectangle(){}

			float	getWidth(){ return mWidth; }
			float	getHeight(){ return mHeight; }

			void	setWidth(float width){ mWidth = width; }
			void	setHeight(float height){ mHeight = height; }

		protected:

			float mWidth;
			float mHeight;
	};
}

//=============================================================================

#endif


