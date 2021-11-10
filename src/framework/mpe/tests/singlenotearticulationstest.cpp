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

#include "mpe/events.h"
#include "mpe/tests/utils/articulationutils.h"

using namespace mu;
using namespace mu::mpe;
using namespace mu::mpe::tests;

class SingleNoteArticulationsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // [GIVEN] Nominal arrangement data of the very first quarter note on the score, with the 120BPM tempo and 4/4 time signature
        m_nominalTimestamp = 0; // msecs
        m_nominalDuration = 500; // msecs
        m_voiceIdx = 0; // first voice

        // [GIVEN] Pitch data of the note
        m_pitchClass = PitchClass::A;
        m_octave = 4;

        // [GIVEN] Expression data of the note - no dynamic/articulations modificators
        m_dynamic = DynamicType::Natural;

        // [GIVEN] Articulation pattern "None", which means that note should be played without any modifications
        m_standardPattern.arrangementPattern = createArrangementPattern(0.f /*duration_factor*/, 0 /*timestamp_offset*/);
        m_standardPattern.pitchPattern = createSimplePitchPattern(0.f /*increment_pitch_diff*/);
        m_standardPattern.expressionPattern = createSimpleExpressionPattern(dynamicLevelFromType(DynamicType::Natural));
    }

    timestamp_t m_nominalTimestamp;
    duration_t m_nominalDuration;
    voice_layer_idx_t m_voiceIdx;

    PitchClass m_pitchClass;
    octave_t m_octave;

    DynamicType m_dynamic;

    ArticulationPatternSegment m_standardPattern;
};

/**
 * @brief SingleNoteArticulationsTest_StandardPattern
 * @details In this case we're gonna build a simple note event without any articulation applied on the top of it
 *          So the actual PitchCurve and ExpressionCurve would be equal to the "StandardPattern" from the articulation Profile
 */
TEST_F(SingleNoteArticulationsTest, StandardPattern)
{
    // [GIVEN] No articulations applied on the top of the note
    ArticulationMap appliedArticulations = {};

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_standardPattern,
                    m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_pitchClass,
                    m_octave,
                    m_dynamic,
                    appliedArticulations);

    // [THEN] We expect that expression curve will be using default curve from standard patterns
    EXPECT_EQ(event.expressionCtx().expressionCurve,
              m_standardPattern.expressionPattern.dynamicOffsetMap);

    // [THEN] We expect that there is no pitch offset at all on this note
    for (const auto& pair : event.pitchCtx().pitchCurve) {
        EXPECT_EQ(pair.second, 0.f);
    }
}

/**
 * @brief SingleNoteArticulationsTest_StaccatoPattern
 * @details In this case we're gonna build a simple note event with the staccato articulation applied on the top of it
 *          So the actual arrangement context of the note would reflect the parameters from staccato articulation pattern
 */
TEST_F(SingleNoteArticulationsTest, StaccatoPattern)
{
    // [GIVEN] Articulation pattern "Staccato", which instructs a performer to shorten duration of a note
    ArticulationPatternSegment staccatoArticulation;
    staccatoArticulation.arrangementPattern = createArrangementPattern(0.5 /*duration_factor*/, 0 /*timestamp_offset*/);
    staccatoArticulation.pitchPattern = createSimplePitchPattern(0.f /*increment_pitch_diff*/);
    staccatoArticulation.expressionPattern = createSimpleExpressionPattern(0.f /* no dynamic changes comparing to the standard one*/);

    ArticulationPattern scope;
    scope.emplace(0.f, staccatoArticulation);

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationData staccatoApplied(ArticulationType::Staccato, scope, 0.f, 1.f);

    ArticulationMap appliedArticulations = {};
    appliedArticulations.emplace(ArticulationType::Staccato, staccatoApplied);

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_standardPattern,
                    m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_pitchClass,
                    m_octave,
                    m_dynamic,
                    appliedArticulations);

    // [THEN] We expect the nominal duration of a note to be unchanged
    EXPECT_EQ(event.arrangementCtx().nominalDuration, m_nominalDuration);

    // [THEN] We expect the actual duration of the staccato note to be halved.
    EXPECT_EQ(event.arrangementCtx().actualDuration, m_nominalDuration * 0.5);
}

