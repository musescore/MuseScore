/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/dom/system.h"
#include "engraving/dom/systemdivider.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"
#include "utils/testutils.h"

using namespace mu::engraving;

static const String SYSTEM_LOCKS_DATA_DIR("system_dividers_data/");

class Engraving_SystemLocksTests : public ::testing::Test
{
};

TEST_F(Engraving_SystemLocksTests, createAndSaveLocks)
{
    MasterScore* score = ScoreRW::readScore(SYSTEM_LOCKS_DATA_DIR + u"systemDividers-1.mscx");
    EXPECT_TRUE(score);

    // Enable system dividers
    score->startCmd(TranslatableString::untranslatable("Engraving system divider test"));
    score->undoChangeStyleValues({ { Sid::dividerLeft, true }, { Sid::dividerRight, true } });
    score->endCmd();

    EXPECT_TRUE(score->systemDividers().size());

    System* firstSystem = score->systems().front();
    EXPECT_TRUE(firstSystem);

    // Check system dividers has been created
    SystemDivider* dividerLeft = firstSystem->systemDividerLeft();
    EXPECT_TRUE(dividerLeft);
    SystemDivider* dividerRight = firstSystem->systemDividerRight();
    EXPECT_TRUE(dividerRight);

    // Make manual edits to system dividers so they get written to file
    score->startCmd(TranslatableString::untranslatable("Edit divider"));
    dividerLeft->undoChangeProperty(Pid::VISIBLE, false);
    dividerRight->undoChangeProperty(Pid::OFFSET, PointF(100, 100));
    score->endCmd();

    ScoreComp::saveCompareScore(score, String("dividers.mscx"),
                                ScoreRW::rootPath() + u"/" + SYSTEM_LOCKS_DATA_DIR + u"systemDividers-1-ref.mscx");

    delete score;
}
