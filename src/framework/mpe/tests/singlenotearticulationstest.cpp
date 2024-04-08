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

using namespace muse;
using namespace muse::mpe;
using namespace muse::mpe::tests;

class MPE_SingleNoteArticulationsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // [GIVEN] Nominal arrangement data of the very first quarter note on the score, with the 120BPM tempo and 4/4 time signature
        m_nominalTimestamp = 0; // msecs
        m_nominalDuration = 500; // msecs
        m_voiceIdx = 0; // first voice
        m_staffIdx = 0; // first staff

        // [GIVEN] Pitch data of the note
        m_pitchClass = PitchClass::A;
        m_octave = 4;

        // [GIVEN] Expression data of the note - no dynamic/articulations modifiers
        m_nominalDynamic = dynamicLevelFromType(DynamicType::Natural);

        // [GIVEN] Articulation pattern "None", which means that note should be played without any modifications
        m_standardPattern.arrangementPattern = createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
        m_standardPattern.pitchPattern = createSimplePitchPattern(0 /*increment_pitch_diff*/);
        m_standardPattern.expressionPattern = createSimpleExpressionPattern(dynamicLevelFromType(DynamicType::Natural));
    }

    timestamp_t m_nominalTimestamp;
    duration_t m_nominalDuration;
    voice_layer_idx_t m_voiceIdx;
    staff_layer_idx_t m_staffIdx;

    PitchClass m_pitchClass;
    octave_t m_octave;

    dynamic_level_t m_nominalDynamic;

    ArticulationPatternSegment m_standardPattern;
};

/**
 * @brief MPE_SingleNoteArticulationsTest_StandardPattern
 * @details In this case we're gonna build a simple note event without any articulation applied on the top of it
 *          So the actual PitchCurve and ExpressionCurve would be equal to the "StandardPattern" from the articulation Profile
 */
TEST_F(MPE_SingleNoteArticulationsTest, StandardPattern)
{
    // [GIVEN] No articulations applied on the top of the note
    ArticulationPattern scope;
    scope.emplace(0, m_standardPattern);

    ArticulationMeta standardMeta;
    standardMeta.type = ArticulationType::Staccato;
    standardMeta.pattern = scope;
    standardMeta.timestamp = m_nominalTimestamp;
    standardMeta.overallDuration = m_nominalDuration;

    ArticulationAppliedData standardArticulationApplied(std::move(standardMeta), 0, HUNDRED_PERCENT);

    ArticulationMap appliedArticulations;
    appliedArticulations.emplace(ArticulationType::Standard, std::move(standardArticulationApplied));
    appliedArticulations.preCalculateAverageData();

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_staffIdx,
                    pitchLevel(m_pitchClass, m_octave),
                    m_nominalDynamic,
                    std::move(appliedArticulations),
                    0);

    // [THEN] We expect that expression curve will be using default curve from standard patterns
    EXPECT_EQ(event.expressionCtx().expressionCurve,
              m_standardPattern.expressionPattern.dynamicOffsetMap);

    // [THEN] We expect that there is no pitch offset at all on this note
    for (const auto& pair : event.pitchCtx().pitchCurve) {
        EXPECT_EQ(pair.second, 0);
    }
}

/**
 * @brief MPE_SingleNoteArticulationsTest_StaccatoPattern
 * @details In this case we're gonna build a simple note event with the staccato articulation applied on the top of it
 *          So the actual arrangement context of the note would reflect the parameters from staccato articulation pattern
 */
TEST_F(MPE_SingleNoteArticulationsTest, StaccatoPattern)
{
    // [GIVEN] Articulation pattern "Staccato", which instructs a performer to shorten duration of a note
    ArticulationPatternSegment staccatoArticulation;
    staccatoArticulation.arrangementPattern = createArrangementPattern(5 * TEN_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
    staccatoArticulation.pitchPattern = createSimplePitchPattern(0 /*increment_pitch_diff*/);
    staccatoArticulation.expressionPattern
        = createSimpleExpressionPattern(m_nominalDynamic /* no dynamic changes comparing to the standard one*/);

    ArticulationPattern scope;
    scope.emplace(0, staccatoArticulation);

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationMeta staccatoMeta;
    staccatoMeta.type = ArticulationType::Staccato;
    staccatoMeta.pattern = scope;
    staccatoMeta.timestamp = m_nominalTimestamp;
    staccatoMeta.overallDuration = m_nominalDuration;

    ArticulationAppliedData staccatoApplied(std::move(staccatoMeta), 0, HUNDRED_PERCENT);

    ArticulationMap appliedArticulations = {};
    appliedArticulations.emplace(ArticulationType::Staccato, std::move(staccatoApplied));
    appliedArticulations.preCalculateAverageData();

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_staffIdx,
                    pitchLevel(m_pitchClass, m_octave),
                    m_nominalDynamic,
                    std::move(appliedArticulations),
                    0);

    // [THEN] We expect the nominal duration of a note to be unchanged
    EXPECT_EQ(event.arrangementCtx().nominalDuration, m_nominalDuration);

    // [THEN] We expect the actual duration of the staccato note to be halved.
    EXPECT_EQ(event.arrangementCtx().actualDuration, m_nominalDuration * 0.5);
}

