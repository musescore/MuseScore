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

#include "wasapiaudioclient.h"

#include "log.h"

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media::Devices;
using namespace winrt::Windows::Devices::Enumeration;

WasapiAudioClient::WasapiAudioClient(HANDLE clientStartedEvent, HANDLE clientStoppedEvent)
    : m_clientStartedEvent(clientStartedEvent), m_clientStoppedEvent(clientStoppedEvent)
{
    check_hresult(MFStartup(MF_VERSION, MFSTARTUP_LITE));
}

WasapiAudioClient::~WasapiAudioClient()
{
    MFShutdown();
}

void WasapiAudioClient::setHardWareOffload(bool value)
{
    m_isHWOffload = value;
}

void WasapiAudioClient::setBackgroundAudio(bool value)
{
    m_isBackground = value;
}

void WasapiAudioClient::setRawAudio(bool value)
{
    m_isRawAudio = value;
}

void WasapiAudioClient::setLowLatency(bool value)
{
    m_isLowLatency = value;
}

void WasapiAudioClient::setBufferDuration(REFERENCE_TIME value)
{
    m_hnsBufferDuration = value;
}

void WasapiAudioClient::setSampleRequestCallback(SampleRequestCallback callback)
{
    m_sampleRequestCallback = callback;
}

unsigned int WasapiAudioClient::lowLatencyUpperBound() const
{
    //!Note See https://learn.microsoft.com/en-us/windows-hardware/drivers/audio/low-latency-audio

    static constexpr unsigned int LOWER_BOUND_SAMPLES_PER_CHANNEL = 1024;

    return LOWER_BOUND_SAMPLES_PER_CHANNEL;
}

unsigned int WasapiAudioClient::sampleRate() const
{
    if (!m_mixFormat.get()) {
        return 0;
    }

    return m_mixFormat.get()->nSamplesPerSec;
}

unsigned int WasapiAudioClient::channelCount() const
{
    if (!m_mixFormat.get()) {
        return 0;
    }

    return m_mixFormat.get()->nChannels;
}

DeviceInformationCollection WasapiAudioClient::availableDevices() const
{
    // Get the string identifier of the audio renderer
    hstring AudioSelector = MediaDevice::GetAudioRenderSelector();

    IAsyncOperation<DeviceInformationCollection> deviceRequest
        = DeviceInformation::FindAllAsync(AudioSelector, {});

    DeviceInformationCollection deviceInfoCollection = nullptr;

    try {
        deviceInfoCollection = deviceRequest.get();
    } catch (...) {
        LOGE() << to_string(hresult_error(to_hresult()).message());
    }

    return deviceInfoCollection;
}

hstring WasapiAudioClient::defaultDeviceId() const
{
    return MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);
}

void WasapiAudioClient::asyncInitializeAudioDevice(const hstring& deviceId) noexcept
{
    try {
        // This call can be made safely from a background thread because we are asking for the IAudioClient3
        // interface of an audio device. Async operation will call back to
        // IActivateAudioInterfaceCompletionHandler::ActivateCompleted, which must be an agile interface implementation
        m_deviceIdString = deviceId;

        com_ptr<IActivateAudioInterfaceAsyncOperation> asyncOp;
        check_hresult(ActivateAudioInterfaceAsync(m_deviceIdString.c_str(), __uuidof(IAudioClient3), nullptr, this, asyncOp.put()));
    } catch (...) {
        setStateAndNotify(DeviceState::Error, to_hresult());
    }
}

