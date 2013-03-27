//=============================================================================
//  Zerberus
//  Zample player
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "midievent.h"
#include "zerberus.h"
#include "channel.h"
#include "voice.h"

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

Channel::Channel(Zerberus* ms, int i)
      {
      _msynth     = ms;
      _idx        = i;
      _instrument = 0;
      _sustain    = 0;
      _gain       = 1.0;
      }

//---------------------------------------------------------
//   pitchBend
//---------------------------------------------------------

void Channel::pitchBend(int)
      {
      }

//---------------------------------------------------------
//   controller
//    realtime
//---------------------------------------------------------

void Channel::controller(int ctrl, int val)
      {
      if (ctrl == CTRL_SUSTAIN) {
            _sustain = val;
            if (_sustain < 0x40) {
                  for (Voice* v = _msynth->getActiveVoices(); v; v = v->next()) {
                        if (v->isSustained()) {
                              // printf("sustain off %p\n", v);
                              v->stop();
                              }
                        }
                  }
            }
      else if (ctrl == CTRL_ALL_NOTES_OFF) {
            for (Voice* v = _msynth->getActiveVoices(); v; v = v->next()) {
                  if (!v->isOff())
                        v->stop();
                  }
            }
      else if (ctrl == CTRL_PROGRAM) {
            printf("Zerberus: program %d\n", val);
            ZInstrument* zi = _msynth->instrument(val);
            if (zi == 0)
                  printf("   not found\n");
            if (zi && zi != _instrument) {
                  for (Voice* v = _msynth->getActiveVoices(); v; v = v->next())
                        v->off();
                  _instrument = zi;
                  }
            }
//      else
//            qDebug("Zerberus: ctrl 0x%02x 0x%02x", ctrl, val);
      }

