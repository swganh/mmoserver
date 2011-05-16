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
