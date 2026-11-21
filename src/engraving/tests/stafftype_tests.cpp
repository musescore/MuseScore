/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <memory>

#include <gtest/gtest.h>

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/stafftype.h"

using namespace mu::engraving;

class Engraving_StaffTypeTests : public ::testing::Test
{
};

TEST_F(Engraving_StaffTypeTests, enablingDurationSymbolsMeasuresThem)
{
    std::unique_ptr<MasterScore> score(compat::ScoreAccess::createMasterScore(nullptr));
    StaffType tab(*StaffType::preset(StaffTypes::TAB_6COMMON));
    ASSERT_FALSE(tab.genDurations());
    tab.setScore(score.get());

    tab.setGenDurations(true);

    EXPECT_GT(tab.durationBoxH(), 0.0);
}
