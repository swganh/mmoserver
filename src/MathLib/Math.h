/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_MATHLIB_MATH_H
#define ANH_MATHLIB_MATH_H


//=============================================================================
//
// generic functions
//

namespace Anh_Math
{
	//=============================================================================
	//
	// rounds a float to the number of places given
	//

	inline float roundF(float x,const int places)
	{
		const float shift = pow(10.0f,places);

		return floorf(x * shift + 0.5f) / shift;
	}

	//=============================================================================
}

//=============================================================================

#endif



