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
using namespace mu::audio::synth;
using namespace mu::audio;

static const std::set<Steinberg::Vst::CtrlNumber> SUPPORTED_CONTROLLERS = {
    Steinberg::Vst::kCtrlVolume,
    Steinberg::Vst::kCtrlExpression,
    Steinberg::Vst::kCtrlSustainOnOff,
    Steinberg::Vst::kPitchBend,
};

VstSynthesiser::VstSynthesiser(const TrackId trackId, const audio::AudioInputParams& params)
    : AbstractSynthesizer(params),
    m_pluginPtr(std::make_shared<VstPlugin>(params.resourceMeta.id)),
    m_vstAudioClient(std::make_unique<VstAudioClient>()),
    m_trackId(trackId)
{
}

VstSynthesiser::~VstSynthesiser()
{
    pluginsRegister()->unregisterInstrPlugin(m_trackId, m_params.resourceMeta.id);
}

void VstSynthesiser::init()
{
    pluginsRegister()->registerInstrPlugin(m_trackId, m_pluginPtr);
    m_pluginPtr->load();

    m_samplesPerChannel = config()->driverBufferSize();

    m_vstAudioClient->init(AudioPluginType::Instrument, m_pluginPtr);

    auto onPluginLoaded = [this]() {
        m_pluginPtr->updatePluginConfig(m_params.configuration);
        m_vstAudioClient->setBlockSize(m_samplesPerChannel);
        m_sequencer.init(m_vstAudioClient->paramsMapping(SUPPORTED_CONTROLLERS));
    };

    if (m_pluginPtr->isLoaded()) {
        onPluginLoaded();
    } else {
        m_pluginPtr->loadingCompleted().onNotify(this, onPluginLoaded);
    }

    m_pluginPtr->pluginSettingsChanged().onReceive(this, [this](const audio::AudioUnitConfig& newConfig) {
        if (m_params.configuration == newConfig) {
            return;
        }

        m_params.configuration = newConfig;
        m_paramsChanges.send(m_params);
    });

    m_sequencer.setOnOffStreamFlushed([this]() {
        revokePlayingNotes();
    });
}

void VstSynthesiser::toggleVolumeGain(const bool isActive)
{
    static constexpr audio::gain_t NON_ACTIVE_GAIN = 0.5f;

    if (isActive) {
        m_vstAudioClient->setVolumeGain(m_sequencer.currentGain());
    } else {
        m_vstAudioClient->setVolumeGain(NON_ACTIVE_GAIN);
    }
}

bool VstSynthesiser::isValid() const
{
    if (!m_pluginPtr) {
        return false;
    }

    return m_pluginPtr->isValid();
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

void VstSynthesiser::revokePlayingNotes()
{
    if (m_vstAudioClient) {
        m_vstAudioClient->flush();
    }
}

void VstSynthesiser::flushSound()
{
    revokePlayingNotes();
}

void VstSynthesiser::setupSound(const mpe::PlaybackSetupData& /*setupData*/)
{
    NOT_SUPPORTED;
    return;
}

void VstSynthesiser::setupEvents(const mpe::PlaybackData& playbackData)
{
    m_sequencer.load(playbackData);
}

bool VstSynthesiser::isActive() const
{
    return m_sequencer.isActive();
}

void VstSynthesiser::setIsActive(const bool isActive)
{
    m_sequencer.setActive(isActive);
    toggleVolumeGain(isActive);
}

audio::msecs_t VstSynthesiser::playbackPosition() const
{
    return m_sequencer.playbackPosition();
}

void VstSynthesiser::setPlaybackPosition(const audio::msecs_t newPosition)
{
    m_sequencer.setPlaybackPosition(newPosition);

    if (isActive()) {
        m_vstAudioClient->setVolumeGain(m_sequencer.currentGain());
    }
}

void VstSynthesiser::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;
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

audio::samples_t VstSynthesiser::process(float* buffer, audio::samples_t samplesPerChannel)
{
    if (!buffer) {
        return 0;
    }

    audio::msecs_t nextMsecs = samplesToMsecs(samplesPerChannel, m_sampleRate);
    VstSequencer::EventSequence sequence = m_sequencer.eventsToBePlayed(nextMsecs);

    for (const VstSequencer::EventType& event : sequence) {
        if (std::holds_alternative<VstEvent>(event)) {
            m_vstAudioClient->handleEvent(std::get<VstEvent>(event));
        } else if (std::holds_alternative<PluginParamInfo>(event)) {
            m_vstAudioClient->handleParamChange(std::get<PluginParamInfo>(event));
        } else {
            audio::gain_t newGain = std::get<audio::gain_t>(event);
            m_vstAudioClient->setVolumeGain(newGain);
        }
    }

    return m_vstAudioClient->process(buffer, samplesPerChannel);
}
