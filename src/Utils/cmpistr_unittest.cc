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

#include "Utils/utils.h"

 // Wrapping tests in an anonymous namespace prevents potential name conflicts
 namespace {

TEST(CmpistrTests, CanCompareTwoEqualStrings) {
    char string1[] = "test string";
    char string2[] = "test string";

    EXPECT_EQ(0, Anh_Utils::cmpistr(string1, string2));
}

TEST(CmpistrTests, CanCompareTwoSameStringsWithDifferentCases) {
    char string1[] = "test string";
    char string2[] = "TeST stRInG";

    EXPECT_EQ(0, Anh_Utils::cmpistr(string1, string2));
}

TEST(CmpnistrTests, CanCompareTwoEqualStrings) {
    char string1[] = "test string";
    char string2[] = "test string";

    EXPECT_EQ(0, Anh_Utils::cmpnistr(string1, string2, 11));
}

TEST(CmpnistrTests, CanCompareTwoSameStringsWithDifferentCases) {
    char string1[] = "test string";
    char string2[] = "TeST stRInG";

    EXPECT_EQ(0, Anh_Utils::cmpnistr(string1, string2, 11));
}

TEST(CmpnistrTests, CanCompareTwoDifferentLengthStringsWithDifferentCases) {
    char string1[] = "test string one is longer";
    char string2[] = "TeST stRInG";

    EXPECT_EQ(0, Anh_Utils::cmpnistr(string1, string2, 11));
}

}  // namespace
