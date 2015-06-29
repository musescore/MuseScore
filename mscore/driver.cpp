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
#include "libmscore/score.h"
#include "seq.h"

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

//---------------------------------------------------------
//   readMMC
//   reads MIDI Machine Control messages
//---------------------------------------------------------

void Driver::readMMC(int type, int len, int* data)
      {
      if (!preferences.acceptMMCmessages)
            return;

      switch (type) {
            case ME_START:
                  seq->seekRT(0);
                  if (preferences.useJackTransport)
                        seekTransport(0);
                  startTransport();
            break;
            case ME_STOP:
                  stopTransport();
            break;
            case ME_CONTINUE:
                  startTransport();
            break;
            case ME_SONGPOS: {
                  int utick = (128 * data[1] + data[0]) * MScore::division / 4.0;
                  seq->accurateSeek(utick);
                  if (preferences.useJackTransport)
                        seekTransport(utick);
                  }
            break;
            case ME_SYSEX:
                  // MIDI Machine Control (MMC)
                  if (data[0] == 0x7F) {
                        int deviceId = data[1];
                        if (deviceId != preferences.mmcDeviceId && preferences.mmcDeviceId != 127)
                              break;
                        // Goto MMC message
                        // 0xF0 0x7F <deviceID> 0x06 0x44 0x06 0x01 <hr> <mn> <sc> <fr> <ff> 0xF7
                        if (len == 12 && data[2] == 0x06 && data[3] == 0x44
                                      && data[4] == 0x06 && data[5] == 0x01) {
                              int hour     = data[6] & 0x1f;
                              int minute   = data[7];
                              int second   = data[8];
                              int frameNumber      = data[9];
                              int subFrameNumber   = data[10];

                              qreal utime = hour*3600 + minute*60 + second + (qreal)frameNumber/30.0 + (qreal)subFrameNumber/300.0;
                              int utick = seq->score()->utime2utick(utime);
                              seq->accurateSeek(utick);
                              if (preferences.useJackTransport)
                                    seekTransport(utick);
                              break;
                              }

                        // MMC format:
                        // 0xF0 0x7F <deviceID> 0x06 <command> 0xF7
                        if (data[2] != 0x06)
                              break;
                        int command = data[3];
                        switch (command) {
                              case 1: // Stop
                              case 9: // Pause
                                    stopTransport();
                              break;
                              case 2: // Play
                              case 3: // Deferred Play
                                    startTransport();
                              break;
                              case 4: // Fast Forward
                                      // TODO
                              break;
                              case 5: // Rewind
                                    seq->seekRT(0);  // Don't use seq->rewindStart() - it's for UI thread
                                    if (preferences.useJackTransport)
                                          seekTransport(0);
                              break;
                              }
                        }
            break;
            }
      }
}
