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

#include "../../abstractsynthesizer.h"
#include "../fluidsynth/fluidsequencer.h"
#include "iplayback.h"

#ifndef MU_AUDIO_MIDIOUTPUTSYNTH_H
#define MU_AUDIO_MIDIOUTPUTSYNTH_H

namespace muse::audio::synth {
class MidiOutputSynth : public AbstractSynthesizer
{
    Inject<IPlayback> playback;

public:
    MidiOutputSynth(const AudioSourceParams& params);

    std::string name() const override;
    AudioSourceType type() const override;
    void setupSound(const mpe::PlaybackSetupData& setupData) override;
    void setupEvents(const mpe::PlaybackData& playbackData) override;
    void flushSound() override;

    bool isActive() const override;
    void setIsActive(const bool isActive) override;

    msecs_t playbackPosition() const override;
    void setPlaybackPosition(const msecs_t newPosition) override;

    void revokePlayingNotes() override; // all channels

    unsigned int audioChannelsCount() const override;
    samples_t process(float* buffer, samples_t samplesPerChannel) override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    void setSampleRate(unsigned int sampleRate) override;

    bool isValid() const override;

private:
    bool handleEvent(const midi::Event& event);

    FluidSequencer m_sequencer;
    std::optional<midi::Program> m_preset;
    async::Channel<unsigned int> m_streamsCountChanged;
};

using MidiOutputSynthPtr = std::shared_ptr<MidiOutputSynth>;
}

#endif //MU_AUDIO_MIDIOUTPUTSYNTH_H
