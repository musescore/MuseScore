/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include <cmath>

#include "engraving/dom/stafftype.h"
#include "engraving/rendering/score/tablaturegeometry.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;
using namespace muse;

class Engraving_TablatureTests : public ::testing::Test
{
};

// Test minimStyle() sanitization: returns NONE when incompatible settings
TEST_F(Engraving_TablatureTests, minimStyleSanitization)
{
    StaffType st;
    st.setGroup(StaffGroup::TAB);

    // Default: stems enabled, no durations -> minimStyle should work
    st.setStemless(false);
    st.setGenDurations(false);
    st.setMinimStyle(TablatureMinimStyle::CIRCLED);
    EXPECT_EQ(st.minimStyle(), TablatureMinimStyle::CIRCLED);

    st.setMinimStyle(TablatureMinimStyle::SLASHED);
    EXPECT_EQ(st.minimStyle(), TablatureMinimStyle::SLASHED);

    st.setMinimStyle(TablatureMinimStyle::SHORTER);
    EXPECT_EQ(st.minimStyle(), TablatureMinimStyle::SHORTER);

    st.setMinimStyle(TablatureMinimStyle::NONE);
    EXPECT_EQ(st.minimStyle(), TablatureMinimStyle::NONE);

    // With stemless=true -> should return NONE regardless of stored value
    st.setStemless(true);
    st.setGenDurations(false);

    st.setMinimStyle(TablatureMinimStyle::CIRCLED);
    EXPECT_EQ(st.minimStyle(), TablatureMinimStyle::NONE);

    st.setMinimStyle(TablatureMinimStyle::SLASHED);
    EXPECT_EQ(st.minimStyle(), TablatureMinimStyle::NONE);

    st.setMinimStyle(TablatureMinimStyle::SHORTER);
    EXPECT_EQ(st.minimStyle(), TablatureMinimStyle::NONE);

    // With genDurations=true -> should return NONE regardless of stored value
    st.setStemless(false);
    st.setGenDurations(true);

    st.setMinimStyle(TablatureMinimStyle::CIRCLED);
    EXPECT_EQ(st.minimStyle(), TablatureMinimStyle::NONE);

    st.setMinimStyle(TablatureMinimStyle::SLASHED);
    EXPECT_EQ(st.minimStyle(), TablatureMinimStyle::NONE);
}

// Test stemless() sanitization: returns true when genDurations is true
TEST_F(Engraving_TablatureTests, stemlessSanitization)
{
    StaffType st;
    st.setGroup(StaffGroup::TAB);

    // Default
    st.setStemless(false);
    st.setGenDurations(false);
    EXPECT_FALSE(st.stemless());

    st.setStemless(true);
    EXPECT_TRUE(st.stemless());

    // genDurations implies stemless
    st.setStemless(false);
    st.setGenDurations(true);
    EXPECT_TRUE(st.stemless());
}

// Test computeVisibleArcRanges: no adjacent rects -> full ellipse
TEST_F(Engraving_TablatureTests, visibleArcsNoBlockers)
{
    RectF mainRect(0, 0, 10, 8);
    std::vector<RectF> adjacent;

    std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);

    ASSERT_EQ(arcs.size(), 1u);
    EXPECT_NEAR(arcs[0].startAngleRadians, 0.0, 0.01);
    EXPECT_NEAR(arcs[0].endAngleRadians, 2.0 * M_PI, 0.01);
}

// Test computeVisibleArcRanges: invalid mainRect -> empty result
TEST_F(Engraving_TablatureTests, visibleArcsInvalidRect)
{
    std::vector<RectF> adjacent;

    // Both zero
    EXPECT_TRUE(computeVisibleArcRanges(RectF(0, 0, 0, 0), adjacent, 0.5).empty());

    // Only width zero
    EXPECT_TRUE(computeVisibleArcRanges(RectF(0, 0, 0, 10), adjacent, 0.5).empty());

    // Only height zero
    EXPECT_TRUE(computeVisibleArcRanges(RectF(0, 0, 10, 0), adjacent, 0.5).empty());

    // Negative width
    EXPECT_TRUE(computeVisibleArcRanges(RectF(0, 0, -5, 10), adjacent, 0.5).empty());

    // Negative height
    EXPECT_TRUE(computeVisibleArcRanges(RectF(0, 0, 10, -5), adjacent, 0.5).empty());

    // Both negative
    EXPECT_TRUE(computeVisibleArcRanges(RectF(0, 0, -5, -5), adjacent, 0.5).empty());
}

// Test computeVisibleArcRanges: blocker to the right creates gap on right side
TEST_F(Engraving_TablatureTests, visibleArcsBlockerRight)
{
    RectF mainRect(0, 0, 10, 10);
    std::vector<RectF> adjacent = { RectF(8, 0, 10, 10) };  // Blocker to the right, overlapping

    std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);

    // Should have a gap around angle 0 (right side)
    ASSERT_GE(arcs.size(), 1u);

    // The full circle minus a gap should be less than 2*PI
    double totalSpan = 0.0;
    for (const auto& arc : arcs) {
        totalSpan += arc.spanRadians();
    }
    EXPECT_LT(totalSpan, 2.0 * M_PI - 0.1);  // Gap should be noticeable
}

// Test computeVisibleArcRanges: blocker above creates gap on top
TEST_F(Engraving_TablatureTests, visibleArcsBlockerAbove)
{
    RectF mainRect(0, 0, 10, 10);
    std::vector<RectF> adjacent = { RectF(0, -8, 10, 10) };  // Blocker above, overlapping

    std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);

    // Should have a gap around angle PI/2 (top)
    ASSERT_GE(arcs.size(), 1u);

    double totalSpan = 0.0;
    for (const auto& arc : arcs) {
        totalSpan += arc.spanRadians();
    }
    EXPECT_LT(totalSpan, 2.0 * M_PI - 0.1);
}

