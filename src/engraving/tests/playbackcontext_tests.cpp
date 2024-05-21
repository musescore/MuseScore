/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "utils/scorerw.h"

#include "engraving/dom/part.h"
#include "engraving/dom/repeatlist.h"

#include "playback/playbackcontext.h"
#include "playback/utils/arrangementutils.h"

#include "types/typesconv.h"

using namespace mu::engraving;
using namespace mu::mpe;
using namespace mu;

static const mu::String PLAYBACK_CONTEXT_TEST_FILES_DIR("playbackcontext_data/");

static constexpr int HAIRPIN_STEPS = 24;
static constexpr int TICKS_STEP = 480;

class Engraving_PlaybackContextTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        //! NOTE: allows to read test files using their version readers
        //! instead of using 302 (see mscloader.cpp, makeReader)
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = true;
    }
};

TEST_F(Engraving_PlaybackContextTests, Hairpins_Repeats)
{
    // [GIVEN] Score with hairpins and repeats
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "dynamics/hairpins_and_repeats.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    const RepeatList& repeats = score->repeatList();
    ASSERT_EQ(repeats.size(), 2);

    // [GIVEN] Context for parsing dynamics
    PlaybackContext ctx;

    // [WHEN] Parse dynamics
    ctx.update(parts.front()->id(), score);

    // [GIVEN]
    DynamicLevelMap expectedDynamics;

    // [GIVEN] 1st hairpin (inside the repeat): f -> fff
    constexpr mu::mpe::dynamic_level_t f = dynamicLevelFromType(mu::mpe::DynamicType::f);
    constexpr mu::mpe::dynamic_level_t fff = dynamicLevelFromType(mu::mpe::DynamicType::fff);
    constexpr int f_to_fff_startTick = 0;

    std::map<int, int> f_to_fff_curve = TConv::easingValueCurve(1440, HAIRPIN_STEPS, static_cast<int>(fff - f), ChangeMethod::NORMAL);

    for (const mu::engraving::RepeatSegment* repeatSegment : repeats) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const auto& pair : f_to_fff_curve) {
            mu::mpe::timestamp_t time = timestampFromTicks(score, f_to_fff_startTick + pair.first + tickPositionOffset);
            ASSERT_FALSE(mu::contains(expectedDynamics, time));
            expectedDynamics.emplace(time, static_cast<int>(f) + pair.second);
        }
    }

    // [GIVEN] 2nd hairpin (right after the repeat): ppp -> p
    constexpr mu::mpe::dynamic_level_t ppp = dynamicLevelFromType(mu::mpe::DynamicType::ppp);
    constexpr mu::mpe::dynamic_level_t p = dynamicLevelFromType(mu::mpe::DynamicType::p);
    constexpr int ppp_to_p_startTick = 1920 + 1920; // real tick + repeat tick

    std::map<int, int> ppp_to_p_curve = TConv::easingValueCurve(1440, HAIRPIN_STEPS, static_cast<int>(p - ppp), ChangeMethod::NORMAL);

    for (const auto& pair : ppp_to_p_curve) {
        mu::mpe::timestamp_t time = timestampFromTicks(score, ppp_to_p_startTick + pair.first);
        ASSERT_FALSE(mu::contains(expectedDynamics, time));
        expectedDynamics.emplace(time, static_cast<int>(ppp) + pair.second);
    }

    ASSERT_FALSE(expectedDynamics.empty());

    // [WHEN] Get the actual dynamics map
    DynamicLevelMap actualDynamics = ctx.dynamicLevelMap(score);

    // [THEN] The dynamics map matches the expectation
    EXPECT_EQ(actualDynamics, expectedDynamics);

    delete score;
}

TEST_F(Engraving_PlaybackContextTests, Dynamics_MeasureRepeats)
{
    // [GIVEN] Score with 5 measures. There is a measure repeat on the last 2 measures
    // (so the previous 2 measures will be repeated)
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "dynamics/dynamics_and_measure_repeats.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing dynamics
    PlaybackContext ctx;

    // [WHEN] Parse dynamics
    ctx.update(parts.front()->id(), score);

    // [WHEN] Get the actual dynamics map
    DynamicLevelMap actualDynamics = ctx.dynamicLevelMap(score);

    // [THEN] The dynamics map matches the expectation
    DynamicLevelMap expectedDynamics {
        { timestampFromTicks(score, 0), dynamicLevelFromType(mpe::DynamicType::Natural) },

        // 2nd measure
        { timestampFromTicks(score, 1920), dynamicLevelFromType(mpe::DynamicType::ppp) }, // 1st quarter note
        { timestampFromTicks(score, 3360), dynamicLevelFromType(mpe::DynamicType::p) }, // 4th quarter note

        // 3rd measure
        { timestampFromTicks(score, 4320), dynamicLevelFromType(mpe::DynamicType::mf) }, // 2nd quarter note
        { timestampFromTicks(score, 5280), dynamicLevelFromType(mpe::DynamicType::fff) }, // 4th quarter note

        // copy of 2nd measure
        { timestampFromTicks(score, 5760), dynamicLevelFromType(mpe::DynamicType::ppp) },
        { timestampFromTicks(score, 7200), dynamicLevelFromType(mpe::DynamicType::p) },

        // copy of 3rd measure
        { timestampFromTicks(score, 8160), dynamicLevelFromType(mpe::DynamicType::mf) },
        { timestampFromTicks(score, 9120), dynamicLevelFromType(mpe::DynamicType::fff) },
    };

    EXPECT_EQ(actualDynamics, expectedDynamics);
}

