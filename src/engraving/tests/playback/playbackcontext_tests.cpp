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

#include "engraving/playback/playbackcontext.h"
#include "engraving/playback/utils/arrangementutils.h"

#include "engraving/types/typesconv.h"

using namespace mu::engraving;
using namespace muse::mpe;
using namespace muse;

static const muse::String PLAYBACK_CONTEXT_TEST_FILES_DIR("playback/playbackcontext_data/");

static constexpr int HAIRPIN_STEPS = 24;
static constexpr int COMPOUND_DYNAMIC_STEPS = 6;
static constexpr int TICKS_STEP = 480;

class Engraving_PlaybackContextTests : public ::testing::Test
{
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
        ASSERT_FALSE(expectedDynamics.contains(time));
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
                ASSERT_FALSE(expectedDynamics.contains(time));
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
        ASSERT_FALSE(expectedDynamics.contains(time));
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

    // 1st measure: Dim. al niente with 'n' dynamic
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

    // [THEN] No technique parsed, returns the "Natural" type
    int maxTick = score->endTick().ticks();

    for (int tick = 0; tick <= maxTick; tick += TICKS_STEP) {
        std::pair<timestamp_t, PlayingTechniqueType> actualType = ctx.playingTechnique(score, tick);
        EXPECT_EQ(actualType.first, 0);
        EXPECT_EQ(actualType.second, PlayingTechniqueType::Natural);
    }

    // [WHEN] Parse techniques
    ctx.update(parts.front()->id(), score);

    // [THEN] The techniques successfully parsed
    constexpr int ticksPerMeasure = 1920;

    const std::map<int /*tick*/, PlayingTechniqueType> expectedTypes {
        { 0, PlayingTechniqueType::Pizzicato },
        { 1440, PlayingTechniqueType::Open },
        { ticksPerMeasure* 1, PlayingTechniqueType::Mute },
        { ticksPerMeasure* 2, PlayingTechniqueType::Tremolo },
        { ticksPerMeasure* 3, PlayingTechniqueType::Detache },
        { ticksPerMeasure* 4, PlayingTechniqueType::Martele },
        { ticksPerMeasure* 5, PlayingTechniqueType::ColLegno },
        { ticksPerMeasure* 6, PlayingTechniqueType::SulPonticello },
        { ticksPerMeasure* 7, PlayingTechniqueType::SulTasto },
        { ticksPerMeasure* 8, PlayingTechniqueType::Vibrato },
        { ticksPerMeasure* 9, PlayingTechniqueType::Legato },
        { ticksPerMeasure* 10, PlayingTechniqueType::Distortion },
        { ticksPerMeasure* 11, PlayingTechniqueType::Overdrive },
        { ticksPerMeasure* 12, PlayingTechniqueType::Harmonics },
        { ticksPerMeasure* 13, PlayingTechniqueType::JazzTone },
        { ticksPerMeasure* 14, PlayingTechniqueType::HandbellsSwing },
        { ticksPerMeasure* 15, PlayingTechniqueType::HandbellsSwingUp },
        { ticksPerMeasure* 16, PlayingTechniqueType::HandbellsSwingDown },
        { ticksPerMeasure* 17, PlayingTechniqueType::HandbellsEcho1 },
        { ticksPerMeasure* 18, PlayingTechniqueType::HandbellsEcho2 },
        { ticksPerMeasure* 19, PlayingTechniqueType::HandbellsR },
        { ticksPerMeasure* 20, PlayingTechniqueType::HandbellsLV },
        { ticksPerMeasure* 21, PlayingTechniqueType::HandbellsDamp },
    };

    auto findExpectedType = [&expectedTypes, score](int tick) -> std::pair<timestamp_t, PlayingTechniqueType> {
        auto it = muse::findLessOrEqual(expectedTypes, tick);
        if (it == expectedTypes.end()) {
            return std::make_pair(0, PlayingTechniqueType::Natural);
        }

        return std::make_pair(timestampFromTicks(score, it->first), it->second);
    };

