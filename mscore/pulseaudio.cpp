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

#define FRAMES 2048

//---------------------------------------------------------
//   PulseAudio
//---------------------------------------------------------

class PulseAudio : public Driver {
      int state;
      int runState;
      int _sampleRate;
      pa_sample_spec ss;
      pa_mainloop* pa_ml;
      pa_buffer_attr bufattr;
      float buffer[FRAMES * 2 * sizeof(short)];
      pthread_t thread;

      static void paCallback(pa_stream* s, size_t len, void* data);
      static void* paLoop(void*);

   public:
      PulseAudio(Seq*);
      virtual ~PulseAudio();
      virtual bool init();
      virtual bool start();
      virtual bool stop();
      virtual int getState() { return state; }
      virtual int sampleRate() const { return _sampleRate;          }
      virtual void stopTransport()   { state = Seq::TRANSPORT_STOP; }
      virtual void startTransport()  { state = Seq::TRANSPORT_PLAY; }
      };

//---------------------------------------------------------
//   paCallback
//---------------------------------------------------------

void PulseAudio::paCallback(pa_stream* s, size_t len, void* data)
      {
      PulseAudio* pa = (PulseAudio*)data;
      size_t n = FRAMES * 2 * sizeof(float);
      if (len > n)
            len = n;
      int frames = len / (2 * sizeof(float));
      float* p = pa->buffer;
      memset(p, 0, len);
      pa->seq->process(frames, p);
      pa_stream_write(s, p, len, NULL, 0LL, PA_SEEK_RELATIVE);
      }

//---------------------------------------------------------
//   PulseAudio
//---------------------------------------------------------

PulseAudio::PulseAudio(Seq* s)
   : Driver(s)
      {
      _sampleRate = 48000;
      state       = Seq::TRANSPORT_STOP;
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

bool PulseAudio::init()
      {
      pa_ml                     = pa_mainloop_new();
      pa_mainloop_api* pa_mlapi = pa_mainloop_get_api(pa_ml);
      pa_context* pa_ctx        = pa_context_new(pa_mlapi, "MuseScore");
      pa_context_connect(pa_ctx, NULL, pa_context_flags_t(0), NULL);

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
            printf("pa_stream_new failed\n");
            return false;
            }
      pa_stream_set_write_callback(playstream, paCallback, this);

      bufattr.fragsize  = (uint32_t)-1;
      bufattr.maxlength = FRAMES * 2 * sizeof(float);
      bufattr.minreq    = FRAMES * 1 * sizeof(float); // pa_usec_to_bytes(0, &ss);
      bufattr.prebuf    = (uint32_t)-1;
      bufattr.tlength   = bufattr.maxlength;
      int r = pa_stream_connect_playback(playstream, NULL, &bufattr,
         pa_stream_flags_t(PA_STREAM_INTERPOLATE_TIMING
         | PA_STREAM_ADJUST_LATENCY
         | PA_STREAM_AUTO_TIMING_UPDATE),
         NULL, NULL);

      if (r < 0) {
            // Old pulse audio servers don't like the ADJUST_LATENCY flag, so retry without that
            r = pa_stream_connect_playback(playstream, NULL, &bufattr,
               pa_stream_flags_t(PA_STREAM_INTERPOLATE_TIMING
               | PA_STREAM_AUTO_TIMING_UPDATE),
               NULL, NULL);
            }
      if (r < 0) {
            printf("pa_stream_connect_playback failed\n");
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
      while (pa->runState == 2)
            pa_mainloop_iterate(pa->pa_ml, 1, NULL);
      pa->runState = 0;
      return 0;
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

bool PulseAudio::start()
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

