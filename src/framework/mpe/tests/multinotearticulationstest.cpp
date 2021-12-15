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
#include <map>

#include "mpe/events.h"
#include "mpe/tests/utils/articulationutils.h"

using namespace mu;
using namespace mu::mpe;
using namespace mu::mpe::tests;

class MultiNoteArticulationsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // [GIVEN] Rendering data of the very first quarter note on the score, with the 120BPM tempo and 4/4 time signature
        NoteMetaData& firstNoteData = m_initialData[0];
        firstNoteData.nominalTimestamp = 0;
        firstNoteData.nominalDuration = 500;
        firstNoteData.voiceIdx = 0; // first voice
        firstNoteData.pitchClass = PitchClass::A;
        firstNoteData.octave = 3;
        firstNoteData.nominalDynamicLevel = dynamicLevelFromType(DynamicType::f);

        // [GIVEN] Rendering data of the second quarter note on the score, with the 120BPM tempo and 4/4 time signature
        NoteMetaData& secondNoteData = m_initialData[1];
        secondNoteData.nominalTimestamp = 500;
        secondNoteData.nominalDuration = 500;
        secondNoteData.voiceIdx = 0; // first voice
        secondNoteData.pitchClass = PitchClass::C;
        secondNoteData.octave = 4;
        secondNoteData.nominalDynamicLevel = dynamicLevelFromType(DynamicType::f);

        // [GIVEN] Rendering data of the second quarter note on the score, with the 120BPM tempo and 4/4 time signature
        NoteMetaData& thirdNoteData = m_initialData[2];
        thirdNoteData.nominalTimestamp = 1000;
        thirdNoteData.nominalDuration = 500;
        thirdNoteData.voiceIdx = 0; // first voice
        thirdNoteData.pitchClass = PitchClass::A;
        thirdNoteData.octave = 3;
        thirdNoteData.nominalDynamicLevel = dynamicLevelFromType(DynamicType::f);

        // [GIVEN] Rendering data of the second quarter note on the score, with the 120BPM tempo and 4/4 time signature
        NoteMetaData& fourthNoteData = m_initialData[3];
        fourthNoteData.nominalTimestamp = 1500;
        fourthNoteData.nominalDuration = 500;
        fourthNoteData.voiceIdx = 0; // first voice
        fourthNoteData.pitchClass = PitchClass::C;
        fourthNoteData.octave = 4;
        fourthNoteData.nominalDynamicLevel = dynamicLevelFromType(DynamicType::f);

        // [GIVEN] Articulation pattern "Standard", which means that note should be played without any modifications
        m_standardPattern.arrangementPattern = createArrangementPattern(HUNDRED_PERCENTS /*duration_factor*/, 0 /*timestamp_offset*/);
        m_standardPattern.pitchPattern = createSimplePitchPattern(0 /*increment_pitch_diff*/);
        m_standardPattern.expressionPattern = createSimpleExpressionPattern(dynamicLevelFromType(DynamicType::Natural));
    }

    struct NoteMetaData {
        timestamp_t nominalTimestamp = 0;
        duration_t nominalDuration = 0;
        voice_layer_idx_t voiceIdx = 0;

        PitchClass pitchClass = PitchClass::Undefined;
        octave_t octave = 0;

        dynamic_level_t nominalDynamicLevel = 0;
    };

    std::map<size_t, NoteMetaData> m_initialData;

    ArticulationPatternSegment m_standardPattern;
};

/**
 * @brief MultiNoteArticulationsTest_StandardPattern
 * @details In this case we're gonna build a very simple note sequence without any articulation applied on the top of them
 *          So the actual PitchCurve of every note would be equal to the data from the "StandardPattern" in Articulation Profile.
 *          However, the actual ExpressionCurve would be adapted by the current dynamic value ("forte")
 */
