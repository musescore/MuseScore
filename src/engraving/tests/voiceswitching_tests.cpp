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

#include "dom/chord.h"
#include "dom/note.h"

#include "utils/scorerw.h"

using namespace mu;
using namespace mu::engraving;

static const String VOICESWITCHING_DATA_DIR("voiceswitching_data/");

class Engraving_VoiceSwitchingTests : public ::testing::Test
{
};

/**
 * @brief Engraving_VoiceSwitchingTests_voiceSwitching
 * @details Each measure in this test consists of two voices, a "plus" notehead, and a "cross" notehead. When the "plus" notehead is
 *          selected and the voice is changed - we expect the input state to "jump" to the segment of the "cross" notehead (or not to
 *          jump at all in some cases). This test is mainly designed to cover to mid-tuplet voice switching (see InputState::setVoice)
 */
TEST_F(Engraving_VoiceSwitchingTests, voiceSwitching)
{
    Score* score = ScoreRW::readScore(VOICESWITCHING_DATA_DIR + "voiceswitching.mscx");
    EXPECT_TRUE(score);

    InputState& inputState = score->inputState();
    score->doLayout();

    for (Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        score->deselectAll();

        ChordRest* startCR = nullptr;
        ChordRest* destinationCR = nullptr;

        // Setup: Find the startCR (HEAD_PLUS) and destinationCR (HEAD_CROSS) for each measure...
        for (Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
            for (track_idx_t track = 0; track < 2; ++track) { // Search both voices...
                mu::engraving::EngravingItem* element = segment->element(track);
                if (!element || !element->isChord()) {
                    continue;
                }

                Chord* chord = toChord(element);
                if (chord->notes().empty()) {
                    continue;
                }

                for (Note* note : chord->notes()) {
                    ChordRest* cr = toChord(note->chord());
                    if (note->headGroup() == NoteHeadGroup::HEAD_PLUS) {
                        startCR = cr;
                    } else if (note->headGroup() == NoteHeadGroup::HEAD_CROSS) {
                        destinationCR = cr;
                    }
                }
            }
        }

        //! [GIVEN] A starting ChordRest, and an expected destination ChordRest...
        EXPECT_TRUE(startCR && destinationCR);

        //! [WHEN] The starting ChordRest is selected...
        score->select(startCR);

        //! [THEN] The input state should hold the same segment/voice as the starting ChordRest...
        EXPECT_EQ(inputState.segment(), startCR->segment());
        EXPECT_EQ(inputState.voice(), startCR->voice());

        //! [WHEN] The voice is switched (from 1 to 2, or 2 to 1)...
        const voice_idx_t newVoice = startCR->voice() == 0 ? 1 : 0;
        inputState.setVoice(newVoice);

        //! [THEN] The input state should hold the same segment as the destination ChordRest...
        EXPECT_EQ(inputState.segment(), destinationCR->segment());
    }

    delete score;
}
