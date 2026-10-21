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

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::rpc;

std::vector<std::string> AudioDriverController::availableAudioApiList() const
{
    return audioDriverFactory()->availableAudioDrivers();
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
            async::Async::call(this, [this]() {
                LOGI() << "Available output devices changed, checking connection...";
                handleOutputDeviceChange();
                m_availableOutputDevicesChanged.notify();
            });
        });

        m_audioDriver->activeSpecChanged().onReceive(this, [this](const IAudioDriver::Spec& spec) {
            if (spec.isValid()) {
                m_lastValidSpec = spec;
            } else {
                LOGW() << "Driver reported an invalid spec";
            }

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

std::string AudioDriverController::currentAudioApi() const
{
    return configuration()->currentAudioApi();
}

void AudioDriverController::changeCurrentAudioApi(const std::string& name)
{
    IF_ASSERT_FAILED(m_audioDriver) {
        return;
    }

    if (m_audioDriver->isOpened()) {
        m_audioDriver->close();
    }

    IAudioDriverPtr driver = audioDriverFactory()->createDriver(name);
    setNewDriver(driver);
    m_audioDriver->init();
    LOGI() << "Used audio driver: " << m_audioDriver->name();

    // reset to default
    IAudioDriver::Spec spec;
    spec.output = configuration()->defaultOutputSpec();
    spec.callback = m_callback;

    if (m_audioDriver && !m_audioDriver->defaultDevice().empty()) {
        spec.deviceId = DEFAULT_DEVICE_ID;
        bool ok = m_audioDriver->open(spec, nullptr);
        if (!ok) {
            LOGE() << "Failed to open audio driver: " << name;
        }
    } else {
        LOGW() << "No devices for " << name;
    }

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
    m_callback = spec.callback;

    const std::string currentAudioApi = configuration()->currentAudioApi();
    IAudioDriverPtr driver = audioDriverFactory()->createDriver(currentAudioApi);
    driver->init();
    setNewDriver(driver);

    LOGI() << "Trying to open audio driver: " << m_audioDriver->name() << ", " << spec;
    bool ok = m_audioDriver->open(spec, activeSpec);
    if (!ok) {
        // reset to default device
        IAudioDriver::Spec defaultDeviceSpec = spec;
        defaultDeviceSpec.deviceId = m_audioDriver->defaultDevice();
        LOGW() << "Failed to open device: " << spec.deviceId << ", falling back to default: " << defaultDeviceSpec.deviceId;
        ok = m_audioDriver->open(defaultDeviceSpec, activeSpec);
    }

    if (!ok) {
        ok = switchToDefaultAudioDriver(activeSpec);
    }

    if (ok) {
        LOGI() << "Opened audio driver: " << m_audioDriver->name() << ", " << m_audioDriver->activeSpec();
    } else {
        LOGE() << "Failed to open any audio driver, last tried: " << m_audioDriver->name();
    }

    return ok;
}

void AudioDriverController::close()
{
    if (m_audioDriver) {
        m_audioDriver->close();
    }
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

    // Audio has never been started yet
    if (!m_audioDriver->isOpened() && !m_lastValidSpec.isValid()) {
        configuration()->setAudioOutputDeviceId(deviceId);
        return true;
    }

    IAudioDriver::Spec spec;
    spec.deviceId = deviceId;
    spec.callback = m_callback;
    spec.output = configuration()->desiredOutputSpec();

    LOGI() << "Trying to change output device to " << spec;

    return reopenWithSpec(spec);
}

async::Notification AudioDriverController::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

void AudioDriverController::handleOutputDeviceChange()
{
    if (!m_audioDriver->isOpened() && !m_retryOpenDevice) {
        return;
    }

    // Some drivers reset their internal spec to a blank state after a failed open attempt, so fall
    // back to the last spec that was actually confirmed to work
    IAudioDriver::Spec spec = m_audioDriver->activeSpec();
    if (!spec.isValid()) {
        if (m_lastValidSpec.isValid()) {
            LOGW() << "Active spec is invalid, using last valid spec";
            spec = m_lastValidSpec;
        } else {
            LOGW() << "Active spec is invalid, using default spec";
            spec.deviceId = DEFAULT_DEVICE_ID;
            spec.output = configuration()->defaultOutputSpec();
            spec.callback = m_callback;
        }
    }

    LOGI() << "Reopening output device, " << spec;

    m_audioDriver->close();
    bool ok = m_audioDriver->open(spec, nullptr);
    if (!ok) {
        // reset to default device
        LOGW() << "Failed to reopen device: " << spec.deviceId << ", falling back to default";
        spec.deviceId = DEFAULT_DEVICE_ID;
        ok = m_audioDriver->open(spec, nullptr);
        if (!ok) {
            LOGE() << "Failed to reopen default device on " << m_audioDriver->name() << ", switching to default audio driver";
            ok = switchToDefaultAudioDriver();
        }
    }

    if (ok) {
        LOGI() << "Reopened output device: " << m_audioDriver->activeSpec();
    } else {
        LOGE() << "Failed to reopen output device, will retry on next device list change";
    }

    m_retryOpenDevice = !ok;
}

bool AudioDriverController::switchToDefaultAudioDriver(IAudioDriver::Spec* activeSpec)
{
    const std::string defaultAudioApi = configuration()->defaultAudioApi();
    const std::string currentAudioApi = configuration()->currentAudioApi();

    if (defaultAudioApi == currentAudioApi) {
        LOGE() << "Already on the default audio driver: " << defaultAudioApi << ", cannot fall back further";
        return false;
    }

    LOGW() << "Switching from " << currentAudioApi << " to default audio driver: " << defaultAudioApi;

    IAudioDriverPtr defaultDriver = audioDriverFactory()->createDriver(defaultAudioApi);
    defaultDriver->init();
    setNewDriver(defaultDriver);

    IAudioDriver::Spec defSpec;
    defSpec.deviceId = DEFAULT_DEVICE_ID;
    defSpec.output = configuration()->defaultOutputSpec();
    defSpec.callback = m_callback;

    bool ok = m_audioDriver->open(defSpec, activeSpec);
    if (ok) {
        configuration()->setCurrentAudioApi(defaultAudioApi);
        m_currentAudioApiChanged.notify();
        LOGI() << "Successfully switched to default audio driver: " << defaultAudioApi;
    } else {
        LOGE() << "Failed to open default audio driver: " << defaultAudioApi;
    }

    return ok;
}

bool AudioDriverController::reopenWithSpec(const IAudioDriver::Spec& spec)
{
    // A failed reopen can blank the driver's spec
    IAudioDriver::Spec oldSpec = m_audioDriver->activeSpec();
    if (!oldSpec.isValid()) {
        oldSpec = m_lastValidSpec;
    }

    m_audioDriver->close();
    if (m_audioDriver->open(spec, nullptr)) {
        LOGI() << "Reopened with: " << spec;
        m_retryOpenDevice = false;
        return true;
    }

    LOGE() << "Failed to reopen with: " << spec << ", returning to: " << oldSpec;
    if (m_audioDriver->open(oldSpec, nullptr)) {
        LOGI() << "Restored previous spec: " << oldSpec;
        m_retryOpenDevice = false;
    } else {
        LOGE() << "Failed to restore previous spec: " << oldSpec << ", will retry on next device list change";
        m_retryOpenDevice = true;
    }

    return false;
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
        spec.output.samplesPerChannel = samples;
        ok = reopenWithSpec(spec);
    }

    if (ok) {
        updateOutputSpec();
        configuration()->setDriverBufferSize(samples);
        m_outputDeviceBufferSizeChanged.notify();
    } else {
        LOGE() << "Failed to change buffer size to: " << samples;
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
        spec.output.sampleRate = sampleRate;
        ok = reopenWithSpec(spec);
    }

    if (ok) {
        updateOutputSpec();
        configuration()->setSampleRate(sampleRate);
        m_outputDeviceSampleRateChanged.notify();
    } else {
        LOGE() << "Failed to change sample rate to: " << sampleRate;
    }
}

async::Notification AudioDriverController::outputDeviceSampleRateChanged() const
{
    return m_outputDeviceSampleRateChanged;
}
