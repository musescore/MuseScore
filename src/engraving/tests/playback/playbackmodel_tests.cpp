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
#include <gmock/gmock.h>
#include <memory>

#include "async/asyncable.h"
#include "async/channel.h"
#include "mpe/tests/utils/articulationutils.h"
#include "mpe/tests/mocks/articulationprofilesrepositorymock.h"

#include "utils/scorerw.h"
#include "dom/part.h"
#include "dom/measure.h"
#include "dom/chord.h"

#include "playback/playbackmodel.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

using namespace mu::engraving;
using namespace muse::mpe;
using namespace muse;

static const String PLAYBACK_MODEL_TEST_FILES_DIR("playback/playbackmodel_data/");
static constexpr duration_t QUARTER_NOTE_DURATION = 500000; // duration in microseconds for 4/4 120BPM

class Engraving_PlaybackModelTests : public ::testing::Test, public muse::async::Asyncable
{
protected:
    void SetUp() override
    {
        //! NOTE: allows to read test files using their version readers
        //! instead of using 302 (see mscloader.cpp, makeReader)
        MScore::useRead302InTestMode = false;

        m_dummyPatternSegment.arrangementPattern
            = tests::createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
        m_dummyPatternSegment.pitchPattern = tests::createSimplePitchPattern(0 /*increment_pitch_diff*/);
        m_dummyPatternSegment.expressionPattern = tests::createSimpleExpressionPattern(dynamicLevelFromType(mpe::DynamicType::Natural));
        m_dummyPattern.emplace(0, m_dummyPatternSegment);

        m_defaultProfile = std::make_shared<ArticulationsProfile>();

        m_repositoryMock = std::make_shared<NiceMock<ArticulationProfilesRepositoryMock> >();
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = true;
    }

    ArticulationPattern buildTestArticulationPattern() const
    {
        ArticulationPatternSegment blankSegment(ArrangementPattern(HUNDRED_PERCENT /*durationFactor*/, 0 /*timestampOffset*/),
                                                PitchPattern(EXPECTED_SIZE, TEN_PERCENT, 0),
                                                ExpressionPattern(EXPECTED_SIZE, TEN_PERCENT, 0));

        ArticulationPattern pattern;
        pattern.emplace(0, std::move(blankSegment));

        return pattern;
    }

    ArticulationsProfilePtr m_defaultProfile = nullptr;

    ArticulationPattern m_dummyPattern;
    ArticulationPatternSegment m_dummyPatternSegment;

    std::shared_ptr<NiceMock<ArticulationProfilesRepositoryMock> > m_repositoryMock = nullptr;
};

/**
 * @brief PlaybackModelTests_SimpleRepeat
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 4 measures
 *          Additionally, there is a simple repeat from measure 2 up to measure 3. In total, we'll be playing 6 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, SimpleRepeat)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_range/repeat_range.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 6 overall measures which should be played
    int expectedSize = 24;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Two_Ending_Repeat
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a repeat at the end of the second measure. Measure 2 is the first ending of the repeat.
 *          Measure 3 is the second ending of the repeat. In total, we'll be playing 7 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, Two_Ending_Repeat)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_with_2_voltas/repeat_with_2_voltas.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 7 measures which should be played
    int expectedSize = 28;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Repeat_And_Tremolo
 * @details Checks that tremolos after a repeated section are rendered at the correct timestamp
 */
TEST_F(Engraving_PlaybackModelTests, Repeat_And_Tremolo)
{
    // [GIVEN] Simple piece of score (Flute, 4/4, 120 bpm)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_and_tremolo/repeat_and_tremolo.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [WHEN] The articulation profiles repository will be returning profiles
    m_defaultProfile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    m_defaultProfile->setPattern(ArticulationType::Tremolo32nd, buildTestArticulationPattern());

    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [GIVEN] Expected amount of events per timestamp
    const std::map<timestamp_t, std::pair<size_t /*notes*/, size_t /*rests*/> > expectedSizePerTimestamp {
        // The first four half notes; repeated
        { 0 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 1 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 2 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 3 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 4 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 5 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 6 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 7 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },

        // After the repeat
        { 8 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 9 * 2 * QUARTER_NOTE_DURATION, { 0, 1 } },

        // Final three half notes
        { 10 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 11 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 12 * 2 * QUARTER_NOTE_DURATION, { 16, 0 } },
        { 13 * 2 * QUARTER_NOTE_DURATION, { 0, 1 } }
    };

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events per timestamp matches expectations
    auto isExpectedTimestamp = [&expectedSizePerTimestamp](timestamp_t timestamp) {
        if (muse::contains(expectedSizePerTimestamp, timestamp)) {
            return testing::AssertionSuccess();
        } else {
            return testing::AssertionFailure() << "timestamp " << timestamp << " is not contained in expectedSizePerTimestamp";
        }
    };

    size_t timestampCount = 0;
    for (const auto& pair : result) {
        ++timestampCount;

        ASSERT_TRUE(isExpectedTimestamp(pair.first));

        size_t notes = 0, rests = 0;
        for (const PlaybackEvent& event : pair.second) {
            if (std::holds_alternative<mpe::NoteEvent>(event)) {
                // Check actual timestamp
                const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
                EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, pair.first + notes * (2 * QUARTER_NOTE_DURATION / 16));

                ++notes;
            } else {
                ++rests;
            }
        }

        EXPECT_EQ(std::make_pair(notes, rests), expectedSizePerTimestamp.at(pair.first));
    }

    EXPECT_EQ(timestampCount, expectedSizePerTimestamp.size());
}

