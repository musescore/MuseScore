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
#include <memory>

#include "mpe/tests/utils/articulationutils.h"

#include "dom/note.h"
#include "dom/factory.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/segment.h"
#include "dom/harppedaldiagram.h"
#include "dom/ornament.h"

#include "utils/scorerw.h"
#include "playback/playbackeventsrenderer.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

static const String PLAYBACK_EVENTS_RENDERING_DIR("playback/playbackeventsrenderer_data/");
static constexpr duration_t QUARTER_NOTE_DURATION = 500000; // duration in microseconds for 4/4 120BPM
static constexpr duration_t QUAVER_NOTE_DURATION = QUARTER_NOTE_DURATION / 2; // duration in microseconds for 4/4 120BPM
static constexpr duration_t SEMI_QUAVER_NOTE_DURATION = QUAVER_NOTE_DURATION / 2; // duration in microseconds for 4/4 120BPM
static constexpr duration_t DEMI_SEMI_QUAVER_NOTE_DURATION = QUARTER_NOTE_DURATION / 8; // duration in microseconds for 4/4 120BPM
static constexpr duration_t HALF_NOTE_DURATION = QUARTER_NOTE_DURATION * 2; // duration in microseconds for 1/2 120BPM
static constexpr duration_t WHOLE_NOTE_DURATION = QUARTER_NOTE_DURATION * 4; // duration in microseconds for 4/4 120BPM

class Engraving_PlaybackEventsRendererTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_dummyPatternSegment.arrangementPattern
            = tests::createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
        m_dummyPatternSegment.pitchPattern = tests::createSimplePitchPattern(0 /*increment_pitch_diff*/);
        m_dummyPatternSegment.expressionPattern = tests::createSimpleExpressionPattern(dynamicLevelFromType(mpe::DynamicType::Natural));
        m_dummyPattern.emplace(0, m_dummyPatternSegment);

        m_defaultProfile = std::make_shared<ArticulationsProfile>();

        //! NOTE: allows to read test files using their version readers
        //! instead of using 302 (see mscloader.cpp, makeReader)
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = true;
    }

    const Chord* findChord(const Score* score, int tick, track_idx_t track = 0) const
    {
        for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (!mb->isMeasure()) {
                continue;
            }

            const Chord* chord = toMeasure(mb)->findChord(Fraction::fromTicks(tick), track);
            if (chord) {
                return chord;
            }
        }

        return nullptr;
    }

    ArticulationsProfilePtr m_defaultProfile = nullptr;

    ArticulationPattern m_dummyPattern;
    ArticulationPatternSegment m_dummyPatternSegment;

    PlaybackEventsRenderer m_renderer;
};

/**
 * @brief PlaybackEventsRendererTests_SingleNote_TenutoAccent
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note marked by TENUTO-ACCENT articulation.
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_TenutoAccent)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "single_note_tenuto_accent/tenuto_accent.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Accent, m_dummyPattern);
    m_defaultProfile->setPattern(ArticulationType::Tenuto, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    // [THEN] We expect that a single note event will be rendered from the chord
    EXPECT_EQ(result.size(), 1);

    mpe::NoteEvent event = std::get<mpe::NoteEvent>(result.begin()->second.front());

    // [THEN] We expect that the note event will match time expectations of the very first quarter note with 120BPM tempo
    EXPECT_EQ(event.arrangementCtx().nominalTimestamp, 0);
    EXPECT_EQ(event.arrangementCtx().nominalDuration, QUARTER_NOTE_DURATION);

    // [THEN] We expect that the note event will match pitch expectations of F4 quarter note
    EXPECT_EQ(event.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 4));

    // [THEN] We expect that the note event will match expression expectations of our note
    EXPECT_EQ(event.expressionCtx().nominalDynamicLevel, dynamicLevelFromType(mpe::DynamicType::Natural));
    EXPECT_EQ(event.expressionCtx().articulations.size(), 2);
    EXPECT_TRUE(event.expressionCtx().articulations.contains(ArticulationType::Tenuto));
    EXPECT_TRUE(event.expressionCtx().articulations.contains(ArticulationType::Accent));
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_NoArticulations
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note without any articulations applied
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_NoArticulations)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "single_note_no_articulations/no_articulations.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Standard, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    // [THEN] We expect that a single note event will be rendered from the chord
    EXPECT_EQ(result.size(), 1);

    mpe::NoteEvent event = std::get<mpe::NoteEvent>(result.begin()->second.front());

    // [THEN] We expect that the note event will match time expectations of the very first quarter note with 120BPM tempo
    EXPECT_EQ(event.arrangementCtx().nominalTimestamp, 0);
    EXPECT_EQ(event.arrangementCtx().nominalDuration, QUARTER_NOTE_DURATION);

    // [THEN] We expect that the note event will match pitch expectations of F4 quarter note
    EXPECT_EQ(event.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 4));

    // [THEN] We expect that the note event will match expression expectations of our note
    EXPECT_EQ(event.expressionCtx().nominalDynamicLevel, dynamicLevelFromType(mpe::DynamicType::Natural));
    EXPECT_EQ(event.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(event.expressionCtx().articulations.contains(ArticulationType::Standard));
}

/**
 * @brief PlaybackEventsRendererTests_Rest
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which consists a rest only
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Rest)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "whole_measure_rest/whole_measure_rest.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* rest = firstSegment->nextChordRest(0);
    ASSERT_TRUE(rest);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render the rest
    PlaybackEventsMap result;
    m_renderer.render(rest, 0, m_defaultProfile, ctx, result);

    // [THEN] We expect that a single rest event will be rendered
    EXPECT_EQ(result.size(), 1);

    RestEvent event = std::get<RestEvent>(result.begin()->second.front());

    // [THEN] We expect that the rest event will match time expectations of the whole measure rest with 120BPM tempo
    EXPECT_EQ(event.arrangementCtx().nominalTimestamp, 0);
    EXPECT_EQ(event.arrangementCtx().nominalDuration, QUARTER_NOTE_DURATION * 4);
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_Trill_Modern
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note marked by Trill articulation.
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_Trill_Modern)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_trill_default_tempo/single_note_trill_default_tempo.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected trill disclosure
    int expectedTrillSubNotesCount = 8;

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Trill, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedTrillSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Trill
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Trill));

            // [THEN] The amount of trill alterations depends on many things, such as a tempo
            //        However, there is a couple of general rules of this disclosure:
            //        - Trill should start from a principal note
            //        - Trill should end up on a principal note. For that reasons is highly common to put a triplet on the last notes
            if (i == 0 || i == result.size() - 1) {
                EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 4));
            }

            // [THEN] In modern trills each even note should be higher than the principal one on a single diatonic step
            if ((i + 1) % 2 == 0) {
                EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 4) + 2 * PITCH_LEVEL_STEP);
            }
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_Unexpandable_Trill
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 32-nd note marked by Trill articulation.
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_Unexpandable_Trill)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_unexpandable_trill/single_note_unexpandable_trill.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected trill disclosure
    int expectedTrillSubNotesCount = 1;

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Trill, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    // [THEN] We expect that the only one note event will be rendered,
    //        since Trill consists of 32-nd note, we can't break down 32-nd principal note into smaller sub-notes
    EXPECT_EQ(result.size(), expectedTrillSubNotesCount);
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_Trill_Baroque
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note marked by Trill(Baroque) articulation.
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_Trill_Baroque)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_trill_baroque/single_note_trill_baroque.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected trill disclosure
    int expectedTrillSubNotesCount = 10;

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::TrillBaroque, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedTrillSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - TrillBaroque
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::TrillBaroque));

            // [THEN] The amount of trill alterations depends on various things, such as a tempo
            //        However, there is a couple of general rules of this disclosure in Baroque style:
            //        - Trill should start from a principal note + 1 diatonic step
            //        - Trill should end up on a principal note. For that reasons is highly common to put a triplet on the last notes
            if (i == 0) {
                EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 4) + 2 * PITCH_LEVEL_STEP);
            }

            if (i == pair.second.size() - 1) {
                EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 4));
            }

            // [THEN] In baroque trills each odd note should be higher than the principal one on a single diatonic step
            if (i % 2 == 0 && i < pair.second.size() - 2) {
                EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 4) + 2 * PITCH_LEVEL_STEP);
            }

            // [THEN] However, the note before the last should be lowe than the principal one on a single diatonic step
            if (i == pair.second.size() - 2) {
                EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 4) - PITCH_LEVEL_STEP);
            }
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_Turn_Regular
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note marked by Turn articulation.
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_Turn_Regular)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_regular_turn/single_note_regular_turn.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 4;
    duration_t expectedSubNoteDuration = QUAVER_NOTE_DURATION / expectedSubNotesCount;
    duration_t expectedPrincipalNoteDuration = 312500;

    std::vector<duration_t> expectedDurations = { expectedSubNoteDuration,
                                                  expectedSubNoteDuration,
                                                  expectedSubNoteDuration,
                                                  expectedPrincipalNoteDuration };

    std::vector<timestamp_t> expectedTimestamps = { 0,
                                                    expectedSubNoteDuration,
                                                    expectedSubNoteDuration* 2,
                                                    expectedSubNoteDuration* 3 };

    pitch_level_t nominalPitchLevel = pitchLevel(PitchClass::F, 4);
    pitch_level_t plus = nominalPitchLevel + 2 * PITCH_LEVEL_STEP;
    pitch_level_t minus = nominalPitchLevel - PITCH_LEVEL_STEP;
    std::vector<pitch_level_t> expectedPitches = { plus,
                                                   nominalPitchLevel,
                                                   minus,
                                                   nominalPitchLevel };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Turn, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Turn
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Turn));

            // [THEN] We expect that each sub-note in Regular Turn articulation will be equal to 0.25 of the principal note
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations[i]);
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps[i]);

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_Turn_Inverted
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note marked by Inverted Turn articulation.
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_Turn_Inverted)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_inverted_turn/single_note_inverted_turn.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 4;
    duration_t expectedSubNoteDuration = QUAVER_NOTE_DURATION / expectedSubNotesCount;
    duration_t expectedPrincipalNoteDuration = 312500;

    std::vector<duration_t> expectedDurations = { expectedSubNoteDuration,
                                                  expectedSubNoteDuration,
                                                  expectedSubNoteDuration,
                                                  expectedPrincipalNoteDuration };

    std::vector<timestamp_t> expectedTimestamps = { 0,
                                                    expectedSubNoteDuration,
                                                    expectedSubNoteDuration* 2,
                                                    expectedSubNoteDuration* 3 };

    pitch_level_t nominalPitchLevel = pitchLevel(PitchClass::F, 4);
    pitch_level_t plus = nominalPitchLevel + 2 * PITCH_LEVEL_STEP;
    pitch_level_t minus = nominalPitchLevel - PITCH_LEVEL_STEP;
    std::vector<pitch_level_t> expectedPitches = { minus,
                                                   nominalPitchLevel,
                                                   plus,
                                                   nominalPitchLevel };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::InvertedTurn, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - InvertedTurn
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::InvertedTurn));

            // [THEN] We expect that each sub-note in Inverted Turn articulation will be equal to 0.25 of the principal note
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations[i]);
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps[i]);

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_Turn_Inverted_Slash_Variation
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note marked by "Turn with slash" articulation,
 *          which is just a variation of Inverted Turn articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_Turn_Inverted_Slash_Variation)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_inverted_turn_slash_variation/single_note_inverted_turn_slash_variation.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 4;
    duration_t expectedSubNoteDuration = QUAVER_NOTE_DURATION / expectedSubNotesCount;
    duration_t expectedPrincipalNoteDuration = 312500;

    std::vector<duration_t> expectedDurations = { expectedSubNoteDuration,
                                                  expectedSubNoteDuration,
                                                  expectedSubNoteDuration,
                                                  expectedPrincipalNoteDuration };

    std::vector<timestamp_t> expectedTimestamps = { 0,
                                                    expectedSubNoteDuration,
                                                    expectedSubNoteDuration* 2,
                                                    expectedSubNoteDuration* 3 };

    pitch_level_t nominalPitchLevel = pitchLevel(PitchClass::F, 4);
    pitch_level_t plus = nominalPitchLevel + 2 * PITCH_LEVEL_STEP;
    pitch_level_t minus = nominalPitchLevel - PITCH_LEVEL_STEP;
    std::vector<pitch_level_t> expectedPitches = { minus,
                                                   nominalPitchLevel,
                                                   plus,
                                                   nominalPitchLevel };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::InvertedTurn, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - InvertedTurn
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::InvertedTurn));

            // [THEN] We expect that each sub-note in Inverted Turn articulation will be equal to 0.25 of the principal note
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations[i]);
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps[i]);

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_Upper_Mordent
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note marked by "Upper Mordent" articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_Upper_Mordent)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_upper_mordent/single_note_upper_mordent.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 3;

    std::vector<duration_t> expectedDurations = {
        DEMI_SEMI_QUAVER_NOTE_DURATION,
        DEMI_SEMI_QUAVER_NOTE_DURATION,
        3 * QUARTER_NOTE_DURATION / 4
    };

    std::vector<timestamp_t> expectedTimestamps = {
        0,
        DEMI_SEMI_QUAVER_NOTE_DURATION,
        2 * DEMI_SEMI_QUAVER_NOTE_DURATION
    };

    pitch_level_t nominalPitchLevel = pitchLevel(PitchClass::F, 4);
    pitch_level_t plus = nominalPitchLevel + 2 * PITCH_LEVEL_STEP;

    std::vector<pitch_level_t> expectedPitches = {
        nominalPitchLevel,
        plus,
        nominalPitchLevel
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::UpperMordent, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Upper Mordent
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::UpperMordent));

            // [THEN] We expect that each sub-note in Upper Mordent articulation has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_Lower_Mordent
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note marked by "Lower Mordent" articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_Lower_Mordent)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_lower_mordent/single_note_lower_mordent.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 3;

    std::vector<duration_t> expectedDurations = {
        DEMI_SEMI_QUAVER_NOTE_DURATION,
        DEMI_SEMI_QUAVER_NOTE_DURATION,
        3 * QUARTER_NOTE_DURATION / 4
    };

    std::vector<timestamp_t> expectedTimestamps = {
        0,
        DEMI_SEMI_QUAVER_NOTE_DURATION,
        2 * DEMI_SEMI_QUAVER_NOTE_DURATION
    };

    pitch_level_t nominalPitchLevel = pitchLevel(PitchClass::F, 4);
    pitch_level_t minus = nominalPitchLevel - PITCH_LEVEL_STEP;

    std::vector<pitch_level_t> expectedPitches = {
        nominalPitchLevel,
        minus,
        nominalPitchLevel
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::LowerMordent, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Lower Mordent
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::LowerMordent));

            // [THEN] We expect that each sub-note has an expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_TwoNotes_Discrete_Chromatic_Glissando
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with F4 and B4 quarter notes connected by Discrete Glissando articulation.
 */
