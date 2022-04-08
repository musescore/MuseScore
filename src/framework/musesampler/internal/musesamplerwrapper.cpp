/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "musesamplerwrapper.h"

#include "realfn.h"

using namespace mu;
using namespace mu::audio;
using namespace mu::musesampler;

static constexpr int AUDIO_CHANNELS_COUNT = 2;

static const std::unordered_map<mpe::ArticulationType, ms_NoteArticulation> ARTICULATION_TYPES = {
    { mpe::ArticulationType::Standard, ms_NoteArticulation_None },
    { mpe::ArticulationType::Staccato, ms_NoteArticulation_Staccato },
    { mpe::ArticulationType::Staccatissimo, ms_NoteArticulation_Staccatissimo },
    { mpe::ArticulationType::Accent, ms_NoteArticulation_Accent },
    { mpe::ArticulationType::Tenuto, ms_NoteArticulation_Tenuto },
    { mpe::ArticulationType::Marcato, ms_NoteArticulation_Marcato },
    { mpe::ArticulationType::Harmonic, ms_NoteArticulation_Harmonics },
    { mpe::ArticulationType::Mute, ms_NoteArticulation_Mute },
    { mpe::ArticulationType::Trill, ms_NoteArticulation_WholeTrill },
    { mpe::ArticulationType::TrillBaroque, ms_NoteArticulation_WholeTrill },

    // Turn, Mordent

    { mpe::ArticulationType::Arpeggio, ms_NoteArticulation_ArpeggioUp },
    { mpe::ArticulationType::ArpeggioUp, ms_NoteArticulation_ArpeggioUp },
    { mpe::ArticulationType::ArpeggioDown, ms_NoteArticulation_ArpeggioDown },
    { mpe::ArticulationType::Tremolo8th, ms_NoteArticulation_Tremolo1 },
    { mpe::ArticulationType::Tremolo16th, ms_NoteArticulation_Tremolo2 },
    { mpe::ArticulationType::Tremolo32nd, ms_NoteArticulation_Tremolo3 },
    { mpe::ArticulationType::Tremolo64th, ms_NoteArticulation_Tremolo3 },

    { mpe::ArticulationType::Scoop, ms_NoteArticulation_Scoop },
    { mpe::ArticulationType::Plop, ms_NoteArticulation_Plop },
    { mpe::ArticulationType::Doit, ms_NoteArticulation_Doit },
    { mpe::ArticulationType::Fall, ms_NoteArticulation_Fall },
    { mpe::ArticulationType::PreAppoggiatura, ms_NoteArticulation_Appoggiatura },
    { mpe::ArticulationType::PostAppoggiatura, ms_NoteArticulation_Appoggiatura },
    { mpe::ArticulationType::Acciaccatura, ms_NoteArticulation_Acciaccatura },

    { mpe::ArticulationType::CrossNote, ms_NoteArticulation_XNote },
    { mpe::ArticulationType::GhostNote, ms_NoteArticulation_Ghost },
    { mpe::ArticulationType::CircleNote, ms_NoteArticulation_Circle },
    { mpe::ArticulationType::TriangleNote, ms_NoteArticulation_Triangle },
    { mpe::ArticulationType::DiamondNote, ms_NoteArticulation_Diamond }
};

MuseSamplerWrapper::MuseSamplerWrapper(MuseSamplerLibHandlerPtr samplerLib, const audio::AudioSourceParams& params)
    : AbstractSynthesizer(params), m_samplerLib(samplerLib)
{
    if (!m_samplerLib || !m_samplerLib->isValid()) {
        return;
    }

    m_samplerLib->initLib();
}

MuseSamplerWrapper::~MuseSamplerWrapper()
{
    if (!m_samplerLib || !m_sampler) {
        return;
    }

    m_samplerLib->destroy(m_sampler);
}

void MuseSamplerWrapper::setSampleRate(unsigned int sampleRate)
{
    m_sampleRate = sampleRate;

    m_sampler = m_samplerLib->create();

    if (!m_sampler) {
        return;
    }

    m_samplerLib->initSampler(m_sampler, m_sampleRate, 1024, AUDIO_CHANNELS_COUNT);

    static std::array<float, 1024> left;
    static std::array<float, 1024> right;

    m_bus._num_channels = AUDIO_CHANNELS_COUNT;
    m_bus._num_data_pts = 1024;

    static std::array<float*, AUDIO_CHANNELS_COUNT> channels;
    channels[0] = left.data();
    channels[1] = right.data();
    m_bus._channels = channels.data();
}

unsigned int MuseSamplerWrapper::audioChannelsCount() const
{
    return AUDIO_CHANNELS_COUNT;
}

async::Channel<unsigned int> MuseSamplerWrapper::audioChannelsCountChanged() const
{
    return m_audioChannelsCountChanged;
}

samples_t MuseSamplerWrapper::process(float* buffer, audio::samples_t samplesPerChannel)
{
    if (!m_samplerLib || !m_sampler || !m_track) {
        return 0;
    }

    if (!isActive()) {
        return 0;
    }

    if (m_samplerLib->process(m_sampler, m_bus, m_playbackPosition) != ms_Result_OK) {
        return 0;
    }

    extractOutputSamples(samplesPerChannel, buffer);

    audio::msecs_t newPlaybackPosition = m_playbackPosition + samplesToMsecs(samplesPerChannel, m_sampleRate);
    setPlaybackPosition(newPlaybackPosition);

    return samplesPerChannel;
}

