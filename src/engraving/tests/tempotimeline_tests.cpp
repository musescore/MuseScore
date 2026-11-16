/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <chrono>
#include <iostream>
#include <map>
#include <optional>
#include <vector>

#include "engraving/automation/tempovalues.h"
#include "engraving/dom/tempotimeline.h"
#include "engraving/types/constants.h"

#include "log.h"

using namespace mu::engraving;

static constexpr double TEMPO_ERROR(0.000001);
static constexpr double TIME_ERROR(0.000001);

using AutomationEase = AutomationPoint::Ease;

class Engraving_TempoTimelineTests : public ::testing::Test
{
};

//! NOTE: a flat point - the tempo jumps to bpm immediately and holds, no ramp
static AutomationPoint flatTempo(double bpm)
{
    AutomationPoint p;
    p.value.outValue = normalizeTempo(BeatsPerSecond::fromBPM(bpm));
    return p;
}

//! NOTE: the arrival of a gradual tempo change - the curve ramps towards targetBpm as this point is
//! approached, then holds at heldBpm from here on
//! See: https://github.com/musescore/MuseScore/issues/12140
static AutomationPoint rampArrival(double heldBpm, double targetBpm, AutomationEase ease = AutomationEase::none())
{
    AutomationPoint p;
    p.value.outValue = normalizeTempo(BeatsPerSecond::fromBPM(heldBpm));
    p.value.inValue = AutomationPoint::ExplicitArrival { normalizeTempo(BeatsPerSecond::fromBPM(targetBpm)), ease };
    return p;
}

static std::optional<double> bpsAtUTick(const TempoTimeline& timeline, int utick)
{
    for (const TempoTimePoint& point : timeline.points()) {
        if (point.utick == utick) {
            return point.bps;
        }
    }

    return std::nullopt;
}

TEST_F(Engraving_TempoTimelineTests, EMPTY_CURVE_USES_DEFAULT_TEMPO)
{
    // [GIVEN] An empty tempo curve (no tempo markings anywhere in the score)
    TempoTimeline timeline;

    // [WHEN] Building the timeline from it
    timeline.rebuild(AutomationCurve {}, PausesMap {});

    // [THEN] It falls back to a single point at the default tempo
    const std::vector<TempoTimePoint>& points = timeline.points();
    ASSERT_EQ(points.size(), 1);
    EXPECT_EQ(points[0].utick, 0);
    EXPECT_NEAR(points[0].bps, Constants::DEFAULT_TEMPO.val, TEMPO_ERROR);
}

/**
 * @brief Engraving_TempoTimelineTests_FLAT_TEMPO_CHANGES
 * @details Three flat tempo markings: 80 BPM from the start, 120 BPM from beat 4 (tick 1920),
 *          60 BPM from beat 8 (tick 3840). None of them ramp into one another
 */
