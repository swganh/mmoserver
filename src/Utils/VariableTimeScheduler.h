/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_UTILS_VARIABLETIMESCHEDULER_H
#define ANH_UTILS_VARIABLETIMESCHEDULER_H

#include "typedefs.h"
#include "FastDelegate.h"
#include "PriorityVector.h"
#include "clock.h"
#include <algorithm>


typedef fastdelegate::FastDelegate2<uint64,void*,uint64> VariableTimeCallback;


namespace Anh_Utils
{
	//======================================================================================================================

	class VariableTimeTask
	{
		public:

			VariableTimeTask(uint64 id,uint8 priority,uint64 lastCallTime,uint64 interval,VariableTimeCallback callback,void* async)
				: mId(id),mPriority(priority),mLastCallTime(lastCallTime),mInterval(interval),mCallback(callback),mAsync(async){}
			
			~VariableTimeTask(){}

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
			void	reset(){ mNextTask = 0; }
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



