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
#ifndef MUSE_AUDIO_BUFFER_H
#define MUSE_AUDIO_BUFFER_H

#include <vector>
#include <memory>
#include <atomic>

#include "audio/worker/iaudiosource.h"
#include "audio/common/audiotypes.h"

//!Note Somehow clang has this define, but doesn't have symbols for std::hardware_destructive_interference_size
#if defined(__cpp_lib_hardware_interference_size) && !defined(Q_OS_MACOS)
#include <new>
constexpr size_t cache_line_size = std::hardware_destructive_interference_size;
#else
constexpr size_t cache_line_size = 64;
#endif

#if (defined (_MSCVER) || defined (_MSC_VER))
// structure was padded due to alignment specifier
#pragma warning(disable: 4324)
#endif

namespace muse::audio {
class AudioBuffer
{
public:
    AudioBuffer() = default;

    void init(const audioch_t audioChannelsCount);

    void setSource(worker::IAudioSourcePtr source);
    void setMinSamplesPerChannelToReserve(const samples_t samplesPerChannel);
    void setRenderStep(const samples_t renderStep);

    void forward();
    void pop(float* dest, size_t sampleCount);

    void reset();

    audioch_t audioChannelCount() const;

private:
    alignas(cache_line_size) std::atomic<size_t> m_writeIndex = 0;
    alignas(cache_line_size) std::atomic<size_t> m_readIndex = 0;
    alignas(cache_line_size) std::vector<float> m_data;

    samples_t m_samplesPerChannel = 0;
    audioch_t m_audioChannelsCount = 0;
    samples_t m_minSamplesToReserve = 0;
    samples_t m_renderStep = 0;

    worker::IAudioSourcePtr m_source = nullptr;
};

using AudioBufferPtr = std::shared_ptr<AudioBuffer>;
}

#endif // MUSE_AUDIO_BUFFER_H
