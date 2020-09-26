//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUDIO_OSXAUDIODRIVER_H
#define MU_AUDIO_OSXAUDIODRIVER_H

#include <memory>
#include <map>
#include <MacTypes.h>
#include "iaudiodriver.h"

struct AudioTimeStamp;
struct AudioQueueBuffer;
struct OpaqueAudioQueue;

namespace mu::audio {
class OSXAudioDriver : public IAudioDriver
{
public:
    OSXAudioDriver();
    ~OSXAudioDriver();
    const static std::string DEFAULT_DEVICE_NAME;

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;
    void resume() override;
    void suspend() override;

    std::string outputDevice() const override;
    bool selectOutputDevice(const std::string& name) override;
    std::vector<std::string> availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;
    void updateDeviceMap();

private:
    static void OnFillBuffer(void* context, OpaqueAudioQueue* queue, AudioQueueBuffer* buffer);
    static void logError(const std::string message, OSStatus error);

    void initDeviceMapListener();
    bool audioQueueSetDeviceName(const std::string& deviceName);

    struct Data;

    std::shared_ptr<Data> m_data = nullptr;
    std::map<unsigned int, std::string> m_outputDevices = {}, m_inputDevices = {};
    async::Notification m_availableOutputDevicesChanged;
    std::string m_deviceName = DEFAULT_DEVICE_NAME;
};
}
#endif // MU_AUDIO_OSXAUDIODRIVER_H
