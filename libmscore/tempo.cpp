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

#include "tempo.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   TEvent
//---------------------------------------------------------

TEvent::TEvent()
      {
      type     = TempoType::INVALID;
      tempo    = 0.0;
      pause    = 0.0;
      }

TEvent::TEvent(const TEvent& e)
      {
      type  = e.type;
      tempo = e.tempo;
      pause = e.pause;
      time  = e.time;
      }

TEvent::TEvent(qreal t, qreal p, TempoType tp)
      {
      type  = tp;
      tempo = t;
      pause = p;
      time  = 0.0;
      }

bool TEvent::valid() const
      {
      return !(!type);
      }

//---------------------------------------------------------
//   TempoMap
//---------------------------------------------------------

TempoMap::TempoMap()
      {
      _tempo    = 2.0;        // default fixed tempo in beat per second
      _tempoSN  = 1;
      _relTempo = 1.0;
      }

//---------------------------------------------------------
//   setPause
//---------------------------------------------------------

void TempoMap::setPause(const Fraction& tick, qreal pause)
      {
      auto e = find(TimePosition(tick));
      if (e != end()) {
            e->second.pause = pause;
            e->second.type |= TempoType::PAUSE;
            }
      else {
            qreal t = tempo(tick);
            insert(std::pair<const TimePosition, TEvent> (TimePosition(tick), TEvent(t, pause, TempoType::PAUSE)));
            }
      normalize();
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void TempoMap::setTempo(const Fraction& tick, qreal tempo)
      {
      auto e = find(tick);
      if (e != end()) {
            e->second.tempo = tempo;
            e->second.type |= TempoType::FIX;
            }
      else
            insert(std::pair<const TimePosition, TEvent> (TimePosition(tick), TEvent(tempo, 0.0, TempoType::FIX)));
      normalize();
      }

//---------------------------------------------------------
//   TempoMap::normalize
//---------------------------------------------------------

void TempoMap::normalize()
      {
      qreal time  = 0;
      Fraction tick { 0, 1 };
      qreal tempo = 2.0;
      for (auto e = begin(); e != end(); ++e) {
            // entries that represent a pause *only* (not tempo change also)
            // need to be corrected to continue previous tempo
            if (!(e->second.type & (TempoType::FIX|TempoType::RAMP)))
                  e->second.tempo = tempo;
            Fraction delta = e->first.tick() - tick;
            time += qreal(delta.ticks()) / (MScore::division * tempo * _relTempo);   // TODO: simplify
            time += e->second.pause;
            e->second.time = time;
            tick  = e->first.tick();
            tempo = e->second.tempo;
            }
      ++_tempoSN;
      }

//---------------------------------------------------------
//   TempoMap::dump
//---------------------------------------------------------

void TempoMap::dump() const
      {
      qDebug("\nTempoMap:");
      for (auto i = begin(); i != end(); ++i)
            qDebug("%6d type: %2d tempo: %f pause: %f time: %f",
               i->first.tick().ticks(), static_cast<int>(i->second.type), i->second.tempo, i->second.pause, i->second.time);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void TempoMap::clear()
      {
      std::map<TimePosition,TEvent>::clear();
      ++_tempoSN;
      }

//---------------------------------------------------------
//   clearRange
//    Clears the given range, start tick included, end tick
//    excluded.
//---------------------------------------------------------

void TempoMap::clearRange(const Fraction& tick1, const Fraction& tick2)
      {
      iterator first = lower_bound(TimePosition(tick1));
      iterator last  = lower_bound(TimePosition(tick2));
      if (first == last)
            return;
      erase(first, last);
      ++_tempoSN;
      }

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

qreal TempoMap::tempo(const Fraction& tick) const
      {
      if (empty())
            return 2.0;
      auto i = lower_bound(TimePosition(tick));
      if (i == end()) {
            --i;
            return i->second.tempo;
            }
      if (i->first.tick() == tick)
            return i->second.tempo;
      if (i == begin())
            return 2.0;
      --i;
      return i->second.tempo;
      }

//---------------------------------------------------------
//   del
//---------------------------------------------------------

void TempoMap::del(const Fraction& tick)
      {
      auto e = find(TimePosition(tick));
      if (e == end()) {
            qDebug("TempoMap::del event at (%d): not found", tick.ticks());
            // abort();
            return;
            }
      // don't delete event if still being used for pause
      if (e->second.type & TempoType::PAUSE)
            e->second.type = TempoType::PAUSE;
      else
            erase(e);
      normalize();
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void TempoMap::setRelTempo(qreal val)
      {
      _relTempo = val;
      normalize();
      }

//---------------------------------------------------------
//   delTempo
//---------------------------------------------------------

void TempoMap::delTempo(const Fraction& tick)
      {
      del(tick);
      ++_tempoSN;
      }

//---------------------------------------------------------
//   tick2time
//---------------------------------------------------------

qreal TempoMap::tick2time(const Fraction& tick, qreal time, int* sn) const
      {
      return (*sn == _tempoSN) ? time : tick2time(tick, sn);
      }

//---------------------------------------------------------
//   time2tick
//    return cached value t if list did not change
//---------------------------------------------------------

Fraction TempoMap::time2tick(qreal time, const Fraction& t, int* sn) const
      {
      return (*sn == _tempoSN) ? t : time2tick(time, sn);
      }

//---------------------------------------------------------
//   tick2time
//---------------------------------------------------------

qreal TempoMap::tick2time(const Fraction& tick, int* sn) const
      {
      qreal time  = 0.0;
      qreal delta = qreal(tick.ticks());
      qreal tempo = 2.0;

      if (!empty()) {
            int ptick  = 0;
            auto e = lower_bound(TimePosition(tick));
            if (e == end()) {
                  auto pe = e;
                  --pe;
                  ptick = pe->first.tick().ticks();
                  tempo = pe->second.tempo;
                  time  = pe->second.time;
                  }
            else if (e->first.tick() == tick) {
                  ptick = tick.ticks();
                  tempo = e->second.tempo;
                  time  = e->second.time;
                  }
            else if (e != begin()) {
                  auto pe = e;
                  --pe;
                  ptick = pe->first.tick().ticks();
                  tempo = pe->second.tempo;
                  time  = pe->second.time;
                  }
            delta = qreal(tick.ticks() - ptick);
            }
      else
            qDebug("TempoMap: empty");
      if (sn)
            *sn = _tempoSN;
      time += delta / (MScore::division * tempo * _relTempo);
      return time;
      }

//---------------------------------------------------------
//   time2tick
//---------------------------------------------------------

Fraction TempoMap::time2tick(qreal time, int* sn) const
      {
      Fraction tick { 0, 1 };
      qreal delta = time;
      qreal tempo = _tempo;

      delta = 0.0;
      tempo = 2.0;
      for (auto e = begin(); e != end(); ++e) {
            // if in a pause period, wait on previous tick
            if ((time <= e->second.time) && (time > e->second.time - e->second.pause)) {
                  delta = (time - (e->second.time - e->second.pause) + delta);
                  break;
                  }
            if (e->second.time >= time)
                  break;
            delta = e->second.time;
            tick  = e->first.tick();
            tempo = e->second.tempo;
            }
      delta = time - delta;
      tick += Fraction::fromTicks(lrint(delta * _relTempo * MScore::division * tempo));
      if (sn)
            *sn = _tempoSN;
      return tick;
      }

}

