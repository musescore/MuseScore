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
#include "vstsynthesiser.h"

#include "log.h"

#include "internal/vstplugin.h"

using namespace mu;
using namespace mu::vst;

VstSynthesiser::VstSynthesiser(VstPluginPtr&& pluginPtr, const audio::AudioInputParams& params)
    : m_pluginPtr(pluginPtr), m_vstAudioClient(std::make_unique<VstAudioClient>()), m_params(params)
{
}

Ret VstSynthesiser::init()
{
    m_vstAudioClient->init(VstPluginType::Instrument, m_pluginPtr);

    if (m_pluginPtr->isLoaded()) {
        m_pluginPtr->updatePluginConfig(m_params.configuration);
    } else {
        m_pluginPtr->loadingCompleted().onNotify(this, [this]() {
            m_pluginPtr->updatePluginConfig(m_params.configuration);
        });
    }

    m_pluginPtr->pluginSettingsChanged().onReceive(this, [this](const audio::AudioUnitConfig& newConfig) {
        if (m_params.configuration == newConfig) {
            return;
        }

        m_params.configuration = newConfig;
        m_paramsChanges.send(m_params);
    });

    return make_ret(Ret::Code::Ok);
}

bool VstSynthesiser::isValid() const
{
    if (!m_pluginPtr) {
        return false;
    }

    return m_pluginPtr->isValid();
}

bool VstSynthesiser::isActive() const
{
    return m_isActive;
}

void VstSynthesiser::setIsActive(bool arg)
{
    m_isActive = arg;
}

audio::AudioSourceType VstSynthesiser::type() const
{
    return m_params.type();
}

std::string VstSynthesiser::name() const
{
    if (!m_pluginPtr) {
        return std::string();
    }

    return m_pluginPtr->name();
}

const audio::AudioInputParams& VstSynthesiser::params() const
{
    return m_params;
}

async::Channel<audio::AudioInputParams> VstSynthesiser::paramsChanged() const
{
    return m_paramsChanges;
}

bool VstSynthesiser::handleEvent(const midi::Event& e)
{
    if (!m_vstAudioClient) {
        return false;
    }

    return m_vstAudioClient->handleEvent(e);
}

void VstSynthesiser::flushSound()
{
    m_vstAudioClient->flush();
}

Ret VstSynthesiser::setupSound(const std::vector<midi::Event>& /*events*/)
{
    NOT_IMPLEMENTED;
    return Ret(Ret::Code::Ok);
}

void VstSynthesiser::setSampleRate(unsigned int sampleRate)
{
    m_vstAudioClient->setSampleRate(sampleRate);
}

unsigned int VstSynthesiser::audioChannelsCount() const
{
    return config()->audioChannelsCount();
}

async::Channel<unsigned int> VstSynthesiser::audioChannelsCountChanged() const
{
    return m_streamsCountChanged;
}

audio::samples_t VstSynthesiser::process(float* buffer, audio::samples_t samplelPerChannel)
{
    if (!buffer) {
        return 0;
    }

    m_vstAudioClient->setBlockSize(samplelPerChannel);

    return m_vstAudioClient->process(buffer, samplelPerChannel);
}