/**
 * @brief SingleNoteArticulationsTest_AccentPattern
 * @details In this case we're gonna build a simple note event with the accent articulation applied on the top of it
 *          So the actual expression context of the note would reflect the parameters from articulation pattern
 */
TEST_F(SingleNoteArticulationsTest, AccentPattern)
{
    // [GIVEN] Articulation pattern "Accent", which instructs a performer to play a note louder (usually on 1 dynamic level)
    ArticulationPatternSegment accentArticulation;
    accentArticulation.arrangementPattern = createArrangementPattern(1.f /*duration_factor*/, 0 /*timestamp_offset*/);
    accentArticulation.pitchPattern = createSimplePitchPattern(0.f /*increment_pitch_diff*/);
    accentArticulation.expressionPattern = createSimpleExpressionPattern(
        DYNAMIC_LEVEL_STEP /* increasing a note's dynamic on a single level*/);

    ArticulationPattern scope;
    scope.emplace(0.f, accentArticulation);

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationData accentApplied(ArticulationType::Accent, scope, 0.f, 1.f);

    ArticulationMap appliedArticulations = {};
    appliedArticulations.emplace(ArticulationType::Accent, accentApplied);

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_standardPattern,
                    m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_pitchClass,
                    m_octave,
                    m_dynamic,
                    appliedArticulations);

    // [THEN] We expect the nominal duration of a note to be unchanged,
    //        since accent doesn't affect any arrangement data
    EXPECT_EQ(event.arrangementCtx().nominalDuration, m_nominalDuration);

    // [THEN] We expect the applied dynamic type is still unchanged
    EXPECT_EQ(event.expressionCtx().dynamic, DynamicType::Natural);

    // [THEN] However, an amplitude dynamic value in ExpressionCurve has been increased on single level due to Accent Pattern
    EXPECT_EQ(event.expressionCtx().expressionCurve.maxAmplitudeLevel(), dynamicLevelFromType(DynamicType::mf));
}

/**
 * @brief SingleNoteArticulationsTest_PocoTenuto
 * @details In this case we're gonna build a simple note event with the combination of staccato and tenuto articulations
 *          applied on the top of it. So the actual expression context of the note would reflect the average parameters
 *          from articulation patterns
 */
TEST_F(SingleNoteArticulationsTest, PocoTenuto)
{
    // [GIVEN] Articulation pattern "Staccato", which instructs a performer to shorten duration of a note
    ArticulationPatternSegment staccatoPattern;
    staccatoPattern.arrangementPattern = createArrangementPattern(0.5 /*duration_factor*/, 0 /*timestamp_offset*/);
    staccatoPattern.pitchPattern = createSimplePitchPattern(0.f /*increment_pitch_diff*/);
    staccatoPattern.expressionPattern = createSimpleExpressionPattern(0.f /* no dynamic changes comparing to the standard one*/);

    ArticulationPattern staccatoScope;
    staccatoScope.emplace(0.f, staccatoPattern);

    // [GIVEN] Articulation pattern "Tenuto", which instructs a performer to play a note for the whole nominal duration
    ArticulationPatternSegment tenutoPattern;
    tenutoPattern.arrangementPattern = createArrangementPattern(1.f /*duration_factor*/, 0 /*timestamp_offset*/);
    tenutoPattern.pitchPattern = createSimplePitchPattern(0.f /*increment_pitch_diff*/);
    tenutoPattern.expressionPattern = createSimpleExpressionPattern(0.f /* no dynamic changes comparing to the standard one*/);

    ArticulationPattern tenutoScope;
    tenutoScope.emplace(0.f, tenutoPattern);

    ArticulationMap appliedArticulations = {};

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationData staccatoApplied(ArticulationType::Staccato, staccatoScope, 0.f, 1.f);
    appliedArticulations.emplace(ArticulationType::Staccato, staccatoApplied);

    // [GIVEN] Tenuto articulation applied on the note
    ArticulationData tenutoApplied(ArticulationType::Tenuto, tenutoScope, 0.f, 1.f);
    appliedArticulations.emplace(ArticulationType::Tenuto, tenutoApplied);

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_standardPattern,
                    m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_pitchClass,
                    m_octave,
                    m_dynamic,
                    appliedArticulations);

    // [THEN] We expect the nominal duration of a note to be unchanged
    EXPECT_EQ(event.arrangementCtx().nominalDuration, m_nominalDuration);

    // [THEN] We expect the actual duration of the note marked by staccato + tenuto to be equal to 0.75 of nominal duration
    EXPECT_EQ(event.arrangementCtx().actualDuration, m_nominalDuration * 0.75);
}

