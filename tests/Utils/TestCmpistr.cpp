/*! SWGANH MMOServer - Tests
 *
 * @copyright Copyright (c) 2006-2009 The swgANH Team
 */

#include <gtest/gtest.h>

#include "Utils/utils.h"

TEST(CmpistrTests, CanCompareTwoEqualStrings)
{
	char string1[] = "test string";
	char string2[] = "test string";

	EXPECT_EQ(0, Anh_Utils::cmpistr(string1, string2));
}

TEST(CmpistrTests, CanCompareTwoSameStringsWithDifferentCases)
{
	char string1[] = "test string";
	char string2[] = "TeST stRInG";

	EXPECT_EQ(0, Anh_Utils::cmpistr(string1, string2));
}

TEST(CmpnistrTests, CanCompareTwoEqualStrings)
{
	char string1[] = "test string";
	char string2[] = "test string";

	EXPECT_EQ(0, Anh_Utils::cmpnistr(string1, string2, 11));
}

TEST(CmpnistrTests, CanCompareTwoSameStringsWithDifferentCases)
{
	char string1[] = "test string";
	char string2[] = "TeST stRInG";

	EXPECT_EQ(0, Anh_Utils::cmpnistr(string1, string2, 11));
}

TEST(CmpnistrTests, CanCompareTwoDifferentLengthStringsWithDifferentCases)
{
	char string1[] = "test string one is longer";
	char string2[] = "TeST stRInG";

	EXPECT_EQ(0, Anh_Utils::cmpnistr(string1, string2, 11));
}