TEST_F(Engraving_PlaybackEventsRendererTests, TwoNotes_Discrete_Glissando)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "two_notes_discrete_glissando/two_notes_discrete_glissando.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected glissando disclosure
    size_t expectedSubNotesCount = pitchStepsCount(std::abs(pitchLevelDiff(PitchClass::F, 4, PitchClass::B, 4)));

    float expectedDuration = static_cast<float>(QUARTER_NOTE_DURATION) / expectedSubNotesCount;

    pitch_level_t nominalPitchLevel = pitchLevel(PitchClass::F, 4);

    std::vector<pitch_level_t> expectedPitches;
    for (size_t i = 0; i < expectedSubNotesCount; ++i) {
        expectedPitches.push_back(nominalPitchLevel + static_cast<int>(i) * PITCH_LEVEL_STEP);
    }

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::DiscreteGlissando, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Discrete Glissando
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::DiscreteGlissando));

            // [THEN] We expect that each sub-note in Discrete Glissando articulation has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, static_cast<int>(expectedDuration));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, static_cast<int>(i * expectedDuration));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_TwoNotes_Continuous_Glissando
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with F4 and B4 quarter notes connected by ContinuousGlissando articulation.
 *          This measure is then repeated, see: https://github.com/musescore/MuseScore/issues/27287
 */
TEST_F(Engraving_PlaybackEventsRendererTests, TwoNotes_Continuous_Glissando)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score
        = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "two_notes_continuous_glissando/two_notes_continuous_glissando.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->repeatList().size(), 2);

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected glissando disclosure
    int expectedSubNotesCount = 1;

    duration_t expectedDuration = QUARTER_NOTE_DURATION;

    pitch_level_t nominalPitchLevel = pitchLevel(PitchClass::F, 4);

    std::vector<pitch_level_t> expectedPitches;
    for (int i = 0; i < expectedSubNotesCount; ++i) {
        expectedPitches.push_back(nominalPitchLevel + i * PITCH_LEVEL_STEP);
    }

    std::vector<timestamp_t> expectedTimestamps {
        0,
        WHOLE_NOTE_DURATION, // 2nd repeat segment
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::ContinuousGlissando, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;
        m_renderer.render(chord, tickPositionOffset, m_defaultProfile, ctx, result);
    }

    // [THEN] 2 events, as we have 2 repeat segments
    EXPECT_EQ(result.size(), 2);
    ASSERT_EQ(expectedTimestamps.size(), result.size());

    size_t noteEventNum = 0;

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each sub-note has an expected duration
            timestamp_t expectedTimestamp = expectedTimestamps.at(noteEventNum);
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDuration);
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamp);

            // [THEN] We expect that each note event has only one articulation applied - Continuous Glissando
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            auto glissIt = noteEvent.expressionCtx().articulations.find(ArticulationType::ContinuousGlissando);
            ASSERT_TRUE(glissIt != noteEvent.expressionCtx().articulations.end());

            // [THEN] The articulation has the correct timestamp & duration
            const ArticulationMeta& artMeta = glissIt->second.meta;
            EXPECT_EQ(artMeta.timestamp, expectedTimestamp);
            EXPECT_EQ(artMeta.overallDuration, expectedDuration);

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));

            noteEventNum++;
        }
    }

    delete score;
}

/**
 * @brief PlaybackEventsRendererTests_TwoNotes_Glissando_NoPlay
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with F4 and B4 quarter notes connected by ContinuousGlissando articulation.
 *          However, the glissando has been marked as "no play", so we'll render the Standard articulation instead
 */
TEST_F(Engraving_PlaybackEventsRendererTests, TwoNotes_Glissando_NoPlay)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "two_notes_continuous_glissando_no_play/two_notes_continuous_glissando_no_play.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected glissando disclosure
    int expectedSubNotesCount = 1;

    duration_t expectedDuration = QUARTER_NOTE_DURATION;

    pitch_level_t nominalPitchLevel = pitchLevel(PitchClass::F, 4);

    std::vector<pitch_level_t> expectedPitches;
    for (int i = 0; i < expectedSubNotesCount; ++i) {
        expectedPitches.push_back(nominalPitchLevel + i * PITCH_LEVEL_STEP);
    }

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Standard, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Standard articulation
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));

            // [THEN] We expect that each sub-note has an expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDuration);
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, static_cast<duration_t>(i) * expectedDuration);

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_TwoNotes_Discrete_Harp_Glissando
 * @details Render a score with 3 measures containing whole notes (C4, C5, C4) and glissandos
 *          between them.  Add harp pedal diagrams to the first and second measures set to a
 *          whole tone scale, then back to Cmaj.  Check the glisses render as expected
 */