/**
 * @brief MPE_SingleNoteArticulationsTest_AccentPattern
 * @details In this case we're gonna build a simple note event with the accent articulation applied on the top of it
 *          So the actual expression context of the note would reflect the parameters from articulation pattern
 */
TEST_F(MPE_SingleNoteArticulationsTest, AccentPattern)
{
    // [GIVEN] Articulation pattern "Accent", which instructs a performer to play a note louder (usually on 1 dynamic level)
    ArticulationPatternSegment accentArticulation;
    accentArticulation.arrangementPattern = createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
    accentArticulation.pitchPattern = createSimplePitchPattern(0 /*increment_pitch_diff*/);
    accentArticulation.expressionPattern = createSimpleExpressionPattern(
        dynamicLevelFromType(DynamicType::mf) /* increasing a note's dynamic on a single level from Natural dynamic*/);

    ArticulationPattern scope;
    scope.emplace(0, accentArticulation);

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationMeta accentMeta;
    accentMeta.type = ArticulationType::Accent;
    accentMeta.pattern = scope;
    accentMeta.timestamp = m_nominalTimestamp;
    accentMeta.overallDuration = m_nominalDuration;

    ArticulationAppliedData accentApplied(std::move(accentMeta), 0, HUNDRED_PERCENT);

    ArticulationMap appliedArticulations = {};
    appliedArticulations.emplace(ArticulationType::Accent, std::move(accentApplied));
    appliedArticulations.preCalculateAverageData();

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_staffIdx,
                    pitchLevel(m_pitchClass, m_octave),
                    m_nominalDynamic,
                    std::move(appliedArticulations),
                    0);

    // [THEN] We expect the nominal duration of a note to be unchanged,
    //        since accent doesn't affect any arrangement data
    EXPECT_EQ(event.arrangementCtx().nominalDuration, m_nominalDuration);

    // [THEN] We expect the nominal dynamic type is still unchanged
    EXPECT_EQ(event.expressionCtx().nominalDynamicLevel, dynamicLevelFromType(DynamicType::Natural));

    // [THEN] However, an amplitude dynamic value in ExpressionCurve has been increased on single level due to Accent Pattern
    EXPECT_EQ(event.expressionCtx().expressionCurve.maxAmplitudeLevel(), dynamicLevelFromType(DynamicType::mf));
}

/**
 * @brief MPE_SingleNoteArticulationsTest_AccentPattern
 * @details In this case we're gonna build a simple note event with the accent articulation applied on the top of it
 *          So the actual expression context of the note would reflect the parameters from articulation pattern.
 *
 *          The note is marked by "mezzo forte" dynamic already, so accent articulation should increase this value up to one level - "fortissimo"
 */
TEST_F(MPE_SingleNoteArticulationsTest, AccentPattern_Nominal_MezzoForte)
{
    // [GIVEN] The note marked by mezzo forte dynamic
    m_nominalDynamic = dynamicLevelFromType(DynamicType::mf);

    // [GIVEN] Articulation pattern "Accent", which instructs a performer to play a note louder (usually on 1 dynamic level)
    ArticulationPatternSegment accentArticulation;
    accentArticulation.arrangementPattern = createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
    accentArticulation.pitchPattern = createSimplePitchPattern(0 /*increment_pitch_diff*/);
    accentArticulation.expressionPattern = createSimpleExpressionPattern(
        dynamicLevelFromType(DynamicType::Natural)
        + DYNAMIC_LEVEL_STEP /* increasing a note's dynamic on a single level from Natural dynamic*/);

    ArticulationPattern scope;
    scope.emplace(0, accentArticulation);

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationMeta accentMeta;
    accentMeta.type = ArticulationType::Accent;
    accentMeta.pattern = scope;
    accentMeta.timestamp = m_nominalTimestamp;
    accentMeta.overallDuration = m_nominalDuration;

    ArticulationAppliedData accentApplied(std::move(accentMeta), 0, HUNDRED_PERCENT);

    ArticulationMap appliedArticulations = {};
    appliedArticulations.emplace(ArticulationType::Accent, std::move(accentApplied));
    appliedArticulations.preCalculateAverageData();

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_staffIdx,
                    pitchLevel(m_pitchClass, m_octave),
                    m_nominalDynamic,
                    std::move(appliedArticulations),
                    0);

    // [THEN] We expect the nominal duration of a note to be unchanged,
    //        since accent doesn't affect any arrangement data
    EXPECT_EQ(event.arrangementCtx().nominalDuration, m_nominalDuration);

    // [THEN] We expect the nominal dynamic type is still unchanged
    EXPECT_EQ(event.expressionCtx().nominalDynamicLevel, dynamicLevelFromType(DynamicType::mf));

    // [THEN] However, an amplitude dynamic value in ExpressionCurve has been increased on single level due to Accent Pattern
    EXPECT_EQ(event.expressionCtx().expressionCurve.maxAmplitudeLevel(), dynamicLevelFromType(DynamicType::f));
}