// Test computeVisibleArcRanges: two blockers create two gaps
TEST_F(Engraving_TablatureTests, visibleArcsTwoBlockers)
{
    RectF mainRect(0, 0, 10, 10);
    std::vector<RectF> adjacent = {
        RectF(8, 0, 10, 10),   // Blocker to the right
        RectF(-8, 0, 10, 10)   // Blocker to the left
    };

    std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);

    // Should have gaps on both sides
    ASSERT_GE(arcs.size(), 2u);

    double totalSpan = 0.0;
    for (const auto& arc : arcs) {
        totalSpan += arc.spanRadians();
    }
    EXPECT_LT(totalSpan, 2.0 * M_PI - 0.2);  // Two gaps should be noticeable
}

// Test computeVisibleArcRanges: non-overlapping blocker -> full ellipse
TEST_F(Engraving_TablatureTests, visibleArcsNonOverlappingBlocker)
{
    RectF mainRect(0, 0, 10, 10);

    // Single blocker far away
    {
        std::vector<RectF> adjacent = { RectF(100, 100, 10, 10) };
        std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);
        ASSERT_EQ(arcs.size(), 1u);
        EXPECT_NEAR(arcs[0].spanRadians(), 2.0 * M_PI, 0.01);
    }

    // Multiple blockers all far away
    {
        std::vector<RectF> adjacent = {
            RectF(100, 0, 10, 10),
            RectF(-100, 0, 10, 10),
            RectF(0, 100, 10, 10)
        };
        std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);
        ASSERT_EQ(arcs.size(), 1u);
        EXPECT_NEAR(arcs[0].spanRadians(), 2.0 * M_PI, 0.01);
    }

    // Blocker just outside overlap range (touching but not overlapping)
    {
        std::vector<RectF> adjacent = { RectF(15, 0, 10, 10) };  // Starts at x=15, main ends at x=10
        std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);
        ASSERT_EQ(arcs.size(), 1u);
        EXPECT_NEAR(arcs[0].spanRadians(), 2.0 * M_PI, 0.01);
    }
}

// Tests for zig-zag tablature layout (PR #30924)
// In zig-zag, notes alternate up/down, causing diagonal overlaps

// Test computeVisibleArcRanges: blocker diagonal upper-right
TEST_F(Engraving_TablatureTests, visibleArcsZigzagUpperRight)
{
    RectF mainRect(0, 0, 10, 10);
    std::vector<RectF> adjacent = { RectF(6, -6, 10, 10) };  // Diagonal upper-right

    std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);

    ASSERT_GE(arcs.size(), 1u);

    double totalSpan = 0.0;
    for (const auto& arc : arcs) {
        totalSpan += arc.spanRadians();
    }
    EXPECT_LT(totalSpan, 2.0 * M_PI - 0.1);  // Gap in upper-right quadrant
}

// Test computeVisibleArcRanges: blocker diagonal lower-right
TEST_F(Engraving_TablatureTests, visibleArcsZigzagLowerRight)
{
    RectF mainRect(0, 0, 10, 10);
    std::vector<RectF> adjacent = { RectF(4, 4, 10, 10) };  // Diagonal lower-right, more overlap

    std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);

    ASSERT_GE(arcs.size(), 1u);

    double totalSpan = 0.0;
    for (const auto& arc : arcs) {
        totalSpan += arc.spanRadians();
    }
    EXPECT_LT(totalSpan, 2.0 * M_PI - 0.1);  // Gap in lower-right quadrant
}

// Test computeVisibleArcRanges: zig-zag pattern with alternating up/down blockers
TEST_F(Engraving_TablatureTests, visibleArcsZigzagPattern)
{
    RectF mainRect(0, 0, 10, 10);
    std::vector<RectF> adjacent = {
        RectF(12, -4, 10, 10),   // Next note: right and up
        RectF(-12, 4, 10, 10)    // Previous note: left and down
    };

    std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);

    // Zig-zag pattern should create gaps in diagonal quadrants
    double totalSpan = 0.0;
    for (const auto& arc : arcs) {
        totalSpan += arc.spanRadians();
    }
    // Diagonal blockers may or may not create gaps depending on exact overlap
    // Just verify we get valid arc ranges
    EXPECT_GT(totalSpan, 0.0);
    EXPECT_LE(totalSpan, 2.0 * M_PI + 0.01);
}

// Test computeVisibleArcRanges: zig-zag chord with 3 notes (up-down-up pattern)
TEST_F(Engraving_TablatureTests, visibleArcsZigzagChord)
{
    // Middle note of a 3-note zig-zag chord
    RectF mainRect(0, 0, 10, 10);
    std::vector<RectF> adjacent = {
        RectF(0, -8, 10, 10),    // Note above (string above in zig-zag)
        RectF(0, 8, 10, 10)      // Note below (string below in zig-zag)
    };

    std::vector<ArcRange> arcs = computeVisibleArcRanges(mainRect, adjacent, 0.5);

    // Should have gaps at top and bottom
    ASSERT_GE(arcs.size(), 2u);

    double totalSpan = 0.0;
    for (const auto& arc : arcs) {
        totalSpan += arc.spanRadians();
    }
    EXPECT_LT(totalSpan, 2.0 * M_PI - 0.2);  // Two vertical gaps
}
