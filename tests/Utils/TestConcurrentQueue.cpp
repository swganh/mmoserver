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

#include <string>
#include <gtest/gtest.h>
#include "Utils/ConcurrentQueue.h"

using ::utils::ConcurrentQueue;

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
