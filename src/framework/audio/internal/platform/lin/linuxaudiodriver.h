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

#ifndef MU_AUDIO_LINUXAUDIODRIVER_H
#define MU_AUDIO_LINUXAUDIODRIVER_H

#include "async/asyncable.h"

#include "iaudiodriver.h"

#include "audiodeviceslistener.h"

namespace mu::audio {
class LinuxAudioDriver : public IAudioDriver, public async::Asyncable
{
public:
    LinuxAudioDriver();
    ~LinuxAudioDriver();

    void init() override;

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    AudioDeviceID outputDevice() const override;
    bool selectOutputDevice(const AudioDeviceID& deviceId) override;
    bool resetToDefaultOutputDevice() override;
    async::Notification outputDeviceChanged() const override;

    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;

    unsigned int outputDeviceBufferSize() const override;
    bool setOutputDeviceBufferSize(unsigned int bufferSize) override;
    async::Notification outputDeviceBufferSizeChanged() const override;

    std::vector<unsigned int> availableOutputDeviceBufferSizes() const override;

    void resume() override;
    void suspend() override;

private:
    async::Notification m_outputDeviceChanged;

    mutable std::mutex m_devicesMutex;
    AudioDevicesListener m_devicesListener;
    async::Notification m_availableOutputDevicesChanged;

    std::string m_deviceId;

    async::Notification m_bufferSizeChanged;
    async::Notification m_sampleRateChanged;
};
}

#endif // MU_AUDIO_LINUXAUDIODRIVER_H