TEST_F(Engraving_TempoTimelineTests, FLAT_TEMPO_CHANGES)
{
    // [GIVEN] The curve described above
    AutomationCurve curve {
        { 0, flatTempo(80.0) },
        { 1920, flatTempo(120.0) },
        { 3840, flatTempo(60.0) },
    };

    // [WHEN] Build the timeline using the curve
    TempoTimeline timeline;
    timeline.rebuild(curve, {});

    // [GIVEN] The same tempo values, expressed directly as tick->bps instead of an AutomationCurve
    TempoValues values {
        { 0, BeatsPerSecond::fromBPM(80.0).val },
        { 1920, BeatsPerSecond::fromBPM(120.0).val },
        { 3840, BeatsPerSecond::fromBPM(60.0).val },
    };

    // [WHEN] Build the timeline using the values
    TempoTimeline valuesTimeline;
    valuesTimeline.rebuild(values, {});

    // [THEN] rebuild(AutomationCurve, PausesMap) forwards to rebuild(TempoValues, PausesMap)
    // after resampling, so equivalent input produces byte-identical points
    ASSERT_EQ(timeline.points().size(), valuesTimeline.points().size());
    for (size_t i = 0; i < timeline.points().size(); ++i) {
        EXPECT_EQ(timeline.points()[i].utick, valuesTimeline.points()[i].utick) << "i=" << i;
        EXPECT_NEAR(timeline.points()[i].time, valuesTimeline.points()[i].time, TIME_ERROR) << "i=" << i;
        EXPECT_NEAR(timeline.points()[i].bps, valuesTimeline.points()[i].bps, TEMPO_ERROR) << "i=" << i;
    }

    // [AND] Each marking produces exactly one point - none of them get resampled
    const std::vector<TempoTimePoint>& points = timeline.points();
    ASSERT_EQ(points.size(), 3);

    struct Segment {
        int fromUTick;
        double bpm;
        double fromTime;
    };
    const std::vector<Segment> segments {
        { 0, 80.0, 0.0 }, // 1920 ticks @ 80 BPM (640 ticks/sec) = 3s
        { 1920, 120.0, 3.0 }, // 1920 ticks @ 120 BPM (960 ticks/sec) = 2s
        { 3840, 60.0, 5.0 }, // held indefinitely from here on
    };

    for (size_t i = 0; i < segments.size(); ++i) {
        const Segment& seg = segments[i];
        const int segEnd = seg.fromUTick + (i + 1 < segments.size() ? segments[i + 1].fromUTick - seg.fromUTick : 1920);
        const double ticksPerSec = Constants::DIVISION * BeatsPerSecond::fromBPM(seg.bpm).val;

        for (int utick = seg.fromUTick; utick < segEnd; utick += 480) {
            const double expectedTime = seg.fromTime + (utick - seg.fromUTick) / ticksPerSec;

            EXPECT_NEAR(timeline.tempo(utick).val, BeatsPerSecond::fromBPM(seg.bpm).val, TEMPO_ERROR) << "utick=" << utick;
            EXPECT_NEAR(timeline.utick2utime(utick), expectedTime, TIME_ERROR) << "utick=" << utick;
            EXPECT_EQ(timeline.utime2utick(expectedTime), utick) << "time=" << expectedTime;
        }
    }

    // [AND] both hold indefinitely far past the last marking too
    EXPECT_NEAR(timeline.tempo(100000).val, BeatsPerSecond::fromBPM(60.0).val, TEMPO_ERROR);
    EXPECT_NEAR(timeline.utick2utime(100000), 5.0 + (100000 - 3840) / (Constants::DIVISION * BeatsPerSecond::fromBPM(60.0).val),
                TIME_ERROR);
}

/**
 * @brief Engraving_TempoTimelineTests_GRADUAL_TEMPO_CHANGE_RAMP_AND_OVERRIDE
 * @details A ritardando ramps from 120 BPM to 90 BPM across ticks 0-3840 (factor 0.75, per
 *          GradualTempoChange::DEFAULT_FACTORS_MAP). An 80 BPM tempo marking sits at tick 1440,
 *          inside the ramp's span. A "Presto" marking (187 BPM) sits at tick 3840, exactly where
 *          the ritardando arrives - ScoreAutomationController merges into it
 *          rather than overwriting it, so its outValue stays 187 BPM while its
 *          inValue becomes the ritardando's arrival (target 90 BPM)
 *          See: https://github.com/musescore/MuseScore/issues/12140
 *
 *          A second, unrelated accelerando then runs from 150 to 200 BPM across ticks 5760-7680,
 *          preceded by a flat 150 BPM marking at tick 4800. Its own start (5760) is flat relative
 *          to that marking, not to the first ritardando's 90 BPM target, so it must not be treated
 *          as a continuation of the first ramp
 */
