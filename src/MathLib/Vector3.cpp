/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Vector3.h"

#include "Utils/rand.h"
#include <math.h>

const Anh_Math::Vector3 Anh_Math::Vector3::ZERO(0,0,0);

//==============================================================================

bool Anh_Math::Vector3::operator== (const Vector3& v)
{
	if(mX == v.mX && mY == v.mY && mZ == v.mZ)
		return(true);

	return(false);
}

//==============================================================================

bool Anh_Math::Vector3::operator!= (const Vector3& v)
{
	if(mX != v.mX || mY != v.mY || mZ != v.mZ)
		return(true);

	return(false);
}

//==============================================================================


Anh_Math::Vector3& Anh_Math::Vector3::operator=(const Anh_Math::Vector3& v) 
{
	mX = v.mX;
	mY = v.mY;
	mZ = v.mZ;

	return *this;
}

//==============================================================================

Anh_Math::Vector3 Anh_Math::Vector3::operator+(const Anh_Math::Vector3& v)const 
{
	Anh_Math::Vector3 vSum;

	vSum.mX = mX + v.mX;
	vSum.mY = mY + v.mY;
	vSum.mZ = mZ + v.mZ;

	return vSum;
}

//==============================================================================

Anh_Math::Vector3 Anh_Math::Vector3::operator-(const Anh_Math::Vector3& v)const 
{
	Anh_Math::Vector3 vDif;

	vDif.mX = mX - v.mX;
	vDif.mY = mY - v.mY;
	vDif.mZ = mZ - v.mZ;

	return vDif;
}

//==============================================================================

Anh_Math::Vector3 Anh_Math::Vector3::operator*(const Anh_Math::Vector3& v)const 
{
	Anh_Math::Vector3 vProd;

	vProd.mX = mX * v.mX;
	vProd.mY = mY * v.mY;
	vProd.mZ = mZ * v.mZ;

	return vProd;
}

//==============================================================================

Anh_Math::Vector3 Anh_Math::Vector3::operator/(const Anh_Math::Vector3& v)const 
{
	Vector3 vDiv;

	vDiv.mX = mX / v.mX;
	vDiv.mY = mY / v.mY;
	vDiv.mZ = mZ / v.mZ;

	return vDiv;
}

//==============================================================================

Anh_Math::Vector3& Anh_Math::Vector3::operator+=(const Anh_Math::Vector3& v) 
{
	mX += v.mX;
	mY += v.mY;
	mZ += v.mZ;

	return *this;
}

//==============================================================================

Anh_Math::Vector3& Anh_Math::Vector3::operator-=(const Anh_Math::Vector3& v) 
{
	mX -= v.mX;
	mY -= v.mY;
	mZ -= v.mZ;

	return *this;
}

//==============================================================================

Anh_Math::Vector3& Anh_Math::Vector3::operator*=(const Anh_Math::Vector3& v) 
{
	mX *= v.mX;
	mY *= v.mY;
	mZ *= v.mZ;

	return *this;
}

//==============================================================================

Anh_Math::Vector3& Anh_Math::Vector3::operator/=(const Anh_Math::Vector3& v) 
{
	mX /= v.mX;
	mY /= v.mY;
	mZ /= v.mZ;

	return *this;
}

//==============================================================================

float Anh_Math::Vector3::getLength()
{
	return sqrt(mX*mX + mY*mY + mZ*mZ);
}

//==============================================================================

float Anh_Math::Vector3::DotProduct(Anh_Math::Vector3& v)
{
	return (mX*v.mX + mY*v.mY + mZ*v.mZ);
}

//==============================================================================

float Anh_Math::Vector3::normalize()
{
	float len = getLength();

	if (len > 0)
	{
		float inv = 1.0f / len;

		mX *= inv;
		mY *= inv;
		mZ *= inv;
	}

	return len;
}

//==============================================================================

Anh_Math::Vector3 Anh_Math::Vector3::CrossProduct(Anh_Math::Vector3& v)
{
	Anh_Math::Vector3 vCross;

	vCross.mX = mY*v.mZ - mZ*v.mY;
	vCross.mY = mZ*v.mX - mX*v.mZ;
	vCross.mZ = mX*v.mY - mY*v.mX;

	return vCross;
}

//=============================================================================

float Anh_Math::Vector3::distance2D(const Anh_Math::Vector3& v) const
{
	float dx,dz;

	dx = (float)fabs(mX - v.mX);
	dz = (float)fabs(mZ - v.mZ);

	return sqrt(dx*dx + dz*dz);
}

//=============================================================================

float Anh_Math::Vector3::distance2D(float x,float z) const
{
	float dx,dz;

	dx = (float)fabs(mX - x);
	dz = (float)fabs(mZ - z);

	return sqrt(dx*dx + dz*dz);
}

//=============================================================================

bool Anh_Math::Vector3::inRange2D(const Anh_Math::Vector3& v,float distance)
{
	distance *= distance;

	float dx = (float)fabs(mX - v.mX);
	float dz = (float)fabs(mZ - v.mZ);
	
	if(dx*dx + dz*dz <= distance)
		return(true);

	return(false);
}

//=============================================================================

bool Anh_Math::Vector3::inRange2D(float x,float z,float distance)
{
	distance *= distance;

	float dx = (float)fabs(mX - x);
	float dz = (float)fabs(mZ - z);

	if(dx*dx + dz*dz <= distance)
		return(true);

	return(false);
}

//=============================================================================

Anh_Math::Vector3 Anh_Math::Vector3::new2DVectorNRadius(float radius)
{
	Anh_Math::Vector3 v2;
	while(!inRange2D(v2,radius))
	{
		v2.mX = (float)(mX - (radius/2)) + gRandom->getRand() % (int)radius;
		v2.mY = mY;
		v2.mZ = (float)(mZ - (radius/2)) + gRandom->getRand() % (int)radius;
	}
return v2;
}


