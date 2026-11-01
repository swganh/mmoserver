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

#include "Utils/ActiveObject.h"

#ifdef _WIN32
#include <windows.h>
#endif

using boost::thread;

namespace utils {

ActiveObject::ActiveObject() : done_(false) {
    thread_ = std::move(thread([=] { this->Run(); }));

#ifdef _WIN32
    HANDLE mtheHandle = thread_.native_handle();
    SetPriorityClass(mtheHandle,REALTIME_PRIORITY_CLASS);
#endif
}

ActiveObject::~ActiveObject() {
    Send([&] { done_ = true; });
    thread_.join();
}

void ActiveObject::Send(Message message) {
    {
        // Push under the lock so the worker can't miss the notification between
        // checking the queue and going to sleep.
        boost::lock_guard<boost::mutex> lock(mutex_);
        message_queue_.push(std::move(message));
    }
    condition_.notify_one();
}

void ActiveObject::Run() {
    Message message;

    while (! done_) {
        {
            // The wait predicate must be free of side effects: it can be evaluated
            // more than once per wakeup, and popping inside it silently drops messages.
            boost::unique_lock<boost::mutex> lock(mutex_);
            condition_.wait(lock, [this] { return ! message_queue_.empty(); });
        }

        // Run the message without holding the lock so it may call Send() itself.
        if (message_queue_.try_pop(message)) {
            message();
        }
    }
}

}  // namespace utils
