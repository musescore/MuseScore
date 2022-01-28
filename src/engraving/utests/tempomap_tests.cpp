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

#include "utils/scorerw.h"
#include "realfn.h"
#include "types/constants.h"
#include "libmscore/tempo.h"

using namespace mu;
using namespace mu::engraving;

static const QString TEMPOMAP_TEST_FILES_DIR("tempomap_data/");

class TempoMapTests : public ::testing::Test
{
protected:
    void SetUp() override {}
};

/**
 * @brief TempoMapTests_DEFAULT_TEMPO
 * @details In this case we're loading a simple score with 8 measures (Viollin, 4/4, 120 bpm, Treble Cleff)
 *          There is no visible tempo marking on the score, so default tempo will be applied 120BPM
 */
TEST_F(TempoMapTests, DEFAULT_TEMPO)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(TEMPOMAP_TEST_FILES_DIR + "default_tempo/default_tempo.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempo
    BeatsPerSecond expectedTempo = Constants::defaultTempo;

    // [WHEN] We request score's tempomap it should contain only 1 value, which is our expected tempo
    const Ms::TempoMap* tempoMap = score->tempomap();
    EXPECT_EQ(tempoMap->size(), 1);

    // [THEN] Applied tempo matches our expectations
    for (const auto& pair : *tempoMap) {
        EXPECT_EQ(pair.second.tempo, expectedTempo);
    }
}

/**
 * @brief TempoMapTests_ABSOLUTE_TEMPO_80_BPM
 * @details In this case we're loading a simple score with 8 measures (Viollin, 4/4, 80 bpm, Treble Cleff)
 *          Tempo marking (80 BPM) should be applied on the entire score
 */
TEST_F(TempoMapTests, ABSOLUTE_TEMPO_80_BPM)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(TEMPOMAP_TEST_FILES_DIR + "custom_tempo_80_bpm/custom_tempo_80_bpm.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempo
    BeatsPerSecond expectedTempo = BeatsPerSecond::fromBPM(BeatsPerMinute(80.f));

    // [WHEN] We request score's tempomap it should contain only 1 value, which is our expected tempo
    const Ms::TempoMap* tempoMap = score->tempomap();
    EXPECT_EQ(tempoMap->size(), 1);

    // [THEN] Applied tempo matches with our expectations
    for (const auto& pair : *tempoMap) {
        EXPECT_TRUE(RealIsEqual(RealRound(pair.second.tempo.val, 2), RealRound(expectedTempo.val, 2)));
    }
}

/**
 * @brief TempoMapTests_ABSOLUTE_TEMPO_FROM_80_TO_120_BPM
 * @details In this case we're loading a simple score with 8 measures (Viollin, 4/4, 80 bpm, Treble Cleff)
 *          There is a tempo marking (80 BPM) on the very first measure. The 4-th measure marked by 120BPM tempo
 */
TEST_F(TempoMapTests, ABSOLUTE_TEMPO_FROM_80_TO_120_BPM)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 80 bpm, Treble Cleff)
    Ms::Score* score = ScoreRW::readScore(TEMPOMAP_TEST_FILES_DIR + "absolute_tempo_80_to_120_bpm/absolute_tempo_80_to_120_bpm.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempomap
    std::map<int, BeatsPerSecond> expectedTempoMap = {
        { 0, BeatsPerSecond::fromBPM(BeatsPerMinute(80.f)) }, // first measure
        { 4 * 4 * Constants::division, BeatsPerSecond::fromBPM(BeatsPerMinute(120.f)) } // 4-th measure
    };

    // [WHEN] We request score's tempomap its size matches with our expectations
    const Ms::TempoMap* tempoMap = score->tempomap();
    EXPECT_EQ(tempoMap->size(), expectedTempoMap.size());

    // [THEN] Applied tempo matches with our expectations
    for (const auto& pair : *tempoMap) {
        EXPECT_TRUE(RealIsEqual(RealRound(pair.second.tempo.val, 2), RealRound(expectedTempoMap.at(pair.first).val, 2)));
    }
}

/**
 * @brief TempoMapTests_GRADUAL_TEMPO_CHANGE_ACCELERANDO
 * @details In this case we're loading a simple score with 8 measures (Viollin, 4/4, 120 bpm, Treble Cleff)
 *          There is a tempo marking (120 BPM) on the very first measure. Additionally, there is "accelerando" tempo annotation
 *          above measures 5 and 6
 */
TEST_F(TempoMapTests, GRADUAL_TEMPO_CHANGE_ACCELERANDO)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score
        = ScoreRW::readScore(TEMPOMAP_TEST_FILES_DIR + "gradual_tempo_change_accelerando/gradual_tempo_change_accelerando.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempomap
    std::map<int, BeatsPerSecond> expectedTempoMap = {
        { 0, BeatsPerSecond::fromBPM(BeatsPerMinute(120.f)) }, // beginning of the first measure
        { 6 * 4 * Constants::division, BeatsPerSecond::fromBPM(BeatsPerMinute(150.f)) } // beginning of the last measure
    };

    // [WHEN] We request score's tempomap its size matches with our expectations
    const Ms::TempoMap* tempoMap = score->tempomap();
    EXPECT_FALSE(tempoMap->empty());

    // [THEN] Applied tempo matches with our expectations
    for (const auto& pair : expectedTempoMap) {
        EXPECT_TRUE(RealIsEqual(RealRound(tempoMap->at(pair.first).tempo.val, 2), RealRound(pair.second.val, 2)));
    }
}

/**
 * @brief TempoMapTests_GRADUAL_TEMPO_CHANGE_RALLENTANDO
 * @details In this case we're loading a simple score with 8 measures (Viollin, 4/4, 120 bpm, Treble Cleff)
 *          There is a tempo marking (120 BPM) on the very first measure. Additionally, there is "rallentando" tempo annotation
 *          above measures 5 and 6
 */
TEST_F(TempoMapTests, GRADUAL_TEMPO_CHANGE_RALLENTANDO)
{
    // [GIVEN] Simple piece of score (Viollin, 4/4, 120 bpm, Treble Cleff)
    Ms::Score* score
        = ScoreRW::readScore(TEMPOMAP_TEST_FILES_DIR + "gradual_tempo_change_rallentando/gradual_tempo_change_rallentando.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempomap
    std::map<int, BeatsPerSecond> expectedTempoMap = {
        { 0, BeatsPerSecond::fromBPM(BeatsPerMinute(120.f)) }, // beginning of the first measure
        { 6 * 4 * Constants::division, BeatsPerSecond::fromBPM(BeatsPerMinute(90.f)) } // beginning of the last measure
    };

    // [WHEN] We request score's tempomap its size matches with our expectations
    const Ms::TempoMap* tempoMap = score->tempomap();
    EXPECT_FALSE(tempoMap->empty());

    // [THEN] Applied tempo matches with our expectations
    for (const auto& pair : expectedTempoMap) {
        EXPECT_TRUE(RealIsEqual(RealRound(tempoMap->at(pair.first).tempo.val, 2), RealRound(pair.second.val, 2)));
    }
}
