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

}  // namespace
