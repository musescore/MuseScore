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

#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/segment.h"

#include "engraving/editing/editmeasurerepeat.h"
#include "engraving/editing/noteinput.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String PARTS_DATA_DIR("parts_data/");

//! The note input entry points delete a measure repeat without asking,
//! so they tell beforehand whether they would, for the user to be asked
class Engraving_NoteInputMeasureRepeatTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_score = ScoreRW::readScore(PARTS_DATA_DIR + u"part-empty-parts.mscx");
        ASSERT_TRUE(m_score);

        //! A 2-measure repeat in the second and the third measures of the first staff
        m_repeatMeasure = m_score->firstMeasure()->nextMeasure();
        m_score->startCmd(TranslatableString::untranslatable("Note input measure repeat tests"));
        EditMeasureRepeat::addMeasureRepeat(tx(), m_score, m_repeatMeasure, 2, 0);
        m_score->endCmd();
        ASSERT_TRUE(m_repeatMeasure->isMeasureRepeatGroup(0));

        m_score->inputState().setDuration(DurationType::V_QUARTER);
        m_score->inputState().setNoteEntryMode(true);
    }

    void TearDown() override
    {
        delete m_score;
    }

    Transaction& tx()
    {
        return m_score->transactionManager()->currentOrDummyTransaction();
    }

    void inputAt(Measure* measure, staff_idx_t staffIdx)
    {
        m_score->inputState().setTrack(staffIdx * VOICES);
        m_score->inputState().setSegment(measure->first(SegmentType::ChordRest));
    }

    MasterScore* m_score = nullptr;
    Measure* m_repeatMeasure = nullptr;
};

TEST_F(Engraving_NoteInputMeasureRepeatTests, AddPitch_InMeasureRepeat_RemovesIt)
{
    //! [GIVEN] The input position is in the measure repeat
    inputAt(m_repeatMeasure, 0);

    //! [THEN] Entering a note there removes it
    EXPECT_TRUE(NoteInput::addPitchRemovesMeasureRepeat(m_score, false));
    EXPECT_TRUE(NoteInput::addFretRemovesMeasureRepeat(m_score));
}

TEST_F(Engraving_NoteInputMeasureRepeatTests, AddPitch_BeforeMeasureRepeat_DoesNotRemoveIt)
{
    //! [GIVEN] The input position is in the measure before the measure repeat
    inputAt(m_score->firstMeasure(), 0);

    //! [THEN] Entering a note there does not remove it
    EXPECT_FALSE(NoteInput::addPitchRemovesMeasureRepeat(m_score, false));
    EXPECT_FALSE(NoteInput::addFretRemovesMeasureRepeat(m_score));
}

TEST_F(Engraving_NoteInputMeasureRepeatTests, AddPitch_OnAnotherStaff_DoesNotRemoveIt)
{
    //! [GIVEN] The input position is in the same measure, but on a staff without the measure repeat
    inputAt(m_repeatMeasure, 1);

    //! [THEN] Entering a note there does not remove it
    EXPECT_FALSE(NoteInput::addPitchRemovesMeasureRepeat(m_score, false));
    EXPECT_FALSE(NoteInput::addFretRemovesMeasureRepeat(m_score));
}

TEST_F(Engraving_NoteInputMeasureRepeatTests, AddPitch_Repitch_DoesNotRemoveIt)
{
    //! [GIVEN] The input position is in the measure repeat, but the notes are repitched there
    inputAt(m_repeatMeasure, 0);
    m_score->inputState().setNoteEntryMethod(NoteEntryMethod::REPITCH);

    //! [THEN] Repitching does not remove it
    EXPECT_FALSE(NoteInput::addPitchRemovesMeasureRepeat(m_score, false));
}

TEST_F(Engraving_NoteInputMeasureRepeatTests, PutNote_ClickInMeasureRepeat_RemovesIt)
{
    //! [GIVEN] A click on the measure repeat
    m_score->doLayout();
    const MeasureRepeat* repeat = m_repeatMeasure->nextMeasure()->measureRepeatElement(0);
    if (!repeat) {
        repeat = m_repeatMeasure->measureRepeatElement(0);
    }
    ASSERT_TRUE(repeat);
    const PointF click = repeat->canvasBoundingRect().center();

    //! [THEN] Putting a note there removes it
    EXPECT_TRUE(NoteInput::putNoteRemovesMeasureRepeat(m_score, click));

    //! [THEN] ...unless the notes are repitched
    m_score->inputState().setNoteEntryMethod(NoteEntryMethod::REPITCH);
    EXPECT_FALSE(NoteInput::putNoteRemovesMeasureRepeat(m_score, click));
}

TEST_F(Engraving_NoteInputMeasureRepeatTests, PutNote_ClickBeforeMeasureRepeat_DoesNotRemoveIt)
{
    //! [GIVEN] A click in the measure before the measure repeat, on the same staff
    m_score->doLayout();
    const Segment* segment = m_score->firstMeasure()->first(SegmentType::ChordRest);
    ASSERT_TRUE(segment);
    const EngravingItem* rest = segment->element(0);
    ASSERT_TRUE(rest);
    const PointF click = rest->canvasBoundingRect().center();

    //! [THEN] Putting a note there does not remove it
    EXPECT_FALSE(NoteInput::putNoteRemovesMeasureRepeat(m_score, click));
}
