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

#include "log.h"
#include "audiosanitizer.h"

using namespace mu::audio;

static constexpr size_t DEFAULT_SIZE_PER_CHANNEL = 1024 * 32;
static constexpr size_t DEFAULT_SIZE = DEFAULT_SIZE_PER_CHANNEL * 2;

static const std::vector<float> SILENT_FRAMES(DEFAULT_SIZE, 0.f);

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

void AudioBuffer::init(const audioch_t audioChannelsCount, const samples_t renderStep)
{
    m_samplesPerChannel = DEFAULT_SIZE_PER_CHANNEL;
    m_audioChannelsCount = audioChannelsCount;
    m_renderStep = renderStep;

    m_data.resize(m_samplesPerChannel * m_audioChannelsCount, 0.f);
}

void AudioBuffer::setSource(std::shared_ptr<IAudioSource> source)
{
    if (m_source == source) {
        return;
    }

    if (m_source) {
        reset();
    }

    m_source = source;
}

void AudioBuffer::forward()
{
    if (!m_source) {
        return;
    }

    const auto currentWriteIdx = m_writeIndex.load(std::memory_order_relaxed);
    const auto currentReadIdx = m_readIndex.load(std::memory_order_acquire);
    size_t nextWriteIdx = currentWriteIdx;

    samples_t framesToReserve = DEFAULT_SIZE / 2;

    while (reservedFrames(nextWriteIdx, currentReadIdx) < framesToReserve) {
        m_source->process(m_data.data() + nextWriteIdx, m_renderStep);

        nextWriteIdx = incrementWriteIndex(nextWriteIdx, m_renderStep);
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

    if (reservedFrames(currentWriteIdx, currentReadIdx) < (sampleCount * 2)) {
        static size_t missingFramesTotal = 0;
        missingFramesTotal += (sampleCount * 2);
        LOGD() << "\n FRAMES MISSED " << sampleCount * 2 << ", reserve: " <<
            reservedFrames(currentWriteIdx, currentReadIdx) << ", total: " << missingFramesTotal;
    }

    size_t newReadIdx = currentReadIdx;

    size_t from = newReadIdx;
    auto memStep = sizeof(float);
    size_t to = from + sampleCount * m_audioChannelsCount;
    if (to > DEFAULT_SIZE) {
        to = DEFAULT_SIZE;
    }
    auto count = to - from;
    std::memcpy(dest, m_data.data() + from, count * memStep);
    newReadIdx += count;

    size_t left = sampleCount * m_audioChannelsCount - count;
    if (left > 0) {
        std::memcpy(dest + count, m_data.data(), left * memStep);
        newReadIdx = left;
    }

    if (newReadIdx >= DEFAULT_SIZE) {
        newReadIdx -= DEFAULT_SIZE;
    }

    m_readIndex.store(newReadIdx, std::memory_order_release);
}

void AudioBuffer::setMinSamplesToReserve(size_t lag)
{
    IF_ASSERT_FAILED(lag < DEFAULT_SIZE) {
        lag = DEFAULT_SIZE;
    }
    m_minSamplesToReserve = lag;
}

void AudioBuffer::reset()
{
    m_readIndex.store(0, std::memory_order_release);
    m_writeIndex.store(0, std::memory_order_release);

    m_data = SILENT_FRAMES;
}

size_t AudioBuffer::incrementWriteIndex(const size_t writeIdx, const samples_t samplesPerChannel)
{
    size_t result = writeIdx;
    size_t from = writeIdx;

    auto to = writeIdx + samplesPerChannel * m_audioChannelsCount;
    if (to > DEFAULT_SIZE) {
        to = DEFAULT_SIZE - 1;
    }
    auto count = to - from;
    result += count;

    if (result >= DEFAULT_SIZE) {
        result -= DEFAULT_SIZE;
    }

    return result;
}

size_t AudioBuffer::reservedFrames(const size_t writeIdx, const size_t readIdx) const
{
    size_t result = 0;
    if (readIdx <= writeIdx) {
        result = writeIdx - readIdx;
    } else {
        result = writeIdx + DEFAULT_SIZE - readIdx;
    }

    return result;
}