TEST_F(Engraving_PlaybackContextTests, PlayTechniques_MeasureRepeats)
{
    // [GIVEN] Score with 5 measures. The 1st measure is repeated. There also is a measure repeat on the last 2 measures
    // (so the previous 2 measures will be repeated)
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "play_techniques/play_techniques_measure_repeats.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] the 1st measure is repeated
    constexpr int repeatOffsetTick = 1920;

    // [GIVEN] Context for parsing playing techniques
    PlaybackContext ctx;

    // [WHEN] Parse playing techniques
    ctx.update(parts.front()->id(), score);

    // [THEN] The articulation map matches the expectation
    std::map<int, mpe::ArticulationType> expectedArticulations {
        // 1st measure
        { 0, mpe::ArticulationType::Standard },

        // 2nd measure
        { 1920 + repeatOffsetTick, mpe::ArticulationType::Mute }, // 1st quarter note

        // 3rd measure
        { 4320 + repeatOffsetTick, mpe::ArticulationType::Distortion }, // 2nd quarter note

        // copy of 2nd measure
        { 5760 + repeatOffsetTick, mpe::ArticulationType::Mute },

        // copy of 3rd measure
        { 8160 + repeatOffsetTick, mpe::ArticulationType::Distortion },
    };

    for (const auto& pair : expectedArticulations) {
        mpe::ArticulationType actualArticulation = ctx.persistentArticulationType(pair.first);
        EXPECT_EQ(actualArticulation, pair.second);
    }
}

TEST_F(Engraving_PlaybackContextTests, SoundFlags)
{
    // [GIVEN] Score (piano with 2 staves) with sound flags
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "sound_flags/sound_flags.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing sound flags
    PlaybackContext ctx;

    // [WHEN] Parse the sound flags
    const Part* part = parts.front();
    ctx.update(part->id(), score);

    // [WHEN] Get the actual params
    PlaybackParamMap params = ctx.playbackParamMap(score);

    // [THEN] Expected params
    staff_layer_idx_t startIdx = 0;
    staff_layer_idx_t endIdx = static_cast<staff_layer_idx_t>(part->nstaves());

    PlaybackParamList sulTasto;
    for (staff_layer_idx_t i = startIdx; i < endIdx; ++i) {
        sulTasto.emplace_back(PlaybackParam { mpe::PLAY_TECHNIQUE_PARAM_CODE, Val("Sul Tasto"), i });
    }

    PlaybackParam bartok { mpe::PLAY_TECHNIQUE_PARAM_CODE, Val("bartok"), 0 };
    PlaybackParam pizz { mpe::PLAY_TECHNIQUE_PARAM_CODE, Val("pizzicato"), 1 }; // "apply to all staves" is off
    PlaybackParam espressivo { mpe::PLAY_TECHNIQUE_PARAM_CODE, Val("Espressivo"), 1 }; // "apply to all staves" is off

    PlaybackParamMap expectedParams {
        { timestampFromTicks(score, 1920), sulTasto },
        { timestampFromTicks(score, 3840), { bartok, pizz } },
        { timestampFromTicks(score, 7680), { espressivo } },
    };

    EXPECT_EQ(params, expectedParams);

    // [THEN] We can get the params for a specific tick & staff
    for (staff_layer_idx_t i = startIdx; i < endIdx; ++i) {
        params = ctx.playbackParamMap(score, 0, i);
        EXPECT_TRUE(params.empty());

        params = ctx.playbackParamMap(score, 2000, i);
        ASSERT_EQ(params.size(), 1);
        EXPECT_EQ(params.begin()->second, PlaybackParamList { sulTasto.at(i) });

        params = ctx.playbackParamMap(score, 4500, i);
        ASSERT_EQ(params.size(), 1);

        if (i == 1) {
            EXPECT_EQ(params.begin()->second, PlaybackParamList { pizz });
        } else {
            EXPECT_EQ(params.begin()->second, PlaybackParamList { bartok });
        }

        params = ctx.playbackParamMap(score, 7680, i);
        ASSERT_EQ(params.size(), 1);

        if (i == 1) {
            EXPECT_EQ(params.begin()->second, PlaybackParamList { espressivo });
        } else {
            EXPECT_EQ(params.begin()->second, PlaybackParamList { bartok });
        }
    }

    delete score;
}

