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

#include <algorithm>

using namespace mu::audio;

static constexpr size_t DEFAULT_SIZE_PER_CHANNEL = 1024 * 8;
//static constexpr size_t DEFAULT_SIZE = DEFAULT_SIZE_PER_CHANNEL * 2;

//static const std::vector<float> SILENT_FRAMES(DEFAULT_SIZE, 0.f);

//#define DEBUG_AUDIO
#ifdef DEBUG_AUDIO
#define LOG_AUDIO LOGD
#else
#define LOG_AUDIO LOGN
#endif

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

void AudioBuffer::init(const audioch_t audioChannelsCount, const samples_t renderStep)
{
    // Prevent both write and read thread from accessing the data buffer as it might be reallocated
    std::scoped_lock lock(m_writeMutex, m_readMutex);
    m_samplesPerChannel = DEFAULT_SIZE_PER_CHANNEL;
    m_audioChannelsCount = audioChannelsCount;
    m_renderStep = renderStep;

    m_data.resize(m_samplesPerChannel * m_audioChannelsCount, 0.f);
    // Reset read and write indices
    m_readIndex = 0;
    m_writeIndex = 0;
}

void AudioBuffer::setAudioChannelsCount(const audioch_t audioChannelsCount)
{
    if (m_audioChannelsCount != audioChannelsCount) {
        init(audioChannelsCount, m_renderStep);
    }
}

void AudioBuffer::setSource(std::shared_ptr<IAudioSource> source)
{
    // Source pointer is only used in write thread
    std::scoped_lock lock(m_writeMutex);
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
    std::scoped_lock lock(m_writeMutex);
    if (!m_source) {
        return;
    }

    const auto currentWriteIdx = m_writeIndex.load(std::memory_order_relaxed);
    const auto currentReadIdx = m_readIndex.load(std::memory_order_acquire);
    size_t nextWriteIdx = currentWriteIdx;

    samples_t framesToReserve = m_data.size() / 2;

    while (reservedFrames(nextWriteIdx, currentReadIdx) < framesToReserve) {
        samples_t num = m_source->process(m_data.data() + nextWriteIdx, m_data.size() - nextWriteIdx, m_renderStep);
        if (num == 0) {
            // Free audio worker thread if there is no progress
            break;
        }
        // Increment by render step even if num is less, because otherwise we could write over the buffer end
        nextWriteIdx = incrementWriteIndex(nextWriteIdx, m_renderStep);
    }

    m_writeIndex.store(nextWriteIdx, std::memory_order_release);
}

void AudioBuffer::pop(float* dest, size_t byteCount)
{
    // Number of floats to put in buffer
    const size_t size = byteCount / sizeof(float);

    const auto currentReadIdx = m_readIndex.load(std::memory_order_relaxed);
    const auto currentWriteIdx = m_writeIndex.load(std::memory_order_acquire);
    std::unique_lock lock(m_readMutex, std::defer_lock);
    if (!lock.try_lock() || currentReadIdx == currentWriteIdx || m_audioChannelsCount == 0) {
        // empty queue, not initialized or currently blocked (resizing)
        std::fill(dest, dest + size, 0.0f);
        return;
    }

    // Number of samples to put in buffer
    const size_t sampleCount = size / m_audioChannelsCount;

    if (reservedFrames(currentWriteIdx, currentReadIdx) < (sampleCount * 2)) {
        static size_t missingFramesTotal = 0;
        missingFramesTotal += (sampleCount * 2);
        LOG_AUDIO() << "\n FRAMES MISSED " << sampleCount * 2 << ", reserve: " <<
            reservedFrames(currentWriteIdx, currentReadIdx) << ", total: " << missingFramesTotal;
    }

    size_t newReadIdx = currentReadIdx;

    size_t from = newReadIdx;
    auto memStep = sizeof(float);
    size_t to = from + size;
    if (to > m_data.size()) {
        to = m_data.size();
    }
    auto count = to - from;
    std::memcpy(dest, m_data.data() + from, count * memStep);
    newReadIdx += count;

    size_t left = size - count;
    if (left > 0) {
        std::memcpy(dest + count, m_data.data(), left * memStep);
        newReadIdx = left;
    }

    if (newReadIdx >= m_data.size()) {
        newReadIdx -= m_data.size();
    }

    m_readIndex.store(newReadIdx, std::memory_order_release);
}

void AudioBuffer::setMinSamplesToReserve(size_t lag)
{
    IF_ASSERT_FAILED(lag < m_data.size()) {
        lag = m_data.size();
    }
    m_minSamplesToReserve = lag;
}

void AudioBuffer::reset()
{
    m_readIndex.store(0, std::memory_order_release);
    m_writeIndex.store(0, std::memory_order_release);

    std::fill(m_data.begin(), m_data.end(), 0.0f);
}

size_t AudioBuffer::incrementWriteIndex(const size_t writeIdx, const samples_t samplesPerChannel)
{
    size_t result = writeIdx;
    size_t from = writeIdx;

    auto to = writeIdx + samplesPerChannel * m_audioChannelsCount;
    if (to > m_data.size()) {
        to = m_data.size() - 1;
    }
    auto count = to - from;
    result += count;

    if (result >= m_data.size()) {
        result -= m_data.size();
    }

    return result;
}

size_t AudioBuffer::reservedFrames(const size_t writeIdx, const size_t readIdx) const
{
    size_t result = 0;
    if (readIdx <= writeIdx) {
        result = writeIdx - readIdx;
    } else {
        result = writeIdx + m_data.size() - readIdx;
    }

    return result;
}
