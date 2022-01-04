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

#include <cstring>

#include "log.h"

using namespace mu::audio;

void AudioBuffer::init(const audioch_t audioChannelsCount, const samples_t samplesPerChannel)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_samplesPerChannel = samplesPerChannel;
    m_audioChannelsCount = audioChannelsCount;

    m_data.resize(m_samplesPerChannel * m_audioChannelsCount, 0.f);
}

void AudioBuffer::setSource(std::shared_ptr<IAudioSource> source)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_source = source;
}

void AudioBuffer::forward()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    fillup();
}

void AudioBuffer::pop(float* dest, size_t sampleCount)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t from = m_readIndex;
    auto memStep = sizeof(float);
    size_t to = m_readIndex + sampleCount * m_audioChannelsCount;
    if (to > m_data.size()) {
        to = m_data.size();
    }
    auto count = to - from;
    std::memcpy(dest, m_data.data() + from, count * memStep);
    m_readIndex += count;

    size_t left = sampleCount * m_audioChannelsCount - count;
    if (left > 0) {
        std::memcpy(dest + count, m_data.data(), left * memStep);
        m_readIndex = left;
    }

    if (m_readIndex >= m_data.size()) {
        m_readIndex -= m_data.size();
    }
}

void AudioBuffer::setMinSampleLag(size_t lag)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    IF_ASSERT_FAILED(lag < m_data.size()) {
        lag = m_data.size();
    }
    m_minSampleLag = lag;
}

void AudioBuffer::fillup()
{
    if (!m_source) {
        return;
    }

    while (sampleLag() < m_minSampleLag + FILL_OVER) {
        m_source->process(m_data.data() + m_writeIndex, FILL_SAMPLES);
        updateWriteIndex(FILL_SAMPLES);
    }
}

void AudioBuffer::updateWriteIndex(const unsigned int samplesPerChannel)
{
    size_t from = m_writeIndex;

    auto to = m_writeIndex + samplesPerChannel * m_audioChannelsCount;
    if (to > m_data.size()) {
        to = m_data.size() - 1;
    }
    auto count = to - from;
    m_writeIndex += count;

    if (m_writeIndex >= m_data.size()) {
        m_writeIndex -= m_data.size();
    }
}

unsigned int AudioBuffer::sampleLag() const
{
    size_t lag = 0;
    if (m_readIndex <= m_writeIndex) {
        lag = m_writeIndex - m_readIndex;
    } else {
        lag = m_writeIndex + m_data.size() - m_readIndex;
    }

    return static_cast<unsigned int>(lag / m_audioChannelsCount);
}