TEST_F(Engraving_TempoTimelineTests, GRADUAL_TEMPO_CHANGE_RAMP_AND_OVERRIDE)
{
    // [GIVEN] The curve described above
    AutomationCurve curve {
        { 0, flatTempo(120.0) },
        { 1440, flatTempo(80.0) },
        { 3840, rampArrival(187.0, 90.0) },
        { 4800, flatTempo(150.0) },
        { 5760, rampArrival(150.0, 150.0) }, // 2nd ramp's own flat start anchor
        { 7680, rampArrival(200.0, 200.0) },
    };

    TempoTimeline timeline;
    timeline.rebuild(curve, {});

    // [THEN] The 1st ramp is resampled in 8 steps (3840 ticks / DIVISION(480) = 8), landing on every
    // multiple of 480 ticks. The authored points at 1440 and 3840 win over their resampled value.
    // Ticks 4800-5760 stay flat (150 BPM, no interpolation) rather than ramping from the 1st ramp's
    // 90 BPM arrival. The 2nd ramp is then resampled the same way, from its own 150 BPM start
    std::map<int, double> expectedBpm {
        { 0, 120.0 },
        { 480, 116.25 },
        { 960, 112.5 },
        { 1440, 80.0 }, // authored marking - NOT the ramp's resampled ~108.75
        { 1920, 105.0 },
        { 2400, 101.25 },
        { 2880, 97.5 },
        { 3360, 93.75 },
        { 3840, 187.0 }, // authored Presto - NOT the ramp's own target of 90
        { 4800, 150.0 }, // flat marking, unrelated to either ramp
        { 5760, 150.0 }, // 2nd ramp's own flat start - NOT bridged to the 1st ramp's 90 BPM arrival
        { 6000, 156.25 },
        { 6240, 162.5 },
        { 6480, 168.75 },
        { 6720, 175.0 },
        { 6960, 181.25 },
        { 7200, 187.5 },
        { 7440, 193.75 },
        { 7680, 200.0 },
    };

    const std::vector<TempoTimePoint>& points = timeline.points();
    ASSERT_EQ(points.size(), expectedBpm.size());

    for (const auto& [utick, expectedBpmValue] : expectedBpm) {
        std::optional<double> bps = bpsAtUTick(timeline, utick);
        ASSERT_TRUE(bps.has_value()) << "utick=" << utick;
        EXPECT_NEAR(*bps, BeatsPerSecond::fromBPM(expectedBpmValue).val, TEMPO_ERROR) << "utick=" << utick;
    }
}

/**
 * @brief Engraving_TempoTimelineTests_PAUSES
 * @details A single flat 120 BPM tempo with a 1.5s pause at tick 960 (2 beats in). Checks that the
 *          pause both shows up as its own point and is accounted for by utick2utime/utime2utick
 */
TEST_F(Engraving_TempoTimelineTests, PAUSES)
{
    // [GIVEN] A flat 120 BPM tempo curve and a 1.5s pause 2 beats in (tick 960)
    AutomationCurve curve {
        { 0, flatTempo(120.0) },
    };
    PausesMap pauses {
        { 960, 1.5 },
    };

    TempoTimeline timeline;
    timeline.rebuild(curve, pauses);

    // [THEN] The pause is inserted as its own point, holding tempo but marking the hold duration
    const std::vector<TempoTimePoint>& points = timeline.points();
    ASSERT_EQ(points.size(), 2);
    EXPECT_EQ(points[0].utick, 0);
    EXPECT_NEAR(points[0].time, 0.0, TIME_ERROR);
    EXPECT_NEAR(points[0].pause, 0.0, TIME_ERROR);
    EXPECT_EQ(points[1].utick, 960);
    EXPECT_NEAR(points[1].time, 2.5, TIME_ERROR); // 1s nominal to reach tick 960 + the 1.5s pause
    EXPECT_NEAR(points[1].pause, 1.5, TIME_ERROR);

    // [AND] utick2utime jumps by the pause duration once the pause tick is reached
    EXPECT_NEAR(timeline.utick2utime(480), 0.5, TIME_ERROR);
    EXPECT_NEAR(timeline.utick2utime(960), 2.5, TIME_ERROR);

    // [AND] utime2utick holds at the pause's tick for its entire duration, then resumes advancing
    EXPECT_EQ(timeline.utime2utick(0.5), 480); // before the pause
    EXPECT_EQ(timeline.utime2utick(1.0), 960); // pause starts
    EXPECT_EQ(timeline.utime2utick(1.8), 960); // mid-pause
    EXPECT_EQ(timeline.utime2utick(2.5), 960); // pause ends
    EXPECT_EQ(timeline.utime2utick(3.0), 1440); // resumed: 0.5s @ 120 BPM = 480 ticks past 960
}

/**
 * @brief Engraving_TempoTimelineTests_TEMPO_NORMALIZE_DENORMALIZE_ROUND_TRIP
 * @details normalizeTempo/denormalizeTempo are used to bring every Tempo
 *          AutomationCurve value into [0, 1], and back out again when TempoTimeline materializes its points
 *          Check that round-tripping through them doesn't lose precision
 */