    for (int tick = 0; tick <= maxTick; tick += TICKS_STEP) {
        std::pair<timestamp_t, PlayingTechniqueType> actualType = ctx.playingTechnique(score, tick);
        std::pair<timestamp_t, PlayingTechniqueType> expectedType = findExpectedType(tick);
        EXPECT_EQ(actualType, expectedType);
    }

    // [WHEN] Find position of Damp
    const timestamp_t actualDampPosition = ctx.findPlayingTechniqueTimestamp(score, PlayingTechniqueType::HandbellsDamp,
                                                                             ticksPerMeasure * 20);

    // [THEN] Position is correct
    EXPECT_EQ(actualDampPosition, timestampFromTicks(score, ticksPerMeasure * 21));

    // [WHEN] Clear the context
    ctx.clear();

    // [THEN] No technique parsed, returns the "Natural" type
    for (int tick = 0; tick <= maxTick; tick += TICKS_STEP) {
        std::pair<timestamp_t, PlayingTechniqueType> actualType = ctx.playingTechnique(score, tick);
        EXPECT_EQ(actualType.first, 0);
        EXPECT_EQ(actualType.second, PlayingTechniqueType::Natural);
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

    // [THEN] The playing technique map matches the expectation
    std::map<int, PlayingTechniqueType> expectedTypes {
        // 1st measure
        { 0, PlayingTechniqueType::Natural },

        // 2nd measure
        { 1920 + repeatOffsetTick, PlayingTechniqueType::Mute }, // 1st quarter note

        // 3rd measure
        { 4320 + repeatOffsetTick, PlayingTechniqueType::Distortion }, // 2nd quarter note

        // copy of 2nd measure
        { 5760 + repeatOffsetTick, PlayingTechniqueType::Mute },

        // copy of 3rd measure
        { 8160 + repeatOffsetTick, PlayingTechniqueType::Distortion },
    };

    for (const auto& pair : expectedTypes) {
        std::pair<timestamp_t, PlayingTechniqueType> actualType = ctx.playingTechnique(score, pair.first);
        EXPECT_EQ(actualType.first, timestampFromTicks(score, pair.first));
        EXPECT_EQ(actualType.second, pair.second);
    }
}

TEST_F(Engraving_PlaybackContextTests, SoundFlags_TextArticulations)
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

    // [WHEN] Get the actual articulations
    std::map<timestamp_t, TextArticulationEventList> actualArticulations = ctx.textArticulations(score);

    // [THEN] Expected articulations
    const layer_idx_t secondStaffLayer = static_cast<layer_idx_t>(staff2track(1));

    TextArticulationEvent sulTasto1Staff, sulTasto2Staff;
    sulTasto1Staff.text = u"Sul Tasto";
    sulTasto1Staff.layerIdx = 0;
    sulTasto2Staff.text = sulTasto1Staff.text;
    sulTasto2Staff.layerIdx = secondStaffLayer;

    TextArticulationEvent bartok;  // "apply to all staves" is OFF (apply to 1st staff)
    bartok.text = u"bartok";
    bartok.layerIdx = 0;

    TextArticulationEvent pizz; // "apply to all staves" is ON (apply to 2nd staff)
    pizz.text = u"pizzicato";
    pizz.layerIdx = secondStaffLayer;

    TextArticulationEvent espressivo; // "apply to all staves" is OFF (apply to 1st staff)
    espressivo.text = u"Espressivo";
    espressivo.layerIdx = secondStaffLayer;

    std::map<timestamp_t, TextArticulationEventList> expectedArticulations {
        { timestampFromTicks(score, 1920), { sulTasto1Staff, sulTasto2Staff } },
        { timestampFromTicks(score, 3840), { bartok, pizz } },
        { timestampFromTicks(score, 7680), { espressivo } },
    };

    EXPECT_EQ(actualArticulations, expectedArticulations);

