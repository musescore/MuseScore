//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "wincoreaudiodriver.h"

#include <system_error>
#include "mmdeviceapi.h"
#include "log.h"

#define CHECK_HRESULT(hr); if (hr != S_OK) { \
        logError(hr); \
        return false; \
} \

using namespace mu::audio;

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
    HRESULT hr = S_OK;
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    CHECK_HRESULT(hr);

    IMMDevice* pDdevice = nullptr;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDdevice);
    pEnumerator->Release();
    CHECK_HRESULT(hr);

    hr = pDdevice->Activate(IID_IAudioClient, CLSCTX_ALL,NULL, (void**)&m_audioClient);
    pDdevice->Release();
    CHECK_HRESULT(hr);

    WAVEFORMATEX pFormat, * deviceFormat;
    hr = m_audioClient->GetMixFormat(&deviceFormat);
    CHECK_HRESULT(hr);

    REFERENCE_TIME defaultTime, minimumTime;
    hr = m_audioClient->GetDevicePeriod(&defaultTime, &minimumTime);
    CHECK_HRESULT(hr);

    pFormat = *deviceFormat;
    pFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    pFormat.nChannels = spec.channels;
    LOGI() << pFormat.nSamplesPerSec;
    pFormat.wBitsPerSample = 32;
    pFormat.nAvgBytesPerSec = pFormat.nSamplesPerSec * pFormat.nChannels * sizeof(float);
    pFormat.nBlockAlign = (pFormat.nChannels * pFormat.wBitsPerSample) / 8;
    pFormat.cbSize = 0;
    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioF32;
        activeSpec->sampleRate = pFormat.nSamplesPerSec;
    }
    hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                   defaultTime, defaultTime, &pFormat, NULL);
    CHECK_HRESULT(hr);

    UINT32 bufferFrameCount;
    hr = m_audioClient->GetBufferSize(&bufferFrameCount);
    CHECK_HRESULT(hr);

    hr = m_audioClient->GetService(IID_IAudioRenderClient, (void**)&m_renderClient);
    CHECK_HRESULT(hr);

    auto hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    CHECK_HRESULT(hr);
    hr = m_audioClient->SetEventHandle(hEvent);

    m_active = true;
    m_thread = std::thread([=]() {
        BYTE* pData;
        HRESULT hr = S_OK;
        do {
            UINT32 bufferFrameCount, bufferPading;
            hr = m_audioClient->GetBufferSize(&bufferFrameCount);
            logError(hr);

            hr = m_audioClient->GetCurrentPadding(&bufferPading);
            logError(hr);

            auto bufferSize = bufferFrameCount - bufferPading;
            hr = m_renderClient->GetBuffer(bufferSize, &pData);
            logError(hr);
            if (!pData || hr != S_OK) {
                continue;
            }
            activeSpec->callback(nullptr, reinterpret_cast<uint8_t*>(pData),
                                 bufferSize * pFormat.wBitsPerSample * pFormat.nChannels / 8);
            hr = m_renderClient->ReleaseBuffer(bufferSize, 0);
            logError(hr);

            DWORD waitResult = WaitForSingleObject(hEvent, INFINITE);
            if (waitResult != WAIT_OBJECT_0) {
                LOGE() << "audio driver wait for signal failed: " << waitResult;
                break;
            }
        } while(m_active);
    });
    m_thread.detach();

    hr = m_audioClient->Start();
    CHECK_HRESULT(hr);

    LOGI() << "Core Audio driver started";
    return true;
}

void CoreAudioDriver::close()
{
    if (m_audioClient) {
        m_audioClient->Stop();
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

void CoreAudioDriver::logError(HRESULT hr)
{
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
    if (m_audioClient) {
        m_audioClient->Release();
    }
    if (m_renderClient) {
        m_renderClient->Release();
    }
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
