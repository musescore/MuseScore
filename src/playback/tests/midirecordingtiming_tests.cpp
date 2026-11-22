/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <cmath>

#include <gtest/gtest.h>

#include "playback/internal/midirecordingtiming.h"

using namespace mu::playback::midirecording;

namespace {
constexpr double TICKS_PER_QUARTER = 480.0;
constexpr int SIXTEENTH = 120;

//! audio::minSamplesToReserve(RenderMode::RealTimeMode)

//! 120 BPM: 2 quarters per second
double secsToTicksAt120Bpm(double secs)
{
    return secs * 2.0 * TICKS_PER_QUARTER;
}

//! Same rounding as the MIDI import engine with 16th quantization
int quantizeTo16th(int tick)
{
    return static_cast<int>(std::lround(static_cast<double>(tick) / SIXTEENTH)) * SIXTEENTH;
}
}

class MidiRecordingTiming_Tests : public ::testing::Test
{
};

//! The reported playback position is the position being rendered, which is ahead of what is heard.
//! With the default buffer size the difference is larger than half a 16th at 120 BPM:
//! this is what moved every note one 16th later
TEST_F(MidiRecordingTiming_Tests, outputLatencyWithDefaultBufferIsLargerThanHalfA16th)
{
    const double latency = estimateOutputLatencySecs(1024, 48000);

    // (1024 reported ahead + 2 * 1024 AudioQueue + 512 device) / 48000
    EXPECT_NEAR(latency, 3584.0 / 48000.0, 1e-9);

    const double halfSixteenthAt120Bpm = 0.125 / 2.0;
    EXPECT_GT(latency, halfSixteenthAt120Bpm);
}

TEST_F(MidiRecordingTiming_Tests, outputLatencyFollowsTheDriverBufferSize)
{
    // Small buffers: the engine renders in the driver callback, so the latency gets smaller too
    const double latency = estimateOutputLatencySecs(256, 48000);
    EXPECT_NEAR(latency, (256.0 + 512.0 + 128.0) / 48000.0, 1e-9);

    EXPECT_EQ(estimateOutputLatencySecs(0, 48000), 0.0);
    EXPECT_EQ(estimateOutputLatencySecs(1024, 0), 0.0);
}

//! The player presses a key exactly when they hear each beat of the recorded measure
TEST_F(MidiRecordingTiming_Tests, notesOnTheBeatAreNotShiftedWhenLatencyIsCompensated)
{
    const double startSecs = 2.0; // start of the recorded measure
    const double latency = estimateOutputLatencySecs(1024, 48000);

    for (int beat = 0; beat < 4; ++beat) {
        const double heardSecs = startSecs + beat * 0.5;
        const double renderedSecs = heardSecs + latency; // the position reported at that moment

        const int expectedTick = beat * static_cast<int>(TICKS_PER_QUARTER);

        // Without compensation: the bug, one 16th later after quantization
        const int uncompensated = positionToRelativeTick(renderedSecs, startSecs, 0.0, secsToTicksAt120Bpm);
        EXPECT_EQ(quantizeTo16th(uncompensated), expectedTick + SIXTEENTH);

        // With compensation: exactly on the beat
        const int compensated = positionToRelativeTick(renderedSecs, startSecs, latency, secsToTicksAt120Bpm);
        EXPECT_EQ(compensated, expectedTick);
        EXPECT_EQ(quantizeTo16th(compensated), expectedTick);
    }
}

TEST_F(MidiRecordingTiming_Tests, userCompensationMovesNotesEarlier)
{
    const double startSecs = 0.0;

    // 50 ms of extra latency (e.g. the output device itself) at 120 BPM = 48 ticks
    const double heardSecs = 1.0;
    const double renderedSecs = heardSecs + 0.050;

    EXPECT_EQ(positionToRelativeTick(renderedSecs, startSecs, 0.050, secsToTicksAt120Bpm), 960);
    EXPECT_EQ(positionToRelativeTick(renderedSecs, startSecs, 0.0, secsToTicksAt120Bpm), 1008);
}

TEST_F(MidiRecordingTiming_Tests, notesPlayedJustBeforeTheFirstBeatAreNegative)
{
    // Played 25 ms before the first beat is heard: a small negative tick (moved onto the beat later)
    const double startSecs = 1.0;
    const double latency = 0.1;
    const double renderedSecs = startSecs - 0.025 + latency;

    EXPECT_EQ(positionToRelativeTick(renderedSecs, startSecs, latency, secsToTicksAt120Bpm), -24);
}
