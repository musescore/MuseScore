/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "midioutputsynth.h"
#include "midi/midievent.h"
#include "iplayer.h"

#include "log.h"

using namespace muse;
using namespace muse::midi;
using namespace muse::audio;
using namespace muse::audio::synth;

/// @note
///  Fluid does not support MONO, so they start counting audio channels from 1, which means "1 pair of audio channels"
/// @see https://www.fluidsynth.org/api/settings_synth.html
static constexpr unsigned int AUDIO_CHANNELS_PAIR = 1;
static constexpr unsigned int AUDIO_CHANNELS_COUNT = AUDIO_CHANNELS_PAIR * 2;

MidiOutputSynth::MidiOutputSynth(const AudioSourceParams& params)
    : AbstractSynthesizer(params)
{
    m_sequencer.setOnOffStreamFlushed([this]() {
        revokePlayingNotes();
    });

    LOGD() << "MIDI output synth inited\n";
}

std::string MidiOutputSynth::name() const
{
    return "MidiOutput";
}

AudioSourceType MidiOutputSynth::type() const
{
    return AudioSourceType::MidiOutput;
}

void MidiOutputSynth::setupSound(const mpe::PlaybackSetupData& setupData)
{
    m_sequencer.init(setupData, m_preset);
}

void MidiOutputSynth::setupEvents(const mpe::PlaybackData& playbackData)
{
    m_sequencer.load(playbackData);
}

void MidiOutputSynth::flushSound()
{
    revokePlayingNotes();
}

bool MidiOutputSynth::isActive() const
{
    return m_sequencer.isActive();
}

void MidiOutputSynth::setIsActive(const bool isActive)
{
    m_sequencer.setActive(isActive);
}

msecs_t MidiOutputSynth::playbackPosition() const
{
    return m_sequencer.playbackPosition();
}

void MidiOutputSynth::setPlaybackPosition(const msecs_t newPosition)
{
    m_sequencer.setPlaybackPosition(newPosition);
}

void MidiOutputSynth::revokePlayingNotes()
{
    playback()->playingNotesRevoked().send(m_trackId);
}

unsigned int MidiOutputSynth::audioChannelsCount() const
{
    return AUDIO_CHANNELS_COUNT;
}

samples_t MidiOutputSynth::process(float* /*buffer*/, samples_t samplesPerChannel)
{
    IF_ASSERT_FAILED(samplesPerChannel > 0) {
        return 0;
    }

    msecs_t nextMsecs = samplesToMsecs(samplesPerChannel, m_sampleRate);
    FluidSequencer::EventSequence sequence = m_sequencer.eventsToBePlayed(nextMsecs);

    for (const FluidSequencer::EventType& event : sequence) {
        handleEvent(std::get<midi::Event>(event));
    }

    return samplesPerChannel;
}

async::Channel<unsigned int> MidiOutputSynth::audioChannelsCountChanged() const
{
    return m_streamsCountChanged;
}

void MidiOutputSynth::setSampleRate(unsigned int sampleRate)
{
    if (m_sampleRate == sampleRate) {
        return;
    }

    m_sampleRate = sampleRate;
}

bool MidiOutputSynth::isValid() const
{
    return true;
}

bool MidiOutputSynth::handleEvent(const midi::Event& event)
{
    playback()->midiEvent().send(m_trackId, event);
    return true;
}
