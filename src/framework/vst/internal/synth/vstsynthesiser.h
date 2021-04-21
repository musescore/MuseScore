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

#ifndef MU_VST_VSTSYNTHESISER_H
#define MU_VST_VSTSYNTHESISER_H

#include "audio/isynthesizer.h"
#include "audio/iaudioconfiguration.h"
#include "modularity/ioc.h"

#include "vsttypes.h"
#include "ivstpluginrepository.h"
#include "vstaudioclient.h"

namespace mu::vst {
class VstSynthesiser : public audio::synth::ISynthesizer
{
    INJECT(vst, IVstPluginRepository, repository)
    INJECT(vst, audio::IAudioConfiguration, config)

public:
    explicit VstSynthesiser(const VstPluginMeta& meta);

    Ret init() override;

    bool isValid() const override;
    bool isActive() const override;
    void setIsActive(bool arg) override;

    std::string name() const override;

    audio::synth::SoundFontFormats soundFontFormats() const override;
    Ret addSoundFonts(const std::vector<io::path>& sfonts) override;
    Ret removeSoundFonts() override;

    bool handleEvent(const midi::Event& e) override;
    void writeBuf(float* stream, unsigned int samples) override;
    void allSoundsOff() override;
    void flushSound() override;

    Ret setupMidiChannels(const std::vector<midi::Event>& events) override;
    void midiChannelSoundsOff(midi::channel_t chan) override;
    bool midiChannelVolume(midi::channel_t chan, float val) override;
    bool midiChannelBalance(midi::channel_t chan, float val) override;
    bool midiChannelPitch(midi::channel_t chan, int16_t val) override;

    // IAudioSource
    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    async::Channel<unsigned int> audioChannelsCountChanged() const override;
    void process(float* buffer, unsigned int sampleCount) override;

private:

    VstPluginMeta m_pluginMeta;
    VstPluginPtr m_pluginPtr = nullptr;

    std::unique_ptr<VstAudioClient> m_vstAudioClient = nullptr;

    bool m_isActive = false;

    async::Channel<unsigned int> m_streamsCountChanged;
};
}

#endif // MU_VST_VSTSYNTHESISER_H
