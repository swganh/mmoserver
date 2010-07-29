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

#ifndef TESTS_UTILS_MOCKOBJECTS_MOCKACTIVEOBJECTIMPL_H_
#define TESTS_UTILS_MOCKOBJECTS_MOCKACTIVEOBJECTIMPL_H_

#include <boost/thread.hpp>

#include "Utils/ActiveObject.h"

class MockActiveObjectImpl {
public:
    MockActiveObjectImpl() : called_(false) {}

    // Asyncronously set the called_ value to true. Use atomic operations
    // for setting the value because we also read this value from other threads.
    void SomeAsyncInteraction() { active_obj_.Send([=] {
            called_ = true;
    } ); }

    boost::unique_future<bool> called() { 
        // Create a packaged task for retrieving the value.
        std::shared_ptr<boost::packaged_task<bool>> task = std::make_shared<boost::packaged_task<bool>>([=] {   
            return called_;
        } );

        // Add the message to the active object's queue that runs the task which in turn
        // updates the future.
        active_obj_.Send([task] {
            (*task)();
        });

        // Return the future to the caller.
        return task->get_future();
    }

private:
    ::utils::ActiveObject active_obj_;
    bool called_;
};

#endif  // TESTS_UTILS_MOCKOBJECTS_MOCKACTIVEOBJECTIMPL_H_