/**
 * @brief SingleNoteArticulationsTest_QuickFall
 * @details In this case we're gonna build a simple note event with the quick fall articulation
 *          applied on the top of it. So the actual pitch context of the note would reflect the
 *          parameters from articulation pattern
 */
TEST_F(SingleNoteArticulationsTest, QuickFall)
{
    // [GIVEN] Articulation pattern "Tenuto", which instructs a performer to play a note for the whole nominal duration
    ArticulationPatternSegment quickFallPattern;
    quickFallPattern.arrangementPattern = createArrangementPattern(1.f /*duration_factor*/, 0 /*timestamp_offset*/);

    // Linear decreasing pitch
    quickFallPattern.pitchPattern
        = createSimplePitchPattern(-PITCH_LEVEL_STEP / (1.f / PERCENTAGE_PRECISION_STEP) /*increment_pitch_diff*/);
    quickFallPattern.expressionPattern = createSimpleExpressionPattern(0.f /* no dynamic changes comparing to the standard one*/);

    ArticulationPattern quickFallScope;
    quickFallScope.emplace(0.f, quickFallPattern);

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationData quickFallApplied(ArticulationType::QuickFall, quickFallScope, 0.f, 1.f);

    ArticulationMap appliedArticulations = {};
    appliedArticulations.emplace(ArticulationType::QuickFall, quickFallApplied);

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_standardPattern,
                    m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_pitchClass,
                    m_octave,
                    m_dynamic,
                    appliedArticulations);

    // [THEN] We expect the pitch curve of the note marked by quick fall articulation to be equal to the pitch curve from corresponding pattern
    EXPECT_EQ(event.pitchCtx().pitchCurve, quickFallPattern.pitchPattern.pitchOffsetMap);
}

/**
 * @brief SingleNoteArticulationsTest_Scoop
 * @details In this case we're gonna build a simple note event with the scoop articulation
 *          applied on the top of it. So the actual pitch context of the note would reflect the
 *          parameters from articulation pattern
 */
TEST_F(SingleNoteArticulationsTest, Scoop)
{
    // [GIVEN] Articulation pattern "Scoop", which instructs a performer to play ahead of a nominal duration,
    //         starting at a lower pitch, and then placing it on the note being played.
    duration_percentage_t timestampOffset = -0.25;
    m_nominalTimestamp = 1000; //msecs

    ArticulationPatternSegment scoopPattern;
    scoopPattern.arrangementPattern = createArrangementPattern(1.f /*duration_factor*/, timestampOffset /*timestamp_offset*/);

    // Linear increasing pitch
    scoopPattern.pitchPattern
        = createSimplePitchPattern(EXPECTED_SIZE /*increment_pitch_diff*/);
    scoopPattern.expressionPattern = createSimpleExpressionPattern(0.f /* no dynamic changes comparing to the standard one*/);

    ArticulationPattern scope;
    scope.emplace(0.f, scoopPattern);

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationData scoopApplied(ArticulationType::Scoop, scope, 0.f, 1.f);

    ArticulationMap appliedArticulations = {};
    appliedArticulations.emplace(ArticulationType::Scoop, scoopApplied);

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_standardPattern,
                    m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_pitchClass,
                    m_octave,
                    m_dynamic,
                    appliedArticulations);

    // [THEN] We expect the pitch curve of the note marked by scoop articulation to be equal to the pitch curve from corresponding pattern
    EXPECT_EQ(event.pitchCtx().pitchCurve, scoopPattern.pitchPattern.pitchOffsetMap);

    // [THEN] We expect that actual timestamp of the note will consider timestamp offset from the articulation pattern
    //        In other words, we'll start to playback a note with pitch offset and then finally land on the note being played
    EXPECT_EQ(event.arrangementCtx().actualTimestamp, m_nominalTimestamp + m_nominalDuration * timestampOffset);
}
