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

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <algorithm>
#include <thread>
#include <chrono>

#include "translation.h"
#include "log.h"
#include "runtime.h"

/* How many milliseconds to we allow musescore to be out-of-sync to jack
 * before we tell musescore to adjust (seek) its position
 */
#define FRAMESLIMIT 500

#define JACK_DEFAULT_DEVICE_ID "jack"
#define JACK_DEFAULT_IDENTIFY_AS "MuseScore"
static int g_jackTransportDelay = 0;

using namespace muse::audio;
namespace muse::audio {
static bool g_transportEnable = false;
// variables to communicate between soft-realtime jack thread and musescore
static msecs_t mpos_frame; // musescore frame and state
static jack_nframes_t muse_frame; // musescore frame and state
static jack_transport_state_t muse_state;
static jack_nframes_t jack_frame; // jack frame and state
static jack_transport_state_t jack_state;
static int g_musescore_is_synced; // tells jack-transport if musescore is synced
static jack_nframes_t g_nframes;
unsigned int g_samplerate;
msecs_t g_frameslimit;
static jack_nframes_t muse_seek_requested;
static int muse_seek_countdown = 0;

std::chrono::time_point<std::chrono::steady_clock> musescore_act_time;
static bool musescore_act;
static msecs_t musescore_act_seek;
static bool running_musescore_state;
//static std::vector<std::thread> threads;
static JackDriverState* s_jackDriver;

muse::audio::secs_t g_secs = 0;
muse::midi::tick_t g_tick = 0;

void musescore_state_check_musescore()
{
    jack_nframes_t pos = mpos_frame;
    if (pos > static_cast<jack_nframes_t>(g_jackTransportDelay)) {
        pos -= g_jackTransportDelay;
    }
    muse_frame = pos;

    if (s_jackDriver->isPlaying()) {
        muse_state = JackTransportRolling;
    } else {
        muse_state = JackTransportStopped;
    }
}

void musescore_state_do_seek()
{
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - musescore_act_time);
    auto ms = static_cast<msecs_t>(diff.count());
    auto millis = static_cast<msecs_t>((double)musescore_act_seek * 1000 / (double)g_samplerate);

    millis = static_cast<msecs_t>(std::max(static_cast<long int>(millis - ms), 0L));
    LOGW("Jack mst: really do musescore-seek to %lu (%lims) (diff: %i)  mf=%u/%li jf=%u lag: %lims",
         musescore_act_seek, millis, muse_frame - jack_frame, muse_frame, mpos_frame, jack_frame, ms);
    s_jackDriver->remoteSeek(millis);
    //mpos_frame = static_cast<msecs_t>(millis * g_samplerate / 1000);
}

/*
 * state thread
 */
