/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <gmock/gmock.h>

#include "audio/main/iaudioconfiguration.h"

namespace muse::audio {
class AudioConfigurationMock : public IAudioConfiguration
{
public:
    MOCK_METHOD(AudioEngineConfig, engineConfig, (), (const, override));

    MOCK_METHOD(std::string, defaultAudioApi, (), (const, override));
    MOCK_METHOD(std::string, currentAudioApi, (), (const, override));
    MOCK_METHOD(void, setCurrentAudioApi, (const std::string&), (override));
    MOCK_METHOD(async::Notification, currentAudioApiChanged, (), (const, override));

    MOCK_METHOD(std::string, audioOutputDeviceId, (), (const, override));
    MOCK_METHOD(void, setAudioOutputDeviceId, (const std::string&), (override));
    MOCK_METHOD(async::Notification, audioOutputDeviceIdChanged, (), (const, override));

    MOCK_METHOD(audioch_t, audioChannelsCount, (), (const, override));

    MOCK_METHOD(unsigned int, driverBufferSize, (), (const, override));
    MOCK_METHOD(void, setDriverBufferSize, (unsigned int), (override));
    MOCK_METHOD(async::Notification, driverBufferSizeChanged, (), (const, override));

    MOCK_METHOD(unsigned int, sampleRate, (), (const, override));
    MOCK_METHOD(void, setSampleRate, (unsigned int), (override));
    MOCK_METHOD(async::Notification, sampleRateChanged, (), (const, override));

    MOCK_METHOD(OutputSpec, defaultOutputSpec, (), (const, override));
    MOCK_METHOD(OutputSpec, desiredOutputSpec, (), (const, override));

    MOCK_METHOD(io::paths_t, soundFontDirectories, (), (const, override));
    MOCK_METHOD(io::paths_t, userSoundFontDirectories, (), (const, override));
    MOCK_METHOD(void, setUserSoundFontDirectories, (const io::paths_t&), (override));
    MOCK_METHOD(async::Channel<io::paths_t>, soundFontDirectoriesChanged, (), (const, override));

    MOCK_METHOD(bool, autoProcessOnlineSoundsInBackground, (), (const, override));
    MOCK_METHOD(void, setAutoProcessOnlineSoundsInBackground, (bool), (override));
    MOCK_METHOD(async::Channel<bool>, autoProcessOnlineSoundsInBackgroundChanged, (), (const, override));

    MOCK_METHOD(bool, useSoundFontLowPassFilter, (), (const, override));
    MOCK_METHOD(void, setUseSoundFontLowPassFilter, (bool), (override));
    MOCK_METHOD(async::Channel<bool>, useSoundFontLowPassFilterChanged, (), (const, override));

    MOCK_METHOD(bool, shouldMeasureInputLag, (), (const, override));
};
}
