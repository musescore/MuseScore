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
#include <memory>

#include "mpe/tests/utils/articulationutils.h"

#include "dom/factory.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/segment.h"
#include "dom/harppedaldiagram.h"

#include "utils/scorerw.h"
#include "playback/playbackeventsrenderer.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::mpe;

static const String PLAYBACK_EVENTS_RENDERING_DIR("playbackeventsrenderer_data/");
static constexpr duration_t QUARTER_NOTE_DURATION = 500000; // duration in microseconds for 4/4 120BPM
static constexpr duration_t QUAVER_NOTE_DURATION = QUARTER_NOTE_DURATION / 2; // duration in microseconds for 4/4 120BPM
static constexpr duration_t SEMI_QUAVER_NOTE_DURATION = QUAVER_NOTE_DURATION / 2; // duration in microseconds for 4/4 120BPM
static constexpr duration_t DEMI_SEMI_QUAVER_NOTE_DURATION = QUARTER_NOTE_DURATION / 8; // duration in microseconds for 4/4 120BPM
static constexpr duration_t WHOLE_NOTE_DURATION = QUARTER_NOTE_DURATION * 4;    // duration in microseconds for 4/4 120BPM

class Engraving_PlaybackEventsRendererTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_dummyPatternSegment.arrangementPattern
            = tests::createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
        m_dummyPatternSegment.pitchPattern = tests::createSimplePitchPattern(0 /*increment_pitch_diff*/);
        m_dummyPatternSegment.expressionPattern = tests::createSimpleExpressionPattern(dynamicLevelFromType(mu::mpe::DynamicType::Natural));
        m_dummyPattern.emplace(0, m_dummyPatternSegment);

        m_defaultProfile = std::make_shared<ArticulationsProfile>();
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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    // [THEN] We expect that a single note event will be rendered from the chord
    EXPECT_EQ(result.size(), 1);

    mu::mpe::NoteEvent event = std::get<mu::mpe::NoteEvent>(result.begin()->second.front());

    // [THEN] We expect that the note event will match time expectations of the very first quarter note with 120BPM tempo
    EXPECT_EQ(event.arrangementCtx().nominalTimestamp, 0);
    EXPECT_EQ(event.arrangementCtx().nominalDuration, QUARTER_NOTE_DURATION);

    // [THEN] We expect that the note event will match pitch expectations of F4 quarter note
    EXPECT_EQ(event.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 4));

    // [THEN] We expect that the note event will match expression expectations of our note
    EXPECT_EQ(event.expressionCtx().nominalDynamicLevel, dynamicLevelFromType(mu::mpe::DynamicType::Natural));
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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    // [THEN] We expect that a single note event will be rendered from the chord
    EXPECT_EQ(result.size(), 1);

    mu::mpe::NoteEvent event = std::get<mu::mpe::NoteEvent>(result.begin()->second.front());

    // [THEN] We expect that the note event will match time expectations of the very first quarter note with 120BPM tempo
    EXPECT_EQ(event.arrangementCtx().nominalTimestamp, 0);
    EXPECT_EQ(event.arrangementCtx().nominalDuration, QUARTER_NOTE_DURATION);

    // [THEN] We expect that the note event will match pitch expectations of F4 quarter note
    EXPECT_EQ(event.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 4));

    // [THEN] We expect that the note event will match expression expectations of our note
    EXPECT_EQ(event.expressionCtx().nominalDynamicLevel, dynamicLevelFromType(mu::mpe::DynamicType::Natural));
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

    // [WHEN] Request to render the rest
    PlaybackEventsMap result;
    m_renderer.render(rest, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedTrillSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedTrillSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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
 */
TEST_F(Engraving_PlaybackEventsRendererTests, TwoNotes_Continuous_Glissando)
{
    // [GIVEN] Simple piece of score (piano, 4/4, 120 bpm, Treble Cleff)
    Score* score
        = ScoreRW::readScore(PLAYBACK_EVENTS_RENDERING_DIR + "two_notes_continuous_glissando/two_notes_continuous_glissando.mscx");

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
    m_defaultProfile->setPattern(ArticulationType::ContinuousGlissando, m_dummyPattern);

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Continuous Glissando
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::ContinuousGlissando));

            // [THEN] We expect that each sub-note has an expected duration
            EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, expectedDuration);
            EXPECT_EQ(noteEvent.arrangementCtx().nominalTimestamp, static_cast<duration_t>(i) * expectedDuration);

            // [THEN] We expect that each note event will match expected pitch disclosure
            EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, expectedPitches.at(i));
        }
    }
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

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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
    m_renderer.render(secondChord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result2);

    for (const auto& pair : result2) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Acciaccatura, m_dummyPattern);

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Acciaccatura
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Acciaccatura));

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

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Acciaccatura, m_dummyPattern);

    // [WHEN] Request to render a chord with the F4 note on it
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Acciaccatura
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Acciaccatura));

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

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Acciaccatura, m_dummyPattern);

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied - Acciaccatura
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Acciaccatura));

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

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::PostAppoggiatura, m_dummyPattern);

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::PostAppoggiatura));

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

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::PostAppoggiatura, m_dummyPattern);

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::PostAppoggiatura));

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

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::PostAppoggiatura, m_dummyPattern);

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

            // [THEN] We expect that each note event has only one articulation applied
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::PostAppoggiatura));

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
 *          which starts with the F4+A4+C4 quarter chord marked by the Arpeggio articulation
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
        pitchLevel(PitchClass::C, 5)
    };

    // [GIVEN] Fulfill articulations profile with dummy patterns
    m_defaultProfile->setPattern(ArticulationType::Arpeggio, m_dummyPattern);

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    for (const auto& pair : result) {
        // [THEN] We expect that rendered note events number will match expectations
        EXPECT_EQ(pair.second.size(), expectedSubNotesCount);

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(pair.second.at(i));

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

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    // [THEN] We expect that rendered note events number will match expectations
    EXPECT_EQ(result.begin()->second.size(), expectedChordsCount * expectedSubNotesCount);

    for (int chordIdx = 0; chordIdx < expectedChordsCount; ++chordIdx) {
        for (int noteIdx = 0; noteIdx < expectedSubNotesCount; ++noteIdx) {
            int eventIdx = (chordIdx * expectedSubNotesCount) + noteIdx;

            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(result.begin()->second.at(eventIdx));

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

    // [WHEN] Request to render a chord
    PlaybackEventsMap result;
    m_renderer.render(chord, dynamicLevelFromType(mu::mpe::DynamicType::Natural),
                      ArticulationType::Standard, m_defaultProfile, result);

    // [THEN] We expect that rendered note events number will match expectations
    EXPECT_EQ(result.begin()->second.size(), expectedChordsCount * chordNotesCount);

    for (int chordIdx = 0; chordIdx < expectedChordsCount; ++chordIdx) {
        for (int noteIdx = 0; noteIdx < chordNotesCount; ++noteIdx) {
            int eventIdx = (chordIdx * chordNotesCount) + noteIdx;

            const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(result.begin()->second.at(eventIdx));

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
