/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_UTILS_CONCURRENT_QUEUE_H
#define ANH_UTILS_CONCURRENT_QUEUE_H

#include "zthread/Mutex.h"
#include <queue>


//======================================================================================================================

namespace Anh_Utils
{
	template<class T,class Container = std::deque<T>,class QueueMutex = ZThread::Mutex >
	class concurrent_queue
	{
		public:

			void push(const T& item)
			{
				mMutex.acquire();

				mContainer.push_back(item);

				mMutex.release();
			}

			T pop()
			{
				mMutex.acquire();

				T item = mContainer.front();
				mContainer.pop_front();

				mMutex.release();

				return(item);
			}

			bool empty() const
			{
				mMutex.acquire();

				bool result = mContainer.empty();

				mMutex.release();

				return(result);
			}

			typename Container::size_type size()
			{
				mMutex.acquire();

				size_t result = mContainer.size();

				mMutex.release();

				return(result);
			}

			T& front()
			{
				mMutex.acquire();

				T item = mContainer.front();

				mMutex.release();

				return(item);
			}

		private:

			Container		mContainer;
			QueueMutex		mMutex;
	};
}

//======================================================================================================================

#endif

