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
#include "wasapiaudiodriver2.h"

#include <codecvt>
#include <combaseapi.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#include "global/defer.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;

struct WasapiAudioDriver2::DeviceListener : public IMMNotificationClient
{
    DeviceListener(WasapiAudioDriver2* driver)
        : m_driver(driver) {}

<<<<<<< HEAD
    STDMETHOD_(ULONG, AddRef)() { return 1; }
    STDMETHOD_(ULONG, Release)() { return 1; }
    STDMETHOD(QueryInterface)(REFIID, void**) { return S_OK; }
    STDMETHOD(OnDeviceStateChanged)(LPCWSTR, DWORD) { return S_OK; }
    STDMETHOD(OnDeviceAdded)(LPCWSTR) { m_driver->updateAudioDeviceList(); return S_OK;}
    STDMETHOD(OnDeviceRemoved)(LPCWSTR) { m_driver->updateAudioDeviceList(); return S_OK;}
=======
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
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)
    STDMETHOD(OnDefaultDeviceChanged)(EDataFlow flow, ERole role, LPCWSTR) {
        if ((role != eConsole && role != eCommunications) || (flow != eRender)) {
            return S_OK;
        }

        m_driver->updateAudioDeviceList();

        return S_OK;
    }
<<<<<<< HEAD
    STDMETHOD(OnPropertyValueChanged)(LPCWSTR, const PROPERTYKEY) { return S_OK; }
=======
    STDMETHOD(OnPropertyValueChanged)(LPCWSTR, const PROPERTYKEY) {
        return S_OK;
    }
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)

    WasapiAudioDriver2* m_driver = nullptr;
};

struct WasapiAudioDriver2::Data {
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
<<<<<<< HEAD
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
=======
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t> > converter;
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)
    return converter.from_bytes(str);
}

static std::string to_string(LPWSTR wstr)
{
<<<<<<< HEAD
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
=======
    std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)
    return converter.to_bytes(wstr);
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

WasapiAudioDriver2::WasapiAudioDriver2()
{
    m_data = std::make_shared<Data>();
    m_deviceListener = std::make_shared<DeviceListener>(this);
}

WasapiAudioDriver2::~WasapiAudioDriver2()
{
    if (m_data->enumerator) {
        m_data->enumerator->UnregisterEndpointNotificationCallback(m_deviceListener.get());
    }

    m_data->releaseAll();

    CoUninitialize();
}

std::string WasapiAudioDriver2::name() const
{
    return "WASAPI_driver";
}

void WasapiAudioDriver2::init()
{
    LOGI() << "begin driver init";
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        LOGE() << "failed CoInitializeEx";
    }

    hr = CoCreateInstance(
<<<<<<< HEAD
                __uuidof(MMDeviceEnumerator),
                nullptr,
                CLSCTX_ALL,
                __uuidof(IMMDeviceEnumerator),
                (void**)&m_data->enumerator
                );
=======
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&m_data->enumerator
        );
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)

    if (FAILED(hr)) {
        LOGE() << "failed create DeviceEnumerator";
        return;
    }

    hr = m_data->enumerator->RegisterEndpointNotificationCallback(m_deviceListener.get());
    if (FAILED(hr)) {
        LOGE() << "failed RegisterEndpointNotificationCallback";
    }

    updateAudioDeviceList();

    LOGI() << "success driver init";
}

