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
#include <cstdint>
#include <functional>

#include "modularity/ioc.h"

#include "isynthesizer.h"

namespace mu::audio::synth {
struct Fluid;
class FluidSynth : public ISynthesizer
{
public:
    FluidSynth();

    bool isValid() const override;

    std::string name() const override;
    SoundFontFormats soundFontFormats() const override;

    Ret init() override;
    void setSampleRate(unsigned int sampleRate) override;
    Ret addSoundFonts(const std::vector<io::path>& sfonts) override;
    Ret removeSoundFonts() override;

    bool isActive() const override;
    void setIsActive(bool arg) override;

    Ret setupMidiChannels(const std::vector<midi::Event>& events) override;
    bool handleEvent(const midi::Event& e) override;
    void writeBuf(float* stream, unsigned int samples) override;

    void allSoundsOff() override; // all channels
    void flushSound() override;

    void midiChannelSoundsOff(midi::channel_t chan) override;
    bool midiChannelVolume(midi::channel_t chan, float val) override;  // 0. - 1.
    bool midiChannelBalance(midi::channel_t chan, float val) override; // -1. - 1.
    bool midiChannelPitch(midi::channel_t chan, int16_t pitch) override; // -12 - 12

    unsigned int audioChannelsCount() const override;
    void process(float* buffer, unsigned int sampleCount) override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;

private:

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

    std::shared_ptr<Fluid> m_fluid = nullptr;
    std::vector<SoundFont> m_soundFonts;

    bool m_isLoggingSynthEvents = false;

    std::vector<float> m_preallocated; // used to flush a sound
    bool m_isActive = false;

    unsigned int m_sampleRate = 0;
    async::Channel<unsigned int> m_streamsCountChanged;
};
}

#endif //MU_AUDIO_FLUIDSYNTH_H
