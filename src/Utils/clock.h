/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
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
		static Clock* getSingleton(){return mSingleton;}
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
		uint64	getStoredTime(){return mStoredTime;}
		bool	_setStoredTime(uint64 callTime, void* ref){mStoredTime = getLocalTime(); return true;}
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

