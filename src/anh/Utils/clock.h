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

#ifndef ANH_UTILS_CLOCK_H
#define ANH_UTILS_CLOCK_H

#include "Utils/typedefs.h"
#include "Scheduler.h"
#include "boost/date_time/posix_time/posix_time.hpp"

//==============================================================================================================================

class Scheduler;

#define	 gClock	Anh_Utils::Clock::getSingleton()


namespace boost	{
namespace posix_time	{
	class ptime;
}
}

namespace Anh_Utils
{
/*@brief Class Clock keeps track of system time. either absolute in msecs, or as msecs since server start
*	getLocalTime() is used to get the current time with one second resolution, as getting the system time is expensive in terms of
*	processing time. Use this when dealing with messages as we can handle some tenthousand of them per second
*
*/
class Clock	//: public TimerCallback
{
public:
    static Clock* getSingleton() {
        return mSingleton;
    }
    static Clock* Init();
    static void	destroySingleton(void);


protected:
    Clock();
    ~Clock();

public:
    char*			GetCurrentDateTimeChar();
	std::string		GetCurrentDateTimeString();
	
	/*	@brief	getGlobalTime() is used to get the current time with with milisecond (windows)
	*	or microsecond (*nix) resolution.
	*	as getting the system time is expensive in terms of processing time. 
	*	Use this only in non time critical situations 
	*/
    uint64	getGlobalTime() const;

	/*	@brief	getLocalTime() is used to get the current server up time with milisecond (windows)
	*	or microsecond (*nix) resolution.
	*	as getting the system time is expensive in terms of processing time. 
	*	Use this only in non time critical situations 
	*/
    uint64	getLocalTime() const;

    /*	@brief	getStoredTime() is used to get the current server up time with one second resolution.
	*	as getting the system time is expensive in terms of processing time,
	*	Use this in timecritical situations, like when dealing with messages as we might handle some tenthousands of them per second
	*/
    uint64	getStoredTime() {
        return mStoredTime;
    }

	boost::posix_time::ptime 		getBoostTime();
	boost::posix_time::ptime 		getStoredBoostTime();

	/*	@brief	_setStoredTime() is used by the clocks schedulér to set mStoredTime approx. once per second
	*/
    bool	_setStoredTime(uint64 callTime, void* ref) {
        mStoredTime = getLocalTime();
		mBoostTime  = getBoostTime();
        return true;
    }

    void	process();

private:


    uint64							mTimeDelta;     
    uint64							mStoredTime;
	boost::posix_time::ptime 		mBoostTime; 

    Scheduler*						mClockScheduler;

    static Clock*					mSingleton;
    static bool						mInsFlag;
};
}

//==============================================================================================================================

#endif

