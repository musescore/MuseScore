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
#include "mixer.h"
#include "log.h"

using namespace mu::audio;

Mixer::Mixer()
{
    buildRpcReflection();
}

Mixer::~Mixer()
{
}

Mixer::Mode Mixer::mode() const
{
    return m_mode;
}

void Mixer::setMode(const Mixer::Mode& mode)
{
    m_mode = mode;
}

void Mixer::setClock(std::shared_ptr<Clock> clock)
{
    m_clock = clock;
}

void Mixer::setSampleRate(unsigned int sampleRate)
{
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
    switch (m_mode) {
    case MONO: return 1;
    case STEREO: return 2;
    }

    LOGE() << "mixer mode's size is not defined";
    return 0;
}

void Mixer::setLevel(float level)
{
    m_masterLevel = level;
}

std::shared_ptr<IAudioInsert> Mixer::insert(unsigned int number) const
{
    IF_ASSERT_FAILED(m_insertList.find(number) != m_insertList.end()) {
        return nullptr;
    }
    return m_insertList.at(number);
}

void Mixer::setInsert(unsigned int number, std::shared_ptr<IAudioInsert> insert)
{
    IF_ASSERT_FAILED(insert->streamCount() == streamCount()) {
        LOGE() << "Insert's stream count not equal to the channel";
        return;
    }
    m_insertList[number] = insert;
}

unsigned int Mixer::addChannel(std::shared_ptr<IAudioSource> source)
{
    auto guard = std::lock_guard(m_process);
    auto lastId = (m_inputList.size() > 0 ? m_inputList.rbegin()->first : -1);
    auto newId = lastId + 1;

    auto channel = std::make_shared<MixerChannel>();
    channel->setSource(source);
    channel->setBufferSize(m_buffer.size());
    channel->setSampleRate(m_sampleRate);

    m_inputList[newId] = channel;
    return newId;
}

void Mixer::removeChannel(unsigned int channelId)
{
    auto guard = std::lock_guard(m_process);
    m_inputList.erase(channelId);
}

void Mixer::setActive(unsigned int channelId, bool active)
{
    auto guard = std::lock_guard(m_process);
    m_inputList[channelId]->setActive(active);
}

void Mixer::setLevel(unsigned int channelId, unsigned int streamId, float level)
{
    auto guard = std::lock_guard(m_process);
    m_inputList[channelId]->setLevel(streamId, level);
}

void Mixer::setBalance(unsigned int channelId, unsigned int streamId, std::complex<float> balance)
{
    auto guard = std::lock_guard(m_process);
    m_inputList[channelId]->setBalance(streamId, balance);
}

std::shared_ptr<IMixerChannel> Mixer::channel(unsigned int number) const
{
    IF_ASSERT_FAILED(m_inputList.find(number) != m_inputList.end()) {
        return nullptr;
    }
    return m_inputList.at(number);
}

void Mixer::setBufferSize(unsigned int samples)
{
    AbstractAudioSource::setBufferSize(samples);
    for (auto& input : m_inputList) {
        input.second->setBufferSize(samples);
    }
}

void Mixer::forward(unsigned int sampleCount)
{
    auto guard = std::lock_guard(m_process);
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

void Mixer::buildRpcReflection()
{
    using namespace rpc;
    m_rpcReflection = {
        {
            "setActive",  [this](ArgumentList args) -> Variable {
                IF_ASSERT_FAILED(args.size() == 2) {
                    return {};
                }
                setActive(args[0].toUInt(), args[1].toBool());
                return {};
            }
        },
        {
            "setLevel",   [this](ArgumentList args) -> Variable {
                IF_ASSERT_FAILED(args.size() == 3) {
                    return {};
                }
                setLevel(args[0].toUInt(), args[1].toUInt(), args[2].toFloat());
                return {};
            }
        },
        {
            "setBalance", [this](ArgumentList args) -> Variable {
                IF_ASSERT_FAILED(args.size() == 3) {
                    return {};
                }
                setBalance(args[0].toUInt(), args[1].toUInt(), args[2].toFloat());
                return {};
            }
        }
    };
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
