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
#include "audiodriverstub.h"

using namespace muse;
using namespace muse::audio;

void AudioDriverStub::init()
{
}

std::string AudioDriverStub::name() const
{
    return {};
}

AudioDeviceID AudioDriverStub::defaultDevice() const
{
    return {};
}

bool AudioDriverStub::open(const IAudioDriver::Spec&, IAudioDriver::Spec*)
{
    return false;
}

void AudioDriverStub::close()
{
}

bool AudioDriverStub::isOpened() const
{
    return false;
}

const AudioDriverStub::Spec& AudioDriverStub::activeSpec() const
{
    static IAudioDriver::Spec dummySpec;
    return dummySpec;
}

async::Channel<AudioDriverStub::Spec> AudioDriverStub::activeSpecChanged() const
{
    static async::Channel<Spec> activeSpecChanged;
    return activeSpecChanged;
}

AudioDeviceList AudioDriverStub::availableOutputDevices() const
{
    return {};
}

async::Notification AudioDriverStub::availableOutputDevicesChanged() const
{
    return async::Notification();
}

std::vector<samples_t> AudioDriverStub::availableOutputDeviceBufferSizes() const
{
    return {};
}

std::vector<sample_rate_t> AudioDriverStub::availableOutputDeviceSampleRates() const
{
    return {};
}
