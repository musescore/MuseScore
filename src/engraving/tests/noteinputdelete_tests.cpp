/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "dom/chord.h"
#include "dom/chordrest.h"
#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/mscore.h"
#include "dom/note.h"
#include "dom/segment.h"

#include "engraving/compat/scoreaccess.h"

using namespace mu;
using namespace mu::engraving;

static const String NOTE_DATA_DIR("note_data/");

class Engraving_NoteInputDeleteTests : public ::testing::Test
{
};

//---------------------------------------------------------
///   note
///   read/write test of note
//---------------------------------------------------------

TEST_F(Engraving_NoteInputDeleteTests, note)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore();
    score->setNoteEntryMode(true);

    const size_t NUM_NOTES = 4;
    track_idx_t track;

    // Insert notes in note input mode
    for (size_t i = 0; i < NUM_NOTES; ++i) {
        Chord* c = Factory::createChord(score->dummy()->segment());
        track = c->track();
        Note* note = Factory::createNote(c);
        c->add(note);
        note->setPitch(72 + i);
    }

    // Consecutively delete all notes
    for (size_t i = 0; i < NUM_NOTES; ++i) {
        score->cmdDeleteSelection();
    }

    // Check all notes were deleted
    for (size_t i = 0; i < NUM_NOTES; ++i) {
        Fraction tick = { (int)(i + 1), 4 };
        ChordRest* cr = score->findCR(tick, track);
        EXPECT_FALSE(cr);  //nullptr
    }

    delete score;
}
