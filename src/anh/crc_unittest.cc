/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2010 The SWG:ANH Team

 MMOServer is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 MMOServer is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with MMOServer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>

#include "anh/crc.h"

using anh::memcrc;

/// This test shows how to find the 32bit checksum of a c-style string.
TEST(CrcTest, CanCrcCstyleStrings) {
    EXPECT_EQ(0x338BCFAC, memcrc("test"));
    EXPECT_EQ(0x2643D57C, memcrc("anothertest"));
    EXPECT_EQ(0x19522193, memcrc("aThirdTest"));
}

/// This test shows how to find the 32bit checksum of a std::string.
TEST(CrcTest, CanCrcStdStrings) {
    EXPECT_EQ(0x338BCFAC, memcrc(std::string("test")));
    EXPECT_EQ(0x2643D57C, memcrc(std::string("anothertest")));
    EXPECT_EQ(0x19522193, memcrc(std::string("aThirdTest")));
}
