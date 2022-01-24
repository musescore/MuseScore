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
#include <gmock/gmock.h>
#include <memory>

#include "async/channel.h"
#include "mpe/tests/utils/articulationutils.h"
#include "mpe/tests/mocks/articulationprofilesrepositorymock.h"

#include "utils/scorerw.h"
#include "libmscore/part.h"

#include "playback/playbackmodel.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::AnyOf;

using namespace mu::engraving;
using namespace mu::mpe;
using namespace mu::async;

static const QString PLAYBACK_MODEL_TEST_FILES_DIR("playbackmodel_data/");

class PlaybackModelTests : public ::testing::Test
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

        m_repositoryMock = std::make_shared<ArticulationProfilesRepositoryMock>();
    }

    ArticulationsProfilePtr m_defaultProfile = nullptr;

    ArticulationPattern m_dummyPattern;
    ArticulationPatternSegment m_dummyPatternSegment;

    std::shared_ptr<ArticulationProfilesRepositoryMock> m_repositoryMock = nullptr;
    Channel<int, int, int, int> m_notationChangesRangeChannel;
};

/**
 * @brief PlaybackModelTests_SimpleRepeat
 * @details In this case we're building up a playback model of a simple score - Viollin, 4/4, 120bpm, Treble Cleff, 4 measures
 *          Additionally, there is a simple repeat from measure 2 up to measure 3. In total, we'll be playing 6 measures overall
 */
TEST_F(PlaybackModelTests, SimpleRepeat)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_range/repeat_range.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Ms::Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments()->size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 6 overall measures which should be played
    int expectedSize = 24;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(ArticulationFamily::StringsArticulation)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score, m_notationChangesRangeChannel);

    const PlaybackEventsMap& result = model.events(part->id(), part->instrumentId().toStdString());

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Two_Ending_Repeat
 * @details In this case we're building up a playback model of a simple score - Viollin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a repeat at the end of the second measure. Measure 2 is the first ending of the repeat.
 *          Measure 3 is the second ending of the repeat. In total, we'll be playing 7 measures overall
 */
TEST_F(PlaybackModelTests, Two_Ending_Repeat)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_with_2_voltas/repeat_with_2_voltas.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Ms::Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments()->size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 7 measures which should be played
    int expectedSize = 28;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(ArticulationFamily::StringsArticulation)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score, m_notationChangesRangeChannel);

    const PlaybackEventsMap& result = model.events(part->id(), part->instrumentId().toStdString());

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Da_Capo_Al_Fine
 * @details In this case we're building up a playback model of a simple score - Viollin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.C. Al Fine" marking at the end of the 6-th measure. Measure 2 is marked by "Fine"
 *          In total, we'll be playing 8 measures overall
 */
TEST_F(PlaybackModelTests, Da_Capo_Al_Fine)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "da_capo_al_fine/da_capo_al_fine.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Ms::Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments()->size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 8 measures which should be played
    int expectedSize = 32;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(ArticulationFamily::StringsArticulation)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score, m_notationChangesRangeChannel);

    const PlaybackEventsMap& result = model.events(part->id(), part->instrumentId().toStdString());

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Dal_Segno_Al_Coda
 * @details In this case we're building up a playback model of a simple score - Viollin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.S. Al Coda" marking at the end of the 4-th measure. Measure 2 is marked by "Segno" marking.
 *          The end of the 3-rd measure is marked by "To Coda" marking. The beginning of the 5-th measure is marked by "Coda" sign
 *          In total, we'll be playing 8 measures overall
 */
TEST_F(PlaybackModelTests, Dal_Segno_Al_Coda)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "dal_segno_al_coda/dal_segno_al_coda.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Ms::Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments()->size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 8 measures which should be played
    int expectedSize = 32;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(ArticulationFamily::StringsArticulation)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score, m_notationChangesRangeChannel);

    const PlaybackEventsMap& result = model.events(part->id(), part->instrumentId().toStdString());

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Dal_Segno_Al_Fine
 * @details In this case we're building up a playback model of a simple score - Viollin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.S. Al Fine" marking at the end of the 6-th measure. Measure 2 is marked by "Segno" marking.
 *          The end of the 4-th measure is marked by "Fine" marking. In total, we'll be playing 9 measures overall
 */
