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
#ifndef MU_AUDIO_BUFFER_H
#define MU_AUDIO_BUFFER_H

#include <vector>
#include <memory>
#include <atomic>

#include "iaudiosource.h"
#include "audiotypes.h"

//!Note Somehow clang has this define, but doesn't have symbols for std::hardware_destructive_interference_size
#if defined(__cpp_lib_hardware_interference_size) && !defined(Q_OS_MACOS)
#include <new>
constexpr size_t cache_line_size = std::hardware_destructive_interference_size;
#else
constexpr size_t cache_line_size = 64;
#endif

namespace mu::audio {
class AudioBuffer
{
public:
    AudioBuffer() = default;

    void init(const audioch_t audioChannelsCount, const samples_t renderStep);

    void setSource(std::shared_ptr<IAudioSource> source);
    void forward();

    void pop(float* dest, size_t sampleCount);
    void setMinSamplesToReserve(size_t lag);

    void reset();

private:
    size_t reservedFrames(const size_t writeIdx, const size_t readIdx) const;
    size_t incrementWriteIndex(const size_t writeIdx, const samples_t samplesPerChannel);

    size_t m_minSamplesToReserve = 0;

    alignas(cache_line_size) std::atomic<size_t> m_writeIndex = 0;
    alignas(cache_line_size) std::atomic<size_t> m_readIndex = 0;
    alignas(cache_line_size) std::vector<float> m_data;

    samples_t m_samplesPerChannel = 0;
    audioch_t m_audioChannelsCount = 0;

    samples_t m_renderStep = 0;

    std::shared_ptr<IAudioSource> m_source = nullptr;
};

using AudioBufferPtr = std::shared_ptr<AudioBuffer>;
}

#endif // MU_AUDIO_BUFFER_H
