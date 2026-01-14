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

#include "engraving/dom/stafftype.h"

using namespace mu::engraving;

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