TEST_F(Engraving_PlaybackContextTests, SoundFlags_MeasureRepeats)
{
    // [GIVEN] Score with 5 measures. There is a measure repeat on the last 2 measures
    // (so the previous 2 measures will be repeated)
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "sound_flags/sound_flags_measure_repeats.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing sound flags
    PlaybackContext ctx;

    // [WHEN] Parse sound flags
    ctx.update(parts.front()->id(), score);

    // [THEN] The actual params match the expectation
    PlaybackParamList secondMeasureParams { { mpe::PLAY_TECHNIQUE_PARAM_CODE, Val("Espressivo"), 0 } };
    PlaybackParamList thirdMeasureParams { { mpe::PLAY_TECHNIQUE_PARAM_CODE, Val("bartok"), 0 } };

    PlaybackParamMap expectedParams {
        { timestampFromTicks(score, 1920), secondMeasureParams },
        { timestampFromTicks(score, 3840), thirdMeasureParams },
        { timestampFromTicks(score, 5760), secondMeasureParams }, // measure repeat
        { timestampFromTicks(score, 7680), thirdMeasureParams }, // measure repeat
    };

    PlaybackParamMap actualParams = ctx.playbackParamMap(score);
    EXPECT_EQ(actualParams, expectedParams);
}

/**
 * @brief Engraving_PlaybackContextTests_SoundFlags_CancelPlayingTechniques
 *  @details Checks whether Arco & Open & "Ord." correctly cancel playing techniques. See:
 *  https://github.com/musescore/MuseScore/issues/22403
 */
TEST_F(Engraving_PlaybackContextTests, SoundFlags_CancelPlayingTechniques)
{
    // [GIVEN] Score (violin + brass) with sound flags & playing techniques
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "sound_flags/sound_flags_arco.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2);

    // [GIVEN] Context for parsing sound flags & playing techniques
    PlaybackContext ctx;

    // [WHEN] Parse the violin part
    const Part* violinPart = parts.at(0);
    ctx.update(violinPart->id(), score);

    // [THEN] 1st measure: Pizz.
    for (int tick = 0; tick < 1920; tick += TICKS_STEP) {
        ArticulationType actualType = ctx.persistentArticulationType(tick);
        EXPECT_EQ(actualType, ArticulationType::Pizzicato);
    }
    // [THEN] "Standard" for all the other measures, as Pizz. was canceled with "Ord." in the 2nd measure
    int lastTick = score->lastMeasure()->tick().ticks();
    for (int tick = 1920; tick < lastTick; tick += TICKS_STEP) {
        ArticulationType actualType = ctx.persistentArticulationType(tick);
        EXPECT_EQ(actualType, ArticulationType::Standard);
    }
    // [THEN] The actual params match the expectation
    PlaybackParamList ordinary { { mu::mpe::PLAY_TECHNIQUE_PARAM_CODE, mu::Val(mu::mpe::ORDINARY_PLAYING_TECHNIQUE_CODE), 0 } };
    PlaybackParamList bartok { { mu::mpe::PLAY_TECHNIQUE_PARAM_CODE, mu::Val("bartok"), 0 } };
    PlaybackParamMap expectedParams {
        { timestampFromTicks(score, 1920), ordinary }, // 2nd measure (cancels Pizz.)
        { timestampFromTicks(score, 3840), bartok }, // 3rd measure
        { timestampFromTicks(score, 5760), ordinary }, // 4th (canceled by Arco)
    };
    PlaybackParamMap params = ctx.playbackParamMap(score);
    EXPECT_EQ(params, expectedParams);

    // [WHEN] Parse the brass part
    const Part* brassPart = parts.at(1);
    ctx.clear();
    ctx.update(brassPart->id(), score);

    // [THEN] 1st measure: Standard
    for (int tick = 0; tick < 1920; tick += TICKS_STEP) {
        ArticulationType actualType = ctx.persistentArticulationType(tick);
        EXPECT_EQ(actualType, ArticulationType::Standard);
    }

    // [THEN] "Open" starting from the 2nd measure
    for (int tick = 1920; tick < lastTick; tick += TICKS_STEP) {
        ArticulationType actualType = ctx.persistentArticulationType(tick);
        EXPECT_EQ(actualType, ArticulationType::Open);
    }

    // [THEN] The actual params match the expectation
    PlaybackParamList mute { { mu::mpe::PLAY_TECHNIQUE_PARAM_CODE, mu::Val("mute"), 0 } };

    expectedParams = {
        { timestampFromTicks(score, 0), mute }, // 1st measure
        { timestampFromTicks(score, 1920), ordinary }, // 2nd measure (canceled by Open)
    };

    params = ctx.playbackParamMap(score);
    EXPECT_EQ(params, expectedParams);

    delete score;
}
