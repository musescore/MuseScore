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
#include "audiooutputdevicecontroller.h"

#include "audio/common/rpc/rpcpacker.h"

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::rpc;

void AudioOutputDeviceController::init()
{
    audioDriver()->availableOutputDevicesChanged().onNotify(this, [this]() {
        LOGI() << "Available output devices changed, checking connection...";
        changeOutputDevice();
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
            rpcChannel()->send(rpc::make_request(Method::SetOutputSpec, RpcPacker::pack(audioDriver()->activeSpec().output)));
        }
    });

    configuration()->sampleRateChanged().onNotify(this, [this]() {
        sample_rate_t sampleRate = configuration()->sampleRate();
        LOGI() << "Trying to change sample rate: " << sampleRate;

        bool ok = audioDriver()->setOutputDeviceSampleRate(sampleRate);
        if (ok) {
            rpcChannel()->send(rpc::make_request(Method::SetOutputSpec, RpcPacker::pack(audioDriver()->activeSpec().output)));
        }
    });
}

void AudioOutputDeviceController::changeOutputDevice()
{
    AudioDeviceID preferredDeviceId = configuration()->audioOutputDeviceId();
    //! NOTE If the driver cannot open with the selected device,
    //! it will open with the default device.
    bool deviceChanged = audioDriver()->selectOutputDevice(preferredDeviceId);
    if (deviceChanged) {
        onOutputDeviceChanged();
    }
}

void AudioOutputDeviceController::onOutputDeviceChanged()
{
    if (!audioDriver()->isOpened()) {
        return;
    }

    IAudioDriver::Spec activeSpec = audioDriver()->activeSpec();
    rpcChannel()->send(rpc::make_request(Method::SetOutputSpec, RpcPacker::pack(activeSpec.output)));
}

IAudioDriverPtr AudioOutputDeviceController::audioDriver() const
{
    return audioDriverController()->audioDriver();
}
