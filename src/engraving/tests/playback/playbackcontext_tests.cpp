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
#include "engraving/dom/staff.h"
#include "engraving/dom/repeatlist.h"

#include "playback/playbackcontext.h"
#include "playback/utils/arrangementutils.h"

#include "types/typesconv.h"

using namespace mu::engraving;
using namespace muse::mpe;
using namespace muse;

static const muse::String PLAYBACK_CONTEXT_TEST_FILES_DIR("playback/playbackcontext_data/");

static constexpr int HAIRPIN_STEPS = 24;
static constexpr int COMPOUND_DYNAMIC_STEPS = 6;
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

    static void addParamToStaff(const PlaybackParam& param, staff_idx_t staffIdx, timestamp_t timestamp, PlaybackParamLayers& dest)
    {
        for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
            layer_idx_t layerIdx = static_cast<layer_idx_t>(staff2track(staffIdx, voiceIdx));
            dest[layerIdx][timestamp].push_back(param);
        }
    }
};

//! Checks that hairpins outside/inside repeats don't overlap. See:
//! https://github.com/musescore/MuseScore/issues/16167
//! https://github.com/musescore/MuseScore/issues/23940
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

    // [GIVEN] 1st hairpin (before the repeat): p -> f
    constexpr mpe::dynamic_level_t p = dynamicLevelFromType(mpe::DynamicType::p);
    constexpr mpe::dynamic_level_t f = dynamicLevelFromType(mpe::DynamicType::f);

    const std::map<int, int> p_to_f_curve = TConv::easingValueCurve(1920, HAIRPIN_STEPS, static_cast<int>(f - p),
                                                                    ChangeMethod::NORMAL);

    for (const auto& pair : p_to_f_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first);
        ASSERT_FALSE(muse::contains(expectedDynamics, time));
        expectedDynamics.emplace(time, p + static_cast<dynamic_level_t>(pair.second));
    }

    // [GIVEN] 2nd hairpin (inside the repeat): f -> fff
    constexpr mpe::dynamic_level_t fff = dynamicLevelFromType(mpe::DynamicType::fff);
    constexpr int f_to_fff_startTick = 1920;

    const std::map<int, int> f_to_fff_curve = TConv::easingValueCurve(1440, HAIRPIN_STEPS, static_cast<int>(fff - f),
                                                                      ChangeMethod::NORMAL);

    for (const mu::engraving::RepeatSegment* repeatSegment : repeats) {
        int tickPositionOffset = repeatSegment->utick - repeatSegment->tick;

        for (const auto& pair : f_to_fff_curve) {
            int tick = f_to_fff_startTick + pair.first + tickPositionOffset;
            mpe::timestamp_t time = timestampFromTicks(score, tick);

            if (tick != f_to_fff_startTick) { // f already added by the previous hairpin
                ASSERT_FALSE(muse::contains(expectedDynamics, time));
            }

            expectedDynamics.emplace(time, f + static_cast<dynamic_level_t>(pair.second));
        }
    }

    // [GIVEN] 3rd hairpin (right after the repeat): ppp -> p
    constexpr mpe::dynamic_level_t ppp = dynamicLevelFromType(mpe::DynamicType::ppp);
    constexpr int ppp_to_p_startTick = 3840 + 1920; // real tick + repeat tick

    const std::map<int, int> ppp_to_p_curve = TConv::easingValueCurve(1440, HAIRPIN_STEPS, static_cast<int>(p - ppp),
                                                                      ChangeMethod::NORMAL);

    for (const auto& pair : ppp_to_p_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, ppp_to_p_startTick + pair.first);
        ASSERT_FALSE(muse::contains(expectedDynamics, time));
        expectedDynamics.emplace(time, ppp + static_cast<dynamic_level_t>(pair.second));
    }

    ASSERT_FALSE(expectedDynamics.empty());

    // [WHEN] Get the actual dynamics
    DynamicLevelLayers layers = ctx.dynamicLevelLayers(score);

    // [THEN] The dynamics match the expectation
    EXPECT_FALSE(layers.empty());

    for (const auto& layer : layers) {
        const DynamicLevelMap& actualDynamics = layer.second;
        EXPECT_EQ(actualDynamics, expectedDynamics);
    }

    delete score;
}