void musescore_state()
{
    LOGI("Jack: start musescore_state thread");
    int is_seeking = 0;
    int cnt = 0;
    while (running_musescore_state) {
        musescore_state_check_musescore();
        cnt++;
        if (cnt > 4) {
            cnt = 0;
            /*
            LOGI("state: mframe=%u/%li jframe=%u (framedrift: %i / %i)  ms=%s  js=%s",
                 muse_frame, mpos_frame, jack_frame,
                 muse_frame - jack_frame,
                 mpos_frame - jack_frame,
                 (muse_state == JackTransportStopped ? "stop"
                  : (muse_state == JackTransportStarting ? "start"
                     : (muse_state == JackTransportRolling ? "roll" : "other"))),
                 (jack_state == JackTransportStopped ? "stop"
                  : (jack_state == JackTransportStarting ? "start"
                     : (jack_state == JackTransportRolling ? "roll" : "other"))));
                     */
        }
        if (musescore_act) {
            if (is_seeking) {
                // already seeking
            } else {
                if (labs(muse_frame - jack_frame) > g_frameslimit
                    && muse_seek_countdown == 0) { // && (muse_frame - jack_frame) != 0) {
                    musescore_state_do_seek();
                    is_seeking = 1;
                } else {
                    LOGI("Jack mst: act avoid musescore-seek to %lu (jack: %u) ", musescore_act_seek, jack_frame);
                    musescore_act = false;
                }
            }
        }
        if (is_seeking) {
            is_seeking++;
            if (is_seeking > 10) {
                is_seeking = 0;
                musescore_act = false;
            }
        }
        if (muse_seek_countdown) {
            muse_seek_countdown--;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    LOGW("Jack: quiting musescore_state thread");
}

bool musescore_seek(unsigned int pos)
{
    if (musescore_act) {
        return false; // already seeking
    } else {
        musescore_act_seek = pos;
        musescore_act = true;
        musescore_act_time = std::chrono::steady_clock::now(); // record clock that pos is valid for
    }
    return true;
}

void JackDriverState::changedPlaying() const
{
    if (!g_transportEnable) {
        return;
    }
    jack_client_t* client = static_cast<jack_client_t*>(jackDeviceHandle);

    if (s_jackDriver->isPlaying()) {
        jack_nframes_t frames = static_cast<jack_nframes_t>(s_jackDriver->playbackPositionInSeconds() * g_samplerate);
        LOGW("musescore tell transport to START at %u", frames);
        jack_transport_locate(client, frames);
        jack_transport_start(client);
    } else {
        LOGW("musescore tell transport to STOP");
        jack_transport_stop(client);
    }
}

void JackDriverState::changedPosition(muse::audio::secs_t secs, muse::midi::tick_t tick) const
{
    // ugly, but provide position to playbackPositionInSeconds
    g_secs = secs;
    g_tick = tick;

    if (!g_transportEnable) {
        return;
    }
    jack_client_t* client = static_cast<jack_client_t*>(jackDeviceHandle);
    jack_nframes_t frames = static_cast<jack_nframes_t>(s_jackDriver->playbackPositionInSeconds() * g_samplerate);

    // worker thread is updating muse_frame (which runs every 100ms)
    // thats why we can get out of sync with "ourself"
    if (labs((long int)muse_frame - (long int)frames) > g_frameslimit) {
        LOGW("musescore tell transport to LOCATE mf=%u, fr=%u diff: %i (lim: %li) g_secs: %f",
             muse_frame, frames,
             muse_frame - frames,
             g_frameslimit, g_secs.to_double());
        jack_transport_locate(client, frames);
        muse_frame = mpos_frame = frames;
        muse_seek_countdown = 10; // KLUDGE: avoid transport seeking musescore for some time
    }
}

// musescore has around 200ms inaccuracy in playbackPositionInSeconds
bool is_muse_jack_frame_sync(jack_nframes_t mf, jack_nframes_t jf)
{
    return labs((long int)jack_frame - (long int)muse_frame + g_jackTransportDelay) < g_frameslimit;
}

bool is_muse_jack_state_sync(jack_transport_state_t ms, jack_transport_state_t js)
{
    if (muse_state == JackTransportRolling) {
        return jack_state == JackTransportRolling
               || jack_state == JackTransportStarting;
    } else {
        return jack_state == JackTransportStopped;
    }
}

void jack_muse_update_verify_sync(JackDriverState* state, jack_client_t* client)
{
    jack_position_t jpos;
    jack_state = jack_transport_query(client, &jpos);
    jack_frame = jpos.frame;
    if (is_muse_jack_state_sync(muse_state, jack_state)
        && is_muse_jack_frame_sync(muse_frame, jack_frame)) {
        g_musescore_is_synced = 1;
    } else {
        g_musescore_is_synced = 0;
    }
}

static int framecnt = 0;

void check_jack_midi_transport(JackDriverState* state, jack_nframes_t nframes)
{
    jack_client_t* client = static_cast<jack_client_t*>(state->jackDeviceHandle);

    jack_muse_update_verify_sync(state, client);

    if (g_musescore_is_synced) {
        return;
    }

    framecnt++;
    if (framecnt > 40) {
        framecnt = 0;
        LOGI("jack-transport: mframe=%u/%li jframe=%u d=%i  ms=%s  js=%s  nf=%i d=%i\n",
             muse_frame, mpos_frame, jack_frame,
             muse_frame - jack_frame,
             (muse_state == JackTransportStopped ? "stop"
              : (muse_state == JackTransportStarting ? "start"
                 : (muse_state == JackTransportRolling ? "roll" : "other"))),
             (jack_state == JackTransportStopped ? "stop"
              : (jack_state == JackTransportStarting ? "start"
                 : (jack_state == JackTransportRolling ? "roll" : "other"))),
             nframes,
             g_jackTransportDelay);
    }

    bool state_sync = false;
    if (muse_state == JackTransportStopped
        && (jack_state == JackTransportStarting || jack_state == JackTransportRolling)) {
        state->remotePlayOrStop(true);
        muse_state = JackTransportRolling;
    } else if (muse_state == JackTransportRolling
               && jack_state == JackTransportStopped) {
        state->remotePlayOrStop(false);
        muse_state = JackTransportStopped;
    } else {
        state_sync = true;
    }

    if (!(is_muse_jack_frame_sync(muse_frame, jack_frame))) {
        jack_nframes_t pos = jack_frame;
        if (pos > static_cast<jack_nframes_t>(g_jackTransportDelay)) {
            pos -= g_jackTransportDelay;
        } else {
            pos = 0;
        }
        muse_seek_requested = pos;
        musescore_seek(pos);
    } else {
        g_musescore_is_synced = state_sync; // only sync if both state and position matches jack
        LOGW("jack-transport: SYNCED!");
    }
}

// because jack callbacks are soft-realtime we use no resources
static int handle_jack_sync(jack_transport_state_t ts, jack_position_t* pos, void* args)
{
    if (jack_frame != pos->frame
        || jack_state != ts
        || (!is_muse_jack_frame_sync(jack_frame, muse_frame))
        || (!is_muse_jack_state_sync(jack_state, muse_state))
        ) {
        jack_frame = pos->frame;
        jack_state = ts;
        g_musescore_is_synced = 0;
    }
    /*
    LOGW("jack-transport: SYNC ms=%s ts=%s  m/j-frame: %lu/%lu  sync? %s",
         (muse_state == JackTransportStopped ? "stop" :
          (muse_state == JackTransportStarting ? "start" :
           (muse_state == JackTransportRolling ? "roll" : "other"))),
         (ts == JackTransportStopped ? "stop" :
          (ts == JackTransportStarting ? "start" :
           (ts == JackTransportRolling ? "roll" : "other"))),
         muse_frame,
         jack_frame,
         g_musescore_is_synced ? "---- YES ----" : " -- no --");
    */
    return g_musescore_is_synced;
}

/*
 * AUDIO
 */

static int jack_process_callback(jack_nframes_t nframes, void* args)
{
    JackDriverState* state = static_cast<JackDriverState*>(args);

    jack_default_audio_sample_t* l = (float*)jack_port_get_buffer(state->outputPorts[0], nframes);
    jack_default_audio_sample_t* r = (float*)jack_port_get_buffer(state->outputPorts[1], nframes);

    uint8_t* stream = (uint8_t*)state->buffer;
    state->deviceSpec.callback(state->deviceSpec.userdata, stream, nframes * state->deviceSpec.channels * sizeof(float));
    float* sp = state->buffer;
    for (size_t i = 0; i < nframes; i++) {
        *l++ = *sp++;
        *r++ = *sp++;
    }
    jack_client_t* client = static_cast<jack_client_t*>(state->jackDeviceHandle);
    // if (!isConnected()) {
    //    LOGI() << "---- JACK-midi output sendEvent SORRY, not connected";
    //    return make_ret(Err::MidiNotConnected);
    // }

    if (g_transportEnable) {
        if (muse_state == JackTransportRolling) {
            mpos_frame += nframes;
        }
        check_jack_midi_transport(state, nframes);
    }

    return 0;
}

static int handle_buffersize_change(jack_nframes_t nframes, void* arg)
{
    g_nframes = nframes;
    return 0; // successfully reallocated buffer
}

static void jack_cleanup_callback(void*)
{
}
}

JackDriverState::JackDriverState(IAudioDriver* amm, bool transportEnable)
{
    s_jackDriver = this;
    deviceId = JACK_DEFAULT_DEVICE_ID;
    m_deviceName = JACK_DEFAULT_IDENTIFY_AS;
    m_audiomidiManager = amm;
    g_transportEnable = transportEnable;
}

JackDriverState::~JackDriverState()
{
    if (jackDeviceHandle != nullptr) {
        jack_client_close(static_cast<jack_client_t*>(jackDeviceHandle));
    }
    delete[] buffer;
}

std::string JackDriverState::name() const
{
    return deviceId;
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

void JackDriverState::setAudioDelayCompensate(const int frames)
{
    g_jackTransportDelay = frames;
}

bool JackDriverState::open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec)
{
    LOGI("using jack-transport: %b, jackTransportDelay: %i", g_transportEnable, g_jackTransportDelay);
    if (isOpened()) {
        LOGW() << "Jack is already opened";
        return true;
    }

    deviceSpec.samples  = spec.samples; // client doesn't set sample-rate
    deviceSpec.channels = spec.channels;
    deviceSpec.callback = spec.callback;
    deviceSpec.userdata = spec.userdata;

    const char* clientName = m_deviceName.c_str();
    jack_status_t status;
    jack_client_t* handle;
    if (!(handle = jack_client_open(clientName, JackNullOption, &status))) {
        LOGE() << "jack_client_open() failed: " << status;
        return false;
    }
    jackDeviceHandle = handle;

    unsigned int jackSamplerate = jack_get_sample_rate(handle);
    deviceSpec.sampleRate = jackSamplerate;
    g_samplerate = jackSamplerate;
    // FIX: at samplerate change, this need to be adjusted
    g_frameslimit = static_cast<msecs_t>((double)g_samplerate * (double)FRAMESLIMIT / 1000.0L);
    if (spec.sampleRate != jackSamplerate) {
        LOGW() << "Musescores samplerate: " << spec.sampleRate << ", is NOT the same as jack's: " << jackSamplerate;
        // FIX: enable this if it is possible for user to adjust samplerate (AUDIO_SAMPLE_RATE_KEY)
        //jack_client_close(handle);
        //return false;
    }

    if (g_transportEnable) {
        // start musescore state thread
        running_musescore_state = true;
        std::thread thread_musescore_state(musescore_state);
        thread_musescore_state.detach();
        //std::vector<std::thread> threadv;
        //threadv.push_back(std::move(thread_musescore_state));
        //threads = std::move(threadv);
    }

    jack_set_sample_rate_callback(handle, jack_srate_callback, (void*)&deviceSpec);

    jack_port_t* output_port_left = jack_port_register(handle, "audio_out_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    outputPorts.push_back(output_port_left);
    jack_port_t* output_port_right = jack_port_register(handle, "audio_out_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    outputPorts.push_back(output_port_right);
    deviceSpec.samples = jack_get_buffer_size(handle);
    buffer = new float[deviceSpec.samples * deviceSpec.channels];

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = IAudioDriver::Format::AudioF32;
        activeSpec->sampleRate = jackSamplerate;
        deviceSpec = *activeSpec;
    }

    jack_on_shutdown(handle, jack_cleanup_callback, (void*)this);
    jack_set_process_callback(handle, jack_process_callback, (void*)this);
    jack_set_sync_callback(handle, handle_jack_sync, (void*)this);
    jack_set_buffer_size_callback(handle, handle_buffersize_change, NULL);
    if (jack_activate(handle)) {
        LOGE() << "cannot activate client";
        return false;
    }

    muse_seek_requested = 0;
    mpos_frame = muse_frame = static_cast<unsigned int>(s_jackDriver->playbackPositionInSeconds() * jackSamplerate);
    if (muse_frame >= g_jackTransportDelay) {
        muse_frame -= g_jackTransportDelay;
    }

    if (s_jackDriver->isPlaying()) {
        muse_state = JackTransportRolling;
    } else {
        muse_state = JackTransportStopped;
    }

    return true;
}

void JackDriverState::close()
{
    jack_client_close(static_cast<jack_client_t*>(jackDeviceHandle));
    jackDeviceHandle = nullptr;
    delete[] buffer;
    buffer = nullptr;
}

bool JackDriverState::isOpened() const
{
    return jackDeviceHandle != nullptr;
}

bool JackDriverState::isPlaying() const
{
    return m_audiomidiManager->isPlaying();
}

// FIX: return type double
float JackDriverState::playbackPositionInSeconds() const
{
    // this round-trip could be avoided if the caller uses info from changedposition
    return g_secs.to_double();
}

void JackDriverState::remotePlayOrStop(bool ps) const
{
    m_audiomidiManager->remotePlayOrStop(ps);
}

void JackDriverState::remoteSeek(msecs_t millis) const
{
    m_audiomidiManager->remoteSeek(millis);
}
