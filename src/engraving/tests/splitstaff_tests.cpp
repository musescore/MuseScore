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

#include "dom/masterscore.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String SPLITSTAFF_DATA_DIR(u"splitstaff_data/");

class Engraving_SplitStaffTests : public ::testing::Test
{
public:
    void splitstaff(int, int);
};

void Engraving_SplitStaffTests::splitstaff(int idx, int staffIdx)
{
    MasterScore* score = ScoreRW::readScore(SPLITSTAFF_DATA_DIR + String(u"splitstaff0%1.mscx").arg(idx));
    EXPECT_TRUE(score);

    score->startCmd(TranslatableString::untranslatable("Engraving split staff tests"));
    score->splitStaff(staffIdx, 60);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, String(u"splitstaff0%1.mscx").arg(idx),
                                            SPLITSTAFF_DATA_DIR + String(u"splitstaff0%1-ref.mscx").arg(idx)));
    delete score;
}

TEST_F(Engraving_SplitStaffTests, splitstaff01)
{
    splitstaff(1, 0); //single notes
}

TEST_F(Engraving_SplitStaffTests, splitstaff02)
{
    splitstaff(2, 0); //chord
}

TEST_F(Engraving_SplitStaffTests, splitstaff03)
{
    splitstaff(3, 1); //non-top staff
}

TEST_F(Engraving_SplitStaffTests, splitstaff04)
{
    splitstaff(4, 0); //slur up
}

TEST_F(Engraving_SplitStaffTests, splitstaff05)
{
    splitstaff(5, 0); //slur down
}

TEST_F(Engraving_SplitStaffTests, splitstaff06)
{
    splitstaff(6, 0); //tuplet
}
