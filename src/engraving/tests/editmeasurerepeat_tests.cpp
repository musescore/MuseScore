/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
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
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"

#include "engraving/editing/editmeasurerepeat.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String PARTS_DATA_DIR("parts_data/");

//! Adding a measure repeat replaces the contents of the measures without asking,
//! so it tells beforehand whether it would, for the user to be asked
class Engraving_EditMeasureRepeatTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        //! The first measure has notes and a quarter rest, the next ones are empty
        m_score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-empty-parts.mscx");
        ASSERT_TRUE(m_score);

        m_withNotes = m_score->firstMeasure();
        m_empty = m_withNotes->nextMeasure();
    }

    void TearDown() override
    {
        delete m_score;
    }

    EngravingItem* firstItem(Measure* measure, staff_idx_t staffIdx) const
    {
        Segment* segment = measure->first(SegmentType::ChordRest);
        return segment ? segment->element(staff2track(staffIdx)) : nullptr;
    }

    MasterScore* m_score = nullptr;
    Measure* m_withNotes = nullptr;
    Measure* m_empty = nullptr;
};

TEST_F(Engraving_EditMeasureRepeatTests, Adding_EmptyMeasures_ReplacesNothing)
{
    EXPECT_FALSE(EditMeasureRepeat::addingReplacesContents(m_empty, 2, 0));
}

TEST_F(Engraving_EditMeasureRepeatTests, Adding_MeasureWithNotes_ReplacesThem)
{
    EXPECT_TRUE(EditMeasureRepeat::addingReplacesContents(m_withNotes, 1, 0));
}

TEST_F(Engraving_EditMeasureRepeatTests, Adding_GroupReachingTheNotes_ReplacesThem)
{
    //! [GIVEN] A group of empty measures and the one with notes...
    //! [THEN] ...replaces the notes
    EXPECT_TRUE(EditMeasureRepeat::addingReplacesContents(m_withNotes, 2, 0));
}

TEST_F(Engraving_EditMeasureRepeatTests, Adding_NotEnoughMeasures_ReplacesNothing)
{
    //! [GIVEN] More measures than the score has: the measure repeat is not added at all
    EXPECT_FALSE(EditMeasureRepeat::addingReplacesContents(m_score->lastMeasure(), 2, 0));
}

TEST_F(Engraving_EditMeasureRepeatTests, Dropping_OntoMeasureWithNotes_ReplacesThem)
{
    EXPECT_TRUE(EditMeasureRepeat::droppingReplacesContents(m_withNotes, staff2track(0), 1));

    //! [THEN] ...but not onto an empty measure, nor without a staff
    EXPECT_FALSE(EditMeasureRepeat::droppingReplacesContents(m_empty, staff2track(0), 1));
    EXPECT_FALSE(EditMeasureRepeat::droppingReplacesContents(m_withNotes, muse::nidx, 1));
}

TEST_F(Engraving_EditMeasureRepeatTests, Dropping_OntoMeasureRest_DependsOnTheGroup)
{
    EngravingItem* measureRest = firstItem(m_empty, 0);
    ASSERT_TRUE(measureRest && measureRest->isRest());

    //! [THEN] The empty measures are not replaced
    EXPECT_FALSE(EditMeasureRepeat::droppingReplacesContents(measureRest, measureRest->track(), 2));
}

TEST_F(Engraving_EditMeasureRepeatTests, Dropping_OntoWhatDoesNotTakeIt_ReplacesNothing)
{
    //! [GIVEN] A chord, and a quarter rest: a measure repeat is not dropped onto them
    EngravingItem* chord = firstItem(m_withNotes, 0);
    ASSERT_TRUE(chord && chord->isChord());

    Segment* lastSegment = m_withNotes->last(SegmentType::ChordRest);
    ASSERT_TRUE(lastSegment);
    EngravingItem* quarterRest = lastSegment->element(0);
    ASSERT_TRUE(quarterRest && quarterRest->isRest());

    EXPECT_FALSE(EditMeasureRepeat::droppingReplacesContents(chord, chord->track(), 1));
    EXPECT_FALSE(EditMeasureRepeat::droppingReplacesContents(quarterRest, quarterRest->track(), 1));
    EXPECT_FALSE(EditMeasureRepeat::droppingReplacesContents(nullptr, staff2track(0), 1));
}