TEST_F(Engraving_PlaybackContextTests, Dynamics_MeasureRepeats)
{
    // [GIVEN] Score with 5 measures and 2 instruments. There is a measure repeat on the last 2 measures of the 1st instrument
    // (so the previous 2 measures will be repeated)
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "dynamics/dynamics_and_measure_repeats.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_EQ(parts.size(), 2);

    // [GIVEN] Context for parsing dynamics
    PlaybackContext ctx;

    // [WHEN] Parse dynamics for the 1st instrument (with measure repeats)
    ctx.update(parts.at(0)->id(), score);

    DynamicLevelMap expectedDynamics {
        // 1st measure
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

    // [WHEN] Get the actual dynamics
    DynamicLevelLayers layers = ctx.dynamicLevelLayers(score);

    // [THEN] The dynamics match the expectation
    EXPECT_FALSE(layers.empty());
    for (const auto& layer : layers) {
        const DynamicLevelMap& actualDynamics = layer.second;
        EXPECT_EQ(actualDynamics, expectedDynamics);
    }

    // [WHEN] Parse dynamics for the 2nd instrument (without measure repeats)
    ctx.clear();
    ctx.update(parts.at(1)->id(), score);

    // [WHEN] Get the actual dynamics
    layers = ctx.dynamicLevelLayers(score);

    // [THEN] Measure repeat on the 1st instrument doesn't affect other instruments
    expectedDynamics = {
        // 1st measure
        { timestampFromTicks(score, 0), dynamicLevelFromType(mpe::DynamicType::f) },

        // 3rd measure
        { timestampFromTicks(score, 3840), dynamicLevelFromType(mpe::DynamicType::ff) },
    };

    // [THEN] The dynamics match the expectation
    EXPECT_FALSE(layers.empty());
    for (const auto& layer : layers) {
        const DynamicLevelMap& actualDynamics = layer.second;
        EXPECT_EQ(actualDynamics, expectedDynamics);
    }
}

TEST_F(Engraving_PlaybackContextTests, Dynamics_OnDifferentVoices)
{
    // [GIVEN]
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "dynamics/dynamics_on_voices.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing dynamics
    PlaybackContext ctx;

    // [WHEN] Parse dynamics
    const Part* part = parts.front();
    ctx.update(part->id(), score);

    // [WHEN] Get the actual dynamics
    DynamicLevelLayers actualLayers = ctx.dynamicLevelLayers(score);

    // [THEN] The dynamics match the expectation
    DynamicLevelLayers expectedLayers;

    auto addDynToAllStavesAndVoices = [score, part, &expectedLayers](mpe::DynamicType dyn, int tick) {
        for (track_idx_t trackIdx = part->startTrack(); trackIdx < part->endTrack(); ++trackIdx) {
            expectedLayers[static_cast<layer_idx_t>(trackIdx)][timestampFromTicks(score, tick)] = dynamicLevelFromType(dyn);
        }
    };

    auto addDynToStaffAndVoice = [score, &expectedLayers](mpe::DynamicType dyn, staff_idx_t staffIdx, voice_idx_t voiceIdx, int tick) {
        layer_idx_t layerIdx = static_cast<layer_idx_t>(staff2track(staffIdx, voiceIdx));
        expectedLayers[layerIdx][timestampFromTicks(score, tick)] = dynamicLevelFromType(dyn);
    };

    // 1st measure
    addDynToAllStavesAndVoices(mpe::DynamicType::ppp, 0); // "Apply to all voices"

    // 2nd measure
    addDynToStaffAndVoice(mpe::DynamicType::pp, 0, 1, 1920); // 1st staff & 2nd voice ("Apply to 2nd voice")

    // 3rd measure
    addDynToStaffAndVoice(mpe::DynamicType::mf, 0, 0, 3840); // 1st staff & 1st voice ("Apply to 1st voice")

    for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
        // "Apply to all voices" is on but there is the mf on the 1st staff & 1st at the same tick
        if (voiceIdx != 0) {
            addDynToStaffAndVoice(mpe::DynamicType::p, 0, voiceIdx, 3840);
        }

        addDynToStaffAndVoice(mpe::DynamicType::p, 1, voiceIdx, 3840);
    }

    // 4th measure
    for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
        addDynToStaffAndVoice(mpe::DynamicType::f, 0, voiceIdx, 5760); // 1st staff only ("All voices on this staff")
    }

    // 5th measure
    addDynToAllStavesAndVoices(mpe::DynamicType::ff, 7680); // "Apply to all voices"

    EXPECT_EQ(actualLayers, expectedLayers);

    delete score;
}