TEST_F(Engraving_PlaybackEventsRendererTests, TwoNotes_Discrete_Harp_Glissando) {
    // [GIVEN]
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "TwoNotes_Discrete_Harp_Glissando/TwoNotes_Discrete_Harp_Glissando.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    Measure* secondMeasure = firstMeasure->nextMeasure();
    ASSERT_TRUE(secondMeasure);

    Segment* secondSegment = secondMeasure->segments().firstCRSegment();
    ASSERT_TRUE(secondSegment);

    ChordRest* secondChord = secondSegment->nextChordRest(0);
    ASSERT_TRUE(secondChord);

    // TEST FIRST GLISS

    // [GIVEN] Expected glissando disclosure
    size_t expectedSubNotesCount = 8;

    float expectedDuration = static_cast<float>(WHOLE_NOTE_DURATION) / expectedSubNotesCount;

    pitch_level_t nominalPitchLevel = pitchLevel(PitchClass::C, 4);
    std::vector<int> pitchesWt = { 0, 0, 2, 4, 6, 8, 10, 12 };

    std::vector<pitch_level_t> expectedPitches;
    for (size_t i = 0; i < expectedSubNotesCount; ++i) {
        expectedPitches.push_back(nominalPitchLevel + pitchesWt.at(i) * PITCH_LEVEL_STEP);
    }

    m_defaultProfile->setPattern(ArticulationType::DiscreteGlissando, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Discrete Glissando
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::DiscreteGlissando));

            // [THEN] We expect that each sub-note in Discrete Glissando articulation has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, static_cast<int>(expectedDuration));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, static_cast<int>(i * expectedDuration));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }

    // TEST SECOND GLISS

    // [GIVEN] Expected glissando disclosure
    expectedSubNotesCount = 4;
    expectedDuration = static_cast<float>(WHOLE_NOTE_DURATION) / expectedSubNotesCount;

    nominalPitchLevel = pitchLevel(PitchClass::C, 5);
    std::vector<int> pitches2 = { 0, 1, 3, 6 };

    expectedPitches.clear();
    for (size_t i = 0; i < expectedSubNotesCount; ++i) {
        expectedPitches.push_back(nominalPitchLevel - pitches2.at(i) * PITCH_LEVEL_STEP);
    }

    m_defaultProfile->setPattern(ArticulationType::DiscreteGlissando, m_dummyPattern);

    PlaybackEventsMap result2;
    m_renderer.render(secondChord, 0, m_defaultProfile, ctx, result2);

    for (const auto& pair : result2) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Discrete Glissando
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::DiscreteGlissando));

            // [THEN] We expect that each sub-note in Discrete Glissando articulation has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, static_cast<int>(expectedDuration));

            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, WHOLE_NOTE_DURATION + static_cast<int>(i * expectedDuration));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_Glissando_on_tied_notes
 * @details Make sure we render discrete/continuous glissando on tied notes
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Glissando_on_tied_notes)
{
    // [GIVEN]
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "glissando_on_tied_notes/glissando_on_tied_notes.mscx");

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::DiscreteGlissando, m_dummyPattern);
    m_defaultProfile->setPattern(ArticulationType::ContinuousGlissando, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // ###################################################
    // 1st case: Discrete glissando on a tied note
    // ###################################################
    const Chord* chord = findChord(score, 960, 0); // first tied A4
    ASSERT_TRUE(chord);
    ASSERT_EQ(chord->notes().size(), 1);
    ASSERT_TRUE(chord->notes().front()->tieFor());

    // [WHEN] Request to render a chord with the A4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    // [THEN] Note event has the correct duration and standard articulation (no glissando)
    pitch_level_t nominalPitchLevel = pitchLevel(PitchClass::A, 4);
    duration_t glissandoNoteDuration = QUARTER_NOTE_DURATION / 4;

    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result.begin()->second.size(), 1);
    mpe::NoteEvent resultingNoteEvent = std::get<mpe::NoteEvent>(result.begin()->second.at(0));
    EXPECT_EQ(resultingNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(resultingNoteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));
    EXPECT_EQ(resultingNoteEvent.pitchCtx().nominalPitchLevel, nominalPitchLevel);
    EXPECT_EQ(resultingNoteEvent.arrangementCtx().nominalDuration, QUARTER_NOTE_DURATION + glissandoNoteDuration); // A4 + 1st glissando note

    // [GIVEN] Tied A4 with discrete glissando
    chord = findChord(score, 1440, 0);
    ASSERT_TRUE(chord);
    ASSERT_EQ(chord->notes().size(), 1);
    ASSERT_TRUE(chord->notes().front()->tieBack());

    // [WHEN] Request to render a chord with the A4 note on it
    result.clear();
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    // [THEN] Expected glissando disclosure
    size_t expectedSubNotesCount = 3;
    mpe::timestamp_t expectedTimestamp = timestampFromTicks(score, chord->tick().ticks()) + glissandoNoteDuration;
    std::vector<int> pitchesWt = { -1, -2, -3 };

    std::vector<pitch_level_t> expectedPitches;
    for (size_t i = 0; i < expectedSubNotesCount; ++i) {
        expectedPitches.push_back(nominalPitchLevel + pitchesWt.at(i) * PITCH_LEVEL_STEP);
    }

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Discrete Glissando
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::DiscreteGlissando));

            // [THEN] We expect that each sub-note in Discrete Glissando articulation has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, glissandoNoteDuration);
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamp + static_cast<int>(i * glissandoNoteDuration));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }

    // ###################################################
    // 2nd case: Continuous glissando on a tied note
    // ###################################################
    chord = findChord(score, 4800, 0); // tied A4 with continuous glissando
    ASSERT_TRUE(chord);
    ASSERT_FALSE(chord->notes().empty());
    ASSERT_TRUE(chord->notes().front()->tieFor());

    // [WHEN] Request to render a chord with the A4 note on it
    result.clear();
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    // [THEN] Expected glissando disclosure
    expectedSubNotesCount = 1;
    expectedTimestamp = timestampFromTicks(score, chord->tick().ticks());
    glissandoNoteDuration = QUARTER_NOTE_DURATION * 2;
    nominalPitchLevel = pitchLevel(PitchClass::A, 4);

    expectedPitches.clear();
    for (size_t i = 0; i < expectedSubNotesCount; ++i) {
        expectedPitches.push_back(nominalPitchLevel + int(i) * PITCH_LEVEL_STEP);
    }

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Continuous Glissando
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::ContinuousGlissando));

            // [THEN] We expect that each sub-note has an expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, glissandoNoteDuration);
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamp + static_cast<duration_t>(i) * glissandoNoteDuration);

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }

    delete score;
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_Acciaccatura
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note prepended with G4 8-th acciaccatura note
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_Acciaccatura)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_acciaccatura/single_note_acciaccatura.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 2;

    std::vector<duration_t> expectedDurations = {
        DEMI_SEMI_QUAVER_NOTE_DURATION / 2,
        QUARTER_NOTE_DURATION - (DEMI_SEMI_QUAVER_NOTE_DURATION / 2)
    };

    std::vector<timestamp_t> expectedTimestamps = {
        0,
        DEMI_SEMI_QUAVER_NOTE_DURATION / 2
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::G, 4),
        pitchLevel(PitchClass::F, 4),
    };

    std::vector<ArticulationType> expectedArticulations = {
        ArticulationType::Acciaccatura,
        ArticulationType::Standard,
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Acciaccatura, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that Acciaccatura is only applied to the grace note event
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(expectedArticulations.at(i)));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_AcciaccaturaChord
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note prepended with G4 B4 D5 F5 8th acciaccatura chord
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_AcciaccaturaChord)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_acciaccatura_chord/single_note_acciaccatura_chord.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 5;

    std::vector<duration_t> expectedDurations = {
        DEMI_SEMI_QUAVER_NOTE_DURATION / 2,
        DEMI_SEMI_QUAVER_NOTE_DURATION / 2,
        DEMI_SEMI_QUAVER_NOTE_DURATION / 2,
        DEMI_SEMI_QUAVER_NOTE_DURATION / 2,
        QUARTER_NOTE_DURATION - (DEMI_SEMI_QUAVER_NOTE_DURATION / 2)
    };

    std::vector<timestamp_t> expectedTimestamps = {
        0,
        0,
        0,
        0,
        DEMI_SEMI_QUAVER_NOTE_DURATION / 2
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::G, 4),
        pitchLevel(PitchClass::B, 4),
        pitchLevel(PitchClass::D, 5),
        pitchLevel(PitchClass::F, 5),
        pitchLevel(PitchClass::F, 4),
    };

    std::vector<ArticulationType> expectedArticulations = {
        ArticulationType::Acciaccatura,
        ArticulationType::Acciaccatura,
        ArticulationType::Acciaccatura,
        ArticulationType::Acciaccatura,
        ArticulationType::Standard,
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Acciaccatura, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that Acciaccatura is only applied to the grace note events
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(expectedArticulations.at(i)));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_MultiAcciaccatura
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note prepended with A4+G4 8-th acciaccatura notes
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_MultiAcciaccatura)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_multi_acciaccatura/single_note_multi_acciaccatura.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 3;

    std::vector<duration_t> expectedDurations = {
        DEMI_SEMI_QUAVER_NOTE_DURATION / 2,
        DEMI_SEMI_QUAVER_NOTE_DURATION / 2,
        QUARTER_NOTE_DURATION - DEMI_SEMI_QUAVER_NOTE_DURATION
    };

    std::vector<timestamp_t> expectedTimestamps = {
        0,
        DEMI_SEMI_QUAVER_NOTE_DURATION / 2,
        DEMI_SEMI_QUAVER_NOTE_DURATION
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::A, 4),
        pitchLevel(PitchClass::G, 4),
        pitchLevel(PitchClass::F, 4),
    };

    std::vector<ArticulationType> expectedArticulations = {
        ArticulationType::Acciaccatura,
        ArticulationType::Acciaccatura,
        ArticulationType::Standard,
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Acciaccatura, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that Acciaccatura is only applied to the grace note events
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(expectedArticulations.at(i)));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_GraceNoteWithTiedNotes
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter grace note, followed by the G4 half note and tied to another half note
 */
