/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "clock.h"

#include <cassert>
//#include <ctime>
#include "Scheduler.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost\date_time.hpp"

namespace pt = boost::posix_time;



using namespace Anh_Utils;

//======================================================================================================================

Clock* Clock::mSingleton = NULL;
bool	Clock::mInsFlag    = false;

//======================================================================================================================

Clock::Clock()
{
	
	pt::ptime time  = pt::second_clock::universal_time();
	time_fix = time;

	mStoredTime = 0;
	mBoostTime = time;
    
	mClockScheduler		= new Anh_Utils::Scheduler(this);
    mClockScheduler->addTask(fastdelegate::MakeDelegate(this,&Clock::_setStoredTime),1,100,NULL);
	
	pt::ptime epoch(boost::gregorian::date(1970, 1, 1));
	date_fix = epoch;
	//pt::ptime epoch = time;

	
}

//======================================================================================================================

Clock::~Clock()
{
    delete(mClockScheduler);

}
//======================================================================================================================

void Clock::process()
{
    mClockScheduler->process();
}

Clock* Anh_Utils::Clock::Init()
{
    if(!mInsFlag)
    {
        mSingleton = new Clock();
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//==============================================================================================================================

void Clock::destroySingleton()
{
    delete mSingleton;
    mSingleton = 0;
}

//==============================================================================================================================

std::string	Clock::GetCurrentDateTimeString()
{
	std::stringstream s;
	pt::ptime current_date_microseconds = pt::microsec_clock::local_time();
	s << current_date_microseconds;
	return s.str();
}

char* Clock::GetCurrentDateTimeChar()
{
    time_t ltime;
    time( &ltime);
    return ctime(&ltime);
}

//==============================================================================================================================

pt::ptime 	Clock::getBoostTime()
{
	return pt::second_clock::local_time();
}

pt::ptime 	Clock::getStoredBoostTime()
{
	return mBoostTime;
}


uint64 Clock::getGlobalTime() const
{
    pt::ptime time  = pt::second_clock::universal_time();
	pt::time_duration diff(time - date_fix);
	return  diff.total_milliseconds();
	//return time.time_of_day().total_milliseconds();
}

//==============================================================================================================================




uint64 Clock::getLocalTime() const
{
	//return mStoredTime;
	pt::ptime time  = pt::second_clock::universal_time();
	pt::time_duration diff(time - time_fix);
	return  diff.total_milliseconds();
	//return time.time_of_day().total_milliseconds();
}