
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Quaternion.h"
#include <cmath>

const Anh_Math::Quaternion Anh_Math::Quaternion::ZERO(0.0,0.0,0.0,0.0);
const Anh_Math::Quaternion Anh_Math::Quaternion::IDENTITY(1.0,0.0,0.0,0.0);


//=============================================================================

Anh_Math::Quaternion& Anh_Math::Quaternion::operator= (const Anh_Math::Quaternion& q)
{
	mX = q.mX;
	mY = q.mY;
	mZ = q.mZ;
	mW = q.mW;

	return *this;
}

//=============================================================================

Anh_Math::Quaternion Anh_Math::Quaternion::operator+ (const Anh_Math::Quaternion& q) const
{
	return Quaternion(mW+q.mW,mX+q.mX,mY+q.mY,mZ+q.mZ);
}

//==============================================================================

Anh_Math::Quaternion Anh_Math::Quaternion::operator- (const Anh_Math::Quaternion& q) const
{
	return Quaternion(mW-q.mW,mX-q.mX,mY-q.mY,mZ-q.mZ);
}

//==============================================================================

Anh_Math::Quaternion Anh_Math::Quaternion::operator* (const Anh_Math::Quaternion& q) const
{
	return Anh_Math::Quaternion
	(
			mW * q.mW - mX * q.mX - mY * q.mY - mZ * q.mZ,
			mW * q.mX + mX * q.mW + mY * q.mZ - mZ * q.mY,
			mW * q.mY + mY * q.mW + mZ * q.mX - mX * q.mZ,
			mW * q.mZ + mZ * q.mW + mX * q.mY - mY * q.mX
	);
}

void Anh_Math::Quaternion::Multiplication (const Anh_Math::Quaternion q)
{
	//Note: Quaternions lack the Commutative Property of multiplication 
	//so be sure this is the function you want to use.
	//This function is equivalent to: this = this * q;
	mX = mW * q.mX + mX * q.mW + mY * q.mZ - mZ * q.mY;
	mY = mW * q.mY + mY * q.mW + mZ * q.mX - mX * q.mZ;
	mZ = mW * q.mZ + mZ * q.mW + mX * q.mY - mY * q.mX;
	mW = mW * q.mW - mX * q.mX - mY * q.mY - mZ * q.mZ;

}

void Anh_Math::Quaternion::Multiplication2 (const Anh_Math::Quaternion q)
{
	//Note: Quaternions lack the Commutative Property of multiplication 
	//so be sure this is the function you want to use.
	//This function is equivalent to: this = q * this;
	mX = q.mW * mX + q.mX * mW + q.mY * mZ - q.mZ * mY;
	mY = q.mW * mY + q.mY * mW + q.mZ * mX - q.mX * mZ;
	mZ = q.mW * mZ + q.mZ * mW + q.mX * mY - q.mY * mX;
	mW = q.mW * mW - q.mX * mX - q.mY * mY - q.mZ * mZ;
}

//==============================================================================

float Anh_Math::Quaternion::DotProduct(Anh_Math::Quaternion& q)
{
	return mW*q.mW+mX*q.mX+mY*q.mY+mZ*q.mZ;
}

//==============================================================================

float Anh_Math::Quaternion::getNormal() const
{
	return mW*mW+mX*mX+mY*mY+mZ*mZ;
}

//==============================================================================

float Anh_Math::Quaternion::normalize()
{
	float len = getNormal();
	float f = 1.0f / sqrt(len);
	*this = *this * f;

	return len;
}

//==============================================================================
// with thks to http://gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation
// for help
void Anh_Math::Quaternion::VectorAxis(const Vector3 &v, float angle)
{
	float sinAngle;
	angle *= 0.5f;
	Vector3 vn(v);
	vn.normalize();
 
	sinAngle = sin(angle);
 
	mX = (vn.mX * (sinAngle/2));
	mY = (vn.mY * (sinAngle/2));
	mZ = (vn.mZ * (sinAngle/2));
	mW = cos(angle);
	//this->normalize();

}

//==============================================================================
// with thks to http://gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation
// for help
void Anh_Math::Quaternion::rotatex(float xrmod)
{
	Quaternion nrot;
	nrot.VectorAxis(Vector3(0.0f, 1.0f, 0.0f),(float)( xrmod * 0.01745));

	Multiplication(nrot);
}

// This version does not handle vectors / floats never used, nor does it change values (sign) of any members. (Compared to previous version below).
float Anh_Math::Quaternion::getAnglesToSend() const
{
	float len = getNormal();

	float angleToSend = 0;
	if (len > 0.0)
	{
		// mW = 1.0 gives s = 0, and we divide with s some rows down from here.
		float w = mW;
		float y = mY;
		float s = sqrt(1-mW*mW);
		if (s != 0.0)
		{
			if (mW > 0.0)
			{
				if (mY < 0.0)
				{
					w *= -1;
					y *= -1;
				}
			}
			float angle = 2.0f*acos(w);
			float t = (angle/6.283f)*100;
			angleToSend = (y/s)*t;
		}
	}
	return angleToSend;
}



// Created a more slim version of this.
/*
Anh_Math::Vector3 Anh_Math::Quaternion::getAnglesToSend()
{
	Anh_Math::Vector3 v;
	float len = getNormal();
	float angle,s,t;

	if(len > 0.0)
	{
		// this needs further testing, just assumed its switched like they did to the Y

		if(mW > 0.0)
		{
			if(mX < 0.0)
			{
				mX*=-1;
				mW *= -1;

				if(mY < 0.0)
					mY*=-1;

				if(mZ < 0.0)
					mZ*=-1;
			}
			else if(mY < 0.0)
			{
				mY*=-1;
				mW *= -1;

				if(mZ < 0.0)
					mZ*=-1;
			}
			else if(mZ < 0.0)
			{
				mZ*=-1;
				mW *= -1;
			}
		}
		// This code may not have been tested or reviwed before... mW = 1 gives s = 0, and we divide with s some rows down from here.
		// What do I missunderstand?
		s = sqrt(1-mW*mW);
		angle = 2.0f*acos(mW);

		t = (angle/6.283f)*100;

		v.mX = (mX/s)*t;
		v.mY = (mY/s)*t;
		v.mZ = (mZ/s)*t;
	}
	else
	{
		v.mX = 0.0f;
		v.mY = 0.0f;
		v.mZ = 0.0f;
	}

	return v;
}
*/
//==============================================================================

Anh_Math::Vector3 Anh_Math::Quaternion::getAngles()
{
	Anh_Math::Vector3 v;
	float len = getNormal();
	float angle,s;

	if(len > 0.0)
	{
		if(mW > 0.0)
		{
			if(mX < 0.0)
			{
				mX*=-1;
				mW *= -1;

				if(mY < 0.0)
					mY*=-1;

				if(mZ < 0.0)
					mZ*=-1;
			}
			else if(mY < 0.0)
			{
				mY*=-1;
				mW *= -1;

				if(mZ < 0.0)
					mZ*=-1;
			}
			else if(mZ < 0.0)
			{
				mZ*=-1;
				mW *= -1;
			}
		}

		s = sqrt(1-mW*mW);
		angle = 2.0f*acos(mW);

		v.mX = (mX/s);
		v.mY = (mY/s);
		v.mZ = (mZ/s);
	}
	else
	{
		v.mX = 0.0f;
		v.mY = 0.0f;
		v.mZ = 0.0f;
	}

	return v;
}

//==============================================================================


