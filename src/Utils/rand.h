/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_UTILS_RAND_H
#define ANH_UTILS_RAND_H


//======================================================================================================================
void			seed_rand_mwc1616(unsigned int seed);
unsigned int	rand_mwc1616(void);


#include "typedefs.h"


//==============================================================================================================================

namespace Anh_Utils
{
	//==============================================================================================================================
	class Random
	{
		public:

			// The random generator will be seeded when class becomes instantiated (at first use).
			static Random*	getSingleton();
		
			// The getRand function returns a pseudorandom integer in the range 0 to RAND_MAX (32767). 
			inline int32 getRand(void) const { return rand();}

			// seedRand shall ONLY be used when you have the need to repeat the "random numbers", i.e. debugging.
			void	seedRand(const uint32) const;

		protected:
			Random();
			~Random();

		private:
			static Random*	  mSingleton;
	};

}

#define gRandom	Anh_Utils::Random::getSingleton()

//==============================================================================================================================


#endif //ANH_UTILS_RAND_H


