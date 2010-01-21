/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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
		mTasks.push(Task(mNextTaskId,priority,Anh_Utils::Clock::getSingleton()->getLocalTime(),interval,callback,async));
		return(mNextTaskId++);
	}

	//======================================================================================================================

	void Scheduler::removeTask(uint64 id)
	{
		//printf("scheduler to do remove task : %I64u",id);
		if(!id)
			return;

		TaskContainer::iterator it = mTasks.begin();

		while(it != mTasks.end())
		{
			if(it->mId == id)
			{
				mTasks.erase(it);
		//		printf("scheduler removeD task : %I64u",id);
				break;
			}

			++it;
		}

		mTasks.assureHeap(true);
	}

	bool Scheduler::checkTask(uint64 id)
	{
		//printf("scheduler to do remove task : %I64u",id);
		if(!id)
			return false;

		TaskContainer::iterator it = mTasks.begin();

		while(it != mTasks.end())
		{
			if(it->mId == id)
			{
		//		printf("scheduler removeD task : %I64u",id);
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


