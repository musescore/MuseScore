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

#include "vstfxprocessor.h"

using namespace mu;
using namespace mu::vst;
using namespace mu::audio;

VstFxProcessor::VstFxProcessor(VstPluginPtr&& pluginPtr, const AudioFxParams& params)
    : m_pluginPtr(pluginPtr),
    m_vstAudioClient(std::make_unique<VstAudioClient>()),
    m_params(params)
{
}

void VstFxProcessor::init()
{
    m_vstAudioClient->init(AudioPluginType::Fx, m_pluginPtr);

    if (m_pluginPtr->isLoaded()) {
        m_pluginPtr->updatePluginConfig(m_params.configuration);
        m_inited = true;
    } else {
        m_pluginPtr->loadingCompleted().onNotify(this, [this]() {
            m_pluginPtr->updatePluginConfig(m_params.configuration);
            m_inited = true;
        });
    }

    m_pluginPtr->pluginSettingsChanged().onReceive(this, [this](const audio::AudioUnitConfig& newConfig) {
        if (m_params.configuration == newConfig) {
            return;
        }

        m_params.configuration = newConfig;
        m_paramsChanges.send(m_params);
    });
}

AudioFxType VstFxProcessor::type() const
{
    return audio::AudioFxType::VstFx;
}

const AudioFxParams& VstFxProcessor::params() const
{
    return m_params;
}

async::Channel<AudioFxParams> VstFxProcessor::paramsChanged() const
{
    return m_paramsChanges;
}

void VstFxProcessor::setSampleRate(unsigned int sampleRate)
{
    m_vstAudioClient->setSampleRate(sampleRate);
}

bool VstFxProcessor::active() const
{
    return m_params.active;
}

void VstFxProcessor::setActive(bool active)
{
    m_params.active = active;
}

void VstFxProcessor::process(float* buffer, size_t bufferSize, unsigned int sampleCount)
{
    if (!buffer || !m_inited) {
        return;
    }

    m_vstAudioClient->setBlockSize(sampleCount);
    m_vstAudioClient->process(buffer, bufferSize, sampleCount);
}
