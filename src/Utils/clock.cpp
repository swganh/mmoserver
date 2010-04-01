/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "clock.h"

#include <cassert>
#include <ctime>
#include "Scheduler.h"

#if(ANH_PLATFORM == ANH_PLATFORM_WIN32)
#include <windows.h>
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
	return tv.tv_usec / 1000;
#endif
}

//==============================================================================================================================

void Clock::setGlobalDrift(int64 drift) 
{ 
	mGlobalDrift = drift; 
}

//==============================================================================================================================
