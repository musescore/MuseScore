/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <gtest/gtest.h>

#include "draw/types/geometry.h"

using namespace muse;

class Global_Types_GeometryTests : public ::testing::Test
{
public:
};

TEST_F(Global_Types_GeometryTests, intersects) {
    RectX<double> r1 { 10.0, 10.0, 10.0, 10.0 };

    // above - no collision
    RectX<double> r2 { 10.0, 0.0, 5.0, 5.0 };
    EXPECT_FALSE(r1.intersects(r2));
    // above - collision
    r2 = RectX<double>(10.0, 8.0, 5.0, 5.0);
    EXPECT_TRUE(r1.intersects(r2));
    // above - touching
    r2 = RectX<double>(10.0, 0.0, 10.0, 10.0);
    EXPECT_FALSE(r1.intersects(r2));
    // above - touching (floating point case)
    r1 = RectX<double>(10.0, 10.0, 10.0000002, 10.0);
    r2 = RectX<double>(20.0000001, 10.0, 10.0, 10.0);
    EXPECT_FALSE(r1.intersects(r2));

    r1 = RectX<double>(10.0, 10.0, 10.0, 10.0);
    // adjacent - no collision
    r2 = RectX<double>(25.0, 10.0, 10.0, 10.0);
    EXPECT_FALSE(r1.intersects(r2));
    // adjacent - collision
    r2 = RectX<double>(17.0, 10.0, 10.0, 10.0);
    EXPECT_TRUE(r1.intersects(r2));
    // adjacent - touching
    r2 = RectX<double>(20.0, 10.0, 10.0, 10.0);
    EXPECT_FALSE(r1.intersects(r2));

    r1 = RectX<double>(10.0, 10.0, 10.0000002, 10.0);

    // adjacent - touching (floating point case)
    r2 = RectX<double>(20.0000001, 10.0, 10.0, 10.0);
    EXPECT_FALSE(r1.intersects(r2));
}
