/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#ifndef MUSE_AUDIO_OSXAUDIODRIVER_H
#define MUSE_AUDIO_OSXAUDIODRIVER_H

#include <map>
#include <memory>
#include <mutex>

#include <MacTypes.h>

#include "iaudiodriver.h"

struct AudioTimeStamp;
struct AudioQueueBuffer;
struct OpaqueAudioQueue;

namespace muse::audio {
class OSXAudioDriver : public IAudioDriver
{
public:
    OSXAudioDriver();
    ~OSXAudioDriver();

    void init() override;

    std::string name() const override;

    AudioDeviceID defaultDevice() const override;

    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    const Spec& activeSpec() const override;
    async::Channel<Spec> activeSpecChanged() const override;

    AudioDeviceList availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;
    void updateDeviceMap();

    std::vector<samples_t> availableOutputDeviceBufferSizes() const override;
    std::vector<sample_rate_t> availableOutputDeviceSampleRates() const override;

private:
    static void OnFillBuffer(void* context, OpaqueAudioQueue* queue, AudioQueueBuffer* buffer);
    static void logError(const std::string message, OSStatus error);

    void initDeviceMapListener();
    bool audioQueueSetDeviceName(const AudioDeviceID& deviceId);

    AudioDeviceID defaultDeviceId() const;
    UInt32 osxDeviceId() const;

    struct Data;

    std::shared_ptr<Data> m_data = nullptr;
    async::Channel<Spec> m_activeSpecChanged;
    std::map<unsigned int, std::string> m_outputDevices = {}, m_inputDevices = {};
    mutable std::mutex m_devicesMutex;
    async::Notification m_availableOutputDevicesChanged;
};
}
#endif // MUSE_AUDIO_OSXAUDIODRIVER_H
