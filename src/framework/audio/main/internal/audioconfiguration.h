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

#include "../iaudioconfiguration.h"

#include "global/modularity/ioc.h"
#include "global/io/ifilesystem.h"
#include "global/iglobalconfiguration.h"
#include "audio/common/rpc/irpcchannel.h"

namespace muse::audio {
class AudioConfiguration : public IAudioConfiguration, public Injectable
{
    Inject<IGlobalConfiguration> globalConfiguration = { this };
    Inject<io::IFileSystem> fileSystem = { this };
    Inject<rpc::IRpcChannel> rpcChannel = { this };

public:
    AudioConfiguration(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    AudioEngineConfig engineConfig() const override;
    void onWorkerConfigChanged();

    std::string defaultAudioApi() const override;
    std::string currentAudioApi() const override;
    void setCurrentAudioApi(const std::string& name) override;
    async::Notification currentAudioApiChanged() const override;

    std::string audioOutputDeviceId() const override;
    void setAudioOutputDeviceId(const std::string& deviceId) override;
    async::Notification audioOutputDeviceIdChanged() const override;

    audioch_t audioChannelsCount() const override;

    unsigned int driverBufferSize() const override;
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

private:
    void updateSamplesToPreallocate();

    async::Channel<io::paths_t> m_soundFontDirsChanged;
    async::Channel<samples_t> m_samplesToPreallocateChanged;
    async::Channel<bool> m_autoProcessOnlineSoundsInBackgroundChanged;

    async::Notification m_currentAudioApiChanged;
    async::Notification m_audioOutputDeviceIdChanged;
    async::Notification m_driverBufferSizeChanged;
    async::Notification m_driverSampleRateChanged;

    samples_t m_samplesToPreallocate = 0;
};
}