//! See: https://github.com/musescore/MuseScore/issues/23596
TEST_F(Engraving_PlaybackContextTests, Dynamics_Overlap)
{
    // [GIVEN]
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "dynamics/dynamics_overlap.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing dynamics
    PlaybackContext ctx;

    // [WHEN] Parse dynamics
    const Part* part = parts.front();
    ctx.update(part->id(), score);

    // [WHEN] Get the actual dynamics
    DynamicLevelLayers actualLayers = ctx.dynamicLevelLayers(score);

    // [THEN] The dynamics match the expectation
    DynamicLevelMap expectedDynamics;

    // 1st measure: Diminuendo ff -> pp (ends at the 1920 tick)
    constexpr mpe::dynamic_level_t ff = dynamicLevelFromType(mpe::DynamicType::ff);
    constexpr mpe::dynamic_level_t pp = dynamicLevelFromType(mpe::DynamicType::pp);

    const std::map<int, int> ff_to_pp_curve = TConv::easingValueCurve(1920 - Fraction::eps().ticks(),
                                                                      HAIRPIN_STEPS, static_cast<int>(pp - ff), ChangeMethod::NORMAL);
    for (const auto& pair : ff_to_pp_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first);
        expectedDynamics.emplace(time, ff + static_cast<dynamic_level_t>(pair.second));
    }

    // 2nd measure: ff (starts at the 1920 tick)
    expectedDynamics.emplace(timestampFromTicks(score, 1920), ff);

    EXPECT_FALSE(actualLayers.empty());
    for (const auto& layer : actualLayers) {
        const DynamicLevelMap& actualDynamics = layer.second;
        EXPECT_EQ(actualDynamics, expectedDynamics);
    }
}

TEST_F(Engraving_PlaybackContextTests, Dynamics_Niente)
{
    // [GIVEN]
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "dynamics/dynamics_niente.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing dynamics
    PlaybackContext ctx;

    // [WHEN] Parse dynamics
    const Part* part = parts.front();
    ctx.update(part->id(), score);

    // [WHEN] Get the actual dynamics
    DynamicLevelLayers actualLayers = ctx.dynamicLevelLayers(score);

    // [THEN] The dynamics match the expectation
    DynamicLevelMap expectedDynamics;

    constexpr mpe::dynamic_level_t f = dynamicLevelFromType(mpe::DynamicType::f);
    constexpr mpe::dynamic_level_t n = dynamicLevelFromType(mpe::DynamicType::ppppppppp);

    const std::map<int, int> f_to_n_curve = TConv::easingValueCurve(1920, HAIRPIN_STEPS, static_cast<int>(n - f), ChangeMethod::NORMAL);
    const std::map<int, int> n_to_f_curve = TConv::easingValueCurve(1920, HAIRPIN_STEPS, static_cast<int>(f - n), ChangeMethod::NORMAL);

    // 1st measure: Decresc. al niente with 'n' dynamic
    for (const auto& pair : f_to_n_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first);
        expectedDynamics.emplace(time, f + static_cast<dynamic_level_t>(pair.second));
    }

    // 3rd measure: same, now with niente circle
    for (const auto& pair : f_to_n_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, 3840 + pair.first);
        expectedDynamics.emplace(time, f + static_cast<dynamic_level_t>(pair.second));
    }

    // 5th measure: Cresc. dal niente with 'n' dynamic
    for (const auto& pair : n_to_f_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, 7680 + pair.first);
        expectedDynamics.emplace(time, n + static_cast<dynamic_level_t>(pair.second));
    }

    // 7th measure: same, now with niente circle
    for (const auto& pair : n_to_f_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, 11520 + pair.first);
        expectedDynamics.emplace(time, n + static_cast<dynamic_level_t>(pair.second));
    }

    EXPECT_FALSE(actualLayers.empty());
    for (const auto& layer : actualLayers) {
        const DynamicLevelMap& actualDynamics = layer.second;
        EXPECT_EQ(actualDynamics, expectedDynamics);
    }
}

