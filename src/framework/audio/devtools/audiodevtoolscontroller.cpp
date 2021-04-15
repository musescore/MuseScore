/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#include "audiodevtoolscontroller.h"

#include "log.h"
#include "internal/audiosanitizer.h"

#include "internal/worker/sinesource.h"
#include "internal/worker/noisesource.h"
#include "internal/worker/equaliser.h"

using namespace mu::audio;
using namespace mu::audio::rpc;

AudioDevToolsController* AudioDevToolsController::instance()
{
    ONLY_AUDIO_WORKER_THREAD;
    static AudioDevToolsController c;
    return &c;
}

AudioEngine* AudioDevToolsController::audioEngine() const
{
    return AudioEngine::instance();
}

void AudioDevToolsController::handleRpcMsg(const rpc::Msg& msg)
{
    ONLY_AUDIO_WORKER_THREAD;
    IF_ASSERT_FAILED(msg.target.name == TargetName::DevTools) {
        return;
    }

    static Calls calls;
    if (calls.empty()) {
        // Sine

        bindMethod(calls, "playSine", [this](const Args&) {
            auto source = std::make_shared<SineSource>();
            if (!m_sineChannelId) {
                m_sineChannelId = audioEngine()->mixer()->addChannel(source);
            }
        });

        bindMethod(calls, "stopSine", [this](const Args&) {
            audioEngine()->mixer()->removeChannel(m_sineChannelId.value_or(-1));
            m_sineChannelId.reset();
        });

        bindMethod(calls, "setMuteSine", [this](const Args& args) {
            if (m_sineChannelId) {
                auto channel = audioEngine()->mixer()->channel(m_sineChannelId.value_or(-1));
                channel->setActive(!args.arg<bool>(0));
            }
        });

        bindMethod(calls, "setLevelSine", [this](const Args& args) {
            if (m_sineChannelId) {
                auto channel = audioEngine()->mixer()->channel(m_sineChannelId.value_or(-1));
                if (channel) {
                    channel->setLevel(args.arg<float>(0));
                }
            }
        });

        bindMethod(calls, "setBalanceSine", [this](const Args& args) {
            if (m_sineChannelId) {
                auto channel = audioEngine()->mixer()->channel(m_sineChannelId.value_or(-1));
                if (channel) {
                    channel->setBalance(args.arg<float>(0));
                }
            }
        });

        // Noise

        bindMethod(calls, "playNoise", [this](const Args&) {
            auto source = std::make_shared<NoiseSource>();
            source->setType(NoiseSource::Type::PINK);

            auto eq = std::make_shared<Equaliser>();
            eq->setFrequency(500);
            eq->setQ(3);
            eq->setGain(-12);
            eq->setActive(false);
            if (!m_noiseChannel) {
                m_noiseChannel = audioEngine()->mixer()->addChannel(source);
                auto channel = audioEngine()->mixer()->channel(m_noiseChannel.value_or(-1));
                channel->setProcessor(0, eq);
            }
        });

        bindMethod(calls, "stopNoise", [this](const Args&) {
            audioEngine()->mixer()->removeChannel(m_noiseChannel.value_or(-1));
            m_noiseChannel.reset();
        });

        bindMethod(calls, "setMuteNoise", [this](const Args& args) {
            if (m_noiseChannel) {
                auto channel = audioEngine()->mixer()->channel(m_noiseChannel.value_or(-1));
                if (channel) {
                    channel->setActive(!args.arg<bool>(0));
                }
            }
        });

        bindMethod(calls, "setLevelNoise", [this](const Args& args) {
            if (m_noiseChannel) {
                auto channel = audioEngine()->mixer()->channel(m_noiseChannel.value_or(-1));
                if (channel) {
                    channel->setLevel(args.arg<float>(0));
                }
            }
        });

        bindMethod(calls, "setBalanceNoise", [this](const Args& args) {
            if (m_noiseChannel) {
                auto channel = audioEngine()->mixer()->channel(m_noiseChannel.value_or(-1));
                if (channel) {
                    channel->setBalance(args.arg<float>(0));
                }
            }
        });

        bindMethod(calls, "enableNoiseEq", [this](const Args& args) {
            if (m_noiseChannel) {
                auto channel = audioEngine()->mixer()->channel(m_noiseChannel.value_or(-1));
                if (channel) {
                    channel->processor(0)->setActive(args.arg<bool>(0));
                }
            }
        });
    }

    doCall(calls, msg);
}

void AudioDevToolsController::bindMethod(Calls& calls, const Method& method, const Call& call)
{
    calls.insert({ method, call });
}

void AudioDevToolsController::doCall(const Calls& calls, const Msg& msg)
{
    auto it = calls.find(msg.method);
    if (it == calls.end()) {
        LOGE() << "not found method: " << msg.method;
        return;
    }

    it->second(msg.args);
}