/**
 * @brief PlaybackModelTests_Repeat_Tempo_Changes_And_Tie
 * @details Checks that the length of tied notes is correct even after tempo changes and repeats
 */
TEST_F(Engraving_PlaybackModelTests, Repeat_Tempo_Changes_And_Tie)
{
    // [GIVEN] Score containing some repeated measures, some tempo changes and a tied note
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_tempo_changes_and_tie/repeat_tempo_changes_and_tie.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [WHEN] The articulation profiles repository will be returning profiles
    m_defaultProfile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());

    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] The duration of the tied note matches expectations
    size_t noteEventCount = 0;
    for (const auto& pair : result) {
        for (const PlaybackEvent& event : pair.second) {
            if (std::holds_alternative<mpe::NoteEvent>(event)) {
                const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
                EXPECT_EQ(noteEvent.arrangementCtx().nominalDuration, 8 * QUARTER_NOTE_DURATION);

                ++noteEventCount;
            }
        }
    }

    // [THEN] The amount of note events matches expectations
    EXPECT_EQ(noteEventCount, 1);
}

/**
 * @brief PlaybackModelTests_Da_Capo_Al_Fine
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.C. Al Fine" marking at the end of the 6-th measure. Measure 2 is marked by "Fine"
 *          In total, we'll be playing 8 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, Da_Capo_Al_Fine)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "da_capo_al_fine/da_capo_al_fine.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 8 measures which should be played
    int expectedSize = 32;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Dal_Segno_Al_Coda
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.S. Al Coda" marking at the end of the 4-th measure. Measure 2 is marked by "Segno" marking.
 *          The end of the 3-rd measure is marked by "To Coda" marking. The beginning of the 5-th measure is marked by "Coda" sign
 *          In total, we'll be playing 8 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, Dal_Segno_Al_Coda)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "dal_segno_al_coda/dal_segno_al_coda.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 8 measures which should be played
    int expectedSize = 32;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Dal_Segno_Al_Fine
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.S. Al Fine" marking at the end of the 6-th measure. Measure 2 is marked by "Segno" marking.
 *          The end of the 4-th measure is marked by "Fine" marking. In total, we'll be playing 9 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, Dal_Segno_Al_Fine)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "dal_segno_al_fine/dal_segno_al_fine.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 9 measures which should be played
    int expectedSize = 36;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Da_Capo_Al_Coda
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.C. Al Coda" marking at the end of the 6-th measure. The end of the measure 2 is marked by "To Coda".
 *          The beginning of the 4-th measure is marked by "Coda" sign. In total, we'll be playing 11 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, Da_Capo_Al_Coda)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "da_capo_al_coda/da_capo_al_coda.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 11 measures which should be played
    int expectedSize = 44;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Spanners
 * @details Given a score where in each measure there is a spanner over the second and third note,
 *          we check that the spanner affects indeed the expected notes.
 */
TEST_F(Engraving_PlaybackModelTests, Spanners)
{
    // [GIVEN] Simple piece of score, where in each measure there is a spanner over the second and third note
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "spanners/spanners.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events
    static constexpr int expectedNumberOfEvents = 3 * 4;

    // [WHEN] The articulation profiles repository will be returning profiles
    m_defaultProfile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    m_defaultProfile->setPattern(ArticulationType::Pedal, buildTestArticulationPattern());
    m_defaultProfile->setPattern(ArticulationType::Trill, buildTestArticulationPattern());
    m_defaultProfile->setPattern(ArticulationType::Legato, buildTestArticulationPattern());

    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events matches expectations
    EXPECT_EQ(result.size(), expectedNumberOfEvents);

    // [THEN] Details of applied articulations match expectations
    struct ExpectedArticulation {
        ArticulationType articulationType = ArticulationType::Standard;
        timestamp_t from = 0;
        timestamp_t to = 0;
    };

    // [THEN] Amount of applied articulations matches expectations
    static const std::vector<ExpectedArticulation> expectedArticulations = {
        {},
        { ArticulationType::Pedal, 1 * QUARTER_NOTE_DURATION, 3 * QUARTER_NOTE_DURATION },
        { ArticulationType::Pedal, 1 * QUARTER_NOTE_DURATION, 3 * QUARTER_NOTE_DURATION },
        {},
        {},
        { ArticulationType::Trill, 5 * QUARTER_NOTE_DURATION, 7 * QUARTER_NOTE_DURATION },
        { ArticulationType::Trill, 5 * QUARTER_NOTE_DURATION, 7 * QUARTER_NOTE_DURATION },
        {},
        {},
        { ArticulationType::Legato, 9 * QUARTER_NOTE_DURATION, 11 * QUARTER_NOTE_DURATION },
        { ArticulationType::Legato, 9 * QUARTER_NOTE_DURATION, 11 * QUARTER_NOTE_DURATION },
        {}
    };

    for (size_t i=0; i < expectedNumberOfEvents; ++i) {
        const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(result.at(i * QUARTER_NOTE_DURATION).at(0));

        EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);

        const ArticulationMap::PairType& articulation = *noteEvent.expressionCtx().articulations.cbegin();

        const ExpectedArticulation& expectedArticulation = expectedArticulations[i];

        if (expectedArticulation.articulationType == ArticulationType::Standard) {
            EXPECT_EQ(articulation.first, ArticulationType::Standard);
        } else {
            EXPECT_EQ(articulation.first, expectedArticulation.articulationType);
            EXPECT_EQ(articulation.second.meta.timestamp, expectedArticulation.from);
            EXPECT_EQ(articulation.second.meta.timestamp + articulation.second.meta.overallDuration, expectedArticulation.to);
        }
    }
}

