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
        m_audioDriver->outputDeviceChanged().disconnect(this);
        m_audioDriver->outputDeviceBufferSizeChanged().disconnect(this);
        m_audioDriver->outputDeviceSampleRateChanged().disconnect(this);
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
        });

        m_audioDriver->outputDeviceChanged().onNotify(this, [this]() {
            m_outputDeviceChanged.notify();
        });

        m_audioDriver->outputDeviceBufferSizeChanged().onNotify(this, [this]() {
            m_outputDeviceBufferSizeChanged.notify();
        });

        m_audioDriver->outputDeviceSampleRateChanged().onNotify(this, [this]() {
            m_outputDeviceSampleRateChanged.notify();
        });
    }
}

void AudioDriverController::init()
{
}

std::string AudioDriverController::currentAudioApi() const
{
    return configuration()->currentAudioApi();
}

void AudioDriverController::changeCurrentAudioApi(const std::string& name)
{
    IAudioDriver::Spec activeSpec;
    if (m_audioDriver && m_audioDriver->isOpened()) {
        activeSpec = m_audioDriver->activeSpec();
        m_audioDriver->close();
    }

    IAudioDriverPtr driver = createDriver(name);
    driver->init();
    setNewDriver(driver);
    m_audioDriver->resetToDefaultOutputDevice();
    LOGI() << "Used " << m_audioDriver->name() << " audio driver";

    if (activeSpec.isValid()) {
        activeSpec.output.samplesPerChannel = DEFAULT_BUFFER_SIZE;
        m_audioDriver->open(activeSpec, &activeSpec);
    }

    updateOutputSpec();

    configuration()->setCurrentAudioApi(name);
    configuration()->setAudioOutputDeviceId(m_audioDriver->outputDevice());
    m_currentAudioApiChanged.notify();
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

    const std::string audioOutputDeviceId = configuration()->audioOutputDeviceId();
    m_audioDriver->selectOutputDevice(audioOutputDeviceId);

    bool ok = m_audioDriver->open(spec, activeSpec);
    if (!ok) {
        m_audioDriver->resetToDefaultOutputDevice();
        ok = m_audioDriver->open(spec, activeSpec);
        if (ok) {
            configuration()->setAudioOutputDeviceId(m_audioDriver->outputDevice());
        }
    }

    if (!ok) {
        const std::string defaultAudioApi = configuration()->defaultAudioApi();
        if (defaultAudioApi != currentAudioApi) {
            IAudioDriverPtr defaultDriver = createDriver(defaultAudioApi);
            defaultDriver->init();
            setNewDriver(defaultDriver);
            m_audioDriver->resetToDefaultOutputDevice();
            ok = m_audioDriver->open(spec, activeSpec);
            if (ok) {
                configuration()->setCurrentAudioApi(defaultAudioApi);
                configuration()->setAudioOutputDeviceId(m_audioDriver->outputDevice());
            }
        }
    }

    LOGI() << "Used audio driver: " << m_audioDriver->name()
           << ", opened: " << (ok ? "success" : "failed")
           << ", device: " << m_audioDriver->outputDevice();

    updateOutputSpec();

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
    return m_audioDriver->outputDevice();
}

bool AudioDriverController::selectOutputDevice(const std::string& deviceId)
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return false;
    }

    const AudioDeviceID oldDeviceId = m_audioDriver->outputDevice();
    LOGI() << "Trying to change output device"
           << " from: " << oldDeviceId
           << ", to: " << deviceId;
    bool ok = m_audioDriver->selectOutputDevice(deviceId);
    if (ok) {
        configuration()->setAudioOutputDeviceId(deviceId);
        updateOutputSpec();
    } else {
        LOGE() << "failed select device, return to old: " << oldDeviceId;
        m_audioDriver->selectOutputDevice(oldDeviceId);
    }
    return ok;
}

async::Notification AudioDriverController::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
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

std::vector<unsigned int> AudioDriverController::availableOutputDeviceBufferSizes() const
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
    bool ok = m_audioDriver->setOutputDeviceBufferSize(samples);
    if (ok) {
        configuration()->setDriverBufferSize(samples);
        updateOutputSpec();
    }
}

async::Notification AudioDriverController::outputDeviceBufferSizeChanged() const
{
    return m_outputDeviceBufferSizeChanged;
}

std::vector<unsigned int> AudioDriverController::availableOutputDeviceSampleRates() const
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

    LOGI() << "Trying to change sample rate: " << sampleRate;

    bool ok = m_audioDriver->setOutputDeviceSampleRate(sampleRate);
    if (ok) {
        configuration()->setSampleRate(sampleRate);
        updateOutputSpec();
    }
}

async::Notification AudioDriverController::outputDeviceSampleRateChanged() const
{
    return m_outputDeviceSampleRateChanged;
}