TEST_F(Engraving_PlaybackContextTests, Dynamics_HairpinWithCompound)
{
    // [GIVEN]
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "dynamics/dynamics_compound.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing dynamics
    PlaybackContext ctx;

    // [WHEN] Parse dynamics
    const Part* part = parts.front();
    ctx.update(part->id(), score);

    // [WHEN] Get the actual dynamics
    DynamicLevelLayers actualLayers = ctx.dynamicLevelLayers(score);

    // [THEN] The dynamics match the expectation
    DynamicLevelMap expectedDynamics;

    constexpr mpe::dynamic_level_t f = dynamicLevelFromType(mpe::DynamicType::f);
    constexpr mpe::dynamic_level_t p = dynamicLevelFromType(mpe::DynamicType::p);
    constexpr mpe::dynamic_level_t pp = dynamicLevelFromType(mpe::DynamicType::pp);

    constexpr int measureTicks = 1920;
    constexpr int compoundDynamicTicks = 384;

    // 1st and 2nd measures: fp -> f
    const std::map<int, int> fp_curve = TConv::easingValueCurve(compoundDynamicTicks, COMPOUND_DYNAMIC_STEPS, static_cast<int>(p - f),
                                                                ChangeMethod::NORMAL);
    const std::map<int,
                   int> p_to_f_curve = TConv::easingValueCurve(measureTicks - compoundDynamicTicks, HAIRPIN_STEPS, static_cast<int>(f - p),
                                                               ChangeMethod::NORMAL);

    for (const auto& pair : fp_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first + 0 * measureTicks);
        expectedDynamics.emplace(time, f + static_cast<dynamic_level_t>(pair.second));
    }

    for (const auto& pair : p_to_f_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first + 0 * measureTicks + compoundDynamicTicks);
        expectedDynamics.emplace(time, p + static_cast<dynamic_level_t>(pair.second));
    }

    for (const auto& pair : fp_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first + 1 * measureTicks);
        expectedDynamics.emplace(time, f + static_cast<dynamic_level_t>(pair.second));
    }

    for (const auto& pair : p_to_f_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first + 1 * measureTicks + compoundDynamicTicks);
        expectedDynamics.emplace(time, p + static_cast<dynamic_level_t>(pair.second));
    }

    // 4th and 5th measures: fp -> pp, then jump back to f
    const std::map<int, int> p_to_pp_curve_shorted_by_one = TConv::easingValueCurve(measureTicks - compoundDynamicTicks - 1, HAIRPIN_STEPS,
                                                                                    static_cast<int>(pp - p), ChangeMethod::NORMAL);

    for (const auto& pair : fp_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first + 3 * measureTicks);
        expectedDynamics.emplace(time, f + static_cast<dynamic_level_t>(pair.second));
    }

    for (const auto& pair : p_to_pp_curve_shorted_by_one) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first + 3 * measureTicks + compoundDynamicTicks);
        expectedDynamics.emplace(time, p + static_cast<dynamic_level_t>(pair.second));
    }

    for (const auto& pair : fp_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first + 4 * measureTicks);
        expectedDynamics.emplace(time, f + static_cast<dynamic_level_t>(pair.second));
    }

    for (const auto& pair : p_to_pp_curve_shorted_by_one) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first + 4 * measureTicks + compoundDynamicTicks);
        expectedDynamics.emplace(time, p + static_cast<dynamic_level_t>(pair.second));
    }

    {
        mpe::timestamp_t time = timestampFromTicks(score, 5 * measureTicks);
        expectedDynamics.emplace(time, f);
    }

    // 7th measure: fp -> pp, now don't jump back to f
    const std::map<int,
                   int> p_to_pp_curve = TConv::easingValueCurve(measureTicks - compoundDynamicTicks, HAIRPIN_STEPS,
                                                                static_cast<int>(pp - p),
                                                                ChangeMethod::NORMAL);

    for (const auto& pair : fp_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first + 6 * measureTicks);
        expectedDynamics.emplace(time, f + static_cast<dynamic_level_t>(pair.second));
    }

    for (const auto& pair : p_to_pp_curve) {
        mpe::timestamp_t time = timestampFromTicks(score, pair.first + 6 * measureTicks + compoundDynamicTicks);
        expectedDynamics.emplace(time, p + static_cast<dynamic_level_t>(pair.second));
    }

    EXPECT_FALSE(actualLayers.empty());
    for (const auto& layer : actualLayers) {
        const DynamicLevelMap& actualDynamics = layer.second;
        EXPECT_EQ(actualDynamics, expectedDynamics);
    }
}

