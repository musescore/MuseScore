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
#include "jackaudiodriver.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <thread> // Used by usleep
#include <chrono> // Used by usleep

#include "translation.h"
#include "log.h"
#include "runtime.h"

using namespace muse::audio;

struct JackData
{
    float* buffer = nullptr;
    jack_client_t* jackDeviceHandle = nullptr;
    unsigned long samples = 0;
    int channels = 0;
    std::vector<jack_port_t*> outputPorts;
    IAudioDriver::Callback callback;
    void* userdata = nullptr;
};

static JackData* s_jackData{ nullptr };
static muse::audio::IAudioDriver::Spec s_format2;

int muse::audio::jack_process_callback(jack_nframes_t nframes, void*)
{
    JackData* data = s_jackData;

    jack_default_audio_sample_t* l = (float*)jack_port_get_buffer(data->outputPorts[0], nframes);
    jack_default_audio_sample_t* r = (float*)jack_port_get_buffer(data->outputPorts[1], nframes);

    uint8_t* stream = (uint8_t*)data->buffer;
    data->callback(data->userdata, stream, nframes * data->channels * sizeof(float));
    float* sp = data->buffer;
    for (size_t i = 0; i < nframes; i++) {
        *l++ = *sp++;
        *r++ = *sp++;
    }
    return 0;
}

void muse::audio::jack_cleanup_callback(void*)
{
}

void jackCleanup()
{
    if (!s_jackData) {
        return;
    }

    if (nullptr != s_jackData->buffer) {
        delete[] s_jackData->buffer;
    }

    delete s_jackData;
    s_jackData = nullptr;
}

JackAudioDriver::JackAudioDriver()
{
    m_deviceId = DEFAULT_DEVICE_ID;
}

JackAudioDriver::~JackAudioDriver()
{
    jackCleanup();
}

void JackAudioDriver::init()
{
    m_devicesListener.startWithCallback([this]() {
        return availableOutputDevices();
    });

    m_devicesListener.devicesChanged().onNotify(this, [this]() {
        m_availableOutputDevicesChanged.notify();
    });
}

std::string JackAudioDriver::name() const
{
    return "MUAUDIO(JACK)";
}

int jack_srate_callback(jack_nframes_t nframes, void* args)
{
    IAudioDriver::Spec* spec = (IAudioDriver::Spec*)args;
    LOGI() << "Jack reported sampleRate change. Pray to god, musescores samplerate: " << spec->sampleRate << ", is the same as jacks: " <<
        nframes;
    return 0;
}

bool JackAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    s_jackData = new JackData();
    // s_jackData->samples  = spec.samples; // client doesn't set sample-rate
    s_jackData->channels = spec.channels;
    s_jackData->callback = spec.callback;
    s_jackData->userdata = spec.userdata;
    // FIX: "default" is not a good name for jack-clients
    //  const char *clientName =
    //      outputDevice().c_str() == "default" ? "MuseScore" :
    //      outputDevice().c_str();
    const char* clientName = "MuseScore";
    LOGI() << "clientName: " << clientName;

    jack_status_t status;
    jack_client_t* handle;
    if (!(handle = jack_client_open(clientName, JackNullOption, &status))) {
        LOGE() << "jack_client_open() failed: " << status;
        return false;
    }

    jack_set_sample_rate_callback(handle, jack_srate_callback, (void*)&spec);

    s_jackData->jackDeviceHandle = handle;

    jack_port_t* output_port_left = jack_port_register(handle, "audio_out_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    s_jackData->outputPorts.push_back(output_port_left);
    jack_port_t* output_port_right = jack_port_register(handle, "audio_out_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    s_jackData->outputPorts.push_back(output_port_right);

    s_jackData->samples = jack_get_buffer_size(handle);
    LOGI() << "buffer size (in samples): " << s_jackData->samples;

    unsigned int jackSamplerate = jack_get_sample_rate(handle);
    LOGI() << "sampleRate used by jack: " << jackSamplerate;
    if (spec.sampleRate != jackSamplerate) {
        LOGW() << "Musescores samplerate: " << spec.sampleRate << ", is NOT the same as jack's: " << jackSamplerate;
        // FIX: enable this if it is possible for user to adjust samplerate (AUDIO_SAMPLE_RATE_KEY)
        //jack_client_close(handle);
        //return false;
    }

    s_jackData->buffer = new float[s_jackData->samples * s_jackData->channels];

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioF32;
        activeSpec->sampleRate = jackSamplerate;
        s_format2 = *activeSpec;
    }

    jack_on_shutdown(handle, jack_cleanup_callback, 0);
    jack_set_process_callback(handle, jack_process_callback, (void*)&s_jackData);

    if (jack_activate(handle)) {
        LOGE() << "cannot activate client";
        return false;
    }

    return true;
}

