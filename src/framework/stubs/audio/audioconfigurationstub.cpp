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
#include "audioconfigurationstub.h"

using namespace muse::audio;
using namespace muse;

AudioEngineConfig AudioConfigurationStub::engineConfig() const
{
    return {};
}

std::string AudioConfigurationStub::defaultAudioApi() const
{
    return {};
}

std::string AudioConfigurationStub::currentAudioApi() const
{
    return {};
}

void AudioConfigurationStub::setCurrentAudioApi(const std::string&)
{
}

async::Notification AudioConfigurationStub::currentAudioApiChanged() const
{
    return {};
}

std::string AudioConfigurationStub::audioOutputDeviceId() const
{
    return "";
}

void AudioConfigurationStub::setAudioOutputDeviceId(const std::string&)
{
}

async::Notification AudioConfigurationStub::audioOutputDeviceIdChanged() const
{
    return async::Notification();
}

audioch_t AudioConfigurationStub::audioChannelsCount() const
{
    return 2;
}

unsigned int AudioConfigurationStub::driverBufferSize() const
{
    return 0;
}

void AudioConfigurationStub::setDriverBufferSize(unsigned int)
{
}

async::Notification AudioConfigurationStub::driverBufferSizeChanged() const
{
    return async::Notification();
}

samples_t AudioConfigurationStub::samplesToPreallocate() const
{
    return 0;
}

async::Channel<samples_t> AudioConfigurationStub::samplesToPreallocateChanged() const
{
    return async::Channel<samples_t>();
}

unsigned int AudioConfigurationStub::sampleRate() const
{
    return 0;
}

void AudioConfigurationStub::setSampleRate(unsigned int)
{
}

async::Notification AudioConfigurationStub::sampleRateChanged() const
{
    return async::Notification();
}

// synthesizers
io::paths_t AudioConfigurationStub::soundFontDirectories() const
{
    return {};
}

io::paths_t AudioConfigurationStub::userSoundFontDirectories() const
{
    return {};
}

void AudioConfigurationStub::setUserSoundFontDirectories(const io::paths_t&)
{
}

async::Channel<io::paths_t> AudioConfigurationStub::soundFontDirectoriesChanged() const
{
    return async::Channel<io::paths_t>();
}

bool AudioConfigurationStub::autoProcessOnlineSoundsInBackground() const
{
    return false;
}

void AudioConfigurationStub::setAutoProcessOnlineSoundsInBackground(bool)
{
}

async::Channel<bool> AudioConfigurationStub::autoProcessOnlineSoundsInBackgroundChanged() const
{
    return {};
}

bool AudioConfigurationStub::shouldMeasureInputLag() const
{
    return false;
}
