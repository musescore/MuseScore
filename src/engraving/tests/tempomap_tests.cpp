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

#include "utils/scorerw.h"
#include "realfn.h"
#include "types/constants.h"
#include "dom/tempo.h"

using namespace mu;
using namespace mu::engraving;

static const String TEMPOMAP_TEST_FILES_DIR("tempomap_data/");

class Engraving_TempoMapTests : public ::testing::Test
{
protected:
    void SetUp() override {}
};

/**
 * @brief TempoMapTests_DEFAULT_TEMPO
 * @details In this case we're loading a simple score with 8 measures (Violin, 4/4, 120 bpm, Treble Cleff)
 *          There is no visible tempo marking on the score, so default tempo will be applied 120BPM
 */
TEST_F(Engraving_TempoMapTests, DEFAULT_TEMPO)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(TEMPOMAP_TEST_FILES_DIR + "default_tempo/default_tempo.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempo
    BeatsPerSecond expectedTempo = Constants::DEFAULT_TEMPO;

    // [WHEN] We request score's tempomap it should contain only 1 value, which is our expected tempo
    const TempoMap* tempoMap = score->tempomap();
    EXPECT_EQ(tempoMap->size(), 1);

    // [THEN] Applied tempo matches our expectations
    for (const auto& pair : *tempoMap) {
        EXPECT_EQ(pair.second.tempo, expectedTempo);
    }
}

/**
 * @brief TempoMapTests_ABSOLUTE_TEMPO_80_BPM
 * @details In this case we're loading a simple score with 8 measures (Violin, 4/4, 80 bpm, Treble Cleff)
 *          Tempo marking (80 BPM) should be applied on the entire score
 */
TEST_F(Engraving_TempoMapTests, ABSOLUTE_TEMPO_80_BPM)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(TEMPOMAP_TEST_FILES_DIR + "custom_tempo_80_bpm/custom_tempo_80_bpm.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempo
    BeatsPerSecond expectedTempo = BeatsPerSecond::fromBPM(BeatsPerMinute(80.f));

    // [WHEN] We request score's tempomap it should contain only 1 value, which is our expected tempo
    const TempoMap* tempoMap = score->tempomap();
    EXPECT_EQ(tempoMap->size(), 1);

    // [THEN] Applied tempo matches with our expectations
    for (const auto& pair : *tempoMap) {
        EXPECT_TRUE(muse::RealIsEqual(muse::RealRound(pair.second.tempo.val, 2), muse::RealRound(expectedTempo.val, 2)));
    }
}

/**
 * @brief TempoMapTests_ABSOLUTE_TEMPO_FROM_80_TO_120_BPM
 * @details In this case we're loading a simple score with 8 measures (Violin, 4/4, 80 bpm, Treble Cleff)
 *          There is a tempo marking (80 BPM) on the very first measure. The 4-th measure marked by 120BPM tempo
 */
TEST_F(Engraving_TempoMapTests, ABSOLUTE_TEMPO_FROM_80_TO_120_BPM)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 80 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        TEMPOMAP_TEST_FILES_DIR + "absolute_tempo_80_to_120_bpm/absolute_tempo_80_to_120_bpm.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempomap
    std::map<int, BeatsPerSecond> expectedTempoMap = {
        { 0, BeatsPerSecond::fromBPM(BeatsPerMinute(80.f)) }, // first measure
        { 4 * 4 * Constants::DIVISION, BeatsPerSecond::fromBPM(BeatsPerMinute(120.f)) } // 4-th measure
    };

    // [WHEN] We request score's tempomap its size matches with our expectations
    const TempoMap* tempoMap = score->tempomap();
    EXPECT_EQ(tempoMap->size(), expectedTempoMap.size());

    // [THEN] Applied tempo matches with our expectations
    for (const auto& pair : *tempoMap) {
        EXPECT_TRUE(muse::RealIsEqual(muse::RealRound(pair.second.tempo.val, 2), muse::RealRound(expectedTempoMap.at(pair.first).val, 2)));
    }
}

/**
 * @brief TempoMapTests_TEMPO_MULTIPLIER
 * @details In this case we're loading a simple score with 8 measures (Violin, 4/4, 80 bpm, Treble Cleff)
 *          There is a originalTempo marking (80 BPM) on the very first measure. The 4-th measure marked by 120BPM originalTempo
 *          Then we apply a global multiplier to all originalTempo marks
 */
TEST_F(Engraving_TempoMapTests, TEMPO_MULTIPLIER)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 80 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        TEMPOMAP_TEST_FILES_DIR + "absolute_tempo_80_to_120_bpm/absolute_tempo_80_to_120_bpm.mscx");

    ASSERT_TRUE(score);

    // [WHEN] Apply a global tempo multiplier to all tempo marks
    TempoMap* tempoMap = score->tempomap();

    constexpr double multiplier = 2.2;
    tempoMap->setTempoMultiplier(multiplier);

    // [GIVEN] Expected tempomap
    std::map<int, BeatsPerSecond> expectedTempoMap = {
        { 0, BeatsPerSecond::fromBPM(BeatsPerMinute(80.0)) }, // first measure
        { 4 * 4 * Constants::DIVISION, BeatsPerSecond::fromBPM(BeatsPerMinute(120.0)) } // 4-th measure
    };

    // [WHEN] We request score's tempomap its size matches with our expectations
    EXPECT_EQ(tempoMap->size(), expectedTempoMap.size());

    // [THEN] Applied tempo matches with our expectations
    for (int tick : muse::keys(*tempoMap)) {
        double expectedBps = expectedTempoMap[tick].val;

        EXPECT_TRUE(muse::RealIsEqual(muse::RealRound(tempoMap->originalTempo(tick).val, 2), muse::RealRound(expectedBps * multiplier, 2)));
        EXPECT_TRUE(muse::RealIsEqual(muse::RealRound(tempoMap->at(tick).tempo.val, 2), muse::RealRound(expectedBps, 2)));
    }
}

