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

#include "types.h"
#include "timemap.h"

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

class TempoMap : private TimeMap<TEvent> {
      typedef TimeMap<TEvent> M;
      int _tempoSN;           // serial no to track tempo changes
      qreal _tempo;           // tempo if not using tempo list (beats per second)
      qreal _relTempo;        // rel. tempo

      void normalize();
      void del(const Fraction& tick);

   public:
      TempoMap();
      void clear();
      void clearRange(const Fraction& tick1, const Fraction& tick2);

      // expose some part of TimeMap interface directly
      using M::cbegin;
      using M::cend;
      using M::empty;

      M::const_iterator begin() const { return M::begin(); }
      M::const_iterator end() const { return M::cend(); }
      M::const_iterator lower_bound(const Fraction& tick) const { return M::lower_bound(tick); }
      M::const_iterator upper_bound(const Fraction& tick) const { return M::upper_bound(tick); }

      void dump() const;

      qreal tempo(const Fraction& tick) const;
      bool hasEventAt(const Fraction& tick) const { return M::hasChangeAt(tick); }

      qreal tick2time(const Fraction& tick, int* sn = 0) const;
      qreal tick2timeLC(const Fraction& tick, int* sn) const;
      qreal tick2time(const Fraction& tick, qreal time, int* sn) const;
      Fraction time2tick(qreal time, int* sn = 0) const;
      Fraction time2tick(qreal time, const Fraction& tick, int* sn) const;
      int tempoSN() const { return _tempoSN; }

      void setTempo(const Fraction& t, qreal);
      void setPause(const Fraction& t, qreal);
      void setEvent(const Fraction& t, const TEvent& evt) { M::insert(t, evt); normalize(); }
      void delTempo(const Fraction& tick);


      void setRelTempo(qreal val);
      qreal relTempo() const { return _relTempo; }
      };

}     // namespace Ms
#endif
