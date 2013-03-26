//=============================================================================
//  MuseSynth
//  Music Software Synthesizer
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <stdio.h>
#include "zone.h"
#include "channel.h"

//---------------------------------------------------------
//   Zone
//---------------------------------------------------------

Zone::Zone()
      {
      for (int i = 0; i < 128; ++i) {
            onLocc[i] = -1;
            onHicc[i] = -1;
            locc[i]    = 0;
            hicc[i]    = 127;
            }
      }

//---------------------------------------------------------
//   match
//---------------------------------------------------------

bool Zone::match(Channel* c, int k, int v, Trigger et)
      {
      int cc64 = c->sustain();

      if ((k >= keyLo)
         && (k <= keyHi)
         && (v >= veloLo)
         && (v <= veloHi)
         && (seq == seqPos)
         && (et == trigger)
         && (cc64 >= locc[64] && cc64 <= hicc[64])
         ) {
//printf("   Zone match %d %d %d -- %d %d  %d %d  center %d trigger %d\n",
//         k, v, et, keyLo, keyHi, veloLo, veloHi, keyBase, trigger);
            if (et == Trigger::ATTACK) {
                  ++seq;
                  if (seq >= seqLen)
                        seq = 0;
                  }
            return true;
            }
      return false;
      }
