/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_UTILS_CLOCK_H
#define ANH_UTILS_CLOCK_H

#include "typedefs.h"

//==============================================================================================================================

namespace Anh_Utils
{
    class Clock
	{
    public:
        static Clock* getSingleton();
        static void	destroySingleton(void); 

    protected:
        Clock();
        ~Clock();
    
    public:
        char* GetCurrentDateTimeString();

        uint64 getGlobalTime() const; 
        uint64 getLocalTime() const;

        void setGlobalDrift(int64 drift);

    private:
        int64  mGlobalDrift;      // The amount of time the local clock is from the global system clock

        static Clock* mSingleton;
	};
}

//==============================================================================================================================

#endif