/**
 * @brief PlaybackModelTests_Dynamics
 * @details Test simple dynamic markings and hairpins
 */

TEST_F(Engraving_PlaybackModelTests, Dynamics)
{
    // [GIVEN] Score with piano marking at the start, then crescendo to forte,
    //         then again crescendo, followed by sudden pianissimo
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "dynamics/dynamics.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [WHEN] The articulation profiles repository will be returning profiles
    m_defaultProfile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());

    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const DynamicLevelLayers& dynamics = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).dynamics;
    ASSERT_FALSE(dynamics.empty());
    const DynamicLevelMap& dynamicLevelMap = dynamics.begin()->second;

    // [THEN] Dynamic level map matches expectations
    EXPECT_EQ(dynamicLevelMap.size(), 52);

    static constexpr dynamic_level_t piano = dynamicLevelFromType(mpe::DynamicType::p);
    static constexpr dynamic_level_t forte = dynamicLevelFromType(mpe::DynamicType::f);
    static constexpr dynamic_level_t fortePlusSomething = dynamicLevelFromType(mpe::DynamicType::f) + DYNAMIC_LEVEL_STEP;
    static constexpr dynamic_level_t pianissimo = dynamicLevelFromType(mpe::DynamicType::pp);

    // Start piano
    EXPECT_EQ(dynamicLevelMap.at(0 * QUARTER_NOTE_DURATION), piano);

    // Still piano at the start of the crescendo
    EXPECT_EQ(dynamicLevelMap.at(4 * QUARTER_NOTE_DURATION), piano);

    // Gradually grow to forte after that
    EXPECT_EQ(dynamicLevelMap.at(4 * QUARTER_NOTE_DURATION + (4 * QUARTER_NOTE_DURATION) * 1 / 24), 4312);
    EXPECT_EQ(dynamicLevelMap.at(4 * QUARTER_NOTE_DURATION + (4 * QUARTER_NOTE_DURATION) * 2 / 24), 4375);
    EXPECT_EQ(dynamicLevelMap.at(4 * QUARTER_NOTE_DURATION + (4 * QUARTER_NOTE_DURATION) * 3 / 24), piano + (forte - piano) * 3 / 24);
    EXPECT_EQ(dynamicLevelMap.at(4 * QUARTER_NOTE_DURATION + (4 * QUARTER_NOTE_DURATION) * 7 / 24), 4687);
    EXPECT_EQ(dynamicLevelMap.at(4 * QUARTER_NOTE_DURATION + (4 * QUARTER_NOTE_DURATION) * 8 / 24), piano + (forte - piano) * 8 / 24);
    EXPECT_EQ(dynamicLevelMap.at(4 * QUARTER_NOTE_DURATION + (4 * QUARTER_NOTE_DURATION) * 15 / 24), 5187);
    EXPECT_EQ(dynamicLevelMap.at(4 * QUARTER_NOTE_DURATION + (4 * QUARTER_NOTE_DURATION) * 21 / 24), 5562);
    EXPECT_EQ(dynamicLevelMap.at(4 * QUARTER_NOTE_DURATION + (4 * QUARTER_NOTE_DURATION) * 22 / 24), piano + (forte - piano) * 22 / 24);
    EXPECT_EQ(dynamicLevelMap.at(4 * QUARTER_NOTE_DURATION + (4 * QUARTER_NOTE_DURATION) * 23 / 24), 5687);

    // Reach forte
    EXPECT_EQ(dynamicLevelMap.at(8 * QUARTER_NOTE_DURATION), forte);

    // Still forte at the start of next crescendo
    EXPECT_EQ(dynamicLevelMap.at(12 * QUARTER_NOTE_DURATION), forte);

    // Gradually grow louder than forte after that
    EXPECT_EQ(dynamicLevelMap.at(12 * QUARTER_NOTE_DURATION + int(1919 / 24.f * 1) / 480.0 * QUARTER_NOTE_DURATION), 5770);
    EXPECT_EQ(dynamicLevelMap.at(12 * QUARTER_NOTE_DURATION + int(1919 / 24.f * 2) / 480.0 * QUARTER_NOTE_DURATION),
              forte + (fortePlusSomething - forte) * 2 / 24);
    EXPECT_EQ(dynamicLevelMap.at(12 * QUARTER_NOTE_DURATION + int(1919 / 24.f * 3) / 480.0 * QUARTER_NOTE_DURATION), 5812);
    EXPECT_EQ(dynamicLevelMap.at(12 * QUARTER_NOTE_DURATION + int(1919 / 24.f * 7) / 480.0 * QUARTER_NOTE_DURATION), 5895);
    EXPECT_EQ(dynamicLevelMap.at(12 * QUARTER_NOTE_DURATION + int(1919 / 24.f * 15) / 480.0 * QUARTER_NOTE_DURATION),
              forte + (fortePlusSomething - forte) * 15 / 24);
    EXPECT_EQ(dynamicLevelMap.at(12 * QUARTER_NOTE_DURATION + int(1919 / 24.f * 21) / 480.0 * QUARTER_NOTE_DURATION),
              forte + (fortePlusSomething - forte) * 21 / 24);
    EXPECT_EQ(dynamicLevelMap.at(12 * QUARTER_NOTE_DURATION + int(1919 / 24.f * 22) / 480.0 * QUARTER_NOTE_DURATION), 6208);
    EXPECT_EQ(dynamicLevelMap.at(12 * QUARTER_NOTE_DURATION + int(1919 / 24.f * 23) / 480.0 * QUARTER_NOTE_DURATION), 6229);

    // Reach forte plus something, just before the start of the next measure
    EXPECT_EQ(dynamicLevelMap.at(12 * QUARTER_NOTE_DURATION + 1919 / 480.0 * QUARTER_NOTE_DURATION), fortePlusSomething);

    // Finally, jump to pianissimo
    EXPECT_EQ(dynamicLevelMap.at(16 * QUARTER_NOTE_DURATION), pianissimo);

    // That should be the last event
    EXPECT_EQ(std::prev(dynamicLevelMap.cend())->first, 16 * QUARTER_NOTE_DURATION);
}

