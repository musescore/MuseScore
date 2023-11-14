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
#include "engraving/dom/hairpin.h"

#include "playback/playbackcontext.h"
#include "playback/utils/arrangementutils.h"
#include "playback/utils/expressionutils.h"

#include "types/typesconv.h"

using namespace mu::engraving;
using namespace mu::mpe;

static const mu::String PLAYBACK_CONTEXT_TEST_FILES_DIR("playback/playbackcontext_data/");

static constexpr int TICK_STEP = Constants::DIVISION;

class Engraving_PlaybackContextTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        //! NOTE: allows to read test files using their version readers
        //! instead of using 302 (see mscloader.cpp, makeReader)
        m_useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;
    }

    void TearDown() override
    {
        MScore::useRead302InTestMode = m_useRead302;
    }

    using DynamicsCurve = std::map<int, int>;

    struct DynamicsTransitionInfo {
        dynamic_level_t levelFrom = 0;
        dynamic_level_t levelTo = 0;
        DynamicsCurve dynamicsCurve;
    };

    DynamicsTransitionInfo dynamicsTransitionInfo(mu::engraving::DynamicType type, int transitionDurationTicks) const
    {
        const DynamicTransition& transition = dynamicTransitionFromType(type);

        DynamicsTransitionInfo result;
        result.levelFrom = dynamicLevelFromType(transition.from);
        result.levelTo = dynamicLevelFromType(transition.to);

        dynamic_level_t range = result.levelTo - result.levelFrom;

        result.dynamicsCurve = TConv::easingValueCurve(transitionDurationTicks,
                                                       6 /*stepsCount*/,
                                                       static_cast<int>(range),
                                                       ChangeMethod::NORMAL);
        return result;
    }

    DynamicsCurve hairpinCurve(const Hairpin* hairpin, dynamic_level_t nominalDynamicFrom, dynamic_level_t nominalDynamicTo) const
    {
        mu::engraving::DynamicType dynamicTypeFrom = hairpin->dynamicTypeFrom();
        mu::engraving::DynamicType dynamicTypeTo = hairpin->dynamicTypeTo();

        int tickFrom = hairpin->tick().ticks();
        int tickTo = tickFrom + std::abs(hairpin->ticks().ticks());
        int durationTicks = tickTo - tickFrom;

        dynamic_level_t overallDynamicRange = dynamicLevelRangeByTypes(dynamicTypeFrom,
                                                                       dynamicTypeTo,
                                                                       nominalDynamicFrom,
                                                                       nominalDynamicTo,
                                                                       hairpin->isCrescendo());
        return TConv::easingValueCurve(durationTicks,
                                       24 /*stepsCount*/,
                                       static_cast<int>(overallDynamicRange),
                                       hairpin->veloChangeMethod());
    }

private:
    bool m_useRead302 = false;
};

