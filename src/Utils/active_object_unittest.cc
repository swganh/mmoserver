/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "Utils/ActiveObject.h"

#include <gtest/gtest.h>
#include <boost/thread.hpp>

// Wrapping tests in an anonymous namespace prevents potential name conflicts
namespace {

/*! This class is an example implementation of the active object and displays
* how async
* calls (both with and without return values) should be handled.
*/
class ActiveObjectImpl {
public:
    ActiveObjectImpl() : called_(false) {}

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
    utils::ActiveObject active_obj_;
    bool called_;
};

/*! This test demonstrates that the asyncronous calls into an active object
* work correctly.
*/
TEST(ActiveObjectTests, CanMakeAsyncCallToHostObject) {
    ActiveObjectImpl active_object_impl;

    active_object_impl.SomeAsyncInteraction();
    boost::unique_future<bool> future = active_object_impl.called();

    // Make sure that the async operation occurred.
    EXPECT_EQ(true, future.get());
}

}  // namespace

