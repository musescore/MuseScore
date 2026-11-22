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

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>

//! Timing calculations of the MIDI recording with metronome (no dependencies, to be testable in isolation)
namespace mu::playback::midirecording {
//! Converts a playback position (as reported by the audio engine) into ticks relative to the recording start.
//! The reported position is the position being rendered, which is ahead of what is heard by outputLatencySecs:
//! the player plays along with what they hear.
//! secsToPlayedTicks: the tempo map, i.e. the playback tick at a given time in seconds
inline int positionToRelativeTick(double renderedSecs, double startSecs, double outputLatencySecs,
                                  const std::function<double(double secs)>& secsToPlayedTicks)
{
    const double heardSecs = renderedSecs - outputLatencySecs;
    const double ticks = secsToPlayedTicks(heardSecs) - secsToPlayedTicks(startSecs);

    return static_cast<int>(std::lround(ticks));
}

//! Estimates how much audio is rendered in advance of what is heard, along the output chain:
//! - the reported position is the end of the block being rendered: one driver buffer ahead (Mixer::process forwards the clock first)
//! - the driver queues 2 buffers (OSXAudioDriver: 2 AudioQueue buffers), plus the buffer played by the device (on average half)
//! The engine renders directly in the driver callback, so there is no additional buffering in between.
//! The latency of the output device itself is not known: it can be compensated by the user in the preferences.
inline double estimateOutputLatencySecs(uint64_t driverBufferSize, uint64_t sampleRate)
{
    if (driverBufferSize == 0 || sampleRate == 0) {
        return 0.0;
    }

    const double bufferSize = static_cast<double>(driverBufferSize);

    const double reportedAhead = bufferSize;
    const double driverQueue = 2.0 * bufferSize;
    const double devicePlaying = 0.5 * bufferSize;

    return (reportedAhead + driverQueue + devicePlaying) / static_cast<double>(sampleRate);
}
}
