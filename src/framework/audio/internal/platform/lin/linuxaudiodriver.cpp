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
#include "linuxaudiodriver.h"
#include "../alsa/alsaaudiodriver.h" //FIX: relative path, set path in CMakeLists

#include "translation.h"
#include "log.h"
#include "runtime.h"

using namespace mu::audio;

LinuxAudioDriver::LinuxAudioDriver()
{
    m_current_audioDriverState = std::make_unique<AlsaDriverState>();
}

LinuxAudioDriver::~LinuxAudioDriver()
{
}

void LinuxAudioDriver::init()
{
    m_devicesListener.startWithCallback([this]() {
        return availableOutputDevices();
    });

    m_devicesListener.devicesChanged().onNotify(this, [this]() {
        m_availableOutputDevicesChanged.notify();
    });
}

std::string LinuxAudioDriver::name() const
{
    return m_current_audioDriverState->name();
}

bool LinuxAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    return m_current_audioDriverState->open(spec, activeSpec);
}

void LinuxAudioDriver::close()
{
    return m_current_audioDriverState->close();
}

bool LinuxAudioDriver::isOpened() const
{
    return m_current_audioDriverState->isOpened();
}

AudioDeviceID LinuxAudioDriver::outputDevice() const
{
    return m_current_audioDriverState->name(); // m_deviceId;
}

bool LinuxAudioDriver::selectOutputDevice(const AudioDeviceID& deviceId)
{
    if (m_current_audioDriverState->name() == deviceId) {
        return true;
    }

    //FIX: no, we need to create the new device conditioned on the deviceId
    bool reopen = m_current_audioDriverState->isOpened();
    IAudioDriver::Spec spec(m_current_audioDriverState->m_spec);
    m_current_audioDriverState->close();

    bool ok = true;
    if (reopen) {
        ok = m_current_audioDriverState->open(spec, &spec);
    }
    m_current_audioDriverState->m_spec = spec;

    if (ok) {
        m_outputDeviceChanged.notify();
    }

    return ok;
}

bool LinuxAudioDriver::resetToDefaultOutputDevice()
{
    return selectOutputDevice("alsa"); // FIX:
}

mu::async::Notification LinuxAudioDriver::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

AudioDeviceList LinuxAudioDriver::availableOutputDevices() const
{
    AudioDeviceList devices;
    devices.push_back({ "alsa", trc("audio", "ALSA") });
    devices.push_back({ "jack", trc("audio", "JACK") });

    return devices;
}

mu::async::Notification LinuxAudioDriver::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}

unsigned int LinuxAudioDriver::outputDeviceBufferSize() const
{
    return m_current_audioDriverState->m_spec.samples;
}

bool LinuxAudioDriver::setOutputDeviceBufferSize(unsigned int bufferSize)
{
    if (m_current_audioDriverState->m_spec.samples == (int)bufferSize) {
        return true;
    }

    bool reopen = isOpened();
    close();
    m_current_audioDriverState->m_spec.samples = bufferSize;

    bool ok = true;
    if (reopen) {
        // FIX:
        // FIX:
        //ok = open(m_current_audioDriverState->m_spec, &m_current_audioDriverState->m_spec);
    }

    if (ok) {
        m_bufferSizeChanged.notify();
    }

    return ok;
}

mu::async::Notification LinuxAudioDriver::outputDeviceBufferSizeChanged() const
{
    return m_bufferSizeChanged;
}

std::vector<unsigned int> LinuxAudioDriver::availableOutputDeviceBufferSizes() const
{
    std::vector<unsigned int> result;

    unsigned int n = 4096;
    while (n >= MINIMUM_BUFFER_SIZE) {
        result.push_back(n);
        n /= 2;
    }

    std::sort(result.begin(), result.end());

    return result;
}

void LinuxAudioDriver::resume()
{
}

void LinuxAudioDriver::suspend()
{
}
