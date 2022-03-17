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

#ifndef MU_AUDIO_FLUIDSYNTH_H
#define MU_AUDIO_FLUIDSYNTH_H

#include <memory>
#include <vector>
#include <list>
#include <cstdint>
#include <functional>
#include <unordered_set>

#include "modularity/ioc.h"

#include "abstractsynthesizer.h"
#include "soundmapping.h"

namespace mu::audio::synth {
struct Fluid;
class FluidSynth : public AbstractSynthesizer
{
public:
    FluidSynth(const audio::AudioSourceParams& params);

    SoundFontFormats soundFontFormats() const;
    Ret addSoundFonts(const std::vector<io::path>& sfonts);
    Ret removeSoundFonts();

    std::string name() const override;
    AudioSourceType type() const override;
    void setupSound(const mpe::PlaybackSetupData& setupData) override;
    void flushSound() override;

    bool hasAnythingToPlayback(const msecs_t from, const msecs_t to) const;

    void revokePlayingNotes() override; // all channels

    void midiChannelSoundsOff(midi::channel_t chan);
    bool midiChannelVolume(midi::channel_t chan, float val);  // 0. - 1.
    bool midiChannelBalance(midi::channel_t chan, float val); // -1. - 1.
    bool midiChannelPitch(midi::channel_t chan, int16_t pitch); // -12 - 12

    unsigned int audioChannelsCount() const override;
    samples_t process(float* buffer, samples_t samplesPerChannel) override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    void setSampleRate(unsigned int sampleRate) override;

    bool isValid() const override;

private:
    Ret init();

    void handleMainStreamEvents(const msecs_t nextMsecs);
    void handleOffStreamEvents(const msecs_t nextMsecs);
    void handleAlreadyPlayingEvents(const msecs_t from, const msecs_t to);

    bool handleNoteOnEvents(const mpe::PlaybackEvent& event, const msecs_t from, const msecs_t to);
    bool handleNoteOffEvents(const mpe::PlaybackEvent& event, const msecs_t from, const msecs_t to);

    void handlePitchBendControl(const mpe::NoteEvent& noteEvent, const midi::channel_t channelIdx, const msecs_t from, const msecs_t to);
    void handleAftertouch(const mpe::NoteEvent& noteEvent, const midi::channel_t channelIdx, const msecs_t from, const msecs_t to);

    void enablePortamentoMode(const mpe::NoteEvent& noteEvent, const midi::channel_t channelIdx);
    void enableLegatoMode(const midi::channel_t channelIdx);
    void enablePedalMode(const midi::channel_t channelIdx);

    bool isPortamentoModeApplicable(const mpe::NoteEvent& noteEvent) const;
    bool isLegatoModeApplicable(const mpe::NoteEvent& noteEvent) const;
    bool isPedalModeApplicable(const mpe::NoteEvent& noteEvent) const;

    bool hasToDisablePortamentoMode(const mpe::NoteEvent& noteEvent, const midi::channel_t channelIdx, const msecs_t from,
                                    const msecs_t to) const;

    void disablePortamentoMode(const midi::channel_t channelIdx);
    void disableLegatoMode(const midi::channel_t channelIdx);
    void disablePedalMode(const midi::channel_t channelIdx);

    void turnOffAllControllerModes();

    midi::channel_t channel(const mpe::NoteEvent& noteEvent) const;
    midi::channel_t findChannelByProgram(const midi::Program& program) const;
    midi::note_idx_t noteIndex(const mpe::pitch_level_t pitchLevel) const;
    midi::velocity_t noteVelocity(const mpe::dynamic_level_t dynamicLevel) const;

    enum midi_control
    {
        BANK_SELECT_MSB = 0x00,
        VOLUME_MSB      = 0x07,
        BALANCE_MSB     = 0x08,
        PAN_MSB         = 0x0A
    };

    struct SoundFont {
        int id = -1;
        io::path path;
    };

    struct ControllersModeContext {
        bool isPortamentoModeEnabled = false;
        bool isLegatoModeEnabled = false;
        bool isDamperPedalEnabled = false;
    };

    std::shared_ptr<Fluid> m_fluid = nullptr;
    std::vector<SoundFont> m_soundFonts;

    std::unordered_map<midi::channel_t, midi::Program> m_channels;
    ArticulationMapping m_articulationMapping;

    async::Channel<unsigned int> m_streamsCountChanged;

    mutable std::unordered_map<midi::channel_t, ControllersModeContext> m_controllersModeMap;

    std::list<mpe::PlaybackEvent> m_playingEvents;
};

using FluidSynthPtr = std::shared_ptr<FluidSynth>;
}

#endif //MU_AUDIO_FLUIDSYNTH_H