//
//  ActivateCompleted()
//
//  Callback implementation of ActivateAudioInterfaceAsync function.  This will be called on MTA thread
//  when results of the activation are available.
//
HRESULT WasapiAudioClient::ActivateCompleted(IActivateAudioInterfaceAsyncOperation* operation) noexcept
{
    try {
        if (m_deviceState != DeviceState::Uninitialized) {
            throw hresult_error(E_NOT_VALID_STATE);
        }

        // Check for a successful activation result
        HRESULT hrActivateResult = S_OK;
        com_ptr<::IUnknown> punkAudioInterface;
        check_hresult(operation->GetActivateResult(&hrActivateResult, punkAudioInterface.put()));
        check_hresult(hrActivateResult);

        // Remember that we have been activated, but don't raise the event yet.
        setState(DeviceState::Activated);

        // Get the pointer for the Audio Client
        m_audioClient = punkAudioInterface.as<IAudioClient3>();

        // Configure user defined properties
        check_hresult(configureDeviceInternal());

        // Initialize the AudioClient in Shared Mode with the user specified buffer
        if (!m_isLowLatency) {
            check_hresult(m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                                    AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                                    m_hnsBufferDuration,
                                                    m_hnsBufferDuration,
                                                    m_mixFormat.get(),
                                                    nullptr));
        } else {
            check_hresult(m_audioClient->InitializeSharedAudioStream(AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                                                     m_minPeriodInFrames,
                                                                     m_mixFormat.get(),
                                                                     nullptr));
        }

        // Get the maximum size of the AudioClient Buffer
        check_hresult(m_audioClient->GetBufferSize(&m_bufferFrames));

        // Get the render client
        m_audioRenderClient.capture(m_audioClient, &IAudioClient::GetService);

        // Create Async callback for sample events
        check_hresult(MFCreateAsyncResult(nullptr, &m_sampleReadyCallback, nullptr, m_sampleReadyAsyncResult.put()));

        // Sets the event handle that the system signals when an audio buffer is ready to be processed by the client
        check_hresult(m_audioClient->SetEventHandle(m_sampleReadyEvent.get()));

        // Everything succeeded
        setStateAndNotify(DeviceState::Initialized, S_OK);

        startPlayback();

        return S_OK;
    } catch (...) {
        setStateAndNotify(DeviceState::Error, to_hresult());

        m_audioClient = nullptr;
        m_audioRenderClient = nullptr;
        m_sampleReadyAsyncResult = nullptr;

        // Must return S_OK even on failure.
        return S_OK;
    }
}

//
//  GetBufferFramesPerPeriod()
//
//  Get the time in seconds between passes of the audio device
//
UINT32 WasapiAudioClient::getBufferFramesPerPeriod() noexcept
{
    REFERENCE_TIME defaultDevicePeriod = 0;
    REFERENCE_TIME minimumDevicePeriod = 0;

    if (m_isHWOffload) {
        return m_bufferFrames;
    }

    if (m_isLowLatency) {
        return m_minPeriodInFrames;
    }

    // Get the audio device period
    HRESULT hr = m_audioClient->GetDevicePeriod(&defaultDevicePeriod, &minimumDevicePeriod);
    if (FAILED(hr)) {
        return 0;
    }

    double devicePeriodInSeconds = defaultDevicePeriod / (10000.0 * 1000.0);
    return static_cast<UINT32>(m_mixFormat->nSamplesPerSec * devicePeriodInSeconds + 0.5);
}

//
//  ConfigureDeviceInternal()
//
//  Sets additional playback parameters and opts into hardware offload
//
HRESULT WasapiAudioClient::configureDeviceInternal() noexcept
{
    try {
        if (m_deviceState != DeviceState::Activated) {
            return E_NOT_VALID_STATE;
        }

        // Opt into HW Offloading.  If the endpoint does not support offload it will return AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE
        AudioClientProperties audioProps = { 0 };
        audioProps.cbSize = sizeof(AudioClientProperties);
        audioProps.bIsOffload = m_isHWOffload;
        audioProps.eCategory = AudioCategory_Media;

        if (m_isRawAudio) {
            audioProps.Options = AUDCLNT_STREAMOPTIONS_RAW;
        }

        check_hresult(m_audioClient->SetClientProperties(&audioProps));

        // If application already has a preferred source format available,
        // it can test whether the format is supported by the device:
        //
        // unique_cotaskmem_ptr<WAVEFORMATEX> applicationFormat = { ... };
        // if (S_OK == m_AudioClient->IsFormatSupported(applicationFormat.get()))
        // {
        //     m_MixFormat = std::move(applicationFormat);
        // }
        // else
        // {
        //     // device does not support the application format, so ask the device what format it prefers
        //     check_hresult(m_AudioClient->GetMixFormat(&m_MixFormat.put()));
        // }

        // This sample opens the device is shared mode so we need to find the supported WAVEFORMATEX mix format
        check_hresult(m_audioClient->GetMixFormat(m_mixFormat.put()));

        if (!audioProps.bIsOffload) {
            // The wfx parameter below is optional (Its needed only for MATCH_FORMAT clients). Otherwise, wfx will be assumed
            // to be the current engine format based on the processing mode for this stream
            check_hresult(m_audioClient->GetSharedModeEnginePeriod(m_mixFormat.get(), &m_defaultPeriodInFrames,
                                                                   &m_fundamentalPeriodInFrames,
                                                                   &m_minPeriodInFrames, &m_maxPeriodInFrames));
        }

        return S_OK;
    } catch (...) {
        return to_hresult();
    }
}

