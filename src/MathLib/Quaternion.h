/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_MATHLIB_QUATERNION_H
#define ANH_MATHLIB_QUATERNION_H


//=============================================================================
//
// Quaternion
//

#include "Vector3.h"


//=============================================================================

namespace Anh_Math
{
	class Quaternion
	{
		public:

			Quaternion(float x = 0.0,float y = 0.0,float z = 0.0,float w = 1.0) : mX(x),mY(y),mZ(z),mW(w){}
			Quaternion(const Quaternion& q) : mX(q.mX),mY(q.mY),mZ(q.mZ),mW(q.mW){}
		
			Quaternion&	operator= (const Quaternion& q);
			Quaternion	operator+ (const Quaternion& q) const;
			Quaternion	operator- (const Quaternion& q) const;
			Quaternion	operator* (const Quaternion& q) const;

			float	DotProduct(Quaternion& q); 
			float	getNormal() const;  
			float	normalize(); 
			// Vector3	getAnglesToSend();
			float	getAnglesToSend() const;
			Vector3	getAngles();

			static	const Quaternion ZERO;
			static  const Quaternion IDENTITY;

			float	mX;
			float	mY;
			float	mZ;
			float	mW;
	};
}

//=============================================================================

#endif

