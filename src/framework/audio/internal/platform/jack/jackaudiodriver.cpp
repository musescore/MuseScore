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
#include <jack/midiport.h>
#include "framework/midi/miditypes.h"
#include "framework/midi/midierrors.h"
#include "framework/midi/imidiinport.h"

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
using namespace muse::midi;
namespace muse::audio {
/*
 * MIDI
 */

muse::Ret sendEvent_noteonoff(void* pb, int framePos, const muse::midi::Event& e)
{
    unsigned char* p = jack_midi_event_reserve(pb, framePos, 3);
    if (p == 0) {
        LOGE("JackMidi: buffer overflow, event lost");
        return muse::Ret(false);
    }
    // FIX: is opcode an compatible MIDI enumeration?
    if (e.opcode() == muse::midi::Event::Opcode::NoteOn) {
        p[0] = /* e.opcode() */ 0x90 | e.channel();
    } else {
        p[0] = /* e.opcode() */ 0x80 | e.channel();
    }
    p[1] = e.note();
    p[2] = e.velocity();
    return muse::Ret(true);
}

muse::Ret sendEvent_control(void* pb, int framePos, const Event& e)
{
    unsigned char* p = jack_midi_event_reserve(pb, framePos, 3);
    if (p == 0) {
        LOGE("JackMidi: buffer overflow, event lost");
        return muse::Ret(false);
    }
    p[0] = /* e.opcode() */ 0xb0 | e.channel();
    p[1] = e.index();
    p[2] = e.data();
    return muse::Ret(true);
}

muse::Ret sendEvent_program(void* pb, int framePos, const Event& e)
{
    unsigned char* p = jack_midi_event_reserve(pb, framePos, 2);
    if (p == 0) {
        LOGE("JackMidiOutput: buffer overflow, event lost");
        return muse::Ret(false);
    }
    p[0] = /* e.opcode() */ 0xc0 | e.channel();
    p[1] = e.program();
    return muse::Ret(true);
}

muse::Ret sendEvent_pitchbend(void* pb, int framePos, const Event& e)
{
    unsigned char* p = jack_midi_event_reserve(pb, framePos, 3);
    if (p == 0) {
        LOGE("JackMidiOutput: buffer overflow, event lost");
        return muse::Ret(false);
    }
    p[0] = /* e.opcode() */ 0xe0 | e.channel();
    p[1] = e.data(); // dataA
    p[2] = e.velocity(); // dataB
    return muse::Ret(true);
}

muse::Ret sendEvent(const Event& e, void* pb)
{
    int framePos = 0;
    switch (e.opcode()) {
    // FIX: Event::Opcode::POLYAFTER ?
    case Event::Opcode::NoteOn:
        return sendEvent_noteonoff(pb, framePos, e);
    case Event::Opcode::NoteOff:
        return sendEvent_noteonoff(pb, framePos, e);
    case Event::Opcode::ControlChange:
        return sendEvent_control(pb, framePos, e);
    case Event::Opcode::ProgramChange:
        return sendEvent_control(pb, framePos, e);
    case Event::Opcode::PitchBend:
        return sendEvent_pitchbend(pb, framePos, e);
    default:
        NOT_SUPPORTED << "event: " << e.to_string();
        return make_ret(muse::midi::Err::MidiNotSupported);
    }

    return Ret(true);
}

/*
 * AUDIO
 */

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
    jack_client_t* client = static_cast<jack_client_t*>(state->m_jackDeviceHandle);
    // if (!isConnected()) {
    //    LOGI() << "---- JACK-midi output sendEvent SORRY, not connected";
    //    return make_ret(Err::MidiNotConnected);
    // }
    if (!state->m_midiOutputPorts.empty()) {
        jack_port_t* port = state->m_midiOutputPorts.front();
        if (port) {
            int segmentSize = jack_get_buffer_size(client);
            void* pb = jack_port_get_buffer(port, segmentSize);
            // handle midi
            // FIX: can portBuffer be nullptr?
            muse::midi::Event e;
            while (1) {
                if (state->m_midiQueue.pop(e)) {
                    sendEvent(e, pb);
                } else {
                    break;
                }
            }
        }
    } else {
        muse::midi::Event e;
        while (1) {
            if (state->m_midiQueue.pop(e)) {
                LOGW() << "no jack-midi-outport, consumed unused Event: " << e.to_string();
            } else {
                break;
            }
        }
    }

