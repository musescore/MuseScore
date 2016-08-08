//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __SEQUENCER_H__
#define __SEQUENCER_H__

namespace Ms {

class NPlayEvent;

enum class BeatType : char;

//---------------------------------------------------------
//   Sequencer
//---------------------------------------------------------

class Sequencer {
   public:
      Sequencer() {}
      virtual ~Sequencer() {}

      virtual void sendEvent(const NPlayEvent&) = 0;
      virtual void startNote(int channel, int, int, double nt) = 0;
      virtual void startNote(int channel, int, int, int, double nt) = 0;
      virtual void playMetronomeBeat(BeatType type) = 0;
      };

}     // namespace Ms
#endif