TEST_F(Engraving_PlaybackContextTests, ParseDynamics)
{
    // [GIVEN] Score with dynamics added to different voices
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "dynamics.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing dynamics
    PlaybackContext ctx;

    // [WHEN] Parse dynamics
    ctx.update(parts.front()->id(), score);

    constexpr int CUSTOM_DYNAMICS_START_TICK = 1920;
    constexpr int VOICE_3_DYNAMIC_OVERRIDE_TICK = 3840;
    constexpr int SINGLE_NOTE_DYNAMICS_START_TICK = 5760;
    constexpr int SINGLE_NOTE_DYNAMICS_END_TICK = 7680;
    constexpr int APPLY_TO_ALL_VOICES_TICK = 9600;

    const mu::mpe::timestamp_t CUSTOM_DYNAMICS_START_TIME = timestampFromTicks(score, CUSTOM_DYNAMICS_START_TICK);
    const mu::mpe::timestamp_t VOICE_3_DYNAMIC_OVERRIDE_TIME = timestampFromTicks(score, VOICE_3_DYNAMIC_OVERRIDE_TICK);
    const mu::mpe::timestamp_t SINGLE_NOTE_DYNAMICS_START_TIME = timestampFromTicks(score, SINGLE_NOTE_DYNAMICS_START_TICK);
    const mu::mpe::timestamp_t SINGLE_NOTE_DYNAMICS_END_TIME = timestampFromTicks(score, SINGLE_NOTE_DYNAMICS_END_TICK);
    const mu::mpe::timestamp_t APPLY_TO_ALL_VOICES_TIME = timestampFromTicks(score, APPLY_TO_ALL_VOICES_TICK);

    // [GIVEN] Dynamics (by voices) in the given score
    DynamicLevelMap expectedDynamicsVoice1 {
        { 0, dynamicLevelFromType(mu::mpe::DynamicType::Natural) },

        // The user added a different dynamic for each voice
        { CUSTOM_DYNAMICS_START_TIME, dynamicLevelFromType(mu::mpe::DynamicType::f) },

        // Single note dynamics
        { SINGLE_NOTE_DYNAMICS_START_TIME, dynamicLevelFromType(mu::mpe::DynamicType::f) }, // sf
        { SINGLE_NOTE_DYNAMICS_END_TIME, dynamicLevelFromType(mu::mpe::DynamicType::f) },

        // The user added a ppp to Voice 4, and turned on the "Apply to all voices" setting
        { APPLY_TO_ALL_VOICES_TIME, dynamicLevelFromType(mu::mpe::DynamicType::pppp) },
    };

    DynamicLevelMap expectedDynamicsVoice2 {
        { 0, dynamicLevelFromType(mu::mpe::DynamicType::Natural) },

        // The user added a different dynamic for each voice
        { CUSTOM_DYNAMICS_START_TIME, dynamicLevelFromType(mu::mpe::DynamicType::ff) },

        // Single note dynamics
        { SINGLE_NOTE_DYNAMICS_START_TIME, dynamicLevelFromType(mu::mpe::DynamicType::f) }, // sfz
        { SINGLE_NOTE_DYNAMICS_END_TIME, dynamicLevelFromType(mu::mpe::DynamicType::ff) },

        // The user added a ppp to Voice 4, and turned on the "Apply to all voices" setting
        { APPLY_TO_ALL_VOICES_TIME, dynamicLevelFromType(mu::mpe::DynamicType::pppp) },
    };

    DynamicLevelMap expectedDynamicsVoice3 {
        { 0, dynamicLevelFromType(mu::mpe::DynamicType::Natural) },

        // The user added a different dynamic for each voice
        { CUSTOM_DYNAMICS_START_TIME, dynamicLevelFromType(mu::mpe::DynamicType::fff) },

        // Dynamic for Voice 3 is overriden
        { VOICE_3_DYNAMIC_OVERRIDE_TIME, dynamicLevelFromType(mu::mpe::DynamicType::p) },

        // Single note dynamics
        { SINGLE_NOTE_DYNAMICS_START_TIME, dynamicLevelFromType(mu::mpe::DynamicType::ff) }, // sffz
        { SINGLE_NOTE_DYNAMICS_END_TIME, dynamicLevelFromType(mu::mpe::DynamicType::p) }, // overriden previously

        // The user added a ppp to Voice 4, and turned on the "Apply to all voices" setting
        { APPLY_TO_ALL_VOICES_TIME, dynamicLevelFromType(mu::mpe::DynamicType::pppp) },
    };

    DynamicLevelMap expectedDynamicsVoice4 {
        { 0, dynamicLevelFromType(mu::mpe::DynamicType::Natural) },

        // The user added a different dynamic for each voice
        { CUSTOM_DYNAMICS_START_TIME, dynamicLevelFromType(mu::mpe::DynamicType::ffff) },

        // Single note dynamics
        { SINGLE_NOTE_DYNAMICS_START_TIME, dynamicLevelFromType(mu::mpe::DynamicType::f) }, // rfz
        { SINGLE_NOTE_DYNAMICS_END_TIME, dynamicLevelFromType(mu::mpe::DynamicType::ffff) },

        // The user added a ppp to Voice 4, and turned on the "Apply to all voices" setting
        { APPLY_TO_ALL_VOICES_TIME, dynamicLevelFromType(mu::mpe::DynamicType::pppp) },
    };

    DynamicLevelLayers expectedDynamics {
        { 0, expectedDynamicsVoice1 },
        { 1, expectedDynamicsVoice2 },
        { 2, expectedDynamicsVoice3 },
        { 3, expectedDynamicsVoice4 },
    };

    // [WHEN] Request the dynamics map
    DynamicLevelLayers actualDynamics = ctx.dynamicLevelLayers(score);

    // [THEN] The dynamics map matches the expectation
    EXPECT_EQ(actualDynamics, expectedDynamics);

    // [THEN] Use the Natural dynamic, since no dynamics are specified for this range
    for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
        for (int tick = 0; tick < CUSTOM_DYNAMICS_START_TICK; tick += TICK_STEP) {
            dynamic_level_t actualLevel = ctx.appliableDynamicLevel(voiceIdx, tick);
            EXPECT_EQ(actualLevel, dynamicLevelFromType(mu::mpe::DynamicType::Natural));
        }
    }

    ASSERT_TRUE(SINGLE_NOTE_DYNAMICS_START_TICK > CUSTOM_DYNAMICS_START_TICK
                && SINGLE_NOTE_DYNAMICS_END_TICK < APPLY_TO_ALL_VOICES_TICK);

    for (int tick = CUSTOM_DYNAMICS_START_TICK; tick < APPLY_TO_ALL_VOICES_TICK; tick += TICK_STEP) {
        // [THEN] Single note dynamics are only applied to a specific segment
        if (tick >= SINGLE_NOTE_DYNAMICS_START_TICK && tick < SINGLE_NOTE_DYNAMICS_END_TICK) {
            EXPECT_EQ(ctx.appliableDynamicLevel(0, tick), dynamicLevelFromType(mu::mpe::DynamicType::f)); // sf
            EXPECT_EQ(ctx.appliableDynamicLevel(1, tick), dynamicLevelFromType(mu::mpe::DynamicType::f)); // sfz
            EXPECT_EQ(ctx.appliableDynamicLevel(2, tick), dynamicLevelFromType(mu::mpe::DynamicType::ff)); // sffz
            EXPECT_EQ(ctx.appliableDynamicLevel(3, tick), dynamicLevelFromType(mu::mpe::DynamicType::f)); // rfz
            continue;
        }

        // [THEN] The dynamics from the previous measure still affect the next measure
        EXPECT_EQ(ctx.appliableDynamicLevel(0, tick), dynamicLevelFromType(mu::mpe::DynamicType::f));
        EXPECT_EQ(ctx.appliableDynamicLevel(1, tick), dynamicLevelFromType(mu::mpe::DynamicType::ff));

        // [THEN] The exception is Voice 3, for which the dynamic is overridden
        if (tick >= VOICE_3_DYNAMIC_OVERRIDE_TICK) {
            EXPECT_EQ(ctx.appliableDynamicLevel(2, tick), dynamicLevelFromType(mu::mpe::DynamicType::p));
        } else {
            EXPECT_EQ(ctx.appliableDynamicLevel(2, tick), dynamicLevelFromType(mu::mpe::DynamicType::fff));
        }

        EXPECT_EQ(ctx.appliableDynamicLevel(3, tick), dynamicLevelFromType(mu::mpe::DynamicType::ffff));
    }

    // [THEN] The "apply to all voices" setting is on for the dynamic added to Voice 4
    int maxTick = score->endTick().ticks();

    for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
        for (int tick = APPLY_TO_ALL_VOICES_TICK; tick <= maxTick; tick += TICK_STEP) {
            dynamic_level_t actualLevel = ctx.appliableDynamicLevel(voiceIdx, tick);
            EXPECT_EQ(actualLevel, dynamicLevelFromType(mu::mpe::DynamicType::pppp));
        }
    }

    // [WHEN] Clear the context
    ctx.clear();

    // [THEN] Now use Natural for all voices
    expectedDynamics.clear();

    for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
        DynamicLevelMap map;
        map.emplace(0, dynamicLevelFromType(mu::mpe::DynamicType::Natural));
        expectedDynamics.emplace(voiceIdx, std::move(map));

        for (int tick = 0; tick <= maxTick; tick += TICK_STEP) {
            dynamic_level_t actualLevel = ctx.appliableDynamicLevel(voiceIdx, tick);
            EXPECT_EQ(actualLevel, dynamicLevelFromType(mu::mpe::DynamicType::Natural));
        }
    }

    actualDynamics = ctx.dynamicLevelLayers(score);
    EXPECT_EQ(actualDynamics, expectedDynamics);

    delete score;
}