<<<<<<< HEAD
static std::pair<std::vector<AudioDevice>, std::string/*defaultId*/> audioDevices(IMMDeviceEnumerator* enumerator)
=======
static std::pair<std::vector<AudioDevice>, std::string /*defaultId*/> audioDevices(IMMDeviceEnumerator* enumerator)
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)
{
    IF_ASSERT_FAILED(enumerator) {
        return {};
    }

    IMMDeviceCollection* pCollection = nullptr;
    HRESULT hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
    if (FAILED(hr)) {
        LOGE() << "failed get EnumAudioEndpoints";
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

void WasapiAudioDriver2::updateAudioDeviceList()
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
        LOGE() << "failed get device with id: " << deviceId;
        return nullptr;
    }

    DEFER {
        device->Release();
    };

    IAudioClient* audioClient = nullptr;
    hr = device->Activate(
<<<<<<< HEAD
                __uuidof(IAudioClient),
                CLSCTX_ALL,
                nullptr,
                (void**)&audioClient
                );
=======
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        nullptr,
        (void**)&audioClient
        );
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)
    if (FAILED(hr)) {
        LOGE() << "failed get audioClient";
        return nullptr;
    }

    return audioClient;
}

bool WasapiAudioDriver2::open(const Spec& spec, Spec* activeSpec)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(m_data->enumerator) {
        return false;
    }

    LOGI() << "begin driver open";

    m_data->audioClient = audioClientForDevice(m_data->enumerator, m_deviceId);
    if (!m_data->audioClient) {
        m_deviceId = m_defaultDeviceId;
        m_data->audioClient = audioClientForDevice(m_data->enumerator, m_deviceId);
    }

    if (!m_data->audioClient) {
        return false;
    }

    HRESULT hr = m_data->audioClient->GetMixFormat(&m_data->mixFormat);
    if (FAILED(hr)) {
        LOGE() << "failed get mixFormat";
        return false;
    }

    m_activeSpec = spec;
    m_activeSpec.output.sampleRate = m_data->mixFormat->nSamplesPerSec;

    if (activeSpec) {
        *activeSpec = m_activeSpec;
    }

    m_audioThread = std::thread(&WasapiAudioDriver2::th_audioThread, this);

    return true;
}

void WasapiAudioDriver2::th_audioThread()
{
    m_opened = th_audioInitialize();

    while (m_opened) {
<<<<<<< HEAD

=======
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)
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

bool WasapiAudioDriver2::th_audioInitialize()
{
    TRACEFUNC;

    REFERENCE_TIME bufferDuration = to_reftime(m_activeSpec.output.samplesPerChannel,
                                               m_activeSpec.output.sampleRate);

    HRESULT hr = m_data->audioClient->Initialize(
<<<<<<< HEAD
                AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                bufferDuration,
                0,
                m_data->mixFormat,
                nullptr
                );
=======
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        bufferDuration,
        0,
        m_data->mixFormat,
        nullptr
        );
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)

    if (FAILED(hr)) {
        LOGE() << "failed audioClient->Initialize";
        return false;
    }

    m_data->audioClient->GetBufferSize(&m_bufferFrames);
    REFERENCE_TIME defaultPeriod = 0;
    REFERENCE_TIME minimumPeriod = 0;
    m_data->audioClient->GetDevicePeriod(&defaultPeriod, &minimumPeriod);
    m_defaultPeriod = to_samples(defaultPeriod, m_activeSpec.output.sampleRate);
    m_minimumPeriod = to_samples(minimumPeriod, m_activeSpec.output.sampleRate);

    hr = m_data->audioClient->GetService(
<<<<<<< HEAD
                __uuidof(IAudioRenderClient),
                (void**)&m_data->renderClient
                );
=======
        __uuidof(IAudioRenderClient),
        (void**)&m_data->renderClient
        );
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)
    if (FAILED(hr)) {
        LOGE() << "failed get renderClient";
        return false;
    }

    m_data->audioEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_data->audioEvent) {
        LOGE() << "failed create audioEvent";
        return false;
    }

    hr = m_data->audioClient->SetEventHandle(m_data->audioEvent);
    if (FAILED(hr)) {
        LOGE() << "failed set audioEvent";
        return false;
    }

    hr = m_data->audioClient->Start();
    if (FAILED(hr)) {
        LOGE() << "failed audioClient->Start()";
        return false;
    }

    return true;
}