TEST_F(Engraving_PlaybackContextTests, PlayTechniques)
{
    // [GIVEN] Score with playing technique annotations
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "play_techniques/play_tech_annotations.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing techniques
    PlaybackContext ctx;

    // [THEN] No technique parsed, returns the "Standard" acticulation
    int maxTick = score->endTick().ticks();

    for (int tick = 0; tick <= maxTick; tick += TICKS_STEP) {
        ArticulationType actualType = ctx.persistentArticulationType(tick);
        EXPECT_EQ(actualType, ArticulationType::Standard);
    }

    // [WHEN] Parse techniques
    ctx.update(parts.front()->id(), score);

    // [THEN] The techniques successfully parsed
    std::map<int /*tick*/, ArticulationType> expectedArticulationTypes {
        { 0, ArticulationType::Pizzicato },
        { 1440, ArticulationType::Open },
        { 1920, ArticulationType::Mute },
        { 3840, ArticulationType::Tremolo64th },
        { 5760, ArticulationType::Detache },
        { 7680, ArticulationType::Martele },
        { 9600, ArticulationType::ColLegno },
        { 11520, ArticulationType::SulPont },
        { 13440, ArticulationType::SulTasto },
//        { 15360, ArticulationType::Vibrato },
//        { 17280, ArticulationType::Legato },
        { 19200, ArticulationType::Distortion },
        { 21120, ArticulationType::Overdrive },
        { 23040, ArticulationType::Harmonic },
        { 24960, ArticulationType::JazzTone },
    };

    auto findExpectedType = [&expectedArticulationTypes](int tick) {
        auto it = muse::findLessOrEqual(expectedArticulationTypes, tick);
        if (it == expectedArticulationTypes.end()) {
            return ArticulationType::Standard;
        }

        return it->second;
    };

    for (int tick = 0; tick <= maxTick; tick += TICKS_STEP) {
        ArticulationType actualType = ctx.persistentArticulationType(tick);
        ArticulationType expectedType = findExpectedType(tick);
        EXPECT_EQ(actualType, expectedType);
    }

    // [WHEN] Clear the context
    ctx.clear();

    // [THEN] No technique parsed, returns the "Standard" acticulation
    for (int tick = 0; tick <= maxTick; tick += TICKS_STEP) {
        ArticulationType actualType = ctx.persistentArticulationType(tick);
        EXPECT_EQ(actualType, ArticulationType::Standard);
    }

    delete score;
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
    PlaybackParamLayers actualParams = ctx.playbackParamLayers(score);

    // [THEN] Expected params
    PlaybackParam sulTasto(PlaybackParam::PlayingTechnique, u"Sul Tasto");
    PlaybackParam bartok(PlaybackParam::PlayingTechnique, u"bartok");
    PlaybackParam pizz(PlaybackParam::PlayingTechnique, u"pizzicato");
    PlaybackParam espressivo(PlaybackParam::PlayingTechnique, u"Espressivo");

    PlaybackParamLayers expectedParams;

    for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        addParamToStaff(sulTasto, staffIdx, timestampFromTicks(score, 1920), expectedParams);
    }

    addParamToStaff(bartok, 0, timestampFromTicks(score, 3840), expectedParams); // "apply to all staves" is OFF (apply to 1st staff)
    addParamToStaff(pizz, 1, timestampFromTicks(score, 3840), expectedParams); // "apply to all staves" is ON (apply to 2nd staff)
    addParamToStaff(espressivo, 1, timestampFromTicks(score, 7680), expectedParams); // "apply to all staves" is OFF (apply to 1st staff)

    EXPECT_EQ(actualParams, expectedParams);

    // [THEN] We can get the params for a specific track & tick
    for (track_idx_t trackIdx = part->startTrack(); trackIdx < part->endTrack(); ++trackIdx) {
        PlaybackParamList params = ctx.playbackParams(trackIdx, 0);
        EXPECT_TRUE(params.empty());

        params = ctx.playbackParams(trackIdx, 2000);
        EXPECT_EQ(params, PlaybackParamList { sulTasto });

        params = ctx.playbackParams(trackIdx, 4500);

        staff_idx_t staffIdx = track2staff(trackIdx);

        if (staffIdx == 1) {
            EXPECT_EQ(params, PlaybackParamList { pizz });
        } else {
            EXPECT_EQ(params, PlaybackParamList { bartok });
        }

        params = ctx.playbackParams(trackIdx, 7680);

        if (staffIdx == 1) {
            EXPECT_EQ(params, PlaybackParamList { espressivo });
        } else {
            EXPECT_EQ(params, PlaybackParamList { bartok });
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
    PlaybackParamList secondMeasureParams { { PlaybackParam::PlayingTechnique, u"Espressivo" } };
    PlaybackParamList thirdMeasureParams { { PlaybackParam::PlayingTechnique, u"bartok" } };

    PlaybackParamMap expectedParams {
        { timestampFromTicks(score, 1920), secondMeasureParams },
        { timestampFromTicks(score, 3840), thirdMeasureParams },
        { timestampFromTicks(score, 5760), secondMeasureParams }, // measure repeat
        { timestampFromTicks(score, 7680), thirdMeasureParams }, // measure repeat
    };

    PlaybackParamLayers layers = ctx.playbackParamLayers(score);
    EXPECT_FALSE(layers.empty());

    for (const auto& layer : layers) {
        const PlaybackParamMap& actualParams = layer.second;
        EXPECT_EQ(actualParams, expectedParams);
    }
}

/**
 * @brief Engraving_PlaybackContextTests_SoundFlags_CancelPlayingTechniques
 *  @details Checks whether Arco & Open & "Ord." correctly cancel playing techniques. See:
 *  https://github.com/musescore/MuseScore/issues/22403
 */
TEST_F(Engraving_PlaybackContextTests, SoundFlags_CancelPlayingTechniques)
{
    // [GIVEN] Score (violin + brass) with sound flags & playing techniques
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "sound_flags/cancel_playing_technique.mscx");

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
    PlaybackParam ordinary(PlaybackParam::PlayingTechnique, mpe::ORDINARY_PLAYING_TECHNIQUE_CODE);
    PlaybackParam bartok(PlaybackParam::PlayingTechnique, u"bartok");

    PlaybackParamMap expectedParams {
        { timestampFromTicks(score, 1920), { ordinary } }, // 2nd measure (cancels Pizz.)
        { timestampFromTicks(score, 3840), { bartok } }, // 3rd measure
        { timestampFromTicks(score, 5760), { ordinary } }, // 4th (canceled by Arco)
    };

    PlaybackParamLayers layers = ctx.playbackParamLayers(score);
    EXPECT_FALSE(layers.empty());

    for (const auto& layer : layers) {
        const PlaybackParamMap& actualParams = layer.second;
        EXPECT_EQ(actualParams, expectedParams);
    }

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
    PlaybackParam mute(PlaybackParam::PlayingTechnique, u"mute");

    expectedParams = {
        { timestampFromTicks(score, 0), { mute } }, // 1st measure
        { timestampFromTicks(score, 1920), { ordinary } }, // 2nd measure (canceled by Open)
    };

    layers = ctx.playbackParamLayers(score);
    EXPECT_FALSE(layers.empty());

    for (const auto& layer : layers) {
        const PlaybackParamMap& actualParams = layer.second;
        EXPECT_EQ(actualParams, expectedParams);
    }

    delete score;
}
