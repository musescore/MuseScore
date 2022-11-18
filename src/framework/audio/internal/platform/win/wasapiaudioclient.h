/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#ifndef MU_AUDIO_WASAPIAUDIOCLIENT_H
#define MU_AUDIO_WASAPIAUDIOCLIENT_H

#include "wasapitypes.h"

namespace winrt {
struct WasapiAudioClient : implements<WasapiAudioClient, IActivateAudioInterfaceCompletionHandler>
{
public:
    WasapiAudioClient(HANDLE clientStartedEvent, HANDLE clientStoppedEvent);
    ~WasapiAudioClient();

    void setHardWareOffload(bool value) { m_isHWOffload = value; }
    void setBackgroundAudio(bool value) { m_isBackground = value; }
    void setRawAudio(bool value) { m_isRawAudio = value; }
    void setLowLatency(bool value) { m_isLowLatency = value; }
    void setBufferDuration(REFERENCE_TIME value) { m_hnsBufferDuration = value; }
    void setSampleRequestCallback(SampleRequestCallback callback);

    unsigned int sampleRate() const;
    unsigned int channelCount() const;

    Windows::Devices::Enumeration::DeviceInformationCollection availableDevices() const;
    hstring defaultDeviceId() const;

    void asyncInitializeAudioDevice(const hstring& deviceId) noexcept;
    void startPlayback() noexcept;
    HRESULT stopPlaybackAsync() noexcept;

    // IActivateAudioInterfaceCompletionHandler
    STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation * operation) noexcept;
private:
    HRESULT onStartPlayback(IMFAsyncResult* pResult) noexcept;
    HRESULT onStopPlayback(IMFAsyncResult* pResult);
    HRESULT onSampleReady(IMFAsyncResult* pResult);

    EmbeddedMFAsyncCallback<&WasapiAudioClient::onStartPlayback> m_startPlaybackCallback{ this };
    EmbeddedMFAsyncCallback<&WasapiAudioClient::onStopPlayback> m_stopPlaybackCallback{ this };
    EmbeddedMFAsyncCallback<&WasapiAudioClient::onSampleReady> m_sampleReadyCallback{ this };

    HRESULT configureDeviceInternal() noexcept;
    void validateBufferValue();
    void onAudioSampleRequested(bool IsSilence = false);
    UINT32 getBufferFramesPerPeriod() noexcept;

    void getSamples(uint32_t framesAvailable);
    void setState(const DeviceState newState);
    void setStateAndNotify(const DeviceState newState, hresult resultCode);

    hstring m_deviceIdString;
    uint32_t m_bufferFrames = 0;

    // Event for sample ready or user stop
    handle m_sampleReadyEvent{ check_pointer(CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS)) };

    MFWORKITEM_KEY m_sampleReadyKey;
    mutable slim_mutex m_lock;

    mutable unique_cotaskmem_ptr<WAVEFORMATEX> m_mixFormat;
    uint32_t m_defaultPeriodInFrames;
    uint32_t m_fundamentalPeriodInFrames;
    uint32_t m_maxPeriodInFrames;
    uint32_t m_minPeriodInFrames;

    com_ptr<IAudioClient3> m_audioClient;
    com_ptr<IAudioRenderClient> m_audioRenderClient;
    com_ptr<IMFAsyncResult> m_sampleReadyAsyncResult;

    bool m_isHWOffload = false;
    bool m_isBackground = false;
    bool m_isRawAudio = false;
    bool m_isLowLatency = false;
    REFERENCE_TIME m_hnsBufferDuration = 0;

    DeviceState m_deviceState = DeviceState::Uninitialized;
    SampleRequestCallback m_sampleRequestCallback;
    HANDLE m_clientStartedEvent;
    HANDLE m_clientStoppedEvent;
};
}

#endif // MU_AUDIO_WASAPIAUDIOCLIENT_H
