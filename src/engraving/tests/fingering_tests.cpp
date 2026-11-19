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

#include <gtest/gtest.h>

#include "engraving/dom/chord.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String FINGERING_DATA_DIR("fingering_data/");

class Engraving_FingeringTests : public ::testing::Test
{
};

static std::vector<Fingering*> fingerings(MasterScore* score)
{
    std::vector<Fingering*> result;
    for (Segment* s = score->firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        EngravingItem* item = s->element(0);
        if (!item || !item->isChord()) {
            continue;
        }
        for (Note* note : toChord(item)->notes()) {
            for (EngravingItem* e : note->el()) {
                if (e->isFingering()) {
                    result.push_back(toFingering(e));
                }
            }
        }
    }
    return result;
}

// The fingerings above the staff end at the same height, and those below start at the same height
static void expectFingeringsLevel(MasterScore* score)
{
    std::vector<double> bottomsAbove;
    std::vector<double> topsBelow;
    for (const Fingering* f : fingerings(score)) {
        const RectF r = f->pageBoundingRect();
        if (f->placeAbove()) {
            bottomsAbove.push_back(r.bottom());
        } else {
            topsBelow.push_back(r.top());
        }
    }

    ASSERT_EQ(bottomsAbove.size(), 3u);
    ASSERT_EQ(topsBelow.size(), 3u);

    const double tolerance = 0.01 * score->style().spatium();
    for (double bottom : bottomsAbove) {
        EXPECT_NEAR(bottom, bottomsAbove.back(), tolerance);
    }
    for (double top : topsBelow) {
        EXPECT_NEAR(top, topsBelow.back(), tolerance);
    }
}

// b.1: beamed eighths below the staff, stems up, fingerings above on D4 and C4
// b.2: unbeamed B3, fingering above
// b.3: beamed eighths above the staff, stems down, fingerings below on B5 and C6
// b.4: unbeamed C6, fingering below
static const String BEAMED_STAFF_CLEARANCE_SCORE(FINGERING_DATA_DIR + u"beamedStaffClearance.mscx");

// Fingerings outside the staff keep the same distance from it, whether their notes are beamed or not
TEST_F(Engraving_FingeringTests, beamedStaffClearance)
{
    MasterScore* score = ScoreRW::readScore(BEAMED_STAFF_CLEARANCE_SCORE);
    ASSERT_TRUE(score);

    ASSERT_NO_FATAL_FAILURE(expectFingeringsLevel(score));

    delete score;
}

// The same with a custom minimum distance, including a negative one that takes the fingerings into the staff
TEST_F(Engraving_FingeringTests, beamedStaffClearanceCustomMinDistance)
{
    MasterScore* score = ScoreRW::readScore(BEAMED_STAFF_CLEARANCE_SCORE);
    ASSERT_TRUE(score);

    for (const Spatium minDistance : { 1.0_sp, 0.0_sp, -1.0_sp }) {
        for (Fingering* f : fingerings(score)) {
            f->setProperty(Pid::MIN_DISTANCE, minDistance);
            f->setPropertyFlags(Pid::MIN_DISTANCE, PropertyFlags::UNSTYLED);
        }
        score->doLayout();

        SCOPED_TRACE(minDistance.val());
        ASSERT_NO_FATAL_FAILURE(expectFingeringsLevel(score));
    }

    delete score;
}