/**
 * @brief PlaybackModelTests_Pizz_To_Arco_Technique
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 1 measure
 *          Additionally, the first note is marked by "pizzicato" + "stacattissimo". The 3-rd note is marked by "arco"
 *          We'll be playing 4 events overall
 */
TEST_F(Engraving_PlaybackModelTests, Pizz_To_Arco_Technique)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "pizz_to_arco/pizz_to_arco.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events
    int expectedSize = 4;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    m_defaultProfile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    m_defaultProfile->setPattern(ArticulationType::Pizzicato, buildTestArticulationPattern());
    m_defaultProfile->setPattern(ArticulationType::Staccatissimo, buildTestArticulationPattern());

    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);

    // [THEN] The first note has Pizzicato and Staccatissimo articulations applied
    const mpe::NoteEvent& firstNoteEvent = std::get<mpe::NoteEvent>(result.at(0).at(0));
    EXPECT_EQ(firstNoteEvent.expressionCtx().articulations.size(), 2);
    EXPECT_TRUE(firstNoteEvent.expressionCtx().articulations.contains(ArticulationType::Pizzicato));
    EXPECT_TRUE(firstNoteEvent.expressionCtx().articulations.contains(ArticulationType::Staccatissimo));

    // [THEN] The second note has only Pizzicato articulation applied
    const mpe::NoteEvent& secondNoteEvent = std::get<mpe::NoteEvent>(result.at(500000).at(0));
    EXPECT_EQ(secondNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(secondNoteEvent.expressionCtx().articulations.contains(ArticulationType::Pizzicato));

    // [THEN] The third note has only Standard articulation applied
    const mpe::NoteEvent& thirdNoteEvent = std::get<mpe::NoteEvent>(result.at(1000000).at(0));
    EXPECT_EQ(thirdNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(thirdNoteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));

    // [THEN] The fourth note has only Standard articulation applied
    const mpe::NoteEvent& fourthNoteEvent = std::get<mpe::NoteEvent>(result.at(1500000).at(0));
    EXPECT_EQ(fourthNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(fourthNoteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));
}

/**
 * @brief PlaybackModelTests_FallbackToStandardArticulation
 * @details In this case we're building up a playback model of a simple score - Winds + Voice, 4/4, 120bpm, Treble Cleff, 2 measures
 *          The user added a bunch of articulations that are not supported by these instruments. These include:
 *          * pizz, detache for Winds
 *          * Fall/Doit/Plop/Scoop and notes with crosshead for Voice
 *          Make sure that we will fallback to the Standard articulation
 */
TEST_F(Engraving_PlaybackModelTests, FallbackToStandardArticulation)
{
    // [GIVEN] Simple piece of score (Winds + Voice, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "wrong_articulations/wrong_articulations.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 2);

    const Part* windsPart = score->parts().at(0);
    ASSERT_EQ(windsPart->instrumentId(), "winds");

    const Part* voicePart = score->parts().at(1);
    ASSERT_EQ(voicePart->instrumentId(), "voice");

    constexpr int NOTE_COUNT = 8;

    // [WHEN] The articulation profiles repository will be returning profiles for Winds/Voices families
    m_defaultProfile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());

    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    // [WHEN] Request events for Winds
    const PlaybackEventsMap& windsResult
        = model.resolveTrackPlaybackData(windsPart->id(), windsPart->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(windsResult.size(), NOTE_COUNT);

    for (auto pair : windsResult) {
        const mpe::PlaybackEventList& events = pair.second;

        for (const PlaybackEvent& event : events) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            // [THEN] Use the Standard articulation instead of the one added by the user
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));
            EXPECT_GT(noteEvent.arrangementCtx().actualDuration, 0);
        }
    }

    // [WHEN] Request events for Voice
    const PlaybackEventsMap& voiceResult
        = model.resolveTrackPlaybackData(voicePart->id(), voicePart->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(voiceResult.size(), NOTE_COUNT);

    for (auto pair : voiceResult) {
        const mpe::PlaybackEventList& events = pair.second;

        for (const PlaybackEvent& event : events) {
            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            // [THEN] Use the Standard articulation instead of the one added by the user
            EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);
            EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));
            EXPECT_GT(noteEvent.arrangementCtx().actualDuration, 0);
        }
    }
}

