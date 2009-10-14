/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_UTILS_TIMER_H
#define ANH_UTILS_TIMER_H

#include "typedefs.h"
#include "clock.h"
#include <boost/thread/thread.hpp>

class TimerCallback;

//==============================================================================================================================

class Timer
{
public:
    Timer(uint32 id,TimerCallback* callback,uint32 interval, void* container);
    ~Timer();

    virtual	void	Run();

    uint32			getId(){ return mId; }

private:
    void*				mContainer;
    uint32				mId;
    TimerCallback*		mCallback;
    // Anh_Utils::Clock*	mClock;
    uint32				mLastTick;
    uint32				mInterval;
    boost::thread       mThread;
};

#endif


