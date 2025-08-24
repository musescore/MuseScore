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
#include "audiobuffer.h"

#include <iostream>

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::worker;

static constexpr size_t DEFAULT_SIZE_PER_CHANNEL = 1024 * 8;
static constexpr size_t DEFAULT_SIZE = DEFAULT_SIZE_PER_CHANNEL * 2;

static const std::vector<float> SILENT_FRAMES(DEFAULT_SIZE, 0.f);

//#define DEBUG_AUDIO
#ifdef DEBUG_AUDIO
#define LOG_AUDIO LOGD
#else
#define LOG_AUDIO LOGN
#endif

inline size_t reservedFrames(const size_t writeIdx, const size_t readIdx)
{
    size_t result = 0;
    if (readIdx <= writeIdx) {
        result = writeIdx - readIdx;
    } else {
        result = writeIdx + DEFAULT_SIZE - readIdx;
    }

    return result;
}

struct BaseBufferProfiler {
    size_t reservedFramesMax = 0;
    size_t reservedFramesMin = 0;
    double elapsedMax = 0.0;
    double elapsedSum = 0.0;
    double elapsedMin = 0.0;
    uint64_t callCount = 0;
    uint64_t maxCallCount = 0;
    std::string tag;

    BaseBufferProfiler(std::string&& profileTag, uint64_t profilerMaxCalls)
        : maxCallCount(profilerMaxCalls), tag(std::move(profileTag)) {}

    ~BaseBufferProfiler()
    {
        if (!stopped()) {
            return;
        }

        std::cout << "\n BUFFER PROFILE:    " << tag;
        std::cout << "\n reservedFramesMax: " << reservedFramesMax;
        std::cout << "\n reservedFramesMin: " << reservedFramesMin;
        std::cout << "\n elapsedMax:        " << elapsedMax;
        std::cout << "\n elapsedMin:        " << elapsedMin;
        std::cout << "\n elapsedAvg:        " << elapsedSum / callCount;
        std::cout << "\n total call count:  " << callCount;
        std::cout << "\n ===================\n";
    }

    void add(size_t reservedFrames, double elapsed)
    {
        if (maxCallCount != 0 && callCount > maxCallCount) {
            return;
        }

        callCount++;

        if (callCount == 1) {
            reservedFramesMin = reservedFrames;
            elapsedMin = elapsed;
        } else {
            reservedFramesMin = std::min(reservedFrames, reservedFramesMin);
            elapsedMin = std::min(elapsed, elapsedMin);
        }

        reservedFramesMax = std::max(reservedFrames, reservedFramesMax);
        elapsedMax = std::max(elapsed, elapsedMax);
        elapsedSum += elapsed;
    }

    bool stopped() const
    {
        return callCount >= maxCallCount && maxCallCount != 0;
    }

    void stop()
    {
        if (stopped()) {
            return;
        }

        maxCallCount = callCount - 1;
        LOGD() << "\n PROFILE STOP";
    }
};

static BaseBufferProfiler READ_PROFILE("READ_PROFILE", 3000);
static BaseBufferProfiler WRITE_PROFILE("WRITE_PROFILE", 0);

void AudioBuffer::init(const audioch_t audioChannelsCount)
{
    m_audioChannelsCount = audioChannelsCount;
    m_samplesPerChannel = DEFAULT_SIZE_PER_CHANNEL;
    m_minSamplesToReserve = DEFAULT_SIZE_PER_CHANNEL / 2;
    m_renderStep = m_minSamplesToReserve;

    m_data.resize(m_samplesPerChannel * m_audioChannelsCount, 0.f);
}

void AudioBuffer::setSource(IAudioSourcePtr source)
{
    if (m_source == source) {
        return;
    }

    if (m_source) {
        reset();
    }

    m_source = source;
}

void AudioBuffer::setMinSamplesPerChannelToReserve(const samples_t samplesPerChannel)
{
    IF_ASSERT_FAILED(samplesPerChannel > 0 && samplesPerChannel < DEFAULT_SIZE_PER_CHANNEL) {
        return;
    }

    m_minSamplesToReserve = samplesPerChannel * m_audioChannelsCount;
}

void AudioBuffer::setRenderStep(const samples_t renderStep)
{
    IF_ASSERT_FAILED(renderStep > 0 && renderStep < DEFAULT_SIZE_PER_CHANNEL) {
        return;
    }

    m_renderStep = renderStep;
}

void AudioBuffer::forward()
{
    if (!m_source) {
        return;
    }

    const auto currentWriteIdx = m_writeIndex.load(std::memory_order_relaxed);
    const auto currentReadIdx = m_readIndex.load(std::memory_order_acquire);
    size_t nextWriteIdx = currentWriteIdx;

    while (reservedFrames(nextWriteIdx, currentReadIdx) < m_minSamplesToReserve) {
        samples_t renderStep = m_renderStep;
        samples_t samplesToRender = renderStep * m_audioChannelsCount;

        if (nextWriteIdx + samplesToRender > DEFAULT_SIZE) {
            renderStep = (DEFAULT_SIZE - nextWriteIdx) / m_audioChannelsCount;
            samplesToRender = renderStep * m_audioChannelsCount;
            IF_ASSERT_FAILED(renderStep > 0) {
                break;
            }
        }

        m_source->process(m_data.data() + nextWriteIdx, renderStep);

        nextWriteIdx += samplesToRender;
        if (nextWriteIdx >= DEFAULT_SIZE) {
            nextWriteIdx = 0;
        }
    }

    m_writeIndex.store(nextWriteIdx, std::memory_order_release);
}

void AudioBuffer::pop(float* dest, size_t sampleCount)
{
    const auto currentReadIdx = m_readIndex.load(std::memory_order_relaxed);
    const auto currentWriteIdx = m_writeIndex.load(std::memory_order_acquire);
    if (currentReadIdx == currentWriteIdx) { // empty queue
        std::memcpy(dest, SILENT_FRAMES.data(), sampleCount * sizeof(float) * m_audioChannelsCount);
        return;
    }

#ifdef DEBUG_AUDIO
    if (reservedFrames(currentWriteIdx, currentReadIdx) < (sampleCount * m_audioChannelsCount)) {
        static size_t missingFramesTotal = 0;
        missingFramesTotal += (sampleCount * m_audioChannelsCount);
        LOG_AUDIO() << "\n FRAMES MISSED " << sampleCount * m_audioChannelsCount << ", reserve: " <<
            reservedFrames(currentWriteIdx, currentReadIdx) << ", total: " << missingFramesTotal;
    }
#endif

    size_t newReadIdx = currentReadIdx;
    const size_t totalSampleCount = sampleCount * m_audioChannelsCount;

    size_t from = newReadIdx;
    auto memStep = sizeof(float);
    size_t to = from + totalSampleCount;
    if (to > DEFAULT_SIZE) {
        to = DEFAULT_SIZE;
    }

    auto count = to - from;
    std::memcpy(dest, m_data.data() + from, count * memStep);
    newReadIdx += count;

    size_t left = totalSampleCount - count;
    if (left > 0) {
        std::memcpy(dest + count, m_data.data(), left * memStep);
        newReadIdx = left;
    }

    if (newReadIdx >= DEFAULT_SIZE) {
        newReadIdx -= DEFAULT_SIZE;
    }

    m_readIndex.store(newReadIdx, std::memory_order_release);
}

void AudioBuffer::reset()
{
    m_readIndex.store(0, std::memory_order_release);
    m_writeIndex.store(0, std::memory_order_release);

    m_data = SILENT_FRAMES;
}

audioch_t AudioBuffer::audioChannelCount() const
{
    return m_audioChannelsCount;
}
