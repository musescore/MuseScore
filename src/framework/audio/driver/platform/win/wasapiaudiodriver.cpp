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
#include "wasapiaudiodriver.h"

#include <combaseapi.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#include "global/defer.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;

struct WasapiAudioDriver::DeviceListener : public IMMNotificationClient
{
    DeviceListener(WasapiAudioDriver* driver)
        : m_driver(driver) {}

    STDMETHOD_(ULONG, AddRef)() {
        return 1;
    }
    STDMETHOD_(ULONG, Release)() {
        return 1;
    }
    STDMETHOD(QueryInterface)(REFIID, void**) {
        return S_OK;
    }
    STDMETHOD(OnDeviceStateChanged)(LPCWSTR, DWORD) {
        return S_OK;
    }
    STDMETHOD(OnDeviceAdded)(LPCWSTR) {
        m_driver->updateAudioDeviceList();
        return S_OK;
    }
    STDMETHOD(OnDeviceRemoved)(LPCWSTR) {
        m_driver->updateAudioDeviceList();
        return S_OK;
    }
    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR) {
        if ((role != eConsole && role != eCommunications) || (flow != eRender)) {
            return S_OK;
        }

        m_driver->updateAudioDeviceList();

        return S_OK;
    }
    STDMETHOD(OnPropertyValueChanged)(LPCWSTR, const PROPERTYKEY) {
        return S_OK;
    }

    WasapiAudioDriver* m_driver = nullptr;
};

struct WasapiAudioDriver::Data {
    IMMDeviceEnumerator* enumerator = nullptr;
    IAudioClient* audioClient = nullptr;
    IAudioRenderClient* renderClient = nullptr;
    WAVEFORMATEX* mixFormat = nullptr;
    HANDLE audioEvent = nullptr;

    void releaseClient()
    {
        if (audioEvent) {
            CloseHandle(audioEvent);
            audioEvent = nullptr;
        }

        if (renderClient) {
            renderClient->Release();
            renderClient = nullptr;
        }

        if (audioClient) {
            audioClient->Release();
            audioClient = nullptr;
        }

        if (mixFormat) {
            CoTaskMemFree(mixFormat);
            mixFormat = nullptr;
        }
    }

    void releaseAll()
    {
        releaseClient();
        if (enumerator) {
            enumerator->Release();
            enumerator = nullptr;
        }
    }
};

static REFERENCE_TIME to_reftime(int numSamples, double sampleRate)
{
    return (REFERENCE_TIME)((numSamples * 10000.0 * 1000.0 / sampleRate) + 0.5);
}

static samples_t to_samples(const REFERENCE_TIME& t, double sampleRate)
{
    return sampleRate * ((double)t) * 0.0000001;
}

static std::wstring to_wstring(const std::string& str)
{
    if (str.empty()) {
        return L"";
    }

    int wide_len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (wide_len == 0) {
        return L"";
    }

    std::wstring result(wide_len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], wide_len);

    return result;
}

static std::string to_string(LPWSTR wstr)
{
    if (!wstr || wstr[0] == L'\0') {
        return "";
    }

    int multi_len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (multi_len == 0) {
        return "";
    }

    std::string result(multi_len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], multi_len, nullptr, nullptr);

    return result;
}

static std::string get_deviceId(IMMDevice* device)
{
    LPWSTR devId;
    HRESULT hr = device->GetId(&devId);
    if (FAILED(hr)) {
        LOGE() << "failed get deviceId";
        return std::string();
    }
    std::string str = to_string(devId);
    CoTaskMemFree(devId);
    return str;
}

