// Copyright (c) 2010 ApathyStudios. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the COPYING file.

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
