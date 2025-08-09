/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "dom/excerpt.h"
#include "dom/factory.h"
#include "dom/linkedobjects.h"
#include "dom/masterscore.h"
#include "dom/mcursor.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/segment.h"
#include "dom/undo.h"

#include "engraving/compat/scoreaccess.h"
#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String STAFF_MOVE_DIR(u"staffmove_data/");

class Engraving_StaffMoveTests : public ::testing::Test
{
};

TEST_F(Engraving_StaffMoveTests, hiddenStaff)
{
    MasterScore* score = ScoreRW::readScore(STAFF_MOVE_DIR + u"hiddenStaff.mscx");
    // Get chord
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure();
    EXPECT_TRUE(m);
    Segment* s = m->first(SegmentType::ChordRest);
    EXPECT_TRUE(s);
    Chord* c1 = toChord(s->element(4));
    EXPECT_TRUE(c1);
    Staff* staff = score->staff(0);

    // Move chord
    score->startCmd(TranslatableString::untranslatable("Engraving staff move tests"));
    score->moveUp(c1);
    score->endCmd();

    // Hide staff
    score->startCmd(TranslatableString::untranslatable("Engraving staff move tests"));
    score->undo(new mu::engraving::ChangeStaff(staff, false, staff->defaultClefType(), staff->userDist(), staff->cutaway(),
                                               staff->hideSystemBarLine(), staff->mergeMatchingRests(),
                                               staff->reflectTranspositionInLinkedTab()));
    score->endCmd();

    // Compare
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"hiddenStaff1.mscx", STAFF_MOVE_DIR + u"hiddenStaff-ref.mscx"));

    // Unhide staff
    score->startCmd(TranslatableString::untranslatable("Engraving staff move tests"));
    score->undo(new mu::engraving::ChangeStaff(staff, true, staff->defaultClefType(), staff->userDist(), staff->cutaway(),
                                               staff->hideSystemBarLine(), staff->mergeMatchingRests(),
                                               staff->reflectTranspositionInLinkedTab()));
    score->endCmd();

    // Move chord back
    score->startCmd(TranslatableString::untranslatable("Engraving staff move tests"));
    score->moveDown(c1);
    score->endCmd();

    // Compare
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"hiddenStaff2.mscx", STAFF_MOVE_DIR + u"hiddenStaff.mscx"));
}

TEST_F(Engraving_StaffMoveTests, linkedStaff)
{
    MasterScore* score = ScoreRW::readScore(STAFF_MOVE_DIR + u"linkedStaff.mscx");
    EXPECT_TRUE(score);
    Measure* m = score->firstMeasure();
    EXPECT_TRUE(m);
    Segment* s = m->first(SegmentType::ChordRest);
    EXPECT_TRUE(s);
    Chord* c1 = toChord(s->element(0));
    EXPECT_TRUE(c1);
    Chord* c2 = toChord(s->element(8));
    EXPECT_TRUE(c2);

    score->startCmd(TranslatableString::untranslatable("Engraving staff move tests"));
    score->moveDown(c1);
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"linkedStaff1.mscx", STAFF_MOVE_DIR + u"linkedStaff-ref.mscx"));

    // Try to move linked chord - should be no change
    score->startCmd(TranslatableString::untranslatable("Engraving staff move tests"));
    score->moveUp(c2);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"linkedStaff2.mscx", STAFF_MOVE_DIR + u"linkedStaff-ref.mscx"));

    score->startCmd(TranslatableString::untranslatable("Engraving staff move tests"));
    score->moveUp(c1);
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"linkedStaff3.mscx", STAFF_MOVE_DIR + u"linkedStaff.mscx"));
    delete score;
}
