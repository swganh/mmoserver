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

#ifndef ANH_UTILS_CLOCK_H
#define ANH_UTILS_CLOCK_H

#include "typedefs.h"
#include "Scheduler.h"

//==============================================================================================================================

class Scheduler;

#define	 gClock	Anh_Utils::Clock::getSingleton()

namespace Anh_Utils
{
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
    char*	GetCurrentDateTimeString();

    uint64	getGlobalTime() const;
    uint64	getLocalTime() const;

    void	setGlobalDrift(int64 drift);

    //timegettime uses 8µs to execute
    //by stamping our timestamp every second and using it instead
    //we have a much lower resolution but save a lot of processing time
    uint64	getStoredTime() {
        return mStoredTime;
    }
    bool	_setStoredTime(uint64 callTime, void* ref) {
        mStoredTime = getLocalTime();
        return true;
    }
    void	process();

private:


    int64			mGlobalDrift;      // The amount of time the local clock is from the global system clock
    uint64			mStoredTime;
    Scheduler*		mClockScheduler;

    static Clock*	mSingleton;
    static bool		mInsFlag;
};
}

//==============================================================================================================================

#endif