TEST_F(Engraving_PlaybackContextTests, ParseDynamicTransitions)
{
    // [GIVEN] Score with dynamics added to different voices
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "dynamic_transitions.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing dynamics
    PlaybackContext ctx;

    // [WHEN] Parse dynamics
    ctx.update(parts.front()->id(), score);

    constexpr int APPLY_TO_ALL_VOICES_TICK = 3840;
    constexpr int DYNAMIC_TRANSITION_DURATION_TICKS = 384;

    // [GIVEN] The user added dynamics to some voices
    std::vector<mu::engraving::DynamicType> addedDynamics {
        mu::engraving::DynamicType::FP, // Voice 1
        mu::engraving::DynamicType::PF, // Voice 2
        mu::engraving::DynamicType::SFP, // Voice 3
    };

    for (size_t voiceIdx = 0; voiceIdx < addedDynamics.size(); ++voiceIdx) {
        DynamicsTransitionInfo transition = dynamicsTransitionInfo(addedDynamics[voiceIdx],
                                                                   DYNAMIC_TRANSITION_DURATION_TICKS);

        // [THEN] The dynamics are changing according to the calculated curve
        for (const auto& pair: transition.dynamicsCurve) {
            dynamic_level_t actualLevel = ctx.appliableDynamicLevel(voiceIdx, pair.first);
            dynamic_level_t expectedLevel = transition.levelFrom + pair.second;
            EXPECT_EQ(actualLevel, expectedLevel);
        }

        // [THEN] The last dynamic affects the score until we encounter the dynamic with "apply to all voices" enabled
        auto lastDynamicIt = transition.dynamicsCurve.rbegin();
        for (int tick = lastDynamicIt->first; tick < APPLY_TO_ALL_VOICES_TICK; tick += TICK_STEP) {
            dynamic_level_t actualLevel = ctx.appliableDynamicLevel(voiceIdx, tick);
            dynamic_level_t expectedLevel = transition.levelFrom + lastDynamicIt->second;
            EXPECT_EQ(actualLevel, expectedLevel);
        }
    }

    // [THEN] No dynamic specified
    for (int tick = 0; tick < APPLY_TO_ALL_VOICES_TICK; tick += TICK_STEP) {
        EXPECT_EQ(ctx.appliableDynamicLevel(3, tick), dynamicLevelFromType(mu::mpe::DynamicType::Natural));
    }

    // [THEN] The "apply to all voices" setting is on for the dynamic added to Voice 4
    DynamicsTransitionInfo dynamicsAppliedToAllVoices = dynamicsTransitionInfo(mu::engraving::DynamicType::SFPP,
                                                                               DYNAMIC_TRANSITION_DURATION_TICKS);

    ASSERT_FALSE(dynamicsAppliedToAllVoices.dynamicsCurve.empty());

    // [THEN] The dynamics are changing according to the calculated curve
    for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
        for (const auto& pair: dynamicsAppliedToAllVoices.dynamicsCurve) {
            dynamic_level_t actualLevel = ctx.appliableDynamicLevel(voiceIdx, APPLY_TO_ALL_VOICES_TICK + pair.first);
            dynamic_level_t expectedLevel = dynamicsAppliedToAllVoices.levelFrom + pair.second;
            EXPECT_EQ(actualLevel, expectedLevel);
        }
    }

    // [THEN] The last dynamic on the curve affects the score until its end
    auto lastDynamicIt = dynamicsAppliedToAllVoices.dynamicsCurve.rbegin();
    int maxTick = score->endTick().ticks();

    for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
        for (int tick = APPLY_TO_ALL_VOICES_TICK + lastDynamicIt->first; tick < maxTick; tick += TICK_STEP) {
            dynamic_level_t actualLevel = ctx.appliableDynamicLevel(voiceIdx, tick);
            dynamic_level_t expectedLevel = dynamicsAppliedToAllVoices.levelFrom + lastDynamicIt->second;
            EXPECT_EQ(actualLevel, expectedLevel);
        }
    }

    delete score;
}

