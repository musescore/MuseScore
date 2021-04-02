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
#ifndef MU_AUDIO_AUDIOCONFIGURATION_H
#define MU_AUDIO_AUDIOCONFIGURATION_H

#include "../iaudioconfiguration.h"
#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

namespace mu::audio {
class AudioConfiguration : public IAudioConfiguration
{
    INJECT(audio, framework::IGlobalConfiguration, globalConfiguration)
public:
    AudioConfiguration() = default;

    void init();

    std::vector<std::string> availableAudioApiList() const override;

    std::string currentAudioApi() const override;
    void setCurrentAudioApi(const std::string& name) override;

    unsigned int driverBufferSize() const override;

    std::vector<io::path> soundFontPaths() const override;

    bool isShowControlsInMixer() const override;
    void setIsShowControlsInMixer(bool show) override;

    const synth::SynthesizerState& defaultSynthesizerState() const;
    const synth::SynthesizerState& synthesizerState() const override;
    Ret saveSynthesizerState(const synth::SynthesizerState& state) override;
    async::Notification synthesizerStateChanged() const override;
    async::Notification synthesizerStateGroupChanged(const std::string& groupName) const override;

private:

    io::path stateFilePath() const;
    bool readState(const io::path& path, synth::SynthesizerState& state) const;
    bool writeState(const io::path& path, const synth::SynthesizerState& state);

    mutable synth::SynthesizerState m_state;
    async::Notification m_synthesizerStateChanged;
    mutable std::map<std::string, async::Notification> m_synthesizerStateGroupChanged;
};
}

#endif // MU_AUDIO_AUDIOCONFIGURATION_H
