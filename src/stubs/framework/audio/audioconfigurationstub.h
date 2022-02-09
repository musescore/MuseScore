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
#ifndef MU_AUDIO_AUDIOCONFIGURATIONSTUB_H
#define MU_AUDIO_AUDIOCONFIGURATIONSTUB_H

#include "audio/iaudioconfiguration.h"

namespace mu::audio {
class AudioConfigurationStub : public IAudioConfiguration
{
public:

    std::vector<std::string> availableAudioApiList() const override;

    std::string currentAudioApi() const override;
    void setCurrentAudioApi(const std::string& name) override;

    int audioChannelsCount() const override;
    unsigned int driverBufferSize() const override;  // samples

    // synthesizers
    std::vector<io::path> soundFontPaths() const override;
    const synth::SynthesizerState& synthesizerState() const override;
    Ret saveSynthesizerState(const synth::SynthesizerState& state) override;
    async::Notification synthesizerStateChanged() const override;
    async::Notification synthesizerStateGroupChanged(const std::string& groupName) const override;
};
}

#endif // MU_AUDIO_AUDIOCONFIGURATIONSTUB_H
