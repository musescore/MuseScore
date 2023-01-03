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
#include "audiodriverstub.h"

using namespace mu::audio;

void AudioDriverStub::init()
{
}

std::string AudioDriverStub::name() const
{
    return std::string();
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

std::string AudioDriverStub::outputDevice() const
{
    return std::string();
}

bool AudioDriverStub::selectOutputDevice(const std::string&)
{
    return false;
}

bool AudioDriverStub::resetToDefaultOutputDevice()
{
    return false;
}

mu::async::Notification AudioDriverStub::outputDeviceChanged() const
{
    return async::Notification();
}

AudioDeviceList AudioDriverStub::availableOutputDevices() const
{
    return {};
}

mu::async::Notification AudioDriverStub::availableOutputDevicesChanged() const
{
    return async::Notification();
}

unsigned int AudioDriverStub::outputDeviceBufferSize() const
{
    return 0;
}

bool AudioDriverStub::setOutputDeviceBufferSize(unsigned int)
{
    return false;
}

mu::async::Notification AudioDriverStub::outputDeviceBufferSizeChanged() const
{
    return async::Notification();
}

std::vector<unsigned int> AudioDriverStub::availableOutputDeviceBufferSizes() const
{
    return {};
}

void AudioDriverStub::resume()
{
}

void AudioDriverStub::suspend()
{
}