TEST_F(MultiNoteArticulationsTest, StandardPattern)
{
    // [GIVEN] No articulations applied on the top of the note
    ArticulationPattern scope;
    scope.emplace(0, m_standardPattern);

    ArticulationData standardArticulationApplied(ArticulationType::Standard, scope, 0, HUNDRED_PERCENTS);

    ArticulationMap appliedArticulations;
    appliedArticulations.emplace(ArticulationType::Standard, standardArticulationApplied);

    // [WHEN] Notes sequence with given parameters being built
    std::map<size_t, NoteEvent> noteEvents;

    for (const auto& pair : m_initialData) {
        NoteEvent noteEvent(pair.second.nominalTimestamp,
                            pair.second.nominalDuration,
                            pair.second.voiceIdx,
                            pair.second.pitchClass,
                            pair.second.octave,
                            pair.second.nominalDynamicLevel,
                            appliedArticulations);

        noteEvents.emplace(pair.first, std::move(noteEvent));
    }

    for (const auto& pair : noteEvents) {
        // [THEN] We expect that nominal timestamp of every note will be equal to the actual timestamp, since there is no arrangement modificators
        EXPECT_EQ(pair.second.arrangementCtx().nominalTimestamp,
                  pair.second.arrangementCtx().actualTimestamp);

        // [THEN] We expect that nominal duration of every note will be equal to the actual duration, since there is no arrangement modificators
        EXPECT_EQ(pair.second.arrangementCtx().nominalDuration,
                  pair.second.arrangementCtx().actualDuration);

        // [THEN] We expect that amplitude dynamic value in ExpressionCurve adapted for the current dynamic value ("forte")
        EXPECT_EQ(pair.second.expressionCtx().expressionCurve.maxAmplitudeLevel(),
                  dynamicLevelFromType(DynamicType::f));

        // [THEN] We expect that pitch curve of every note will be using default curve from the standard patterns
        EXPECT_EQ(pair.second.pitchCtx().pitchCurve,
                  m_standardPattern.pitchPattern.pitchOffsetMap);
    }
}

/**
 * @brief MultiNoteArticulationsTest_GlissandoPattern
 * @details In this case we're gonna build a very simple note sequence with glissando articulation applied on the top of the first two notes
 *          So the actual PitchCurve of the first two notes would be equal to the data from the "GlissandoPattern" in Articulation Profile
 */
TEST_F(MultiNoteArticulationsTest, GlissandoPattern)
{
    std::map<size_t, ArticulationMap> appliedArticulations;

    // [GIVEN] Articulation pattern "Glissando", which instructs a performer to start on the pitch/rhythm
    //         and slide the pitch up/down to land on the next pitch/rhythm

    pitch_level_t pitchDiff = pitchLevelDiff(m_initialData[0].pitchClass, m_initialData[0].octave,
                                             m_initialData[1].pitchClass, m_initialData[1].octave);

    ArticulationPatternSegment glissandoPattern;
    glissandoPattern.arrangementPattern = createArrangementPattern(HUNDRED_PERCENTS /*duration_factor*/, 0 /*timestamp_offset*/);
    glissandoPattern.pitchPattern = createSimplePitchPattern(pitchDiff / (MAX_PITCH_LEVEL / TEN_PERCENTS) /*increment_pitch_diff*/);
    glissandoPattern.expressionPattern
        = createSimpleExpressionPattern(dynamicLevelFromType(DynamicType::Natural) /* no dynamic changes comparing to the natural one*/);

    ArticulationPattern glissandoScope;
    glissandoScope.emplace(0, glissandoPattern);

    ArticulationPattern standardScope;
    standardScope.emplace(0, m_standardPattern);

    // [GIVEN] Glissando articulation applied on the first note, occupied range is from 0% to 50% of the entire articulation range
    ArticulationData glissandoAppliedOnTheFirstNote(ArticulationType::Glissando, glissandoScope, 0, 5 * TEN_PERCENTS);
    appliedArticulations[0].emplace(ArticulationType::Glissando, glissandoAppliedOnTheFirstNote);

    // [GIVEN] Glissando articulation applied on the second note, occupied range is from 50% to 100% of the entire articulation range
    ArticulationData glissandoAppliedOnTheSecondNote(ArticulationType::Glissando, glissandoScope, 50, HUNDRED_PERCENTS);
    appliedArticulations[1].emplace(ArticulationType::Glissando, glissandoAppliedOnTheSecondNote);

    // [GIVEN] No articulations applied on the third note, occupied range is from 0% to 100% of the entire articulation range
    ArticulationData thirdNoteStandardArticulation(ArticulationType::Standard, standardScope, 0, HUNDRED_PERCENTS);
    appliedArticulations[2].emplace(ArticulationType::Standard, thirdNoteStandardArticulation);

    // [GIVEN] No articulations applied on the third note, occupied range is from 0% to 100% of the entire articulation range
    ArticulationData fourthNoteStandardArticulation(ArticulationType::Standard, standardScope, 0, HUNDRED_PERCENTS);
    appliedArticulations[3].emplace(ArticulationType::Standard, thirdNoteStandardArticulation);

    // [WHEN] Notes sequence with given parameters being built
    std::map<size_t, NoteEvent> noteEvents;

    for (const auto& pair : m_initialData) {
        NoteEvent noteEvent(pair.second.nominalTimestamp,
                            pair.second.nominalDuration,
                            pair.second.voiceIdx,
                            pair.second.pitchClass,
                            pair.second.octave,
                            pair.second.nominalDynamicLevel,
                            appliedArticulations[pair.first]);

        noteEvents.emplace(pair.first, std::move(noteEvent));
    }

    for (const auto& pair : noteEvents) {
        // [THEN] We expect that ExpressionCurve of every note will be adapted for the current dynamic value ("forte")
        EXPECT_EQ(pair.second.expressionCtx().expressionCurve.maxAmplitudeLevel(),
                  dynamicLevelFromType(DynamicType::f));
    }

    for (size_t i = 0; i < 2; ++i) {
        // [THEN] We expect that pitch curve of the notes 1 and 2 will be using a curve from the glissando pattern
        EXPECT_EQ(noteEvents.at(i).pitchCtx().pitchCurve,
                  glissandoPattern.pitchPattern.pitchOffsetMap);
    }

    for (size_t i = 2; i < 4; ++i) {
        // [THEN] We expect that pitch curve of the notes 3 and 4 will be using a curve from the standard pattern
        EXPECT_EQ(noteEvents.at(i).pitchCtx().pitchCurve,
                  m_standardPattern.pitchPattern.pitchOffsetMap);
    }
}

