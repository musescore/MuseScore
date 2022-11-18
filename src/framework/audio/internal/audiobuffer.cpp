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

static constexpr size_t DEFAULT_SIZE_PER_CHANNEL = 1024 * 16;
static constexpr size_t DEFAULT_SIZE = DEFAULT_SIZE_PER_CHANNEL * 2;

static const std::vector<float> SILENT_FRAMES(DEFAULT_SIZE, 0.f);

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

    samples_t sampleToReserve = m_minSamplesToReserve * 4;

    while (reservedSamples(nextWriteIdx, currentReadIdx) < sampleToReserve) {
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

size_t AudioBuffer::reservedSamples(const size_t writeIdx, const size_t readIdx) const
{
    size_t result = 0;
    if (readIdx <= writeIdx) {
        result = writeIdx - readIdx;
    } else {
        result = writeIdx + DEFAULT_SIZE - readIdx;
    }

    return result;
}