/**
 * @brief MPE_SingleNoteArticulationsTest_PocoTenuto
 * @details In this case we're gonna build a simple note event with the combination of staccato and tenuto articulations
 *          applied on the top of it. So the actual expression context of the note would reflect the average parameters
 *          from articulation patterns
 */
TEST_F(MPE_SingleNoteArticulationsTest, PocoTenuto)
{
    // [GIVEN] Articulation pattern "Staccato", which instructs a performer to shorten duration of a note
    ArticulationPatternSegment staccatoPattern;
    staccatoPattern.arrangementPattern = createArrangementPattern(5 * TEN_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
    staccatoPattern.pitchPattern = createSimplePitchPattern(0 /*increment_pitch_diff*/);
    staccatoPattern.expressionPattern
        = createSimpleExpressionPattern(m_nominalDynamic /* no dynamic changes comparing to the standard one*/);

    ArticulationPattern staccatoScope;
    staccatoScope.emplace(0, staccatoPattern);

    // [GIVEN] Articulation pattern "Tenuto", which instructs a performer to play a note for the whole nominal duration
    ArticulationPatternSegment tenutoPattern;
    tenutoPattern.arrangementPattern = createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
    tenutoPattern.pitchPattern = createSimplePitchPattern(0 /*increment_pitch_diff*/);
    tenutoPattern.expressionPattern = createSimpleExpressionPattern(m_nominalDynamic /* no dynamic changes comparing to the standard one*/);

    ArticulationPattern tenutoScope;
    tenutoScope.emplace(0, tenutoPattern);

    ArticulationMap appliedArticulations = {};

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationMeta staccatoMeta;
    staccatoMeta.type = ArticulationType::Staccato;
    staccatoMeta.pattern = staccatoScope;
    staccatoMeta.timestamp = m_nominalTimestamp;
    staccatoMeta.overallDuration = m_nominalDuration;

    ArticulationAppliedData staccatoApplied(std::move(staccatoMeta), 0, HUNDRED_PERCENT);
    appliedArticulations.emplace(ArticulationType::Staccato, std::move(staccatoApplied));

    ArticulationMeta tenutoMeta;
    tenutoMeta.type = ArticulationType::Tenuto;
    tenutoMeta.pattern = tenutoScope;
    tenutoMeta.timestamp = m_nominalTimestamp;
    tenutoMeta.overallDuration = m_nominalDuration;

    // [GIVEN] Tenuto articulation applied on the note
    ArticulationAppliedData tenutoApplied(std::move(tenutoMeta), 0, HUNDRED_PERCENT);
    appliedArticulations.emplace(ArticulationType::Tenuto, std::move(tenutoApplied));
    appliedArticulations.preCalculateAverageData();

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_staffIdx,
                    pitchLevel(m_pitchClass, m_octave),
                    m_nominalDynamic,
                    std::move(appliedArticulations),
                    0);

    // [THEN] We expect the nominal duration of a note to be unchanged
    EXPECT_EQ(event.arrangementCtx().nominalDuration, m_nominalDuration);

    // [THEN] We expect the actual duration of the note marked by staccato + tenuto to be equal to 0.75 of nominal duration
    EXPECT_EQ(event.arrangementCtx().actualDuration, m_nominalDuration * 0.75);
}

/**
 * @brief MPE_SingleNoteArticulationsTest_QuickFall
 * @details In this case we're gonna build a simple note event with the quick fall articulation
 *          applied on the top of it. So the actual pitch context of the note would reflect the
 *          parameters from articulation pattern
 */
