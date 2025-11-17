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

#include "audiodrivercontrollerstub.h"

using namespace muse;
using namespace muse::audio;

// Api
std::vector<std::string> AudioDriverControllerStub::availableAudioApiList() const
{
    return {};
}

std::string AudioDriverControllerStub::currentAudioApi() const
{
    return {};
}

void AudioDriverControllerStub::changeCurrentAudioApi(const std::string&)
{
}

async::Notification AudioDriverControllerStub::currentAudioApiChanged() const
{
    return async::Notification();
}

// Current driver operation
AudioDeviceList AudioDriverControllerStub::availableOutputDevices() const
{
    return {};
}

async::Notification AudioDriverControllerStub::availableOutputDevicesChanged() const
{
    return async::Notification();
}

bool AudioDriverControllerStub::open(const IAudioDriver::Spec&, IAudioDriver::Spec*)
{
    return false;
}

void AudioDriverControllerStub::close()
{
}

bool AudioDriverControllerStub::isOpened() const
{
    return false;
}

const IAudioDriver::Spec& AudioDriverControllerStub::activeSpec() const
{
    static IAudioDriver::Spec spec;
    return spec;
}

async::Channel<IAudioDriver::Spec> AudioDriverControllerStub::activeSpecChanged() const
{
    return async::Channel<IAudioDriver::Spec>();
}

AudioDeviceID AudioDriverControllerStub::outputDevice() const
{
    return {};
}

bool AudioDriverControllerStub::selectOutputDevice(const AudioDeviceID&)
{
    return false;
}

async::Notification AudioDriverControllerStub::outputDeviceChanged() const
{
    return async::Notification();
}

std::vector<samples_t> AudioDriverControllerStub::availableOutputDeviceBufferSizes() const
{
    return {};
}

void AudioDriverControllerStub::changeBufferSize(samples_t)
{
}

async::Notification AudioDriverControllerStub::outputDeviceBufferSizeChanged() const
{
    return async::Notification();
}

std::vector<sample_rate_t> AudioDriverControllerStub::availableOutputDeviceSampleRates() const
{
    return {};
}

void AudioDriverControllerStub::changeSampleRate(sample_rate_t)
{
}

async::Notification AudioDriverControllerStub::outputDeviceSampleRateChanged() const
{
    return async::Notification();
}
