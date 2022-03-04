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

VstSynthesiser::VstSynthesiser(VstPluginPtr&& pluginPtr, const audio::AudioInputParams& params)
    : AbstractSynthesizer(params), m_pluginPtr(pluginPtr), m_vstAudioClient(std::make_unique<VstAudioClient>())
{
    init();
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
    for (const mpe::PlaybackEvent& event : m_playingEvents) {
        if (!std::holds_alternative<mpe::NoteEvent>(event)) {
            continue;
        }

        const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);
        mpe::timestamp_t from = noteEvent.arrangementCtx().actualTimestamp;
        mpe::timestamp_t to = from + noteEvent.arrangementCtx().actualDuration;

        m_vstAudioClient->handleNoteOffEvents(event, from, to);
    }

    m_playingEvents.clear();
    m_vstAudioClient->flush();
}

void VstSynthesiser::flushSound()
{
    revokePlayingNotes();
}

bool VstSynthesiser::hasAnythingToPlayback(const audio::msecs_t from, const audio::msecs_t to) const
{
    if (!m_offStreamEvents.empty() || !m_playingEvents.empty()) {
        return true;
    }

    if (!m_isActive || m_mainStreamEvents.empty()) {
        return false;
    }

    audio::msecs_t startMsec = m_mainStreamEvents.from;
    audio::msecs_t endMsec = m_mainStreamEvents.to;

    return from >= startMsec && to <= endMsec;
}

void VstSynthesiser::setupSound(const mpe::PlaybackSetupData& /*setupData*/)
{
    NOT_SUPPORTED;
    return;
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

    if (hasAnythingToPlayback(m_playbackPosition, m_playbackPosition + nextMsecs)) {
        if (isActive()) {
            handleMainStreamEvents(nextMsecs);
        } else {
            handleOffStreamEvents(nextMsecs);
        }
    }

    m_vstAudioClient->setBlockSize(samplesPerChannel);

    return m_vstAudioClient->process(buffer, samplesPerChannel);
}

void VstSynthesiser::handleMainStreamEvents(const audio::msecs_t nextMsecs)
{
    audio::msecs_t from = m_playbackPosition;
    audio::msecs_t to = from + nextMsecs;

    EventsMapIteratorList range = m_mainStreamEvents.findEventsRange(from, to);

    for (const auto& it : range) {
        for (const mpe::PlaybackEvent& event : it->second) {
            if (m_vstAudioClient->handleNoteOnEvents(event, from, from + nextMsecs)) {
                m_playingEvents.emplace_back(event);
            }
        }
    }

    handleAlreadyPlayingEvents(from, from + nextMsecs);

    setPlaybackPosition(to);
}

void VstSynthesiser::handleOffStreamEvents(const audio::msecs_t nextMsecs)
{
    audio::msecs_t from = m_offStreamEvents.from;
    audio::msecs_t to = m_offStreamEvents.to;

    EventsMapIteratorList range = m_offStreamEvents.findEventsRange(from, to);

    for (const auto& it : range) {
        for (const mpe::PlaybackEvent& event : it->second) {
            if (m_vstAudioClient->handleNoteOnEvents(event, from, from + nextMsecs)) {
                m_playingEvents.emplace_back(event);
            }
        }
    }

    handleAlreadyPlayingEvents(from, from + nextMsecs);

    m_offStreamEvents.from += nextMsecs;
    if (m_offStreamEvents.from >= m_offStreamEvents.to) {
        m_offStreamEvents.clear();
    }
}

void VstSynthesiser::handleAlreadyPlayingEvents(const audio::msecs_t from, const audio::msecs_t to)
{
    auto it = m_playingEvents.cbegin();
    while (it != m_playingEvents.cend()) {
        if (m_vstAudioClient->handleNoteOffEvents(*it, from, to)) {
            it = m_playingEvents.erase(it);
        } else {
            ++it;
        }
    }
}
