//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PITCHVALUE_H__
#define __PITCHVALUE_H__

//---------------------------------------------------------
//   PitchValue
//    used in class Bend, BendCanvas
//
//    - time is 0 - 60 for 0-100% of the chord duration the
//      bend is attached to
//    - pitch is 100 for one semitone
//---------------------------------------------------------

struct PitchValue {
      int time;
      int pitch;
      bool vibrato;
      PitchValue() {}
      PitchValue(int a, int b, bool c = false) : time(a), pitch(b), vibrato(c) {}
      };

#endif

