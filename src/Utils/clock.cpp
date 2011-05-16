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

#include "clock.h"

#include <cassert>
#include <ctime>
#include "Scheduler.h"

#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)
#include <windows.h>
#include <MMSystem.h>
#else
#include <sys/time.h>
#endif

using namespace Anh_Utils;

//======================================================================================================================

Clock* Clock::mSingleton = NULL;
bool	Clock::mInsFlag    = false;

//======================================================================================================================

Clock::Clock()
{
    mStoredTime = getLocalTime();
    mClockScheduler		= new Anh_Utils::Scheduler();
    mClockScheduler->addTask(fastdelegate::MakeDelegate(this,&Clock::_setStoredTime),1,1000,NULL);

#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)
    timeBeginPeriod(1);
#endif
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

char* Clock::GetCurrentDateTimeString()
{
    time_t ltime;
    time( &ltime);
    return ctime(&ltime);
}

//==============================================================================================================================

uint64 Clock::getGlobalTime() const
{
    return getLocalTime() + mGlobalDrift;
}

//==============================================================================================================================

uint64 Clock::getLocalTime() const
{
#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)
    return timeGetTime();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000)+(tv.tv_usec / 1000);
#endif
}

//==============================================================================================================================

void Clock::setGlobalDrift(int64 drift)
{
    mGlobalDrift = drift;
}

//==============================================================================================================================
