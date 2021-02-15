//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUDIO_SANITYSYNTHESIZER_H
#define MU_AUDIO_SANITYSYNTHESIZER_H

#include "isynthesizer.h"

namespace mu::audio::synth {
class SanitySynthesizer : public ISynthesizer
{
public:
    SanitySynthesizer(ISynthesizerPtr synth);

    bool isValid() const override;

    std::string name() const override;
    SoundFontFormats soundFontFormats() const override;

    Ret init() override;
    Ret addSoundFonts(const std::vector<io::path>& sfonts) override;
    Ret removeSoundFonts() override;

    bool isActive() const override;
    void setIsActive(bool arg) override;

    Ret setupChannels(const std::vector<midi::Event>& events) override;
    bool handleEvent(const midi::Event& e) override;
    void writeBuf(float* stream, unsigned int samples) override;

    void allSoundsOff() override;  // all channels
    void flushSound() override;
    void channelSoundsOff(midi::channel_t chan) override;
    bool channelVolume(midi::channel_t chan, float val) override;   // 0. - 1.
    bool channelBalance(midi::channel_t chan, float val) override;  // -1. - 1.
    bool channelPitch(midi::channel_t chan, int16_t val) override;  // -12 - 12

    // IAudioSource
    void setSampleRate(unsigned int sampleRate) override;
    unsigned int streamCount() const override;
    async::Channel<unsigned int> streamsCountChanged() const override;
    void forward(unsigned int sampleCount) override;
    const float* data() const override;
    void setBufferSize(unsigned int samples) override;

private:

    ISynthesizerPtr m_synth;
};
}

#endif // MU_AUDIO_SANITYSYNTHESIZER_H