TEST_F(Engraving_PlaybackEventsRendererTests, GraceNoteWithTiedNotes)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "grace_note_with_tied_notes/grace_note_with_tied_notes.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 2;

    std::vector<duration_t> expectedDurations = {
        HALF_NOTE_DURATION / 2, // PreAppoggiatura, quarter note
        2 * HALF_NOTE_DURATION - HALF_NOTE_DURATION / 2, // 2 * half note duration - grace note duration
    };

    std::vector<timestamp_t> expectedTimestamps = {
        0,
        HALF_NOTE_DURATION / 2
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::G, 4),
        pitchLevel(PitchClass::G, 4),
    };

    std::vector<ArticulationType> expectedArticulations = {
        ArticulationType::PreAppoggiatura,
        ArticulationType::Standard,
        ArticulationType::Standard,
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::PreAppoggiatura, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    // [THEN] The events map is not empty
    EXPECT_FALSE(result.empty());

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that PreAppoggiatura is only applied to the grace note event
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(expectedArticulations.at(i)));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_Appoggiatura_Post
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note prepended with G4 8-th appoggiatura note
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_Appoggiatura_Post)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score
        = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "single_note_appoggiatura_post/single_note_appoggiatura_post.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 2;

    std::vector<duration_t> expectedDurations = {
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION
    };

    std::vector<timestamp_t> expectedTimestamps = {
        0,
        QUAVER_NOTE_DURATION
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::G, 4)
    };

    std::vector<ArticulationType> expectedArticulations = {
        ArticulationType::Standard,
        ArticulationType::PostAppoggiatura,
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::PostAppoggiatura, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that PostAppoggiatura is only applied to the grace note event
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(expectedArticulations.at(i)));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_AppoggiaturaChord_Post
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note prepended with G4 B4 D5 F5 8th appoggiatura chord
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_AppoggiaturaChord_Post)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_appoggiatura_chord_post/single_note_appoggiatura_chord_post.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 5;

    std::vector<duration_t> expectedDurations = {
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION
    };

    std::vector<timestamp_t> expectedTimestamps = {
        0,
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::G, 4),
        pitchLevel(PitchClass::B, 4),
        pitchLevel(PitchClass::D, 5),
        pitchLevel(PitchClass::F, 5),
    };

    std::vector<ArticulationType> expectedArticulations = {
        ArticulationType::Standard,
        ArticulationType::PostAppoggiatura,
        ArticulationType::PostAppoggiatura,
        ArticulationType::PostAppoggiatura,
        ArticulationType::PostAppoggiatura,
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::PostAppoggiatura, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that PostAppoggiatura is only applied to the grace note events
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(expectedArticulations.at(i)));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_SingleNote_MultiAppoggiatura_Post
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 quarter note prepended with G4 16-th and A4 32-nd appoggiatura notes
 */
TEST_F(Engraving_PlaybackEventsRendererTests, SingleNote_MultiAppoggiatura_Post)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_multi_appoggiatura_post/single_note_multi_appoggiatura_post.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 3;

    std::vector<duration_t> expectedDurations = {
        QUARTER_NOTE_DURATION - SEMI_QUAVER_NOTE_DURATION - DEMI_SEMI_QUAVER_NOTE_DURATION,
        SEMI_QUAVER_NOTE_DURATION,
        DEMI_SEMI_QUAVER_NOTE_DURATION
    };

    std::vector<timestamp_t> expectedTimestamps = {
        0,
        QUARTER_NOTE_DURATION - SEMI_QUAVER_NOTE_DURATION - DEMI_SEMI_QUAVER_NOTE_DURATION,
        QUARTER_NOTE_DURATION - DEMI_SEMI_QUAVER_NOTE_DURATION
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::G, 4),
        pitchLevel(PitchClass::A, 4),
    };

    std::vector<ArticulationType> expectedArticulations = {
        ArticulationType::Standard,
        ArticulationType::PostAppoggiatura,
        ArticulationType::PostAppoggiatura,
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::PostAppoggiatura, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that PostAppoggiatura is only applied to the grace note events
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(expectedArticulations.at(i)));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamps.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_Chord_Arpeggio
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4+A4+C4 16th chord marked by the Arpeggio articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Chord_Arpeggio)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "chord_arpeggio/chord_arpeggio.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 3;
    int expectedOffset = SEMI_QUAVER_NOTE_DURATION / 3;

    std::vector<timestamp_t> expectedTimestamp = {
        0,
        expectedOffset,
        expectedOffset* 2,
    };

    std::vector<duration_t> expectedDurations = {
        SEMI_QUAVER_NOTE_DURATION,
        SEMI_QUAVER_NOTE_DURATION - expectedOffset,
        SEMI_QUAVER_NOTE_DURATION - expectedOffset * 2
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::A, 4),
        pitchLevel(PitchClass::C, 5)
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Arpeggio, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Arpeggio));

            // [THEN] We expect that each sub-note has expected timestamp and duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamp.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_Chord_Arpeggio_Up
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4+A4+C4 quarter chord marked by the Arpeggio Up articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Chord_Arpeggio_Up)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "chord_arpeggio_up/chord_arpeggio_up.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 3;
    int expectedOffset = 60000;

    std::vector<timestamp_t> expectedTimestamp = {
        0,
        expectedOffset,
        expectedOffset* 2,
    };

    std::vector<duration_t> expectedDurations = {
        QUARTER_NOTE_DURATION,
        QUARTER_NOTE_DURATION - expectedOffset,
        QUARTER_NOTE_DURATION - expectedOffset * 2
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::A, 4),
        pitchLevel(PitchClass::C, 5),
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::ArpeggioUp, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::ArpeggioUp));

            // [THEN] We expect that each sub-note has expected timestamp and duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamp.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief Chord_Arpeggio_TiedNotes
 * @details In this case we're gonna render a simple piece of score with 3 measures,
 *          which starts with the B4+D5 half chord marked by the Arpeggio Up articulation
 *          Check that Arpeggio gets extended by tied notes
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Chord_Arpeggio_Up_TiedNotes)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "chord_arpeggio_with_tied_notes/chord_arpeggio_with_tied_notes.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 2;
    int expectedOffset = 60000;

    std::vector<timestamp_t> expectedTimestamp = {
        0,
        expectedOffset,
    };

    std::vector<duration_t> expectedDurations = {
        HALF_NOTE_DURATION* 3, // + 2 tied notes
        HALF_NOTE_DURATION* 3 - expectedOffset, // + 2 tied notes
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::B, 4),
        pitchLevel(PitchClass::D, 5),
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::ArpeggioUp, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::ArpeggioUp));

            // [THEN] We expect that each sub-note has expected duration
            // [THEN] We expect that each sub-note has expected timestamp and duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamp.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_Chord_Arpeggio_Down
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4+A4+C4 quarter chord marked by the Arpeggio Down articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Chord_Arpeggio_Down)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "chord_arpeggio_down/chord_arpeggio_down.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 3;
    int expectedOffset = 60000;

    std::vector<timestamp_t> expectedTimestamp = {
        0,
        expectedOffset,
        expectedOffset* 2,
    };

    std::vector<duration_t> expectedDurations = {
        QUARTER_NOTE_DURATION,
        QUARTER_NOTE_DURATION - expectedOffset,
        QUARTER_NOTE_DURATION - expectedOffset * 2
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::C, 5),
        pitchLevel(PitchClass::A, 4),
        pitchLevel(PitchClass::F, 4),
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::ArpeggioDown, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::ArpeggioDown));

            // [THEN] We expect that each sub-note has expected timestamp and duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamp.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_Chord_Arpeggio_Straight_Down
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4+A4+C4 quarter chord marked by the Arpeggio Straight Down articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Chord_Arpeggio_Straight_Down)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "chord_arpeggio_straight_down/chord_arpeggio_straight_down.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 3;
    int expectedOffset = 60000;

    std::vector<timestamp_t> expectedTimestamp = {
        0,
        expectedOffset,
        expectedOffset* 2,
    };

    std::vector<duration_t> expectedDurations = {
        QUARTER_NOTE_DURATION,
        QUARTER_NOTE_DURATION - expectedOffset,
        QUARTER_NOTE_DURATION - expectedOffset * 2
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::C, 5),
        pitchLevel(PitchClass::A, 4),
        pitchLevel(PitchClass::F, 4),
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::ArpeggioStraightDown, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::ArpeggioStraightDown));

            // [THEN] We expect that each sub-note has expected timestamp and duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamp.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_Chord_Arpeggio_Straight_Up
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4+A4+C4 quarter chord marked by the Arpeggio Straight Up articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Chord_Arpeggio_Straight_Up)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "chord_arpeggio_straight_up/chord_arpeggio_straight_up.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 3;
    int expectedOffset = 60000;

    std::vector<timestamp_t> expectedTimestamp = {
        0,
        expectedOffset,
        expectedOffset* 2,
    };

    std::vector<duration_t> expectedDurations = {
        QUARTER_NOTE_DURATION,
        QUARTER_NOTE_DURATION - expectedOffset,
        QUARTER_NOTE_DURATION - expectedOffset * 2
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::A, 4),
        pitchLevel(PitchClass::C, 5),
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::ArpeggioStraightUp, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::ArpeggioStraightUp));

            // [THEN] We expect that each sub-note has expected timestamp and duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, expectedTimestamp.at(i));
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_Chord_Arpeggio_Straight_Up
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4+A4+C4 quarter chord marked by the Arpeggio Bracket, which means that chord should be played
 *          like it has not arpeggio
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Chord_Arpeggio_Bracket)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "chord_arpeggio_bracket/chord_arpeggio_bracket.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 3;

    std::vector<duration_t> expectedDurations = {
        QUARTER_NOTE_DURATION,
        QUARTER_NOTE_DURATION,
        QUARTER_NOTE_DURATION
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::A, 4),
        pitchLevel(PitchClass::C, 5),
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Standard, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_PartiallyTiedArpeggio
 */