/**
 * @brief PlaybackModelTests_Single_Measure_Repeat
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 7 measures
 *          The first measure contains 4 quarter notes, and the second measure contains a "repeat last measure" sign to repeat these.
 *          The third measure contains 8 eighth notes, and the fourth through sixth measures contain "repeat last measure" signs to
 *          repeat these three times.
 *          The final measure contains a final whole note.
 */
TEST_F(Engraving_PlaybackModelTests, Single_Measure_Repeat)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "single_measure_repeat/single_measure_repeat.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 2 measures of 4 quarter notes, 4 measures of 8 eighth notes, 1 final note
    int expectedSize = 2 * 4 + 4 * 8 + 1;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Multi_Measure_Repeat
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 9 measures
 *          The first measure contains a simple pattern of 4 quarter notes.
 *          The second measure contains a "repeat last measure" sign; the third and fourth measures contain a "repeat last
 *          two measures" sign; the fifth through eighth measures contain a "repeat last four measures" sign.
 *          This means that we will repeat the same pattern eight times. The final measure contains a whole note.
 */
TEST_F(Engraving_PlaybackModelTests, Multi_Measure_Repeat)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "multi_measure_repeat/multi_measure_repeat.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 8 measures of 4 quarter notes, 1 final note
    int expectedSize = 8 * 4 + 1;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_SimpleRepeat_Changes_Notification
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 4 measures
 *          Additionally, there is a simple repeat from measure 2 up to measure 3. In total, we'll be playing 6 measures overall
 *
 *          When the model will be loaded we'll emulate a change notification on the 2-nd measure, so that there will be updated events
 *          on the main stream channel
 */
TEST_F(Engraving_PlaybackModelTests, SimpleRepeat_Changes_Notification)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_range/repeat_range.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    ON_CALL(*m_repositoryMock, defaultProfile(_)).WillByDefault(Return(m_defaultProfile));

    // [GIVEN] Expected amount of changed events
    int expectedChangedEventsCount = 24;

    // [GIVEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    PlaybackData result = model.resolveTrackPlaybackData(part->id(), part->instrumentId());
    EXPECT_EQ(result.originEvents.size(), expectedChangedEventsCount);

    // [THEN] Updated events map will match our expectations
    result.mainStream.onReceive(this, [expectedChangedEventsCount](const PlaybackEventsMap& updatedEvents, const DynamicLevelLayers&) {
        EXPECT_EQ(updatedEvents.size(), expectedChangedEventsCount);
    });

    // [WHEN] Score has been changed: the range starts ouside the repeat and ends inside it
    ScoreChanges changes;
    changes.tickFrom = 480; // 2nd note of the 1st measure (outside the repeat)
    changes.tickTo = 3840; // 1st note of the 3rd measure (inside the repeat)
    changes.staffIdxFrom = 0;
    changes.staffIdxTo = 0;
    changes.changedTypes = { ElementType::NOTE };

    score->changesChannel().send(changes);

    // [WHEN] Score has been changed: the range is inside the repeat and tickTo == the end tick of the repeat
    // See: https://github.com/musescore/MuseScore/issues/25899
    changes.tickFrom = 4800; // 3rd note of the 3rd measure (inside the repeat)
    changes.tickTo = 5760; // end tick of the repeat
    changes.staffIdxFrom = 0;
    changes.staffIdxTo = 0;
    changes.changedTypes = { ElementType::PEDAL };

    score->changesChannel().send(changes);
}

/**
 * @brief PlaybackModelTests_TempoChangesDuringNotes
 * @details Test that notes and other elements have the correct length when tempo changes occur during them
 */
