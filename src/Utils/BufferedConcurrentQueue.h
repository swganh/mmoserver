/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#ifndef SRC_UTILS_BUFFEREDCONCURRENTQUEUE_H_
#define SRC_UTILS_BUFFEREDCONCURRENTQUEUE_H_

#include <tbb/atomic.h>
#include "utils/ConcurrentQueue.h"

namespace utils {

/**
 * ConcurrentQueue is a multi-producer, multi-consumer queue intended for use across multiple threads.
 *
 * This code is an implementation of a design discussed in a series of concept articles by Herb Sutter
 * on developing a concurrent queue that uses light weight spin-locks at the head and tail of the queue
 * to keep it concurrent without resulting to context switching.
 *
 * @see http://www.drdobbs.com/high-performance-computing/212201163
 */


template <typename T>
class BufferedConcurrentQueue {
public:

	typedef ConcurrentQueue<T>*			queue;


	BufferedConcurrentQueue ()	{
		rolloverQueueHandle = 1;
		windowQueueHandle	= 2;

		bufferedQueue[rolloverQueueHandle]	= new ConcurrentQueue<T>;
		bufferedQueue[windowQueueHandle]	= new ConcurrentQueue<T>;
	}

	bool	WindowFilled ()	{
		return bufferedQueue[WindowQueueHandle]->filled();
	}

	bool	RolloverFilled ()	{
		return bufferedQueue[RolloverQueueHandle]->filled();
	}

	bool	Rolloverfront(T& t) {
		return bufferedQueue[RolloverQueueHandle]->front(t);
	}

	bool	Windowfront(T& t) {
		return bufferedQueue[WindowQueueHandle]->front(t);
	}

	void	handleRollover()	{
		rolloverQueueHandle = rolloverQueueHandle^3;
		windowQueueHandle	= windowQueueHandle^3;
	}

	bool Windowpop(T& t) {
		return bufferedQueue[WindowQueueHandle]->pop(t);
	}

	bool Rolloverpop(T& t) {
		return bufferedQueue[RolloverQueueHandle]->pop(t);
	}

	void Rolloverpush(const T& t) {
		bufferedQueue[RolloverQueueHandle]->push(t);
	}

	void Windowpush(const T& t) {
		bufferedQueue[WindowQueueHandle]->push(t);
	}

	~BufferedConcurrentQueue ()	{
	}
private:

	tbb::atomic<int>			windowQueueHandle;
	tbb::atomic<int>			rolloverQueueHandle;

	tbb::atomic<queue>			bufferedQueue[2];
};

template <typename T>
class BufferedConcurrentQueueLight {
public:
  
	BufferedConcurrentQueueLight ()	{
	}

	~BufferedConcurrentQueueLight ()	{
	}

private:
  
};


}  // namespace utils

#endif  // SRC_UTILS_CONCURRENTQUEUE_H_
