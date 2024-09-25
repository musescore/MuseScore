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

WasapiAudioClient::WasapiAudioClient(HANDLE clientStartedEvent, HANDLE clientFailedToStartEvent, HANDLE clientStoppedEvent)
    : m_clientStartedEvent(clientStartedEvent), m_clientFailedToStartEvent(clientFailedToStartEvent), m_clientStoppedEvent(
        clientStoppedEvent)
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

unsigned int WasapiAudioClient::minPeriodInFrames() const
{
    return m_minPeriodInFrames;
}

void WasapiAudioClient::setFallbackDevice(const hstring& deviceId)
{
    m_fallbackDeviceIdString = deviceId;
}

void WasapiAudioClient::asyncInitializeAudioDevice(const hstring& deviceId, bool useClosestSupportedFormat) noexcept
{
    try {
        // This call can be made safely from a background thread because we are asking for the IAudioClient3
        // interface of an audio device. Async operation will call back to
        // IActivateAudioInterfaceCompletionHandler::ActivateCompleted, which must be an agile interface implementation
        m_deviceIdString = deviceId;

        m_useClosestSupportedFormat = useClosestSupportedFormat;
        m_deviceState = DeviceState::Uninitialized;

        com_ptr<IActivateAudioInterfaceAsyncOperation> asyncOp;
        check_hresult(ActivateAudioInterfaceAsync(m_deviceIdString.c_str(), __uuidof(IAudioClient3), nullptr, this, asyncOp.put()));
    } catch (...) {
        setStateAndNotify(DeviceState::Error, to_hresult());
    }
}

static void logWAVEFORMATEX(WAVEFORMATEX* format)
{
    LOGI() << "Format tag: " << format->wFormatTag;
    LOGI() << "Channels: " << format->nChannels;
    LOGI() << "Sample rate: " << format->nSamplesPerSec;
    LOGI() << "Average bytes per second: " << format->nAvgBytesPerSec;
    LOGI() << "Block align: " << format->nBlockAlign;
    LOGI() << "Bits per sample: " << format->wBitsPerSample;
    LOGI() << "cbSize: " << format->cbSize;
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
                                                    AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
                                                    m_hnsBufferDuration,
                                                    0,
                                                    m_mixFormat.get(),
                                                    nullptr));
        } else {
            check_hresult(m_audioClient->InitializeSharedAudioStream(AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                                                     m_minPeriodInFrames,
                                                                     m_mixFormat.get(),
                                                                     nullptr));
        }

        LOGI() << "Initialized WASAPI audio endpoint with: ";
        logWAVEFORMATEX(m_mixFormat.get());
        LOGI() << "HnsBufferDuration: " << m_hnsBufferDuration;
        LOGI() << "Minimal period in frames: " << m_minPeriodInFrames;
        LOGI() << "Default period in frames: " << m_defaultPeriodInFrames;
        LOGI() << "Fundamental period in frames: " << m_fundamentalPeriodInFrames;
        LOGI() << "Max period in frames: " << m_maxPeriodInFrames;
        LOGI() << "Min period in frames: " << m_minPeriodInFrames;

        // Get the maximum size of the AudioClient Buffer
        check_hresult(m_audioClient->GetBufferSize(&m_bufferFrames));
        LOGI() << "Buffer size: " << m_bufferFrames;

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

        SetEvent(m_clientFailedToStartEvent);

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

        LOGI() << "WASAPI: Settings device client properties";
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
        LOGI() << "WASAPI: Getting device mix format";
        check_hresult(m_audioClient->GetMixFormat(m_mixFormat.put()));

        LOGI() << "WASAPI: Mix format after getting from audio client:";
        logWAVEFORMATEX(m_mixFormat.get());

        m_mixFormat->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        m_mixFormat->nChannels = 2;
        m_mixFormat->wBitsPerSample = 32;
        m_mixFormat->nAvgBytesPerSec = m_mixFormat->nSamplesPerSec * m_mixFormat->nChannels * sizeof(float);
        m_mixFormat->nBlockAlign = (m_mixFormat->nChannels * m_mixFormat->wBitsPerSample) / 8;
        m_mixFormat->cbSize = 0;

        LOGI() << "WASAPI: Modified mix format:";
        logWAVEFORMATEX(m_mixFormat.get());

        if (m_useClosestSupportedFormat) {
            LOGI() << "WASAPI: Querying closest supported format";

            unique_cotaskmem_ptr<WAVEFORMATEX> closestSupported;
            m_audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, m_mixFormat.get(), closestSupported.put());

            // According to the documentation, closestSupported may be null
            if (closestSupported) {
                LOGI() << "WASAPI: Closest supported format:";
                logWAVEFORMATEX(closestSupported.get());
                m_mixFormat = std::move(closestSupported);
            } else {
                LOGW() << "WASAPI: Could not query closest supported format";
            }
        }

        if (!audioProps.bIsOffload) {
            LOGI() << "WASAPI: Getting shared mode engine period";
            // The wfx parameter below is optional (Its needed only for MATCH_FORMAT clients). Otherwise, wfx will be assumed
            // to be the current engine format based on the processing mode for this stream
            check_hresult(m_audioClient->GetSharedModeEnginePeriod(m_mixFormat.get(), &m_defaultPeriodInFrames,
                                                                   &m_fundamentalPeriodInFrames,
                                                                   &m_minPeriodInFrames, &m_maxPeriodInFrames));
        }

        LOGI() << "WASAPI: Device successfully configured";

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

        SetEvent(m_clientFailedToStartEvent);
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

        SetEvent(m_clientFailedToStartEvent);

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
        LOGI() << "Attempting to auto-recovery audio endpoint";

        setState(DeviceState::Uninitialized);
        m_audioClient = nullptr;
        m_audioRenderClient = nullptr;
        m_sampleReadyAsyncResult = nullptr;

        asyncInitializeAudioDevice(m_fallbackDeviceIdString);
    }
}