TEST_F(Engraving_PlaybackModelTests, TempoChangesDuringNotes)
{
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "tempo_changes_during_notes/tempo_changes_during_notes.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [WHEN] The articulation profiles repository will be returning profiles
    m_defaultProfile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    m_defaultProfile->setPattern(ArticulationType::Tremolo8th, buildTestArticulationPattern());
    m_defaultProfile->setPattern(ArticulationType::Pedal, buildTestArticulationPattern());

    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId()).originEvents;

    // [GIVEN] Expected events durations
    auto quarterAtTempo = [](double tempo) { return static_cast<duration_t>(QUARTER_NOTE_DURATION / (tempo / 120.0)); };

    const std::vector<duration_t> expectedDurations {
        // Tied note of two measures long, with tempo changes in the middle of it
        2 * quarterAtTempo(100) + 4 * quarterAtTempo(10) + 2 * quarterAtTempo(100),

        // Tied note of two measures long, with ritenuto over it
        quarterAtTempo(100) + quarterAtTempo(95) + quarterAtTempo(90) + quarterAtTempo(85) + quarterAtTempo(80) + quarterAtTempo(75)
        + quarterAtTempo(70) + quarterAtTempo(65),

        // Tied note of two measures long, with tempo changes in the middle of it, with eighth notes tremolo
        static_cast<duration_t>(quarterAtTempo(100) * 0.5),
        static_cast<duration_t>(quarterAtTempo(100) * 0.5),
        static_cast<duration_t>(quarterAtTempo(100) * 0.5),
        static_cast<duration_t>(quarterAtTempo(100) * 0.5),
        static_cast<duration_t>(quarterAtTempo(10) * 0.5),
        static_cast<duration_t>(quarterAtTempo(10) * 0.5),
        static_cast<duration_t>(quarterAtTempo(10) * 0.5),
        static_cast<duration_t>(quarterAtTempo(10) * 0.5),
        static_cast<duration_t>(quarterAtTempo(10) * 0.5),
        static_cast<duration_t>(quarterAtTempo(10) * 0.5),
        static_cast<duration_t>(quarterAtTempo(10) * 0.5),
        static_cast<duration_t>(quarterAtTempo(10) * 0.5),
        static_cast<duration_t>(quarterAtTempo(100) * 0.5),
        static_cast<duration_t>(quarterAtTempo(100) * 0.5),
        static_cast<duration_t>(quarterAtTempo(100) * 0.5),
        static_cast<duration_t>(quarterAtTempo(100) * 0.5),

        // Tied note of two measures long, with ritenuto over it, with eighth notes tremolo
        static_cast<duration_t>(quarterAtTempo(100) * 0.5),
        static_cast<duration_t>(quarterAtTempo(100) * 0.5),
        static_cast<duration_t>(quarterAtTempo(95) * 0.5),
        static_cast<duration_t>(quarterAtTempo(95) * 0.5),
        static_cast<duration_t>(quarterAtTempo(90) * 0.5),
        static_cast<duration_t>(quarterAtTempo(90) * 0.5),
        static_cast<duration_t>(quarterAtTempo(85) * 0.5),
        static_cast<duration_t>(quarterAtTempo(85) * 0.5),
        static_cast<duration_t>(quarterAtTempo(80) * 0.5),
        static_cast<duration_t>(quarterAtTempo(80) * 0.5),
        static_cast<duration_t>(quarterAtTempo(75) * 0.5),
        static_cast<duration_t>(quarterAtTempo(75) * 0.5),
        static_cast<duration_t>(quarterAtTempo(70) * 0.5),
        static_cast<duration_t>(quarterAtTempo(70) * 0.5),
        static_cast<duration_t>(quarterAtTempo(65) * 0.5),
        static_cast<duration_t>(quarterAtTempo(65) * 0.5),

        // Same as beginning, but now with pedal
        2 * quarterAtTempo(100) + 4 * quarterAtTempo(10) + 2 * quarterAtTempo(100),
        quarterAtTempo(100) + quarterAtTempo(95) + quarterAtTempo(90) + quarterAtTempo(85) + quarterAtTempo(80) + quarterAtTempo(75)
        + quarterAtTempo(70) + quarterAtTempo(65),
    };

    // Allow slight deviations because of rounding errors
    auto approxEqual = [](duration_t lhs, duration_t rhs) {
        if (std::abs(lhs - rhs) < 2000) {
            return testing::AssertionSuccess();
        } else {
            return testing::AssertionFailure() << lhs << " vs " << rhs;
        }
    };

    // [THEN] Events match expectations
    size_t index = 0;
    for (const auto& pair : result) {
        for (const auto& event : pair.second) {
            SCOPED_TRACE("event " + std::to_string(index) + " at timestamp " + std::to_string(pair.first));

            ASSERT_TRUE(index < expectedDurations.size());

            ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));
            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            EXPECT_TRUE(approxEqual(noteEvent.arrangementCtx().nominalDuration, expectedDurations.at(index)));

            if (index >= expectedDurations.size() - 2) { // For the last two notes
                // [THEN] Expect Pedal to be present
                EXPECT_EQ(noteEvent.expressionCtx().articulations.size(), 1);

                const ArticulationMap::PairType& articulation = *noteEvent.expressionCtx().articulations.cbegin();

                // and have same length as note
                EXPECT_EQ(articulation.first, ArticulationType::Pedal);
                EXPECT_TRUE(approxEqual(articulation.second.meta.timestamp, noteEvent.arrangementCtx().nominalTimestamp));
                EXPECT_TRUE(approxEqual(articulation.second.meta.overallDuration, noteEvent.arrangementCtx().nominalDuration));
            }

            ++index;
        }
    }

    // [THEN] Number of events matches expectations
    ASSERT_EQ(index, expectedDurations.size());
}

/**
 * @brief PlaybackModelTests_Metronome_4_4
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 4 measures
 *          Measure 1: 4 quarter notes, Measure 2: full measure rest, Measure 3: 8 eighth-notes, Measure 4: 16 sixteen-notes
 *          So that we'll be playing 16 beats in total
 */
