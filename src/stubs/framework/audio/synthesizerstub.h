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
#ifndef MU_AUDIO_SYNTHESIZERSTUB_H
#define MU_AUDIO_SYNTHESIZERSTUB_H

#include "audio/isynthesizer.h"

namespace mu::audio::synth {
class SynthesizerStub : public ISynthesizer
{
public:
    // IAudioSource
    void setSampleRate(unsigned int sampleRate) override;

    unsigned int streamCount() const override;

    async::Channel<unsigned int> streamsCountChanged() const override;

    void forward(unsigned int sampleCount) override;

    const float* data() const override;

    void setBufferSize(unsigned int samples) override;

    // ISynthesizer
    bool isValid() const override;

    std::string name() const override;
    SoundFontFormats soundFontFormats() const override;

    Ret init(float samplerate) override;
    Ret addSoundFonts(const std::vector<io::path_t>& sfonts) override;
    Ret removeSoundFonts() override;

    bool isActive() const override;
    void setIsActive(bool arg) override;

    Ret setupChannels(const std::vector<midi::Event>& events) override;
    bool handleEvent(const midi::Event& e) override;
    void writeBuf(float* stream, unsigned int samples) override;

    void allSoundsOff() override;
    void flushSound() override;
    void channelSoundsOff(midi::channel_t chan) override;
    bool channelVolume(midi::channel_t chan, float val) override;
    bool channelBalance(midi::channel_t chan, float val) override;
    bool channelPitch(midi::channel_t chan, int16_t val) override;
};
}

#endif // MU_AUDIO_SYNTHESIZERSTUB_H