    // [THEN] We can get articulations for a specific track & tick
    for (track_idx_t trackIdx = part->startTrack(); trackIdx < part->endTrack(); ++trackIdx) {
        TextArticulationEvent event = ctx.textArticulation(trackIdx, 0);
        EXPECT_TRUE(event.text.empty());

        event = ctx.textArticulation(trackIdx, 2000);
        staff_idx_t staffIdx = track2staff(trackIdx);

        if (staffIdx == 0) {
            sulTasto1Staff.flags.setFlag(TextArticulationEvent::StartsAtPlaybackPosition, false);
            sulTasto1Staff.layerIdx = static_cast<layer_idx_t>(trackIdx);
            EXPECT_EQ(event, sulTasto1Staff);
        } else {
            sulTasto2Staff.flags.setFlag(TextArticulationEvent::StartsAtPlaybackPosition, false);
            sulTasto2Staff.layerIdx = static_cast<layer_idx_t>(trackIdx);
            EXPECT_EQ(event, sulTasto2Staff);
        }

        event = ctx.textArticulation(trackIdx, 4500);

        if (staffIdx == 0) {
            bartok.flags.setFlag(TextArticulationEvent::StartsAtPlaybackPosition, false);
            bartok.layerIdx = static_cast<layer_idx_t>(trackIdx);
            EXPECT_EQ(event, bartok);
        } else {
            pizz.flags.setFlag(TextArticulationEvent::StartsAtPlaybackPosition, false);
            pizz.layerIdx = static_cast<layer_idx_t>(trackIdx);
            EXPECT_EQ(event, pizz);
        }

        event = ctx.textArticulation(trackIdx, 7680);

        if (staffIdx == 0) {
            bartok.flags.setFlag(TextArticulationEvent::StartsAtPlaybackPosition, false);
            bartok.layerIdx = static_cast<layer_idx_t>(trackIdx);
            EXPECT_EQ(event, bartok);
        } else {
            espressivo.flags.setFlag(TextArticulationEvent::StartsAtPlaybackPosition, true);
            espressivo.layerIdx = static_cast<layer_idx_t>(trackIdx);
            EXPECT_EQ(event, espressivo);
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

    // [THEN] The actual text articulations match the expectation
    TextArticulationEvent espressivo, bartok;
    espressivo.text = u"Espressivo";
    bartok.text = u"bartok";

    std::map<timestamp_t, TextArticulationEventList> expectedArticulations {
        { timestampFromTicks(score, 1920), { espressivo } }, // 2nd measure
        { timestampFromTicks(score, 3840), { bartok } }, // 3rd measure
        { timestampFromTicks(score, 5760), { espressivo } }, // measure repeat
        { timestampFromTicks(score, 7680), { bartok } }, // measure repeat
    };

    std::map<timestamp_t, TextArticulationEventList> actualArticulations = ctx.textArticulations(score);
    EXPECT_EQ(expectedArticulations, actualArticulations);
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
        std::pair<timestamp_t, PlayingTechniqueType> actualType = ctx.playingTechnique(score, tick);
        EXPECT_EQ(actualType.first, 0);
        EXPECT_EQ(actualType.second, PlayingTechniqueType::Pizzicato);
    }

    // [THEN] "Natural" for all the other measures, as Pizz. was canceled with "Ord." in the 2nd measure
    const timestamp_t expectedNaturalTimestamp = timestampFromTicks(score, 1920);
    int lastTick = score->lastMeasure()->tick().ticks();
    for (int tick = 1920; tick < lastTick; tick += TICKS_STEP) {
        std::pair<timestamp_t, PlayingTechniqueType> actualType = ctx.playingTechnique(score, tick);
        EXPECT_EQ(actualType.first, expectedNaturalTimestamp);
        EXPECT_EQ(actualType.second, PlayingTechniqueType::Natural);
    }

    // [THEN] The actual text articulations match the expectation
    TextArticulationEvent ordinary;
    ordinary.text = mpe::ORDINARY_PLAYING_TECHNIQUE_CODE;
    ordinary.layerIdx = 0;

    TextArticulationEvent bartok;
    bartok.text = u"bartok";
    bartok.layerIdx = 0;

    std::map<timestamp_t, TextArticulationEventList> expectedArticulations {
        { timestampFromTicks(score, 1920), { ordinary } }, // 2nd measure (cancels Pizz.)
        { timestampFromTicks(score, 3840), { bartok } }, // 3rd measure
        { timestampFromTicks(score, 5760), { ordinary } }, // 4th (canceled by Arco)
    };

    std::map<timestamp_t, TextArticulationEventList> actualArticulations = ctx.textArticulations(score);
    EXPECT_EQ(expectedArticulations, actualArticulations);

    // [WHEN] Parse the brass part
    const Part* brassPart = parts.at(1);
    ctx.clear();
    ctx.update(brassPart->id(), score);

    // [THEN] 1st measure: Standard
    for (int tick = 0; tick < 1920; tick += TICKS_STEP) {
        std::pair<timestamp_t, PlayingTechniqueType> actualType = ctx.playingTechnique(score, tick);
        EXPECT_EQ(actualType.first, 0);
        EXPECT_EQ(actualType.second, PlayingTechniqueType::Natural);
    }

    // [THEN] "Open" starting from the 2nd measure
    const timestamp_t expectedOpenTimestamp = timestampFromTicks(score, 1920);
    for (int tick = 1920; tick < lastTick; tick += TICKS_STEP) {
        std::pair<timestamp_t, PlayingTechniqueType> actualType = ctx.playingTechnique(score, tick);
        EXPECT_EQ(actualType.first, expectedOpenTimestamp);
        EXPECT_EQ(actualType.second, PlayingTechniqueType::Open);
    }

    // [THEN] The actual text articulations match the expectation
    const layer_idx_t secondStaffLayer = static_cast<layer_idx_t>(staff2track(1));
    ordinary.layerIdx = secondStaffLayer;

    TextArticulationEvent mute;
    mute.text = u"mute";
    mute.layerIdx = secondStaffLayer;

    expectedArticulations = {
        { timestampFromTicks(score, 0), { mute } }, // 1st measure
        { timestampFromTicks(score, 1920), { ordinary } }, // 2nd measure (canceled by Open)
    };

    actualArticulations = ctx.textArticulations(score);
    EXPECT_EQ(expectedArticulations, actualArticulations);

    delete score;
}

TEST_F(Engraving_PlaybackContextTests, Lyrics_Multiverses)
{
    // Score with two verses
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "lyrics_multiverses.mscx");
    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    // [GIVEN] Context for parsing lyrics
    PlaybackContext ctx;