TEST_F(Engraving_PlaybackModelTests, Metronome_4_4)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "metronome_4_4/metronome_4_4.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of metronome events - 4 beats on every measure * 4 measures which should be played
    int expectedSize = 16;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded with Metronome enabled
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.setIsMetronomeEnabled(true);
    model.load(score);

    const PlaybackEventsMap& eventsWhenMetronomeEnabled = model.resolveTrackPlaybackData(model.metronomeTrackId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(eventsWhenMetronomeEnabled.size(), expectedSize);

    // [WHEN] The playback model requested to be loaded with Metronome disabled
    model.setIsMetronomeEnabled(false);
    model.reload();

    const PlaybackEventsMap& eventsWhenMetronomeDisabled = model.resolveTrackPlaybackData(model.metronomeTrackId()).originEvents;

    // [THEN] No Metronome events
    EXPECT_TRUE(eventsWhenMetronomeDisabled.empty());

    delete score;
}

/**
 * @brief PlaybackModelTests_Metronome_6_4_Repeat
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 4 measures
 *          Additionally, there is a simple repeat on measure 2. In total, we'll be playing 30 beats, including repeated measure
 */
TEST_F(Engraving_PlaybackModelTests, Metronome_6_4_Repeat)
{
    // [GIVEN] Simple piece of score (Violin, 6/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_MODEL_TEST_FILES_DIR + "metronome_6_4_with_repeat/metronome_6_4_with_repeat.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of metronome events - 6 beats on every measure * 5 measures which should be played
    int expectedSize = 30;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.setIsMetronomeEnabled(true);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(model.metronomeTrackId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);

    delete score;
}

/**
 * @brief PlaybackModelTests_Note_Entry_Playback_Note
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 1 measure
 *          Additionally, we'll emulate the situation where user clicks on the first note, so that we should playback it
 *
 */
TEST_F(Engraving_PlaybackModelTests, Note_Entry_Playback_Note)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_MODEL_TEST_FILES_DIR + "note_entry_playback/note_entry_playback_note.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] The articulation profiles repository will be returning required profiles
    ON_CALL(*m_repositoryMock, defaultProfile(_)).WillByDefault(Return(m_defaultProfile));

    // [GIVEN] The very first note of the score
    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    const Chord* chord = toChord(firstSegment->nextChordRest(0));
    ASSERT_TRUE(chord);
    ASSERT_TRUE(chord->notes().size() == 1);

    const Note* firstNote = chord->notes().front();
    mpe::timestamp_t firstNoteTimestamp = 0;

    // [GIVEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    PlaybackData result = model.resolveTrackPlaybackData(part->id(), part->instrumentId());

    // [GIVEN] Expected note event
    const mpe::NoteEvent& expectedEvent = std::get<mpe::NoteEvent>(result.originEvents.at(firstNoteTimestamp).front());

    // [THEN] Triggered events map will match our expectations
    result.offStream.onReceive(this, [firstNoteTimestamp, expectedEvent](const PlaybackEventsMap& triggeredEvents,
                                                                         const DynamicLevelLayers& triggeredDynamics,
                                                                         bool flushOffstream) {
        ASSERT_EQ(triggeredEvents.size(), 1);
        const PlaybackEventList& eventList = triggeredEvents.at(firstNoteTimestamp);

        ASSERT_EQ(eventList.size(), 1);
        const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(eventList.front());

        EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, expectedEvent.arrangementCtx().actualTimestamp);
        EXPECT_EQ(noteEvent.expressionCtx(), expectedEvent.expressionCtx());
        EXPECT_EQ(noteEvent.pitchCtx(), expectedEvent.pitchCtx());

        // Use the score dynamics for offstream playback by default
        ASSERT_EQ(triggeredDynamics.size(), 1);
        const mpe::DynamicLevelMap& dynamicsMap = triggeredDynamics.at(0);
        ASSERT_EQ(dynamicsMap.size(), 1);
        EXPECT_EQ(dynamicsMap.begin()->second, dynamicLevelFromType(mpe::DynamicType::ppp));

        EXPECT_TRUE(flushOffstream);
    });

    // [WHEN] User has clicked on the first note
    model.triggerEventsForItems({ firstNote }, QUARTER_NOTE_DURATION, true /*flushSounds*/);
}

/**
 * @brief PlaybackModelTests_Note_Entry_Playback_Chord
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 1 measure
 *          Additionally, we'll emulate the situation where user wants to playback the third chord and make sure that we'll sent
 *          all the necessary events into audio-engine
 *
 */
