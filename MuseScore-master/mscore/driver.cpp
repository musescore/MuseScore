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


#ifdef USE_ALSA
#include "alsa.h"
#endif
#ifdef USE_PORTAUDIO
#include "pa.h"
#endif

namespace Ms {

#ifdef USE_PULSEAUDIO
extern Driver* getPulseAudioDriver(Seq*);
#endif

//---------------------------------------------------------
//   driverFactory
//    driver can be: jack alsa pulse portaudio
//---------------------------------------------------------

Driver* driverFactory(Seq* seq, QString driverName)
      {
      Driver* driver = 0;
#if 1 // DEBUG: force "no audio"
      bool useJackFlag       = (preferences.useJackAudio || preferences.useJackMidi);
      bool useAlsaFlag       = preferences.useAlsaAudio;
      bool usePortaudioFlag  = preferences.usePortaudioAudio;
      bool usePulseAudioFlag = preferences.usePulseAudio;

      if (!driverName.isEmpty()) {
            driverName        = driverName.toLower();
            useJackFlag       = false;
            useAlsaFlag       = false;
            usePortaudioFlag  = false;
            usePulseAudioFlag = false;
            if (driverName == "jack")
                  useJackFlag = true;
            else if (driverName == "alsa")
                  useAlsaFlag = true;
            else if (driverName == "pulse")
                  usePulseAudioFlag = true;
            else if (driverName == "portaudio")
                  usePortaudioFlag = true;
            }

      useALSA       = false;
      useJACK       = false;
      usePortaudio  = false;
      usePulseAudio = false;

#ifdef USE_PULSEAUDIO
      if (usePulseAudioFlag) {
            driver = getPulseAudioDriver(seq);
            if (!driver->init()) {
                  qDebug("init PulseAudio failed");
                  delete driver;
                  driver = 0;
                  }
            else
                  usePulseAudio = true;
            }
#else
      (void)usePulseAudioFlag; // avoid compiler warning
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
#else
      (void)usePortaudioFlag; // avoid compiler warning
#endif
#ifdef USE_ALSA
      if (driver == 0 && useAlsaFlag) {
            driver = new AlsaAudio(seq);
            if (!driver->init()) {
                  qDebug("init ALSA driver failed");
                  delete driver;
                  driver = 0;
                  }
            else {
                  useALSA = true;
                  }
            }
#else
      (void)useAlsaFlag; // avoid compiler warning
#endif
#ifdef USE_JACK
      if (useJackFlag) {
            useAlsaFlag      = false;
            usePortaudioFlag = false;
            driver = new JackAudio(seq);
            if (!driver->init()) {
                  qDebug("no JACK server found");
                  delete driver;
                  driver = 0;
                  }
            else
                  useJACK = true;
            }
#else
       (void)useJackFlag; // avoid compiler warning
#endif
#endif
      if (driver == 0)
            qDebug("no audio driver found");

      return driver;
      }

}

