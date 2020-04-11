//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
//
//  AlsaDriver based on code from Fons Adriaensen (clalsadr.cc)
//  Copyright (C) 2003 Fons Adriaensen
//  partly based on original work from Paul Davis
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

#ifndef __ALSA_H__
#define __ALSA_H__

#include <alsa/asoundlib.h>
#include <poll.h>

#include "config.h"
#include "driver.h"
#include "mididriver.h"

typedef struct pollfd PollFd;

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API

namespace Ms {

class MidiDriver;
enum class Transport : char;

//---------------------------------------------------------
//   AlsaDriver
//---------------------------------------------------------

class AlsaDriver {
      QString _name;
      typedef char* (*play_function)(const float*, char*, int, int);
      typedef char* (*clear_function)(char*, int, int);

      enum { MAXPFD = 8, MAXPLAY = 4 };

      bool setHwpar(snd_pcm_t* handle, snd_pcm_hw_params_t* hwpar);
      bool setSwpar(snd_pcm_t* handle, snd_pcm_sw_params_t* swpar);
      bool recover();

      unsigned int           _rate;
      snd_pcm_uframes_t      _frsize;
      unsigned int           _nfrags;
      snd_pcm_format_t       _play_format;
      snd_pcm_access_t       _play_access;
      snd_pcm_t*             _play_handle;
      snd_pcm_hw_params_t*   _play_hwpar;
      snd_pcm_sw_params_t*   _play_swpar;
      unsigned int           _play_nchan;
      int                    _play_npfd;
      PollFd                 _pfd [MAXPFD];
      snd_pcm_uframes_t      _capt_offs;
      snd_pcm_uframes_t      _play_offs;
      int                    _play_step;
      char*                  _play_ptr [MAXPLAY];
      int                    _stat;
      int                    _pcnt;
      bool                   _xrun;
      clear_function         _clear_func;
      play_function          _play_func;
      bool                   mmappedInterface;

      static char* clear_32le(char* dst, int step, int nfrm);
      static char* clear_24le(char* dst, int step, int nfrm);
      static char* clear_16le(char* dst, int step, int nfrm);
      static char* play_32le(const float* src, char* dst, int step, int nfrm);
      static char* play_24le(const float* src, char* dst, int step, int nfrm);
      static char* play_16le(const float* src, char* dst, int step, int nfrm);
      int playInit(snd_pcm_uframes_t len);
      snd_pcm_sframes_t pcmWait();
      snd_pcm_t* playHandle() const   { return _play_handle; }
      void clearChan(int chan, snd_pcm_uframes_t len) {
            _play_ptr[chan] = _clear_func(_play_ptr[chan], _play_step, len);
            }

    public:
      AlsaDriver(QString, unsigned, snd_pcm_uframes_t, unsigned);
      ~AlsaDriver();
      bool init();
      void printinfo();
      bool pcmStart();
      int pcmStop();
      snd_pcm_uframes_t fsize() const { return _frsize;      }
      unsigned int sampleRate() const { return _rate; }
      void write(int n, float* l, float* r);
      };

//---------------------------------------------------------
//   AlsaAudio
//---------------------------------------------------------

class AlsaAudio : public Driver {
      pthread_t thread;
      AlsaDriver* alsa;
      volatile int runAlsa;
      Transport state;
      bool seekflag;

      MidiDriver* midiDriver;

      void registerClient();

   public:
      AlsaAudio(Seq*);
      virtual ~AlsaAudio();
      virtual bool init(bool hot = false);
      virtual bool start(bool hotPlug = false);
      virtual bool stop();
      float* getLBuffer(long n);
      float* getRBuffer(long n);
      virtual bool isRealtime() const   { return false; }
      virtual void startTransport();
      virtual void stopTransport();
      virtual Transport getState() override;
      virtual int sampleRate() const;
      void alsaLoop();
      void write(int n, void* l);

      virtual void midiRead();
      virtual void updateOutPortCount(int maxport);
      };

}
#endif

