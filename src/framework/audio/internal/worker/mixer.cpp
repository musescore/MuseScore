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
#include "mixer.h"
#include "log.h"
#include "internal/audiosanitizer.h"

using namespace mu::audio;

Mixer::Mixer()
{
    ONLY_AUDIO_WORKER_THREAD;
}

Mixer::~Mixer()
{
    ONLY_AUDIO_WORKER_THREAD;
}

IAudioSourcePtr Mixer::mixedSource()
{
    ONLY_AUDIO_WORKER_THREAD;
    return shared_from_this();
}

Mixer::Mode Mixer::mode() const
{
    ONLY_AUDIO_WORKER_THREAD;
    return m_mode;
}

void Mixer::setMode(const Mixer::Mode& mode)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_mode = mode;
}

void Mixer::setClock(std::shared_ptr<Clock> clock)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_clock = clock;
}

void Mixer::setSampleRate(unsigned int sampleRate)
{
    ONLY_AUDIO_WORKER_THREAD;
    AbstractAudioSource::setSampleRate(sampleRate);
    for (auto& input : m_inputList) {
        input.second->setSampleRate(sampleRate);
    }
    if (m_clock) {
        m_clock->setSampleRate(sampleRate);
    }
}

unsigned int Mixer::streamCount() const
{
    ONLY_AUDIO_WORKER_THREAD;
    switch (m_mode) {
    case MONO: return 1;
    case STEREO: return 2;
    }

    LOGE() << "mixer mode's size is not defined";
    return 0;
}

void Mixer::setLevel(float level)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_masterLevel = level;
}

std::shared_ptr<IAudioProcessor> Mixer::processor(unsigned int number) const
{
    ONLY_AUDIO_WORKER_THREAD;
    IF_ASSERT_FAILED(m_insertList.find(number) != m_insertList.end()) {
        return nullptr;
    }
    return m_insertList.at(number);
}

void Mixer::setProcessor(unsigned int number, std::shared_ptr<IAudioProcessor> insert)
{
    ONLY_AUDIO_WORKER_THREAD;
    IF_ASSERT_FAILED(insert->streamCount() == streamCount()) {
        LOGE() << "Insert's stream count not equal to the channel";
        return;
    }
    m_insertList[number] = insert;
}

IMixer::ChannelID Mixer::addChannel(std::shared_ptr<IAudioSource> source)
{
    ONLY_AUDIO_WORKER_THREAD;
    auto lastId = (m_inputList.size() > 0 ? m_inputList.rbegin()->first : -1);
    auto newId = lastId + 1;

    auto channel = std::make_shared<MixerChannel>();
    channel->setSource(source);
    channel->setBufferSize(m_buffer.size());
    channel->setSampleRate(m_sampleRate);

    m_inputList[newId] = channel;
    return newId;
}

void Mixer::removeChannel(ChannelID channelId)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_inputList.erase(channelId);
}

void Mixer::setActive(ChannelID channelId, bool active)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_inputList[channelId]->setActive(active);
}

void Mixer::setLevel(ChannelID channelId, unsigned int streamId, float level)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_inputList[channelId]->setLevel(streamId, level);
}

void Mixer::setBalance(ChannelID channelId, unsigned int streamId, std::complex<float> balance)
{
    ONLY_AUDIO_WORKER_THREAD;
    m_inputList[channelId]->setBalance(streamId, balance);
}

std::shared_ptr<IMixerChannel> Mixer::channel(unsigned int number) const
{
    ONLY_AUDIO_WORKER_THREAD;
    IF_ASSERT_FAILED(m_inputList.find(number) != m_inputList.end()) {
        return nullptr;
    }
    return m_inputList.at(number);
}

void Mixer::setBufferSize(unsigned int samples)
{
    ONLY_AUDIO_WORKER_THREAD;
    AbstractAudioSource::setBufferSize(samples);
    for (auto& input : m_inputList) {
        input.second->setBufferSize(samples);
    }
}

void Mixer::forward(unsigned int sampleCount)
{
    ONLY_AUDIO_WORKER_THREAD;
    std::fill(m_buffer.begin(), m_buffer.end(), 0.f);

    if (m_clock) {
        m_clock->forward(sampleCount);
    }

    for (auto& input : m_inputList) {
        input.second->forward(sampleCount);
        mixinChannel(input.second, sampleCount);
    }

    for (auto& insert : m_insertList) {
        if (insert.second->active()) {
            insert.second->process(m_buffer.data(), m_buffer.data(), sampleCount);
        }
    }
    std::transform(m_buffer.begin(), m_buffer.end(), m_buffer.begin(),
                   [this](float sample) -> float { return sample * m_masterLevel; });
}

void Mixer::mixinChannel(std::shared_ptr<MixerChannel> channel, unsigned int samplesCount)
{
    if (!channel->active()) {
        return;
    }
    channel->checkStreams();
    switch (m_mode) {
    case MONO:
    case STEREO:
        for (unsigned int i = 0; i < channel->streamCount(); ++i) {
            mixinChannelStream(channel, i, samplesCount);
        }
        break;
    }
}

void Mixer::mixinChannelStream(std::shared_ptr<MixerChannel> channel, unsigned int streamId, unsigned int samplesCount)
{
    auto balance = channel->balance(streamId).real();
    auto level = channel->level(streamId);
    for (unsigned int i = 0; i < samplesCount; ++i) {
        for (unsigned int j = 0; j < streamCount(); ++j) {
            //linear cross
            float gain = 0.5f * balance * ((j * 2.f) - 1) + 0.5f;
            if (gain < 0) {
                gain = 0;
            }
            if (gain > 1) {
                gain = 1;
            }

            auto channelBuffer = channel->data();
            if (channelBuffer) {
                m_buffer[i * streamCount() + j] += gain * level * channelBuffer[i * channel->streamCount() + streamId];
            }
        }
    }
}