TEST_F(Engraving_PlaybackEventsRendererTests, PartiallyTiedArpeggio)
{
    // [GIVEN] Simple piece of score (violin, 4/4, 120 bpm, Treble Cleff)
    Score* score
        = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "partially_tied_arpeggio.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::ArpeggioUp, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Render the score
    PlaybackEventsMap result;
    for (const RepeatSegment* repeat : score->repeatList()) {
        const int tickPositionOffset = repeat->utick - repeat->tick;

        for (const Measure* m : repeat->measureList()) {
            for (const Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                const EngravingItem* el = s->element(0);
                if (!el || !el->isChord()) {
                    continue;
                }

                m_renderer.render(toChord(el), tickPositionOffset, m_defaultProfile, ctx, result);
            }
        }
    }

    const std::vector<timestamp_t> expectedArpeggioTimestamps {
        0, // 1st measure
        3500000, // 2nd measure
        7500000, // 2nd measure (repeated)
    };

    const std::vector<duration_t> expectedArpeggioDurations {
        QUARTER_NOTE_DURATION, // 1st measure
        QUARTER_NOTE_DURATION* 2, // 2nd measure
        QUARTER_NOTE_DURATION, // 2nd measure (repeated)
    };

    size_t arpeggioNum = 0;

    for (const auto& pair : result) {
        if (pair.second.empty()) {
            continue;
        }

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const PlaybackEvent& event = pair.second.at(i);
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            ASSERT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::ArpeggioUp));

            // [THEN] Each note event has the correct timestamp & duration
            ASSERT_TRUE(arpeggioNum < expectedArpeggioTimestamps.size());
            ASSERT_TRUE(arpeggioNum < expectedArpeggioDurations.size());

            const duration_t expectedOffset = 60000 * i;
            const timestamp_t expectedTimestamp = expectedArpeggioTimestamps.at(arpeggioNum) + expectedOffset;
            const duration_t expectedDuration = expectedArpeggioDurations.at(arpeggioNum) - expectedOffset;

            EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, expectedTimestamp);
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, expectedDuration);
        }

        arpeggioNum++;
    }

    delete score;
}

/**
 * @brief PlaybackEventsRendererTests_Single_Note_Tremolo
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4 half note marked by the 8-th Tremolo articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Single_Note_Tremolo)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "single_note_tremolo/single_note_tremolo.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedSubNotesCount = 4;

    std::vector<duration_t> expectedDurations = {
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION,
        QUAVER_NOTE_DURATION
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::F, 4)
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Tremolo8th, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Tremolo8th));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(i));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_Single_Note_Tremolo_OnTiedNote
 * @details In this case we're gonna render a simple piece of score with 3 measures,
 *          which starts with an A4 half note tied to another A4 half note marked by the 32nd Tremolo articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Single_Note_Tremolo_OnTiedNote)
{
    // [GIVEN] Simple piece of score (violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_EVENTS_RENDERING_DIR + "single_note_tremolo_on_tied_note/single_note_tremolo_on_tied_note.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Standard, m_dummyPattern);
    m_defaultProfile->setPattern(ArticulationType::Tremolo32nd, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [GIVEN] Chord of the 1st tied note (without tremolo)
    const Chord* chord = findChord(score, 1920);
    ASSERT_TRUE(chord);
    ASSERT_EQ(chord->notes().size(), 1);

    const Note* firstTiedNote = chord->notes().front();
    ASSERT_TRUE(firstTiedNote->tieFor() && !firstTiedNote->tieBack());

    // [WHEN] Request to render the 1st chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    // [THEN] Note event has normal duration and standard articulation (no tremolo)
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result.begin()->second.size(), 1);
    mpe::NoteEvent resultingNoteEvent = std::get<mpe::NoteEvent>(result.begin()->second.at(0));
    EXPECT_EQ(resultingNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(resultingNoteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));
    EXPECT_EQ(resultingNoteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::A, 4));
    EXPECT_EQ(resultingNoteEvent.arrangementCtx().nominalDuration, HALF_NOTE_DURATION);

    // [GIVEN] Chord of the 2nd tied note (with tremolo)
    chord = findChord(score, 2880);
    ASSERT_TRUE(chord);
    ASSERT_EQ(chord->notes().size(), 1);

    const Note* lastTiedNote = chord->notes().front();
    ASSERT_TRUE(lastTiedNote->tieBack() && !lastTiedNote->tieFor());

    // [WHEN] Request to render the 2nd chord
    result.clear();
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    constexpr int expectedSubNotesCount = 16;
    constexpr mpe::duration_t expectedDuration = HALF_NOTE_DURATION / expectedSubNotesCount;

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Tremolo32nd));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDuration);

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::A, 4));
        }
    }

    delete score;
}

/**
 * @brief PlaybackEventsRendererTests_Single_Chord_Tremolo
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4+A4+C5 quarter notes chord marked by the 16-th Tremolo articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Single_Chord_Tremolo)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "single_chord_tremolo/single_chord_tremolo.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedChordsCount = 4;
    int expectedSubNotesCount = 3;

    std::vector<duration_t> expectedDurations = {
        SEMI_QUAVER_NOTE_DURATION,
        SEMI_QUAVER_NOTE_DURATION,
        SEMI_QUAVER_NOTE_DURATION,
        SEMI_QUAVER_NOTE_DURATION
    };

    std::vector<pitch_level_t> expectedPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::A, 4),
        pitchLevel(PitchClass::C, 5)
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Tremolo16th, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    // [THEN] We expect that rendered note events number will match expectations
    EXPECT_EQ(result.begin()->second.size(), expectedChordsCount * expectedSubNotesCount);

    for (int chordIdx = 0; chordIdx < expectedChordsCount; ++chordIdx) {
        for (int noteIdx = 0; noteIdx < expectedSubNotesCount; ++noteIdx) {
            int eventIdx = (chordIdx * expectedSubNotesCount) + noteIdx;

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(result.begin()->second.at(eventIdx));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Tremolo16th));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(chordIdx));

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(noteIdx));
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_Single_Chord_Tremolo_TiedNotes
 * @details In this case we're gonna render a simple piece of score with 3 single measures.
 *          The second measure has a whole note tied to a whole note in the third measure.
 *          Each note has a Tremolo32nd.
 *          This test checks that all tied notes have the correct tremolo time and duration
 *          See: https://github.com/musescore/MuseScore/issues/23168
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Single_Chord_Tremolo_TiedNotes)
{
    // [GIVEN] Simple piece of score (violin, 4/4, 120 bpm, Treble Cleff)
    Score* score
        = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "single_chord_tremolo_tied_notes/single_chord_tremolo_tied_notes.mscx");

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Tremolo32nd, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Render the score
    PlaybackEventsMap result;

    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (const Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
            const mu::engraving::EngravingItem* element = segment->element(0);
            if (element && element->isChord()) {
                m_renderer.render(toChord(element), 0, m_defaultProfile, ctx, result);
            }
        }
    }

    // [THEN] We expect 2 lists of events (for 2 tied notes)
    EXPECT_EQ(result.size(), 2);

    constexpr timestamp_t expectedTremoloTimestamp = 2000000; // timestamp of the 1st tied note
    constexpr timestamp_t expectedTremoloDuration = WHOLE_NOTE_DURATION * 2; // total duration of all tied notes
    constexpr timestamp_t expectedTremoloTimestampTo = expectedTremoloTimestamp + expectedTremoloDuration;

    constexpr pitch_level_t expectedPitchLevel = pitchLevel(PitchClass::F, 4);
    constexpr duration_t expectedSubNoteDuration = 62500;

    size_t tremoloStartRangeEventCount = 0;
    size_t tremoloEndRangeEventCount = 0;

    for (const auto& pair : result) {
        // [THEN] Expected number of sub-notes
        EXPECT_EQ(pair.second.size(), 32);

        for (const PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            ASSERT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Tremolo32nd));

            // [THEN] Each note event has the correct articulation time and duration
            const ArticulationAppliedData& articulationData = noteEvent.expressionCtx().articulations.at(ArticulationType::Tremolo32nd);
            EXPECT_EQ(articulationData.meta.timestamp, expectedTremoloTimestamp);
            EXPECT_EQ(articulationData.meta.overallDuration, expectedTremoloDuration);

            const timestamp_t noteTimestampTo = noteEvent.arrangementCtx().actualTimestamp + noteEvent.arrangementCtx().actualDuration;

            // [THEN] Each note has the correct occupiedFrom/To
            if (noteEvent.arrangementCtx().nominalTimestamp == expectedTremoloTimestamp) {
                EXPECT_EQ(articulationData.occupiedFrom, 0);
                EXPECT_NE(articulationData.occupiedTo, HUNDRED_PERCENT);
                tremoloStartRangeEventCount++;
            } else if (noteTimestampTo == expectedTremoloTimestampTo) {
                EXPECT_NE(articulationData.occupiedFrom, 0);
                EXPECT_EQ(articulationData.occupiedTo, HUNDRED_PERCENT);
                tremoloEndRangeEventCount++;
            } else {
                EXPECT_NE(articulationData.occupiedFrom, 0);
                EXPECT_NE(articulationData.occupiedTo, HUNDRED_PERCENT);
            }

            // [THEN] Each note event has the correct pitch level
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitchLevel);

            // [THEN] Each note event has the correct duration
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, expectedSubNoteDuration);

            // [THEN] Each note event has the correct pitch level
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitchLevel);

            // [THEN] Each note event has the correct duration
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, expectedSubNoteDuration);
        }
    }

    // [THEN] We expect only 1 start & end event for the tremolo
    EXPECT_EQ(tremoloStartRangeEventCount, 1);
    EXPECT_EQ(tremoloEndRangeEventCount, 1);

    delete score;
}

/**
 * @brief PlaybackEventsRendererTests_PartiallyTiedTremolo
 */
