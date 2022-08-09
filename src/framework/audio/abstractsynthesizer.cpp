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

#include "abstractsynthesizer.h"

#include "internal/audiosanitizer.h"

using namespace mu;
using namespace mu::mpe;
using namespace mu::audio;
using namespace mu::audio::synth;

AbstractSynthesizer::AbstractSynthesizer(const AudioInputParams& params)
    : m_params(params)
{
    ONLY_AUDIO_WORKER_THREAD;
}

AbstractSynthesizer::~AbstractSynthesizer()
{
    m_mainStreamChanges.resetOnReceive(this);
    m_offStreamChanges.resetOnReceive(this);
    m_dynamicLevelChanges.resetOnReceive(this);
}

const AudioInputParams& AbstractSynthesizer::params() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_params;
}

async::Channel<AudioInputParams> AbstractSynthesizer::paramsChanged() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_paramsChanges;
}

void AbstractSynthesizer::setup(const mpe::PlaybackData& playbackData)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_setupData = playbackData.setupData;

    setupSound(playbackData.setupData);
    setupEvents(playbackData);
}

void AbstractSynthesizer::setupEvents(const mpe::PlaybackData& playbackData)
{
    ONLY_AUDIO_WORKER_THREAD;

    loadMainStreamEvents(playbackData.originEvents);
    m_mainStreamChanges = playbackData.mainStream;
    m_offStreamChanges = playbackData.offStream;

    loadDynamicLevelChanges(playbackData.dynamicLevelMap);
    m_dynamicLevelChanges = playbackData.dynamicLevelChanges;

    m_mainStreamChanges.onReceive(this, [this](const PlaybackEventsMap& updatedEvents) {
        loadMainStreamEvents(updatedEvents);
    });

    m_offStreamChanges.onReceive(this, [this](const PlaybackEventsMap& triggeredEvents) {
        loadOffStreamEvents(triggeredEvents);
    });

    m_dynamicLevelChanges.onReceive(this, [this](const DynamicLevelMap& dynamicLevelMap) {
        loadDynamicLevelChanges(dynamicLevelMap);
    });
}

void AbstractSynthesizer::loadMainStreamEvents(const mpe::PlaybackEventsMap& updatedEvents)
{
    m_mainStreamEvents.clear();
    m_mainStreamEvents.load(updatedEvents);
}

void AbstractSynthesizer::loadOffStreamEvents(const mpe::PlaybackEventsMap& updatedEvents)
{
    m_offStreamEvents.clear();
    m_offStreamEvents.load(updatedEvents);
}

void AbstractSynthesizer::loadDynamicLevelChanges(const mpe::DynamicLevelMap& updatedDynamicLevelMap)
{
    m_dynamicLevelMap = updatedDynamicLevelMap;
}

void AbstractSynthesizer::revokePlayingNotes()
{
    ONLY_AUDIO_WORKER_THREAD;
}

bool AbstractSynthesizer::isActive() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_isActive;
}

void AbstractSynthesizer::setIsActive(bool arg)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_isActive = arg;
}

audio::msecs_t AbstractSynthesizer::samplesToMsecs(const samples_t samplesPerChannel, const samples_t sampleRate) const
{
    ONLY_AUDIO_WORKER_THREAD;

    return samplesPerChannel * 1000000 / sampleRate;
}

msecs_t AbstractSynthesizer::actualPlaybackPositionStart() const
{
    //!Note Some events might be started RIGHT before the "official" start of the track
    //!     Need to make sure that we don't miss those events
    return std::min(m_playbackPosition, m_mainStreamEvents.from);
}

audio::msecs_t mu::audio::synth::AbstractSynthesizer::playbackPosition() const
{
    ONLY_AUDIO_WORKER_THREAD;

    return m_playbackPosition;
}

void AbstractSynthesizer::setPlaybackPosition(const msecs_t newPosition)
{
    ONLY_AUDIO_WORKER_THREAD;

    m_playbackPosition = newPosition;
}