//
//  ValidateBufferValue()
//
//  Verifies the user specified buffer value for hardware offload
//  Throws an exception on failure.
//
void WasapiAudioClient::validateBufferValue()
{
    if (!m_isHWOffload) {
        // If we aren't using HW Offload, set this to 0 to use the default value
        m_hnsBufferDuration = 0;
        return;
    }

    REFERENCE_TIME hnsMinBufferDuration;
    REFERENCE_TIME hnsMaxBufferDuration;

    check_hresult(m_audioClient->GetBufferSizeLimits(m_mixFormat.get(), true, &hnsMinBufferDuration, &hnsMaxBufferDuration));
    if (m_hnsBufferDuration < hnsMinBufferDuration) {
        // using MINIMUM size instead
        m_hnsBufferDuration = hnsMinBufferDuration;
    } else if (m_hnsBufferDuration > hnsMaxBufferDuration) {
        // using MAXIMUM size instead
        m_hnsBufferDuration = hnsMaxBufferDuration;
    }
}

//
//  StartPlaybackAsync()
//
//  Starts asynchronous playback on a separate thread via MF Work Item
//  Errors are reported via the DeviceStateChanged event.
//
void WasapiAudioClient::startPlayback() noexcept
{
    try {
        switch (m_deviceState) {
        // We should be stopped if the user stopped playback, or we should be
        // initialized if this is the first time through getting ready to playback.
        case DeviceState::Stopped:
        case DeviceState::Initialized:
            setStateAndNotify(DeviceState::Starting, S_OK);
            check_hresult(MFPutWorkItem2(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, 0, &m_startPlaybackCallback, nullptr));
            break;

        default:
            // Otherwise something else happened
            throw hresult_error(E_FAIL);
        }
    } catch (...) {
        hresult error = to_hresult();
        setStateAndNotify(DeviceState::Error, error);
    }
}

//
//  OnStartPlayback()
//
//  Callback method to start playback
//
HRESULT WasapiAudioClient::onStartPlayback(IMFAsyncResult*) noexcept
{
    try {
        // Pre-Roll the buffer with silence
        onAudioSampleRequested(true);

        // Set the initial volume.
        //SetAudioClientChannelVolume();

        // Actually start the playback
        check_hresult(m_audioClient->Start());

        setStateAndNotify(DeviceState::Playing, S_OK);
        check_hresult(MFPutWaitingWorkItem(m_sampleReadyEvent.get(), 0, m_sampleReadyAsyncResult.get(), &m_sampleReadyKey));

        SetEvent(m_clientStartedEvent);

        return S_OK;
    } catch (...) {
        setStateAndNotify(DeviceState::Error, to_hresult());
        // Must return S_OK.
        return S_OK;
    }
}

//
//  StopPlaybackAsync()
//
//  Stop playback asynchronously via MF Work Item
//
HRESULT WasapiAudioClient::stopPlaybackAsync() noexcept
{
    if ((m_deviceState != DeviceState::Playing)
        && (m_deviceState != DeviceState::Paused)
        && (m_deviceState != DeviceState::Error)) {
        return E_NOT_VALID_STATE;
    }

    setStateAndNotify(DeviceState::Stopping, S_OK);

    return MFPutWorkItem2(MFASYNC_CALLBACK_QUEUE_MULTITHREADED, 0, &m_stopPlaybackCallback, nullptr);
}

