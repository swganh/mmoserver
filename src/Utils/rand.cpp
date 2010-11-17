/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

/*

 MWC1616
 return value made by concatenating two 16-bit
 multiply-with-carry generators.
 The two generators have the form
    x(n)=a*x(n-1)+carry mod 2^16      and
    y(n)=b*y(n-1)+carry mod 2^16,
 in this case a and b are choosen as 18000 and 30903

 Some other good choices for a and b
 18000 18030 18273 18513 18879 19074 19098 19164 19215 19584
 19599 19950 20088 20508 20544 20664 20814 20970 21153 21243
 21423 21723 21954 22125 22188 22293 22860 22938 22965 22974
 23109 23124 23163 23208 23508 23520 23553 23658 23865 24114
 24219 24660 24699 24864 24948 25023 25308 25443 26004 26088
 26154 26550 26679 26838 27183 27258 27753 27795 27810 27834
 27960 28320 28380 28689 28710 28794 28854 28959 28980 29013
 29379 29889 30135 30345 30459 30714 30903 30963 31059 31083

 The `carry'  c works as follows:  If a and x are 16-bit  and
 c at most 14 bits, then forming a*x+c produces an at-most 31-
 bit result.  That result mod 2^16 (the rightmost 16 bits) is
 the new x and the topmost 16 bits the new carry c.  The sequ-
 ence of resulting x's has period the order of 2^16 in the
 group of residues relatively prime to m=a*2^16-1, which will
 be a*2^15-1 for the multipliers suggested here.
 You will be prompted to choose a and b and two seeds.  Output
 is a 32-bit integer, the pair x,y side by side.
 This multiply-with-carry generator is best done in assembler,
 where it takes about 200 nanosecs with a Pentium 120. A Fort-
 ran version takes about 300 ns.  It seems to pass all tests
 and is highly recommended for speed and simplicity.
 */

#include <stdlib.h>
#include <assert.h>

#include "rand.h"
#include "clock.h"

static unsigned int mwc1616_x = 1;
static unsigned int mwc1616_y = 2;

void seed_rand_mwc1616(unsigned int seed) {
    mwc1616_x = seed | 1;   /* Can't seed with 0 */
    mwc1616_y = seed | 2;
}

unsigned int rand_mwc1616(void) {
    mwc1616_x = 18000 * (mwc1616_x & 0xffff) + (mwc1616_x >> 16);
    mwc1616_y = 30903 * (mwc1616_y & 0xffff) + (mwc1616_y >> 16);
    return (mwc1616_x<<16)+(mwc1616_y&0xffff);
}

//======================================================================================================================

Anh_Utils::Random*	Anh_Utils::Random::mSingleton = NULL;

//======================================================================================================================

Anh_Utils::Random* Anh_Utils::Random::getSingleton()
{
    if (!mSingleton)
    {
        mSingleton = new Anh_Utils::Random();
        srand((uint32)(Anh_Utils::Clock::getSingleton()->getLocalTime()));
    }
    return mSingleton;
}

//==============================================================================================================================
Anh_Utils::Random::Random()
{

}

//======================================================================================================================

Anh_Utils::Random::~Random()
{

}

void Anh_Utils::Random::seedRand(const uint32 seed) const
{
    srand(seed);
}

//======================================================================================================================