TEST_F(Engraving_PlaybackEventsRendererTests, PartiallyTiedTremolo)
{
    // [GIVEN] Simple piece of score (violin, 4/4, 120 bpm, Treble Cleff)
    Score* score
        = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "partially_tied_tremolo.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Tremolo16th, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Render the score
    PlaybackEventsMap result;
    for (const RepeatSegment* repeat : score->repeatList()) {
        const int tickPositionOffset = repeat->utick - repeat->tick;

        for (const Measure* m : repeat->measureList()) {
            for (const Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                const EngravingItem* el = s->element(0);
                if (!el || !el->isChord()) {
                    continue;
                }

                m_renderer.render(toChord(el), tickPositionOffset, m_defaultProfile, ctx, result);
            }
        }
    }

    // [THEN] We expect 4 lists of events (for each note with tremolo)
    EXPECT_EQ(result.size(), 4);

    const std::vector<timestamp_t> expectedTremoloTimestamps {
        0, // 1st measure
        5000000, // 3rd measure, outgoing partially tied A4
        5000000, // 1st measure (repeated), incoming A4
        11000000, // 3rd measure (repeated), outgoing A4 note
    };

    const std::vector<duration_t> expectedTremoloDurations {
        HALF_NOTE_DURATION, // 1st measure
        HALF_NOTE_DURATION* 2, // 3rd measure: total duration of all partially tied notes
        HALF_NOTE_DURATION* 2, // 1st measure (repeated): total duration of all partially tied notes
        HALF_NOTE_DURATION, // 3rd measure (repeated)
    };

    constexpr pitch_level_t expectedPitchLevel = pitchLevel(PitchClass::A, 4);
    constexpr size_t expectedSubNoteCount = 8;
    constexpr duration_t expectedSubNoteDuration = HALF_NOTE_DURATION / expectedSubNoteCount;

    size_t tremoloNoteNum = 0;
    size_t tremoloStartRangeEventCount = 0;
    size_t tremoloEndRangeEventCount = 0;

    for (const auto& pair : result) {
        // [THEN] Expected number of sub-notes
        EXPECT_EQ(pair.second.size(), expectedSubNoteCount);

        const timestamp_t expectedTremoloTimestamp = expectedTremoloTimestamps.at(tremoloNoteNum);
        const duration_t expectedTremoloDuration = expectedTremoloDurations.at(tremoloNoteNum);
        const timestamp_t expectedTremoloTimestampTo = expectedTremoloTimestamp + expectedTremoloDuration;

        for (const PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            ASSERT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Tremolo16th));

            // [THEN] Each note event has the correct articulation time and duration
            const ArticulationAppliedData& articulationData = noteEvent.expressionCtx().articulations.at(ArticulationType::Tremolo16th);
            EXPECT_EQ(articulationData.meta.timestamp, expectedTremoloTimestamp);
            EXPECT_EQ(articulationData.meta.overallDuration, expectedTremoloDuration);

            const timestamp_t noteTimestampTo = noteEvent.arrangementCtx().actualTimestamp + noteEvent.arrangementCtx().actualDuration;

            // [THEN] Each note has the correct occupiedFrom/To
            if (noteEvent.arrangementCtx().nominalTimestamp == expectedTremoloTimestamp) {
                EXPECT_EQ(articulationData.occupiedFrom, 0);
                EXPECT_NE(articulationData.occupiedTo, HUNDRED_PERCENT);
                tremoloStartRangeEventCount++;
            } else if (noteTimestampTo == expectedTremoloTimestampTo) {
                EXPECT_NE(articulationData.occupiedFrom, 0);
                EXPECT_EQ(articulationData.occupiedTo, HUNDRED_PERCENT);
                tremoloEndRangeEventCount++;
            } else {
                EXPECT_NE(articulationData.occupiedFrom, 0);
                EXPECT_NE(articulationData.occupiedTo, HUNDRED_PERCENT);
            }

            // [THEN] Each note event has the correct pitch level
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitchLevel);

            // [THEN] Each note event has the correct duration
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, expectedSubNoteDuration);
        }

        tremoloNoteNum++;
    }

    // [THEN] We expect 3 start & end events for each tremolo
    EXPECT_EQ(tremoloStartRangeEventCount, 3);
    EXPECT_EQ(tremoloEndRangeEventCount, 3);

    delete score;
}

/**
 * @brief PlaybackEventsRendererTests_Two_Chords_Tremolo
 * @details In this case we're gonna render a simple piece of score with a single measure,
 *          which starts with the F4+A4+C5 chord and ends with C5+E5+G5 chord connected by the 16-th Tremolo articulation
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Two_Chords_Tremolo)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "two_chords_tremolo/two_chords_tremolo.mscx");

    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    ChordRest* chord = firstSegment->nextChordRest(0);
    ASSERT_TRUE(chord);

    // [GIVEN] Expected disclosure
    int expectedChordsCount = 16;
    int chordNotesCount = 3;

    std::vector<duration_t> expectedDurations(expectedChordsCount, SEMI_QUAVER_NOTE_DURATION);

    std::vector<pitch_level_t> firstChordPitches = {
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::A, 4),
        pitchLevel(PitchClass::C, 5)
    };

    std::vector<pitch_level_t> secondChordPitches = {
        pitchLevel(PitchClass::C, 5),
        pitchLevel(PitchClass::E, 5),
        pitchLevel(PitchClass::G, 5)
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Tremolo16th, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, 0, m_defaultProfile, ctx, result);

    // [THEN] We expect that rendered note events number will match expectations
    EXPECT_EQ(result.begin()->second.size(), expectedChordsCount * chordNotesCount);

    for (int chordIdx = 0; chordIdx < expectedChordsCount; ++chordIdx) {
        for (int noteIdx = 0; noteIdx < chordNotesCount; ++noteIdx) {
            int eventIdx = (chordIdx * chordNotesCount) + noteIdx;

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(result.begin()->second.at(eventIdx));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Tremolo16th));

            // [THEN] We expect that each sub-note has expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(chordIdx));

            // [THEN] We expect that each note event will match expected pitch disclosure
            if (chordIdx % 2 == 0) {
                EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, firstChordPitches.at(noteIdx));
            } else {
                EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, secondChordPitches.at(noteIdx));
            }
        }
    }
}

/**
 * @brief PlaybackEventsRendererTests_Pauses
 *  @details Checks whether we correctly calculate note durations when there are pauses in the score. See:
 * https://github.com/musescore/MuseScore/issues/20669
 * https://github.com/musescore/MuseScore/issues/20557
 * https://github.com/musescore/MuseScore/issues/21289
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Pauses)
{
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "pauses/pauses.mscx");

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Standard, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Render the score
    PlaybackEventsMap result;

    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* m : repeatSegment->measureList()) {
            for (const Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                const EngravingItem* el = s->element(0);
                if (!el || !el->isChord()) {
                    continue;
                }

                m_renderer.render(toChord(el), tickPositionOffset, m_defaultProfile, ctx, result);
            }
        }
    }

    // [THEN] Expected time and duration of each event
    timestamp_t secondMeasureTime = QUARTER_NOTE_DURATION * 4;

    timestamp_t thirdMeasureTime = secondMeasureTime + QUARTER_NOTE_DURATION + 2000000 + HALF_NOTE_DURATION
                                   + QUARTER_NOTE_DURATION + 4000000;

    timestamp_t fourthMeasureTime = thirdMeasureTime + HALF_NOTE_DURATION + QUARTER_NOTE_DURATION
                                    + 3000000 + QUARTER_NOTE_DURATION;

    timestamp_t fifthMeasureTime = fourthMeasureTime + (WHOLE_NOTE_DURATION + 5000000) * 2; // repeat

    std::vector<TimestampAndDuration> expectedTnDList {
        // 1st measure (no notes)

        // 2nd measure
        { secondMeasureTime, QUARTER_NOTE_DURATION }, // + 2s pause (caesura)
        { secondMeasureTime + QUARTER_NOTE_DURATION + 2000000, HALF_NOTE_DURATION },
        { secondMeasureTime + QUARTER_NOTE_DURATION + 2000000 + HALF_NOTE_DURATION, QUARTER_NOTE_DURATION }, // + 4s pause (section break)

        // 3rd measure
        { thirdMeasureTime, HALF_NOTE_DURATION + QUARTER_NOTE_DURATION }, // tied half + quarter notes + 3s pause (caesura)
        { thirdMeasureTime + HALF_NOTE_DURATION + QUARTER_NOTE_DURATION + 3000000, QUARTER_NOTE_DURATION },

        // 4th measure (whole note inside repeat + section break)
        { fourthMeasureTime, WHOLE_NOTE_DURATION }, // + 5s pause (section break)
        { fourthMeasureTime + WHOLE_NOTE_DURATION + 5000000, WHOLE_NOTE_DURATION }, // repeat

        // 5th measure
        { fifthMeasureTime, HALF_NOTE_DURATION },
    };

    EXPECT_FALSE(result.empty());

    int tndNum = 0;
    for (const auto& pair : result) {
        for (const mpe::PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));

            const TimestampAndDuration& expectedTnD = expectedTnDList.at(tndNum);
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            EXPECT_EQ(pair.first, expectedTnD.timestamp);
            EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, expectedTnD.timestamp);
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, expectedTnD.duration);

            ++tndNum;
        }
    }

    EXPECT_EQ(tndNum, expectedTnDList.size());
}

/**
 * @brief PlaybackEventsRendererTests_TiedNotesAndRepeats
 *  @details Checks whether we correctly calculate tied note durations when they are inside/outside repeats. See:
 *  https://github.com/musescore/MuseScore/issues/22863
 */