void WasapiAudioClient::getSamples(uint32_t framesAvailable)
{
    uint8_t* data;

    uint32_t actualFramesToRead = framesAvailable;

    // WASAPI: "nBlockAlign must be equal to the product of nChannels and wBitsPerSample divided by 8 (bits per byte)"
    const uint32_t clientFrameSize = m_mixFormat->nBlockAlign;

    // MuseScore assumes only 2 audio channels (same calculation as above to determine frame size)
    const uint32_t muFrameSize = 2 * m_mixFormat->wBitsPerSample / 8;

    check_hresult(m_audioRenderClient->GetBuffer(actualFramesToRead, &data));
    if (actualFramesToRead > 0) {
        // Based on the previous calculations, the only way that clientFrameSize will be larger than muFrameSize is
        // if the client specifies more than 2 channels. MuseScore doesn't support this (yet), so we use a workaround
        // where the missing channels are padded with zeroes...
        if (clientFrameSize > muFrameSize) {
            const size_t surroundBufferDesiredSize = actualFramesToRead * muFrameSize;
            if (m_surroundAudioBuffer.size() < surroundBufferDesiredSize) {
                m_surroundAudioBuffer.resize(surroundBufferDesiredSize, 0);
            }

            m_sampleRequestCallback(nullptr, m_surroundAudioBuffer.data(), (int)surroundBufferDesiredSize);

            for (uint32_t i = 0; i < actualFramesToRead; ++i) {
                uint8_t* frameStartPos = data + i * clientFrameSize;
                std::memcpy(frameStartPos, m_surroundAudioBuffer.data() + i * muFrameSize, muFrameSize);
                std::memset(frameStartPos + muFrameSize, 0, clientFrameSize - muFrameSize);
            }
        } else {
            m_sampleRequestCallback(nullptr, data, actualFramesToRead * clientFrameSize);
        }
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
    case AUDCLNT_E_NOT_INITIALIZED: errMsg = "AUDCLNT_E_NOT_INITIALIZED";
        break;
    case AUDCLNT_E_ALREADY_INITIALIZED: errMsg = "AUDCLNT_E_ALREADY_INITIALIZED";
        break;
    case AUDCLNT_E_WRONG_ENDPOINT_TYPE: errMsg = "AUDCLNT_E_WRONG_ENDPOINT_TYPE";
        break;
    case AUDCLNT_E_DEVICE_INVALIDATED: errMsg = "AUDCLNT_E_DEVICE_INVALIDATED";
        break;
    case AUDCLNT_E_NOT_STOPPED: errMsg = "AUDCLNT_E_NOT_STOPPED";
        break;
    case AUDCLNT_E_BUFFER_TOO_LARGE: errMsg = "AUDCLNT_E_BUFFER_TOO_LARGE";
        break;
    case AUDCLNT_E_OUT_OF_ORDER: errMsg = "AUDCLNT_E_OUT_OF_ORDER";
        break;
    case AUDCLNT_E_UNSUPPORTED_FORMAT: errMsg = "AUDCLNT_E_UNSUPPORTED_FORMAT";
        break;
    case AUDCLNT_E_INVALID_SIZE: errMsg = "AUDCLNT_E_INVALID_SIZE";
        break;
    case AUDCLNT_E_DEVICE_IN_USE: errMsg = "AUDCLNT_E_DEVICE_IN_USE";
        break;
    case AUDCLNT_E_BUFFER_OPERATION_PENDING: errMsg = "AUDCLNT_E_BUFFER_OPERATION_PENDING";
        break;
    case AUDCLNT_E_THREAD_NOT_REGISTERED: errMsg = "AUDCLNT_E_THREAD_NOT_REGISTERED";
        break;
    case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED: errMsg = "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED";
        break;
    case AUDCLNT_E_ENDPOINT_CREATE_FAILED: errMsg = "AUDCLNT_E_ENDPOINT_CREATE_FAILED";
        break;
    case AUDCLNT_E_SERVICE_NOT_RUNNING: errMsg = "AUDCLNT_E_SERVICE_NOT_RUNNING";
        break;
    case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED: errMsg = "AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED";
        break;
    case AUDCLNT_E_EXCLUSIVE_MODE_ONLY: errMsg = "AUDCLNT_E_EXCLUSIVE_MODE_ONLY";
        break;
    case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL: errMsg = "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL";
        break;
    case AUDCLNT_E_EVENTHANDLE_NOT_SET: errMsg = "AUDCLNT_E_EVENTHANDLE_NOT_SET";
        break;
    case AUDCLNT_E_INCORRECT_BUFFER_SIZE: errMsg = "AUDCLNT_E_INCORRECT_BUFFER_SIZE";
        break;
    case AUDCLNT_E_BUFFER_SIZE_ERROR: errMsg = "AUDCLNT_E_BUFFER_SIZE_ERROR";
        break;
    case AUDCLNT_E_CPUUSAGE_EXCEEDED: errMsg = "AUDCLNT_E_CPUUSAGE_EXCEEDED";
        break;
    case AUDCLNT_E_BUFFER_ERROR: errMsg = "AUDCLNT_E_BUFFER_ERROR";
        break;
    case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED: errMsg = "AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED";
        break;
    case AUDCLNT_E_INVALID_DEVICE_PERIOD: errMsg = "AUDCLNT_E_INVALID_DEVICE_PERIOD";
        break;
    case AUDCLNT_E_INVALID_STREAM_FLAG: errMsg = "AUDCLNT_E_INVALID_STREAM_FLAG";
        break;
    case AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE: errMsg = "AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE";
        break;
    case AUDCLNT_E_OUT_OF_OFFLOAD_RESOURCES: errMsg = "AUDCLNT_E_OUT_OF_OFFLOAD_RESOURCES";
        break;
    case AUDCLNT_E_OFFLOAD_MODE_ONLY: errMsg = "AUDCLNT_E_OFFLOAD_MODE_ONLY";
        break;
    case AUDCLNT_E_NONOFFLOAD_MODE_ONLY: errMsg = "AUDCLNT_E_NONOFFLOAD_MODE_ONLY";
        break;
    case AUDCLNT_E_RESOURCES_INVALIDATED: errMsg = "AUDCLNT_E_RESOURCES_INVALIDATED";
        break;
    case AUDCLNT_E_RAW_MODE_UNSUPPORTED: errMsg = "AUDCLNT_E_RAW_MODE_UNSUPPORTED";
        break;
    case AUDCLNT_E_ENGINE_PERIODICITY_LOCKED: errMsg = "AUDCLNT_E_ENGINE_PERIODICITY_LOCKED";
        break;
    case AUDCLNT_E_ENGINE_FORMAT_LOCKED: errMsg = "AUDCLNT_E_ENGINE_FORMAT_LOCKED";
        break;
    case AUDCLNT_E_HEADTRACKING_ENABLED: errMsg = "AUDCLNT_E_HEADTRACKING_ENABLED";
        break;
    case AUDCLNT_E_HEADTRACKING_UNSUPPORTED: errMsg = "AUDCLNT_E_HEADTRACKING_UNSUPPORTED";
        break;
    case AUDCLNT_S_BUFFER_EMPTY: errMsg = "AUDCLNT_S_BUFFER_EMPTY";
        break;
    case AUDCLNT_S_THREAD_ALREADY_REGISTERED: errMsg = "AUDCLNT_S_THREAD_ALREADY_REGISTERED";
        break;
    case AUDCLNT_S_POSITION_STALLED: errMsg = "AUDCLNT_S_POSITION_STALLED";
        break;

    case S_OK:
        break;

    default:
        errMsg = "ERROR: " + std::to_string(resultCode) + " " + to_string(hresult_error(resultCode).message());
        break;
    }

    if (!errMsg.empty()) {
        LOGE() << errMsg;
    }

    m_deviceState = newState;
}
