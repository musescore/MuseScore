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

#include <mmdeviceapi.h>
#include <windows.h>
#include <audioclient.h>

#if defined __MINGW32__ // hack against "undefined reference" when linking in MinGW
#define INITGUID
#endif
#include <Functiondiscoverykeys_devpkey.h>

#include "translation.h"
#include "log.h"

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define CHECK_HRESULT(hr, errorValue); if (hr != S_OK) { \
        logError(hr); \
        return errorValue; \
} \

using namespace muse::audio;

struct WinCoreData {
    IAudioClient* audioClient = nullptr;
    IAudioRenderClient* renderClient = nullptr;
    IAudioDriver::Callback callback;
    WAVEFORMATEX pFormat;
    HANDLE hEvent;
};

static std::wstring stringToWString(const std::string& str)
{
    return QString::fromStdString(str).toStdWString();
}

static std::string lpwstrToString(const LPWSTR& lpwstr)
{
    return QString::fromStdWString(lpwstr).toStdString();
}

static int refTimeToSamples(const REFERENCE_TIME& t, double sampleRate) noexcept
{
    return sampleRate * ((double)t) * 0.0000001;
}

static REFERENCE_TIME samplesToRefTime(int numSamples, double sampleRate) noexcept
{
    return (REFERENCE_TIME)((numSamples * 10000.0 * 1000.0 / sampleRate) + 0.5);
}

void copyWavFormat(WAVEFORMATEXTENSIBLE& dest, const WAVEFORMATEX* src) noexcept
{
    memcpy(&dest, src, src->wFormatTag == WAVE_FORMAT_EXTENSIBLE ? sizeof(WAVEFORMATEXTENSIBLE)
           : sizeof(WAVEFORMATEX));
}

static void logError(HRESULT hr);

static WinCoreData* s_data = nullptr;
static IAudioDriver::Spec s_activeSpec;
static HRESULT s_lastError = S_OK;

const IID MU_IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID MU_IID_IAudioClient = __uuidof(IAudioClient);
const IID MU_IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

CoreAudioDriver::CoreAudioDriver()
{
    m_deviceId = DEFAULT_DEVICE_ID;
    CoInitialize(NULL);
}

CoreAudioDriver::~CoreAudioDriver()
{
    close();
}

void CoreAudioDriver::init()
{
    m_devicesListener.startWithCallback([this]() {
        return availableOutputDevices();
    });

    m_devicesListener.devicesChanged().onNotify(this, [this]() {
        m_availableOutputDevicesChanged.notify();
    });

    CoInitialize(NULL);
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

    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, MU_IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    CHECK_HRESULT(hr, false);

    IMMDevice* pDdevice = nullptr;
    AudioDeviceID outputDeviceId = outputDevice();
    if (outputDeviceId == DEFAULT_DEVICE_ID) {
        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDdevice);
    } else {
        std::wstring wstr = stringToWString(outputDeviceId);
        hr = pEnumerator->GetDevice(wstr.c_str(), &pDdevice);
    }
    pEnumerator->Release();
    CHECK_HRESULT(hr, false);

    hr = pDdevice->Activate(MU_IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&s_data->audioClient);
    pDdevice->Release();
    CHECK_HRESULT(hr, false);

    WAVEFORMATEX* deviceFormat;
    hr = s_data->audioClient->GetMixFormat(&deviceFormat);
    CHECK_HRESULT(hr, false);

    REFERENCE_TIME defaultTime, minimumTime;
    hr = s_data->audioClient->GetDevicePeriod(&defaultTime, &minimumTime);
    CHECK_HRESULT(hr, false);

    unsigned int minBufferSize = refTimeToSamples(minimumTime, spec.sampleRate);
    m_bufferSizes = resolveBufferSizes(minBufferSize);

    s_data->pFormat = *deviceFormat;
    s_data->pFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    s_data->pFormat.nChannels = spec.channels;
    s_data->pFormat.wBitsPerSample = 32;
    s_data->pFormat.nAvgBytesPerSec = s_data->pFormat.nSamplesPerSec * s_data->pFormat.nChannels * sizeof(float);
    s_data->pFormat.nBlockAlign = (s_data->pFormat.nChannels * s_data->pFormat.wBitsPerSample) / 8;
    s_data->pFormat.cbSize = 0;
    s_data->callback = spec.callback;
    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioF32;
        activeSpec->sampleRate = s_data->pFormat.nSamplesPerSec;
        s_activeSpec = *activeSpec;
    }

    defaultTime = std::max(minimumTime, samplesToRefTime(spec.samples, s_data->pFormat.nSamplesPerSec));

    hr = s_data->audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                         AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
                                         defaultTime, defaultTime, &s_data->pFormat, NULL);
    CHECK_HRESULT(hr, false);

    UINT32 bufferFrameCount;
    hr = s_data->audioClient->GetBufferSize(&bufferFrameCount);
    CHECK_HRESULT(hr, false);

    hr = s_data->audioClient->GetService(MU_IID_IAudioRenderClient, (void**)&s_data->renderClient);
    CHECK_HRESULT(hr, false);

    s_data->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    CHECK_HRESULT(hr, false);
    hr = s_data->audioClient->SetEventHandle(s_data->hEvent);

    REFERENCE_TIME hnsActualDuration = (double)REFTIMES_PER_SEC
                                       * bufferFrameCount / s_data->pFormat.nSamplesPerSec;

    m_active = true;
    m_thread = std::thread([this, hnsActualDuration]() {
        BYTE* pData;
        HRESULT hr = S_OK;
        do {
            Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

            UINT32 bufferFrameCount, bufferPadding;
            hr = s_data->audioClient->GetBufferSize(&bufferFrameCount);
            logError(hr);

            hr = s_data->audioClient->GetCurrentPadding(&bufferPadding);
            logError(hr);

            auto bufferSize = bufferFrameCount - bufferPadding;
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
                LOGE() << "audio driver wait for signal failed: " << int32_t(waitResult);
                break;
            }
        } while (m_active);
    });

    hr = s_data->audioClient->Start();
    CHECK_HRESULT(hr, false);

    LOGI() << "Core Audio driver started, id " << outputDeviceId << ", buffer size " << bufferFrameCount;
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
    return m_active || s_lastError != S_OK;
}