static std::string hrToString(HRESULT hr)
{
    if (SUCCEEDED(hr)) {
        return "SUCCEEDED";
    }

    switch (hr) {
    // Common WASAPI errors
    case AUDCLNT_E_NOT_INITIALIZED:          return "AUDCLNT_E_NOT_INITIALIZED - Audio client not initialized";
    case AUDCLNT_E_ALREADY_INITIALIZED:      return "AUDCLNT_E_ALREADY_INITIALIZED - Audio client already initialized";
    case AUDCLNT_E_WRONG_ENDPOINT_TYPE:      return "AUDCLNT_E_WRONG_ENDPOINT_TYPE - Wrong endpoint type";
    case AUDCLNT_E_DEVICE_INVALIDATED:       return "AUDCLNT_E_DEVICE_INVALIDATED - Device invalidated";
    case AUDCLNT_E_NOT_STOPPED:              return "AUDCLNT_E_NOT_STOPPED - Audio client not stopped";
    case AUDCLNT_E_BUFFER_TOO_LARGE:         return "AUDCLNT_E_BUFFER_TOO_LARGE - Buffer too large";
    case AUDCLNT_E_OUT_OF_ORDER:             return "AUDCLNT_E_OUT_OF_ORDER - Operation out of order";
    case AUDCLNT_E_UNSUPPORTED_FORMAT:       return "AUDCLNT_E_UNSUPPORTED_FORMAT - Unsupported format";
    case AUDCLNT_E_INVALID_SIZE:             return "AUDCLNT_E_INVALID_SIZE - Invalid size";
    case AUDCLNT_E_DEVICE_IN_USE:            return "AUDCLNT_E_DEVICE_IN_USE - Device in use";
    case AUDCLNT_E_BUFFER_OPERATION_PENDING: return "AUDCLNT_E_BUFFER_OPERATION_PENDING - Buffer operation pending";
    case AUDCLNT_E_THREAD_NOT_REGISTERED:    return "AUDCLNT_E_THREAD_NOT_REGISTERED - Thread not registered";
    case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED: return "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED - Exclusive mode not allowed";
    case AUDCLNT_E_ENDPOINT_CREATE_FAILED:   return "AUDCLNT_E_ENDPOINT_CREATE_FAILED - Endpoint create failed";
    case AUDCLNT_E_SERVICE_NOT_RUNNING:      return "AUDCLNT_E_SERVICE_NOT_RUNNING - Audio service not running";
    case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED: return "AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED - Event handle not expected";
    case AUDCLNT_E_EXCLUSIVE_MODE_ONLY:      return "AUDCLNT_E_EXCLUSIVE_MODE_ONLY - Exclusive mode only";
    case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL: return "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL - Buffer duration/period not equal";
    case AUDCLNT_E_EVENTHANDLE_NOT_SET:      return "AUDCLNT_E_EVENTHANDLE_NOT_SET - Event handle not set";
    case AUDCLNT_E_INCORRECT_BUFFER_SIZE:    return "AUDCLNT_E_INCORRECT_BUFFER_SIZE - Incorrect buffer size";
    case AUDCLNT_E_BUFFER_SIZE_ERROR:        return "AUDCLNT_E_BUFFER_SIZE_ERROR - Buffer size error";
    case AUDCLNT_E_CPUUSAGE_EXCEEDED:        return "AUDCLNT_E_CPUUSAGE_EXCEEDED - CPU usage exceeded";

    // Common COM errors
    case E_INVALIDARG:                       return "E_INVALIDARG - Invalid arguments";
    case E_OUTOFMEMORY:                      return "E_OUTOFMEMORY - Out of memory";
    case E_NOINTERFACE:                      return "E_NOINTERFACE - No interface";
    case E_POINTER:                          return "E_POINTER - Invalid pointer";
    case E_ACCESSDENIED:                     return "E_ACCESSDENIED - Access denied";
    case E_FAIL:                             return "E_FAIL - Unspecified error";

    default:
        return "Unknown error: " + std::to_string(hr);
    }
}

WasapiAudioDriver::WasapiAudioDriver()
{
    m_data = std::make_shared<Data>();
    m_deviceListener = std::make_shared<DeviceListener>(this);
}

WasapiAudioDriver::~WasapiAudioDriver()
{
    close();

    if (m_data->enumerator) {
        m_data->enumerator->UnregisterEndpointNotificationCallback(m_deviceListener.get());
    }

    m_data->releaseAll();

    CoUninitialize();
}

std::string WasapiAudioDriver::name() const
{
    return "WASAPI";
}

void WasapiAudioDriver::init()
{
    LOGI() << "begin driver init";
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        LOGE() << "failed CoInitializeEx, error: " << hrToString(hr);
    }

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&m_data->enumerator
        );

    if (FAILED(hr)) {
        LOGE() << "failed create DeviceEnumerator, error: " << hrToString(hr);
        return;
    }

    hr = m_data->enumerator->RegisterEndpointNotificationCallback(m_deviceListener.get());
    if (FAILED(hr)) {
        LOGE() << "failed RegisterEndpointNotificationCallback, error: " << hrToString(hr);
    }

    updateAudioDeviceList();

    LOGI() << "success driver init";
}