TEST_F(Engraving_PlaybackEventsRendererTests, TiedNotesAndRepeats)
{
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "tied_notes_and_repeats.mscx");

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Standard, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Render the score
    PlaybackEventsMap result;

    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* m : repeatSegment->measureList()) {
            for (const Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                const EngravingItem* el = s->element(0);
                if (!el || !el->isChord()) {
                    continue;
                }

                m_renderer.render(toChord(el), tickPositionOffset, m_defaultProfile, ctx, result);
            }
        }
    }

    // [THEN] Expected pitch, time and duration of each event
    std::vector<pitch_level_t> expectedPitchList {
        pitchLevel(PitchClass::C, 5),
        pitchLevel(PitchClass::A, 4),
        pitchLevel(PitchClass::C, 5),
        pitchLevel(PitchClass::A, 4),
    };

    timestamp_t secondMeasureTime = WHOLE_NOTE_DURATION;
    timestamp_t secondMesaureRepeatedTime = secondMeasureTime + QUARTER_NOTE_DURATION * 2 + HALF_NOTE_DURATION;

    std::vector<TimestampAndDuration> expectedTnDList {
        // 1st measure (no notes)

        // 2nd measure
        { secondMeasureTime, QUARTER_NOTE_DURATION* 2 },  // 2 tied C5
        { secondMeasureTime + QUARTER_NOTE_DURATION * 2, HALF_NOTE_DURATION }, // A4 tied to a whole A4 outside of the repeat

        // 2nd measure (repeated)
        { secondMesaureRepeatedTime, QUARTER_NOTE_DURATION* 2 },  // 2 tied C5
        { secondMesaureRepeatedTime + QUARTER_NOTE_DURATION * 2, HALF_NOTE_DURATION + WHOLE_NOTE_DURATION }, // A4 tied to a whole A4 outside of the repeat

        // 3rd measure (there is only the whole tied A4)
    };

    ASSERT_EQ(expectedPitchList.size(), expectedTnDList.size());
    EXPECT_FALSE(result.empty());

    int eventNum = 0;
    for (const auto& pair : result) {
        for (const mpe::PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));

            const TimestampAndDuration& expectedTnD = expectedTnDList.at(eventNum);
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            EXPECT_EQ(pair.first, expectedTnD.timestamp);
            EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, expectedTnD.timestamp);
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, expectedTnD.duration);
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitchList.at(eventNum));

            ++eventNum;
        }
    }

    EXPECT_EQ(eventNum, expectedTnDList.size());

    delete score;
}

/**
 * @brief PlaybackEventsRendererTests_PartialTie
 * @details Checks whether we correctly calculate the duration of partially tied notes
 */
TEST_F(Engraving_PlaybackEventsRendererTests, PartialTie)
{
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "partial_tie.mscx");

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Standard, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Render the score
    PlaybackEventsMap result;

    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* m : repeatSegment->measureList()) {
            for (const Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                const EngravingItem* el = s->element(0);
                if (!el || !el->isChord()) {
                    continue;
                }

                m_renderer.render(toChord(el), tickPositionOffset, m_defaultProfile, ctx, result);
            }
        }
    }

    // [THEN] Expected pitch, time and duration of each event
    std::vector<pitch_level_t> expectedPitchList {
        // 1st measure
        pitchLevel(PitchClass::A, 4),

        // 2nd measure: no notes

        // 3rd measure
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::A, 4), // partial tie to a whole note, also tied to a quarter note in the 4th measure

        // 1st measure (repeated)
        // 2nd measure (repeated)

        // 3rd measure (repeated)
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::F, 4),
        pitchLevel(PitchClass::A, 4),

        // 4th measure (2nd repeat segment): 1 tied quarter A4 note, skip
    };

    timestamp_t thirdMeasureTime = WHOLE_NOTE_DURATION * 2;
    timestamp_t thirdMeasureRepeatedTime = WHOLE_NOTE_DURATION * 3 + WHOLE_NOTE_DURATION * 2;

    std::vector<TimestampAndDuration> expectedTnDList {
        // 1st measure
        { 0, WHOLE_NOTE_DURATION },

        // 2nd measure: no notes

        // 3rd measure
        { thirdMeasureTime, QUARTER_NOTE_DURATION },
        { thirdMeasureTime + QUARTER_NOTE_DURATION, QUARTER_NOTE_DURATION },
        { thirdMeasureTime + QUARTER_NOTE_DURATION * 2, QUARTER_NOTE_DURATION },
        { thirdMeasureTime + QUARTER_NOTE_DURATION * 3, QUARTER_NOTE_DURATION + WHOLE_NOTE_DURATION }, // partial tie to a whole A4 note

        // 1st measure (repeated): skip the whole A4 note
        // 2nd measure (repeated): no notes

        // 3rd measure (repeated)
        { thirdMeasureRepeatedTime,  QUARTER_NOTE_DURATION },
        { thirdMeasureRepeatedTime + QUARTER_NOTE_DURATION, QUARTER_NOTE_DURATION },
        { thirdMeasureRepeatedTime + QUARTER_NOTE_DURATION * 2, QUARTER_NOTE_DURATION },
        { thirdMeasureRepeatedTime + QUARTER_NOTE_DURATION * 3, QUARTER_NOTE_DURATION* 2 }, // also tied to a quarter note in the 4th measure

        // 4th measure (2nd repeat segment): 1 tied quarter A4 note, skip
    };

    ASSERT_EQ(expectedPitchList.size(), expectedTnDList.size());
    EXPECT_FALSE(result.empty());

    int eventNum = 0;
    for (const auto& pair : result) {
        for (const mpe::PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));

            const TimestampAndDuration& expectedTnD = expectedTnDList.at(eventNum);
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            EXPECT_EQ(pair.first, expectedTnD.timestamp);
            EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, expectedTnD.timestamp);
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, expectedTnD.duration);
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitchList.at(eventNum));

            ++eventNum;
        }
    }

    EXPECT_EQ(eventNum, expectedTnDList.size());

    delete score;
}

/**
 * @brief PlaybackEventsRendererTests_TrillLine_TiedNotes
 * Checks that we can render the trill line if it doesn't last the full duration of the tied notes
 * See: https://github.com/musescore/MuseScore/issues/18676
 */
TEST_F(Engraving_PlaybackEventsRendererTests, TrillLine_TiedNotes)
{
    // [GIVEN] Simple piece of score (violin, 4/4, 120 bpm, Treble Cleff)
    Score* score
        = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "trill_line_on_tied_notes.mscx");

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Standard, m_dummyPattern);
    m_defaultProfile->setPattern(ArticulationType::Trill, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [WHEN] Render the score
    PlaybackEventsMap result;

    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (const Segment* segment = measure->first(SegmentType::ChordRest); segment; segment = segment->next(SegmentType::ChordRest)) {
            const mu::engraving::EngravingItem* element = segment->element(0);
            if (element && element->isChord()) {
                m_renderer.render(toChord(element), 0, m_defaultProfile, ctx, result);
            }
        }
    }

    // [THEN] We expect 4 lists of events (for 4 tied notes)
    EXPECT_EQ(result.size(), 4);

    constexpr timestamp_t expectedTrillTimestamp = WHOLE_NOTE_DURATION; // timestamp of the 2nd tied note (where the trill starts)
    constexpr duration_t expectedTrillDuration = WHOLE_NOTE_DURATION * 2;
    constexpr timestamp_t expectedTrillTimestampTo = expectedTrillTimestamp + expectedTrillDuration;

    size_t noteNum = 0;
    size_t trillStartRangeEventCount = 0;
    size_t trillEndRangeEventCount = 0;

    for (const auto& pair : result) {
        for (const PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            // [THEN] We expect that each note event has only one articulation applied
            ASSERT_EQ(noteEvent.expressionCtx().articulations.size(), 1);

            if (noteNum == 0 || noteNum == 3) {
                // [THEN] Normal notes (no trill)
                EXPECT_EQ(pair.second.size(), 1);
                EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));
                EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::A, 4));

                // [THEN] Each note event has the correct duration
                EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, WHOLE_NOTE_DURATION);
            } else {
                // [THEN] Notes under the trill line
                constexpr size_t expectedSubNotes = 32;
                EXPECT_EQ(pair.second.size(), expectedSubNotes);

                // [THEN] Each note event has the correct articulation time and duration
                const ArticulationAppliedData& articulationData = noteEvent.expressionCtx().articulations.at(ArticulationType::Trill);
                EXPECT_EQ(articulationData.meta.timestamp, expectedTrillTimestamp);
                EXPECT_EQ(articulationData.meta.overallDuration, expectedTrillDuration);

                const timestamp_t noteTimestampTo = noteEvent.arrangementCtx().actualTimestamp + noteEvent.arrangementCtx().actualDuration;

                // [THEN] Each note has the correct occupiedFrom/To
                if (noteEvent.arrangementCtx().nominalTimestamp == expectedTrillTimestamp) {
                    EXPECT_EQ(articulationData.occupiedFrom, 0);
                    EXPECT_NE(articulationData.occupiedTo, HUNDRED_PERCENT);
                    trillStartRangeEventCount++;
                } else if (noteTimestampTo == expectedTrillTimestampTo) {
                    EXPECT_NE(articulationData.occupiedFrom, 0);
                    EXPECT_EQ(articulationData.occupiedTo, HUNDRED_PERCENT);
                    trillEndRangeEventCount++;
                } else {
                    EXPECT_NE(articulationData.occupiedFrom, 0);
                    EXPECT_NE(articulationData.occupiedTo, HUNDRED_PERCENT);
                }

                // [THEN] Each note event has the correct duration
                constexpr duration_t expectedSubNoteDuration = WHOLE_NOTE_DURATION / expectedSubNotes;
                EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, expectedSubNoteDuration);
            }
        }

        ++noteNum;
    }

    // [THEN] We expect only 1 start & end event for the trill
    EXPECT_EQ(trillStartRangeEventCount, 1);
    EXPECT_EQ(trillEndRangeEventCount, 1);

    delete score;
}

