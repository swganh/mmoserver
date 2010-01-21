/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_UTILS_SCHEDULER_H
#define ANH_UTILS_SCHEDULER_H

#include "typedefs.h"
#include "FastDelegate.h"
#include "PriorityVector.h"
#include "clock.h"
#include <algorithm>


typedef fastdelegate::FastDelegate2<uint64,void*,bool> FDCallback;


namespace Anh_Utils
{
	//======================================================================================================================

	class Task
	{
		public:

			Task(uint64 id,uint8 priority,uint64 lastCallTime,uint64 interval,FDCallback callback,void* async)
				: mId(id),mPriority(priority),mLastCallTime(lastCallTime),mInterval(interval),mCallback(callback),mAsync(async){}
			
			~Task(){}

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
			void	reset(){ mNextTask = 0; }
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



