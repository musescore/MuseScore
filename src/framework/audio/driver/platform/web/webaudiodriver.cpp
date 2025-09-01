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
#include "webaudiodriver.h"
#include "log.h"

using namespace muse;
using namespace muse::audio;

void WebAudioDriver::init()
{
}

std::string WebAudioDriver::name() const
{
    return "web";
}

bool WebAudioDriver::open(const Spec&, Spec*)
{
    return false;
}

void WebAudioDriver::close()
{
}

bool WebAudioDriver::isOpened() const
{
    return false;
}

const WebAudioDriver::Spec& WebAudioDriver::activeSpec() const
{
    static Spec spec;
    return spec;
}

bool WebAudioDriver::setOutputDeviceBufferSize(unsigned int)
{
    return false;
}

async::Notification WebAudioDriver::outputDeviceBufferSizeChanged() const
{
    return async::Notification();
}

bool WebAudioDriver::setOutputDeviceSampleRate(unsigned int)
{
    return false;
}

async::Notification WebAudioDriver::outputDeviceSampleRateChanged() const
{
    return async::Notification();
}

std::vector<unsigned int> WebAudioDriver::availableOutputDeviceBufferSizes() const
{
    return {};
}

std::vector<unsigned int> WebAudioDriver::availableOutputDeviceSampleRates() const
{
    return {};
}

AudioDeviceID WebAudioDriver::outputDevice() const
{
    return AudioDeviceID();
}

bool WebAudioDriver::selectOutputDevice(const AudioDeviceID&)
{
    return false;
}

bool WebAudioDriver::resetToDefaultOutputDevice()
{
    return true;
}

async::Notification WebAudioDriver::outputDeviceChanged() const
{
    return async::Notification();
}

AudioDeviceList WebAudioDriver::availableOutputDevices() const
{
    return {};
}

async::Notification WebAudioDriver::availableOutputDevicesChanged() const
{
    return async::Notification();
}

void WebAudioDriver::resume()
{
}

void WebAudioDriver::suspend()
{
}