/**
 * @brief TempoMapTests_GRADUAL_TEMPO_CHANGE_ACCELERANDO
 * @details In this case we're loading a simple score with 8 measures (Violin, 4/4, 120 bpm, Treble Cleff)
 *          There is a tempo marking (120 BPM) on the very first measure. Additionally, there is "accelerando" tempo annotation
 *          above measures 5 and 6
 */
TEST_F(Engraving_TempoMapTests, GRADUAL_TEMPO_CHANGE_ACCELERANDO)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score
        = ScoreRW::readScore(TEMPOMAP_TEST_FILES_DIR + "gradual_tempo_change_accelerando/gradual_tempo_change_accelerando.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempomap
    std::map<int, BeatsPerSecond> expectedTempoMap = {
        { 0, BeatsPerSecond::fromBPM(BeatsPerMinute(120.f)) }, // beginning of the first measure
        { 6 * 4 * Constants::DIVISION, BeatsPerSecond::fromBPM(BeatsPerMinute(159.6f)) } // beginning of the last measure
    };

    // [WHEN] We request score's tempomap its size matches with our expectations
    const TempoMap* tempoMap = score->tempomap();
    EXPECT_FALSE(tempoMap->empty());

    // [THEN] Applied tempo matches with our expectations
    for (const auto& pair : expectedTempoMap) {
        EXPECT_TRUE(muse::RealIsEqual(muse::RealRound(tempoMap->at(pair.first).tempo.val, 2), muse::RealRound(pair.second.val, 2)));
    }
}

/**
 * @brief TempoMapTests_GRADUAL_TEMPO_CHANGE_RALLENTANDO
 * @details In this case we're loading a simple score with 8 measures (Violin, 4/4, 120 bpm, Treble Cleff)
 *          There is a tempo marking (120 BPM) on the very first measure. Additionally, there is "rallentando" tempo annotation
 *          above measures 5 and 6
 */
TEST_F(Engraving_TempoMapTests, GRADUAL_TEMPO_CHANGE_RALLENTANDO)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(TEMPOMAP_TEST_FILES_DIR + "gradual_tempo_change_rallentando/gradual_tempo_change_rallentando.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempomap
    std::map<int, BeatsPerSecond> expectedTempoMap = {
        { 0, BeatsPerSecond::fromBPM(BeatsPerMinute(120.f)) }, // beginning of the first measure
        { 6 * 4 * Constants::DIVISION, BeatsPerSecond::fromBPM(BeatsPerMinute(90.f)) } // beginning of the last measure
    };

    // [WHEN] We request score's tempomap its size matches with our expectations
    const TempoMap* tempoMap = score->tempomap();
    EXPECT_FALSE(tempoMap->empty());

    // [THEN] Applied tempo matches with our expectations
    for (const auto& pair : expectedTempoMap) {
        EXPECT_TRUE(muse::RealIsEqual(muse::RealRound(tempoMap->at(pair.first).tempo.val, 2), muse::RealRound(pair.second.val, 2)));
    }
}

/**
 * @brief TempoMapTests_GRADUAL_TEMPO_CHANGE_DOESNT_OVERWRITE_OTHER_TEMPO
 * @details In this case we're loading a simple score with 3 measures (Piano, 4/4, Treble Cleff)
 *          There is a "ritardando" tempo annotation on the very first measure, which continues throughout the second measure.
 *          There is a tempo marking at the end of the first measure (80 BPM).
 *          Additionally, there is "presto" tempo annotation at the beginning of the third measure.
 *
 *          Check that the "ritardando" tempo doesn't overwrite the tempo marking (80 BMP) and the "presto" tempo
 *          See: https://github.com/musescore/MuseScore/issues/12140
 */
TEST_F(Engraving_TempoMapTests, GRADUAL_TEMPO_CHANGE_DOESNT_OVERWRITE_OTHER_TEMPO)
{
    // [GIVEN] Simple piece of score (Piano, 4/4, Treble Cleff)
    Score* score = ScoreRW::readScore(
        TEMPOMAP_TEST_FILES_DIR
        + "gradual_tempo_change_doesnt_overwrite_other_tempo/gradual_tempo_change_doesnt_overwrite_other_tempo.mscx");

    ASSERT_TRUE(score);

    // [GIVEN] Expected tempomap
    std::map<int, BeatsPerSecond> expectedTempoMap = {
        // ritardando (beginning of the first measure)
        { 0, BeatsPerSecond::fromBPM(BeatsPerMinute(120.f)) },
        { 960, BeatsPerSecond::fromBPM(BeatsPerMinute(112.5f)) },
        // tempo marking (80 BPM, the first measure)
        { 1440, BeatsPerSecond::fromBPM(BeatsPerMinute(80.f)) },
        // ritardando (the second measure)
        { 1920, BeatsPerSecond::fromBPM(BeatsPerMinute(105.f)) },
        { 2880, BeatsPerSecond::fromBPM(BeatsPerMinute(97.5f)) },
        // presto (beginning of the third measure)
        { 3840, BeatsPerSecond::fromBPM(BeatsPerMinute(187.2f)) },
    };

    // [WHEN] We request score's tempomap its size matches with our expectations
    const TempoMap* tempoMap = score->tempomap();
    EXPECT_FALSE(tempoMap->empty());

    // [THEN] Applied tempo matches with our expectations
    for (const auto& pair : expectedTempoMap) {
        EXPECT_TRUE(muse::RealIsEqual(muse::RealRound(tempoMap->at(pair.first).tempo.val, 2), muse::RealRound(pair.second.val, 2)));
    }
}
