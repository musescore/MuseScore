/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore BVBA and others
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

#define JACK_DEFAULT_DEVICE_ID "jack"
#define JACK_DEFAULT_IDENTIFY_AS "MuseScore"

using namespace muse::audio;

namespace muse::audio {
static int jack_process_callback(jack_nframes_t nframes, void* args)
{
    JackDriverState* state = static_cast<JackDriverState*>(args);

    jack_default_audio_sample_t* l = (float*)jack_port_get_buffer(state->m_outputPorts[0], nframes);
    jack_default_audio_sample_t* r = (float*)jack_port_get_buffer(state->m_outputPorts[1], nframes);

    uint8_t* stream = (uint8_t*)state->m_buffer;
    state->m_spec.callback(state->m_spec.userdata, stream, nframes * state->m_spec.channels * sizeof(float));
    float* sp = state->m_buffer;
    for (size_t i = 0; i < nframes; i++) {
        *l++ = *sp++;
        *r++ = *sp++;
    }
    return 0;
}

static void jack_cleanup_callback(void*)
{
}
}

JackDriverState::JackDriverState()
{
    m_deviceId = JACK_DEFAULT_DEVICE_ID;
    m_deviceName = JACK_DEFAULT_IDENTIFY_AS;
}

JackDriverState::~JackDriverState()
{
    if (m_jackDeviceHandle != nullptr) {
        jack_client_close(static_cast<jack_client_t*>(m_jackDeviceHandle));
    }
    delete[] m_buffer;
}

std::string JackDriverState::name() const
{
    return m_deviceId;
}

std::string JackDriverState::deviceName() const
{
    return m_deviceName;
}

void JackDriverState::deviceName(const std::string newDeviceName)
{
    m_deviceName = newDeviceName;
}

int jack_srate_callback(jack_nframes_t newSampleRate, void* args)
{
    IAudioDriver::Spec* spec = (IAudioDriver::Spec*)args;
    if (newSampleRate != spec->sampleRate) {
        LOGW() << "Jack reported system sampleRate change. new samplerate: " << newSampleRate << ", MuseScore: " << spec->sampleRate;
        // FIX: notify Musescore audio-layer to adjust musescores samplerate
    }
    spec->sampleRate = newSampleRate;
    return 0;
}

bool JackDriverState::open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec)
{
    if (isOpened()) {
        LOGW() << "Jack is already opened";
        return true;
    }
    // m_spec.samples  = spec.samples; // client doesn't set sample-rate
    m_spec.channels = spec.channels;
    m_spec.callback = spec.callback;
    m_spec.userdata = spec.userdata;
    const char* clientName = m_deviceName.c_str();
    jack_status_t status;
    jack_client_t* handle;
    if (!(handle = jack_client_open(clientName, JackNullOption, &status))) {
        LOGE() << "jack_client_open() failed: " << status;
        return false;
    }
    m_jackDeviceHandle = handle;

    unsigned int jackSamplerate = jack_get_sample_rate(handle);
    m_spec.sampleRate = jackSamplerate;
    if (spec.sampleRate != jackSamplerate) {
        LOGW() << "Musescores samplerate: " << spec.sampleRate << ", is NOT the same as jack's: " << jackSamplerate;
        // FIX: enable this if it is possible for user to adjust samplerate (AUDIO_SAMPLE_RATE_KEY)
        //jack_client_close(handle);
        //return false;
    }

    jack_set_sample_rate_callback(handle, jack_srate_callback, (void*)&m_spec);

    jack_port_t* output_port_left = jack_port_register(handle, "audio_out_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    m_outputPorts.push_back(output_port_left);
    jack_port_t* output_port_right = jack_port_register(handle, "audio_out_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    m_outputPorts.push_back(output_port_right);
    m_spec.samples = jack_get_buffer_size(handle);
    m_buffer = new float[m_spec.samples * m_spec.channels];

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = IAudioDriver::Format::AudioF32;
        activeSpec->sampleRate = jackSamplerate;
        m_spec = *activeSpec;
    }

    jack_on_shutdown(handle, jack_cleanup_callback, (void*)this);
    jack_set_process_callback(handle, jack_process_callback, (void*)this);

    if (jack_activate(handle)) {
        LOGE() << "cannot activate client";
        return false;
    }

    return true;
}

void JackDriverState::close()
{
    jack_client_close(static_cast<jack_client_t*>(m_jackDeviceHandle));
    m_jackDeviceHandle = nullptr;
    delete[] m_buffer;
    m_buffer = nullptr;
}

bool JackDriverState::isOpened() const
{
    return m_jackDeviceHandle != nullptr;
}
