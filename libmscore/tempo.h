//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __AL_TEMPO_H__
#define __AL_TEMPO_H__

#include "repeatlist.h"

namespace Ms {

class RepeatSegment;
class Xml;

enum class TempoType : char {
      INVALID           = 0x0,
      PAUSE_BEFORE_TICK = 0x1,
      FIX               = 0x2,
      RAMP              = 0x4,
      PAUSE_THROUGH_TICK= 0x8,
      PAUSE_AFTER_TICK  = 0x10,
      PAUSE_ANY = PAUSE_BEFORE_TICK | PAUSE_THROUGH_TICK | PAUSE_AFTER_TICK
      };

Q_DECLARE_FLAGS(TempoTypes, TempoType);
Q_DECLARE_OPERATORS_FOR_FLAGS(TempoTypes);

//---------------------------------------------------------
//   Tempo Event
//---------------------------------------------------------

struct TEvent {
      TempoTypes type;
      qreal tempo;     // beats per second
      qreal time;      // precomputed time for tick in sec

      // may have both types of pauses on the same tick
      qreal pauseBeforeTick;  // pause in seconds just before the tick
      qreal pauseThroughTick; // pause in seconds precisely at the tick

      TEvent();
      TEvent(const TEvent& e);
      TEvent(qreal tempo, qreal pauseBeforeTick, qreal pauseThroughTick, TempoType type);
      bool valid() const;
      void dump() const;
      };

//---------------------------------------------------------
//   Tempomap
//---------------------------------------------------------

class TempoMap : public std::map<int, TEvent> {
      int _tempoSN;           // serial no to track tempo changes
      qreal _tempo;           // tempo if not using tempo list (beats per second)
      qreal _relTempo;        // rel. tempo

      void normalize();
      void del(int tick);

   public:
      TempoMap();
      TempoMap( const RepeatList* repeatList, const TempoMap* graphicalTempoMap ); // creates unrolled tempomap

      void clear();

      void dump() const;

      qreal tempo(int tick) const;

      qreal tick2time(int tick, int* sn = 0) const;
      qreal tick2timeLC(int tick, int* sn) const;
      qreal tick2time(int tick, qreal time, int* sn) const;
      int time2tick(qreal time, int* sn = 0) const;
      int time2tick(qreal time, int tick, int* sn) const;
      int tempoSN() const { return _tempoSN; }

      void setPauseBeforeTick(int tick, qreal seconds);
      void setPauseThroughTick(int tick, qreal seconds);
      void setTempo(int tick, qreal tempo);
      void delTempo(int tick);

      void setRelTempo(qreal val);
      qreal relTempo() const { return _relTempo; }
      };

}     // namespace Ms
#endif
