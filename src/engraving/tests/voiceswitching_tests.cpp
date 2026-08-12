/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "engraving/dom/note.h"
#include "engraving/dom/rest.h"

#include "engraving/editing/editvoice.h"
#include "engraving/editing/transaction/transaction.h"

#include "utils/scorerw.h"

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

TEST_F(Engraving_VoiceSwitchingTests, articulationsAfterVoiceSwitch)
{
    //! [GIVEN] A measure with two voices, each beat having articulations in both voices
    Score* score = ScoreRW::readScore(VOICESWITCHING_DATA_DIR + "voiceswitching-articulation.mscx");
    EXPECT_TRUE(score);

    //! [WHEN] The first bar is range selected and all elements are moved to voice 0
    score->transactionManager()->transaction(TranslatableString::untranslatable("Engraving voice switching tests"), [&](Transaction& tx) {
        score->cmdSelectAll();
        EditVoice::changeSelectedElementsVoice(tx, score, 0);
    });

    //! [THEN] Articulations from both voices are merged into voice 0, with duplicates removed

    Measure* measure = score->firstMeasure();
    ASSERT_TRUE(measure);

    std::vector<Chord*> chords;
    for (Segment* seg = measure->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        EngravingItem* item = seg->element(0);
        if (item && item->isChord()) {
            chords.push_back(toChord(item));
        }
    }
    ASSERT_EQ(chords.size(), 4);

    // Beat 1: 1 staccato
    {
        const std::vector<Articulation*>& articulations = chords[0]->articulations();
        int staccatoCount = 0;
        for (Articulation* a : articulations) {
            if (a->isStaccato()) {
                ++staccatoCount;
            }
        }
        EXPECT_EQ(articulations.size(), 1);
        EXPECT_EQ(staccatoCount, 1);
    }

    // Beat 2: 1 staccato, 1 accent
    {
        const std::vector<Articulation*>& articulations = chords[1]->articulations();
        int staccatoCount = 0;
        int accentCount = 0;
        for (Articulation* a : articulations) {
            if (a->isStaccato()) {
                ++staccatoCount;
            }
            if (a->isAccent()) {
                ++accentCount;
            }
        }
        EXPECT_EQ(articulations.size(), 2);
        EXPECT_EQ(staccatoCount, 1);
        EXPECT_EQ(accentCount, 1);
    }

    // Beat 3: 1 tenuto
    {
        const std::vector<Articulation*>& articulations = chords[2]->articulations();
        int tenutoCount = 0;
        for (Articulation* a : articulations) {
            if (a->isTenuto()) {
                ++tenutoCount;
            }
        }
        EXPECT_EQ(articulations.size(), 1);
        EXPECT_EQ(tenutoCount, 1);
    }

    // Beat 4: 1 marcato, 1 staccato
    {
        const std::vector<Articulation*>& articulations = chords[3]->articulations();
        int staccatoCount = 0;
        int marcatoCount = 0;
        for (Articulation* a : articulations) {
            if (a->isStaccato()) {
                ++staccatoCount;
            }
            if (a->isMarcato()) {
                ++marcatoCount;
            }
        }
        EXPECT_EQ(articulations.size(), 2);
        EXPECT_EQ(staccatoCount, 1);
        EXPECT_EQ(marcatoCount, 1);
    }

    delete score;
}

TEST_F(Engraving_VoiceSwitchingTests, voicesSwitchingGapRests)
{
    Score* score = ScoreRW::readScore(VOICESWITCHING_DATA_DIR + "voiceswitching-2.mscx");
    EXPECT_TRUE(score);

    Segment* segment = score->tick2segment(Fraction(3, 4), true, SegmentType::ChordRest);
    EXPECT_TRUE(segment);

    //! [GIVEN] A measure with some notes in voice zero
    Chord* chord = toChord(segment->element(0));
    EXPECT_TRUE(chord);

    //! [WHEN] The last note of the measure is selected and moved to voice one
    score->select(chord->upNote());
    score->startCmd(TranslatableString("undoableAction", "Change voice"));
    EditVoice::changeSelectedElementsVoice(score->transactionManager()->currentOrDummyTransaction(), score, 1);
    score->endCmd();

    //! [THEN] Voice 1 should be filled with gap rests from the start of the measure
    Segment* firstSeg = score->firstSegment(SegmentType::ChordRest);
    EXPECT_TRUE(firstSeg);

    EngravingItem* item = firstSeg->element(1);
    EXPECT_TRUE(item && item->isRest() && toRest(item)->isGap());

    delete score;
}