TEST_F(Engraving_PlaybackContextTests, ParseHairpins)
{
    // [GIVEN] Score with dynamics/hairpins added to different voices
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "hairpins.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Added hairpins
    int lastTick = score->endTick().ticks();
    const SpannerMap& spannerMap = score->spannerMap();
    const SpannerMap::IntervalList& intervals = spannerMap.findOverlapping(0, lastTick);

    std::vector<Hairpin*> hairpins;
    bool applyToAllVoicesFound = false;

    constexpr int HAIRPINS_START_TICK = 1920;

    for (const auto& interval : intervals) {
        Spanner* spanner = interval.value;
        if (spanner && spanner->isHairpin()) {
            Hairpin* hairpin = toHairpin(spanner);
            hairpins.push_back(hairpin);

            if (hairpin->applyToAllVoices()) {
                applyToAllVoicesFound = true;
            }
        }
    }

    ASSERT_EQ(hairpins.size(), 3);
    ASSERT_TRUE(applyToAllVoicesFound);

    // [GIVEN] Context for parsing dynamics
    PlaybackContext ctx;

    // [WHEN] Parse dynamics
    ctx.update(parts.front()->id(), score);

    for (const Hairpin* hairpin : hairpins) {
        voice_idx_t voiceIdx = hairpin->voice();
        int tickFrom = hairpin->tick().ticks();

        dynamic_level_t nominalDynamicFrom = dynamicLevelFromType(hairpin->dynamicTypeFrom());
        dynamic_level_t nominalDynamicTo = dynamicLevelFromType(hairpin->dynamicTypeTo());

        if (tickFrom == HAIRPINS_START_TICK && voiceIdx == 3) {
            nominalDynamicFrom = dynamicLevelFromType(mu::engraving::DynamicType::PPP); // added as a separate dynamic
        }

        DynamicsCurve curve = hairpinCurve(hairpin, nominalDynamicFrom, nominalDynamicTo);
        ASSERT_FALSE(curve.empty());

        if (hairpin->applyToAllVoices()) {
            for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
                for (const auto& pair: curve) {
                    dynamic_level_t actualLevel = ctx.appliableDynamicLevel(voiceIdx, tickFrom + pair.first);
                    dynamic_level_t expectedLevel = nominalDynamicFrom + pair.second;
                    EXPECT_EQ(actualLevel, expectedLevel);
                }
            }
        } else {
            for (const auto& pair: curve) {
                dynamic_level_t actualLevel = ctx.appliableDynamicLevel(voiceIdx, tickFrom + pair.first);
                dynamic_level_t expectedLevel = nominalDynamicFrom + pair.second;
                EXPECT_EQ(actualLevel, expectedLevel);
            }
        }
    }

    delete score;
}