/**
 * @brief PlaybackEventsRendererTests_Trill_TiedNotes
 * Checks that we can render the normal trill (ornament) if it starts from a tied note
 * See: https://github.com/musescore/MuseScore/issues/18676
 * See: https://github.com/musescore/MuseScore/issues/27472
 */
TEST_F(Engraving_PlaybackEventsRendererTests, Trill_TiedNotes)
{
    // [GIVEN] Simple piece of score (violin, 4/4, 120 bpm, Treble Cleff)
    Score* score
        = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "trill_on_tied_notes.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->repeatList().size(), 2);

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Trill, m_dummyPattern);

    // [GIVEN] Dummy context
    PlaybackContextPtr ctx = std::make_shared<PlaybackContext>();

    // [GIVEN] The chord we want to render
    const Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    const Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    const ChordRest* cr = firstSegment->nextChordRest(0);
    ASSERT_TRUE(cr && cr->isChord());

    const Chord* chord = toChord(cr);
    ASSERT_EQ(chord->notes().size(), 1);
    ASSERT_EQ(chord->articulations().size(), 1);

    const Articulation* articulation = chord->articulations().front();
    ASSERT_TRUE(articulation->isOrnament());
    ASSERT_EQ(toOrnament(articulation)->symId(), SymId::ornamentTrill);

    // [WHEN] Render the chord
    PlaybackEventsMap result;
    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;
        m_renderer.render(toChord(chord), tickPositionOffset, m_defaultProfile, ctx, result);
    }

    // [THEN] We expect 2 lists of events (for each repeat segment)
    EXPECT_EQ(result.size(), score->repeatList().size());

    constexpr duration_t expectedTrillDuration = WHOLE_NOTE_DURATION * 2; // 2 tied notes
    const std::vector<timestamp_t> expectedTrillTimestamps {
        0,
        expectedTrillDuration, // 2nd repeat segment
    };

    size_t repeatSegmentNum = 0;
    size_t trillStartRangeEventCount = 0;
    size_t trillEndRangeEventCount = 0;

    for (const auto& pair : result) {
        // [THEN] Sub-notes of the trill
        constexpr size_t expectedSubNotes = 64;
        EXPECT_EQ(pair.second.size(), expectedSubNotes);

        const timestamp_t expectedTrillTimestamp = expectedTrillTimestamps.at(repeatSegmentNum);
        const timestamp_t expectedTrillTimestampTo = expectedTrillTimestamp + expectedTrillDuration;
        ++repeatSegmentNum;

        for (const PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            // [THEN] We expect that each note event has only one articulation applied
            ASSERT_EQ(noteEvent.expressionCtx().articulations.size(), 1);

            // [THEN] Each note event has the correct articulation time and duration
            const ArticulationAppliedData& articulationData = noteEvent.expressionCtx().articulations.at(ArticulationType::Trill);
            EXPECT_EQ(articulationData.meta.timestamp, expectedTrillTimestamp);
            EXPECT_EQ(articulationData.meta.overallDuration, expectedTrillDuration);

            const timestamp_t noteTimestampTo = noteEvent.arrangementCtx().actualTimestamp + noteEvent.arrangementCtx().actualDuration;

            // [THEN] Each note has the correct occupiedFrom/To
            if (noteEvent.arrangementCtx().nominalTimestamp == expectedTrillTimestamp) {
                EXPECT_EQ(articulationData.occupiedFrom, 0);
                EXPECT_NE(articulationData.occupiedTo, HUNDRED_PERCENT);
                trillStartRangeEventCount++;
            } else if (noteTimestampTo == expectedTrillTimestampTo) {
                EXPECT_NE(articulationData.occupiedFrom, 0);
                EXPECT_EQ(articulationData.occupiedTo, HUNDRED_PERCENT);
                trillEndRangeEventCount++;
            } else {
                EXPECT_NE(articulationData.occupiedFrom, 0);
                EXPECT_NE(articulationData.occupiedTo, HUNDRED_PERCENT);
            }

            // [THEN] Each note event has the correct duration
            constexpr duration_t expectedSubNoteDuration = expectedTrillDuration / expectedSubNotes;
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, expectedSubNoteDuration);
        }
    }

    // [THEN] We expect 2 start & end event for each trill
    EXPECT_EQ(trillStartRangeEventCount, 2);
    EXPECT_EQ(trillEndRangeEventCount, 2);

    delete score;
}

TEST_F(Engraving_PlaybackEventsRendererTests, CountIn)
{
    Score* score = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "count_in.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Standard, m_dummyPattern);

    // [GIVEN] Anacrusis measure (1/4)
    const Measure* anacrusisMeasure_1_4 = score->firstMeasure();
    ASSERT_TRUE(anacrusisMeasure_1_4);

    // [WHEN] Render the anacrusis measure
    PlaybackEventsMap result;
    duration_t totalCountInDuration = 0;
    m_renderer.renderCountIn(score, anacrusisMeasure_1_4->tick().ticks(), 0, m_defaultProfile, result, totalCountInDuration);

    // [THEN] 7 quarter note events
    EXPECT_EQ(result.size(), 7);
    EXPECT_EQ(totalCountInDuration, QUARTER_NOTE_DURATION * 7);

    timestamp_t expectedTimestamp = 0;

    for (const auto& pair : result) {
        EXPECT_EQ(pair.second.size(), 1);

        for (const PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, QUARTER_NOTE_DURATION);
            EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, expectedTimestamp);

            expectedTimestamp += QUARTER_NOTE_DURATION;
        }
    }

    // [GIVEN] Second measure (4/4)
    const Measure* secondMeasure_4_4 = anacrusisMeasure_1_4->nextMeasure();
    ASSERT_TRUE(secondMeasure_4_4);

    // [WHEN] Render Count In events starting from the beginning of the measure (4/4)
    result.clear();
    totalCountInDuration = 0;
    m_renderer.renderCountIn(score, secondMeasure_4_4->tick().ticks(), 0, m_defaultProfile, result, totalCountInDuration);

    // [THEN] 4 quarter note events
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(totalCountInDuration, QUARTER_NOTE_DURATION * 4);

    expectedTimestamp = 0;

    for (const auto& pair : result) {
        EXPECT_EQ(pair.second.size(), 1);

        for (const PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, QUARTER_NOTE_DURATION);
            EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, expectedTimestamp);

            expectedTimestamp += QUARTER_NOTE_DURATION;
        }
    }

    // [WHEN] Render the same measure (4/4), but starting from the 3rd beat
    result.clear();
    totalCountInDuration = 0;
    m_renderer.renderCountIn(score, secondMeasure_4_4->tick().ticks() + 480 + 480, 0, m_defaultProfile, result, totalCountInDuration);

    // [THEN] 6 quarter note events
    EXPECT_EQ(result.size(), 6);
    EXPECT_EQ(totalCountInDuration, QUARTER_NOTE_DURATION * 6);

    expectedTimestamp = 0;

    for (const auto& pair : result) {
        EXPECT_EQ(pair.second.size(), 1);

        for (const PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, QUARTER_NOTE_DURATION);
            EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, expectedTimestamp);

            expectedTimestamp += QUARTER_NOTE_DURATION;
        }
    }

    // [GIVEN] The next measure we want to render (3/8)
    const Measure* thirdMeasure_3_8 = secondMeasure_4_4->nextMeasure();
    ASSERT_TRUE(thirdMeasure_3_8);

    // [WHEN] Render the 3rd measure (3/8)
    result.clear();
    totalCountInDuration = 0;
    m_renderer.renderCountIn(score, thirdMeasure_3_8->tick().ticks(), 0, m_defaultProfile, result, totalCountInDuration);

    // [THEN] 3 quaver note events
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(totalCountInDuration, QUAVER_NOTE_DURATION * 3);

    expectedTimestamp = 0;

    for (const auto& pair : result) {
        EXPECT_EQ(pair.second.size(), 1);

        for (const PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, QUAVER_NOTE_DURATION);
            EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, expectedTimestamp);

            expectedTimestamp += QUAVER_NOTE_DURATION;
        }
    }

    // [WHEN] Render the 3rd measure (3/8) from some random tick that is not equal to any beat position
    result.clear();
    totalCountInDuration = 0;
    m_renderer.renderCountIn(score, thirdMeasure_3_8->tick().ticks() + 333, 0, m_defaultProfile, result, totalCountInDuration); // tick: 240 + 93

    // [THEN] 5 quaver note events (3 + 2)
    EXPECT_EQ(result.size(), 5);
    EXPECT_EQ(totalCountInDuration, QUAVER_NOTE_DURATION * 5);

    expectedTimestamp = 0;

    for (const auto& pair : result) {
        EXPECT_EQ(pair.second.size(), 1);

        for (const PlaybackEvent& event : pair.second) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
            EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, QUAVER_NOTE_DURATION);
            EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, expectedTimestamp);

            expectedTimestamp += QUAVER_NOTE_DURATION;
        }
    }

    delete score;
}
