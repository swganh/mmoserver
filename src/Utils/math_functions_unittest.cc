// Copyright (c) 2010 ApathyStudios. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the COPYING file.

#include "Utils/MathFunctions.h"

#include <gtest/gtest.h>

// Wrapping tests in an anonymous namespace prevents potential name conflicts
namespace {
    
TEST(MathFunctionsTests, CanCheckIfPointIsInRectangleBoundries) {
    // Load up some initial data
    glm::vec2 rec_center(-5160.0f, 4380.0f);
    glm::vec2 player_pos1(-4000.0f, 4000.0f);
    glm::vec2 player_pos2(-6500.0f, 3000.0f);

    float width = 2500.0f;
    float height = 1500.0f;

    // This point should be inside the rectangle.
    EXPECT_TRUE(IsPointInRectangle(player_pos1, rec_center, width, height));

    // This point should not be inside the rectangle.
    EXPECT_FALSE(IsPointInRectangle(player_pos2, rec_center, width, height));
}

TEST(MathFunctionsTests, CanCheckIfPointIsInCircle) {
    glm::vec2 center(-1204.0f, -3692.0f);
    EXPECT_TRUE(IsPointInCircle(glm::vec2(-1204.0f, -3692.0f), center, 375.0f));
    EXPECT_TRUE(IsPointInCircle(glm::vec2(-1204.0f + 375.0f, -3692.0f), center, 375.0f));
    EXPECT_FALSE(IsPointInCircle(glm::vec2(-1373.0f, -3322.0f), center, 375.0f));  // 407 m away
}

TEST(MathFunctionsTests, RegionWithNoWidthIsACircleWithTheHeightAsDiameter) {
    glm::vec2 center(-1204.0f, -3692.0f);
    // 750 high: radius 375.
    EXPECT_TRUE(IsPointInRegion(glm::vec2(-1204.0f, -3692.0f + 370.0f), center, 0.0f, 750.0f));
    EXPECT_FALSE(IsPointInRegion(glm::vec2(-1204.0f, -3692.0f + 380.0f), center, 0.0f, 750.0f));
    // Inside the bounding square but outside the circle (a corner).
    EXPECT_FALSE(IsPointInRegion(glm::vec2(-1204.0f + 300.0f, -3692.0f + 300.0f), center, 0.0f, 750.0f));
}

TEST(MathFunctionsTests, RegionWithAWidthIsARectangleAroundItsCenter) {
    // The Lars homestead badge: 100 x 100 around the homestead.
    glm::vec2 center(-2579.0f, -5500.0f);
    EXPECT_TRUE(IsPointInRegion(glm::vec2(-2579.0f, -5500.0f), center, 100.0f, 100.0f));
    EXPECT_TRUE(IsPointInRegion(glm::vec2(-2620.0f, -5540.0f), center, 100.0f, 100.0f));
    EXPECT_FALSE(IsPointInRegion(glm::vec2(-2479.0f, -5400.0f), center, 100.0f, 100.0f));
    // Width is along x, height along z.
    EXPECT_TRUE(IsPointInRegion(glm::vec2(-2579.0f + 90.0f, -5500.0f), center, 200.0f, 20.0f));
    EXPECT_FALSE(IsPointInRegion(glm::vec2(-2579.0f, -5500.0f + 90.0f), center, 200.0f, 20.0f));
}

TEST(MathFunctionsTests, RegionBoundsSurroundTheCenter) {
    glm::vec2 lower_left, upper_right;
    GetRegionBounds(glm::vec2(100.0f, -200.0f), 0.0f, 750.0f, lower_left, upper_right);
    EXPECT_FLOAT_EQ(-275.0f, lower_left.x);
    EXPECT_FLOAT_EQ(-575.0f, lower_left.y);
    EXPECT_FLOAT_EQ(475.0f, upper_right.x);
    EXPECT_FLOAT_EQ(175.0f, upper_right.y);

    GetRegionBounds(glm::vec2(0.0f, 0.0f), 200.0f, 20.0f, lower_left, upper_right);
    EXPECT_FLOAT_EQ(-100.0f, lower_left.x);
    EXPECT_FLOAT_EQ(-10.0f, lower_left.y);
    EXPECT_FLOAT_EQ(100.0f, upper_right.x);
    EXPECT_FLOAT_EQ(10.0f, upper_right.y);
}

}  // namespace
