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

using namespace muse;
using namespace muse::mpe;
using namespace muse::mpe::tests;

class MPE_MultiNoteArticulationsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // [GIVEN] Rendering data of the very first quarter note on the score, with the 120BPM tempo and 4/4 time signature
        NoteMetaData& firstNoteData = m_initialData[0];
        firstNoteData.nominalTimestamp = 0;
        firstNoteData.nominalDuration = 500;
        firstNoteData.voiceIdx = 0; // first voice
        firstNoteData.staffIdx = 0; // first staff;
        firstNoteData.nominalPitchLevel = pitchLevel(PitchClass::A, 3);
        firstNoteData.nominalDynamicLevel = dynamicLevelFromType(DynamicType::f);

        // [GIVEN] Rendering data of the second quarter note on the score, with the 120BPM tempo and 4/4 time signature
        NoteMetaData& secondNoteData = m_initialData[1];
        secondNoteData.nominalTimestamp = 500;
        secondNoteData.nominalDuration = 500;
        secondNoteData.voiceIdx = 0; // first voice
        secondNoteData.staffIdx = 0; // first staff;
        secondNoteData.nominalPitchLevel = pitchLevel(PitchClass::C, 4);
        secondNoteData.nominalDynamicLevel = dynamicLevelFromType(DynamicType::f);

        // [GIVEN] Rendering data of the second quarter note on the score, with the 120BPM tempo and 4/4 time signature
        NoteMetaData& thirdNoteData = m_initialData[2];
        thirdNoteData.nominalTimestamp = 1000;
        thirdNoteData.nominalDuration = 500;
        thirdNoteData.voiceIdx = 0; // first voice
        thirdNoteData.staffIdx = 0; // first staff;
        thirdNoteData.nominalPitchLevel = pitchLevel(PitchClass::A, 3);
        thirdNoteData.nominalDynamicLevel = dynamicLevelFromType(DynamicType::f);

        // [GIVEN] Rendering data of the second quarter note on the score, with the 120BPM tempo and 4/4 time signature
        NoteMetaData& fourthNoteData = m_initialData[3];
        fourthNoteData.nominalTimestamp = 1500;
        fourthNoteData.nominalDuration = 500;
        fourthNoteData.voiceIdx = 0; // first voice
        fourthNoteData.staffIdx = 0; // first staff;
        fourthNoteData.nominalPitchLevel = pitchLevel(PitchClass::C, 4);
        fourthNoteData.nominalDynamicLevel = dynamicLevelFromType(DynamicType::f);

        // [GIVEN] Articulation pattern "Standard", which means that note should be played without any modifications
        m_standardPattern.arrangementPattern = createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
        m_standardPattern.pitchPattern = createSimplePitchPattern(0 /*increment_pitch_diff*/);
        m_standardPattern.expressionPattern = createSimpleExpressionPattern(dynamicLevelFromType(DynamicType::Natural));
    }

    struct NoteMetaData {
        timestamp_t nominalTimestamp = 0;
        duration_t nominalDuration = 0;
        voice_layer_idx_t voiceIdx = 0;
        staff_layer_idx_t staffIdx = 0;

        pitch_level_t nominalPitchLevel = 0;

        dynamic_level_t nominalDynamicLevel = 0;
    };

    std::map<size_t, NoteMetaData> m_initialData;

    ArticulationPatternSegment m_standardPattern;
};

/**
 * @brief MPE_MultiNoteArticulationsTest_StandardPattern
 * @details In this case we're gonna build a very simple note sequence without any articulation applied on the top of them
 *          So the actual PitchCurve of every note would be equal to the data from the "StandardPattern" in Articulation Profile.
 *          However, the actual ExpressionCurve would be adapted by the current dynamic value ("forte")
 */
