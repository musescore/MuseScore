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
#include "wincoreaudiodriver.h"

#include <system_error>
#include "mmdeviceapi.h"
#include "windows.h"
#include "audioclient.h"
#include "log.h"

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define CHECK_HRESULT(hr); if (hr != S_OK) { \
        logError(hr); \
        return false; \
} \

using namespace mu::audio;

struct WinCoreData {
    IAudioClient* audioClient = nullptr;
    IAudioRenderClient* renderClient = nullptr;
    IAudioDriver::Callback callback;
    WAVEFORMATEX pFormat;
    HANDLE hEvent;
};

static void logError(HRESULT hr);

static WinCoreData* s_data = nullptr;

CoreAudioDriver::CoreAudioDriver()
{
}

CoreAudioDriver::~CoreAudioDriver()
{
    close();
}

std::string CoreAudioDriver::name() const
{
    return "MUAUDIO(WinCoreAudio)";
}

bool CoreAudioDriver::open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec)
{
    if (s_data) {
        delete s_data;
    }

    s_data = new WinCoreData();

    HRESULT hr = S_OK;
    const IID MU_IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const IID MU_IID_IAudioClient = __uuidof(IAudioClient);
    const IID MU_IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, MU_IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    CHECK_HRESULT(hr);

    IMMDevice* pDdevice = nullptr;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDdevice);
    pEnumerator->Release();
    CHECK_HRESULT(hr);

    hr = pDdevice->Activate(MU_IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&s_data->audioClient);
    pDdevice->Release();
    CHECK_HRESULT(hr);

    WAVEFORMATEX* deviceFormat;
    hr = s_data->audioClient->GetMixFormat(&deviceFormat);
    CHECK_HRESULT(hr);

    REFERENCE_TIME defaultTime, minimumTime;
    hr = s_data->audioClient->GetDevicePeriod(&defaultTime, &minimumTime);
    CHECK_HRESULT(hr);

    s_data->pFormat = *deviceFormat;
    s_data->pFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    s_data->pFormat.nChannels = spec.channels;
    LOGI() << s_data->pFormat.nSamplesPerSec;
    s_data->pFormat.wBitsPerSample = 32;
    s_data->pFormat.nAvgBytesPerSec = s_data->pFormat.nSamplesPerSec * s_data->pFormat.nChannels * sizeof(float);
    s_data->pFormat.nBlockAlign = (s_data->pFormat.nChannels * s_data->pFormat.wBitsPerSample) / 8;
    s_data->pFormat.cbSize = 0;
    s_data->callback = spec.callback;
    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioF32;
        activeSpec->sampleRate = s_data->pFormat.nSamplesPerSec;
    }
    hr = s_data->audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                         defaultTime, defaultTime, &s_data->pFormat, NULL);
    CHECK_HRESULT(hr);

    UINT32 bufferFrameCount;
    hr = s_data->audioClient->GetBufferSize(&bufferFrameCount);
    CHECK_HRESULT(hr);

    hr = s_data->audioClient->GetService(MU_IID_IAudioRenderClient, (void**)&s_data->renderClient);
    CHECK_HRESULT(hr);

    s_data->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    CHECK_HRESULT(hr);
    hr = s_data->audioClient->SetEventHandle(s_data->hEvent);

    REFERENCE_TIME hnsActualDuration = (double)REFTIMES_PER_SEC
                                       * bufferFrameCount / s_data->pFormat.nSamplesPerSec;

    m_active = true;
    m_thread = std::thread([this, hnsActualDuration]() {
        BYTE* pData;
        HRESULT hr = S_OK;
        do {
            Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

            UINT32 bufferFrameCount, bufferPading;
            hr = s_data->audioClient->GetBufferSize(&bufferFrameCount);
            logError(hr);

            hr = s_data->audioClient->GetCurrentPadding(&bufferPading);
            logError(hr);

            auto bufferSize = bufferFrameCount - bufferPading;
            hr = s_data->renderClient->GetBuffer(bufferSize, &pData);
            logError(hr);
            if (!pData || hr != S_OK) {
                continue;
            }
            s_data->callback(nullptr, reinterpret_cast<uint8_t*>(pData),
                             bufferSize * s_data->pFormat.wBitsPerSample * s_data->pFormat.nChannels / 8);
            hr = s_data->renderClient->ReleaseBuffer(bufferSize, 0);
            logError(hr);

            DWORD waitResult = WaitForSingleObject(s_data->hEvent, INFINITE);
            if (waitResult != WAIT_OBJECT_0) {
                LOGE() << "audio driver wait for signal failed: " << waitResult;
                break;
            }
        } while (m_active);
    });

    hr = s_data->audioClient->Start();
    CHECK_HRESULT(hr);

    LOGI() << "Core Audio driver started";
    return true;
}

void CoreAudioDriver::close()
{
    if (!s_data) {
        return;
    }

    if (s_data->audioClient) {
        s_data->audioClient->Stop();
    }
    m_active = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
    clean();
    LOGI() << "Core Audio driver stoped";
}

bool CoreAudioDriver::isOpened() const
{
    return m_active;
}

