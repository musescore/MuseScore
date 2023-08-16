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
#ifndef MU_FRAMEWORK_AUDIOCONFIGURATIONMOCK_H
#define MU_FRAMEWORK_AUDIOCONFIGURATIONMOCK_H

#include <gmock/gmock.h>

#include "framework/audio/iaudioconfiguration.h"

namespace mu::audio {
class AudioConfigurationMock : public IAudioConfiguration
{
public:
    MOCK_METHOD(std::vector<std::string>, availableAudioApiList, (), (const, override));

    MOCK_METHOD(std::string, currentAudioApi, (), (const, override));
    MOCK_METHOD(void, setCurrentAudioApi, (const std::string&), (override));

    MOCK_METHOD(std::string, audioOutputDeviceId, (), (const, override));
    MOCK_METHOD(void, setAudioOutputDeviceId, (const std::string&), (override));
    MOCK_METHOD(async::Notification, audioOutputDeviceIdChanged, (), (const, override));

    MOCK_METHOD(audioch_t, audioChannelsCount, (), (const, override));

    MOCK_METHOD(unsigned int, driverBufferSize, (), (const, override));
    MOCK_METHOD(void, setDriverBufferSize, (unsigned int), (override));
    MOCK_METHOD(async::Notification, driverBufferSizeChanged, (), (const, override));
    MOCK_METHOD(samples_t, renderStep, (), (const, override));

    MOCK_METHOD(unsigned int, sampleRate, (), (const, override));
    MOCK_METHOD(void, setSampleRate, (unsigned int), (override));
    MOCK_METHOD(async::Notification, sampleRateChanged, (), (const, override));

    // synthesizers
    MOCK_METHOD(AudioInputParams, defaultAudioInputParams, (), (const, override));

    MOCK_METHOD(io::paths_t, soundFontDirectories, (), (const, override));
    MOCK_METHOD(io::paths_t, userSoundFontDirectories, (), (const, override));
    MOCK_METHOD(void, setUserSoundFontDirectories, (const io::paths_t&), (override));
    MOCK_METHOD(async::Channel<io::paths_t>, soundFontDirectoriesChanged, (), (const, override));

    MOCK_METHOD(io::path_t, knownAudioPluginsFilePath, (), (const, override));
};
}

#endif // MU_FRAMEWORK_AUDIOCONFIGURATIONMOCK_H
