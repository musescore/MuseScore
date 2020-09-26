//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "audiobuffer.h"
#include <cstring>
#include "log.h"

using namespace mu::audio;

AudioBuffer::AudioBuffer(unsigned int streamsPerSample, unsigned int size)
    : m_streamsPerSample(streamsPerSample)
{
    m_data.resize(size * m_streamsPerSample, 0.f);
}

void AudioBuffer::setSource(std::shared_ptr<IAudioSource> source)
{
    m_source = source;
}

void AudioBuffer::fillup()
{
    IF_ASSERT_FAILED(m_source) {
        return;
    }
    while (sampleLag() < m_minSampleLag + FILL_OVER) {
        m_source->setBufferSize(FILL_SAMPLES);
        m_source->forward(FILL_SAMPLES);
        push(m_source->data(), FILL_SAMPLES);
    }
}

void AudioBuffer::forward()
{
    std::lock_guard<std::mutex> guard(m_dataMutex);
    fillup();
}

void AudioBuffer::push(const float* source, int sampleCount)
{
    unsigned int from = m_writeIndex;
    auto memStep = sizeof(float);
    auto to = m_writeIndex + sampleCount * m_streamsPerSample;
    if (to > m_data.size()) {
        to = m_data.size() - 1;
    }
    auto count = to - from;
    std::memcpy(m_data.data() + m_writeIndex, source, count * memStep);
    m_writeIndex += count;

    int left = sampleCount * m_streamsPerSample - count;
    if (left > 0) {
        std::memcpy(m_data.data(), source + count, left * memStep);
        m_writeIndex = left;
    }

    if (m_writeIndex >= m_data.size()) {
        m_writeIndex -= m_data.size();
    }
}

void AudioBuffer::pop(float* dest, unsigned int sampleCount)
{
    std::lock_guard<std::mutex> guard(m_dataMutex);

    //catch up if we are fall behind
    if (sampleCount > sampleLag()) {
        fillup();
    }

    unsigned int from = m_readIndex;
    auto memStep = sizeof(float);
    auto to = m_readIndex + sampleCount * m_streamsPerSample;
    if (to > m_data.size()) {
        to = m_data.size();
    }
    auto count = to - from;
    std::memcpy(dest, m_data.data() + from, count * memStep);
    m_readIndex += count;

    int left = sampleCount * m_streamsPerSample - count;
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
    IF_ASSERT_FAILED(lag < m_data.size()) {
        lag = m_data.size();
    }
    m_minSampleLag = lag;
    if (m_data.size() < 2 * m_minSampleLag * m_streamsPerSample) {
        m_data.resize(2 * m_minSampleLag * m_streamsPerSample, 0.f);
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

    return lag / m_streamsPerSample;
}
