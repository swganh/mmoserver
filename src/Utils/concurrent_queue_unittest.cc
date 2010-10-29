// Copyright (c) 2010 ApathyStudios. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the COPYING file.

#include "Utils/ConcurrentQueue.h"

#include <string>
#include <gtest/gtest.h>

// Wrapping tests in an anonymous namespace prevents potential name conflicts
namespace {

using utils::ConcurrentQueue;
    
TEST(ConcurrentQueueTests, CanPushAndPopItem) {
    std::string test_string("test_string");

    ConcurrentQueue<std::string> string_queue;

    string_queue.push(test_string);

    std::string out_string;
    
    bool result = string_queue.pop(out_string);

    EXPECT_EQ(true, result);
    EXPECT_EQ(test_string, out_string);
}

TEST(ConcurrentQueueTests, CanPushAndPopMultipleItems) {
    std::string test_string1("test_string1");
    std::string test_string2("test_string2");
    std::string test_string3("test_string3");
    
    ConcurrentQueue<std::string> string_queue;

    string_queue.push(test_string1);
    string_queue.push(test_string2);
    string_queue.push(test_string3);

    std::string out_string;    
    bool result = string_queue.pop(out_string);

    EXPECT_EQ(true, result);
    EXPECT_EQ(test_string1, out_string);

    result = string_queue.pop(out_string);

    EXPECT_EQ(true, result);
    EXPECT_EQ(test_string2, out_string);
    
    result = string_queue.pop(out_string);

    EXPECT_EQ(true, result);
    EXPECT_EQ(test_string3, out_string);
}

TEST(ConcurrentQueueTests, CanPushAfterPop) {   
    std::string test_string1("test_string1");
    std::string test_string2("test_string2");
    
    ConcurrentQueue<std::string> string_queue;

    string_queue.push(test_string1);

    std::string out_string;    
    bool result = string_queue.pop(out_string);

    EXPECT_EQ(true, result);
    EXPECT_EQ(test_string1, out_string);
    
    string_queue.push(test_string2);

    result = string_queue.pop(out_string);

    EXPECT_EQ(true, result);
    EXPECT_EQ(test_string2, out_string);
}

}  // namespace