static std::pair<std::vector<AudioDevice>, std::string /*defaultId*/> audioDevices(IMMDeviceEnumerator* enumerator)
{
    IF_ASSERT_FAILED(enumerator) {
        return {};
    }

    IMMDeviceCollection* pCollection = nullptr;
    HRESULT hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
    if (FAILED(hr)) {
        LOGE() << "failed get EnumAudioEndpoints, error: " << hrToString(hr);
        return {};
    }

    UINT count = 0;
    pCollection->GetCount(&count);

    std::pair<std::vector<AudioDevice>, std::string> devices;

    for (UINT i = 0; i < count; i++) {
        IMMDevice* pDevice = nullptr;
        hr = pCollection->Item(i, &pDevice);
        if (SUCCEEDED(hr)) {
            AudioDevice deviceInfo;
            deviceInfo.id = get_deviceId(pDevice);

            IPropertyStore* pProps = nullptr;
            hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
            if (SUCCEEDED(hr)) {
                PROPVARIANT varName;
                PropVariantInit(&varName);

                hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
                if (SUCCEEDED(hr) && varName.vt == VT_LPWSTR) {
                    deviceInfo.name = to_string(varName.pwszVal);
                }

                PropVariantClear(&varName);
                pProps->Release();
            }

            devices.first.push_back(deviceInfo);
            pDevice->Release();
        }
    }

    pCollection->Release();

    IMMDevice* defaultDevice = nullptr;
    enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    if (defaultDevice) {
        devices.second = get_deviceId(defaultDevice);
        defaultDevice->Release();
    }

    return devices;
}

void WasapiAudioDriver::updateAudioDeviceList()
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_data->enumerator) {
        return;
    }

    auto devices = audioDevices(m_data->enumerator);
    m_deviceList = devices.first;
    m_defaultDeviceId = devices.second;
    m_deviceListChanged.notify();
}

static IAudioClient* audioClientForDevice(IMMDeviceEnumerator* enumerator, const std::string& deviceId)
{
    std::wstring wdeviceId = to_wstring(deviceId);
    IMMDevice* device = nullptr;
    HRESULT hr = enumerator->GetDevice(wdeviceId.c_str(), &device);
    if (FAILED(hr)) {
        LOGE() << "failed get device with id: " << deviceId << ", error: " << hrToString(hr);
        return nullptr;
    }

    DEFER {
        device->Release();
    };

    IAudioClient* audioClient = nullptr;
    hr = device->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        nullptr,
        (void**)&audioClient
        );

    if (FAILED(hr)) {
        LOGE() << "failed get audioClient, error: " << hrToString(hr);
        return nullptr;
    }

    return audioClient;
}

AudioDeviceID WasapiAudioDriver::defaultDevice() const
{
    return m_defaultDeviceId;
}

bool WasapiAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(!spec.deviceId.empty()) {
        return false;
    }

    IF_ASSERT_FAILED(m_data->enumerator) {
        return false;
    }

    LOGI() << "try open driver, device: " << spec.deviceId;

    m_data->audioClient = audioClientForDevice(m_data->enumerator, spec.deviceId);
    if (!m_data->audioClient) {
        LOGE() << "failed get audio client for device: " << spec.deviceId;
        return false;
    }

    HRESULT hr = m_data->audioClient->GetMixFormat(&m_data->mixFormat);
    if (FAILED(hr)) {
        LOGE() << "failed get mixFormat, error: " << hrToString(hr);
        return false;
    }

    m_activeSpec = spec;
    m_activeSpec.output.sampleRate = m_data->mixFormat->nSamplesPerSec;
    m_activeSpecChanged.send(m_activeSpec);

    if (activeSpec) {
        *activeSpec = m_activeSpec;
    }

    m_audioThread = std::thread(&WasapiAudioDriver::th_audioThread, this);

    return true;
}

void WasapiAudioDriver::th_audioThread()
{
    m_opened = th_audioInitialize();

    while (m_opened) {
        DWORD waitResult = WaitForSingleObject(m_data->audioEvent, 100);
        if (waitResult == WAIT_OBJECT_0) {
            th_processAudioData();
        } else if (waitResult == WAIT_TIMEOUT) {
            continue;
        } else {
            // error
            break;
        }
    }
}