void JackAudioDriver::close()
{
    jackCleanup();
}

bool JackAudioDriver::isOpened() const
{
    return s_jackData != nullptr;
}

const JackAudioDriver::Spec& JackAudioDriver::activeSpec() const
{
    return s_format2;
}

AudioDeviceID JackAudioDriver::outputDevice() const
{
    return m_deviceId;
}

bool JackAudioDriver::selectOutputDevice(const AudioDeviceID& deviceId)
{
    if (m_deviceId == deviceId) {
        return true;
    }

    bool reopen = isOpened();
    close();
    m_deviceId = deviceId;

    bool ok = true;
    if (reopen) {
        ok = open(s_format2, &s_format2);
    }

    if (ok) {
        m_outputDeviceChanged.notify();
    }

    return ok;
}

bool JackAudioDriver::resetToDefaultOutputDevice()
{
    return selectOutputDevice(DEFAULT_DEVICE_ID);
}

async::Notification JackAudioDriver::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

AudioDeviceList JackAudioDriver::availableOutputDevices() const
{
    AudioDeviceList devices;
    devices.push_back({ DEFAULT_DEVICE_ID, muse::trc("audio", "System default") });

    return devices;
}

async::Notification JackAudioDriver::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}

unsigned int JackAudioDriver::outputDeviceBufferSize() const
{
    return s_format2.samples;
}

bool JackAudioDriver::setOutputDeviceBufferSize(unsigned int bufferSize)
{
    if (s_format2.samples == bufferSize) {
        return true;
    }

    bool reopen = isOpened();
    close();
    s_format2.samples = bufferSize;

    bool ok = true;
    if (reopen) {
        ok = open(s_format2, &s_format2);
    }

    if (ok) {
        m_bufferSizeChanged.notify();
    }

    return ok;
}

async::Notification JackAudioDriver::outputDeviceBufferSizeChanged() const
{
    return m_bufferSizeChanged;
}

std::vector<unsigned int> JackAudioDriver::availableOutputDeviceBufferSizes() const
{
    std::vector<unsigned int> result;

    unsigned int n = MAXIMUM_BUFFER_SIZE;
    while (n >= MINIMUM_BUFFER_SIZE) {
        result.push_back(n);
        n /= 2;
    }

    std::sort(result.begin(), result.end());

    return result;
}

unsigned int JackAudioDriver::outputDeviceSampleRate() const
{
    return s_format2.sampleRate;
}

bool JackAudioDriver::setOutputDeviceSampleRate(unsigned int sampleRate)
{
    if (s_format2.sampleRate == sampleRate) {
        return true;
    }

    bool reopen = isOpened();
    close();
    s_format2.sampleRate = sampleRate;

    bool ok = true;
    if (reopen) {
        ok = open(s_format2, &s_format2);
    }

    if (ok) {
        m_sampleRateChanged.notify();
    }

    return ok;
}

async::Notification JackAudioDriver::outputDeviceSampleRateChanged() const
{
    return m_sampleRateChanged;
}

std::vector<unsigned int> JackAudioDriver::availableOutputDeviceSampleRates() const
{
    return {
        44100,
        48000,
        88200,
        96000,
    };
}

void JackAudioDriver::resume()
{
}

void JackAudioDriver::suspend()
{
}
