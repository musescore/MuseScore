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

    audioch_t audioChannelsCount() const override;
    unsigned int driverBufferSize() const override;

    io::paths soundFontDirectories() const override;
    async::Channel<io::paths> soundFontDirectoriesChanged() const override;

    AudioInputParams defaultAudioInputParams() const override;

    const synth::SynthesizerState& defaultSynthesizerState() const;
    const synth::SynthesizerState& synthesizerState() const override;
    Ret saveSynthesizerState(const synth::SynthesizerState& state) override;
    async::Notification synthesizerStateChanged() const override;
    async::Notification synthesizerStateGroupChanged(const std::string& groupName) const override;

private:
    async::Channel<io::paths> m_soundFontDirsChanged;

    io::path stateFilePath() const;
    bool readState(const io::path& path, synth::SynthesizerState& state) const;
    bool writeState(const io::path& path, const synth::SynthesizerState& state);

    mutable synth::SynthesizerState m_state;
    async::Notification m_synthesizerStateChanged;
    mutable std::map<std::string, async::Notification> m_synthesizerStateGroupChanged;
};
}

#endif // MU_AUDIO_AUDIOCONFIGURATION_H
