//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
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

class Event;
struct Channel;

//---------------------------------------------------------
//   Sequencer
//---------------------------------------------------------

class Sequencer {
   public:
      Sequencer() {}
      virtual ~Sequencer() {}

      virtual void sendEvent(const Event&) = 0;
      virtual void startNote(const Channel&, int, int, int, double nt) = 0;
      };
#endif

