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

#include "engraving/dom/accidental.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/score.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/tremolosinglechord.h"
#include "utils/scorerw.h"

using namespace mu::engraving;

static const String LV_DATA_DIR("lv_data/");

class Engraving_LVTests : public ::testing::Test
{
};

//---------------------------------------------------------
///    LV_Double_Notehead_test
///     Put a note
///     Add a Laissez-Vibrer tie to it
///     Put another note below it, making a chord
///     Verifies that the new note doesn't have any notes tied it, as would happen if it had a double notehead.
//---------------------------------------------------------

TEST_F(Engraving_LVTests, LV_Double_Notehead_test)
{
    MasterScore* score = ScoreRW::readScore(LV_DATA_DIR + u"empty.mscx");

    score->doLayout();

    score->inputState().setTrack(0);
    score->inputState().setSegment(score->tick2segment(Fraction(0, 1), false, SegmentType::ChordRest));
    score->inputState().setDuration(DurationType::V_QUARTER);
    score->inputState().setNoteEntryMode(true);

    // Add the First Note
    score->cmdAddPitch(45, true, false);

    Measure* m = score->firstMeasure();
    Chord* c = m->findChord(Fraction(0, 1), 0);
    EXPECT_EQ(c->ticks(), Fraction(1, 4));

    // Add a Laissez-Vibrer tie to the First Note
    score->select(c->upNote());
    score->cmdToggleLaissezVib();

    EXPECT_TRUE(c->upNote()->tieFor()->isLaissezVib());
    score->doLayout();

    // Add a Second Note
    NoteVal newNoteVal(71);
    Note* second_note = score->addPitch(newNoteVal, true);

    // Check if there is a double notehead
    std::vector<Note*> tn = second_note->tiedNotes();
    EXPECT_TRUE(tn.size() < 2);

    const String savePath = u"lv_test.mscx";
    ScoreRW::saveScore(score, savePath);
}
