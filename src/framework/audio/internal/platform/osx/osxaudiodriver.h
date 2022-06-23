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
#ifndef MU_AUDIO_OSXAUDIODRIVER_H
#define MU_AUDIO_OSXAUDIODRIVER_H

#include <memory>
#include <map>

#include <MacTypes.h>

#include "modularity/ioc.h"
#include "iaudioconfiguration.h"

#include "iaudiodriver.h"

struct AudioTimeStamp;
struct AudioQueueBuffer;
struct OpaqueAudioQueue;

namespace mu::audio {
class OSXAudioDriver : public IAudioDriver
{
    INJECT(audio, IAudioConfiguration, configuration)

public:
    OSXAudioDriver();
    ~OSXAudioDriver();

    void init() override;

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;
    void resume() override;
    void suspend() override;

    AudioDeviceID outputDevice() const override;
    bool selectOutputDevice(const AudioDeviceID& deviceId) override;
    bool resetToDefaultOutputDevice() override;
    async::Notification outputDeviceChanged() const override;

    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;
    void updateDeviceMap();

private:
    static void OnFillBuffer(void* context, OpaqueAudioQueue* queue, AudioQueueBuffer* buffer);
    static void logError(const std::string message, OSStatus error);

    void initDeviceMapListener();
    bool audioQueueSetDeviceName(const AudioDeviceID& deviceId);

    struct Data;

    std::shared_ptr<Data> m_data = nullptr;
    std::map<unsigned int, std::string> m_outputDevices = {}, m_inputDevices = {};
    async::Notification m_outputDeviceChanged;
    async::Notification m_availableOutputDevicesChanged;
    AudioDeviceID m_deviceId;
};
}
#endif // MU_AUDIO_OSXAUDIODRIVER_H