    if (!state->m_midiInputPorts.empty()) {
        jack_port_t* port = state->m_midiInputPorts.front();
        if (port) {
            int segmentSize = jack_get_buffer_size(client);
            void* pb = jack_port_get_buffer(port, segmentSize);
            if (pb) {
                muse::midi::Event ev;
                jack_nframes_t n = jack_midi_get_event_count(pb);
                for (jack_nframes_t i = 0; i < n; ++i) {
                    jack_midi_event_t event;
                    if (jack_midi_event_get(&event, pb, i) != 0) {
                        continue;
                    }
                    int type = event.buffer[0];
                    uint32_t data = 0;
                    if ((type & 0xf0) == 0x90
                        || (type & 0xf0) == 0x90) {
                        data = 0x90
                               | (type & 0x0f)
                               | ((event.buffer[1] & 0x7F) << 8)
                               | ((event.buffer[2] & 0x7F) << 16);
                        Event e = Event::fromMIDI10Package(data);
                        e = e.toMIDI20();
                        if (e) {
                            LOGI("-- jack midi-input-port send %i,%i,%i",
                                 event.buffer[1],
                                 event.buffer[2],
                                 event.buffer[0]);
                            state->m_eventReceived.send(static_cast<tick_t>(0), e);
                        }
                    }
                }
            }
        }
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

    // midi input
    jack_port_t* midi_input_port = jack_port_register(handle, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    m_midiInputPorts.push_back(midi_input_port);

    // midi output
    int portFlag = JackPortIsOutput;
    const char* portType = JACK_DEFAULT_MIDI_TYPE;
    jack_port_t* port = jack_port_register(handle, "Musescore", portType, portFlag, 0);
    m_midiOutputPorts.push_back(port);

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

/*
 * MIDI
 */

bool JackDriverState::pushMidiEvent(muse::midi::Event& e)
{
    m_midiQueue.push(e);
    return true;
}

void JackDriverState::registerMidiInputQueue(async::Channel<muse::midi::tick_t, muse::midi::Event > midiInputQueue)
{
    m_eventReceived = midiInputQueue;
}

std::vector<muse::midi::MidiDevice> JackDriverState::availableMidiDevices(muse::midi::MidiPortDirection direction) const
{
    std::vector<muse::midi::MidiDevice> ports;
    std::vector<muse::midi::MidiDevice> ret;
    jack_client_t* client = static_cast<jack_client_t*>(m_jackDeviceHandle);
    const char** prts = jack_get_ports(client, 0, "midi", 0);
    if (!prts) {
        return ports;
    }
    int devIndex = 0;
    for (const char** p = prts; p && *p; ++p) {
        jack_port_t* port = jack_port_by_name(client, *p);
        int flags = jack_port_flags(port);

        if ((flags & JackPortIsInput)
            && direction == muse::midi::MidiPortDirection::Output) {
            continue;
        }
        if ((flags & JackPortIsOutput)
            && direction == muse::midi::MidiPortDirection::Input) {
            continue;
        }

        char buffer[128];
        strncpy(buffer, *p, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = 0;

        if (strncmp(buffer, "MuseScore", 9) == 0) {
            continue;
        }
        muse::midi::MidiDevice dev;
        dev.name = buffer;
        dev.id = makeUniqueDeviceId(devIndex++, 0, 0);
        ports.push_back(std::move(dev));
    }

    free(prts);

    return ports;
}
