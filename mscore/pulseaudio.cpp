//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "seq.h"
#include <pulse/pulseaudio.h>
#include "driver.h"
#include "preferences.h"

namespace Ms {

#define FRAMES 2048

//---------------------------------------------------------
//   PulseAudio
//---------------------------------------------------------

class PulseAudio : public Driver {
      Transport state;
      int runState;
      int _sampleRate;
      pa_sample_spec ss;
      pa_mainloop* pa_ml;
      pa_buffer_attr bufattr;
      float buffer[FRAMES * 2];
      pthread_t thread;

      static void paCallback(pa_stream* s, size_t len, void* data);
      static void* paLoop(void*);

   public:
      PulseAudio(Seq*);
      virtual ~PulseAudio();
      virtual bool init(bool hot = false);
      virtual bool start(bool hotPlug = false);
      virtual bool stop();
      virtual Transport getState() override { return state; }
      virtual int sampleRate() const { return _sampleRate;          }
      virtual void stopTransport()   { state = Transport::STOP; }
      virtual void startTransport()  { state = Transport::PLAY; }
      };

//---------------------------------------------------------
//   paCallback
//---------------------------------------------------------

void PulseAudio::paCallback(pa_stream* s, size_t len, void* data)
      {
      PulseAudio* pa = (PulseAudio*)data;
      constexpr size_t n = FRAMES * 2 * sizeof(float);
      if (len > n) {
            qDebug("PulseAudio:: buffer too large!");
            len = n;
            }
      pa->seq->process(len / (2 * sizeof(float)), pa->buffer);
      pa_stream_write(s, pa->buffer, len, nullptr, int64_t(0), PA_SEEK_RELATIVE);
      }

//---------------------------------------------------------
//   PulseAudio
//---------------------------------------------------------

PulseAudio::PulseAudio(Seq* s)
   : Driver(s)
      {
      _sampleRate = preferences.getInt(PREF_IO_ALSA_SAMPLERATE);
      state       = Transport::STOP;
      runState    = 0;
      }

//---------------------------------------------------------
//   ~PulseAudio
//---------------------------------------------------------

PulseAudio::~PulseAudio()
      {
      stop();
      }

//---------------------------------------------------------
//   pa_state_cb
//---------------------------------------------------------

static void pa_state_cb(pa_context* c, void* data)
      {
      int* pa_ready = (int*)data;
      switch (pa_context_get_state(c)) {
            // These are just here for reference
            case PA_CONTEXT_UNCONNECTED:
            case PA_CONTEXT_CONNECTING:
            case PA_CONTEXT_AUTHORIZING:
            case PA_CONTEXT_SETTING_NAME:
            default:
                  break;
            case PA_CONTEXT_FAILED:
            case PA_CONTEXT_TERMINATED:
                  *pa_ready = 2;
                  break;
            case PA_CONTEXT_READY:
                  *pa_ready = 1;
                  break;
            }
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool PulseAudio::init(bool)
      {
      pa_ml                     = pa_mainloop_new();
      pa_mainloop_api* pa_mlapi = pa_mainloop_get_api(pa_ml);
      pa_context* pa_ctx        = pa_context_new(pa_mlapi, "MuseScore");
      if (pa_context_connect(pa_ctx, NULL, pa_context_flags_t(0), NULL) != 0) {
            qDebug("PulseAudio Context Connect Failed with Error: %s", pa_strerror(pa_context_errno(pa_ctx)));
            return false;
            }

      int pa_ready = 0;
      pa_context_set_state_callback(pa_ctx, pa_state_cb, &pa_ready);

      while (pa_ready == 0)
            pa_mainloop_iterate(pa_ml, 1, NULL);
      if (pa_ready == 2)
            return false;

      ss.rate     = _sampleRate;
      ss.channels = 2;
      ss.format   = PA_SAMPLE_FLOAT32LE;

      pa_stream* playstream = pa_stream_new(pa_ctx, "Playback", &ss, NULL);
      if (!playstream) {
            qDebug("pa_stream_new failed: %s", pa_strerror(pa_context_errno(pa_ctx)));
            return false;
            }
      pa_stream_set_write_callback(playstream, paCallback, this);

      bufattr.fragsize  = (uint32_t)-1;
      bufattr.maxlength = FRAMES * 2 * sizeof(float);
      bufattr.minreq    = FRAMES * 1 * sizeof(float); // pa_usec_to_bytes(0, &ss);
      bufattr.prebuf    = (uint32_t)-1;
      bufattr.tlength   = bufattr.maxlength;

      int r = pa_stream_connect_playback(playstream, nullptr, &bufattr,
         PA_STREAM_NOFLAGS, nullptr, nullptr);

      if (r < 0) {
            qDebug("pa_stream_connect_playback failed");
            pa_context_disconnect(pa_ctx);
            pa_context_unref(pa_ctx);
            pa_mainloop_free(pa_ml);
            pa_ml = 0;
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   paLoop
//---------------------------------------------------------

void* PulseAudio::paLoop(void* data)
      {
      PulseAudio* pa = (PulseAudio*)data;
      pa->runState = 2;
      while (pa->runState == 2) {
            pa_mainloop_iterate(pa->pa_ml, 1, NULL);
            }
      pa->runState = 0;
      return 0;
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

bool PulseAudio::start(bool)
      {
      pthread_attr_t* attributes = (pthread_attr_t*) malloc(sizeof(pthread_attr_t));
      pthread_attr_init(attributes);
      if (pthread_create(&thread, attributes, paLoop, this))
            perror("creating thread failed:");
      pthread_attr_destroy(attributes);
      return true;
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

bool PulseAudio::stop()
      {
      if (runState == 2) {
            runState = 1;
            int i = 0;
            for (;i < 4; ++i) {
                  if (runState == 0)
                        break;
                  sleep(1);
                  }
            pthread_cancel(thread);
            pthread_join(thread, 0);
            }
      return true;
      }

//---------------------------------------------------------
//   getPulseaudioDriver
//    driver factory
//---------------------------------------------------------

Driver* getPulseAudioDriver(Seq* seq)
      {
      return new PulseAudio(seq);
      }
}