void logError(HRESULT hr)
{
    if (hr == s_lastError) {
        return;
    }

    s_lastError = hr;

    switch (hr) {
    case S_OK: return;
    case S_FALSE: LOGE() << "S_FALSE";
        break;
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
    case E_POINTER: LOGE() << "E_POINTER";
        break;
    case E_INVALIDARG: LOGE() << "E_INVALIDARG";
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

    s_lastError = S_OK;

    delete s_data;
    s_data = nullptr;
}

void CoreAudioDriver::resume()
{
}

void CoreAudioDriver::suspend()
{
}

AudioDeviceID CoreAudioDriver::outputDevice() const
{
    return m_deviceId;
}

bool CoreAudioDriver::selectOutputDevice(const AudioDeviceID& id)
{
    if (m_deviceId == id) {
        return true;
    }

    bool reopen = isOpened();
    close();
    m_deviceId = id;

    bool ok = true;
    if (reopen) {
        ok = open(s_activeSpec, &s_activeSpec);
    }

    if (ok) {
        m_outputDeviceChanged.notify();
    }

    return ok;
}

bool muse::audio::CoreAudioDriver::resetToDefaultOutputDevice()
{
    return selectOutputDevice(defaultDeviceId());
}

std::string CoreAudioDriver::defaultDeviceId() const
{
    HRESULT hr;
    LPWSTR devId;
    IMMDevice* device;

    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, MU_IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    CHECK_HRESULT(hr, std::string());

    EDataFlow dataFlow = eRender;
    ERole role = eConsole;
    hr = pEnumerator->GetDefaultAudioEndpoint(dataFlow, role, &device);
    CHECK_HRESULT(hr, std::string());

    hr = device->GetId(&devId);
    CHECK_HRESULT(hr, std::string());

    std::wstring ws(devId);
    return std::string(ws.begin(), ws.end());
}

std::vector<unsigned int> CoreAudioDriver::resolveBufferSizes(unsigned int minBufferSize)
{
    std::vector<unsigned int> result;

    unsigned int minimum = std::max(static_cast<int>(minBufferSize), MINIMUM_BUFFER_SIZE);

    unsigned int n = MAXIMUM_BUFFER_SIZE;
    while (n >= minimum) {
        result.push_back(n);
        n /= 2;
    }

    std::sort(result.begin(), result.end());

    return result;
}

async::Notification CoreAudioDriver::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

AudioDeviceList CoreAudioDriver::availableOutputDevices() const
{
    std::lock_guard lock(m_devicesMutex);

    //! NOTE: Required because the method can be called from another thread
    CoInitialize(NULL);

    AudioDeviceList result;
    result.push_back({ DEFAULT_DEVICE_ID, muse::trc("audio", "System default") });

    HRESULT hr;
    IMMDeviceCollection* devices;

    IMMDeviceEnumerator* enumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, MU_IID_IMMDeviceEnumerator, (void**)&enumerator);
    CHECK_HRESULT(hr, {});

    hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices);
    CHECK_HRESULT(hr, {});

    UINT deviceCount;

    hr = devices->GetCount(&deviceCount);
    CHECK_HRESULT(hr, {});

    for (UINT i = 0; i < deviceCount; ++i) {
        IMMDevice* device;

        hr = devices->Item(i, &device);
        CHECK_HRESULT(hr, {});

        LPWSTR devId;
        hr = device->GetId(&devId);
        CHECK_HRESULT(hr, {});

        IPropertyStore* pProps = NULL;

        hr = device->OpenPropertyStore(STGM_READ, &pProps);
        CHECK_HRESULT(hr, {});

        PROPVARIANT name;
        PropVariantInit(&name);

        hr = pProps->GetValue(PKEY_Device_FriendlyName, &name);
        CHECK_HRESULT(hr, {});

        AudioDevice deviceInfo;
        deviceInfo.id = lpwstrToString(devId);
        deviceInfo.name = lpwstrToString(name.pwszVal);
        result.push_back(deviceInfo);

        PropVariantClear(&name);
        device->Release();
    }

    devices->Release();

    return result;
}

async::Notification CoreAudioDriver::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}

unsigned int CoreAudioDriver::outputDeviceBufferSize() const
{
    if (!s_data) {
        return 0;
    }

    return s_activeSpec.samples;
}

bool CoreAudioDriver::setOutputDeviceBufferSize(unsigned int bufferSize)
{
    if (s_activeSpec.samples == bufferSize) {
        return true;
    }

    bool reopen = isOpened();
    close();
    s_activeSpec.samples = bufferSize;

    bool ok = true;
    if (reopen) {
        ok = open(s_activeSpec, &s_activeSpec);
    }

    if (ok) {
        m_bufferSizeChanged.notify();
    }

    return ok;
}

async::Notification CoreAudioDriver::outputDeviceBufferSizeChanged() const
{
    return m_bufferSizeChanged;
}

std::vector<unsigned int> CoreAudioDriver::availableOutputDeviceBufferSizes() const
{
    return m_bufferSizes;
}
