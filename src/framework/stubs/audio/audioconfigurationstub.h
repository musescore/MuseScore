/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#pragma once

#include "audio/main/iaudioconfiguration.h"

namespace muse::audio {
class AudioConfigurationStub : public IAudioConfiguration
{
public:
    AudioEngineConfig engineConfig() const override;

    std::string defaultAudioApi() const override;
    std::string currentAudioApi() const override;
    void setCurrentAudioApi(const std::string& name) override;
    async::Notification currentAudioApiChanged() const override;

    std::string audioOutputDeviceId() const override;
    void setAudioOutputDeviceId(const std::string& deviceId) override;
    async::Notification audioOutputDeviceIdChanged() const override;

    audioch_t audioChannelsCount() const override;

    unsigned int driverBufferSize() const override; // samples
    void setDriverBufferSize(unsigned int size) override;
    async::Notification driverBufferSizeChanged() const override;

    samples_t samplesToPreallocate() const override;
    async::Channel<samples_t> samplesToPreallocateChanged() const override;

    unsigned int sampleRate() const override;
    void setSampleRate(unsigned int sampleRate) override;
    async::Notification sampleRateChanged() const override;

    // synthesizers

    io::paths_t soundFontDirectories() const override;
    io::paths_t userSoundFontDirectories() const override;
    void setUserSoundFontDirectories(const io::paths_t& paths) override;
    async::Channel<io::paths_t> soundFontDirectoriesChanged() const override;

    bool autoProcessOnlineSoundsInBackground() const override;
    void setAutoProcessOnlineSoundsInBackground(bool process) override;
    async::Channel<bool> autoProcessOnlineSoundsInBackgroundChanged() const override;

    bool shouldMeasureInputLag() const override;
};
}
