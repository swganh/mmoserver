
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "VariableTimeScheduler.h"
namespace Anh_Utils
{
	//======================================================================================================================

	VariableTimeScheduler::VariableTimeScheduler(uint64 processTimeLimit, uint64 throttleLimit) : mNextTask(0),mNextTaskId(1),mProcessTimeLimit(processTimeLimit),mThrottleLimit(throttleLimit)
	{
		mLastProcessTime = 0;
		// We do have a global clock object, don't use seperate clock and times for every process.
		// mClock = new Anh_Utils::Clock();
	}

	//======================================================================================================================

	VariableTimeScheduler::~VariableTimeScheduler()
	{
		// delete(mClock);
	}

	//======================================================================================================================

	uint64 VariableTimeScheduler::addTask(VariableTimeCallback callback,uint8 priority,uint64 interval,void* async)
	{
		mTasks.push(VariableTimeTask(mNextTaskId,priority,Anh_Utils::Clock::getSingleton()->getLocalTime(),interval,callback,async));
		return(mNextTaskId++);
	}

	//======================================================================================================================

	void VariableTimeScheduler::removeTask(uint64 id)
	{
		//printf("scheduler to do remove task : %I64u",id);
		if(!id)
			return;

		VariableTaskContainer::iterator it = mTasks.begin();

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

	bool VariableTimeScheduler::checkTask(uint64 id)
	{
		//printf("scheduler to do remove task : %I64u",id);
		if(!id)
			return false;

		VariableTaskContainer::iterator it = mTasks.begin();

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

	void VariableTimeScheduler::process()
	{
		uint64	frameStartTime = Anh_Utils::Clock::getSingleton()->getLocalTime();

		//Check for throttle
		if(frameStartTime < mLastProcessTime + mThrottleLimit)
		{
			return;
		}

		while(runTask() && ((Anh_Utils::Clock::getSingleton()->getLocalTime() - frameStartTime) < mProcessTimeLimit));
		
		//Set internal Clock so we know when the last call was
		mLastProcessTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
	}

	//======================================================================================================================

	bool VariableTimeScheduler::runTask()
	{
		if(!(mTasks.empty()))
		{
			VariableTimeTask&	task(mTasks[mNextTask]);
			uint64	currentTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
			if((currentTime - task.mLastCallTime) > task.mInterval)
			{
				uint64 nextTick = task.mCallback(currentTime,task.mAsync);
				if(nextTick == 0)
				{
					removeTask(task.mId);
				}
				else
				{
					task.mInterval = nextTick;
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
