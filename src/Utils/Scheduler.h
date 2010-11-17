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

#ifndef ANH_UTILS_SCHEDULER_H
#define ANH_UTILS_SCHEDULER_H

#include <algorithm>
#include "typedefs.h"
#include "FastDelegate.h"
#include "PriorityVector.h"
#include "clock.h"

typedef fastdelegate::FastDelegate2<uint64,void*,bool> FDCallback;


namespace Anh_Utils
{
//======================================================================================================================

class Task
{
public:

    Task(uint64 id,uint8 priority,uint64 lastCallTime,uint64 interval,FDCallback callback,void* async)
        : mId(id),mPriority(priority),mLastCallTime(lastCallTime),mInterval(interval),mCallback(callback),mAsync(async) {}

    ~Task() {}

    bool operator< (const Task& right) const
    {
        return(mPriority < right.mPriority);
    }

    uint64		mId;
    uint8		mPriority;
    uint64		mLastCallTime;
    uint64		mInterval;
    FDCallback	mCallback;
    void*		mAsync;
};

//======================================================================================================================

typedef priority_vector<Task> TaskContainer;

//======================================================================================================================

class Scheduler
{
public:

    Scheduler(uint64 processTimeLimit = 100, uint64 throttleLimit = 0);
    ~Scheduler();

    uint64	addTask(FDCallback callback,uint8 priority,uint64 interval,void* async);
    void	removeTask(uint64 id);
    bool	checkTask(uint64 id);
    void	reset() {
        mNextTask = 0;
    }
    void	process();
    bool	runTask();

protected:

    TaskContainer		mTasks;

    uint32				mNextTask;
    uint64				mNextTaskId;
    // Anh_Utils::Clock*	mClock;
    uint64				mProcessTimeLimit, mThrottleLimit, mLastProcessTime;
};
}

#endif

//======================================================================================================================