void logError(HRESULT hr)
{
    static HRESULT lastError = S_OK;

    if (hr == lastError) {
        return;
    }

    lastError = hr;

    switch (hr) {
    case S_OK: return;
    case AUDCLNT_E_NOT_INITIALIZED: LOGE() << "AUDCLNT_E_NOT_INITIALIZED";
        break;
    case AUDCLNT_E_ALREADY_INITIALIZED: LOGE() << "AUDCLNT_E_ALREADY_INITIALIZED";
        break;
    case AUDCLNT_E_WRONG_ENDPOINT_TYPE: LOGE() << "AUDCLNT_E_WRONG_ENDPOINT_TYPE";
        break;
    case AUDCLNT_E_DEVICE_INVALIDATED: LOGE() << "AUDCLNT_E_DEVICE_INVALIDATED";
        break;
    case AUDCLNT_E_NOT_STOPPED: LOGE() << "AUDCLNT_E_NOT_STOPPED";
        break;
    case AUDCLNT_E_BUFFER_TOO_LARGE: LOGE() << "AUDCLNT_E_BUFFER_TOO_LARGE";
        break;
    case AUDCLNT_E_OUT_OF_ORDER: LOGE() << "AUDCLNT_E_OUT_OF_ORDER";
        break;
    case AUDCLNT_E_UNSUPPORTED_FORMAT: LOGE() << "AUDCLNT_E_UNSUPPORTED_FORMAT";
        break;
    case AUDCLNT_E_INVALID_SIZE: LOGE() << "AUDCLNT_E_INVALID_SIZE";
        break;
    case AUDCLNT_E_DEVICE_IN_USE: LOGE() << "AUDCLNT_E_DEVICE_IN_USE";
        break;
    case AUDCLNT_E_BUFFER_OPERATION_PENDING: LOGE() << "AUDCLNT_E_BUFFER_OPERATION_PENDING";
        break;
    case AUDCLNT_E_THREAD_NOT_REGISTERED: LOGE() << "AUDCLNT_E_THREAD_NOT_REGISTERED";
        break;
    case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED: LOGE() << "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED";
        break;
    case AUDCLNT_E_ENDPOINT_CREATE_FAILED: LOGE() << "AUDCLNT_E_ENDPOINT_CREATE_FAILED";
        break;
    case AUDCLNT_E_SERVICE_NOT_RUNNING: LOGE() << "AUDCLNT_E_SERVICE_NOT_RUNNING";
        break;
    case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED: LOGE() << "AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED";
        break;
    case AUDCLNT_E_EXCLUSIVE_MODE_ONLY: LOGE() << "AUDCLNT_E_EXCLUSIVE_MODE_ONLY";
        break;
    case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL: LOGE() << "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL";
        break;
    case AUDCLNT_E_EVENTHANDLE_NOT_SET: LOGE() << "AUDCLNT_E_EVENTHANDLE_NOT_SET";
        break;
    case AUDCLNT_E_INCORRECT_BUFFER_SIZE: LOGE() << "AUDCLNT_E_INCORRECT_BUFFER_SIZE";
        break;
    case AUDCLNT_E_BUFFER_SIZE_ERROR: LOGE() << "AUDCLNT_E_BUFFER_SIZE_ERROR";
        break;
    case AUDCLNT_E_CPUUSAGE_EXCEEDED: LOGE() << "AUDCLNT_E_CPUUSAGE_EXCEEDED";
        break;
    case AUDCLNT_E_BUFFER_ERROR: LOGE() << "AUDCLNT_E_BUFFER_ERROR";
        break;
    case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED: LOGE() << "AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED";
        break;
    case AUDCLNT_E_INVALID_DEVICE_PERIOD: LOGE() << "AUDCLNT_E_INVALID_DEVICE_PERIOD";
        break;
    case AUDCLNT_E_INVALID_STREAM_FLAG: LOGE() << "AUDCLNT_E_INVALID_STREAM_FLAG";
        break;
    case AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE: LOGE() << "AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE";
        break;
    case AUDCLNT_E_OUT_OF_OFFLOAD_RESOURCES: LOGE() << "AUDCLNT_E_OUT_OF_OFFLOAD_RESOURCES";
        break;
    case AUDCLNT_E_OFFLOAD_MODE_ONLY: LOGE() << "AUDCLNT_E_OFFLOAD_MODE_ONLY";
        break;
    case AUDCLNT_E_NONOFFLOAD_MODE_ONLY: LOGE() << "AUDCLNT_E_NONOFFLOAD_MODE_ONLY";
        break;
    case AUDCLNT_E_RESOURCES_INVALIDATED: LOGE() << "AUDCLNT_E_RESOURCES_INVALIDATED";
        break;
    case AUDCLNT_S_BUFFER_EMPTY: LOGE() << "AUDCLNT_S_BUFFER_EMPTY";
        break;
    case AUDCLNT_S_THREAD_ALREADY_REGISTERED: LOGE() << "AUDCLNT_S_THREAD_ALREADY_REGISTERED";
        break;
    case AUDCLNT_S_POSITION_STALLED: LOGE() << "AUDCLNT_S_POSITION_STALLED";
        break;
    default:
        LOGE() << std::system_category().message(hr);
    }
}

void CoreAudioDriver::clean()
{
    if (s_data->audioClient) {
        s_data->audioClient->Release();
    }
    if (s_data->renderClient) {
        s_data->renderClient->Release();
    }

    delete s_data;
    s_data = nullptr;
}

void CoreAudioDriver::resume()
{
}

void CoreAudioDriver::suspend()
{
}

std::string CoreAudioDriver::outputDevice() const
{
    NOT_IMPLEMENTED;
    return "default";
}

bool CoreAudioDriver::selectOutputDevice(const std::string& name)
{
    UNUSED(name);
    NOT_IMPLEMENTED;
    return false;
}

std::vector<std::string> CoreAudioDriver::availableOutputDevices() const
{
    NOT_IMPLEMENTED;
    return { "default" };
}

mu::async::Notification CoreAudioDriver::availableOutputDevicesChanged() const
{
    NOT_IMPLEMENTED;
    return mu::async::Notification();
}