void WasapiAudioDriver2::th_processAudioData()
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

            m_activeSpec.callback(m_activeSpec.userdata,
                                  m_surroundAudioBuffer.data(),
                                  (int)surroundBufferDesiredSize);

            for (uint32_t i = 0; i < actualFramesToRead; ++i) {
                uint8_t* frameStartPos = data + i * clientFrameSize;
                std::memcpy(frameStartPos, m_surroundAudioBuffer.data() + i * muFrameSize, muFrameSize);
                std::memset(frameStartPos + muFrameSize, 0, clientFrameSize - muFrameSize);
            }
        } else {
            m_activeSpec.callback(m_activeSpec.userdata, data, actualFramesToRead * clientFrameSize);
        }
    }
    m_data->renderClient->ReleaseBuffer(actualFramesToRead, 0);
}

void WasapiAudioDriver2::close()
{
    m_opened = false;
    m_audioThread.join();

    m_data->releaseClient();
}

bool WasapiAudioDriver2::isOpened() const
{
    return m_opened;
}

const IAudioDriver::Spec& WasapiAudioDriver2::activeSpec() const
{
    return m_activeSpec;
}

AudioDeviceID WasapiAudioDriver2::outputDevice() const
{
    return m_deviceId;
}

bool WasapiAudioDriver2::selectOutputDevice(const AudioDeviceID& id)
{
    bool result = true;

    if (m_deviceId == id) {
        return result;
    }

    m_deviceId = id;

    if (isOpened()) {
        close();
        result = open(m_activeSpec, &m_activeSpec);
    }

    if (result) {
        m_outputDeviceChanged.notify();
    }

    return result;
}

bool WasapiAudioDriver2::resetToDefaultOutputDevice()
{
    return selectOutputDevice(m_defaultDeviceId);
}

async::Notification WasapiAudioDriver2::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

AudioDeviceList WasapiAudioDriver2::availableOutputDevices() const
{
    return m_deviceList;
}

async::Notification WasapiAudioDriver2::availableOutputDevicesChanged() const
{
    return m_deviceListChanged;
}

bool WasapiAudioDriver2::setOutputDeviceBufferSize(unsigned int bufferSize)
{
    bool result = true;

    if (isOpened()) {
        close();
        m_activeSpec.output.samplesPerChannel = bufferSize;
        result = open(m_activeSpec, &m_activeSpec);
    }
    m_outputDeviceBufferSizeChanged.notify();

    return result;
}

async::Notification WasapiAudioDriver2::outputDeviceBufferSizeChanged() const
{
    return m_outputDeviceBufferSizeChanged;
}

std::vector<unsigned int> WasapiAudioDriver2::availableOutputDeviceBufferSizes() const
{
    std::vector<unsigned int> result;

    samples_t n = MAXIMUM_BUFFER_SIZE;
    samples_t min = std::max(m_minimumPeriod, MINIMUM_BUFFER_SIZE);

    while (n >= min) {
        result.push_back(n);
        n /= 2;
    }

    return result;
}

bool WasapiAudioDriver2::setOutputDeviceSampleRate(unsigned int sampleRate)
{
    bool result = true;

    if (isOpened()) {
        close();

        m_activeSpec.output.sampleRate = sampleRate;
        result = open(m_activeSpec, &m_activeSpec);
    }

    m_outputDeviceSampleRateChanged.notify();

    return result;
}

async::Notification WasapiAudioDriver2::outputDeviceSampleRateChanged() const
{
    return m_outputDeviceSampleRateChanged;
}

std::vector<unsigned int> WasapiAudioDriver2::availableOutputDeviceSampleRates() const
{
    return {
        44100,
        48000,
        88200,
        96000,
    };
}

void WasapiAudioDriver2::resume()
{
<<<<<<< HEAD

=======
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)
}

void WasapiAudioDriver2::suspend()
{
<<<<<<< HEAD

=======
>>>>>>> ba26e91143 (added new implementation of WasapiAudioDriver)
}
