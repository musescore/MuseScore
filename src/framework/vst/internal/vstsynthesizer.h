//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_VST_VSTSYNTHESIZER_H
#define MU_VST_VSTSYNTHESIZER_H

#include "modularity/ioc.h"
#include "framework/audio/isynthesizer.h"
#include "framework/audio/isynthesizersregister.h"

namespace mu::vst {
class PluginInstance;
class VSTSynthesizer : public mu::audio::synth::ISynthesizer
{
    INJECT_STATIC(vst, mu::audio::synth::ISynthesizersRegister, synthesizersRegister)

public:
    static std::shared_ptr<VSTSynthesizer> create(std::shared_ptr<PluginInstance> instance);
    VSTSynthesizer(std::string name, std::shared_ptr<PluginInstance> instance);

    bool isValid() const override;

    std::string name() const override;
    audio::synth::SoundFontFormats soundFontFormats() const override;

    Ret init() override;
    void setSampleRate(unsigned int sampleRate) override;
    Ret addSoundFonts(const std::vector<io::path>& sfonts) override;
    Ret removeSoundFonts() override;

    bool isActive() const override;
    void setIsActive(bool arg) override;

    Ret setupChannels(const std::vector<mu::midi::Event>& events) override;
    bool handleEvent(const mu::midi::Event& e) override;
    void writeBuf(float* stream, unsigned int samples) override;

    void allSoundsOff() override;
    void flushSound() override;
    void channelSoundsOff(mu::midi::channel_t chan) override;
    bool channelVolume(mu::midi::channel_t chan, float val) override;  // 0. - 1.
    bool channelBalance(mu::midi::channel_t chan, float val) override; // -1. - 1.
    bool channelPitch(mu::midi::channel_t chan, int16_t val) override; // -12 - 12

    unsigned int streamCount() const override;
    void forward(unsigned int sampleCount) override;
    async::Channel<unsigned int> streamsCountChanged() const override;
    const float* data() const override;
    void setBufferSize(unsigned int samples) override;

private:
    std::string m_name = "";
    std::shared_ptr<PluginInstance> m_instance = nullptr;
    unsigned int m_sampleRate = 1;
    std::vector<float> m_buffer = {};
    async::Channel<unsigned int> m_streamsCountChanged;
};
}

#endif // MU_VST_VSTSYNTHESIZER_H
