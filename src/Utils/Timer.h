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

#ifndef ANH_UTILS_TIMER_H
#define ANH_UTILS_TIMER_H

#include <boost/thread/thread.hpp>

#include "Utils/clock.h"
#include "Utils/typedefs.h"

class TimerCallback;

//==============================================================================================================================

class Timer
{
public:
    Timer(uint32 id,TimerCallback* callback,uint64 interval, void* container);
    ~Timer();

    virtual	void	Run();

    uint32			getId() {
        return mId;
    }

private:
    boost::thread   mThread;

    void*						mContainer;
    TimerCallback*	mCallback;
    uint32					mId;
    uint64					mInterval;
    uint64					mLastTick;
};

#endif


