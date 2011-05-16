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

#ifndef ANH_UTILS_VARIABLETIMESCHEDULER_H
#define ANH_UTILS_VARIABLETIMESCHEDULER_H

#include <algorithm>

#include "typedefs.h"
#include "FastDelegate.h"
#include "PriorityVector.h"
#include "clock.h"

typedef fastdelegate::FastDelegate2<uint64,void*,uint64> VariableTimeCallback;


namespace Anh_Utils
{
//======================================================================================================================

class VariableTimeTask
{
public:

    VariableTimeTask(uint64 id,uint8 priority,uint64 lastCallTime,uint64 interval,VariableTimeCallback callback,void* async)
        : mId(id),mPriority(priority),mLastCallTime(lastCallTime),mInterval(interval),mCallback(callback),mAsync(async) {}

    ~VariableTimeTask() {}

    bool operator< (const VariableTimeTask& right) const
    {
        return(mPriority < right.mPriority);
    }

    uint64		mId;
    uint8		mPriority;
    uint64		mLastCallTime;
    uint64		mInterval;
    VariableTimeCallback	mCallback;
    void*		mAsync;
};

//======================================================================================================================

typedef priority_vector<VariableTimeTask> VariableTaskContainer;

//======================================================================================================================

class VariableTimeScheduler
{
public:

    VariableTimeScheduler(uint64 processTimeLimit = 100, uint64 throttleLimit = 0);
    ~VariableTimeScheduler();

    uint64	addTask(VariableTimeCallback callback,uint8 priority,uint64 interval,void* async);
    void	removeTask(uint64 id);
    bool	checkTask(uint64 id);
    void	reset() {
        mNextTask = 0;
    }
    void	process();
    bool	runTask();

protected:

    VariableTaskContainer		mTasks;

    uint32				mNextTask;
    uint64				mNextTaskId;
    // Anh_Utils::Clock*	mClock;
    uint64				mProcessTimeLimit, mThrottleLimit, mLastProcessTime;
};
}

#endif

//======================================================================================================================