TEST_F(Engraving_PlaybackModelTests, Note_Entry_Playback_Chord)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_MODEL_TEST_FILES_DIR + "note_entry_playback/note_entry_playback_chord.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] The articulation profiles repository will be returning required profiles
    ON_CALL(*m_repositoryMock, defaultProfile(_)).WillByDefault(Return(m_defaultProfile));

    // [GIVEN] The third chord of the score
    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    size_t expectedNoteCount = 3;
    int thirdChordPositionTick = 480 * 2;
    timestamp_t thirdChordTimestamp = 500000 * 2;

    const Chord* thirdChord = firstMeasure->findChord(Fraction::fromTicks(thirdChordPositionTick), 0);
    ASSERT_TRUE(thirdChord);
    ASSERT_TRUE(thirdChord->notes().size() == expectedNoteCount);

    // [GIVEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    PlaybackData result = model.resolveTrackPlaybackData(part->id(), part->instrumentId());

    // [GIVEN] Expected note event
    const PlaybackEventList& expectedEvents = result.originEvents.at(thirdChordTimestamp);

    // [THEN] Triggered events map will match our expectations
    result.offStream.onReceive(this, [expectedEvents](const PlaybackEventsMap& triggeredEvents,
                                                      const DynamicLevelLayers& triggeredDynamics,
                                                      bool flushOffstream) {
        ASSERT_EQ(triggeredEvents.size(), 1);
        const PlaybackEventList& actualEvents = triggeredEvents.at(0);
        ASSERT_EQ(actualEvents.size(), expectedEvents.size());

        for (size_t i = 0; i < expectedEvents.size(); ++i) {
            const mpe::NoteEvent expectedNoteEvent = std::get<mpe::NoteEvent>(expectedEvents.at(i));
            const mpe::NoteEvent actualNoteEvent = std::get<mpe::NoteEvent>(actualEvents.at(i));

            EXPECT_TRUE(actualNoteEvent.arrangementCtx().actualTimestamp == 0);
            EXPECT_FALSE(actualNoteEvent.expressionCtx() == expectedNoteEvent.expressionCtx());
            EXPECT_TRUE(actualNoteEvent.pitchCtx() == expectedNoteEvent.pitchCtx());

            EXPECT_TRUE(triggeredDynamics.empty());
            EXPECT_TRUE(flushOffstream);
        }
    });

    // [WHEN] Don't use the score dynamics
    model.setUseScoreDynamicsForOffstreamPlayback(false);

    // [WHEN] User has clicked on the first note
    model.triggerEventsForItems({ thirdChord }, QUARTER_NOTE_DURATION, true /*flushSounds*/);
}

/**
 * @brief PlaybackModelTests_Playback_Setup_Data_MultiInstrument
 * @details In this case we're building up a playback model of a score with 12 instruments:
 *          - sopranissimo-saxophone
 *          - marching-tenor-drums
 *          - hand-clap
 *          - guitar-steel
 *          - bass-steel-drums
 *          - alto-viol
 *          - f-wagner-tuba
 *          - bass-harmonica-hohner
 *          - chinese-tom-toms
 *          - electric-piano
 *          - boy-soprano
 *
 *          We need to make sure that playback setup data will be properly prepared, so that the score will be played with appropriate sounds
 */
TEST_F(Engraving_PlaybackModelTests, Playback_Setup_Data_MultiInstrument)
{
    // [GIVEN] Score with 12 instruments
    Score* score = ScoreRW::readScore(
        PLAYBACK_MODEL_TEST_FILES_DIR + "playback_setup_instruments/playback_setup_instruments.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 12);

    constexpr bool supportsSND = true; // supports single note dynamics

    // [GIVEN] Expected setup data for each instrument
    std::unordered_map<String, mpe::PlaybackSetupData> expectedSetupData = {
        { u"sopranissimo-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Sopranissimo }, supportsSND } },
        { u"marching-tenor-drums", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Marching,
                                                                                  SoundSubCategory::Snare,
                                                                                  SoundSubCategory::Tenor } } },
        { u"hand-clap", { SoundId::Clap, SoundCategory::Percussions, { SoundSubCategory::Hand } } },
        { u"guitar-steel", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                        SoundSubCategory::Steel,
                                                                        SoundSubCategory::Plucked } } },
        { u"bass-steel-drums", { SoundId::SteelDrums, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                                    SoundSubCategory::Steel,
                                                                                    SoundSubCategory::Bass } } },
        { u"alto-viol", { SoundId::Viol, SoundCategory::Strings, { SoundSubCategory::Alto }, supportsSND } },
        { u"f-wagner-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Wagner }, supportsSND } },
        { u"bass-harmonica-hohner", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Bass,
                                                                                  SoundSubCategory::Hohner }, supportsSND } },
        { u"chinese-tom-toms", { SoundId::TomToms, SoundCategory::Percussions, { SoundSubCategory::Chinese } } },
        { u"electric-piano", { SoundId::Piano, SoundCategory::Keyboards, { SoundSubCategory::Electric } } },
        { u"crystal-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                                SoundSubCategory::FX_Crystal }, supportsSND } },
        { u"boy-soprano", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Soprano,
                                                                     SoundSubCategory::Boy }, supportsSND } },
    };

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model(modularity::globalCtx());
    model.profilesRepository.set(m_repositoryMock);
    model.load(score);

    // [THEN] Result matches with our expectations
    for (const Part* part : score->parts()) {
        for (const auto& pair : part->instruments()) {
            const String& instrumentId = pair.second->id();
            const PlaybackData& result = model.resolveTrackPlaybackData(part->id(), instrumentId);

            EXPECT_EQ(result.setupData, expectedSetupData.at(instrumentId));
        }
    }
}
