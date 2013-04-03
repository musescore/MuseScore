//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "config.h"
#include "preferences.h"
#include "driver.h"

#ifdef USE_JACK
#include "jackaudio.h"
#endif

#ifdef USE_PULSEAUDIO
extern Driver* getPulseAudioDriver(Seq*);
#endif

#ifdef USE_ALSA
#include "alsa.h"
#endif
#ifdef USE_PORTAUDIO
#include "pa.h"
#endif

//---------------------------------------------------------
//   driverFactory
//---------------------------------------------------------

Driver* driverFactory(Seq* seq)
      {
      Driver* driver = 0;

#define useJackFlag       (preferences.useJackAudio || preferences.useJackMidi)
#define useAlsaFlag       preferences.useAlsaAudio
#define usePortaudioFlag  preferences.usePortaudioAudio
#define usePulseAudioFlag preferences.usePulseAudio

#ifdef USE_PULSEAUDIO
      if (usePulseAudioFlag) {
            driver = getPulseAudioDriver(seq);
            if (!driver->init()) {
                  qDebug("init PulseAudio failed");
                  delete driver;
                  driver = 0;
                  }
            else
                  usePortaudio = true;
            }
#endif
#ifdef USE_PORTAUDIO
      if (usePortaudioFlag) {
            driver = new Portaudio(seq);
            if (!driver->init()) {
                  qDebug("init PortAudio failed");
                  delete driver;
                  driver = 0;
                  }
            else
                  usePortaudio = true;
            }
#endif
#ifdef USE_ALSA
      if (driver == 0 && useAlsaFlag) {
            driver = new AlsaAudio(seq);
            if (!driver->init()) {
                  qDebug("init ALSA driver failed\n");
                  delete driver;
                  driver = 0;
                  }
            else {
                  useALSA = true;
                  }
            }
#endif
#ifdef USE_JACK
      if (useJackFlag) {
            useAlsaFlag      = false;
            usePortaudioFlag = false;
            driver = new JackAudio(seq);
            if (!driver->init()) {
                  qDebug("no JACK server found\n");
                  delete driver;
                  driver = 0;
                  }
            else
                  useJACK = true;
            }
#endif
      if (driver == 0)
            qDebug("no audio driver found");

      return driver;
      }


