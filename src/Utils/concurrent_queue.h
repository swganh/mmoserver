/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_UTILS_CONCURRENT_QUEUE_H
#define ANH_UTILS_CONCURRENT_QUEUE_H

#include <boost/thread/recursive_mutex.hpp>
#include <queue>


//======================================================================================================================

namespace Anh_Utils
{
	template<class T,class Container = std::deque<T>,class QueueMutex = boost::recursive_mutex >
	class concurrent_queue
	{
		public:

			void push(const T& item)
			{
                boost::recursive_mutex::scoped_lock lk(mMutex);

				mContainer.push_back(item);
			}

			T pop()
			{
                boost::recursive_mutex::scoped_lock lk(mMutex);

				T item = mContainer.front();
				mContainer.pop_front();

				return(item);
			}

			bool empty() const
			{
                boost::recursive_mutex::scoped_lock lk(mMutex);

				bool result = mContainer.empty();

				return(result);
			}

			typename Container::size_type size()
			{
                boost::recursive_mutex::scoped_lock lk(mMutex);

				size_t result = mContainer.size();

				return(result);
			}

			T& front()
			{
                boost::recursive_mutex::scoped_lock lk(mMutex);

				T item = mContainer.front();

				return(item);
			}

		private:

			Container		mContainer;
			QueueMutex		mMutex;
	};
}

//======================================================================================================================

#endif

