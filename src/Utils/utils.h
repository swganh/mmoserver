/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_UTILS_UTILS_H
#define ANH_UTILS_UTILS_H

#include "typedefs.h"
#include <string>


//==============================================================================================================================

uint64 swap64(uint64 value);
std::string strRep(std::string a,std::string b,std::string c);

namespace Anh_Utils
{
	int cmpistr(const char* s1, const char* s2);
	int cmpnistr(const char* s1, const char* s2, size_t length);
	int kbhit(void);
}

//==============================================================================================================================

#endif

