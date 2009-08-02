/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Timer.h"
#include "TimerCallback.h"


//==============================================================================================================================

Timer::Timer(uint32 id,TimerCallback* callback,uint32 interval, void* container) : mId(id),mCallback(callback),mInterval(interval),mContainer(container)
{
	// mClock = Anh_Utils::Clock::getSingleton();

	if(mInterval < 100)
		mInterval = 100;
}

//==============================================================================================================================

Timer::~Timer()
{
}

//==============================================================================================================================

void Timer::Start()
{
	mLastTick = Anh_Utils::Clock::getSingleton()->getLocalTime();
	StartThread();
}

//==============================================================================================================================

void Timer::Run()
{
	uint64 currentTick;

	while(IsRunning())
	{
		currentTick = Anh_Utils::Clock::getSingleton()->getLocalTime();

		if(currentTick - mLastTick >= mInterval)
		{
			mCallback->handleTimer(mId,mContainer);
			mLastTick = currentTick;
		}
		msleep(10);
	}
}

//==============================================================================================================================