/**
 * @brief MultiNoteArticulationsTest_CrescendoPattern
 * @details In this case we're gonna build a very simple note sequence with crescendo articulation applied on
 *          the top of these notes (from forte to fortissimo). So the actual ExpressionCurve of the notes would be equal
 *          to the data from the "CrescendoPattern" in Articulation Profile
 */
TEST_F(MultiNoteArticulationsTest, CrescendoPattern)
{
    std::map<size_t, ArticulationMap> appliedArticulations;

    // [GIVEN] Articulation pattern "Crescendo", which instructs a performer to gradually increase a volume of every note,
    //         using a certain range of dynamics. In our case there will be a crescendo from forte up to fortissimo

    size_t noteCount = m_initialData.size();
    size_t dynamicSegmentsCount = noteCount - 1;

    dynamic_level_t dynamicLevelDiff = dynamicLevelFromType(DynamicType::ff) - dynamicLevelFromType(DynamicType::f);

    ArticulationPattern crescendoScope;

    for (size_t i = 0; i <= dynamicSegmentsCount; ++i) {
        ArticulationPatternSegment crescendoPattern;
        crescendoPattern.arrangementPattern = createArrangementPattern(HUNDRED_PERCENTS /*duration_factor*/, 0 /*timestamp_offset*/);
        crescendoPattern.pitchPattern = createSimplePitchPattern(0 /*increment_pitch_diff*/);
        crescendoPattern.expressionPattern = createSimpleExpressionPattern(dynamicLevelFromType(
                                                                               DynamicType::Natural) + static_cast<int>(i) * DYNAMIC_LEVEL_STEP
                                                                           / static_cast<dynamic_level_t>(dynamicSegmentsCount));

        crescendoScope.emplace(25 * ONE_PERCENT * static_cast<int>(i), std::move(crescendoPattern));
    }

    for (size_t i = 0; i < noteCount; ++i) {
        // [GIVEN] Crescendo articulation applied on the note
        ArticulationData crescendoApplied(ArticulationType::Crescendo, crescendoScope, 25 * ONE_PERCENT * static_cast<int>(i),
                                          25 * ONE_PERCENT * (static_cast<int>(i) + 1), 0, dynamicLevelDiff);
        appliedArticulations[i].emplace(ArticulationType::Crescendo, crescendoApplied);
    }

    // [WHEN] Notes sequence with given parameters being built
    std::map<size_t, NoteEvent> noteEvents;

    for (const auto& pair : m_initialData) {
        NoteEvent noteEvent(pair.second.nominalTimestamp,
                            pair.second.nominalDuration,
                            pair.second.voiceIdx,
                            pair.second.pitchClass,
                            pair.second.octave,
                            pair.second.nominalDynamicLevel,
                            appliedArticulations[pair.first]);

        noteEvents.emplace(pair.first, std::move(noteEvent));
    }

    for (size_t i = 0; i < noteCount; ++i) {
        // [THEN] We expect that ExpressionCurve of every note will be adapted to the applied CrescendoPattern
        //        That means that every note will be played louder on 125% than the previous one

        dynamic_level_t actualResult = dynamicLevelFromType(DynamicType::f) + i * static_cast<float>(dynamicLevelDiff)
                                       / dynamicSegmentsCount;
        EXPECT_EQ(noteEvents.at(i).expressionCtx().expressionCurve.maxAmplitudeLevel(),
                  actualResult);
    }
}