bool WasapiAudioDriver::th_audioInitialize()
{
    TRACEFUNC;

    REFERENCE_TIME bufferDuration = to_reftime(m_activeSpec.output.samplesPerChannel,
                                               m_activeSpec.output.sampleRate);

    HRESULT hr = m_data->audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        bufferDuration,
        0,
        m_data->mixFormat,
        nullptr
        );

    if (FAILED(hr)) {
        LOGE() << "failed audioClient->Initialize, error: " << hrToString(hr);
        return false;
    }

    m_data->audioClient->GetBufferSize(&m_bufferFrames);
    REFERENCE_TIME defaultPeriod = 0;
    REFERENCE_TIME minimumPeriod = 0;
    m_data->audioClient->GetDevicePeriod(&defaultPeriod, &minimumPeriod);
    m_defaultPeriod = to_samples(defaultPeriod, m_activeSpec.output.sampleRate);
    m_minimumPeriod = to_samples(minimumPeriod, m_activeSpec.output.sampleRate);

    hr = m_data->audioClient->GetService(
        __uuidof(IAudioRenderClient),
        (void**)&m_data->renderClient
        );

    if (FAILED(hr)) {
        LOGE() << "failed get renderClient, error: " << hrToString(hr);
        return false;
    }

    m_data->audioEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_data->audioEvent) {
        LOGE() << "failed create audioEvent";
        return false;
    }

    hr = m_data->audioClient->SetEventHandle(m_data->audioEvent);
    if (FAILED(hr)) {
        LOGE() << "failed set audioEvent, error: " << hrToString(hr);
        return false;
    }

    hr = m_data->audioClient->Start();
    if (FAILED(hr)) {
        LOGE() << "failed audioClient->Start(), error: " << hrToString(hr);
        return false;
    }

    return true;
}

void WasapiAudioDriver::th_processAudioData()
{
    // Get padding in existing buffer
    UINT32 paddingFrames = 0;
    m_data->audioClient->GetCurrentPadding(&paddingFrames);

    // GetCurrentPadding represents the number of queued frames
    // so we can subtract that from the overall number of frames we have
    uint32_t framesAvailable = m_bufferFrames - paddingFrames;

    // Only continue if we have buffer to write data
    if (framesAvailable == 0) {
        return;
    }

    uint8_t* data = nullptr;

    uint32_t actualFramesToRead = framesAvailable;

    // WASAPI: "nBlockAlign must be equal to the product of nChannels and wBitsPerSample divided by 8 (bits per byte)"
    const uint32_t clientFrameSize = m_data->mixFormat->nBlockAlign;

    // MuseScore assumes only 2 audio channels (same calculation as above to determine frame size)
    const uint32_t muFrameSize = 2 * m_data->mixFormat->wBitsPerSample / 8;

    m_data->renderClient->GetBuffer(actualFramesToRead, &data);
    if (actualFramesToRead > 0) {
        // Based on the previous calculations, the only way that clientFrameSize will be larger than muFrameSize is
        // if the client specifies more than 2 channels. MuseScore doesn't support this (yet), so we use a workaround
        // where the missing channels are padded with zeroes...
        if (clientFrameSize > muFrameSize) {
            const size_t surroundBufferDesiredSize = actualFramesToRead * muFrameSize;
            if (m_surroundAudioBuffer.size() < surroundBufferDesiredSize) {
                m_surroundAudioBuffer.resize(surroundBufferDesiredSize, 0);
            }

            m_activeSpec.callback(m_surroundAudioBuffer.data(),
                                  (int)surroundBufferDesiredSize);

            for (uint32_t i = 0; i < actualFramesToRead; ++i) {
                uint8_t* frameStartPos = data + i * clientFrameSize;
                std::memcpy(frameStartPos, m_surroundAudioBuffer.data() + i * muFrameSize, muFrameSize);
                std::memset(frameStartPos + muFrameSize, 0, clientFrameSize - muFrameSize);
            }
        } else {
            m_activeSpec.callback(data, actualFramesToRead * clientFrameSize);
        }
    }
    m_data->renderClient->ReleaseBuffer(actualFramesToRead, 0);
}

void WasapiAudioDriver::close()
{
    m_opened = false;
    if (m_audioThread.joinable()) {
        m_audioThread.join();
    }

    m_data->releaseClient();
}

bool WasapiAudioDriver::isOpened() const
{
    return m_opened;
}

const IAudioDriver::Spec& WasapiAudioDriver::activeSpec() const
{
    return m_activeSpec;
}

async::Channel<IAudioDriver::Spec> WasapiAudioDriver::activeSpecChanged() const
{
    return m_activeSpecChanged;
}

AudioDeviceList WasapiAudioDriver::availableOutputDevices() const
{
    return m_deviceList;
}

async::Notification WasapiAudioDriver::availableOutputDevicesChanged() const
{
    return m_deviceListChanged;
}

std::vector<samples_t> WasapiAudioDriver::availableOutputDeviceBufferSizes() const
{
    std::vector<samples_t> result;

    samples_t n = MAXIMUM_BUFFER_SIZE;
    samples_t min = std::max(m_minimumPeriod, MINIMUM_BUFFER_SIZE);

    while (n >= min) {
        result.push_back(n);
        n /= 2;
    }

    return result;
}

std::vector<sample_rate_t> WasapiAudioDriver::availableOutputDeviceSampleRates() const
{
    return {
        44100,
        48000,
        88200,
        96000,
    };
}