TEST_F(MPE_MultiNoteArticulationsTest, StandardPattern)
{
    // [GIVEN] No articulations applied on the top of the note
    ArticulationPattern pattern;
    pattern.emplace(0, m_standardPattern);

    ArticulationMeta meta;
    meta.type = ArticulationType::Standard;
    meta.pattern = pattern;
    meta.timestamp = m_initialData.at(0).nominalTimestamp;
    meta.overallDuration = m_initialData.at(0).nominalDuration;

    ArticulationAppliedData standardArticulationApplied(std::move(meta), 0, HUNDRED_PERCENT);

    ArticulationMap appliedArticulations;
    appliedArticulations.emplace(ArticulationType::Standard, std::move(standardArticulationApplied));
    appliedArticulations.preCalculateAverageData();

    // [WHEN] Notes sequence with given parameters being built
    std::map<size_t, NoteEvent> noteEvents;

    for (const auto& pair : m_initialData) {
        NoteEvent noteEvent(pair.second.nominalTimestamp,
                            pair.second.nominalDuration,
                            pair.second.voiceIdx,
                            pair.second.staffIdx,
                            pair.second.nominalPitchLevel,
                            pair.second.nominalDynamicLevel,
                            appliedArticulations,
                            0);

        noteEvents.emplace(pair.first, std::move(noteEvent));
    }

    for (const auto& pair : noteEvents) {
        // [THEN] We expect that nominal timestamp of every note will be equal to the actual timestamp, since there is no arrangement modifiers
        EXPECT_EQ(pair.second.arrangementCtx().nominalTimestamp,
                  pair.second.arrangementCtx().actualTimestamp);

        // [THEN] We expect that nominal duration of every note will be equal to the actual duration, since there is no arrangement modifiers
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
 * @brief MPE_MultiNoteArticulationsTest_GlissandoPattern
 * @details In this case we're gonna build a very simple note sequence with glissando articulation applied on the top of the first two notes
 *          So the actual PitchCurve of the first two notes would be equal to the data from the "GlissandoPattern" in Articulation Profile
 */
TEST_F(MPE_MultiNoteArticulationsTest, GlissandoPattern)
{
    std::map<size_t, ArticulationMap> appliedArticulations;

    // [GIVEN] Articulation pattern "Glissando", which instructs a performer to start on the pitch/rhythm
    //         and slide the pitch up/down to land on the next pitch/rhythm

    pitch_level_t pitchDiff = m_initialData[0].nominalPitchLevel - m_initialData[1].nominalPitchLevel;

    ArticulationPatternSegment glissandoPattern;
    glissandoPattern.arrangementPattern = createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
    glissandoPattern.pitchPattern = createSimplePitchPattern(pitchDiff / (MAX_PITCH_LEVEL / TEN_PERCENT) /*increment_pitch_diff*/);
    glissandoPattern.expressionPattern
        = createSimpleExpressionPattern(dynamicLevelFromType(DynamicType::Natural) /* no dynamic changes comparing to the natural one*/);

    ArticulationPattern glissandoScope;
    glissandoScope.emplace(0, glissandoPattern);

    ArticulationMeta glissandoMeta;
    glissandoMeta.type = ArticulationType::DiscreteGlissando;
    glissandoMeta.pattern = glissandoScope;
    glissandoMeta.timestamp = m_initialData.at(0).nominalTimestamp;
    glissandoMeta.overallDuration = m_initialData.at(0).nominalDuration + m_initialData.at(1).nominalDuration;

    ArticulationPattern standardScope;
    standardScope.emplace(0, m_standardPattern);

    ArticulationMeta thirdNoteStandardMeta;
    thirdNoteStandardMeta.type = ArticulationType::Standard;
    thirdNoteStandardMeta.pattern = standardScope;
    thirdNoteStandardMeta.timestamp = m_initialData.at(2).nominalTimestamp;
    thirdNoteStandardMeta.overallDuration = m_initialData.at(0).nominalDuration;

    ArticulationMeta fourthNoteStandardMeta;
    fourthNoteStandardMeta.type = ArticulationType::Standard;
    fourthNoteStandardMeta.pattern = standardScope;
    fourthNoteStandardMeta.timestamp = m_initialData.at(3).nominalTimestamp;
    fourthNoteStandardMeta.overallDuration = m_initialData.at(0).nominalDuration;

    // [GIVEN] Glissando articulation applied on the first note, occupied range is from 0% to 50% of the entire articulation range
    ArticulationAppliedData glissandoAppliedOnTheFirstNote(glissandoMeta, 0, 5 * TEN_PERCENT);
    appliedArticulations[0].emplace(ArticulationType::DiscreteGlissando, std::move(glissandoAppliedOnTheFirstNote));
    appliedArticulations[0].preCalculateAverageData();

    // [GIVEN] Glissando articulation applied on the second note, occupied range is from 50% to 100% of the entire articulation range
    ArticulationAppliedData glissandoAppliedOnTheSecondNote(glissandoMeta, 50, HUNDRED_PERCENT);
    appliedArticulations[1].emplace(ArticulationType::DiscreteGlissando, std::move(glissandoAppliedOnTheSecondNote));
    appliedArticulations[1].preCalculateAverageData();

    // [GIVEN] No articulations applied on the third note, occupied range is from 0% to 100% of the entire articulation range
    ArticulationAppliedData thirdNoteStandardArticulation(thirdNoteStandardMeta, 0, HUNDRED_PERCENT);
    appliedArticulations[2].emplace(ArticulationType::Standard, std::move(thirdNoteStandardArticulation));
    appliedArticulations[2].preCalculateAverageData();

    // [GIVEN] No articulations applied on the third note, occupied range is from 0% to 100% of the entire articulation range
    ArticulationAppliedData fourthNoteStandardArticulation(fourthNoteStandardMeta, 0, HUNDRED_PERCENT);
    appliedArticulations[3].emplace(ArticulationType::Standard, std::move(fourthNoteStandardArticulation));
    appliedArticulations[3].preCalculateAverageData();

    // [WHEN] Notes sequence with given parameters being built
    std::map<size_t, NoteEvent> noteEvents;

    for (const auto& pair : m_initialData) {
        NoteEvent noteEvent(pair.second.nominalTimestamp,
                            pair.second.nominalDuration,
                            pair.second.voiceIdx,
                            pair.second.staffIdx,
                            pair.second.nominalPitchLevel,
                            pair.second.nominalDynamicLevel,
                            appliedArticulations[pair.first],
                            0);

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
 * @brief MPE_MultiNoteArticulationsTest_CrescendoPattern
 * @details In this case we're gonna build a very simple note sequence with crescendo articulation applied on
 *          the top of these notes (from forte to fortissimo). So the actual ExpressionCurve of the notes would be equal
 *          to the data from the "CrescendoPattern" in Articulation Profile
 */
TEST_F(MPE_MultiNoteArticulationsTest, CrescendoPattern)
{
    std::map<size_t, ArticulationMap> appliedArticulations;

    // [GIVEN] Articulation pattern "Crescendo", which instructs a performer to gradually increase a volume of every note,
    //         using a certain range of dynamics. In our case there will be a crescendo from forte up to fortissimo

    int noteCount = static_cast<int>(m_initialData.size());
    int dynamicSegmentsCount = noteCount - 1;

    dynamic_level_t dynamicLevelDiff = dynamicLevelFromType(DynamicType::ff) - dynamicLevelFromType(DynamicType::f);

    ArticulationPattern crescendoScope;

    for (int i = 0; i <= dynamicSegmentsCount; ++i) {
        ArticulationPatternSegment crescendoPattern;
        crescendoPattern.arrangementPattern = createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
        crescendoPattern.pitchPattern = createSimplePitchPattern(0 /*increment_pitch_diff*/);
        crescendoPattern.expressionPattern = createSimpleExpressionPattern(dynamicLevelFromType(
                                                                               DynamicType::Natural) + i * DYNAMIC_LEVEL_STEP
                                                                           / dynamicSegmentsCount);

        crescendoScope.emplace(25 * ONE_PERCENT * i, std::move(crescendoPattern));
    }

    ArticulationMeta crescendoMeta;
    crescendoMeta.type = ArticulationType::DiscreteGlissando;
    crescendoMeta.pattern = crescendoScope;
    crescendoMeta.timestamp = m_initialData.at(0).nominalTimestamp;
    crescendoMeta.overallDuration = m_initialData.at(1).nominalTimestamp + m_initialData.at(1).nominalDuration;
    crescendoMeta.overallDynamicChangesRange = dynamicLevelDiff;

    for (const auto& pair : m_initialData) {
        crescendoMeta.overallDuration += pair.second.nominalDuration;
    }

    for (int i = 0; i < noteCount; ++i) {
        // [GIVEN] Crescendo articulation applied on the note
        ArticulationAppliedData crescendoApplied(crescendoMeta, 25 * ONE_PERCENT * i, 25 * ONE_PERCENT * (i + 1));
        appliedArticulations[i].emplace(ArticulationType::Crescendo, std::move(crescendoApplied));
        appliedArticulations[i].preCalculateAverageData();
    }

    // [WHEN] Notes sequence with given parameters being built
    std::map<size_t, NoteEvent> noteEvents;

    for (const auto& pair : m_initialData) {
        NoteEvent noteEvent(pair.second.nominalTimestamp,
                            pair.second.nominalDuration,
                            pair.second.voiceIdx,
                            pair.second.staffIdx,
                            pair.second.nominalPitchLevel,
                            pair.second.nominalDynamicLevel,
                            appliedArticulations[pair.first],
                            0);

        noteEvents.emplace(pair.first, std::move(noteEvent));
    }

    for (int i = 0; i < noteCount; ++i) {
        // [THEN] We expect that ExpressionCurve of every note will be adapted to the applied CrescendoPattern
        //        That means that every note will be played louder on 125% than the previous one

        dynamic_level_t actualResult = dynamicLevelFromType(DynamicType::f) + i * static_cast<dynamic_level_t>(dynamicLevelDiff)
                                       / dynamicSegmentsCount;
        EXPECT_EQ(noteEvents.at(i).expressionCtx().expressionCurve.maxAmplitudeLevel(),
                  actualResult);
    }
}

TEST_F(MPE_MultiNoteArticulationsTest, IsMultiNoteArticulation)
{
    const ArticulationTypeSet MULTI_TYPES = {
        ArticulationType::Trill,
        ArticulationType::Crescendo,
        ArticulationType::Decrescendo,
        ArticulationType::DiscreteGlissando,
        ArticulationType::Legato,
        ArticulationType::Pedal,
        ArticulationType::Multibend,
        ArticulationType::Arpeggio,
        ArticulationType::ArpeggioUp,
        ArticulationType::ArpeggioDown,
        ArticulationType::ArpeggioStraightUp,
        ArticulationType::ArpeggioStraightDown,
    };

    const ArticulationTypeSet RANGED_TYPES {
        ArticulationType::Legato,
        ArticulationType::Pedal,
        ArticulationType::Multibend,
    };

    for (int i = int(ArticulationType::Standard); i < int(ArticulationType::Last); ++i) {
        ArticulationType type = ArticulationType(i);
        bool isMulti = muse::contains(MULTI_TYPES, type);
        bool isRanged = muse::contains(RANGED_TYPES, type);

        EXPECT_EQ(mpe::isMultiNoteArticulation(type), isMulti);
        EXPECT_EQ(mpe::isSingleNoteArticulation(type), !isMulti);
        EXPECT_EQ(mpe::isRangedArticulation(type), isRanged);
    }
}
