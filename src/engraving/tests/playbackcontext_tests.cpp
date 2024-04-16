/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
using namespace muse::mpe;
using namespace muse;

static const muse::String PLAYBACK_CONTEXT_TEST_FILES_DIR("playbackcontext_data/");

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

TEST_F(Engraving_PlaybackContextTests, ParseHairpins_Repeats)
{
    // [GIVEN] Score with hairpins and repeats
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "hairpins_and_repeats.mscx");

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
    constexpr mpe::dynamic_level_t f = dynamicLevelFromType(mpe::DynamicType::f);
    constexpr mpe::dynamic_level_t fff = dynamicLevelFromType(mpe::DynamicType::fff);
    constexpr int f_to_fff_startTick = 0;

    std::map<int, int> f_to_fff_curve = TConv::easingValueCurve(1440, HAIRPIN_STEPS, static_cast<int>(fff - f), ChangeMethod::NORMAL);

    for (const mu::engraving::RepeatSegment* repeatSegment : repeats) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const auto& pair : f_to_fff_curve) {
            mpe::timestamp_t time = timestampFromTicks(score, f_to_fff_startTick + pair.first + tickPositionOffset);
            ASSERT_FALSE(muse::contains(expectedDynamics, time));
            expectedDynamics.emplace(time, static_cast<int>(f) + pair.second);
        }
    }

    // [GIVEN] 2nd hairpin (right after the repeat): ppp -> p
    constexpr mpe::dynamic_level_t ppp = dynamicLevelFromType(mpe::DynamicType::ppp);
    constexpr mpe::dynamic_level_t p = dynamicLevelFromType(mpe::DynamicType::p);
    constexpr int ppp_to_p_startTick = 1920 + 1920; // real tick + repeat tick

    std::map<int, int> ppp_to_p_curve = TConv::easingValueCurve(1440, HAIRPIN_STEPS, static_cast<int>(p - ppp), ChangeMethod::NORMAL);

    for (const auto& pair : ppp_to_p_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, ppp_to_p_startTick + pair.first);
        ASSERT_FALSE(muse::contains(expectedDynamics, time));
        expectedDynamics.emplace(time, static_cast<int>(ppp) + pair.second);
    }

    ASSERT_FALSE(expectedDynamics.empty());

    // [WHEN] Get the actual dynamics map
    DynamicLevelMap actualDynamics = ctx.dynamicLevelMap(score);

    // [THEN] The dynamics map matches the expectation
    EXPECT_EQ(actualDynamics, expectedDynamics);

    delete score;
}

TEST_F(Engraving_PlaybackContextTests, ParseSoundFlags)
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

    PlaybackParamList studio, pop;
    for (staff_layer_idx_t i = startIdx; i < endIdx; ++i) {
        studio.emplace_back(PlaybackParam { mpe::SOUND_PRESET_PARAM_CODE, Val("Studio"), i });
        pop.emplace_back(PlaybackParam { mpe::SOUND_PRESET_PARAM_CODE, Val("Pop"), i });
    }

    PlaybackParamList orchestral  { { mpe::SOUND_PRESET_PARAM_CODE, Val("Orchestral"), 1 } }; // "apply to all staves" is off

    PlaybackParamMap expectedParams {
        { timestampFromTicks(score, 960), studio },
        { timestampFromTicks(score, 3840), pop },
        { timestampFromTicks(score, 5760), orchestral },
    };

    EXPECT_EQ(params, expectedParams);

    // [THEN] We can get the params for a specific tick & staff
    for (staff_layer_idx_t i = startIdx; i < endIdx; ++i) {
        params = ctx.playbackParamMap(score, 0, i);
        EXPECT_TRUE(params.empty());

        params = ctx.playbackParamMap(score, 960, i);
        ASSERT_EQ(params.size(), 1);
        EXPECT_EQ(params.begin()->second, PlaybackParamList { studio.at(i) });

        params = ctx.playbackParamMap(score, 4500, i);
        ASSERT_EQ(params.size(), 1);
        EXPECT_EQ(params.begin()->second, PlaybackParamList { pop.at(i) });

        params = ctx.playbackParamMap(score, 8000, i);

        if (i == 1) {
            ASSERT_EQ(params.size(), 1);
            EXPECT_EQ(params.begin()->second, orchestral);
        } else {
            EXPECT_TRUE(params.empty());
        }
    }

    delete score;
}

/**
 * @brief Engraving_PlaybackContextTests_ParseSoundFlags_ArcoAndOrdCancelPlayingTechniques
 *  @details Checks whether Arco & "Ord." correctly cancel playing techniques. See:
 *  https://github.com/musescore/MuseScore/issues/22403
 */
TEST_F(Engraving_PlaybackContextTests, ParseSoundFlags_ArcoAndOrdCancelPlayingTechniques)
{
    // [GIVEN] Score with sound flags & playing techniques
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "sound_flags/sound_flags_arco.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing sound flags & playing techniques
    PlaybackContext ctx;

    // [WHEN] Parse the sound flags & playing techniques
    const Part* part = parts.front();
    ctx.update(part->id(), score);

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
    PlaybackParamList ordinary { { mpe::PLAY_TECHNIQUE_PARAM_CODE, Val(mpe::ORDINARY_PLAYING_TECHNIQUE_CODE), 0 } };
    PlaybackParamList bartok { { mpe::PLAY_TECHNIQUE_PARAM_CODE, Val("bartok"), 0 } };

    PlaybackParamMap expectedParams {
        { timestampFromTicks(score, 1920), ordinary }, // 2nd measure (cancels Pizz.)
        { timestampFromTicks(score, 3840), bartok }, // 3rd measure
        { timestampFromTicks(score, 5760), ordinary }, // 4th (canceled by Arco)
    };

    PlaybackParamMap params = ctx.playbackParamMap(score);
    EXPECT_EQ(params, expectedParams);

    delete score;
}
