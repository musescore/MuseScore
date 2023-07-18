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
#include "audioconfigurationstub.h"

using namespace mu::audio;
using namespace mu;

std::vector<std::string> AudioConfigurationStub::availableAudioApiList() const
{
    return {};
}

std::string AudioConfigurationStub::currentAudioApi() const
{
    return std::string();
}

void AudioConfigurationStub::setCurrentAudioApi(const std::string&)
{
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

samples_t AudioConfigurationStub::renderStep() const
{
    return 0;
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

AudioInputParams AudioConfigurationStub::defaultAudioInputParams() const
{
    return {};
}

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

// synthesizers
const synth::SynthesizerState& AudioConfigurationStub::synthesizerState() const
{
    static synth::SynthesizerState s;
    return s;
}

Ret AudioConfigurationStub::saveSynthesizerState(const synth::SynthesizerState&)
{
    return make_ret(Ret::Code::NotImplemented);
}

async::Notification AudioConfigurationStub::synthesizerStateChanged() const
{
    return async::Notification();
}

async::Notification AudioConfigurationStub::synthesizerStateGroupChanged(const std::string&) const
{
    return async::Notification();
}

io::path_t AudioConfigurationStub::knownAudioPluginsFilePath() const
{
    return {};
}
