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

#include "global/async/async.h"

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

using namespace muse;
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

void AudioDriverController::setNewDriver(IAudioDriverPtr newDriver)
{
    if (m_audioDriver) {
        // unsubscribe
        m_audioDriver->availableOutputDevicesChanged().disconnect(this);
        m_audioDriver->activeSpecChanged().disconnect(this);
    }

    m_audioDriver = newDriver;

    if (m_audioDriver) {
        // subscribe
        m_audioDriver->availableOutputDevicesChanged().onNotify(this, [this]() {
            m_availableOutputDevicesChanged.notify();
            LOGI() << "Available output devices changed, checking connection...";
            checkOutputDevice();
        });

        m_audioDriver->activeSpecChanged().onReceive(this, [this](const IAudioDriver::Spec& spec) {
            m_activeSpecChanged.send(spec);

            async::Async::call(this, [this, spec]() {
                configuration()->setAudioOutputDeviceId(spec.deviceId);

                m_outputDeviceChanged.notify();
                m_outputDeviceBufferSizeChanged.notify();
                m_outputDeviceSampleRateChanged.notify();

                updateOutputSpec();
            });
        });
    }
}

IAudioDriver::Spec AudioDriverController::defaultSpec() const
{
    IAudioDriver::Spec spec;
    spec.deviceId = m_audioDriver ? m_audioDriver->defaultDevice() : AudioDeviceID();
    spec.output.audioChannelCount = 2;
    spec.output.sampleRate = 44100;
    spec.output.samplesPerChannel = DEFAULT_BUFFER_SIZE;
    return spec;
}

std::string AudioDriverController::currentAudioApi() const
{
    return configuration()->currentAudioApi();
}

void AudioDriverController::changeCurrentAudioApi(const std::string& name)
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return;
    }

    IAudioDriver::Spec oldSpec = m_audioDriver->activeSpec();
    if (m_audioDriver->isOpened()) {
        m_audioDriver->close();
    }

    IAudioDriverPtr driver = createDriver(name);
    driver->init();
    setNewDriver(driver);
    LOGI() << "Used " << m_audioDriver->name() << " audio driver";

    // reset to default
    IAudioDriver::Spec spec = defaultSpec();
    spec.callback = oldSpec.callback;
    m_audioDriver->open(spec, nullptr);

    configuration()->setCurrentAudioApi(name);
    m_currentAudioApiChanged.notify();
    m_availableOutputDevicesChanged.notify();
}

async::Notification AudioDriverController::currentAudioApiChanged() const
{
    return m_currentAudioApiChanged;
}

AudioDeviceList AudioDriverController::availableOutputDevices() const
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return {};
    }
    return m_audioDriver->availableOutputDevices();
}

async::Notification AudioDriverController::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}

bool AudioDriverController::open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec)
{
    const std::string currentAudioApi = configuration()->currentAudioApi();
    IAudioDriverPtr driver = createDriver(currentAudioApi);
    driver->init();
    setNewDriver(driver);

    bool ok = m_audioDriver->open(spec, activeSpec);
    if (!ok) {
        // reset to default device
        IAudioDriver::Spec defaultDeviceSpec = spec;
        defaultDeviceSpec.deviceId = m_audioDriver->defaultDevice();
        ok = m_audioDriver->open(defaultDeviceSpec, activeSpec);
    }

    if (!ok) {
        const std::string defaultAudioApi = configuration()->defaultAudioApi();
        if (defaultAudioApi != currentAudioApi) {
            IAudioDriverPtr defaultDriver = createDriver(defaultAudioApi);
            defaultDriver->init();
            setNewDriver(defaultDriver);
            // reset to default
            IAudioDriver::Spec defSpec = defaultSpec();
            defSpec.callback = spec.callback;
            ok = m_audioDriver->open(defSpec, activeSpec);
            if (ok) {
                configuration()->setCurrentAudioApi(defaultAudioApi);
            }
        }
    }

    LOGI() << "Used audio driver: " << m_audioDriver->name()
           << ", opened: " << (ok ? "success" : "failed")
           << ", device: " << m_audioDriver->activeSpec().deviceId;

    return ok;
}

