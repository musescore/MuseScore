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

namespace Ms {

class XmlWriter;

enum class TempoType : char { INVALID = 0x0, PAUSE = 0x1, FIX = 0x2, RAMP = 0x4};

typedef QFlags<TempoType> TempoTypes;
Q_DECLARE_OPERATORS_FOR_FLAGS(TempoTypes);

//---------------------------------------------------------
//   Tempo Event
//---------------------------------------------------------

struct TEvent {
      TempoTypes type;
      qreal tempo;     // beats per second
      qreal pause;     // pause in seconds
      qreal time;      // precomputed time for tick in sec

      TEvent();
      TEvent(const TEvent& e);
      TEvent(qreal bps, qreal seconds, TempoType t);
      bool valid() const;
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
      void clear();
      void clearRange(int tick1, int tick2);

      void dump() const;

      qreal tempo(int tick) const;

      qreal tick2time(int tick, int* sn = 0) const;
      qreal tick2timeLC(int tick, int* sn) const;
      qreal tick2time(int tick, qreal time, int* sn) const;
      int time2tick(qreal time, int* sn = 0) const;
      int time2tick(qreal time, int tick, int* sn) const;
      int tempoSN() const { return _tempoSN; }

      void setTempo(int t, qreal);
      void setPause(int t, qreal);
      void delTempo(int tick);

      void setRelTempo(qreal val);
      qreal relTempo() const { return _relTempo; }
      };

}     // namespace Ms
#endif