TEST_F(PlaybackModelTests, Dal_Segno_Al_Fine)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "dal_segno_al_fine/dal_segno_al_fine.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Ms::Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments()->size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 9 measures which should be played
    int expectedSize = 36;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(ArticulationFamily::StringsArticulation)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score, m_notationChangesRangeChannel);

    const PlaybackEventsMap& result = model.events(part->id(), part->instrumentId().toStdString());

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Da_Capo_Al_Coda
 * @details In this case we're building up a playback model of a simple score - Viollin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.C. Al Coda" marking at the end of the 6-th measure. The end of the measure 2 is marked by "To Coda".
 *          The beginning of the 4-th measure is marked by "Coda" sign. In total, we'll be playing 11 measures overall
 */
TEST_F(PlaybackModelTests, Da_Capo_Al_Coda)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "da_capo_al_coda/da_capo_al_coda.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Ms::Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments()->size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 11 measures which should be played
    int expectedSize = 44;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(ArticulationFamily::StringsArticulation)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score, m_notationChangesRangeChannel);

    const PlaybackEventsMap& result = model.events(part->id(), part->instrumentId().toStdString());

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Da_Capo_Al_Coda
 * @details In this case we're building up a playback model of a simple score - Viollin, 4/4, 120bpm, Treble Cleff, 1 measure
 *          Additionally, the first note is marked by "pizzicato" + "stacattissimo". The 3-rd note is marked by "arco"
 *          We'll be playing 4 events overall
 */
TEST_F(PlaybackModelTests, Pizz_To_Arco_Technique)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "pizz_to_arco/pizz_to_arco.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Ms::Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments()->size(), 1);

    // [GIVEN] Expected amount of events
    int expectedSize = 4;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(ArticulationFamily::StringsArticulation)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score, m_notationChangesRangeChannel);

    const PlaybackEventsMap& result = model.events(part->id(), part->instrumentId().toStdString());

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);

    // [THEN] The first note has Pizzicato and Staccatissimo articulations applied
    const NoteEvent& firstNoteEvent = std::get<NoteEvent>(result.at(0).at(0));
    EXPECT_EQ(firstNoteEvent.expressionCtx().articulations.size(), 2);
    EXPECT_TRUE(firstNoteEvent.expressionCtx().articulations.contains(ArticulationType::Pizzicato));
    EXPECT_TRUE(firstNoteEvent.expressionCtx().articulations.contains(ArticulationType::Staccatissimo));

    // [THEN] The second note has only Pizzicato articulation applied
    const NoteEvent& secondNoteEvent = std::get<NoteEvent>(result.at(500).at(0));
    EXPECT_EQ(secondNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(secondNoteEvent.expressionCtx().articulations.contains(ArticulationType::Pizzicato));

    // [THEN] The third note has only Standard articulation applied
    const NoteEvent& thirdNoteEvent = std::get<NoteEvent>(result.at(1000).at(0));
    EXPECT_EQ(thirdNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(thirdNoteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));

    // [THEN] The fourth note has only Standard articulation applied
    const NoteEvent& fourthNoteEvent = std::get<NoteEvent>(result.at(1500).at(0));
    EXPECT_EQ(fourthNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(fourthNoteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));
}

/**
 * @brief PlaybackModelTests_Repeat_Last_Measure
 * @details In this case we're building up a playback model of a simple score - Viollin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "repeat last measure" sign on the 6-th measure. In total, we'll be playing 7 measures overall
 *
 * @bug The test is currently disabled. At the moment it shows a flaw in libmscore - repeatSegments. RepeatSegments calculations don't
 *      take into account MeasureRepeat elements which leads to issues with playback model. Whenever the root issue will be finished, this test
 *      will be enabled
 */
TEST_F(PlaybackModelTests, DISABLED_Repeat_Last_Measure)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_last_measure/repeat_last_measure.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Ms::Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments()->size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 7 measures which should be played
    int expectedSize = 24;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(ArticulationFamily::StringsArticulation)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score, m_notationChangesRangeChannel);

    const PlaybackEventsMap& result = model.events(part->id(), part->instrumentId().toStdString());

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}