void AudioDriverController::close()
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return;
    }
    m_audioDriver->close();
}

bool AudioDriverController::isOpened() const
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return false;
    }
    return m_audioDriver->isOpened();
}

const IAudioDriver::Spec& AudioDriverController::activeSpec() const
{
    IF_ASSERT_FAILED(m_audioDriver) {
        static IAudioDriver::Spec dummy;
        return dummy;
    }
    return m_audioDriver->activeSpec();
}

async::Channel<IAudioDriver::Spec> AudioDriverController::activeSpecChanged() const
{
    return m_activeSpecChanged;
}

AudioDeviceID AudioDriverController::outputDevice() const
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return AudioDeviceID();
    }
    return m_audioDriver->activeSpec().deviceId;
}

bool AudioDriverController::selectOutputDevice(const AudioDeviceID& deviceId)
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return false;
    }

    if (!m_audioDriver->isOpened()) {
        configuration()->setAudioOutputDeviceId(deviceId);
        return true;
    }

    const IAudioDriver::Spec oldSpec = m_audioDriver->activeSpec();
    LOGI() << "Trying to change output device"
           << " from: " << oldSpec.deviceId
           << ", to: " << deviceId;

    IAudioDriver::Spec spec = defaultSpec();
    spec.deviceId = deviceId;
    spec.callback = oldSpec.callback;

    m_audioDriver->close();
    bool ok = m_audioDriver->open(spec, nullptr);
    if (!ok) {
        LOGE() << "failed select device, return to old: " << oldSpec.deviceId;
        m_audioDriver->open(oldSpec, nullptr);
    }
    return ok;
}

async::Notification AudioDriverController::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

void AudioDriverController::checkOutputDevice()
{
    if (!m_audioDriver->isOpened()) {
        return;
    }

    IAudioDriver::Spec spec = m_audioDriver->activeSpec();
    m_audioDriver->close();
    bool ok = m_audioDriver->open(spec, nullptr);
    if (!ok) {
        // reset to default device
        spec.deviceId = m_audioDriver->defaultDevice();
        m_audioDriver->open(spec, nullptr);
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

std::vector<samples_t> AudioDriverController::availableOutputDeviceBufferSizes() const
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return {};
    }
    return m_audioDriver->availableOutputDeviceBufferSizes();
}

void AudioDriverController::changeBufferSize(samples_t samples)
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return;
    }

    LOGI() << "Trying to change buffer size: " << samples;
    bool ok = true;

    if (m_audioDriver->isOpened()) {
        IAudioDriver::Spec spec = m_audioDriver->activeSpec();
        m_audioDriver->close();
        spec.output.samplesPerChannel = samples;
        ok = m_audioDriver->open(spec, &spec);
    }

    if (ok) {
        updateOutputSpec();
        configuration()->setDriverBufferSize(samples);
        m_outputDeviceBufferSizeChanged.notify();
    }
}

async::Notification AudioDriverController::outputDeviceBufferSizeChanged() const
{
    return m_outputDeviceBufferSizeChanged;
}

std::vector<sample_rate_t> AudioDriverController::availableOutputDeviceSampleRates() const
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return {};
    }
    return m_audioDriver->availableOutputDeviceSampleRates();
}

void AudioDriverController::changeSampleRate(sample_rate_t sampleRate)
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return;
    }

    LOGI() << "Trying to change sampleRate: " << sampleRate;
    bool ok = true;

    if (m_audioDriver->isOpened()) {
        IAudioDriver::Spec spec = m_audioDriver->activeSpec();
        m_audioDriver->close();
        spec.output.sampleRate = sampleRate;
        ok = m_audioDriver->open(spec, &spec);
    }

    if (ok) {
        updateOutputSpec();
        configuration()->setSampleRate(sampleRate);
        m_outputDeviceSampleRateChanged.notify();
    }
}

async::Notification AudioDriverController::outputDeviceSampleRateChanged() const
{
    return m_outputDeviceSampleRateChanged;
}