TEST_F(MPE_SingleNoteArticulationsTest, QuickFall)
{
    // [GIVEN] Articulation pattern "Tenuto", which instructs a performer to play a note for the whole nominal duration
    ArticulationPatternSegment quickFallPattern;
    quickFallPattern.arrangementPattern = createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);

    // Linear decreasing pitch
    quickFallPattern.pitchPattern
        = createSimplePitchPattern(-PITCH_LEVEL_STEP / (MAX_PITCH_LEVEL / TEN_PERCENT) /*increment_pitch_diff*/);
    quickFallPattern.expressionPattern
        = createSimpleExpressionPattern(m_nominalDynamic /* no dynamic changes comparing to the standard one*/);

    ArticulationPattern quickFallScope;
    quickFallScope.emplace(0, quickFallPattern);

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationMeta quickFallMeta;
    quickFallMeta.type = ArticulationType::QuickFall;
    quickFallMeta.pattern = quickFallScope;
    quickFallMeta.timestamp = m_nominalTimestamp;
    quickFallMeta.overallDuration = m_nominalDuration;

    ArticulationAppliedData quickFallApplied(std::move(quickFallMeta), 0, HUNDRED_PERCENT);

    ArticulationMap appliedArticulations = {};
    appliedArticulations.emplace(ArticulationType::QuickFall, std::move(quickFallApplied));
    appliedArticulations.preCalculateAverageData();

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_staffIdx,
                    pitchLevel(m_pitchClass, m_octave),
                    m_nominalDynamic,
                    std::move(appliedArticulations),
                    0);

    // [THEN] We expect the pitch curve of the note marked by quick fall articulation to be equal to the pitch curve from corresponding pattern
    EXPECT_EQ(event.pitchCtx().pitchCurve, quickFallPattern.pitchPattern.pitchOffsetMap);
}

/**
 * @brief MPE_SingleNoteArticulationsTest_Scoop
 * @details In this case we're gonna build a simple note event with the scoop articulation
 *          applied on the top of it. So the actual pitch context of the note would reflect the
 *          parameters from articulation pattern
 */
TEST_F(MPE_SingleNoteArticulationsTest, Scoop)
{
    // [GIVEN] Articulation pattern "Scoop", which instructs a performer to play ahead of a nominal duration,
    //         starting at a lower pitch, and then placing it on the note being played.
    duration_percentage_t timestampOffset = static_cast<duration_percentage_t>(-2.5 * TEN_PERCENT);
    m_nominalTimestamp = 1000; //msecs

    ArticulationPatternSegment scoopPattern;
    scoopPattern.arrangementPattern = createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, timestampOffset /*timestamp_offset*/);

    // Linear increasing pitch
    scoopPattern.pitchPattern
        = createSimplePitchPattern(EXPECTED_SIZE /*increment_pitch_diff*/);
    scoopPattern.expressionPattern = createSimpleExpressionPattern(m_nominalDynamic /* no dynamic changes comparing to the standard one*/);

    ArticulationPattern scope;
    scope.emplace(0, scoopPattern);

    // [GIVEN] Staccato articulation applied on the note, since staccato is a single-note articulation
    //         occupied range is from 0% to 100%
    ArticulationMeta scoopMeta;
    scoopMeta.type = ArticulationType::Scoop;
    scoopMeta.pattern = scope;
    scoopMeta.timestamp = m_nominalTimestamp;
    scoopMeta.overallDuration = m_nominalDuration;

    ArticulationAppliedData scoopApplied(std::move(scoopMeta), 0, HUNDRED_PERCENT);

    ArticulationMap appliedArticulations = {};
    appliedArticulations.emplace(ArticulationType::Scoop, std::move(scoopApplied));
    appliedArticulations.preCalculateAverageData();

    // [WHEN] Note event with given parameters being built
    NoteEvent event(m_nominalTimestamp,
                    m_nominalDuration,
                    m_voiceIdx,
                    m_staffIdx,
                    pitchLevel(m_pitchClass, m_octave),
                    m_nominalDynamic,
                    std::move(appliedArticulations),
                    0);

    // [THEN] We expect the pitch curve of the note marked by scoop articulation to be equal to the pitch curve from corresponding pattern
    EXPECT_EQ(event.pitchCtx().pitchCurve, scoopPattern.pitchPattern.pitchOffsetMap);

    // [THEN] We expect that actual timestamp of the note will consider timestamp offset from the articulation pattern
    //        In other words, we'll start to playback a note with pitch offset and then finally land on the note being played
    EXPECT_EQ(event.arrangementCtx().actualTimestamp, m_nominalTimestamp + m_nominalDuration * percentageToFactor(timestampOffset));
}
