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

#include <gtest/gtest.h>

#include "Common/Crc.h"

/// This test shows how to find the 32bit checksum of a c-style string.
TEST(CrcTests, CanCrcCstyleStrings) {
    EXPECT_EQ(0x338BCFAC, ::common::memcrc("test"));
    EXPECT_EQ(0x2643D57C, ::common::memcrc("anothertest"));
    EXPECT_EQ(0x19522193, ::common::memcrc("aThirdTest"));
}

/// This test shows how to find the 32bit checksum of a std::string.
TEST(CrcTests, CanCrcStdStrings) {
    EXPECT_EQ(0x338BCFAC, ::common::memcrc(std::string("test")));
    EXPECT_EQ(0x2643D57C, ::common::memcrc(std::string("anothertest")));
    EXPECT_EQ(0x19522193, ::common::memcrc(std::string("aThirdTest")));
}
