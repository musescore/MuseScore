/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "libmscore/shape.h"

using namespace Ms;
using namespace mu;

class ShapesTests : public ::testing::Test
{
};

TEST_F(ShapesTests, minDistance)
{
    Shape a;
    Shape b;

    a.add(RectF(-10, -10, 20, 20));
    qreal d = a.minHorizontalDistance(b); // b is empty
    EXPECT_EQ(d, 0.0);

    b.add(RectF(0, 0, 10, 10));
    d = a.minHorizontalDistance(b);
    EXPECT_EQ(d, 10.0);

    d = a.minVerticalDistance(b);
    EXPECT_EQ(d, 10.0);
}
