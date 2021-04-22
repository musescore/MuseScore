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

void AudioBuffer::init(int samplesPerChannel)
{
    m_audioChannelsCount = config()->audioChannelsCount();
    m_data.resize(samplesPerChannel * m_audioChannelsCount, 0.f);
}

void AudioBuffer::setSource(std::shared_ptr<IAudioSource> source)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_source = source;
}

void AudioBuffer::forward()
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    fillup();
}

void AudioBuffer::pop(float* dest, unsigned int sampleCount)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    //catch up if we are fall behind
    if (sampleCount > sampleLag()) {
        //! TODO We have to decide to wait or skip.
        //! We cannot make a direct call, this is a thread-unsafe.
        //fillup();
    }

    unsigned int from = m_readIndex;
    auto memStep = sizeof(float);
    auto to = m_readIndex + sampleCount * m_audioChannelsCount;
    if (to > m_data.size()) {
        to = m_data.size();
    }
    auto count = to - from;
    std::memcpy(dest, m_data.data() + from, count * memStep);
    m_readIndex += count;

    int left = sampleCount * m_audioChannelsCount - count;
    if (left > 0) {
        std::memcpy(dest + count, m_data.data(), left * memStep);
        m_readIndex = left;
    }

    if (m_readIndex >= m_data.size()) {
        m_readIndex -= m_data.size();
    }
}

void AudioBuffer::setMinSampleLag(unsigned int lag)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

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
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    unsigned int from = m_writeIndex;

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
    unsigned int lag = 0;
    if (m_readIndex <= m_writeIndex) {
        lag = m_writeIndex - m_readIndex;
    } else {
        lag = m_writeIndex + m_data.size() - m_readIndex;
    }

    return lag / m_audioChannelsCount;
}
