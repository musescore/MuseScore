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
#ifndef MUSE_AUDIO_AUDIOCONFIGURATION_H
#define MUSE_AUDIO_AUDIOCONFIGURATION_H

#include "global/modularity/ioc.h"
#include "global/io/ifilesystem.h"
#include "global/iglobalconfiguration.h"

#include "../iaudioconfiguration.h"

namespace muse::audio {
class AudioConfiguration : public IAudioConfiguration, public Injectable
{
    Inject<IGlobalConfiguration> globalConfiguration = { this };
    Inject<io::IFileSystem> fileSystem = { this };

public:
    AudioConfiguration(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    std::vector<std::string> availableAudioApiList() const override;

    std::string currentAudioApi() const override;
    void setCurrentAudioApi(const std::string& name) override;

    std::string audioOutputDeviceId() const override;
    void setAudioOutputDeviceId(const std::string& deviceId) override;
    async::Notification audioOutputDeviceIdChanged() const override;

    audioch_t audioChannelsCount() const override;

    unsigned int driverBufferSize() const override;
    void setDriverBufferSize(unsigned int size) override;
    async::Notification driverBufferSizeChanged() const override;

    msecs_t audioWorkerInterval(const samples_t samples, const sample_rate_t sampleRate) const override;
    samples_t minSamplesToReserve(RenderMode mode) const override;

    samples_t samplesToPreallocate() const override;
    async::Channel<samples_t> samplesToPreallocateChanged() const override;

    unsigned int sampleRate() const override;
    void setSampleRate(unsigned int sampleRate) override;
    async::Notification sampleRateChanged() const override;

    size_t desiredAudioThreadNumber() const override;
    int audioDelayCompensate() const override;
    void setAudioDelayCompensate(const int frames) override;

    size_t minTrackCountForMultithreading() const override;

    // synthesizers
    AudioInputParams defaultAudioInputParams() const override;

    io::paths_t soundFontDirectories() const override;
    io::paths_t userSoundFontDirectories() const override;
    void setUserSoundFontDirectories(const io::paths_t& paths) override;
    async::Channel<io::paths_t> soundFontDirectoriesChanged() const override;

    bool shouldMeasureInputLag() const override;

private:
    void updateSamplesToPreallocate();
    int m_audioDelayCompensate = 0;

    async::Channel<io::paths_t> m_soundFontDirsChanged;
    async::Channel<samples_t> m_samplesToPreallocateChanged;

    async::Notification m_audioOutputDeviceIdChanged;
    async::Notification m_driverBufferSizeChanged;
    async::Notification m_driverSampleRateChanged;

    samples_t m_samplesToPreallocate = 0;
};
}

#endif // MUSE_AUDIO_AUDIOCONFIGURATION_H