TEST_F(Engraving_TempoTimelineTests, TEMPO_NORMALIZE_DENORMALIZE_ROUND_TRIP)
{
    // [GIVEN] Every named tempo marking in Palettes
    std::vector<BeatsPerSecond> tempos {
        Constants::MIN_TEMPO,
        Constants::MAX_TEMPO,
        Constants::DEFAULT_TEMPO,
        BeatsPerSecond::fromBPM(35.0), // Grave
        BeatsPerSecond::fromBPM(50.0), // Largo
        BeatsPerSecond::fromBPM(52.5), // Lento
        BeatsPerSecond::fromBPM(60.0), // FLAT_TEMPO_CHANGES / PAUSES
        BeatsPerSecond::fromBPM(63.0), // Larghetto
        BeatsPerSecond::fromBPM(71.0), // Adagio
        BeatsPerSecond::fromBPM(80.0), // custom tempo / default metronome mark
        BeatsPerSecond::fromBPM(90.0), // ritardando target
        BeatsPerSecond::fromBPM(92.0), // Andante
        BeatsPerSecond::fromBPM(94.0), // Andantino
        BeatsPerSecond::fromBPM(105.0), // ritardando midpoint
        BeatsPerSecond::fromBPM(114.0), // Moderato
        BeatsPerSecond::fromBPM(116.0), // Allegretto
        BeatsPerSecond::fromBPM(120.0), // default tempo
        BeatsPerSecond::fromBPM(144.0), // Allegro
        BeatsPerSecond::fromBPM(172.0), // Vivace
        BeatsPerSecond::fromBPM(187.0), // Presto
        BeatsPerSecond::fromBPM(200.0), // Prestissimo
    };

    // [GIVEN] A dense sweep across the entire valid tempo range for exhaustive coverage
    static constexpr int SWEEP_STEPS = 1000;
    tempos.reserve(tempos.size() + SWEEP_STEPS + 1);
    for (int i = 0; i <= SWEEP_STEPS; ++i) {
        const double t = double(i) / SWEEP_STEPS;
        tempos.push_back(BeatsPerSecond(Constants::MIN_TEMPO.val + t * (Constants::MAX_TEMPO.val - Constants::MIN_TEMPO.val)));
    }

    // [WHEN] Normalizing and denormalizing each tempo
    for (const BeatsPerSecond& bps : tempos) {
        const real_t normalized = normalizeTempo(bps);
        const BeatsPerSecond roundTripped = denormalizeTempo(normalized);

        // [THEN] The round trip preserves the value to within double-precision rounding
        EXPECT_NEAR(roundTripped.val, bps.val, 1e-12) << "bps=" << bps.val;
    }
}

//! NOTE: benchmarks rebuild() against a worst-case chain of gradual tempo ramps and pauses
TEST_F(Engraving_TempoTimelineTests, DISABLED_REBUILD_PERFORMANCE)
{
    // [GIVEN] A chain of RAMP_COUNT gradual tempo changes, each spanning RAMP_SPAN_MEASURES
    // measures, alternating faster/slower so no two neighbors are flat relative to each other
    constexpr int RAMP_COUNT = 2500;
    constexpr int RAMP_SPAN_MEASURES = 8;
    constexpr int TICKS_PER_MEASURE = 1920; // 4/4 at DIVISION(480)
    constexpr int RAMP_SPAN_TICKS = RAMP_SPAN_MEASURES * TICKS_PER_MEASURE;

    AutomationCurve curve;
    double bpm = 80.0;
    curve.emplace(0, flatTempo(bpm));
    for (int i = 0; i < RAMP_COUNT; ++i) {
        bpm = (i % 2 == 0) ? bpm * 1.25 : bpm / 1.25;
        curve.emplace((i + 1) * RAMP_SPAN_TICKS, rampArrival(bpm, bpm));
    }

    PausesMap pauses;
    for (int i = 0; i < RAMP_COUNT; i += 4) {
        pauses.emplace(i * RAMP_SPAN_TICKS, 0.1);
    }

    // [WHEN] Rebuilding repeatedly
    constexpr int ITERATIONS = 20;
    TempoTimeline timeline;

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        timeline.rebuild(curve, pauses);
    }
    const auto end = std::chrono::steady_clock::now();

    const double totalMs = std::chrono::duration<double, std::milli>(end - start).count();
    LOGD() << "[PERF] TempoTimeline::rebuild(): " << curve.size() << " authored points -> "
           << timeline.points().size() << " resulting points, "
           << (totalMs / ITERATIONS) << " ms/iteration (avg over " << ITERATIONS << " iterations)";

    // [THEN] Nothing to assert on timing itself
    // just confirm the rebuild actually did the expected amount of work
    EXPECT_GT(timeline.points().size(), curve.size());
}
