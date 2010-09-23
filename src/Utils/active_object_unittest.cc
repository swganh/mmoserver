// Copyright (c) 2010 ApathyStudios. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the COPYING file.

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