//
//  OnStopPlayback()
//
//  Callback method to stop playback
//
HRESULT WasapiAudioClient::onStopPlayback(IMFAsyncResult*)
{
    // Stop playback by cancelling Work Item
    // Cancel the queued work item (if any)

    if (0 != m_sampleReadyKey) {
        MFCancelWorkItem(m_sampleReadyKey);
        m_sampleReadyKey = 0;
    }

    // Flush anything left in buffer with silence, best effort.
    try {
        onAudioSampleRequested(true);
    } catch (...) { }

    m_audioClient->Stop();
    m_sampleReadyAsyncResult = nullptr;

    setStateAndNotify(DeviceState::Stopped, S_OK);

    SetEvent(m_clientStoppedEvent);

    return S_OK;
}

//
//  OnSampleReady()
//
//  Callback method when ready to fill sample buffer
//
HRESULT WasapiAudioClient::onSampleReady(IMFAsyncResult*)
{
    try {
        onAudioSampleRequested(false);

        // Re-queue work item for next sample
        if (m_deviceState == DeviceState::Playing) {
            check_hresult(MFPutWaitingWorkItem(m_sampleReadyEvent.get(), 0, m_sampleReadyAsyncResult.get(), &m_sampleReadyKey));
        }

        return S_OK;
    } catch (...) {
        hresult error = to_hresult();
        setStateAndNotify(DeviceState::Error, error);
        return error;
    }
}

//
//  OnAudioSampleRequested()
//
//  Called when audio device fires m_SampleReadyEvent
//
void WasapiAudioClient::onAudioSampleRequested(bool IsSilence)
{
    try {
        auto guard = slim_lock_guard(m_lock);

        // Get padding in existing buffer
        UINT32 PaddingFrames = 0;
        check_hresult(m_audioClient->GetCurrentPadding(&PaddingFrames));

        // GetCurrentPadding represents the number of queued frames
        // so we can subtract that from the overall number of frames we have
        uint32_t framesAvailable = m_bufferFrames - PaddingFrames;

        // Only continue if we have buffer to write data
        if (framesAvailable == 0) {
            return;
        }

        if (IsSilence) {
            // Fill the buffer with silence
            uint8_t* data;
            check_hresult(m_audioRenderClient->GetBuffer(framesAvailable, &data));
            check_hresult(m_audioRenderClient->ReleaseBuffer(framesAvailable, AUDCLNT_BUFFERFLAGS_SILENT));
            return;
        }

        // Even if we cancel a work item, this may still fire due to the async
        // nature of things.  There should be a queued work item already to handle
        // the process of stopping or stopped
        if (m_deviceState == DeviceState::Playing) {
            // Fill the buffer with a playback sample
            getSamples(framesAvailable);
        }
    }
    catch (hresult_error const& error)
    {
        if (error.code() != AUDCLNT_E_RESOURCES_INVALIDATED) {
            throw;
        }

        // Attempt auto-recovery from loss of resources.
        setState(DeviceState::Uninitialized);
        m_audioClient = nullptr;
        m_audioRenderClient = nullptr;
        m_sampleReadyAsyncResult = nullptr;

        asyncInitializeAudioDevice(defaultDeviceId());
    }
}

void WasapiAudioClient::getSamples(uint32_t framesAvailable)
{
    uint8_t* data;

    uint32_t actualFramesToRead = framesAvailable;
    uint32_t actualBytesToRead = actualFramesToRead * m_mixFormat->nBlockAlign;

    check_hresult(m_audioRenderClient->GetBuffer(actualFramesToRead, &data));
    if (actualBytesToRead > 0) {
        m_sampleRequestCallback(nullptr, data, actualBytesToRead);
    }
    check_hresult(m_audioRenderClient->ReleaseBuffer(actualFramesToRead, 0));
}

void WasapiAudioClient::setState(const DeviceState newState)
{
    if (m_deviceState == newState) {
        return;
    }

    m_deviceState = newState;
}

void WasapiAudioClient::setStateAndNotify(const DeviceState newState, hresult resultCode)
{
    if (m_deviceState == newState) {
        return;
    }

    std::string errMsg;

    switch (resultCode) {
    case AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE:
        errMsg = "ERROR: Endpoint Does Not Support HW Offload";
        break;

    case AUDCLNT_E_RESOURCES_INVALIDATED:
        errMsg = "ERROR: Endpoint Lost Access To Resources";
        break;

    case S_OK:
        break;

    default:
        errMsg = "ERROR: " + to_string(hresult_error(resultCode).message());
        break;
    }

    LOGE() << errMsg;

    m_deviceState = newState;
}