    // [WHEN] Parse lyrics
    ctx.update(score->parts().front()->id(), score);

    // [THEN] Lyrics have been parsed correctly
    auto makeSyllable = [](const String& text, bool hyphenedToNext = false) {
        SyllableEvent e;
        e.text = text;
        e.flags.setFlag(SyllableEvent::HyphenedToNext, hyphenedToNext);
        return e;
    };

    const std::map<timestamp_t, SyllableEventList> expectedEvents {
        // 1st verse
        { 0, { makeSyllable(u"Wal", true) } },
        { 250000, { makeSyllable(u"king") } },
        { 500000, { makeSyllable(u"a", true) } },
        { 750000, { makeSyllable(u"round") } },
        { 1000000, { makeSyllable(u"in") } },
        { 1250000, { makeSyllable(u"the") } },
        { 1500000, { makeSyllable(u"park") } },

        // 2nd verse
        { 2000000, { makeSyllable(u"Should") } },
        { 2500000, { makeSyllable(u"feel") } },
        { 3000000, { makeSyllable(u"like") } },
        { 3500000, { makeSyllable(u"work") } },
    };

    const std::map<timestamp_t, SyllableEventList> actualEvents = ctx.syllables(score);
    EXPECT_EQ(actualEvents.size(), expectedEvents.size());

    for (const auto& pair : actualEvents) {
        auto expectedIt = expectedEvents.find(pair.first);
        ASSERT_TRUE(expectedIt != expectedEvents.end());
        ASSERT_EQ(expectedIt->second.size(), pair.second.size());

        for (size_t i = 0; i < expectedIt->second.size(); ++i) {
            const SyllableEvent& expectedEvent = expectedIt->second.at(i);
            const SyllableEvent& actualEvent = pair.second.at(i);

            EXPECT_EQ(actualEvent.text, expectedEvent.text);
            EXPECT_EQ(actualEvent.layerIdx, expectedEvent.layerIdx);
            EXPECT_EQ(actualEvent.flags, expectedEvent.flags);
        }
    }

    delete score;
}
