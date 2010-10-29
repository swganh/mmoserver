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


#include "Scheduler.h"


namespace Anh_Utils
{
//======================================================================================================================

Scheduler::Scheduler(uint64 processTimeLimit, uint64 throttleLimit) : mNextTask(0),mNextTaskId(1),mProcessTimeLimit(processTimeLimit),mThrottleLimit(throttleLimit)
{
    mLastProcessTime = 0;
    // We do have a global clock object, don't use seperate clock and times for every process.
    // mClock = new Anh_Utils::Clock();
}

//======================================================================================================================

Scheduler::~Scheduler()
{
    // delete(mClock);
}

//======================================================================================================================

uint64 Scheduler::addTask(FDCallback callback,uint8 priority,uint64 interval,void* async)
{
    mTasks.push(Task(mNextTaskId,priority,gClock->getLocalTime(),interval,callback,async));
    return(mNextTaskId++);
}

//======================================================================================================================

void Scheduler::removeTask(uint64 id)
{
    if(!id)
        return;

    TaskContainer::iterator it = mTasks.begin();

    while(it != mTasks.end())
    {
        if(it->mId == id)
        {
            mTasks.erase(it);
            break;
        }

        ++it;
    }

    mTasks.assureHeap(true);
}

bool Scheduler::checkTask(uint64 id)
{
    if(!id)
        return false;

    TaskContainer::iterator it = mTasks.begin();

    while(it != mTasks.end())
    {
        if(it->mId == id)
        {
            return true;
        }

        ++it;
    }

    //mTasks.assureHeap(true);
    return false;
}

//======================================================================================================================

void Scheduler::process()
{
    // uint64	frameStartTime = mClock->UpdateAndGetTime();

    // while(runTask() && ((mClock->UpdateAndGetTime() - frameStartTime) < mProcessTimeLimit));
    uint64	frameStartTime = Anh_Utils::Clock::getSingleton()->getLocalTime();

    //Check for throttle
    if(frameStartTime < (mLastProcessTime + mThrottleLimit))
    {
        return;
    }

    while(runTask() && ((Anh_Utils::Clock::getSingleton()->getLocalTime() - frameStartTime) < mProcessTimeLimit));

    //Set internal Clock so we know when the last call was
    mLastProcessTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
}

//======================================================================================================================

bool Scheduler::runTask()
{
    if(!(mTasks.empty()))
    {
        Task&	task(mTasks[mNextTask]);
        uint64	currentTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
        if((currentTime - task.mLastCallTime) > task.mInterval)
        {
            if(task.mCallback(currentTime,task.mAsync) == false)
            {
                removeTask(task.mId);
            }
            else
            {
                task.mLastCallTime = currentTime;
                ++mNextTask;
            }
        }
        else
            ++mNextTask;

        if(mNextTask >= mTasks.size())
        {
            reset();
            return(false);
        }
        else
            return(true);
    }
    else
    {
        reset();
        return(false);
    }
}
}

//======================================================================================================================


