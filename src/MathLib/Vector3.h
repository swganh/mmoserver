/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_MATHLIB_VECTOR3_H
#define ANH_MATHLIB_VECTOR3_H


//=============================================================================
//
// Vector3
//

namespace Anh_Math
{
	class Vector3
	{
		public:

			Vector3(float x = 0.0,float y = 0.0,float z = 0.0) : mX(x),mY(y),mZ(z){}
			Vector3(const Vector3& v) : mX(v.mX),mY(v.mY),mZ(v.mZ){}

			Vector3&	operator= (const Vector3& v);
			Vector3		operator+ (const Vector3& v)const;
			Vector3		operator- (const Vector3& v)const;
			Vector3		operator* (const Vector3& v)const;
			Vector3		operator/ (const Vector3& v)const;
			Vector3&	operator+=(const Vector3& v);
			Vector3&	operator-=(const Vector3& v);
			Vector3&	operator*=(const Vector3& v);
			Vector3&	operator/=(const Vector3& v);
			bool		operator== (const Vector3& v);
			bool		operator!= (const Vector3& v);

			float	getLength();
			float	DotProduct(Vector3& v);
			float	normalize();
			Vector3 CrossProduct(Vector3& v);
			float	distance2D(const Vector3& v) const;
			float	distance2D(float x,float z) const;
			bool	inRange2D(const Vector3& v,float distance);
			bool	inRange2D(float x,float z,float distance);
			Vector3 new2DVectorNRadius(float radius);

			static const Vector3 ZERO;

			float	mX;
			float	mY;
			float	mZ;
	};
}

//=============================================================================

#endif

