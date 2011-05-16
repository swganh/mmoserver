/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_UTILS_RAND_H
#define ANH_UTILS_RAND_H


#include <cstdlib>
#include "typedefs.h"

//======================================================================================================================
void seed_rand_mwc1616(unsigned int seed);
unsigned int rand_mwc1616(void);


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
    inline int32 getRand(void) const {
        return rand();
    }

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


