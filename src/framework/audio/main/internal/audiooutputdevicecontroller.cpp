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
#include "audiooutputdevicecontroller.h"

#include "audio/common/rpc/rpcpacker.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::rpc;

void AudioOutputDeviceController::init()
{
    checkConnection();

    audioDriver()->availableOutputDevicesChanged().onNotify(this, [this]() {
        LOGI() << "Available output devices changed, checking connection...";
        checkConnection();
    });

    configuration()->audioOutputDeviceIdChanged().onNotify(this, [this]() {
        AudioDeviceID deviceId = configuration()->audioOutputDeviceId();
        LOGI() << "Trying to change output device: " << deviceId;

        bool ok = audioDriver()->selectOutputDevice(deviceId);
        if (ok) {
            onOutputDeviceChanged();
        }
    });

    configuration()->driverBufferSizeChanged().onNotify(this, [this]() {
        uint16_t bufferSize = configuration()->driverBufferSize();
        LOGI() << "Trying to change buffer size: " << bufferSize;

        bool ok = audioDriver()->setOutputDeviceBufferSize(bufferSize);
        if (ok) {
            rpcChannel()->send(rpc::make_request(Method::SetReadBufferSize, RpcPacker::pack(bufferSize)));
        }
    });

    configuration()->sampleRateChanged().onNotify(this, [this]() {
        sample_rate_t sampleRate = configuration()->sampleRate();
        LOGI() << "Trying to change sample rate: " << sampleRate;

        bool ok = audioDriver()->setOutputDeviceSampleRate(sampleRate);
        if (ok) {
            rpcChannel()->send(rpc::make_request(Method::SetSampleRate, RpcPacker::pack(sampleRate)));
        }
    });
}

void AudioOutputDeviceController::checkConnection()
{
    auto containsDevice = [](const AudioDeviceList& devices, const AudioDeviceID& deviceId) {
        for (const AudioDevice& device : devices) {
            if (device.id == deviceId) {
                return true;
            }
        }

        return false;
    };

    AudioDeviceID preferredDeviceId = configuration()->audioOutputDeviceId();
    AudioDeviceID currentDeviceId = audioDriver()->outputDevice();
    AudioDeviceList devices = audioDriver()->availableOutputDevices();

    bool deviceChanged = false;

    if (!preferredDeviceId.empty() && preferredDeviceId != currentDeviceId && containsDevice(devices, preferredDeviceId)) {
        LOGI() << "Changing output device: " << preferredDeviceId;
        deviceChanged = audioDriver()->selectOutputDevice(preferredDeviceId);
    } else if (!containsDevice(devices, currentDeviceId)) {
        LOGW() << "Device " << currentDeviceId << " not found, resetting to default";
        deviceChanged = audioDriver()->resetToDefaultOutputDevice();
    }

    if (deviceChanged) {
        onOutputDeviceChanged();
    }
}

void AudioOutputDeviceController::onOutputDeviceChanged()
{
    IAudioDriver::Spec activeSpec = audioDriver()->activeSpec();

    // TODO: audioEngine()->setAudioChannelsCount(activeSpec.channels);
    rpcChannel()->send(rpc::make_request(Method::SetSampleRate, RpcPacker::pack(activeSpec.sampleRate)));
    rpcChannel()->send(rpc::make_request(Method::SetReadBufferSize, RpcPacker::pack(activeSpec.samples)));
}
