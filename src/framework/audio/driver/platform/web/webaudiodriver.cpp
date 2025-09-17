/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include <emscripten/val.h>

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

bool WebAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    LOGI() << "try open driver";

    emscripten::val::module_property("driver")["open"]();

    emscripten::val specVal = emscripten::val::module_property("driver")["outputSpec"]();
    m_activeSpec = spec;
    m_activeSpec.output.sampleRate = specVal["sampleRate"].as<double>();
    m_activeSpec.output.samplesPerChannel = specVal["samplesPerChannel"].as<int>();

    LOGI() << "activeSpec: "
           << "sampleRate: " << m_activeSpec.output.sampleRate
           << ", samplesPerChannel: " << m_activeSpec.output.samplesPerChannel
           << ", audioChannelCount: " << m_activeSpec.output.audioChannelCount
           << " (real: " << specVal["audioChannelCount"].as<int>() << ")";

    if (activeSpec) {
        *activeSpec = m_activeSpec;
    }

    m_opened = true;
    return true;
}

void WebAudioDriver::resume()
{
    emscripten::val::module_property("driver")["resume"]();
}

void WebAudioDriver::suspend()
{
    emscripten::val::module_property("driver")["suspend"]();
}

void WebAudioDriver::close()
{
    m_opened = false;
    emscripten::val::module_property("driver")["close"]();
}

bool WebAudioDriver::isOpened() const
{
    return m_opened;
}

const WebAudioDriver::Spec& WebAudioDriver::activeSpec() const
{
    return m_activeSpec;
}

bool WebAudioDriver::setOutputDeviceBufferSize(unsigned int)
{
    NOT_SUPPORTED;
    return false;
}

async::Notification WebAudioDriver::outputDeviceBufferSizeChanged() const
{
    static async::Notification n;
    return n;
}

bool WebAudioDriver::setOutputDeviceSampleRate(unsigned int)
{
    NOT_SUPPORTED;
    return false;
}

async::Notification WebAudioDriver::outputDeviceSampleRateChanged() const
{
    static async::Notification n;
    return n;
}

std::vector<unsigned int> WebAudioDriver::availableOutputDeviceBufferSizes() const
{
    std::vector<unsigned int> sizes;
    sizes.push_back(m_activeSpec.output.samplesPerChannel);
    return sizes;
}

std::vector<unsigned int> WebAudioDriver::availableOutputDeviceSampleRates() const
{
    std::vector<unsigned int> sizes;
    sizes.push_back(m_activeSpec.output.sampleRate);
    return sizes;
}

AudioDeviceID WebAudioDriver::outputDevice() const
{
    static AudioDeviceID id("default");
    return id;
}

bool WebAudioDriver::selectOutputDevice(const AudioDeviceID&)
{
    NOT_SUPPORTED;
    return false;
}

bool WebAudioDriver::resetToDefaultOutputDevice()
{
    return true;
}

async::Notification WebAudioDriver::outputDeviceChanged() const
{
    static async::Notification n;
    return n;
}

AudioDeviceList WebAudioDriver::availableOutputDevices() const
{
    AudioDeviceList list;
    AudioDevice d;
    d.id = outputDevice();
    d.name = d.id;
    list.push_back(d);
    return list;
}

async::Notification WebAudioDriver::availableOutputDevicesChanged() const
{
    static async::Notification n;
    return n;
}
