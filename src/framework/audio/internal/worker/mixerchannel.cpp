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
#include "mixerchannel.h"
#include <algorithm>
#include <cstring>
#include "log.h"

using namespace mu::audio;

MixerChannel::MixerChannel()
{
}

unsigned int MixerChannel::streamCount() const
{
    IF_ASSERT_FAILED(m_source) {
        return 0;
    }
    return m_source->streamCount();
}

void MixerChannel::checkStreams()
{
    if (streamCount() != m_level.size()) {
        updateBalanceLevelMaps();
    }
}

void MixerChannel::forward(unsigned int sampleCount)
{
    IF_ASSERT_FAILED(m_source) {
        return;
    }
    m_source->forward(sampleCount);

    //you can use source's buffer as pre proccesing KEY, current buffer as post processing KEY
    std::memcpy(m_buffer.data(), m_source->data(), m_buffer.size() * sizeof(float));

    for (auto& insert : m_insertList) {
        if (insert.second->active()) {
            insert.second->process(m_buffer.data(), m_buffer.data(), sampleCount);
        }
    }
}

void MixerChannel::setBufferSize(unsigned int samples)
{
    AbstractAudioSource::setBufferSize(samples);
    IF_ASSERT_FAILED(m_source) {
        return;
    }
    m_source->setBufferSize(samples);
}

void MixerChannel::setSampleRate(unsigned int sampleRate)
{
    AbstractAudioSource::setSampleRate(sampleRate);
    IF_ASSERT_FAILED(m_source) {
        return;
    }
    m_source->setSampleRate(sampleRate);
    for (auto& insert : m_insertList) {
        insert.second->setSampleRate(sampleRate);
    }
}

void MixerChannel::setSource(std::shared_ptr<IAudioSource> source)
{
    m_source = source;
    updateBalanceLevelMaps();
    m_source->streamsCountChanged().onReceive(this, [this](unsigned int) {
        updateBalanceLevelMaps();
    });
    setActive(true);
}

bool MixerChannel::active() const
{
    return m_active;
}

void MixerChannel::setActive(bool active)
{
    m_active = active;
}

float MixerChannel::level(unsigned int streamId) const
{
    IF_ASSERT_FAILED(m_level.find(streamId) != m_level.end()) {
        return 0.f;
    }
    return m_level.at(streamId);
}

void MixerChannel::setLevel(float level)
{
    for (auto& stream : m_level) {
        m_level[stream.first] = level;
    }
}

void MixerChannel::setLevel(unsigned int streamId, float level)
{
    m_level[streamId] = level;
}

std::complex<float> MixerChannel::balance(unsigned int streamId) const
{
    IF_ASSERT_FAILED(m_balance.find(streamId) != m_balance.end()) {
        return 0.f;
    }
    return m_balance.at(streamId);
}

void MixerChannel::setBalance(std::complex<float> value)
{
    for (auto& stream : m_balance) {
        m_balance[stream.first] = value;
    }
}

void MixerChannel::setBalance(unsigned int streamId, std::complex<float> value)
{
    m_balance[streamId] = value;
}

std::shared_ptr<IAudioInsert> MixerChannel::insert(unsigned int number) const
{
    IF_ASSERT_FAILED(m_insertList.find(number) != m_insertList.end()) {
        return nullptr;
    }
    return m_insertList.at(number);
}

void MixerChannel::setInsert(unsigned int number, std::shared_ptr<IAudioInsert> insert)
{
    IF_ASSERT_FAILED(insert->streamCount() == streamCount()) {
        LOGE() << "Insert's stream count not equal to the channel";
        return;
    }
    insert->setSampleRate(m_sampleRate);
    m_insertList[number] = insert;
}

void MixerChannel::updateBalanceLevelMaps()
{
    for (unsigned int c = 0; c < m_source->streamCount(); ++c) {
        m_level[c] = 1.f;

        if (m_source->streamCount() > 1) {
            m_balance[c] = 2.f * c / (m_source->streamCount() - 1) - 1.f;
        } else {
            m_balance[c] = 0.f;
        }
    }
}
