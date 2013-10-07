//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: pa.cpp 5662 2012-05-23 07:35:47Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "preferences.h"
#include "libmscore/score.h"
#include "musescore.h"
#include "seq.h"
#include "pa.h"

#ifdef USE_ALSA
#include "alsa.h"
#include "alsamidi.h"
#endif

#include <portaudio.h>
#include "mididriver.h"
#include "pm.h"

namespace Ms {

static PaStream* stream;

//---------------------------------------------------------
//   paCallback
//---------------------------------------------------------

int paCallback(const void*, void* out, long unsigned frames,
   const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void *)
      {
      seq->process((unsigned)frames, (float*)out);
      return 0;
      }

//---------------------------------------------------------
//   Portaudio
//---------------------------------------------------------

Portaudio::Portaudio(Seq* s)
   : Driver(s)
      {
      _sampleRate = 48000;    // will be replaced by device default sample rate
      initialized = false;
      state       = Seq::TRANSPORT_STOP;
      seekflag    = false;
      midiDriver  = 0;
      }

//---------------------------------------------------------
//   ~Portaudio
//---------------------------------------------------------

Portaudio::~Portaudio()
      {
      if (initialized) {
            PaError err = Pa_CloseStream(stream);
            if (err != paNoError)
                  qDebug("Portaudio close stream failed: %s", Pa_GetErrorText(err));
            Pa_Terminate();
            }
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool Portaudio::init()
      {
      PaError err = Pa_Initialize();
      if (err != paNoError) {
            qDebug("Portaudio initialize failed: %s", Pa_GetErrorText(err));
            return false;
            }
      initialized = true;
      if (MScore::debugMode)
            qDebug("using PortAudio Version: %s", Pa_GetVersionText());

      PaDeviceIndex idx = preferences.portaudioDevice;
      if (idx < 0)
            idx = Pa_GetDefaultOutputDevice();

      const PaDeviceInfo* di = Pa_GetDeviceInfo(idx);
      _sampleRate = int(di->defaultSampleRate);

      /* Open an audio I/O stream. */
      struct PaStreamParameters out;
      memset(&out, 0, sizeof(out));

      out.device           = idx;
      out.channelCount     = 2;
      out.sampleFormat     = paFloat32;
#ifdef Q_OS_MAC
      out.suggestedLatency = 0.020;
#else // on windows, this small latency causes some problem
      out.suggestedLatency = 0.100;
#endif
      out.hostApiSpecificStreamInfo = 0;

      err = Pa_OpenStream(&stream, 0, &out, double(_sampleRate), 0, 0, paCallback, (void*)this);
      if (err != paNoError) {
            // fall back to default device:
            out.device = Pa_GetDefaultOutputDevice();
            err = Pa_OpenStream(&stream, 0, &out, double(_sampleRate), 0, 0, paCallback, (void*)this);
            if (err != paNoError) {
                  qDebug("Portaudio open stream %d failed: %s", idx, Pa_GetErrorText(err));
                  return false;
                  }
            }
      const PaStreamInfo* si = Pa_GetStreamInfo(stream);
      if (si)
            _sampleRate = int(si->sampleRate);
#ifdef USE_ALSA
      midiDriver = new AlsaMidiDriver(seq);
#endif
#ifdef USE_PORTMIDI
      midiDriver = new PortMidiDriver(seq);
#endif
      if (midiDriver && !midiDriver->init()) {
            qDebug("Init midi driver failed");
            delete midiDriver;
            midiDriver = 0;
#ifdef USE_PORTMIDI
            return true;                  // return OK for audio driver; midi is only input
#else
            return false;
#endif
            }
      return true;
      }

//---------------------------------------------------------
//   apiList
//---------------------------------------------------------

QStringList Portaudio::apiList() const
      {
      QStringList al;

      PaHostApiIndex apis = Pa_GetHostApiCount();
      for (PaHostApiIndex i = 0; i < apis; ++i) {
            const PaHostApiInfo* info = Pa_GetHostApiInfo(i);
            if (info)
                  al.append(info->name);
            }
      return al;
      }

//---------------------------------------------------------
//   deviceList
//---------------------------------------------------------

QStringList Portaudio::deviceList(int apiIdx)
      {
      QStringList dl;
      const PaHostApiInfo* info = Pa_GetHostApiInfo(apiIdx);
      if (info) {
            for (int i = 0; i < info->deviceCount; ++i) {
                  PaDeviceIndex idx = Pa_HostApiDeviceIndexToDeviceIndex(apiIdx, i);
                  const PaDeviceInfo* di = Pa_GetDeviceInfo(idx);
                  if (di)
                        dl.append(di->name);
                  }
            }
      return dl;
      }

//---------------------------------------------------------
//   deviceIndex
//---------------------------------------------------------

int Portaudio::deviceIndex(int apiIdx, int apiDevIdx)
      {
      return Pa_HostApiDeviceIndexToDeviceIndex(apiIdx, apiDevIdx);
      }

//---------------------------------------------------------
//   registerPort
//---------------------------------------------------------

void Portaudio::registerPort(const QString&, bool, bool)
      {
      }

//---------------------------------------------------------
//   unregisterPort
//---------------------------------------------------------

void Portaudio::unregisterPort(int)
      {
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

QList<QString> Portaudio::inputPorts()
      {
      QList<QString> clientList;
      return clientList;
      }

//---------------------------------------------------------
//   connect
//---------------------------------------------------------

void Portaudio::connect(void* /*src*/, void* /*dst*/)
      {
      }

//---------------------------------------------------------
//   disconnect
//---------------------------------------------------------

void Portaudio::disconnect(void* /*src*/, void* /*dst*/)
      {
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

bool Portaudio::start()
      {
      PaError err = Pa_StartStream(stream);
      if (err != paNoError) {
            qDebug("Portaudio: start stream failed: %s", Pa_GetErrorText(err));
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

bool Portaudio::stop()
      {
      PaError err = Pa_StopStream(stream);      // sometimes the program hangs here on exit
      if (err != paNoError) {
            qDebug("Portaudio: stop failed: %s", Pa_GetErrorText(err));
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   framePos
//---------------------------------------------------------

int Portaudio::framePos() const
      {
      return 0;
      }

//---------------------------------------------------------
//   startTransport
//---------------------------------------------------------

void Portaudio::startTransport()
      {
      state = Seq::TRANSPORT_PLAY;
      }

//---------------------------------------------------------
//   stopTransport
//---------------------------------------------------------

void Portaudio::stopTransport()
      {
      state = Seq::TRANSPORT_STOP;
      }

//---------------------------------------------------------
//   getState
//---------------------------------------------------------

int Portaudio::getState()
      {
      return state;
      }

//---------------------------------------------------------
//   midiRead
//---------------------------------------------------------

void Portaudio::midiRead()
      {
      if (midiDriver)
            midiDriver->read();
      }

//---------------------------------------------------------
//   currentApi
//---------------------------------------------------------

int Portaudio::currentApi() const
      {
      PaDeviceIndex idx = preferences.portaudioDevice;
      if (idx < 0)
            idx = Pa_GetDefaultOutputDevice();

      for (int api = 0; api < Pa_GetHostApiCount(); ++api) {
            const PaHostApiInfo* info = Pa_GetHostApiInfo(api);
            if (info) {
                  for (int k = 0; k < info->deviceCount; ++k) {
                        PaDeviceIndex i = Pa_HostApiDeviceIndexToDeviceIndex(api, k);
                        if (i == idx)
                              return api;
                        }
                  }
            }
      qDebug("Portaudio: no current api found for device %d", idx);
      return -1;
      }

//---------------------------------------------------------
//   currentDevice
//---------------------------------------------------------

int Portaudio::currentDevice() const
      {
      PaDeviceIndex idx = preferences.portaudioDevice;
      if (idx < 0)
            idx = Pa_GetDefaultOutputDevice();

      for (int api = 0; api < Pa_GetHostApiCount(); ++api) {
            const PaHostApiInfo* info = Pa_GetHostApiInfo(api);
            if (info) {
                  for (int k = 0; k < info->deviceCount; ++k) {
                        PaDeviceIndex i = Pa_HostApiDeviceIndexToDeviceIndex(api, k);
                        if (i == idx)
                              return k;
                        }
                  }
            }
      qDebug("Portaudio: no current ApiDevice found for device %d", idx);
      return -1;
      }
}

