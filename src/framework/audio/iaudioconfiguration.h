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
#ifndef MU_AUDIO_IAUDIOCONFIGURATION_H
#define MU_AUDIO_IAUDIOCONFIGURATION_H

#include "modularity/imoduleinterface.h"
#include "io/path.h"
#include "types/ret.h"
#include "async/channel.h"
#include "async/notification.h"

#include "audiotypes.h"

namespace mu::audio {
class IAudioConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IAudioConfiguration)
public:
    virtual ~IAudioConfiguration() = default;

    virtual std::vector<std::string> availableAudioApiList() const = 0;

    virtual std::string currentAudioApi() const = 0;
    virtual void setCurrentAudioApi(const std::string& name) = 0;

    virtual std::string audioOutputDeviceId() const = 0;
    virtual void setAudioOutputDeviceId(const std::string& deviceId) = 0;
    virtual async::Notification audioOutputDeviceIdChanged() const = 0;

    virtual audioch_t audioChannelsCount() const = 0;

    virtual unsigned int driverBufferSize() const = 0; // samples
    virtual void setDriverBufferSize(unsigned int size) = 0;
    virtual async::Notification driverBufferSizeChanged() const = 0;
    virtual samples_t renderStep() const = 0;

    virtual unsigned int sampleRate() const = 0;
    virtual void setSampleRate(unsigned int sampleRate) = 0;
    virtual async::Notification sampleRateChanged() const = 0;

    // synthesizers
    virtual AudioInputParams defaultAudioInputParams() const = 0;

    virtual io::paths_t soundFontDirectories() const = 0;
    virtual io::paths_t userSoundFontDirectories() const = 0;
    virtual void setUserSoundFontDirectories(const io::paths_t& paths) = 0;
    virtual async::Channel<io::paths_t> soundFontDirectoriesChanged() const = 0;

    virtual io::path_t knownAudioPluginsFilePath() const = 0;
};
}

#endif // MU_AUDIO_IAUDIOCONFIGURATION_H