std::string MuseSamplerWrapper::name() const
{
    return "musesampler";
}

AudioSourceType MuseSamplerWrapper::type() const
{
    return AudioSourceType::MuseSampler;
}

void MuseSamplerWrapper::flushSound()
{
    //TODO
}

bool MuseSamplerWrapper::isValid() const
{
    return m_samplerLib && m_sampler;
}

void MuseSamplerWrapper::setupSound(const mpe::PlaybackSetupData& setupData)
{
    IF_ASSERT_FAILED(m_samplerLib) {
        return;
    }

    if (!setupData.musicXmlSoundId.has_value()) {
        LOGE() << "Unable to setup MuseSampler";
        return;
    }

    auto instrumentList = m_samplerLib->getInstrumentList();
    if (instrumentList == nullptr) {
        LOGE() << "Unable to get instrument list";
        return;
    }

    int internalId = -1;

    while (auto instrument = m_samplerLib->getNextInstrument(instrumentList))
    {
        internalId = m_samplerLib->getInstrumentId(instrument);
        const char* internalName = m_samplerLib->getInstrumentName(instrument);
        const char* musicXmlId = m_samplerLib->getMusicXmlSoundId(instrument);

        LOGD() << internalId
               << ": " << internalName
               << " - " << musicXmlId;

        if (setupData.musicXmlSoundId->data() == musicXmlId) {
            break;
        }
    }

    if (internalId == -1) {
        LOGE() << "Unable to find sound for " << setupData.musicXmlSoundId.value();
        return;
    }

    m_track = m_samplerLib->addTrack(m_sampler, internalId);
}

void MuseSamplerWrapper::loadMainStreamEvents(const mpe::PlaybackEventsMap& events)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler && m_track) {
        return;
    }

    m_samplerLib->clearTrack(m_sampler, m_track);

    for (const auto& pair : events) {
        for (const auto& event : pair.second) {
            if (!std::holds_alternative<mpe::NoteEvent>(event)) {
                continue;
            }

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            addNoteEvent(noteEvent);
        }
    }

    m_samplerLib->finalizeScore(m_sampler);
}

void MuseSamplerWrapper::loadOffStreamEvents(const mpe::PlaybackEventsMap& events)
{
    UNUSED(events);
    NOT_IMPLEMENTED;
}

void MuseSamplerWrapper::loadDynamicLevelChanges(const mpe::DynamicLevelMap& dynamicLevels)
{
    UNUSED(dynamicLevels);
    NOT_IMPLEMENTED;
}

void MuseSamplerWrapper::extractOutputSamples(audio::samples_t samples, float* output)
{
    for (audio::samples_t sampleIndex = 0; sampleIndex < samples; ++sampleIndex) {
        for (audio::audioch_t audioChannelIndex = 0; audioChannelIndex < m_bus._num_channels; ++audioChannelIndex) {
            float sample = m_bus._channels[audioChannelIndex][sampleIndex];
            output[sampleIndex * m_bus._num_channels + audioChannelIndex] += sample;
        }
    }
}

void MuseSamplerWrapper::addNoteEvent(const mpe::NoteEvent& noteEvent)
{
    IF_ASSERT_FAILED(m_samplerLib && m_sampler && m_track) {
        return;
    }

    ms_Event event;
    event._event_type = ms_EventTypeNote;
    event._event._note = ms_NoteEvent();
    event._event._note._voice = noteEvent.arrangementCtx().voiceLayerIndex;
    event._event._note._location_ms = noteEvent.arrangementCtx().nominalTimestamp;
    event._event._note._duration_ms = noteEvent.arrangementCtx().nominalDuration;
    event._event._note._pitch = pitchIndex(noteEvent.pitchCtx().nominalPitchLevel);
    event._event._note._articulation = noteArticulationTypes(noteEvent);

    if (m_samplerLib->addTrackEvent(m_sampler, m_track, event) != ms_Result_OK) {
        LOGE() << "Unable to add event for track";
    }
}

int MuseSamplerWrapper::pitchIndex(const mpe::pitch_level_t pitchLevel) const
{
    static constexpr mpe::pitch_level_t MIN_SUPPORTED_LEVEL = mpe::pitchLevel(mpe::PitchClass::C, 0);
    static constexpr int MIN_SUPPORTED_NOTE = 12; // equivalent for C0
    static constexpr mpe::pitch_level_t MAX_SUPPORTED_LEVEL = mpe::pitchLevel(mpe::PitchClass::C, 8);
    static constexpr int MAX_SUPPORTED_NOTE = 108; // equivalent for C8

    if (pitchLevel <= MIN_SUPPORTED_LEVEL) {
        return MIN_SUPPORTED_NOTE;
    }

    if (pitchLevel >= MAX_SUPPORTED_LEVEL) {
        return MAX_SUPPORTED_NOTE;
    }

    float stepCount = MIN_SUPPORTED_NOTE + ((pitchLevel - MIN_SUPPORTED_LEVEL) / static_cast<float>(mpe::PITCH_LEVEL_STEP));

    return RealRound(stepCount, 0);
}

ms_NoteArticulation MuseSamplerWrapper::noteArticulationTypes(const mpe::NoteEvent& noteEvent) const
{
    uint64_t result = 0;

    for (const auto& pair : noteEvent.expressionCtx().articulations) {
        auto search = ARTICULATION_TYPES.find(pair.first);

        if (search == ARTICULATION_TYPES.cend()) {
            continue;
        }

        result |= search->second;
    }

    return static_cast<ms_NoteArticulation>(result);
}
