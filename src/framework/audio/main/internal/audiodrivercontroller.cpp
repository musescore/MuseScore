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

#include "audiodrivercontroller.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_AUDIO_JACK
#include "audio/driver/platform/jack/jackaudiodriver.h"
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#include <QtEnvironmentVariables>
#include "audio/driver/platform/lin/alsaaudiodriver.h"
#ifdef MUSE_MODULE_AUDIO_PIPEWIRE
#include "audio/driver/platform/lin/pwaudiodriver.h"
#endif
#endif

#ifdef Q_OS_WIN
//#include "audio/driver/platform/win/winmmdriver.h"
//#include "audio/driver/platform/win/wincoreaudiodriver.h"
//#include "audio/driver/platform/win/wasapiaudiodriver.h"
#include "audio/driver/platform/win/wasapiaudiodriver2.h"
#ifdef MUSE_MODULE_AUDIO_ASIO
#include "audio/driver/platform/win/asio/asioaudiodriver.h"
#endif
#endif

#ifdef Q_OS_MACOS
#include "audio/driver/platform/osx/osxaudiodriver.h"
#endif

#ifdef Q_OS_WASM
#include "audio/driver/platform/web/webaudiodriver.h"
#endif

#include "log.h"

using namespace muse::audio;
using namespace muse::audio::rpc;

IAudioDriverPtr AudioDriverController::createDriver(const std::string& name) const
{
#ifdef MUSE_MODULE_AUDIO_JACK
    LOGW() << "required: " << name << ", but is set MUSE_MODULE_AUDIO_JACK";
    return std::shared_ptr<IAudioDriver>(new JackAudioDriver());
#endif

#ifdef Q_OS_WIN
    if (name == "ASIO") {
#ifdef MUSE_MODULE_AUDIO_ASIO
        return std::shared_ptr<IAudioDriver>(new AsioAudioDriver());
#else
        LOGW() << "ASIO is required but is not available, WASAPI will be used";
#endif
    }

    // required WASAPI or fallback
    return std::shared_ptr<IAudioDriver>(new WasapiAudioDriver2());
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    if (qEnvironmentVariableIsSet("MUSESCORE_FORCE_ALSA")) {
        if (name != "ALSA") {
            LOGW() << "required: " << name << ", but is set MUSESCORE_FORCE_ALSA";
        }
        return std::make_shared<AlsaAudioDriver>();
    }

    if (name == "PipeWire") {
#ifdef MUSE_MODULE_AUDIO_PIPEWIRE
        auto driver = std::make_shared<PwAudioDriver>();
        if (driver->connectedToPwServer()) {
            return driver;
        } else {
            LOGE() << "PipeWire driver failed connected to server";
        }
#else
        LOGW() << "PipeWire is required but is not available, ALSA will be used";
#endif
    }

    // required ALSA or fallback
    return std::make_shared<AlsaAudioDriver>();
#endif

#ifdef Q_OS_MACOS
    UNUSED(name);
    return std::shared_ptr<IAudioDriver>(new OSXAudioDriver());
#endif

#ifdef Q_OS_WASM
    UNUSED(name);
    return std::shared_ptr<IAudioDriver>(new WebAudioDriver());
#endif
}

std::vector<std::string> AudioDriverController::availableAudioApiList() const
{
    std::vector<std::string> names;
#ifdef MUSE_MODULE_AUDIO_JACK
    names.push_back("JACK");
    return names;
#endif

#ifdef Q_OS_WIN
    names.push_back("WASAPI");
#ifdef MUSE_MODULE_AUDIO_ASIO
    names.push_back("ASIO");
#endif

    return names;
#endif // Q_OS_WIN

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    names.push_back("ALSA");
    if (qEnvironmentVariableIsSet("MUSESCORE_FORCE_ALSA")) {
        return names;
    }

#ifdef MUSE_MODULE_AUDIO_PIPEWIRE
    names.push_back("PipeWire");
#endif

    return names;
#endif // Q_OS_LINUX

#ifdef Q_OS_MACOS
    names.push_back("CoreAudio");
    return names;
#endif

#ifdef Q_OS_WASM
    names.push_back("WASM");
    return names;
#endif
}

void AudioDriverController::init()
{
    std::string name = configuration()->currentAudioApi();
    m_audioDriver = createDriver(name);
    m_audioDriver->init();
    subscribeOnDriver();
    LOGI() << "Used " << m_audioDriver->name() << " audio driver";
}

void AudioDriverController::changeAudioDriver(const std::string& name)
{
    IAudioDriver::Spec activeSpec;
    if (m_audioDriver && m_audioDriver->isOpened()) {
        activeSpec = m_audioDriver->activeSpec();
        m_audioDriver->availableOutputDevicesChanged().disconnect(this);
        m_audioDriver->close();
    }

    m_audioDriver = createDriver(name);
    m_audioDriver->init();
    subscribeOnDriver();
    LOGI() << "Used " << m_audioDriver->name() << " audio driver";

    if (activeSpec.isValid()) {
        m_audioDriver->open(activeSpec, &activeSpec);
    }

    configuration()->setCurrentAudioApi(name);
    m_audioDriverChanged.notify();
}

void AudioDriverController::subscribeOnDriver()
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return;
    }

    m_audioDriver->availableOutputDevicesChanged().onNotify(this, [this]() {
        LOGI() << "Available output devices changed, checking connection...";
        checkOutputDevice();
    });
}

void AudioDriverController::selectOutputDevice(const std::string& deviceId)
{
    LOGI() << "Trying to change output device: " << deviceId;
    bool ok = audioDriver()->selectOutputDevice(deviceId);
    if (ok) {
        configuration()->setAudioOutputDeviceId(deviceId);
        updateOutputSpec();
    }
}

void AudioDriverController::checkOutputDevice()
{
    AudioDeviceID preferredDeviceId = configuration()->audioOutputDeviceId();
    //! NOTE If the driver cannot open with the selected device,
    //! it will open with the default device.
    bool deviceChanged = m_audioDriver->selectOutputDevice(preferredDeviceId);
    if (deviceChanged) {
        updateOutputSpec();
    }
}

void AudioDriverController::updateOutputSpec()
{
    if (!m_audioDriver->isOpened()) {
        return;
    }

    IAudioDriver::Spec activeSpec = m_audioDriver->activeSpec();
    rpcChannel()->send(rpc::make_request(Method::SetOutputSpec, RpcPacker::pack(activeSpec.output)));
}

void AudioDriverController::changeBufferSize(samples_t samples)
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return;
    }

    LOGI() << "Trying to change buffer size: " << samples;
    bool ok = m_audioDriver->setOutputDeviceBufferSize(samples);
    if (ok) {
        configuration()->setDriverBufferSize(samples);
        updateOutputSpec();
    }
}

void AudioDriverController::changeSampleRate(sample_rate_t sampleRate)
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return;
    }

    LOGI() << "Trying to change sample rate: " << sampleRate;

    bool ok = m_audioDriver->setOutputDeviceSampleRate(sampleRate);
    if (ok) {
        configuration()->setSampleRate(sampleRate);
        updateOutputSpec();
    }
}

std::string AudioDriverController::currentAudioApi() const
{
    return configuration()->currentAudioApi();
}

IAudioDriverPtr AudioDriverController::audioDriver() const
{
    DO_ASSERT(m_audioDriver);
    return m_audioDriver;
}

muse::async::Notification AudioDriverController::audioDriverChanged() const
{
    return configuration()->currentAudioApiChanged();
}