TEST_F(Engraving_PlaybackContextTests, ParsePlayTechAnnotations)
{
    // [GIVEN] Score with playing technique annotations
    Score* score = ScoreRW::readScore(PLAYBACK_CONTEXT_TEST_FILES_DIR + "play_tech_annotations.mscx");

    const std::vector<Part*>& parts = score->parts();
    ASSERT_FALSE(parts.empty());

    // [GIVEN] Context for parsing techniques
    PlaybackContext ctx;

    // [THEN] No technique parsed, returns the "Standard" acticulation
    int maxTick = score->endTick().ticks();

    for (int tick = 0; tick <= maxTick; tick += TICK_STEP) {
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
        auto it = mu::findLessOrEqual(expectedArticulationTypes, tick);
        if (it == expectedArticulationTypes.end()) {
            return ArticulationType::Standard;
        }

        return it->second;
    };

    for (int tick = 0; tick <= maxTick; tick += TICK_STEP) {
        ArticulationType actualType = ctx.persistentArticulationType(tick);
        ArticulationType expectedType = findExpectedType(tick);
        EXPECT_EQ(actualType, expectedType);
    }

    // [WHEN] Clear the context
    ctx.clear();

    // [THEN] No technique parsed, returns the "Standard" acticulation
    for (int tick = 0; tick <= maxTick; tick += TICK_STEP) {
        ArticulationType actualType = ctx.persistentArticulationType(tick);
        EXPECT_EQ(actualType, ArticulationType::Standard);
    }

    delete score;
}
